/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Base_Windows.cpp:  Windows specific stuff

  Copyright (c) 2001-2007, Hugh Bailey
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * The name of Hugh Bailey may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

  THIS SOFTWARE IS PROVIDED BY HUGH BAILEY "AS IS" AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL HUGH BAILEY BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUISNESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Blah blah blah blah.  To the fricken' code already!
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifdef WIN32

#define _WIN32_WINDOWS 0x0500
#define _WIN32_WINNT   0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Base.h"



void ENGINEAPI ProcessStandardInput(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void ENGINEAPI SIWindowActivate(BOOL bActive);
void ENGINEAPI DIWindowActivate(BOOL bActive);
void ENGINEAPI ResetCursorClip(HANDLE window);


HANDLE      hwndMain        = NULL;
HANDLE      hinstMain       = NULL;
HANDLE      hBaseMutex      = NULL;
DWORD       dwIconID        = 0;

BOOL        bHidingCursor   = FALSE;
BOOL        bClippingCursor = FALSE;
BOOL        bWindowActive   = TRUE;
BOOL        bSetColorRamp   = FALSE;
BOOL        bFullscreen     = FALSE;

LARGE_INTEGER clockFreq, startTime;
DWORD startTick = 0;
LONGLONG prevElapsedTime = 0;

BOOL        bWasClippingCursor = FALSE;

int         windowX=0,
            windowY=0,
            windowCX=0,
            windowCY=0;

DEVMODE     curDevMode;

float       curGamma=1.0f, curBrightness=1.0f, curContrast=1.0f;

TSTR        *lpCommandLine = NULL;
int         numArgsUsed = 0;


BOOL ENGINEAPI DllMain(HINSTANCE hInst, DWORD dwWhy, LPVOID lpReserved1)
{
    switch(dwWhy)
    {
        case DLL_PROCESS_ATTACH:
            {
                QueryPerformanceFrequency(&clockFreq);
                QueryPerformanceCounter(&startTime);
                startTick = GetTickCount();

                srand(startTime.LowPart);

                BOOL bError = SetProcessWorkingSetSize(GetCurrentProcess(), (5*1024*1024), (50*1024*1024));
                break;
            }

        case DLL_PROCESS_DETACH:
            {
                if(hBaseMutex)
                    ReleaseMutex(hBaseMutex);

                if(bBaseLoaded)
                    CriticalExit();

                if(lpCommandLine)
                {
                    for(int i=0; i<numArgsUsed; i++)
                        free(lpCommandLine[i]);
                    free(lpCommandLine);
                }
            }
    }

    return TRUE;
}

void __cdecl OSMessageBoxva(const TCHAR *format, va_list argptr)
{
    TCHAR blah[4096];
    vtsprintf_s(blah, 4095, format, argptr);

    if(engine) engine->bBlockViewUpdates++;

    MessageBox((hwndMain != NULL) ? (HWND)hwndMain : NULL, blah, NULL, MB_ICONWARNING);

    if(engine) engine->bBlockViewUpdates--;
}

void __cdecl OSMessageBox(const TCHAR *format, ...)
{
    va_list arglist;

    va_start(arglist, format);

    OSMessageBoxva(format, arglist);
}


void __cdecl OSDebugOutva(const TCHAR *format, va_list argptr)
{
    TCHAR blah[4096];
    vtsprintf_s(blah, 4095, format, argptr);

    OutputDebugString(blah);
}

void __cdecl OSDebugOut(const TCHAR *format, ...)
{
    va_list arglist;

    va_start(arglist, format);

    OSDebugOutva(format, arglist);
}


BOOL   ENGINEAPI ProgramOpened(TSTR name)
{
    hBaseMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, name);
    if(hBaseMutex)
    {
        CloseHandle(hBaseMutex);
        return TRUE;
    }
    else
    {
        hBaseMutex = CreateMutex(NULL, TRUE, name);
        return FALSE;
    }
}

void   ENGINEAPI OSLogSystemStats()
{
    HKEY key;
    TCHAR data[1024];
    DWORD dwSize, dwSpeed;

    zero(data, 1024);

    if(RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"), &key) != ERROR_SUCCESS)
    {
        AppWarning(TEXT("Could not open system information registry key"));
        return;
    }

    dwSize = 1024;
    RegQueryValueEx(key, TEXT("ProcessorNameString"), NULL, NULL, (LPBYTE)data, &dwSize);
    Log(TEXT("CPU Name: %s"), sfix(data));

    dwSize = 4;
    RegQueryValueEx(key, TEXT("~MHz"), NULL, NULL, (LPBYTE)&dwSpeed, &dwSize);
    Log(TEXT("CPU Speed: %dMHz"), dwSpeed);

    RegCloseKey(key);

    MEMORYSTATUS ms;

    GlobalMemoryStatus(&ms);

    Log(TEXT("Physical Memory:  %ldMB Total, %ldMB Free"), (ms.dwTotalPhys/1048576), (ms.dwAvailPhys/1048576));
}

DWORD  ENGINEAPI OSGetSysPageSize()
{
    SYSTEM_INFO SI;
    GetSystemInfo(&SI);
    return SI.dwPageSize;
}

LPVOID ENGINEAPI OSVirtualAlloc(size_t dwSize)
{
    return VirtualAlloc(NULL, dwSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

void   ENGINEAPI OSVirtualFree(LPVOID lpData)
{
    VirtualFree(lpData, 0, MEM_RELEASE);
}

void   ENGINEAPI OSCriticalExit()
{
    TerminateProcess(GetCurrentProcess(), -1);
}

BOOL   ENGINEAPI OSDeleteFile(CTSTR lpFile)
{
    return DeleteFile(lpFile);
}

BOOL   ENGINEAPI OSCopyFile(CTSTR lpFileDest, CTSTR lpFileSrc)
{
    return CopyFile(lpFileSrc, lpFileDest, TRUE);
}

BOOL   ENGINEAPI OSCreateDirectory(CTSTR lpDirectory)
{
    return CreateDirectory(lpDirectory, NULL);
}

BOOL   ENGINEAPI OSSetCurrentDirectory(CTSTR lpDirectory)
{
    return SetCurrentDirectory(lpDirectory);
}

BOOL   ENGINEAPI OSFileExists(CTSTR lpFile)
{
    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(lpFile, ofd);
    if(hFind)
    {
        OSFindClose(hFind);
        return TRUE;
    }

    return FALSE;
}

BOOL   ENGINEAPI OSGetFileTime(CTSTR lpFile, OSFileTime &fileTime)
{
    HANDLE hFile = CreateFile(lpFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    FILETIME winFileCreated, winFileModified;
    BOOL bSuccess;

    if(bSuccess = GetFileTime(hFile, &winFileCreated, NULL, &winFileModified))
    {
        SYSTEMTIME sysTime;

        FileTimeToSystemTime(&winFileCreated, &sysTime);
        fileTime.timeCreated.year = sysTime.wYear;
        fileTime.timeCreated.month = sysTime.wMonth;
        fileTime.timeCreated.dayOfWeek = sysTime.wDayOfWeek;
        fileTime.timeCreated.day = sysTime.wDay;
        fileTime.timeCreated.hour = sysTime.wHour;
        fileTime.timeCreated.minute = sysTime.wMinute;
        fileTime.timeCreated.second = sysTime.wSecond;
        fileTime.timeCreated.millisecond = sysTime.wMilliseconds;

        FileTimeToSystemTime(&winFileModified, &sysTime);
        fileTime.timeModified.year = sysTime.wYear;
        fileTime.timeModified.month = sysTime.wMonth;
        fileTime.timeModified.dayOfWeek = sysTime.wDayOfWeek;
        fileTime.timeModified.day = sysTime.wDay;
        fileTime.timeModified.hour = sysTime.wHour;
        fileTime.timeModified.minute = sysTime.wMinute;
        fileTime.timeModified.second = sysTime.wSecond;
        fileTime.timeModified.millisecond = sysTime.wMilliseconds;
    }

    return bSuccess;
}


int    ENGINEAPI OSProcessEvent()
{
    MSG msg;

    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            if(msg.wParam == 0)
                bExiting = TRUE;
            return 0;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            return 2;
        }
    }
    else
        return 1;
}

TSTR*  ENGINEAPI OSGetCommandLine(int &numArgs)
{
    if(!lpCommandLine)
    {
        BOOL bNoAlloc = !MainAllocator;
        if(bNoAlloc)
        {
            MainAllocator = new DefaultAlloc;
        }

        {
            //lpCommandLine = CommandLineToArgvW(GetCommandLine(), &numArgsUsed);
            CTSTR lpBareCommandLine = GetCommandLine();
            CTSTR lpTemp = lpBareCommandLine;

            StringList commandList;

            while(lpTemp && *lpTemp)
            {
                CTSTR lpNext;
                if(*lpTemp == '"')
                {
                    ++lpTemp;
                    lpNext = schr(lpTemp, '"');
                }
                else if(*lpTemp == ' ')
                {
                    ++lpTemp;
                    continue;
                }
                else
                    lpNext = schr(lpTemp, ' ');

                if(!lpNext)
                {
                    commandList << lpTemp;
                    break;
                }
                else
                    commandList << GetStringSection(lpTemp, lpNext);

                lpTemp = lpNext+1;
            }

            lpCommandLine = (TSTR*)malloc(commandList.Num()*sizeof(TSTR));
            for(int i=0; i<commandList.Num(); i++)
            {
                lpCommandLine[i] = (TSTR)malloc(commandList[i].DataLength());
                mcpy(lpCommandLine[i], commandList[i].Array(), commandList[i].DataLength());
            }

            numArgsUsed = commandList.Num();
        }

        if(bNoAlloc)
        {
            delete MainAllocator;
            MainAllocator = NULL;
        }
    }

    numArgs = numArgsUsed;
    return lpCommandLine;
}


HANDLE ENGINEAPI OSCreateMainWindow(CTSTR lpTitle, int cx, int cy, BOOL bResizable)
{
    WNDCLASS wc;

    zero(&wc, sizeof(wc));

    if(FindWindow(ENGINE_WINDOW_CLASS_NAME, NULL)) return 0;

    cy += GetSystemMetrics(SM_CYCAPTION);

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.hInstance = (HINSTANCE)hinstMain;
    if(!dwIconID)
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    else
        wc.hIcon = LoadIcon((HINSTANCE)hinstMain, MAKEINTRESOURCE(dwIconID));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = ENGINE_WINDOW_CLASS_NAME;
    wc.cbWndExtra = sizeof(LPVOID);

    if(!RegisterClass(&wc)) {CrashError(TEXT("Could not register the window class")); return 0;}

    windowCX = cx;
    windowCY = cy;

    int x = (GetSystemMetrics(SM_CXFULLSCREEN)/2)-(cx/2);
    int y = (GetSystemMetrics(SM_CYFULLSCREEN)/2)-(cy/2);

    windowX = x;
    windowY = y;

    DWORD flags;

    if(bResizable)
    {
        flags = WS_OVERLAPPEDWINDOW;
        cx += GetSystemMetrics(SM_CXSIZEFRAME)*2;
        cy += GetSystemMetrics(SM_CYSIZEFRAME)*2;
    }
    else
    {
        flags = (WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
        cx += GetSystemMetrics(SM_CXFIXEDFRAME)*2;
        cy += GetSystemMetrics(SM_CYFIXEDFRAME)*2;
    }

    HANDLE hwndMainWindow = (HANDLE)CreateWindow(ENGINE_WINDOW_CLASS_NAME, lpTitle, WS_VISIBLE|flags, x, y, cx, cy, NULL, NULL, (HINSTANCE)hinstMain, NULL);

    ResetCursorClip(hwndMainWindow);

    return hwndMainWindow;
}

void   ENGINEAPI OSDestroyMainWindow()
{
    DestroyWindow((HWND)hwndMain);
    hwndMain = NULL;
}

BOOL   ENGINEAPI OSAppHasFocus()
{
    return (GetForegroundWindow() == (HWND)hwndMain);
}

void   ENGINEAPI OSSetWindowSize(int cx, int cy, BOOL bCenter)
{
    UINT styles = GetWindowLongPtr((HWND)hwndMain, GWL_STYLE);
    HMENU hMenu = GetMenu((HWND)hwndMain);

    int borderXSize = 0;
    int borderYSize = 0;

    if(styles & WS_THICKFRAME)
    {
        borderXSize += GetSystemMetrics(SM_CXSIZEFRAME)*2;
        borderYSize += GetSystemMetrics(SM_CYSIZEFRAME)*2;
    }
    else if(styles & WS_BORDER)
    {
        borderXSize += GetSystemMetrics(SM_CXFIXEDFRAME)*2;
        borderYSize += GetSystemMetrics(SM_CYFIXEDFRAME)*2;
    }

    if(styles & WS_CAPTION)
        borderYSize += GetSystemMetrics(SM_CYCAPTION);

    if(hMenu)
        borderYSize += GetSystemMetrics(SM_CYMENU);

    cx += borderXSize;
    cy += borderYSize;

    windowCX = cx;
    windowCY = cy;

    int x = (GetSystemMetrics(SM_CXFULLSCREEN)/2)-(cx/2);
    int y = (GetSystemMetrics(SM_CYFULLSCREEN)/2)-(cy/2);

    SetWindowPos((HWND)hwndMain, NULL, x, y, cx, cy, (bCenter ? 0 : SWP_NOMOVE));

    ResetCursorClip(hwndMain);
}

void   ENGINEAPI OSSetWindowPos(int x, int y)
{
    SetWindowPos((HWND)hwndMain, NULL, x, y, 0, 0, SWP_NOSIZE);

    windowX = x;
    windowY = y;
}

void   ENGINEAPI OSGetWindowRect(XRect &rect)
{
    GetWindowRect((HWND)hwndMain, (LPRECT)&rect);
}

void   ENGINEAPI OSCenterWindow()
{
    int x = (GetSystemMetrics(SM_CXFULLSCREEN)/2)-(windowCX/2);
    int y = (GetSystemMetrics(SM_CYFULLSCREEN)/2)-(windowCY/2);

    OSSetWindowPos(x, y);
}


void   ENGINEAPI OSShowCursor(BOOL bShow)
{
    /*HCURSOR hCursor = bShow ? LoadCursor(NULL, IDC_ARROW) : NULL;
    bHidingCursor = !bShow;
    SetCursor(hCursor);

    ResetCursorClip(hwndMain);*/

    if(bShow)
    {
        int displayCount = ShowCursor(TRUE);
        while(displayCount > 0)
            displayCount = ShowCursor(FALSE);
    }
    else
    {
        int displayCount = ShowCursor(FALSE);
        while(displayCount > -1)
            displayCount = ShowCursor(FALSE);
    }
}

void   ENGINEAPI OSSetCursorPos(int x, int y)
{
    POINT point;
    point.x = x;
    point.y = y;
    SetCursorPos(point.x, point.y);
}

void  ENGINEAPI OSGetCursorPos(int &x, int &y)
{
    POINT point;

    GetCursorPos(&point);
    x = point.x;
    y = point.y;
}

void  ENGINEAPI OSClipCursor(BOOL bClip)
{
    bClippingCursor = bClip;
    ResetCursorClip(hwndMain);
}

void ENGINEAPI OSCenterCursor()
{
    RECT r;
    GetClientRect((HWND)hwndMain, &r);

    POINT newPos;
    newPos.x = r.right/2;
    newPos.y = r.bottom/2;

    ClientToScreen((HWND)hwndMain, &newPos);

    SetCursorPos(newPos.x, newPos.y);
}


void  ENGINEAPI ResetCursorClip(HANDLE window)
{
    if(bClippingCursor)
    {
        RECT r;
        POINT p;

        GetClientRect((HWND)window, &r);

        p.x = p.y = 0;
        ClientToScreen((HWND)window, &p);
        r.left = p.x; r.top = p.y;

        p.x = r.right;
        p.y = r.bottom;
        ClientToScreen((HWND)window, &p);
        r.right = p.x; r.bottom = p.y;

        ClipCursor(&r);
    }
    else
        ClipCursor(NULL);
}


HANDLE ENGINEAPI OSFindFirstFile(CTSTR lpFileName, OSFindData &findData)
{
    WIN32_FIND_DATA wfd;
    HANDLE hFind = FindFirstFile(lpFileName, &wfd);

    if(hFind == INVALID_HANDLE_VALUE)
        hFind = NULL;
    else
    {
        BOOL bFoundDumbDir;
        do
        {
            bFoundDumbDir = FALSE;
            if( (scmp(wfd.cFileName, TEXT("..")) == 0) ||
                (scmp(wfd.cFileName, TEXT(".")) == 0)  )
            {
                if(!FindNextFile(hFind, &wfd))
                {
                    FindClose(hFind);
                    return NULL;
                }
                bFoundDumbDir = TRUE;
            }
        }while(bFoundDumbDir);

        scpy(findData.fileName, wfd.cFileName);
        findData.bDirectory = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        findData.bHidden = (wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
    }

    return hFind;
}

BOOL   ENGINEAPI OSFindNextFile(HANDLE hFind, OSFindData &findData)
{
    WIN32_FIND_DATA wfd;
    BOOL bSuccess = FindNextFile(hFind, &wfd);

    BOOL bFoundDumbDir;
    do
    {
        bFoundDumbDir = FALSE;
        if( (scmp(wfd.cFileName, TEXT("..")) == 0) ||
            (scmp(wfd.cFileName, TEXT(".")) == 0)  )
        {
            if(!FindNextFile(hFind, &wfd))
            {
                bSuccess = FALSE;
                break;
            }
            bFoundDumbDir = TRUE;
        }
    }while(bFoundDumbDir);

    if(bSuccess)
    {
        scpy(findData.fileName, wfd.cFileName);
        findData.bDirectory = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        findData.bHidden = (wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
    }
    else
        *findData.fileName = 0;

    return bSuccess;
}

void   ENGINEAPI OSFindClose(HANDLE hFind)
{
    FindClose(hFind);
}


BOOL   ENGINEAPI OSSetFullscreen(const DisplayMode *dm)
{
    if(!dm)
    {
        BOOL bResult = (ChangeDisplaySettings(NULL, 0) == DISP_CHANGE_SUCCESSFUL);
        OSColorAdjust(curGamma, curBrightness, curContrast);

        OSSetWindowPos(windowX, windowY);

        bFullscreen = FALSE;

        return bResult;
    }
    else
    {
        curDevMode.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY|DM_BITSPERPEL;
        curDevMode.dmSize = sizeof(DEVMODE);
        curDevMode.dmBitsPerPel = dm->dwBitsPerPixel;
        curDevMode.dmPelsHeight = dm->dwHeight;
        curDevMode.dmPelsWidth  = dm->dwWidth;
        curDevMode.dmDisplayFrequency = dm->dwFrequency;

        BOOL bResult = (ChangeDisplaySettings(&curDevMode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL);
        OSColorAdjust(curGamma, curBrightness, curContrast);

        RECT windowRect;
        GetWindowRect((HWND)hwndMain, &windowRect);

        windowX = windowRect.left;
        windowY = windowRect.top;

        int borderXSize = GetSystemMetrics(SM_CXFIXEDFRAME);
        int borderYSize = GetSystemMetrics(SM_CYFIXEDFRAME);
        borderYSize += GetSystemMetrics(SM_CYCAPTION);
        SetWindowPos((HWND)hwndMain, NULL, -borderXSize, -borderYSize, 0, 0, SWP_NOSIZE);

        bFullscreen = TRUE;

        return bResult;
    }
}

void   ENGINEAPI OSGetDisplaySettings(DisplayMode *dm)
{
    DEVMODE devmode;

    if(!dm) return;

    if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode))
    {
        dm->dwBitsPerPixel  = devmode.dmBitsPerPel;
        dm->dwFrequency     = devmode.dmDisplayFrequency;
        dm->dwHeight        = devmode.dmPelsHeight;
        dm->dwWidth         = devmode.dmPelsWidth;
    }
}

void ENGINEAPI OSEnumDisplayModes(List<DisplayMode> &displayModes)
{
    DEVMODE dm;
    DWORD i=0;

    displayModes.Clear();

    zero(&dm, sizeof(DEVMODE));
    dm.dmSize = sizeof(DEVMODE);

    while(EnumDisplaySettingsEx(TEXT("\\\\.\\DISPLAY1"), i++, &dm, 0))
    {
        if((dm.dmBitsPerPel != 32) || dm.dmDefaultSource || (dm.dmDisplayFrequency < 60))
            continue;

        /*for(i=0;i<displayModes.Num();i++)
        {
            if( (dm.dmPelsHeight == displayModes[i].dwHeight) && 
                (dm.dmPelsWidth  == displayModes[i].dwWidth))
            {
                if(dm.dmDisplayFrequency > displayModes[i].dwFrequency)
                {
                    displayModes[i].dwBitsPerPixel   = dm.dmBitsPerPel;
                    displayModes[i].dwFrequency      = dm.dmDisplayFrequency;
                    displayModes[i].dwHeight         = dm.dmPelsHeight;
                    displayModes[i].dwWidth          = dm.dmPelsWidth;
                }
                continue;
            }
        }*/

        DisplayMode *lpdm = displayModes.CreateNew();
        lpdm->dwBitsPerPixel   = dm.dmBitsPerPel;
        lpdm->dwFrequency      = dm.dmDisplayFrequency;
        lpdm->dwHeight         = dm.dmPelsHeight;
        lpdm->dwWidth          = dm.dmPelsWidth;
    }
}

BOOL   ENGINEAPI OSColorAdjust(float gamma, float brightness, float contrast)
{
    BOOL bResult = TRUE;

    if(gamma > 2.0f) gamma = 2.0f;
    if(gamma < 0.5f) gamma = 0.5f;

    if(contrast > 1.25f) contrast = 1.25f;
    if(contrast < 0.75f) contrast = 0.75f;

    if(brightness > 1.25f) brightness = 1.25f;
    if(brightness < 0.75f) brightness = 0.75f;

    curGamma = gamma;
    curBrightness = brightness;
    curContrast = contrast;

    if(bWindowActive)
    {
        LPWORD ColorRamp = (LPWORD)malloc(3*256*2);

        gamma = 1.0f/gamma;
        brightness -= 1.0f;

        for(int i=0; i<256; i++)
        {
            float val=0.0f, fI = ((float)i)/255.0f;

            val  = pow(fI, gamma);
            val *= contrast;
            val += brightness;

            if(val > 1.0f) val = 1.0f;
            if(val < 0.0f) val = 0.0f;

            ColorRamp[i] = (((WORD)((val*255.0f)+0.5f)) << 8);
        }

        mcpy(&ColorRamp[256], ColorRamp, 256*2);
        mcpy(&ColorRamp[512], ColorRamp, 256*2);

        HDC hdcScreen = GetDC(NULL);
        bResult = SetDeviceGammaRamp(hdcScreen, ColorRamp);
        ReleaseDC(NULL, hdcScreen);

        free(ColorRamp);

        //if(!bResult)  //ignoring this because on some versions of windows it returns false even when it succeeds
        //    AppWarning(TEXT("Warning: Could not set color ramp."));
    }

    return bResult;
}


HANDLE ENGINEAPI OSLoadLibrary(CTSTR lpFile)
{
    return (HANDLE)LoadLibrary(lpFile);
}

LPVOID ENGINEAPI OSGetProcAddress(HANDLE hLibrary, LPCSTR lpProcedure)
{
    return (LPVOID)GetProcAddress((HMODULE)hLibrary, lpProcedure);
}

void   ENGINEAPI OSFreeLibrary(HANDLE hLibrary)
{
    FreeLibrary((HMODULE)hLibrary);
}



void   ENGINEAPI OSSleep(DWORD dwMSeconds)
{
    Sleep(dwMSeconds);
}

void   ENGINEAPI OSSignalAppExit()
{
    PostQuitMessage(0);
    bExiting = 1;
}

extern int realtimeCounter;

int ENGINEAPI OSApplicationLoop()
{
    traceIn(OSApplicationLoop);

    if(!bBaseLoaded)
        return 0;

    while (!bExiting)
    {
        if(realtimeCounter)
        {
            int next = OSProcessEvent();
            if(next == 1)
                engine->Update();
        }
        else
        {
            MSG msg;
            long iRet;

            while(iRet = (long)GetMessage(&msg, NULL, 0, 0))
            {
                if(iRet == -1)
                    break;
                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            }

            if(msg.wParam == 0)
                bExiting = TRUE;
        }
    }

    return 0;

    traceOut;
}

int ENGINEAPI EnableRealTime(BOOL bEnable)
{
    BOOL bWasEnabled = realtimeCounter != 0;

    if(bEnable)
    {
        ++realtimeCounter;
        if(realtimeCounter == 1)
            PostQuitMessage(1);
    }
    else if(realtimeCounter > 0)
        --realtimeCounter;

    return realtimeCounter;
}



UINT ENGINEAPI OSGetProcessorCount()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    return si.dwNumberOfProcessors;
}

HANDLE ENGINEAPI OSCreateThread(ENGINETHREAD lpThreadFunc, LPVOID param)
{
    DWORD dummy;
    return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)lpThreadFunc, param, 0, &dummy);
}

BOOL   ENGINEAPI OSWaitForThread(HANDLE hThread, DWORD dwTimeoutMS)
{
	BOOL bSuccess = (WaitForSingleObject(hThread, dwTimeoutMS) == WAIT_OBJECT_0);

    return bSuccess;
}

BOOL   ENGINEAPI OSCloseThread(HANDLE hThread, LPDWORD ret)
{
    if(WaitForSingleObject(hThread, 0) == WAIT_TIMEOUT)
        return 0;

    if(ret)
        GetExitCodeThread(hThread, ret);

    CloseHandle(hThread);
    return 1;
}

BOOL   ENGINEAPI OSTerminateThread(HANDLE hThread)
{
    return TerminateThread(hThread, 0);
}


HANDLE ENGINEAPI OSCreateMutex()
{
    CRITICAL_SECTION *pSection = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
    zero(pSection, sizeof(CRITICAL_SECTION));
    InitializeCriticalSection(pSection);

    return (HANDLE)pSection;
}

void   ENGINEAPI OSEnterMutex(HANDLE hMutex)
{
    assert(hMutex);
    EnterCriticalSection((CRITICAL_SECTION*)hMutex);
}

BOOL   ENGINEAPI OSTryEnterMutex(HANDLE hMutex)
{
    assert(hMutex);
    return TryEnterCriticalSection((CRITICAL_SECTION*)hMutex);
}

void   ENGINEAPI OSLeaveMutex(HANDLE hMutex)
{
    assert(hMutex);
    LeaveCriticalSection((CRITICAL_SECTION*)hMutex);
}

void   ENGINEAPI OSCloseMutex(HANDLE hMutex)
{
    assert(hMutex);
    DeleteCriticalSection((CRITICAL_SECTION*)hMutex);
    free(hMutex);
}



HANDLE ENGINEAPI OSCreateEvent(BOOL bInitialState)
{
    return (HANDLE)CreateEvent(NULL, FALSE, bInitialState, NULL);
}

void   ENGINEAPI OSSignalEvent(HANDLE hEvent)
{
    SetEvent(hEvent);
}

BOOL   ENGINEAPI OSWaitForEvent(HANDLE hEvent, DWORD dwTimeoutMS)
{
    return (WaitForSingleObject(hEvent, dwTimeoutMS) == WAIT_OBJECT_0);
}

void   ENGINEAPI OSCloseEvent(HANDLE hEvent)
{
    CloseHandle(hEvent);
}



HANDLE ENGINEAPI OSCreateSemaphore(UINT initialValue)
{
    HANDLE hBla = CreateSemaphore(NULL, initialValue, 0x7FFFFFFF, NULL);

    DWORD chi = GetLastError();

    nop();

    return hBla;
}

void   ENGINEAPI OSIncrementSemaphore(HANDLE hSemaphore)
{
    LONG test123;
    ReleaseSemaphore(hSemaphore, 1, &test123);

    nop();
}

void   ENGINEAPI OSWaitForSemaphore(HANDLE hSemaphore)
{
    WaitForSingleObject(hSemaphore, INFINITE);
}

void   ENGINEAPI OSCloseSemaphore(HANDLE hSemaphore)
{
    CloseHandle(hSemaphore);
}




DWORD  ENGINEAPI OSGetTime()
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    LONGLONG elapsedTime = currentTime.QuadPart - 
        startTime.QuadPart;

    // Compute the number of millisecond ticks elapsed.
    unsigned long msecTicks = (unsigned long)(1000 * elapsedTime / 
        clockFreq.QuadPart);

    // Check for unexpected leaps in the Win32 performance counter.  
    // (This is caused by unexpected data across the PCI to ISA 
    // bridge, aka south bridge.  See Microsoft KB274323.)
    unsigned long elapsedTicks = GetTickCount() - startTick;
    signed long msecOff = (signed long)(msecTicks - elapsedTicks);
    if (msecOff < -100 || msecOff > 100)
    {
        // Adjust the starting time forwards.
        LONGLONG msecAdjustment = MIN(msecOff * 
            clockFreq.QuadPart / 1000, elapsedTime - 
            prevElapsedTime);
        startTime.QuadPart += msecAdjustment;
        elapsedTime -= msecAdjustment;

        // Recompute the number of millisecond ticks elapsed.
        msecTicks = (unsigned long)(1000 * elapsedTime / clockFreq.QuadPart);
    }

    // Store the current elapsed time for adjustments next time.
    prevElapsedTime = elapsedTime;

    return msecTicks;
}

QWORD  ENGINEAPI OSGetTimeMicroseconds()
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    LONGLONG elapsedTime = currentTime.QuadPart - 
        startTime.QuadPart;

    // Compute the number of millisecond ticks elapsed.
    unsigned long msecTicks = (unsigned long)(1000 * elapsedTime / 
        clockFreq.QuadPart);

    // Check for unexpected leaps in the Win32 performance counter.  
    // (This is caused by unexpected data across the PCI to ISA 
    // bridge, aka south bridge.  See Microsoft KB274323.)
    unsigned long elapsedTicks = GetTickCount() - startTick;
    signed long msecOff = (signed long)(msecTicks - elapsedTicks);
    if (msecOff < -100 || msecOff > 100)
    {
        // Adjust the starting time forwards.
        LONGLONG msecAdjustment = MIN(msecOff * 
            clockFreq.QuadPart / 1000, elapsedTime - 
            prevElapsedTime);
        startTime.QuadPart += msecAdjustment;
        elapsedTime -= msecAdjustment;
    }

    // Store the current elapsed time for adjustments next time.
    prevElapsedTime = elapsedTime;

    // Convert to microseconds.
    QWORD usecTicks = (QWORD)(1000000 * elapsedTime / clockFreq.QuadPart);

    return usecTicks;
}

BOOL   ENGINEAPI OSDebuggerPresent()
{
    return IsDebuggerPresent();
}



LRESULT ENGINEAPI MainWndProc(HANDLE hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //if(engine)
    //    engine->ProcessMessage(hwnd, message, wParam, lParam);
    GraphicsSystem *curGSystem = (GraphicsSystem*)(ULONG_PTR)GetWindowLongPtr((HWND)hwnd, 0);

    if(curGSystem)
        ProcessStandardInput((HWND)hwnd, message, wParam, lParam);

    switch(message)
    {
        case WM_ACTIVATE:
            {
                if(hwnd == hwndMain)
                {
                    bWindowActive = (wParam != WA_INACTIVE);
                    bWasClippingCursor = bClippingCursor;
                    SIWindowActivate(bWindowActive);
                    //DIWindowActivate(bWindowActive);

                    if(!bWindowActive)
                    {
                        bClippingCursor = FALSE;
                        ResetCursorClip(hwndMain);

                        if(bFullscreen)
                        {
                            ChangeDisplaySettings(NULL, 0);
                            ShowWindow((HWND)hwndMain, SW_MINIMIZE);
                        }

                        LPWORD ramp = (LPWORD)malloc(3*256*2);

                        for(int i=0; i<256; i++)
                            ramp[i] = (i) << 8;
                        mcpy(&ramp[256], ramp, 256*2);
                        mcpy(&ramp[512], ramp, 256*2);

                        HDC hdcScreen = GetDC(NULL);
                        SetDeviceGammaRamp(hdcScreen, ramp);
                        ReleaseDC(NULL, hdcScreen);

                        free(ramp);
                    }
                    else
                    {
                        bClippingCursor = bWasClippingCursor;
                        ResetCursorClip(hwndMain);

                        if(bFullscreen)
                        {
                            ChangeDisplaySettings(&curDevMode, CDS_FULLSCREEN);
                            ShowWindow((HWND)hwndMain, SW_RESTORE);

                            int borderXSize = GetSystemMetrics(SM_CXFIXEDFRAME);
                            int borderYSize = GetSystemMetrics(SM_CYFIXEDFRAME);
                            borderYSize += GetSystemMetrics(SM_CYCAPTION);
                            SetWindowPos((HWND)hwndMain, NULL, -borderXSize, -borderYSize, 0, 0, SWP_NOSIZE);

                            if(GS)
                                GS->ResetDevice();
                        }


                        OSColorAdjust(curGamma, curBrightness, curContrast);
                    }
                }

                break;
            }

        case WM_SIZE:
            {
                if(wParam == SIZE_RESTORED)
                {
                    if(curGSystem)
                    {
                        curGSystem->SetSize(LOWORD(lParam), HIWORD(lParam));
                        curGSystem->ResetDevice();
                    }
                }
                break;
            }

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
            if(hwnd == hwndMain)
            {
                if(bHidingCursor)
                {
                    SetCursor(NULL);
                    return 0;
                }
            }
            return DefWindowProc((HWND)hwnd, message, wParam, lParam);

        case WM_IME_CHAR:
        case WM_IME_STARTCOMPOSITION:
        case WM_IME_COMPOSITION:
        case WM_IME_ENDCOMPOSITION:
        case WM_IME_CONTROL:
        case WM_IME_KEYDOWN:
        case WM_IME_KEYUP:
        case WM_IME_NOTIFY:
        case WM_IME_REQUEST:
        case WM_IME_SELECT:
        case WM_IME_SETCONTEXT:
            break;

        case WM_SIZING:
            if(hwnd == hwndMain)
            {
                DWORD styles = GetWindowLongPtr((HWND)hwndMain, GWL_STYLE);
                HMENU hMenu = GetMenu((HWND)hwndMain);

                int borderXSize = 100;
                int borderYSize = 100;

                if(styles & WS_THICKFRAME)
                {
                    borderXSize += GetSystemMetrics(SM_CXSIZEFRAME)*2;
                    borderYSize += GetSystemMetrics(SM_CYSIZEFRAME)*2;
                }
                else if(styles & WS_BORDER)
                {
                    borderXSize += GetSystemMetrics(SM_CXFIXEDFRAME)*2;
                    borderYSize += GetSystemMetrics(SM_CYFIXEDFRAME)*2;
                }

                if(styles & WS_CAPTION)
                    borderYSize += GetSystemMetrics(SM_CYCAPTION);

                if(hMenu)
                    borderYSize += GetSystemMetrics(SM_CYMENU);

                RECT *pRect = (RECT*)lParam;

                if((pRect->right - pRect->left) <= borderXSize)
                    pRect->right = pRect->left + borderXSize;

                if((pRect->bottom - pRect->top) <= borderYSize)
                    pRect->bottom = pRect->top + borderYSize;
            }
            return TRUE;

        case WM_PAINT:
            {
                RECT rect;
                PAINTSTRUCT ps;

                if(GetUpdateRect((HWND)hwnd, &rect, FALSE))
                {
                    BeginPaint((HWND)hwnd, &ps);
                    EndPaint((HWND)hwnd, &ps);
                    if(engine && !realtimeCounter)
                        engine->Update(FALSE);
                }

                break;
            }

        case WM_CLOSE:
            if(hwnd == hwndMain)
                OSSignalAppExit();
            return 0;

        case WM_SETCURSOR:
            if(hwnd == hwndMain)
            {
                if(bHidingCursor)
                {
                    SetCursor(NULL);
                    return 1;
                }
            }

        default:
            return DefWindowProc((HWND)hwnd, message, wParam, lParam);
    }

    return 0;
}




#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    BasePlatform.h
  
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


#ifndef BASE_PLATFORM
#define BASE_PLATFORM

//-----------------------------------------
//OS structures
//-----------------------------------------
struct OSFindData
{
    TCHAR fileName[512];
    BOOL bDirectory;
    BOOL bHidden;
};

struct XRect
{
    int x;
    int y;
    int cx;
    int cy;
};

struct DisplayMode
{
    DWORD dwWidth;
    DWORD dwHeight;
    DWORD dwBitsPerPixel;
    DWORD dwFrequency;
};

struct OSTimeInfo
{
    int year;
    int month;
    int dayOfWeek; //0=sunday(日), 1=monday(月), 2=tuesday(火), 3=wednesday(水), 4=thursday(木), 5=friday(金), 6=saturday(土)
    int day;
    int hour;
    int minute;
    int second;
    int millisecond;

    inline BOOL operator<(const OSTimeInfo &compareTime) const
    {
        if(year < compareTime.year)             return TRUE;
        else if(year > compareTime.year)        return FALSE;

        if(month < compareTime.month)           return TRUE;
        else if(month > compareTime.month)      return FALSE;

        if(day < compareTime.day)               return TRUE;
        else if(day > compareTime.day)          return FALSE;

        if(hour < compareTime.hour)             return TRUE;
        else if(hour > compareTime.hour)        return FALSE;

        if(minute < compareTime.minute)         return TRUE;
        else if(minute > compareTime.minute)    return FALSE;

        if(second < compareTime.second)         return TRUE;
        else if(second > compareTime.second)    return FALSE;

        if(millisecond < compareTime.millisecond)
            return TRUE;

        return FALSE;
    }

    inline BOOL operator>(const OSTimeInfo &compareTime) const
    {
        if(year > compareTime.year)             return TRUE;
        else if(year < compareTime.year)        return FALSE;

        if(month > compareTime.month)           return TRUE;
        else if(month < compareTime.month)      return FALSE;

        if(day > compareTime.day)               return TRUE;
        else if(day < compareTime.day)          return FALSE;

        if(hour > compareTime.hour)             return TRUE;
        else if(hour < compareTime.hour)        return FALSE;

        if(minute > compareTime.minute)         return TRUE;
        else if(minute < compareTime.minute)    return FALSE;

        if(second > compareTime.second)         return TRUE;
        else if(second < compareTime.second)    return FALSE;

        if(millisecond > compareTime.millisecond)
            return TRUE;

        return FALSE;
    }

    inline BOOL operator==(const OSTimeInfo &compareTime) const
    {
        return  (year == compareTime.year)              &&
                (month == compareTime.month)            &&
                (day == compareTime.day)                &&
                (hour == compareTime.hour)              &&
                (minute == compareTime.minute)          &&
                (second == compareTime.second)          &&
                (millisecond == compareTime.millisecond);
    }
};

struct OSFileTime
{
    OSTimeInfo timeCreated;
    OSTimeInfo timeModified;
};

//-----------------------------------------
//OS functions
//-----------------------------------------

BASE_EXPORT void   ENGINEAPI OSLogSystemStats();
BASE_EXPORT DWORD  ENGINEAPI OSGetSysPageSize();
BASE_EXPORT LPVOID ENGINEAPI OSVirtualAlloc(size_t dwSize);
BASE_EXPORT void   ENGINEAPI OSVirtualFree(LPVOID lpData);
BASE_EXPORT void   ENGINEAPI OSCriticalExit();
BASE_EXPORT int    ENGINEAPI OSProcessEvent();

BASE_EXPORT BOOL   ENGINEAPI OSDeleteFile(CTSTR lpFile);
BASE_EXPORT BOOL   ENGINEAPI OSCopyFile(CTSTR lpFileDest, CTSTR lpFileSrc);
BASE_EXPORT BOOL   ENGINEAPI OSCreateDirectory(CTSTR lpDirectory);
BASE_EXPORT BOOL   ENGINEAPI OSSetCurrentDirectory(CTSTR lpDirectory);
BASE_EXPORT BOOL   ENGINEAPI OSFileExists(CTSTR lpFile);
BASE_EXPORT BOOL   ENGINEAPI OSGetFileTime(CTSTR lpFile, OSFileTime &fileTime);

BASE_EXPORT HANDLE ENGINEAPI OSCreateMainWindow(CTSTR lpTitle, int cx, int cy, BOOL bResizable);
BASE_EXPORT void   ENGINEAPI OSDestroyMainWindow();
BASE_EXPORT BOOL   ENGINEAPI OSAppHasFocus();
BASE_EXPORT void   ENGINEAPI OSSetWindowSize(int cx, int cy, BOOL bCenter=FALSE);
BASE_EXPORT void   ENGINEAPI OSSetWindowPos(int x, int y);
BASE_EXPORT void   ENGINEAPI OSGetWindowRect(XRect &rect);
BASE_EXPORT void   ENGINEAPI OSCenterWindow();

BASE_EXPORT void   ENGINEAPI OSShowCursor(BOOL bShow);
BASE_EXPORT void   ENGINEAPI OSSetCursorPos(int x, int y);
BASE_EXPORT void   ENGINEAPI OSGetCursorPos(int &x, int &y);
BASE_EXPORT void   ENGINEAPI OSClipCursor(BOOL bClip);
BASE_EXPORT void   ENGINEAPI OSCenterCursor();

BASE_EXPORT HANDLE ENGINEAPI OSFindFirstFile(CTSTR lpFileName, OSFindData &findData);
BASE_EXPORT BOOL   ENGINEAPI OSFindNextFile(HANDLE hFind, OSFindData &findData);
BASE_EXPORT void   ENGINEAPI OSFindClose(HANDLE hFind);

BASE_EXPORT BOOL   ENGINEAPI OSSetFullscreen(const DisplayMode *dm);
BASE_EXPORT void   ENGINEAPI OSGetDisplaySettings(DisplayMode *dm);
BASE_EXPORT void   ENGINEAPI OSEnumDisplayModes(List<DisplayMode> &displayModes);
BASE_EXPORT BOOL   ENGINEAPI OSColorAdjust(float gamma=1.0f, float brightness=1.0f, float contrast=1.0f);

BASE_EXPORT HANDLE ENGINEAPI OSLoadLibrary(CTSTR lpFile);
BASE_EXPORT LPVOID ENGINEAPI OSGetProcAddress(HANDLE hLibrary, LPCSTR lpProcedure);
BASE_EXPORT void   ENGINEAPI OSFreeLibrary(HANDLE hLibrary);

BASE_EXPORT void   ENGINEAPI OSSleep(DWORD dwMSeconds);
BASE_EXPORT void   ENGINEAPI OSSignalAppExit();
BASE_EXPORT int    ENGINEAPI OSApplicationLoop();

#define WAIT_INFINITE 0xFFFFFFFF

BASE_EXPORT UINT   ENGINEAPI OSGetProcessorCount();
BASE_EXPORT HANDLE ENGINEAPI OSCreateThread(ENGINETHREAD lpThreadFunc, LPVOID param);
BASE_EXPORT BOOL   ENGINEAPI OSWaitForThread(HANDLE hThread, DWORD dwTimeoutMS);
BASE_EXPORT BOOL   ENGINEAPI OSCloseThread(HANDLE hThread, LPDWORD ret);
BASE_EXPORT BOOL   ENGINEAPI OSTerminateThread(HANDLE hThread);

BASE_EXPORT HANDLE ENGINEAPI OSCreateMutex();
BASE_EXPORT void   ENGINEAPI OSEnterMutex(HANDLE hMutex);
BASE_EXPORT BOOL   ENGINEAPI OSTryEnterMutex(HANDLE hMutex);
BASE_EXPORT void   ENGINEAPI OSLeaveMutex(HANDLE hMutex);
BASE_EXPORT void   ENGINEAPI OSCloseMutex(HANDLE hMutex);

BASE_EXPORT HANDLE ENGINEAPI OSCreateEvent(BOOL bInitialState);
BASE_EXPORT void   ENGINEAPI OSSignalEvent(HANDLE hEvent);
BASE_EXPORT BOOL   ENGINEAPI OSWaitForEvent(HANDLE hEvent, DWORD dwTimeoutMS);
BASE_EXPORT void   ENGINEAPI OSCloseEvent(HANDLE hEvent);

BASE_EXPORT HANDLE ENGINEAPI OSCreateSemaphore(UINT initialValue);
BASE_EXPORT void   ENGINEAPI OSIncrementSemaphore(HANDLE hSemaphore);
BASE_EXPORT void   ENGINEAPI OSWaitForSemaphore(HANDLE hSemaphore);
BASE_EXPORT void   ENGINEAPI OSCloseSemaphore(HANDLE hSemaphore);

BASE_EXPORT DWORD  ENGINEAPI OSGetTime();
BASE_EXPORT QWORD  ENGINEAPI OSGetTimeMicroseconds();

BASE_EXPORT BOOL   ENGINEAPI OSDebuggerPresent();

BASE_EXPORT void __cdecl  OSMessageBoxva(const TCHAR *format, va_list argptr);
BASE_EXPORT void __cdecl  OSMessageBox(const TCHAR *format, ...);

BASE_EXPORT void __cdecl  OSDebugOutva(const TCHAR *format, va_list argptr);
BASE_EXPORT void __cdecl  OSDebugOut(const TCHAR *format, ...);

#endif

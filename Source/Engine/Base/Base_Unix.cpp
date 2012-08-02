/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Base.cpp:  Main source file

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

#include "Base.h"

#ifdef __UNIX__

#include <unistd.h>
#include <malloc.h>
#include <dlfcn.h>
#include <time.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

#define CLOCKS_PER_MSEC  (CLOCKS_PER_SEC/1000)


LPVOID xdisplay;
Window wndMain;
GLXWindow glxwindowMain;
XRect mainrect;
BOOL  bExit = 0;

BOOL MainWndProc(Display *d, XEvent *e);



void _init()
{
    xdisplay = (LPVOID)XOpenDisplay(0);
    if(!xdisplay)
        CrashError(TEXT("Could not open XWindows Display"));
}

void _fini()
{
    XCloseDisplay((Display*)xdisplay);
}



void __cdecl Messageva(const TCHAR *format, va_list argptr)
{
    TCHAR blah[1024];
    vsprintf(blah, format, argptr);

    //MessageBox((HWND)hwndMain, blah, NULL, MB_ICONWARNING);
    Log(blah);
}

void __cdecl Message(const TCHAR *format, ...)
{
    va_list arglist;

    va_start(arglist, format);

    Messageva(format, arglist);
}

DWORD  ENGINEAPI OSGetSysPageSize()
{
    return getpagesize();
}

LPVOID ENGINEAPI OSVirtualAlloc(DWORD dwSize)
{
    return valloc(dwSize);
}

void   ENGINEAPI OSVirtualFree(LPVOID lpData)
{
    free(lpData);
}

void   ENGINEAPI OSCriticalExit()
{
    //TerminateProcess(GetCurrentProcess(), -1);
    abort();
    //_Exit(0);
}

void   ENGINEAPI OSDeleteFile(TSTR lpFile)
{
    //DeleteFile(lpFile);
}

int    ENGINEAPI OSProcessEvent()
{
    XEvent event;

    if(XCheckWindowEvent((Display*)xdisplay, wndMain, 0xFFFFFFFF, &event))
    {
        if(event.type == LeaveNotify)
            return 0;
        else
            MainWndProc((Display*)xdisplay, &event);
        return -1;
    }
    else
        return 1;
    /*MSG msg;

    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return 0;
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return -1;
    }
    else
        return 1;*/
}



BOOL ENGINEAPI OSCreateMainWindow(int cx, int cy)
{
    GLXFBConfig *fbconfig;
    XVisualInfo *vi;
    Colormap colormap;
    XSetWindowAttributes swa;
    GLXContext glxcontext;
    //XEvent event;
    XTextProperty windowtitle;
    TSTR title = ENGINE_WINDOW_TITLE;

    XStringListToTextProperty(&title, 1, &windowtitle);

     
    int nelements[] = {
                GLX_DOUBLEBUFFER, 1,
                GLX_RED_SIZE,4,GLX_GREEN_SIZE,4,GLX_BLUE_SIZE,4,GLX_ALPHA_SIZE,4, //GLX_COLOR_BITS,32
                GLX_STENCIL_SIZE, 1,
                GLX_DEPTH_SIZE, 24,
                0};
    fbconfig = glXChooseFBConfig((Display*)xdisplay, DefaultScreen(xdisplay), 0, nelements);

    vi = glXGetVisualFromFBConfig((Display*)xdisplay, *fbconfig);
    glxcontext = glXCreateNewContext((Display*)xdisplay, *fbconfig, GLX_RGBA_TYPE, 0, 1);


    colormap = XCreateColormap((Display*)xdisplay, RootWindow(xdisplay, vi->screen), vi->visual, AllocNone);

    swa.colormap = colormap;
    swa.border_pixel = 0;
    swa.event_mask = StructureNotifyMask;
    wndMain = XCreateWindow((Display*)xdisplay, RootWindow(xdisplay, vi->screen), mainrect.x=x, mainrect.y=y, mainrect.cx=cx, mainrect.cy=cy, 0, vi->depth, InputOutput,
                        vi->visual, CWBorderPixel|CWColormap|CWEventMask, &swa);
    if(!wndMain)
    {
        CrashError(TEXT("Could not create main window"));
        return 0;
    }

    XSetWMName((Display*)xdisplay, wndMain, &windowtitle);
    XMapWindow((Display*)xdisplay, wndMain);
    //XIfEvent((Display*)xdisplay, &event, MainWndProc, (Char*)wndMain);

    glxwindowMain = glXCreateWindow((Display*)xdisplay, *fbconfig, wndMain, 0);
    glXMakeContextCurrent((Display*)xdisplay, glxwindowMain, glxwindowMain, glxcontext);

    return 1;
}

void   ENGINEAPI OSDestroyMainWindow()
{
    glXDestroyWindow((Display*)xdisplay, glxwindowMain);
    XDestroyWindow((Display*)xdisplay, wndMain);
}

void   ENGINEAPI OSSetWindowSize(int cx, int cy)
{
    XResizeWindow((Display*)xdisplay, wndMain, mainrect.cx=cx, mainrect.cy=cy);
    //SetWindowPos((HWND)hwndMain, NULL, 0, 0, cx, cy, SWP_NOMOVE);
}

void   ENGINEAPI OSSetWindowPos(int x, int y)
{
    XMoveWindow((Display*)xdisplay, wndMain, mainrect.x=x, mainrect.y=y);
}

void   ENGINEAPI OSGetWindowRect(XRect &rect)
{
    rect.x = mainrect.x;
    rect.y = mainrect.y;
    rect.cx = mainrect.cx;
    rect.cy = mainrect.cy;
}


void   ENGINEAPI OSShowCursor(BOOL bShow)
{
    if(!bShow)  XDefineCursor((Display*)xdisplay, wndMain, None);
    else        XUndefineCursor((Display*)xdisplay, wndMain);
}

void   ENGINEAPI OSSetCursorPos(int x, int y)
{
    //SetCursorPos(x, y);
    XEvent event;
    zero(&event, sizeof(XEvent));

    event.type = MotionNotify;
    event.xmotion.type = MotionNotify;
    event.xmotion.send_event = 1;
    event.xmotion.display = (Display*)xdisplay;
    event.xmotion.window = DefaultRootWindow(xdisplay);
    event.xmotion.root = DefaultRootWindow(xdisplay);
    event.xmotion.subwindow = None;
    event.xmotion.x_root = x;
    event.xmotion.y_root = y;

    XSendEvent((Display*)xdisplay, DefaultRootWindow(xdisplay), 0, PointerMotionMask, &event);
}



BOOL   ENGINEAPI OSSetFullscreen(DisplayMode *dm)
{
    /*DEVMODE devmode;
    if(!dm)
        return (ChangeDisplaySettings(NULL, 0) == DISP_CHANGE_SUCCESSFUL);
    else
    {
        devmode.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY|DM_BITSPERPEL;
        devmode.dmSize = sizeof(DEVMODE);
        devmode.dmBitsPerPel = dm->dwBitsPerPixel;
        devmode.dmPelsHeight = dm->dwHeight;
        devmode.dmPelsWidth  = dm->dwWidth;
        devmode.dmDisplayFrequency = dm->dwFrequency;

        return (ChangeDisplaySettings(&devmode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL);
    }*/
    return 0;
}

List<DisplayMode> *ENGINEAPI OSEnumDisplayModes()
{
    /*List<DisplayMode> *lpValues = new List<DisplayMode>;
    DEVMODE dm;
    DWORD i=0;

    zero(&dm, sizeof(DEVMODE));
    dm.dmSize = sizeof(DEVMODE);

    while(EnumDisplaySettings(NULL, i++, &dm))
    {
        if(dm.dmBitsPerPel != 32)
            continue;

        for(i=0;i<lpValues->Num();i++)
        {
            if( (dm.dmPelsHeight == lpValues->array[i].dwHeight) && 
                (dm.dmPelsWidth  == lpValues->array[i].dwWidth))
            {
                if(dm.dmDisplayFrequency > lpValues->array[i].dwFrequency)
                {
                    lpValues->array[i].dwBitsPerPixel   = dm.dmBitsPerPel;
                    lpValues->array[i].dwFrequency      = dm.dmDisplayFrequency;
                    lpValues->array[i].dwHeight         = dm.dmPelsHeight;
                    lpValues->array[i].dwWidth          = dm.dmPelsWidth;
                }
                continue;
            }
        }

        DisplayMode *lpdm = lpValues->CreateNew();
        lpdm->dwBitsPerPixel   = dm.dmBitsPerPel;
        lpdm->dwFrequency      = dm.dmDisplayFrequency;
        lpdm->dwHeight         = dm.dmPelsHeight;
        lpdm->dwWidth          = dm.dmPelsWidth;
    }

    return lpValues;*/
    return 0;
}



HANDLE ENGINEAPI OSLoadLibrary(CTSTR lpFile)
{
    TCHAR FullFileName[250];

    scpy(FullFileName, lpFile);
    scat(FullFileName, TEXT(".so"));
    return (HANDLE)dlopen(FullFileName, RTLD_LAZY);
}

DEFPROC ENGINEAPI OSGetProcAddress(HANDLE hLibrary, CTSTR lpProcedure)
{
    return (DEFPROC)dlsym(hLibrary, lpProcedure);
}

void   ENGINEAPI OSFreeLibrary(HANDLE hLibrary)
{
    dlclose(hLibrary);
}


BASE_EXPOT  void   ENGINEAPI OSSleep(DWORD dwMSeconds)
{
    usleep(dwMSeconds*1000);
}


typedef void* (*PTHREADPROC)(void *);

HANDLE ENGINEAPI OSCreateThread(ENGINETHREAD lpThreadFunc, LPVOID param)
{
    HANDLE hThread;
    if(pthread_create((pthread_t*)&hThread, NULL, (PTHREADPROC)lpThreadFunc, param))
        return NULL;

    return hThread;
}

BOOL   ENGINEAPI OSWaitForThread(HANDLE hThread, LPDWORD ret)
{
    return !pthread_join((pthread_t)hThread, (LPVOID)ret));
}

BOOL   ENGINEAPI OSTerminateThread(HANDLE hThread)
{
    return !pthread_cancel((pthread_t)hThread);
}




DWORD  ENGINEAPI OSGetTime()
{
    return clock()/CLOCKS_PER_MSEC;
}



void ENGINEAPI InputProc(XEvent *event);

BOOL MainWndProc(Display *curdisplay, XEvent *event)
{
    InputProc(event);
    return 0;
}




#endif

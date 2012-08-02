/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    WindowsBase.h:  Windows header
  
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


#ifndef BASE_WINDOWS_HEADER
#define BASE_WINDOWS_HEADER

//-----------------------------------------
//warnings we don't want to hear over and over that have no relevance
//-----------------------------------------
#pragma warning(disable : 4018)
#pragma warning(disable : 4200)
#pragma warning(disable : 4244)
//#pragma warning(disable : 4699)
//#pragma warning(disable : 4245)
#pragma warning(disable : 4305)
#pragma warning(disable : 4711)
#pragma warning(disable : 4251)  //class '' needs to have dll-interface to be used by clients of class ''


#undef CreateFont
#undef DrawText


//-----------------------------------------
//defines
//-----------------------------------------
#if defined(NO_IMPORT)
    #define BASE_EXPORT
#elif !defined(BASE_EXPORTING)
    #define BASE_EXPORT     __declspec(dllimport)
#else
    #define BASE_EXPORT     __declspec(dllexport)
#endif

#define ENGINEAPI                  __stdcall
#define strcmpi                 lstrcmpi


#define FORCEINLINE             __forceinline


#define ENGINE_WINDOW_CLASS_NAME   TEXT("XR3DWindowClass")

#ifndef FORCE_TRACE
    #ifdef _DEBUG
        #undef USE_TRACE
    #endif
#else
    #define USE_TRACE 1
#endif


#ifndef USE_TRACE
    #define traceIn(name)
    #define traceOut
    #define traceInFast(name)
    #define traceOutFast
    #define traceOutStop
#else
    #define traceIn(name)       {static const TCHAR *__FUNC_NAME__ = TEXT(#name); try{
    #define traceOut            }catch(...){TraceCrash(__FUNC_NAME__); throw;}}
    #define traceOutStop        }catch(...){TraceCrash(__FUNC_NAME__); TraceCrashEnd();}}
#endif

#ifdef USE_TRACE
    #ifdef FULL_TRACE
        #define traceInFast(name)   traceIn(name)
        #define traceOutFast        traceOut
    #else
        #define traceInFast(name)
        #define traceOutFast
    #endif
#endif

//-----------------------------------------
//variables
//-----------------------------------------
BASE_EXPORT extern   HANDLE hwndMain;
BASE_EXPORT extern   HANDLE hinstMain;


//-----------------------------------------
//main window proc
//-----------------------------------------
#ifdef C64
typedef unsigned __int64 WPARAM;
typedef __int64 LPARAM,LRESULT;
#else
typedef unsigned int WPARAM;
typedef long LPARAM,LRESULT;
#endif
BASE_EXPORT LRESULT ENGINEAPI MainWndProc(HANDLE hwnd, UINT message, WPARAM wParam, LPARAM lParam);


//-----------------------------------------
//startup declarations
//-----------------------------------------

#define ProgramBreak() __debugbreak()
BASE_EXPORT TSTR*  ENGINEAPI OSGetCommandLine(int &numArgs);

#if defined(ENGINE_MAIN_PROGRAM)

    int ENGINEAPI EngineMain(TSTR *lpCommandLine, int c);

    #ifdef _INC_WINDOWS
    int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lp, int c)
    {
        hinstMain = (HANDLE)hInst;

        int numArgs;
        TSTR *commandLine = OSGetCommandLine(numArgs);
        int retVal = EngineMain(commandLine, numArgs);

        return retVal;
    }
    #else
    int ENGINEAPI WinMain(HANDLE hInst, HANDLE hPrevInst, LPSTR lp, int c)
    {
        hinstMain = (HANDLE)hInst;

        int numArgs;
        TSTR *commandLine = OSGetCommandLine(numArgs);
        int retVal = EngineMain(commandLine, numArgs);

        return retVal;
    }
    #endif

#elif defined(DLL_EXTENSION)

    BOOL ENGINEAPI EngineDLL(DWORD dwWhy);


    #ifdef _INC_WINDOWS
    BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwWhy, LPVOID lpReserved1)
    #else
    BOOL ENGINEAPI DllMain(HANDLE hInst, DWORD dwWhy, LPVOID lpReserved1)
    #endif
    {
        if(dwWhy == 1)
            return EngineDLL(DLL_LOADING);
        else if(dwWhy == 0)
            return EngineDLL(DLL_UNLOADING);

        return 1;
    }

#endif



//-----------------------------------------
//typedefs
//-----------------------------------------
typedef void (ENGINEAPI* DEFPROC)();
typedef DWORD (ENGINEAPI* ENGINETHREAD)(LPVOID);


#endif

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


#include <ft2build.h>
#include FT_FREETYPE_H



struct TickInfo
{
    TickInfo(DWORD dwTime, BOOL bGameTimer) : dwTime(dwTime), bGameTimer(bGameTimer) {}

    DWORD dwTime;
    BOOL bGameTimer;
};



////////////////
//globals
Alloc                   *MainAllocator  = NULL;
GraphicsSystem          *GS             = NULL;
SoundSystem             *SS             = NULL;
MusicManager            *MM             = NULL;
Engine                  *engine         = NULL;
Game                    *CurrentGame    = NULL;
BOOL                    bBaseLoaded     = 0;
BOOL                    bWrittenToLog   = 0;
BOOL                    bTimePaused     = 0;
BOOL                    bExiting        = 0;
double                  TimePauseStart  = 0.0;
double                  TimeSpeed       = 1.0;
SafeList<TickInfo>      TickList;
XFile                   LogFile;
ConfigFile              *AppConfig      = NULL;
BOOL                    bDebugBreak     = 0;
BOOL                    bErrorMessage   = 0;
int                     memoryBreakID   = -1;
int                     realtimeCounter = 0;
StringList              TraceFuncList;

BOOL ENGINEAPI ProgramOpened(TSTR name);

BOOL ENGINEAPI InitBase(TSTR lpConfig)
{
    if(bBaseLoaded)
        return FALSE;

    if(ProgramOpened(lpConfig))
        return FALSE;

    OSSetCurrentDirectory(TEXT("../"));

    TCHAR logName[256];
    scpy(logName, lpConfig);
    scat(logName, TEXT(".log"));

    //unsigned int ret;
    LogFile.Open(logName, XFILE_WRITE, XFILE_CREATEALWAYS);
    LogFile.Write("\xEF\xBB\xBF", 3);

#ifdef C64
    LogFile.WriteStr(TEXT("JimEngine (aka Jimjin, XR3D, etc) v2.51 64bit (　^ω^)\r\n"));
#else
    LogFile.WriteStr(TEXT("JimEngine (aka Jimjin, XR3D, etc) v2.51 32bit (´・ω・｀)\r\n"));
#endif
    LogFile.WriteStr(TEXT("==============================================================\r\n"));

    //----------------------------------------------------------------------

    TCHAR configName[256];
    scpy(configName, lpConfig);
    scat(configName, TEXT(".ini"));

    MainAllocator = new FastAlloc;

    OSLogSystemStats();

    AppConfig = new ConfigFile;
    if(!AppConfig->Open(configName))
    {
        TCHAR defaultName[256];
        scpy(defaultName, lpConfig);
        scat(defaultName, TEXT(".default.ini"));
        if(!OSCopyFile(configName, defaultName))
            CrashError(TEXT("Could not open ini file '%s' and was not able to copy the default ini '%s'"), configName, defaultName);

        if(!AppConfig->Open(configName))
            CrashError(TEXT("Could not open ini file '%s'"), configName);
    }

    String strAllocator = AppConfig->GetString(TEXT("Engine"), TEXT("Allocator"), TEXT("FastAlloc"));

    DWORD dwAllocator;

    if(strAllocator.CompareI(TEXT("SeriousMemoryDebuggingAlloc")))
        dwAllocator = 3;
    else if(strAllocator.CompareI(TEXT("FastAlloc")))
        dwAllocator = 0;
    else if(strAllocator.CompareI(TEXT("DebugAlloc")))
        dwAllocator = 1;
    else //DefaultAlloc
        dwAllocator = 2;

    strAllocator.Clear();

    if(dwAllocator)
    {
        delete AppConfig;
        delete MainAllocator;

        switch(dwAllocator)
        {
            case 1:
                MainAllocator = new DebugAlloc; break;
            case 2:
                MainAllocator = new DefaultAlloc; break;
            case 3:
                MainAllocator = new SeriousMemoryDebuggingAlloc; break;
        }

        AppConfig = new ConfigFile;
        AppConfig->Open(configName);
    }

    switch(dwAllocator)
    {
        case 0:
            Log(TEXT("Allocator set to FastAlloc"));  break;
        case 1:
            Log(TEXT("Allocator set to DebugAlloc")); break;
        case 2:
            Log(TEXT("Allocator set to DefaultAlloc")); break;
        case 3:
            Log(TEXT("Woa!  Time for some serious memory debugging")); break;
    }

    traceIn(InitBase);

    //----------------------------------------------------------------------

    memoryBreakID = AppConfig->GetInt(TEXT("Engine"), TEXT("BreakOnMemoryID"), -1);
    bDebugBreak = AppConfig->GetInt(TEXT("Engine"), TEXT("EnableBreakpoints"));

    //----------------------------------------------------------------------

    InitFontStuff();

    //----------------------------------------------------------------------

    String strEngineClass = AppConfig->GetString(TEXT("Engine"), TEXT("Engine"), TEXT("Engine"));

    engine = (Engine*)CreateFactoryObject(strEngineClass);
    if(!engine)
        CrashError(TEXT("Could not create engine class '%s'"), (TSTR)strEngineClass);

    //----------------------------------------------------------------------

    StringList moduleList;
    AppConfig->GetStringList(TEXT("Engine"), TEXT("GameModule"), moduleList);
    for(int i=0; i<moduleList.Num(); i++)
        LoadGameModule(moduleList[i]);

    //----------------------------------------------------------------------

    traceOutStop;

    bBaseLoaded = TRUE;

    return TRUE;
}

int ENGINEAPI BaseLoop(BOOL bRealTime)
{
    traceIn(BaseLoop);

    if(bRealTime)
        realtimeCounter++;

    return OSApplicationLoop();

    traceOutStop;
}

void ENGINEAPI TerminateBase()
{
    traceIn(TerminateBase);

    if(CurrentGame)
        DestroyObject(CurrentGame);
    DestroyObject(engine);
    engine = NULL;

    OSColorAdjust();

    TerminateObjectEngine();
    delete Scripting;

    delete AppConfig;
    AppConfig = NULL;

    DumpProfileData();

    UnloadAllModules();

    DestroyFontStuff();

    traceOutStop;

    TickList.Clear();
    FreeProfileData();

    delete locale;

    if(MainAllocator)
    {
        delete MainAllocator;
        MainAllocator = NULL;
    }

    LogFile.WriteStr(TEXT("Base Library Freed Sucessfully\r\n"));

    LogFile.Close();

    OSClipCursor(FALSE);

    bBaseLoaded = 0;
}

#define MAX_STACK_TRACE 1000

void ENGINEAPI TraceCrash(const TCHAR *trackName)
{
    TraceFuncList.Insert(0, trackName);

    if(TraceFuncList.Num() > MAX_STACK_TRACE)
        TraceFuncList.SetSize(MAX_STACK_TRACE);
}

void ENGINEAPI TraceCrashEnd()
{
    String strStackTrace = TEXT("\r\nException Fault - Stack Trace:");

    for(int i=0; i<TraceFuncList.Num(); i++)
    {
        if(i) strStackTrace << TEXT(" -> ");
        if(!(i%10)) strStackTrace << TEXT("\r\n    ");
        strStackTrace << TraceFuncList[i];
    }

    if(TraceFuncList.Num() == MAX_STACK_TRACE)
        strStackTrace << TEXT(" -> ...");

    Log(TEXT("%s\r\n"), strStackTrace.Array());

    OSMessageBox(TEXT("Error: Exception fault - See log files for more details."));

    TraceFuncList.Clear();
    CriticalExit();
}

void ENGINEAPI CriticalExit()
{
    OSSignalAppExit();

    if(GS)
        DestroyObject(GS);

    OSColorAdjust();

    Log(TEXT("Critical Exit: Freeing all memory.."));

    if(AppConfig)
    {
        AppConfig->Close();
        delete AppConfig;
        AppConfig = NULL;
    }

    UnloadAllModules();

    TickList.Clear();
    FreeProfileData();

    delete locale;

    if(MainAllocator)
    {
        MainAllocator->ErrorTermination();
        free(MainAllocator);
    }

    LogFile.WriteStr(TEXT("Critical Exit: Terminating..\r\n"));

    LogFile.Close();

    OSClipCursor(FALSE);

    bBaseLoaded = 0;

    OSCriticalExit();
}

void   ENGINEAPI EndProgram()
{
    OSSignalAppExit();
}

CTSTR  ENGINEAPI GetBinDir()
{
#ifdef C64
    return TEXT("64bit/");
#else
    return TEXT("32bit/");
#endif
}



double ENGINEAPI SetTimeSpeed(double speed)
{
    double lastSpeed = TimeSpeed;
    TimeSpeed = speed;
    return lastSpeed;
}

void   ENGINEAPI PauseTime(BOOL bPause)
{
    assert(bPause != bTimePaused);
    if(bPause == bTimePaused) return;

    if((bTimePaused = bPause))
        TimePauseStart = TimeSpeed;
    else
        TimeSpeed = TimePauseStart;
}

BOOL  ENGINEAPI IsTimePaused()
{
    return bTimePaused;
}


DWORD ENGINEAPI TrackTimeBegin(BOOL bGameTimer)
{
    return TickList.Add(TickInfo(OSGetTime(), bGameTimer));
}

//returns how much time has passed since TrackTimeBegin, modified by time speed.
DWORD ENGINEAPI TrackTimeEnd(DWORD timeID)
{
    double dTime = (double)(OSGetTime() - TickList[timeID].dwTime);
    DWORD dwTime;
    if(TickList[timeID].bGameTimer && CurrentGame)
        dwTime = (DWORD)(dTime * TimeSpeed * double(CurrentGame->GetGameSpeed()));
    else
        dwTime = (DWORD)(dTime * TimeSpeed);

    TickList.Remove(timeID);

    return dwTime;
}

DWORD ENGINEAPI TrackTimeRestart(DWORD timeID)
{
    DWORD curTime = OSGetTime();

    double dTime = (double)(curTime - TickList[timeID].dwTime);
    DWORD dwTime;
    if(TickList[timeID].bGameTimer && CurrentGame)
        dwTime = (DWORD)(dTime * TimeSpeed * double(CurrentGame->GetGameSpeed()));
    else
        dwTime = (DWORD)(dTime * TimeSpeed);

    TickList[timeID].dwTime = curTime;

    return dwTime;
}


void __cdecl Logva(const TCHAR *format, va_list argptr)
{
    if(LogFile.IsOpen())
    {
        String strOut = FormattedStringva(format, argptr);
        LogFile.WriteAsUTF8(strOut, strOut.Length());
        LogFile.WriteAsUTF8(TEXT("\r\n"));

        LogFile.FlushWorthlessPieceOfGarbageFileBuffers();
    }

    bWrittenToLog = 1;
}

void __cdecl Log(const TCHAR *format, ...)
{
    va_list arglist;

    va_start(arglist, format);

    if(LogFile.IsOpen())
        Logva(format, arglist);
}

void __cdecl AppWarning(const TCHAR *format, ...)
{
    va_list arglist;

    va_start(arglist, format);

    if(LogFile.IsOpen())
    {
        LogFile.WriteStr(TEXT("Warning -- "));

        if(LogFile.IsOpen())
        {
            String strOut = FormattedStringva(format, arglist);
            LogFile.WriteAsUTF8(strOut, strOut.Length());
            LogFile.WriteAsUTF8(TEXT("\r\n"));
        }

        bWrittenToLog = 1;
    }

    OSDebugOut(TEXT("Warning -- "));
    OSDebugOutva(format, arglist);
    OSDebugOut(TEXT("\r\n"));

    //------------------------------------------------------
    // NOTE:
    // If you're seeting this, you can safely continue running, but I recommend fixing whatever's causing this warning.
    //
    // The debug output window contains the warning that has occured.
    //------------------------------------------------------

#if defined(_DEBUG) && defined(_WIN32)
    if(bDebugBreak && OSDebuggerPresent())
    {
        OSColorAdjust();
        ProgramBreak();
    }
#endif
}

void __cdecl CrashError(const TCHAR *format, ...)
{
    va_list arglist;

    va_start(arglist, format);

    if(LogFile.IsOpen())
    {
        LogFile.WriteStr(TEXT("\r\n======================================================\r\n"));
        LogFile.WriteStr(TEXT("\r\nError: "));

        Logva(format, arglist);

        LogFile.WriteStr(TEXT("\r\n"));
    }

    OSMessageBoxva(format, arglist);

#ifndef USE_TRACE

#if defined(_DEBUG) && defined(_WIN32)
    if(bDebugBreak && OSDebuggerPresent())
    {
        OSColorAdjust();
        ProgramBreak();
    }
#endif

    CriticalExit();
#else
    throw 1;
#endif
}

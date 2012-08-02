/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Main.cpp: script compiler main source file

  Copyright (c) 2009, Hugh Bailey
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


#include <stdio.h>
#include <Base.h>


class ScriptCompilerEngine : public Engine
{
    DeclareClass(ScriptCompilerEngine, Engine);

public:
    void Init()
    {
        traceIn(ScriptCompilerEngine::Init);

        Log(TEXT("Script Compiler Initialized"));

        engine = this;

        bEditor = TRUE;

        locale = new LocaleStringLookup;

        int numArgs;
        TSTR *params = OSGetCommandLine(numArgs);

        StringList CompileModules;

        Scripting = new ScriptSystem;

        DWORD timeID = TrackTimeBegin(FALSE);

        StringList LoadFiles;
        AppConfig->GetStringList(TEXT("Compiler"), TEXT("Load"), LoadFiles);
        AppConfig->GetStringList(TEXT("Compiler"), TEXT("Compile"), CompileModules);
        CompileModules.RemoveDupesI();

        String errorList;
        BOOL bGenerateHeaders = AppConfig->GetInt(TEXT("Compiler"), TEXT("Source"), 0);
        BOOL bGenerateBytecode = FALSE;
        BOOL bSetLoadModules = FALSE;

        BOOL bGetHelp = FALSE;  //meaning a psychiatrist.

        int curArg = 1;
        if(numArgs == 1 && !LoadFiles.Num() && !CompileModules.Num()) bGetHelp = TRUE;
        while(curArg < numArgs)
        {
            if(params[curArg][0] == '-')
            {
                if(scmpi(params[curArg], TEXT("-source")) == 0)
                    bGenerateHeaders = TRUE;
                else if(scmpi(params[curArg], TEXT("-bytecode")) == 0)
                    bGenerateBytecode = TRUE;
                else if(scmpi(params[curArg], TEXT("-load")) == 0)
                    bSetLoadModules = TRUE;
                else if(scmpi(params[curArg], TEXT("-compile")) == 0)
                    bSetLoadModules = FALSE;
                else if( (scmpi(params[curArg], TEXT("-help")) == 0) ||
                         (scmpi(params[curArg], TEXT("-h")) == 0) )
                {
                    bGetHelp = TRUE;
                }
                else
                {
                    wprintf(TEXT("Unknown parameter '%s'\r\n"), params[curArg]);
                    Log(TEXT("Unknown parameter '%s'\r\n"), params[curArg]);
                }
            }
            else if(bSetLoadModules)
                LoadFiles.SafeAddI(params[curArg]);
            else
                CompileModules.SafeAddI(params[curArg]);

            ++curArg;
        }

        if(bGetHelp)
        {
            wprintf(TEXT("Syntax:\r\n    ScriptCompiler [-source] [-load module1 module2 ...] [-compile module3 module4 ...]\r\n\r\n"));
            wprintf(TEXT("  -source: Generates cpp/h files inside the ScriptOutput directory for\r\n           native functions/classes\r\n\r\n"));
            wprintf(TEXT("  -load [module-files]: Loads modules files to be used as dependancies\r\n           for compiling.\r\n\r\n"));
            wprintf(TEXT("  -compile [module-files]: Compiles modules and reports any\r\n           errors/warnings, as well as generates source/heads if -source\r\n           is used in conjunction.\r\n\r\n"));
            TrackTimeEnd(timeID);
            delete Scripting;
            Scripting = NULL;
            return;
        }

        for(int i=0; i<LoadFiles.Num(); i++)
            LoadGameModule(LoadFiles[i]);

        for(int i=0; i<CompileModules.Num(); i++)
        {
            wprintf(TEXT("Compiling %s...\r\n"), CompileModules[i].Array());
            Log(TEXT("Compiling %s...\r\n"), CompileModules[i].Array());

            Scripting->LoadModuleScriptData(CompileModules[i], errorList, bGenerateBytecode, bGenerateHeaders);

            if(!errorList.IsEmpty())
            {
                wprintf(TEXT("%s"), errorList.Array());
                Log(TEXT("%s"), errorList.Array());
            }
            else
            {
                DWORD totalTimeTaken = TrackTimeRestart(timeID);
                float fSeconds = MSToSeconds(totalTimeTaken);

                wprintf(TEXT("Compilation of module '%s' successful, compiled in %g seconds.\r\n"), CompileModules[i].Array(), fSeconds);
                Log(TEXT("Compilation of module '%s' successful, compiled in %g seconds.\r\n"), CompileModules[i].Array(), fSeconds);
            }
        }

        TrackTimeEnd(timeID);

        delete Scripting;
        Scripting = NULL;

        traceOut;
    }

    void Destroy()
    {
        Log(TEXT("Script Compiler Destroyed"));
    }
};

DefineClass(ScriptCompilerEngine);

int main()
{
    if(!InitBase(TEXT("ScriptCompiler")))
        return 0;

    TerminateBase();

    return 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Main.cpp:  Main Game Progam

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


//This is always defined in the main program cpp file.
#define ENGINE_MAIN_PROGRAM
//#include <windows.h>
#include <Base.h>
#include "..\resource1.h"


int ENGINEAPI EngineMain(TSTR *lp, int c)
{
    int exitcode=0;

    dwIconID = IDI_GAME;

    //ugh.
    //CoInitializeEx(NULL, 0);

    if(InitBase(TEXT("Game")))
    {
        DWORD newWidth, newHeight, newFreq;
        BOOL bUseFullscreen = AppConfig->GetInt(TEXT("Display"), TEXT("Fullscreen"));

        if(!bUseFullscreen)
        {
            DisplayMode currentMode;
            OSGetDisplaySettings(&currentMode);

            if(currentMode.dwBitsPerPixel != 32)
                ErrOut(TEXT("Your display settings must be set to 32bit mode."));
        }

        SS->SetEffectsVol(AppConfig->GetFloat(TEXT("Sound"), TEXT("EffectsVolume"), 1.0f));
        MM->SetVolume(AppConfig->GetFloat(TEXT("Sound"), TEXT("MusicVolume"), 1.0f));

        List<DisplayMode> DisplayModes;
        zero(&DisplayModes, 8);

        OSEnumDisplayModes(DisplayModes);

        newWidth  = AppConfig->GetInt(TEXT("Display"), TEXT("Width"));
        newHeight = AppConfig->GetInt(TEXT("Display"), TEXT("Height"));
        newFreq   = AppConfig->GetInt(TEXT("Display"), TEXT("Frequency"));

        BOOL bFoundDisplayMode = FALSE;
        DisplayMode mode;

        for(int i=0; i<DisplayModes.Num(); i++)
        {
            if( (DisplayModes[i].dwHeight == newHeight) &&
                (DisplayModes[i].dwWidth  == newWidth) )
            {
                if(DisplayModes[i].dwFrequency == newFreq)
                {
                    bFoundDisplayMode = TRUE;
                    mcpy(&mode, &DisplayModes[i], sizeof(DisplayMode));
                    break;
                }
            }
        }

        mode.dwBitsPerPixel = 32;

        if(!bFoundDisplayMode)
        {
            mode.dwFrequency = 60;
            mode.dwWidth     = 640;
            mode.dwHeight    = 480;
            Log(TEXT("Monitor does not support %dx%d resolution or doesn't support %d frequency"), newHeight, newWidth, newFreq);
        }

        SetResolution(mode, TRUE);

        if(bUseFullscreen)
            ToggleFullScreen();

        DisplayModes.Clear();

        float gamma      = AppConfig->GetFloat(TEXT("Display"), TEXT("Gamma"),      1.0f);
        float brightness = AppConfig->GetFloat(TEXT("Display"), TEXT("Brightness"), 1.0f);
        float contrast   = AppConfig->GetFloat(TEXT("Display"), TEXT("Contrast"),   1.0f);
        AdjustDisplayColors(gamma, brightness, contrast);

        GS->ClearColorBuffer();

        GameModule *module = (GameModule*)CreateFactoryObject(TEXT("MainGameModule"));
        if(!module)
            OSMessageBox(TEXT("Could not load the class 'MainGameModule'."));
        else
        {
            if(module->ModuleStartup(lp, c))
            {
                //loop
                exitcode = BaseLoop();

                module->ModuleExit();
            }

            DestroyObject(module);
        }

        TerminateBase();
    }

    //CoUninitialize();

    return 0;
}


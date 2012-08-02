/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  DXInput.h:  Direct Input

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

#ifdef dxinputshit

#ifdef WIN32

#define DIRECTINPUT_VERSION 0x800

#define _WIN32_WINDOWS 0x0410
#define _WIN32_WINNT   0x0403
#include <windows.h>
#include <dinput.h>
#include "Base.h"


DefineClass(DXInput);



unsigned char ENGINEAPI GetDIKInputCode(unsigned int dikcode);
void WINAPI DICreateMouseDevice(HANDLE window);
void WINAPI DICreateKeyboardDevice(HANDLE window);
void WINAPI DIDestroyMouseDevice();
void WINAPI DIDestroyKeyboardDevice();
DWORD WINAPI InputThread(LPVOID lpVoid);



unsigned char DIKtoKB[0x65] = {
DIK_ESCAPE          ,
DIK_1               ,
DIK_2               ,
DIK_3               ,
DIK_4               ,
DIK_5               ,
DIK_6               ,
DIK_7               ,
DIK_8               ,
DIK_9               ,
DIK_0               ,
DIK_MINUS           ,
DIK_EQUALS          ,
DIK_BACK            ,
DIK_TAB             ,
DIK_Q               ,
DIK_W               ,
DIK_E               ,
DIK_R               ,
DIK_T               ,
DIK_Y               ,
DIK_U               ,
DIK_I               ,
DIK_O               ,
DIK_P               ,
DIK_LBRACKET        ,
DIK_RBRACKET        ,
DIK_RETURN          ,
DIK_LCONTROL        ,
DIK_A               ,
DIK_S               ,
DIK_D               ,
DIK_F               ,
DIK_G               ,
DIK_H               ,
DIK_J               ,
DIK_K               ,
DIK_L               ,
DIK_SEMICOLON       ,
DIK_APOSTROPHE      ,
DIK_GRAVE           ,
DIK_LSHIFT          ,
DIK_BACKSLASH       ,
DIK_Z               ,
DIK_X               ,
DIK_C               ,
DIK_V               ,
DIK_B               ,
DIK_N               ,
DIK_M               ,
DIK_COMMA           ,
DIK_PERIOD          ,
DIK_SLASH           ,
DIK_RSHIFT          ,
DIK_MULTIPLY        ,
DIK_LMENU           ,
DIK_SPACE           ,
DIK_CAPITAL         ,
DIK_F1              ,
DIK_F2              ,
DIK_F3              ,
DIK_F4              ,
DIK_F5              ,
DIK_F6              ,
DIK_F7              ,
DIK_F8              ,
DIK_F9              ,
DIK_F10             ,
DIK_NUMLOCK         ,
DIK_SCROLL          ,
DIK_NUMPAD7         ,
DIK_NUMPAD8         ,
DIK_NUMPAD9         ,
DIK_SUBTRACT        ,
DIK_NUMPAD4         ,
DIK_NUMPAD5         ,
DIK_NUMPAD6         ,
DIK_ADD             ,
DIK_NUMPAD1         ,
DIK_NUMPAD2         ,
DIK_NUMPAD3         ,
DIK_NUMPAD0         ,
DIK_DECIMAL         ,
DIK_F11             ,
DIK_F12             ,
DIK_NUMPADENTER     ,
DIK_RCONTROL        ,
DIK_DIVIDE          ,
DIK_SYSRQ           ,
DIK_RMENU           ,
DIK_PAUSE           ,
DIK_HOME            ,
DIK_UP              ,
DIK_PRIOR           ,
DIK_LEFT            ,
DIK_RIGHT           ,
DIK_END             ,
DIK_DOWN            ,
DIK_NEXT            ,
DIK_INSERT          ,
DIK_DELETE
};


BOOL    bInputExiting=0, bDInputActive=0;
HANDLE  InputEvents[2];  //0=KB, 1=Mouse
HANDLE  hInputThread=NULL;
unsigned char   keys[256];
DIMOUSESTATE    mousestate;

#define KEYBOARD_EVENT_TRIGGER  (WAIT_OBJECT_0)
#define MOUSE_EVENT_TRIGGER     (WAIT_OBJECT_0+1)


/*=========================================================
    DXInput
===========================================================*/

LPDIRECTINPUT8 diDevice = NULL;
LPDIRECTINPUTDEVICE8 diKeyboard = NULL, diMouse = NULL;

extern int realtimeCounter;

DXInput::DXInput()
{
    traceIn(DXInput::DXInput);

    Log(TEXT("Initializing DirectInput"));

    ++realtimeCounter;

    bDInputActive = 1;

    if(DirectInput8Create((HINSTANCE)hinstMain, DIRECTINPUT_VERSION, IID_IDirectInput8A, (void**)&diDevice, NULL) != DI_OK)
        CrashError(TEXT("DInput: Could not create Direct Input device"));

    InputEvents[0] = CreateEvent(NULL, 0, 0, NULL);
    InputEvents[1] = CreateEvent(NULL, 0, 0, NULL);

    DICreateKeyboardDevice(hwndMain);
    DICreateMouseDevice(hwndMain);

    //DWORD dummy;
    //hInputThread = CreateThread(NULL, 0, InputThread, NULL, 0, &dummy);
    //if(hInputThread == INVALID_HANDLE_VALUE)
    //    CrashError(TEXT("DInput: could not create input thread"));

    traceOut;
}

void WINAPI DICreateMouseDevice(HANDLE window)
{
    traceIn(DICreateMouseDevice);

    if(bDInputActive)
    {
        HRESULT result;

        if(diDevice->CreateDevice(GUID_SysMouse, &diMouse, NULL) != DI_OK)
            CrashError(TEXT("DInput: Could not Create Mouse device"));
        if(diMouse->SetDataFormat(&c_dfDIMouse) != DI_OK)
            CrashError(TEXT("DInput: Could not set mouse data format"));
        if(diMouse->SetCooperativeLevel((HWND)window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE) != DI_OK)
            CrashError(TEXT("DInput: Could not set mouse cooperation level"));

        if((result = diMouse->SetEventNotification(InputEvents[1])) != DI_OK)
            CrashError(TEXT("DInput: couldn't set a mouse notification.  that sucks."));

        if((result = diMouse->Acquire()) != DI_OK)
        {
            if(result != DIERR_OTHERAPPHASPRIO)
                CrashError(TEXT("DInput: Could not aquire mouse device"));
        }

        if((result = diMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mousestate)) != DI_OK)
        {
            if((result != DIERR_OTHERAPPHASPRIO) && (result != DIERR_NOTACQUIRED))
                CrashError(TEXT("DInput: Could not get mouse state"));

            curMouseButtonStates = 0;
        }
        else
        {
            if(mousestate.rgbButtons[0])
                curMouseButtonStates |= STATE_LBUTTONDOWN;
            if(mousestate.rgbButtons[1])
                curMouseButtonStates |= STATE_MBUTTONDOWN;
            if(mousestate.rgbButtons[2])
                curMouseButtonStates |= STATE_RBUTTONDOWN;
        }
    }

    traceOut;
}

void WINAPI DICreateKeyboardDevice(HANDLE window)
{
    traceIn(DICreateKeyboardDevice);

    if(bDInputActive)
    {
        HRESULT result;

        if(diDevice->CreateDevice(GUID_SysKeyboard, &diKeyboard, NULL) != DI_OK)
            CrashError(TEXT("DInput: Could not Create Keyboard device"));

        if(diKeyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK)
            CrashError(TEXT("DInput: Could not set keyboard data format"));

        if(diKeyboard->SetCooperativeLevel((HWND)window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE) != DI_OK)
            CrashError(TEXT("DInput: Could not set keyboard cooperation level"));


        if((result = diKeyboard->SetEventNotification(InputEvents[0])) != DI_OK)
            CrashError(TEXT("DInput: couldn't set a keyboard notification.  that sucks."));

        if((result = diKeyboard->Acquire()) != DI_OK)
        {
            if(result != DIERR_OTHERAPPHASPRIO)
                CrashError(TEXT("DInput: Could not aquire keyboard device"));
        }

        if((result = diKeyboard->GetDeviceState(256, keys)) != DI_OK)
        {
            if((result != DIERR_OTHERAPPHASPRIO) && (result != DIERR_NOTACQUIRED))
                CrashError(TEXT("DInput: Could not get keyboard device state"));
        }
        /*else
        {
            keys[DIK_LSHIFT] |= keys[DIK_RSHIFT];
            keys[DIK_RSHIFT] = 0;

            keys[DIK_LCONTROL] |= keys[DIK_RCONTROL];
            keys[DIK_RCONTROL] = 0;
        }*/
    }

    traceOut;
}

void WINAPI DIDestroyMouseDevice()
{
    if(bDInputActive)
    {
        diMouse->Unacquire();
        diMouse->Release();
    }
}

void WINAPI DIDestroyKeyboardDevice()
{
    if(bDInputActive)
    {
        diKeyboard->Unacquire();
        diKeyboard->Release();
    }
}

DXInput::~DXInput()
{
    traceIn(DXInput::~DXInput);

    bInputExiting = 1;

    SetEvent(InputEvents[0]);
    SetEvent(InputEvents[1]);
    //WaitForSingleObject(hInputThread, 5000);
    //CloseHandle(hInputThread);
    diKeyboard->SetEventNotification(NULL);
    diMouse->SetEventNotification(NULL);
    CloseHandle(InputEvents[0]);
    CloseHandle(InputEvents[1]);

    DIDestroyKeyboardDevice();
    DIDestroyMouseDevice();
    diDevice->Release();

    bDInputActive = 0;

    Log(TEXT("DirectInput Freed"));

    traceOut;
}

BOOL DXInput::GetButtonState(unsigned int key)
{
    if(key == KBC_CONTROL)
        return (keys[DIKtoKB[KBC_LCONTROL]] & 0x80) || (keys[DIKtoKB[KBC_RCONTROL]] & 0x80);
    else if(key == KBC_ALT)
        return (keys[DIKtoKB[KBC_LALT]] & 0x80)     || (keys[DIKtoKB[KBC_RALT]] & 0x80);
    else if(key == KBC_SHIFT)
        return (keys[DIKtoKB[KBC_LSHIFT]] & 0x80)   || (keys[DIKtoKB[KBC_RSHIFT]] & 0x80);
    else
        return (keys[DIKtoKB[key]] & 0x80);
}



/*=========================================================
    Other
===========================================================*/

unsigned char ENGINEAPI GetDIKInputCode(unsigned int dikcode)
{
    for(unsigned char i=0; i<0x65; i++)
    {
        if(DIKtoKB[i] == dikcode)
            return i;
    }

    return 0xFF;
}

void ENGINEAPI DIWindowActivate(BOOL bActive)
{
    if(bDInputActive)
    {
        if(bActive)
        {
            if(diKeyboard)
                diKeyboard->Acquire();
            if(diMouse)
                diMouse->Acquire();
        }
    }
}


void DXInput::ProcessInput()
{
    traceIn(DXInput::ProcessInput);

    DWORD   dwEventStatus;
    HRESULT result;

    while(!bInputExiting && ((dwEventStatus = WaitForMultipleObjects(2, InputEvents, FALSE, 0)) != WAIT_TIMEOUT))
    {
        if(bInputExiting)
            break;

        if(dwEventStatus == KEYBOARD_EVENT_TRIGGER)
        {
            unsigned char newkeys[256];

            if((result = diKeyboard->GetDeviceState(256, newkeys)) != DI_OK)
            {
                if((result != DIERR_INPUTLOST) && (result != DIERR_OTHERAPPHASPRIO) && (result != DIERR_NOTACQUIRED))
                    AppWarning(TEXT("DInput: Could not get keyboard device state"));
                continue;
            }

            if(bIgnoreNextKeyFrame)
            {
                bIgnoreNextKeyFrame = FALSE;
                continue;
            }

            /*newkeys[DIK_LSHIFT] |= newkeys[DIK_RSHIFT];
            newkeys[DIK_RSHIFT] = 0;

            newkeys[DIK_LCONTROL] |= newkeys[DIK_RCONTROL];
            newkeys[DIK_RCONTROL] = 0;*/

            for(int i=0; i<256; i++)
            {
                if(keys[i] != newkeys[i])
                {
                    if(!curKBHandler.Num() || !curKBHandler[0]->bCharInput)
                    {
                        if(curKBHandler.Num())
                            curKBHandler[0]->KeyboardHandler(GetDIKInputCode(i), ((newkeys[i] & 0x80) != 0));
                        else
                            ControlWindow::WindowKeyboardHandler(GetDIKInputCode(i), ((newkeys[i] & 0x80) != 0), GS);
                    }
                    keys[i] = newkeys[i];
                }
            }
        }
        else if(dwEventStatus == MOUSE_EVENT_TRIGGER)
        {
            DIMOUSESTATE newMouseState; //6x25

            if((result = diMouse->GetDeviceState(sizeof(DIMOUSESTATE), &newMouseState)) != DI_OK)
            {
                if((result != DIERR_INPUTLOST) && (result != DIERR_OTHERAPPHASPRIO) && (result != DIERR_NOTACQUIRED))
                    AppWarning(TEXT("DInput: Could not get keyboard device state"));
                continue;
            }

            if((newMouseState.lX != 0) || (newMouseState.lY != 0))
            {
                if(curMouseHandler.Num())
                    curMouseHandler[0]->MouseHandler(MOUSE_MOVE, curMouseButtonStates, MAKELPARAM(newMouseState.lX, mousestate.lY));
                else
                    ControlWindow::WindowMouseHandler(MOUSE_MOVE, curMouseButtonStates, MAKELPARAM(newMouseState.lX, mousestate.lY), GS);
            }
            if(newMouseState.lZ != 0)
            {
                if(curMouseHandler.Num())
                    curMouseHandler[0]->MouseHandler(MOUSE_WHEEL, curMouseButtonStates, (int)newMouseState.lZ);
                else
                     ControlWindow::WindowMouseHandler(MOUSE_WHEEL, curMouseButtonStates, (int)newMouseState.lZ, GS);
            }
            if(newMouseState.rgbButtons[0] != mousestate.rgbButtons[0])
            {
                if(newMouseState.rgbButtons[0])
                    curMouseButtonStates |= STATE_LBUTTONDOWN;
                else
                    curMouseButtonStates &= ~STATE_LBUTTONDOWN;

                BOOL bDown = ((curMouseButtonStates & STATE_LBUTTONDOWN) != 0);
                if(curMouseHandler.Num())
                    curMouseHandler[0]->MouseHandler(MOUSE_LEFTBUTTON, curMouseButtonStates, (int)bDown);
                else
                     ControlWindow::WindowMouseHandler(MOUSE_LEFTBUTTON, curMouseButtonStates, (int)bDown, GS);
            }
            if(newMouseState.rgbButtons[1] != mousestate.rgbButtons[1])
            {
                if(newMouseState.rgbButtons[1])
                    curMouseButtonStates |= STATE_RBUTTONDOWN;
                else
                    curMouseButtonStates &= ~STATE_RBUTTONDOWN;

                BOOL bDown = ((curMouseButtonStates & STATE_RBUTTONDOWN) != 0);
                if(curMouseHandler.Num())
                    curMouseHandler[0]->MouseHandler(MOUSE_RIGHTBUTTON, curMouseButtonStates, (int)bDown);
                else
                     ControlWindow::WindowMouseHandler(MOUSE_RIGHTBUTTON, curMouseButtonStates, (int)bDown, GS);
            }
            if(newMouseState.rgbButtons[2] != mousestate.rgbButtons[2])
            {
                if(newMouseState.rgbButtons[2])
                    curMouseButtonStates |= STATE_MBUTTONDOWN;
                else
                    curMouseButtonStates &= ~STATE_MBUTTONDOWN;

                BOOL bDown = ((curMouseButtonStates & STATE_MBUTTONDOWN) != 0);
                if(curMouseHandler.Num())
                    curMouseHandler[0]->MouseHandler(MOUSE_MIDDLEBUTTON, curMouseButtonStates, (int)bDown);
                else
                     ControlWindow::WindowMouseHandler(MOUSE_MIDDLEBUTTON, curMouseButtonStates, (int)bDown, GS);
            }

            mcpy(&mousestate, &newMouseState, sizeof(DIMOUSESTATE));
        }
    }

    Super::ProcessInput();

    //return 0;

    traceOut;
}


#endif

#endif

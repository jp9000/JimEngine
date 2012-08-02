/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  InputWindows.h:  Input Devices

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

#define _WIN32_WINDOWS 0x0410
#define _WIN32_WINNT   0x0403
#include <windows.h>
#include "Base.h"


DefineClass(StandardInput);


unsigned char ENGINEAPI GetVKInputCode(unsigned int vkcode);
void ENGINEAPI ProcessStandardInput(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL            bInputActive=0;
BOOL            KeyDown[0x65];


unsigned char VKtoKB[0x65] = {

VK_ESCAPE     ,
'1'           ,
'2'           ,
'3'           ,
'4'           ,
'5'           ,
'6'           ,
'7'           ,
'8'           ,
'9'           ,
'0'           ,
VK_OEM_MINUS  ,
VK_OEM_PLUS   ,
VK_BACK       ,
VK_TAB        ,
'Q'           ,
'W'           ,
'E'           ,
'R'           ,
'T'           ,
'Y'           ,
'U'           ,
'I'           ,
'O'           ,
'P'           ,
VK_OEM_4      ,
VK_OEM_6      ,
VK_RETURN     ,
VK_LCONTROL   ,
'A'           ,
'S'           ,
'D'           ,
'F'           ,
'G'           ,
'H'           ,
'J'           ,
'K'           ,
'L'           ,
VK_OEM_1      , 
VK_OEM_7      ,
VK_OEM_3      ,
VK_LSHIFT     ,
VK_OEM_5      ,
'Z'           ,
'X'           ,
'C'           ,
'V'           ,
'B'           ,
'N'           ,
'M'           ,
VK_OEM_COMMA  ,
VK_OEM_PERIOD ,
VK_DIVIDE     ,
VK_RSHIFT     ,
VK_MULTIPLY   ,
VK_LMENU      ,
VK_SPACE      ,
VK_CAPITAL    ,
VK_F1         ,
VK_F2         ,
VK_F3         ,
VK_F4         ,
VK_F5         ,
VK_F6         ,
VK_F7         ,
VK_F8         ,
VK_F9         ,
VK_F10        ,
VK_NUMLOCK    ,
VK_SCROLL     ,
VK_NUMPAD7    ,
VK_NUMPAD8    ,
VK_NUMPAD9    ,
VK_SUBTRACT   ,
VK_NUMPAD4    ,
VK_NUMPAD5    ,
VK_NUMPAD6    ,
VK_ADD        ,
VK_NUMPAD1    ,
VK_NUMPAD2    ,
VK_NUMPAD3    ,
VK_NUMPAD0    ,
VK_DECIMAL    ,
VK_F11        ,
VK_F12        ,
VK_RETURN     ,
VK_RCONTROL   ,
VK_DIVIDE     ,
VK_SNAPSHOT   ,
VK_RMENU      ,
VK_PAUSE      ,
VK_HOME       ,
VK_UP         ,
VK_PRIOR      ,
VK_LEFT       ,
VK_RIGHT      ,
VK_END        ,
VK_DOWN       ,
VK_NEXT       ,
VK_INSERT     ,
VK_DELETE
};




/*=========================================================
    StandardInput
===========================================================*/

StandardInput::StandardInput()
{
    zero(KeyDown, 65);
    bInputActive = 1;
}

BOOL StandardInput::GetButtonState(unsigned int key)
{
    if(key == KBC_CONTROL)
        return GetButtonState(KBC_LCONTROL) || GetButtonState(KBC_RCONTROL);
    else if(key == KBC_SHIFT)
        return GetButtonState(KBC_LSHIFT) || GetButtonState(KBC_RSHIFT);
    else if(key == KBC_ALT)
        return GetButtonState(KBC_LALT) || GetButtonState(KBC_RALT);

    if(!curKBHandler.Num() || !curKBHandler[0]->bCharInput)
        return bInputActive ? HIBYTE(GetKeyState(VKtoKB[key])) : 1;
    else
        return 0;
}

/*=========================================================
    Other
===========================================================*/

unsigned char ENGINEAPI GetVKInputCode(unsigned int vkcode)
{
    if(vkcode == VK_CONTROL)
        vkcode = VK_LCONTROL;

    for(unsigned char i=0; i<0x65; i++)
        {if(VKtoKB[i] == vkcode) return i;}

    return 0xFF;
}


BOOL bIsCapturing=0;

void ENGINEAPI SIWindowActivate(BOOL bActive)
{
    bInputActive = bActive;
    bIsCapturing = 0;
    ReleaseCapture();
}

extern int realtimeCounter;

void ENGINEAPI ProcessStandardInput(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(ProcessStandardInput);

    GraphicsSystem *curSystem = (GraphicsSystem*)(ULONG_PTR)GetWindowLongPtr(hwnd, 0);

    if(!curSystem)
        return;

    Input *userInput = curSystem->GetInput();

    if(!userInput)
        return;

    KeyboardInputHandler *curKBHandler    = userInput->GetCurKBHandler();
    MouseInputHandler    *curMouseHandler = userInput->GetCurMouseHandler();

    unsigned char key;
    POINT point;
    short x_drag, y_drag;

    key = GetVKInputCode(wParam);

    StandardInput* sInput = (StandardInput*)userInput;

    switch(message)
    {
        case WM_CHAR:
            if(curKBHandler && curKBHandler->bCharInput)
                curKBHandler->KeyboardHandler(wParam, 1);
            else return;

            break;

        case WM_KEYDOWN:
            if((wParam == VK_SHIFT) || (wParam == VK_RSHIFT))
                wParam = VK_LSHIFT;

            if(key != 0xFF && !KeyDown[key])
            {
                KeyDown[key] = 1;
                if(curKBHandler)
                    curKBHandler->KeyboardHandler(key, 1);
                else
                    ControlWindow::WindowKeyboardHandler(key, 1, GS);
            }
            else return;

            break;

        case WM_KEYUP:
            if((wParam == VK_SHIFT) || (wParam == VK_RSHIFT))
                wParam = VK_LSHIFT;

            if(key != 0xFF && KeyDown[key])
            {
                KeyDown[key] = 0;
                if(curKBHandler)
                    curKBHandler->KeyboardHandler(key, 0);
                else
                    ControlWindow::WindowKeyboardHandler(key, 0, GS);
            }
            else return;

            break;

        case WM_MOUSEMOVE:
            //x, y);
            point.x = LOWORD(lParam);
            point.y = HIWORD(lParam);

            x_drag = (short)(point.x-sInput->last_x);
            y_drag = (short)(point.y-sInput->last_y);

            if(!x_drag && !y_drag) break;

            sInput->last_x = point.x;
            sInput->last_y = point.y;

            if(curMouseHandler)
                curMouseHandler->MouseHandler(MOUSE_MOVE, curMouseButtonStates, MAKELPARAM(x_drag, y_drag));
            else
                ControlWindow::WindowMouseHandler(MOUSE_MOVE, curMouseButtonStates, MAKELPARAM(x_drag, y_drag), curSystem);
            break;

        case WM_MOUSEWHEEL:
            //0, dist);
            if(curMouseHandler)
                curMouseHandler->MouseHandler(MOUSE_WHEEL, curMouseButtonStates, (int)(short)HIWORD(wParam));
            else
                ControlWindow::WindowMouseHandler(MOUSE_WHEEL, curMouseButtonStates, (int)(short)HIWORD(wParam), curSystem);
            break;

        case WM_LBUTTONDOWN:
            //0, down);
            SetCapture((HWND)hwnd);
            ++bIsCapturing;

            curMouseButtonStates |= STATE_LBUTTONDOWN;
            if(curMouseHandler)
                curMouseHandler->MouseHandler(MOUSE_LEFTBUTTON, curMouseButtonStates, 1);
            else
                ControlWindow::WindowMouseHandler(MOUSE_LEFTBUTTON, curMouseButtonStates, 1, curSystem);
            break;

        case WM_MBUTTONDOWN:
            //0, down);
            SetCapture((HWND)hwnd);
            ++bIsCapturing;

            curMouseButtonStates |= STATE_MBUTTONDOWN;
            if(curMouseHandler)
                curMouseHandler->MouseHandler(MOUSE_MIDDLEBUTTON, curMouseButtonStates, 1);
            else
                ControlWindow::WindowMouseHandler(MOUSE_MIDDLEBUTTON, curMouseButtonStates, 1, curSystem);
            break;

        case WM_RBUTTONDOWN:
            //0, down);
            SetCapture((HWND)hwnd);
            ++bIsCapturing;

            curMouseButtonStates |= STATE_RBUTTONDOWN;
            if(curMouseHandler)
                curMouseHandler->MouseHandler(MOUSE_RIGHTBUTTON, curMouseButtonStates, 1);
            else
                ControlWindow::WindowMouseHandler(MOUSE_RIGHTBUTTON, curMouseButtonStates, 1, curSystem);
            break;

        case WM_LBUTTONUP:
            //0, ip);
            if(bIsCapturing)
            {
                --bIsCapturing;
                if(!bIsCapturing)
                    ReleaseCapture();
            }

            curMouseButtonStates &= ~STATE_LBUTTONDOWN;
            if(curMouseHandler)
                curMouseHandler->MouseHandler(MOUSE_LEFTBUTTON, curMouseButtonStates, 0);
            else
                ControlWindow::WindowMouseHandler(MOUSE_LEFTBUTTON, curMouseButtonStates, 0, curSystem);
            break;

        case WM_MBUTTONUP:
            //0, ip);
            if(bIsCapturing)
            {
                --bIsCapturing;
                if(!bIsCapturing)
                    ReleaseCapture();
            }

            curMouseButtonStates &= ~STATE_MBUTTONDOWN;
            if(curMouseHandler)
                curMouseHandler->MouseHandler(MOUSE_MIDDLEBUTTON, curMouseButtonStates, 0);
            else
                ControlWindow::WindowMouseHandler(MOUSE_MIDDLEBUTTON, curMouseButtonStates, 0, curSystem);
            break;

        case WM_RBUTTONUP:
            //0, ip);
            if(bIsCapturing)
            {
                --bIsCapturing;
                if(!bIsCapturing)
                    ReleaseCapture();
            }

            curMouseButtonStates &= ~STATE_RBUTTONDOWN;
            if(curMouseHandler)
                curMouseHandler->MouseHandler(MOUSE_RIGHTBUTTON, curMouseButtonStates, 0);
            else
                ControlWindow::WindowMouseHandler(MOUSE_RIGHTBUTTON, curMouseButtonStates, 0, curSystem);
            break;

        default:
            return;
    }

    if(!realtimeCounter)
        engine->Update(FALSE);

    traceOut;
}


#endif

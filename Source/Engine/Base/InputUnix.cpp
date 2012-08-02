/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  InputUnix.h:  Input Devices

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

#include <X11/Xlib.h>


DefineClass(StandardInput);


unsigned char ENGINEAPI GetUnixInputCode(unsigned int keycode);
void ENGINEAPI InputProc(XEvent *event);


BOOL            bUsingStandardInput=0;


unsigned char UKCtoKB[0x65] = {

0,//VK_ESCAPE     ,
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
'-'           ,
'='           ,
0,//VK_BACK       ,
9,//VK_TAB        ,
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
'['           ,
']'           ,
12,//VK_RETURN     ,
0,//VK_LCONTROL   ,
'A'           ,
'S'           ,


'D'           ,
'F'           ,
'G'           ,
'H'           ,
'J'           ,
'K'           ,
'L'           ,
';'           ,
'\''          ,
0,
0,//VK_LSHIFT     ,
'/'          ,
'Z'           ,
'X'           ,
'C'           ,
'V'           ,
'B'           ,
'N'           ,
'M'           ,
','           ,
'.'           ,
'/'           ,
0,//VK_RSHIFT     ,
'*',//VK_MULTIPLY   ,
0,//VK_LMENU      ,
' ',//VK_SPACE      ,
0,//VK_CAPITAL    ,
0,//VK_F1         ,
0,//VK_F2         ,
0,//VK_F3         ,
0,//VK_F4         ,
0,//VK_F5         ,
0,//VK_F6         ,
0,//VK_F7         ,
0,//VK_F8         ,
0,//VK_F9         ,
0,//VK_F10        ,
0,//VK_NUMLOCK    ,
0,//VK_SCROLL     ,
0,//VK_NUMPAD7    ,
0,//VK_NUMPAD8    ,
0,//VK_NUMPAD9    ,
0,//VK_SUBTRACT   ,
0,//VK_NUMPAD4    ,
0,//VK_NUMPAD5    ,
0,//VK_NUMPAD6    ,
0,//VK_ADD        ,
0,//VK_NUMPAD1    ,
0,//VK_NUMPAD2    ,
0,//VK_NUMPAD3    ,
0,//VK_NUMPAD0    ,
0,//VK_DECIMAL    ,
0,//VK_F11        ,
0,//VK_F12        ,
0,
0,//VK_RCONTROL   ,
0,//VK_DIVIDE     ,
0,//VK_SNAPSHOT   ,
0,//VK_RMENU      ,
0,//VK_PAUSE      ,
0,//VK_HOME       ,
0,//VK_UP         ,
0,//VK_PRIOR      ,
0,//VK_LEFT       ,
0,//VK_RIGHT      ,
0,//VK_END        ,
0,//VK_DOWN       ,
0,//VK_NEXT       ,
0,//VK_INSERT     ,
0//VK_DELETE
};



/*=========================================================
    StandardInput
===========================================================*/

StandardInput::StandardInput()
{
    bUsingStandardInput = 1;
}

BOOL StandardInput::GetButtonState(unsigned int key)
{
    //return HIBYTE(GetKeyState(UKCtoKB[key]));
    return 0;
}


/*=========================================================
    Other
===========================================================*/

unsigned char ENGINEAPI GetUnixInputCode(unsigned int keycode)
{
    for(unsigned char i=0; i<0x65; i++)
        if(UKCtoKB[i] == keycode) return i;

    return 0xFF;
}


void ENGINEAPI InputProc(XEvent *event)
{
    if(bUsingStandardInput)
    {
        struct {int x; int y;} point;

        if(curKBHandler.Num())
        {
            switch(event->type)
            {
                case KeyPress:
                    curKBHandler[0](GetUnixInputCode(event->xkey.keycode), 1);
                    break;
                case KeyRelease:
                    curKBHandler[0](GetUnixInputCode(event->xkey.keycode), 0);
                    break;
            }
        }

        if(curMouseHandler.Num())
        {
            switch(event->type)
            {
                case MotionNotify:
                    //x, y);
                    point.x = event->xmotion.x_root;
                    point.y = event->xmotion.y_root;
                    //ClientToScreen(hwnd, &point);
                    curMouseHandler[0](MOUSE_MOVE, curMouseButtonStates, MAKEDWORD(point.x, point.y));
                    break;
                /*case WM_MOUSEWHEEL:
                    //0, dist);
                    curMouseHandler[0](MOUSE_WHEEL, curMouseButtonStates, (int)(short)HIWORD(wParam));
                    break;*/
                case ButtonPress:
                    switch(event->xbutton.button)
                    {
                        case Button1:
                            //0, down);
                            curMouseButtonStates |= STATE_LBUTTONDOWN;
                            curMouseHandler[0](MOUSE_LEFTBUTTON, curMouseButtonStates, 1);
                            break;
                        case Button3:
                            //0, down);
                            curMouseButtonStates |= STATE_MBUTTONDOWN;
                            curMouseHandler[0](MOUSE_MIDDLEBUTTON, curMouseButtonStates, 1);
                            break;
                        case Button2:
                            //0, down);
                            curMouseButtonStates |= STATE_RBUTTONDOWN;
                            curMouseHandler[0](MOUSE_RIGHTBUTTON, curMouseButtonStates, 1);
                            break;
                    }
                    break;
                case ButtonRelease:
                    switch(event->type)
                    {
                        case Button1:
                            //0, up);
                            curMouseButtonStates &= ~STATE_LBUTTONDOWN;
                            curMouseHandler[0](MOUSE_LEFTBUTTON, curMouseButtonStates, 0);
                            break;
                        case Button3:
                            //0, up);
                            curMouseButtonStates &= ~STATE_MBUTTONDOWN;
                            curMouseHandler[0](MOUSE_MIDDLEBUTTON, curMouseButtonStates, 0);
                            break;
                        case Button2:
                            //0, up);
                            curMouseButtonStates &= ~STATE_RBUTTONDOWN;
                            curMouseHandler[0](MOUSE_RIGHTBUTTON, curMouseButtonStates, 0);
                            break;
                    }
                    break;
            }
        }
    }
}


#endif

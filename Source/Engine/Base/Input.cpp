/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Input.h:  Input

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


DefineClass(MouseInputHandler);
DefineClass(KeyboardInputHandler);
DefineClass(Input);


CTSTR CodeNames[0x68] = {
TEXT("Esc")         ,
TEXT("1")           ,
TEXT("2")           ,
TEXT("3")           ,
TEXT("4")           ,
TEXT("5")           ,
TEXT("6")           ,
TEXT("7")           ,
TEXT("8")           ,
TEXT("9")           ,
TEXT("0")           ,
TEXT("-")           ,
TEXT("=")           ,
TEXT("Back")        ,
TEXT("Tab")         ,
TEXT("Q")           ,
TEXT("W")           ,
TEXT("E")           ,
TEXT("R")           ,
TEXT("T")           ,
TEXT("Y")           ,
TEXT("U")           ,
TEXT("I")           ,
TEXT("O")           ,
TEXT("P")           ,
TEXT("[")           ,
TEXT("]")           ,
TEXT("Enter")       ,
TEXT("Left Ctrl")   ,
TEXT("A")           ,
TEXT("S")           ,
TEXT("D")           ,
TEXT("F")           ,
TEXT("G")           ,
TEXT("H")           ,
TEXT("J")           ,
TEXT("K")           ,
TEXT("L")           ,
TEXT(";")           ,
TEXT("\"")          ,
TEXT("Tilde")       ,
TEXT("Left Shift")  ,
TEXT("/")          ,
TEXT("Z")           ,
TEXT("X")           ,
TEXT("C")           ,
TEXT("V")           ,
TEXT("B")           ,
TEXT("N")           ,
TEXT("M")           ,
TEXT(",")           ,
TEXT(".")           ,
TEXT("/")           ,
TEXT("Right Shift") ,
TEXT("*")           ,
TEXT("Left Alt")    ,
TEXT("Space")       ,
TEXT("CapsLock")    ,
TEXT("F1")          ,
TEXT("F2")          ,
TEXT("F3")          ,
TEXT("F4")          ,
TEXT("F5")          ,
TEXT("F6")          ,
TEXT("F7")          ,
TEXT("F8")          ,
TEXT("F9")          ,
TEXT("F10")         ,
TEXT("NumLock")     ,
TEXT("Scroll")      ,
TEXT("NP 7")        ,
TEXT("NP 8")        ,
TEXT("NP 9")        ,
TEXT("NP -")        ,
TEXT("NP 4")        ,
TEXT("NP 5")        ,
TEXT("NP 6")        ,
TEXT("+")           ,
TEXT("NP 1")        ,
TEXT("NP 2")        ,
TEXT("NP 3")        ,
TEXT("NP 0")        ,
TEXT("NP .")        ,
TEXT("F11")         ,
TEXT("F12")         ,
TEXT("NP Enter")    ,
TEXT("Ctrl")        ,
TEXT("/")           ,
TEXT("SysReq")      ,
TEXT("Right Alt")   ,
TEXT("Pause")       ,
TEXT("Home")        ,
TEXT("Up")          ,
TEXT("PgUp")        ,
TEXT("Left")        ,
TEXT("Right")       ,
TEXT("End")         ,
TEXT("Down")        ,
TEXT("PgDown")      ,
TEXT("Insert")      ,
TEXT("Delete")      ,
TEXT("Mouse Left")  ,
TEXT("Mouse Middle"),
TEXT("Mouse Right")
};


DWORD curMouseButtonStates = 0;


/*=========================================================
    handlers
===========================================================*/

MouseInputHandler::~MouseInputHandler()
{
    inputCaller->curMouseHandler.RemoveItem(this);
}

void MouseInputHandler::MouseHandler(int action, DWORD buttonStates, int value)
{
    scriptMouseHandler(action, buttonStates, value);

    if(action == MOUSE_LEFTBUTTON)
    {
        if(value)
            this->MouseDown(MOUSE_LEFTBUTTON);
        else
            this->MouseUp(MOUSE_LEFTBUTTON);
    }
    else if(action == MOUSE_MIDDLEBUTTON)
    {
        if(value)
            this->MouseDown(MOUSE_MIDDLEBUTTON);
        else
            this->MouseUp(MOUSE_MIDDLEBUTTON);
    }
    else if(action == MOUSE_RIGHTBUTTON)
    {
        if(value)
            this->MouseDown(MOUSE_RIGHTBUTTON);
        else
            this->MouseUp(MOUSE_RIGHTBUTTON);
    }
    else if(action == MOUSE_WHEEL)
        this->MouseWheel(value);
    else if(action == MOUSE_MOVE)
    {
        int x, y;
        GS->GetLocalMousePos(x, y);
        short x_offset = LOWORD(value);
        short y_offset = HIWORD(value);

        this->MouseMove(x, y, x_offset, y_offset);
    }
}

KeyboardInputHandler::~KeyboardInputHandler()
{
    inputCaller->curKBHandler.RemoveItem(this);
}

void KeyboardInputHandler::KeyboardHandler(unsigned int kbc, BOOL bDown)
{
    scriptKeyboardHandler(kbc, bDown);

    if(bDown)
        this->KeyDown(kbc);
    else
        this->KeyUp(kbc);
}



/*=========================================================
    Input
===========================================================*/

CTSTR ENGINEAPI GetInputCodeName(unsigned int code)
{
    if(code > 0x67)
        return TEXT("??");
    return CodeNames[code];
}

void Input::PushKBHandler(KeyboardInputHandler *kbHandler, BOOL bCharInput)
{
    kbHandler->inputCaller = this;
    kbHandler->bCharInput = bCharInput;
    curKBHandler.Insert(0, kbHandler);
}

void Input::PushMouseHandler(MouseInputHandler *mouseHandler)
{
    mouseHandler->inputCaller = this;
    curMouseHandler.Insert(0, mouseHandler);
}

void Input::ProcessInput()
{
}

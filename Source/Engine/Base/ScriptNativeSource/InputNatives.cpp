/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Input

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


#include "..\Base.h"


//<Script module="Base" filedefs="Input.xscript">
void Input::native_PushKBHandler(CallStruct &cs)
{
    KeyboardInputHandler* kbHandler = (KeyboardInputHandler*)cs.GetObject(0);
    BOOL bCharInput = (BOOL)cs.GetInt(1);

    PushKBHandler(kbHandler, bCharInput);
}

void Input::native_PushMouseHandler(CallStruct &cs)
{
    MouseInputHandler* mouseHandler = (MouseInputHandler*)cs.GetObject(0);

    PushMouseHandler(mouseHandler);
}

void Input::native_GetCurKBHandler(CallStruct &cs)
{
    KeyboardInputHandler*& returnVal = (KeyboardInputHandler*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetCurKBHandler();
}

void Input::native_GetCurMouseHandler(CallStruct &cs)
{
    MouseInputHandler*& returnVal = (MouseInputHandler*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetCurMouseHandler();
}

void Input::native_EmulateMouseInput(CallStruct &cs)
{
    int action = cs.GetInt(0);
    int buttonStates = cs.GetInt(1);
    int param = cs.GetInt(2);

    EmulateMouseInput(action, buttonStates, param);
}

void Input::native_EmulateKBInput(CallStruct &cs)
{
    int kbc = cs.GetInt(0);
    BOOL keydown = (BOOL)cs.GetInt(1);

    EmulateKBInput(kbc, keydown);
}

void Input::native_GetButtonState(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    int key = cs.GetInt(0);

    returnVal = GetButtonState(key);
}

void ENGINEAPI NativeGlobal_GetInputCodeName(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    int code = cs.GetInt(0);

    returnVal = GetInputCodeName(code);
}

void ENGINEAPI NativeGlobal_PushKBHandler(CallStruct &cs)
{
    KeyboardInputHandler* kbHandler = (KeyboardInputHandler*)cs.GetObject(0);
    BOOL bCharInput = (BOOL)cs.GetInt(1);

    PushKBHandler(kbHandler, bCharInput);
}

void ENGINEAPI NativeGlobal_PushMouseHandler(CallStruct &cs)
{
    MouseInputHandler* mouseHandler = (MouseInputHandler*)cs.GetObject(0);

    PushMouseHandler(mouseHandler);
}
//</Script>

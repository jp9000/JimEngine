/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Window/ControlWindow

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


//<Script module="Base" filedefs="Window.xscript">
void Window::native_SetPos(CallStruct &cs)
{
    const Vect2 &pos = (const Vect2&)cs.GetStruct(0);

    SetPos(pos);
}

void Window::native_SetSize(CallStruct &cs)
{
    const Vect2 &size = (const Vect2&)cs.GetStruct(0);

    SetSize(size);
}

void Window::native_GetRealPos(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);

    returnVal = GetRealPos();
}

void Window::native_GetLocalPos(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);

    returnVal = GetLocalPos();
}

void Window::native_GetSize(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);

    returnVal = GetSize();
}

void Window::native_SetOffsetType(CallStruct &cs)
{
    OffsetType newOffset = (OffsetType)cs.GetInt(0);

    SetOffsetType(newOffset);
}

void Window::native_GetOffsetType(CallStruct &cs)
{
    OffsetType& returnVal = (OffsetType&)cs.GetIntOut(RETURNVAL);

    returnVal = GetOffsetType();
}

void Window::native_SetPosOffset(CallStruct &cs)
{
    const Vect2 &offsetPos = (const Vect2&)cs.GetStruct(0);
    BOOL bCentered = (BOOL)cs.GetInt(1);

    SetPosOffset(offsetPos, bCentered);
}

void Window::native_GetOffsetPoint(CallStruct &cs)
{
    Vect2& returnVal = (Vect2&)cs.GetStructOut(RETURNVAL);
    OffsetType offset = (OffsetType)cs.GetInt(0);

    returnVal = GetOffsetPoint(offset);
}

void Window::native_SetFullScreen(CallStruct &cs)
{
    BOOL bSet = (BOOL)cs.GetInt(0);

    SetFullScreen(bSet);
}

void Window::native_Attach(CallStruct &cs)
{
    Window* new_parent = (Window*)cs.GetObject(0);

    Attach(new_parent);
}

void Window::native_Detach(CallStruct &cs)
{
    Detach();
}

void Window::native_GetParent(CallStruct &cs)
{
    Window*& returnVal = (Window*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetParent();
}

void Window::native_NumChildren(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = NumChildren();
}

void Window::native_GetChild(CallStruct &cs)
{
    Window*& returnVal = (Window*&)cs.GetObjectOut(RETURNVAL);
    int i = cs.GetInt(0);

    returnVal = GetChild(i);
}

void Window::native_SetTopLevel(CallStruct &cs)
{
    SetTopLevel();
}

void Window::native_SetSystem(CallStruct &cs)
{
    GraphicsSystem* graphicsSystem = (GraphicsSystem*)cs.GetObject(0);

    SetSystem(graphicsSystem);
}

void Window::native_GetSystem(CallStruct &cs)
{
    GraphicsSystem*& returnVal = (GraphicsSystem*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetSystem();
}

void ControlWindow::native_TakeInputControl(CallStruct &cs)
{
    BOOL bTake = (BOOL)cs.GetInt(0);

    TakeInputControl(bTake);
}

void ControlWindow::native_HasFocus(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = HasFocus();
}

void ENGINEAPI NativeGlobal_PushCursorPos(CallStruct &cs)
{
    PushCursorPos();
}

void ENGINEAPI NativeGlobal_PopCursorPos(CallStruct &cs)
{
    BOOL bSendMouseMove = (BOOL)cs.GetInt(0);

    PopCursorPos(bSendMouseMove);
}

void ENGINEAPI NativeGlobal_CurrentInputWindow(CallStruct &cs)
{
    ControlWindow*& returnVal = (ControlWindow*&)cs.GetObjectOut(RETURNVAL);

    returnVal = CurrentInputWindow();
}

void ENGINEAPI NativeGlobal_CurrentFocusWindow(CallStruct &cs)
{
    ControlWindow*& returnVal = (ControlWindow*&)cs.GetObjectOut(RETURNVAL);

    returnVal = CurrentFocusWindow();
}

void ENGINEAPI NativeGlobal_SetUnechoedCursorPos(CallStruct &cs)
{
    int x = cs.GetInt(0);
    int y = cs.GetInt(1);

    SetUnechoedCursorPos(x, y);
}
//</Script>

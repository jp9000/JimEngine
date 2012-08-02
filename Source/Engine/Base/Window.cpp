/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Window.h:  2D Objects

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


DefineClass(Window);
DefineClass(ControlWindow);

ControlWindow *ControlWindow::focusedWindow = NULL;
BOOL ControlWindow::bIgnoreMove;
List<CursorPosInfo> ControlWindow::CursorPosStack;
List<ControlWindow*> ControlWindow::InputControlStack;


Window::Window()
{
    traceIn(Window::Window);

    SetSystem(GS);
    id = -1;

    traceOut;
}

Window::~Window()
{
    traceIn(Window::~Window);

    if(Parent)
    {
        Parent->Children.RemoveItem(this);
        Parent = NULL;
    }
    else
    {
        if(GetSystem())
            GetSystem()->WindowList.RemoveItem(this);
    }

    traceOut;
}

void Window::Destroy()
{
    traceIn(Window::Destroy);

    Super::Destroy();

    int num = Children.Num();
    for(int i=0; i<num; i++)
        DestroyObject(Children[0]);

    traceOut;
}

void Window::PreFrame()
{
    Super::PreFrame();

    if(bFullScreenObj && GS)
    {
        zero(&Pos, sizeof(Pos));
        posOffset = Offset_TopLeft;
        Size = GS->GetSize();
    }
}


Vect2 Window::GetRealPos()
{
    Vect2 tl, br, center, outPos;

    if(Parent)
    {
        tl = Parent->GetRealPos();
        br = tl+Parent->GetSize();
        center = Lerp<Vect2>(tl, br, 0.5f);
    }
    else
    {
        zero(&tl, sizeof(tl));
        br = (GS) ? GS->GetSize() : Vect2(0.0f, 0.0f);
        center = br*0.5;
    }


    center.x = floorf(center.x);
    center.y = floorf(center.y);

    outPos = Pos;

    switch(posOffset)
    {
        case Offset_TopLeft:
            outPos += tl;
            break;
        case Offset_TopCenter:
            outPos += Vect2(center.x, tl.y);
            break;
        case Offset_TopRight:
            outPos += Vect2(br.x, tl.y);
            break;

        case Offset_CenterLeft:
            outPos += Vect2(tl.x, center.y);
            break;
        case Offset_Center:
            outPos += center;
            break;
        case Offset_CenterRight:
            outPos += Vect2(br.x, center.y);
            break;

        case Offset_BottomLeft:
            outPos += Vect2(tl.x, br.y);
            break;
        case Offset_BottomCenter:
            outPos += Vect2(center.x, br.y);
            break;
        case Offset_BottomRight:
            outPos += br;
            break;
    }

    return outPos;
}

void Window::SetPosOffset(const Vect2 &offsetPos, BOOL bCentered)
{
    Vect2 centerOffset = offsetPos;
    
    if(bCentered)
    {
        centerOffset -= (Size*0.5f);
        centerOffset.x = floorf(centerOffset.x);
        centerOffset.y = floorf(centerOffset.y);
    }

    switch(posOffset)
    {
        case Offset_TopLeft:
            Pos = offsetPos;
            break;
        case Offset_TopRight:
            Pos.Set(offsetPos.x-Size.x, offsetPos.y);
            break;
        case Offset_BottomLeft:
            Pos.Set(offsetPos.x, offsetPos.y-Size.y);
            break;
        case Offset_BottomRight:
            Pos = offsetPos-Size;
            break;

        case Offset_TopCenter:
            Pos.Set(centerOffset.x, offsetPos.y-Size.y);
            break;

        case Offset_CenterLeft:
            Pos.Set(offsetPos.x, centerOffset.y);
            break;
        case Offset_Center:
            Pos = centerOffset;
            break;
        case Offset_CenterRight:
            Pos.Set(offsetPos.x-Size.x, centerOffset.y);
            break;

        case Offset_BottomCenter:
            Pos.Set(centerOffset.x, offsetPos.x-Size.x);
            break;
    }
}



Vect2 Window::GetOffsetPoint(OffsetType offset)
{
    Vect2 tl, br, center, outPos;

    br = Size;
    center = br*0.5f;

    center.x = floorf(center.x);
    center.y = floorf(center.y);

    switch(offset)
    {
        case Offset_TopLeft:
            outPos = 0.0f;
            break;
        case Offset_TopCenter:
            outPos.Set(center.x, 0.0f);
            break;
        case Offset_TopRight:
            outPos.Set(br.x, 0.0f);
            break;

        case Offset_CenterLeft:
            outPos.Set(0.0f, center.y);
            break;
        case Offset_Center:
            outPos = center;
            break;
        case Offset_CenterRight:
            outPos.Set(br.x, center.y);
            break;

        case Offset_BottomLeft:
            outPos.Set(0.0f, br.y);
            break;
        case Offset_BottomCenter:
            outPos.Set(center.x, br.y);
            break;
        case Offset_BottomRight:
            outPos = br;
            break;
    }

    return outPos;
}

void Window::SetFullScreen(BOOL bSet)
{
    traceIn(Window::SetFullScreen);

    if(GetParent())
        return;

    bFullScreenObj = bSet;
    Size = GS->GetSize();
    Pos = 0.0f;
    posOffset = Offset_TopLeft;

    traceOut;
}


void Window::Attach(Window *new_parent)
{
    traceIn(Window::Attach);

    Detach();

    new_parent->Children << this;
    Parent = new_parent;

    if(curGraphicsSystem)
        curGraphicsSystem->WindowList.RemoveItem(this);

    traceOut;
}

void Window::Detach()
{
    traceIn(Window::Detach);

    if(Parent)
    {
        Parent->Children.RemoveItem(this);
        if(curGraphicsSystem)
            curGraphicsSystem->WindowList << this;
        Parent = NULL;
    }

    traceOut;
}

void Window::SetSystem(GraphicsSystem *graphicsSystem)
{
    traceIn(Window::SetSystem);

    if(graphicsSystem != curGraphicsSystem)
    {
        if(curGraphicsSystem && !Parent)
            curGraphicsSystem->WindowList.RemoveItem(this);

        if(Parent && (Parent->GetSystem() != graphicsSystem))
            Detach();

        curGraphicsSystem = graphicsSystem;

        if(graphicsSystem && !Parent)
            graphicsSystem->WindowList << this;
    }

    traceOut;
}

void Window::SetTopLevel()
{
    traceIn(Window::SetTopLevel);

    if(GetSystem()->WindowList[GetSystem()->WindowList.Num()-1] != this)
    {
        GetSystem()->WindowList.RemoveItem(this);
        GetSystem()->WindowList << this;
    }

    traceOut;
}



void ControlWindow::Destroy()
{
    traceIn(ControlWindow::Destroy);

    if(this == focusedWindow)
        focusedWindow = NULL;
    TakeInputControl(FALSE);

    Super::Destroy();

    traceOut;
}

void ControlWindow::PushCursorPos()
{
    traceIn(ControlWindow::PushCursorPos);

    CursorPosInfo csPos;

    OSGetCursorPos(csPos.x, csPos.y);
    CursorPosStack.Insert(0, csPos);

    traceOut;
}

void ControlWindow::PopCursorPos(BOOL bSendMouseMove)
{
    traceIn(ControlWindow::PopCursorPos);

    if(!CursorPosStack.Num())
        return;

    CursorPosInfo &csPos = CursorPosStack[0];
    if(!bSendMouseMove)
        bIgnoreMove = TRUE;
    OSSetCursorPos(csPos.x, csPos.y);

    CursorPosStack.Remove(0);

    traceOut;
}


void ControlWindow::SetUnechoedCursorPos(int x, int y)
{
    traceIn(ControlWindow::SetUnechoedCursorPos);

    bIgnoreMove = TRUE;
    GS->SetLocalMousePos(x, y);

    traceOut;
}

void ControlWindow::SetFocusWindow(ControlWindow *window)
{
    traceIn(ControlWindow::SetFocusWindow);

    if(!ControlWindow::focusedWindow)
    {
        ControlWindow::focusedWindow = window;
        window->GotFocus();
    }
    else if(ControlWindow::focusedWindow != window)
    {
        ControlWindow::focusedWindow->LostFocus();
        ControlWindow::focusedWindow = window;
        window->GotFocus();
    }

    traceOut;
}

void ControlWindow::TakeInputControl(BOOL bTake)
{
    if(bTake)
    {
        int id = InputControlStack.FindValueIndex(this);
        if(id != INVALID)
            InputControlStack.MoveToFront(id);
        else
            InputControlStack.Insert(0, this);
    }
    else
        InputControlStack.RemoveItem(this);
}


void ENGINEAPI ControlWindow::WindowMouseHandler(int action, PARAM param1, PARAM param2, GraphicsSystem *curGraphicsSystem)
{
    traceIn(ControlWindow::WindowMouseHandler);

    int x, y;

    switch(action)
    {
        case MOUSE_MOVE:
            {
                if(bIgnoreMove)
                {
                    bIgnoreMove = FALSE;
                    return;
                }

                curGraphicsSystem->GetLocalMousePos(x, y);
                short x_offset = LOWORD(param2);
                short y_offset = HIWORD(param2);

                if(ControlWindow::focusedWindow && ControlWindow::focusedWindow->KeepingFocus())
                {
                    ControlWindow::focusedWindow->MouseMove(x, y, x_offset, y_offset);
                    break;
                }

                ControlWindow *curWindow = curGraphicsSystem->GetMouseOver(x, y);

                if(!curWindow)
                {
                    if(ControlWindow::focusedWindow)
                    {
                        ControlWindow::focusedWindow->LostFocus();
                        ControlWindow::focusedWindow = NULL;
                    }
                }
                else
                {
                    if(!ControlWindow::focusedWindow)
                    {
                        ControlWindow::focusedWindow = curWindow;
                        curWindow->GotFocus();
                    }
                    else if(ControlWindow::focusedWindow != curWindow)
                    {
                        ControlWindow::focusedWindow->LostFocus();
                        ControlWindow::focusedWindow = curWindow;
                        curWindow->GotFocus();
                    }
                }

                if(ControlWindow::focusedWindow)
                    ControlWindow::focusedWindow->MouseMove(x, y, x_offset, y_offset);
            }
            break;

        case MOUSE_WHEEL:
            if(ControlWindow::focusedWindow)
                ControlWindow::focusedWindow->MouseWheel((short)param2);
            break;

        case MOUSE_LEFTBUTTON:
            if(ControlWindow::focusedWindow)
            {
                if(param2)
                    ControlWindow::focusedWindow->MouseDown(MOUSE_LEFTBUTTON);
                else
                    ControlWindow::focusedWindow->MouseUp(MOUSE_LEFTBUTTON);
            }
            break;

        case MOUSE_RIGHTBUTTON:
            if(ControlWindow::focusedWindow)
            {
                if(param2)
                    ControlWindow::focusedWindow->MouseDown(MOUSE_RIGHTBUTTON);
                else
                    ControlWindow::focusedWindow->MouseUp(MOUSE_RIGHTBUTTON);
            }
            break;

        case MOUSE_MIDDLEBUTTON:
            if(ControlWindow::focusedWindow)
            {
                if(param2)
                    ControlWindow::focusedWindow->MouseDown(MOUSE_MIDDLEBUTTON);
                else
                    ControlWindow::focusedWindow->MouseUp(MOUSE_MIDDLEBUTTON);
            }
    }

    traceOut;
}

void ENGINEAPI ControlWindow::WindowKeyboardHandler(int kbc, BOOL bKeyDown, GraphicsSystem *curGraphicsSystem)
{
    traceIn(ControlWindow::WindowKeyboardHandler);

    ControlWindow *inputWindow = CurrentInputWindow();
    ControlWindow *focusWindow = CurrentFocusWindow();
    if(!inputWindow) return;

    if(bKeyDown)
    {
        if(inputWindow->KeyDown(kbc) && focusWindow && inputWindow != focusWindow)
            focusWindow->KeyDown(kbc);
    }
    else
    {
        if(inputWindow->KeyUp(kbc) && focusWindow && inputWindow != focusWindow)
            focusWindow->KeyUp(kbc);
    }

    traceOut;
}

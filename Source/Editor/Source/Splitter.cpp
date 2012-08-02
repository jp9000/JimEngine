/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Splitter.h

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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#include "Xed.h"


DefineClass(Splitter);



void Splitter::Init()
{
    traceIn(Splitter::Init);

    SetSystem(GS);

    traceOut;
}

void Splitter::Destroy()
{
    traceIn(Splitter::Destroy);

    delete buffer1;
    delete buffer2;

    traceOut;
}


void Splitter::ResetBuffers()
{
    traceIn(Splitter::ResetBuffers);

    delete buffer1;
    delete buffer2;

    RenderStartNew();
        Color(Color_LightGray);
        if(splitterType == SPLITTER_HORIZONTAL)
        {
            Vertex(0.0f,              1.0f);
            Vertex(0.0f,              Size.y-1.0f);
            Vertex(Size.x,            1.0f);
            Vertex(Size.x,            Size.y-1.0f);
        }
        else
        {
            Vertex(1.0f,               1.0f);
            Vertex(1.0f,               Size.y+1.0f);
            Vertex(Size.x-1.0f,        1.0f);
            Vertex(Size.x-1.0f,        Size.y+1.0f);
        }
    buffer1 = RenderSave();
    //RenderStop(GS_TRIANGLESTRIP);

    RenderStartNew();
        Color(Color_LightGray);
        if(splitterType == SPLITTER_HORIZONTAL)
        {
            Vertex(1.0f,      0.0f);
            Vertex(Size.x,    0.0f);
            Color(Color_DarkGray);
            Vertex(1.0f,      Size.y-1.0f);
            Vertex(Size.x,    Size.y-1.0f);
        }
        else
        {
            Vertex(0.0f,           1.0f);
            Vertex(0.0f,           Size.y-1.0f);
            Color(Color_DarkGray);
            Vertex(Size.x-1.0f,    1.0f);
            Vertex(Size.x-1.0f,    Size.y-1.0f);
        }

        Color(Color_VeryLightGray);
        if(splitterType == SPLITTER_HORIZONTAL)
        {
            Vertex(1.0f,          1.0f);
            Vertex(Size.x,        1.0f);
            Color(Color_Gray);
            Vertex(1.0f,          Size.y-2.0f);
            Vertex(Size.x,        Size.y-2.0f);
        }
        else
        {
            Vertex(1.0f,          2.0f);
            Vertex(1.0f,          Size.y);
            Color(Color_Gray);
            Vertex(Size.x-2.0f,   2.0f);
            Vertex(Size.x-2.0f,   Size.y);
        }
    buffer2 = RenderSave();
    //RenderStop(GS_LINES);

    traceOut;
}


void Splitter::MouseMove(int x, int y, short x_offset, short y_offset)
{
    traceIn(Splitter::MouseMove);

    if(bKeepFocus) //if selected
    {
        switch(splitterType)
        {
            case SPLITTER_HORIZONTAL:
                splitterPos.y = y;
                break;
            case SPLITTER_VERTICAL:
                splitterPos.x = x;
                break;
            case SPLITTER_CENTER:
                splitterPos.Set(x, y);
        }

        if(SplitterHandler)
            SplitterHandler(splitterPos, param1, param2);
    }

    ResetCursor();

    traceOut;
}

void Splitter::MouseDown(DWORD button)
{
    traceIn(Splitter::MouseDown);

    if(button == MOUSE_LEFTBUTTON)
        bKeepFocus = TRUE;

    ResetCursor();

    traceOut;
}

void Splitter::MouseUp(DWORD button)
{
    traceIn(Splitter::MouseUp);

    if(button == MOUSE_LEFTBUTTON)
        bKeepFocus = FALSE;

    ResetCursor();

    traceOut;
}


void Splitter::Render()
{
    traceInFast(Splitter::Render);

    if(splitterType == SPLITTER_HORIZONTAL)
        Size.y = 5.0f;
    else if(splitterType == SPLITTER_VERTICAL)
        Size.x = 5.0f;

    if(splitterType != SPLITTER_CENTER)
    {
        EnableDepthTest(FALSE);

        LoadIndexBuffer(NULL);

        LoadVertexBuffer(buffer1);
        Draw(GS_TRIANGLESTRIP);

        LoadVertexBuffer(buffer2);
        Draw(GS_LINES);

        EnableDepthTest(TRUE);
    }

    traceOutFast;
}


void Splitter::ResetCursor()
{
    traceIn(Splitter::ResetCursor);

    switch(splitterType)
    {
        case SPLITTER_HORIZONTAL:
            SetCursor(LoadCursor(NULL, IDC_SIZENS));
            break;
        case SPLITTER_VERTICAL:
            SetCursor(LoadCursor(NULL, IDC_SIZEWE));
            break;
        case SPLITTER_CENTER:
            SetCursor(LoadCursor(NULL, IDC_SIZEALL));
    }

    traceOut;
}


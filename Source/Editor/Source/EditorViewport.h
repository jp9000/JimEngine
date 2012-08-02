/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditorViewport.h

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


#pragma once


/*==============================================================
  Editor Viewport
===============================================================*/

//--------------------------------------------------
// View enum
enum ViewportType
{
    ViewportType_Main=1,
    ViewportType_Left,
    ViewportType_Right,
    ViewportType_Top,
    ViewportType_Front,
    ViewportType_Back,
    ViewportType_Bottom
};

//--------------------------------------------------
// View enum
enum ViewportDrawType
{
    ViewportDrawType_Wireframe=1,
    ViewportDrawType_Brushes,
    ViewportDrawType_FullRender,
    ViewportDrawType_TexturedOnly
};

//--------------------------------------------------
// EditorViewport
class EditorViewport : public Viewport
{
    DeclareClass(EditorViewport, Viewport);

    bool bInsideName;
    String typeName;

    ViewportType type;

public:
    void Init();
    void Destroy();

    void MouseMove(int x, int y, short x_offset, short y_offset);
    void MouseDown(DWORD button);
    void MouseUp(DWORD button);

    void MouseWheel(short scroll);

    void GotFocus();
    void LostFocus();

    void PreFrame();

    void Render();

    void Tick(float fSeconds);

    void SetViewportType(ViewportType newType);
    ViewportType GetViewportType() {return type;}

    void ProcessMovement(short x_offset, short y_offset);

    Vect mouseWorldPos;

    void SerializeSettings(Serializer &s);

    void ProcessMultiSelection();

    float zoom;

    ViewportDrawType drawType;

    bool bWasDragging;

    bool bRightMouseDown;
    bool bLeftMouseDown;
    bool bMiddleMouseDown;

    bool bMovingForward,bMovingBackward,bMovingLeft,bMovingRight;

    BOOL bShowGrid;

    bool bSelecting;
    int selStartX, selStartY, selEndX, selEndY;

    int lastMouseX, lastMouseY;

    float rotX, rotY;

    bool bIgnoreMove;

    bool bMovedMouse;

    Vect mouseOrig, mouseDir;
};



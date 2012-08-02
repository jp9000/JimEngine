/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ShapeEditor.h

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



enum ShapeMode
{
    ShapeMode_Default,
    ShapeMode_DragVerts,
    ShapeMode_DragTangent,
    ShapeMode_Zoom,
    ShapeMode_Move,
};



class ShapeWindow : public ControlWindow
{
    DeclareClass(ShapeWindow, ControlWindow);

    VertexBuffer *vbGridLineH, *vbGridLineV, *snapThingy, *vbUnselectedVert, *vbSelectedVert;
    Shader *solidShader, *solidPixelShader;

    float zoomAdjustX, zoomAdjustY;
    float gridSpacing;

    BOOL  bSnap, bCreating;

    Vect2 realMousePos, mousePosition;

    DWORD mouseButtons;

    Vect2 camPos;

    BOOL bMouseDown, bMouseMoved;

    BOOL bVertWasSelected;

    BOOL bClosedPopup;

    int lastMouseX, lastMouseY;

    int splinePos;
    BOOL TraverseSpline(Vect2 &v);

    Vect2& SnapPoint(Vect2 &point) const;

    void  ResetSizeData();

    void  DeselectAll();
    DWORD SelectedVert();
    BOOL  SelectedTangent(int &vert, int &tangent);
    DWORD SelectedLine(Vect2 &lineIntersection);

    List<ShapeVert> Shape;

    ShapeMode curMode;

    BOOL bOriginallyLinked, bAdjustBothTangents, bSaveLink;
    int dragVert, dragTangent;
    int vertID, splineTime;

    BOOL  bSavedChanges;

    TCHAR lpCurFile[255];

    BOOL  LineIntersectsShape(Vect2 &v1, Vect2 &v2, List<int> &FaceRefs, List<Vect2> &ShapeData);

    void  GenerateShapeData(List<Vect2> &ShapeData);

public:
    ShapeWindow() {bRenderable = TRUE;}

    void  Init();
    void  Destroy();

    void  Render();

    void  MouseDown(DWORD button);
    void  MouseUp(DWORD button);
    void  MouseMove(int x, int y, short x_offset, short y_offset);

    void  DeleteSelectedVerts();

    void  SerializeData(Serializer &s);

    void  OpenFile();
    void  SaveFile(BOOL bSaveAs);
    void  CreateNewShape();

    void  SaveUndoData(CTSTR lpUndoName);

    void  Extrude();
    void  Spin();

    BOOL  ValidShape();

    int splineDetail;

    float zoom;
};



class ShapeEditor
{
public:
    ShapeEditor();
    ~ShapeEditor();

    static void RegisterWindowClasses();

    //------------------------------------

    void UpdateShapeView();

    GraphicsSystem *shapeView;
    ShapeWindow *shapeWindow;

    UndoRedoStack *undoStack, *redoStack;

    HWND hwndShapeEditor;
};


extern ShapeEditor *shapeEditor;


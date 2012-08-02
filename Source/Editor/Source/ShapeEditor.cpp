/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ShapeEditor.cpp

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


DefineClass(ShapeWindow);


LRESULT WINAPI ShapeViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void ENGINEAPI UpdateShapeEditMenu();

void ENGINEAPI UndoRedoShapeAction(Serializer &s, Serializer &sOut, BOOL bUndo);

BOOL CALLBACK CustomSplineDetailDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ExtrudeDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


ShapeEditor *shapeEditor = NULL;


struct ExtrudeInfo
{
    float amount;
    float smoothAngle;
    BOOL bTextureToSmoothing;
    BOOL bTextureAlwaysFaceUp;
};


void ShapeEditor::RegisterWindowClasses()
{
    WNDCLASS wc;
    zero(&wc, sizeof(wc));

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)ShapeViewProc;
    wc.hIcon = LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ICON1));
    wc.lpszMenuName = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_SHAPEMENU);
    wc.lpszClassName = TEXT("XR3DShapeWindow");
    wc.cbWndExtra = sizeof(LPVOID);

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the shape view class"));
}


ShapeEditor::ShapeEditor()
{
    shapeEditor = this;

    int borderXSize = 500;
    int borderYSize = 400;

    borderXSize += GetSystemMetrics(SM_CXSIZEFRAME)*2;

    borderYSize += GetSystemMetrics(SM_CYSIZEFRAME)*2;
    borderYSize += GetSystemMetrics(SM_CYCAPTION);
    borderYSize += GetSystemMetrics(SM_CYMENU);

    hwndShapeEditor = CreateWindow(TEXT("XR3DShapeWindow"), TEXT("Shape Editor - New Shape"),
                                   WS_VISIBLE|WS_THICKFRAME|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   borderXSize, borderYSize,
                                   hwndEditor, NULL, hinstMain, NULL);

    SetFocus(hwndShapeEditor);

    String strGraphicsSystem = AppConfig->GetString(TEXT("Engine"), TEXT("GraphicsSystem"));
    shapeView = (GraphicsSystem*)CreateFactoryObject(strGraphicsSystem, FALSE);

    if(!shapeView)
        CrashError(TEXT("Bad Graphics System: %s"), strGraphicsSystem);

    shapeView->InitializeDevice(hwndShapeEditor);
    shapeView->InitializeObject();
    shapeView->SetSize(500, 400);

    //-----------------------------------------

    shapeWindow = CreateObject(ShapeWindow);

    CreateUndoRedoStacks(UpdateShapeEditMenu, undoStack, redoStack);
}

ShapeEditor::~ShapeEditor()
{
    delete undoStack;
    delete redoStack;

    DestroyObject(shapeWindow);
    DestroyObject(shapeView);
    DestroyWindow(hwndShapeEditor);

    shapeEditor = NULL;
}

void ShapeEditor::UpdateShapeView()
{
    if(engine->bBlockViewUpdates)
        return;

    shapeView->PreRenderScene();
    shapeView->RenderScene(TRUE, 0xFF000000);
    shapeView->PostRenderScene();
}


void ShapeWindow::Init()
{
    Super::Init();

    SetSystem(shapeEditor->shapeView);

    GraphicsSystem* shapeView = shapeEditor->shapeView;

    bClosedPopup = TRUE;

    shapeView->RenderStartNew();
        shapeView->Vertex(-1.0f, 0.0f, 0.0f);
        shapeView->Vertex( 1.0f, 0.0f, 0.0f);
    vbGridLineV = shapeView->RenderSave();

    shapeView->RenderStartNew();
        shapeView->Vertex(0.0f, -1.0f, 0.0f);
        shapeView->Vertex(0.0f,  1.0f, 0.0f);
    vbGridLineH = shapeView->RenderSave();

    shapeView->RenderStartNew();
        shapeView->Vertex(0.0f, -1.5f, 0.0f);
        shapeView->Vertex(0.0f,  1.5f, 0.0f);

        shapeView->Vertex(-1.5f, 0.0f, 0.0f);
        shapeView->Vertex( 1.5f, 0.0f, 0.0f);

        shapeView->Vertex(-1.0f, 1.0f, 0.0f);
        shapeView->Vertex( 1.0f, 1.0f, 0.0f);

        shapeView->Vertex(-1.0f, -1.0f, 0.0f);
        shapeView->Vertex( 1.0f, -1.0f, 0.0f);

        shapeView->Vertex(-1.0f, -1.0f, 0.0f);
        shapeView->Vertex(-1.0f, 1.0f, 0.0f);

        shapeView->Vertex(1.0f, -1.0f, 0.0f);
        shapeView->Vertex(1.0f, 1.0f, 0.0f);
    snapThingy = shapeView->RenderSave();


    shapeView->RenderStartNew();
        shapeView->Vertex(-1.0f, -1.0f, 0.0f);
        shapeView->Vertex(-1.0f, 1.0f, 0.0f);
        shapeView->Vertex(1.0f, 1.0f, 0.0f);
        shapeView->Vertex(1.0f, -1.0f, 0.0f);
        shapeView->Vertex(-1.0f, -1.0f, 0.0f);
    vbUnselectedVert = shapeView->RenderSave();

    shapeView->RenderStartNew();
        shapeView->Vertex(-1.0f, 1.0f, 0.0f);
        shapeView->Vertex(-1.0f, -1.0f, 0.0f);
        shapeView->Vertex(1.0f, 1.0f, 0.0f);
        shapeView->Vertex(1.0f, -1.0f, 0.0f);
    vbSelectedVert = shapeView->RenderSave();


    solidShader = shapeView->CreateVertexShaderFromFile(TEXT("data/Base/shaders/SolidColor.vShader"));
    solidPixelShader = GetPixelShader(TEXT("data/Base/shaders/SolidColor.pShader"));

    gridSpacing = 0.5f;

    camPos.Set(0.0f, 0.0f);

    zoom = 30.0f;
    bSnap = TRUE;

    splineDetail = 5;

    bCreating = TRUE;

    bSavedChanges = TRUE;
    lpCurFile[0] = 0;

    if(levelInfo->SavedShape.Num())
    {
        Shape.CopyList(levelInfo->SavedShape);
        bCreating = FALSE;
    }
}

void ShapeWindow::Destroy()
{
    if(!bCreating)
        levelInfo->SavedShape.CopyList(Shape);
    else
        levelInfo->SavedShape.Clear();

    delete solidShader;
    delete solidPixelShader;

    delete vbSelectedVert;
    delete vbUnselectedVert;

    delete snapThingy;

    delete vbGridLineH;
    delete vbGridLineV;

    Super::Destroy();
}

void ShapeWindow::ResetSizeData()
{
    Size = shapeEditor->shapeView->GetSize();

    float aspectx = Size.x/Size.y;
    float aspecty = Size.y/Size.x;

    zoomAdjustX = zoom*(Size.x/shapeEditor->shapeView->GetSizeXF());
    zoomAdjustY = zoom*(Size.y/shapeEditor->shapeView->GetSizeXF());
}

void ShapeWindow::Render()
{
    GraphicsSystem* shapeView = shapeEditor->shapeView;

    ResetSizeData();

    //------------------------

    shapeView->Ortho(camPos.x-zoomAdjustX, camPos.x+zoomAdjustX, camPos.y-zoomAdjustY, camPos.y+zoomAdjustY, -2048.0f, 2048.0f);

    EnableBlending(TRUE);
    BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);

    shapeView->LoadVertexShader(solidShader);
    shapeView->LoadPixelShader(solidPixelShader);

    shapeView->LoadIndexBuffer(NULL);

    //------------------------

    float xPos = ceilf((camPos.x-zoomAdjustX)/gridSpacing)*gridSpacing;
    float yPos = ceilf((camPos.y-zoomAdjustY)/gridSpacing)*gridSpacing;

    shapeView->LoadVertexBuffer(vbGridLineH);

    float spacing = gridSpacing;

    while(((zoomAdjustX/spacing) > 50.0f) || ((zoomAdjustY/spacing) > 50.0f))
        spacing *= 2.0f;

    xPos = ceilf(xPos/spacing)*spacing;
    yPos = ceilf(yPos/spacing)*spacing;

    while(xPos < camPos.x+zoomAdjustX)
    {
        float realX = fabsf(xPos);

        Vect4 gridColor(0.75f, 1.0f, 0.75f, 0.5f);

        if(realX < EPSILON)
            solidShader->SetColor(solidShader->GetParameter(1), gridColor);
        else
        {
            float val = fmodf(realX, 10.0f);

            if((val < EPSILON) || ((10.0f-val) < EPSILON))
                gridColor *= 0.5f;
            else
                gridColor *= 0.25f;

            gridColor.w = 0.5f;
            solidShader->SetColor(solidShader->GetParameter(1), gridColor);
        }

        shapeView->MatrixPush();
        shapeView->MatrixIdentity();
        shapeView->MatrixTranslate(xPos, camPos.y, 0.0f);
        shapeView->MatrixScale(1.0f, zoomAdjustY, 1.0f);
        shapeView->Draw(GS_LINES);
        shapeView->MatrixPop();

        xPos += spacing;
    }

    shapeView->LoadVertexBuffer(vbGridLineV);

    while(yPos < camPos.y+zoomAdjustY)
    {
        float realY = fabsf(yPos);

        Vect4 gridColor(0.75f, 1.0f, 0.75f, 0.5f);

        if(realY < EPSILON)
            solidShader->SetColor(solidShader->GetParameter(1), gridColor);
        else
        {
            float val = fmodf(realY, 10.0f);

            if((val < EPSILON) || ((10.0f-val) < EPSILON))
                gridColor *= 0.5f;
            else
                gridColor *= 0.25f;

            gridColor.w = 0.5f;
            solidShader->SetColor(solidShader->GetParameter(1), gridColor);
        }

        shapeView->MatrixPush();
        shapeView->MatrixIdentity();
        shapeView->MatrixTranslate(camPos.x, yPos, 0.0f);
        shapeView->MatrixScale(zoomAdjustX, 1.0f, 1.0f);
        shapeView->Draw(GS_LINES);
        shapeView->MatrixPop();

        yPos += spacing;
    }

    //------------------------
    // draw snap guide

    if(bSnap && bCreating && (curMode != ShapeMode_Zoom) && (curMode != ShapeMode_Move))
    {
        Color4 color(1.0f, 1.0f, 1.0f, 1.0f);
        solidShader->SetColor(solidShader->GetParameter(1), color);

        shapeView->LoadVertexBuffer(snapThingy);
        shapeView->MatrixPush();
        shapeView->MatrixIdentity();
        shapeView->MatrixTranslate(mousePosition);
        shapeView->MatrixScale(gridSpacing, gridSpacing, 1.0f);
        shapeView->MatrixScale(0.5f, 0.5f, 1.0f);
        shapeView->Draw(GS_LINES);
        shapeView->MatrixPop();
    }

    //------------------------
    // draw lines

    Color4 color;

    color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    solidShader->SetColor(solidShader->GetParameter(1), color);

    shapeView->MatrixPush();
    shapeView->MatrixIdentity();

    if(Shape.Num() > 1)
    {
        shapeView->RenderStartNew();

        Vect2 v;
        splinePos = 0;
        int curVertID = 0;

        while(TraverseSpline(v))
        {
            shapeView->Vertex(v);

            if(vertID != curVertID)
            {
                curVertID = vertID;

                shapeView->RenderStop(GS_LINESTRIP);
                shapeView->RenderStartNew();

                shapeView->Vertex(v);
            }
        }
        shapeView->RenderStop(GS_LINESTRIP);
    }
    shapeView->MatrixPop();

    //------------------------
    // draw verts

    color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    solidShader->SetColor(solidShader->GetParameter(1), color);

    for(int i=0; i<Shape.Num(); i++)
    {
        ShapeVert &line = Shape[i];

        shapeView->MatrixPush();
        shapeView->MatrixIdentity();
        shapeView->MatrixTranslate(line.pos);
        shapeView->MatrixScale(zoom*0.006f, zoom*0.006f, 1.0f);
        if(!line.bSelected)
        {
            shapeView->LoadVertexBuffer(vbUnselectedVert);
            shapeView->Draw(GS_LINESTRIP);
        }
        else
        {
            shapeView->LoadVertexBuffer(vbSelectedVert);
            shapeView->Draw(GS_TRIANGLESTRIP);
        }
        shapeView->MatrixPop();
    }

    //------------------------
    // draw tangents

    for(int i=0; i<Shape.Num(); i++)
    {
        ShapeVert &line = Shape[i];

        shapeView->MatrixPush();
        shapeView->MatrixIdentity();

        if(line.bSelected)
        {
            if(!line.t[0].CloseTo(Vect2(0.0f, 0.0f)))
            {
                color.w = 0.5f;
                solidShader->SetColor(solidShader->GetParameter(1), color);

                shapeView->RenderStartNew();
                shapeView->Vertex(line.pos);
                shapeView->Vertex(line.pos-line.t[0]);
                shapeView->RenderStop(GS_LINES);

                color.w = 1.0f;
                solidShader->SetColor(solidShader->GetParameter(1), color);

                shapeView->MatrixPush();
                shapeView->MatrixTranslate(line.pos-line.t[0]);
                shapeView->MatrixScale(zoom*0.004f, zoom*0.004f, 1.0f);
                shapeView->LoadVertexBuffer(vbSelectedVert);
                shapeView->Draw(GS_TRIANGLESTRIP);
                shapeView->MatrixPop();
            }

            if(!line.t[1].CloseTo(Vect2(0.0f, 0.0f)))
            {
                color.w = 0.5f;
                solidShader->SetColor(solidShader->GetParameter(1), color);

                shapeView->RenderStartNew();
                shapeView->Vertex(line.pos);
                shapeView->Vertex(line.pos+line.t[1]);
                shapeView->RenderStop(GS_LINES);

                color.w = 1.0f;
                solidShader->SetColor(solidShader->GetParameter(1), color);

                shapeView->MatrixPush();
                shapeView->MatrixTranslate(line.pos+line.t[1]);
                shapeView->MatrixScale(zoom*0.004f, zoom*0.004f, 1.0f);
                shapeView->LoadVertexBuffer(vbSelectedVert);
                shapeView->Draw(GS_TRIANGLESTRIP);
                shapeView->MatrixPop();
            }
        }
        shapeView->MatrixPop();
    }

    //------------------------

    shapeView->LoadVertexShader(NULL);
    shapeView->LoadPixelShader(NULL);
}

void ShapeWindow::MouseDown(DWORD button)
{
    int v, t;
    Vect2 lineIntersection;

    switch(button)
    {
        case MOUSE_LEFTBUTTON:
            mouseButtons |= 1;
            break;

        case MOUSE_RIGHTBUTTON:
            mouseButtons |= 2;
            break;
    };

    if(mouseButtons == 1)
    {
        if(bCreating)
        {
            if(SelectedTangent(v, t))
            {
                dragVert = v;
                dragTangent = t;

                if(GetButtonState(KBC_ALT))
                {
                    if(Shape[v].bTangentsLinked)
                    {
                        bSaveLink = TRUE;
                        Shape[v].bTangentsLinked = FALSE;
                    }
                }

                bOriginallyLinked = Shape[v].bTangentsLinked;

                curMode = ShapeMode_DragTangent;
            }
            else if(GetButtonState(KBC_SHIFT))
            {
                curMode = ShapeMode_Move;

                if(!bKeepFocus)
                {
                    ShowCursor(FALSE);

                    GetSystem()->GetLocalMousePos(lastMouseX, lastMouseY);
                }
            }
            else
            {
                if(Shape.Num() >= 2)
                {
                    if(mousePosition.CloseTo(Shape[0].pos, 0.3f))
                    {
                        SaveUndoData(TEXT("Close Shape"));

                        if(Shape.Num())
                            Shape.Last().bSelected = FALSE;

                        bCreating = FALSE;
                        Shape[0].bSelected = TRUE;
                    }
                }

                if(bCreating) //if still creating
                {
                    SaveUndoData(TEXT("New Vertex"));
                    if(Shape.Num())
                        Shape.Last().bSelected = FALSE;

                    ShapeVert vert;
                    zero(&vert, sizeof(vert));
                    vert.pos = mousePosition;
                    vert.bSelected = TRUE;
                    vert.bTangentsLinked = TRUE;
                    Shape << vert;
                }

                shapeEditor->UpdateShapeView();
            }
        }
        else  //move tangent/vert if selected, otherwise zoom in/out
        {
            if((v = SelectedVert()) != INVALID)
            {
                ShapeVert &vert = Shape[v];

                dragVert = v;

                if(!GetButtonState(KBC_CONTROL))
                {
					int i;

                    for(i=0; i<Shape.Num(); i++)
                    {
                        if((i != v) && Shape[i].bSelected)
                            break;
                    }

                    if(!vert.bSelected || (i != Shape.Num()))
                        SaveUndoData(TEXT("Select Vertex"));

                    DeselectAll();

                    vert.bSelected = !vert.bSelected;

                    if(GetButtonState(KBC_ALT))
                    {
                        dragTangent = 0;
                        bAdjustBothTangents = TRUE;
                        bSaveLink = vert.bTangentsLinked;
                        curMode = ShapeMode_DragTangent;
                        vert.bTangentsLinked = TRUE;
                    }
                    else
                        curMode = ShapeMode_DragVerts;
                }
                else
                {
                    bVertWasSelected = vert.bSelected;
                    if(!vert.bSelected)
                    {
                        SaveUndoData(TEXT("Select Vertex"));
                        vert.bSelected = TRUE;
                    }

                    curMode = ShapeMode_DragVerts;
                }

                shapeEditor->UpdateShapeView();
            }
            else if(SelectedTangent(v, t))
            {
                bAdjustBothTangents = FALSE;

                dragVert = v;
                dragTangent = t;

                if(GetButtonState(KBC_ALT))
                {
                    if(Shape[v].bTangentsLinked)
                    {
                        bSaveLink = TRUE;
                        Shape[v].bTangentsLinked = FALSE;
                    }
                }

                bOriginallyLinked = Shape[v].bTangentsLinked;

                curMode = ShapeMode_DragTangent;
            }
            else
            {
                if(!GetButtonState(KBC_CONTROL))
                {
                    for(int i=0; i<Shape.Num(); i++)
                    {
                        if(Shape[i].bSelected)
                        {
                            SaveUndoData(TEXT("Deselect All"));
                            break;
                        }
                    }
                    DeselectAll();
                }

                curMode = ShapeMode_Move;

                if(!bKeepFocus)
                {
                    ShowCursor(FALSE);

                    GetSystem()->GetLocalMousePos(lastMouseX, lastMouseY);
                }
            }
        }

        if(!bKeepFocus)
        {
            bMouseDown = TRUE;
            bMouseMoved = FALSE;

            bKeepFocus = TRUE;
            SetCapture(shapeEditor->hwndShapeEditor);
        }
    }
    else if(mouseButtons == 3)
    {
        if(!bCreating || GetButtonState(KBC_SHIFT))
        {
            curMode = ShapeMode_Zoom;

            if(!bKeepFocus)
            {
                ShowCursor(FALSE);

                GetSystem()->GetLocalMousePos(lastMouseX, lastMouseY);

                bMouseDown = TRUE;
                bMouseMoved = FALSE;

                bKeepFocus = TRUE;
                SetCapture(shapeEditor->hwndShapeEditor);
            }
        }
    }
}

void ShapeWindow::MouseUp(DWORD button)
{
    int v;
    Vect2 lineIntersection;

    switch(button)
    {
        case MOUSE_LEFTBUTTON:
            mouseButtons &= ~1L;
            break;

        case MOUSE_RIGHTBUTTON:
            mouseButtons &= ~2L;
            break;
    };

    if(button == MOUSE_LEFTBUTTON)
    {
        if(!bCreating)
        {
            if(!bMouseMoved && GetButtonState(KBC_CONTROL))
            {
                int v;
                if(bVertWasSelected)
                {
                    if((v = SelectedVert()) != INVALID)
                    {
                        ShapeVert &vert = Shape[v];

                        if(vert.bSelected)
                            SaveUndoData(TEXT("Deselect Vertex"));
                        else
                            SaveUndoData(TEXT("Select Vertex"));

                        vert.bSelected = !vert.bSelected;

                        dragVert = v;

                        shapeEditor->UpdateShapeView();
                    }
                }
            }
        }

        if((curMode == ShapeMode_Zoom) || (curMode == ShapeMode_Move))
        {
            GetSystem()->SetLocalMousePos(lastMouseX, lastMouseY);
            ShowCursor(TRUE);
        }

        curMode = ShapeMode_Default;

        if(bKeepFocus)
        {
            bMouseDown = FALSE;

            bKeepFocus = FALSE;
            ReleaseCapture();
        }
    }
    else if(button == MOUSE_RIGHTBUTTON)
    {
        if(!bKeepFocus && bClosedPopup)
        {
            if((v = SelectedVert()) != INVALID)
            {
                HMENU hmenu = LoadMenu(hinstMain, MAKEINTRESOURCE(IDR_POPUPS));
                HMENU hmenuPopup = GetSubMenu(hmenu, 7);

                bClosedPopup = FALSE;

                POINT p;
                GetCursorPos(&p);
                if(TrackPopupMenuEx(hmenuPopup, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, shapeEditor->hwndShapeEditor, NULL) == ID_SHAPEVERTCLICK_DELETEVERTEX)
                {
                    if(Shape.Num() > 3)
                    {
                        SaveUndoData(TEXT("Delete Vertex"));
                        Shape.Remove(v);
                    }
                }

                GetCursorPos(&p);
                ScreenToClient(shapeEditor->hwndShapeEditor, &p);

                realMousePos.x = 2.0f*(((float(p.x)-Pos.x)/Size.x)-0.5f)*zoomAdjustX;
                realMousePos.y = -2.0f*(((float(p.y)-Pos.y)/Size.y)-0.5f)*zoomAdjustY;

                StandardInput *input = (StandardInput*)(shapeEditor->shapeView->GetInput());
                lastMouseX = input->last_x = p.x;
                lastMouseY = input->last_y = p.y;

                realMousePos += camPos;

                bClosedPopup = TRUE;
                DestroyMenu(hmenu);

                shapeEditor->UpdateShapeView();
            }
            else if((v = SelectedLine(lineIntersection)) != INVALID)
            {
                HMENU hmenu = LoadMenu(hinstMain, MAKEINTRESOURCE(IDR_POPUPS));
                HMENU hmenuPopup = GetSubMenu(hmenu, 6);

                bClosedPopup = FALSE;

                POINT p;
                GetCursorPos(&p);
                if(TrackPopupMenuEx(hmenuPopup, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, shapeEditor->hwndShapeEditor, NULL) == ID_SHAPELINECLICK_INSERTVERTEX)
                {
                    SaveUndoData(TEXT("Insert Vertex"));

                    DeselectAll();
                    //int vp1 = (v == (Shape.Num()-1)) ? 0 : v+1;
                    //
                    //ShapeVert &prev = Shape[v];
                    //ShapeVert &next = Shape[vp1];

                    ShapeVert vert;
                    vert.pos = lineIntersection;
                    vert.bTangentsLinked = TRUE;
                    vert.bSelected = TRUE;
                    vert.t[0] = vert.t[1].Set(0.0f, 0.0f);

                    Shape.Insert(v+1, vert);

                    shapeEditor->UpdateShapeView();
                }

                GetCursorPos(&p);
                ScreenToClient(shapeEditor->hwndShapeEditor, &p);

                realMousePos.x = 2.0f*(((float(p.x)-Pos.x)/Size.x)-0.5f)*zoomAdjustX;
                realMousePos.y = -2.0f*(((float(p.y)-Pos.y)/Size.y)-0.5f)*zoomAdjustY;

                StandardInput *input = (StandardInput*)(shapeEditor->shapeView->GetInput());
                lastMouseX = input->last_x = p.x;
                lastMouseY = input->last_y = p.y;

                realMousePos += camPos;

                bClosedPopup = TRUE;
                DestroyMenu(hmenu);
            }
        }
        else if(curMode == ShapeMode_Zoom)
        {
            curMode = ShapeMode_Move;
        }
    }
}

void ShapeWindow::MouseMove(int x, int y, short x_offset, short y_offset)
{
    mousePosition.x = 2.0f*(((float(x)-Pos.x)/Size.x)-0.5f)*zoomAdjustX;
    mousePosition.y = -2.0f*(((float(y)-Pos.y)/Size.y)-0.5f)*zoomAdjustY;

    mousePosition += camPos;

    realMousePos = mousePosition;

    SnapPoint(mousePosition);

    if(curMode == ShapeMode_Default)
    {
        if(bMouseDown && bCreating)
        {
            ShapeVert &line = Shape.Last();
            line.t[0] = (mousePosition-line.pos);
            line.t[1] = line.t[0];
        }
    }
    else if(curMode == ShapeMode_DragVerts)
    {
        if(!bMouseMoved)
            SaveUndoData(TEXT("Drag Vertices"));

        ShapeVert &line = Shape[dragVert];
        Vect2 posOffset = mousePosition-line.pos;

        for(int i=0; i<Shape.Num(); i++)
        {
            if(Shape[i].bSelected)
                Shape[i].pos += posOffset;
        }
    }
    else if(curMode == ShapeMode_DragTangent)
    {
        ShapeVert &line = Shape[dragVert];
        Vect2 tangent = (mousePosition-line.pos);

        if(!bMouseMoved)
        {
            if(bAdjustBothTangents)
                line.bTangentsLinked = bSaveLink;
            else if(bSaveLink)
                line.bTangentsLinked = TRUE;

            SaveUndoData(TEXT("Drag Tangents"));

            if(bAdjustBothTangents)
                line.bTangentsLinked = TRUE;
            else if(bSaveLink)
                line.bTangentsLinked = FALSE;

            bSaveLink = FALSE;
        }

        if(bOriginallyLinked && !bAdjustBothTangents)
            line.bTangentsLinked = !GetButtonState(KBC_ALT);

        if(dragTangent == 0)
        {
            line.t[0] = -tangent;

            if(line.bTangentsLinked)
            {
                if(bAdjustBothTangents)
                    line.t[1] = line.t[0];
                else if(line.t[0].Len() > EPSILON)
                {
                    float oppositeLen = line.t[1].Len();
                    line.t[1] = -tangent.Norm()*oppositeLen;
                }
            }
        }
        else
        {
            line.t[1] = tangent;

            if(line.bTangentsLinked)
            {
                if(bAdjustBothTangents)
                    line.t[0] = line.t[1];
                else if(line.t[1].Len() > EPSILON)
                {
                    float oppositeLen = line.t[0].Len();
                    line.t[0] = tangent.Norm()*oppositeLen;
                }
            }
        }
    }
    else if(curMode == ShapeMode_Zoom)
    {
        zoom += float(y_offset)*0.1f;

        if(zoom < 10.0f)
            zoom = 10.0f;
        else if(zoom > 400.0f)
            zoom = 400.0f;
    }
    else if(curMode == ShapeMode_Move)
    {
        Vect2 add(float(x_offset)*0.1f, -float(y_offset)*0.1f);

        if(mouseButtons == 2)
            add *= zoom/8.0f;
        else
            add *= zoom/16.0f;

        camPos += add;
    }

    bMouseMoved = TRUE;

    shapeEditor->UpdateShapeView();
}

Vect2& ShapeWindow::SnapPoint(Vect2 &v) const
{
    if(bSnap)
    {
        float spacingD2 = gridSpacing*0.5f;

        v.x += (v.x > 0.0f) ? spacingD2 : -spacingD2;
        v.y += (v.y > 0.0f) ? spacingD2 : -spacingD2;

        v.x -= fmodf(v.x, gridSpacing);
        v.y -= fmodf(v.y, gridSpacing);
    }

    return v;
}

void ShapeWindow::DeselectAll()
{
    for(int i=0; i<Shape.Num(); i++)
        Shape[i].bSelected = FALSE;
}

DWORD ShapeWindow::SelectedVert()
{
    for(int i=0; i<Shape.Num(); i++)
    {
        ShapeVert &vert = Shape[i];

        if(realMousePos.CloseTo(vert.pos, zoom*0.008f))
            return i;
    }

    return INVALID;
}

BOOL ShapeWindow::SelectedTangent(int &v, int &t)
{
    float epsilon = zoom*0.006f;
    if(bSnap)
        epsilon = zoom*0.02f;

    for(int i=0; i<Shape.Num(); i++)
    {
        ShapeVert &vert = Shape[i];

        if(vert.bSelected)
        {
            Vect2 tan1Pos = (vert.pos-vert.t[0]);
            if(realMousePos.CloseTo(tan1Pos, epsilon))
            {
                v = i;
                t = 0;
                return TRUE;
            }

            Vect2 tan2Pos = (vert.pos+vert.t[1]);
            if(realMousePos.CloseTo(tan2Pos, epsilon))
            {
                v = i;
                t = 1;
                return TRUE;
            }
        }
    }

    return FALSE;
}

DWORD ShapeWindow::SelectedLine(Vect2 &lineIntersection)
{
    Vect2 v1, v2;
    splinePos = 0;

    TraverseSpline(v1);

    while(TraverseSpline(v2))
    {
        Vect2 norm      = (v2-v1).Norm();
        Vect2 cross     = norm.GetCross();
        float crossDist = v1.Dot(cross);

        float distFromLine = cross.Dot(realMousePos)-crossDist;

        if(fabsf(distFromLine) < (zoom*0.006f))
        {
            Vect2 intersectPosition = realMousePos+(cross*distFromLine);

            float lineLen  = (v2-v1).Len();
            float normDist = norm.Dot(v1);
            float posLen   = norm.Dot(intersectPosition)-normDist;

            if((posLen <= lineLen) && (posLen > 0.0f))
            {
                lineIntersection = intersectPosition;

                return (splineTime == 0) ? (vertID-1) : vertID;
            }
        }

        mcpy(&v1, &v2, sizeof(Vect2));
    }

    return INVALID;
}

BOOL ShapeWindow::TraverseSpline(Vect2 &v)
{
    vertID     = splinePos/splineDetail;
    splineTime = splinePos%splineDetail;

    if(bCreating)
    {
        if(vertID == Shape.Num()-1)
        {
            if(splineTime == 0)
            {
                v = Shape.Last().pos;
                ++splinePos;
                return TRUE;
            }
            else
                return FALSE;
        }
        else if(vertID == Shape.Num())
            return FALSE;
    }
    else if(vertID == Shape.Num())
    {
        if(splineTime == 0)
        {
            v = Shape[0].pos;
            ++splinePos;
            return TRUE;
        }
        else
            return FALSE;
    }
    else if(vertID > Shape.Num())
        return FALSE;

    int vertIDp1 = (vertID == (Shape.Num()-1)) ? 0 : vertID+1;
    ShapeVert &vert1 = Shape[vertID];
    ShapeVert &vert2 = Shape[vertIDp1];

    float fT = float(splineTime)/float(splineDetail);
    v = GetHSpline(Vect(vert1.pos), Vect(vert2.pos), Vect(vert1.t[1]*3.0f), Vect(vert2.t[0]*3.0f), fT);

    ++splinePos;

    return TRUE;
}

void ShapeWindow::DeleteSelectedVerts()
{
    SaveUndoData(TEXT("Delete Selected Vertices"));

    DWORD nSelected = 0;
    for(int i=0; i<Shape.Num(); i++)
    {
        if(Shape[i].bSelected)
            ++nSelected;
    }

    if((Shape.Num()-nSelected) < 3)
        return;

    for(int i=0; i<Shape.Num(); i++)
    {
        if(Shape[i].bSelected)
        {
            Shape.Remove(i);
            --i;
        }
    }

    if(nSelected)
        shapeEditor->UpdateShapeView();
}


void ShapeWindow::SaveUndoData(CTSTR lpUndoName)
{
    bSavedChanges = FALSE;

    Action action;
    action.strName = lpUndoName;
    action.actionProc = UndoRedoShapeAction;
    BufferOutputSerializer sOut(action.data);

    SerializeData(sOut);

    shapeEditor->undoStack->Push(action);
}


void ShapeWindow::SerializeData(Serializer &s)
{
    s   << Shape
        << bCreating
        << splineDetail;
}

void ShapeWindow::CreateNewShape()
{
    /*if(!bSavedChanges)
    {
        if(MessageBox(shapeEditor->hwndShapeEditor, TEXT("You haven't saved your shape.  Are you sure?"), TEXT("Hi!"), MB_YESNO) == IDNO)
            return;
    }*/

    SetWindowText(shapeEditor->hwndShapeEditor, TEXT("Shape Editor - New Shape"));

    Shape.Clear();
    bCreating = TRUE;
    bSavedChanges = TRUE;

    shapeEditor->undoStack->Clear();
    shapeEditor->redoStack->Clear();
}

void ShapeWindow::OpenFile()
{
    /*if(!bSavedChanges)
    {
        if(MessageBox(shapeEditor->hwndShapeEditor, TEXT("You haven't saved your shape.  Are you sure?"), TEXT("Hi!"), MB_YESNO) == IDNO)
            return;
    }*/

    TCHAR lpFile[256];
    lpFile[0] = 0;

    OPENFILENAME ofn;
    zero(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = shapeEditor->hwndShapeEditor;
    ofn.lpstrFile = lpFile;
    ofn.nMaxFile = 255;

    ofn.lpstrFilter = TEXT("Shape Files (*.shp)\0*.shp\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = TEXT(".\\EditorFiles\\");
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    TCHAR curDirectory[256];
    GetCurrentDirectory(255, curDirectory);

    if(GetOpenFileName(&ofn))
    {
        SetCurrentDirectory(curDirectory);

        String strFile = lpFile;
        strFile.FindReplace(TEXT("\\"), TEXT("/"));

        scpy(lpCurFile, strFile);

        String title;
        TSTR lpTemp = lpCurFile;
        while(schr(lpTemp, '/')) lpTemp = schr(lpTemp, '/')+1;
        title << TEXT("Shape Editor - ") << lpTemp;
        SetWindowText(shapeEditor->hwndShapeEditor, title);

        XFileInputSerializer s;
        s.Open(lpFile);

        DWORD shapeVer;
        s << shapeVer;

        if(shapeVer != '\0phs')
        {
            String message;
            message << TEXT("File '") << lpFile << TEXT("' is not a valid shape file.");
            MessageBox(shapeEditor->hwndShapeEditor, message, NULL, MB_ICONERROR);
        }
        else
        {
            SerializeData(s);
            bSavedChanges = TRUE;
        }

        s.Close();
    }
}

void ShapeWindow::SaveFile(BOOL bSaveAs)
{
    if(!Shape.Num())
        return;

    TCHAR lpFile[256];
    lpFile[0] = 0;

    if(bSaveAs || (lpCurFile[0] == 0))
    {
        OPENFILENAME ofn;
        zero(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = shapeEditor->hwndShapeEditor;
        ofn.lpstrFile = lpFile;
        ofn.nMaxFile = 255;

        ofn.lpstrFilter = TEXT("Shape Files (*.shp)\0*.shp\0");
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = TEXT(".\\EditorFiles\\");
        ofn.Flags = OFN_PATHMUSTEXIST;

        TCHAR curDirectory[256];
        GetCurrentDirectory(255, curDirectory);

        if(!GetSaveFileName(&ofn))
        {
            SetCurrentDirectory(curDirectory);;
            return;
        }

        SetCurrentDirectory(curDirectory);

        scpy(lpCurFile, lpFile);

        String title;
        TSTR lpTemp = lpCurFile;
        while(schr(lpTemp, '/')) lpTemp = schr(lpTemp, '/')+1;

        if(!GetPathExtension(lpTemp).CompareI(TEXT("shp")))
            scat(lpCurFile, TEXT(".shp"));

        title << TEXT("Shape Editor - ") << lpTemp;
        SetWindowText(shapeEditor->hwndShapeEditor, title);
    }

    XFileOutputSerializer s;
    s.Open(lpCurFile, XFILE_OPENALWAYS);

    DWORD shapeVer = '\0phs';
    s << shapeVer;

    SerializeData(s);

    s.Close();

    bSavedChanges = TRUE;
}


void ShapeWindow::GenerateShapeData(List<Vect2> &ShapeData)
{
    Vect2 v1, v2, prevDir;
    int i;

    ShapeData.Clear();

    splinePos = 0;

    TraverseSpline(v1);
    ShapeData << v1;
    TraverseSpline(v2);
    prevDir = (v2-v1).Norm();
    v1 = v2;

    while(TraverseSpline(v2))
    {
        Vect2 curDir = (v2-v1).Norm();

        if(!curDir.CloseTo(prevDir))
            ShapeData << v1;

        prevDir = curDir;
        v1 = v2;
    }

    //reverse if counter clockwise
    DWORD bestVert = INVALID;
    float fBestDistS = 0.0f;

    for(i=0; i<ShapeData.Num(); i++)
    {
        Vect2 &pos = ShapeData[i];

        float fDistS = pos.Dot(pos);

        if(fDistS > fBestDistS)
        {
            bestVert = i;
            fBestDistS = fDistS;
        }
    }

    int prevID = (bestVert == 0) ? (ShapeData.Num()-1) : bestVert-1;
    int nextID = (bestVert == (ShapeData.Num()-1)) ? 0 : bestVert+1;

    Vect2 &vert = ShapeData[bestVert];
    Vect2 &prev = ShapeData[prevID];
    Vect2 &next = ShapeData[nextID];

    Vect2 norm1 = (prev-vert).Norm().GetCross();
    Vect2 norm2 = (vert-next).Norm().GetCross();
    Vect2 vertNorm = (norm1+norm2).Norm();

    if(vert.GetNorm().Dot(vertNorm) < 0.0f)
    {
        List<Vect2> newShapeData;

        newShapeData << ShapeData[0];

        for(i=1; i<ShapeData.Num(); i++)
            newShapeData.Insert(1, ShapeData[i]);

        ShapeData.CopyList(newShapeData);
    }
}


BOOL ShapeWindow::ValidShape()
{
    if((Shape.Num() < 3) || bCreating)
        return FALSE;

    List<Vect2> curShape;
    GenerateShapeData(curShape);

    int i, j;

    for(i=0; i<curShape.Num(); i++)
    {
        int ip1 = (i == (curShape.Num()-1)) ? 0 : (i+1);
        Line2 line1(curShape[i], curShape[ip1]);
        Vect2 line1Dir = line1.GetDirection();

        for(j=i+1; j<curShape.Num(); j++)
        {
            int jp1 = (j == (curShape.Num()-1)) ? 0 : (j+1);
            Line2 line2(curShape[j], curShape[jp1]);

            if((j == i+1) || (jp1 == i))
            {
                Vect2 line2Dir = line2.GetDirection();

                if(line2Dir.CloseTo(-line1Dir))
                    return FALSE;

                continue;
            }

            if((j == i) || (ip1 == j) || (jp1 == ip1))
                continue;

            if(line1.LinesIntersect(line2))
                return FALSE;
        }
    }

    return TRUE;
}


BOOL ShapeWindow::LineIntersectsShape(Vect2 &v1, Vect2 &v2, List<int> &FaceRefs, List<Vect2> &ShapeData)
{
    Line2 line1(v1, v2);

    for(int i=0; i<ShapeData.Num(); i++)
    {
        int ip1 = (i == (ShapeData.Num()-1)) ? 0 : (i+1);
        Line2 line2(ShapeData[i], ShapeData[ip1]);

        if(line1.LinesIntersect(line2))
            return TRUE;
    }

    for(int i=0; i<FaceRefs.Num(); i++)
    {
        int ip1 = (i == (FaceRefs.Num()-1)) ? 0 : (i+1);
        Line2 line2(ShapeData[FaceRefs[i]], ShapeData[FaceRefs[ip1]]);

        if(line1.LinesIntersect(line2))
            return TRUE;
    }

    return FALSE;
}


void ShapeWindow::Extrude()
{
    if(!ValidShape())
    {
        MessageBox(shapeEditor->hwndShapeEditor, TEXT("This is not a valid shape."), NULL, MB_ICONEXCLAMATION);
        return;
    }

    ExtrudeInfo ei;
    int i;

    if(DialogBoxParam(hinstMain, MAKEINTRESOURCE(IDD_EXTRUDE), shapeEditor->hwndShapeEditor, (DLGPROC)ExtrudeDialogProc, (LPARAM)&ei) != IDOK)
        return;

    List<Vect2> ShapeData;
    List<Face> Faces;
    GenerateShapeData(ShapeData);

    Triangulator triangulator;
    triangulator.Verts.CopyList(ShapeData);
    triangulator.Triangulate();
    Faces.CopyList(triangulator.Faces);
    //TriangulateShape(Faces, ShapeData);

    DWORD faceNum = Faces.Num();
    DWORD vertNum = ShapeData.Num();

    //------------------------------------------

    EditorBrush *brush = CreateObject(EditorBrush);

    //----------------------------------------------------

    brush->PointList.SetSize(vertNum*2);

    float minX = ShapeData[0].x, maxX = ShapeData[0].x;

    float minY = ShapeData[0].y, maxY = ShapeData[0].y;

    for(i=0; i<vertNum; i++)
    {
        brush->PointList[i] = 
        brush->PointList[i+vertNum] = ShapeData[i];

        if(ei.amount < 0.0f)
            brush->PointList[i].z = ei.amount;
        else
            brush->PointList[i+vertNum].z = ei.amount;

        minX = MIN(ShapeData[i].x, minX);
        minY = MIN(ShapeData[i].y, minY);

        maxX = MAX(ShapeData[i].x, maxX);
        maxY = MAX(ShapeData[i].y, maxY);
    }

    float multX = 1.0f/(maxX-minX);
    float multY = 1.0f/(maxY-minY);

    //----------------------------------------------------

    brush->UVList.SetSize(vertNum*2);

    for(i=0; i<vertNum; i++)
    {
        Vect2 &vert = ShapeData[i];

        UVCoord &frontUV = brush->UVList[i+vertNum];
        UVCoord &backUV = brush->UVList[i];

        backUV.y  = frontUV.y = -(vert.y-minY)*multY;

        backUV.x  = (vert.x-minX)*multX;
        frontUV.x = -backUV.x;
    }

    //----------------------------------------------------

    brush->FaceList.SetSize((faceNum*2)+(vertNum*2));

    for(i=0; i<faceNum; i++)
    {
        Face &firstFace = Faces[i];

        EditFace &frontFace = brush->FaceList[i];
        EditFace &backFace = brush->FaceList[i+faceNum];

        frontFace.pointFace.Set(firstFace.A, firstFace.C, firstFace.B);
        backFace.pointFace.Set(firstFace.A+vertNum, firstFace.B+vertNum, firstFace.C+vertNum);

        frontFace.smoothFlags = backFace.smoothFlags = 0x1;

        mcpy(frontFace.uvFace.ptr, frontFace.pointFace.ptr, sizeof(Face));
        mcpy(backFace.uvFace.ptr,  backFace.pointFace.ptr,  sizeof(Face));

        //todo: uv stuff, smooth stuff
        frontFace.polyID = 0;
        backFace.polyID = 1;
    }

    //----------------------------------------------------

    DWORD vertPos = (faceNum*2);
    Vect2 lastNorm;

    DWORD curSmooth = 0x2;

    float curY = 0.0f;

    BOOL bReverseDirection = FALSE;

    DWORD firstSmooth = INVALID;

    for(i=0; i<vertNum; i++)
    {
        DWORD ip1 = (i == (vertNum-1)) ? 0 : (i+1);

        DWORD curPos = vertPos+(i*2);

        EditFace &face1 = brush->FaceList[curPos];
        EditFace &face2 = brush->FaceList[curPos+1];

        face1.pointFace.Set(i, ip1, i+vertNum);
        face2.pointFace.Set(ip1, ip1+vertNum, i+vertNum);

        face1.polyID = face2.polyID = i+2;

        //------------------

        BOOL bNewSection = FALSE;

        Vect2 dir = (ShapeData[ip1]-ShapeData[i]);
        float len = dir.Len();
        Vect2 norm = dir/len;

        if(i == 0)
            lastNorm = (ShapeData[vertNum-1]-ShapeData[vertNum-2]).Norm();

        float dotVal = lastNorm.Dot(norm);

        if(CloseFloat(dotVal, 1.0f, EPSILON))
            dotVal = 1.0f;
        else if(CloseFloat(dotVal, -1.0f, EPSILON))
            dotVal = -1.0f;

        if(DEG(acosf(dotVal)) > ei.smoothAngle)
        {
            bNewSection = TRUE;

            if(firstSmooth == INVALID)
                firstSmooth = i;
        }

        face1.smoothFlags = face2.smoothFlags = curSmooth;

        lastNorm = norm;

        //------------------

        if(bNewSection && ei.bTextureToSmoothing)
            curY = 0.0f;

        if(!bReverseDirection && ei.bTextureAlwaysFaceUp && (norm.y > 0.0f))
        {
            curY = 0.0f;
            bReverseDirection = TRUE;
        }
        else if(bReverseDirection && ei.bTextureAlwaysFaceUp && (norm.y < 0.0f))
        {
            curY = 0.0f;
            bReverseDirection = FALSE;
        }

        UVCoord uvFront1, uvFront2, uvBack1, uvBack2;

        if(!bReverseDirection)
        {
            uvFront1.x = uvFront2.x = 0.0f;

            uvBack1.x = uvBack2.x = ei.amount*0.1f;

            uvFront1.y = uvBack1.y = curY;
            uvFront2.y = uvBack2.y = (curY += len*0.1f);
        }
        else
        {
            uvBack1.x = uvBack2.x = 0.0f;

            uvFront1.x = uvFront2.x = ei.amount*0.1f;

            uvFront1.y = uvBack1.y = -curY;
            uvFront2.y = uvBack2.y = -(curY += len*0.1f);
        }

        face1.uvFace.Set(brush->UVList.Add(uvFront1), brush->UVList.Add(uvFront2), brush->UVList.Add(uvBack1));
        face2.uvFace.Set(face1.uvFace.B, brush->UVList.Add(uvBack2), face1.uvFace.C);
    }

    //----------------------------------------------------

    if(firstSmooth != INVALID)
    {
        for(i=0; i<vertNum; i++)
        {
            DWORD curVert = i+firstSmooth;
            DWORD curVertP1 = (curVert+1);

            if(curVert >= vertNum)
                curVert -= vertNum;
            if(curVertP1 >= vertNum)
                curVertP1 -= vertNum;

            Vect2 norm = (ShapeData[curVertP1]-ShapeData[curVert]).Norm();

            if(i == 0)
                lastNorm = norm;

            float dotVal = lastNorm.Dot(norm);

            if(CloseFloat(dotVal, 1.0f, EPSILON))
                dotVal = 1.0f;
            else if(CloseFloat(dotVal, -1.0f, EPSILON))
                dotVal = -1.0f;

            if(DEG(acosf(dotVal)) > ei.smoothAngle)
            {
                if(curSmooth == 0x8)
                    curSmooth = 0x2;
                else
                    curSmooth <<= 1;

                if(i == (vertNum-1))
                {
                    if(curSmooth == 0x2)
                        curSmooth = 0x4;
                }
            }

            DWORD curFace = vertPos+(curVert*2);

            EditFace &face1 = brush->FaceList[curFace];
            EditFace &face2 = brush->FaceList[curFace+1];

            face1.smoothFlags = face2.smoothFlags = curSmooth;

            lastNorm = norm;
        }
    }

    //----------------------------------------------------

    Bounds newBounds;

    newBounds.Min = brush->PointList[0];
    for(i=1; i<brush->PointList.Num(); i++)
    {
        newBounds.Min.x = MIN(brush->PointList[i].x, newBounds.Min.x);
        newBounds.Min.y = MIN(brush->PointList[i].y, newBounds.Min.y);
        newBounds.Min.z = MIN(brush->PointList[i].z, newBounds.Min.z);
    }
    newBounds.Max = newBounds.Min;
    for(i=0; i<brush->PointList.Num(); i++)
    {
        newBounds.Max.x = MAX(brush->PointList[i].x, newBounds.Max.x);
        newBounds.Max.y = MAX(brush->PointList[i].y, newBounds.Max.y);
        newBounds.Max.z = MAX(brush->PointList[i].z, newBounds.Max.z);
    }

    //----------------------------------------------------

    brush->ProcessBasicMeshData();

    brush->MergeAllDuplicateVertices();

    brush->RebuildFaceNormals();
    brush->RebuildNormals();

    brush->UpdatePositionalData();

    brush->brushType = BrushType_WorkBrush;

    brush->GenerateUniqueName(TEXT("Extrude"));

    if(levelInfo->WorkBrush)
        EditorLevelInfo::DestroyWorkbrush();

    EditorLevelInfo::SaveCreateWorkbrushUndoData(TEXT("Extrude Shape"), brush);

    UpdateViewports();

    levelInfo->WorkBrush = brush;
}

void ShapeWindow::Spin()
{
}



LRESULT WINAPI ShapeViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_KEYDOWN:
            switch(wParam)
            {
                case VK_DELETE:
                    shapeEditor->shapeWindow->DeleteSelectedVerts();
                    break;
            }
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_SHAPEEDITOR_FILE_CLOSE:
                    //if(MessageBox(shapeEditor->hwndShapeEditor, TEXT("You haven't saved your shape.  Are you sure?"), TEXT("Hi!"), MB_YESNO) == IDYES)
                    delete shapeEditor;
                    break;

                case ID_SHAPEEDITOR_FILE_NEW:
                    shapeEditor->shapeWindow->CreateNewShape();
                    break;

                case ID_SHAPEEDITOR_FILE_OPEN:
                    shapeEditor->shapeWindow->OpenFile();
                    break;

                case ID_SHAPEEDITOR_FILE_SAVE:
                    shapeEditor->shapeWindow->SaveFile(FALSE);
                    break;

                case ID_SHAPEEDITOR_FILE_SAVEAS:
                    shapeEditor->shapeWindow->SaveFile(TRUE);
                    break;

                case ID_MYEDIT_UNDO:
                    if(shapeEditor->undoStack->IsValid())
                    {
                        shapeEditor->undoStack->Pop();
                        shapeEditor->UpdateShapeView();
                    }
                    break;

                case ID_MYEDIT_REDO:
                    if(shapeEditor->redoStack->IsValid())
                    {
                        shapeEditor->redoStack->Pop();
                        shapeEditor->UpdateShapeView();
                    }
                    break;

                case ID_SPLINEDETAIL_1:
                    shapeEditor->shapeWindow->SaveUndoData(TEXT("Change Detail Level"));
                    shapeEditor->shapeWindow->splineDetail = 1;
                    shapeEditor->UpdateShapeView();
                    break;

                case ID_SPLINEDETAIL_2:
                    shapeEditor->shapeWindow->SaveUndoData(TEXT("Change Detail Level"));
                    shapeEditor->shapeWindow->splineDetail = 2;
                    shapeEditor->UpdateShapeView();
                    break;

                case ID_SPLINEDETAIL_3:
                    shapeEditor->shapeWindow->SaveUndoData(TEXT("Change Detail Level"));
                    shapeEditor->shapeWindow->splineDetail = 3;
                    shapeEditor->UpdateShapeView();
                    break;

                case ID_SPLINEDETAIL_4:
                    shapeEditor->shapeWindow->SaveUndoData(TEXT("Change Detail Level"));
                    shapeEditor->shapeWindow->splineDetail = 4;
                    shapeEditor->UpdateShapeView();
                    break;

                case ID_SPLINEDETAIL_5:
                    shapeEditor->shapeWindow->SaveUndoData(TEXT("Change Detail Level"));
                    shapeEditor->shapeWindow->splineDetail = 5;
                    shapeEditor->UpdateShapeView();
                    break;

                case ID_SPLINEDETAIL_10:
                    shapeEditor->shapeWindow->SaveUndoData(TEXT("Change Detail Level"));
                    shapeEditor->shapeWindow->splineDetail = 10;
                    shapeEditor->UpdateShapeView();
                    break;

                case ID_SPLINEDETAIL_CUSTOM:
                    {
                        int val;
                        if(DialogBoxParam(hinstMain, MAKEINTRESOURCE(IDD_CUSTOMSPLINEDETAIL), hwnd, (DLGPROC)CustomSplineDetailDialogProc, (LPARAM)&val) == IDOK)
                        {
                            shapeEditor->shapeWindow->SaveUndoData(TEXT("Change Detail Level"));
                            shapeEditor->shapeWindow->splineDetail = val;
                            shapeEditor->UpdateShapeView();
                        }
                        break;
                    }

                case ID_SHAPE_EXTRUDE:
                    shapeEditor->shapeWindow->Extrude();
                    break;

                case ID_SHAPE_SPIN:
                    shapeEditor->shapeWindow->Spin();
                    break;
            }
            break;

        case WM_SIZING:
            {
                RECT *rc = (RECT*)lParam;

                int borderXSize = 320;
                int borderYSize = 260;

                borderXSize += GetSystemMetrics(SM_CXSIZEFRAME)*2;

                borderYSize += GetSystemMetrics(SM_CYSIZEFRAME)*2;
                borderYSize += GetSystemMetrics(SM_CYCAPTION);
                borderYSize += GetSystemMetrics(SM_CYMENU);

                long width  = rc->right  - rc->left;
                long height = rc->bottom - rc->top;

                if(width < borderXSize)
                {
                    if( (wParam == WMSZ_TOPLEFT)    ||
                        (wParam == WMSZ_LEFT)       ||
                        (wParam == WMSZ_BOTTOMLEFT) )
                    {
                        rc->left = rc->right - borderXSize;
                    }
                    else
                        rc->right = rc->left + borderXSize;
                }
                if(height < borderYSize)
                {
                    if( (wParam == WMSZ_TOPLEFT)  ||
                        (wParam == WMSZ_TOP)      ||
                        (wParam == WMSZ_TOPRIGHT) )
                    {
                        rc->top = rc->bottom - borderYSize;
                    }
                    else
                        rc->bottom = rc->top + borderYSize;
                }

                return TRUE;
            }

        case WM_MOUSEWHEEL:
            if(LOWORD(wParam) & (MK_LBUTTON|MK_RBUTTON))
                break;
            else
            {
                float wheelAdjust = float((short)HIWORD(wParam));

                if(wheelAdjust > 0.0f)
                    wheelAdjust = (wheelAdjust*0.0025f)+1.0f;
                else
                    wheelAdjust = 1.0f/(fabs(wheelAdjust*0.0025f)+1.0f);

                float &zoom = shapeEditor->shapeWindow->zoom;

                zoom *= wheelAdjust;

                if(zoom < 10.0f)
                    zoom = 10.0f;
                else if(zoom > 400.0f)
                    zoom = 400.0f;

                shapeEditor->UpdateShapeView();
            }
            break;

        case WM_SIZE:
            if(shapeEditor && shapeEditor->shapeView)
                PostMessage(hwnd, WM_UPDATEVIEWPORTS, 0, 0);
            break;

        case WM_CLOSE:
            //if(MessageBox(shapeEditor->hwndShapeEditor, TEXT("You haven't saved your shape.  Are you sure?"), TEXT("Hi!"), MB_YESNO) == IDYES)
            delete shapeEditor;
            break;

        case WM_PAINT:
            {
                RECT rect;
                PAINTSTRUCT ps;

                if(GetUpdateRect(hwnd, &rect, FALSE))
                {
                    BeginPaint(hwnd, &ps);
                    EndPaint(hwnd, &ps);
                    if(shapeEditor && !bErrorMessage)
                        shapeEditor->UpdateShapeView();
                }
                return 0;
            }

        case WM_UPDATEVIEWPORTS:
            if(shapeEditor)
                shapeEditor->UpdateShapeView();
            return 0;

        case WM_PROCESSACCELERATORS:
            return TRUE;
    }

    return CallWindowProc((FARPROC)MainWndProc, hwnd, message, wParam, lParam);
}

void ENGINEAPI UpdateShapeEditMenu()
{
    HMENU hMainMenu = GetMenu(shapeEditor->hwndShapeEditor);
    HMENU hEditMenu = GetSubMenu(hMainMenu, 1);

    String strName;

    MENUITEMINFO mii;
    zero(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);

    strName = TEXT("&Undo");
    if(shapeEditor->undoStack->IsValid())
        strName << TEXT(" ") << shapeEditor->undoStack->GetCurrentName();
    strName << TEXT("\tCtrl-Z");

    mii.fMask  = MIIM_STATE|MIIM_STRING;
    mii.dwTypeData = (TSTR)strName;
    mii.fState = shapeEditor->undoStack->IsValid() ? MFS_ENABLED : MFS_GRAYED;
    SetMenuItemInfo(hEditMenu, ID_MYEDIT_UNDO, FALSE, &mii);

    strName = TEXT("&Redo");
    if(shapeEditor->redoStack->IsValid())
        strName << TEXT(" ") << shapeEditor->redoStack->GetCurrentName();
    strName << TEXT("\tCtrl-Z");

    mii.fMask  = MIIM_STATE|MIIM_STRING;
    mii.dwTypeData = (TSTR)strName;
    mii.fState = shapeEditor->redoStack->IsValid() ? MFS_ENABLED : MFS_GRAYED;
    SetMenuItemInfo(hEditMenu, ID_MYEDIT_REDO, FALSE, &mii);
}

void ENGINEAPI UndoRedoShapeAction(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    shapeEditor->shapeWindow->SerializeData(sOut);
    shapeEditor->shapeWindow->SerializeData(s);
}


BOOL CALLBACK CustomSplineDetailDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int *pIntVal;

    switch(message)
    {
        case WM_INITDIALOG:
            pIntVal = (int*)lParam;
            SetFocus(GetDlgItem(hwnd, IDC_EDIT_SPLINEDETAIL));
            return FALSE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    {
                        HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT_SPLINEDETAIL);

                        String value;
                        value.SetLength(SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0));
                        GetWindowText(hwndEdit, value, value.Length()+1);
                        *pIntVal = tstoi(value);

                        if(value.Length() && (*pIntVal < 40) && (*pIntVal > 0))
                            EndDialog(hwnd, IDOK);
                        else
                            MessageBox(hwnd, TEXT("You must enter a valid spline detail."), NULL, 0);
                        break;
                    }

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwnd, IDCANCEL);
    }
    return FALSE;
}


BOOL CALLBACK ExtrudeDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static ExtrudeInfo *pExtrudeInfo;

    switch(message)
    {
        case WM_INITDIALOG:
            {
                pExtrudeInfo = (ExtrudeInfo*)lParam;

                //-----------------------

                HWND hwndEdit = GetDlgItem(hwnd, IDC_EXTRUDEAMOUNT);
                SetFocus(hwndEdit);

                //-----------------------

                HWND hwndScroller = GetDlgItem(hwnd, IDC_SMOOTHANGLE_SCROLLER);
                hwndEdit = GetDlgItem(hwnd, IDC_SMOOTHANGLE);

                LinkUpDown(hwndScroller, hwndEdit);
                InitUpDownFloatData(hwndScroller, 45.0f, 0.0f, 180.0f);

                //-----------------------

                SendMessage(GetDlgItem(hwnd, IDC_TEXTURETOSMOOTHGROUPS), BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(GetDlgItem(hwnd, IDC_FACEUP), BM_SETCHECK, BST_CHECKED, 0);

                return FALSE;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                /*case IDC_TEXTURETOSMOOTHGROUPS:
                    EnableWindow(GetDlgItem(hwnd, IDC_FACEUP), TRUE);
                    break;

                case IDC_FULLWRAPAROUND:
                    EnableWindow(GetDlgItem(hwnd, IDC_FACEUP), FALSE);
                    SendMessage(GetDlgItem(hwnd, IDC_FACEUP), BM_SETCHECK, BST_UNCHECKED, 0);
                    break;*/

                case IDOK:
                    {
                        HWND hwndEdit = GetDlgItem(hwnd, IDC_EXTRUDEAMOUNT);

                        String value;
                        value.SetLength(SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0));

                        if(value.Length())
                        {
                            GetWindowText(hwndEdit, value, value.Length()+1);

                            if(ValidFloatString(value))
                            {
                                pExtrudeInfo->amount = (float)tstof(value);

                                if((fabsf(pExtrudeInfo->amount) < 400.0f) && (fabsf(pExtrudeInfo->amount) > EPSILON))
                                {
                                    pExtrudeInfo->smoothAngle = GetUpDownFloat(GetDlgItem(hwnd, IDC_SMOOTHANGLE_SCROLLER));
                                    pExtrudeInfo->bTextureToSmoothing = SendMessage(GetDlgItem(hwnd, IDC_TEXTURETOSMOOTHGROUPS), BM_GETCHECK, 0, 0) == BST_CHECKED;
                                    pExtrudeInfo->bTextureAlwaysFaceUp = SendMessage(GetDlgItem(hwnd, IDC_FACEUP), BM_GETCHECK, 0, 0) == BST_CHECKED;
                                    EndDialog(hwnd, IDOK);
                                    break;
                                }
                            }
                        }

                        MessageBox(hwnd, TEXT("You must enter a valid amount."), NULL, 0);
                        break;
                    }

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwnd, IDCANCEL);
    }
    return FALSE;
}


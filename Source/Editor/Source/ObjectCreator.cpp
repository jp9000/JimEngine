/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ObjectCreator.cpp

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


DefineAbstractClass(ObjectCreator);
DefineClass(BoxPrimitive);
DefineClass(FloorPrimitive);
DefineClass(PlanePrimitive);
DefineClass(EntityPlacer);
DefineClass(PrefabPlacer);
DefineClass(YPlaneAdjuster);


BoxPrimitive::BoxPrimitive()
{
    traceIn(BoxPrimitive::BoxPrimitive);

    curMode = -1;

    VBData *vbd = new VBData;

    vbd->VertList.SetSize(8);
    PointList = vbd->VertList.Array();

    vertBuffer = CreateVertexBuffer(vbd, FALSE);

    DWORD *indexList = (DWORD*)Allocate(12*2*sizeof(DWORD));

    indexList[0] = 0; indexList[1] = 4;
    indexList[2] = 4; indexList[3] = 5;
    indexList[4] = 5; indexList[5] = 1;
    indexList[6] = 1; indexList[7] = 0;

    indexList[ 8] = 2; indexList[ 9] = 6;
    indexList[10] = 6; indexList[11] = 7;
    indexList[12] = 7; indexList[13] = 3;
    indexList[14] = 3; indexList[15] = 2;

    indexList[16] = 2; indexList[17] = 0;
    indexList[18] = 6; indexList[19] = 4;
    indexList[20] = 7; indexList[21] = 5;
    indexList[22] = 3; indexList[23] = 1;

    idxBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, indexList, 12*2);

    traceOut;
}

BoxPrimitive::~BoxPrimitive()
{
    traceIn(BoxPrimitive::~BoxPrimitive);

    delete vertBuffer;
    delete idxBuffer;

    traceOut;
}

bool BoxPrimitive::Create(EditorViewport* vp, const Vect &startPosition)
{
    traceIn(BoxPrimitive::);

    DWORD i;

    if(vp->GetViewportType() == ViewportType_Main)
        return false;

    curAxis = vp->GetViewportType();

    for(i=0; i<8; i++)
        PointList[i] = startPosition;

    curMode = 0;

    if(levelInfo->WorkBrush)
        EditorLevelInfo::DestroyWorkbrush();
    levelInfo->WorkBrush = NULL;

    return true;

    traceOut;
}

void BoxPrimitive::MouseMove(EditorViewport* vp, const Vect &inputPosition)
{
    traceIn(BoxPrimitive::MouseMove);

    curMousePos = inputPosition;

    if(curMode == -1)
        return;

    DWORD i;

    if(curMode == 0)
    {
        for(i=0; i<8; i++)
        {
            switch(curAxis)
            {
                case ViewportType_Right:
                    if(i & 4)    PointList[i].z = inputPosition.z;
                    if(!(i & 2)) PointList[i].y = inputPosition.y;
                    break;
                case ViewportType_Left:
                    if(!(i & 4)) PointList[i].z = inputPosition.z;
                    if(!(i & 2)) PointList[i].y = inputPosition.y;
                    break;

                case ViewportType_Front:
                    if(i & 1)    PointList[i].x = inputPosition.x;
                    if(!(i & 2)) PointList[i].y = inputPosition.y;
                    break;
                case ViewportType_Back:
                    if(!(i & 1)) PointList[i].x = inputPosition.x;
                    if(!(i & 2)) PointList[i].y = inputPosition.y;
                    break;

                case ViewportType_Top:
                    if(i & 1)    PointList[i].x = inputPosition.x;
                    if(!(i & 4)) PointList[i].z = inputPosition.z;
                    break;
                case ViewportType_Bottom:
                    if(i & 1)    PointList[i].x = inputPosition.x;
                    if(i & 4)    PointList[i].z = inputPosition.z;
                    break;
            }
        }
    }
    else if(curMode == 1)
    {
        for(i=0; i<8; i++)
        {
            switch(curAxis)
            {
                case ViewportType_Right:
                    if(i & 1)    PointList[i].x = inputPosition.y-curHeightOffset; break;
                case ViewportType_Left:
                    if(!(i & 1)) PointList[i].x = curHeightOffset-inputPosition.y; break;

                case ViewportType_Front:
                    if(i & 4)    PointList[i].z = inputPosition.y-curHeightOffset; break;
                case ViewportType_Back:
                    if(!(i & 4)) PointList[i].z = curHeightOffset-inputPosition.y; break;

                case ViewportType_Top:
                    if(i & 2)    PointList[i].y = curHeightOffset-inputPosition.z; break;
                case ViewportType_Bottom:
                    if(!(i & 2)) PointList[i].y = curHeightOffset-inputPosition.z; break;
            }
        }
    }

    vertBuffer->FlushBuffers();

    traceOut;
}

bool BoxPrimitive::MouseDown(DWORD button, const Vect &inputPosition)
{
    traceIn(BoxPrimitive::MouseDown);

    if(curMode == -1)
        return true;

    if(button == MOUSE_LEFTBUTTON)
    {
        ++curMode;

        if(curMode == 2)
        {
            DWORD flatAxii=3;
            Vect &firstVector = PointList[0];
            BOOL bIgnoreX = FALSE, bIgnoreY = FALSE, bIgnoreZ = FALSE;

            for(DWORD i=1; i<8; i++)
            {
                if(!bIgnoreX && !CloseFloat(firstVector.x, PointList[i].x))
                {
                    --flatAxii;
                    bIgnoreX = TRUE;
                }
                if(!bIgnoreY && !CloseFloat(firstVector.y, PointList[i].y))
                {
                    --flatAxii;
                    bIgnoreY = TRUE;
                }
                if(!bIgnoreZ && !CloseFloat(firstVector.z, PointList[i].z))
                {
                    --flatAxii;
                    bIgnoreZ = TRUE;
                }
            }

            if(flatAxii)
            {
                --curMode;
                return true;
            }

            SaveMesh();

            zero(PointList, 8*sizeof(Vect));
            vertBuffer->FlushBuffers();

            curMode = -1;
            return false;
        }
    }

    return true;

    traceOut;
}

bool BoxPrimitive::MouseUp(DWORD button, const Vect &inputPosition)
{
    traceIn(BoxPrimitive::MouseUp);

    if(curMode == -1)
        return true;

    if(button == MOUSE_LEFTBUTTON)
    {
        if(curMode < 1)
            ++curMode;

        if(curMode == 1)
        {
            DWORD flatAxii=3;
            Vect &firstVector = PointList[0];
            BOOL bIgnoreX = FALSE, bIgnoreY = FALSE, bIgnoreZ = FALSE;

            for(DWORD i=1; i<8; i++)
            {
                if(!bIgnoreX && !CloseFloat(firstVector.x, PointList[i].x))
                {
                    --flatAxii;
                    bIgnoreX = TRUE;
                }
                if(!bIgnoreY && !CloseFloat(firstVector.y, PointList[i].y))
                {
                    --flatAxii;
                    bIgnoreY = TRUE;
                }
                if(!bIgnoreZ && !CloseFloat(firstVector.z, PointList[i].z))
                {
                    --flatAxii;
                    bIgnoreZ = TRUE;
                }
            }

            if(flatAxii != 1)
            {
                zero(PointList, 8*sizeof(Vect));
                vertBuffer->FlushBuffers();

                curMode = -1;
                return false;
            }

            switch(curAxis)
            {
                case ViewportType_Back:
                case ViewportType_Left:
                case ViewportType_Front:
                case ViewportType_Right:
                    curHeightOffset = inputPosition.y; break;

                case ViewportType_Bottom:
                case ViewportType_Top:
                    curHeightOffset = inputPosition.z; break;
            }
        }
    }

    return true;

    traceOut;
}

void BoxPrimitive::SetStatusString()
{
    traceIn(BoxPrimitive::SetStatusString);

    String strSize;

    float fXSize = fabsf(PointList[7].x-PointList[0].x);
    float fYSize = fabsf(PointList[7].y-PointList[0].y);
    float fZSize = fabsf(PointList[7].z-PointList[0].z);

    strSize << TEXT("X: ")
            << FormattedString(TEXT("%0.2f"), fXSize)
            << TEXT("), Y: ")
            << FormattedString(TEXT("%0.2f"), fYSize)
            << TEXT("), Z: ")
            << FormattedString(TEXT("%0.2f"), fZSize);

    editor->SetStatusText(0, strSize);

    traceOut;
}

void BoxPrimitive::Render()
{
    Shader* solidShader = GetVertexShader(TEXT("Base:SolidColor.vShader"));
    LoadVertexShader(solidShader);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    solidShader->SetColor(solidShader->GetParameter(1), 1.0f, 1.0f, 1.0f, 1.0f);
    LoadVertexBuffer(vertBuffer);
    LoadIndexBuffer(idxBuffer);

    Draw(GS_LINES);

    LoadIndexBuffer(NULL);

    MatrixPush();
    MatrixTranslate(curMousePos);
        LoadVertexBuffer(editor->snapGuide);
        Draw(GS_LINES);
    MatrixPop();

    LoadVertexBuffer(NULL);
    LoadVertexShader(NULL);
    LoadPixelShader(NULL);
}

void BoxPrimitive::SaveMesh()
{
    traceIn(BoxPrimitive::SaveMesh);

    DWORD i=0;

    EditorBrush *brush = CreateObject(EditorBrush);

    EditFace newFaces[12];
    Vect newVector;

    brush->PointList.SetSize(8);

    Bounds newBounds;

    newBounds.Min = PointList[0];
    for(i=1; i<8; i++)
    {
        newBounds.Min.x = MIN(PointList[i].x, newBounds.Min.x);
        newBounds.Min.y = MIN(PointList[i].y, newBounds.Min.y);
        newBounds.Min.z = MIN(PointList[i].z, newBounds.Min.z);
    }
    newBounds.Max = newBounds.Min;
    for(i=0; i<8; i++)
    {
        newBounds.Max.x = MAX(PointList[i].x, newBounds.Max.x);
        newBounds.Max.y = MAX(PointList[i].y, newBounds.Max.y);
        newBounds.Max.z = MAX(PointList[i].z, newBounds.Max.z);
    }

    brush->SetPos(newBounds.GetCenter());

    newBounds.Min -= brush->GetLocalPos();
    newBounds.Max -= brush->GetLocalPos();

    brush->PointList[0] = newBounds.GetPoint(0);
    brush->PointList[1] = newBounds.GetPoint(MAX_X);
    brush->PointList[2] = newBounds.GetPoint(MAX_Y);
    brush->PointList[3] = newBounds.GetPoint(MAX_Y|MAX_X);
    brush->PointList[4] = newBounds.GetPoint(MAX_Z);
    brush->PointList[5] = newBounds.GetPoint(MAX_Z|MAX_X);
    brush->PointList[6] = newBounds.GetPoint(MAX_Z|MAX_Y);
    brush->PointList[7] = newBounds.GetPoint(MAX_Z|MAX_Y|MAX_X);

    zero(newFaces, sizeof(EditFace)*12);

    /*newFaces[0].pointFace.Set(0, 1, 2);
    newFaces[1].pointFace.Set(3, 2, 1);
    newFaces[2].pointFace.Set(4, 6, 5);
    newFaces[3].pointFace.Set(7, 5, 6);

    newFaces[4].pointFace.Set(4, 0, 6);
    newFaces[5].pointFace.Set(2, 6, 0);
    newFaces[6].pointFace.Set(5, 7, 1);
    newFaces[7].pointFace.Set(3, 1, 7);

    newFaces[8] .pointFace.Set(4, 5, 0);
    newFaces[9] .pointFace.Set(1, 0, 5);
    newFaces[10].pointFace.Set(6, 2, 7);
    newFaces[11].pointFace.Set(3, 7, 2);*/

    newFaces[0].pointFace.Set(0, 2, 1);
    newFaces[1].pointFace.Set(3, 1, 2);
    newFaces[2].pointFace.Set(4, 5, 6);
    newFaces[3].pointFace.Set(7, 6, 5);

    newFaces[4].pointFace.Set(4, 6, 0);
    newFaces[5].pointFace.Set(2, 0, 6);
    newFaces[6].pointFace.Set(5, 1, 7);
    newFaces[7].pointFace.Set(3, 7, 1);

    newFaces[8] .pointFace.Set(4, 0, 5);
    newFaces[9] .pointFace.Set(1, 5, 0);
    newFaces[10].pointFace.Set(6, 7, 2);
    newFaces[11].pointFace.Set(3, 2, 7);

    for(i=0; i<12; i++)
    {
        newFaces[i].smoothFlags = 1<<(i/4);
        newFaces[i].polyID = i/2;
        brush->FaceList << newFaces[i];
    }

    brush->ProcessBasicMeshData();

    for(i=0; i<3; i++)
    {
        DWORD ip1 = (i == 2) ? 0 : i+1;

        DWORD face = i*4;
        DWORD uvOffset = i*8;

        float maxU,maxV;

        if(!i)
        {
            maxU = brush->PointList[7].ptr[i]   - brush->PointList[0].ptr[i];
            maxV = brush->PointList[7].ptr[ip1] - brush->PointList[0].ptr[ip1];
        }
        else
        {
            maxU = brush->PointList[7].ptr[ip1] - brush->PointList[0].ptr[ip1];
            maxV = brush->PointList[7].ptr[i]   - brush->PointList[0].ptr[i];
        }

        maxU *= 0.1f;
        maxV *= 0.1f;

        brush->UVList << UVCoord(0.0f , 0.0f);
        brush->UVList << UVCoord(0.0f , maxU);
        brush->UVList << UVCoord(-maxV, 0.0f);
        brush->UVList << UVCoord(-maxV, maxU);
                                       
        brush->UVList << UVCoord(0.0f , maxU);
        brush->UVList << UVCoord(-maxV, maxU);
        brush->UVList << UVCoord(0.0f , 0.0f);
        brush->UVList << UVCoord(-maxV, 0.0f);

        brush->FaceList[face]  .uvFace.Set(uvOffset,   uvOffset+2, uvOffset+1);
        brush->FaceList[face+1].uvFace.Set(uvOffset+3, uvOffset+1, uvOffset+2);

        brush->FaceList[face+2].uvFace.Set(uvOffset+4, uvOffset+6, uvOffset+5);
        brush->FaceList[face+3].uvFace.Set(uvOffset+7, uvOffset+5, uvOffset+6);
    }

    brush->RebuildFaceNormals();
    brush->RebuildNormals();

    brush->RebuildBounds();

    brush->GenerateUniqueName(TEXT("Box"));

    EditorLevelInfo::SaveCreateWorkbrushUndoData(TEXT("Create Box"), brush);

    levelInfo->WorkBrush = brush;
    brush = NULL;

    traceOut;
}


void ENGINEAPI EditorLevelInfo::SaveCreateWorkbrushUndoData(const String &undoName, EditorBrush *brush)
{
    traceIn(EditorLevelInfo::SaveCreateWorkbrushUndoData);

    Action action;
    action.strName = undoName;
    action.actionProc = EditorLevelInfo::UndoRedoCreateWorkBrush;
    BufferOutputSerializer s(action.data);

    s << brush->name;

    editor->undoStack->Push(action);

    traceOut;
}


void EditorLevelInfo::DestroyWorkbrush()
{
    traceIn(EditorLevelInfo::DestroyWorkbrush);

    Action action;
    action.strName = TEXT("Delete WorkBrush");
    action.actionProc = EditorLevelInfo::UndoRedoDeleteWorkBrush;
    BufferOutputSerializer s(action.data);

    s << levelInfo->WorkBrush->name;
    levelInfo->WorkBrush->Serialize(s);

    editor->undoStack->Push(action);

    DestroyObject(levelInfo->WorkBrush);
    levelInfo->WorkBrush = NULL;

    traceOut;
}



void ENGINEAPI EditorLevelInfo::UndoRedoCreateWorkBrush(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoCreateWorkBrush);

    String strBrush;

    s    << strBrush;
    sOut << strBrush;

    if(bUndo)
    {
        Entity *brush = Entity::FindByName(strBrush);
        brush->Serialize(sOut);

        DestroyObject(brush);

        levelInfo->WorkBrush = NULL;
    }
    else
    {
        EditorBrush *brush = CreateObject(EditorBrush);
        brush->name = strBrush;
        brush->Serialize(s);

        levelInfo->WorkBrush = brush;
    }

    traceOut;
}

void ENGINEAPI EditorLevelInfo::UndoRedoDeleteWorkBrush(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoDeleteWorkBrush);

    String strBrush;

    s    << strBrush;
    sOut << strBrush;

    if(!bUndo)
    {
        Entity *brush = Entity::FindByName(strBrush);
        brush->Serialize(sOut);

        DestroyObject(brush);

        levelInfo->WorkBrush = NULL;
    }
    else
    {
        EditorBrush *brush = CreateObject(EditorBrush);
        brush->name = strBrush;
        brush->Serialize(s);

        levelInfo->WorkBrush = brush;
    }

    traceOut;
}


bool FloorPrimitive::Create(EditorViewport *vp, const Vect &startPosition)
{
    traceIn(FloorPrimitive::Create);

    DWORD i;

    if( (vp->GetViewportType() != ViewportType_Main)    &&
        (vp->GetViewportType() != ViewportType_Top)     &&
        (vp->GetViewportType() != ViewportType_Bottom)  )
        return false;

    Vect snappedValue = startPosition;

    //SnapToSpacing(snappedValue, 10.0f);

    curAxis = vp->GetViewportType();

    for(i=0; i<8; i++)
        PointList[i] = snappedValue;

    curMode = 0;

    if(levelInfo->WorkBrush)
        EditorLevelInfo::DestroyWorkbrush();
    levelInfo->WorkBrush = NULL;

    for(i=0; i<8; i++)
        PointList[i].y = (i & 2) ? levelInfo->curYPlanePosition : (levelInfo->curYPlanePosition-1.0f);

    return true;

    traceOut;
}

void FloorPrimitive::MouseMove(EditorViewport *vp, const Vect &inputPosition)
{
    traceIn(FloorPrimitive::MouseMove);

    DWORD i;

    Vect snappedValue = inputPosition;

    //SnapToSpacing(snappedValue, 10.0f);

    curMousePos = snappedValue;

    if(curMode == -1)
        return;

    //-------------------

    if(curMode == 0)
    {
        for(i=0; i<8; i++)
        {
            switch(curAxis)
            {
                case ViewportType_Main:
                case ViewportType_Top:
                    if(i & 1)    PointList[i].x = snappedValue.x;
                    if(!(i & 4)) PointList[i].z = snappedValue.z;
                    break;
                case ViewportType_Bottom:
                    if(i & 1)    PointList[i].x = snappedValue.x;
                    if(i & 4)    PointList[i].z = snappedValue.z;
                    break;
            }
        }
    }

    vertBuffer->FlushBuffers();

    traceOut;
}

bool FloorPrimitive::MouseUp(DWORD button, const Vect &inputPosition)
{
    traceIn(FloorPrimitive::MouseUp);

    if(button == MOUSE_LEFTBUTTON)
    {
        ++curMode;

        DWORD flatAxii=3;
        Vect &firstVector = PointList[0];
        BOOL bIgnoreX = FALSE, bIgnoreY = FALSE, bIgnoreZ = FALSE;

        for(DWORD i=1; i<8; i++)
        {
            if(!bIgnoreX && !CloseFloat(firstVector.x, PointList[i].x))
            {
                --flatAxii;
                bIgnoreX = TRUE;
            }
            if(!bIgnoreY && !CloseFloat(firstVector.y, PointList[i].y))
            {
                --flatAxii;
                bIgnoreY = TRUE;
            }
            if(!bIgnoreZ && !CloseFloat(firstVector.z, PointList[i].z))
            {
                --flatAxii;
                bIgnoreZ = TRUE;
            }
        }

        if(flatAxii)
        {
            zero(PointList, 8*sizeof(Vect));
            vertBuffer->FlushBuffers();

            curMode = -1;
            return true;
        }

        SaveMesh();

        zero(PointList, 8*sizeof(Vect));
        vertBuffer->FlushBuffers();

        curMode = -1;

        return false;
    }
    return true;

    traceOut;
}

void FloorPrimitive::SetStatusString()
{
}

bool FloorPrimitive::ProcessKeyStroke(unsigned int kbc, bool bKeydown)
{
    traceIn(FloorPrimitive::ProcessKeyStroke);

    if(kbc == KBC_RETURN && bKeydown)
    {
        if(levelInfo->WorkBrush)
            levelInfo->WorkBrush->AddGeometry();
    }
    return false;

    traceOut;
}

//----------------------------------------

PlanePrimitive::PlanePrimitive()
{
    traceIn(PlanePrimitive::PlanePrimitive);

    curMode = -1;

    VBData *vbd = new VBData;

    vbd->VertList.SetSize(4);
    PointList = vbd->VertList.Array();

    vertBuffer = CreateVertexBuffer(vbd, FALSE);

    DWORD *indexList = (DWORD*)Allocate(5*sizeof(DWORD));

    indexList[0] = 0;
    indexList[1] = 1;
    indexList[2] = 3;
    indexList[3] = 2;
    indexList[4] = 0;

    idxBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, indexList, 5);

    traceOut;
}

PlanePrimitive::~PlanePrimitive()
{
    traceIn(PlanePrimitive::~PlanePrimitive);

    delete vertBuffer;
    delete idxBuffer;

    traceOut;
}

bool PlanePrimitive::Create(EditorViewport *vp, const Vect &startPosition)
{
    traceIn(PlanePrimitive::Create);

    DWORD i;

    Vect snappedValue = startPosition;

    curAxis = vp->GetViewportType();

    for(i=0; i<4; i++)
        PointList[i] = snappedValue;

    curMode = 0;

    if(levelInfo->WorkBrush)
        EditorLevelInfo::DestroyWorkbrush();
    levelInfo->WorkBrush = NULL;

    return true;

    traceOut;
}

void PlanePrimitive::MouseMove(EditorViewport *vp, const Vect &inputPosition)
{
    traceIn(PlanePrimitive::MouseMove);

    DWORD i;

    Vect snappedValue = inputPosition;

    curMousePos = snappedValue;

    if(curMode == -1)
        return;

    //-------------------

    if(curMode == 0)
    {
        for(i=1; i<4; i++)
        {
            switch(curAxis)
            {
                case ViewportType_Main:
                case ViewportType_Top:
                case ViewportType_Bottom:
                    if(i & 1) PointList[i].x = snappedValue.x;
                    if(i & 2) PointList[i].z = snappedValue.z;
                    break;
                case ViewportType_Left:
                case ViewportType_Right:
                    if(i & 1) PointList[i].z = snappedValue.z;
                    if(i & 2) PointList[i].y = snappedValue.y;
                case ViewportType_Front:
                case ViewportType_Back:
                    if(i & 1) PointList[i].x = snappedValue.x;
                    if(i & 2) PointList[i].y = snappedValue.y;
            }
        }
    }

    vertBuffer->FlushBuffers();

    traceOut;
}

bool PlanePrimitive::MouseUp(DWORD button, const Vect &inputPosition)
{
    traceIn(PlanePrimitive::MouseUp);

    if(button == MOUSE_LEFTBUTTON)
    {
        ++curMode;

        BOOL flatAxii = FALSE;
        Vect prevVector = PointList[0];

        for(DWORD i=1; i<4; i++)
        {
            if(prevVector.CloseTo(PointList[i]))
            {
                flatAxii = TRUE;
                break;
            }

            prevVector = PointList[i];
        }

        if(flatAxii)
        {
            zero(PointList, 4*sizeof(Vect));
            vertBuffer->FlushBuffers();

            curMode = -1;
            return true;
        }

        SaveMesh();

        zero(PointList, 4*sizeof(Vect));
        vertBuffer->FlushBuffers();

        curMode = -1;

        return false;
    }
    return true;

    traceOut;
}

void PlanePrimitive::SetStatusString()
{
}

bool PlanePrimitive::ProcessKeyStroke(unsigned int kbc, bool bKeydown)
{
    traceIn(PlanePrimitive::ProcessKeyStroke);

    if(kbc == KBC_RETURN && bKeydown)
    {
        if(levelInfo->WorkBrush)
            levelInfo->WorkBrush->AddGeometry();
    }
    return false;

    traceOut;
}

void PlanePrimitive::Render()
{
    Shader* solidShader = GetVertexShader(TEXT("Base:SolidColor.vShader"));
    LoadVertexShader(solidShader);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    solidShader->SetColor(solidShader->GetParameter(1), 1.0f, 1.0f, 1.0f, 1.0f);
    LoadVertexBuffer(vertBuffer);
    LoadIndexBuffer(idxBuffer);

    Draw(GS_LINESTRIP);

    LoadIndexBuffer(NULL);

    MatrixPush();
    MatrixTranslate(curMousePos);
        LoadVertexBuffer(editor->snapGuide);
        Draw(GS_LINES);
    MatrixPop();

    LoadVertexBuffer(NULL);
    LoadVertexShader(NULL);
    LoadPixelShader(NULL);
}

void PlanePrimitive::SaveMesh()
{
    traceIn(PlanePrimitive::SaveMesh);

    DWORD i=0;

    EditorBrush *brush = CreateObject(EditorBrush);

    EditFace newFaces[2];
    Vect newVector;

    brush->PointList.SetSize(4);
    brush->UVList.SetSize(4);

    brush->bCanSubtract = FALSE;

    Bounds newBounds;

    newBounds.Min = PointList[0];
    for(i=1; i<4; i++)
    {
        newBounds.Min.x = MIN(PointList[i].x, newBounds.Min.x);
        newBounds.Min.y = MIN(PointList[i].y, newBounds.Min.y);
        newBounds.Min.z = MIN(PointList[i].z, newBounds.Min.z);
    }
    newBounds.Max = newBounds.Min;
    for(i=0; i<4; i++)
    {
        newBounds.Max.x = MAX(PointList[i].x, newBounds.Max.x);
        newBounds.Max.y = MAX(PointList[i].y, newBounds.Max.y);
        newBounds.Max.z = MAX(PointList[i].z, newBounds.Max.z);
    }

    brush->SetPos(newBounds.GetCenter());

    newBounds.Min -= brush->GetLocalPos();
    newBounds.Max -= brush->GetLocalPos();

    brush->PointList[0] = newBounds.GetPoint(0);

    switch(curAxis)
    {
        case ViewportType_Main:
        case ViewportType_Top:
        case ViewportType_Bottom:
            brush->PointList[1] = newBounds.GetPoint(MAX_X);
            brush->PointList[2] = newBounds.GetPoint(MAX_Z);
            brush->PointList[3] = newBounds.GetPoint(MAX_Z|MAX_X);
            break;

        case ViewportType_Left:
        case ViewportType_Right:
            brush->PointList[1] = newBounds.GetPoint(MAX_Z);
            brush->PointList[2] = newBounds.GetPoint(MAX_Y);
            brush->PointList[3] = newBounds.GetPoint(MAX_Z|MAX_Y);
            break;

        case ViewportType_Front:
        case ViewportType_Back:
            brush->PointList[1] = newBounds.GetPoint(MAX_X);
            brush->PointList[2] = newBounds.GetPoint(MAX_Y);
            brush->PointList[3] = newBounds.GetPoint(MAX_X|MAX_Y);
    }

    zero(newFaces, sizeof(EditFace)*2);

    newFaces[0].pointFace.Set(0, 2, 1);
    newFaces[1].pointFace.Set(3, 1, 2);

    for(i=0; i<2; i++)
    {
        newFaces[i].smoothFlags = 1;
        newFaces[i].polyID = 0;
    }

    brush->FaceList.CopyArray(newFaces, 2);

    brush->ProcessBasicMeshData();

    float maxU,maxV;

    switch(curAxis)
    {
        case ViewportType_Main:
        case ViewportType_Top:
        case ViewportType_Bottom:
            maxU = brush->PointList[3].x - brush->PointList[0].x;
            maxV = brush->PointList[3].z - brush->PointList[0].z;
            break;

        case ViewportType_Left:
        case ViewportType_Right:
            maxU = brush->PointList[3].z - brush->PointList[0].z;
            maxV = brush->PointList[3].y - brush->PointList[0].y;
            break;

        case ViewportType_Front:
        case ViewportType_Back:
            maxU = brush->PointList[3].x - brush->PointList[0].x;
            maxV = brush->PointList[3].y - brush->PointList[0].y;
    }

    maxU *= 0.1f;
    maxV *= 0.1f;

    brush->UVList[0].Set(0.0f, 0.0f );
    brush->UVList[1].Set(0.0f, -maxU);
    brush->UVList[2].Set(maxV, 0.0f );
    brush->UVList[3].Set(maxV, -maxU);

    brush->FaceList[0].uvFace.Set(0, 2, 1);
    brush->FaceList[1].uvFace.Set(3, 1, 2);

    brush->RebuildFaceNormals();
    brush->RebuildNormals();

    brush->GenerateUniqueName(TEXT("Plane"));

    EditorLevelInfo::SaveCreateWorkbrushUndoData(TEXT("Create Plane"), brush);

    brush->RebuildBounds();

    levelInfo->WorkBrush = brush;
    brush = NULL;

    traceOut;
}

//----------------------------------------


EntityPlacer::EntityPlacer(EditMode lastEditMode, EntityType entityTypeIn)
 : prevEditMode(lastEditMode), entityType(entityTypeIn)
{
    curRot.SetIdentity();
}

EntityPlacer::~EntityPlacer()
{
    traceIn(EntityPlacer::~EntityPlacer);

    if(desiredEnt)
    {
        DestroyObject(desiredEnt);
        UpdateViewports();
    }

    traceOut;
}

bool  EntityPlacer::Create(EditorViewport* vp, const Vect &startPosition)
{
    traceIn(EntityPlacer::Create);

    if(desiredEnt)
    {
        //desiredEnt->UserCreatedObjectType = TYPE_OBJECT;
        desiredEnt->SetSelected(FALSE);
        desiredEnt->GenerateUniqueName();
        levelInfo->SaveObjectCreationUndoData(desiredEnt);
    }
    desiredEnt = NULL;

    /*levelInfo->curEditMode = prevEditMode;
    levelInfo->newObject = NULL;
    delete this;
    return false;*/

    return false;

    traceOut;
}

void  EntityPlacer::MouseMove(EditorViewport* vp, const Vect &inputPosition)
{
    traceIn(EntityPlacer::MouseMove);

    if(!bEnabled)
        return;

    lastViewport = NULL;

    if(editor->selectedEntityClass->IsAbstract())
        return;

    if(vp->GetViewportType() != ViewportType_Main)
    {
        if(desiredEnt)
        {
            DestroyObject(desiredEnt);
            desiredEnt = NULL;
        }
        return;
    }

    lastViewport = vp;

    BOOL bFoundSomething = FALSE;
    DWORD i;

    DWORD bestBrush;
    DWORD bestPoly = INVALID;
    Entity *bestEnt = NULL;
    Plane bestPlane;
    float bestDist = INFINITE;

    //---------------------------------------

    //todo - optimization required here
    Entity *ent = Entity::FirstEntity();
    while(ent)
    {
        BOOL bIsEditorObj = ent->IsOf(GetClass(EditorObject));

        if(ent->IsOf(GetClass(EditorBrush)))
        {
            ent = ent->NextEntity();
            continue;
        }
            
        if(bIsEditorObj || ent->GetEditType())
        {
            PhyCollisionInfo ci;

            if(ent->GetRayCollision(vp->mouseOrig, vp->mouseDir, &ci))
            {
                float potentialDist = ci.hitPos.Dist(vp->mouseOrig);
                if((potentialDist > 0.0f) && (potentialDist < bestDist))
                {
                    bestDist = potentialDist;
                    bestPlane = ci.HitPlane();
                    bestEnt = ent;

                    bFoundSomething = TRUE;
                }
            }
        }

        ent = ent->NextEntity();
    }

    //---------------------------------------

    for(i=0; i<levelInfo->BrushList.Num(); i++)
    {
        EditorBrush *brush = levelInfo->BrushList[i];
        EditorMesh  &mesh  = brush->mesh;

        float fT;
        Plane facePlane;
        DWORD dwPoly = mesh.RayMeshTest(vp->mouseOrig, vp->mouseDir, &fT, &facePlane);

        if(dwPoly != INVALID)
        {
            if((fT > 0.0f) && (fT < bestDist))
            {
                bestPoly = dwPoly;
                bestBrush = i;
                bestDist = fT;
                bestPlane = facePlane;

                bFoundSomething = TRUE;
            }
        }
    }

    //---------------------------------------

    Vect hitPos = vp->mouseOrig+(vp->mouseDir*bestDist);

    if(!desiredEnt)
    {
        desiredEnt = (Entity*)editor->selectedEntityClass->Create();
        desiredEnt->InitializeObject(TRUE);
        desiredEnt->SetRot(curRot);
        desiredEnt->SetEditType(TYPE_OBJECT);
        desiredEnt->SetSelected(TRUE);
    }

    if(entityType == EntityYAlign)
    {
        if(desiredEnt->IsOf(GetClass(MeshEntity)))
        {
            MeshEntity *meshEnt = (MeshEntity*)desiredEnt;
            hitPos.y -= meshEnt->GetMeshBounds().Min.y+0.041f;
        }
        else if(desiredEnt->phyShape)
        {
            if(desiredEnt->phyShape->IsOf(GetClass(PhyCylinder)))
            {
                PhyCylinder *cylinder = static_cast<PhyCylinder*>(desiredEnt->phyShape);
                hitPos.y += cylinder->GetHalfHeight()+0.04f;
            }
            else if(desiredEnt->phyShape->IsOf(GetClass(PhyCapsule)))
            {
                PhyCapsule *capsule = static_cast<PhyCapsule*>(desiredEnt->phyShape);
                if(capsule->GetAxis() == PhyAxis_Y)
                    hitPos.y += capsule->GetHalfHeight()+0.04f;
                else
                    hitPos.y += capsule->GetRadius()+0.04f;
            }
            else if(desiredEnt->phyShape->IsOf(GetClass(PhyBox)))
            {
                PhyBox *box = static_cast<PhyBox*>(desiredEnt->phyShape);
                Vect halfExtents = box->GetHalfExtents();
                hitPos.y += halfExtents.y+0.04f;
            }
            else if(desiredEnt->phyShape->IsOf(GetClass(PhySphere)))
            {
                PhySphere *sphere = static_cast<PhySphere*>(desiredEnt->phyShape);
                hitPos.y += sphere->GetRadius()+0.04f;
            }
        }
        else
            hitPos.y += 2.04f;

        float spacing = levelInfo->gridSpacing;
        float halfSpacing = spacing*0.5f;
        hitPos.x -= (hitPos.x > 0.0f) ? fmodf(hitPos.x, spacing) : fmodf(hitPos.x, -spacing);
        hitPos.z -= (hitPos.z > 0.0f) ? fmodf(hitPos.z, spacing) : fmodf(hitPos.z, -spacing);
    }
    else if(entityType == EntityDetail)
    {
        Matrix mRot(curRot.GetInv());
        bestPlane.Transform(mRot);
        float offset;

        if(desiredEnt->IsOf(GetClass(MeshEntity)))
        {
            MeshEntity *meshEnt = (MeshEntity*)desiredEnt;
            offset = meshEnt->GetInitialBounds().MinDistFrom(bestPlane)-0.04f;
        }
        else if(desiredEnt->phyShape)
            offset = desiredEnt->phyShape->GetNormalOffset(bestPlane.Dir);
        else
            offset = 2.04f;

        Vect normOffset = bestPlane.Dir*offset;
        normOffset.TransformVector(mRot.Transpose());
        hitPos -= normOffset;
    }

    desiredEnt->SetPos(hitPos);

    traceOut;
}

bool  EntityPlacer::MouseDown(DWORD button, const Vect &inputPosition)
{
    traceIn(EntityPlacer::MouseDown);

    if((button == MOUSE_RIGHTBUTTON) && desiredEnt)
    {
        curRot *= AxisAngle(0.0f, 1.0f, 0.0f, RAD(90.0f));
        desiredEnt->SetRot(curRot);
        MouseMove(lastViewport, Vect(0.0f, 0.0f, 0.0f));
        UpdateViewports();
    }
    return true;

    traceOut;
}

bool  EntityPlacer::ProcessKeyStroke(unsigned int kbc, bool bKeydown)
{
    traceIn(EntityPlacer::ProcessKeyStroke);

    if(!bKeydown)
        return false;

    if(!desiredEnt)
        return false;

    switch(kbc)
    {
        case KBC_LEFT:
            curRot *= AxisAngle(0.0f, 1.0f, 0.0f, RAD(90.0f));
            desiredEnt->SetRot(curRot);
            MouseMove(lastViewport, Vect(0.0f, 0.0f, 0.0f));
            UpdateViewports();
            return true;
        case KBC_RIGHT:
            curRot *= AxisAngle(0.0f, 1.0f, 0.0f, RAD(-90.0f));
            desiredEnt->SetRot(curRot);
            MouseMove(lastViewport, Vect(0.0f, 0.0f, 0.0f));
            UpdateViewports();
            return true;
    }

    return false;

    traceOut;
}

void  EntityPlacer::SetStatusString()
{
}

void EntityPlacer::SetEnabled(bool bEnabledIn)
{
    traceIn(EntityPlacer::SetEnabled);

    Super::SetEnabled(bEnabledIn);

    if(!bEnabledIn && desiredEnt)
    {
        DestroyObject(desiredEnt);
        desiredEnt = NULL;
    }

    traceOut;
}


PrefabPlacer::PrefabPlacer(EditMode lastEditMode, PrefabType prefabTypeIn)
 : prevEditMode(lastEditMode), prefabType(prefabTypeIn)
{
    curRot.SetIdentity();
}

PrefabPlacer::~PrefabPlacer()
{
    traceIn(PrefabPlacer::PrefabPlacer);

    if(desiredEnt)
    {
        DestroyObject(desiredEnt);
        UpdateViewports();
    }

    traceOut;
}


bool  PrefabPlacer::Create(EditorViewport* vp, const Vect &startPosition)
{
    traceIn(PrefabPlacer::Create);

    if(desiredEnt)
    {
        desiredEnt->SetEditType(TYPE_PREFAB);
        desiredEnt->GenerateUniqueName();
        levelInfo->SaveObjectCreationUndoData(desiredEnt);
    }
    desiredEnt = NULL;

    /*levelInfo->curEditMode = prevEditMode;
    levelInfo->newObject = NULL;
    delete this;*/
    return false;

    traceOut;
}

void  PrefabPlacer::MouseMove(EditorViewport* vp, const Vect &inputPosition)
{
    traceIn(PrefabPlacer::MouseMove);

    if(!bEnabled)
        return;

    lastViewport = NULL;

    if(!levelInfo || levelInfo->selectedPrefab.IsEmpty())
        return;

    if(vp->GetViewportType() != ViewportType_Main)
    {
        if(desiredEnt)
        {
            DestroyObject(desiredEnt);
            desiredEnt = NULL;
        }
        return;
    }

    lastViewport = vp;

    BOOL bFoundSomething = FALSE;
    DWORD i;

    DWORD bestBrush;
    DWORD bestPoly = INVALID;
    Entity *bestEnt = NULL;
    Plane bestPlane;
    float bestDist = INFINITE;

    //---------------------------------------

    if(prefabType != PrefabFloor && prefabType != PrefabYAlign)
    {
        // todo - optimization required here
        Entity *ent = Entity::FirstEntity();
        while(ent)
        {
            BOOL bIsEditorObj = ent->IsOf(GetClass(EditorObject));

            if(ent->IsOf(GetClass(EditorBrush)))
            {
                ent = ent->NextEntity();
                continue;
            }

            if(bIsEditorObj || ent->GetEditType())
            {
                PhyCollisionInfo ci;
                if(ent->GetRayCollision(vp->mouseOrig, vp->mouseDir, &ci))
                {
                    float potentialDist = ci.hitPos.Dist(vp->mouseOrig);
                    if((potentialDist > 0.0f) && (potentialDist < bestDist))
                    {
                        bestDist = potentialDist;
                        bestPlane = ci.HitPlane();
                        bestEnt = ent;

                        bFoundSomething = TRUE;
                    }
                }
            }

            ent = ent->NextEntity();
        }

        //---------------------------------------

        for(i=0; i<levelInfo->BrushList.Num(); i++)
        {
            EditorBrush *brush = levelInfo->BrushList[i];
            EditorMesh  &mesh  = brush->mesh;

            float fT;
            Plane facePlane;
            DWORD dwPoly = mesh.RayMeshTest(vp->mouseOrig, vp->mouseDir, &fT, &facePlane);

            if(dwPoly != INVALID)
            {
                if((fT > 0.0f) && (fT < bestDist))
                {
                    bestPoly = dwPoly;
                    bestBrush = i;
                    bestDist = fT;
                    bestPlane = facePlane;

                    bFoundSomething = TRUE;
                }
            }
        }
    }

    //---------------------------------------

    //if all else fails, try the Y plane
    if(!bFoundSomething)
    {
        bestPlane.Set(0.0f, 1.0f, 0.0f, levelInfo->curYPlanePosition);
        float fT;

        if(CloseFloat(vp->mouseDir.Dot(bestPlane.Dir), 1.57f, 0.2) || !bestPlane.GetRayIntersection(vp->mouseOrig, vp->mouseDir, fT))
        {
            if(desiredEnt)
            {
                DestroyObject(desiredEnt);
                desiredEnt = NULL;
            }
            return;
        }

        bestDist = fT;
    }

    Vect hitPos = vp->mouseOrig+(vp->mouseDir*bestDist);

    if(!desiredEnt)
    {
        String prefabPath;
        Engine::ConvertResourceName(levelInfo->selectedPrefab, TEXT("prefabs"), prefabPath, editor->bAddonModule);

        ConfigFile prefabFile;
        prefabFile.Open(prefabPath);
        BOOL bIsAnimated = prefabFile.HasKey(TEXT("Prefab"), TEXT("DefaultAnimation"));
        int lightmapRes = prefabFile.GetInt(TEXT("Prefab"), TEXT("LightmapRes"));
        prefabFile.Close();

        if(bIsAnimated)
        {
            AnimatedPrefab *prefab = (AnimatedPrefab*)CreateBare(AnimatedPrefab);
            desiredEnt = (MeshEntity*)prefab;

            prefab->prefabName = levelInfo->selectedPrefab;
        }
        else
        {
            Prefab *prefab = new Prefab;
            desiredEnt = (MeshEntity*)prefab;

            prefab->prefabName = levelInfo->selectedPrefab;

            desiredEnt->lightmapResolution = lightmapRes;
        }

        desiredEnt->InitializeObject();
        desiredEnt->SetRot(curRot);
    }

    if(desiredEnt->HasValidMesh())
    {
        if(prefabType == PrefabYAlign)
            meshOffset.Set(0.0f, desiredEnt->GetMeshBounds().Min.y, 0.0f);
        else if(prefabType == PrefabFloor)
            meshOffset.Set(0.0f, desiredEnt->GetMeshBounds().Max.y, 0.0f);
        else if(prefabType == PrefabDetail)
        {
            Matrix mRot(curRot.GetInv());
            bestPlane.Transform(mRot);
            meshOffset = bestPlane.Dir*desiredEnt->GetInitialBounds().MinDistFrom(bestPlane);
            meshOffset.TransformVector(mRot.Transpose());
        }
    }
    else
        meshOffset = 0.0f;

    if((prefabType == PrefabYAlign) || (prefabType == PrefabFloor))
    {
        float spacing = levelInfo->gridSpacing;
        float halfSpacing = spacing*0.5f;
        hitPos.x -= (hitPos.x > 0.0f) ? fmodf(hitPos.x, spacing) : fmodf(hitPos.x, -spacing);
        hitPos.z -= (hitPos.z > 0.0f) ? fmodf(hitPos.z, spacing) : fmodf(hitPos.z, -spacing);
    }

    hitPos -= meshOffset;

    desiredEnt->SetPos(hitPos);

    traceOut;
}

bool  PrefabPlacer::MouseDown(DWORD button, const Vect &inputPosition)
{
    traceIn(PrefabPlacer::MouseDown);

    if((button == MOUSE_RIGHTBUTTON) && desiredEnt)
    {
        curRot *= AxisAngle(0.0f, 1.0f, 0.0f, RAD(90.0f));
        desiredEnt->SetRot(curRot);
        MouseMove(lastViewport, Vect(0.0f, 0.0f, 0.0f));
        UpdateViewports();
    }
    return true;

    traceOut;
}

bool  PrefabPlacer::ProcessKeyStroke(unsigned int kbc, bool bKeydown)
{
    traceIn(PrefabPlacer::ProcessKeyStroke);

    if(!bKeydown)
        return false;

    if(!desiredEnt)
        return false;

    switch(kbc)
    {
        case KBC_LEFT:
            curRot *= AxisAngle(0.0f, 1.0f, 0.0f, RAD(90.0f));
            desiredEnt->SetRot(curRot);
            MouseMove(lastViewport, Vect(0.0f, 0.0f, 0.0f));
            UpdateViewports();
            return true;
        case KBC_RIGHT:
            curRot *= AxisAngle(0.0f, 1.0f, 0.0f, RAD(-90.0f));
            desiredEnt->SetRot(curRot);
            MouseMove(lastViewport, Vect(0.0f, 0.0f, 0.0f));
            UpdateViewports();
            return true;
    }

    return false;

    traceOut;
}


void  PrefabPlacer::SetStatusString()
{
}


void PrefabPlacer::SetEnabled(bool bEnabledIn)
{
    traceIn(PrefabPlacer::SetEnabled);

    Super::SetEnabled(bEnabledIn);

    if(!bEnabledIn && desiredEnt)
    {
        DestroyObject(desiredEnt);
        desiredEnt = NULL;
    }

    traceOut;
}



YPlaneAdjuster::YPlaneAdjuster()
{
    bStarted = false;
}


bool  YPlaneAdjuster::Create(EditorViewport* vp, const Vect &startPosition)
{
    traceIn(YPlaneAdjuster::Create);

    if(vp->GetViewportType() != ViewportType_Main)
        return false;

    realPos = curPos = lastPos = levelInfo->curYPlanePosition;

    GS->GetLocalMousePos(startX, startY);
    ControlWindow::PushCursorPos();
    OSShowCursor(FALSE);

    bRealTimeWasOn = editor->bRealTimeEnabled;
    if(bRealTimeWasOn)
        editor->EnableRealTimeRendering(false);

    bStarted = true;

    return true;

    traceOut;
}


void  YPlaneAdjuster::MouseMove(EditorViewport* vp, const Vect &inputPosition)
{
    traceIn(YPlaneAdjuster::MouseMove);

    if(!bStarted)
        return;

    int x, y;
    GS->GetLocalMousePos(x, y);
    y -= startY;

    realPos += float(y)*0.1f;

    if(realPos < -1280.0f)
        realPos = -1280.0f;
    else if(realPos > 1280.0f)
        realPos = 1280.0f;

    curPos = realPos;
    if(levelInfo->bSnapToGrid)
        curPos -= fmodf(curPos, levelInfo->gridSpacing);

    levelInfo->curYPlanePosition = curPos;

    ControlWindow::SetUnechoedCursorPos(startX, startY);

    traceOut;
}

bool  YPlaneAdjuster::MouseUp(DWORD button, const Vect &inputPosition)
{
    traceIn(YPlaneAdjuster::MouseUp);

    if(!bStarted)
        return true;

    if(button == MOUSE_LEFTBUTTON)
    {
        ControlWindow::PopCursorPos();
        OSShowCursor(TRUE);

        bStarted = false;

        if(!CloseFloat(curPos, lastPos, 0.1f))
        {
            EditorLevelInfo::SaveModifyYPlaneUndoData(TEXT("Change Grid Position"), lastPos);
            levelInfo->curYPlanePosition = curPos;
        }

        if(bRealTimeWasOn)
            editor->EnableRealTimeRendering(true);

        return false;
    }
    return true;

    traceOut;
}

bool  YPlaneAdjuster::ProcessKeyStroke(unsigned int kbc, bool bKeydown)
{
    if(kbc == KBC_ESCAPE)
    {
        if(bStarted)
        {
            ControlWindow::PopCursorPos();
            OSShowCursor(TRUE);

            realPos = curPos = levelInfo->curYPlanePosition = lastPos;
            bStarted = false;

            UpdateViewports();
            return false;
        }
    }
    return true;
}

void  YPlaneAdjuster::SetStatusString()
{
    if(!bStarted)
        editor->SetStatusText(0, NULL);
    else
        editor->SetStatusText(0, FormattedString(TEXT("New Y Grid Offset: %.3f"), curPos));
}

void ENGINEAPI EditorLevelInfo::SaveModifyYPlaneUndoData(CTSTR lpUndoName, float yPos)
{
    traceIn(EditorLevelInfo::SaveModifyYPlaneUndoData);

    OctLevel *octLevel = (OctLevel*)level;

    Action action;
    action.strName = lpUndoName;
    action.actionProc = EditorLevelInfo::UndoRedoModifyYPlane;

    BufferOutputSerializer s(action.data);
    s << yPos;

    editor->undoStack->Push(action);

    traceOut;
}

void ENGINEAPI EditorLevelInfo::UndoRedoModifyYPlane(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoModifyYPlane);

    OctLevel *octLevel = (OctLevel*)level;

    sOut << levelInfo->curYPlanePosition;
    s << levelInfo->curYPlanePosition;

    if(levelInfo->newObject && levelInfo->newObject->IsOf(GetClass(YPlaneAdjuster)))
    {
        YPlaneAdjuster *adjuster = (YPlaneAdjuster*)levelInfo->newObject;
        adjuster->realPos = adjuster->curPos = adjuster->lastPos = levelInfo->curYPlanePosition;
    }

    UpdateViewports();

    traceOut;
}

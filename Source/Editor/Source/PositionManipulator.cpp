/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  PositionManipulator.cpp

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

DefineClass(PositionManipulator);


void PositionManipulator::Init()
{
    traceIn(PositionManipulator::Init);

    Super::Init();

    int i;

    RenderStartNew();
        Vertex(0.0f, 0.0f, 1.0f);
        Vertex(0.0f, 0.0f, 8.0f);
    vbArrowLine = RenderSave();

    RenderStartNew();
        Vertex(0.0f, 4.0f, 0.0f);
        Vertex(4.0f, 4.0f, 0.0f);
    vbOffAxis1 = RenderSave();

    RenderStartNew();
        Vertex(4.0f, 0.0f, 0.0f);
        Vertex(4.0f, 4.0f, 0.0f);
    vbOffAxis2 = RenderSave();

    RenderStartNew();
        Vertex(0.0f, 0.0f, 8.0f);

        for(i=0; i<=10; i++)
        {
            float angle = RAD(float(i)*36.0f);

            Vertex(sin(angle)*0.5f, cos(angle)*0.5f, 8.0f);
        }
    vbArrowHeadBottom = RenderSave();

    RenderStartNew();
        Vertex(0.0f, 0.0f, 12.0f);

        for(i=10; i>=0; i--)
        {
            float angle = RAD(float(i)*36.0f);

            Vertex(sin(angle)*0.5f, cos(angle)*0.5f, 8.0f);
        }
    vbArrowHeadTop = RenderSave();

    traceOut;
}

void PositionManipulator::Destroy()
{
    traceIn(PositionManipulator::Destroy);

    Super::Destroy();

    delete vbArrowLine;
    delete vbOffAxis1;
    delete vbOffAxis2;
    delete vbArrowHeadBottom;
    delete vbArrowHeadTop;

    traceOut;
}


void PositionManipulator::ProcessMouseRay(const Vect &cameraDir, float scale, const Vect &rayOrig, const Vect &rayDir)
{
    traceIn(PositionManipulator::ProcessMouseRay);

    Bounds axisX(Vect(0.0f, -1.0f, -1.0f), Vect(12.0f, 1.0f, 1.0f));
    Bounds axisY(Vect(-1.0f, 0.0f, -1.0f), Vect(1.0f, 12.0f, 1.0f));
    Bounds axisZ(Vect(-1.0f, -1.0f, 0.0f), Vect(1.0f, 1.0f, 12.0f));

    Bounds axisXY(Vect(1.0f, 1.0f, -1.0f), Vect(5.0f, 5.0f, 1.0f));
    Bounds axisXZ(Vect(1.0f, -1.0f, 1.0f), Vect(5.0f, 1.0f, 5.0f));
    Bounds axisYZ(Vect(-1.0f, 1.0f, 1.0f), Vect(1.0f, 5.0f, 5.0f));

    axisX.Min *= scale; axisX.Max *= scale;
    axisY.Min *= scale; axisY.Max *= scale;
    axisZ.Min *= scale; axisZ.Max *= scale;

    axisXY.Min *= scale; axisXY.Max *= scale;
    axisXZ.Min *= scale; axisXZ.Max *= scale;
    axisYZ.Min *= scale; axisYZ.Max *= scale;

    axisX.Min += GetWorldPos(); axisX.Max += GetWorldPos();
    axisY.Min += GetWorldPos(); axisY.Max += GetWorldPos();
    axisZ.Min += GetWorldPos(); axisZ.Max += GetWorldPos();

    axisXY.Min += GetWorldPos(); axisXY.Max += GetWorldPos();
    axisXZ.Min += GetWorldPos(); axisXZ.Max += GetWorldPos();
    axisYZ.Min += GetWorldPos(); axisYZ.Max += GetWorldPos();

    bManipulating = true;

    zero(&clickOffset, sizeof(clickOffset));

    if((1.0f-fabsf(rayDir.x) > 0.05f) && axisX.RayIntersects(rayOrig, rayDir))
    {
        activeAxis = AXIS_X;
        ClosestLinePoint(GetWorldPos(), Vect(1.0f, 0.0f, 0.0f), rayOrig, rayDir, clickOffset.x);
    }
    else if((1.0f-fabsf(rayDir.y) > 0.05f) && axisY.RayIntersects(rayOrig, rayDir))
    {
        activeAxis = AXIS_Y;
        ClosestLinePoint(GetWorldPos(), Vect(0.0f, 1.0f, 0.0f), rayOrig, rayDir, clickOffset.y);
    }
    else if((1.0f-fabsf(rayDir.z) > 0.05f) && axisZ.RayIntersects(rayOrig, rayDir))
    {
        activeAxis = AXIS_Z;
        ClosestLinePoint(GetWorldPos(), Vect(0.0f, 0.0f, 1.0f), rayOrig, rayDir, clickOffset.z);
    }
    else if((fabsf(rayDir.Dot(Vect(0.0f, 0.0f, 1.0f))) > 0.05f) && axisXY.RayIntersects(rayOrig, rayDir))
    {
        activeAxis = AXIS_X|AXIS_Y;

        Plane xyPlane(0.0f, 0.0f, 1.0f, GetWorldPos().z);

        float fT;
        xyPlane.GetRayIntersection(rayOrig, rayDir, fT);
        clickOffset = rayOrig+(rayDir*fT);
    }
    else if((fabsf(rayDir.Dot(Vect(0.0f, 1.0f, 0.0f))) > 0.05f) && axisXZ.RayIntersects(rayOrig, rayDir))
    {
        activeAxis = AXIS_X|AXIS_Z;

        Plane xzPlane(0.0f, 1.0f, 0.0f, GetWorldPos().y);

        float fT;
        xzPlane.GetRayIntersection(rayOrig, rayDir, fT);
        clickOffset = rayOrig+(rayDir*fT);
    }
    else if((fabsf(rayDir.Dot(Vect(1.0f, 0.0f, 0.0f))) > 0.05f) && axisYZ.RayIntersects(rayOrig, rayDir))
    {
        activeAxis = AXIS_Y|AXIS_Z;

        Plane yzPlane(1.0f, 0.0f, 0.0f, GetWorldPos().x);

        float fT;
        yzPlane.GetRayIntersection(rayOrig, rayDir, fT);
        clickOffset = rayOrig+(rayDir*fT);
    }
    else
        bManipulating = false;

    traceOut;
}


void PositionManipulator::Manipulate(const Vect &rayOrig, const Vect &rayDir)
{
    traceIn(PositionManipulator::Manipulate);

    Vect adjust;

    BOOL bMoveRelative = (levelInfo->SelectedObjects.Num() > 1) ||
                         (levelInfo->SelectedObjects[0]->IsOf(GetClass(EditorBrush)));

    if((activeAxis <= 2) || (activeAxis == 4))
    {
        Vect axisDir;

        DWORD axis = activeAxis/2;

        zero(&axisDir, sizeof(axisDir));
        axisDir.ptr[axis] = 1.0f;

        float mouseOffset;
        ClosestLinePoint(GetWorldPos(), axisDir, rayOrig, rayDir, mouseOffset);

        float moveOffset = (mouseOffset-clickOffset.ptr[axis]);

        if(levelInfo->bSnapToGrid && bMoveRelative)
        {
            float spacing = levelInfo->gridSpacing;
            float spacingD2 = spacing*0.5f;

            moveOffset += (moveOffset > 0.0) ? spacingD2 : -spacingD2;
            moveOffset -= fmodf(moveOffset, spacing);
        }

        adjust = axisDir*moveOffset;
    }
    else
    {
        Plane testPlane;
        zero(&testPlane, sizeof(testPlane));

        switch(activeAxis)
        {
            case (AXIS_X|AXIS_Y):
                testPlane.Dir.z = 1.0f;
                testPlane.Dist = GetWorldPos().z;
                break;

            case (AXIS_X|AXIS_Z):
                testPlane.Dir.y = 1.0f;
                testPlane.Dist = GetWorldPos().y;
                break;

            case (AXIS_Y|AXIS_Z):
                testPlane.Dir.x = 1.0f;
                testPlane.Dist = GetWorldPos().x;
                break;
        }

        float fT;
        if(!testPlane.GetRayIntersection(rayOrig, rayDir, fT))
            return;

        adjust = (rayOrig+(rayDir*fT))-clickOffset;

        if(levelInfo->bSnapToGrid && bMoveRelative)
            editor->SnapPoint(adjust);
    }

    Vect newPos = GetWorldPos() + adjust;

    if(levelInfo->bSnapToGrid && !bMoveRelative)
        editor->SnapPoint(newPos);

    if((activeAxis == 3) || (activeAxis >= 5))
        clickOffset += newPos-GetWorldPos();

    SetPos(newPos);

    DWORD numObjects = levelInfo->SelectedObjects.Num();

    if(bBeginManipulation)
    {
        BOOL bHasWorkbrush = FALSE;

        for(DWORD i=0; i<numObjects; i++)
        {
            if(levelInfo->SelectedObjects[i] == levelInfo->WorkBrush)
            {
                bHasWorkbrush = TRUE;
                break;
            }
        }

        if(GetButtonState(KBC_SHIFT) && !bHasWorkbrush)
        {
            Action action;
            action.actionProc = EditorLevelInfo::UndoRedoDuplicate;
            action.strName = TEXT("Duplicate Object(s)");

            BufferOutputSerializer s(action.data);

            s << numObjects;
            for(DWORD i=0; i<numObjects; i++)
            {
                Entity *ent = levelInfo->SelectedObjects[i];
                s << ent->GetName();
                ent->SetSelected(FALSE);
            }

            List<Entity*> NewSelection;

            for(DWORD i=0; i<numObjects; i++)
            {
                Entity *ent = levelInfo->SelectedObjects[i];
                Entity *newEnt = ent->DuplicateEntity();

                if(newEnt)
                {
                    newEnt->SetSelected(TRUE);
                    NewSelection << newEnt;

                    if(!newEnt->IsOf(GetClass(EditorObject)))
                        newEnt->SetEditType(ent->GetEditType());
                }
            }

            levelInfo->SelectedObjects.Clear();
            levelInfo->SelectedObjects.CopyList(NewSelection);

            numObjects = levelInfo->SelectedObjects.Num();
            s << numObjects;
            for(DWORD i=0; i<numObjects; i++)
            {
                Entity *ent = levelInfo->SelectedObjects[i];
                s << ent->GetName();
            }

            editor->undoStack->Push(action);
        }
        else
        {
            Action action;
            action.actionProc = EditorLevelInfo::UndoRedoMovement;
            action.strName = TEXT("Move Object(s)");

            BufferOutputSerializer s(action.data);

            s << numObjects;

            for(DWORD i=0; i<numObjects; i++)
            {
                s << levelInfo->SelectedObjects[i]->GetName();
                s << Vect(levelInfo->SelectedObjects[i]->GetLocalPos());
            }

            editor->undoStack->Push(action);
        }
    }

    for(DWORD i=0; i<numObjects; i++)
    {
        Entity *ent = levelInfo->SelectedObjects[i];

        if(levelInfo->SelectedObjects.Num() == 1)
            ent->SetPos(newPos);
        else
            ent->SetPos(ent->GetWorldPos() + adjust);
    }

    bBeginManipulation = FALSE;

    traceOut;
}


void PositionManipulator::RenderScaled(const Vect &cameraDir, float scale)
{
    traceInFast(PositionManipulator::RenderScaled);

    AxisAngle baseRotation(0.0f, 0.0f, 1.0f, RAD(90.0f));

    Shader *solidShader = GetVertexShader(TEXT("Base:SolidColor.vShader"));
    LoadVertexShader(solidShader);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    for(DWORD i=0; i<3; i++)
    {
        if(fabsf(cameraDir.ptr[i]) == 1.0f)
            continue;

        AxisAngle rotation;
        Vect4 axisColor;

        zero(&rotation, sizeof(rotation));
        rotation.ptr[i] = 1.0f;
        rotation.w = RAD(90.0f);

        zero(&axisColor, sizeof(axisColor));
        axisColor.ptr[i] = 1.0f;
        axisColor.w = 1.0f;

        MatrixPush();
        MatrixTranslate(GetWorldPos());
        MatrixRotate(baseRotation);
        MatrixRotate(rotation);
        MatrixScale(scale, scale, scale);

            LoadIndexBuffer(NULL);

            DWORD axis = i*2;

            if(!axis)
                axis = 1;

            if(bManipulating && (activeAxis & axis))
                solidShader->SetColor(solidShader->GetParameter(1), 1.0f, 1.0f, 0.0f, 1.0f);
            else
                solidShader->SetColor(solidShader->GetParameter(1), axisColor);

            LoadVertexBuffer(vbArrowLine);
            Draw(GS_LINES);

            solidShader->SetColor(solidShader->GetParameter(1), axisColor);
            LoadVertexBuffer(vbArrowHeadTop);
            Draw(GS_TRIANGLEFAN);

            solidShader->SetColor(solidShader->GetParameter(1), axisColor*0.5f);
            LoadVertexBuffer(vbArrowHeadBottom);
            Draw(GS_TRIANGLEFAN);


        MatrixPop();
    }

    AxisAngle adjustRotation(0.0f, 0.0f, 1.0f, RAD(180.0f));

    for(DWORD i=0; i<3; i++)
    {
        Vect axisDir;
        zero(&axisDir, sizeof(axisDir));
        axisDir.ptr[i] = 1.0f;

        if(fabsf(axisDir.Dot(cameraDir.GetAbs())) < EPSILON)  //cameraDir.Dot(axisDir) == 0.0f
            continue;

        DWORD i2 = (i  == 2) ? 0 : i+1;
        DWORD i1 = (i2 == 2) ? 0 : i2+1;

        AxisAngle rotation;
        Vect4 axisColor1, axisColor2;

        zero(&rotation, sizeof(rotation));
        rotation.ptr[i] = 1.0f;
        rotation.w = RAD(90.0f);

        zero(&axisColor1, sizeof(axisColor1));
        axisColor1.ptr[i1] = 1.0f;
        axisColor1.w = 1.0f;
        zero(&axisColor2, sizeof(axisColor2));
        axisColor2.ptr[i2] = 1.0f;
        axisColor2.w = 1.0f;

        MatrixPush();
        MatrixTranslate(GetWorldPos());
        MatrixRotate(baseRotation);
        MatrixRotate(rotation);
        if(i)
            MatrixRotate(adjustRotation);
        MatrixScale(scale, scale, scale);

            DWORD axis = i*2;

            if(!axis)
                axis = 1;

            if(bManipulating && (activeAxis == (~axis&0x7)))
            {
                solidShader->SetColor(solidShader->GetParameter(1), 1.0f, 1.0f, 0.0f, 1.0f);

                LoadVertexBuffer(vbOffAxis1);
                Draw(GS_LINES);

                LoadVertexBuffer(vbOffAxis2);
                Draw(GS_LINES);
            }
            else
            {
                solidShader->SetColor(solidShader->GetParameter(1), axisColor1);
                LoadVertexBuffer(vbOffAxis1);
                Draw(GS_LINES);

                solidShader->SetColor(solidShader->GetParameter(1), axisColor2);
                LoadVertexBuffer(vbOffAxis2);
                Draw(GS_LINES);
            }

        MatrixPop();
    }

    LoadVertexBuffer(NULL);
    LoadVertexShader(NULL);
    LoadPixelShader(NULL);

    traceOutFast;
}


void ENGINEAPI EditorLevelInfo::UndoRedoMovement(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoMovement);

    DWORD numObjects;
    s << numObjects;

    sOut << numObjects;

    for(int i=0; i<numObjects; i++)
    {
        String name;
        Vect newPos;

        s << name;
        s << newPos;

        Entity *ent = Entity::FindByName(name);

        sOut << name;
        sOut << Vect(ent->GetLocalPos());

        SetWorldPos(ent, newPos);
        ent->UpdatePositionalData();

        //todo - has world position set here
        //ent->GetWorldPos() = newPos;
    }

    levelInfo->UpdateManipulatorPos();

    traceOut;
}

void ENGINEAPI EditorLevelInfo::UndoRedoDuplicate(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoDuplicate);

    //restore or remove old object selections
    DWORD numObjects;
    s << numObjects;
    sOut << numObjects;

    if(bUndo)
        levelInfo->Deselect();

    for(int i=0; i<numObjects; i++)
    {
        String name;
        s << name;
        sOut << name;

        Entity *ent = Entity::FindByName(name);
        if(bUndo)
            levelInfo->Select(ent);
    }

    if(bUndo)
    {
        s << numObjects;

        //delete new objects
        for(int i=0; i<numObjects; i++)
        {
            String name;

            s << name;

            Entity *ent = Entity::FindByName(name);

            sOut << ent->Pos << ent->Rot << ent->UserCreatedObjectType;

            levelInfo->Deselect(ent);
            DestroyObject(ent);
        }
    }
    else
    {
        List<Entity*> NewSelection;

        for(DWORD i=0; i<numObjects; i++)
        {
            Entity *ent = levelInfo->SelectedObjects[i];
            Entity *newEnt = ent->DuplicateEntity();

            if(newEnt)
            {
                newEnt->bSelected = TRUE;
                if(!newEnt->IsOf(GetClass(EditorObject)))
                    newEnt->UserCreatedObjectType = ent->UserCreatedObjectType;
                NewSelection << newEnt;

                s << newEnt->worldPos << newEnt->Rot << newEnt->UserCreatedObjectType;
                newEnt->SetPos(newEnt->worldPos);
            }
        }

        levelInfo->Deselect();

        numObjects = NewSelection.Num();
        sOut << numObjects;
        for(DWORD i=0; i<numObjects; i++)
        {
            Entity *ent = NewSelection[i];
            sOut << ent->name;

            levelInfo->Select(ent);
        }
    }

    levelInfo->UpdateManipulatorPos();

    traceOut;
}


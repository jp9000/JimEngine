/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  RotationManipulator.cpp

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

DefineClass(RotationManipulator);



void RotationManipulator::Init()
{
    traceIn(RotationManipulator::Init);

    Super::Init();

    DWORD i,j;

    for(i=0; i<30; i++)
    {
        float angle = RAD(float(i)*12.0f);

        for(j=0; j<3; j++)
        {
            Vect &pos  = axisLines[j][i];
            Vect &norm = axisNorms[j][i];

            DWORD offset1 = (j == 2) ? 0 : j+1;
            DWORD offset2 = (offset1 == 2) ? 0 : offset1+1;

            norm.ptr[j]       = 0.0f;
            norm.ptr[offset1] = sinf(angle);
            norm.ptr[offset2] = cosf(angle);

            pos = norm*8.0f;
        }
    }

    for(i=0; i<3; i++)
    {
        RenderStartNew();
            for(j=0; j<30; j++)
            {
                DWORD jp1 = (j == 29) ? 0 : j+1;

                Vertex(axisLines[i][j]);
                Normal(axisNorms[i][j]);
            }

            Vertex(axisLines[i][0]);
            Normal(axisNorms[i][0]);

        axisBuffers[i] = RenderSave();
    }

    traceOut;
}

void RotationManipulator::Destroy()
{
    traceIn(RotationManipulator::Destroy);

    Super::Destroy();

    delete axisBuffers[0];
    delete axisBuffers[1];
    delete axisBuffers[2];

    traceOut;
}


void RotationManipulator::ProcessMouseRay(const Vect &cameraDir, float scale, const Vect &rayOrig, const Vect &rayDir)
{
    traceIn(RotationManipulator::ProcessMouseRay);

    activeAxis = -1;
    curCameraDir = cameraDir;

    Matrix mat;
    mat.SetIdentity();
    mat.Translate(GetWorldPos());
    mat.Scale(scale, scale, scale);

    bManipulating = false;

    DWORD i,j;

    for(i=0; i<3; i++)
    {
        for(j=0; j<30; j++)
        {
            DWORD jp1 = (j == 29) ? 0 : j+1;

            Vect norm = axisNorms[i][j];

            if(norm.Dot(cameraDir) > 0.01f)
                continue;

            Vect  v1 = axisLines[i][j];
            Vect  v2 = axisLines[i][jp1];

            v1.TransformPoint(mat);
            v2.TransformPoint(mat);

            Vect  lineVec = (v2-v1);
            float lineLen = lineVec.Len();

            Vect  lineDir = lineVec*(1.0f/lineLen);

            float fT1, fT2;
            if(ClosestLinePoints(rayOrig, rayDir, v1, lineDir, fT1, fT2))
            {
                if((fT2 < 0.0f) || (fT2 > lineLen))
                    continue;

                Vect closestPoint1 = rayOrig+(rayDir*fT1);
                Vect closestPoint2 = v1+(lineDir*fT2);

                if(closestPoint1.Dist(closestPoint2) < (0.4f*scale))
                {
                    Vect axis(0.0f, 0.0f, 0.0f);
                    axis.ptr[i] = 1.0f;

                    activeAxis = i;
                    clickDir  = cameraDir.Cross(axis.Cross(norm)).Cross(cameraDir).Norm();
                    clickOrig = closestPoint1;

                    bManipulating = true;
                    return;
                }
            }
        }
    }

    mat.SetIdentity();
    mat.Translate(GetWorldPos());
    mat.Rotate(Quat().SetLookDirection(cameraDir));
    mat.Scale(scale*1.25f, scale*1.25f, scale*1.25f);

    for(j=0; j<30; j++)
    {
        DWORD jp1 = (j == 29) ? 0 : j+1;

        Vect norm = axisNorms[2][j];
        Vect v1   = axisLines[2][j];
        Vect v2   = axisLines[2][jp1];

        v1.TransformPoint(mat);
        v2.TransformPoint(mat);

        norm.TransformVector(mat);

        Vect  lineVec = (v2-v1);
        float lineLen = lineVec.Len();

        Vect  lineDir = lineVec*(1.0f/lineLen);

        float fT1, fT2;
        if(ClosestLinePoints(rayOrig, rayDir, v1, lineDir, fT1, fT2))
        {
            if((fT2 < 0.0f) || (fT2 > lineLen))
                continue;

            Vect closestPoint1 = rayOrig+(rayDir*fT1);
            Vect closestPoint2 = v1+(lineDir*fT2);

            if(closestPoint1.Dist(closestPoint2) < (0.6f*scale))
            {
                activeAxis = 4;
                clickDir  = cameraDir.Cross(norm);
                clickOrig = closestPoint1;

                curCameraDir = cameraDir;

                bManipulating = true;
                return;
            }
        }
    }

    traceOut;
}


void RotationManipulator::Manipulate(const Vect &rayOrig, const Vect &rayDir)
{
    traceIn(RotationManipulator::Manipulate);

    AxisAngle newRot;
    newRot.Clear();

    if(activeAxis < 3)
        newRot.ptr[activeAxis] = 1.0f;
    else
        newRot = curCameraDir;

    float fT = 0.0f;
    ClosestLinePoint(clickOrig, clickDir, rayOrig, rayDir, fT);

    newRot.w = fT*8.0f;

    if(levelInfo->bSnapToGrid)
    {
        //if(fabsf(adjustRot.w) > 5.0f)
        //    adjustRot.w = (adjustRot.w < 0.0f) ? -5.0f : 5.0f;
        //else
        //    return;
        newRot.w = floorf(newRot.w/10.0)*10.0f;
    }

    newRot.w = RAD(newRot.w);

    DWORD numObjects = levelInfo->SelectedObjects.Num();

    if(bBeginManipulation)
    {
        Action action;
        action.actionProc = EditorLevelInfo::UndoRedoRotation;
        action.strName = TEXT("Rotate Object(s)");

        BufferOutputSerializer s(action.data);

        s << numObjects;

        origRots.SetSize(numObjects);

        for(DWORD i=0; i<numObjects; i++)
        {
            s << levelInfo->SelectedObjects[i]->GetName();
            s << Quat(levelInfo->SelectedObjects[i]->GetLocalRot());

            origRots[i] = levelInfo->SelectedObjects[i]->GetLocalRot();
        }

        editor->undoStack->Push(action);
    }

    bBeginManipulation = FALSE;

    for(int i=0; i<numObjects; i++)
    {
        Entity *ent = levelInfo->SelectedObjects[i];

        ent->SetRot(newRot.GetQuat()*origRots[i]);
    }

    traceOut;
}


void RotationManipulator::RenderScaled(const Vect &cameraDir, float scale)
{
    traceInFast(RotationManipulator::RenderScaled);

    DWORD i;

    Shader *rotManipShader = GetVertexShader(TEXT("Editor:RotationManipulator.vShader"));
    LoadVertexShader(rotManipShader);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    rotManipShader->SetVector(rotManipShader->GetParameter(2), cameraDir);

    MatrixPush();
    MatrixTranslate(GetWorldPos());
    MatrixScale(scale, scale, scale);

        for(i=0; i<3; i++)
        {
            Vect axisColor(0.0f, 0.0f, 0.0f);

            axisColor.ptr[i] = 1.0f;
            if(i == activeAxis)
                axisColor.Set(1.0f, 1.0f, 0.0f);
            else
                axisColor.ptr[i] = 1.0f;

            rotManipShader->SetVector(rotManipShader->GetParameter(1), axisColor);

            LoadVertexBuffer(axisBuffers[i]);
            LoadIndexBuffer(NULL);

            Draw(GS_LINESTRIP);
        }

        LoadVertexBuffer(NULL);

        //----------------------------------------------------

        Shader *solidShader = GetVertexShader(TEXT("Base:SolidColor.vShader"));
        LoadVertexShader(solidShader);

        MatrixPush();
        MatrixRotate(Quat().SetLookDirection(cameraDir));

            LoadVertexBuffer(axisBuffers[2]);
            LoadIndexBuffer(NULL);

            solidShader->SetColor(solidShader->GetParameter(1), 0.5, 0.5, 0.5, 0.5);

            Draw(GS_LINESTRIP);

            MatrixScale(1.25f, 1.25f, 1.25f);

            //---------------

            if(activeAxis == 4)
                solidShader->SetColor(solidShader->GetParameter(1), 1.0f, 1.0f, 0.0, 1.0f);
            else
                solidShader->SetColor(solidShader->GetParameter(1), 0.8, 0.8, 0.8, 0.8);

            Draw(GS_LINESTRIP);

        MatrixPop();

    MatrixPop();

    LoadVertexShader(NULL);
    LoadPixelShader(NULL);

    traceOutFast;
}


void ENGINEAPI EditorLevelInfo::UndoRedoRotation(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoRotation);

    DWORD numObjects;
    s << numObjects;

    sOut << numObjects;

    for(int i=0; i<numObjects; i++)
    {
        String name;
        Quat newRot;

        s << name;
        s << newRot;

        Entity *ent = Entity::FindByName(name);

        sOut << name;
        sOut << Quat(ent->GetLocalRot());

        SetWorldRot(ent, newRot);
        ent->UpdatePositionalData();
        //todo: manually set here
        //ent->Rot = ent->GetWorldRot() = newRot;
    }

    UpdateViewports();

    traceOut;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Viewport.h

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


DefineClass(Viewport);


/*=========================================================
    Viewport functions
==========================================================*/

void Viewport::SetCamera(Camera *cam)
{
    if(camera)
        camera->assignedViewport = NULL;

    camera = cam;

    if(cam)
        cam->assignedViewport = this;
}

Camera *Viewport::GetCamera()
{
    return camera;
}

BOOL Viewport::GetMouseRay(int x, int y, Vect &rayOrig, Vect &rayDir)
{
    traceIn(Viewport::GetMouseRay);

    if(!camera)
        return FALSE;

    rayOrig.x = 2.0f*(((float(x)-GetLocalX())/GetSizeX())-0.5f)*camera->Right();
    rayOrig.y = 2.0f*(((float(y)-GetLocalY())/GetSizeY())-0.5f)*camera->Top();
    rayOrig.z = -camera->Near();

    Matrix camMatrix;
    camMatrix.SetIdentity();
    camMatrix *= camera->GetWorldPos();
    camMatrix *= camera->GetWorldRot();

    rayOrig.TransformPoint(camMatrix);

    if(camera->bPerspective)
        rayDir = (rayOrig - camera->GetWorldPos()).Norm();
    else
        rayDir = -camera->GetWorldRot().GetDirectionVector();

    return TRUE;

    traceOut;
}

BOOL Viewport::GetPointViewportPos(const Vect &point, Vect2 &pos)
{
    traceIn(Viewport::GetPointViewportPos);

    if(!camera)
        return FALSE;

    Plane nearPlane(0.0f, 0.0f, 1.0f, -camera->Near());

    Matrix invCamMatrix;
    invCamMatrix.SetIdentity();
    invCamMatrix *= camera->GetWorldPos();
    invCamMatrix *= camera->GetWorldRot();
    invCamMatrix.Transpose();

    Vect testPoint = point.GetTransformedPoint(invCamMatrix);

    float fT;
    if(nearPlane.GetIntersection(Vect(0.0f), testPoint, fT))
    {
        Vect2 screenPos(testPoint * fT);

        screenPos /= Vect2(camera->Right(), camera->Top());
        screenPos = ((screenPos*0.5f)+0.5f)*GetSize();

        screenPos.x = floorf(screenPos.x+0.5f);
        screenPos.y = floorf(screenPos.y+0.5f);

        if( screenPos.x < 0.0f || screenPos.x > GetSizeX() ||
            screenPos.y < 0.0f || screenPos.y > GetSizeY() )
            return FALSE;

        pos = screenPos;
        return TRUE;
    }

    return FALSE;

    traceOut;
}

void Viewport::Render()
{
    traceIn(Viewport::Render);

    if(camera && level && (GetSizeX() != 0.0f) && (GetSizeY() != 0.0f))
    {
        //------------------------------------------------
        //  Set Audio Listener Position
        if(camera->bSoundCamera)
        {
            SS->SetListenerOriantation(GetListenerRot());
            SS->SetListenerPosition(GetListenerPos());
        }

        //------------------------------------------------
        //  Draw World
        Vect curPos(GetRealPos());
        SetViewport((int)curPos.x, (int)curPos.y, (int)GetSizeX(), (int)GetSizeY());

        camera->LoadProjectionTransform();

        MatrixPush();
            MatrixIdentity();
            level->Draw(camera);
        MatrixPop();

        Set2DMode();

        ResetViewport();
    }

    traceOut;
}


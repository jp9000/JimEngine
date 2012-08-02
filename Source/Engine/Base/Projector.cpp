/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Projector:  Texture projector

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


DefineClass(Projector);
DefineClass(Decal);
DefineClass(LitDecal);
DefineClass(ShadowDecal);

List<Projector*> Projector::ProjectorList;


void Projector::Init()
{
    traceIn(Projector::Init);

    Class *thisClass = GetObjectClass();

    //this orders the projectors by class so we can have it automatically ordered by effect
    int i;
    for(i=0; i<ProjectorList.Num(); i++)
    {
        Class *curClass = ProjectorList[i]->GetObjectClass();
        if(curClass == thisClass)
            break;
    }

    ProjectorList.Insert(i, this);

    Super::Init();

    traceOut;
}

void Projector::Destroy()
{
    traceIn(Projector::Destroy);

    ProjectorList.RemoveItem(this);

    Super::Destroy();

    traceOut;
}

void Projector::OnUpdatePosition()
{
    traceIn(Projector::OnUpdatePosition);

    MeshTargets.Clear();
    BrushTargets.Clear();
    level->GetStaticGeometry(GetBounds().Transform(GetEntityTransform()), BrushTargets, MeshTargets);

    traceOut;
}

Bounds Projector::GetBounds()
{
    Bounds projBounds;
    projBounds.Min.z = znear;
    projBounds.Max.z = zfar;

    projBounds.Min.x = left;
    projBounds.Min.y = -bottom;

    projBounds.Max.x = right;
    projBounds.Max.y = -top;

    if(bPerspective)
    {
        float fdn = zfar/znear;
        projBounds.Min.x *= fdn;
        projBounds.Min.y *= fdn;

        projBounds.Max.x *= fdn;
        projBounds.Max.y *= fdn;
    }

    return projBounds;
}


void Decal::Init()
{
    traceIn(Decal::Init);

    effect = RM->GetInternalEffect(TEXT("Base:Decal.effect"));
    bRenderable = TRUE;

    SetPerspective(50.0f, 1.3f, 1.0f, 3.0f);
    texture = GetTexture(TEXT("Base:GrayTex1.tga"));

    Super::Init();

    traceOut;
}

void Decal::LoadProjector()
{
    effect->SetColor(effect->GetParameterByName(TEXT("decalColor")), decalColor);
}

void Decal::EditorRender()
{
    Super::EditorRender();
}

ShadowDecal::ShadowDecal(MeshEntity *ent)
{
    meshEnt = ent;
}


BOOL ShadowDecal::UpdatingPosition()
{
    traceInFast(ShadowDecal::UpdatingPosition);

    SetPos(meshEnt->GetMeshAdjustPos());

    if(GetParent())
        SetRot(GetParent()->GetWorldRot().GetInv() * shadowRot);
    else
        SetRot(shadowRot);

    Matrix rotMatrix = GetLocalRot();

    const Bounds &bounds = meshEnt->GetMeshBounds();

    float size = 0.0f;

    for(int i=0; i<8; i++)
    {
        Vect p = bounds.GetPoint(i);

        float dist;
        dist = fabsf(rotMatrix.X.Dot(p));
        if(dist > size) size = dist;

        dist = fabsf(rotMatrix.Y.Dot(p));
        if(dist > size) size = dist;

        dist = fabsf(rotMatrix.Z.Dot(p));
        if(dist > size) size = dist;
    }

    SetOrtho(-size, size, -size, size, -1.0f, 2.0f);
    bRenderable = TRUE;

    Shader *vShader = GetVertexShader(TEXT("Base:SolidColor.vShader"));

    LoadVertexShader(vShader);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    Texture *shadowTex = meshEnt->GetCompositeShadow();
    SetFrameBufferTarget(shadowTex);

    SetViewport(0, 0, shadowTex->Width(), shadowTex->Height());

    BeginScene();

    ClearColorBuffer(TRUE, 0);

    EnableDepthTest(FALSE);
    Ortho(-size, size, -size, size, -100, 100.0f);

    vShader->SetColor(vShader->GetParameter(1), 0xFF000000);

    MatrixPush();
    MatrixIdentity();
    MatrixRotate(GetLocalRot().GetInv());

    meshEnt->RenderBare(FALSE);

    MatrixPop();

    EndScene();

    SetFrameBufferTarget(NULL);

    EnableDepthTest(TRUE);

    LoadPixelShader(NULL);
    LoadVertexShader(NULL);

    ResetViewport();

    return TRUE;

    traceOutFast;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Light.cpp:  Dynamic Lighting

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


DefineAbstractClass(Light);
DefineClass(PointLight);
DefineClass(SpotLight);
DefineClass(DirectionalLight);


List<Light*> Light::LightList;
VertexBuffer *Light::editorPointVB = NULL;

Light::Light()
{
    traceIn(Light::Light);

    if(engine->InEditor())
    {
        if(!LightList.Num())
        {
            RenderStartNew();

            for(int i=0; i<=30; i++)
            {
                float angle = RAD(float(i)*12.0f);
                Vertex(-cosf(angle), sinf(angle), 0.0f);
            }

            for(int i=0; i<=30; i++)
            {
                float angle = RAD(float(i)*12.0f);
                Vertex(-cosf(angle), 0.0f, sinf(angle));
            }

            for(int i=0; i<=30; i++)
            {
                float angle = RAD(float(i)*12.0f);
                Vertex(0.0f, -cosf(angle), sinf(angle));
            }

            editorPointVB = RenderSave();
        }
    }

    LightList << this;

    bCastShadows = TRUE;
    bStaticLight = TRUE;
    color = 0xFFFFFFFF;
    intensity = 100;
    bEnableEmission = TRUE;

    lightVolume = 3.0f;

    traceOut;
}

Light::~Light()
{
    traceIn(Light::~Light);

    LightList.RemoveItem(this);

    if(engine->InEditor())
    {
        if(!LightList.Num())
        {
            DestroyObject(editorPointVB);
            editorPointVB = NULL;
        }
    }

    traceOut;
}

void Light::SetColor(float newR, float newG, float newB)
{
    color = Vect_to_RGB(Vect(newR, newG, newB));
}

void Light::SetColor(Color3 &newColor)
{
    color = Vect_to_RGB(newColor);
}

void Light::SetColor(DWORD newColor)
{
    color = newColor;
}

Entity *Light::DuplicateEntity()
{
    Light *ent = (Light*)this->GetObjectClass()->Create(FALSE);
    List<BYTE> data;

    BufferOutputSerializer sSave(data);
    this->Serialize(sSave);

    BufferInputSerializer sLoad(data);
    ent->Serialize(sLoad);
    ent->GenerateUniqueName();

    ent->bLightmapped = FALSE;

    ent->Init();

    return ent;
}

void Light::Serialize(Serializer &s)
{
    traceIn(Light::Serialize);

    Super::Serialize(s);

    s << bLightmapped;

    traceOut;
}


PointLight::PointLight()
{
    lightRange = 40.0f;
}

void PointLight::Init()
{
    traceIn(PointLight::Init);

    Entity::Init();

    if(!bStaticLight && bCastShadows && !bLightmapped)
        renderTexture = CreateCubeFrameBuffer(256, GS_RG16F);

    updateFaces = 0x3F;

    traceOut;
}

void PointLight::Destroy()
{
    traceIn(PointLight::Destroy);

    for(int i=0; i<LitEntities.Num(); i++)
        LitEntities[i]->RemoveLight(this);

    delete renderTexture;

    Entity::Destroy();

    traceOut;
}

void PointLight::Reinitialize()
{
    traceIn(PointLight::Reinitialize);

    Super::Reinitialize();

    if(!bStaticLight && bCastShadows && !renderTexture && !bLightmapped)
        renderTexture = CreateCubeFrameBuffer(256, GS_RG16F); //6144
    else if(bLightmapped || (!bCastShadows && renderTexture))
    {
        delete renderTexture;
        renderTexture = NULL;
    }

    updateFaces = 0x3F;
    UpdatePositionalData();

    traceOut;
}

void PointLight::OnUpdatePosition()
{
    updateFaces = 0x3F;

    for(int i=0; i<LitEntities.Num(); i++)
        LitEntities[i]->RemoveLight(this);

    LitEntities.Clear();
    LitBrushes.Clear();

    List<LevelObject*> objects;
    level->GetObjects(GetBounds() + GetWorldPos(), objects);

    for(int i=0; i<objects.Num(); i++)
    {
        LevelObject *levelObj = objects[i];

        if(levelObj->type == ObjectType_Entity)
        {
            if(levelObj->ent->IsOf(GetClass(MeshEntity)))
                ProcessEntity((MeshEntity*)levelObj->ent);
        }
        else if(!IsLightmapped() && levelObj->type == ObjectType_Brush)
            LitBrushes << levelObj->brush;
    }
}

void PointLight::ProcessEntity(MeshEntity *meshEnt)
{
    traceInFast(PointLight::ProcessEntity);

    if(IsLightmapped() && meshEnt->IsLightmapped())
        return;

    ViewClip clip;
    clip.SetFrustum(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, lightRange+1.0f);

    BOOL bMeshLit = FALSE;

    if(meshEnt->CanCastShadow())
    {
        for(int i=0; i<6; i++)
        {
            Vect dir(0.0f, 0.0f, 0.0f);
            dir.ptr[i/2] = (i%2) ? -1.0f : 1.0f;

            Quat rot;
            rot.SetLookDirection(dir);

            Matrix rotMatrix;
            rotMatrix.SetIdentity();
            rotMatrix.Translate(GetWorldPos());
            rotMatrix.Rotate(rot);

            ViewClip copyClip;
            copyClip = clip;
            copyClip.Transform(rotMatrix);
            copyClip.Transform(meshEnt->GetInvTransform());

            if(copyClip.BoundsVisible(meshEnt->GetMeshBounds()))
            {
                MeshLightInfo meshLight;
                meshLight.light = this;
                meshLight.side  = i;
                AddMeshLight(meshEnt, meshLight);
                bMeshLit = TRUE;
            }
        }
    }

    if(bMeshLit)
        LitEntities << meshEnt;

    traceOutFast;
}

void PointLight::RemoveEntity(MeshEntity *ent, int side)
{
    traceInFast(PointLight::RemoveEntity);

    LitEntities.RemoveItem(ent);
    if(ent->CanCastShadow())
        updateFaces |= 1<<side;

    traceOutFast;
}

BOOL PointLight::InsideClip(const ViewClip &clip)
{
    traceInFast(PointLight::InsideClip);

    return clip.SphereVisible(GetWorldPos(), IsSelected() ? lightRange : 0.5f);

    traceOutFast;
}

void PointLight::EditorRender()
{
    traceInFast(PointLight::EditorRender);

    Shader *shader = GetVertexShader(TEXT("Base:SolidColor.vShader"));
    LoadVertexShader(shader);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    LoadVertexBuffer(editorPointVB);
    LoadIndexBuffer(NULL);

    MatrixPush();
    MatrixTranslate(GetWorldPos());

    if(IsSelected())
    {
        shader->SetColor(shader->GetParameter(1), 0.75f, 0.75f, 0.5f);

        MatrixPush();
        MatrixScale(lightRange, lightRange, lightRange);
        Draw(GS_LINESTRIP);
        MatrixPop();

        shader->SetColor(shader->GetParameter(1), 1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
        shader->SetColor(shader->GetParameter(1), 0.75f, 0.75f, 0.0f, 1.0f);

    MatrixPush();
    MatrixScale(0.25f, 0.25f, 0.25f);
    Draw(GS_LINESTRIP);
    MatrixPop();

    MatrixPop();

    LoadVertexShader(NULL);
    LoadPixelShader(NULL);

    traceOutFast;
}


SpotLight::SpotLight()
{
    cutoff = 75.0f;
    lightRange = 40.0f;
    bStaticLight = FALSE;
    SpotlightMap = TEXT("Base:Default/spotlight.tga");
}


void SpotLight::Init()
{
    traceIn(SpotLight::Init);

    Entity::Init();

    if(!bStaticLight && bCastShadows && !bLightmapped)
        renderTexture = CreateFrameBuffer(256, 256, GS_RG16F);

    spotTexture = GetTexture(SpotlightMap, FALSE);

    updateFaces = 1;

    traceOut;
}

void SpotLight::Reinitialize()
{
    traceIn(SpotLight::Reinitialize);

    if(spotTexture)
        ReleaseTexture(spotTexture);
    spotTexture = GetTexture(SpotlightMap, FALSE);

    if(!bStaticLight && bCastShadows && !renderTexture && !bLightmapped)
        renderTexture = CreateFrameBuffer(256, 256, GS_RG16F);
    else if(bLightmapped || (!bCastShadows && renderTexture))
    {
        delete renderTexture;
        renderTexture = NULL;
    }

    updateFaces = TRUE;
    UpdatePositionalData();

    traceOut;
}

Bounds SpotLight::GetBounds()
{
    Bounds projBounds;
    projBounds.Min.z = -lightRange;
    projBounds.Max.z = 0.0f;//lightRange;

    float val = tanf(RAD(cutoff)*0.5f) * lightRange;

    projBounds.Min.x = -val;
    projBounds.Min.y = -val;

    projBounds.Max.x = val;
    projBounds.Max.y = val;

    return projBounds;
}

void SpotLight::OnUpdatePosition()
{
    traceInFast(SpotLight::OnUpdatePosition);

    for(int i=0; i<LitEntities.Num(); i++)
        LitEntities[i]->RemoveLight(this);

    LitBrushes.Clear();
    LitEntities.Clear();

    List<LevelObject*> objects;
    level->GetObjects(GetBounds().GetTransformedBounds(GetEntityTransform()), objects);

    for(int i=0; i<objects.Num(); i++)
    {
        LevelObject *levelObj = objects[i];

        if(levelObj->type == ObjectType_Entity)
        {
            if(levelObj->ent->IsOf(GetClass(MeshEntity)))
                ProcessEntity((MeshEntity*)levelObj->ent);
        }
        else if(!IsLightmapped() && levelObj->type == ObjectType_Brush)
            LitBrushes << levelObj->brush;
    }

    traceOutFast;
}

void SpotLight::ProcessEntity(MeshEntity *ent)
{
    traceInFast(SpotLight::ProcessEntity);

    if(IsLightmapped() && ent->IsLightmapped())
        return;

    ViewClip clip;
    clip.SetPerspective(cutoff, 1.0f, 1.0, lightRange+1.0f);
    clip.Transform(GetEntityTransform());
    clip.Transform(ent->GetInvTransform());

    if(clip.BoundsVisible(ent->GetMeshBounds()))
    {
        MeshLightInfo meshLight;
        meshLight.light = this;
        meshLight.side = 1;
        AddMeshLight(ent, meshLight);
        updateFaces = TRUE;

        LitEntities << ent;
    }

    traceOutFast;
}

void SpotLight::RemoveEntity(MeshEntity *meshEnt, int side)
{
    traceInFast(SpotLight::RemoveEntity);

    LitEntities.RemoveItem(meshEnt);
    updateFaces = TRUE;

    traceOutFast;
}

void SpotLight::Destroy()
{
    traceIn(SpotLight::Destroy);

    for(int i=0; i<LitEntities.Num(); i++)
        LitEntities[i]->RemoveLight(this);

    if(spotTexture)
        ReleaseTexture(spotTexture);
    spotTexture = NULL;

    delete renderTexture;

    Entity::Destroy();

    traceOut;
}

void SpotLight::EditorRender()
{
    traceInFast(SpotLight::EditorRender);

    DWORD i;

    Shader *shader = GetVertexShader(TEXT("Base:SolidColor.vShader"));
    LoadVertexShader(shader);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    float halfCutoff = RAD(cutoff*0.5f);
    float cutoffFourth = halfCutoff*0.25f;
    float sinCutoff = sinf(halfCutoff);
    float cosCutoff = -cosf(halfCutoff);

    MatrixPush();
        MatrixTranslate(GetWorldPos());
        MatrixRotate(GetWorldRot());

        RenderStart();

        Vertex(0.0f, 0.0f, 0.0f);
        Vertex(sinCutoff, 0.0f, cosCutoff);
        Vertex(0.0f, 0.0f, 0.0f);
        Vertex(-sinCutoff, 0.0f, cosCutoff);
        Vertex(0.0f, 0.0f, 0.0f);
        Vertex(0.0f, sinCutoff, cosCutoff);
        Vertex(0.0f, 0.0f, 0.0f);
        Vertex(0.0f, -sinCutoff, cosCutoff);

        for(i=0; i<4; i++)
        {
            float curPos1 = cutoffFourth*float(i);
            float curPos2 = cutoffFourth*float(i+1);
            float sinCutoff1 = sinf(curPos1);
            float cosCutoff1 = -cosf(curPos1);
            float sinCutoff2 = sinf(curPos2);
            float cosCutoff2 = -cosf(curPos2);

            Vertex(sinCutoff1, 0.0f, cosCutoff1);
            Vertex(sinCutoff2, 0.0f, cosCutoff2);

            Vertex(-sinCutoff1, 0.0f, cosCutoff1);
            Vertex(-sinCutoff2, 0.0f, cosCutoff2);

            Vertex(0.0f, sinCutoff1, cosCutoff1);
            Vertex(0.0f, sinCutoff2, cosCutoff2);

            Vertex(0.0f, -sinCutoff1, cosCutoff1);
            Vertex(0.0f, -sinCutoff2, cosCutoff2);
        }

        for(i=0; i<16; i++)
        {
            float curPos1 = RAD(22.5f)*float(i);
            float curPos2 = RAD(22.5f)*float(i+1);

            float sinCutoff1 = sinf(curPos1);
            float cosCutoff1 = cosf(curPos1);
            float sinCutoff2 = sinf(curPos2);
            float cosCutoff2 = cosf(curPos2);

            Vertex(sinCutoff1*sinCutoff, cosCutoff1*sinCutoff, cosCutoff);
            Vertex(sinCutoff2*sinCutoff, cosCutoff2*sinCutoff, cosCutoff);
        }

        if(IsSelected())
        {
            shader->SetColor(shader->GetParameter(1), 0.75f, 0.75f, 0.5f);

            MatrixPush();
            MatrixScale(lightRange, lightRange, lightRange);
            RenderStop(GS_LINES);
            MatrixPop();

            shader->SetColor(shader->GetParameter(1), 1.0f, 1.0f, 1.0f);
        }
        else
            shader->SetColor(shader->GetParameter(1), 0.75f, 0.75f, 0.0f);

        RenderStop(GS_LINES);

        RenderStart();
    MatrixPop();

    LoadVertexShader(NULL);
    LoadPixelShader(NULL);

    traceOutFast;
}

void DirectionalLight::Init()
{
    editorSprite = NULL;

    Super::Init();
}

void DirectionalLight::EditorRender()
{
    traceInFast(DirectionalLight::EditorRender);

    Super::EditorRender();

    Shader *shader = GetVertexShader(TEXT("Base:SolidColor.vShader"));
    LoadVertexShader(shader);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    MatrixPush();
        MatrixTranslate(GetWorldPos());
        MatrixRotate(GetWorldRot());

        RenderStart();

        for(int i=0; i<16; i++)
        {
            float curPos1 = RAD(22.5f)*float(i);
            float curPos2 = RAD(22.5f)*float(i+1);

            float sinCutoff1 = sinf(curPos1);
            float cosCutoff1 = cosf(curPos1);
            float sinCutoff2 = sinf(curPos2);
            float cosCutoff2 = cosf(curPos2);

            Vertex(sinCutoff1, cosCutoff1, 0.5f);
            Vertex(sinCutoff2, cosCutoff2, 0.5f);

            Vertex(sinCutoff1, cosCutoff1, -0.5f);
            Vertex(sinCutoff2, cosCutoff2, -0.5f);

            Vertex(sinCutoff1, cosCutoff1, 0.5f);
            Vertex(sinCutoff1, cosCutoff1, -0.5f);
        }

        if(IsSelected())
            shader->SetColor(shader->GetParameter(1), 1.0f, 1.0f, 1.0f);
        else
            shader->SetColor(shader->GetParameter(1), 0.75f, 0.75f, 0.0f);

        RenderStop(GS_LINES);
        RenderStart();

        if(IsSelected())
        {
            shader->SetColor(shader->GetParameter(1), 0.75f, 0.75f, 0.5f);

            for(int i=0; i<16; i++)
            {
                float curPos1 = RAD(22.5f)*float(i);
                float curPos2 = RAD(22.5f)*float(i+1);

                float sinCutoff1 = sinf(curPos1)*5.0f;
                float cosCutoff1 = cosf(curPos1)*5.0f;
                float sinCutoff2 = sinf(curPos2)*5.0f;
                float cosCutoff2 = cosf(curPos2)*5.0f;

                Vertex(sinCutoff1, cosCutoff1, 0.5f);
                Vertex(sinCutoff2, cosCutoff2, 0.5f);

                Vertex(sinCutoff1, cosCutoff1, -3.0f);
                Vertex(sinCutoff2, cosCutoff2, -3.0f);

                Vertex(sinCutoff1, cosCutoff1, 0.5f);
                Vertex(sinCutoff1, cosCutoff1, -3.0f);
            }

            RenderStop(GS_LINES);
            RenderStart();
        }
    MatrixPop();

    LoadVertexShader(NULL);
    LoadPixelShader(NULL);

    traceOutFast;
}

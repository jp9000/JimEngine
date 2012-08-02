/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Entity.cpp:  Bsae Entity class

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



DefineClass(MeshObject);
DefineClass(MeshEntity);


/*========================================================
  MeshObject
=========================================================*/

MeshObject::MeshObject(CTSTR lpMeshResource)
{
    traceIn(MeshObject::MeshObject);

    SetMesh(lpMeshResource);

    traceOut;
}

MeshObject::~MeshObject()
{
    traceIn(MeshObject::~MeshObject);

    RM->ReleaseMesh(mesh);

    for(int i=0; i<MaterialList.Num(); i++)
        ReleaseMaterial(MaterialList[i]);
    MaterialList.Clear();

    traceOut;
}

void MeshObject::SetMesh(CTSTR lpMeshResource)
{
    traceIn(MeshObject::SetMesh);

    mesh = RM->GetMesh(lpMeshResource);

    if(!MaterialList.Num())
    {
        MaterialList.SetSize(mesh->DefaultMaterialList.Num());
        for(int i=0; i<MaterialList.Num(); i++)
        {
            String resName = mesh->DefaultMaterialList[i].name; //converting from utf to wide
            MaterialList[i] = GetMaterial(String(resName));
        }
    }

    traceOut;
}

void MeshObject::SetMaterial(int texture, Material *material)
{
    traceIn(MeshObject::SetMaterial);

    assert(texture < MaterialList.Num());

    if(MaterialList[texture] == material)
        return;

    if(MaterialList[texture])
        ReleaseMaterial(MaterialList[texture]);

    if(material)
    {
        MaterialList[texture] = material;
        RM->AddMaterialRef(material);
    }
    else
        MaterialList[texture] = GetMaterial(String(mesh->DefaultMaterialList[texture].name));

    traceOut;
}

void MeshObject::Render()
{
    traceIn(MeshObject::Render);

    LoadVertexBuffer(mesh->VertBuffer);
    LoadIndexBuffer(mesh->IdxBuffer);

    for(DWORD i=0;i<mesh->nSections;i++)
    {
        DrawSection &section  = mesh->SectionList[i];
        Material    *material = MaterialList[i];

        if(!material)
            continue;

        if(material->GetEffect() == GetActiveEffect())
            material->LoadParameters();

        if(material->GetFlags() & MATERIAL_TWOSIDED)
        {
            GSCullMode cullMode;
            cullMode = GetCullMode();
            SetCullMode(GS_NEITHER);
            GS->Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
            SetCullMode(cullMode);
        }
        else
            GS->Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
    }

    traceOut;
}

void MeshObject::QuickRender()
{
    traceIn(MeshObject::RenderBare);

    LoadVertexBuffer(mesh->VertBuffer);
    LoadIndexBuffer(mesh->IdxBuffer);

    GS->Draw(GS_TRIANGLES, 0, 0, mesh->nFaces*3);

    LoadVertexBuffer(NULL);
    LoadIndexBuffer(NULL);

    traceOut;
}

void MeshObject::RenderBare()
{
    traceIn(MeshObject::RenderBare);

    LoadVertexBuffer(mesh->VertBuffer);
    LoadIndexBuffer(mesh->IdxBuffer);

    GS->DrawBare(GS_TRIANGLES, 0, 0, mesh->nFaces*3);

    LoadVertexBuffer(NULL);
    LoadIndexBuffer(NULL);

    traceOut;
}

/*========================================================
  MeshEntity
=========================================================*/

MeshEntity::MeshEntity()
: scale(1.0f, 1.0f, 1.0f), lightmapResolution(128), indirectLightDist(8.0f), adjustPos(0.0f, 0.0f, 0.0f), adjustRot(0.0f, 0.0f, 0.0f, 1.0f), bCastShadow(TRUE), wireframeColor(0xFF7F0000)
{
}

void MeshEntity::Destroy()
{
    traceIn(MeshEntity::Destroy);

    int i;

    DestroyObject(shadowDecal);
    DestroyObject(compositeShadow);

    lightmap.FreeData();

    for(i=0; i<MeshLights.Num(); i++)
        MeshLights[i].light->RemoveEntity(this, MeshLights[i].side);

    for(i=0; i<MaterialList.Num(); i++)
        ReleaseMaterial(MaterialList[i]);
    MaterialList.Clear();

    if(mesh)
        RM->ReleaseMesh(mesh);

    Super::Destroy();

    traceOut;
}

void MeshEntity::SetMesh(CTSTR lpMesh)
{
    traceIn(MeshEntity::SetMesh);

    if(!mesh)
    {
        mesh = RM->GetMesh(lpMesh);

        if(!MaterialList.Num())
        {
            MaterialList.SetSize(mesh->DefaultMaterialList.Num());
            for(int i=0; i<MaterialList.Num(); i++)
            {
                String resName = mesh->DefaultMaterialList[i].name; //converting from utf to wide
                MaterialList[i] = GetMaterial(String(resName));
            }
        }
    }

    if(bCastCompositeShadow && !bStaticGeometry && level->UsesLightmaps())
    {
        compositeShadow = CreateFrameBuffer(64, 64, GS_RGBA, FALSE);
        shadowDecal = CreateObjectParam(ShadowDecal, this);
        shadowDecal->texture = compositeShadow;
        shadowDecal->bRenderable = FALSE;
        shadowDecal->SetPos(mesh->bounds.GetCenter());
        shadowDecal->shadowRot = AxisAngle(1.0f, 0.0f, 0.0f, RAD(-80.0f));
        shadowDecal->shadowRot *= AxisAngle(0.0f, 1.0f, 0.0f, RAD(-10.0f));
        shadowDecal->decalColor.Set(1.0f, 1.0f, 1.0f, 0.75f);
        shadowDecal->Attach(this);
    }

    VertBuffer = mesh->VertBuffer;
    VBData *vbd = VertBuffer->GetData();
    VertList = vbd->VertList.Array();

    FaceNormalList = NULL;

    bounds = mesh->bounds;

    traceOut;
}

void MeshEntity::OnUpdatePosition()
{
    traceInFast(MeshEntity::OnUpdatePosition);

    Super::OnUpdatePosition();

    for(int i=0; i<MeshLights.Num(); i++)
        MeshLights[i].light->RemoveEntity(this, MeshLights[i].side);
    MeshLights.Clear();

    List<LevelObject*> objects;
    level->GetObjects(bounds.GetTransformedBounds(transform), objects);
    for(int i=0; i<objects.Num(); i++)
    {
        LevelObject *levelObj = objects[i];

        if(levelObj->type == ObjectType_Entity)
        {
            if(!levelObj->ent->IsOf(GetClass(Light)))
                continue;

            Light *light = (Light*)levelObj->ent;
            light->ProcessEntity(this);
        }
    }

    if(bStaticGeometry)
        return;

    Vect position = GetWorldPos()+adjustPos;
    Quat rotation = GetWorldRot()*adjustRot;

    Matrix transform;
    transform.SetIdentity();
    transform.Rotate(rotation.GetInv());
    transform.Translate(-position);

    SH.Clear();
    level->GetSHIndirectLightSamples(GetWorldPos(), indirectLightDist, transform, SH);

    if(shadowDecal)
    {
        Vect shadowRot = -shadowDecal->shadowRot.GetDirectionVector();
        Vect color(0.0f);

        if(SH.Num())
        {
            color += Vect(SH[0]) * shadowRot.x;
            color += Vect(SH[1]) * shadowRot.y;
            color += Vect(SH[2]) * shadowRot.z;
            color += Vect(SH[3]) * fabsf(shadowRot.x);
            color += Vect(SH[4]) * fabsf(shadowRot.y);
            color += Vect(SH[5]) * fabsf(shadowRot.z);
        }
        else
            color = 0.3f;
        shadowDecal->decalColor = Vect4(color);
        shadowDecal->decalColor.w = (color.x+color.y+color.z)/3.0f;
        shadowDecal->decalColor.w = MIN(shadowDecal->decalColor.w*0.5f, 1.0f);
        shadowDecal->decalColor.w = 1.0f-(shadowDecal->decalColor.w*0.8f);
    }

    traceOutFast;
}

void MeshEntity::Render()
{
}

void MeshEntity::WorldRender()
{
    traceInFast(MeshEntity::WorldRender);

    if(!mesh)
        return;

    LoadEffectData();

    //render mesh
    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(mesh->IdxBuffer);

    for(DWORD i=0;i<mesh->nSections;i++)
    {
        DrawSection &section  = mesh->SectionList[i];
        Material    *material = MaterialList[i];

        if(material && material->effect == GetActiveEffect())
        {
            material->LoadParameters();

            if(material->flags & MATERIAL_TWOSIDED)
            {
                GSCullMode cullMode;
                cullMode = GetCullMode();
                SetCullMode(GS_NEITHER);
                Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
                SetCullMode(cullMode);
            }
            else
                Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
        }
    }

    ResetEffectData();

    traceOutFast;
}

void MeshEntity::RenderInitialPass()
{
    traceInFast(MeshEntity::RenderInitialPass);

    profileSegment("MeshEntity::RenderInitialPass");

    if(!mesh)
        return;

    LoadEffectData();

    LoadVertexBuffer(VertBuffer);       
    LoadIndexBuffer(mesh->IdxBuffer);

    for(DWORD i=0;i<mesh->nSections;i++)
    {
        DrawSection &section  = mesh->SectionList[i];
        Material    *material = MaterialList[i];

        if(material && section.numFaces)
        {
            if(material->effect == GetActiveEffect())
            {
                material->LoadParameters();

                if(material->flags & MATERIAL_TWOSIDED)
                {
                    GSCullMode cullMode;
                    cullMode = GetCullMode();
                    SetCullMode(GS_NEITHER);
                    Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
                    SetCullMode(cullMode);
                }
                else
                    Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
            }
        }
    }

    ResetEffectData();

    traceOutFast;
}

void MeshEntity::RenderLightmaps()
{
    traceInFast(MeshEntity::RenderLightmaps);

    if(!mesh || !bLightmapped)
        return;

    LoadEffectData();

    LoadVertexBuffer(VertBuffer);       
    LoadIndexBuffer(mesh->IdxBuffer);

    HANDLE param[3];

    param[0] = level->GetLightmapU();//GetActiveEffect()->GetParameterByName(TEXT("lightmapU"));
    param[1] = level->GetLightmapV();//GetActiveEffect()->GetParameterByName(TEXT("lightmapV"));
    param[2] = level->GetLightmapW();//GetActiveEffect()->GetParameterByName(TEXT("lightmapW"));

    if(lightmap.X)
    {
        GetActiveEffect()->SetTexture(param[0], lightmap.X);
        GetActiveEffect()->SetTexture(param[1], lightmap.Y);
        GetActiveEffect()->SetTexture(param[2], lightmap.Z);
    }
    else
    {
        //actually it should never get here
        Texture *black = GetTexture(TEXT("Base:Default/black.tga"));
        GetActiveEffect()->SetTexture(param[0], black);
        GetActiveEffect()->SetTexture(param[1], black);
        GetActiveEffect()->SetTexture(param[2], black);
    }

    for(DWORD i=0;i<mesh->nSections;i++)
    {
        DrawSection &section  = mesh->SectionList[i];
        Material    *material = MaterialList[i];

        if(material && section.numFaces)
        {
            if(material->effect == GetActiveEffect())
            {
                material->LoadParameters();

                if(material->flags & MATERIAL_TWOSIDED)
                {
                    GSCullMode cullMode;
                    cullMode = GetCullMode();
                    SetCullMode(GS_NEITHER);
                    Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
                    SetCullMode(cullMode);
                }
                else
                    Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
            }
        }
    }

    ResetEffectData();

    traceOutFast;
}

void MeshEntity::QuickRender()
{
    traceInFast(MeshEntity::QuickRender);

    if(!mesh)
        return;

    if(GetActiveEffect())
        LoadEffectData();
    else
    {
        MatrixPush();
        MatrixMultiply(invTransform);
        if(bHasScale) MatrixScale(scale);
    }

    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(mesh->IdxBuffer);

    for(DWORD i=0;i<mesh->nSections;i++)
    {
        DrawSection &section  = mesh->SectionList[i];
        Material    *material = MaterialList[i];

        if(!material)
            continue;

        if(material->effect == GetActiveEffect())
            material->LoadParameters();

        if(material->flags & MATERIAL_TWOSIDED)
        {
            GSCullMode cullMode;
            cullMode = GetCullMode();
            SetCullMode(GS_NEITHER);
            GS->Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
            SetCullMode(cullMode);
        }
        else
            GS->Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
    }

    if(GetActiveEffect())
        ResetEffectData();
    else
        MatrixPop();

    traceOutFast;
}

void MeshEntity::RenderBare(BOOL bUseTransform)
{
    traceInFast(MeshEntity::RenderBare);

    if(!mesh)
        return;

    if(bUseTransform)
    {
        MatrixPush();
        MatrixMultiply(invTransform);
        if(bHasScale) MatrixScale(scale);
    }

    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(mesh->IdxBuffer);

    GS->DrawBare(GS_TRIANGLES, 0, 0, mesh->nFaces*3);

    LoadVertexBuffer(NULL);
    LoadIndexBuffer(NULL);

    if(bUseTransform)
        MatrixPop();

    traceOutFast;
}


void MeshEntity::ResetScale()
{
    traceIn(MeshEntity::ResetScale);

    if(!mesh)
    {
        AppWarning(TEXT("ResetScale must be called after MeshEntity::Init()"));
        return;
    }
    bounds = mesh->bounds;

    bHasScale = !scale.CloseTo(Vect(1.0f, 1.0f, 1.0f));
    if(bHasScale)
    {
        bounds.Min *= scale;
        bounds.Max *= scale;
    }
    else
        scale = 1.0f;

    traceOut;
}

void MeshEntity::SetScale(const Vect &scaleAmount)
{
    traceIn(MeshEntity::SetScale);

    scale = scaleAmount;
    ResetScale();

    traceOut;
}

void MeshEntity::RemoveLight(Light *light)
{
    traceInFast(MeshEntity::RemoveLight);

    for(int i=MeshLights.Num()-1; i>=0; i--)
    {
        if(MeshLights[i].light == light)
            MeshLights.Remove(i);
    }

    traceOutFast;
}



void MeshEntity::ResetTransform()
{
    traceInFast(MeshEntity::ResetTransform);

    Vect position = GetWorldPos()+adjustPos;
    Quat rotation = GetWorldRot()*adjustRot;

    transform.SetIdentity();
    transform.Translate(position);
    transform.Rotate(rotation);

    invTransform = transform;
    invTransform.Transpose();

    traceOutFast;
}

Entity *MeshEntity::DuplicateEntity()
{
    MeshEntity *ent = (MeshEntity*)this->GetObjectClass()->Create(FALSE);
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

void MeshEntity::RenderWireframe()
{
    traceInFast(MeshEntity::RenderWireframe);

    Shader *shader = GetCurrentVertexShader();
    if(shader)
        shader->SetColor(shader->GetParameter(1), wireframeColor | 0xFF000000);

    MatrixPush();
    MatrixMultiply(invTransform);

    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(mesh->WireframeBuffer);

    GS->DrawBare(GS_LINES);

    MatrixPop();

    traceOutFast;
}

BOOL MeshEntity::CanSelect(const Vect &rayOrig, const Vect &rayDir)
{
    traceIn(MeshEntity::CanSelect);

    if(!phyShape || !phyObject)
        return Super::CanSelect(rayOrig, rayDir);

    if(bounds.RayIntersects(rayOrig.GetTransformedPoint(invTransform), rayDir.GetTransformedVector(invTransform)))
        return GetRayCollision(rayOrig, rayDir);
    else
        return FALSE;

    traceOut;
}


void MeshEntity::SetMaterial(DWORD texture, Material *material)
{
    traceIn(MeshEntity::SetMaterial);

    assert(texture < MaterialList.Num());

    if(MaterialList[texture])
        ReleaseMaterial(MaterialList[texture]);

    MaterialList[texture] = (material) ? material : GetMaterial(String(mesh->DefaultMaterialList[texture].name));

    traceOut;
}


inline void MeshEntity::LoadEffectData()
{
    GetActiveEffect()->SetMatrix(GetActiveEffect()->GetWorld(), invTransform);

    if(bHasScale)
    {
        HANDLE hScaleVal = GetActiveEffect()->GetScale();
        if(hScaleVal)
            GetActiveEffect()->SetVector(hScaleVal, scale);
    }
}

inline void MeshEntity::ResetEffectData()
{
    if(bHasScale)
    {
        HANDLE hScaleVal = GetActiveEffect()->GetScale();
        if(hScaleVal)
            GetActiveEffect()->SetVector(hScaleVal, 1.0f, 1.0f, 1.0f);
    }
}

void MeshEntity::Serialize(Serializer &s)
{
    traceIn(MeshEntity::Serialize);

    Super::Serialize(s);

    s << bLightmapped;
    if(bLightmapped)
    {
        s << lightmap;
        level->bHasLightmaps = TRUE;
    }

    traceOut;
}

void MeshEntity::MeshCollisionCallback(PhyObject *collider, int triIndex, float appliedImpulse, int lifeTime, const Vect &hitPos, float &friction, float &restitution)
{
    traceIn(MeshEntity::MeshCollisionCallback);

    for(int i=0; i<mesh->nSections; i++)
    {
        DrawSection &section = mesh->SectionList[i];
        if(triIndex < (section.startFace+section.numFaces))
        {
            Material *material = MaterialList[i];
            if(material)
            {
                if(lifeTime == 1)
                    material->ProcessCollision(phyObject, collider, appliedImpulse, hitPos);
                friction = material->GetFriction();
                restitution = material->GetRestitution();
            }
            return;
        }
    }

    traceOut;
}

Serializer& operator<<(Serializer &s, Lightmap &lm)
{
    traceIn(MeshEntity::operator<<);

    DWORD dwSize, dwWidth, dwHeight, dwFormat;
    void *lpTexData;

    if(s.IsLoading())
    {
        s << dwSize;
        if(dwSize)
        {
            s << dwWidth << dwHeight << dwFormat;
            lpTexData = Allocate(dwSize);
            s.Serialize(lpTexData, dwSize);
            lm.plain = CreateTexture(dwWidth, dwHeight, dwFormat, lpTexData, FALSE);
            Free(lpTexData);
        }

        s << dwSize;
        if(dwSize)
        {
            s << dwWidth << dwHeight << dwFormat;
            lpTexData = Allocate(dwSize);
            s.Serialize(lpTexData, dwSize);
            lm.X = CreateTexture(dwWidth, dwHeight, dwFormat, lpTexData, FALSE);
            Free(lpTexData);
        }

        s << dwSize;
        if(dwSize)
        {
            s << dwWidth << dwHeight << dwFormat;
            lpTexData = Allocate(dwSize);
            s.Serialize(lpTexData, dwSize);
            lm.Y = CreateTexture(dwWidth, dwHeight, dwFormat, lpTexData, FALSE);
            Free(lpTexData);
        }

        s << dwSize;
        if(dwSize)
        {
            s << dwWidth << dwHeight << dwFormat;
            lpTexData = Allocate(dwSize);
            s.Serialize(lpTexData, dwSize);
            lm.Z = CreateTexture(dwWidth, dwHeight, dwFormat, lpTexData, FALSE);
            Free(lpTexData);
        }
    }
    else
    {
        dwSize = 0;
        if(!lm.plain)
            s << dwSize;
        else
        {
            dwSize   = lm.plain->GetSize(); dwWidth  = lm.plain->Width();
            dwHeight = lm.plain->Height();  dwFormat = lm.plain->GetFormat();

            s << dwSize << dwWidth << dwHeight << dwFormat;
            lpTexData = lm.plain->GetImage(TRUE);
            s.Serialize(lpTexData, dwSize);
            Free(lpTexData);
        }

        dwSize = 0;
        if(!lm.X)
            s << dwSize;
        else
        {
            dwSize   = lm.X->GetSize(); dwWidth  = lm.X->Width();
            dwHeight = lm.X->Height();  dwFormat = lm.X->GetFormat();

            s << dwSize << dwWidth << dwHeight << dwFormat;
            lpTexData = lm.X->GetImage(TRUE);
            s.Serialize(lpTexData, dwSize);
            Free(lpTexData);
        }

        dwSize = 0;
        if(!lm.Y)
            s << dwSize;
        else
        {
            dwSize   = lm.Y->GetSize(); dwWidth  = lm.Y->Width();
            dwHeight = lm.Y->Height();  dwFormat = lm.Y->GetFormat();

            s << dwSize << dwWidth << dwHeight << dwFormat;
            lpTexData = lm.Y->GetImage(TRUE);
            s.Serialize(lpTexData, dwSize);
            Free(lpTexData);
        }

        dwSize = 0;
        if(!lm.Z)
            s << dwSize;
        else
        {
            dwSize   = lm.Z->GetSize(); dwWidth  = lm.Z->Width();
            dwHeight = lm.Z->Height();  dwFormat = lm.Z->GetFormat();

            s << dwSize << dwWidth << dwHeight << dwFormat;
            lpTexData = lm.Z->GetImage(TRUE);
            s.Serialize(lpTexData, dwSize);
            Free(lpTexData);
        }
    }

    s << lm.ILSamples;

    return s;

    traceOut;
}

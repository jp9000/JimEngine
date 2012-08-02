/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Level.cpp:  3D Scene

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


DefineAbstractClass(Level);


Level *level;

UINT Level::entityIDCounter = 0;


/*=========================================================
  Level Loading function

      Manages loading and unloading of levels, detecting
    the level format and setting up the classes accordingly
==========================================================*/

BOOL ENGINEAPI LoadLevel(CTSTR lpLevel)
{
    traceIn(LoadLevel);

    String fileName;
    Engine::ConvertResourceName(lpLevel, TEXT("levels"), fileName);

    XFile levelFile;
    if(levelFile.Open(fileName, XFILE_READ, XFILE_OPENEXISTING))
    {
        DWORD levelFormat;
        levelFile.Read(&levelFormat, 4);
        levelFile.Close();

        Class *targetClass;

        if(levelFormat == '\0lox')      //outdoor level
            targetClass = GetClass(OutdoorLevel);
        else if(levelFormat == '\0lix') //indoor level
            targetClass = GetClass(IndoorLevel);
        else if(levelFormat == '\0ltx') //oct level
            targetClass = GetClass(OctLevel);

        if(level)
        {
            /*if(level->IsOf(targetClass))
            {
                if(level->IsLoaded())
                    level->Unload();
            }
            else
            {*/
                DestroyObject(level);
                level = (Level*)targetClass->Create();
            //}
        }
        else
            level = (Level*)targetClass->Create();

        level->InitializeObject();
        if(!level->Load(lpLevel))
        {
            DestroyObject(level);
            level = NULL;
            return FALSE;
        }

        engine->ResetTime();

        return TRUE;
    }
    else
        return FALSE;

    traceOut;
}



/*=========================================================
    Level Base Class
==========================================================*/


Level::Level()
: curCutscene(0xFFFFFFFF)
{
    traceIn(Level::Level);

    floatTextureFormat = !GS->Use32FTextures() ? GS_RGBA16F : GS_RGBA32F;

    zero(matScaleTrans, 64);
    matScaleTrans[12] = matScaleTrans[13] = matScaleTrans[14] =
    matScaleTrans[10] = matScaleTrans[0] = matScaleTrans[5] = 0.5f;
    matScaleTrans[15] = 1.0f;

    bUsingTwoSidedStencil = GS->SupportsTwoSidedStencil();

    CreateAttMap(FALSE);

    int gdWidth, gdHeight;
    GS->GetSize(gdWidth, gdHeight);

    DWORD width  = ((gdWidth/4)+1)&0xFFFE;
    DWORD height = ((gdHeight/4)+1)&0xFFFE;

    GlareRenderTexture  = CreateFrameBuffer(width, height, floatTextureFormat);
    GlareRenderTexture2 = CreateFrameBuffer(width, height, floatTextureFormat);
    GlareRenderTexture3 = CreateFrameBuffer(width, height, floatTextureFormat);

    MainRenderTexture   = CreateFrameBuffer(gdWidth, gdHeight, floatTextureFormat);
    MainRenderTexture2  = CreateFrameBuffer(gdWidth, gdHeight, floatTextureFormat);

    shadowEffect = GS->CreateEffectFromFile(TEXT("data/Base/InternalEffects/depthShadowyGoodness.effect"));
    postProcessEffect = GS->CreateEffectFromFile(TEXT("data/Base/InternalEffects/PostProcess.effect"));

    defaultMaterial = CreateObject(Material);
    defaultMaterial->effect = GetEffect(TEXT("Base:Bump.effect"));

    MaterialParameter parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("diffuseTexture"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:default/TEST.dds"));
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("normalMap"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:default/TEST_bm.dds"));
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("specularMap"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:default/white.tga"), FALSE);
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("illuminationMap"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:default/white.tga"), FALSE);
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("specularLevel"));
    parameter.type = Parameter_Vector3;
    *(float*)parameter.data = 0.0f;
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("specularPower"));
    parameter.type = Parameter_Float;
    *(float*)parameter.data = 32.0f;
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("illuminationColor"));
    parameter.type = Parameter_Vector3;
    zero((Vect*)parameter.data, sizeof(Vect));
    defaultMaterial->Params << parameter;

    EnableProfiling(TRUE);

    traceOut;
}

Level::~Level()
{
    traceIn(Level::~Level);

    EnableProfiling(FALSE);

    //------------------------------------
    // Free Other stuff
    DestroyObject(GlareRenderTexture);
    DestroyObject(GlareRenderTexture2);
    DestroyObject(GlareRenderTexture3);

    DestroyObject(MainRenderTexture);
    DestroyObject(MainRenderTexture2);

    DestroyObject(TempRenderTexture);
    DestroyObject(TempZStencilBuffer);

    if(AttenuationMap)
        DestroyObject(AttenuationMap);

    DestroyObject(defaultMaterial);

    DestroyObject(shadowEffect);
    DestroyObject(postProcessEffect);

    RM->Clear();

    for(int i=0; i<LevelModules.Num(); i++)
        UnloadGameModule(LevelModules[i]);

    LevelModules.Clear();

    traceOut;
}

void Level::Destroy()
{
    traceIn(Level::Destroy);

    this->Unload();

    traceOut;
}

void Level::PreFrame()
{
    traceIn(Level::PreFrame);

    if(!bLoaded || !bWindowActive)
        return;

    int gdWidth, gdHeight;
    GS->GetSize(gdWidth, gdHeight);

    DWORD width  = ((gdWidth/4)+1)&0xFFFE;
    DWORD height = ((gdHeight/4)+1)&0xFFFE;

    if( !MainRenderTexture ||
        (MainRenderTexture->Height() != gdHeight) ||
        (MainRenderTexture->Width()  != gdWidth) )
    {
        delete GlareRenderTexture;
        delete GlareRenderTexture2;
        delete GlareRenderTexture3;

        delete MainRenderTexture;
        delete MainRenderTexture2;

        GlareRenderTexture3 = CreateFrameBuffer(width, height, floatTextureFormat);
        GlareRenderTexture2 = CreateFrameBuffer(width, height, floatTextureFormat);
        GlareRenderTexture  = CreateFrameBuffer(width, height, floatTextureFormat);

        MainRenderTexture   = CreateFrameBuffer(gdWidth, gdHeight, floatTextureFormat);
        MainRenderTexture2  = CreateFrameBuffer(gdWidth, gdHeight, floatTextureFormat);
    }

    if( !TempRenderTexture ||
        (TempRenderTexture->Height() != TempRenderHeight) ||
        (TempRenderTexture->Width()  != TempRenderWidth)  )
    {
        delete TempRenderTexture;
        delete TempZStencilBuffer;

        TempRenderTexture = CreateFrameBuffer(TempRenderWidth, TempRenderHeight, floatTextureFormat);
        TempZStencilBuffer  = CreateZStencilBuffer(TempRenderWidth, TempRenderHeight);
    }

    /*GS->SetViewport(0, 0, width, height);

    SetFrameBufferTarget(GlareRenderTexture);
    ClearColorBuffer(TRUE);
    SetFrameBufferTarget(GlareRenderTexture2);
    ClearColorBuffer(TRUE);

    GS->SetViewport(0, 0, TempRenderWidth, TempRenderHeight);

    SetFrameBufferTarget(TempRenderTexture);
    ClearColorBuffer(TRUE);
    SetFrameBufferTarget(NULL);*/

    GS->SetViewport(0, 0, gdWidth, gdHeight);

    if(GS->MustRebuildRenderTargets())
    {
        for(int i=0; i<Light::LightList.Num(); i++)
        {
            Light *light = Light::LightList[i];

            if(light->IsOf(GetClass(PointLight)))
            {
                PointLight* pointLight = (PointLight*)light;
                pointLight->updateFaces = 0x3F;
            }
        }
    }

    traceOut;
}

void CallEntityPositionalUpdate(Entity *ent);

void CallEntityPositionalUpdate(Entity *ent)
{
    ent->OnUpdatePosition();

    for(int i=0; i<ent->NumChildren(); i++)
        CallEntityPositionalUpdate(ent->GetChild(i));
}


void Level::ArrangeEntities()
{
    traceIn(Level::ArrangeEntities);
    profileSingularSegment("Entity Placement");

    Entity *ent;

    if(bUpdateAllEntities)
    {
        ent = Entity::FirstEntity();
        while(ent)
        {
            if(!ent->bMarkedForDestruction)
                CalculateEntityPosition(ent);
            ent = ent->NextEntity();
        }

        ent = Entity::FirstEntity();
        while(ent)
        {
            if(!ent->bMarkedForDestruction)
                CallEntityPositionalUpdate(ent);
            ent = ent->NextEntity();
        }

        bUpdateAllEntities = FALSE;
    }
    else
    {
        ent = Entity::pFirstUpdatingEntity;
        while(ent)
        {
            if(!ent->bMarkedForDestruction)
                CalculateEntityPosition(ent);

            ent = ent->pNextUpdatingEntity;
        }

        ent = Entity::pFirstUpdatingEntity;
        while(ent)
        {
            if(!ent->bMarkedForDestruction)
                CallEntityPositionalUpdate(ent);

            ent = ent->pNextUpdatingEntity;
        }
    }

    //clear updates
    ent = Entity::pFirstUpdatingEntity;
    while(ent)
    {
        Entity *nextEnt = ent->pNextUpdatingEntity;
        ent->pPrevUpdatingEntity = ent->pNextUpdatingEntity = NULL;
        ent->bInUpdateList = FALSE;
        ent = nextEnt;
    }

    Entity::pFirstUpdatingEntity = Entity::pLastUpdatingEntity = NULL;

    traceOut;
}


void Level::CalculateEntityPosition(Entity *ent)
{
    traceInFast(Level::CalculateEntityPosition);

    BOOL isLight = ent->IsOf(GetClass(Light));
    BOOL isMeshEnt = ent->IsOf(GetClass(MeshEntity));
    int i;
    static List<Matrix> curTransform;
    static List<Quat>   curRotation;

    BOOL bInside=0;

    if(!curTransform.Num())
    {
        curTransform.CreateNew();
        curTransform[0].SetIdentity();
        curRotation.CreateNew();
        curRotation[0].SetIdentity();
    }
    else
    {
        curTransform.Insert(0, curTransform[0]);
        curRotation.Insert(0, curRotation[0]);
    }

    if(ent->bCalculateLocalPosition)
    {
        if(ent->GetParent())
        {
            Matrix invTransform = curTransform[0].GetTranspose();

            ent->Pos = ent->worldPos.GetTransformedPoint(invTransform);
            ent->Rot = ent->worldRot*curRotation[0].GetInv();
        }
        else
        {
            ent->Pos = ent->worldPos;
            ent->Rot = ent->worldRot;
        }

        curTransform[0].SetIdentity();
        curTransform[0] *= ent->worldPos;
        curTransform[0] *= ent->worldRot;
        curRotation[0]   = ent->worldRot;

        ent->bCalculateLocalPosition = FALSE;
    }
    else
    {
        curTransform[0] *= ent->Pos;
        curTransform[0] *= ent->Rot;
        curRotation[0]  *= ent->Rot;

        ent->worldPos = curTransform[0].GetTranspose().T;
        ent->worldRot = curRotation[0];
    }

    //-------------------------------------------------------------------------------

    if(ent->UpdatingPosition())
    {
        if(curTransform.Num() > 1)
        {
            curTransform[0] = curTransform[1];
            curTransform[0] *= ent->Pos;
            curTransform[0] *= ent->Rot;

            curRotation[0]  = curRotation[1];
            curRotation[0]  *= ent->Rot;

            ent->worldPos = curTransform[0].GetTranspose().T;
            ent->worldRot = curRotation[0];
        }
        else
        {
            curTransform[0].SetIdentity();
            curTransform[0] *= ent->Pos;
            curTransform[0] *= ent->Rot;

            curRotation[0]  = ent->Rot;

            ent->worldPos = ent->Pos;
            ent->worldRot = ent->Rot;
        }
    }

    if(ent->bPositionalOnly)
    {
        for(i=0; i<ent->NumChildren(); i++)
            CalculateEntityPosition(ent->GetChild(i));

        curTransform.Remove(0);
        curRotation.Remove(0);

        ent->bHasMoved = FALSE;

        return;
    }

    //-------------------------------------------------------------------------------

    Matrix transform;

    if(isMeshEnt)
    {
        MeshEntity *meshEnt = (MeshEntity*)ent;
        meshEnt->ResetTransform();

        transform = meshEnt->GetTransform();
    }
    else
        transform = curTransform[0];

    //-------------------------------------------------------------------------------

    UpdateEntityPositionData(ent, transform);

    for(i=0; i<ent->NumChildren(); i++)
        CalculateEntityPosition(ent->GetChild(i));

    curTransform.Remove(0);
    curRotation.Remove(0);

    ent->bHasMoved = FALSE;

    traceOutFast;
}


void Level::BeginCutscene(int cutscene, BOOL bLoop, CSCALLBACK callback, LPVOID lpParam)
{
}

void Level::EndCutscene()
{
}

void Level::Tick(float fSeconds)
{
}

BOOL Level::LoadLevelModule(CTSTR lpModule)
{
    traceIn(Level::LoadLevelModule);

    if(IsModuleLoaded(lpModule))
        return TRUE;
    else
    {
        if(!LoadGameModule(lpModule))
            return FALSE;

        *level->LevelModules.CreateNew() = lpModule;
        return TRUE;
    }

    traceOut;
}

void Level::UnloadLevelModule(CTSTR lpModule)
{
    traceIn(Level::UnloadLevelModule);

    UnloadGameModule(lpModule);

    for(int i=0; i<LevelModules.Num(); i++)
    {
        if(LevelModules[i].CompareI(lpModule))
        {
            LevelModules[i].Clear();
            LevelModules.Remove(i);
            return;
        }
    }

    traceOut;
}

void Level::GetLoadedModules(StringList &ModulesOut)
{
    ModulesOut.CopyList(LevelModules);
}



void Brush::ClearFogTable()
{
    if(FogTable)
    {
        delete FogTable;
        FogTable = NULL;
    }
}

void Brush::Clear()
{
    traceIn(Brush::Clear);

    int i;

    DestroyObject(phyObj);
    DestroyObject(meshShape);

    delete FogTable;

    lightmap.FreeData();
    bLightmapped = FALSE;

    for(i=0; i<Materials.Num(); i++)
    {
        if(Materials[i])
            RM->ReleaseMaterial(Materials[i]);
    }
    Materials.Clear();
    Free(SectionList);
    Free(EdgeList);

    delete WireframeBuffer;

    delete VertBuffer;
    delete IdxBuffer;

    zero(this, sizeof(Brush));

    traceOut;
}

void Brush::Serialize(Serializer &s)
{
    traceIn(Brush::Serialize);

    DWORD i;

    if(s.IsLoading())
        Clear();

    i=0;

    s << nVerts
      << nFaces
      << nEdges
      << nSections
      << i; //padding for prev versions

    VBData *vbd;

    if(s.IsLoading())
        vbd = new VBData;
    else
        vbd = VertBuffer->GetData();

    vbd->Serialize(s);

    if(s.IsLoading())
    {
        VertList = vbd->VertList.Array();
        TangentList = vbd->TangentList.Array();
        UVList = (UVCoord*)vbd->TVList[0].Array();
        VertBuffer = CreateVertexBuffer(vbd);

        if(nFaces)
            FaceList = (Face*)Allocate(sizeof(Face)*nFaces);
        if(nEdges)
            EdgeList = (Edge*)Allocate(sizeof(Edge)*nEdges);
        if(nSections)
            SectionList = (DrawSection*)Allocate(sizeof(DrawSection)*nSections);
    }

    if(nFaces)
    {
        s.Serialize(FaceList, sizeof(Face)*nFaces);

        if(s.IsLoading())
            IdxBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, FaceList, nFaces*3);
    }

    if(nEdges)
        s.Serialize(EdgeList, sizeof(Edge)*nEdges);

    if(s.IsLoading())
    {
        if(engine->InEditor())
        {
            DWORD *wireframeIndices = (DWORD*)Allocate(nEdges*2*sizeof(DWORD));

            for(i=0; i<nEdges; i++)
            {
                DWORD curPos = i*2;
                wireframeIndices[curPos]   = EdgeList[i].v1;
                wireframeIndices[curPos+1] = EdgeList[i].v2;
            }

            WireframeBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, wireframeIndices, nEdges*2);
        }
    }

    if(nSections)
        s.Serialize(SectionList, sizeof(DrawSection)*nSections);

    if(s.IsLoading())
    {
        Materials.SetSize(nSections);
        for(i=0; i<Materials.Num(); i++)
        {
            String strMaterial;
            s << strMaterial;

            if(!strMaterial.IsEmpty())
                Materials[i] = RM->GetMaterial(strMaterial);
        }
    }
    else
    {
        for(i=0; i<Materials.Num(); i++)
            s << RM->GetMaterialName(Materials[i]);
    }

    DWORD bla;
    s << bla;

    s << dwFogType
      << fogIntensity
      << fogOpacity
      << FogColor
      << fogDistance
      << AmbientColor;

    /*if(s.IsLoading())
    {
        if(dwFogType)
        {
            float lpFogTable[512];
            CreateFogTable(dwFogType, fogIntensity, fogOpacity, lpFogTable);
            FogTable = CreateTexture(512, 1, GS_R32F, lpFogTable, FALSE);
        }
    }*/

    s << bounds;

    if(s.IsLoading())
    {
        meshShape = physics->MakeStaticMeshShape(VertList, nVerts, (UINT*)FaceList, nFaces*3);
        phyObj    = physics->CreateStaticObject(meshShape, Vect(0.0f, 0.0f, 0.0f), Quat::Identity());
        phyObj->EnableCollisionCallback(TRUE);
        phyObj->SetBrushOwner(this);
    }

    s << bLightmapped;
    if(bLightmapped)
    {
        s << lightmap;

        level->bHasLightmaps = TRUE;
    }

    /*LightmapList.SetSize(nLightmaps);

    for(j=0; j<brush.nLightmaps; j++)
    {
        WORD width, height;

        LevelFile.Read(&width,  2);
        LevelFile.Read(&height, 2);

        DWORD dwSize = width*height*3;

        BOOL bUseLightmap;
        LevelFile.Read(&bUseLightmap, 4);

        if(bUseLightmap)
        {
            LPBYTE lpLightmap = (LPBYTE)Allocate(dwSize);

            LevelFile.Read(lpLightmap, dwSize);
            brush.LightmapList[j].textures[0] = CreateTexture(width, height, GS_RGB, lpLightmap, FALSE);

            LevelFile.Read(lpLightmap, dwSize);
            brush.LightmapList[j].textures[1] = CreateTexture(width, height, GS_RGB, lpLightmap, FALSE);

            LevelFile.Read(lpLightmap, dwSize);
            brush.LightmapList[j].textures[2] = CreateTexture(width, height, GS_RGB, lpLightmap, FALSE);

            Free(lpLightmap);
        }
    }*/

    traceOut;
}


void ENGINEAPI CreateFogTable(DWORD FogType, float density, float opacity, float* lpTable)
{
    assert(lpTable);

    if(FogType == FOG_EXP)
    {
        for(int i=0; i<512; i++)
        {
            float curValue = float(i)/51.2f;
            curValue = expf(-(density*curValue));

            if(curValue > 1.0f)
                curValue = 1.0f;
            else if(curValue < 0.0f)
                curValue = 0.0f;

            curValue = 1.0f-((1.0f-curValue)*opacity);

            lpTable[i] = curValue;
        }
    }
    else if(FogType == FOG_EXP2)
    {
        for(int i=0; i<512; i++)
        {
            float curValue = float(i)/51.2f;
            curValue = expf(-powf(density*curValue, 2));

            if(curValue > 1.0f)
                curValue = 1.0f;
            else if(curValue < 0.0f)
                curValue = 0.0f;

            curValue = 1.0f-((1.0f-curValue)*opacity);

            lpTable[i] = curValue;
        }
    }
    else if(FogType == FOG_LINEAR)
    {
        for(int i=0; i<512; i++)
        {
            float curValue = float(i)/512.0f;
            curValue = (1.0f - curValue) / (1.0f - density);

            if(curValue > 1.0f)
                curValue = 1.0f;
            else if(curValue < 0.0f)
                curValue = 0.0f;

            curValue = 1.0f-((1.0f-curValue)*opacity);

            lpTable[i] = curValue;
        }
    }
}


void Brush::BrushCollisionCallback(PhyObject *collider, int triIndex, float appliedImpulse, int lifeTime, const Vect &hitPos, float &friction, float &restitution)
{
    traceIn(Brush::BrushCollisionCallback);

    for(int i=0; i<nSections; i++)
    {
        DrawSection &section = SectionList[i];
        if(triIndex < (section.startFace+section.numFaces))
        {
            Material *material = Materials[i];
            if(material)
            {
                if(lifeTime == 1)
                    material->ProcessCollision(phyObj, collider, appliedImpulse, hitPos);
                friction = material->GetFriction();
                restitution = material->GetRestitution();
            }
            return;
        }
    }

    traceOut;
}

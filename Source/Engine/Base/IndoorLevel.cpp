/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  IndoorLevel.cpp:  Indoor Level

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


DefineClass(IndoorLevel);


BOOL IndoorLevel::Load(CTSTR lpName)
{
    traceIn(IndoorLevel::Load);

    assert(lpName);

    XFileInputSerializer levelData;

    //--------------------------------------------------

    DWORD dwSigniture, dwEditorOffset;
    DWORD i;
    WORD wVersion;

    Log(TEXT("Loading Indoor Level: %s"), lpName);

    Engine::ConvertResourceName(lpName, TEXT("levels"), strName);
    if(!levelData.Open(strName))
    {
        AppWarning(TEXT("Unable to open the file: \"%s\""), lpName);
        return FALSE;
    }

    levelData << dwSigniture;
    levelData << wVersion;
    levelData << dwEditorOffset;

    if(dwSigniture != '\0lix')
    {
        AppWarning(TEXT("'%s' Bad Level file"), lpName);
        return FALSE;
    }

    if(wVersion != INDOOR_VERSION)
    {
        AppWarning(TEXT("'%s' outdated file format"), lpName);
        return FALSE;
    }

    //--------------------------------------------------

    DWORD nModules, nObjects, nPVSs, nPortals, nBrushes, nCinematics;

    levelData << nModules;
    levelData << nPVSs;
    levelData << nPortals;
    levelData << nBrushes;
    levelData << nObjects;
    levelData << nCinematics;
    levelData << LightmapTechnique;
    levelData << bGlobalFog;
    levelData << dwGlobalFogColor;

    //--------------------------------------------------

    LevelModules.SetSize(nModules);
    for(i=0; i<nModules; i++)
    {
        String &strModuleName = LevelModules[i];

        levelData << strModuleName;
        LoadGameModule(strModuleName);
    }

    //--------------------------------------------------

    for(i=0; i<nObjects; i++)
        Entity::LoadEntity(levelData);

    //--------------------------------------------------

    CinematicList.SetSize(nCinematics);
    for(i=0; i<CinematicList.Num(); i++)
        levelData << CinematicList[i].Keys;

    //--------------------------------------------------

    PortalList.SetSize(nPortals);
    for(i=0; i<nPortals; i++)
        PortalList[i].Serialize(levelData);

    //--------------------------------------------------

    BrushList.SetSize(nBrushes);
    for(i=0; i<nBrushes; i++)
    {
        BrushList[i].Serialize(levelData);
        /*
        brush.LightmapList.SetSize(brush.nLightmaps);

        if(LightmapTechnique == LIGHTMAPPING_DIFFUSEBUMP)
        {
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
            }
        }*/
    }

    //--------------------------------------------------

    PVSList.SetSize(nPVSs);
    for(i=0; i<nPVSs; i++)
        PVSList[i].Serialize(levelData);

    //--------------------------------------------------

    levelData << entityIDCounter;

    levelData.Close();

    bLoaded = TRUE;

    return TRUE;

    traceOut;
}


void IndoorLevel::Unload()
{
    traceIn(IndoorLevel::Unload);

    int i;

    DestroyEntities();

    //------------------------------------
    // Free PVS Brushes
    for(i=0; i<PVSList.Num(); i++)
        PVSList[i].Clear();
    PVSList.Clear();

    //------------------------------------
    // Free Detail Brushes
    for(i=0; i<BrushList.Num(); i++)
        BrushList[i].Clear();
    BrushList.Clear();

    //------------------------------------
    // Free Cinematic List
    for(i=0; i<CinematicList.Num(); i++)
        CinematicList[i].Keys.Clear();
    CinematicList.Clear();

    //------------------------------------
    // Free Portal List
    for(i=0; i<PortalList.Num(); i++)
        PortalList[i].FreeBuffers();
    PortalList.Clear();

    //------------------------------------
    // Free Material List

    bLoaded = 0;

    traceOut;
}


void IndoorLevel::PreFrame()
{
    traceIn(IndoorLevel::PreFrame);

    GS->GetSize(TempRenderWidth, TempRenderHeight);

    Super::PreFrame();

    traceOut;
}


void IndoorLevel::AddEntity(Entity *ent)
{
    traceInFast(IndoorLevel::AddEntity);

    GetEntityData(ent) = CreateObject(IndoorEntityData);

    traceOutFast;
}

void IndoorLevel::RemoveEntity(Entity *ent, BOOL bChildren)
{
    traceInFast(IndoorLevel::RemoveEntity);

    if(!GetEntityData(ent))
        return;

    int i;
    BOOL isLight = ent->IsOf(GetClass(Light));
    BOOL isMeshEnt = ent->IsOf(GetClass(MeshEntity));

    IndoorEntityData *entData = GetIndoorEntityData(ent);

    if(entData->PVSRefs.Num() || entData->VisPVSRefs.Num())
    {
        if(isLight)
        {
            for(i=0; i<entData->PVSRefs.Num(); i++)
                PVSList[entData->PVSRefs[i]].lights.RemoveItem(static_cast<Light*>(ent));

            for(i=0; i<entData->VisPVSRefs.Num(); i++)
                PVSList[entData->VisPVSRefs[i]].visLights.RemoveItem(static_cast<Light*>(ent));
        }
        else
        {
            for(i=0; i<entData->PVSRefs.Num(); i++)
                PVSList[entData->PVSRefs[i]].entities.RemoveItem(ent);

            if(isMeshEnt)
            {
                for(i=0; i<entData->VisPVSRefs.Num(); i++)
                    PVSList[entData->VisPVSRefs[i]].visMeshEntities.RemoveItem((MeshEntity*)ent);
            }
            else
            {
                for(i=0; i<entData->VisPVSRefs.Num(); i++)
                    PVSList[entData->VisPVSRefs[i]].visEntities.RemoveItem(ent);
            }
        }
    }

    DestroyObject(GetEntityData(ent));
    GetEntityData(ent) = NULL;

    if(bChildren)
    {
        for(i=0; i<ent->NumChildren(); i++)
            RemoveEntity(ent->GetChild(i));
    }

    traceOutFast;
}

void IndoorLevel::UpdateEntityPositionData(Entity *ent, const Matrix &transform)
{
    traceInFast(IndoorLevel::UpdateEntityPositionData);

    BOOL isLight = ent->IsOf(GetClass(Light));
    BOOL isMeshEnt = ent->IsOf(GetClass(MeshEntity));

    IndoorEntityData *entData = GetIndoorEntityData(ent);

    traceOutFast;
}

void IndoorLevel::GetStaticGeometry(const Bounds &bounds, List<Brush*> &Brushes, List<MeshEntity*> &StaticMeshes)
{
}

void IndoorLevel::GetObjects(const Bounds &bounds, List<LevelObject*> &objects)
{
}

void IndoorLevel::GetObjects(const Vect &eyePos, const ViewClip &clip, List<LevelObject*> &objects)
{
}


void IndoorLevel::GetVisiblePVSs(PVS &curPVS, PVS *lpLastPVS, const Vect &eye, ViewClip *lpClip, List<PVS*> &VisPVSList)
{
    traceInFast(IndoorLevel::GetVisiblePVSs);

    assert(lpClip);
    DWORD i,j;
    BOOL bExists = 0, bBadPVS = 0;
    static List<PVS*> PVSStack;

    for(i=0; i<PVSStack.Num(); i++)
    {
        if(PVSStack[i] == &curPVS)
        {
            bBadPVS = 1;
            break;
        }
    }
    if(bBadPVS)
        return;

    PVSStack.Insert(0, &curPVS);

    for(j=0; j<VisPVSList.Num(); j++)
    {
        if(VisPVSList[j] == &curPVS)
        {
            bExists = 1;
            break;
        }
    }

    if(!bExists)
    {
        VisPVSList << &curPVS;
        curPVS.bVisible = 1;
    }

    for(i=0; i<curPVS.BrushRefs.Num(); i++)
    {
        Brush *brush = &BrushList[curPVS.BrushRefs[i]];
        if(!brush->bVisible)
            brush->bVisible = lpClip->BoundsVisible(brush->bounds);
    }

    for(i=0; i<curPVS.visMeshEntities.Num(); i++)
    {
        MeshEntity *ment = curPVS.visMeshEntities[i];
        if(!EntityVisible(ment))
        {
            Matrix      mObjectInverse;
            ViewClip    entClip;

            entClip = *lpClip;
            entClip.Transform(ment->invTransform);

            EntityVisible(ment) = entClip.BoundsVisible(ment->bounds);
        }
    }

    for(i=0; i<curPVS.visLights.Num(); i++)
    {
        Light *light = curPVS.visLights[i];

        if(!EntityVisible(light))
        {
            if(light->IsOf(GetClass(DirectionalLight)))
                EntityVisible(light) = TRUE;
            else
                EntityVisible(light) = lpClip->SphereVisible(light->GetWorldPos(), static_cast<PointLight*>(light)->lightRange);
        }
    }

    for(i=0; i<curPVS.visEntities.Num(); i++)
    {
        Entity *ent = curPVS.visEntities[i];
        EntityVisible(ent) = TRUE;
    }

    for(i=0; i<curPVS.PortalRefs.Num(); i++)
    {
        Portal &portal  = PortalList[curPVS.PortalRefs[i]];
        PVS &nextPVS = PortalPVS(curPVS, portal);

        if(&nextPVS == lpLastPVS) continue;

        DWORD test      = lpClip->BoundsTest(portal.bounds);

        if(portal.bounds.PointInside(eye))
            test = BOUNDS_PARTIAL;

        if(test & BOUNDS_OUTSIDE)
            continue;
        else if(test == BOUNDS_PARTIAL)
            GetVisiblePVSs(nextPVS, &curPVS, eye, lpClip, VisPVSList);
        else    //BOUNDS_INSIDE|BOUNDS_PARTIAL
        {
            ViewClip nextClip;

            nextClip = *lpClip;
            nextClip.TruncateWithBounds(eye, portal.bounds);

            GetVisiblePVSs(nextPVS, &curPVS, eye, &nextClip, VisPVSList);
        }
    }

    PVSStack.Remove(0);

    traceOutFast;
}

void IndoorLevel::GetLightDrawInfo(DWORD curPVSID, DWORD lastPVSID, Light *light, ViewClip *lpClip)
{
    traceInFast(IndoorLevel::GetLightDrawInfo);

    static List<DWORD> PVSStack;

    if(PVSStack.HasValue(curPVSID))
        return;

    DWORD i;
    IndoorEntityData *entData = GetIndoorEntityData(light);
    BOOL bAdd = 1, bBadPVS = 0;
    BOOL bDirectional = light->IsOf(GetClass(DirectionalLight));
    PVS &curPVS = PVSList[curPVSID];

    PVSStack.Insert(0, curPVSID);

    entData->VisPVSRefs.SafeAdd(curPVSID);

    if(lpClip)
    {
        for(i=0; i<curPVS.PortalRefs.Num(); i++)
        {
            Portal &portal  = PortalList[curPVS.PortalRefs[i]];
            DWORD nextPVSID = PortalPVSRef(curPVSID, portal);

            if(nextPVSID == lastPVSID) continue;

            PVS &nextPVS = PVSList[nextPVSID];
            if(!bDirectional && !nextPVS.bounds.SphereIntersects(light->GetWorldPos(), static_cast<PointLight*>(light)->lightRange))
                continue;

            DWORD test      = lpClip->BoundsTest(portal.bounds);

            if(portal.bounds.PointInside(light->GetWorldPos()))
                test = BOUNDS_PARTIAL;

            if(test & BOUNDS_OUTSIDE)
                continue;
            else if(test == BOUNDS_PARTIAL)
                GetLightDrawInfo(nextPVSID, curPVSID, light, lpClip);
            else    //BOUNDS_INSIDE|BOUNDS_PARTIAL
            {
                ViewClip nextClip;

                nextClip = *lpClip;
                nextClip.TruncateWithBounds(light->GetWorldPos(), portal.bounds);

                GetLightDrawInfo(nextPVSID, curPVSID, light, &nextClip);
            }
                    
        }
    }
    else
    {
        for(i=0; i<curPVS.PortalRefs.Num(); i++)
        {
            Portal &portal  = PortalList[curPVS.PortalRefs[i]];
            BOOL bContinue  = 0;
            DWORD nextPVSID = PortalPVSRef(curPVSID, portal);

            if(entData->VisPVSRefs.HasValue(nextPVSID))
                continue;

            PVS &nextPVS = PVSList[nextPVSID];
            if(!bDirectional && !nextPVS.bounds.SphereIntersects(light->GetWorldPos(), static_cast<PointLight*>(light)->lightRange))
                continue;

            if(portal.bounds.PointInside(light->GetWorldPos()))
                GetLightDrawInfo(nextPVSID, curPVSID, light, NULL);
            else
            {
                ViewClip nextClip;
                nextClip.CreateFromBounds(light->GetWorldPos(), portal.bounds);

                GetLightDrawInfo(nextPVSID, curPVSID, light, &nextClip);
            }
        }
    }

    PVSStack.Remove(0);

    traceOutFast;
}

void PVS::RemoveEntities()
{
    traceIn(PVS::RemoveEntities);

    IndoorLevel *indoorLevel = (IndoorLevel*)level;
    int i;

    DWORD num = entities.Num();
    for(i=0; i<num; i++)
    {
        entities[0]->UpdatePositionalData();
        indoorLevel->RemoveEntity(entities[0]);
    }

    num = lights.Num();
    for(i=0; i<num; i++)
    {
        lights[0]->UpdatePositionalData();
        indoorLevel->RemoveEntity(lights[0]);
    }

    traceOut;
}

void PVS::Clear()
{
    traceIn(PVS::Clear);

    entities.Clear();
    visMeshEntities.Clear();
    visEntities.Clear();
    lights.Clear();
    visLights.Clear();

    BrushRefs.Clear();
    PortalRefs.Clear();

    Brush::Clear();

    traceOut;
}


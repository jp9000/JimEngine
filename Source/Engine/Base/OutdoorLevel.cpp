/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  OutdoorLevel.cpp:  Outdoor Level

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


DefineClass(OutdoorLevel);


OutdoorLevel::OutdoorLevel()
{
    /*DWORD i;
    DWORD x, y;

    //-------------------------------------------------
    // Generate Skydome

    float domeSize = 2000.0f;
    float positionAdjust = domeSize*cos(RAD(22.5f));

    Vect polar, cart;
    polar.z = domeSize;

    VBData *skydomeData = new VBData;

    skydomeData->VertList.SetSize(161);

    LPWORD indices;

    for(y=0; y<5; y++)
    {
        DWORD yPos = y*32;
        
        if(y < 4)
            indices = (LPWORD)Allocate(33*2*sizeof(WORD));

        for(x=0; x<32; x++)
        {
            polar.x = RAD(22.5f-(float(y)*(22.5f/5.0f)));
            polar.y = RAD(float(x)*11.25f);

            cart = PolarToCart(polar);
            cart.y -= positionAdjust;

            skydomeData->VertList[yPos+x] = cart;

            if(y < 4)
            {
                DWORD x2 = (x*2);
                indices[x2  ] = yPos+x+32;
                indices[x2+1] = yPos+x;
            }
        }

        if(y < 4)
        {
            DWORD x2 = (x*2);
            indices[x2  ] = yPos+32;
            indices[x2+1] = yPos;
            skydomeIBs[y] = CreateIndexBuffer(GS_UNSIGNED_SHORT, indices, 33*2);
        }
    }

    indices = (LPDWORD)Allocate((32+2)*sizeof(DWORD));

    indices[0] = 160;

    for(i=0; i<32; i++)
        indices[i+1] = 128+i;

    indices[33] = 128;

    skydomeIBs[4] = CreateIndexBuffer(GS_UNSIGNED_LONG, indices, 32+2);

    skydomeData->VertList[160].Set(0.0f, domeSize-positionAdjust, 0.0f);
    skydomeVB = CreateVertexBuffer(skydomeData);

    //-------------------------------------------------
    // Water polygon
    for(y=0; y<6; y++)
    {
        float zVal  = (float(y)  *1000.0f)-3000.0f;
        float zVal2 = (float(y+1)*1000.0f)-3000.0f;

        VBData *waterData = new VBData;

        for(x=0; x<=6; x++)
        {
            float xVal = (float(x)*1000.0f)-3000.0f;

            Vect newVect;

            waterData->VertList << newVect.Set(xVal, 0.0f, zVal);
            waterData->VertList << newVect.Set(xVal, 0.0f, zVal2);
        }

        waterVBs[y] = CreateVertexBuffer(waterData);
    }

    //-------------------------------------------------
    // Load LOD indices

    for(DWORD lodLevel=0; lodLevel<3; lodLevel++)
    {
        for(DWORD type=0; type<16; type++)
        {
            String strFile;
            XFile lodFile;

            strFile << TEXT("data/Base/terrainLOD/") << (int) lodLevel << TEXT("/") << (int)type << TEXT(".lod");

            if(lodFile.Open(strFile, XFILE_READ, XFILE_OPENEXISTING))
            {
                WORD numBuffers;
                lodFile.Read(&numBuffers, 2);

                TerrainLOD[lodLevel][type].DrawSegments.SetSize(numBuffers);

                for(i=0; i<numBuffers; i++)
                {
                    WORD drawType, numIndices;
                    lodFile.Read(&drawType, 2);
                    lodFile.Read(&numIndices, 2);

                    indices = (LPWORD)Allocate(numIndices*sizeof(WORD));

                    lodFile.Read(indices, numIndices*sizeof(WORD));

                    TerrainLOD[lodLevel][type].DrawSegments[i] = CreateIndexBuffer(GS_UNSIGNED_SHORT, indices, numIndices);
                }

                lodFile.Close();
            }
        }
    }

    XFile lodFile;

    if(lodFile.Open(TEXT("data/Base/terrainLOD/Lowest.lod"), XFILE_READ, XFILE_OPENEXISTING))
    {
        WORD numBuffers;
        lodFile.Read(&numBuffers, 2);

        LowestLOD.DrawSegments.SetSize(numBuffers);

        for(i=0; i<numBuffers; i++)
        {
            WORD drawType, numIndices;
            lodFile.Read(&drawType, 2);
            lodFile.Read(&numIndices, 2);

            indices = (LPWORD)Allocate(numIndices*sizeof(WORD));

            lodFile.Read(indices, numIndices*sizeof(WORD));

            LowestLOD.DrawSegments[i] = CreateIndexBuffer(GS_UNSIGNED_SHORT, indices, numIndices);
        }

        lodFile.Close();
    }

    //-------------------------------------------------
    // Generate block face data

    Faces.SetSize(512);
    i = 0;

    for(y=0; y<16; y++)
    {
        DWORD y1 = y*17;
        DWORD y2 = y1+17;

        for(x=0; x<16; x++)
        {
            DWORD vert1 = y1+x;
            DWORD vert2 = vert1+1;
            DWORD vert3 = y2+x;
            DWORD vert4 = vert3+1;

            Face &face1 = Faces[i++];
            Face &face2 = Faces[i++];

            face1.A = vert1;
            face1.B = vert3;
            face1.C = vert2;

            face2.A = vert2;
            face2.B = vert3;
            face2.C = vert4;
        }
    }

    //-------------------------------------------------

    sunLight = CreateObject(DirectionalLight);
    sunLight->SetColor(0xFFFFFF);
    sunLight->bCastShadows = TRUE;
    sunLight->Rot.Set(1.0f, 0.0f, 0.0f, -60.0f).MakeQuat();

    moonLight = CreateObject(DirectionalLight);
    moonLight->SetColor(0xFFFFFF);
    moonLight->bCastShadows = TRUE;
    moonLight->Rot.Set(1.0f, 0.0f, 0.0f, -60.0f).MakeQuat();

    sunFlare = CreateObject(LenseFlare);

    //-------------------------------------------------

    sunTex = GetTexture(TEXT("Base:Default/sunTex.tga"), FALSE);
    whiteTex = GetTexture(TEXT("Base:Default/white.tga"), FALSE);

    //-------------------------------------------------

    fVisibilityDistance = 600.0f;
    fLODFactor = 400.0f;
    fDaySpeed = 0.0f;

    dwWaterLOD = 1;

    fWaterIntensity = 6.0f;
    Ripple1Speed.Set(0.02f, 0.04f);
    Ripple2Speed.Set(-0.03f, -0.02f);

    curTimeOfDay  = 1700.0f;

    terrainEffect = GS->CreateEffectFromFile(TEXT("data/Base/terrainEffects/Default.fx"));*/
}


OutdoorLevel::~OutdoorLevel()
{
    DWORD i, j, k;

    delete terrainEffect;

    ClearFogTable();

    //------------------------------------
    // Free Base Face List
    Faces.Clear();

    //------------------------------------
    // Free Water polygon
    for(i=0; i<6; i++)
    {
        delete waterVBs[i];
        waterVBs[i] = NULL;
    }

    //------------------------------------
    // Free Skydome
    delete skydomeVB;
    skydomeVB = NULL;
    for(i=0; i<10; i++)
    {
        delete skydomeIBs[i];
        skydomeIBs[i] = NULL;
    }

    //------------------------------------
    // Free Terrain LOD Index Buffers
    for(i=0; i<3; i++)
    {
        for(j=0; j<16; j++)
        {
            BlockDetail &LOD = TerrainLOD[i][j];
            for(k=0; k<LOD.DrawSegments.Num(); k++)
                delete LOD.DrawSegments[k];
            LOD.DrawSegments.Clear();
        }
    }

    for(k=0; k<LowestLOD.DrawSegments.Num(); k++)
        delete LowestLOD.DrawSegments[k];
    LowestLOD.DrawSegments.Clear();
}


BOOL OutdoorLevel::Load(CTSTR lpName)
{
    return FALSE;
}

void OutdoorLevel::Unload()
{
    /*int i,j;

    Entity::DestroyAll();

    //------------------------------------
    // Free Weather Data
    for(i=0; i<WeatherList.Num(); i++)
    {
        WeatherData &weather = WeatherList[i];

        weather.weatherName.Clear();
        if(weather.CloudTexture)
            ReleaseTexture(weather.CloudTexture);
        delete weather.AmbientSound;
        if(weather.PrecipTexture)
            ReleaseTexture(weather.PrecipTexture);
    }

    //------------------------------------
    // Free Moon Data
    Moons.Clear();

    //------------------------------------
    // Free Quad Tree
    QuadTree.Clear();

    //------------------------------------
    // Free Terrain Blocks
    for(i=0; i<TerrainBlocks.Num(); i++)
    {
        TerrainBlock &block = TerrainBlocks[i];
        block.BrushRefs.Clear();
        block.entities.Clear();
        block.lights.Clear();
        block.visEntities.Clear();
        block.visLights.Clear();
        block.visMeshEntities.Clear();
        block.PlaneList.Clear();
        block.LeafBounds.Clear();
        delete block.VertBuffer;
    }
    TerrainBlocks.Clear();

    BlockQuadTree.Clear();

    //------------------------------------
    // Free Detail Brushes
    for(i=0; i<BrushList.Num(); i++)
    {
        AddBrush *brush = &BrushList[i];

        delete brush->FogTable;

        for(j=0; j<brush->nLightmaps; j++)
        {
            delete brush->LightmapList[j].colorTexture;
            delete brush->LightmapList[j].vecTexture;
        }
        brush->LightmapList.Clear();

        brush->Materials.Clear();
        Free(brush->SectionList);
        Free(brush->LightmapSectionList);
        Free(brush->EdgeList);

        delete brush->WireframeBuffer;

        delete brush->VertBuffer;
        delete brush->IdxBuffer;
    }
    BrushList.Clear();

    //------------------------------------
    // Free Cinematic List
    for(i=0; i<CinematicList.Num(); i++)
        CinematicList[i].Keys.Clear();
    CinematicList.Clear();

    //------------------------------------

    bLoaded = 0;*/
}


void OutdoorLevel::PreFrame()
{
    int gdWidth, gdHeight;
    GS->GetSize(gdWidth, gdHeight);

    TempRenderWidth = (gdWidth/dwWaterLOD);
    TempRenderHeight = (gdHeight/dwWaterLOD);

    TempRenderWidth  &= 0xFFFFFFFE;
    TempRenderHeight &= 0xFFFFFFFE;

    Super::PreFrame();
}


void OutdoorLevel::AddEntity(Entity *ent)
{
    GetEntityData(ent) = CreateObject(OutdoorEntityData);
}

void OutdoorLevel::RemoveEntity(Entity *ent, BOOL bChildren)
{
    if(!GetEntityData(ent))
        return;

    int i;
    BOOL isLight = ent->IsOf(GetClass(Light));
    BOOL isMeshEnt = ent->IsOf(GetClass(MeshEntity));

    OutdoorEntityData *entData = GetOutdoorEntityData(ent);

    if(entData->Blocks.Num() || entData->VisBlocks.Num())
    {
        if(isLight)
        {
            for(i=0; i<entData->Blocks.Num(); i++)
                entData->Blocks[i]->lights.RemoveItem(static_cast<Light*>(ent));

            for(i=0; i<entData->VisBlocks.Num(); i++)
                entData->VisBlocks[i]->visLights.RemoveItem(static_cast<Light*>(ent));
        }
        else
        {
            for(i=0; i<entData->Blocks.Num(); i++)
                entData->Blocks[i]->entities.RemoveItem(ent);

            if(isMeshEnt)
            {
                for(i=0; i<entData->VisBlocks.Num(); i++)
                    entData->VisBlocks[i]->visMeshEntities.RemoveItem(static_cast<MeshEntity*>(ent));
            }
            else
            {
                for(i=0; i<entData->VisBlocks.Num(); i++)
                    entData->VisBlocks[i]->visEntities.RemoveItem(ent);
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
}

void OutdoorLevel::UpdateEntityPositionData(Entity *ent, const Matrix &transform)
{
    BOOL isLight = ent->IsOf(GetClass(Light));
    BOOL isMeshEnt = ent->IsOf(GetClass(MeshEntity));
    OutdoorEntityData *entData = GetOutdoorEntityData(ent);
    int i;

    //-------------------------------------------------------------------------------

    if(isMeshEnt)
    {
        MeshEntity *meshEnt = (MeshEntity*)ent;
        meshEnt->ResetTransform();
    }

    Bounds entBounds = ent->GetBounds();
    for(i=0; i<TerrainBlocks.Num(); i++)
    {
        TerrainBlock &block = TerrainBlocks[i];
        if(block.bounds.Intersects(entBounds))
        {
            if(isLight)
            {
                Light *light = static_cast<Light*>(ent);
                block.lights << light;
            }
            else
            {
                if(!isMeshEnt && ent->bRenderable)
                {
                    block.visEntities << ent;
                    entData->VisBlocks << &block;
                }
                block.entities << ent;
            }
            entData->Blocks << &block;
        }

        if(isMeshEnt && ent->bRenderable)
        {
            MeshEntity *meshEnt = (MeshEntity*)ent;

            meshEnt->ResetTransform();

            if(block.bounds.IntersectsOBB(meshEnt->bounds, meshEnt->transform))
            {
                block.visMeshEntities << meshEnt;
                entData->VisBlocks << &block;
            }
        }
    }


    //recalculate light visibility data
    if(isLight)
    {
        Light *light = (Light*)ent;

        if(!light->IsOff() && !(light->color&0xFFFFFF))
        {
            for(i=0; i<TerrainBlocks.Num(); i++)
            {
                TerrainBlock &block = TerrainBlocks[i];
                if(light->IsOf(GetClass(SpotLight)))
                {
                    SpotLight *spot = static_cast<SpotLight*>(light);
                    ViewClip clip;
                    Matrix rotMatrix = spot->GetEntityInvTransform();

                    clip.planes.Clear();
                    clip.SetPerspective(spot->cutoff+1.0f, 1.0f, 1.0, 4096.0f);
                    clip.Transform(rotMatrix.GetTranspose());

                    if(clip.BoundsVisible(block.bounds) && block.bounds.SphereIntersects(spot->GetWorldPos(), spot->lightRange))
                    {
                        entData->VisBlocks << &block;
                        block.visLights << light;
                    }
                }
                else if(light->IsOf(GetClass(DirectionalLight)))
                {
                    entData->VisBlocks << &block;
                    block.visLights << light;
                }
                else
                {
                    PointLight *point = static_cast<PointLight*>(light);

                    if(block.bounds.SphereIntersects(point->GetWorldPos(), point->lightRange))
                    {
                        entData->VisBlocks << &block;
                        block.visLights << light;
                    }
                }
            }
        }
    }
}

void OutdoorLevel::GetVisibleBlocks(const Vect &eye, const ViewClip &clip, List<TerrainBlock*> &VisBlockList, DWORD curNode)
{
}


void OutdoorLevel::CalculateLOD(const Vect &camPos, float detailAdjust)
{
    DWORD i;
    Vect adjCamPos = camPos;
    adjCamPos.y = 0.0f;

    for(i=0; i<TerrainBlocks.Num(); i++)
    {
        TerrainBlock &block = TerrainBlocks[i];

        Vect blockPos = block.bounds.GetCenter();
        blockPos.y = 0.0f;

        float dist = blockPos.Dist(adjCamPos);

        block.curLOD = int(dist / detailAdjust);
    }

    for(i=0; i<TerrainBlocks.Num(); i++)
    {
        TerrainBlock &block = TerrainBlocks[i];

        block.LODFlags = 0;

        DWORD firstRow = rowCount-1;
        DWORD lastRow = (rowCount*rowCount)-rowCount;

        if( (i % rowCount)     && (TerrainBlocks[i-1].curLOD > block.curLOD))
            block.LODFlags |= LOD_LEFTOPEN;
        if( ((i+1) % rowCount) && (TerrainBlocks[i+1].curLOD > block.curLOD))
            block.LODFlags |= LOD_RIGHTOPEN;
        if( (i > firstRow)     && (TerrainBlocks[i-rowCount].curLOD > block.curLOD))
            block.LODFlags |= LOD_TOPOPEN;
        if( (i < lastRow)      && (TerrainBlocks[i+rowCount].curLOD > block.curLOD))
            block.LODFlags |= LOD_BOTTOMOPEN;
    }
}


void OutdoorLevel::ClearFogTable()
{
    if(FogTable)
    {
        delete FogTable;
        FogTable = NULL;
    }
}


void GetQChildBounds(const Bounds &bounds, int childnum, Bounds &child)
{
    Vect center = bounds.GetCenter();

    child.Max.y = bounds.Max.y;
    child.Min.y = bounds.Min.y;

    if(childnum > 1)    {child.Min.z = center.z;     child.Max.z = bounds.Max.z;  childnum -= 2;}
    else                {child.Min.z = bounds.Min.z; child.Max.z = center.z;}

    if(childnum == 1)   {child.Min.x = center.x;     child.Max.x = bounds.Max.x;}
    else                {child.Min.x = bounds.Min.x; child.Max.x = center.x;}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  OutdoorLevel.h:  Outdoor Level

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


#ifndef OUTDOORLEVEL_HEADER
#define OUTDOORLEVEL_HEADER


//-----------------------------------------
// Terrain Block Detail
struct BlockDetail
{
    List<IndexBuffer*> DrawSegments;

    //wireframe
    List<IndexBuffer*> WireframeSegments;
};


//-----------------------------------------
// Terrain Block
#define LOD_LEFTOPEN    1
#define LOD_RIGHTOPEN   2
#define LOD_TOPOPEN     4
#define LOD_BOTTOMOPEN  8


//-----------------------------------------
// Terrain Segment
struct TerrainBlock
{
    Vect                Pos;
    Bounds              bounds;

    BOOL                bVisible;

    TerrainBlock        *block;
    List<Entity*>       entities;
    List<Entity*>       visEntities;
    List<MeshEntity*>   visMeshEntities;
    List<Light*>        lights;
    List<Light*>        visLights;
    List<DWORD>         BrushRefs;

    List<Plane>         PlaneList;

    //------------------------------------------------

    VertexBuffer        *VertBuffer;

    int                 curLOD;
    int                 LODFlags;

    void Render();
    void QuickRender();
    void RenderInitialPass();
    void RenderWireframe();

    //------------------------------------------------

    BOOL InSolidSpace(const Vect &pos, DWORD curNodeID=0);
    BOOL GetCollision(const Vect &curPos, Vect &nextPos, Plane &collisionPlane, DWORD curNodeID=0);
};

//-----------------------------------------
// Weather Data
struct WeatherData
{
    String  weatherName;

    Color3  SkyColorDawn;
    Color3  SkyColorDay;
    Color3  SkyColorDusk;
    Color3  SkyColorNight;

    Color3  AmbientColorDawn;
    Color3  AmbientColorDay;
    Color3  AmbientColorDusk;
    Color3  AmbientColorNight;

    Color3  SunColorDawn;
    Color3  SunColorDay;
    Color3  SunColorDusk;
    Color3  MoonShineColor;

    BOOL    bHasGlare;

    DWORD   FogType;
    Color3  FogColorDawn;
    Color3  FogColorDay;
    Color3  FogColorDusk;
    Color3  FogColorNight;
    float   fFogDepth;

    float   fTransitionSpeed;

    float   fWindSpeed;

    float   fCloudSpeed;
    Texture *CloudTexture;

    Sound   *AmbientSound;

    BOOL    bHasPrecip;
    float   fPrecipMass;
    float   fPrecipDensity;
    float   fPrecipSize;
    Texture *PrecipTexture;
};

//-----------------------------------------
// Moon Data
struct MoonData
{
    Texture *MoonTexture;
    float fSize, fLatitude, fLongitude, curPos;
};


/*=========================================================
    OutdoorEntityData
==========================================================*/

class OutdoorEntityData : public LevelData
{
public:
    List<TerrainBlock*>     Blocks,VisBlocks;

protected:
    virtual ~OutdoorEntityData()
    {
        Blocks.Clear();
        VisBlocks.Clear();
    }
};

#define GetOutdoorEntityData(ent) static_cast<OutdoorEntityData*>(GetEntityData(ent))


/*=========================================================
    OutdoorLevel
==========================================================*/

//-----------------------------------------
// Terrain Level
class BASE_EXPORT OutdoorLevel : public Level
{
    DeclareClass(OutdoorLevel, Level);

    friend struct       Brush;
    friend struct       AddRef;
    friend struct       TerrainBlock;
    friend class        MeshEntity;
    friend class        PointLight;
    friend class        SpotLight;
    friend class        DirectionalLight;

protected:
    virtual void UpdateEntityPositionData(Entity *ent, const Matrix &transform);

    void GetVisibleBlocks(const Vect &eye, const ViewClip &clip, List<TerrainBlock*> &VisBlockList, DWORD curNode=0);

    void CalculateLOD(const Vect &camPos, float detailAdjust);

    BOOL                bHasWater;
    Plane               waterPlane;
    float               waterHeight;

    VertexBuffer        *waterVBs[6];

    VertexBuffer        *skydomeVB;
    IndexBuffer         *skydomeIBs[10];

    LenseFlare          *sunFlare;

    virtual void CalculateRenderOrder(Camera *camera, const ViewClip &clip) {}

    void DrawSkydome();
    void DrawWater();

    void ClearFogTable();

public:
    OutdoorLevel();
    ~OutdoorLevel();

    virtual void PreFrame();

    virtual BOOL Load(CTSTR lpName);
    virtual void Unload();

    virtual void AddEntity(Entity *ent);
    virtual void RemoveEntity(Entity *ent, BOOL bChildren=TRUE);

    virtual AIPath* GetEntityPath(Entity *ent, const Vect &targetPos) {return NULL;}

    virtual void GetStaticGeometry(const Bounds &bounds, List<Brush*> &Brushes, List<MeshEntity*> &StaticMeshes) {}

    virtual void GetObjects(const Bounds &bounds, List<LevelObject*> &objects) {}
    virtual void GetObjects(const Vect &eyePos, const ViewClip &clip, List<LevelObject*> &objects) {}

    virtual void Tick(float fSeconds);


    void SetWeather(TSTR lpWeather, BOOL bInstant=FALSE);


    ViewClip            fullClip;

    DWORD               rowCount;
    List<TerrainBlock>  TerrainBlocks;
    List<Brush>         BrushList;

    DWORD               terrainMaterial;

    float               blockSize;

    List<Face>          Faces;
    List<Plane>         PlaneList;

    Effect              *terrainEffect;

    Texture             *sunTex;
    Texture             *whiteTex;

    //Moons
    List<MoonData>      Moons;

    //Water
    BOOL                bOmitReflectiveWater;

    float               fWaterIntensity;
    Vect2               Ripple1Speed;
    Vect2               Ripple2Speed;
    Vect2               Ripple1Value;
    Vect2               Ripple2Value;

    //visibility and LOD detail
    BlockDetail         TerrainLOD[3][16];
    BlockDetail         LowestLOD;
    float               fLODFactor;
    float               fVisibilityDistance;
    BOOL                bLimitVisDistance;
    DWORD               dwWaterLOD;

    //Outdoor Lighting
    DirectionalLight    *sunLight;
    DirectionalLight    *moonLight;

    //Outdoor Fog
    Texture             *FogTable;

    //Outdoor Weather
    List<WeatherData>   WeatherList;
    WeatherData         *curWeather;
    WeatherData         *newWeather;
    Color3              curSkyColor;
    Color3              curAmbientColor;
    Color3              curSunColor;
    Color3              curFogColor;
    float               fCurCloudOffset;
    BOOL                bTransitioningWeather;
    float               fWeatherTransition;
    float               fSunVisibility;

    Matrix              skyOriantation;

    float               fDaySpeed;
    float               curTimeOfDay;
};


BASE_EXPORT void GetQChildBounds(const Bounds &bounds, int childnum, Bounds &child);


#endif

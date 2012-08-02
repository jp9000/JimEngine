/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Level.h:  3D Scene management

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


#ifndef LEVEL_HEADER
#define LEVEL_HEADER


#define OUTDOOR_VERSION 0x100
#define OCTLEVEL_VERSION 0x140
#define INDOOR_VERSION 0x230


class Light;
class PointLight;
class SpotLight;
class DirectionalLight;

typedef void (ENGINEAPI *CSCALLBACK)(LPVOID lpParam);


BASE_EXPORT BOOL ENGINEAPI LoadLevel(CTSTR lpLevel);


//-----------------------------------------
// Cinematics
struct CinKey
{
    DWORD dwTime;

    Vect pos, posTan;
    Quat rot, rotTan;
};

struct Cinematic
{
    List<CinKey> Keys;
};


//-----------------------------------------
// Brush
struct BASE_EXPORT Brush
{
    DWORD           nVerts;
    DWORD           nFaces;
    DWORD           nEdges;
    DWORD           nSections;

    //rendering data
    VertexBuffer    *VertBuffer;
    Vect            *VertList;
    UVCoord         *UVList;
    Vect            *TangentList;
    Edge            *EdgeList;
    IndexBuffer     *IdxBuffer;
    Face            *FaceList;
    DrawSection     *SectionList;
    List<Material*> Materials;

    Bounds          bounds;

    BOOL            bVisible;
    BOOL            bAlreadyUsed;
    BOOL            bLightmapped;

    List<Light*>    Lights;

    //wireframe
    IndexBuffer     *WireframeBuffer;

    //fog
    Texture         *FogTable;

    DWORD           dwFogType;
    float           fogIntensity, fogOpacity;

    float           fogDistance;
    DWORD           FogColor;

    //ambient light
    DWORD           AmbientColor;

    //lightmaps
    Lightmap        lightmap;

    PhyStaticMesh   *meshShape;
    PhyObject       *phyObj;

    void Render();
    void RenderInitialPass();
    void RenderLightmaps();
    //void RenderIllumination();
    void QuickRender();
    void RenderRefractive();
    void RenderWireframe();

    void BrushCollisionCallback(PhyObject *collider, int triIndex, float appliedImpulse, int lifeTime, const Vect &hitPos, float &friction, float &restitution);

    void RenderLight(Light *light);

    void ClearFogTable();

    void Clear();

    void Serialize(Serializer &s);
};


//-----------------------------------------
// LevelObject

enum ObjectType {ObjectType_Brush, ObjectType_Entity};

struct LevelObject
{
    LevelObject(Entity *ent, const Bounds &bounds) {type = ObjectType_Entity; this->bounds = bounds;        this->ent = ent;}
    LevelObject(Brush *brush)                      {type = ObjectType_Brush;  this->bounds = brush->bounds; this->brush = brush;}

    Bounds bounds;
    ObjectType type;
    union
    {
        Entity *ent;
        Brush  *brush;
    };
};


//-----------------------------------------
// MeshOrderInfo

struct MeshOrderItem
{
    const Mesh *mesh;
    List<MeshEntity*> RenderItems;

    inline void FreeData()
    {
        RenderItems.Clear();
    }
};

struct MeshOrderInfo
{
    List<MeshOrderItem> renderOrder;

    inline ~MeshOrderInfo()
    {
        for(int i=0; i<renderOrder.Num(); i++)
            renderOrder[i].FreeData();
    }

    inline void Sort()
    {
        int num = renderOrder.Num()-1;

        for(int i=0; i<num; i++)
        {
            MeshOrderItem &itemA = renderOrder[i];

            for(int j=i+1; j<renderOrder.Num(); j++)
            {
                MeshOrderItem &itemB = renderOrder[j];

                if(itemB.RenderItems.Num() > itemA.RenderItems.Num())
                    renderOrder.SwapValues(i, j);
            }
        }
    }

    inline void AddEntity(MeshEntity *ent)
    {
        MeshOrderItem *item = NULL;
        for(int i=0; i<renderOrder.Num(); i++)
        {
            MeshOrderItem *curItem = &renderOrder[i];
            if(curItem->mesh == ent->GetMesh())
            {
                item = curItem;
                break;
            }
        }

        if(!item)
        {
            item = renderOrder.CreateNew();
            item->mesh = ent->GetMesh();
        }

        item->RenderItems << ent;
    }
};


//-----------------------------------------
// LightShaderParams

struct LightShaderParams
{
    HANDLE lightPos;
    HANDLE lightDir;
    HANDLE lightRange;
    HANDLE lightColor;

    HANDLE attenuationMap;

    HANDLE spotlightMatrix;
    HANDLE spotlightMap;

    HANDLE spotDepthTexture;
    HANDLE pointDepthCube;
};


/*=========================================================
    Level Base Class
==========================================================*/

class BASE_EXPORT Level : public FrameObject
{
    DeclareClass(Level, FrameObject);

    friend struct Brush;
    friend struct AddBrush;
    friend class  Entity;
    friend class  MeshEntity;
    friend class  Engine;
    friend class  Game;
    friend class  EditorLevelInfo;
    friend class  EditorViewport;
    friend class  EditorEngine;

protected:
    static UINT         entityIDCounter;

    Material            *defaultMaterial;

    Texture             *AttenuationMap;
    Texture             *SpecularTableMap;

    Plane               curClipPlane;

    Texture             *MainRenderTexture;
    Texture             *MainRenderTexture2;

    Texture             *GlareRenderTexture;
    Texture             *GlareRenderTexture2;
    Texture             *GlareRenderTexture3;

    Texture             *TempRenderTexture;
    ZStencilBuffer      *TempZStencilBuffer;
    int                 TempRenderWidth;
    int                 TempRenderHeight;

    Plane               camPlane;

    DWORD               floatTextureFormat;

    BOOL                bUsingTwoSidedStencil;

    BOOL                bHasLightmaps;

    Effect              *curEffect;

    ViewClip            curClip;

    BOOL                bNoClip;

    BOOL                bLoaded;

    BOOL                bKillCollision;

    Camera              *renderCamera;
    Light               *curRenderLight;

    StringList          LevelModules;

    List<Cinematic>     CinematicList;
    Camera              *cutsceneCamera, *prevCamera;
    int                 curCutscene;
    DWORD               curKey,nextKey,keyTime;
    BOOL                bLoopCutscene;
    CSCALLBACK          callbackProc;

    BOOL                bRenderRecursion;

    float               spotlightMatrix[16];
    float               matScaleTrans[16];

    Effect              *shadowEffect;
    Effect              *postProcessEffect;

    float               curZValue;
    BOOL                bHideBumpmaps;

    List<UINT>          DestroyedUserObjects;

    String              strName;

    List<Brush*>            renderBrushes;
    List<MeshEntity*>       renderMeshes;
    List<AnimatedEntity*>   renderAnimatedMeshes;
    List<Entity*>           renderEntities;
    List<Projector*>        renderProjectors;
    List<LitDecal*>         renderLitDecals;
    List<DirectionalLight*> renderDirectionalLights;
    List<PointLight*>       renderPointLights;
    List<SpotLight*>        renderSpotLights;

    LightShaderParams   shaderParams;

    BOOL                bUpdateAllEntities;

    HANDLE              lightmapU, lightmapV, lightmapW;
    HANDLE              boneTransforms;

    void CreateAttMap(BOOL bQuadratic=FALSE);

    void RenderLights();

    virtual void QuickRender();
    virtual void FullRender(Light *lighty, BOOL bAnimation=FALSE);

    static inline LevelData*& GetEntityData(Entity *ent) {return ent->levelData;}

    static inline BOOL& EntityVisible(Entity *ent)       {return ent->bVisible;}
    static inline BOOL& AlreadyUsed(Entity *ent)         {return ent->bAlreadyUsed;}

    static inline void SetCurrentVisibleSets(DWORD num)  {engine->nPVSs = num;}

    static inline void DestroyEntities()                 {Entity::DestroyAll();}

    void CalculateEntityPosition(Entity *ent);

    void ArrangeEntities();

    void ClearRenderItems();

    //------------------------------------------------

    virtual void RenderFromLight(Light *light, ViewClip &clip, BOOL bAnimation=FALSE);
    virtual void CalculateRenderOrder(Camera *camera, const ViewClip &clip)=0;
    virtual void UpdateEntityPositionData(Entity *ent, const Matrix &transform)=0;

public:
    Level();
    virtual ~Level();

    virtual void Destroy();

    virtual void PreFrame();

    virtual void Tick(float fSeconds);

    virtual BOOL Load(CTSTR lpName)=0;
    virtual void Unload()=0;

    virtual void Draw(Camera *camera, BaseTexture *renderTarget=NULL, BOOL bRenderOnlyTexture=FALSE);
    virtual void DrawWireframe(Camera *camera);

    virtual void AddEntity(Entity *ent)=0;
    virtual void RemoveEntity(Entity *ent, BOOL bChildren=TRUE)=0;

    virtual AIPath* GetEntityPath(Entity *ent, const Vect &targetPos)=0;

    virtual void GetSHIndirectLightSamples(const Vect& pos, float radius, const Matrix &transform, List<CompactVect> &SH) {}

    virtual void GetStaticGeometry(const Bounds &bounds, List<Brush*> &Brushes, List<MeshEntity*> &StaticMeshes)=0;

    virtual void GetObjects(const Bounds &bounds, List<LevelObject*> &objects)=0;
    virtual void GetObjects(const Vect &eyePos, const ViewClip &clip, List<LevelObject*> &objects)=0;

    //------------------------------------------------

    BOOL LoadLevelModule(CTSTR lpModule);
    void UnloadLevelModule(CTSTR lpModule);
    void GetLoadedModules(StringList &ModulesOut);

    inline Camera* GetCurrentCamera() const  {return renderCamera;}
    inline BOOL UsingTwoSidedStencil() const {return bUsingTwoSidedStencil;}
    inline BOOL IsLoaded() const             {return bLoaded;}

    inline BOOL UsesLightmaps() const        {return bHasLightmaps;}

    inline void EnableTwoSidedStencil(BOOL bEnable) {bUsingTwoSidedStencil = bEnable ? GS->SupportsTwoSidedStencil() : 0;}

    inline String GetLevelName() const {return strName;}

    inline HANDLE GetLightmapU() const {return lightmapU;}
    inline HANDLE GetLightmapV() const {return lightmapV;}
    inline HANDLE GetLightmapW() const {return lightmapW;}

    inline HANDLE GetBoneTransforms() const {return boneTransforms;}

    void BeginCutscene(int cutscene, BOOL bLoop, CSCALLBACK callback, LPVOID lpParam);
    void EndCutscene();

    //<Script module="Base" classdecs="Level">
    Declare_Internal_Member(native_GetEntityPath);
    Declare_Internal_Member(native_GetCurrentCamera);
    //</Script>
};


//<Script module="Base" globaldecs="Level.xscript">
Declare_Native_Global(NativeGlobal_CreateBareLevel);
Declare_Native_Global(NativeGlobal_LoadLevel);
Declare_Native_Global(NativeGlobal_GetLevel);
//</Script>


BASE_EXPORT void ENGINEAPI CreateFogTable(DWORD FogType, float density, float opacity, float* lpTable);


BASE_EXPORT extern Level *level;


#endif

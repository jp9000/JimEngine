/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditorLevel.h

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


#pragma once


#define SCALE_U     1
#define SCALE_V     2


class ObjectCreator;


struct ShapeVert
{
    Vect2 pos;
    Vect2 t[2];

    BOOL bTangentsLinked;
    BOOL bSelected;
};


//--------------------------------------------------
// Main Edit Mode enum
enum EditMode
{
    EditMode_Modify,
    EditMode_Create,
    EditMode_Terrain
};

//--------------------------------------------------
// Modify Mode enum
enum ModifyMode
{
    ModifyMode_Select,
    ModifyMode_Move,
    ModifyMode_Rotate
};

//--------------------------------------------------
// Selection Mode enum
enum SelectMode
{
    SelectMode_Brushes,
    SelectMode_WorldPrefabs,
    SelectMode_ObjectsAndDetails,
    SelectMode_Textures
};


//--------------------------------------------------
// Lightmap stuff

enum LightingType
{
    LightingType_Simple,
    LightingType_Radiosity,
    LightingType_ImageBased
};

enum ShadowType
{
    ShadowType_None,
    ShadowType_Fast,
    ShadowType_Soft
};

struct LightmapRenderInfo
{
    BOOL bLighting;
    ShadowType shadowType;

    BOOL  bAmbientOcclusion;
    float fAmbientOcclusionOuterRadius;
    float fAmbientOcclusionInnerRadius;
    float fAmbientOcclusionExponent;
};

struct LightmapMeshInfo
{
    DWORD nPlanes, nFaces, nVerts;

    Plane           *Planes;
    Face            *Faces;
    DWORD           *FacePlanes;
    UVCoord         *LightmapCoords;
    Vect            *VertList;
    Vect            *NormalList;
    Vect            *TangentList;
};

struct LMLightInfo
{
    Light *light;
    PhyGhost *ghost;

    inline void FreeData()
    {
        if(ghost)
        {
            PhyShape *shape = ghost->GetShape();
            DestroyObject(ghost);
            DestroyObject(shape);
        }
    }
};

struct TexelInfo
{
    DWORD tri;
    Vect  pos;
    float bary[3];
};

struct TriData
{
    Vect2 edgeVectors[3];
    float edgeDist[3];
    float edgeAdj[3];

    inline void GetBaryCoords(const Vect2 &point, float *baryCoords) const
    {
        baryCoords[0] = ((edgeVectors[1]|point)-edgeDist[1])*edgeAdj[1];
        baryCoords[1] = ((edgeVectors[2]|point)-edgeDist[2])*edgeAdj[2];
        baryCoords[2] = ((edgeVectors[0]|point)-edgeDist[0])*edgeAdj[0];
    }
};

struct LightmapRender
{
    ~LightmapRender()
    {
        Free(baseLightmap);
        Free(lightmap[0]);
        Free(lightmap[1]);
        Free(lightmap[2]);
    }

    List<TexelInfo> texelInfo;
    Vect *baseLightmap;
    Vect *lightmap[3];

    inline void Serialize(Serializer &s, int width, int height, BOOL bTexInfo)
    {
        if(s.IsLoading())
        {
            baseLightmap = (Vect*)Allocate(width*height*sizeof(Vect));
            lightmap[0]  = (Vect*)Allocate(width*height*sizeof(Vect));
            lightmap[1]  = (Vect*)Allocate(width*height*sizeof(Vect));
            lightmap[2]  = (Vect*)Allocate(width*height*sizeof(Vect));
            zero(baseLightmap, width*height*sizeof(Vect));
            zero(lightmap[0],  width*height*sizeof(Vect));
            zero(lightmap[1],  width*height*sizeof(Vect));
            zero(lightmap[2],  width*height*sizeof(Vect));
        }

        Vect::SerializeArray(s, baseLightmap, width*height);
        Vect::SerializeArray(s, lightmap[0],  width*height);
        Vect::SerializeArray(s, lightmap[1],  width*height);
        Vect::SerializeArray(s, lightmap[2],  width*height);

        if(bTexInfo)
            s << texelInfo;
    }
};

struct LightmapInfo
{
    inline LightmapInfo(LightmapMeshInfo &mInfo) : mi(mInfo) {bHasTransform = FALSE; BuildData();}

    BOOL bHasTransform;
    Matrix transform;

    LightmapMeshInfo &mi;

    List<Vect> Binormals;
    List<TriData> triData;
    List<Vect> LightVectors;

    void BuildData();
    void GenerateTexelInfo(DWORD size, LightmapRender &TexelData);
    void GenerateLightData(Light *light);

    inline Vect GetPos(const Face &face, float bary[3]) const
    {
        return (mi.VertList[face.A]*bary[0]) + (mi.VertList[face.B]*bary[1]) + (mi.VertList[face.C]*bary[2]);
    }

    inline Vect GetNorm(const Face &face, float bary[3]) const
    {
        return (mi.NormalList[face.A]*bary[0]) + (mi.NormalList[face.B]*bary[1]) + (mi.NormalList[face.C]*bary[2]);
    }

    inline Vect GetTanU(const Face &face, float bary[3]) const
    {
        return (mi.TangentList[face.A]*bary[0]) + (mi.TangentList[face.B]*bary[1]) + (mi.TangentList[face.C]*bary[2]);
    }

    inline Vect GetTanV(const Face &face, float bary[3]) const
    {
        return (Binormals[face.A]*bary[0]) + (Binormals[face.B]*bary[1]) + (Binormals[face.C]*bary[2]);
    }

    inline Vect GetLightVec(const Face &face, float bary[3]) const
    {
        return (LightVectors[face.A]*bary[0]) + (LightVectors[face.B]*bary[1]) + (LightVectors[face.C]*bary[2]);
    }
};

struct LMMesh
{
    Mesh *mesh;
    List<MeshEntity*> Entities;

    inline void FreeData() {Entities.Clear();}
};

struct LightmapScene
{
    inline LightmapScene() {counter = 0;}

    List<EditorBrush*> Brushes;
    List<LMMesh> LMMeshList;

    int counter;
};


struct LightmapSettings
{
    inline LightmapSettings()
    {
        wStructVer = 0x104;
        nGIPasses = 1;
        aoDist = maxPhotonDist = 50.0f;
        hemicubeResolution = 32;
        bBlurShadows = TRUE;
        bUseIBL = bUseGI = bUseAO = bVisualizeAO = bUseDXT1 = FALSE;
        aoExponent = gamma = brightness = contrast = 1.0f;
        aoDarknessCutoff = 0.0f;
        resMultiplier = 1;
    }

    WORD  wStructVer;
    BOOL  bUseIBL;
    BOOL  bUseGI;
    int   nGIPasses;

    BOOL  bBlurShadows;

    float maxPhotonDist;
    int   hemicubeResolution;

    BOOL  bFilter, bUseDXT1;
    float gamma;
    float brightness;
    float contrast;

    BOOL  bUseAO, bVisualizeAO;
    float aoDist;
    float aoExponent;
    float aoDarknessCutoff;

    int   resMultiplier;

    friend inline Serializer& operator<<(Serializer &s, LightmapSettings &lms)
    {
        long wStructSize = sizeof(LightmapSettings);
        WORD wStructVer = lms.wStructVer;
        s << wStructSize << wStructVer;

        if(wStructVer != lms.wStructVer)
        {
            s.Seek(wStructSize, SERIALIZE_SEEK_CURRENT);
            return s;
        }

        s << lms.bUseIBL
          << lms.bUseGI
          << lms.nGIPasses
          << lms.bBlurShadows
          << lms.maxPhotonDist
          << lms.hemicubeResolution
          << lms.bUseDXT1
          << lms.bFilter
          << lms.gamma
          << lms.brightness
          << lms.contrast
          << lms.bUseAO
          << lms.bVisualizeAO
          << lms.aoDist
          << lms.aoExponent
          << lms.aoDarknessCutoff
          << lms.resMultiplier;

        return s;
    }
};


/*==============================================================
  Editor Level Info
===============================================================*/

class EditorLevelInfo : public Object
{
    DeclareClass(EditorLevelInfo, Object);

    friend class    EditorBrush;
    friend class    EditorEngine;
    friend class    EditorViewport;

    void            DrawEditorObjects(Camera *camera, float cameraZoom, EditorViewport *vp);

public:
    EditorLevelInfo();
    ~EditorLevelInfo();

    void CreateBoxBrush();

    void Deselect(Entity *ent=NULL);
    void Select(Entity *ent);
    void SelectMultiple(List<Entity*> entities);

    void UpdateManipulatorPos();

    void ProcessSelection(const Vect &rayOrig, const Vect &rayDir, BOOL bSelectBrushes);

    void RightClick(const Vect &rayOrig, const Vect &rayDir, BOOL bSelectBrushes);

    void SetPolyMaterials(CTSTR lpMaterialName, BOOL bSaveUndo=TRUE);
    void RemoveMaterialFromScene(Material *material);

    void PanUVs(BOOL bPanU, float amount);
    void RotateUVs(float angle);
    void ScaleUVs(DWORD dwScale, float amount);
    void FitUVs(float uTile, float vTile);
    void FloorAlignUVs(HWND hwndAlignWindow);
    void WallAlignUVs(HWND hwndAlignWindow);
    void SaveUVUndoData();

    void RebuildScene();
    void RebuildSubtractions();
    void RebuildAdditions();

    void ImportBrush();

    void SetCurEditMode(EditMode newEditMode);
    void SetCurModifyMode(ModifyMode newModifyMode);
    void SetCurSelectMode(SelectMode newSelectMode);

    void SetContextMenuItems(HMENU hMenu);

    void DeleteSelectedObjects(BOOL bRedo=FALSE);

    void SavePolySelectionUndoData();
    void SaveObjectSelectionUndoData();
    void SaveObjectCreationUndoData(Entity *ent);

    void ResetEntityLevelData();

    void DrawHemicube(const List<Brush*> &Brushes, const List<MeshEntity*> &StaticMeshes, const Vect &texelPos, DWORD pass, Effect *curEffect);

    void BuildLightmaps(LightmapSettings &settings);
    void BuildSubLightmaps(LightmapScene &scene, DWORD pass);
    void RenderLightmap(LightmapInfo &lightmapInfo, DWORD size, LMLightInfo &lightInfo, LightmapRender &lightmap);
    void RenderIBLLightmap(LightmapInfo &lightmapInfo, const Bounds &bounds, DWORD size, DWORD pass, LightmapRender &lightmap);

    void UploadLightmaps(LightmapScene &scene, BOOL bFinal);

    void UpdateLightmapProgress(float percentage, CTSTR lpText, CTSTR lpText2=NULL);

    Mesh                *sphereMesh;
    Texture             *whiteTex, *blackTex;
    HANDLE              lightmapCamDir;
    HANDLE              lightmapWorldMatrix;
    HANDLE              lightmapIllumColor;
    HANDLE              lightmapIllumTexture;
    HANDLE              lightmapDiffuse;
    HANDLE              lightmapTexture;
    HANDLE              lightmapMeshScale;
    LightmapSettings    lightmapSettings;

    HWND                hwndProgressBox;

    EditMode            curEditMode;
    ModifyMode          curModifyMode;
    SelectMode          curSelectMode;

    BOOL                bSnapToGrid;
    float               gridSpacing;

    float               curYPlanePosition;

    String              strLevelName;

    ObjectCreator       *newObject;

    DWORD               curSidebarTab;

    BOOL                bModified;

    String              selectedPrefab;

    EditorBrush         *WorkBrush;
    List<EditorBrush*>  BrushList;

    List<Entity*>       CreatedObjects;

    List<Entity*>       SelectedObjects;

    Manipulator         *curManipulator;

    List<ShapeVert>     SavedShape;

    static void DestroyWorkbrush();

    static void ENGINEAPI UndoRedoDelete(Serializer &s, Serializer &sOut, BOOL bUndo);
    static void ENGINEAPI UndoRedoSelectPolys(Serializer &s, Serializer &sOut, BOOL bUndo);
    static void ENGINEAPI UndoRedoSelectObjects(Serializer &s, Serializer &sOut, BOOL bUndo);
    static void ENGINEAPI UndoRedoCreateObject(Serializer &s, Serializer &sOut, BOOL bUndo);
    static void ENGINEAPI UndoRedoChangeSurfaceMaterials(Serializer &s, Serializer &sOut, BOOL bUndo);

    static void ENGINEAPI SaveCreateWorkbrushUndoData(const String &undoName, EditorBrush *brush);
    static void ENGINEAPI UndoRedoCreateWorkBrush(Serializer &s, Serializer &sOut, BOOL bUndo);
    static void ENGINEAPI UndoRedoDeleteWorkBrush(Serializer &s, Serializer &sOut, BOOL bUndo);

    static void ENGINEAPI UndoRedoMovement(Serializer &s, Serializer &sOut, BOOL bUndo);
    static void ENGINEAPI UndoRedoDuplicate(Serializer &s, Serializer &sOut, BOOL bUndo);

    static void ENGINEAPI UndoRedoChangeProperties(Serializer &s, Serializer &sOut, BOOL bUndo);

    static void ENGINEAPI UndoRedoUVOperation(Serializer &s, Serializer &sOut, BOOL bUndo);

    static void ENGINEAPI UndoRedoRotation(Serializer &s, Serializer &sOut, BOOL bUndo);

    static void ENGINEAPI SaveModifyYPlaneUndoData(CTSTR lpUndoName, float yPos);
    static void ENGINEAPI UndoRedoModifyYPlane(Serializer &s, Serializer &sOut, BOOL bUndo);

    static void SetWorldPos(Entity *ent, const Vect &newPos);
    static void SetWorldRot(Entity *ent, const Quat &newRot);
};

inline void SetWorldPos(Entity *ent, const Vect& newPos) {EditorLevelInfo::SetWorldPos(ent, newPos);}
inline void SetWorldRot(Entity *ent, const Quat& newRot) {EditorLevelInfo::SetWorldRot(ent, newRot);}

extern EditorLevelInfo *levelInfo;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Entity.h

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


#ifndef MESHENTITY_HEADER
#define MESHENTITY_HEADER


//-----------------------------------------
// IndirectLightSample

#pragma pack(push, 1)

//28 bytes.  hrm.  wonder if this can't be trimmed down somehow.
struct IndirectLightSample
{
    CompactVect pos, norm;
    DWORD color;
};

#pragma pack(pop)


//-----------------------------------------
// Lightmap Data
struct Lightmap
{
    Texture *plain;
    Texture *X;
    Texture *Y;
    Texture *Z;

    List<IndirectLightSample> ILSamples;

    inline void FreeData()
    {
        DestroyObject(plain);
        DestroyObject(X);
        DestroyObject(Y);
        DestroyObject(Z);
        ILSamples.Clear();
        plain = X = Y = Z = NULL;
    }

    friend Serializer& operator<<(Serializer &s, Lightmap &lm);
};

/*=========================================================
    MeshEntity
==========================================================*/

class BASE_EXPORT MeshObject : public Object
{
    DeclareClass(MeshObject, Object);

    Mesh *mesh;
    List<Material*> MaterialList;

    void SetMesh(CTSTR lpMeshResource);

public:
    inline MeshObject() {}

    MeshObject(CTSTR lpMeshResource);
    ~MeshObject();

    void Render();
    void QuickRender();
    void RenderBare();

    void SetMaterial(int materialID, Material *material);

    //<Script module="Base" classdecs="MeshObject">
    Declare_Internal_Member(native_MeshObject);
    Declare_Internal_Member(native_Render);
    Declare_Internal_Member(native_RenderBare);
    Declare_Internal_Member(native_SetMaterial);
    //</Script>
};


/*=========================================================
    MeshEntity
==========================================================*/

struct MeshLightInfo
{
    Light *light;
    DWORD side;

    inline bool operator==(const MeshLightInfo& info) const
    {
        return (info.light == light && info.side == side);
    }
};

class ShadowDecal;

class BASE_EXPORT MeshEntity : public Entity
{
    friend class EditorViewport;
    friend class EditorLevelInfo;
    friend class EditorEngine;
    friend class AnimatedEntity;

    friend class Level;
    friend class IndoorLevel;
    friend class OutdoorLevel;
    friend class OctLevel;
    friend class SpaceLevel;

    friend class Prefab;
    friend class Light;
    friend class AnimatedPrefab;

    DeclareClass(MeshEntity, Entity);

    Vect            scale;
    BOOL            bHasScale;

    //mesh info
    Mesh           *mesh;

    Matrix          transform;
    Matrix          invTransform;
    VertexBuffer   *VertBuffer;

    //shadow/lighting information
    Vect           *VertList;  //ref to mesh vertex buffer data
    Vect           *FaceNormalList;
    Edge           *EdgeList;
    BOOL            bCastShadow;
    BOOL            bLightmapped;
    Lightmap        lightmap;

    Texture         *compositeShadow;
    ShadowDecal     *shadowDecal;

    Vect            adjustPos;
    Quat            adjustRot;

    List<CompactVect> SH;

    //tracks what dynamic lights are lighting this object
    List<MeshLightInfo> MeshLights;

    //visibility info
    Bounds          bounds, prevBounds;

    inline void LoadEffectData();
    inline void ResetEffectData();

    List<Material*> MaterialList;
    DWORD           wireframeColor;

    void ResetScale();

public:
    MeshEntity();

    virtual void Destroy();

    virtual void Render();

    virtual void SetMeshAdjustPos(const Vect &posAdj) {adjustPos = posAdj;}
    virtual void SetMeshAdjustRot(const Quat &rotAdj) {adjustRot = rotAdj;}

    virtual const Vect& GetMeshAdjustPos() const {return adjustPos;}
    virtual const Quat& GetMeshAdjustRot() const {return adjustRot;}

    virtual void ResetTransform();

    virtual void WorldRender();
    virtual void QuickRender();
    virtual void RenderInitialPass();
    virtual void RenderLightmaps();
    //virtual void RenderIllumination();
    virtual void RenderWireframe();
    virtual void RenderBare(BOOL bUseTransform);

    void SetScale(const Vect &scale);
    inline void SetScale(float scaleX, float scaleY, float scaleZ) {SetScale(Vect(scaleX, scaleY, scaleZ));}

    void SetMaterial(DWORD texture, Material *material);

    virtual Bounds GetBounds() {return bounds;}

    virtual void OnUpdatePosition();

    inline const Bounds& GetMeshBounds() const          {return bounds;}
    inline const Bounds& GetInitialBounds() const       {return mesh->bounds;}

    inline const Matrix& GetTransform() const           {return transform;}
    inline const Matrix& GetInvTransform() const        {return invTransform;}

    virtual void SetMesh(CTSTR lpMesh);

    inline const Mesh* GetMesh() const                  {return mesh;}

    inline PhyShape* GetMeshShape(BOOL bDynamic=FALSE)  {return mesh->GetShape(bDynamic);}

    inline BOOL HasValidMesh() const                    {return mesh != NULL;}

    void RemoveLight(Light *light);

    void Serialize(Serializer &s);

    void MeshCollisionCallback(PhyObject *collider, int triIndex, float appliedImpulse, int lifeTime, const Vect &hitPos, float &friction, float &restitution);

    inline Texture* GetCompositeShadow() const {return compositeShadow;}

    inline BOOL IsLightmapped() const {return bLightmapped;}
    inline BOOL CanCastShadow() const {return bCastShadow;}

    //editor stuff
    virtual void    EditorRender()  {}
    virtual BOOL    CanSelect(const Vect &rayOrig, const Vect &rayDir);

    virtual Entity* DuplicateEntity();

    float indirectLightDist;

    //<Script module="Base" classdecs="MeshEntity">
    BOOL bUseLightmapping;
    int lightmapResolution;
    BOOL bStaticGeometry;
    BOOL bCastCompositeShadow;

    Declare_Internal_Member(native_SetMesh);
    Declare_Internal_Member(native_SetMeshAdjustPos);
    Declare_Internal_Member(native_SetMeshAdjustRot);
    Declare_Internal_Member(native_GetMeshAdjustPos);
    Declare_Internal_Member(native_GetMeshAdjustRot);
    Declare_Internal_Member(native_SetScale);
    Declare_Internal_Member(native_SetScale_2);
    Declare_Internal_Member(native_GetMeshBounds);
    Declare_Internal_Member(native_GetInitialBounds);
    Declare_Internal_Member(native_GetTransform);
    Declare_Internal_Member(native_GetInvTransform);
    Declare_Internal_Member(native_HasValidMesh);
    Declare_Internal_Member(native_SetMaterial);
    //</Script>
};


#endif

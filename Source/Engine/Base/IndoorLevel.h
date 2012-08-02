/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  IndoorLevel.h:  Indoor Level

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


#ifndef INDOORLEVEL_HEADER
#define INDOORLEVEL_HEADER



//-----------------------------------------
// Potentially Visible Set
struct BASE_EXPORT PVS : public Brush
{
    //current entities and lights within this PVS
    List<Entity*>   entities;
    List<Entity*>   visEntities;
    List<MeshEntity*> visMeshEntities;
    List<Light*>    lights;
    List<Light*>    visLights;
    
    //other data
    List<DWORD>     PortalRefs;
    List<DWORD>     BrushRefs;

    void RemoveEntities();

    void Clear();

    void Serialize(Serializer &s)
    {
        if(s.IsLoading())
        {
            entities.Clear();
            visEntities.Clear();
            visMeshEntities.Clear();
            lights.Clear();
            visLights.Clear();
            PortalRefs.Clear();
            BrushRefs.Clear();
        }

        Brush::Serialize(s);

        s << PortalRefs;
        s << BrushRefs;
    }
};

//-----------------------------------------
// Portal
struct Portal
{
    DWORD           nVerts;
    DWORD           nFaces;

    DWORD           PVSRefs[2];
    Bounds          bounds;
    VertexBuffer   *VertBuffer;
    IndexBuffer    *IdxBuffer;
    Face           *FaceList;

    void Render(BOOL bDrawBack);

    void Serialize(Serializer &s)
    {
        s << nVerts
          << nFaces
          << PVSRefs[0]
          << PVSRefs[1]
          << bounds;

        if(s.IsLoading())
        {
            delete VertBuffer;
            delete IdxBuffer;

            VBData *vbd = new VBData;

            s << vbd->VertList;

            VertBuffer = CreateVertexBuffer(vbd);

            FaceList = (Face*)Allocate(nFaces*sizeof(Face));
            s.Serialize(FaceList, nFaces*sizeof(Face));
            IdxBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, FaceList, nFaces*3);
        }
        else
        {
            VBData *vbd = VertBuffer->GetData();
            s << vbd->VertList;

            s.Serialize(FaceList, nFaces*sizeof(Face));
        }
    }

    void FreeBuffers()
    {
        delete VertBuffer;
        delete IdxBuffer;
        VertBuffer = NULL;
        IdxBuffer  = NULL;
        FaceList   = NULL;
    }
};

#define PortalPVSRef(pvsID, portal) ((portal.PVSRefs[0] == pvsID) ? portal.PVSRefs[1] : portal.PVSRefs[0])
#define PortalPVS(pvs, portal)      ((&PVSList[portal.PVSRefs[0]] == &pvs) ? PVSList[portal.PVSRefs[1]] : PVSList[portal.PVSRefs[0]])

/*=========================================================
    IndoorEntityData
==========================================================*/

class IndoorEntityData : public LevelData
{
public:
    List<DWORD>     PVSRefs,VisPVSRefs;

protected:
    virtual ~IndoorEntityData()
    {
        PVSRefs.Clear();
        VisPVSRefs.Clear();
    }
};

#define GetIndoorEntityData(ent) static_cast<IndoorEntityData*>(GetEntityData(ent))


/*=========================================================
    IndoorLevel
==========================================================*/

enum {LIGHTMAPPING_NONE, LIGHTMAPPING_BASIC, LIGHTMAPPING_DIFFUSEBUMP};

//-----------------------------------------
// Portal-based Level
class BASE_EXPORT IndoorLevel : public Level
{
    DeclareClass(IndoorLevel, Level);

    friend struct       PVS;
    friend struct       Brush;
    friend struct       AddRef;
    friend class        MeshEntity;
    friend class        PointLight;
    friend class        SpotLight;
    friend class        DirectionalLight;
    friend class        EditorLevelInfo;
    friend class        EditorBrush;
    friend class        EditorEngine;

    List<PVS*>          curRenderingPVSs;
    List<Light*>        curRenderingLights;

    BOOL                bGlobalFog;
    DWORD               dwGlobalFogColor;

    DWORD               LightmapTechnique;

    List<PVS>           PVSList;
    List<Portal>        PortalList;
    List<Brush>         BrushList;

protected:
    virtual void UpdateEntityPositionData(Entity *ent, const Matrix &transform);

    void GetVisiblePVSs(PVS &curPVS, PVS *lpLastPVS, const Vect &eye, ViewClip *lpClip, List<PVS*> &VisPVSList);
    void GetLightDrawInfo(DWORD curPVSID, DWORD lastPVSID, Light *light, ViewClip *lpClip);

    virtual void CalculateRenderOrder(Camera *camera, const ViewClip &clip);

public:
    virtual void PreFrame();

    virtual BOOL Load(CTSTR lpName);
    virtual void Unload();

    virtual void GetStaticGeometry(const Bounds &bounds, List<Brush*> &Brushes, List<MeshEntity*> &StaticMeshes);

    virtual void GetObjects(const Bounds &bounds, List<LevelObject*> &objects);
    virtual void GetObjects(const Vect &eyePos, const ViewClip &clip, List<LevelObject*> &objects);

    virtual void AddEntity(Entity *ent);
    virtual void RemoveEntity(Entity *ent, BOOL bChildren=TRUE);

    virtual AIPath* GetEntityPath(Entity *ent, const Vect &targetPos) {return NULL;}
};



#endif


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  OctLevel.h

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#ifndef OCTLEVEL_HEADER
#define OCTLEVEL_HEADER



/*=========================================================
    OctBVH
==========================================================*/

#define OCT_X 1
#define OCT_Y 2
#define OCT_Z 4

struct BASE_EXPORT OctNode
{
    inline ~OctNode()
    {
        for(int i=0; i<8; i++)
            delete children[i];
    }

    Bounds bounds;
    OctNode *Parent;
    List<LevelObject*> Leaves;
    BYTE parentPos, numChildren;
    BYTE depth, x, y, z;
    OctNode *children[8];
};

class BASE_EXPORT OctBVH
{
    float maxSize, maxSizeD2;
    float minSize;
    int   maxDepth;

    float maxMarginPercentage;

    OctNode *root;

public:
    OctBVH();
    ~OctBVH();

    OctNode* Add(LevelObject *leaf);
    void Remove(OctNode *node, LevelObject *leaf);
    OctNode* Update(OctNode *node, const Bounds &bounds, LevelObject *leaf);

    inline OctNode* GetRoot() const {return root;}
};


/*=========================================================
    OctEntityData
==========================================================*/

class OctEntityData : public LevelData
{
public:
    OctNode    *node;
    LevelObject *leaf;
};

#define GetOctEntityData(ent) static_cast<OctEntityData*>(GetEntityData(ent))


/*=========================================================
    OctLevel
==========================================================*/

struct BASE_EXPORT OctBrush : Brush
{
    inline ~OctBrush() {Clear();}

    OctNode    *node;
    LevelObject *leaf;

    void Clear();
};


class BASE_EXPORT OctLevel : public Level
{
    DeclareClass(OctLevel, Level);

    friend struct       Brush;
    friend struct       OctBrush;
    friend class        MeshEntity;
    friend class        PointLight;
    friend class        SpotLight;
    friend class        DirectionalLight;
    friend class        EditorLevelInfo;
    friend class        EditorBrush;
    friend class        EditorEngine;


    List<OctBrush*>        BrushList;

    OctBVH                 *objectTree;

protected:
    virtual void UpdateEntityPositionData(Entity *ent, const Matrix &transform);
    virtual void CalculateRenderOrder(Camera *camera, const ViewClip &clip);

    void CalcRenderOrderRecurse(const Vect &eye, const ViewClip &clip, MeshOrderInfo &meshOrder, OctNode *curNode, BOOL bFullyVisible=FALSE);

    void GetVisibleLeaves(const Vect &eye, const ViewClip &clip, List<LevelObject*> &leafList, OctNode *curNode, BOOL bFullyVisible=FALSE);
    void GetSphereIntersectingLeaves(const Vect &pos, float radius, List<LevelObject*> &leafList, OctNode *curNode);
    void GetBoundsIntersectingLeaves(const Bounds &bounds, List<LevelObject*> &leafList, OctNode *curNode);

public:
    OctLevel();
    ~OctLevel();

    virtual void PreFrame();

    virtual BOOL Load(CTSTR lpName);
    virtual void Unload();

    virtual void AddEntity(Entity *ent);
    virtual void RemoveEntity(Entity *ent, BOOL bChildren=TRUE);

    virtual AIPath* GetEntityPath(Entity *ent, const Vect &targetPos);

    virtual void GetSHIndirectLightSamples(const Vect& pos, float radius, const Matrix &transform, List<CompactVect> &SH);

    virtual void GetStaticGeometry(const Bounds &bounds, List<Brush*> &Brushes, List<MeshEntity*> &StaticMeshes);

    virtual void GetObjects(const Bounds &bounds, List<LevelObject*> &objects) {GetBoundsIntersectingLeaves(bounds, objects, objectTree->GetRoot());}
    virtual void GetObjects(const Vect &eyePos, const ViewClip &clip, List<LevelObject*> &objects) {GetVisibleLeaves(eyePos, clip, objects, objectTree->GetRoot());}

    //----------------------------------------------------------------------------------

    inline static OctLevel* GetOctLevel() {return level->IsOf(GetClass(OctLevel)) ? (OctLevel*)level : NULL;}

    inline BOOL GetMouseLineEntities(const Vect &startPos, const Vect &endPos, List<Entity*> &Entities, DWORD filter=PHY_ALL)
    {
        Vect newStart = startPos;

        List<PhyObject*> objs;
        if(physics->GetLineObjects(newStart, endPos, objs, filter))
        {
            for(int i=0; i<objs.Num(); i++)
            {
                Entity *ent = objs[i]->GetEntityOwner();
                if(ent)
                    Entities << ent;
            }

            if(Entities.Num())
                return TRUE;
        }

        return FALSE;
    }

    inline BOOL GetMouseRayEntities(const Vect &vOrig, const Vect &rayDir, List<Entity*> &Entities, DWORD filter=PHY_ALL, float dist=1000.0f)
    {
        Vect targetPos = vOrig+(rayDir*dist);
        return GetMouseLineEntities(vOrig, targetPos, Entities, filter);
    }

    inline BOOL GetMouseLineCollision(const Vect &startPos, const Vect &endPos, PhyCollisionInfo *collisionInfo=NULL, DWORD filter=PHY_ALL)
    {
        return physics->GetLineCollision(startPos, endPos, collisionInfo, filter);
    }

    inline BOOL GetMouseRayCollision(const Vect &vOrig, const Vect &rayDir, PhyCollisionInfo *collisionInfo=NULL, DWORD filter=PHY_ALL, float dist=1000.0f)
    {
        Vect targetPos = vOrig+(rayDir*dist);
        return GetMouseLineCollision(vOrig, targetPos, collisionInfo, filter);
    }
};


#endif

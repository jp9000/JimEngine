/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  OctLevel.cpp

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#include "Base.h"

DefineClass(OctLevel);


OctLevel::OctLevel()
{
}

OctLevel::~OctLevel()
{
}


BOOL OctLevel::Load(CTSTR lpName)
{
    traceIn(OctLevel::Load);

    assert(lpName);

    XFileInputSerializer levelData;

    //--------------------------------------------------

    DWORD dwSigniture, dwEditorOffset;
    DWORD i;
    WORD wVersion;

    Log(TEXT("Loading Oct Level: %s"), lpName);
    strName = lpName;

    String strPath;
    Engine::ConvertResourceName(lpName, TEXT("levels"), strPath);
    if(!levelData.Open(strPath))
    {
        AppWarning(TEXT("Unable to open the file: \"%s\""), lpName);
        return FALSE;
    }

    levelData << dwSigniture;
    levelData << wVersion;
    levelData << dwEditorOffset;

    if(dwSigniture != '\0ltx')
    {
        AppWarning(TEXT("'%s' Bad Level file"), lpName);
        return FALSE;
    }

    if(wVersion != OCTLEVEL_VERSION)
    {
        AppWarning(TEXT("'%s' outdated file format"), lpName);
        return FALSE;
    }

    //--------------------------------------------------

    BOOL bObsolete;
    DWORD nModules, nObjects, nBrushes, nCinematics, dwObsolete;

    levelData << nModules;
    levelData << nBrushes;
    levelData << nObjects;
    levelData << nCinematics;
    levelData << dwObsolete;
    levelData << bObsolete;
    levelData << dwObsolete;

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

    levelData << dwObsolete << dwObsolete;
    objectTree = new OctBVH;

    //--------------------------------------------------

    BrushList.SetSize(nBrushes);
    for(i=0; i<nBrushes; i++)
    {
        OctBrush *brush = new OctBrush;
        brush->Serialize(levelData);

        brush->leaf = new LevelObject(brush);
        brush->node = objectTree->Add(brush->leaf);

        BrushList[i] = brush;
    }

    //--------------------------------------------------

    levelData << entityIDCounter;

    //--------------------------------------------------

    levelData.Close();

    bLoaded = TRUE;

    bUpdateAllEntities = TRUE;
    ArrangeEntities();

    return TRUE;

    traceOut;
}

void OctLevel::Unload()
{
    traceIn(OctLevel::Unload);

    DWORD i;

    DestroyEntities();

    //------------------------------------

    for(i=0; i<BrushList.Num(); i++)
        delete BrushList[i];
    BrushList.Clear();

    //------------------------------------
    // Free Object Tree
    delete objectTree;

    traceOut;
}

void OctLevel::PreFrame()
{
    GS->GetSize(TempRenderWidth, TempRenderHeight);

    Super::PreFrame();
}

void OctLevel::AddEntity(Entity *ent)
{
    if(!ent->bPositionalOnly)
        GetEntityData(ent) = CreateObject(OctEntityData);
}

void OctLevel::RemoveEntity(Entity *ent, BOOL bChildren)
{
    traceInFast(OctLevel::RemoveEntity);

    int i;

    if(!GetEntityData(ent))
    {
        if(bChildren)
        {
            for(i=0; i<ent->NumChildren(); i++)
                RemoveEntity(ent->GetChild(i));
        }

        return;
    }

    OctEntityData *entData = GetOctEntityData(ent);

    objectTree->Remove(entData->node, entData->leaf);
    delete entData->leaf;

    DestroyObject(GetEntityData(ent));
    GetEntityData(ent) = NULL;

    if(bChildren)
    {
        for(i=0; i<ent->NumChildren(); i++)
            RemoveEntity(ent->GetChild(i));
    }

    traceOutFast;
}

void OctLevel::UpdateEntityPositionData(Entity *ent, const Matrix &transform)
{
    traceInFast(OctLevel::UpdateEntityPositionData);

    BOOL isLight = ent->IsOf(GetClass(Light));
    BOOL isMeshEnt = ent->IsOf(GetClass(MeshEntity));
    OctEntityData *entData = GetOctEntityData(ent);

    Bounds entBounds = ent->GetBounds().GetTransformedBounds(transform);

    BOOL bUpdating = TRUE;

    if(!entData)
    {
        entData = CreateObject(OctEntityData);
        GetEntityData(ent) = (LevelData*)entData;

        entData->node = objectTree->Add(entData->leaf = new LevelObject(ent, entBounds));

        bUpdating = FALSE;
    }
    else
        entData->node = objectTree->Update(entData->node, entBounds, entData->leaf);

    traceOutFast;
}

void OctLevel::GetSHIndirectLightSamples(const Vect& pos, float radius, const Matrix &transform, List<CompactVect> &SH)
{
    List<LevelObject*> Leaves;

    Vect newSH[6];
    zero(newSH, sizeof(Vect)*6);

    GetSphereIntersectingLeaves(pos, radius, Leaves, objectTree->GetRoot());
    for(int i=0; i<Leaves.Num(); i++)
    {
        LevelObject *leaf = Leaves[i];

        if(leaf->type == ObjectType_Entity)
        {
            if(!leaf->ent->IsOf(GetClass(Prefab)))
                continue;

            MeshEntity *meshent = (MeshEntity*)leaf->ent;
            if(!meshent->bRenderable || AlreadyUsed(meshent) || !meshent->IsLightmapped())
                continue;
            if(!meshent->GetMeshBounds().SphereIntersects(pos.GetTransformedPoint(meshent->GetInvTransform()), radius))
                continue;

            for(int k=0; k<meshent->lightmap.ILSamples.Num(); k++)
            {
                IndirectLightSample &sample = meshent->lightmap.ILSamples[k];

                Vect line = (Vect(sample.pos)-pos);
                Vect dir = line.GetNorm();

                float dist = line.Len();
                if(dist > radius) continue;

                float shade = dir.Dot(-Vect(sample.norm));
                if(shade <= 0.0f) continue;

                Vect color;
                color.MakeFromRGB(sample.color);
                color *= 1.0f-(dist/radius);
                color *= shade;

                if(!SH.Num()) SH.SetSize(6);

                dir.TransformVector(transform);

                newSH[0] += color * 0.5f * dir.x;
                newSH[1] += color * 0.5f * dir.y;
                newSH[2] += color * 0.5f * dir.z;
                newSH[3] += color * 0.5f * fabsf(dir.x);
                newSH[4] += color * 0.5f * fabsf(dir.y);
                newSH[5] += color * 0.5f * fabsf(dir.z);
            }
        }
        else
        {
            Brush *brush = leaf->brush;

            if(!brush->bLightmapped || brush->bAlreadyUsed)
                continue;
            if(!brush->bounds.SphereIntersects(pos, radius))
                continue;

            for(int k=0; k<brush->lightmap.ILSamples.Num(); k++)
            {
                IndirectLightSample &sample = brush->lightmap.ILSamples[k];

                Vect line = (Vect(sample.pos)-pos);
                Vect dir = line.GetNorm();
                float shade = dir.Dot(-Vect(sample.norm));
                if(shade <= 0.0f) continue;

                float dist = line.Len();
                if(dist > radius) continue;

                Vect color;
                color.MakeFromRGB(sample.color);
                color *= 1.0f-(dist/radius);
                color *= shade;

                if(!SH.Num()) SH.SetSize(6);

                dir.TransformVector(transform);

                newSH[0] += color * 0.5f * dir.x;
                newSH[1] += color * 0.5f * dir.y;
                newSH[2] += color * 0.5f * dir.z;
                newSH[3] += color * 0.5f * fabsf(dir.x);
                newSH[4] += color * 0.5f * fabsf(dir.y);
                newSH[5] += color * 0.5f * fabsf(dir.z);
            }

        }
    }

    if(SH.Num())
    {
        float mulVal = radius*0.0625f;
        SH[0] = newSH[0] * mulVal;
        SH[1] = newSH[1] * mulVal;
        SH[2] = newSH[2] * mulVal;
        SH[3] = newSH[3] * mulVal;
        SH[4] = newSH[4] * mulVal;
        SH[5] = newSH[5] * mulVal;
    }
}

void OctLevel::GetVisibleLeaves(const Vect &eye, const ViewClip &clip, List<LevelObject*> &leafList, OctNode *node, BOOL bFullyVisible)
{
    DWORD i;
    BOOL bCheckBounds = (node->numChildren > 1) || (node->Leaves.Num() > 0);
    if(!bFullyVisible && bCheckBounds)
    {
        int testVal = clip.BoundsTest(node->bounds);
        if(testVal == BOUNDS_INSIDE)
            bFullyVisible = TRUE;
        else if(testVal == BOUNDS_OUTSIDE)
            return;
    }

    if(node->numChildren)
    {
        for(i=0; i<8; i++)
        {
            if(!node->children[i])
                continue;

            OctNode *child = node->children[i];
            GetVisibleLeaves(eye, clip, leafList, child, bFullyVisible);
        }
    }

    for(i=0; i<node->Leaves.Num(); i++)
    {
        LevelObject *leaf = node->Leaves[i];

        if(!bFullyVisible && !clip.BoundsVisible(leaf->bounds))
            continue;

        leafList << leaf;
    }
}


void OctLevel::GetSphereIntersectingLeaves(const Vect &pos, float radius, List<LevelObject*> &leafList, OctNode *node)
{
    DWORD i;
    BOOL bCheckBounds = (node->numChildren > 1) && (node->Leaves.Num() > 0);
    if(bCheckBounds)
    {
        if(!node->bounds.SphereIntersects(pos, radius))
            return;
    }

    if(node->numChildren)
    {
        for(i=0; i<8; i++)
        {
            if(!node->children[i])
                continue;

            OctNode *child = node->children[i];
            GetSphereIntersectingLeaves(pos, radius, leafList, child);
        }
    }

    for(i=0; i<node->Leaves.Num(); i++)
    {
        LevelObject *leaf = node->Leaves[i];

        if(!leaf->bounds.SphereIntersects(pos, radius))
            continue;

        leafList << leaf;
    }
}

void OctLevel::GetBoundsIntersectingLeaves(const Bounds &bounds, List<LevelObject*> &leafList, OctNode *node)
{
    DWORD i;
    BOOL bCheckBounds = (node->numChildren > 1) && (node->Leaves.Num() > 0);
    if(bCheckBounds)
    {
        if(!node->bounds.Intersects(bounds))
            return;
    }

    if(node->numChildren)
    {
        for(i=0; i<8; i++)
        {
            if(!node->children[i])
                continue;

            OctNode *child = node->children[i];
            GetBoundsIntersectingLeaves(bounds, leafList, child);
        }
    }

    for(i=0; i<node->Leaves.Num(); i++)
    {
        LevelObject *leaf = node->Leaves[i];

        if(!leaf->bounds.Intersects(bounds))
            continue;

        leafList << leaf;
    }
}

AIPath* OctLevel::GetEntityPath(Entity *ent, const Vect &targetPos)
{
    return NULL;
}

void OctBrush::Clear()
{
    OctLevel::GetOctLevel()->objectTree->Remove(node, leaf);
    delete leaf;

    Brush::Clear();
}



OctBVH::OctBVH()
{
    maxMarginPercentage = 0.60f;
    minSize = 20.0f;
    maxDepth = 8;

    maxSize = minSize*float(1<<(maxDepth-1));
    maxSizeD2 = maxSize*0.5f;
    root = new OctNode;
    root->depth = maxDepth-1;
    root->bounds.Min = M_INFINITE;
    root->bounds.Max = -M_INFINITE;
}

OctBVH::~OctBVH()
{
    delete root;
}

OctNode* OctBVH::Add(LevelObject *leaf)
{
    const Bounds &bounds = leaf->bounds;
    Vect size = bounds.GetSize();
    Vect center = bounds.Min+(size*0.5f);
    BOOL bAdjustBounds = TRUE;

    Bounds octBounds;
    octBounds.Min = -maxSizeD2;
    octBounds.Max = maxSizeD2;

    OctNode *node = root;
    int depth = maxDepth;

    while(--depth > 0)
    {
        if(bAdjustBounds)
            node->bounds.Merge(bounds);

        float nodeSize = minSize*float(1<<depth);
        float maxNodeMargin = (nodeSize*maxMarginPercentage);

        //too big for sub nodes, use this node
        if( (size.x >= maxNodeMargin) ||
            (size.y >= maxNodeMargin) ||
            (size.z >= maxNodeMargin) )
            break;

        Vect octCenter = octBounds.Min + (nodeSize*0.5f);

        BYTE x = (center.x > octCenter.x);
        BYTE y = (center.y > octCenter.y);
        BYTE z = (center.z > octCenter.z);

        int child;
        child  = x;
        child |= y << 1;
        child |= z << 2;

        if(!node->children[child])
        {
            OctNode *&newNode = node->children[child];
            newNode = new OctNode;
            newNode->Parent = node;
            newNode->bounds = bounds;
            newNode->parentPos = child;
            newNode->depth = depth-1;
            bAdjustBounds = FALSE;

            newNode->x = (node->x << 1) + x;
            newNode->y = (node->y << 1) + y;
            newNode->z = (node->z << 1) + z;

            ++node->numChildren;
            node = newNode;
        }
        else
            node = node->children[child];

        if(x)   octBounds.Min.x = octCenter.x;
        else    octBounds.Max.x = octCenter.x;

        if(y)   octBounds.Min.y = octCenter.y;
        else    octBounds.Max.y = octCenter.y;

        if(z)   octBounds.Min.z = octCenter.z;
        else    octBounds.Max.z = octCenter.z;
    }

    node->Leaves << leaf;

    return node;
}

void OctBVH::Remove(OctNode *curNode, LevelObject *leaf)
{
    OctNode *node = curNode;
    const Bounds &bounds = leaf->bounds;
    node->Leaves.RemoveItem(leaf);

    while(node)
    {
        //gets rid of empty nodes
        if((node != root) && !node->Leaves.Num() && !node->numChildren)
        {
            OctNode *parent = node->Parent;
            if(parent)
            {
                --parent->numChildren;
                parent->children[node->parentPos] = NULL;
            }

            delete node;
            node = parent;
            continue;
        }

        Bounds newBounds(Vect(M_INFINITE), Vect(-M_INFINITE));

        //adjust only if bordering
        if( (node->bounds.Max.x == bounds.Max.x) ||
            (node->bounds.Max.y == bounds.Max.y) ||
            (node->bounds.Max.z == bounds.Max.z) ||
            (node->bounds.Min.x == bounds.Min.x) ||
            (node->bounds.Min.y == bounds.Min.y) ||
            (node->bounds.Min.z == bounds.Min.z) )
        {
            if(node->numChildren)
            {
                for(int i=0; i<8; i++)
                {
                    OctNode *child = node->children[i];
                    if(child)
                        newBounds.Merge(child->bounds);
                }
            }

            for(int i=0; i<node->Leaves.Num(); i++)
                newBounds.Merge(node->Leaves[i]->bounds);
        }
        else
            break;

        node->bounds = newBounds;
        node = node->Parent;
    }
}

OctNode* OctBVH::Update(OctNode *curNode, const Bounds &bounds, LevelObject *leaf)
{
    //---------------------------------------------------
    // if no cell change, exit

    if(curNode->bounds.BoundsInside(bounds))
        return curNode;

    //---------------------------------------------------
    // if completely outisde of cell, readd

    float nodeSize = minSize*float(1<<curNode->depth);
    Bounds octBounds;
    octBounds.Min.Set(nodeSize*float(curNode->x), nodeSize*float(curNode->y), nodeSize*float(curNode->z));
    octBounds.Min -= maxSizeD2;
    octBounds.Max = octBounds.Min+nodeSize;

    Vect size = bounds.GetSize();
    Vect center = bounds.Min+(size*0.5f);
    if(!octBounds.PointInside(center))
    {
        Remove(curNode, leaf);
        leaf->bounds = bounds;
        return Add(leaf);
    }

    //---------------------------------------------------
    // otherwise same cell, but the cell must be recalculated

    OctNode *node = curNode;
    Bounds oldBounds = leaf->bounds;
    leaf->bounds = bounds;

    while(node)
    {
        Bounds newBounds(Vect(M_INFINITE), Vect(-M_INFINITE));

        //adjust only if bordering
        if( (node->bounds.Max.x == oldBounds.Max.x) ||
            (node->bounds.Max.y == oldBounds.Max.y) ||
            (node->bounds.Max.z == oldBounds.Max.z) ||
            (node->bounds.Min.x == oldBounds.Min.x) ||
            (node->bounds.Min.y == oldBounds.Min.y) ||
            (node->bounds.Min.z == oldBounds.Min.z) )
        {
            if(node->numChildren)
            {
                for(int i=0; i<8; i++)
                {
                    OctNode *child = node->children[i];
                    if(child)
                        newBounds.Merge(child->bounds);
                }
            }

            for(int i=0; i<node->Leaves.Num(); i++)
                newBounds.Merge(node->Leaves[i]->bounds);
        }
        else
            break;

        node = node->Parent;
    }

    return curNode;
}


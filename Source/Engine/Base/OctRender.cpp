/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  OctRender.cpp

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#include "Base.h"


void OctLevel::CalcRenderOrderRecurse(const Vect &eye, const ViewClip &clip, MeshOrderInfo &meshOrder, OctNode *curNode, BOOL bFullyVisible)
{
    OctNode *node = (curNode != NULL) ? curNode : objectTree->GetRoot();

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
            CalcRenderOrderRecurse(eye, clip, meshOrder, child, bFullyVisible);
        }
    }

    for(i=0; i<node->Leaves.Num(); i++)
    {
        LevelObject *leaf = node->Leaves[i];

        if(!bFullyVisible && !clip.BoundsVisible(leaf->bounds))
            continue;

        if(leaf->type == ObjectType_Entity)
        {
            if(leaf->ent->IsOf(GetClass(MeshEntity)))
            {
                MeshEntity *meshEnt = (MeshEntity*)leaf->ent;

                if(meshEnt->bRenderable)
                {
                    EntityVisible(meshEnt) = TRUE;
                    meshOrder.AddEntity(meshEnt);
                }
            }
            else if(leaf->ent->IsOf(GetClass(Projector)))
            {
                Projector *projector = (Projector*)leaf->ent;
                if(projector->bRenderable)
                {
                    if(projector->IsOf(GetClass(LitDecal)))
                        renderLitDecals << (LitDecal*)projector;
                    else
                        renderProjectors << projector;

                    EntityVisible(projector) = TRUE;
                }
            }
            else if(leaf->ent->IsOf(GetClass(Light)))
            {
                Light *light = (Light*)leaf->ent;

                if(light->IsOf(GetClass(SpotLight)))
                {
                    SpotLight *spotLight = (SpotLight*)light;
                    if(spotLight->IsLightmapped() && !spotLight->NumLitEntities())
                        continue;

                    renderSpotLights << spotLight;
                }
                else if(light->IsOf(GetClass(PointLight)))
                {
                    PointLight *pointLight = (PointLight*)light;
                    if(pointLight->IsLightmapped() && !pointLight->NumLitEntities())
                        continue;

                    renderPointLights << pointLight;
                }
            }
            else
            {
                Entity *ent = leaf->ent;

                if(ent->bRenderable)
                {
                    EntityVisible(ent) = TRUE;
                    renderEntities << ent;
                }
            }
        }
        else if(leaf->type == ObjectType_Brush)
        {
            Brush *brush = leaf->brush;

            brush->bVisible = TRUE;
            renderBrushes << brush;
        }
    }
}

void OctLevel::CalculateRenderOrder(Camera *camera, const ViewClip &clip)
{
    traceIn(OctLevel::CalculateRenderOrder);
    profileSegment("OctLevel::CalculateRenderOrder");

    assert(camera);

    if(!bLoaded) return;

    //--------------------------------------------------------------

    MeshOrderInfo meshOrder;
    CalcRenderOrderRecurse(camera->GetWorldPos(), clip, meshOrder, objectTree->GetRoot());

    //meshOrder.Sort();
    for(int i=meshOrder.renderOrder.Num()-1; i>=0; i--)
    {
        if(meshOrder.renderOrder[i].mesh->bHasAnimation)
            renderAnimatedMeshes.InsertList(0, (const List<AnimatedEntity*>&)meshOrder.renderOrder[i].RenderItems);
        else
            renderMeshes.InsertList(0, meshOrder.renderOrder[i].RenderItems);
    }

    traceOut;
}

void OctLevel::GetStaticGeometry(const Bounds &bounds, List<Brush*> &Brushes, List<MeshEntity*> &StaticMeshes)
{
    List<LevelObject*> Leaves;

    GetBoundsIntersectingLeaves(bounds, Leaves, objectTree->GetRoot());

    for(int i=0; i<Leaves.Num(); i++)
    {
        LevelObject *leaf = Leaves[i];

        if(leaf->type == ObjectType_Entity)
        {
            MeshEntity *meshEnt = (MeshEntity*)leaf->ent;
            if(meshEnt->bRenderable && meshEnt->bStaticGeometry)
                StaticMeshes << meshEnt;
        }
        else if(leaf->type == ObjectType_Brush)
            Brushes << leaf->brush;
    }
}

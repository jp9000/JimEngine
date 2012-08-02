/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  IndoorRender.cpp

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



void IndoorLevel::CalculateRenderOrder(Camera *camera, const ViewClip &clip)
{
    traceIn(IndoorLevel::CalculateRenderOrder);

    assert(camera);

    if(!bLoaded) return;

    List<PVS*> renderPVSs;

    IndoorEntityData *cameraData = GetIndoorEntityData(camera);
    int i, j;

    if(cameraData->PVSRefs.Num() && camera->IsPerspective())
    {
        for(i=0; i<cameraData->PVSRefs.Num(); i++)
        {
            DWORD cameraPVS = cameraData->PVSRefs[i];
            GetVisiblePVSs(PVSList[cameraPVS], NULL, camera->GetWorldPos(), &curClip, renderPVSs);
        }
    }
    else
    {
        for(i=0; i<Light::NumLights(); i++)
        {
            Light *light = Light::GetLight(i);

            if(light->IsOf(GetClass(DirectionalLight)))
                EntityVisible(light) = TRUE;
            else
                EntityVisible(light) = curClip.SphereVisible(light->GetWorldPos(), static_cast<PointLight*>(light)->lightRange);
        }

        for(i=0; i<PVSList.Num(); i++)
        {
            if(curClip.BoundsVisible(PVSList[i].bounds))
            {
                PVS &pvs = PVSList[i];

                renderPVSs << &pvs;
                for(j=0; j<pvs.entities.Num(); j++)
                {
                    Entity      *ent = pvs.entities[j];

                    if(ent->IsOf(GetClass(MeshEntity)))
                    {
                        if(!EntityVisible(ent))
                        {
                            MeshEntity *ment = (MeshEntity*)ent;
                            Matrix      mObjectInverse;

                            EntityVisible(ent) = curClip.BoundsVisible(ment->GetMeshBounds().GetTransformedBounds(ment->GetInvTransform()));
                        }
                    }
                }
                for(j=0; j<pvs.BrushRefs.Num(); j++)
                {
                    Brush &brush = BrushList[pvs.BrushRefs[j]];
                    if(!brush.bVisible)
                        brush.bVisible = TRUE;
                }
                pvs.bVisible = TRUE;
            }
        }
    }

    //--------------------------------------------------------------

    MeshOrderInfo meshOrder;

    for(i=0; i<renderPVSs.Num(); i++)
    {
        PVS *pvs = renderPVSs[i];

        for(j=0; j<pvs->visMeshEntities.Num(); j++)
        {
            MeshEntity *meshEnt = pvs->visMeshEntities[j];
            if(AlreadyUsed(meshEnt)) continue;
            AlreadyUsed(meshEnt) = TRUE;

            if(meshEnt->bRenderable && (EntityVisible(meshEnt) || meshEnt->bAlwaysVisible))
                meshOrder.AddEntity(meshEnt);
        }

        for(j=0; j<pvs->visEntities.Num(); j++)
        {
            Entity *ent = pvs->visEntities[j];
            if(AlreadyUsed(ent)) continue;
            AlreadyUsed(ent) = TRUE;

            if(ent->bRenderable && (EntityVisible(ent) || ent->bAlwaysVisible))
            {
                if(ent->IsOf(GetClass(Projector)))
                {
                    if(ent->IsOf(GetClass(LitDecal)))
                        renderLitDecals << (LitDecal*)ent;
                    else
                        renderProjectors << (Projector*)ent;
                }
                else
                    renderEntities << ent;
            }
        }

        for(j=0; j<pvs->BrushRefs.Num(); j++)
        {
            Brush &brush = BrushList[pvs->BrushRefs[j]];
            if(brush.bAlreadyUsed) continue;
            brush.bAlreadyUsed = TRUE;

            if(brush.bVisible)
                renderBrushes << &brush;
        }

        renderBrushes << pvs;
    }

    for(i=0; i<renderPVSs.Num(); i++)
    {
        PVS *pvs = renderPVSs[i];
        for(j=0; j<pvs->visMeshEntities.Num(); j++)
            AlreadyUsed(pvs->visMeshEntities[j]) = FALSE;
        for(j=0; j<pvs->visEntities.Num(); j++)
            AlreadyUsed(pvs->visEntities[j]) = FALSE;
        for(j=0; j<pvs->BrushRefs.Num(); j++)
            BrushList[pvs->BrushRefs[j]].bAlreadyUsed = FALSE;
    }

    SetCurrentVisibleSets(renderPVSs.Num());

    //--------------------------------------------------------------

    for(i=0; i<renderPVSs.Num(); i++)
    {
        PVS *pvs = renderPVSs[i];

        for(j=0; j<pvs->visLights.Num(); j++)
        {
            Light *light = pvs->visLights[j];

            if(AlreadyUsed(light)) continue;
            AlreadyUsed(light) = TRUE;

            if(light->IsOf(GetClass(SpotLight)))
            {
                SpotLight *spotLight = (SpotLight*)light;
                if(spotLight->IsLightmapped() && !spotLight->LitEntities.Num())
                    continue;

                renderSpotLights << spotLight;
            }
            else if(light->IsOf(GetClass(PointLight)))
            {
                PointLight *pointLight = (PointLight*)light;
                if(pointLight->IsLightmapped() && !pointLight->LitEntities.Num())
                    continue;

                renderPointLights << pointLight;
            }
        }
    }

    for(i=0; i<renderPVSs.Num(); i++)
    {
        PVS *pvs = renderPVSs[i];
        for(j=0; j<pvs->visLights.Num(); j++)
            AlreadyUsed(pvs->visLights[j]) = FALSE;
    }

    //--------------------------------------------------------------

    traceOut;
}

/*void IndoorLevel::RenderRefractiveObjects()
{
    traceInFast(IndoorLevel::RenderRefractiveObjects);

    int j;
    List<AddRef*> OrderedRefList;
    List<AddRef*> RefractiveRefList;

    for(j=0; j<BrushList.Num(); j++)
    {
        Brush *brush = &BrushList[j];
        if(brush->bRefractive && brush->bVisible)
            RefractiveRefList.SafeAdd(brush);
    }

    while(RefractiveRefList.Num())
    {
        float bestDist = -99999.0f;
        int bestRef = -1;
        for(j=0; j<RefractiveRefList.Num(); j++)
        {
            AddRef *brush = RefractiveRefList[j];
            float dist = brush->bounds.GetCenter().DistFromPlane(camPlane);
            if(dist > bestDist)
            {
                bestDist = dist;
                bestRef = j;
            }
        }

        OrderedRefList << RefractiveRefList[bestRef];
        RefractiveRefList.Remove(bestRef);
    }

    if(OrderedRefList.Num() && !bRenderRecursion)
    {
        Matrix camMatrix;
        camMatrix.SetIdentity();
        camMatrix.Rotate(-renderCamera->worldRot);
        camMatrix.Translate(-renderCamera->worldPos);

        float mat[16], brushMat[16], matPerspective[16];
        zero(brushMat, sizeof(brushMat));
        Matrix4x4Convert(mat, camMatrix);
        Matrix4x4Frustum(matPerspective, renderCamera->left, renderCamera->right, renderCamera->top, renderCamera->bottom, renderCamera->znear, 1000.0f);
        Matrix4x4Multiply(mat, mat, matPerspective);
        Matrix4x4Multiply(mat, mat, matScaleTrans);

        for(j=0; j<OrderedRefList.Num(); j++)
        {
            AddRef *brush = OrderedRefList[j];

            SetFrameBufferTarget(TempRenderTexture);
            ClearColorBuffer(TRUE, 0);

            LoadPixelShader(GetPixelShader(TEXT("Base:PS2.0/Refraction.txt")));
            LoadVertexShader(GetVertexShader(TEXT("Base:VS2.0/Refraction.txt")));

            Matrix brushMatrix;
            brushMatrix.SetIdentity();
            brushMatrix.Translate(brush->Pos);
            brushMatrix.Rotate(brush->Rot);

            Matrix4x4Convert(brushMat, brushMatrix);
            Matrix4x4Multiply(brushMat, brushMat, mat);
            Matrix4x4Transpose(brushMat, brushMat);

            SetVertexShaderConstant(5, brushMat, 4);

            EnableBlending(TRUE);
            EnableDepthTest(TRUE);
            DepthWriteEnable(FALSE);
            DepthFunction(GS_LESS);

            BlendFunction(GS_BLEND_ONE, GS_BLEND_ZERO);

            LoadTexture(MainRenderTexture2, 2);
            LoadDefault3DSampler(2);

            brush->RenderRefractive();

            LoadTexture(NULL, 2);

            LoadPixelShader(NULL);
            LoadVertexShader(NULL);

            SetFrameBufferTarget(MainRenderTexture2);

            BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
            GS->DrawSprite(TempRenderTexture, 0.0f, 0.0f, GS->Size.x, GS->Size.y);
        }
    }

    traceOutFast;
}*/


void Portal::Render(BOOL bDrawBack)
{
    GSCullMode cullMode;
    GSStencilOp zfail;

    //dwMode = (dwMode == GS_FRONT) ? GS_BACK : GS_FRONT;

    if(!bDrawBack)
    {
        if(level->UsingTwoSidedStencil())
        {
            GS->GetStencilOp(NULL, &zfail, NULL);
            StencilOp(GS_KEEP, ((zfail == GS_INCR) ? GS_DECR : GS_INCR), GS_KEEP);
            GS->StencilOpCCW(GS_KEEP, zfail, GS_KEEP);
        }
        else
        {
            cullMode = GetCullMode();
            SetCullMode((cullMode == GS_FRONT) ? GS_BACK : GS_FRONT);
        }
    }

    //---------------------------

    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(IdxBuffer);
    GS->DrawBare(GS_TRIANGLES);

    //---------------------------

    if(!bDrawBack)
    {
        if(level->UsingTwoSidedStencil())
        {
            StencilOp(GS_KEEP, zfail, GS_KEEP);
            GS->StencilOpCCW(GS_KEEP, ((zfail == GS_INCR) ? GS_DECR : GS_INCR), GS_KEEP);
        }
        else
            SetCullMode(cullMode);
    }
}

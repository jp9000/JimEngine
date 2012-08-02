/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  LevelRender.cpp

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


int bla = 0;

void Level::Draw(Camera *camera, BaseTexture *renderTarget, BOOL bRenderOnlyTexture)
{
    traceIn(Level::Draw);

    Matrix              m = camera->GetEntityTransform();
    XRect               glareViewport, mainViewport;
    int i, j;

    if(!bRenderRecursion)
    {
        GetViewport(mainViewport);

        glareViewport.x  = glareViewport.y = 0;
        glareViewport.cx = GlareRenderTexture->Width();
        glareViewport.cy = GlareRenderTexture->Height();
    }

    renderCamera = camera;

    curClip = camera->clip;
    curClip.Transform(m);

    MatrixSet(m.GetTranspose());

    //if(!bla)
    {
        CalculateRenderOrder(camera, curClip);
    }

    //if(++bla == 10) bla = 0;

    //--------------------------------------------------------------

    Plane               OldCamPlane;
    float               OldCamZ;

    XRect curVP;
    GS->GetViewport(curVP);

    profileIn("Light Update");

    if(!bRenderRecursion)
    {
        if(!engine->InEditor() && !bRenderOnlyTexture)
        {
            SetFrameBufferTarget(MainRenderTexture);
            ClearColorBuffer(TRUE);
        }

        camPlane.Dir = -camera->worldRot.GetDirectionVector();
        camPlane.Dist = camPlane.Dir|camera->worldPos;

        //-------------------------------------------------------

        for(j=0; j<Light::NumLights(); j++)
        {
            Light *light = Light::GetLight(j);

            if(light->IsOf(GetClass(DirectionalLight)))
            {
                if(light->IsOff())
                    continue;

                renderDirectionalLights << (DirectionalLight*)light;
            }
        }

        Entity *entIterator = Entity::FirstEntity();
        while(entIterator)
        {
            if(!EntityVisible(entIterator) && entIterator->bAlwaysVisible)
            {
                if(entIterator->IsOf(GetClass(AnimatedEntity)))
                    renderAnimatedMeshes << static_cast<AnimatedEntity*>(entIterator);
                else if(entIterator->IsOf(GetClass(MeshEntity)))
                    renderMeshes << static_cast<MeshEntity*>(entIterator);
                else if(!entIterator->IsOf(GetClass(Light)))
                    renderEntities << entIterator;
            }

            entIterator = entIterator->NextEntity();
        }

        for(j=0; j<renderMeshes.Num(); j++)
            renderMeshes[j]->PreRender();
        for(j=0; j<renderAnimatedMeshes.Num(); j++)
            renderAnimatedMeshes[j]->PreRender();
        for(j=0; j<renderEntities.Num(); j++)
            renderEntities[j]->PreRender();

        //-------------------------------------------------------

        BOOL bRenderTargetChanged = FALSE;

        EnableDepthTest(1);
        GS->DepthWriteEnable(1);
        DepthFunction(GS_LEQUAL);
        EnableBlending(0);
        ColorWriteEnable(1, 1, 1, 0);

        curEffect = shadowEffect;
        boneTransforms = curEffect->GetParameterByName(TEXT("boneTransforms"));

        MatrixPush();
        MatrixIdentity();

        SetCullMode(GS_FRONT);

        if(renderSpotLights.Num())
        {
            shadowEffect->BeginTechnique(shadowEffect->GetTechnique(TEXT("GetZDepth")));
            MatrixPush();

            for(i=0; i<renderSpotLights.Num(); i++)
            {
                SpotLight *spotLight = renderSpotLights[i];

                if(!spotLight->updateFaces || !spotLight->renderTexture)
                    continue;

                //--------------

                curRenderLight = spotLight;

                shadowEffect->SetVector(shadowEffect->GetParameterByName(TEXT("lightPos")),
                                        spotLight->GetWorldPos());
                shadowEffect->SetFloat(shadowEffect->GetParameterByName(TEXT("lightRange")),
                                       spotLight->lightRange);

                Texture *target = (Texture*)spotLight->renderTexture;

                SetFrameBufferTarget(target);
                SetViewport(0, 0, target->Width(), target->Height());
                ClearColorBuffer(TRUE, 0xFFFFFFFF);
                ClearDepthBuffer();

                Matrix rotMatrix;
                rotMatrix.SetIdentity();
                rotMatrix.Translate(spotLight->worldPos);
                rotMatrix.Rotate(spotLight->worldRot);

                ViewClip clip;
                clip.SetPerspective(spotLight->cutoff, 1.0f, 1.0, spotLight->lightRange+1.0f);
                clip.Transform(rotMatrix);

                double frustMin, frustMax;

                frustMax = 0.1f * tan(RAD(spotLight->cutoff)*0.5);
                frustMin = -frustMax;

                float pixelOffsetX = frustMax/(float)target->Width();
                float pixelOffsetY = frustMax/(float)target->Height();

                Frustum(frustMin+pixelOffsetX, frustMax+pixelOffsetX,
                        frustMin-pixelOffsetY, frustMax-pixelOffsetY,
                        0.1f, spotLight->lightRange);

                MatrixSet(rotMatrix.GetTranspose());
                RenderFromLight(spotLight, clip);
                if(GS->UseHardwareAnimation())
                    RenderFromLight(spotLight, clip, TRUE);

                bRenderTargetChanged = TRUE;

                spotLight->updateFaces = 0;
            }

            MatrixPop();
            shadowEffect->EndTechnique();
        }

        if(renderPointLights.Num())
        {
            shadowEffect->BeginTechnique(shadowEffect->GetTechnique(TEXT("GetDepth")));
            MatrixPush();

            for(i=0; i<renderPointLights.Num(); i++)
            {
                PointLight *pointLight = renderPointLights[i];

                if(!pointLight->updateFaces || !pointLight->renderTexture)
                    continue;

                //--------------

                CubeTexture *target = (CubeTexture*)pointLight->renderTexture;

                curRenderLight = pointLight;

                shadowEffect->SetVector(shadowEffect->GetParameterByName(TEXT("lightPos")),
                                        pointLight->GetWorldPos());
                shadowEffect->SetFloat(shadowEffect->GetParameterByName(TEXT("lightRange")),
                                       pointLight->lightRange);

                //WTF..  I can't believe I somehow managed to figure this out.
                //       IT ACCOUNTS FOR THE EXTRA TEXELS.  MY BRAIN IS MELTED.
                //       EXTRA TEXELS?!  EXTRA TERRESTRIALS?!?!!
                //       HA...  HAHHAHA..  HAHHHAHAHAHHAHAHAHAHAHAHAHAHAHAHAHAHA!!!!!!!!!
                //
                //       ...*dies violently*
                float pixelOffset = (0.1f/(float)target->Width());

                ViewClip clip;
                clip.SetFrustum(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, pointLight->lightRange+1.0f);
                Frustum(-0.1f+pixelOffset, 0.1f+pixelOffset,
                        -0.1f-pixelOffset, 0.1f-pixelOffset,
                        0.1f, pointLight->lightRange);

                Vect lightPos = pointLight->worldPos;

                for(j=0; j<6; j++)
                {
                    DWORD faceCheck = 1<<j;

                    if(pointLight->updateFaces & faceCheck)
                    {
                        SetFrameBufferTarget(pointLight->renderTexture, j);
                        SetViewport(0, 0, target->Width(), target->Width());
                        ClearColorBuffer(TRUE, 0xFFFFFFFF);
                        ClearDepthBuffer();

                        Vect dir(0.0f, 0.0f, 0.0f);
                        dir.ptr[j/2] = (j%2) ? -1.0f : 1.0f;

                        Quat rot;
                        rot.SetLookDirection(dir);

                        Matrix rotMatrix;
                        rotMatrix.SetIdentity();
                        rotMatrix.Translate(lightPos);
                        rotMatrix.Rotate(rot);

                        ViewClip copyClip;
                        copyClip = clip;
                        copyClip.Transform(rotMatrix);

                        MatrixSet(rotMatrix.GetTranspose());
                        RenderFromLight(pointLight, copyClip);
                        if(GS->UseHardwareAnimation())
                            RenderFromLight(pointLight, copyClip, TRUE);

                        bRenderTargetChanged = TRUE;
                    }
                }

                pointLight->updateFaces = 0;
            }

            MatrixPop();
            shadowEffect->EndTechnique();
        }

        curRenderLight = NULL;

        MatrixPop();

        SetCullMode(GS_BACK);
        ColorWriteEnable(TRUE, TRUE, TRUE, TRUE);

        if(bRenderTargetChanged)
        {
            if(bRenderOnlyTexture)
                SetFrameBufferTarget(NULL);
            else
                SetFrameBufferTarget(MainRenderTexture);
            SetViewport(curVP);
            ClearDepthBuffer(TRUE);
        }

        //-------------------------------------------------------

        renderCamera->LoadProjectionTransform();
    }
    else
    {
        OldCamPlane = camPlane;

        OldCamZ = curZValue;

        camPlane.Dir = -camera->worldRot.GetDirectionVector();
        camPlane.Dist = camPlane.Dir|camera->worldPos;
    }

    profileOut;

    profileIn("Main Level Drawing Stuff");
    
    ColorWriteEnable(1, 1, 1, 1);
    EnableBlending(FALSE);
    BlendFunction(GS_BLEND_ONE, GS_BLEND_ONE);

    EnableDepthTest(TRUE);
    DepthWriteEnable(TRUE);
    DepthFunction(GS_LESS);

    if(!bRenderOnlyTexture)
    {
        //-------------------------------------------------
        // Initial Pass

        profileIn("Initial pass");

        for(i=0; i<NumEffects(); i++)
        {
            curEffect = GetEffect(i);

            //-------------------------------------------------
            // Lightmapped

            HANDLE hTechnique = curEffect->GetTechnique(TEXT("Lightmap"));
            if(!hTechnique)
                continue;

            curEffect->BeginTechnique(hTechnique);

            lightmapU = curEffect->GetParameterByName(TEXT("lightmapU"));
            lightmapV = curEffect->GetParameterByName(TEXT("lightmapV"));
            lightmapW = curEffect->GetParameterByName(TEXT("lightmapW"));
            boneTransforms = curEffect->GetParameterByName(TEXT("boneTransforms"));

            curEffect->BeginPass(0);

            for(j=0; j<renderBrushes.Num(); j++)
            {
                Brush *brush = renderBrushes[j];
                if(brush->bLightmapped)
                    brush->RenderLightmaps();
            }
            for(j=0; j<renderMeshes.Num(); j++)
            {
                MeshEntity *ent = renderMeshes[j];
                if(ent->bLightmapped)
                    ent->RenderLightmaps();
            }

            curEffect->EndPass();
            curEffect->EndTechnique();

            //-------------------------------------------------
            // Non-Lightmapped

            hTechnique = curEffect->GetTechnique(TEXT("InitialPass"));
            if(!hTechnique)
                continue;

            curEffect->BeginTechnique(hTechnique);
            curEffect->BeginPassByName(TEXT("Normal"));

            for(j=0; j<renderBrushes.Num(); j++)
            {
                Brush *brush = renderBrushes[j];
                if(!brush->bLightmapped)
                    brush->RenderInitialPass();
            }
            for(j=0; j<renderMeshes.Num(); j++)
            {
                MeshEntity *ent = renderMeshes[j];
                if(!ent->bLightmapped)
                    ent->RenderInitialPass();
            }

            if(GS->UseHardwareAnimation())
            {
                curEffect->EndPass();
                curEffect->BeginPassByName(TEXT("Animated"));
            }

            for(j=0; j<renderAnimatedMeshes.Num(); j++)
            {
                MeshEntity *ent = renderAnimatedMeshes[j];
                ent->RenderInitialPass();
            }

            curEffect->EndPass();

            curEffect->EndTechnique();
        }

        profileOut;

        //-------------------------------------------------
        // Sphheeerical Harmonics!

        /*if(level->UsesLightmaps())
        {
            EnableBlending(1);
            DepthFunction(GS_EQUAL);
            DepthWriteEnable(0);
            BlendFunction(GS_BLEND_ONE, GS_BLEND_ONE);
            ColorWriteEnable(1, 1, 1, 1);

            curEffect = RM->GetInternalEffect(TEXT("Base:shlighting.effect"));
            boneTransforms = curEffect->GetParameterByName(TEXT("boneTransforms"));

            HANDLE hDiffuse = curEffect->GetParameterByName(TEXT("diffuseTexture"));
            HANDLE hNormalMap = curEffect->GetParameterByName(TEXT("normalMap"));
            HANDLE hMatrix = curEffect->GetWorld();
            HANDLE hScale = curEffect->GetScale();
            HANDLE hSH = curEffect->GetParameterByName(TEXT("shVal"));

            curEffect->BeginTechnique(curEffect->GetTechnique(TEXT("SH")));
            curEffect->BeginPass(0);

            for(j=0; j<renderMeshes.Num(); j++)
            {
                MeshEntity *meshEnt = renderMeshes[j];
                if(!meshEnt->bLightmapped && meshEnt->SH.Num())
                {
                    curEffect->SetMatrix(hMatrix, meshEnt->GetInvTransform());
                    curEffect->SetVector(hScale, meshEnt->scale);

                    curEffect->SetValue(hSH, meshEnt->SH.Array(), sizeof(CompactVect)*6);

                    LoadVertexBuffer(meshEnt->VertBuffer);
                    LoadIndexBuffer(meshEnt->mesh->IdxBuffer);

                    for(DWORD i=0;i<meshEnt->mesh->nSections;i++)
                    {
                        DrawSection &section  = meshEnt->mesh->SectionList[i];
                        Material    *material = meshEnt->MaterialList[i];

                        if(!material) continue;

                        curEffect->SetTexture(hDiffuse, material->GetCurrentTexture(TEXT("diffuseTexture")));
                        curEffect->SetTexture(hNormalMap, material->GetCurrentTexture(TEXT("normalMap")));

                        GS->Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
                    }
                }
            }

            for(j=0; j<renderAnimatedMeshes.Num(); j++)
            {
                MeshEntity *meshEnt = renderAnimatedMeshes[j];
                if(meshEnt->SH.Num())
                {
                    curEffect->SetMatrix(hMatrix, meshEnt->GetInvTransform());
                    curEffect->SetVector(hScale, meshEnt->scale);

                    curEffect->SetValue(hSH, meshEnt->SH.Array(), sizeof(CompactVect)*6);

                    LoadVertexBuffer(meshEnt->VertBuffer);
                    LoadIndexBuffer(meshEnt->mesh->IdxBuffer);

                    for(DWORD i=0;i<meshEnt->mesh->nSections;i++)
                    {
                        DrawSection &section  = meshEnt->mesh->SectionList[i];
                        Material    *material = meshEnt->MaterialList[i];

                        if(!material) continue;

                        curEffect->SetTexture(hDiffuse, material->GetCurrentTexture(TEXT("diffuseTexture")));
                        curEffect->SetTexture(hNormalMap, material->GetCurrentTexture(TEXT("normalMap")));

                        GS->Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
                    }
                }
            }

            curEffect->EndPass();
            curEffect->EndTechnique();
        }*/

        //---------------------------------------------

        profileIn("Individual Lights");

        LoadVertexShader(NULL);
        LoadPixelShader(NULL);

        SetCullMode(GS_BACK);
        EnableBlending(1);
        DepthFunction(GS_EQUAL);
        DepthWriteEnable(0);
        BlendFunction(GS_BLEND_ONE, GS_BLEND_ONE);

        ColorWriteEnable(1, 1, 1, 0);

        RenderLights();

        ColorWriteEnable(1, 1, 1, 1);
        DepthWriteEnable(1);
        DepthFunction(GS_LESS);
        BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);

        profileOut;
    }
    else //if rendering only textures.
    {
        profileSegment("rendering unlit!");

        curEffect = GetEffect(0);

        HANDLE hTechnique = curEffect->GetTechnique(TEXT("TextureOnly"));
        curEffect->BeginTechnique(hTechnique);
        curEffect->BeginPass(0);

        for(j=0; j<renderBrushes.Num(); j++)
        {
            Brush *brush = renderBrushes[j];
            brush->RenderInitialPass();
        }
        for(j=0; j<renderMeshes.Num(); j++)
        {
            MeshEntity *ent = renderMeshes[j];
            ent->RenderInitialPass();
        }
        for(j=0; j<renderAnimatedMeshes.Num(); j++)
        {
            MeshEntity *ent = renderAnimatedMeshes[j];
            ent->RenderInitialPass();
        }

        curEffect->EndPass();
        curEffect->EndTechnique();
    }

    profileOut;

    profileIn("Projector Rendering");

    //----------------------------------------------------
    // projectors

    ColorWriteEnable(1, 1, 1, 1);
    DepthWriteEnable(1);
    DepthFunction(GS_EQUAL);
    BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);

    Effect *lastEffect = NULL;
    int nPasses = 0;

    for(i=0; i<renderProjectors.Num(); i++)
    {
        Projector *projector = renderProjectors[i];

        if(projector->bRenderable && projector->bVisible)
        {
            Effect *projEffect = projector->effect;
            if(projEffect != lastEffect)
            {
                if(lastEffect)
                    lastEffect->EndTechnique();

                lastEffect = projEffect;
                HANDLE hTech = projEffect->GetTechnique(TEXT("Projector"));
                if(!hTech)
                {
                    lastEffect = NULL;
                    continue;
                }

                nPasses = projEffect->BeginTechnique(hTech);

                curEffect = projEffect;
            }

            Matrix projMat;
            projMat.SetIdentity();
            projMat.Translate(projector->worldPos);
            projMat.Rotate(projector->worldRot);

            float mat[16];
            Matrix4x4Convert(mat, projMat);
            Matrix4x4Multiply(mat, mat, projector->GetProjectionMatrix());
            Matrix4x4Multiply(mat, mat, matScaleTrans);

            ViewClip clip = projector->GetViewClip().GetTransform(projMat);

            projEffect->SetTexture(projEffect->GetParameterByName(TEXT("projTexture")), projector->texture);
            projEffect->SetFloat(projEffect->GetParameterByName(TEXT("projStart")), projector->GetStartDist());
            projEffect->SetFloat(projEffect->GetParameterByName(TEXT("projEnd")), projector->GetEndDist());

            projector->LoadProjector();

            HANDLE hProjMatrix = projEffect->GetParameterByName(TEXT("projMatrix"));
            HANDLE hProjPlane = projEffect->GetParameterByName(TEXT("projPlane"));

            Vect projDir = -projector->GetWorldRot().GetDirectionVector();
            Plane projPlane(projDir, projDir.Dot(projector->GetWorldPos()));

            for(int pass=0; pass<nPasses; pass++)
            {
                projEffect->BeginPass(pass);

                for(j=0; j<projector->MeshTargets.Num(); j++)
                {
                    MeshEntity *ent = projector->MeshTargets[j];
                    if(ent->bStaticGeometry && ent->bVisible)
                    {
                        if(!clip.GetTransform(ent->GetInvTransform()).BoundsVisible(ent->GetMeshBounds()))
                            continue;

                        float entMat[16];
                        Matrix4x4Convert(entMat, ent->invTransform);
                        Matrix4x4Multiply(entMat, entMat, mat);

                        Plane trasformedPlane = projPlane.GetTransform(ent->invTransform);
                        Vect4 v4Plane(trasformedPlane.Dir, trasformedPlane.Dist);

                        projEffect->SetMatrix(hProjMatrix, entMat);
                        projEffect->SetVector4(hProjPlane, v4Plane);

                        ent->QuickRender();
                    }
                }
                for(j=0; j<projector->BrushTargets.Num(); j++)
                {
                    Brush &brush = *projector->BrushTargets[j];

                    Vect4 v4Plane(projPlane.Dir, projPlane.Dist);

                    projEffect->SetMatrix(hProjMatrix, mat);
                    projEffect->SetVector4(hProjPlane, v4Plane);

                    if(brush.bVisible)
                    {
                        if(!clip.BoundsVisible(brush.bounds))
                            continue;

                        brush.QuickRender();
                    }
                }

                curEffect->EndPass();
            }
        }
    }

    if(lastEffect)
        lastEffect->EndTechnique();

    curEffect = NULL;

    DepthFunction(GS_LESS);

    profileOut;

    //----------------------------------------------------

    profileIn("Post-Processing");

    DepthWriteEnable(1);
    LoadPixelShader(NULL);
    LoadVertexShader(NULL);

    Vect2 gdSize;
    GS->GetSize(gdSize);

    if(!bRenderRecursion)
    {
        if(!bRenderOnlyTexture)
        {
            MatrixPush();
            MatrixIdentity();

            DWORD dwFogColor = 0xFF000000;

            EnableBlending(1);
            BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
            ColorWriteEnable(1, 1, 1, 1);

            float aspect = gdSize.x/gdSize.y;

            float adjustU = (1.0f/(aspect*350.0f));
            float adjustV = (1.0f/350.0f);

            SetFrameBufferTarget(MainRenderTexture2);
            ClearColorBuffer(TRUE, dwFogColor);
            GS->DrawSprite(MainRenderTexture, 0.0f, 0.0f, gdSize.x, gdSize.y);

            if(!GS->OmitPostProcess())
            {
                EnableDepthTest(FALSE);
                EnableBlending(FALSE);
                BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);

                Vect2 viewportMin, viewportMax;

                viewportMin.x = float(mainViewport.x)/(gdSize.x-1);
                viewportMin.y = float(mainViewport.y)/(gdSize.y-1);
                viewportMax.x = float(mainViewport.x+mainViewport.cx)/(gdSize.x-1);
                viewportMax.y = float(mainViewport.y+mainViewport.cy)/(gdSize.y-1);

                postProcessEffect->SetFloat(postProcessEffect->GetParameterByName(TEXT("aspect")), aspect);
                postProcessEffect->SetVector2(postProcessEffect->GetParameterByName(TEXT("viewportMin")), viewportMin);
                postProcessEffect->SetVector2(postProcessEffect->GetParameterByName(TEXT("viewportMax")), viewportMax);

                //----------------------------------

                postProcessEffect->BeginTechnique(postProcessEffect->GetTechnique(TEXT("Bloom")));

                //----------------------------------

                SetFrameBufferTarget(GlareRenderTexture);
                SetViewport(glareViewport);
                ClearColorBuffer(TRUE, dwFogColor);

                postProcessEffect->BeginPass(0);
                GS->DrawSprite(MainRenderTexture, 0.0f, 0.0f, glareViewport.cx, glareViewport.cy);
                postProcessEffect->EndPass();

                //----------------------------------

                SetFrameBufferTarget(GlareRenderTexture2);

                postProcessEffect->BeginPass(1);
                GS->DrawSprite(GlareRenderTexture, 0.0f, 0.0f, glareViewport.cx, glareViewport.cy);
                postProcessEffect->EndPass();

                //----------------------------------

                SetFrameBufferTarget(GlareRenderTexture);

                postProcessEffect->BeginPass(2);
                GS->DrawSprite(GlareRenderTexture2, 0.0f, 0.0f, glareViewport.cx, glareViewport.cy);
                postProcessEffect->EndPass();

                //----------------------------------

                postProcessEffect->EndTechnique();

                EnableBlending(TRUE);

                SetFrameBufferTarget(MainRenderTexture2);
                SetViewport(mainViewport);

                BlendFunction(GS_BLEND_ONE, GS_BLEND_ONE);

                GS->DrawSprite(GlareRenderTexture, 0, 0, gdSize.x, gdSize.y);
            }

            //RenderRefractiveObjects();

            BlendFunction(GS_BLEND_ONE, GS_BLEND_ZERO);

            if(renderTarget)
            {
                SetFrameBufferTarget(renderTarget);
                if(renderTarget->IsOf(GetClass(Texture)))
                {
                    Texture *renderTex = (Texture*)renderTarget;
                    GS->DrawSprite(MainRenderTexture2, 0.0f, 0.0f, renderTex->Width(), renderTex->Height());
                }
                else if(renderTarget->IsOf(GetClass(CubeTexture)))
                {
                    CubeTexture *renderTex = (CubeTexture*)renderTarget;
                    GS->DrawSprite(MainRenderTexture2, 0.0f, 0.0f, renderTex->Width(), renderTex->Width());
                }
            }
            else
            {
                SetFrameBufferTarget(NULL);
                GS->DrawSprite(MainRenderTexture2, 0.0f, 0.0f, gdSize.x, gdSize.y);
            }

            MatrixPop();
        }


        //---------------------------------------------

        EnableBlending(FALSE);
        EnableDepthTest(TRUE);

        //////////////////////////////////////////////////
        // Entity Post Render
        EnableDepthTest(TRUE);
        EnableBlending(TRUE);
        SetCullMode(GS_BACK);
        BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);

        for(j=0; j<renderEntities.Num(); j++)
            renderEntities[j]->Render();

        EnableDepthTest(TRUE);

        if(renderTarget)
            SetFrameBufferTarget(NULL);

        for(i=0; i<renderMeshes.Num(); i++)
            renderMeshes[i]->PostRender();
        for(i=0; i<renderAnimatedMeshes.Num(); i++)
            renderAnimatedMeshes[i]->PostRender();
        for(i=0; i<renderEntities.Num(); i++)
            renderEntities[i]->PostRender();
    }
    else
    {
        camPlane = OldCamPlane;
        curZValue = OldCamZ;
    }

    ClearRenderItems();

    profileOut;

    traceOut;
}

void Level::DrawWireframe(Camera *camera)
{
    traceIn(Level::DrawWireframe);

    Matrix              m = camera->GetEntityTransform();
    XRect               mainViewport;
    int i;

    if(!bRenderRecursion)
        GetViewport(mainViewport);

    renderCamera = camera;

    curClip = camera->clip;
    curClip.Transform(m);

    MatrixSet(m.GetTranspose());

    ClearRenderItems();
    CalculateRenderOrder(camera, curClip);

    //--------------------------------------------------------------

    Shader *solidColor = GetVertexShader(TEXT("Base:SolidColor.vShader"));
    LoadVertexShader(solidColor);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    for(i=0; i<renderAnimatedMeshes.Num(); i++)
        renderAnimatedMeshes[i]->RenderWireframe();
    for(i=0; i<renderMeshes.Num(); i++)
        renderMeshes[i]->RenderWireframe();
    for(i=0; i<renderBrushes.Num(); i++)
        renderBrushes[i]->RenderWireframe();

    //--------------------------------------------------------------

    traceOut;
}

void Level::ClearRenderItems()
{
    int i;

    for(i=0; i<renderAnimatedMeshes.Num(); i++)
        renderAnimatedMeshes[i]->bVisible = FALSE;
    for(i=0; i<renderMeshes.Num(); i++)
        renderMeshes[i]->bVisible = FALSE;
    for(i=0; i<renderBrushes.Num(); i++)
        renderBrushes[i]->bVisible = FALSE;

    for(i=0; i<renderProjectors.Num(); i++)
        renderProjectors[i]->bVisible = FALSE;
    for(i=0; i<renderLitDecals.Num(); i++)
        renderLitDecals[i]->bVisible = FALSE;
    for(i=0; i<renderSpotLights.Num(); i++)
        renderSpotLights[i]->bVisible = FALSE;
    for(i=0; i<renderPointLights.Num(); i++)
        renderPointLights[i]->bVisible = FALSE;
    for(i=0; i<renderEntities.Num(); i++)
        renderEntities[i]->bVisible = FALSE;

    renderDirectionalLights.Clear();
    renderSpotLights.Clear();
    renderPointLights.Clear();
    renderAnimatedMeshes.Clear();
    renderMeshes.Clear();
    renderBrushes.Clear();
    renderEntities.Clear();
    renderProjectors.Clear();
    renderLitDecals.Clear();
}

void Level::QuickRender()
{
    traceInFast(Level::QuickRender);

    for(int i=0; i<renderAnimatedMeshes.Num(); i++)
        renderAnimatedMeshes[i]->QuickRender();
    for(int i=0; i<renderMeshes.Num(); i++)
        renderMeshes[i]->QuickRender();
    for(int i=0; i<renderBrushes.Num(); i++)
        renderBrushes[i]->QuickRender();

    traceOutFast;
}


void Brush::Render()
{
    traceInFast(Brush::Render);

    DWORD j;

    if(GetActiveEffect())
        GetActiveEffect()->SetMatrixIdentity(GetActiveEffect()->GetWorld());

    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(IdxBuffer);

    for(j=0; j<nSections; j++)
    {
        Material *material = Materials[j];

        if(!material)
            material = level->defaultMaterial;

        if(material->effect == level->curEffect)
        {
            if(!material->LoadParameters())
                continue;

            if(SectionList[j].numFaces)
            {
                if(material->flags & MATERIAL_TWOSIDED)
                {
                    GSCullMode cullMode;
                    cullMode = GetCullMode();
                    SetCullMode(GS_NEITHER);
                    GS->Draw(GS_TRIANGLES, 0, SectionList[j].startFace*3, SectionList[j].numFaces*3);
                    SetCullMode(cullMode);
                }
                else
                    GS->Draw(GS_TRIANGLES, 0, SectionList[j].startFace*3, SectionList[j].numFaces*3);
            }
        }
    }

    traceOutFast;
}

void Brush::RenderLight(Light *light)
{
    traceInFast(Brush::RenderLight);


    if(light->IsOf(GetClass(DirectionalLight)))
        Render();
    else
    {
        float range;
        if(light->IsOf(GetClass(SpotLight)))
            range = static_cast<SpotLight*>(light)->lightRange;
        else if(light->IsOf(GetClass(PointLight)))
            range = static_cast<PointLight*>(light)->lightRange;

        if(bounds.SphereIntersects(light->GetWorldPos(), range))
            Render();
    }

    traceOutFast;
}


void Brush::RenderLightmaps()
{
    traceInFast(Brush::RenderLightmaps);

    DWORD i;

    if(GetActiveEffect())
        GetActiveEffect()->SetMatrixIdentity(GetActiveEffect()->GetWorld());

    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(IdxBuffer);

    HANDLE param[3];

    param[0] = level->curEffect->GetParameterByName(TEXT("lightmapU"));
    param[1] = level->curEffect->GetParameterByName(TEXT("lightmapV"));
    param[2] = level->curEffect->GetParameterByName(TEXT("lightmapW"));

    if(lightmap.X)
    {
        level->curEffect->SetTexture(param[0], lightmap.X);
        level->curEffect->SetTexture(param[1], lightmap.Y);
        level->curEffect->SetTexture(param[2], lightmap.Z);
    }
    else
    {
        Texture *black = GetTexture(TEXT("Base:Default/black.tga"));
        level->curEffect->SetTexture(param[0], black);
        level->curEffect->SetTexture(param[1], black);
        level->curEffect->SetTexture(param[2], black);
    }

    for(i=0; i<nSections; i++)
    {
        DrawSection &section = SectionList[i];
        Material *material = Materials[i];

        if(!material)
            material = level->defaultMaterial;

        if(material->effect == level->curEffect)
        {
            if(!material->LoadParameters())
                continue;

            GS->Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
        }
    }

    traceOutFast;
}


void Brush::RenderInitialPass()
{
    traceInFast(Brush::RenderInitialPass);

    DWORD j;
    BOOL bLightFound = 0;

    if(GetActiveEffect())
        GetActiveEffect()->SetMatrixIdentity(GetActiveEffect()->GetWorld());

    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(IdxBuffer);

    for(j=0; j<nSections; j++)
    {
        if(SectionList[j].numFaces)
        {
            Material *material = Materials[j];

            if(!material)
                material = level->defaultMaterial;

            if(material->effect == level->curEffect)
            {
                if(!material->LoadParameters())
                    continue;

                if(material->flags & MATERIAL_TWOSIDED)
                {
                    GSCullMode cullMode;
                    cullMode = GetCullMode();
                    SetCullMode(GS_NEITHER);
                    GS->Draw(GS_TRIANGLES, 0, SectionList[j].startFace*3, SectionList[j].numFaces*3);
                    SetCullMode(cullMode);
                }
                else
                    GS->Draw(GS_TRIANGLES, 0, SectionList[j].startFace*3, SectionList[j].numFaces*3);
            }
        }
    }

    traceOutFast;
}

void Brush::QuickRender()
{
    traceInFast(Brush::QuickRender);

    if(GetActiveEffect())
        GetActiveEffect()->SetMatrixIdentity(GetActiveEffect()->GetWorld());

    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(IdxBuffer);
    DWORD j;

    for(j=0; j<nSections; j++)
    {
        Material *material = Materials[j];

        if(!material)
            material = level->defaultMaterial;

        if(SectionList[j].numFaces)
        {
            if(material->flags & MATERIAL_TWOSIDED)
            {
                GSCullMode cullMode;
                cullMode = GetCullMode();
                SetCullMode(GS_NEITHER);
                GS->Draw(GS_TRIANGLES, 0, SectionList[j].startFace*3, SectionList[j].numFaces*3);
                SetCullMode(cullMode);
            }
            else
                GS->Draw(GS_TRIANGLES, 0, SectionList[j].startFace*3, SectionList[j].numFaces*3);
        }
    }

    traceOutFast;
}

void Brush::RenderRefractive()
{
    traceInFast(Brush::RenderRefractive);

//    LoadVertexBuffer(VertBuffer);
//    LoadIndexBuffer(IdxBuffer);
//    DWORD j;
//
//    for(j=0; j<nSections; j++)
//    {
//        Material &material = level->MaterialList[Materials[j]];
//
//        if(SectionList[j].numFaces)
//        {
//            DWORD specular = material.specular;
//            BOOL  bTransparent = (material.flags & MATERIAL_TRANSPARENT);
//
//            if(bTransparent)
//                continue;
//
//            LoadTexture(material.texture);
//            LoadTexture(material.bumpmap, 1);
//            LoadDefault3DSampler(0);
//            LoadDefault3DSampler(1);
//
//            if(!(material.flags & MATERIAL_TWOSIDED))
//                GS->Draw(GS_TRIANGLES, 0, SectionList[j].startFace*3, SectionList[j].numFaces*3);
//
//            LoadTexture(NULL, 1);
//            LoadTexture(NULL);
//        }
//    }

    traceOutFast;
}


void Brush::RenderWireframe()
{
    traceInFast(Brush::RenderWireframe);

    assert(WireframeBuffer && engine->InEditor());
    if(!WireframeBuffer || !engine->InEditor())
        return;

    Shader *shader = GetCurrentVertexShader();
    if(shader)
        shader->SetColor(shader->GetParameter(1), 0.6f, 0.4f, 0.3f);

    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(WireframeBuffer);

    GS->DrawBare(GS_LINES);

    traceOutFast;
}

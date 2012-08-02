/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  LevelLighting.cpp:  World Lighting

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



void Level::RenderLights()
{
    traceInFast(Level::RenderLights);

    int i;

    shaderParams.lightPos           = curEffect->GetParameterByName(TEXT("lightPos"));
    shaderParams.lightDir           = curEffect->GetParameterByName(TEXT("lightDir"));
    shaderParams.lightRange         = curEffect->GetParameterByName(TEXT("lightRange"));
    shaderParams.lightColor         = curEffect->GetParameterByName(TEXT("lightColor"));

    shaderParams.attenuationMap     = curEffect->GetParameterByName(TEXT("attenuationMap"));

    shaderParams.spotlightMatrix    = curEffect->GetParameterByName(TEXT("spotlightMatrix"));
    shaderParams.spotlightMap       = curEffect->GetParameterByName(TEXT("spotlightMap"));

    shaderParams.spotDepthTexture   = curEffect->GetParameterByName(TEXT("spotDepthTexture"));
    shaderParams.pointDepthCube     = curEffect->GetParameterByName(TEXT("pointDepthCube"));

    HANDLE hTechnique;

    engine->nLights = renderSpotLights.Num() + renderPointLights.Num() + renderDirectionalLights.Num();

    for(i=0; i<NumEffects(); i++)
    {
        curEffect = GetEffect(i);
        boneTransforms = curEffect->GetParameterByName(TEXT("boneTransforms"));

        curEffect->SetVector(curEffect->GetParameterByName(TEXT("eyePos")), renderCamera->GetWorldPos());
        curEffect->SetFloat(curEffect->GetParameterByName(TEXT("shadowBias")), 0.001f);
        curEffect->SetTexture(shaderParams.attenuationMap, AttenuationMap);

        if(renderSpotLights.Num())
        {
            //----------------------------------------------------
            // render non-shadowed spotlights
            hTechnique = curEffect->GetTechnique(TEXT("SpotLight"));
            if(hTechnique)
            {
                curEffect->BeginTechnique(hTechnique);
                for(i=0; i<renderSpotLights.Num(); i++)
                {
                    SpotLight *curLight = renderSpotLights[i];

                    if(!curLight->CanCastShadows())
                        FullRender(curLight);
                }
                if(GS->UseHardwareAnimation())
                {
                    for(i=0; i<renderSpotLights.Num(); i++)
                    {
                        SpotLight *curLight = renderSpotLights[i];

                        if(!curLight->CanCastShadows())
                            FullRender(curLight, TRUE);
                    }
                }
            }

            //----------------------------------------------------
            // render shadowed spotlights
            hTechnique = curEffect->GetTechnique(TEXT("SpotLightShadow"));
            if(hTechnique)
            {
                curEffect->BeginTechnique(hTechnique);
                for(i=0; i<renderSpotLights.Num(); i++)
                {
                    SpotLight *curLight = renderSpotLights[i];

                    if(curLight->CanCastShadows())
                        FullRender(curLight);
                }
                if(GS->UseHardwareAnimation())
                {
                    for(i=0; i<renderSpotLights.Num(); i++)
                    {
                        SpotLight *curLight = renderSpotLights[i];

                        if(curLight->CanCastShadows())
                            FullRender(curLight, TRUE);
                    }
                }
            }
        }

        if(renderPointLights.Num())
        {
            //----------------------------------------------------
            // render shadowed pointlights
            hTechnique = curEffect->GetTechnique(TEXT("PointLightShadow"));
            if(hTechnique)
            {
                curEffect->BeginTechnique(hTechnique);
                for(i=0; i<renderPointLights.Num(); i++)
                {
                    PointLight *curLight = renderPointLights[i];

                    if(curLight->CanCastShadows())
                        FullRender(curLight);
                }
                if(GS->UseHardwareAnimation())
                {
                    for(i=0; i<renderPointLights.Num(); i++)
                    {
                        PointLight *curLight = renderPointLights[i];

                        if(curLight->CanCastShadows())
                            FullRender(curLight, TRUE);
                    }
                }
            }

            //----------------------------------------------------
            // render non-shadowed pointlights
            hTechnique = curEffect->GetTechnique(TEXT("PointLight"));
            if(hTechnique)
            {
                curEffect->BeginTechnique(hTechnique);
                for(i=0; i<renderPointLights.Num(); i++)
                {
                    PointLight *curLight = renderPointLights[i];

                    if(!curLight->CanCastShadows())
                        FullRender(curLight);
                }
                if(GS->UseHardwareAnimation())
                {
                    for(i=0; i<renderPointLights.Num(); i++)
                    {
                        PointLight *curLight = renderPointLights[i];

                        if(!curLight->CanCastShadows())
                            FullRender(curLight, TRUE);
                    }
                }
            }
        }


        if(renderDirectionalLights.Num())
        {
            //----------------------------------------------------
            // render directionallights
            hTechnique = curEffect->GetTechnique(TEXT("DirectionalLight"));
            if(hTechnique)
            {
                curEffect->BeginTechnique(hTechnique);
                for(i=0; i<renderDirectionalLights.Num(); i++)
                {
                    DirectionalLight *curLight = renderDirectionalLights[i];

                    if(!curLight->CanCastShadows())
                        FullRender(curLight);
                }

                if(GS->UseHardwareAnimation())
                {
                    for(i=0; i<renderDirectionalLights.Num(); i++)
                    {
                        DirectionalLight *curLight = renderDirectionalLights[i];

                        if(!curLight->CanCastShadows())
                            FullRender(curLight, TRUE);
                    }
                }
            }
        }

        curEffect->EndTechnique();
    }

    curRenderLight = NULL;

    traceOutFast;
}

void Level::FullRender(Light *curLight, BOOL bAnimation)
{
    traceInFast(Level::FullRender);

    curRenderLight = curLight;

    float mat[16];

    curEffect->SetVector(shaderParams.lightPos, curLight->GetWorldPos());
    curEffect->SetColor3(shaderParams.lightColor, RGB_to_Vect(curLight->color)*(float(curLight->intensity)*0.01f));

    HANDLE hPass = bAnimation ? curEffect->GetPassByName(NULL, TEXT("Animated")) : curEffect->GetPassByName(NULL, TEXT("Normal"));

    if(curLight->IsOf(GetClass(SpotLight)))
    {
        SpotLight *light = (SpotLight*)curLight;

        curEffect->SetTexture(shaderParams.spotDepthTexture, light->renderTexture);
        curEffect->SetTexture(shaderParams.spotlightMap, light->spotTexture);
        curEffect->SetFloat(shaderParams.lightRange, light->lightRange);

        float matPerspective[16];

        Matrix lightMatrix;
        lightMatrix.SetIdentity();
        lightMatrix.Translate(light->worldPos);
        lightMatrix.Rotate(light->worldRot);

        Matrix4x4Convert(spotlightMatrix, lightMatrix);
        Matrix4x4Perspective(matPerspective, light->cutoff, 1.0f, 0.1f, light->lightRange);
        Matrix4x4Multiply(spotlightMatrix, spotlightMatrix, matPerspective);
        Matrix4x4Multiply(spotlightMatrix, spotlightMatrix, matScaleTrans);

        curEffect->BeginPassByHandle(hPass);

        for(int j=0; j<light->LitEntities.Num(); j++)
        {
            MeshEntity *ent = light->GetLitEntity(j);

            if(EntityVisible(ent))
            {
                if(GS->UseHardwareAnimation())
                {
                    BOOL bEntityIsAnimated = FALSE;
                    if(ent->IsOf(GetClass(AnimatedEntity)))
                        bEntityIsAnimated = static_cast<AnimatedEntity*>(ent)->GetSkinMesh()->bHasAnimation;

                    if(bAnimation != bEntityIsAnimated)
                        continue;
                }

                if(light->bLightmapped && ent->bLightmapped)
                    continue;

                Matrix4x4Convert(mat, ent->invTransform);
                Matrix4x4Multiply(mat, mat, spotlightMatrix);
                Matrix4x4Transpose(mat, mat);

                curEffect->SetMatrix(shaderParams.spotlightMatrix, mat);
                ent->WorldRender();
            }
        }

        if(!GS->UseHardwareAnimation() || !bAnimation)
        {
            mcpy(mat, spotlightMatrix, 64);
            Matrix4x4Transpose(mat, mat);

            curEffect->SetMatrix(shaderParams.spotlightMatrix, mat);

            for(int j=0; j<light->NumLitBrushes(); j++)
            {
                Brush *brush = light->GetLitBrush(j);
                if(brush->bVisible)
                {
                    if(curLight->bLightmapped && brush->bLightmapped)
                        continue;
                    brush->RenderLight(light);
                }
            }
        }

        curEffect->EndPass();
    }
    else if(curLight->IsOf(GetClass(PointLight)))
    {
        PointLight *light = (PointLight*)curLight;
        curEffect->SetTexture(shaderParams.pointDepthCube, light->renderTexture);
        curEffect->SetFloat(shaderParams.lightRange, light->lightRange);

        curEffect->BeginPassByHandle(hPass);

        for(int j=0; j<light->LitEntities.Num(); j++)
        {
            MeshEntity *ent = light->GetLitEntity(j);

            if(EntityVisible(ent))
            {
                if(GS->UseHardwareAnimation())
                {
                    /*BOOL bAnimatedEntity = ent->IsOf(GetClass(AnimatedEntity));
                    if(bAnimation != bAnimatedEntity)
                        continue;*/
                    BOOL bEntityIsAnimated = FALSE;
                    if(ent->IsOf(GetClass(AnimatedEntity)))
                        bEntityIsAnimated = static_cast<AnimatedEntity*>(ent)->GetSkinMesh()->bHasAnimation;

                    if(bAnimation != bEntityIsAnimated)
                        continue;
                }

                if(light->bLightmapped && ent->bLightmapped)
                    continue;
                ent->WorldRender();
            }
        }

        if(!GS->UseHardwareAnimation() || !bAnimation)
        {
            for(int j=0; j<light->NumLitBrushes(); j++)
            {
                Brush *brush = light->GetLitBrush(j);
                if(brush->bVisible)
                {
                    if(curLight->bLightmapped && brush->bLightmapped)
                        continue;
                    brush->RenderLight(light);
                }
            }
        }

        curEffect->EndPass();
    }
    else if(curLight->IsOf(GetClass(DirectionalLight)))
    {
        DirectionalLight *light = (DirectionalLight*)curLight;

        curEffect->SetVector(shaderParams.lightDir, light->GetWorldRot().GetDirectionVector());

        curEffect->BeginPassByHandle(hPass);

        for(int j=0; j<renderMeshes.Num(); j++)
        {
            MeshEntity *ent = renderMeshes[j];

            if(EntityVisible(ent))
            {
                if(GS->UseHardwareAnimation())
                {
                    BOOL bEntityIsAnimated = FALSE;
                    if(ent->IsOf(GetClass(AnimatedEntity)))
                        bEntityIsAnimated = static_cast<AnimatedEntity*>(ent)->GetSkinMesh()->bHasAnimation;

                    if(bAnimation != bEntityIsAnimated)
                        continue;
                }

                if(light->bLightmapped && ent->bLightmapped)
                    continue;
                ent->WorldRender();
            }
        }

        if(!GS->UseHardwareAnimation() || !bAnimation)
        {
            for(int j=0; j<renderBrushes.Num(); j++)
            {
                Brush *brush = renderBrushes[j];
                if(brush->bVisible)
                {
                    if(curLight->bLightmapped && brush->bLightmapped)
                        continue;
                    brush->RenderLight(light);
                }
            }
        }

        curEffect->EndPass();
    }

    traceOutFast;
}

void Level::RenderFromLight(Light *light, ViewClip &clip, BOOL bAnimation)
{
    traceInFast(Level::RenderFromLight);

    HANDLE hPass = bAnimation ? curEffect->GetPassByName(NULL, TEXT("Animated")) : curEffect->GetPassByName(NULL, TEXT("Normal"));
    curEffect->BeginPassByHandle(hPass);

    for(int i=0; i<light->NumLitEntities(); i++)
    {
        MeshEntity *meshent = light->GetLitEntity(i);
        if(GS->UseHardwareAnimation())
        {
            BOOL bEntityIsAnimated = FALSE;
            if(meshent->IsOf(GetClass(AnimatedEntity)))
                bEntityIsAnimated = static_cast<AnimatedEntity*>(meshent)->GetSkinMesh()->bHasAnimation;

            if(bAnimation != bEntityIsAnimated)
                continue;
        }

        if(meshent->bRenderable && meshent->CanCastShadow())
        {
            ViewClip copyClip;
            copyClip = clip;
            copyClip.Transform(meshent->GetInvTransform());

            if(copyClip.BoundsVisible(meshent->GetMeshBounds()))
                meshent->QuickRender();
        }
    }

    if(!GS->UseHardwareAnimation() || !bAnimation)
    {
        for(int i=0; i<light->NumLitBrushes(); i++)
        {
            Brush *brush = light->GetLitBrush(i);
            if(clip.BoundsVisible(brush->bounds))
                brush->QuickRender();
        }
    }

    curEffect->EndPass();

    traceOutFast;
}

void Level::CreateAttMap(BOOL bQuadratic)
{
    traceIn(Level::CreateAttMap);

    /*float *lpAttMap = (float*)Allocate(512*2*sizeof(float));
    for(DWORD x=0; x<512; x++)
    {
        DWORD pos = x*2;
        float attenuation = 1.0f-(float(x)/511.0f);
        if(attenuation < 0.0f) attenuation = 0.0f;

        if(bQuadratic)
            attenuation *= attenuation;

        attenuation = pow(attenuation, 0.8f);

        lpAttMap[pos] = lpAttMap[pos+1] = attenuation;
    }

    if(!GS->Use32FTextures())
    {
        Float16 *lpAttMap16 = (Float16*)Allocate(512*2*sizeof(Float16));
        FloatArrayTo16(lpAttMap, lpAttMap16, 512*2);

        AttenuationMap = CreateTexture(512, 1, GS_RG16F, lpAttMap16, FALSE);

        Free(lpAttMap16);
    }
    else
        AttenuationMap = CreateTexture(512, 1, GS_RG32F, lpAttMap, FALSE);

    Free(lpAttMap);*/

    float *lpAttMap = (float*)Allocate(512*sizeof(float));
    for(DWORD x=0; x<512; x++)
    {
        float attenuation = 1.0f-(float(x)/511.0f);
        if(attenuation < 0.0f) attenuation = 0.0f;

        if(bQuadratic)
            attenuation *= attenuation;

        attenuation = pow(attenuation, 0.8f);

        lpAttMap[x] = attenuation;
    }

    if(!GS->Use32FTextures())
    {
        WORD *lpAttMap16 = (WORD*)Allocate(512*sizeof(WORD));
        for(int i=0; i<512; i++)
            lpAttMap16[i] = WORD(lpAttMap[i]*float(0xFFFF));

        AttenuationMap = CreateTexture(512, 1, GS_L16, lpAttMap16, FALSE);

        Free(lpAttMap16);
    }
    else
        AttenuationMap = CreateTexture(512, 1, GS_RG32F, lpAttMap, FALSE);

    Free(lpAttMap);

    traceOut;
}


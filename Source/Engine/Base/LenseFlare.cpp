/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  LenseFlare.cpp

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

DefineClass(LenseFlare);


void LenseFlare::Init()
{
    traceIn(LenseFlare::Init);

    flares.SetSize(13);
    flareLoc.SetSize(13);

    bHidden = TRUE;
    fadeType = fadeValue = 0;

    for(int i=0; i<13; i++)
        flares[i] = CreateBare(Sprite3D);

    flares[0]->spriteTextureName  = TEXT("Base:LenseEffects/BigGlow.tga");
    flares[1]->spriteTextureName  = TEXT("Base:LenseEffects/BigGlow.tga");
    flares[2]->spriteTextureName  = TEXT("Base:LenseEffects/Halo.tga");
    flares[3]->spriteTextureName  = TEXT("Base:LenseEffects/Halo.tga");
    flares[4]->spriteTextureName  = TEXT("Base:LenseEffects/HardGlow.tga");
    flares[5]->spriteTextureName  = TEXT("Base:LenseEffects/Halo.tga");
    flares[6]->spriteTextureName  = TEXT("Base:LenseEffects/HardGlow.tga");
    flares[7]->spriteTextureName  = TEXT("Base:LenseEffects/Halo.tga");
    flares[8]->spriteTextureName  = TEXT("Base:LenseEffects/HardGlow.tga");
    flares[9]->spriteTextureName  = TEXT("Base:LenseEffects/BigGlow.tga");
    flares[10]->spriteTextureName = TEXT("Base:LenseEffects/Halo.tga");
    flares[11]->spriteTextureName = TEXT("Base:LenseEffects/BigGlow.tga");
    flares[12]->spriteTextureName = TEXT("Base:LenseEffects/Halo.tga");

    SetColor(0xFFFFFFFF);

    flares[0]->spriteSize  = 15.0f;
    flares[1]->spriteSize  = 0.5f;
    flares[2]->spriteSize  = 0.3f;
    flares[3]->spriteSize  = 0.7f;
    flares[4]->spriteSize  = 0.6f;
    flares[5]->spriteSize  = 0.5f;
    flares[6]->spriteSize  = 0.3f;
    flares[7]->spriteSize  = 0.5f;
    flares[8]->spriteSize  = 0.6f;
    flares[9]->spriteSize = 0.5f;
    flares[10]->spriteSize = 0.6f;
    flares[11]->spriteSize = 0.5f;
    flares[12]->spriteSize = 0.7f;
 
    flareLoc[0]  = 0.0f;
    flareLoc[1]  = 0.3f;
    flareLoc[2]  = 0.42f;
    flareLoc[3]  = 0.5f;
    flareLoc[4]  = 0.7f;
    flareLoc[5]  = 0.76f;
    flareLoc[6]  = 0.95f;
    flareLoc[7]  = 1.2f;
    flareLoc[8]  = 1.22f;
    flareLoc[9] = 1.35f;
    flareLoc[10] = 1.4f;
    flareLoc[11] = 1.5f;
    flareLoc[12] = 1.56f;

    for(int i=0; i<13; i++)
    {
        flares[i]->Init();
        flares[i]->bOmitDepthTest = TRUE;
        flares[i]->bAlwaysVisible = TRUE;
    }

    traceOut;
}

void LenseFlare::Destroy()
{
    flares.Clear();
    flareLoc.Clear();
}

void LenseFlare::PreRender()
{
    //Implement Collision
    /*CollisionInfo ci;
    Vect hitPos = GetWorldPos();

    if(engine->InEditor())
        bHidden = TRUE;
    else
    {
        Vect camPos = level->GetCurrentCamera()->GetWorldPos();
        Quat camRot = level->GetCurrentCamera()->GetWorldRot();

        bHidden = level->GetCollision(camPos, hitPos, ci);

        Vect flareDir = (GetWorldPos()-camPos).Norm();
        Vect camDir = -camRot.GetDirectionVector();

        Vect baseFlareOffset = flareDir*10.0f;
        baseFlareOffset += camPos;

        Vect baseCamOffset = camDir*10.0f;
        baseCamOffset += camPos;

        for(int i=0; i<13; i++)
            flares[i]->SetPos(Lerp(baseFlareOffset, baseCamOffset, flareLoc[i]));

        float angle = flareDir|camDir;

        if(angle < 0.0f)
            angle = 0.0f;
        else if(angle > 1.0f)
            angle = 1.0f;

        angle = powf(angle, 4.0f);

        fadeValue = angle;
    }*/
}

void LenseFlare::Tick(float fSeconds)
{
    Super::Tick(fSeconds);

    if(level)
    {
        if(fadeType == 1)
        {
            float increase = fSeconds*3.0f;
            float maxIncrease = (1.0-baseFadeValue);

            baseFadeValue += MIN(maxIncrease, increase);
        }
        else if(fadeType == 2)
        {
            float decrease = fSeconds*3.0f;

            baseFadeValue -= MIN(decrease, baseFadeValue);
        }

        if(bHidden)
        {
            if(baseFadeValue != 0.0f)
                fadeType = 2;
            else
                fadeType = 0;
        }
        else
        {
            if(baseFadeValue != 1.0f)
                fadeType = 1;
            else
                fadeType = 0;
        }

        if(fadeValue > 1.0f || baseFadeValue > 1.0f)
        {
            fadeValue = MIN(fadeValue, 1.0f);
            baseFadeValue = MIN(baseFadeValue, 1.0f);
            //AppWarning(TEXT("LenseFlare::Tick -- fadeValues bigger than 255?!  VZH!!!"));
        }

        SetColor(color);
    }
}

void LenseFlare::SetColor(DWORD newColor)
{
    Color3 vNewColor = RGB_to_Vect(newColor);

    flares[0]->spriteColor  = 0xAFFFFFFF;
    flares[1]->spriteColor  = 0x9FFFEFDF;
    flares[2]->spriteColor  = 0x9FFFDFCF;
    flares[3]->spriteColor  = 0x9FFFCFBF;
    flares[4]->spriteColor  = 0x9FAFFFEF;
    flares[5]->spriteColor  = 0x9FAFFFFF;
    flares[6]->spriteColor  = 0x9FCFFFFF;
    flares[7]->spriteColor  = 0x9FDFEFFF;
    flares[8]->spriteColor  = 0x9FBFFFFF;
    flares[9]->spriteColor = 0x9FFFBFCF;
    flares[10]->spriteColor = 0x9FFFCFBF;
    flares[11]->spriteColor = 0x9FCFCFFF;
    flares[12]->spriteColor = 0x9FBFCFFF;

    for(int i=0; i<13; i++)
    {
        Color4 vFlareColor = RGBA_to_Vect4(flares[i]->spriteColor);

        vFlareColor.x *= vNewColor.x;
        vFlareColor.y *= vNewColor.y;
        vFlareColor.z *= vNewColor.z;
        vFlareColor.w *= baseFadeValue * fadeValue;

        flares[i]->spriteColor = Vect4_to_RGBA(vFlareColor);
    }

    color = newColor;
}



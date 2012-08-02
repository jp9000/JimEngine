/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Particle.cpp

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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


/*#include "Base.h"


DefineAbstractClass(Particle);
DefineClass(SpriteParticle);



void Particle::Init()
{
    Super::Init();

    //start the fade value at 1.0f
    fadeValue = 1.0f;

    bCollideWithWorld = TRUE;
    collisionType = SphereCollision;
    sphereRadius = 0.5f;

    if(sprite)
        RM->AddTextureRef(sprite);
}

void Particle::Destroy()
{
    if(sprite)
        RM->ReleaseTexture(sprite);

    Super::Destroy();
}

void Particle::Tick(DWORD dwTick, float fSeconds)
{
    curTime += dwTick;

    if(curTime > endTime)
    {
        SafeDestroy();
        return;
    }

    //calls Entity::Tick, which handles all the basic physics functions
    Super::Tick(dwTick, fSeconds);

    //process friction for the particle
    if(friction != 0.0f)
        MoveObject(((GetNextPos()-GetLocalPos())/friction)+GetLocalPos());

    if(bFade)
        fadeValue = 1.0f-((float)curTime/(float)endTime);

    if(growth != 0.0f)
    {
        curSize += (growth*fSeconds);

        if(curSize <= 0.0f)
            curSize = 0.0f;
    }

    if(rotRate != 0.0f)
    {
        rotAngle += (rotRate*fSeconds);

        while(rotAngle >= 360.0f)
            rotAngle -= 360.0f;

        while(rotAngle <= 0.0f)
            rotAngle += 360.0f;
    }
}

void SpriteParticle::Init()
{
    Super::Init();

    VBData *vbData = new VBData;
    vbData->VertList << Vect(1.0f, -1.0f, 0.0f);
    vbData->VertList << Vect(1.0f, 1.0f, 0.0f);
    vbData->VertList << Vect(-1.0f, -1.0f, 0.0f);
    vbData->VertList << Vect(-1.0f, 1.0f, 0.0f);
    vbData->TVList.SetSize(1);
    vbData->TVList[0] << Vect(0.0f, 1.0f, 0.0f);
    vbData->TVList[0] << Vect(0.0f, 0.0f, 0.0f);
    vbData->TVList[0] << Vect(1.0f, 1.0f, 0.0f);
    vbData->TVList[0] << Vect(1.0f, 0.0f, 0.0f);

    spriteVertexBuffer = CreateVertexBuffer(vbData, FALSE);
}


void SpriteParticle::Destroy()
{
    if(spriteVertexBuffer)
        delete spriteVertexBuffer;

    Super::Destroy();
}


void SpriteParticle::Render()
{
    if(curSize <= 0.0f)
        return;

    DepthWriteEnable(FALSE);    //turn off writing to the depth buffer

    DepthFunction(GS_LESS);     //only draw if it's less than the depth
                                //buffer (if it's visible)

    //if(fadeValue == 0.0f)
    //    return;

    EnableBlending(TRUE);       //enable alpha blending

    //make it so the color blending is multiplied by the source alpha
    BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);

    //load the sprite texture
    LoadTexture(sprite);
    LoadDefault3DSampler();

    //set material color
    DWORD alphaFade = ((DWORD)(fadeValue*255.0f)) << 24;

    DWORD newColor = (color&0xFFFFFF) | alphaFade;


    SetMaterialColor(newColor);
    EnableHardwareLighting(TRUE);

    //---------------------------------------------------------------------

    float halfSize = curSize*0.5f;

    VBData *vbData = spriteVertexBuffer->GetData();
    vbData->VertList[0].Set(halfSize, -halfSize);
    vbData->VertList[1].Set(halfSize, halfSize);
    vbData->VertList[2].Set(-halfSize, -halfSize);
    vbData->VertList[3].Set(-halfSize, halfSize);
    spriteVertexBuffer->FlushBuffers();

    LoadVertexBuffer(spriteVertexBuffer);
    LoadIndexBuffer(NULL);

    MatrixPush();  //save the current matrix, make peace with the machines
    MatrixTranslate(GetWorldPos());
    MatrixRotate(level->GetCurrentCamera()->GetWorldRot());
    MatrixRotate(0.0f, 0.0f, 1.0f, rotAngle);

        Draw(GS_TRIANGLESTRIP);

    MatrixPop();   //matrix reloaded...  reload the original matrix

    LoadIndexBuffer(NULL);
    LoadVertexBuffer(NULL);

    //---------------------------------------------------------------------

    EnableHardwareLighting(FALSE);

    //unload texture
    LoadTexture(NULL);

    DepthWriteEnable(TRUE);
}
*/

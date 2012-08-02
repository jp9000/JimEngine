/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Sprite3D.cpp:  3D Sprite class

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


DefineClass(Sprite3D);


void Sprite3D::Init()
{
    traceIn(Sprite3D::Init);

    Super::Init();

    RenderStartNew();
        Vertex(1.0f, -1.0f, 0.0f);
        Vertex(1.0f, 1.0f, 0.0f);
        Vertex(-1.0f, -1.0f, 0.0f);
        Vertex(-1.0f, 1.0f, 0.0f);

        TexCoord(1.0f, 1.0f);
        TexCoord(1.0f, 0.0f);
        TexCoord(0.0f, 1.0f);
        TexCoord(0.0f, 0.0f);
    spriteVertexBuffer = RenderSave();

    curAngle = spriteRotation;

    if(!spriteTextureName.IsEmpty())
        spriteTexture = GetTexture(spriteTextureName);

    traceOut;
}


void Sprite3D::Destroy()
{
    traceIn(Sprite3D::Destroy);

    if(spriteTexture)
        ReleaseTexture(spriteTexture);

    if(spriteVertexBuffer)
        delete spriteVertexBuffer;

    Super::Destroy();

    traceOut;
}


void Sprite3D::Tick(float fTime)
{
    Super::Tick(fTime);

    curAngle += spriteRotationRate*fTime;

    while(curAngle < 0.0f)
        curAngle += 360.0f;

    while(curAngle >= 360.0f)
        curAngle -= 360.0f;
}


void Sprite3D::Render()
{
    if(!RGB_A(spriteColor) || !spriteTexture || bHide)
        return;

    //turn off writing to the depth buffer, so that the sprites can draw over
    //each other
    DepthWriteEnable(FALSE);

    EnableDepthTest(!bOmitDepthTest);

    //enable color blending
    EnableBlending(TRUE);

    //make it so the color blending is multiplied by the source alpha
    BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);

    //load the sprite texture
    LoadTexture(spriteTexture);
    LoadDefault3DSampler();

    //set material color
    SetMaterialColor(spriteColor);
    EnableHardwareLighting(TRUE);

    //---------------------------------------------------------------------

    float halfSize = spriteSize*0.5f;

    LoadVertexBuffer(spriteVertexBuffer);
    LoadIndexBuffer(NULL);

    MatrixPush();  //save the current matrix, make peace with the machines
    MatrixTranslate(GetWorldPos());
    MatrixRotate(level->GetCurrentCamera()->GetWorldRot());
    MatrixRotate(0.0f, 0.0f, 1.0f, curAngle);
    MatrixScale(halfSize, halfSize, 1.0f);

        Draw(GS_TRIANGLESTRIP);

    MatrixPop();   //matrix reloaded...  reload the original matrix

    LoadIndexBuffer(NULL);
    LoadVertexBuffer(NULL);

    //---------------------------------------------------------------------

    EnableHardwareLighting(FALSE);

    //unload texture
    LoadTexture(NULL);

    if(bOmitDepthTest)
        EnableDepthTest(TRUE);

    DepthWriteEnable(TRUE);
}


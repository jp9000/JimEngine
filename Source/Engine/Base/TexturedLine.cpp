/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TexturedLine

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

DefineClass(TexturedLine);

void TexturedLine::Init()
{
    traceIn(TexturedLine::Init);

    Super::Init();

    bRenderable = TRUE;

    traceOut;
}

void TexturedLine::Destroy()
{
    traceIn(TexturedLine::Destroy);

    if(lineTexture)
        ReleaseTexture(lineTexture);

    Super::Destroy();

    traceOut;
}

void TexturedLine::SetLineTexture(Texture *texture)
{
    traceIn(TexturedLine::SetLineTexture);

    if(lineTexture)
        ReleaseTexture(lineTexture);

    lineTexture = texture;

    traceOut;
}

void TexturedLine::Render()
{
    if(!lineTexture) return;

    Vect lineDir = (pointB-pointA).Norm();

    float halfWidth = width*0.5f;

    Camera *viewCam = level->GetCurrentCamera();
    Vect camPos = viewCam->GetWorldPos();
    Vect crossLine;

    DepthWriteEnable(FALSE);
    EnableBlending(TRUE);

    if(lineTexture->HasAlpha())
        BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
    else
        BlendFunction(GS_BLEND_ONE, GS_BLEND_ONE);

    LoadTexture(lineTexture);
    LoadDefault3DSampler();

    MatrixPush();
    MatrixTranslate(GetWorldPos());
    MatrixRotate(GetWorldRot());

        RenderStart();
            Vect endAdjust = (lineDir*halfWidth);
            Vect backEnd  = (pointA-endAdjust);
            Vect frontEnd = (pointB+endAdjust);

            Color(color);

            crossLine = (camPos-backEnd).Cross(lineDir).Norm() * halfWidth;
            Vertex(backEnd+crossLine);  TexCoord(1.0f, 0.0f);
            Vertex(backEnd-crossLine);  TexCoord(0.0f, 0.0f);

            crossLine = (camPos-pointA).Cross(lineDir).Norm() * halfWidth;
            Vertex(pointA+crossLine);   TexCoord(1.0f, 0.5f);
            Vertex(pointA-crossLine);   TexCoord(0.0f, 0.5f);

            crossLine = (camPos-pointB).Cross(lineDir).Norm() * halfWidth;
            Vertex(pointB+crossLine);   TexCoord(1.0f, 0.5f);
            Vertex(pointB-crossLine);   TexCoord(0.0f, 0.5f);

            crossLine = (camPos-frontEnd).Cross(lineDir).Norm() * halfWidth;
            Vertex(frontEnd+crossLine); TexCoord(1.0f, 1.0f);
            Vertex(frontEnd-crossLine); TexCoord(0.0f, 1.0f);
        RenderStop(GS_TRIANGLESTRIP);

    MatrixPop();

    LoadTexture(NULL);
}

Bounds TexturedLine::GetBounds() const
{
    return Bounds(Vect::Min(pointA, pointB)-width, Vect::Max(pointA, pointB)+width);
}

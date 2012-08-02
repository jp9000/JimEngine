/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  OutdoorRender.cpp

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


void OutdoorLevel::DrawSkydome()
{
    LoadVertexBuffer(skydomeVB);

    for(int i=0; i<4; i++)
    {
        LoadIndexBuffer(skydomeIBs[i]);
        GS->Draw(GS_TRIANGLESTRIP);
    }
    LoadIndexBuffer(skydomeIBs[4]);
    GS->Draw(GS_TRIANGLEFAN);
}

void OutdoorLevel::DrawWater()
{
    LoadIndexBuffer(NULL);

    for(int i=0; i<6; i++)
    {
        LoadVertexBuffer(waterVBs[i]);
        GS->DrawBare(GS_TRIANGLESTRIP);
    }
}

#define TerrainDrawType GS_TRIANGLESTRIP

void TerrainBlock::RenderInitialPass()
{
    OutdoorLevel *outdoorLevel = static_cast<OutdoorLevel*>(level);

    LoadVertexBuffer(VertBuffer);

    //bla bla bla.  todo:  do something reasonable for terrain you bum

    //---------------------------------------

    if(curLOD < 3)
    {
        List<IndexBuffer*> &DrawSegments = outdoorLevel->TerrainLOD[curLOD][LODFlags].DrawSegments;

        for(DWORD i=0; i<DrawSegments.Num(); i++)
        {
            assert(DrawSegments[i]);
            LoadIndexBuffer(DrawSegments[i]);

            GS->Draw(TerrainDrawType);
        }
    }
    else
    {
        List<IndexBuffer*> &DrawSegments = outdoorLevel->LowestLOD.DrawSegments;

        for(DWORD i=0; i<DrawSegments.Num(); i++)
        {
            assert(DrawSegments[i]);
            LoadIndexBuffer(DrawSegments[i]);

            GS->Draw(TerrainDrawType);
        }
    }
}

void TerrainBlock::QuickRender()
{
    OutdoorLevel *outdoorLevel = static_cast<OutdoorLevel*>(level);

    LoadVertexBuffer(VertBuffer);
    //LoadIndexBuffer(IdxBuffer);

    //---------------------------------------

    if(curLOD < 3)
    {
        List<IndexBuffer*> &DrawSegments = outdoorLevel->TerrainLOD[curLOD][LODFlags].DrawSegments;

        for(DWORD i=0; i<DrawSegments.Num(); i++)
        {
            assert(DrawSegments[i]);
            LoadIndexBuffer(DrawSegments[i]);

            GS->DrawBare(TerrainDrawType);
        }
    }
    else
    {
        List<IndexBuffer*> &DrawSegments = outdoorLevel->LowestLOD.DrawSegments;

        for(DWORD i=0; i<DrawSegments.Num(); i++)
        {
            assert(DrawSegments[i]);
            LoadIndexBuffer(DrawSegments[i]);

            GS->DrawBare(TerrainDrawType);
        }
    }
}

void TerrainBlock::Render()
{
    OutdoorLevel *outdoorLevel = static_cast<OutdoorLevel*>(level);

    LoadVertexBuffer(VertBuffer);

    //---------------------------------------

    if(curLOD < 3)
    {
        List<IndexBuffer*> &DrawSegments = outdoorLevel->TerrainLOD[curLOD][LODFlags].DrawSegments;

        for(DWORD i=0; i<DrawSegments.Num(); i++)
        {
            assert(DrawSegments[i]);
            LoadIndexBuffer(DrawSegments[i]);

            GS->Draw(TerrainDrawType);
        }
    }
    else
    {
        List<IndexBuffer*> &DrawSegments = outdoorLevel->LowestLOD.DrawSegments;

        for(DWORD i=0; i<DrawSegments.Num(); i++)
        {
            assert(DrawSegments[i]);
            LoadIndexBuffer(DrawSegments[i]);

            GS->Draw(TerrainDrawType);
        }
    }
}


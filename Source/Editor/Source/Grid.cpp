/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Grid.cpp

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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#include "Xed.h"


DefineClass(Grid);


void Grid::Init()
{
    int i;

    RenderStart();
        for(i=-100; i<100; i++)
        {
            if(abs(i)%5)
                Color(0x558AAEE0);
            else
                Color(i ? 0x65C2DCFF : 0x80FFFFFF);
            Vertex(float(i),  100.0f, 0.0f);
            Vertex(float(i), -100.0f, 0.0f);

            Vertex( 100.0f, float(i), 0.0f);
            Vertex(-100.0f, float(i), 0.0f);
        }
    vb = RenderSave();

    bDrawGrid = true;
    //bAlwaysVisible = TRUE;
    spacing = 2.0f;
}

void Grid::Destroy()
{
    delete vb;
}


void Grid::Render()
{
    Quat newRot(0.0f, 0.0f, 0.0f, 90.0f);

    switch(oriantation)
    {
        case 0:
            newRot.y = 1.0f;
            break;
        case 1:
            newRot.x = 1.0f;
            break;
        case 2:
            newRot.z = 1.0f;
            break;
    }

    newRot.MakeQuat();

    Matrix scaleMat;
    zero32(&scaleMat, sizeof(Matrix));
    scaleMat.X.x = spacing;
    scaleMat.Y.y = spacing;
    scaleMat.Z.z = spacing;

    MatrixPush();
    MatrixMultiply(scaleMat);
    MatrixRotateQuat(newRot);

        EnableDepthTest(FALSE);

        LoadIndexBuffer(NULL);
        LoadVertexBuffer(vb);
        Draw(GS_LINES);

        EnableDepthTest(TRUE);

    MatrixPop();
}

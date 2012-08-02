/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  DefaultManipulator.cpp

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


DefineClass(DefaultManipulator);


void DefaultManipulator::Init()
{
    traceIn(DefaultManipulator::Init);

    Super::Init();

    RenderStartNew();
        Vertex( 0.0f,   0.0f,   0.0f);
        Vertex( 0.0f,   0.0f,  12.0f);

        Vertex( 0.0f,   0.0f,  12.0f);
        Vertex( 1.0f,   0.0f,  11.0f);

        Vertex( 0.0f,   0.0f,  12.0f);
        Vertex(-1.0f,   0.0f,  11.0f);

        Vertex( 0.0f,   0.0f,  12.0f);
        Vertex( 0.0f,   1.0f,  11.0f);
                    
        Vertex( 0.0f,   0.0f,  12.0f);
        Vertex( 0.0f,  -1.0f,  11.0f);

    buffer = RenderSave();

    traceOut;
}

void DefaultManipulator::Destroy()
{
    traceIn(DefaultManipulator::Destroy);

    delete buffer;

    Super::Destroy();

    traceOut;
}


void DefaultManipulator::RenderScaled(const Vect &cameraDir, float scale)
{
    traceInFast(DefaultManipulator::RenderScaled);

    AxisAngle baseRotation(0.0f, 0.0f, 1.0f, RAD(90.0f));

    Shader *shader = GetVertexShader(TEXT("Base:SolidColor.vShader"));
    LoadVertexShader(shader);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    for(DWORD i=0; i<3; i++)
    {
        if(fabsf(cameraDir.ptr[i]) == 1.0f)
            continue;

        AxisAngle rotation;
        Color4 axisColor;

        zero(&rotation, sizeof(rotation));
        rotation.ptr[i] = 1.0f;
        rotation.w = RAD(90.0f);

        zero(&axisColor, sizeof(axisColor));
        axisColor.ptr[i] = 0.75f;
        axisColor.w = 1.0f;

        MatrixPush();
        MatrixTranslate(GetWorldPos());
        MatrixRotate(baseRotation);
        MatrixRotate(rotation);
        MatrixScale(scale, scale, scale);

            LoadIndexBuffer(NULL);

            DWORD axis = i*2;

            if(!axis)
                axis = 1;

            shader->SetColor(shader->GetParameter(1), axisColor);

            LoadVertexBuffer(buffer);
            Draw(GS_LINES);

        MatrixPop();
    }

    LoadVertexBuffer(NULL);
    LoadVertexShader(NULL);
    LoadPixelShader(NULL);

    traceOutFast;
}


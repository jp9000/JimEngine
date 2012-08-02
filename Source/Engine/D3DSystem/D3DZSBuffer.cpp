/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  D3DZStencilBuffer.cpp:  Direct3D 9 Z-Stencil Buffer

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


#include "D3DSystem.h"


void D3DZStencilBuffer::CreateBuffer()
{
    traceIn(D3DZStencilBuffer::CreateBuffer);

    HRESULT problemo = d3d->d3dDevice->CreateDepthStencilSurface(bufWidth, bufHeight, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE, &d3dSurface, NULL);

    if(!SUCCEEDED(problemo))
    {
        if(problemo == D3DERR_OUTOFVIDEOMEMORY)
            ErrOut(TEXT("Error Creating D3D Surface: Out of Video Memory!"));
        else if(problemo == D3DERR_INVALIDCALL)
            ErrOut(TEXT("Error Creating D3D Surface: Invalid call"));
        else if(problemo == E_OUTOFMEMORY)
            ErrOut(TEXT("Error Creating D3D Surface: Out of general memory!"));
        else
            ErrOut(TEXT("Could not create D3D Surface for some unknown reason"));
    }

    traceOut;
}

void D3DZStencilBuffer::FreeBuffer()
{
    traceIn(D3DZStencilBuffer::FreeBuffer);

    D3DRelease(d3dSurface);

    traceOut;
}

D3DZStencilBuffer::~D3DZStencilBuffer()
{
    traceIn(D3DZStencilBuffer::~D3DZStencilBuffer);

    FreeBuffer();
    d3d->ZStencilBufferList.RemoveItem(this);

    traceOut;
}

DWORD D3DZStencilBuffer::Width()
{
    return bufWidth;
}

DWORD D3DZStencilBuffer::Height()
{
    return bufHeight;
}

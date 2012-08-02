/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  D3DIndexBuffer.cpp

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

const DWORD DXIndexSize[]   = {D3DFMT_INDEX16, D3DFMT_INDEX32};


D3DIndexBuffer::D3DIndexBuffer(D3DSystem *curSystem, int IndexType, void *indicesIn, DWORD dwNum, BOOL bIsStatic)
:   Buffer(NULL), bJustCreated(1)
{
    traceIn(D3DIndexBuffer::D3DIndexBuffer);

    assert(dwNum);
    assert(indicesIn);

    d3d     = curSystem;

    dwSize  = dwNum;
    bStatic = bIsStatic;
    indices = indicesIn;
    Type    = IndexType;
    Pitch   = ((Type == GS_UNSIGNED_LONG) ? 4 : 2);

    CreateBuffer();

    d3d->IndexBufferList << this;

    traceOut;
}

//cloning
D3DIndexBuffer::D3DIndexBuffer(D3DSystem *curSystem, IndexBuffer *buffer, BOOL bIsStatic)
:   Buffer(NULL), bJustCreated(1)
{
    traceIn(D3DIndexBuffer::~D3DIndexBuffer);

    D3DIndexBuffer *ib = (D3DIndexBuffer *)buffer;
    assert(ib->dwSize);
    assert(ib->indices);

    d3d         = curSystem;

    Type        = ib->Type;
    bStatic     = bIsStatic;
    dwSize      = ib->dwSize;
    Pitch       = ib->Pitch;

    DWORD internalPitch = Pitch;
    indices = Allocate(dwSize*internalPitch);
    mcpy(indices, ib->indices, dwSize*internalPitch);

    CreateBuffer();

    d3d->IndexBufferList << this;

    traceOut;
}

D3DIndexBuffer::~D3DIndexBuffer()
{
    traceIn(D3DIndexBuffer::~D3DIndexBuffer);

    d3d->IndexBufferList.RemoveItem(this);

    Clear();

    traceOut;
}

void  D3DIndexBuffer::Clear()
{
    traceIn(D3DIndexBuffer::Clear);

    Free(indices);
    FreeBuffer();

    traceOut;
}

void  D3DIndexBuffer::FreeBuffer()
{
    traceIn(D3DIndexBuffer::FreeBuffer);

    D3DRelease(Buffer);

    traceOut;
}

void  D3DIndexBuffer::CreateBuffer()
{
    traceIn(D3DIndexBuffer::CreateBuffer);

    DWORD dwFlags;

    dwInternalSize = dwSize;

    if(bStatic)
        dwFlags = D3DUSAGE_WRITEONLY;
    else
        dwFlags = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;

    DWORD dwAdjustType = Type;
    HRESULT problemo;

    if(!SUCCEEDED(problemo = d3d->d3dDevice->CreateIndexBuffer(dwInternalSize*Pitch, dwFlags, (D3DFORMAT)(DXIndexSize[dwAdjustType]), D3DPOOL_DEFAULT, &Buffer, NULL)))
    {
        if(problemo == D3DERR_OUTOFVIDEOMEMORY)
            ErrOut(TEXT("Error Creating D3D Index Buffer: Out of Video Memory!"));
        else if(problemo == D3DERR_INVALIDCALL)
            ErrOut(TEXT("Error Creating D3D Index Buffer: Invalid parameter"));
        else if(problemo == E_OUTOFMEMORY)
            ErrOut(TEXT("Error Creating D3D Index Buffer: Out of general memory!"));
        else if(problemo == D3DXERR_INVALIDDATA)
            ErrOut(TEXT("Error Creating D3D Index Buffer: Invalid Data"));
        else
            ErrOut(TEXT("Could not create D3D Index Buffer for some unknown reason"));
    }

    FlushBuffer();
    bJustCreated = 0;

    traceOut;
}

void  D3DIndexBuffer::FlushBuffer()
{
    traceInFast(D3DIndexBuffer::FlushBuffer);

    LPBYTE  lpData, lpIndices = (LPBYTE)indices;

    Buffer->Lock(0, dwInternalSize*Pitch, (void**)&lpData, 0);
    mcpy(lpData, indices, dwSize*Pitch);
    Buffer->Unlock();

    traceOutFast;
}


//copies two buffers of the same size
void  D3DIndexBuffer::CopyList(IndexBuffer *buffer)
{
    traceIn(D3DIndexBuffer::CopyList);

    assert(!bStatic);
    assert(buffer);

    if(bStatic) return;

    D3DIndexBuffer *ib = (D3DIndexBuffer *)buffer;

    assert(ib);
    if(!ib) return;
    assert(dwSize == ib->dwSize);
    if(dwSize != ib->dwSize) return;
    assert(dwInternalSize == ib->dwInternalSize);
    if(dwInternalSize != ib->dwInternalSize) return;
    assert(Type == ib->Type);
    if(Type != ib->Type) return;
    assert(ib->indices);

    Pitch   = ((Type == GS_UNSIGNED_LONG) ? 4 : 2);

    if(indices)
        Free(indices);

    DWORD internalPitch = Pitch;
    indices = Allocate(dwSize*internalPitch);
    mcpy(indices, ib->indices, dwSize*internalPitch);

    FlushBuffer();

    traceOut;
}


void *D3DIndexBuffer::GetData()
{
    return indices;
}

GSIndexType D3DIndexBuffer::GetIndexType()
{
    return (GSIndexType)Type;
}

UINT D3DIndexBuffer::NumIndices()
{
    return dwSize;
}


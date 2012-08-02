/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  D3DVertexBuffer.cpp

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


#ifdef C64

inline void ENGINEAPI copyVertList(void *pDest, const void *pSrc, size_t iLen)
{
    assert(pDest);
    assert(pSrc);

    register size_t iLenDiv12 = (iLen/12);

    register DWORD *srcDW = (DWORD*)pSrc, *destDW = (DWORD*)pDest;
    while(iLenDiv12--)
    {
        *((QWORD*)destDW) = *((QWORD*)srcDW);
        destDW[2] = srcDW[2];
        destDW += 3;
        srcDW  += 4;
    }
}

#else

inline void ENGINEAPI copyVertList(void *pDest, const void *pSrc, size_t iLen)
{
    assert(pDest);
    assert(pSrc);

    register size_t iLenDiv4 = (iLen/4);

    register DWORD *srcDW = (DWORD*)pSrc, *destDW = (DWORD*)pDest;
    while(iLenDiv4--)
    {
        *(destDW++) = *(srcDW++);
        if(!(iLenDiv4%3))
            ++srcDW;
    }
}

#endif



D3DVertexBuffer::D3DVertexBuffer(D3DSystem *curSystem, VBData *vbd, BOOL bIsStatic)
:   VertBuffer(NULL), NormalBuffer(NULL), ColorBuffer(NULL)
{
    traceIn(D3DVertexBuffer::D3DVertexBuffer);

    assert(vbd);
    assert(vbd->VertList.Num() && vbd->VertList.Array());

    if(!vbd || !vbd->VertList.Array())
        return;

    d3d                 = curSystem;

    DWORD nTextures     = vbd->TVList.Num();
    dwSize              = vbd->VertList.Num();
    bStatic             = bIsStatic;
    vbData              = vbd;

    SetupBuffers();

    d3d->VertexBufferList << this;

    traceOut;
}

//cloning
D3DVertexBuffer::D3DVertexBuffer(D3DSystem *curSystem, VertexBuffer *buffer, BOOL bIsStatic)
:   VertBuffer(NULL), NormalBuffer(NULL), ColorBuffer(NULL)
{
    traceIn(D3DVertexBuffer::D3DVertexBuffer(2));

    D3DVertexBuffer *vb = (D3DVertexBuffer *)buffer;
    assert(vb);

    d3d                 = curSystem;

    bStatic             = bIsStatic;
    vbData              = new VBData;
    dwSize              = vb->dwSize;

    if(!vb) return;
    assert(dwSize == vb->dwSize);
    if(dwSize != vb->dwSize)  return;
    dwSize = vb->dwSize;

    vbData->CopyList(*vb->vbData);

    SetupBuffers();

    d3d->VertexBufferList << this;

    traceOut;
}

D3DVertexBuffer::~D3DVertexBuffer()
{
    traceIn(D3DVertexBuffer::~D3DVertexBuffer);

    d3d->VertexBufferList.RemoveItem(this);

    Clear();

    traceOut;
}

void  D3DVertexBuffer::Clear()
{
    traceIn(D3DVertexBuffer::Clear);

    FreeBuffers();
    if(vbData)
    {
        vbData->Clear();
        delete vbData;
    }

    traceOut;
}

void  D3DVertexBuffer::FreeBuffers()
{
    traceIn(D3DVertexBuffer::FreeBuffers);

    D3DRelease(declaration);
    for(int i=0; i<TCBuffer.Num(); i++)
    {
        D3DRelease(TCBuffer[i]);
    }
    TCBuffer.Clear();
    TCSizes.Clear();
    D3DRelease(TangentBuffer);
    D3DRelease(ColorBuffer);
    D3DRelease(NormalBuffer);
    D3DRelease(VertBuffer);

    traceOut;
}

void  D3DVertexBuffer::SetupBuffers()
{
    traceIn(D3DVertexBuffer::SetupBuffers);

    assert(vbData->VertList.Num());
    DWORD dwFlags;
    List<D3DVERTEXELEMENT9> VertexElements;
    D3DVERTEXELEMENT9 *pElement;
    LPBYTE lpData;

    D3DRelease(declaration);

    if(bStatic)
        dwFlags = D3DUSAGE_WRITEONLY;
    else
        dwFlags = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;

    if(!SUCCEEDED(d3d->d3dDevice->CreateVertexBuffer(dwSize*12, dwFlags, 0, D3DPOOL_DEFAULT, &VertBuffer, NULL)))
        ErrOut(TEXT("Could not create VertBuffer"));

    VertBuffer->Lock(0, dwSize*12, (void**)&lpData, 0);
    //for(int j=0; j<dwSize; j++)
    //    mcpy(lpData+(j*12), vbData->VertList+j, 12);
    //mcpy(lpData, vbData->VertList.Array(), dwSize*12);
    copyVertList(lpData, vbData->VertList.Array(), dwSize*12);
    VertBuffer->Unlock();

    pElement = VertexElements.CreateNew();
    zero(pElement, sizeof(D3DVERTEXELEMENT9));
    pElement->Stream    = 0;
    pElement->Type      = D3DDECLTYPE_FLOAT3;
    pElement->Usage     = D3DDECLUSAGE_POSITION;

    FVF = D3DFVF_XYZ;

    TCBuffer.SetSize(vbData->TVList.Num());
    TCSizes.SetSize(vbData->TVList.Num());
    for(int i=0; i<vbData->TVList.Num(); i++)
    {
        TVertList &tvData = vbData->TVList[i];
        if(!tvData.Num())
            ErrOut(TEXT("TVList[%d].Num() is zero."), i);

        TCSizes[i] = tvData.TypeSize();

        if(!SUCCEEDED(d3d->d3dDevice->CreateVertexBuffer(dwSize*tvData.TypeSize(), dwFlags, 0, D3DPOOL_DEFAULT, &TCBuffer[i], NULL)))
            ErrOut(TEXT("Could not create TCBuffer[%d]"), i);

        TCBuffer[i]->Lock(0, dwSize*tvData.TypeSize(), (void**)&lpData, 0);
        if(tvData.GetWidth() != 3)
            mcpy(lpData, tvData.Array(), dwSize*tvData.TypeSize());
        else
            copyVertList(lpData, tvData.Array(), dwSize*tvData.TypeSize());
        TCBuffer[i]->Unlock();

        pElement = VertexElements.CreateNew();
        zero(pElement, sizeof(D3DVERTEXELEMENT9));
        pElement->Stream     = i+1;
        pElement->Usage      = D3DDECLUSAGE_TEXCOORD;
        pElement->UsageIndex = i;

        switch(tvData.GetWidth())
        {
            case 2: pElement->Type = D3DDECLTYPE_FLOAT2; FVF |= D3DFVF_TEXCOORDSIZE2(i); break;
            case 3: pElement->Type = D3DDECLTYPE_FLOAT3; FVF |= D3DFVF_TEXCOORDSIZE3(i); break;
            case 4: pElement->Type = D3DDECLTYPE_FLOAT4; FVF |= D3DFVF_TEXCOORDSIZE4(i); break;
        }
    }

    if(vbData->NormalList.Num())
    {
        if(!SUCCEEDED(d3d->d3dDevice->CreateVertexBuffer(dwSize*12, dwFlags, 0, D3DPOOL_DEFAULT, &NormalBuffer, NULL)))
            ErrOut(TEXT("Could not create NormalBuffer"));

        NormalBuffer->Lock(0, dwSize*12, (void**)&lpData, 0);
        //for(int j=0; j<dwSize; j++)
        //    mcpy(lpData+(j*12), vbData->NormalList+j, 12);
        //mcpy(lpData, vbData->NormalList.Array(), dwSize*12);
        copyVertList(lpData, vbData->NormalList.Array(), dwSize*12);
        NormalBuffer->Unlock();

        pElement = VertexElements.CreateNew();
        zero(pElement, sizeof(D3DVERTEXELEMENT9));
        pElement->Stream    = 9;
        pElement->Type      = D3DDECLTYPE_FLOAT3;
        pElement->Usage     = D3DDECLUSAGE_NORMAL;

        FVF |= D3DFVF_NORMAL;
    }

    if(vbData->ColorList.Num())
    {
        if(!SUCCEEDED(d3d->d3dDevice->CreateVertexBuffer(dwSize*4, dwFlags, 0, D3DPOOL_DEFAULT, &ColorBuffer, NULL)))
            ErrOut(TEXT("Could not create ColorBuffer"));

        ColorBuffer->Lock(0, dwSize*4, (void**)&lpData, 0);
        mcpy(lpData, vbData->ColorList.Array(), dwSize*4);
        ColorBuffer->Unlock();

        pElement = VertexElements.CreateNew();
        zero(pElement, sizeof(D3DVERTEXELEMENT9));
        pElement->Stream    = 10;
        pElement->Type      = D3DDECLTYPE_D3DCOLOR;
        pElement->Usage     = D3DDECLUSAGE_COLOR;

        FVF |= D3DFVF_DIFFUSE;
    }

    if(vbData->TangentList.Num())
    {
        if(!SUCCEEDED(d3d->d3dDevice->CreateVertexBuffer(dwSize*12, dwFlags, 0, D3DPOOL_DEFAULT, &TangentBuffer, NULL)))
            ErrOut(TEXT("Could not create TangentBuffer"));

        TangentBuffer->Lock(0, dwSize*12, (void**)&lpData, 0);
        //for(int j=0; j<dwSize; j++)
        //    mcpy(lpData+(j*12), vbData->TangentList+j, 12);
        //mcpy(lpData, vbData->TangentList.Array(), dwSize*12);
        copyVertList(lpData, vbData->TangentList.Array(), dwSize*12);
        TangentBuffer->Unlock();

        pElement = VertexElements.CreateNew();
        zero(pElement, sizeof(D3DVERTEXELEMENT9));
        pElement->Stream    = 11;
        pElement->Type      = D3DDECLTYPE_FLOAT3;
        pElement->Usage     = D3DDECLUSAGE_TANGENT;
    }

    FVF |= (vbData->TVList.Num() << D3DFVF_TEXCOUNT_SHIFT);

    pElement = VertexElements.CreateNew();
    zero(pElement, sizeof(D3DVERTEXELEMENT9));
    pElement->Stream    = 0xFF;
    pElement->Type      = D3DDECLTYPE_UNUSED;

    HRESULT result;
    if(!SUCCEEDED(result = d3d->d3dDevice->CreateVertexDeclaration(VertexElements.Array(), &declaration)))
    {
        if(result == D3DERR_INVALIDCALL)
            ErrOut(TEXT("CreateVertexDeclaration, invalid call."));
        else if(result == D3DERR_OUTOFVIDEOMEMORY)
            ErrOut(TEXT("CreateVertexDeclaration, out of video mem."));
        else if(result == E_OUTOFMEMORY)
            ErrOut(TEXT("CreateVertexDeclaration, out of normal mem."));
        ErrOut(TEXT("Could not create vertex declaration"));
    }

    VertexElements.Clear();

    traceOut;
}

void  D3DVertexBuffer::FlushBuffers(BOOL bRebuild)
{
    traceInFast(D3DVertexBuffer::FlushBuffers);

    if(bRebuild)
    {
        FreeBuffers();
        SetupBuffers();
    }
    else
    {
        //assert(!bStatic);
        LPBYTE lpData;

        DWORD dwLockFlags = bStatic ? 0 : D3DLOCK_DISCARD;

        VertBuffer->Lock(0, dwSize*12, (void**)&lpData, dwLockFlags);
        //for(int j=0; j<dwSize; j++)
        //    mcpy(lpData+(j*12), vbData->VertList+j, 12);
        //mcpy(lpData, vbData->VertList.Array(), dwSize*12);
        copyVertList(lpData, vbData->VertList.Array(), dwSize*12);
        VertBuffer->Unlock();

        if(NormalBuffer)
        {
            NormalBuffer->Lock(0, dwSize*12, (void**)&lpData, dwLockFlags);
            //for(int j=0; j<dwSize; j++)
            //    mcpy(lpData+(j*12), vbData->NormalList+j, 12);
            //mcpy(lpData, vbData->NormalList.Array(), dwSize*12);
            copyVertList(lpData, vbData->NormalList.Array(), dwSize*12);
            NormalBuffer->Unlock();
        }

        if(ColorBuffer)
        {
            ColorBuffer->Lock(0, dwSize*4, (void**)&lpData, dwLockFlags);
            mcpy(lpData, vbData->ColorList.Array(), dwSize*4);
            ColorBuffer->Unlock();
        }

        for(int i=0; i<TCBuffer.Num(); i++)
        {
            if(TCBuffer[i])
            {
                TCBuffer[i]->Lock(0, dwSize*vbData->TVList[i].TypeSize(), (void**)&lpData, dwLockFlags);

                if(vbData->TVList[i].GetWidth() == 3)
                    copyVertList(lpData, vbData->TVList[i].Array(), dwSize*vbData->TVList[i].TypeSize());
                else
                    mcpy(lpData, vbData->TVList[i].Array(), dwSize*vbData->TVList[i].TypeSize());
                TCBuffer[i]->Unlock();
            }
        }

        if(TangentBuffer)
        {
            TangentBuffer->Lock(0, dwSize*12, (void**)&lpData, dwLockFlags);
            //for(int j=0; j<dwSize; j++)
            //    mcpy(lpData+(j*12), vbData->TangentList+j, 12);
            //mcpy(lpData, vbData->TangentList.Array(), dwSize*12);
            copyVertList(lpData, vbData->TangentList.Array(), dwSize*12);
            TangentBuffer->Unlock();
        }
    }

    traceOutFast;
}


//copies two buffers of the same size
void  D3DVertexBuffer::CopyList(VertexBuffer *buffer)
{
    traceInFast(D3DVertexBuffer::CopyList);

    assert(buffer);

    D3DVertexBuffer *vb = (D3DVertexBuffer *)buffer;

    assert(vb);
    if(!vb) return;
    assert(dwSize == vb->dwSize);
    if(dwSize != vb->dwSize)  return;
    dwSize = vb->dwSize;

    vbData->CopyList(*vb->vbData);

    FlushBuffers(TRUE);

    traceOutFast;
}


VBData* D3DVertexBuffer::GetData()
{
    /*assert(!bStatic);
    return (bStatic) ? NULL : vbData;*/
    return vbData;
}

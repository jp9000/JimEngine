/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  D3DCubeTexture.h

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



D3DCubeTexture::D3DCubeTexture(D3DSystem *curSystem, unsigned int width, DWORD dwFormatIn, BOOL bBuildMipMaps, BOOL bStatic)
{
    traceIn(D3DCubeTexture::D3DCubeTexture);

    assert(dwFormatIn > 0);

    d3d = curSystem;

    bHasMipMaps = bBuildMipMaps;
    bNeedsBlending = 0;
    mipLevel = 0;
    texWidth = width;
    bDynamic = !bStatic;
    dwFormat = dwFormatIn;
    textureData[0] =
    textureData[1] =
    textureData[2] =
    textureData[3] =
    textureData[4] =
    textureData[5] = NULL;

    if(bDynamic)
        d3d->DynamicTextureList << this;

    traceOut;
}

D3DCubeTexture::D3DCubeTexture(D3DSystem *curSystem, CTSTR lpFile, D3DXIMAGE_INFO &imageInfo, BOOL bBuildMipMaps)
{
    traceIn(D3DCubeTexture::D3DCubeTexture);

    DWORD dwMipLevels; 
    
    if(bBuildMipMaps)
    {
        switch(imageInfo.ImageFileFormat)
        {
            case D3DXIFF_DDS:
                dwMipLevels = D3DX_FROM_FILE;
                break;

            default:
                dwMipLevels = D3DX_DEFAULT;
        }
    }
    else
        dwMipLevels = 1;

    d3d = curSystem;

    bHasMipMaps = bBuildMipMaps;
    bNeedsBlending = 0;
    mipLevel = 0;
    dwFormat = -1;
    bDynamic = FALSE;
    dwTexType = D3DTEXTURE_STANDARD_BUFFER;

    if(!SUCCEEDED(D3DXCreateCubeTextureFromFileEx(d3d->d3dDevice, lpFile, D3DX_DEFAULT, dwMipLevels,
                                       0, D3DFMT_FROM_FILE, D3DPOOL_MANAGED, D3DX_DEFAULT,
                                       D3DX_DEFAULT, 0, NULL, NULL, &GetTex())))
        ErrOut(TEXT("Could not load file %s"), lpFile);

    D3DSURFACE_DESC sDesc;
    GetTex()->GetLevelDesc(0, &sDesc);

    texWidth = sDesc.Width;

    traceOut;
}

void D3DCubeTexture::CreateBuffer()
{
    traceIn(D3DCubeTexture::CreateBuffer);

    HRESULT problemo;

    if(dwTexType == D3DTEXTURE_FRAME_BUFFER)
    {
        switch(dwFormat)
        {
            case GS_RGBA:
                dwInternalFormat = D3DFMT_A8R8G8B8;
                bNeedsBlending = 1;
                break;
            case GS_RGBA16:
                dwInternalFormat = D3DFMT_A16B16G16R16;
                break;
            case GS_RGBA16F:
                dwInternalFormat = D3DFMT_A16B16G16R16F;
                break;
            case GS_RGBA32F:
                dwInternalFormat = D3DFMT_A32B32G32R32F;
                break;
            case GS_RG16F:
                dwInternalFormat = D3DFMT_G16R16F;
                break;
            case GS_RG32F:
                dwInternalFormat = D3DFMT_G32R32F;
                break;
            default:
                CrashError(TEXT("Unsupported cube render texture format"));
        }

        problemo = d3d->d3dDevice->CreateCubeTexture(texWidth, !bHasMipMaps, (bHasMipMaps ? D3DUSAGE_AUTOGENMIPMAP : 0) | D3DUSAGE_RENDERTARGET | (bDynamic ? D3DUSAGE_DYNAMIC : 0), (D3DFORMAT)dwInternalFormat, D3DPOOL_DEFAULT, &GetTex(), NULL);

        if(!SUCCEEDED(problemo))
        {
            if(problemo == D3DERR_OUTOFVIDEOMEMORY)
                ErrOut(TEXT("Error Creating D3D Render Texture: Out of Video Memory!"));
            else if(problemo == D3DERR_INVALIDCALL)
                ErrOut(TEXT("Error Creating D3D Render Texture: Invalid call, Format 0x%lX"), dwFormat);
            else if(problemo == E_OUTOFMEMORY)
                ErrOut(TEXT("Error Creating D3D Render Texture: Out of general memory!"));
            else if(problemo == D3DERR_DRIVERINTERNALERROR)
                ErrOut(TEXT("Error Creating D3D Render Texture: Internal driver error"));
            else
                ErrOut(TEXT("Could not create D3D Render Texture for some unknown reason:  exact error code is 0x%lX"), problemo);
        }
    }
    else
    {
        switch(dwFormat)
        {
            case GS_GRAYSCALE:
                dwInternalFormat = D3DFMT_L8;
                break;
            case GS_L16:
                dwInternalFormat = D3DFMT_L16;
                break;
            case GS_ALPHA:
                bNeedsBlending = 1;
                if(d3d->bA8Supported)
                    dwInternalFormat = D3DFMT_A8;
                else
                    dwInternalFormat = D3DFMT_A8L8;
                break;
            case GS_RGB:
                dwInternalFormat = D3DFMT_X8R8G8B8;
                break;
            case GS_RGBA:
                dwInternalFormat = D3DFMT_A8R8G8B8;
                bNeedsBlending = 1;
                break;
            case GS_RGBA16:
                bNeedsBlending = 1;
                dwInternalFormat = D3DFMT_A16B16G16R16;
                break;
            case GS_RGBA16F:
                bNeedsBlending = 1;
                dwInternalFormat = D3DFMT_A16B16G16R16F;
                break;
            case GS_RGBA32F:
                bNeedsBlending = 1;
                dwInternalFormat = D3DFMT_A32B32G32R32F;
                break;
            case GS_RG16F:
                dwInternalFormat = D3DFMT_G16R16F;
                break;
            case GS_RG32F:
                dwInternalFormat = D3DFMT_G32R32F;
                break;
            default:
                CrashError(TEXT("Unsupported cube texture format"));
        }

        if(!SUCCEEDED(problemo = d3d->d3dDevice->CreateCubeTexture(texWidth, bHasMipMaps ? 0 : 1, bDynamic ? D3DUSAGE_DYNAMIC : 0, (D3DFORMAT)dwInternalFormat, bDynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, &GetTex(), NULL)))
        {
            if(problemo == D3DERR_OUTOFVIDEOMEMORY)
                ErrOut(TEXT("Error Creating D3D Texture: Out of Video Memory!"));
            else if(problemo == D3DERR_INVALIDCALL)
                ErrOut(TEXT("Error Creating D3D Texture: Invalid format, Format 0x%lX"), dwFormat);
            else if(problemo == E_OUTOFMEMORY)
                ErrOut(TEXT("Error Creating D3D Texture: Out of general memory!"));
            else
                ErrOut(TEXT("Could not create D3D Texture for some unknown reason"));
        }
    }

    traceOut;
}

void D3DCubeTexture::FreeBuffer()
{
    traceIn(D3DCubeTexture::FreeBuffer);

    D3DRelease(GetTex());

    traceOut;
}

void D3DCubeTexture::SetImage(int side, void *lpData)
{
    traceIn(D3DCubeTexture::SetImage);

    assert((side >= 0) && (side < 6));
    assert(lpData);

    D3DLOCKED_RECT d3dRect;
    int i,j;

    //RECT rect = {0, 0, width, height};
    GetTex()->LockRect(D3DCubeSides[side], 0, &d3dRect, NULL, 0);

    if(dwFormat <= GS_GRAYSCALE) // if dwFormat is GS_ALPHA or GS_GRAYSCALE
    {
        if(dwInternalFormat == D3DFMT_A8L8)
        {
            LPWORD lpBits  = (LPWORD)d3dRect.pBits;
            LPBYTE lpInput = (LPBYTE)lpData;

            for(i=0; i<texWidth; i++)
            {
                for(j=0; j<texWidth; j++)
                {
                    WORD val = ((lpInput[(i*texWidth)+j]) << 8) | 0xFF;
                    lpBits[(i*(d3dRect.Pitch/2))+j] = val;
                }
            }
        }
        else
        {
            LPBYTE lpBits  = (LPBYTE)d3dRect.pBits;
            LPBYTE lpInput = (LPBYTE)lpData;

            for(i=0; i<texWidth; i++)
                mcpy(lpBits+(i*d3dRect.Pitch), lpInput+(i*texWidth), texWidth);
        }
    }
    else if(dwFormat == GS_RGB)
    {
        LPBYTE lpBits = (LPBYTE)d3dRect.pBits, lpInput = (LPBYTE)lpData;
        //DWORD widthX3 = texWidth*3;

        for(i=0; i<texWidth; i++)
        {
            //mcpy(lpBits+(i*d3dRect.Pitch), lpInput+(i*widthX3), widthX3);
            DWORD curY      = (i*texWidth*3);
            DWORD curD3DY   = (i*d3dRect.Pitch);

            for(j=0; j<texWidth; j++)
            {
                DWORD curX      = curY+(j*3);
                DWORD curD3DX   = curD3DY+(j*4);

                lpBits[curD3DX]   = lpInput[curX];
                lpBits[curD3DX+1] = lpInput[curX+1];
                lpBits[curD3DX+2] = lpInput[curX+2];
            }
        }

    }
    else if((dwFormat == GS_RGBA) || (dwFormat == GS_RG16F))
    {
        LPBYTE lpBits = (LPBYTE)d3dRect.pBits, lpInput = (LPBYTE)lpData;
        DWORD widthX4 = texWidth*4;

        for(i=0; i<texWidth; i++)
        {
            mcpy(lpBits+(i*d3dRect.Pitch), lpInput+(i*widthX4), widthX4);
            /*DWORD curY      = (i*texWidth*4);
            DWORD curD3DY   = (i*d3dRect.Pitch);

            for(j=0; j<texWidth; j++)
            {
                DWORD jx4       = (j*4);
                DWORD curX      = curY+jx4;
                DWORD curD3DX   = curD3DY+jx4;

                lpBits[curD3DX]   = lpInput[curX+2];
                lpBits[curD3DX+1] = lpInput[curX+1];
                lpBits[curD3DX+2] = lpInput[curX];
                lpBits[curD3DX+3] = lpInput[curX+3];
            }*/
        }
    }
    else if((dwFormat == GS_A8L8) || (dwFormat == GS_L16))
    {
        LPWORD lpBits  = (LPWORD)d3dRect.pBits;
        LPWORD lpInput = (LPWORD)lpData;
        DWORD widthX2 = texWidth*2;

        for(i=0; i<texWidth; i++)
            mcpy(lpBits+(i*d3dRect.Pitch), lpInput+(i*widthX2), widthX2);
    }
    else if((dwFormat == GS_RGBA16) || (dwFormat == GS_RGBA16F) || (dwFormat == GS_RG32F))
    {
        LPBYTE lpBits = (LPBYTE)d3dRect.pBits;
        LPWORD lpInput = (LPWORD)lpData;
        DWORD widthX8 = texWidth*8;

        for(i=0; i<texWidth; i++)
        {
            mcpy(lpBits+(i*d3dRect.Pitch), lpInput+(i*widthX8), widthX8);
            /*DWORD yOffset = (i*(texWidth*8));
            DWORD yTexOffset = (i*d3dRect.Pitch);
            for(j=0; j<texWidth; j++)
            {
                WORD *lpColors = (WORD*)&lpBits[yTexOffset+(j*8)];
                DWORD offset = yOffset+(j*8);

                lpColors[3] = lpInput[offset+2];
                lpColors[2] = lpInput[offset+1];
                lpColors[1] = lpInput[offset];
                lpColors[0] = lpInput[offset+3];
            }*/
        }
    }
    else if(dwFormat == GS_RGBA32F)
    {
        LPBYTE lpBits = (LPBYTE)d3dRect.pBits;
        LPDWORD lpInput = (LPDWORD)lpData;
        DWORD widthX16 = texWidth*16;

        for(i=0; i<texWidth; i++)
        {
            mcpy(lpBits+(i*d3dRect.Pitch), lpInput+(i*widthX16), widthX16);
            /*DWORD yOffset = (i*(texWidth*16));
            DWORD yTexOffset = (i*d3dRect.Pitch);
            for(j=0; j<texWidth; j++)
            {
                LPDWORD lpColors = (LPDWORD)&lpBits[yTexOffset+(j*8)];
                DWORD offset = yOffset+(j*16);

                lpColors[3] = lpInput[offset+2];
                lpColors[2] = lpInput[offset+1];
                lpColors[1] = lpInput[offset];
                lpColors[0] = lpInput[offset+3];
            }*/
        }
    }
    else
    {
        GetTex()->UnlockRect(D3DCubeSides[side], 0);
        ErrOut(TEXT("eep-chi, this message is required because a texture format needs CubeTexture::SetImage implementation."));
        return;
    }

    GetTex()->UnlockRect(D3DCubeSides[side], 0);

    if(bDynamic && (textureData[side] != lpData))
    {
        Free(textureData[side]);
        textureData[side] = (LPBYTE)lpData;
    }

    traceOut;
}

void* D3DCubeTexture::GetImage(int side)
{
    traceIn(D3DCubeTexture::GetImage);

    assert((side >= 0) && (side < 6));
    if(dwTexType == D3DTEXTURE_FRAME_BUFFER)
        ErrOut(TEXT("Tried to query image for frame buffer"));
    if(!bDynamic)
        ErrOut(TEXT("Tried to query image for non-dynamic texture"));

    return textureData[side];

    traceOut;
}

void D3DCubeTexture::RebuildMipMaps()
{
    traceInFast(D3DCubeTexture::RebuildMipMaps);

    if(bHasMipMaps)
        D3DXFilterTexture(GetTex(), NULL, 0, D3DX_DEFAULT);

    traceOutFast;
}

D3DCubeTexture::~D3DCubeTexture()
{
    traceIn(D3DCubeTexture::~D3DCubeTexture);

    FreeBuffer();

    if(bDynamic || (dwTexType == D3DTEXTURE_FRAME_BUFFER))
    {
        d3d->DynamicTextureList.RemoveItem(this);
        Free(textureData[0]);
        Free(textureData[1]);
        Free(textureData[2]);
        Free(textureData[3]);
        Free(textureData[4]);
        Free(textureData[5]);
    }

    traceOut;
}

void D3DCubeTexture::SetLOD(int level)
{
    if(bHasMipMaps)
        mipLevel = level;
}

DWORD D3DCubeTexture::Width()
{
    return texWidth;
}

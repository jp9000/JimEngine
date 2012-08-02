/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  D3DTexture.cpp:  Direct3D 9 Texture Management

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


D3DTexture::D3DTexture(D3DSystem *curSystem, unsigned int width, unsigned int height, DWORD dwFormatIn, BOOL bBuildMipMaps, BOOL bStatic)
{
    traceIn(D3DTexture::D3DTexture);

    assert(dwFormatIn > 0);

    d3d = curSystem;

    bHasMipMaps = bBuildMipMaps;
    bNeedsBlending = 0;
    mipLevel = 0;
    texWidth = width;
    texHeight = height;
    dwFormat = dwFormatIn;
    bDynamic = !bStatic;
    textureData = NULL;

    if(bDynamic)
        d3d->DynamicTextureList << this;

    traceOut;
}

D3DTexture::D3DTexture(D3DSystem *curSystem, CTSTR lpFile, D3DXIMAGE_INFO &imageInfo, BOOL bBuildMipMaps)
{
    traceIn(D3DTexture::D3DTexture(2));

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
    switch(imageInfo.Format)
    {
        case D3DFMT_DXT1:       dwFormat = GS_DXT1; break;
        case D3DFMT_DXT3:       dwFormat = GS_DXT3; break;
        case D3DFMT_DXT5:       dwFormat = GS_DXT5; break;
        case D3DFMT_R8G8B8:     dwFormat = GS_RGB;  break;
        case D3DFMT_A8R8G8B8:   dwFormat = GS_RGBA;
    }

    bDynamic = FALSE;
    dwTexType = D3DTEXTURE_STANDARD_BUFFER;

    DWORD dwDefaultSize = bBuildMipMaps ? D3DX_DEFAULT : D3DX_DEFAULT_NONPOW2;

    //D3DXCreateTextureFromFile(d3d->d3dDevice, lpFile, &d3dTex);

    if(!SUCCEEDED(D3DXCreateTextureFromFileEx(d3d->d3dDevice,
                                   lpFile,
                                   dwDefaultSize, dwDefaultSize,
                                   dwMipLevels,
                                   0, D3DFMT_FROM_FILE, D3DPOOL_MANAGED,
                                   D3DX_FILTER_NONE, D3DX_DEFAULT, 
                                   0, NULL, NULL, &GetTex())))
        ErrOut(TEXT("Could not load file %s"), lpFile);

    D3DSURFACE_DESC sDesc;
    GetTex()->GetLevelDesc(0, &sDesc);

    texWidth = sDesc.Width;
    texHeight = sDesc.Height;

    traceOut;
}

void D3DTexture::CreateBuffer()
{
    traceIn(D3DTexture::CreateBuffer);

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
                ErrOut(TEXT("render target texture format not supported"));
        }

        problemo = d3d->d3dDevice->CreateTexture(texWidth, texHeight, !bHasMipMaps, (bHasMipMaps ? D3DUSAGE_AUTOGENMIPMAP : 0) | D3DUSAGE_RENDERTARGET | (bDynamic ? D3DUSAGE_DYNAMIC : 0), (D3DFORMAT)dwInternalFormat, D3DPOOL_DEFAULT, &GetTex(), NULL);

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
            case GS_A8L8:
                dwInternalFormat = D3DFMT_A8L8;
                break;
            case GS_DXT1:
                dwInternalFormat = D3DFMT_DXT1;
                break;
            case GS_DXT3:
                dwInternalFormat = D3DFMT_DXT3;
                break;
            case GS_DXT5:
                dwInternalFormat = D3DFMT_DXT5;
                break;
            default:
                ErrOut(TEXT("texture format not supported"));
        }

        problemo = d3d->d3dDevice->CreateTexture(texWidth, texHeight, bHasMipMaps ? 0 : 1, bDynamic ? D3DUSAGE_DYNAMIC : 0, (D3DFORMAT)dwInternalFormat, bDynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, &GetTex(), NULL);

        if(!SUCCEEDED(problemo))
        {
            if(problemo == D3DERR_OUTOFVIDEOMEMORY)
                ErrOut(TEXT("Error Creating D3D Texture: Out of Video Memory!"));
            else if(problemo == D3DERR_INVALIDCALL)
                ErrOut(TEXT("Error Creating D3D Texture: Invalid call, Format 0x%lX"), dwFormat);
            else if(problemo == E_OUTOFMEMORY)
                ErrOut(TEXT("Error Creating D3D Texture: Out of general memory!"));
            else
                ErrOut(TEXT("Could not create D3D Texture for some unknown reason:  exact error code is 0x%lX"), problemo);
        }
    }

    traceOut;
}

void D3DTexture::FreeBuffer()
{
    D3DRelease(GetTex());
}

DWORD D3DTexture::GetSize()
{
    if(dwFormat <= GS_GRAYSCALE)
        return texWidth*texHeight;
    else if(dwFormat == GS_A8L8)
        return texWidth*texHeight*2;
    else if(dwFormat == GS_RGB)
        return texWidth*texHeight*3;
    else if(dwFormat == GS_DXT1)
        return ((texWidth+3)/4)*((texHeight+3)/4)*8;
    else if((dwFormat == GS_DXT3) || (dwFormat == GS_DXT5))
        return ((texWidth+3)/4)*((texHeight+3)/4)*16;
    else if(dwFormat == GS_RGBA)
        return texWidth*texHeight*4;
    else if(dwFormat == GS_RG16F)
        return texWidth*texHeight*4;
    else if(dwFormat == GS_RG32F)
        return texWidth*texHeight*8;
    else if((dwFormat == GS_RGBA16) || (dwFormat == GS_RGBA16F))
        return texWidth*texHeight*8;
    else if(dwFormat == GS_RGBA32F)
        return texWidth*texHeight*16;
    else
        return 0;
}

void D3DTexture::SetImage(void *lpData)
{
    traceInFast(D3DTexture::SetImage);

    assert(lpData);

    D3DLOCKED_RECT d3dRect;
    int i,j;

    HRESULT problemo = GetTex()->LockRect(0, &d3dRect, NULL, 0);

    if(!d3dRect.pBits)
        return;

    if(dwFormat <= GS_GRAYSCALE) // if dwFormat is GS_ALPHA or GS_GRAYSCALE
    {
        if(dwInternalFormat == D3DFMT_A8L8)
        {
            LPWORD lpBits  = (LPWORD)d3dRect.pBits;
            LPBYTE lpInput = (LPBYTE)lpData;

            for(i=0; i<texHeight; i++)
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

            for(i=0; i<texHeight; i++)
                mcpy(lpBits+(i*(d3dRect.Pitch)), lpInput+(i*texWidth), texWidth);
        }
    }
    else if((dwFormat == GS_A8L8) || (dwFormat == GS_L16))
    {
        LPWORD lpBits  = (LPWORD)d3dRect.pBits;
        LPWORD lpInput = (LPWORD)lpData;
        DWORD widthX2 = texWidth*2;

        for(i=0; i<texHeight; i++)
            mcpy(lpBits+(i*d3dRect.Pitch), lpInput+(i*widthX2), widthX2);
    }
    else if(dwFormat == GS_RGB)
    {
        LPBYTE lpBits = (LPBYTE)d3dRect.pBits, lpInput = (LPBYTE)lpData;
        //DWORD widthX3 = texWidth*3;

        for(i=0; i<texHeight; i++)
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
    else if(dwFormat == GS_DXT1)
    {
        LPBYTE lpBits = (LPBYTE)d3dRect.pBits, lpInput = (LPBYTE)lpData;

        DWORD tempWidth  = (texWidth+3)/4;
        DWORD tempHeight = (texHeight+3)/4;

        mcpy(lpBits, lpInput, tempWidth*tempHeight*8);
    }
    else if((dwFormat == GS_DXT3) || (dwFormat == GS_DXT5))
    {
        LPBYTE lpBits = (LPBYTE)d3dRect.pBits, lpInput = (LPBYTE)lpData;

        DWORD tempWidth  = (texWidth+3)/4;
        DWORD tempHeight = (texHeight+3)/4;

        mcpy(lpBits, lpInput, tempWidth*tempHeight*16);
    }
    else if((dwFormat == GS_RGBA) || (dwFormat == GS_RG16F))
    {
        LPBYTE lpBits = (LPBYTE)d3dRect.pBits, lpInput = (LPBYTE)lpData;
        DWORD widthX4 = texWidth*4;

        for(i=0; i<texHeight; i++)
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
    else if((dwFormat == GS_RGBA16) || (dwFormat == GS_RGBA16F) || (dwFormat == GS_RG32F))
    {
        LPBYTE lpBits = (LPBYTE)d3dRect.pBits;
        LPWORD lpInput = (LPWORD)lpData;
        DWORD widthX8 = texWidth*8;

        for(i=0; i<texHeight; i++)
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

        for(i=0; i<texHeight; i++)
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
        GetTex()->UnlockRect(0);
        ErrOut(TEXT("eep-chi, this message is required because a texture format needs Texture::SetImage implementation."));
        return;
    }

    GetTex()->UnlockRect(0);

    if(bHasMipMaps && (dwTexType == D3DTEXTURE_STANDARD_BUFFER))
        D3DXFilterTexture(GetTex(), NULL, 0, D3DX_DEFAULT);

    if(bDynamic && (textureData != lpData))
    {
        Free(textureData);
        textureData = (LPBYTE)lpData;
    }

    traceOutFast;
}

void* D3DTexture::GetImage(BOOL bForce, void *lpInputPtr)
{
    if(!bForce || (bDynamic && (dwTexType != D3DTEXTURE_FRAME_BUFFER)))
    {
        if(dwTexType == D3DTEXTURE_FRAME_BUFFER)
            ErrOut(TEXT("Tried to query image for frame buffer"));
        if(!bDynamic)
            ErrOut(TEXT("Tried to query image for non-dynamic texture"));
    }
    else
    {
        IDirect3DSurface9 *RenderSurface=NULL, *CopySurface=NULL;
        BOOL bSuccess=FALSE;
        LPVOID lpData=NULL;

        D3DLOCKED_RECT d3dRect;
        int i,j;

        if(dwTexType == D3DTEXTURE_FRAME_BUFFER)
        {
            if(SUCCEEDED(GetTex()->GetSurfaceLevel(0, &RenderSurface)))
            {
                profileSegment("is it this?"); //3.14%
                if(SUCCEEDED(d3d->d3dDevice->CreateOffscreenPlainSurface(Width(), Height(), (D3DFORMAT)dwInternalFormat, D3DPOOL_SYSTEMMEM, &CopySurface, NULL)))
                {
                    profileSegment("or is it this?"); //5.48%
                    if(SUCCEEDED(d3d->d3dDevice->GetRenderTargetData(RenderSurface, CopySurface)))
                    {
                        profileSegment("or how about this?");
                        if(SUCCEEDED(CopySurface->LockRect(&d3dRect, NULL, 0)))
                            bSuccess = TRUE;
                    }
                }
            }

            if(RenderSurface)
                RenderSurface->Release();

            if(!bSuccess)
            {
                if(CopySurface)
                    CopySurface->Release();

                return NULL;
            }
        }
        else
        {
            if(!SUCCEEDED(GetTex()->GetSurfaceLevel(0, &CopySurface)))
                return NULL;
            if(!SUCCEEDED(CopySurface->LockRect(&d3dRect, NULL, 0)))
            {
                CopySurface->Release();
                return NULL;
            }
        }

        //-------------------------------------------------------

        if(dwFormat <= GS_GRAYSCALE)
        {
            lpData = lpInputPtr ? lpInputPtr : Allocate(Width()*Height());

            LPBYTE lpBits  = (LPBYTE)d3dRect.pBits;
            LPBYTE lpInput = (LPBYTE)lpData;

            for(i=0; i<texHeight; i++)
                mcpy(lpInput+(i*texWidth), lpBits+(i*(d3dRect.Pitch)), texWidth);
        }
        else if(dwFormat == GS_A8L8)
        {
            lpData = lpInputPtr ? lpInputPtr : Allocate(Width()*Height()*2);

            LPWORD lpBits  = (LPWORD)d3dRect.pBits;
            LPWORD lpInput = (LPWORD)lpData;
            DWORD widthX2 = texWidth*2;

            for(i=0; i<texHeight; i++)
                mcpy(lpInput+(i*widthX2), lpBits+(i*d3dRect.Pitch), widthX2);
        }
        else if(dwFormat == GS_RGB)
        {
            lpData = lpInputPtr ? lpInputPtr : Allocate(Width()*Height()*3);

            LPBYTE lpBits = (LPBYTE)d3dRect.pBits, lpInput = (LPBYTE)lpData;
            //DWORD widthX3 = texWidth*3;

            for(i=0; i<texHeight; i++)
            {
                //mcpy(lpBits+(i*d3dRect.Pitch), lpInput+(i*widthX3), widthX3);
                DWORD curY      = (i*texWidth*3);
                DWORD curD3DY   = (i*d3dRect.Pitch);

                for(j=0; j<texWidth; j++)
                {
                    DWORD curX      = curY+(j*3);
                    DWORD curD3DX   = curD3DY+(j*4);

                    lpInput[curX]   = lpBits[curD3DX];
                    lpInput[curX+1] = lpBits[curD3DX+1];
                    lpInput[curX+2] = lpBits[curD3DX+2];
                }
            }
        }
        else if(dwFormat == GS_DXT1)
        {
            LPBYTE lpBits = (LPBYTE)d3dRect.pBits;

            DWORD tempWidth  = (texWidth+3)/4;
            DWORD tempHeight = (texHeight+3)/4;

            lpData = lpInputPtr ? lpInputPtr : Allocate(tempWidth*tempHeight*8);
            mcpy(lpData, lpBits, tempWidth*tempHeight*8);
        }
        else if((dwFormat == GS_DXT3) || (dwFormat == GS_DXT5))
        {
            LPBYTE lpBits = (LPBYTE)d3dRect.pBits;

            DWORD tempWidth  = (texWidth+3)/4;
            DWORD tempHeight = (texHeight+3)/4;

            lpData = lpInputPtr ? lpInputPtr : Allocate(tempWidth*tempHeight*16);
            mcpy(lpData, lpBits, tempWidth*tempHeight*16);
        }
        else if((dwFormat == GS_RGBA) || (dwFormat == GS_RG16F))
        {
            lpData = lpInputPtr ? lpInputPtr : Allocate(Width()*Height()*4);

            LPBYTE lpBits = (LPBYTE)d3dRect.pBits, lpInput = (LPBYTE)lpData;
            DWORD widthX4 = texWidth*4;

            for(i=0; i<texHeight; i++)
                mcpy(lpInput+(i*widthX4), lpBits+(i*d3dRect.Pitch), widthX4);
        }
        else if((dwFormat == GS_RGBA16) || (dwFormat == GS_RG32F))
        {
            lpData = lpInputPtr ? lpInputPtr : Allocate(Width()*Height()*2*4);

            LPBYTE lpBits = (LPBYTE)d3dRect.pBits;
            LPWORD lpInput = (LPWORD)lpData;
            DWORD widthX8 = texWidth*8;

            for(i=0; i<texHeight; i++)
                mcpy(lpInput+(i*widthX8), lpBits+(i*d3dRect.Pitch), widthX8);
        }
        else if(dwFormat == GS_RGBA16F) //converts to 8bit RGBA
        {
            lpData = lpInputPtr ? lpInputPtr : Allocate(Width()*Height()*sizeof(Vect4));

            LPBYTE lpBits = (LPBYTE)d3dRect.pBits;
            Vect4* lpInput = (Vect4*)lpData;
            DWORD widthXVect = texWidth*sizeof(Vect4);

            for(i=0; i<texHeight; i++)
            {
                DWORD curY      = (i*texWidth);
                DWORD curD3DY   = (i*d3dRect.Pitch);

                for(j=0; j<texWidth; j++)
                {
                    DWORD curX      = curY+(j*4);
                    DWORD curD3DX   = curD3DY+(j*2*4);

                    lpInput[curY+j].x = ((FLOAT)*(D3DXFLOAT16*)(lpBits+(curD3DX)));
                    lpInput[curY+j].y = ((FLOAT)*(D3DXFLOAT16*)(lpBits+(curD3DX+2)));
                    lpInput[curY+j].z = ((FLOAT)*(D3DXFLOAT16*)(lpBits+(curD3DX+4)));
                    lpInput[curY+j].w = ((FLOAT)*(D3DXFLOAT16*)(lpBits+(curD3DX+6)));
                }
            }
        }
        else if(dwFormat == GS_RGBA32F) //converts to 8bit RGBA
        {
            lpData = lpInputPtr ? lpInputPtr : Allocate(Width()*Height()*sizeof(Vect4));

            LPBYTE lpBits = (LPBYTE)d3dRect.pBits;
            Vect4* lpInput = (Vect4*)lpData;
            DWORD widthXVect = texWidth*sizeof(Vect4);

            if(widthXVect == d3dRect.Pitch)
                mcpy(lpInput, lpBits, widthXVect*texHeight);
            else
            {
                for(i=0; i<texHeight; i++)
                    mcpy(lpInput+(i*texWidth), lpBits+(i*d3dRect.Pitch), widthXVect);
            }
        }

        //-------------------------------------------------------

        CopySurface->UnlockRect();
        CopySurface->Release();

        return lpData;
    }

    return textureData;
}

BOOL D3DTexture::GetRenderTargetImage(OffscreenSurface *destination)
{
    if(dwTexType != D3DTEXTURE_FRAME_BUFFER)
    {
        AppWarning(TEXT("D3DTexture::GetRenderTargetImage can only be used with a render target"));
        return FALSE;
    }

    if(!destination)
        return FALSE;

    IDirect3DSurface9 *RenderSurface=NULL;
    BOOL bSuccess=FALSE;
    D3DOffscreenSurface *oss = static_cast<D3DOffscreenSurface*>(destination);

    if(oss->bLocked)
    {
        AppWarning(TEXT("D3DTexture::GetRenderTargetImage cannot be used while destination is locked"));
        return FALSE;
    }

    if(SUCCEEDED(GetTex()->GetSurfaceLevel(0, &RenderSurface)))
    {
        if(SUCCEEDED(d3d->d3dDevice->GetRenderTargetData(RenderSurface, oss->surface)))
            bSuccess = TRUE;

        RenderSurface->Release();
    }

    return bSuccess;
}

D3DTexture::~D3DTexture()
{
    traceIn(D3DTexture::~D3DTexture);

    FreeBuffer();

    if(bDynamic || (dwTexType == D3DTEXTURE_FRAME_BUFFER))
    {
        d3d->DynamicTextureList.RemoveItem(this);
        Free(textureData);
    }

    traceOut;
}

DWORD D3DTexture::Width()
{
    return texWidth;
}

DWORD D3DTexture::Height()
{
    return texHeight;
}

BOOL  D3DTexture::HasAlpha()
{
    return bNeedsBlending;
}

void  D3DTexture::SetLOD(int level)
{
    if(bHasMipMaps)
        mipLevel = level;
}

void  D3DTexture::RebuildMipMaps()
{
    GetTex()->GenerateMipSubLevels();
}



D3DOffscreenSurface::~D3DOffscreenSurface()
{
    if(bLocked)
        surface->UnlockRect();
    surface->Release();
}

LPVOID D3DOffscreenSurface::Lock(BOOL bReadOnly)
{
    if(bLocked)
        return NULL;

    D3DLOCKED_RECT d3dRect;
    if(!SUCCEEDED(surface->LockRect(&d3dRect, NULL, bReadOnly ? D3DLOCK_READONLY : 0)))
        return NULL;

    bLocked = TRUE;
    return d3dRect.pBits;
}

void D3DOffscreenSurface::Unlock()
{
    if(bLocked)
    {
        surface->UnlockRect();
        bLocked = FALSE;
    }
}


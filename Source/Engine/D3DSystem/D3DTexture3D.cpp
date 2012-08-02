/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  D3DTexture3D.h

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



D3DTexture3D::D3DTexture3D(D3DSystem *curSystem, unsigned int width, unsigned int height, unsigned int depth, DWORD nColors, void *lpData, BOOL bBuildMipMaps)
{
    traceIn(D3DTexture3D::D3DTexture3D);

    assert((nColors > 0) && (nColors < 5));

    d3d = curSystem;

    DWORD format;

    bHasMipMaps = bBuildMipMaps;
    bNeedsBlending = 0;
    texWidth = width;
    texHeight = height;
    texDepth = depth;

    switch(nColors)
    {
        case 1:
            bNeedsBlending = 1;
            format = D3DFMT_A8R8G8B8;
            break;
        case 3:
            format = D3DFMT_X8R8G8B8;
            break;
        case 4:
            format = D3DFMT_A8R8G8B8;
            bNeedsBlending = 1;
    }

    HRESULT problemo;

    if(!SUCCEEDED(problemo = d3d->d3dDevice->CreateVolumeTexture(width, height, depth, bHasMipMaps ? 0 : 1, 0, (D3DFORMAT)format, D3DPOOL_MANAGED, &GetTex(), NULL)))
    {
        if(problemo == D3DERR_OUTOFVIDEOMEMORY)
            ErrOut(TEXT("Error Creating D3D Texture: Out of Video Memory!"));
        else if(problemo == D3DERR_INVALIDCALL)
            ErrOut(TEXT("Error Creating D3D Texture: Invalid format, %d colors"), nColors);
        else if(problemo == E_OUTOFMEMORY)
            ErrOut(TEXT("Error Creating D3D Texture: Out of general memory!"));
        else
            ErrOut(TEXT("Could not create D3D Texture for some unknown reason"));
    }

    D3DLOCKED_BOX d3dBox;
    int x,y,z;

    //RECT rect = {0, 0, width, height};
    GetTex()->LockBox(0, &d3dBox, NULL, 0);
    if(nColors == 1)
    {
        LPDWORD lpBits = (LPDWORD)d3dBox.pBits;
        LPBYTE lpInput = (LPBYTE)lpData;

        for(z=0; z<depth; z++)
        {
            DWORD OffsetZ = depth*(height*width);
            DWORD OutOffsetZ = depth*d3dBox.SlicePitch;

            for(y=0; y<height; y++)
            {
                DWORD OffsetY = y*width;
                DWORD OutOffsetY = y*d3dBox.RowPitch;

                for(x=0; x<width; x++)
                {
                    DWORD val = ((lpInput[OffsetZ+OffsetY+x]) << 24) | 0xFFFFFF;
                    //mcpy(&lpBits[(i*d3dRect.Pitch)+(j*4)], &val, 4);
                    lpBits[OutOffsetZ+OutOffsetY+x] = val;
                }
            }
        }

    }
    else if(nColors == 3)
    {
        LPBYTE lpBits = (LPBYTE)d3dBox.pBits, lpInput = (LPBYTE)lpData;

        for(z=0; z<depth; z++)
        {
            DWORD OffsetZ = z*(height*(width*3));
            DWORD OutOffsetZ = z*d3dBox.SlicePitch;

            for(y=0; y<height; y++)
            {
                DWORD OffsetY = y*(width*3);
                DWORD OutOffsetY = y*d3dBox.RowPitch;

                for(x=0; x<width; x++)
                {
                    DWORD OffsetX = x*3;
                    DWORD OutOffsetX = x*4;

                    lpBits[OutOffsetZ+OutOffsetY+OutOffsetX]   = lpInput[OffsetZ+OffsetY+OffsetX+2];
                    lpBits[OutOffsetZ+OutOffsetY+OutOffsetX+1] = lpInput[OffsetZ+OffsetY+OffsetX+1];
                    lpBits[OutOffsetZ+OutOffsetY+OutOffsetX+2] = lpInput[OffsetZ+OffsetY+OffsetX];
                }
            }
        }

    }
    else
    {
        for(z=0; z<depth; z++)
        {
            DWORD OffsetZ = z*(height*(width*4));
            DWORD OutOffsetZ = z*d3dBox.SlicePitch;

            for(y=0; y<height; y++)
            {
                DWORD OffsetY = y*(width*4);
                DWORD OutOffsetY = y*d3dBox.RowPitch;

                mcpy((BYTE*)d3dBox.pBits+OutOffsetZ+OutOffsetY, (BYTE*)lpData+OffsetZ+OffsetY, width*nColors);
            }
        }
    }

    /*if(bHasMipMaps)
    {
        LPDWORD lpBits = (LPDWORD)d3dRect.pBits;
        DWORD dwAverageFactor = 2, dwCurrentLevel = 1;
        DWORD oldPitch = d3dRect.Pitch/4;
        D3DLOCKED_RECT mipmapRect;

        do
        {
            DWORD newWidth    = width  / dwAverageFactor;
            DWORD newHeight   = height / dwAverageFactor;
            GetTex()->LockRect(dwCurrentLevel, &mipmapRect, NULL, 0);
            LPDWORD lpMipBits = (LPDWORD)mipmapRect.pBits;
            DWORD newPitch = (mipmapRect.Pitch/4);

            for(i=0; i<newHeight; i++)
            {
                for(j=0; j<newWidth; j++)
                {
                    DWORD  R = 0;
                    DWORD  G = 0;
                    DWORD  B = 0;
                    DWORD  A = 0;
                    

                    for(int k=0; k<dwAverageFactor; k++)
                    {
                        for(int l=0; l<dwAverageFactor; l++)
                        {
                            //lpMipBits[(i*newPitch)+j] = lpBits[(i*dwAverageFactor*oldPitch)+(j*dwAverageFactor)];
                            DWORD temp = lpBits[(((i*dwAverageFactor)+k)*oldPitch)+((j*dwAverageFactor)+l)];
                            R += RGB_R(temp);
                            G += RGB_G(temp);
                            B += RGB_B(temp);
                            A += RGB_A(temp);
                        }
                    }
                    DWORD dwAveragex2 = (dwAverageFactor*dwAverageFactor);
                    R /= dwAveragex2;
                    G /= dwAveragex2;
                    B /= dwAveragex2;
                    A /= dwAveragex2;
                    lpMipBits[(i*newPitch)+j] = (DWORD)MAKERGBA(R, G, B, A);
                }
            }

            GetTex()->UnlockRect(dwCurrentLevel++);
            dwAverageFactor *= 2;
        }while(mipmapRect.Pitch > 4);
    }*/

    GetTex()->UnlockBox(0);

    if(bBuildMipMaps)
        D3DXFilterTexture(GetTex(), NULL, 0, D3DX_DEFAULT);

    traceOut;
}

D3DTexture3D::D3DTexture3D(D3DSystem *curSystem, CTSTR lpFile, D3DXIMAGE_INFO &imageInfo, BOOL bBuildMipMaps)
{
    traceIn(D3DTexture3D::D3DTexture3D(2));

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

    if(!SUCCEEDED(D3DXCreateVolumeTextureFromFileEx(d3d->d3dDevice, lpFile,
                                         D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
                                         dwMipLevels, 0, D3DFMT_FROM_FILE, D3DPOOL_MANAGED,
                                         D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &GetTex())))
        ErrOut(TEXT("Could not load file %s"), lpFile);

    D3DVOLUME_DESC vDesc;
    GetTex()->GetLevelDesc(0, &vDesc);

    texWidth = vDesc.Width;
    texHeight = vDesc.Height;
    texDepth = vDesc.Depth;

    traceOut;
}

D3DTexture3D::~D3DTexture3D()
{
    traceIn(D3DTexture3D::~D3DTexture3D);

    D3DRelease(GetTex());

    traceOut;
}

void D3DTexture3D::SetLOD(int level)
{
    if(bHasMipMaps)
        mipLevel = level;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  D3DSystem.cpp

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


DefineClass(D3DSampler);
DefineClass(D3DVertexBuffer);
DefineClass(D3DIndexBuffer);
DefineClass(D3DTexture);
DefineClass(D3DOffscreenSurface);
DefineClass(D3DTexture3D);
DefineClass(D3DCubeTexture);
DefineClass(D3DZStencilBuffer);
DefineClass(D3DShader);
DefineClass(D3DHardwareLight);

DefineClass(D3DSystem);


const DWORD D3DTest[]         = {D3DCMP_NEVER, D3DCMP_LESS, D3DCMP_LESSEQUAL, D3DCMP_EQUAL, D3DCMP_GREATEREQUAL, D3DCMP_GREATER, D3DCMP_NOTEQUAL, D3DCMP_ALWAYS};
const DWORD D3DStencilOps[]   = {D3DSTENCILOP_KEEP, D3DSTENCILOP_ZERO, D3DSTENCILOP_REPLACE, D3DSTENCILOP_INCR, D3DSTENCILOP_DECR, D3DSTENCILOP_INVERT};
const DWORD D3DBlendFuncs[]   = {D3DBLEND_ZERO, D3DBLEND_ONE, D3DBLEND_SRCCOLOR, D3DBLEND_INVSRCCOLOR, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_DESTCOLOR, D3DBLEND_INVDESTCOLOR, D3DBLEND_DESTALPHA, D3DBLEND_INVDESTALPHA, D3DBLEND_SRCALPHASAT};
const DWORD D3DDrawTypes[]    = {D3DPT_POINTLIST, D3DPT_LINELIST, D3DPT_LINESTRIP, D3DPT_TRIANGLELIST, D3DPT_TRIANGLESTRIP, D3DPT_TRIANGLEFAN};


const DWORD D3DAddressModes[] = {D3DTADDRESS_CLAMP, D3DTADDRESS_WRAP, D3DTADDRESS_MIRROR, D3DTADDRESS_BORDER, D3DTADDRESS_MIRRORONCE};


IDirect3D9 *d3dMain = NULL;


inline DWORD ENGINEAPI GetD3DPrimitiveSize(DWORD DrawType, DWORD nVerts);


const DWORD VS_bare[24] =
   {0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000,
    0x00000009, 0xC0010000, 0xA0E40000, 0x90E40000,
    0x00000009, 0xC0020000, 0xA0E40001, 0x90E40000,
    0x00000009, 0xC0040000, 0xA0E40002, 0x90E40000,
    0x00000009, 0xC0080000, 0xA0E40003, 0x90E40000,
    0x00000001, 0xD00F0000, 0xA0E40004, 0x0000FFFF};


#define SPIRTEVERTEXFORMAT (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define CUBEPROJVERTEXFORMAT (D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0))

float TBTriangle[12] =
{
    0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f
};

struct SpriteVertex
{
    float x, y, z, rhw;
    DWORD color;
    float u, v;
};


struct CubeProjVertex
{
    float x, y, z, rhw;
    float u, v, w;
};

BOOL ENGINEAPI DllMain(HINSTANCE hInst, DWORD dwWhy, LPVOID lpRes)
{
    return 1;
}


D3DSystem::D3DSystem()
{
    traceIn(D3DSystem::D3DSystem);

    /////////////////////
    //Initialize directx
    if(!d3dMain)
    {
        curVB = NULL;
        curIB = NULL;

        d3dMain = Direct3DCreate9(D3D_SDK_VERSION);

        d3dMain->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps);
        
        if( ((d3dCaps.PixelShaderVersion&0xFFFF)  < 0x101) || 
            ((d3dCaps.VertexShaderVersion&0xFFFF) < 0x101))
        {
            ErrOut(TEXT("This hardware does not support the version of pixel or vertex shaders required"));
        }

        Log(TEXT("Direct3D Device Initialized"));

        D3DADAPTER_IDENTIFIER9 driverinfo;
        d3dMain->GetAdapterIdentifier(0, 0, &driverinfo);

        DWORD dwProduct     = HIWORD(driverinfo.DriverVersion.HighPart);
        DWORD dwVersion     = LOWORD(driverinfo.DriverVersion.HighPart);
        DWORD dwSubVersion  = HIWORD(driverinfo.DriverVersion.LowPart);
        DWORD dwBuild       = LOWORD(driverinfo.DriverVersion.LowPart);

        Log(TEXT("Direct3D Display Driver: %S"), driverinfo.Driver);
        Log(TEXT("Video Adapter: %S"), driverinfo.Description);
        Log(TEXT("Driver Version: %d.%d.%d.%d"), dwProduct, dwVersion, dwSubVersion, dwBuild);
        //Log(TEXT("Available Texture Memory: %d megabytes"), d3dDevice->GetAvailableTextureMem()/1048576);

        Log(TEXT("Pixel Shader Version: %lX, Vertex Shader Version: %lX"), d3dCaps.PixelShaderVersion&0xFFFF, d3dCaps.VertexShaderVersion&0xFFFF);

        bUseTripleBuffering = AppConfig->GetInt(TEXT("Rendering"), TEXT("UseTripleBuffering"));
        if(bUseTripleBuffering)
            Log(TEXT("Triple Buffering On"));
    }
    else
        d3dMain->AddRef();

    traceOut;
}


BOOL D3DSystem::InitializeDevice(HANDLE hWindow)
{
    traceIn(D3DSystem::InitializeDevice);

    HRESULT hErr;

    curWindow = hWindow;

    SetWindowLongPtr((HWND)hWindow, 0, (LONG_PTR)this);

    D3DPRESENT_PARAMETERS pp;
    zero(&pp, sizeof(D3DPRESENT_PARAMETERS));

    curTextureBuffer = 0;

    pp.Windowed                 = 1;
    pp.SwapEffect               = D3DSWAPEFFECT_FLIP;
    pp.BackBufferFormat         = D3DFMT_A8R8G8B8;
    pp.BackBufferCount          = AppConfig->GetInt(TEXT("Display"), TEXT("BackBufferCount"), 1);
    pp.EnableAutoDepthStencil   = 1;              //these don't wanna work right...
    pp.AutoDepthStencilFormat   = D3DFMT_D24S8;
    pp.hDeviceWindow            = (HWND)curWindow;
    if(AppConfig->GetInt(TEXT("Display"), TEXT("vsync"), 0))
        pp.PresentationInterval     = D3DPRESENT_INTERVAL_ONE;
    else
        pp.PresentationInterval     = D3DPRESENT_INTERVAL_IMMEDIATE;

    if(!SUCCEEDED(hErr = d3dMain->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND)curWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &d3dDevice)))
    {
        ErrOut(TEXT("Could not create D3D Device, most likely your hardware is unsupported."));
        return FALSE;
    }

    d3dDevice->SetRenderState(D3DRS_ZENABLE, 1);
    d3dDevice->SetRenderState(D3DRS_LIGHTING, 0);
    d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
    d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    d3dDevice->SetRenderState(D3DRS_SPECULARENABLE, 1);
    d3dDevice->SetRenderState(D3DRS_LOCALVIEWER, 1);
    d3dDevice->SetRenderState(D3DRS_AMBIENT, 0xFFFFFFFF);

    d3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

    d3dDevice->LightEnable(0, FALSE);

    bTexturingEnabled = 1;

    //Initialize data structures
    zero(&mainMaterial, sizeof(mainMaterial));
    mainMaterial.Diffuse.r = 1.0;
    mainMaterial.Diffuse.g = 1.0;
    mainMaterial.Diffuse.b = 1.0;
    mainMaterial.Diffuse.a = 1.0;
    mainMaterial.Ambient.r = 0.3;
    mainMaterial.Ambient.g = 0.3;
    mainMaterial.Ambient.b = 0.3;
    mainMaterial.Ambient.a = 1.0;
    mainMaterial.Specular.a = 1.0;
    mainMaterial.Emissive.a = 1.0;
    bHardwareLightingEnabled = 1;

    D3DVERTEXELEMENT9 BareElements[2] = {{0, 0, D3DDECLTYPE_FLOAT3, 0, D3DDECLUSAGE_POSITION, 0}, D3DDECL_END()};

    if(!SUCCEEDED(d3dDevice->CreateVertexDeclaration(BareElements, &BareDeclaration)))
        ErrOut(TEXT("Could not create 'bare' vertex declaration"));
    if(!SUCCEEDED(d3dDevice->CreateVertexShader(VS_bare, &BareVertexShader)))
        ErrOut(TEXT("Could not create 'bare' vertex shader"));

    if(!SUCCEEDED(d3dDevice->CreateVertexBuffer(4*sizeof(SpriteVertex), D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, SPIRTEVERTEXFORMAT, D3DPOOL_DEFAULT, &spriteBuffer, NULL)))
        ErrOut(TEXT("Could not create the sprite vertex buffer"));

    if(!SUCCEEDED(d3dDevice->CreateRenderTarget(2, 2, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &TB[0], NULL)))
        ErrOut(TEXT("Could not create the render targets for double buffering"));
    d3dDevice->CreateRenderTarget(2, 2, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &TB[1], NULL);
    d3dDevice->CreateRenderTarget(2, 2, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &TB[2], NULL);

    d3dDevice->GetRenderTarget(0, &MainColorSurface);
    d3dDevice->GetDepthStencilSurface(&MainDepthStencilSurface);

    CurrentDepthStencilSurface = MainDepthStencilSurface;
    CurrentColorSurface = MainColorSurface;

    d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

    for(int i=1; i<8; i++)
    {
        d3dDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_MODULATE);
        d3dDevice->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    }

    bA8Supported = SUCCEEDED(d3dMain->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, 0, D3DRTYPE_TEXTURE, D3DFMT_A8));

    minAnisoVal = (d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) ? D3DTEXF_ANISOTROPIC : D3DTEXF_LINEAR;
    magAnisoVal = (d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC) ? D3DTEXF_ANISOTROPIC : D3DTEXF_LINEAR;

    return TRUE;

    traceOut;
}

D3DSystem::~D3DSystem()
{
    traceIn(D3DSystem::~D3DSystem);

    SetWindowLong((HWND)curWindow, 0, (LONG)NULL);

    D3DRelease(MainDepthStencilSurface);
    D3DRelease(MainColorSurface);

    D3DRelease(TB[2]);
    D3DRelease(TB[1]);
    D3DRelease(TB[0]);

    D3DRelease(spriteBuffer);

    D3DRelease(BareVertexShader);
    D3DRelease(BareDeclaration);

    D3DRelease(d3dDevice);

    if(d3dMain->Release() == 1)
    {
        d3dMain = NULL;
        Log(TEXT("Direct3D Device Destroyed"));
    }

    traceOut;
}

TSTR D3DSystem::GetDeviceName()
{
    return TEXT("Direct3D");
}

DWORD D3DSystem::VertexShaderVersion()
{
    return (d3dCaps.VertexShaderVersion&0xFFFF);
}

DWORD D3DSystem::PixelShaderVersion()
{
    return (d3dCaps.PixelShaderVersion&0xFFFF);
}

BOOL D3DSystem::SupportsTwoSidedStencil()
{
    return (d3dCaps.StencilCaps&D3DSTENCILCAPS_TWOSIDED) != 0;
}


HardwareLight* D3DSystem::CreateHardwareLight(DWORD dwType)
{
    return (HardwareLight*)InitializeObjectData(new D3DHardwareLight(this, dwType));
}

void  D3DSystem::EnableHardwareLighting(BOOL bEnable)
{
    bHardwareLightingEnabled = bEnable;
    d3dDevice->SetRenderState(D3DRS_LIGHTING, bEnable);
}

void  D3DSystem::SetFogType(int mode)
{
    DWORD iType;

    switch(mode)
    {
        case FOG_LINEAR:
            iType = D3DFOG_LINEAR;
            break;
        case FOG_EXP:
            iType = D3DFOG_EXP;
            break;
        case FOG_EXP2:
            iType = D3DFOG_EXP2;
            break;
        case FOG_NONE:
            iType = D3DFOG_NONE;
    }

    d3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, iType);
    if(mode != FOG_NONE)
        d3dDevice->SetRenderState(D3DRS_FOGENABLE, 1);
    else
        d3dDevice->SetRenderState(D3DRS_FOGENABLE, 0);
}


void  D3DSystem::SetFogColor(DWORD dwRGB)
{
    d3dDevice->SetRenderState(D3DRS_FOGCOLOR, dwRGB);
}

void  D3DSystem::SetFogDensity(float density)
{
    d3dDevice->SetRenderState(D3DRS_FOGDENSITY, *(DWORD*)&density);
}

void  D3DSystem::SetFogStart(float start)
{
    d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD*)&start);
}

void  D3DSystem::SetFogEnd(float end)
{
    d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD*)&end);
}


void  D3DSystem::SetMaterialColor(const Color4 &rgba)
{
    //might need to take the amient one off if you want normal lighting
    mcpy(&mainMaterial.Ambient, &rgba, sizeof(Color4));
    mcpy(&mainMaterial.Diffuse, &rgba, sizeof(Color4));
    d3dDevice->SetMaterial(&mainMaterial);
}

void  D3DSystem::SetMaterialSpecular(const Color4 &rgb)
{
    mcpy(&mainMaterial.Specular, &rgb, sizeof(Color4));
    d3dDevice->SetMaterial(&mainMaterial);
}

void  D3DSystem::SetMaterialShininess(float shininess)
{
    mainMaterial.Power = shininess;
    d3dDevice->SetMaterial(&mainMaterial);
}

void  D3DSystem::SetMaterialIllumination(const Color4 &rgb)
{
    mcpy(&mainMaterial.Emissive, &rgb, sizeof(Color4));
    d3dDevice->SetMaterial(&mainMaterial);
}


Texture* D3DSystem::CreateTexture(unsigned int width, unsigned int height, DWORD nColors, void *lpData, BOOL bBuildMipMaps, BOOL bStatic)
{
    D3DTexture *tex = (D3DTexture*)InitializeObjectData(new D3DTexture(this, width, height, nColors, bBuildMipMaps, bStatic));
    tex->dwTexType = D3DTEXTURE_STANDARD_BUFFER;

    tex->CreateBuffer();

    if(lpData)
        tex->SetImage(lpData);

    return tex;
}

CubeTexture* D3DSystem::CreateCubeTexture(unsigned int width, DWORD dwColorFormat, BOOL bBuildMipMaps, BOOL bStatic)
{
    D3DCubeTexture *tex = (D3DCubeTexture*)InitializeObjectData(new D3DCubeTexture(this, width, dwColorFormat, bBuildMipMaps, bStatic));
    tex->dwTexType = D3DTEXTURE_STANDARD_BUFFER;

    tex->CreateBuffer();

    return tex;
}

Texture3D* D3DSystem::Create3DTexture(unsigned int width, unsigned int height, unsigned int depth, DWORD nColors, void *lpData, BOOL bBuildMipMaps)
{
    return (Texture3D*)InitializeObjectData(new D3DTexture3D(this, width, height, depth, nColors, lpData, bBuildMipMaps));
}


Texture* D3DSystem::CreateTextureFromFile(CTSTR lpFile, BOOL bBuildMipMaps)
{
    D3DXIMAGE_INFO imageInfo;
    D3DXGetImageInfoFromFile(lpFile, &imageInfo);

    if(imageInfo.ResourceType == D3DRTYPE_TEXTURE)
        return (Texture*)InitializeObjectData(new D3DTexture(this, lpFile, imageInfo, bBuildMipMaps));

    return NULL;
}

CubeTexture* D3DSystem::CreateCubeTextureFromFile(CTSTR lpFile, BOOL bBuildMipMaps)
{
    D3DXIMAGE_INFO imageInfo;
    D3DXGetImageInfoFromFile(lpFile, &imageInfo);

    if(imageInfo.ResourceType == D3DRTYPE_CUBETEXTURE)
        return (CubeTexture*)InitializeObjectData(new D3DCubeTexture(this, lpFile, imageInfo, bBuildMipMaps));

    return NULL;
}

Texture3D* D3DSystem::Create3DTextureFromFile(CTSTR lpFile, BOOL bBuildMipMaps)
{
    D3DXIMAGE_INFO imageInfo;
    D3DXGetImageInfoFromFile(lpFile, &imageInfo);

    if(imageInfo.ResourceType == D3DRTYPE_VOLUMETEXTURE)
        return (Texture3D*)InitializeObjectData(new D3DTexture3D(this, lpFile, imageInfo, bBuildMipMaps));

    return NULL;
}


Texture* D3DSystem::CreateFrameBuffer(unsigned int width, unsigned int height, DWORD dwColorFormat, BOOL bGenMipMaps)
{
    D3DTexture *tex = CreateBare(D3DTexture);
    tex->d3d = this;
    tex->bHasMipMaps = bGenMipMaps;
    tex->texWidth = width;
    tex->texHeight = height;
    tex->dwTexType = D3DTEXTURE_FRAME_BUFFER;
    tex->dwFormat = dwColorFormat;

    tex->CreateBuffer();

    DynamicTextureList << tex;

    return tex;
}

CubeTexture* D3DSystem::CreateCubeFrameBuffer(unsigned int width, DWORD dwColorFormat, BOOL bGenMipMaps)
{
    D3DCubeTexture *tex = CreateBare(D3DCubeTexture);
    tex->d3d = this;
    tex->bHasMipMaps = bGenMipMaps;
    tex->texWidth = width;
    tex->dwTexType = D3DTEXTURE_FRAME_BUFFER;
    tex->dwFormat = dwColorFormat;

    tex->CreateBuffer();

    DynamicTextureList << tex;

    return tex;
}

ZStencilBuffer* D3DSystem::CreateZStencilBuffer(unsigned int width, unsigned int height)
{
    D3DZStencilBuffer *buf = CreateBare(D3DZStencilBuffer);
    buf->d3d = this;
    buf->bufWidth = width;
    buf->bufHeight = height;

    ZStencilBufferList << buf;

    buf->CreateBuffer();

    return buf;
}

OffscreenSurface* D3DSystem::CreateOffscreenSurface(unsigned int width, unsigned int height, DWORD dwColorFormat)
{
    DWORD dwInternalFormat = 0;
    IDirect3DSurface9 *surface = NULL;

    switch(dwColorFormat)
    {
        case GS_GRAYSCALE:
            dwInternalFormat = D3DFMT_L8;
            break;
        case GS_L16:
            dwInternalFormat = D3DFMT_L16;
            break;
        case GS_ALPHA:
            if(bA8Supported)
                dwInternalFormat = D3DFMT_A8;
            else
                dwInternalFormat = D3DFMT_A8L8;
            break;
        case GS_RGB:
            dwInternalFormat = D3DFMT_X8R8G8B8;
            break;
        case GS_RGBA:
            dwInternalFormat = D3DFMT_A8R8G8B8;
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

    if(!SUCCEEDED(d3dDevice->CreateOffscreenPlainSurface(width, height, (D3DFORMAT)dwInternalFormat, D3DPOOL_SYSTEMMEM, &surface, NULL)))
    {
        ErrOut(TEXT("Unable to create offscreen surface"));
        return NULL;
    }

    D3DOffscreenSurface *oss = CreateBare(D3DOffscreenSurface);
    oss->d3d = this;
    oss->dwWidth = width;
    oss->dwHeight = height;
    oss->dwFormat = dwColorFormat;
    oss->dwInternalFormat = dwInternalFormat;
    oss->surface = surface;

    return oss;
}


void  D3DSystem::EnableTexturing(BOOL bEnable)
{
    bTexturingEnabled = bEnable;
}

void  D3DSystem::EnableProjectiveTexturing(BOOL bEnable, int idTexture)
{
    d3dDevice->SetTextureStageState(idTexture, D3DTSS_TEXTURETRANSFORMFLAGS, (bEnable) ? D3DTTFF_PROJECTED : D3DTTFF_DISABLE);
}

BOOL  D3DSystem::TexturingEnabled()
{
    return bTexturingEnabled;
}

SamplerState* D3DSystem::CreateSamplerState(SamplerInfo &info)
{
    D3DSampler *sampler = CreateBare(D3DSampler);

    if(info.filter == 0x1111)
    {
        sampler->minFilter = minAnisoVal;
        sampler->magFilter = magAnisoVal;
        sampler->mipFilter = D3DTEXF_LINEAR;
    }
    else
    {
        sampler->minFilter = (info.filter & 0x001) ? D3DTEXF_LINEAR : D3DTEXF_POINT;
        sampler->magFilter = (info.filter & 0x010) ? D3DTEXF_LINEAR : D3DTEXF_POINT;
        sampler->mipFilter = (info.filter & 0x100) ? D3DTEXF_LINEAR : D3DTEXF_POINT;
    }

    sampler->bUseBorder = ((info.addressU == GS_ADDRESS_BORDER) ||
                           (info.addressV == GS_ADDRESS_BORDER) ||
                           (info.addressW == GS_ADDRESS_BORDER) );

    sampler->addressU    = D3DAddressModes[info.addressU];
    sampler->addressV    = D3DAddressModes[info.addressV];
    sampler->addressW    = D3DAddressModes[info.addressW];
    sampler->borderColor = info.borderColor.GetRGBA();
    sampler->maxAniso    = info.maxAnisotropy;

    return sampler;
}


Shader* D3DSystem::CreateVertexShader(CTSTR lpShader)
{
    return D3DShader::CreateShader(this, Shader_Vertex, lpShader);
}

Shader* D3DSystem::CreatePixelShader(CTSTR lpShader)
{
    return D3DShader::CreateShader(this, Shader_Pixel, lpShader);
}


VertexBuffer* D3DSystem::CreateVertexBuffer(VBData *vbData, BOOL bStatic)
{
    return (VertexBuffer*)InitializeObjectData(new D3DVertexBuffer(this, vbData, bStatic));
}

VertexBuffer* D3DSystem::CloneVertexBuffer(VertexBuffer *vb, BOOL bStatic)
{
    return (VertexBuffer*)InitializeObjectData(new D3DVertexBuffer(this, vb, bStatic));
}



IndexBuffer* D3DSystem::CreateIndexBuffer(GSIndexType IndexType, void *indices, DWORD dwNum, BOOL bStatic)
{
    return (IndexBuffer*)InitializeObjectData(new D3DIndexBuffer(this, IndexType, indices, dwNum, bStatic));
}

IndexBuffer* D3DSystem::CloneIndexBuffer(IndexBuffer *ib, BOOL bStatic)
{
    return (IndexBuffer*)InitializeObjectData(new D3DIndexBuffer(this, ib, bStatic));
}


void  D3DSystem::LoadTexture(BaseTexture *texture, int idTexture)
{
    //profileSingularSegment("D3DSystem::LoadTexture");

    if(curTextures[idTexture] == texture)
        return;

    if(texture)
    {
        if(!SUCCEEDED(d3dDevice->SetTexture(idTexture, GetD3DTex(texture))))
            ErrOut(TEXT("ick"));
    }
    else
        d3dDevice->SetTexture(idTexture, NULL);

    curTextures[idTexture] = texture;
}

void  D3DSystem::LoadSamplerState(SamplerState *sampler, int id)
{
    if(curSamplers[id] == sampler)
        return;

    if(sampler)
    {
        D3DSampler *samp = static_cast<D3DSampler*>(sampler);

        if(curMinFilter[id] != samp->minFilter)
        {
            d3dDevice->SetSamplerState(id, D3DSAMP_MINFILTER, samp->minFilter);
            curMinFilter[id] = samp->minFilter;
        }
        if(curMagFilter[id] != samp->magFilter)
        {
            d3dDevice->SetSamplerState(id, D3DSAMP_MAGFILTER, samp->magFilter);
            curMagFilter[id] = samp->magFilter;
        }
        if(curMipFilter[id] != samp->mipFilter)
        {
            d3dDevice->SetSamplerState(id, D3DSAMP_MIPFILTER, samp->mipFilter);
            curMipFilter[id] = samp->mipFilter;
        }

        if(curAddressU[id] != samp->addressU)
        {
            d3dDevice->SetSamplerState(id, D3DSAMP_ADDRESSU, samp->addressU);
            curAddressU[id] = samp->addressU;
        }
        if(curAddressV[id] != samp->addressV)
        {
            d3dDevice->SetSamplerState(id, D3DSAMP_ADDRESSV, samp->addressV);
            curAddressV[id] = samp->addressV;
        }
        if(curAddressW[id] != samp->addressW)
        {
            d3dDevice->SetSamplerState(id, D3DSAMP_ADDRESSW, samp->addressW);
            curAddressW[id] = samp->addressW;
        }

        if(samp->bUseBorder)
            d3dDevice->SetSamplerState(id, D3DSAMP_BORDERCOLOR, samp->borderColor);

        if(curMaxAniso[id] != samp->maxAniso)
        {
            d3dDevice->SetSamplerState(id, D3DSAMP_MAXANISOTROPY, samp->maxAniso);
            curMaxAniso[id] = samp->maxAniso;
        }
    }

    curSamplers[id] = sampler;
}

void  D3DSystem::LoadVertexBuffer(VertexBuffer *vb)
{
    D3DVertexBuffer *newVB = (D3DVertexBuffer*)vb;
    if(newVB == curVB)
        return;

    //profileSingularSegment("D3DSystem::LoadVertexBuffer");

    curVB = newVB;
    if(curVB)
    {
        if(!SUCCEEDED(d3dDevice->SetStreamSource(0, curVB->VertBuffer, 0, 12)))
            ErrOut(TEXT("argh1"));
        for(int i=0; i<8; i++)
        {
            if(i >= curVB->TCBuffer.Num() || !curVB->TCBuffer[i])
                d3dDevice->SetStreamSource(i+1, NULL, 0, 0);
            else
                d3dDevice->SetStreamSource(i+1, curVB->TCBuffer[i], 0, curVB->TCSizes[i]);
        }
        d3dDevice->SetStreamSource(9, curVB->NormalBuffer, 0, 12);
        d3dDevice->SetStreamSource(10, curVB->ColorBuffer, 0, 4);
        d3dDevice->SetStreamSource(11, curVB->TangentBuffer, 0, 12);

        d3dDevice->SetVertexDeclaration(curVB->declaration);
    }
    else
    {
        for(int i=0; i<12; i++)
            d3dDevice->SetStreamSource(i, NULL, 0, 0);

        //d3dDevice->SetVertexDeclaration(NULL);
    }
}

void  D3DSystem::LoadIndexBuffer(IndexBuffer *ib)
{
    D3DIndexBuffer *newIB = (D3DIndexBuffer*)ib;
    if(newIB == curIB)
        return;

    //profileSingularSegment("D3DSystem::LoadIndexBuffer");

    curIB = newIB;
    d3dDevice->SetIndices(curIB ? curIB->Buffer : NULL);

    if(curIB && !curIB->dwSize)
        CrashError(TEXT("...cannot use an index buffer with zero indices"));
}

void  D3DSystem::LoadVertexShader(Shader *vShader)
{
    //profileSingularSegment("D3DSystem::LoadVertexShader");

    if(!vShader)
    {
        if(curVertexShader)
        {
            curVertexShader->bUsingShader = FALSE;

            bUsingVertexShader = 0;
            d3dDevice->SetVertexShader(NULL);
            curVertexShader = NULL;
        }
    }
    else
    {
        bUsingVertexShader = 1;

        D3DShader *dxShader = (D3DShader*)vShader;

        if(curVertexShader == dxShader)
            return;

        if(curVertexShader) curVertexShader->bUsingShader = FALSE;

        d3dDevice->SetVertexShader(dxShader->vertexShader);
        curVertexShader = dxShader;
        curVertexShader->bUsingShader = TRUE;
    }
}

void  D3DSystem::LoadPixelShader(Shader *pShader)
{
    //profileSingularSegment("D3DSystem::LoadPixelShader");

    if(!pShader)
    {
        if(!curPixelShader)
            return;

        curPixelShader->bUsingShader = FALSE;

        d3dDevice->SetPixelShader(NULL);
        curPixelShader = NULL;

        for(int i=0; i<16; i++)
            LoadTexture(NULL, i);
    }
    else
    {
        D3DShader *dxShader = (D3DShader*)pShader;

        if(curPixelShader == dxShader)
            return;

        if(curPixelShader) curPixelShader->bUsingShader = FALSE;

        d3dDevice->SetPixelShader(dxShader->pixelShader);
        curPixelShader = dxShader;
        curPixelShader->bUsingShader = TRUE;
    }
}


void  D3DSystem::SetFrameBufferTarget(BaseTexture *texture, DWORD side)
{
    //profileSingularSegment("D3DSystem::SetFrameBufferTarget");

    if(texture)
    {
        IDirect3DSurface9 *NewSurface;

        if(texture->IsOf(GetClass(D3DTexture)))
        {
            D3DTexture *dxTex = (D3DTexture*)texture;
            assert(dxTex->GetTex());
            if(!dxTex->GetTex())
                return;

            if(dxTex->dwTexType != D3DTEXTURE_FRAME_BUFFER)
                return;

            if(bInScene)
                d3dDevice->EndScene();

            dxTex->GetTex()->GetSurfaceLevel(0, &NewSurface);
        }
        else if(texture->IsOf(GetClass(D3DCubeTexture)))
        {
            D3DCubeTexture *dxTex = (D3DCubeTexture*)texture;
            assert(dxTex->GetTex());
            if(!dxTex->GetTex())
                return;

            if(dxTex->dwTexType != D3DTEXTURE_FRAME_BUFFER)
                return;

            if(bInScene)
                d3dDevice->EndScene();

            if(!SUCCEEDED(dxTex->GetTex()->GetCubeMapSurface(D3DCubeSides[side], 0, &NewSurface)))
                ErrOut(TEXT("hrm, carone's being ebil again, couldn't get the cube map render target surface"));
        }
        else
            ErrOut(TEXT("Unsupported texture type input as a render target"));

        if(!SUCCEEDED(d3dDevice->SetRenderTarget(0, NewSurface)))
            ErrOut(TEXT("hrm.  couldn't set the render target.  that sucks"));

        if(CurrentColorSurface != MainColorSurface)
            CurrentColorSurface->Release();
        CurrentColorSurface = NewSurface;

        if(bInScene)
            d3dDevice->BeginScene();
        d3dDevice->SetViewport(&curD3DViewport);
    }
    else
    {
        assert(CurrentColorSurface != MainColorSurface);
        if(CurrentColorSurface == MainColorSurface)
            return;

        if(bInScene)
            d3dDevice->EndScene();

        d3dDevice->SetRenderTarget(0, MainColorSurface);
        CurrentColorSurface->Release();
        CurrentColorSurface = MainColorSurface;

        if(bInScene)
            d3dDevice->BeginScene();
        d3dDevice->SetViewport(&curD3DViewport);
    }
}

void  D3DSystem::SetZStencilBufferTarget(ZStencilBuffer *buffer)
{
    D3DZStencilBuffer *dxBuffer = (D3DZStencilBuffer*)buffer;
    if(buffer)
    {
        d3dDevice->SetDepthStencilSurface(dxBuffer->d3dSurface);
        CurrentDepthStencilSurface = dxBuffer->d3dSurface;
    }
    else
    {
        d3dDevice->SetDepthStencilSurface(MainDepthStencilSurface);
        CurrentDepthStencilSurface = MainDepthStencilSurface;
    }
}

void  D3DSystem::GetFrameBuffer(LPBYTE lpData)
{
    D3DLOCKED_RECT d3dRect;
    D3DSURFACE_DESC d3dSd;
    int i,j;

    IDirect3DSurface9 *RenderData;

    CurrentColorSurface->GetDesc(&d3dSd);

    d3dDevice->CreateOffscreenPlainSurface(d3dSd.Width, d3dSd.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &RenderData, NULL);
    d3dDevice->GetRenderTargetData(CurrentColorSurface, RenderData);

    if(SUCCEEDED(RenderData->LockRect(&d3dRect, NULL, D3DLOCK_READONLY)))
    {
        LPBYTE lpBits = (LPBYTE)d3dRect.pBits;
        DWORD width     = d3dSd.Width;
        DWORD height    = d3dSd.Height;

        for(i=0; i<height; i++)
        {
            DWORD curY      = (i*width*3);
            DWORD curD3DY   = (i*d3dRect.Pitch);

            for(j=0; j<width; j++)
            {
                DWORD curX      = curY+(j*3);
                DWORD curD3DX   = curD3DY+(j*4);

                lpData[curX+2] = lpBits[curD3DX];
                lpData[curX+1] = lpBits[curD3DX+1];
                lpData[curX]   = lpBits[curD3DX+2];
            }
        }

        RenderData->UnlockRect();
    }

    RenderData->Release();
}


BOOL  D3DSystem::BeginScene(BOOL bClear, DWORD dwClearColor)
{
    if(bInScene)
        return FALSE;

    if(bClear)
        d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, dwClearColor, 1.0, 0 );
    /*else
        d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0xFF000000, 1.0, 0 );*/

    d3dDevice->BeginScene();

    bInScene = TRUE;
    zero(curTextures, sizeof(BaseTexture*)*16);
    zero(curSamplers, sizeof(SamplerState*)*16);

    curVertexShader = NULL;
    curPixelShader = NULL;

    curVB = NULL;
    curIB = NULL;

    return TRUE;
}

void  D3DSystem::EndScene()
{
    BOOL bRenderingMain = (MainColorSurface == CurrentColorSurface);
    if(bUseTripleBuffering && bRenderingMain)
    {
        DWORD nextTB = curTextureBuffer+1;
        D3DLOCKED_RECT lockedRect;

        if(nextTB == 3)
            nextTB = 0;

        TB[nextTB]->LockRect(&lockedRect, NULL, D3DLOCK_READONLY);
        TB[nextTB]->UnlockRect();

        d3dDevice->SetRenderTarget(0, TB[curTextureBuffer]);

        EnableDepthTest(FALSE);
        EnableBlending(FALSE);

        d3dDevice->SetFVF(D3DFVF_XYZRHW);
        d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, TBTriangle, 16);
        d3dDevice->SetFVF(0);

        EnableBlending(TRUE);
        EnableDepthTest(TRUE);

        d3dDevice->SetRenderTarget(0, CurrentColorSurface);

        if(++curTextureBuffer == 3)
            curTextureBuffer = 0;
    }

    d3dDevice->EndScene();

    if(bRenderingMain)
        d3dDevice->Present(NULL, NULL, NULL, NULL);

    bInScene = FALSE;

    bRebuildRenderTargets = FALSE;
}

void D3DSystem::ResetViewMatrix()
{
    Matrix tran = MatrixStack[curMatrix];
    tran.Z = -tran.Z;
    tran.Transpose();

    Matrix4x4Convert(curViewMatrix, tran);
    d3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)curViewMatrix);

    Matrix4x4Multiply(curViewProjMatrix, curViewMatrix, curProjectionMatrix);

    if(curEffect)
    {
        if(curEffect->GetViewProj())
            curEffect->SetMatrix(curEffect->GetViewProj(), curViewProjMatrix);
    }
}

void  D3DSystem::Draw(GSDrawMode drawMode, int vertexOffset, DWORD StartVert, DWORD nVerts)
{
    assert(curVB);

    //profileSingularSegment("D3DSystem::Draw");

    if(!curVB)
        return;

    assert(D3DDrawTypes[drawMode]);

    if(curEffect)
        curEffect->UpdateParams();
    else if(bUsingVertexShader)
    {
        float matViewProj[16];
        Matrix4x4Transpose(matViewProj, curViewProjMatrix);

        d3dDevice->SetVertexShaderConstantF(0, matViewProj, 4);
    }
    else
        d3dDevice->SetVertexShader(NULL);

    //profileSingularIn("drawing primitive");
    if(!StartVert && !nVerts)
    {
        if(curIB)
        {
            if(!SUCCEEDED(d3dDevice->DrawIndexedPrimitive((D3DPRIMITIVETYPE)D3DDrawTypes[drawMode], vertexOffset, 0, curVB->dwSize, 0, GetD3DPrimitiveSize(drawMode, curIB->dwInternalSize))))
                ErrOut(TEXT("Could not Draw verticies"));
            //engine->nPolys += GetD3DPrimitiveSize(drawMode, curIB->dwInternalSize);
        }
        else
        {
            if(!SUCCEEDED(d3dDevice->DrawPrimitive((D3DPRIMITIVETYPE)D3DDrawTypes[drawMode], 0, GetD3DPrimitiveSize(drawMode, curVB->dwSize))))
                ErrOut(TEXT("Could not Draw verticies"));
            //engine->nPolys += GetD3DPrimitiveSize(drawMode, curVB->dwSize);
        }
    }
    else
    {
        if(curIB)
        {
            if(!SUCCEEDED(d3dDevice->DrawIndexedPrimitive((D3DPRIMITIVETYPE)D3DDrawTypes[drawMode], vertexOffset, 0, curVB->dwSize, StartVert, GetD3DPrimitiveSize(drawMode, nVerts))))
                ErrOut(TEXT("Could not Draw verticies"));
        }
        else
        {
            if(!SUCCEEDED(d3dDevice->DrawPrimitive((D3DPRIMITIVETYPE)D3DDrawTypes[drawMode], StartVert, GetD3DPrimitiveSize(drawMode, nVerts))))
                ErrOut(TEXT("Could not Draw verticies"));
        }
        //engine->nPolys += GetD3DPrimitiveSize(drawMode, nVerts);
    }
    //profileOut;

    //if(!bUsingVertexShader && !curEffect)
    //    d3dDevice->SetFVF(0);
}

void  D3DSystem::DrawBare(GSDrawMode drawMode, int vertexOffset, DWORD StartVert, DWORD nVerts)
{
    assert(curVB);

    if(!curVB)
        return;

    if(!bUsingVertexShader)
    {
        d3dDevice->SetVertexShader(BareVertexShader);

        Quat q(0.0f, 0.0f, 0.0f, 1.0f);
        d3dDevice->SetVertexShaderConstantF(4, q.ptr, 1);
    }

    //d3dDevice->SetVertexDeclaration(BareDeclaration);

    float matViewProj[16];
    Matrix4x4Transpose(matViewProj, curViewProjMatrix);
    d3dDevice->SetVertexShaderConstantF(0, matViewProj, 4);

    assert(D3DDrawTypes[drawMode]);

    if(!StartVert && !nVerts)
    {
        if(curIB)
        {
            d3dDevice->DrawIndexedPrimitive((D3DPRIMITIVETYPE)D3DDrawTypes[drawMode], vertexOffset, 0, curVB->dwSize, 0, GetD3DPrimitiveSize(drawMode, curIB->dwInternalSize));
            engine->nPolys += GetD3DPrimitiveSize(drawMode, curIB->dwInternalSize);
        }
        else
        {
            d3dDevice->DrawPrimitive((D3DPRIMITIVETYPE)D3DDrawTypes[drawMode], 0, GetD3DPrimitiveSize(drawMode, curVB->dwSize));
            engine->nPolys += GetD3DPrimitiveSize(drawMode, curVB->dwSize);
        }
    }
    else
    {
        if(curIB)
            d3dDevice->DrawIndexedPrimitive((D3DPRIMITIVETYPE)D3DDrawTypes[drawMode], vertexOffset, 0, curVB->dwSize, StartVert, GetD3DPrimitiveSize(drawMode, nVerts));
        else
            d3dDevice->DrawPrimitive((D3DPRIMITIVETYPE)D3DDrawTypes[drawMode], StartVert, GetD3DPrimitiveSize(drawMode, nVerts));
        engine->nPolys += GetD3DPrimitiveSize(drawMode, nVerts);
    }

    //if(!bUsingVertexShader)
    //    d3dDevice->SetVertexShader(NULL);
}

void  D3DSystem::ResetDevice()
{
    D3DPRESENT_PARAMETERS pp;
    DWORD i;

    memset(&pp, 0, sizeof(D3DPRESENT_PARAMETERS));

    pp.Windowed                 = 1;
    pp.SwapEffect               = D3DSWAPEFFECT_FLIP;
    pp.BackBufferFormat         = D3DFMT_A8R8G8B8;
    pp.BackBufferCount          = 1;
    pp.EnableAutoDepthStencil   = 1;
    pp.AutoDepthStencilFormat   = D3DFMT_D24S8;
    pp.hDeviceWindow            = (HWND)curWindow;
    if(AppConfig->GetInt(TEXT("Display"), TEXT("vsync"), 0))
        pp.PresentationInterval     = D3DPRESENT_INTERVAL_ONE;
    else
        pp.PresentationInterval     = D3DPRESENT_INTERVAL_IMMEDIATE;


    if( (MainDepthStencilSurface != CurrentDepthStencilSurface) ||
        (MainColorSurface != CurrentColorSurface)               )
        return;

    bRebuildRenderTargets = TRUE;

    D3DRelease(MainDepthStencilSurface);
    D3DRelease(MainColorSurface)

    D3DRelease(TB[2]);
    D3DRelease(TB[1]);
    D3DRelease(TB[0]);

    D3DRelease(spriteBuffer);

    for(i=0; i<VertexBufferList.Num(); i++)
    {
        D3DVertexBuffer *dxVB = VertexBufferList[i];

        dxVB->FreeBuffers();
    }

    for(i=0; i<IndexBufferList.Num(); i++)
    {
        D3DIndexBuffer *dxIB = IndexBufferList[i];

        dxIB->FreeBuffer();
    }

    for(i=0; i<ZStencilBufferList.Num(); i++)
    {
        D3DZStencilBuffer *buf = ZStencilBufferList[i];

        buf->FreeBuffer();
    }

    for(i=0; i<DynamicTextureList.Num(); i++)
    {
        BaseTexture *baseTex = DynamicTextureList[i];
        if(baseTex->IsOf(GetClass(D3DTexture)))
        {
            D3DTexture *tex = (D3DTexture*)baseTex;
            tex->FreeBuffer();
        }
        else if(baseTex->IsOf(GetClass(D3DCubeTexture)))
        {
            D3DCubeTexture *tex = (D3DCubeTexture*)baseTex;
            tex->FreeBuffer();
        }
    }

    DWORD dwErr = (DWORD)d3dDevice->Reset(&pp);

    switch(dwErr)
    {
        case D3DERR_INVALIDCALL:
            ErrOut(TEXT("Reset failed, Invalid Call"));
            break;
        case D3DERR_OUTOFVIDEOMEMORY:
            ErrOut(TEXT("Reset failed, Out of video memory"));
            break;
        case D3DERR_DEVICELOST:
            ErrOut(TEXT("Reset failed, Device Lost"));
            break;
        case E_OUTOFMEMORY:
            ErrOut(TEXT("Reset failed, Out of memory"));
    }

    d3dDevice->SetRenderState(D3DRS_ZENABLE, 1);
    d3dDevice->SetRenderState(D3DRS_LIGHTING, 0);
    d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
    d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    d3dDevice->SetRenderState(D3DRS_SPECULARENABLE, 1);
    d3dDevice->SetRenderState(D3DRS_LOCALVIEWER, 1);
    d3dDevice->SetRenderState(D3DRS_AMBIENT, 0xFFFFFFFF);

    d3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

    for(i=0; i<ZStencilBufferList.Num(); i++)
    {
        D3DZStencilBuffer *buf = ZStencilBufferList[i];

        buf->CreateBuffer();
    }

    for(i=0; i<DynamicTextureList.Num(); i++)
    {
        BaseTexture *baseTex = DynamicTextureList[i];

        if(baseTex->IsOf(GetClass(D3DTexture)))
        {
            D3DTexture *tex = (D3DTexture*)baseTex;

            tex->CreateBuffer();
            if(tex->dwTexType != D3DTEXTURE_FRAME_BUFFER)
                tex->SetImage(tex->textureData);
        }
        else
        {
            D3DCubeTexture *tex = (D3DCubeTexture*)baseTex;

            tex->CreateBuffer();
            if(tex->dwTexType != D3DTEXTURE_FRAME_BUFFER)
            {
                tex->SetImage(0, tex->textureData);
                tex->SetImage(1, tex->textureData);
                tex->SetImage(2, tex->textureData);
                tex->SetImage(3, tex->textureData);
                tex->SetImage(4, tex->textureData);
                tex->SetImage(5, tex->textureData);
                tex->RebuildMipMaps();
            }
        }
    }

    for(i=0; i<VertexBufferList.Num(); i++)
    {
        D3DVertexBuffer *dxVB = VertexBufferList[i];

        dxVB->SetupBuffers();
    }

    for(i=0; i<IndexBufferList.Num(); i++)
    {
        D3DIndexBuffer *dxIB = IndexBufferList[i];

        dxIB->CreateBuffer();
    }

    if(!SUCCEEDED(d3dDevice->CreateVertexBuffer(4*sizeof(SpriteVertex), D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, SPIRTEVERTEXFORMAT, D3DPOOL_DEFAULT, &spriteBuffer, NULL)))
        ErrOut(TEXT("Could not re-create the sprite verex buffer"));

    d3dDevice->CreateRenderTarget(2, 2, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &TB[0], NULL);
    d3dDevice->CreateRenderTarget(2, 2, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &TB[1], NULL);
    d3dDevice->CreateRenderTarget(2, 2, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &TB[2], NULL);

    d3dDevice->GetRenderTarget(0, &MainColorSurface);
    d3dDevice->GetDepthStencilSurface(&MainDepthStencilSurface);

    CurrentDepthStencilSurface = MainDepthStencilSurface;
    CurrentColorSurface = MainColorSurface;
}


void  D3DSystem::UpdateAccumulationBuffer()
{
}

void  D3DSystem::LoadAccumulationBuffer()
{
}

void  D3DSystem::ReverseCullMode(BOOL bReverse)
{
    bReverseCullMode = bReverse;
    SetCullMode(curSide);
}

void  D3DSystem::SetCullMode(GSCullMode side)
{
    if(bReverseCullMode)
    {
        if(side != GS_NEITHER)
            side = (GSCullMode)!side;
    }

    if(curSide == side) return;

    switch(side)
    {
        case GS_FRONT:
            d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
            break;
        case GS_BACK:
            d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
            break;
        case GS_NEITHER:
            d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    }

    curSide = side;
}

void  D3DSystem::EnableBlending(BOOL bEnable)
{
    d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, bEnable);
}

void  D3DSystem::BlendFunction(GSBlendType srcFactor, GSBlendType destFactor)
{
    d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBlendFuncs[srcFactor]);
    d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBlendFuncs[destFactor]);
}


void  D3DSystem::ClearDepthBuffer(BOOL bFullClear)
{
    if(bFullClear)
        d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0, 0);
    else
        d3dDevice->Clear(1, &curViewport, D3DCLEAR_ZBUFFER, 0, 1.0, 0);
}

void  D3DSystem::EnableDepthTest(BOOL bEnable)
{
    d3dDevice->SetRenderState(D3DRS_ZENABLE, bEnable);
}

void  D3DSystem::DepthWriteEnable(BOOL bEnable)
{
    d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, bEnable);
}

void  D3DSystem::DepthFunction(GSDepthTest function)
{
    curDepthFunction = function;
    d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DTest[function]);
}


void  D3DSystem::ClearColorBuffer(BOOL bFullClear, DWORD color)
{
    if(bFullClear)
        d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, color, 0.0, 0);
    else
        d3dDevice->Clear(1, &curViewport, D3DCLEAR_TARGET, color, 0.0, 0);
}

void  D3DSystem::ColorWriteEnable(BOOL bRedEnable, BOOL bGreenEnable, BOOL bBlueEnable, BOOL bAlphaEnable)
{
    DWORD dwWriteFlags=0;

    if(bRedEnable)   dwWriteFlags |= D3DCOLORWRITEENABLE_RED;
    if(bGreenEnable) dwWriteFlags |= D3DCOLORWRITEENABLE_GREEN;
    if(bBlueEnable)  dwWriteFlags |= D3DCOLORWRITEENABLE_BLUE;
    if(bAlphaEnable) dwWriteFlags |= D3DCOLORWRITEENABLE_ALPHA;
    d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwWriteFlags);
}

void  D3DSystem::ClearStencilBuffer(BOOL bFill)
{
    d3dDevice->Clear(1, &curViewport, D3DCLEAR_STENCIL, 0, 0.0, bFill);
}

void  D3DSystem::EnableStencilTest(BOOL bEnable)
{
    d3dDevice->SetRenderState(D3DRS_STENCILENABLE, bEnable);
}

void  D3DSystem::EnableTwoSidedStencil(BOOL bEnable)
{
    d3dDevice->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, bEnable);
}

void  D3DSystem::StencilWriteEnable(BOOL bEnable)
{
    d3dDevice->SetRenderState(D3DRS_STENCILWRITEMASK, (bEnable) ? 0xFFFFFFFF : 0);
}

void  D3DSystem::StencilFunction(GSDepthTest function)
{
    d3dDevice->SetRenderState(D3DRS_STENCILFUNC, D3DTest[function]);
}

void  D3DSystem::StencilFunctionCCW(GSDepthTest function)
{
    d3dDevice->SetRenderState(D3DRS_CCW_STENCILFUNC, D3DTest[function]);
}

void  D3DSystem::StencilOp(GSStencilOp fail, GSStencilOp zfail, GSStencilOp zpass)
{
    d3dDevice->SetRenderState(D3DRS_STENCILFAIL, D3DStencilOps[fail]);
    d3dDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DStencilOps[zfail]);
    d3dDevice->SetRenderState(D3DRS_STENCILPASS, D3DStencilOps[zpass]);
}

void  D3DSystem::StencilOpCCW(GSStencilOp fail, GSStencilOp zfail, GSStencilOp zpass)
{
    d3dDevice->SetRenderState(D3DRS_CCW_STENCILFAIL, D3DStencilOps[fail]);
    d3dDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3DStencilOps[zfail]);
    d3dDevice->SetRenderState(D3DRS_CCW_STENCILPASS, D3DStencilOps[zpass]);
}

void  D3DSystem::GetStencilOp(GSStencilOp *pFail, GSStencilOp *pZfail, GSStencilOp *pZpass)
{
    DWORD failD3D, zfailD3D, zpassD3D;
    int i;

    if(pFail)
    {
        d3dDevice->GetRenderState(D3DRS_STENCILFAIL, &failD3D);
        for(i=0; i<6; i++)
        {
            if(D3DStencilOps[i] == failD3D)
            {
                *pFail = (GSStencilOp)i;
                break;
            }
        }
    }

    if(pZfail)
    {
        d3dDevice->GetRenderState(D3DRS_STENCILZFAIL, &zfailD3D);
        for(i=0; i<6; i++)
        {
            if(D3DStencilOps[i] == zfailD3D)
            {
                *pZfail = (GSStencilOp)i;
                break;
            }
        }
    }

    if(pZpass)
    {
        d3dDevice->GetRenderState(D3DRS_STENCILPASS, &zpassD3D);
        for(i=0; i<6; i++)
        {
            if(D3DStencilOps[i] == zpassD3D)
            {
                *pZpass = (GSStencilOp)i;
                break;
            }
        }
    }
}


void  D3DSystem::EnableClipPlane(BOOL bEnable)
{
    d3dDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, bEnable ? 1 : 0);
}

void  D3DSystem::SetClipPlane(const Plane &plane)
{
    Vect4 newPlane(plane.Dir.x, plane.Dir.y, plane.Dir.z, plane.Dist);

    float modelViewInv[16];
    Matrix4x4Inverse(modelViewInv, curViewMatrix);
    Matrix4x4TransformVect(newPlane, modelViewInv, newPlane);

    d3dDevice->SetClipPlane(0, newPlane.ptr);
}


void  D3DSystem::SetPointSize(float size)
{
    d3dDevice->SetRenderState(D3DRS_POINTSIZE, *(DWORD*)&size);
}


void  D3DSystem::Ortho(double left, double right, double top, double bottom, double znear, double zfar)
{
    D3DXMatrixOrthoOffCenterLH((D3DXMATRIX*)curProjectionMatrix, left, right, top, bottom, znear, zfar);
    pLeft = left;
    pRight = right;
    pTop = top;
    pBottom = bottom;
    pNear = znear;
    pFar = zfar;
    pIsFrustum = 0;

    d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)curProjectionMatrix);

    Matrix4x4Multiply(curViewProjMatrix, curViewMatrix, curProjectionMatrix);
}

void  D3DSystem::Frustum(double left, double right, double top, double bottom, double znear, double zfar)
{
    D3DXMatrixPerspectiveOffCenterLH((D3DXMATRIX*)curProjectionMatrix, left, right, top, bottom, znear, zfar);
    pLeft = left;
    pRight = right;
    pTop = top;
    pBottom = bottom;
    pNear = znear;
    pFar = zfar;
    pIsFrustum = 1;

    d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)curProjectionMatrix);

    Matrix4x4Multiply(curViewProjMatrix, curViewMatrix, curProjectionMatrix);
}


void  D3DSystem::DrawSpriteEx(Texture *texture, DWORD color, float x, float y, float x2, float y2, float u, float v, float u2, float v2)
{
    assert(texture);

    EnableDepthTest(0);
    SetCullMode(GS_NEITHER);

    if(u == -1.0f)  u = 0.0f;
    if(v == -1.0f)  v = 0.0f;
    if(u2 == -1.0f) u2 = 1.0f;
    if(v2 == -1.0f) v2 = 1.0f;

    Matrix &worldMat = MatrixStack[curMatrix];

    x -= worldMat.T.x;
    y -= worldMat.T.y;

    if(x2 == -1.0f) x2 = x+texture->Width();
    if(y2 == -1.0f) y2 = y+texture->Height();

    x2 -= worldMat.T.x;
    y2 -= worldMat.T.y;

    x   -= 0.5f;
    y   -= 0.5f;
    x2  -= 0.5f;
    y2  -= 0.5f;

    d3dDevice->SetTexture(0, GetD3DTex(texture));
    LoadDefault2DSampler();

    SpriteVertex verts[4] =
    {
        {x,  y,  0.0f, 1.0f, color, u,  v},
        {x,  y2, 0.0f, 1.0f, color, u,  v2},
        {x2, y,  0.0f, 1.0f, color, u2, v},
        {x2, y2, 0.0f, 1.0f, color, u2, v2}
    };

    if(spriteBuffer)
    {
        LPBYTE lpVertPointer = NULL;
        spriteBuffer->Lock(0, sizeof(verts), (void**)&lpVertPointer, D3DLOCK_DISCARD);
        mcpy(lpVertPointer, verts, sizeof(verts));
        spriteBuffer->Unlock();

        d3dDevice->SetStreamSource(0, spriteBuffer, 0, sizeof(SpriteVertex));
        d3dDevice->SetFVF(SPIRTEVERTEXFORMAT);

        d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

        d3dDevice->SetFVF(0);

        VertexBuffer *lastVB = curVB;
        curVB = NULL;
        LoadVertexBuffer(lastVB);

        engine->nPolys += 2;
    }

    if(curTextures[0])
        d3dDevice->SetTexture(0, GetD3DTex(curTextures[0]));
    else
        d3dDevice->SetTexture(0, NULL);

    SetCullMode(GS_BACK);
    EnableDepthTest(1);
}

    
void  D3DSystem::DrawCubeBackdrop(CubeTexture *cubetexture, const Quat &qRot, float fLeft, float fRight, float fTop, float fBottom, float fNear)
{
    assert(cubetexture);

    EnableDepthTest(0);
    SetCullMode(GS_NEITHER);

    float x, y, x2, y2;
    x = y = 0;
    x2 = GetSizeXF();
    y2 = GetSizeYF();

    x   -= 0.5f;
    y   -= 0.5f;
    x2  -= 0.5f;
    y2  -= 0.5f;

    Matrix rot(qRot.GetInv() * AxisAngle(0.0f, 0.0f, 1.0f, RAD(180.0f)).GetQuat());

    Vect upperLeft(-fLeft, fTop, fNear);
    Vect upperRight(-fRight, fTop, fNear);
    Vect lowerLeft(-fLeft, fBottom, fNear);
    Vect lowerRight(-fRight, fBottom, fNear);

    upperLeft.TransformVector(rot);
    lowerLeft.TransformVector(rot);
    upperRight.TransformVector(rot);
    lowerRight.TransformVector(rot);

    d3dDevice->SetTexture(0, GetD3DTex(cubetexture));
    LoadDefault3DSampler();

    CubeProjVertex verts[4] =
    {
        {x,  y,  0.0f, 1.0f, upperLeft.x, upperLeft.y, upperLeft.z},
        {x,  y2, 0.0f, 1.0f, lowerLeft.x, lowerLeft.y, lowerLeft.z},
        {x2, y,  0.0f, 1.0f, upperRight.x, upperRight.y, upperRight.z},
        {x2, y2, 0.0f, 1.0f, lowerRight.x, lowerRight.y, lowerRight.z}
    };

    if(spriteBuffer)
    {
        LPBYTE lpVertPointer = NULL;
        spriteBuffer->Lock(0, sizeof(verts), (void**)&lpVertPointer, 0);
        mcpy(lpVertPointer, verts, sizeof(verts));
        spriteBuffer->Unlock();

        d3dDevice->SetStreamSource(0, spriteBuffer, 0, sizeof(CubeProjVertex));
        d3dDevice->SetFVF(CUBEPROJVERTEXFORMAT);

        d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

        d3dDevice->SetFVF(0);

        VertexBuffer *lastVB = curVB;
        curVB = NULL;
        LoadVertexBuffer(lastVB);

        engine->nPolys += 2;
    }

    if(curTextures[0])
        d3dDevice->SetTexture(0, GetD3DTex(curTextures[0]));
    else
        d3dDevice->SetTexture(0, NULL);

    SetCullMode(GS_BACK);
    EnableDepthTest(1);
}

/*void  D3DSystem::DrawCubeBackdrop(Camera *cam, CubeTexture *cubetexture, const Quat &customRot)
{
    assert(cam && cubetexture);

    if(!cam || (!cam->IsPerspective()) return;

    EnableDepthTest(0);
    SetCullMode(GS_NEITHER);

    float x, y, x2, y2;
    x = y = 0;
    x2 = GetSizeXF();
    y2 = GetSizeYF();

    x   -= 0.5f;
    y   -= 0.5f;
    x2  -= 0.5f;
    y2  -= 0.5f;

    Matrix rot(customRot.GetInv() * AxisAngle(0.0f, 0.0f, 1.0f, RAD(180.0f)).GetQuat());

    Vect upperLeft(-cam->Left(), cam->Top(), cam->Near());
    Vect upperRight(-cam->Right(), cam->Top(), cam->Near());
    Vect lowerLeft(-cam->Left(), cam->Bottom(), cam->Near());
    Vect lowerRight(-cam->Right(), cam->Bottom(), cam->Near());

    upperLeft.TransformVector(rot);
    lowerLeft.TransformVector(rot);
    upperRight.TransformVector(rot);
    lowerRight.TransformVector(rot);

    d3dDevice->SetTexture(0, GetD3DTex(cubetexture));
    LoadDefault3DSampler();

    CubeProjVertex verts[4] =
    {
        {x,  y,  0.0f, 1.0f, upperLeft.x, upperLeft.y, upperLeft.z},
        {x,  y2, 0.0f, 1.0f, lowerLeft.x, lowerLeft.y, lowerLeft.z},
        {x2, y,  0.0f, 1.0f, upperRight.x, upperRight.y, upperRight.z},
        {x2, y2, 0.0f, 1.0f, lowerRight.x, lowerRight.y, lowerRight.z}
    };

    if(spriteBuffer)
    {
        LPBYTE lpVertPointer = NULL;
        spriteBuffer->Lock(0, sizeof(verts), (void**)&lpVertPointer, 0);
        mcpy(lpVertPointer, verts, sizeof(verts));
        spriteBuffer->Unlock();

        d3dDevice->SetStreamSource(0, spriteBuffer, 0, sizeof(CubeProjVertex));
        d3dDevice->SetFVF(CUBEPROJVERTEXFORMAT);

        d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

        d3dDevice->SetFVF(0);

        VertexBuffer *lastVB = curVB;
        curVB = NULL;
        LoadVertexBuffer(lastVB);

        engine->nPolys += 2;
    }

    if(curTextures[0])
        d3dDevice->SetTexture(0, GetD3DTex(curTextures[0]));
    else
        d3dDevice->SetTexture(0, NULL);

    SetCullMode(GS_BACK);
    EnableDepthTest(1);
}*/


void  D3DSystem::AdjustZ(double zfar)
{
    D3DXMATRIX mtx;

    if(pIsFrustum)
        D3DXMatrixPerspectiveOffCenterLH(&mtx, pLeft, pRight, pTop, pBottom, pNear, zfar);
    else
        D3DXMatrixOrthoOffCenterLH(&mtx, pLeft, pRight, pTop, pBottom, pNear, zfar);

    d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&mtx);
}

void  D3DSystem::SetViewport(int x, int y, int width, int height)
{
    curD3DViewport.X = x;
    curD3DViewport.Y = y;
    curD3DViewport.Width = width;
    curD3DViewport.Height = height;
    curD3DViewport.MinZ = 0.0f;
    curD3DViewport.MaxZ = 1.0f;

    curViewport.x1 = x;
    curViewport.y1 = y;
    curViewport.x2 = x+width;
    curViewport.y2 = y+height;

    d3dDevice->SetViewport(&curD3DViewport);
}

void  D3DSystem::GetViewport(XRect &rect)
{
    mcpy(&rect, &curD3DViewport, sizeof(XRect));
}


void  D3DSystem::SetScissorRect(XRect *pRect)
{
    if(!pRect)
    {
        RECT rc = {0, 0, curRect.cx, curRect.cy};
        d3dDevice->SetScissorRect((const RECT*)&rc);
    }
    else
        d3dDevice->SetScissorRect((const RECT*)pRect);
}

void D3DSystem::GetLocalMousePos(int &x, int &y)
{
    traceIn(D3DSystem::GetLocalMousePos);

    POINT point;

    GetCursorPos(&point);
    ScreenToClient((HWND)curWindow, &point);
    x = point.x;
    y = point.y;

    traceOut;
}

void D3DSystem::SetLocalMousePos(int x, int y)
{
    traceIn(D3DSystem::SetLocalMousePos);

    POINT point;
    point.x = x;
    point.y = y;
    ClientToScreen((HWND)curWindow, &point);
    SetCursorPos(point.x, point.y);

    traceOut;
}

void D3DSystem::EnableTripleBuffering(BOOL bEnable)
{
    bUseTripleBuffering = bEnable;
}


inline DWORD ENGINEAPI GetD3DPrimitiveSize(DWORD DrawType, DWORD nVerts)
{
    switch(DrawType)
    {
        case GS_POINTS:
            return nVerts;
        case GS_LINES:
            return nVerts/2;
        case GS_LINESTRIP:
            return nVerts-1;
        case GS_TRIANGLES:
            return nVerts/3;
        case GS_TRIANGLESTRIP:
        case GS_TRIANGLEFAN:
            return nVerts-2;
            return nVerts-2;
    }
    ErrOut(TEXT("Unsupported DrawType used"));
    return 0;
}



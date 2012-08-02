/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  D3DSystem.h

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


#ifndef D3DDRIVER_HEADER
#define D3DDRIVER_HEADER

//#undef GetSize

//#define D3D_DEBUG_INFO
#define TokenType TokenTypeDontUseMeOrElse
#define FUNC_STATIC FUNC_STATIC_ARGH_PLEASE_GO_AWAY

#include <d3dx9.h>
#include <d3d9.h>

#undef TokenType
#undef FUNC_STATIC

#include <Base.h>
#include <ScriptDefs.h>
#include <ScriptCompiler.h>

//#define GetSize                 MainAllocator->_GetSize

#define D3DRelease(obj) \
    if(obj) \
    { \
        obj->Release(); \
        obj = NULL; \
    }

class D3DSystem;


class D3DSampler : public SamplerState
{
    DeclareClass(D3DSampler, SamplerState);

public:
    BOOL bUseBorder;
    DWORD minFilter, magFilter, mipFilter, addressU, addressV, addressW;
    UINT maxAniso;

    DWORD borderColor;
};


class D3DVertexBuffer : public VertexBuffer
{
    DeclareClass(D3DVertexBuffer, VertexBuffer);

public:
    ~D3DVertexBuffer();
    void  SetVertexShader(Shader *vShader);
    void  CopyList(VertexBuffer *buffer);
    void  FlushBuffers(BOOL bRebuild);
    VBData* GetData();

    D3DVertexBuffer() {};
    D3DVertexBuffer(D3DSystem *curSystem, VBData *vbd, BOOL bIsStaticr);
    D3DVertexBuffer(D3DSystem *curSystem, VertexBuffer *buffer, BOOL bIsStatic);
    void  Clear();
    void  FreeBuffers();
    void  SetupBuffers();

    IDirect3DVertexBuffer9 *VertBuffer;
    IDirect3DVertexBuffer9 *NormalBuffer;
    IDirect3DVertexBuffer9 *ColorBuffer;
    IDirect3DVertexBuffer9 *TangentBuffer;
    List<IDirect3DVertexBuffer9*> TCBuffer;
    List<UINT> TCSizes;

    IDirect3DVertexDeclaration9 *declaration;

    BOOL    bStatic;
    DWORD   dwSize;

    DWORD   FVF;
    VBData  *vbData;

    D3DSystem *d3d;
};

/////////////////////////////////////
//Index Buffer class
class D3DIndexBuffer : public IndexBuffer
{
    DeclareClass(D3DIndexBuffer, IndexBuffer);

public:
    ~D3DIndexBuffer();
    void  CopyList(IndexBuffer *buffer);
    void  FlushBuffer();
    void* GetData();

    GSIndexType GetIndexType();
    UINT NumIndices();

    D3DIndexBuffer()  {};
    D3DIndexBuffer(D3DSystem *curSystem, int IndexType, void *indicesIn, DWORD dwNum, BOOL bIsStatic);
    D3DIndexBuffer(D3DSystem *curSystem, IndexBuffer *buffer, BOOL bIsStatic);
    void  CreateBuffer();
    void  FreeBuffer();
    void  Clear();

    IDirect3DIndexBuffer9 *Buffer;

    BOOL   bStatic;
    BOOL   bJustCreated;
    DWORD  dwInternalSize, dwSize;
    DWORD  Pitch;
    int    Type;
    void   *indices;

    D3DSystem *d3d;
};



inline IDirect3DBaseTexture9* GetD3DTex(BaseTexture *tex) {return reinterpret_cast<IDirect3DBaseTexture9*>(tex->GetInternalData());}


/////////////////////////////////////
//texture class

enum {D3DTEXTURE_STANDARD_BUFFER=0, D3DTEXTURE_FRAME_BUFFER};

class D3DTexture : public Texture
{
    DeclareClass(D3DTexture, Texture);

public:
    ~D3DTexture();

    //void SetLOD(int lod);
    DWORD Width();
    DWORD Height();
    BOOL  HasAlpha();

    DWORD GetSize();

    void  SetImage(void *lpData);
    void* GetImage(BOOL bForce, void *lpInputPtr);

    BOOL GetRenderTargetImage(OffscreenSurface *destination);

    DWORD GetFormat() {return dwFormat;}

    D3DTexture()   {}
    D3DTexture(D3DSystem *curSystem, CTSTR lpFile, D3DXIMAGE_INFO &imageInfo, BOOL bBuildMipMaps);
    D3DTexture(D3DSystem *curSystem, unsigned int width, unsigned int height, DWORD dwFormatIn, BOOL bBuildMipMaps, BOOL bStatic);

    void  CreateBuffer();
    void  FreeBuffer();

    void  ProcessMipNormalMaps(D3DLOCKED_RECT d3dRect, DWORD dwCurrentLevel);

    int mipLevel;
    void  SetLOD(int level);

    void  RebuildMipMaps();

    inline IDirect3DTexture9*& GetTex() {return *reinterpret_cast<IDirect3DTexture9**>(&lpTexData);}

    DWORD dwTexType, texWidth, texHeight, dwFormat, dwInternalFormat;
    BOOL bNeedsBlending, bHasMipMaps;

    BOOL bDynamic;
    LPBYTE textureData;

    D3DSystem *d3d;
};

class D3DOffscreenSurface : public OffscreenSurface
{
    DeclareClass(D3DOffscreenSurface, OffscreenSurface);

public:
    D3DOffscreenSurface() {}
    ~D3DOffscreenSurface();

    LPVOID Lock(BOOL bReadOnly);
    void Unlock();

    DWORD Width() {return dwWidth;}
    DWORD Height() {return dwHeight;}

    DWORD dwWidth, dwHeight, dwFormat, dwInternalFormat;
    IDirect3DSurface9 *surface;
    BOOL bLocked;

    D3DSystem *d3d;
};


/////////////////////////////////////
//cube texture class

const D3DCUBEMAP_FACES D3DCubeSides[] = {D3DCUBEMAP_FACE_POSITIVE_X, D3DCUBEMAP_FACE_NEGATIVE_X, D3DCUBEMAP_FACE_POSITIVE_Y, D3DCUBEMAP_FACE_NEGATIVE_Y, D3DCUBEMAP_FACE_NEGATIVE_Z, D3DCUBEMAP_FACE_POSITIVE_Z};

class D3DCubeTexture : public CubeTexture
{
    DeclareClass(D3DCubeTexture, CubeTexture);

public:
    ~D3DCubeTexture();
    void SetImage(int side, void *lpData);
    void* GetImage(int side);

    D3DCubeTexture() {}
    D3DCubeTexture(D3DSystem *curSystem, CTSTR lpFile, D3DXIMAGE_INFO &imageInfo, BOOL bBuildMipMaps);
    D3DCubeTexture(D3DSystem *curSystem, unsigned int width, DWORD dwFormatIn, BOOL bBuildMipMaps, BOOL bStatic);

    inline IDirect3DCubeTexture9*& GetTex() {return *reinterpret_cast<IDirect3DCubeTexture9**>(&lpTexData);}

    DWORD texWidth, dwFormat, dwInternalFormat;
    BOOL bNeedsBlending, bHasMipMaps;

    void  CreateBuffer();
    void  FreeBuffer();

    void  RebuildMipMaps();

    int mipLevel;
    void  SetLOD(int level);

    DWORD Width();

    LPBYTE textureData[6];

    BOOL bDynamic;

    DWORD dwTexType;

    D3DSystem *d3d;
};


/////////////////////////////////////
//texture3D class
class D3DTexture3D : public Texture3D
{
    DeclareClass(D3DTexture3D, Texture3D);

public:
    ~D3DTexture3D();

    D3DTexture3D() : bNeedsBlending(0) {}
    D3DTexture3D(D3DSystem *curSystem, CTSTR lpFile, D3DXIMAGE_INFO &imageInfo, BOOL bBuildMipMaps);
    D3DTexture3D(D3DSystem *curSystem, unsigned int width, unsigned int height, unsigned int depth, DWORD nColors, void *lpData, BOOL bBuildMipMaps);

    inline IDirect3DVolumeTexture9*& GetTex() {return *reinterpret_cast<IDirect3DVolumeTexture9**>(&lpTexData);}

    DWORD texWidth, texHeight, texDepth;
    BOOL bNeedsBlending, bHasMipMaps;

    int mipLevel;
    void  SetLOD(int level);

    D3DSystem *d3d;
};


/////////////////////////////////////
//z-stencil buffer class
class D3DZStencilBuffer : public ZStencilBuffer
{
    DeclareClass(D3DZStencilBuffer, ZStencilBuffer);

public:
    ~D3DZStencilBuffer();

    D3DZStencilBuffer()    {}

    void  CreateBuffer();
    void  FreeBuffer();

    DWORD Width();
    DWORD Height();

    DWORD bufWidth, bufHeight;
    IDirect3DSurface9 *d3dSurface;

    D3DSystem *d3d;
};


/////////////////////////////////////
//Shader Processor (converts engine shader text to d3d HLSL shader text)

struct ShaderParam
{
    ShaderParameterType type;
    String name;

    UINT samplerID;
    UINT textureID;

    D3DXHANDLE hD3DObj;

    inline ~ShaderParam() {FreeData();}

    inline void FreeData()
    {
        name.Clear();
    }
};

struct ShaderSampler
{
    String name;
    SamplerState *sampler;

    inline ~ShaderSampler() {FreeData();}

    inline void FreeData()
    {
        name.Clear();
        DestroyObject(sampler);
    }
};

struct ShaderTex
{
    String name;
    String type;

    UINT paramID;

    inline void FreeData() {name.Clear(); type.Clear();}
};

struct ShaderProcessor : CodeTokenizer
{
    BOOL ProcessShader(CTSTR input, String &output, String &errorString);

    BOOL AddState(SamplerInfo &info, String &stateName, String &stateVal);

    List<ShaderTex> Textures;
    List<ShaderSampler> Samplers;

    List<ShaderParam> Params;

    inline ~ShaderProcessor() {FreeData();}

    inline void FreeData()
    {
        int i;
        for(i=0; i<Textures.Num(); i++)
            Textures[i].FreeData();
        Textures.Clear();
        for(i=0; i<Samplers.Num(); i++)
            Samplers[i].FreeData();
        Samplers.Clear();
        for(i=0; i<Params.Num(); i++)
            Params[i].FreeData();
        Params.Clear();
    }

    inline ShaderTex* FindTexture(CTSTR lpTex)
    {
        for(int i=0; i<Textures.Num(); i++)
        {
            if(Textures[i].name.Compare(lpTex))
                return Textures+i;
        }

        return NULL;
    }

    inline UINT GetSamplerID(CTSTR lpSampler)
    {
        for(int i=0; i<Samplers.Num(); i++)
        {
            if(Samplers[i].name.Compare(lpSampler))
                return i;
        }

        return INVALID;
    }
};


/////////////////////////////////////
//D3D Shader Class

class D3DShader : public Shader
{
    DeclareClass(D3DShader, Shader);

public:
    static D3DShader *CreateShader(D3DSystem *curSystem, ShaderType type, CTSTR lpShader);
    ~D3DShader();

    ShaderType GetType() {return shaderType;}

    virtual int    NumParams() {return Params.Num();}
    virtual HANDLE GetParameter(int parameter);
    virtual HANDLE GetParameterByName(CTSTR lpName);
    virtual void   GetParameterInfo(HANDLE hObject, ShaderParameterInfo &paramInfo);

    virtual void   SetBool(HANDLE hObject, BOOL bValue);
    virtual void   SetFloat(HANDLE hObject, float fValue);
    virtual void   SetInt(HANDLE hObject, int iValue);
    virtual void   SetMatrix(HANDLE hObject, float *matrix);
    virtual void   SetVector(HANDLE hObject, const Vect &value);
    virtual void   SetVector2(HANDLE hObject, const Vect2 &value);
    virtual void   SetVector4(HANDLE hObject, const Vect4 &value);
    virtual void   SetTexture(HANDLE hObject, BaseTexture *texture);
    virtual void   SetValue(HANDLE hObject, void *val, DWORD dwSize);

    ShaderType shaderType;

    BOOL bUsingShader;

    List<ShaderParam>   Params;
    List<ShaderSampler> Samplers;

    union
    {
        IDirect3DVertexShader9 *vertexShader;
        IDirect3DPixelShader9 *pixelShader;
    };

    ID3DXConstantTable *hlslConstantTable;

    D3DSystem *d3d;
};


/////////////////////////////////////
//hardware lighting class
class D3DHardwareLight : public HardwareLight
{
    DeclareClass(D3DHardwareLight, HardwareLight);

public:
    ~D3DHardwareLight();
    void  Enable(BOOL bEnable);
    void  SetIntensity(float inten);
    void  SetAmbientColor(DWORD dwRGB);
    void  SetDiffuseColor(DWORD dwRGB);
    void  SetSpecularColor(DWORD dwRGB);
    void  SetAttenuation(DWORD dwAttenuationTypeIn, float attenuationIn);
    void  Retransform();
    void  SetCutoff(float degrees);
    void  SetType(DWORD dwNewType);

    D3DHardwareLight() {}
    D3DHardwareLight(D3DSystem *curSystem, DWORD dwNewType);
    void  ResetLightData();

    Vect  pos;
    int   cutoff;
    DWORD dwType;
    Color4 AmbientRGB;
    Color4 DiffuseRGB;
    Color4 SpecularRGB;
    DWORD dwAttenuationType;
    float attenuation;
    float intensity;
    WORD  LightNum;
    BOOL  isOn;

    D3DSystem *d3d;
};



/////////////////////////////////////

class D3DSystem : public GraphicsSystem
{
    friend class D3DHardwareLight;

    DeclareClass(D3DSystem, GraphicsSystem);

public:
    LPVOID GetInternalData() {return d3dDevice;}

    D3DSystem();

    BOOL InitializeDevice(HANDLE hWindow);
    ~D3DSystem();

    TSTR GetDeviceName();

    DWORD VertexShaderVersion();
    DWORD PixelShaderVersion();
    BOOL  SupportsTwoSidedStencil();

    //Hardware Lighting Functions
    HardwareLight* CreateHardwareLight(DWORD dwType);
    void  EnableHardwareLighting(BOOL bEnable);

    //Software Lighting Functions
    //virtual Light LightCreate();

    //Fog Functions
    void  SetFogType(int mode);
    void  SetFogColor(DWORD dwRGB);
    void  SetFogDensity(float density);
    void  SetFogStart(float start);
    void  SetFogEnd(float end);

    //Material Functions
    void  SetMaterialColor(const Color4 &rgba);
    void  SetMaterialSpecular(const Color4 &rgb);
    void  SetMaterialIllumination(const Color4 &rgb);
    void  SetMaterialShininess(float shininess);

    //Texture Functions
    Texture* CreateTexture(unsigned int width, unsigned int height, DWORD nColors, void *lpData, BOOL bBuildMipMaps, BOOL bStatic);
    CubeTexture* CreateCubeTexture(unsigned int width, DWORD nColors, BOOL bBuildMipMaps, BOOL bStatic);
    Texture3D* Create3DTexture(unsigned int width, unsigned int height, unsigned int depth, DWORD nColors, void *lpData, BOOL bBuildMipMaps);

    Texture* CreateTextureFromFile(CTSTR lpFile, BOOL bBuildMipMaps);
    CubeTexture* CreateCubeTextureFromFile(CTSTR lpFile, BOOL bBuildMipMaps);
    Texture3D* Create3DTextureFromFile(CTSTR lpFile, BOOL bBuildMipMaps);

    Texture* CreateFrameBuffer(unsigned int width, unsigned int height, DWORD dwColorFormat, BOOL bGenMipMaps);
    CubeTexture* CreateCubeFrameBuffer(unsigned int width, DWORD dwColorFormat, BOOL bGenMipMaps);
    ZStencilBuffer* CreateZStencilBuffer(unsigned int width, unsigned int height);
    OffscreenSurface* CreateOffscreenSurface(unsigned int width, unsigned int height, DWORD dwColorFormat);

    void  EnableTexturing(BOOL bEnable);
    void  EnableProjectiveTexturing(BOOL bEnable, int idTexture);
    BOOL  TexturingEnabled();

    SamplerState* CreateSamplerState(SamplerInfo &info);

    //Shader Functions
    Shader* CreateVertexShader(CTSTR lpShader);
    Shader* CreatePixelShader(CTSTR lpShader);

    //Vertex Buffer Functions
    VertexBuffer* CreateVertexBuffer(VBData *vbData, BOOL bStatic);
    VertexBuffer* CloneVertexBuffer(VertexBuffer *vb, BOOL bStatic);

    //Index Buffer Functions
    IndexBuffer* CreateIndexBuffer(GSIndexType IndexType, void *indices, DWORD dwNum, BOOL bStatic);
    IndexBuffer* CloneIndexBuffer(IndexBuffer *ib, BOOL bStatic=0);

    //Rendering Functions
    void  LoadVertexBuffer(VertexBuffer* vb);
    void  LoadTexture(BaseTexture *texture, int idTexture);  //idTexture being which multitexture to set as
    void  LoadSamplerState(SamplerState *sampler, int idSampler);
    void  LoadIndexBuffer(IndexBuffer *ib);
    void  LoadVertexShader(Shader *vShader);
    void  LoadPixelShader(Shader *pShader);

    Shader* GetCurrentVertexShader()    {return curVertexShader;}
    Shader* GetCurrentPixelShader()     {return curPixelShader;}

    void  SetFrameBufferTarget(BaseTexture *texture, DWORD side);
    void  SetZStencilBufferTarget(ZStencilBuffer *buffer);

    void  GetFrameBuffer(LPBYTE lpData);

    BOOL  BeginScene(BOOL bClear, DWORD dwClearColor);
    void  Draw(GSDrawMode DrawMode, int vertexOffset, DWORD StartVert, DWORD nVerts);
    void  DrawBare(GSDrawMode DrawMode, int vertexOffset, DWORD StartVert, DWORD nVerts);
    void  ResetDevice();
    void  EndScene();

    void  UpdateAccumulationBuffer();
    void  LoadAccumulationBuffer();

    //Drawing mode Functions
    void  ReverseCullMode(BOOL bReverse);
    void  SetCullMode(GSCullMode side);

    void  EnableBlending(BOOL bEnable);
    void  BlendFunction(GSBlendType srcFactor, GSBlendType destFactor);

    void  ClearDepthBuffer(BOOL bFullClear);
    void  EnableDepthTest(BOOL bEnable);
    void  DepthWriteEnable(BOOL bEnable);
    void  DepthFunction(GSDepthTest function);

    void  ClearColorBuffer(BOOL bFullClear, DWORD color);
    void  ColorWriteEnable(BOOL bRedEnable, BOOL bGreenEnable, BOOL bBlueEnable, BOOL bAlphaEnable);

    void  ClearStencilBuffer(BOOL bFill);
    void  EnableStencilTest(BOOL bEnable);
    void  EnableTwoSidedStencil(BOOL bEnable);
    void  StencilWriteEnable(BOOL bEnable);
    void  StencilFunction(GSDepthTest function);
    void  StencilFunctionCCW(GSDepthTest function);
    void  StencilOp(GSStencilOp fail, GSStencilOp zfail, GSStencilOp zpass);
    void  StencilOpCCW(GSStencilOp fail, GSStencilOp zfail, GSStencilOp zpass);
    void  GetStencilOp(GSStencilOp *pFail, GSStencilOp *pZfail, GSStencilOp *pZpass);

    void  EnableClipPlane(BOOL bEnable);
    void  SetClipPlane(const Plane &plane);

    void  SetPointSize(float size);

    //Other functions
    void  Ortho(double left, double right, double top, double bottom, double znear, double zfar);
    void  Frustum(double left, double right, double top, double bottom, double znear, double zfar);

    void  DrawCubeBackdrop(CubeTexture *cubetexture, const Quat &rot, float fLeft, float fRight, float fTop, float fBottom, float fNear);
    void  DrawSpriteEx(Texture *texture, DWORD color, float x, float y, float x2, float y2, float u, float v, float u2, float v2);

    void  SetViewport(int x, int y, int width, int height);
    void  GetViewport(XRect &rect);
    void  SetScissorRect(XRect *pRect);
    void  AdjustZ(double zfar);

    void GetLocalMousePos(int &x, int &y);
    void SetLocalMousePos(int x, int y);

    void EnableTripleBuffering(BOOL bEnable);

    HANDLE                      curWindow;

    Matrix                      curWorldViewMatrix;

    D3DVIEWPORT9                curD3DViewport;

    D3DRECT                     curViewport;
    D3DMATERIAL9                mainMaterial;
    BOOL                        bHardwareLightingEnabled;

    BOOL                        bUsingVertexShader;
    BOOL                        bTexturingEnabled;

    float                       curViewMatrix[16];
    float                       curProjectionMatrix[16];
    float                       curViewProjMatrix[16];

    float pLeft, pRight, pTop, pBottom, pNear, pFar;
    BOOL  pIsFrustum;

    D3DVertexBuffer             *curVB;
    D3DIndexBuffer              *curIB;

    BaseTexture                 *curTextures[16];
    SamplerState                *curSamplers[16];

    D3DShader                   *curVertexShader;
    D3DShader                   *curPixelShader;

    IDirect3DSurface9           *MainColorSurface;
    IDirect3DSurface9           *MainDepthStencilSurface;

    IDirect3DSurface9           *CurrentColorSurface;
    IDirect3DSurface9           *CurrentDepthStencilSurface;

    DWORD                       curMinFilter[16], curMagFilter[16], curMipFilter[16], curAddressU[16], curAddressV[16], curAddressW[16];
    UINT                        curMaxAniso[16];

    BOOL                        bInScene;

    D3DCAPS9                    d3dCaps;

    DWORD                       minAnisoVal, magAnisoVal;

    BOOL                        bUseTripleBuffering;

    BOOL                        bA8Supported;

    //------------------------------------------------------------------
    IDirect3DDevice9            *d3dDevice;

    IDirect3DVertexShader9      *BareVertexShader;

    IDirect3DSurface9           *TB[3];

    DWORD                       curTextureBuffer;

    IDirect3DVertexBuffer9      *spriteBuffer;
    IDirect3DVertexDeclaration9 *BareDeclaration;

    //------------------------------------------------------------------
    List<D3DVertexBuffer*>       VertexBufferList;
    List<D3DIndexBuffer*>        IndexBufferList;
    List<D3DZStencilBuffer*>     ZStencilBufferList;
    List<BaseTexture*>           DynamicTextureList;
    int                          curLightCount;

protected:
    void  ResetViewMatrix();
};

extern IDirect3D9 *d3dMain;


#endif
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  GraphicsSystem:  Graphics System

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

#include "Base.h"



DefineAbstractClass(SamplerState);
DefineAbstractClass(VertexBuffer);
DefineAbstractClass(IndexBuffer);
DefineAbstractClass(BaseTexture);
DefineAbstractClass(Texture);
DefineAbstractClass(OffscreenSurface);
DefineAbstractClass(Texture3D);
DefineAbstractClass(CubeTexture);
DefineAbstractClass(ZStencilBuffer);
DefineAbstractClass(Shader);
DefineAbstractClass(HardwareLight);

DefineAbstractClass(GraphicsSystem);

DefineClass(Font);

#define MANUAL_BUFFER_SIZE 512


/*========================================
   GraphicsSystem Class functions
=========================================*/

GraphicsSystem::GraphicsSystem()
:   bFullScreen(0), curSide(GS_BACK),
    curFont(NULL), curMatrix(0)
{
    traceIn(GraphicsSystem::GraphicsSystem);

    zero(&curDisplayMode, sizeof(DisplayMode));

    //curMode.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY|DM_BITSPERPEL;
    curDisplayMode.dwWidth          = 640;
    curDisplayMode.dwHeight         = 480;
    curDisplayMode.dwFrequency      = 85;
    curDisplayMode.dwBitsPerPixel   = 32;

    Size.x = 640.0f;
    Size.y = 480.0f;

    MatrixStack << Matrix().SetIdentity();

    OSGetWindowRect(curRect);

    traceOut;
}

GraphicsSystem::~GraphicsSystem()
{
    traceIn(GraphicsSystem::~GraphicsSystem);

    int lastFont = FontList.Num();

    for(int i=0; i<lastFont; i++)
        DestroyObject(FontList[0]);

    while(WindowList.Num())
        DestroyObject(WindowList[0]);

    traceOut;
}


void GraphicsSystem::Init()
{
    traceIn(GraphicsSystem::Init);

    RenderStartNew();
        Vertex( 1.0f, -1.0f,  0.0f);
        Vertex( 1.0f,  1.0f,  0.0f);
        Vertex(-1.0f, -1.0f,  0.0f);
        Vertex(-1.0f,  1.0f,  0.0f);

        TexCoord(1.0f, 1.0f);
        TexCoord(1.0f, 0.0f);
        TexCoord(0.0f, 1.0f);
        TexCoord(0.0f, 0.0f);
    vb3DSpriteBuffer = RenderSave();

    VBData *vbd = new VBData;
    vbd->TVList.SetSize(1);
    vbd->VertList.SetSize(MANUAL_BUFFER_SIZE);
    vbd->TVList[0].SetWidth(2);
    vbd->TVList[0].SetSize(MANUAL_BUFFER_SIZE);
    vbd->NormalList.SetSize(MANUAL_BUFFER_SIZE);
    vbd->ColorList.SetSize(MANUAL_BUFFER_SIZE);
    vbDefaultStorage = CreateVertexBuffer(vbd, FALSE);

    SamplerInfo si;
    default2DSampler = CreateSamplerState(si);

    si.addressU = si.addressV = si.addressW = GS_ADDRESS_WRAP;
    si.filter = GS_FILTER_ANISOTROPIC;
    default3DSampler = CreateSamplerState(si);

    String strInputClass = AppConfig->GetString(TEXT("Engine"), TEXT("InputSystem"), TEXT("StandardInput"));
    userInput = (Input*)CreateFactoryObject(strInputClass);
    if(!userInput)
        CrashError(TEXT("Bad input system '%s'"), strInputClass.Array());

    bOmitPostProcess       = AppConfig->GetInt(TEXT("Rendering"), TEXT("OmitPostProcess"));
    bUse32BitFloatTextures = AppConfig->GetInt(TEXT("Rendering"), TEXT("Use32FTextures"));
    bUseHardwareAnimation  = !AppConfig->GetInt(TEXT("Rendering"), TEXT("UseSoftwareAnimation"));

    traceOut;
}

void GraphicsSystem::Destroy()
{
    traceIn(GraphicsSystem::Destroy);

    if(userInput)
        DestroyObject(userInput);

    delete default2DSampler;
    delete default3DSampler;

    userInput = NULL;

    delete vb3DSpriteBuffer;
    delete vbDefaultStorage;

    traceOut;
}


void GraphicsSystem::PreFrame()
{
    traceIn(GraphicsSystem::PreFrame);

    userInput->ProcessInput();

    traceOut;
}


void GraphicsSystem::SetSize(int x, int y)
{
    Size.x = float(curRect.cx = curDisplayMode.dwWidth = x);
    Size.y = float(curRect.cy = curDisplayMode.dwHeight = y);
}


/////////////////////////////////
//material functions
void  GraphicsSystem::SetMaterialColor(DWORD dwRGBA)
{
    Color4 rgba(RGB_Rf(dwRGBA), RGB_Gf(dwRGBA), RGB_Bf(dwRGBA), RGB_Af(dwRGBA));

    SetMaterialColor(rgba);
}

void  GraphicsSystem::SetMaterialSpecular(DWORD dwRGB)
{
    Color4 rgb(RGB_Rf(dwRGB), RGB_Gf(dwRGB), RGB_Bf(dwRGB), 1.0);

    SetMaterialSpecular(rgb);
}

void  GraphicsSystem::SetMaterialIllumination(DWORD dwRGB)
{
    Color4 rgb(RGB_Rf(dwRGB), RGB_Gf(dwRGB), RGB_Bf(dwRGB), 1.0);

    SetMaterialIllumination(rgb);
}

void  GraphicsSystem::SetMaterialGlobalTransparancy(float transparency)
{
    globaltransparency = transparency;
}


/////////////////////////////////
//display functions

void  GraphicsSystem::Set2DMode()
{
    Ortho(0, (double)curDisplayMode.dwWidth, (double)curDisplayMode.dwHeight, 0.0, -1.0, 9999.0);
}

void  GraphicsSystem::Set3DMode(double fovy, double znear, double zfar)
{
    Perspective(fovy, ((float)curDisplayMode.dwWidth)/(float)curDisplayMode.dwHeight, znear, zfar);
}

void  GraphicsSystem::Perspective(double fovy, double aspect, double znear, double zfar)
{
    double xmin, xmax, ymin, ymax;

    ymax = znear * tan(RAD(fovy)/2.0);
    ymin = -ymax;

    xmin = ymin * aspect;
    xmax = ymax * aspect;

    Frustum(xmin, xmax, ymin, ymax, znear, zfar);
}


void GraphicsSystem::DrawSprite(Texture *texture, float x, float y, float x2, float y2)
{
    traceIn(GraphicsSystem::DrawSprite);

    assert(texture);

    DrawSpriteEx(texture, 0xFFFFFFFF, x, y, x2, y2);

    traceOut;
};

void GraphicsSystem::DrawSpriteEx(Texture *texture, DWORD color, float x, float y, float x2, float y2, float u, float v, float u2, float v2)
{
    traceIn(GraphicsSystem::DrawSpriteEx);

    assert(texture);

    SetMaterialColor(0xFFFFFFFF);

    EnableDepthTest(0);
    SetCullMode(GS_NEITHER);

    if(u == -1.0f)  u = 0.0f;
    if(v == -1.0f)  v = 0.0f;
    if(u2 == -1.0f) u2 = 1.0f;
    if(v2 == -1.0f) v2 = 1.0f;

    if(x2 == -1.0f) x2 = x+texture->Width();
    if(y2 == -1.0f) y2 = y+texture->Height();

    LoadTexture(texture);
    LoadDefault2DSampler();

    RenderStartNew();
    Vertex(x, y);       TexCoord(u,  v);   Color(color);
    Vertex(x, y2);      TexCoord(u,  v2);  Color(color);
    Vertex(x2, y);      TexCoord(u2, v);   Color(color);
    Vertex(x2, y2);     TexCoord(u2, v2);  Color(color);
    RenderStop(GS_TRIANGLESTRIP);

    LoadTexture(NULL);

    SetCullMode(GS_BACK);
    EnableDepthTest(1);

    traceOut;
}

void GraphicsSystem::DrawSprite3D(const Quat &cameraRot, Texture *texture, const Vect &pos, float sizeX, float sizeY, float rotation)
{
    traceIn(GraphicsSystem::DrawSprite3D);

    if(!texture)
        return;

    EnableBlending(TRUE);

    LoadTexture(texture);
    LoadDefault3DSampler();

    if(sizeX == 0.0f) sizeX = 1.0f;
    if(sizeY == 0.0f) sizeY = 1.0f;

    //---------------------------------------------------------------------

    LoadVertexBuffer(vb3DSpriteBuffer);
    LoadIndexBuffer(NULL);

    MatrixPush();
    MatrixTranslate(pos);
    MatrixRotate(cameraRot);
    MatrixRotate(0.0f, 0.0f, 1.0f, rotation);
    MatrixScale(sizeX*0.5f, sizeY*0.5f, 1.0f);

        Draw(GS_TRIANGLESTRIP);

    MatrixPop();

    LoadVertexBuffer(NULL);

    //---------------------------------------------------------------------

    LoadTexture(NULL);

    traceOut;
}

void GraphicsSystem::DrawSpriteCenter(Texture *texture, float x, float y, DWORD color)
{
    traceIn(GraphicsSystem::DrawSpriteCenter);

    assert(texture);
    float heightD2 = ((float)texture->Height())*0.5f;
    float widthD2  = ((float)texture->Width())*0.5f;

    DrawSpriteEx(texture, color, x-widthD2, y-heightD2, x+widthD2, y+heightD2);

    traceOut;
}


BOOL GraphicsSystem::ToggleFullScreen()
{
    traceIn(GraphicsSystem::ToggleFullScreen);

    //curMode.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY|DM_BITSPERPEL;
    if((bFullScreen = !bFullScreen))
    {
        OSGetWindowRect(curRect);

        OSSetFullscreen(&curDisplayMode);
    }
    else
        OSSetFullscreen(NULL);

    ResetDevice();

    return bFullScreen;

    traceOut;
}

BOOL GraphicsSystem::SetResolution(const DisplayMode &dm, BOOL bCenter)
{
    traceIn(GraphicsSystem::SetResolution);

    mcpy(&curDisplayMode, &dm, sizeof(DisplayMode));
    OSSetWindowSize(curRect.cx=curDisplayMode.dwWidth, curRect.cy=curDisplayMode.dwHeight, bCenter);

    Size.x = float(curDisplayMode.dwWidth);
    Size.y = float(curDisplayMode.dwHeight);

    Perspective(60.0, ((float)curDisplayMode.dwWidth)/(float)curDisplayMode.dwHeight, 4, 1024);

    if(bFullScreen)
        OSSetFullscreen(&dm);

    ResetDevice();

    return 0;

    traceOut;
}

void  GraphicsSystem::ResetViewport()
{
    traceIn(GraphicsSystem::ResetViewport);

    SetViewport(0, 0, curDisplayMode.dwWidth, curDisplayMode.dwHeight);

    traceOut;
}

Shader* GraphicsSystem::CreateVertexShaderFromFile(CTSTR lpFileName)
{
    traceIn(GraphicsSystem::CreateVertexShaderFromFile);

    Shader* shader;
    XFile ShaderFile;

    if(!ShaderFile.Open(lpFileName, XFILE_READ, XFILE_OPENEXISTING))
        return NULL;

    DWORD dwSize = ShaderFile.GetFileSize();
    LPSTR lpShaderUTF8 = (LPSTR)Allocate(dwSize+1);
    lpShaderUTF8[dwSize] = 0;

    ShaderFile.Read(lpShaderUTF8, dwSize);

    TSTR lpShader = utf8_createTstr(lpShaderUTF8);
    Free(lpShaderUTF8);

    shader = CreateVertexShader(lpShader);


    //Free(lpDec);

    Free(lpShader);
    ShaderFile.Close();

    return shader;

    traceOut;
}

Shader* GraphicsSystem::CreatePixelShaderFromFile(CTSTR lpFileName)
{
    traceIn(GraphicsSystem::CreatePixelShaderFromFile);

    Shader* shader;
    XFile ShaderFile;

    if(!ShaderFile.Open(lpFileName, XFILE_READ, XFILE_OPENEXISTING))
        return NULL;

    DWORD dwSize = ShaderFile.GetFileSize();
    LPSTR lpShaderUTF8 = (LPSTR)Allocate(dwSize+1);
    lpShaderUTF8[dwSize] = 0;

    ShaderFile.Read(lpShaderUTF8, dwSize);

    TSTR lpShader = utf8_createTstr(lpShaderUTF8);
    Free(lpShaderUTF8);

    shader = CreatePixelShader(lpShader);

    Free(lpShader);
    ShaderFile.Close();

    return shader;

    traceOut;
}


GSCullMode GraphicsSystem::GetCullMode()
{
    if(bReverseCullMode && (curSide != GS_NEITHER))
        return (GSCullMode)!curSide;

    return curSide;
}


Vect2 GraphicsSystem::ConvertScreenCoordinate(float x, float y, float coordWidth, float coordHeight)
{
    Vect2 ret;

    ret.x = ((x/coordWidth)*GS->Size.x);
    ret.y = ((y/coordHeight)*GS->Size.y);

    return ret;
}



/////////////////////////////////
//manual rendering functions

void GraphicsSystem::RenderStart()
{
    traceInFast(GraphicsSystem::RenderStart);

    bFastUnbufferedRender = TRUE;
    bNormalSet = FALSE;
    bColorSet = FALSE;
    TexCoordSetList.Clear();

    vbd = vbDefaultStorage->GetData();
    msetd(vbd->ColorList.Array(), 0xFFFFFFFF, sizeof(DWORD)*MANUAL_BUFFER_SIZE);
    dwCurPointVert = 0;
    dwCurTexVert   = 0;
    dwCurColorVert = 0;
    dwCurNormVert  = 0;

    traceOutFast;
}

void GraphicsSystem::RenderStartNew()
{
    traceInFast(GraphicsSystem::RenderStartNew);

    bFastUnbufferedRender = FALSE;
    bNormalSet = FALSE;
    bColorSet = FALSE;
    TexCoordSetList.Clear();

    vbd = new VBData;
    dwCurPointVert = 0;
    dwCurTexVert   = 0;
    dwCurColorVert = 0;
    dwCurNormVert  = 0;

    traceOutFast;
}

void GraphicsSystem::RenderStop(GSDrawMode drawMode)
{
    traceInFast(GraphicsSystem::RenderStop);

    if(dwCurPointVert)
    {
        if(dwCurTexVert && (dwCurTexVert != dwCurPointVert))
            AppWarning(TEXT("Unbuffered Render: Texture vertex count does not match point vertex count"));
        if(dwCurColorVert && (dwCurColorVert != dwCurPointVert))
            AppWarning(TEXT("Unbuffered Render: Color vertex count does not match point vertex count"));
        if(dwCurNormVert && (dwCurNormVert != dwCurPointVert))
            AppWarning(TEXT("Unbuffered Render: Normal vertex count does not match point vertex count"));

        if(bFastUnbufferedRender)
        {
            vbDefaultStorage->FlushBuffers();

            LoadVertexBuffer(vbDefaultStorage);
            LoadIndexBuffer(NULL);

            Draw(drawMode, 0, 0, dwCurPointVert);
        }
        else
        {
            VertexBuffer *buffer;
            buffer = CreateVertexBuffer(vbd, FALSE);
            LoadVertexBuffer(buffer);
            LoadIndexBuffer(NULL);
            Draw(drawMode);
            delete buffer;
        }

        vbd = NULL;
    }
    else
    {
        if(!bFastUnbufferedRender)
        {
            delete vbd;
            vbd = NULL;
        }
    }

    traceOutFast;
}

VertexBuffer *GraphicsSystem::RenderSave()
{
    traceInFast(GraphicsSystem::RenderSave);

    assert(!bFastUnbufferedRender);
    if(bFastUnbufferedRender)
        return NULL;

    if(vbd->VertList.Num())
    {
        VertexBuffer *buffer;

        buffer = CreateVertexBuffer(vbd);

        vbd = NULL;

        return buffer;
    }
    else
    {
        delete vbd;
        vbd = NULL;

        return NULL;
    }

    traceOutFast;
}

void GraphicsSystem::Vertex(float x, float y, float z)
{
    traceInFast(GraphicsSystem::Vertex);

    Vect v(x, y, z);
    Vertex(v);

    traceOutFast;
}

void GraphicsSystem::Vertex(const Vect &v)
{
    traceInFast(GraphicsSystem::Vertex);

    if(bFastUnbufferedRender)
    {
        if(dwCurPointVert >= MANUAL_BUFFER_SIZE)
        {
            AppWarning(TEXT("fast unbuffered rendering is angry that you are trying to use more than %d verts."), MANUAL_BUFFER_SIZE);
            return;
        }

        if(!bNormalSet && dwCurNormVert)
            Normal(vbd->NormalList[dwCurPointVert-1]);
        bNormalSet = 0;

        /////////////////
        if(!bColorSet && dwCurColorVert)
            Color(vbd->ColorList[dwCurPointVert-1]);
        bColorSet = 0;

        /////////////////
        for(DWORD i=0;i<TexCoordSetList.Num();i++)
        {
            if(!TexCoordSetList[i] && dwCurTexVert)
            {
                List<Vect2> &UVList = *vbd->TVList[i].GetV2();
                TexCoord(Vect2(UVList[dwCurPointVert-1]), i);
            }
            TexCoordSetList.Clear(i);
        }

        vbd->VertList[dwCurPointVert] = v;
    }
    else
    {
        if(!bNormalSet && vbd->NormalList.Num())
            Normal(vbd->NormalList[vbd->NormalList.Num()-1]);
        bNormalSet = 0;

        /////////////////
        if(!bColorSet && vbd->ColorList.Num())
            Color(vbd->ColorList[vbd->ColorList.Num()-1]);
        bColorSet = 0;

        /////////////////
        for(DWORD i=0;i<TexCoordSetList.Num();i++)
        {
            if(!TexCoordSetList[i] && vbd->TVList[i].Num())
            {
                List<Vect2> &UVList = *vbd->TVList[i].GetV2();
                TexCoord(Vect2(UVList[UVList.Num()-1]), i);
            }
            TexCoordSetList.Clear(i);
        }

        vbd->VertList << v;
    }

    ++dwCurPointVert;

    traceOutFast;
}

void GraphicsSystem::Normal(float x, float y, float z)
{
    traceInFast(GraphicsSystem::Normal);

    Vect v(x, y, z);
    Normal(v);

    traceOutFast;
}

void GraphicsSystem::Normal(const Vect &v)
{
    traceInFast(GraphicsSystem::Normal);

    if(bFastUnbufferedRender)
        vbd->NormalList[dwCurNormVert] = v;
    else
        vbd->NormalList << v;

    ++dwCurNormVert;

    bNormalSet = TRUE;

    traceOutFast;
}

void GraphicsSystem::Color(DWORD dwRGBA)
{
    traceInFast(GraphicsSystem::Color);

    if(bFastUnbufferedRender)
    {
        if(dwCurColorVert >= MANUAL_BUFFER_SIZE)
        {
            AppWarning(TEXT("fast unbuffered rendering is angry that you are trying to use more than %d verts."), MANUAL_BUFFER_SIZE);
            return;
        }

        vbd->ColorList[dwCurColorVert] = dwRGBA;
    }
    else
        vbd->ColorList << dwRGBA;

    ++dwCurColorVert;

    bColorSet = TRUE;

    traceOutFast;
}

void GraphicsSystem::Color(const Color4 &v)
{
    traceInFast(GraphicsSystem::Color);

    Color(Vect4_to_RGBA(v));

    traceOutFast;
}

void GraphicsSystem::TexCoord(float u, float v, int idTexture)
{
    traceInFast(GraphicsSystem::TexCoord);

    UVCoord uv(u, v);
    TexCoord(uv, idTexture);

    traceOutFast;
}

void GraphicsSystem::TexCoord(const UVCoord &uv, int idTexture)
{
    traceInFast(GraphicsSystem::TexCoord);

    if(idTexture && bFastUnbufferedRender)
    {
        AppWarning(TEXT("Fast Unbuffered rendering is angry that you are trying to use more than one texture unit."));
        return;
    }

    if(vbd->TVList.Num() < (DWORD)(idTexture+1))
    {
        vbd->TVList.SetSize(idTexture+1);
        TexCoordSetList.SetSize(idTexture+1);
        vbd->TVList[idTexture].SetWidth(2);
    }

    if(bFastUnbufferedRender)
    {
        if(dwCurTexVert >= MANUAL_BUFFER_SIZE)
        {
            AppWarning(TEXT("fast unbuffered rendering is angry that you are trying to use more than %d verts."), MANUAL_BUFFER_SIZE);
            return;
        }

        if(TexCoordSetList.Num() < (DWORD)(idTexture+1))
            TexCoordSetList.SetSize(idTexture+1);

        List<Vect2> &UVList = *vbd->TVList[idTexture].GetV2();
        UVList[dwCurTexVert] = uv;
    }
    else
        *vbd->TVList[idTexture].GetV2() << uv;

    ++dwCurTexVert;

    TexCoordSetList.Set(idTexture);

    traceOutFast;
}


/*========================================
   Matrix Stack functions
=========================================*/

inline void  GraphicsSystem::MatrixPush()
{
    MatrixStack << Matrix(MatrixStack[curMatrix]);
    ++curMatrix;
}

inline void  GraphicsSystem::MatrixPop()
{
    MatrixStack.Remove(curMatrix);
    --curMatrix;

    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixSet(const Matrix &m)
{
    MatrixStack[curMatrix] = m;
    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixMultiply(const Matrix &m)
{
    MatrixStack[curMatrix] *= m;
    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixRotate(float x, float y, float z, float a)
{
    MatrixStack[curMatrix] *= Quat(AxisAngle(x, y, z, a));
    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixRotate(const AxisAngle &aa)
{
    MatrixStack[curMatrix] *= Quat(aa);
    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixRotate(const Quat &q)
{
    MatrixStack[curMatrix] *= q;
    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixTranslate(float x, float y, float z)
{
    MatrixStack[curMatrix] *= Vect(x, y, z);
    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixTranslate(const Vect &pos)
{
    MatrixStack[curMatrix] *= pos;
    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixScale(const Vect &scale)
{
    MatrixStack[curMatrix].Scale(scale);
    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixScale(float x, float y, float z)
{
    MatrixStack[curMatrix].Scale(x, y, z);
    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixTranspose()
{
    MatrixStack[curMatrix].Transpose();
    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixInverse()
{
    MatrixStack[curMatrix].Inverse();
    ResetViewMatrix();
}

inline void  GraphicsSystem::MatrixIdentity()
{
    MatrixStack[curMatrix].SetIdentity();
    ResetViewMatrix();
}



inline void  GraphicsSystem::MatrixGet(Vect &v, Quat &q)
{
    q.CreateFromMatrix(MatrixStack[curMatrix]);
    v = MatrixStack[curMatrix].T;
}

inline void  GraphicsSystem::MatrixGet(Matrix &m)
{
    m = MatrixStack[curMatrix];
}


void GraphicsSystem::PreRenderScene()
{
    traceIn(GraphicsSystem::PreRenderScene);

    for(DWORD i=0; i<WindowList.Num();i++)
        PreRenderSceneObjects(WindowList[i]);

    traceOut;
}

void GraphicsSystem::PreRenderSceneObjects(Window *obj)
{
    traceIn(GraphicsSystem::PreRenderSceneObjects);

    if(obj->bRenderable)
        obj->PreRender();

    for(DWORD i=0; i<obj->NumChildren(); i++)
        PreRenderSceneObjects(obj->GetChild(i));

    traceOut;
}

void GraphicsSystem::RenderScene(BOOL bClear, DWORD dwClearColor)
{
    traceIn(GraphicsSystem::RenderScene);
    profileSegment("Render Scene");

    if(BeginScene(bClear, dwClearColor))
    {
        Set2DMode();
        SetViewport(0.0f, 0.0f, Size.x, Size.y);

        for(DWORD i=0; i<WindowList.Num();i++)
            RenderSceneObjects(WindowList[i]);

        profileIn("EndScene");

        EndScene();

        profileOut;
    }

    traceOut;
}

void GraphicsSystem::RenderSceneObjects(Window *obj)
{
    traceIn(GraphicsSystem::RenderSceneObjects);

    if(obj->bRenderable)// && (!level || obj->IsOf(GetClass(Viewport))))
    {
        MatrixPush();
        MatrixIdentity();
        MatrixTranslate(Vect(obj->GetRealPos()));
        obj->Render();
        MatrixPop();
    }

    for(DWORD i=0; i<obj->NumChildren(); i++)
        RenderSceneObjects(obj->GetChild(i));

    traceOut;
}

void GraphicsSystem::PostRenderScene()
{
    traceIn(GraphicsSystem::PostRenderScene);

    for(DWORD i=0; i<WindowList.Num();i++)
        PostRenderSceneObjects(WindowList[i]);

    traceOut;
}

void GraphicsSystem::PostRenderSceneObjects(Window *obj)
{
    traceIn(GraphicsSystem::PostRenderSceneObjects);

    if(obj->bRenderable)
        obj->PostRender();

    for(DWORD i=0; i<obj->NumChildren(); i++)
        PostRenderSceneObjects(obj->GetChild(i));

    traceOut;
}

ControlWindow* GraphicsSystem::GetMouseOver(int x, int y)
{
    traceIn(GraphicsSystem::GetMouseOver);

    ControlWindow *foundWindow = NULL;

    if((x == -1) || (y == -1))
        GetLocalMousePos(x, y);

    for(int i=WindowList.Num()-1; i>=0; --i)
    {
        Window *obj2d = WindowList[i];

        foundWindow = GetMouseOverChildren(obj2d, x, y);
        if(foundWindow)
            break;

        if(obj2d->IsOf(GetClass(ControlWindow)))
        {
            ControlWindow *window = (ControlWindow*)obj2d;
            Vect2 curPos = window->GetRealPos();
            Vect2 curSize = window->GetSize();

            if( (x >= curPos.x) &&
                (y >= curPos.y) &&
                (x < (curPos.x+curSize.x)) &&
                (y < (curPos.y+curSize.y)) )
            {
                foundWindow = window;
                break;
            }
        }
    }

    return foundWindow;

    traceOut;
}

ControlWindow *GraphicsSystem::GetMouseOverChildren(Window *parent, int x, int y)
{
    traceIn(GraphicsSystem::GetMouseOverChildren);

    ControlWindow *foundWindow = NULL;

    for(int i=parent->NumChildren()-1; i>=0; --i)
    {
        Window *obj2d = parent->GetChild(i);

        foundWindow = GetMouseOverChildren(obj2d, x, y);
        if(foundWindow)
            break;

        if(obj2d->IsOf(GetClass(ControlWindow)))
        {
            ControlWindow *window = (ControlWindow*)obj2d;

            Vect offsetPos(window->GetRealPos());
            Vect2 curSize = window->GetSize();

            if( (x >= offsetPos.x) &&
                (y >= offsetPos.y) &&
                (x < (offsetPos.x+curSize.x)) &&
                (y < (offsetPos.y+curSize.y)) )
            {
                foundWindow = window;
                break;
            }
        }
    }

    return foundWindow;

    traceOut;
}

BOOL GraphicsSystem::AdjustDisplayColors(float gamma, float brightness, float contrast)
{
    return OSColorAdjust(gamma, brightness, contrast);
}

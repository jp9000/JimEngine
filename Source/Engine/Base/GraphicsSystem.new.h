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

#ifndef XGSYSTEM_HEADER
#define XGSYSTEM_HEADER


class Shader;


/*=========================================================
    Enums
==========================================================*/

//---------------------------------
//HardwareLight types
enum GSLightType {HL_POINT, HL_DIRECTIONAL, HL_SPOT};


//---------------------------------
//fog
enum GSFogType {FOG_NONE, FOG_LINEAR, FOG_EXP, FOG_EXP2};


//---------------------------------
//drawing types
enum GSDrawMode {GS_POINTS, GS_LINES, GS_LINESTRIP, GS_TRIANGLES, GS_TRIANGLESTRIP, GS_TRIANGLEFAN};


//---------------------------------
//texture formats
enum GSTextureType {GS_ALPHA=1, GS_GRAYSCALE, GS_RGB, GS_RGBA, GS_RGBA16, GS_RGBA16F, GS_RGBA32F, GS_RG32F, GS_RG16F, GS_A8L8, GS_DXT1, GS_DXT3, GS_DXT5};


//---------------------------------
//index size
enum GSIndexType {GS_UNSIGNED_SHORT, GS_UNSIGNED_LONG};


//---------------------------------
//culling information
enum GSCullMode {GS_BACK, GS_FRONT, GS_NEITHER};


//---------------------------------
//blend functions
enum GSBlendType {GS_BLEND_ZERO, GS_BLEND_ONE, GS_BLEND_SRCCOLOR, GS_BLEND_INVSRCCOLOR, GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA, GS_BLEND_DSTCOLOR, GS_BLEND_INVDSTCOLOR, GS_BLEND_DSTALPHA, GS_BLEND_INVDSTALPHA, GS_BLEND_SRCALPHASAT};


//---------------------------------
//depth/stencil testing functions
enum GSDepthTest {GS_NEVER, GS_LESS, GS_LEQUAL, GS_EQUAL, GS_GEQUAL, GS_GREATER, GS_NOTEQUAL, GS_ALWAYS};


//---------------------------------
//stencil operations
enum GSStencilOp {GS_KEEP, GS_ZERO, GS_REPLACE, GS_INCR, GS_DECR, GS_INVERT};


//---------------------------------
//cube texture sides
enum GSCubeSides {GS_POSITIVE_X, GS_NEGATIVE_X, GS_POSITIVE_Y, GS_NEGATIVE_Y, GS_POSITIVE_Z, GS_NEGATIVE_Z};


//---------------------------------
//sampling filters
enum GSSampleFilter
{
    GS_FILTER_LINEAR = 0x111,
    GS_FILTER_POINT = 0x000,
    GS_FILTER_ANISOTROPIC = 0x1111,
    GS_FILTER_MIN_MAG_POINT_MIP_LINEAR = 0x100,
    GS_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x010,
    GS_FILTER_MIN_POINT_MAG_MIP_LINEAR = 0x110,
    GS_FILTER_MIN_LINEAR_MAG_MIP_POINT = 0x001,
    GS_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x101,
    GS_FILTER_MIN_MAG_LINEAR_MIP_POINT = 0x011,

    GS_FILTER_MIN_MAG_MIP_POINT=GS_FILTER_POINT,
    GS_FILTER_MIN_MAG_MIP_LINEAR=GS_FILTER_LINEAR
};

//---------------------------------
//sampling address mode
enum GSAddressMode
{
    GS_ADDRESS_CLAMP,
    GS_ADDRESS_WRAP,
    GS_ADDRESS_MIRROR,
    GS_ADDRESS_BORDER,
    GS_ADDRESS_MIRRORONCE,

    GS_ADDRESS_NONE=GS_ADDRESS_CLAMP,
    GS_ADDRESS_REPEAT=GS_ADDRESS_WRAP
};


/*=========================================================
    32bit rgba struct, rarely used
==========================================================*/

struct DWORD_R10G10B10A2
{
    unsigned R:10;
    unsigned G:10;
    unsigned B:10;
    unsigned A:2;
};


/*=========================================================
    Vertex Buffer Data struct
==========================================================*/

typedef List<UVCoord> UVLIST;

struct VBData
{
    List<Vect>      VertList;
    List<Vect>      NormalList;
    List<DWORD>     ColorList;
    List<Vect>      TangentList;
    List<UVLIST>    TVList;

    VBData()                {}

    inline void FreeAll()
    {
        VertList.FreeAll();
        NormalList.FreeAll();
        ColorList.FreeAll();
        TangentList.FreeAll();

        for(DWORD i=0;i<TVList.Num();i++)
            TVList[i].FreeAll();

        TVList.FreeAll();
    }

    inline void SetSize(int size)
    {
        VertList.SetSize(size);
        NormalList.SetSize(size);
        ColorList.SetSize(size);
        TangentList.SetSize(size);

        for(DWORD i=0;i<TVList.Num();i++)
            TVList[i].SetSize(size);
    }

    inline void CopyFrom(const VBData &vbd)
    {
        VertList.CopyFrom(vbd.VertList);
        NormalList.CopyFrom(vbd.NormalList);
        ColorList.CopyFrom(vbd.ColorList);
        TangentList.CopyFrom(vbd.TangentList);
        TVList.SetSize(vbd.TVList.Num());
        for(DWORD i=0; i<TVList.Num(); i++)
            TVList[i].CopyFrom(vbd.TVList[i]);
    }

    inline void operator=(const VBData &vbd)
    {
        VertList    = vbd.VertList;
        NormalList  = vbd.NormalList;
        ColorList   = vbd.ColorList;
        TangentList = vbd.TangentList;
        TVList      = vbd.TVList;
    }

    inline void Serialize(Serializer &s)
    {
        Vect::SerializeList(s, VertList);//s << VertList;
        Vect::SerializeList(s, NormalList);//s << NormalList;
        s << ColorList;
        Vect::SerializeList(s, TangentList);//s << TangentList;

        DWORD dwSize;

        if(s.IsLoading())
        {
            s << dwSize;
            TVList.SetSize(dwSize);
        }
        else
        {
            dwSize = TVList.Num();
            s << dwSize;
        }

        for(DWORD i=0; i<dwSize; i++)
            s << TVList[i];
    }
};


/*=========================================================
    SamplerState class
==========================================================*/

struct SamplerInfo
{
    inline SamplerInfo()
    {
        zero(this, sizeof(SamplerInfo));
        maxAnisotropy = 16;
    }

    GSSampleFilter filter;
    GSAddressMode addressU;
    GSAddressMode addressV;
    GSAddressMode addressW;
    UINT maxAnisotropy;
    Color4 borderColor;
};

class BASE_EXPORT SamplerState : public Object
{
    friend class GraphicsSystem;
    DeclareClass(SamplerState, Object);

protected:
    SamplerInfo info;

public:
    const SamplerInfo& GetSamplerInfo() const {return info;}
};


/*=========================================================
    Vertex Buffer class
==========================================================*/

class BASE_EXPORT VertexBuffer : public Object
{
    DeclareClass(VertexBuffer, Object);

public:
    virtual void  CopyFrom(VertexBuffer *buffer)=0;
    virtual void  FlushBuffers(BOOL bRebuild=FALSE)=0;
    virtual VBData* GetData()=0;
};



/*=========================================================
    Index Buffer class
==========================================================*/

class BASE_EXPORT IndexBuffer : public Object
{
    DeclareClass(IndexBuffer, Object);

public:
    virtual void  CopyFrom(IndexBuffer *buffer)=0;
    virtual void  FlushBuffer()=0;
    virtual void* GetData()=0;
};


/*=========================================================
    Texture class
==========================================================*/

class BASE_EXPORT BaseTexture : public Object
{
    DeclareClass(BaseTexture, Object);

protected:
    LPVOID lpTexData;

public:
    virtual void SetLOD(int level)=0;

    inline LPVOID GetInternalData() const {return lpTexData;}
};


/*=========================================================
    Texture class
==========================================================*/

class BASE_EXPORT Texture : public BaseTexture
{
    friend class ResourceManager;

    DeclareClass(Texture, BaseTexture);

    BOOL bGenMipMaps;

public:
    virtual DWORD Width()=0;
    virtual DWORD Height()=0;
    virtual BOOL HasAlpha()=0;
    virtual void* GetImage(BOOL bForce=FALSE, void *lpInputPtr=NULL)=0;
    virtual void SetImage(void *lpData)=0;
    virtual DWORD GetSize()=0;

    virtual DWORD GetFormat()=0;

    virtual void RebuildMipMaps()=0;

    BOOL GenMipMaps() {return bGenMipMaps;}

    Declare_Internal_Member(native_Width);
    Declare_Internal_Member(native_Height);
    Declare_Internal_Member(native_HasAlpha);
};



/*=========================================================
    Texture3D class
==========================================================*/

class BASE_EXPORT Texture3D : public BaseTexture
{
    DeclareClass(Texture3D, BaseTexture);

public:
    //virtual void GetImage(void *lpData);
};


/*=========================================================
    Cube Texture class
==========================================================*/

class BASE_EXPORT CubeTexture : public BaseTexture
{
    friend class ResourceManager;

    DeclareClass(CubeTexture, BaseTexture);

    BOOL bGenMipMaps;

public:
    virtual DWORD Width()=0;

    virtual void* GetImage(int side)=0;  //only works on dynamic textures.
    virtual void SetImage(int side, void *lpData)=0;

    virtual void RebuildMipMaps()=0;

    BOOL GenMipMaps() {return bGenMipMaps;}

    Declare_Internal_Member(native_Size);
};


/*=========================================================
    ZStencilBuffer
==========================================================*/

class BASE_EXPORT ZStencilBuffer : public Object
{
    DeclareClass(ZStencilBuffer, Object);

public:
    virtual DWORD Width()=0;
    virtual DWORD Height()=0;
};


/*=========================================================
    Shader class
==========================================================*/

enum ShaderParameterType
{
    Parameter_Unknown,
    Parameter_Bool,
    Parameter_Float,
    Parameter_Int,
    Parameter_String,
    Parameter_Vector2,
    Parameter_Vector3,
    Parameter_Vector=Parameter_Vector3,
    Parameter_Vector4,
    Parameter_Matrix3x3,
    Parameter_Matrix,
    Parameter_Texture
};

struct ShaderParameterInfo
{
    String name;
    ShaderParameterType type;
};

enum ShaderType
{
    Shader_Vertex,
    Shader_Pixel,
    Shader_Geometry
};


class BASE_EXPORT Shader : public Object
{
    DeclareClass(Shader, Object);

public:
    virtual ShaderType GetType()=0;

    virtual int    NumParams()=0;
    virtual HANDLE GetParameter(int parameter)=0;
    virtual HANDLE GetParameterByName(CTSTR lpName)=0;
    virtual void   GetParameterInfo(HANDLE hObject, ShaderParameterInfo &paramInfo)=0;

    virtual void   SetBool(HANDLE hObject, BOOL bValue)=0;
    virtual void   SetFloat(HANDLE hObject, float fValue)=0;
    virtual void   SetInt(HANDLE hObject, int iValue)=0;
    virtual void   SetMatrix(HANDLE hObject, float *matrix)=0;
    virtual void   SetVector(HANDLE hObject, const Vect &value)=0;
    virtual void   SetVector2(HANDLE hObject, const Vect2 &value)=0;
    virtual void   SetVector4(HANDLE hObject, const Vect4 &value)=0;
    virtual void   SetTexture(HANDLE hObject, BaseTexture *texture)=0;
    virtual void   SetValue(HANDLE hObject, void *val, DWORD dwSize)=0;

    inline  void   SetColor(HANDLE hObject, const Color4 &value)
    {
        SetVector4(hObject, value);
    }
    inline  void   SetColor(HANDLE hObject, float fR, float fB, float fG, float fA=1.0f)
    {
        SetVector4(hObject, Color4(fR, fB, fG, fA));
    }
    inline  void   SetColor(HANDLE hObject, DWORD color)
    {
        SetVector4(hObject, RGBA_to_Vect4(color));
    }

    inline  void   SetColor3(HANDLE hObject, const Color3 &value)
    {
        SetVector(hObject, value);
    }
    inline  void   SetColor3(HANDLE hObject, float fR, float fB, float fG)
    {
        SetVector(hObject, Color3(fR, fB, fG));
    }
    inline  void   SetColor3(HANDLE hObject, DWORD color)
    {
        SetVector(hObject, RGB_to_Vect(color));
    }

    inline  void   SetVector4(HANDLE hObject, float fX, float fY, float fZ, float fW)
    {
        SetVector4(hObject, Vect4(fX, fY, fZ, fW));
    }

    inline  void   SetVector(HANDLE hObject, float fX, float fY, float fZ)
    {
        SetVector(hObject, Vect(fX, fY, fZ));
    }

    inline void SetMatrix(HANDLE hObject, const Matrix &mat)
    {
        float out[16];
        Matrix4x4Convert(out, mat);
        SetMatrix(hObject, out);
    }

    inline void SetMatrixIdentity(HANDLE hObject)
    {
        float out[16];
        Matrix4x4Identity(out);
        SetMatrix(hObject, out);
    }
};


/*=========================================================
    Effect class
==========================================================*/

enum EffectPropertyType
{
    EffectProperty_None,
    EffectProperty_Bool,
    EffectProperty_Float,
    EffectProperty_Color,
    EffectProperty_Texture,
};

struct EffectParameterInfo
{
    String name;
    ShaderParameterType type;

    String fullName;
    EffectPropertyType propertyType;

    String strDefaultTexture;

    union
    {
        struct {float fMin, fMax, fInc, fMul;};
        struct {int   iMin, iMax, iInc, iMul;};
    };
};

struct EffectParam;
struct EffectPass;
struct EffectTechnique;
struct PassParam;

class BASE_EXPORT Effect : public Object
{
    friend struct EffectProcessor;
    DeclareClass(Effect, Object);

    BOOL bProcessing;
    String strProcessingInfo;

    String effectPath;

    List<EffectParam>       Params;
    List<EffectTechnique>   Techniques;

    String strEffectDir;
    EffectPass *curPass;
    EffectTechnique *curTech;

    EffectParam *firstChanged;

    HANDLE hViewProj, hWorld, hScale;

    GraphicsSystem *system;

    inline void UploadAllShaderParams();

public:
    Effect() {}
    Effect(GraphicsSystem *curSystem, CTSTR lpEffectFile);
    ~Effect();

    inline BOOL    IsProcessing()   {return bProcessing;}
    inline String  ProcessingInfo() {return strProcessingInfo;}

    void   GetEffectParameterInfo(HANDLE hParameter, EffectParameterInfo &paramInfo) const;

    HANDLE GetTechnique(CTSTR lpTechnique) const;
    BOOL   UsableTechnique(HANDLE hObject) const;

    HANDLE GetPass(HANDLE hTechnique, DWORD i) const;
    HANDLE GetPassByName(HANDLE hTechnique, CTSTR lpName) const;

    DWORD  BeginTechnique(HANDLE hTechnique);
    void   EndTechnique();

    void   BeginPass(DWORD i);
    void   BeginPassByHandle(HANDLE hPass);
    void   EndPass();

    inline void UpdateParams();

    inline int    NumParams() const {return Params.Num();}
    inline HANDLE GetParameter(int parameter) const;
    inline HANDLE GetParameterByName(CTSTR lpName) const;

    inline void   GetBool(HANDLE hObject, BOOL &bValue) const;
    inline void   GetFloat(HANDLE hObject, float &fValue) const;
    inline void   GetInt(HANDLE hObject, int &iValue) const;
    inline void   GetMatrix(HANDLE hObject, float *matrix) const;
    inline void   GetVector(HANDLE hObject, Vect &value) const;
    inline void   GetVector2(HANDLE hObject, Vect2 &value) const;
    inline void   GetVector4(HANDLE hObject, Vect4 &value) const;
    inline void   GetString(HANDLE hObject, String &value) const;

    inline BOOL   GetDefaultBool(HANDLE hObject, BOOL &bValue) const;
    inline BOOL   GetDefaultFloat(HANDLE hObject, float &fValue) const;
    inline BOOL   GetDefaultInt(HANDLE hObject, int &iValue) const;
    inline BOOL   GetDefaultMatrix(HANDLE hObject, float *matrix) const;
    inline BOOL   GetDefaultVector(HANDLE hObject, Vect &value) const;
    inline BOOL   GetDefaultVector2(HANDLE hObject, Vect2 &value) const;
    inline BOOL   GetDefaultVector4(HANDLE hObject, Vect4 &value) const;
    inline BOOL   GetDefaultTexture(HANDLE hObject, String &value) const;

    inline void   SetBool(HANDLE hObject, BOOL bValue);
    inline void   SetFloat(HANDLE hObject, float fValue);
    inline void   SetInt(HANDLE hObject, int iValue);
    inline void   SetMatrix(HANDLE hObject, float *matrix);
    inline void   SetVector(HANDLE hObject, const Vect &value);
    inline void   SetVector2(HANDLE hObject, const Vect2 &value);
    inline void   SetVector4(HANDLE hObject, const Vect4 &value);
    inline void   SetTexture(HANDLE hObject, const BaseTexture *texture);
    inline void   SetValue(HANDLE hObject, const void *val, DWORD dwSize);

    inline HANDLE GetViewProj() const {return hViewProj;}
    inline HANDLE GetWorld() const {return hWorld;}
    inline HANDLE GetScale() const {return hScale;}

    //--------------------------------------------

    inline  void   GetColor(HANDLE hObject, Color4 &value)
    {
        GetVector4(hObject, value);
    }
    inline  void   GetColor(HANDLE hObject, DWORD &color)
    {
        Vect4 value;
        GetVector4(hObject, value);
        color = Vect4_to_RGBA(value);
    }

    inline  BOOL   GetDefaultColor(HANDLE hObject, Color4 &value)
    {
        return GetDefaultVector4(hObject, value);
    }
    inline  BOOL   GetDefaultColor(HANDLE hObject, DWORD &color)
    {
        Vect4 value;
        if(GetDefaultVector4(hObject, value))
        {
            color = Vect4_to_RGBA(value);
            return TRUE;
        }
        return FALSE;
    }

    inline  void   SetColor(HANDLE hObject, const Color4 &value)
    {
        SetVector4(hObject, value);
    }
    inline  void   SetColor(HANDLE hObject, float fR, float fB, float fG, float fA=1.0f)
    {
        SetVector4(hObject, Color4(fR, fB, fG, fA));
    }
    inline  void   SetColor(HANDLE hObject, DWORD color)
    {
        SetVector4(hObject, RGBA_to_Vect4(color));
    }

    //--------------------------------------------

    inline  void   GetColor3(HANDLE hObject, Color3 &value)
    {
        GetVector(hObject, value);
    }
    inline  void   GetColor3(HANDLE hObject, DWORD &color)
    {
        Vect value;
        GetVector(hObject, value);
        color = Vect_to_RGB(value);
    }

    inline  BOOL   GetDefaultColor3(HANDLE hObject, Color3 &value)
    {
        return GetDefaultVector(hObject, value);
    }
    inline  BOOL   GetDefaultColor3(HANDLE hObject, DWORD &color)
    {
        Vect value;
        if(GetDefaultVector(hObject, value))
        {
            color = Vect_to_RGB(value);
            return TRUE;
        }
        return FALSE;
    }

    inline  void   SetColor3(HANDLE hObject, const Color3 &value)
    {
        SetVector(hObject, value);
    }
    inline  void   SetColor3(HANDLE hObject, float fR, float fB, float fG)
    {
        SetVector(hObject, Color3(fR, fB, fG));
    }
    inline  void   SetColor3(HANDLE hObject, DWORD color)
    {
        SetVector(hObject, RGB_to_Vect(color));
    }

    //--------------------------------------------

    inline  void   SetVector4(HANDLE hObject, float fX, float fY, float fZ, float fW)
    {
        SetVector4(hObject, Vect4(fX, fY, fZ, fW));
    }

    inline  void   SetVector(HANDLE hObject, float fX, float fY, float fZ)
    {
        SetVector(hObject, Vect(fX, fY, fZ));
    }

    inline void SetMatrix(HANDLE hObject, const Matrix &mat)
    {
        float out[16];
        Matrix4x4Convert(out, mat);
        SetMatrix(hObject, out);
    }

    inline void SetMatrixIdentity(HANDLE hObject)
    {
        float out[16];
        Matrix4x4Identity(out);
        SetMatrix(hObject, out);
    }
};


/*=========================================================
    Hardware (Vertex) Lighting Class
==========================================================*/

class BASE_EXPORT HardwareLight : public Object
{
    DeclareClass(HardwareLight, Object);

public:
    virtual void  Enable(BOOL bEnable)=0;
    virtual void  SetIntensity(float intensity)=0;
    virtual void  SetAmbientColor(DWORD dwRGB)=0;
    virtual void  SetDiffuseColor(DWORD dwRGB)=0;
    virtual void  SetSpecularColor(DWORD dwRGB)=0;
    virtual void  SetAttenuation(DWORD dwType, float attenuation)=0;
    virtual void  Retransform()=0;
    virtual void  SetCutoff(float degrees)=0;
    virtual void  SetType(DWORD dwType)=0;
};


/*=========================================================
    Font class
==========================================================*/

#pragma pack(push, 1)

struct ByteRect
{
    BYTE x,y,x2,y2;
};

#pragma pack(pop)


struct GlyphInfo;
struct FontFaceInfo;


class BASE_EXPORT XenFont : public Object
{
    DeclareClass(XenFont, Object);

    friend class GraphicsSystem;

public:
    ~XenFont();

    int  LetterWidth(TCHAR letter);
    int  WordWidth(CTSTR lpString);
    int  TextWidth(CTSTR lpString);
    int  GetFontHeight();
    void DrawLetter(TCHAR letter, int x, int y, DWORD color=0xFFFFFFFF);

    Declare_Internal_Member(native_LetterWidth);
    Declare_Internal_Member(native_WordWidth);
    Declare_Internal_Member(native_TextWidth);
    Declare_Internal_Member(native_GetFontHeight);
    Declare_Internal_Member(native_DrawLetter);

private:
    String          strName;
    int             faceID;
    LPVOID          fontData;
    int             size;
    int             internalSize;
    int             maxCaches;
    int             maxGlyphsPerCache;
    int             maxGlyphsPerRow;

    List<Texture*>  TexCache;
    List<GlyphInfo> CurGlyphs;
    List<WORD>      LastUsedGlyphs;

    static SafeList<FontFaceInfo> FaceList;

    GlyphInfo* NewGlyph(TCHAR newChar);
    GlyphInfo* GetGlyph(TCHAR chr, BOOL bAdd=TRUE);

    GraphicsSystem *system;
};

void InitFontStuff();
void DestroyFontStuff();


/*=========================================================
    Graphics System Class
==========================================================*/

class BASE_EXPORT GraphicsSystem : public FrameObject
{
    friend class XenFont;
    friend class Effect;
    friend class Window;

    DeclareClass(GraphicsSystem, FrameObject);

public:
    ////////////////////////////
    //Initialization/Destruction
    GraphicsSystem();
    virtual ~GraphicsSystem();

    virtual void Init();
    virtual void Destroy();

    virtual void PreFrame();

    virtual BOOL InitializeDevice(HANDLE hWindow)=0;

    virtual void SetSize(int x, int y);

    inline Vect2 GetSize() const                    {return Size;}
    inline void  GetSize(Vect2 &v2) const           {v2 = Size;}
    inline void  GetSize(float &x, float &y) const  {x = Size.x; y = Size.y;}
    inline void  GetSize(int &x, int &y) const      {x = curRect.cx; y = curRect.cy;}

    inline int   GetSizeX() const                   {return curRect.cx;}
    inline int   GetSizeY() const                   {return curRect.cy;}
    inline float GetSizeXF() const                  {return Size.x;}
    inline float GetSizeYF() const                  {return Size.y;}


    ////////////////////////////
    //Device Information
    virtual TSTR GetDeviceName()=0;

    virtual DWORD VertexShaderVersion()=0;
    virtual DWORD PixelShaderVersion()=0;
    virtual BOOL  SupportsTwoSidedStencil()=0;


    ////////////////////////////
    //Matrix Functions
    inline void  MatrixPush();
    inline void  MatrixPop();
    inline void  MatrixSet(const Matrix &m);
    inline void  MatrixMultiply(const Matrix &m);
    inline void  MatrixRotate(float x, float y, float z, float a);
    inline void  MatrixRotate(const AxisAngle &aa);
    inline void  MatrixRotate(const Quat &q);
    inline void  MatrixTranslate(float x, float y, float z);

    inline void  MatrixTranslate(const Vect &pos);
    inline void  MatrixScale(const Vect &scale);
    inline void  MatrixScale(float x, float y, float z);
    inline void  MatrixTranspose();
    inline void  MatrixInverse();
    inline void  MatrixIdentity();

    inline void  MatrixGet(Vect &v, Quat &q);
    inline void  MatrixGet(Matrix &m);

    inline void  MatrixTranslate(const Vect2 &pos2) {MatrixTranslate(Vect(pos2));}

    ////////////////////////////
    //Hardware Lighting Functions
    virtual HardwareLight* CreateHardwareLight(DWORD dwType=0)=0;
    virtual void  EnableHardwareLighting(BOOL bEnable)=0;


    ////////////////////////////
    //Fog Functions
    virtual void  SetFogType(int mode)=0;
    virtual void  SetFogColor(DWORD dwRGB)=0;
    virtual void  SetFogDensity(float density)=0;
    virtual void  SetFogStart(float start)=0;
    virtual void  SetFogEnd(float end)=0;


    ////////////////////////////
    //Material Functions
    virtual void  SetMaterialColor(DWORD dwRGBA);
    virtual void  SetMaterialSpecular(DWORD dwRGB);
    virtual void  SetMaterialIllumination(DWORD dwRGB);
    virtual void  SetMaterialColor(const Color4 &rgba)=0;
    virtual void  SetMaterialSpecular(const Color4 &rgb)=0;
    virtual void  SetMaterialIllumination(const Color4 &rgb)=0;
    virtual void  SetMaterialShininess(float shininess)=0;
    virtual void  SetMaterialGlobalTransparancy(float transparency);


    ////////////////////////////
    //Texture Functions
    virtual Texture*        CreateTexture(unsigned int width, unsigned int height, DWORD dwColorFormat, void *lpData, BOOL bBuildMipMaps, BOOL bStatic)=0;
    virtual CubeTexture*    CreateCubeTexture(unsigned int width, DWORD dwColorFormat, BOOL bBuildMipMaps, BOOL bStatic)=0;
    virtual Texture3D*      Create3DTexture(unsigned int width, unsigned int height, unsigned int depth, DWORD dwColorFormat, void *lpData, BOOL bBuildMipMaps)=0;

    virtual SamplerState*   CreateSamplerState(SamplerInfo &info)=0;

    virtual Texture*        CreateTextureFromFile(CTSTR lpFile, BOOL bBuildMipMaps)=0;
    virtual CubeTexture*    CreateCubeTextureFromFile(CTSTR lpFile, BOOL bBuildMipMaps)=0;
    virtual Texture3D*      Create3DTextureFromFile(CTSTR lpFile, BOOL bBuildMipMaps)=0;

    virtual Texture*        CreateFrameBuffer(unsigned int width, unsigned int height, DWORD dwColorFormat, BOOL bGenMipMaps)=0;
    virtual CubeTexture*    CreateCubeFrameBuffer(unsigned int width, DWORD dwColorFormat, BOOL bGenMipMaps)=0;
    virtual ZStencilBuffer* CreateZStencilBuffer(unsigned int width, unsigned int height)=0;

    virtual void  EnableTexturing(BOOL bEnable)=0;
    virtual void  EnableProjectiveTexturing(BOOL bEnable, int idTexture=0)=0;
    virtual BOOL  TexturingEnabled()=0;


    ////////////////////////////
    //Font Functions
    virtual XenFont* CreateFont(TSTR lpName);
    virtual void  SetCurFont(TSTR lpName);
    virtual void  SetCurFont(XenFont *font);
    virtual XenFont* GetCurFont() {return curFont;}
    virtual void  SetFontColor(DWORD color);
    inline  void  SetFontColor(Color4 color) {SetFontColor(Vect4_to_RGBA(Vect4(color).ClampColor()));}
    virtual void  DrawText(int x, int y, int cx, int cy, BOOL bWrapWords, CTSTR lpText);
    virtual void  DrawTextCenter(int x, int y, TSTR lpText);
    virtual void  Printf(int x, int y, int cx, int cy, BOOL bWrapWords, CTSTR lpString, ...);
    XenFont* GetFont(TSTR lpName);


    ////////////////////////////
    //Shader Functions
    virtual Shader*         CreateVertexShader(CTSTR lpShader)=0;
    virtual Shader*         CreatePixelShader(CTSTR lpShader)=0;
    Shader*                 CreateVertexShaderFromFile(CTSTR lpFileName);
    Shader*                 CreatePixelShaderFromFile(CTSTR lpFileName);

    inline Effect* CreateEffectFromFile(CTSTR lpEffect)
    {
        return CreateObjectParam2(Effect, this, lpEffect);
    }


    ////////////////////////////
    //Vertex Buffer Functions
    virtual VertexBuffer* CreateVertexBuffer(VBData *vbData, BOOL bStatic=1)=0;
    virtual VertexBuffer* CloneVertexBuffer(VertexBuffer *vb, BOOL bStatic=1)=0;


    ////////////////////////////
    //Index Buffer Functions
    virtual IndexBuffer* CreateIndexBuffer(GSIndexType IndexType, void *indices, DWORD dwNum, BOOL bStatic=1)=0;
    virtual IndexBuffer* CloneIndexBuffer(IndexBuffer *ib, BOOL bStatic=1)=0;


    ////////////////////////////
    //Main Rendering Functions
    virtual void  LoadVertexBuffer(VertexBuffer* vb)=0;
    virtual void  LoadTexture(BaseTexture *texture, int idTexture=0)=0;
    virtual void  LoadSamplerState(SamplerState *sampler, int idSampler=0)=0;
    virtual void  LoadIndexBuffer(IndexBuffer *ib)=0;
    virtual void  LoadVertexShader(Shader *vShader)=0;
    virtual void  LoadPixelShader(Shader *pShader)=0;

    inline  void  LoadDefault2DSampler(int idSampler=0) {LoadSamplerState(default2DSampler);}
    inline  void  LoadDefault3DSampler(int idSampler=0) {LoadSamplerState(default3DSampler);}

    virtual Shader* GetCurrentPixelShader()=0;
    virtual Shader* GetCurrentVertexShader()=0;

    virtual void  SetFrameBufferTarget(BaseTexture *texture, DWORD side=0)=0;
    virtual void  SetZStencilBufferTarget(ZStencilBuffer *buffer)=0;

    virtual void  GetFrameBuffer(LPBYTE lpData)=0;

    virtual BOOL  BeginScene(BOOL bClear=FALSE, DWORD dwClearColor=0xFF000000)=0;
    virtual void  Draw(GSDrawMode drawMode, int vertexOffset=0, DWORD StartVert=0, DWORD nVerts=0)=0;
    virtual void  DrawBare(GSDrawMode drawMode, int vertexOffset=0, DWORD StartVert=0, DWORD nVerts=0)=0;
    virtual void  ResetDevice()=0;
    virtual void  EndScene()=0;

    virtual void  UpdateAccumulationBuffer()=0;
    virtual void  LoadAccumulationBuffer()=0;

    ////////////////////////////
    //Drawing mode functions
    virtual void  ReverseCullMode(BOOL bReverse)=0;
    virtual void  SetCullMode(GSCullMode side)=0;
    virtual GSCullMode GetCullMode();

    virtual void  EnableBlending(BOOL bEnable)=0;
    virtual void  BlendFunction(GSBlendType srcFactor, GSBlendType destFactor)=0;

    virtual void  ClearDepthBuffer(BOOL bFullClear)=0;
    virtual void  EnableDepthTest(BOOL bEnable)=0;
    virtual void  DepthWriteEnable(BOOL bEnable)=0;
    virtual void  DepthFunction(GSDepthTest function)=0;
    inline  GSDepthTest GetDepthFunction() {return curDepthFunction;}

    virtual void  ClearColorBuffer(BOOL bFullClear=TRUE, DWORD color=0xFF000000)=0;
    virtual void  ColorWriteEnable(BOOL bRedEnable, BOOL bGreenEnable, BOOL bBlueEnable, BOOL bAlphaEnable)=0;

    virtual void  ClearStencilBuffer(BOOL bFill)=0;
    virtual void  EnableStencilTest(BOOL bEnable)=0;
    virtual void  EnableTwoSidedStencil(BOOL bEnable)=0;
    virtual void  StencilWriteEnable(BOOL bEnable)=0;
    virtual void  StencilFunction(GSDepthTest function)=0;
    virtual void  StencilFunctionCCW(GSDepthTest function)=0;
    virtual void  StencilOp(GSStencilOp fail, GSStencilOp zfail, GSStencilOp zpass)=0;
    virtual void  StencilOpCCW(GSStencilOp fail, GSStencilOp zfail, GSStencilOp zpass)=0;
    virtual void  GetStencilOp(GSStencilOp *pFail, GSStencilOp *pZfail, GSStencilOp *pZpass)=0;

    virtual void  EnableClipPlane(BOOL bEnable)=0;
    virtual void  SetClipPlane(const Plane &plane)=0;

    virtual void  SetPointSize(float size)=0;

    ////////////////////////////
    //OpenGL style manual rendering functions
    void  RenderStartNew();
    void  RenderStart();
    void  RenderStop(GSDrawMode dwDrawMode);
    VertexBuffer* RenderSave();
    void  Vertex(float x, float y, float z=0);
    void  Vertex(const Vect &v);
    void  Normal(float x, float y, float z);
    void  Normal(const Vect &v);
    void  Color(DWORD dwRGBA);
    void  Color(const Color4 &v);
    void  TexCoord(float u, float v, int idTexture=0);
    void  TexCoord(const UVCoord &uv, int idTexture=0);

    inline void  Vertex(const Vect2 &v2) {Vertex(Vect(v2));}

    ////////////////////////////
    //Resolution/Fullscreen Functions
    virtual BOOL  ToggleFullScreen();
    virtual BOOL  SetResolution(DISPLAYMODE &dm, BOOL bCenter=FALSE);
    virtual void  EnumDisplayInfo(List<DISPLAYMODE> &DisplayModes);

    Vect2 ConvertScreenCoordinate(float x, float y, float coordWidth, float coordHeight);


    ////////////////////////////
    //Other Functions
    void  Set2DMode();                        //Sets up default 2D ortho mode
    void  Set3DMode(double fovy=60.0, double znear=4.0, double zfar=4096.0);        //Sets up default 3D proj. mode with field of view
    void  Perspective(double fovy, double aspect, double znear, double zfar);
    virtual void  Ortho(double left, double right, double top, double bottom, double znear, double zfar)=0;
    virtual void  Frustum(double left, double right, double top, double bottom, double znear, double zfar)=0;
    virtual void  AdjustZ(double zfar=4096.0)=0;

    virtual void  SetViewport(int x, int y, int width, int height)=0;
    virtual void  GetViewport(XRect &rect)=0;
    virtual void  SetScissorRect(XRect *pRect=NULL)=0;
    void  ResetViewport();


    void DrawSprite(Texture *texture, float x, float y, float x2 = -1.0f, float y2 = -1.0f);
    void DrawSprite3D(const Quat &cameraRot, Texture *texture, const Vect &pos, float sizeX, float sizeY, float rotation=0.0f);
    void DrawSpriteCenter(Texture *texture, float x, float y, DWORD color=0xFFFFFFFF);
    virtual void DrawCubeBackdrop(Camera *cam, CubeTexture *cubetexture, float x, float y, float x2 = -1.0f, float y2 = -1.0f)=0;
    virtual void DrawSpriteEx(Texture *texture, DWORD color, float x, float y, float x2 = -1.0f, float y2 = -1.0f, float u = -1.0f, float v = -1.0f, float u2 = -1.0f, float v2 = -1.0f);

    virtual void PreRenderScene();
    virtual void RenderScene(BOOL bClear=TRUE, DWORD dwClearColor=0xFF000000);
    virtual void PostRenderScene();

    virtual void PreRenderSceneObjects(Window *obj);
    virtual void RenderSceneObjects(Window *obj);
    virtual void PostRenderSceneObjects(Window *obj);

    inline Input* GetInput() {return userInput;}

    inline BOOL MustRebuildRenderTargets() {return bRebuildRenderTargets;}

    inline void GetCurrentDisplayMode(DISPLAYMODE &displayMode) const {mcpy(&displayMode, &curDisplayMode, sizeof(DISPLAYMODE));}
    inline BOOL IsFullscreen() {return bFullScreen;}

    ControlWindow *GetMouseOver(int x=-1, int y=-1);
    ControlWindow *GetMouseOverChildren(Window *parent, int x, int y);
    virtual void GetLocalMousePos(int &x, int &y)=0;
    virtual void SetLocalMousePos(int x, int y)=0;

    virtual void EnableTripleBuffering(BOOL bEnable)=0;

    inline Effect* GetActiveEffect() const {return curEffect;}

    inline Effect *CurrentProcessingEffect() const {return curProcessingEffect;}


    Declare_Internal_Member(native_GetSize);
    Declare_Internal_Member(native_GetSizeX);
    Declare_Internal_Member(native_GetSizeY);
    Declare_Internal_Member(native_MatrixPush);
    Declare_Internal_Member(native_MatrixPop);
    Declare_Internal_Member(native_MatrixSet);
    Declare_Internal_Member(native_MatrixMultiply);
    Declare_Internal_Member(native_MatrixRotate);
    Declare_Internal_Member(native_MatrixRotate2);
    Declare_Internal_Member(native_MatrixRotate3);
    Declare_Internal_Member(native_MatrixTranslate);
    Declare_Internal_Member(native_MatrixTranslate2);
    Declare_Internal_Member(native_MatrixTranslate3);
    Declare_Internal_Member(native_MatrixScale);
    Declare_Internal_Member(native_MatrixScale2);
    Declare_Internal_Member(native_MatrixTranspose);
    Declare_Internal_Member(native_MatrixInverse);
    Declare_Internal_Member(native_MatrixIdentity);
    Declare_Internal_Member(native_EnableHardwareLighting);
    Declare_Internal_Member(native_SetMaterialColor);
    Declare_Internal_Member(native_SetMaterialSpecular);
    Declare_Internal_Member(native_SetMaterialIllumination);
    Declare_Internal_Member(native_SetMaterialColor2);
    Declare_Internal_Member(native_SetMaterialSpecular2);
    Declare_Internal_Member(native_SetMaterialIllumination2);
    Declare_Internal_Member(native_SetMaterialShininess);
    Declare_Internal_Member(native_SetMaterialGlobalTransparancy);
    Declare_Internal_Member(native_SetCurFont);
    Declare_Internal_Member(native_SetCurFont2);
    Declare_Internal_Member(native_GetFont);
    Declare_Internal_Member(native_SetFontColor);
    Declare_Internal_Member(native_SetFontColor2);
    Declare_Internal_Member(native_DrawText);
    Declare_Internal_Member(native_DrawTextCenter);
    Declare_Internal_Member(native_RenderStartNew);
    Declare_Internal_Member(native_RenderStart);
    Declare_Internal_Member(native_RenderStop);
    Declare_Internal_Member(native_RenderSave);
    Declare_Internal_Member(native_Vertex);
    Declare_Internal_Member(native_Vertex2);
    Declare_Internal_Member(native_Normal);
    Declare_Internal_Member(native_Normal2);
    Declare_Internal_Member(native_Color);
    Declare_Internal_Member(native_Color2);
    Declare_Internal_Member(native_TexCoord);
    Declare_Internal_Member(native_TexCoord2);
    Declare_Internal_Member(native_LoadVertexBuffer);
    Declare_Internal_Member(native_LoadTexture);
    Declare_Internal_Member(native_LoadSampler);
    Declare_Internal_Member(native_LoadIndexBuffer);
    Declare_Internal_Member(native_LoadDefault2DSampler);
    Declare_Internal_Member(native_LoadDefault3DSampler);
    Declare_Internal_Member(native_SetFrameBufferTarget);
    Declare_Internal_Member(native_Draw);
    Declare_Internal_Member(native_DrawBare);
    Declare_Internal_Member(native_ReverseCullMode);
    Declare_Internal_Member(native_SetCullMode);
    Declare_Internal_Member(native_GetCullMode);
    Declare_Internal_Member(native_EnableBlending);
    Declare_Internal_Member(native_BlendFunction);
    Declare_Internal_Member(native_ClearDepthBuffer);
    Declare_Internal_Member(native_EnableDepthTest);
    Declare_Internal_Member(native_DepthWriteEnable);
    Declare_Internal_Member(native_DepthFunction);
    Declare_Internal_Member(native_GetDepthFunction);
    Declare_Internal_Member(native_ClearColorBuffer);
    Declare_Internal_Member(native_ColorWriteEnable);
    Declare_Internal_Member(native_ClearStencilBuffer);
    Declare_Internal_Member(native_EnableStencilTest);
    Declare_Internal_Member(native_EnableTwoSidedStencil);
    Declare_Internal_Member(native_StencilWriteEnable);
    Declare_Internal_Member(native_StencilFunction);
    Declare_Internal_Member(native_StencilFunctionCCW);
    Declare_Internal_Member(native_StencilOp);
    Declare_Internal_Member(native_StencilOpCCW);
    Declare_Internal_Member(native_SetPointSize);
    Declare_Internal_Member(native_GetInput);
    Declare_Internal_Member(native_GetCurrentDisplayMode);
    Declare_Internal_Member(native_IsFullscreen);
    Declare_Internal_Member(native_GetMouseOver);
    Declare_Internal_Member(native_GetLocalMousePos);
    Declare_Internal_Member(native_SetLocalMousePos);

protected:
    XRect  curRect;

    Vect2  Size;

    GSDepthTest curDepthFunction;

    List<Window*>  WindowList;

    Input  *userInput;

    //manual coordinate generation
    VBData *vbd;

    //matrix stack
    List<Matrix> MatrixStack;
    int curMatrix;

    //font info
    XenFont *curFont;
    List<XenFont*> FontList;
    BOOL bNormalSet;
    BOOL bColorSet;
    DWORD curFontColor;
    BitList TexCoordSetList;

    //3D sprite
    VertexBuffer *vb3DSpriteBuffer;

    //other
    GSCullMode curSide;
    DISPLAYMODE curDisplayMode;
    BOOL  bFullScreen;
    float globaltransparency;
    BOOL  bReverseCullMode;

    VertexBuffer *vbDefaultStorage;
    BOOL bFastUnbufferedRender;
    DWORD dwCurPointVert;
    DWORD dwCurTexVert;
    DWORD dwCurColorVert;
    DWORD dwCurNormVert;

    SamplerState *default3DSampler, *default2DSampler;

    BOOL bRebuildRenderTargets;

    Effect *curEffect;
    Effect *curProcessingEffect;

    virtual void ResetViewMatrix()=0;
};


//-----------------------------------------
//main extern
BASE_EXPORT extern GraphicsSystem *GS;



//-----------------------------------------
//C inline refs
inline TSTR  GetGraphicsDeviceName()                            {return GS->GetDeviceName();}

inline void  MatrixPush()                                       {GS->MatrixPush();}
inline void  MatrixPop()                                        {GS->MatrixPop();}
inline void  MatrixSet(const Matrix &m)                         {GS->MatrixSet(m);}
inline void  MatrixGet(Matrix &m)                               {GS->MatrixGet(m);}
inline void  MatrixMultiply(const Matrix &m)                    {GS->MatrixMultiply(m);}
inline void  MatrixRotate(float x, float y, float z, float a)   {GS->MatrixRotate(x, y, z, a);}  //axis angle
inline void  MatrixRotate(const AxisAngle &aa)                  {GS->MatrixRotate(aa);}
inline void  MatrixRotate(const Quat &q)                        {GS->MatrixRotate(q);}
inline void  MatrixTranslate(float x, float y, float z)         {GS->MatrixTranslate(x, y, z);}
inline void  MatrixTranslate(const Vect &pos)                   {GS->MatrixTranslate(pos);}
inline void  MatrixTranslate(const Vect2 &pos2)                 {GS->MatrixTranslate(pos2);}
inline void  MatrixScale(Vect &scale)                           {GS->MatrixScale(scale);}
inline void  MatrixScale(float x, float y, float z)             {GS->MatrixScale(x, y, z);}
inline void  MatrixInverse()                                    {GS->MatrixInverse();}
inline void  MatrixIdentity()                                   {GS->MatrixIdentity();}

inline Effect* GetActiveEffect()                                {return GS->GetActiveEffect();}

inline HardwareLight* CreateHardwareLight(DWORD dwType=0)       {return GS->CreateHardwareLight(dwType);}
inline void  EnableHardwareLighting(BOOL bEnable)               {GS->EnableHardwareLighting(bEnable);}

inline void  SetFogType(int mode)                               {GS->SetFogType(mode);}
inline void  SetFogColor(DWORD dwRGB)                           {GS->SetFogColor(dwRGB);}
inline void  SetFogDensity(float density)                       {GS->SetFogDensity(density);}
inline void  SetFogStart(float start)                           {GS->SetFogStart(start);}
inline void  SetFogEnd(float end)                               {GS->SetFogEnd(end);}

inline void  SetMaterialColor(const Color4 &rgba)               {GS->SetMaterialColor(rgba);}
inline void  SetMaterialSpecular(const Color4 &rgb)             {GS->SetMaterialSpecular(rgb);}
inline void  SetMaterialIllumination(const Color4 &rgb)         {GS->SetMaterialIllumination(rgb);}
inline void  SetMaterialColor(DWORD dwRGBA)                     {GS->SetMaterialColor(dwRGBA);}
inline void  SetMaterialSpecular(DWORD dwRGB)                   {GS->SetMaterialSpecular(dwRGB);}
inline void  SetMaterialIllumination(DWORD dwRGB)               {GS->SetMaterialIllumination(dwRGB);}
inline void  SetMaterialShininess(float shininess)              {GS->SetMaterialShininess(shininess);}

inline Texture* CreateTexture(unsigned int width, unsigned int height, DWORD dwColorFormat, void *lpData=NULL, BOOL bGenMipMaps=1, BOOL bStatic=1)
    {return GS->CreateTexture(width, height, dwColorFormat, lpData, bGenMipMaps, bStatic);}

inline Texture3D* Create3DTexture(unsigned int width, unsigned int height, unsigned int depth, DWORD dwColorFormat, void *lpData=NULL, BOOL bBuildMipMaps=1)
    {return GS->Create3DTexture(width, height, depth, dwColorFormat, lpData, bBuildMipMaps);}

inline CubeTexture* CreateCubeTexture(unsigned int width, DWORD dwColorFormat=NULL, BOOL bBuildMipMaps=TRUE, BOOL bStatic=TRUE)
    {return GS->CreateCubeTexture(width, dwColorFormat, bBuildMipMaps, bStatic);}

inline ZStencilBuffer* CreateZStencilBuffer(unsigned int width, unsigned int height)
    {return GS->CreateZStencilBuffer(width, height);}

inline Texture* CreateFrameBuffer(unsigned int width, unsigned int height, DWORD dwColorFormat, BOOL bGenMipMaps=0)
    {return GS->CreateFrameBuffer(width, height, dwColorFormat, bGenMipMaps);}

inline CubeTexture* CreateCubeFrameBuffer(unsigned int width, DWORD dwColorFormat, BOOL bGenMipMaps=0)
    {return GS->CreateCubeFrameBuffer(width, dwColorFormat, bGenMipMaps);}

inline SamplerState* CreateSamplerState(SamplerInfo &info)          {return GS->CreateSamplerState(info);}

inline void  EnableTexturing(BOOL bEnable)                          {GS->EnableTexturing(bEnable);}

inline void  SetCurFont(TSTR lpName)                                {GS->SetCurFont(lpName);}
inline void  SetCurFont(XenFont *font)                              {GS->SetCurFont(font);}
inline XenFont* GetCurFont()                                        {return GS->GetCurFont();}
inline void  SetFontColor(DWORD color)                              {GS->SetFontColor(color);}
inline void  SetFontColor(Color4 color)                             {GS->SetFontColor(color);}
inline void  DrawText(int x, int y, int cx, int cy, BOOL bWrapWords, TSTR lpText) {GS->DrawText(x, y, cx, cy, bWrapWords, lpText);}
inline void  DrawTextCenter(int x, int y, TSTR lpText)              {GS->DrawTextCenter(x, y, lpText);}
inline XenFont* GetFont(TSTR lpName)                                {return GS->GetFont(lpName);}

inline Shader* CreateVertexShader(CTSTR lpShader)                   {return GS->CreateVertexShader(lpShader);}
inline Shader* CreatePixelShader(CTSTR lpShader)                    {return GS->CreatePixelShader(lpShader);}
inline Shader* CreateVertexShaderFromFile(CTSTR lpFileName)         {return GS->CreateVertexShaderFromFile(lpFileName);}
inline Shader* CreatePixelShaderFromFile(CTSTR lpFileName)          {return GS->CreatePixelShaderFromFile(lpFileName);}


inline VertexBuffer* CreateVertexBuffer(VBData *vbData, BOOL bStatic=1)     {return GS->CreateVertexBuffer(vbData, bStatic);}
inline VertexBuffer* CloneVertexBuffer(VertexBuffer *vb, BOOL bStatic=1)    {return GS->CloneVertexBuffer(vb, bStatic);}

inline IndexBuffer* CreateIndexBuffer(GSIndexType indexType, void *indices, DWORD dwNum, BOOL bStatic=1) {return GS->CreateIndexBuffer(indexType, indices, dwNum, bStatic);}
inline IndexBuffer* CloneIndexBuffer(IndexBuffer *ib, BOOL bStatic=1) {return GS->CloneIndexBuffer(ib, bStatic);}

inline void  LoadVertexBuffer(VertexBuffer* vb)                 {GS->LoadVertexBuffer(vb);}
inline void  LoadTexture(BaseTexture *texture, int idTexture=0) {GS->LoadTexture(texture, idTexture);}
inline void  LoadSamplerState(SamplerState *sampler, int idSampler=0) {GS->LoadSamplerState(sampler, idSampler);}
inline void  LoadIndexBuffer(IndexBuffer *ib)                   {GS->LoadIndexBuffer(ib);}
inline void  LoadVertexShader(Shader *vShader)                  {GS->LoadVertexShader(vShader);}
inline void  LoadPixelShader(Shader *pShader)                   {GS->LoadPixelShader(pShader);}

inline  void  LoadDefault2DSampler(int idSampler=0) {GS->LoadDefault2DSampler(idSampler);}
inline  void  LoadDefault3DSampler(int idSampler=0) {GS->LoadDefault3DSampler(idSampler);}

inline Shader* GetCurrentPixelShader()                          {return GS->GetCurrentPixelShader();}
inline Shader* GetCurrentVertexShader()                         {return GS->GetCurrentVertexShader();}

inline void  SetFrameBufferTarget(BaseTexture *texture, DWORD side=0) {GS->SetFrameBufferTarget(texture, side);}
inline void  SetZStencilBufferTarget(ZStencilBuffer *buffer)    {GS->SetZStencilBufferTarget(buffer);}


inline void  BeginScene()                                       {GS->BeginScene();}
inline void  Draw(GSDrawMode drawMode, int vertexOffset=0, DWORD StartVert=0, DWORD nVerts=0) {GS->Draw(drawMode, vertexOffset, StartVert, nVerts);}
inline void  EndScene()                                         {GS->EndScene();}

inline void  SetCullMode(GSCullMode side)                       {GS->SetCullMode(side);}
inline GSCullMode GetCullMode()                                 {return GS->GetCullMode();}

inline void  EnableBlending(BOOL bEnable)                       {GS->EnableBlending(bEnable);}
inline void  BlendFunction(GSBlendType srcFactor, GSBlendType destFactor) {GS->BlendFunction(srcFactor, destFactor);}

inline void  ClearDepthBuffer(BOOL bFullClear=FALSE)            {GS->ClearDepthBuffer(bFullClear);}
inline void  EnableDepthTest(BOOL bEnable)                      {GS->EnableDepthTest(bEnable);}
inline void  DepthWriteEnable(BOOL bEnable)                     {GS->DepthWriteEnable(bEnable);}
inline void  DepthFunction(GSDepthTest function)                {GS->DepthFunction(function);}
inline DWORD GetDepthFunction()                                 {return GS->GetDepthFunction();}

inline void  ClearColorBuffer(BOOL bFullClear=FALSE, DWORD color=0xFF000000)
    {GS->ClearColorBuffer(bFullClear, color);}
inline void  ColorWriteEnable(BOOL bRedEnable, BOOL bGreenEnable, BOOL bBlueEnable, BOOL bAlphaEnable)
    {GS->ColorWriteEnable(bRedEnable, bGreenEnable, bBlueEnable, bAlphaEnable);}

inline void  ClearStencilBuffer(BOOL bFill=0)                   {GS->ClearStencilBuffer(bFill);}
inline void  EnableStencilTest(BOOL bEnable)                    {GS->EnableStencilTest(bEnable);}
inline void  StencilWriteEnable(BOOL bEnable)                   {GS->StencilWriteEnable(bEnable);}
inline void  StencilFunction(GSDepthTest function)              {GS->StencilFunction(function);}
inline void  StencilOp(GSStencilOp fail, GSStencilOp zfail, GSStencilOp zpass) {GS->StencilOp(fail, zfail, zpass);}

inline void  EnableClipPlane(BOOL bEnable)                      {GS->EnableClipPlane(bEnable);}
inline void  SetClipPlane(const Plane &plane)                   {GS->SetClipPlane(plane);}

inline void  SetPointSize(float size)                           {GS->SetPointSize(size);}

inline void  RenderStart()                                      {GS->RenderStart();}
inline void  RenderStartNew()                                   {GS->RenderStartNew();}
inline void  RenderStop(GSDrawMode drawMode)                    {GS->RenderStop(drawMode);}
inline VertexBuffer* RenderSave()                               {return GS->RenderSave();}
inline void  Vertex(float x, float y, float z=0)                {GS->Vertex(x, y, z);}
inline void  Vertex(const Vect &v)                              {GS->Vertex(v);}
inline void  Vertex(const Vect2 &v2)                            {GS->Vertex(v2);}
inline void  Normal(float x, float y, float z)                  {GS->Normal(x, y, z);}
inline void  Normal(const Vect &v)                              {GS->Normal(v);}
inline void  Color(DWORD dwRGBA)                                {GS->Color(dwRGBA);}
inline void  Color(const Color4 &v)                             {GS->Color(v);}
inline void  Color(float R, float G, float B, float A)          {Color4 rgba(R,G,B,A); GS->Color(rgba);}
inline void  TexCoord(float u, float v, int idTexture=0)        {GS->TexCoord(u, v, idTexture);}
inline void  TexCoord(const UVCoord &uv, int idTexture=0)       {GS->TexCoord(uv, idTexture);}

inline void  Set2DMode()                                        {GS->Set2DMode();}
inline void  Set3DMode(double fovy=60.0)                        {GS->Set3DMode(fovy);}
inline void  Perspective(double fovy, double aspect, double znear, double zfar)
    {GS->Perspective(fovy, aspect, znear, zfar);}
inline void  Ortho(double left, double right, double top, double bottom, double znear, double zfar)
    {GS->Ortho(left, right, top, bottom, znear, zfar);}
inline void  Frustum(double left, double right, double top, double bottom, double znear, double zfar)
    {GS->Frustum(left, right, top, bottom, znear, zfar);}
inline void  AdjustZ(double zfar)                               {GS->AdjustZ(zfar);}

inline void  SetViewport(int x, int y, int width, int height)   {GS->SetViewport(x, y, width, height);}
inline void  SetViewport(XRect &rect)                           {GS->SetViewport(rect.x, rect.y, rect.cx, rect.cy);}
inline void  GetViewport(XRect &rect)                           {GS->GetViewport(rect);}
inline void  SetScissorRect(XRect *pRect=NULL)                  {GS->SetScissorRect(pRect);}
inline void  ResetViewport()                                    {GS->ResetViewport();}

inline BOOL  ToggleFullScreen()                                 {return GS->ToggleFullScreen();}
inline BOOL  SetResolution(DISPLAYMODE &dm, BOOL bCenter=FALSE) {return GS->SetResolution(dm, bCenter);}
inline void  EnumDisplayInfo(List<DISPLAYMODE> &DisplayModes)   {GS->EnumDisplayInfo(DisplayModes);}


Declare_Native_Global(NativeGlobal_GS);
Declare_Native_Global(NativeGlobal_MatrixPush);
Declare_Native_Global(NativeGlobal_MatrixPop);
Declare_Native_Global(NativeGlobal_MatrixSet);
Declare_Native_Global(NativeGlobal_MatrixMultiply);
Declare_Native_Global(NativeGlobal_MatrixRotate);
Declare_Native_Global(NativeGlobal_MatrixRotate2);
Declare_Native_Global(NativeGlobal_MatrixRotate3);
Declare_Native_Global(NativeGlobal_MatrixTranslate);
Declare_Native_Global(NativeGlobal_MatrixTranslate2);
Declare_Native_Global(NativeGlobal_MatrixTranslate3);
Declare_Native_Global(NativeGlobal_MatrixScale);
Declare_Native_Global(NativeGlobal_MatrixScale2);
Declare_Native_Global(NativeGlobal_MatrixTranspose);
Declare_Native_Global(NativeGlobal_MatrixInverse);
Declare_Native_Global(NativeGlobal_MatrixIdentity);
Declare_Native_Global(NativeGlobal_EnableHardwareLighting);
Declare_Native_Global(NativeGlobal_SetMaterialColor);
Declare_Native_Global(NativeGlobal_SetMaterialSpecular);
Declare_Native_Global(NativeGlobal_SetMaterialIllumination);
Declare_Native_Global(NativeGlobal_SetMaterialColor2);
Declare_Native_Global(NativeGlobal_SetMaterialSpecular2);
Declare_Native_Global(NativeGlobal_SetMaterialIllumination2);
Declare_Native_Global(NativeGlobal_SetMaterialShininess);
Declare_Native_Global(NativeGlobal_SetMaterialGlobalTransparancy);
Declare_Native_Global(NativeGlobal_SetCurFont);
Declare_Native_Global(NativeGlobal_SetCurFont2);
Declare_Native_Global(NativeGlobal_GetFont);
Declare_Native_Global(NativeGlobal_SetFontColor);
Declare_Native_Global(NativeGlobal_SetFontColor2);
Declare_Native_Global(NativeGlobal_DrawText);
Declare_Native_Global(NativeGlobal_DrawTextCenter);
Declare_Native_Global(NativeGlobal_RenderStartNew);
Declare_Native_Global(NativeGlobal_RenderStart);
Declare_Native_Global(NativeGlobal_RenderStop);
Declare_Native_Global(NativeGlobal_RenderSave);
Declare_Native_Global(NativeGlobal_Vertex);
Declare_Native_Global(NativeGlobal_Vertex2);
Declare_Native_Global(NativeGlobal_Normal);
Declare_Native_Global(NativeGlobal_Normal2);
Declare_Native_Global(NativeGlobal_Color);
Declare_Native_Global(NativeGlobal_Color2);
Declare_Native_Global(NativeGlobal_TexCoord);
Declare_Native_Global(NativeGlobal_TexCoord2);
Declare_Native_Global(NativeGlobal_LoadVertexBuffer);
Declare_Native_Global(NativeGlobal_LoadTexture);
Declare_Native_Global(NativeGlobal_LoadSampler);
Declare_Native_Global(NativeGlobal_LoadIndexBuffer);
Declare_Native_Global(NativeGlobal_LoadDefault2DSampler);
Declare_Native_Global(NativeGlobal_LoadDefault3DSampler);
Declare_Native_Global(NativeGlobal_SetFrameBufferTarget);
Declare_Native_Global(NativeGlobal_Draw);
Declare_Native_Global(NativeGlobal_DrawBare);
Declare_Native_Global(NativeGlobal_ReverseCullMode);
Declare_Native_Global(NativeGlobal_SetCullMode);
Declare_Native_Global(NativeGlobal_GetCullMode);
Declare_Native_Global(NativeGlobal_EnableBlending);
Declare_Native_Global(NativeGlobal_BlendFunction);
Declare_Native_Global(NativeGlobal_ClearDepthBuffer);
Declare_Native_Global(NativeGlobal_EnableDepthTest);
Declare_Native_Global(NativeGlobal_DepthWriteEnable);
Declare_Native_Global(NativeGlobal_DepthFunction);
Declare_Native_Global(NativeGlobal_GetDepthFunction);
Declare_Native_Global(NativeGlobal_ClearColorBuffer);
Declare_Native_Global(NativeGlobal_ColorWriteEnable);
Declare_Native_Global(NativeGlobal_ClearStencilBuffer);
Declare_Native_Global(NativeGlobal_EnableStencilTest);
Declare_Native_Global(NativeGlobal_EnableTwoSidedStencil);
Declare_Native_Global(NativeGlobal_StencilWriteEnable);
Declare_Native_Global(NativeGlobal_StencilFunction);
Declare_Native_Global(NativeGlobal_StencilFunctionCCW);
Declare_Native_Global(NativeGlobal_StencilOp);
Declare_Native_Global(NativeGlobal_StencilOpCCW);
Declare_Native_Global(NativeGlobal_SetPointSize);
Declare_Native_Global(NativeGlobal_GetCurrentDisplayMode);
Declare_Native_Global(NativeGlobal_IsFullscreen);
Declare_Native_Global(NativeGlobal_GetMouseOver);
Declare_Native_Global(NativeGlobal_GetLocalMousePos);
Declare_Native_Global(NativeGlobal_SetLocalMousePos);

#endif

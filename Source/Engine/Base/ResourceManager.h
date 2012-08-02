/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ResourceManager.h:  Texture/Shader/Sound Resource Manager

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

#ifndef RESOURCEMANAGER_HEADER
#define RESOURCEMANAGER_HEADER



/*=========================================================
    Texture Manager
==========================================================*/

struct TextureResource
{
    BaseTexture *texture;
    String name;
    DWORD refs;
};

struct MaterialResource
{
    Material *material;
    String name;
    DWORD refs;
};

struct MeshResource
{
    Mesh *mesh;
    String name;
    DWORD refs;
};

//----------------------------
// Texture Manager
class BASE_EXPORT ResourceManager
{
public:
    ResourceManager()            {}
    virtual ~ResourceManager();

    virtual void  Clear();

    //----------------------------------------------------------
    // Textures
    virtual Texture* GetTexture(CTSTR lpName, BOOL bGenMipMaps=1);
    virtual CubeTexture *GetCubeTexture(CTSTR lpName, BOOL bGenMipMaps=1);
    virtual Texture3D *Get3DTexture(CTSTR lpName, BOOL bGenMipMaps=1);

    virtual String GetTextureName(BaseTexture *texture);

    virtual DWORD AddTextureRef(BaseTexture *texture); 
    virtual DWORD ReleaseTexture(CTSTR lpName);
    virtual DWORD ReleaseTexture(BaseTexture *texture);
    virtual void  ForceFreeTexture(CTSTR lpName);
    virtual void  ForceFreeTexture(BaseTexture *texture);

    //----------------------------------------------------------
    // Materials
    virtual Material*       GetMaterial(CTSTR lpName, BOOL bAddRef=TRUE);
    virtual String          GetMaterialName(Material *material);
    virtual Material*       UsingMaterial(CTSTR lpName);

    virtual DWORD           AddMaterialRef(Material *material);
    virtual DWORD           ReleaseMaterial(CTSTR lpName);
    virtual DWORD           ReleaseMaterial(Material *material);
    virtual void            ForceFreeMaterial(CTSTR lpName);
    virtual void            ForceFreeMaterial(Material *material);

    //----------------------------------------------------------
    // Meshes
    virtual Mesh*           GetMesh(CTSTR lpName, BOOL bAddRef=TRUE);

    virtual DWORD           AddMeshRef(Mesh *mesh);
    virtual DWORD           ReleaseMesh(CTSTR lpName);
    virtual DWORD           ReleaseMesh(Mesh *mesh);
    virtual void            ForceFreeMesh(CTSTR lpName);
    virtual void            ForceFreeMesh(Mesh *mesh);

    //----------------------------------------------------------
    // Shaders
    virtual Shader*         GetPixelShader(CTSTR lpName);
    virtual Shader*         GetVertexShader(CTSTR lpName);

    virtual Effect*         GetEffect(CTSTR lpName);
    virtual Effect*         GetInternalEffect(CTSTR lpName);
    virtual Effect*         GetEffect(int i)    {return EffectShaders[i];}
    virtual int             NumEffects()        {return EffectShaders.Num();}

    virtual void            FreeEffect(CTSTR lpName);
    virtual void            FreeInternalEffect(CTSTR lpName);
    virtual void            FreeEffect(Effect *effect);

    //----------------------------------------------------------
    // Sounds
    virtual Sound*  NewSound(CTSTR lpName, BOOL b3DSound, BOOL bSelfDestruct=FALSE);
    virtual LPBYTE  LoadSoundData(CTSTR lpName);

protected:
    List<MaterialResource>  MaterialList;

    List<TextureResource>   TextureList;

    List<MeshResource>      MeshList;

    List<Shader*>           PixelShaders;
    StringList              PixelShaderNames;
    List<Shader*>           VertexShaders;
    StringList              VertexShaderNames;
    List<Effect*>           EffectShaders;
    StringList              EffectShaderNames;
    List<Effect*>           InternalEffectShaders;
    StringList              InternalEffectShaderNames;

    List<LPBYTE>            SoundList;
    StringList              SoundNames;
};


//<Script module="Base" globaldecs="ResourceManagement.xscript">
Declare_Native_Global(NativeGlobal_GetMaterial);
Declare_Native_Global(NativeGlobal_GetTexture);
Declare_Native_Global(NativeGlobal_GetCubeTexture);
Declare_Native_Global(NativeGlobal_AddTextureRef);
Declare_Native_Global(NativeGlobal_ReleaseMaterial);
Declare_Native_Global(NativeGlobal_ReleaseMaterial_2);
Declare_Native_Global(NativeGlobal_ReleaseTexture);
Declare_Native_Global(NativeGlobal_ReleaseTexture_2);
Declare_Native_Global(NativeGlobal_GetPixelShader);
Declare_Native_Global(NativeGlobal_GetVertexShader);
Declare_Native_Global(NativeGlobal_GetEffect);
Declare_Native_Global(NativeGlobal_GetEffect_2);
Declare_Native_Global(NativeGlobal_NewSound);
//</Script>


//-----------------------------------------
//main extern
BASE_EXPORT extern ResourceManager *RM;




inline Material* GetMaterial(CTSTR lpName)                                  {return RM->GetMaterial(lpName);}

inline Texture* GetTexture(CTSTR lpName, BOOL bGenMipMaps=1)                {return RM->GetTexture(lpName, bGenMipMaps);}
inline CubeTexture* GetCubeTexture(CTSTR lpName, BOOL bGenMipMaps=1)        {return RM->GetCubeTexture(lpName);}
inline DWORD    AddTextureRef(BaseTexture *texture)                         {return RM->AddTextureRef(texture);}

inline DWORD ReleaseMaterial(Material *material)                            {return RM->ReleaseMaterial(material);}
inline DWORD ReleaseMaterial(CTSTR lpName)                                  {return RM->ReleaseMaterial(lpName);}
inline DWORD ReleaseTexture(BaseTexture *texture)                           {return RM->ReleaseTexture(texture);}
inline DWORD ReleaseTexture(CTSTR lpName)                                   {return RM->ReleaseTexture(lpName);}

inline Shader*       GetPixelShader(CTSTR lpName)                           {return RM->GetPixelShader(lpName);}
inline Shader*       GetVertexShader(CTSTR lpName)                          {return RM->GetVertexShader(lpName);}
inline Effect*       GetEffect(TSTR lpName)                                 {return RM->GetEffect(lpName);}
inline Effect*       GetEffect(int i)                                       {return RM->GetEffect(i);}
inline int           NumEffects()                                           {return RM->NumEffects();}

inline Sound* NewSound(CTSTR lpName, BOOL b3DSound, BOOL bSelfDestruct=0)   {return RM->NewSound(lpName, b3DSound, bSelfDestruct);}


#endif

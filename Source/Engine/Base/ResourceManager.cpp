/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ResourceManager.cpp:  Texture/Shader/Sound Resource Manager

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


ResourceManager *RM = NULL;


/*=========================================================
    Texture Manager
==========================================================*/

ResourceManager::~ResourceManager()
{
    traceIn(ResourceManager::~ResourceManager);

    Clear();

    traceOut;
}

void ResourceManager::Clear()
{
    traceIn(ResourceManager::Clear);

    //--------------------------------------
    // Materials

    for(DWORD i=0; i<MaterialList.Num(); i++)
    {
        MaterialResource &res = MaterialList[i];
        delete res.material;
        res.name.Clear();
    }

    MaterialList.Clear();

    //--------------------------------------
    // Meshes

    for(DWORD i=0; i<MeshList.Num(); i++)
    {
        MeshResource &res = MeshList[i];
        delete res.mesh;
        res.name.Clear();
    }

    MeshList.Clear();

    //--------------------------------------
    // Textures

    for(DWORD i=0; i<TextureList.Num(); i++)
    {
        TextureResource &res = TextureList[i];
        delete res.texture;
        res.name.Clear();
    }

    TextureList.Clear();

    //--------------------------------------
    // Shaders

    for(int i=0; i<VertexShaders.Num(); i++)
        delete VertexShaders[i];

    for(int i=0; i<PixelShaders.Num(); i++)
        delete PixelShaders[i];

    VertexShaders.Clear();
    VertexShaderNames.Clear();
    PixelShaders.Clear();
    PixelShaderNames.Clear();

    for(int i=0; i<EffectShaders.Num(); i++)
        delete EffectShaders[i];
    EffectShaders.Clear();
    EffectShaderNames.Clear();

    for(int i=0; i<InternalEffectShaders.Num(); i++)
        delete InternalEffectShaders[i];
    InternalEffectShaders.Clear();
    InternalEffectShaderNames.Clear();

    //--------------------------------------
    // Sounds

    for(int i=0; i<SoundList.Num(); i++)
        Free(SoundList[i]);

    SoundList.Clear();
    SoundNames.Clear();

    //Log(TEXT("Resource Manager: Freed all resources."));

    traceOut;
}


Texture *ResourceManager::GetTexture(CTSTR lpName, BOOL bGenMipMaps)
{
    traceInFast(ResourceManager::GetTexture);

    assert(lpName);

    for(DWORD i=0; i<TextureList.Num(); i++)
    {
        TextureResource &res = TextureList[i];
        if(res.name.CompareI(lpName))
        {
            if(res.texture->IsOf(GetClass(Texture)))
            {
                ++res.refs;
                return (Texture*)res.texture;
            }
            else
                return NULL;
        }
    }

    //----------------------------------------

    String path;
    if(!Engine::ConvertResourceName(lpName, TEXT("textures"), path))
        return NULL;

    //----------------------------------------

    Texture *texOut = GS->CreateTextureFromFile(path, bGenMipMaps);

    if(!texOut)
    {
        AppWarning(TEXT("Could not create texture '%s'"), path.Array());
        return NULL;
    }

    TextureResource &res = *TextureList.CreateNew();
    res.texture = texOut;
    res.name = lpName;
    res.refs = 1;

    if(texOut)
        texOut->bGenMipMaps = bGenMipMaps;

    return texOut;

    traceOutFast;
}

CubeTexture *ResourceManager::GetCubeTexture(CTSTR lpName, BOOL bGenMipMaps)
{
    traceInFast(ResourceManager::GetCubeTexture);

    assert(lpName);

    for(DWORD i=0; i<TextureList.Num(); i++)
    {
        TextureResource &res = TextureList[i];
        if(res.name.CompareI(lpName))
        {
            if(res.texture->IsOf(GetClass(CubeTexture)))
            {
                ++res.refs;
                return (CubeTexture*)res.texture;
            }
            else
                return NULL;
        }
    }

    //----------------------------------------

    String path;
    if(!Engine::ConvertResourceName(lpName, TEXT("textures"), path))
        return NULL;

    //----------------------------------------

    CubeTexture *texOut = GS->CreateCubeTextureFromFile(path, bGenMipMaps);

    TextureResource &res = *TextureList.CreateNew();
    res.texture = texOut;
    res.name = lpName;
    res.refs = 1;

    if(texOut)
        texOut->bGenMipMaps = bGenMipMaps;

    return texOut;

    traceOutFast;
}

Texture3D *ResourceManager::Get3DTexture(CTSTR lpName, BOOL bGenMipMaps)
{
    traceInFast(ResourceManager::Get3DTexture);

    assert(lpName);

    for(DWORD i=0; i<TextureList.Num(); i++)
    {
        TextureResource &res = TextureList[i];
        if(res.name.CompareI(lpName))
        {
            if(res.texture->IsOf(GetClass(Texture3D)))
            {
                ++res.refs;
                return (Texture3D*)res.texture;
            }
            else
                return NULL;
        }
    }

    //----------------------------------------

    String path;
    if(!Engine::ConvertResourceName(lpName, TEXT("textures"), path))
        return NULL;

    //----------------------------------------

    Texture3D *texOut = GS->Create3DTextureFromFile(path, bGenMipMaps);

    TextureResource &res = *TextureList.CreateNew();
    res.texture = texOut;
    res.name = lpName;
    res.refs = 1;

    return texOut;

    traceOutFast;
}


String ResourceManager::GetTextureName(BaseTexture *texture)
{
    traceInFast(ResourceManager::GetTextureName);

    assert(texture);

    DWORD i;

    for(i=0; i<TextureList.Num(); i++)
    {
        if(TextureList[i].texture == texture)
            return TextureList[i].name;
    }

    return String();

    traceOutFast;
}


DWORD ResourceManager::AddTextureRef(BaseTexture *texture)
{
    traceInFast(ResourceManager::AddTextureRef);

    assert(texture);

    DWORD i;

    for(i=0; i<TextureList.Num(); i++)
    {
        if(TextureList[i].texture == texture)
            break;
    }

    assert(i < TextureList.Num());
    if(i == TextureList.Num())
        return INVALID;

    DWORD ref = ++TextureList[i].refs;

    return ref;

    traceOutFast;
}


DWORD ResourceManager::ReleaseTexture(CTSTR lpName)
{
    traceInFast(ResourceManager::ReleaseTexture);

    assert(lpName);

    BaseTexture *texture = NULL;
    DWORD i;

    for(i=0; i<TextureList.Num(); i++)
    {
        if(TextureList[i].name.CompareI(lpName))
        {
            texture = TextureList[i].texture;
            break;
        }
    }

    assert(i < TextureList.Num());
    if(i == TextureList.Num())
        return INVALID;

    DWORD ref = --TextureList[i].refs;

    if(!ref)
    {
        delete texture;
        TextureList[i].name.Clear();
        TextureList.Remove(i);
    }

    return ref;

    traceOutFast;
}

DWORD ResourceManager::ReleaseTexture(BaseTexture *texture)
{
    traceInFast(ResourceManager::ReleaseTexture);

    if(!texture) return 0;

    DWORD i;

    for(i=0; i<TextureList.Num(); i++)
    {
        if(TextureList[i].texture == texture)
            break;
    }

    assert(i < TextureList.Num());
    if(i == TextureList.Num())
        return INVALID;

    DWORD ref = --TextureList[i].refs;

    if(!ref)
    {
        delete texture;
        TextureList[i].name.Clear();
        TextureList.Remove(i);
    }

    return ref;

    traceOutFast;
}

void ResourceManager::ForceFreeTexture(CTSTR lpName)
{
    traceInFast(ResourceManager::ForceFreeTexture);

    assert(lpName);

    BaseTexture *texture = NULL;
    DWORD i;

    for(i=0; i<TextureList.Num(); i++)
    {
        if(TextureList[i].name.CompareI(lpName))
        {
            texture = TextureList[i].texture;
            break;
        }
    }

    assert(i < TextureList.Num());
    if(i == TextureList.Num())
        return;

    delete texture;
    TextureList[i].name.Clear();
    TextureList.Remove(i);

    traceOutFast;
}

void ResourceManager::ForceFreeTexture(BaseTexture *texture)
{
    traceInFast(ResourceManager::ForceFreeTexture);

    if(!texture) return;

    DWORD i;

    for(i=0; i<TextureList.Num(); i++)
    {
        if(TextureList[i].texture == texture)
            break;
    }

    assert(i < TextureList.Num());
    if(i == TextureList.Num())
        return;

    delete texture;
    TextureList[i].name.Clear();
    TextureList.Remove(i);

    traceOutFast;
}


Material* ResourceManager::GetMaterial(CTSTR lpName, BOOL bAddRef)
{
    traceInFast(ResourceManager::GetMaterial);

    assert(lpName);

    for(DWORD i=0; i<MaterialList.Num(); i++)
    {
        MaterialResource &res = MaterialList[i];
        if(res.name.CompareI(lpName))
        {
            if(bAddRef)
                ++res.refs;
            return res.material;
        }
    }

    //----------------------------------------

    String path;
    if(!Engine::ConvertResourceName(lpName, TEXT("materials"), path))
        return NULL;

    //----------------------------------------

    Material *newMaterial = CreateObject(Material);
    newMaterial->LoadFromFile(path);

    MaterialResource &res = *MaterialList.CreateNew();
    res.material = newMaterial;
    res.name = lpName;
    res.refs = 1;

    return newMaterial;

    traceOutFast;
}


Material* ResourceManager::UsingMaterial(CTSTR lpName)
{
    traceInFast(ResourceManager::UsingMaterial);

    assert(lpName);

    for(DWORD i=0; i<MaterialList.Num(); i++)
    {
        if(MaterialList[i].name.CompareI(lpName))
            return MaterialList[i].material;
    }

    return NULL;

    traceOutFast;
}


String ResourceManager::GetMaterialName(Material *material)
{
    traceInFast(ResourceManager::GetMaterialName);

    if(!material)
        return String();

    DWORD i;

    for(i=0; i<MaterialList.Num(); i++)
    {
        if(MaterialList[i].material == material)
            break;
    }

    assert(i < MaterialList.Num());
    if(i == MaterialList.Num())
        return String();

    return MaterialList[i].name;

    traceOutFast;
}


DWORD ResourceManager::AddMaterialRef(Material *material)
{
    traceInFast(ResourceManager::AddMaterialRef);

    assert(material);

    DWORD i;

    for(i=0; i<MaterialList.Num(); i++)
    {
        if(MaterialList[i].material == material)
            break;
    }

    assert(i < MaterialList.Num());
    if(i == MaterialList.Num())
        return INVALID;

    return ++MaterialList[i].refs;

    traceOutFast;
}


DWORD ResourceManager::ReleaseMaterial(CTSTR lpName)
{
    traceInFast(ResourceManager::ReleaseMaterial);

    assert(lpName);

    Material *material = NULL;
    DWORD i;

    for(i=0; i<MaterialList.Num(); i++)
    {
        if(MaterialList[i].name.CompareI(lpName))
        {
            material = MaterialList[i].material;
            break;
        }
    }

    assert(i < MaterialList.Num());
    if(i == MaterialList.Num())
        return INVALID;

    DWORD ref = --MaterialList[i].refs;

    if(!ref)
    {
        delete material;
        MaterialList[i].name.Clear();
        MaterialList.Remove(i);
    }

    return ref;

    traceOutFast;
}

DWORD ResourceManager::ReleaseMaterial(Material *material)
{
    traceInFast(ResourceManager::ReleaseMaterial);

    if(!material) return 0;

    DWORD i;

    for(i=0; i<MaterialList.Num(); i++)
    {
        if(MaterialList[i].material == material)
            break;
    }

    assert(i < MaterialList.Num());
    if(i == MaterialList.Num())
        return INVALID;

    DWORD ref = --MaterialList[i].refs;

    if(!ref)
    {
        delete material;
        MaterialList[i].name.Clear();
        MaterialList.Remove(i);
    }

    return ref;

    traceOutFast;
}

void ResourceManager::ForceFreeMaterial(CTSTR lpName)
{
    traceInFast(ResourceManager::ForceFreeMaterial);

    assert(lpName);

    Material *material = NULL;
    DWORD i;

    for(i=0; i<MaterialList.Num(); i++)
    {
        if(MaterialList[i].name.CompareI(lpName))
        {
            material = MaterialList[i].material;
            break;
        }
    }

    assert(i < MaterialList.Num());
    if(i == MaterialList.Num())
        return;

    delete material;
    MaterialList[i].name.Clear();
    MaterialList.Remove(i);

    traceOutFast;
}

void ResourceManager::ForceFreeMaterial(Material *material)
{
    traceInFast(ResourceManager::ForceFreeMaterial);

    if(!material) return;

    DWORD i;

    for(i=0; i<MaterialList.Num(); i++)
    {
        if(MaterialList[i].material == material)
            break;
    }

    assert(i < MaterialList.Num());
    if(i == MaterialList.Num())
        return;

    delete material;
    MaterialList[i].name.Clear();
    MaterialList.Remove(i);

    traceOutFast;
}


Mesh* ResourceManager::GetMesh(CTSTR lpName, BOOL bAddRef)
{
    traceInFast(ResourceManager::GetMesh);

    assert(lpName);

    for(DWORD i=0; i<MeshList.Num(); i++)
    {
        MeshResource &res = MeshList[i];
        if(res.name.CompareI(lpName))
        {
            if(bAddRef)
                ++res.refs;
            return res.mesh;
        }
    }

    //----------------------------------------

    String path;
    if(!Engine::ConvertResourceName(lpName, TEXT("models"), path))
        return NULL;

    //----------------------------------------

    String strAnimPath;
    strAnimPath << GetPathWithoutExtension(path) << TEXT(".xan");

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(strAnimPath, ofd);
    BOOL bHasAnimation = (hFind != NULL);
    if(hFind) OSFindClose(hFind);

    Mesh *newMesh = (bHasAnimation) ? new SkinMesh : new Mesh;
    newMesh->LoadMesh(lpName);
    newMesh->bHasAnimation = bHasAnimation;

    MeshResource &res = *MeshList.CreateNew();
    res.mesh = newMesh;
    res.name = lpName;
    res.refs = 1;

    return newMesh;

    traceOutFast;
}

DWORD ResourceManager::AddMeshRef(Mesh *mesh)
{
    traceInFast(ResourceManager::AddMeshRef);

    assert(mesh);

    DWORD i;

    for(i=0; i<MeshList.Num(); i++)
    {
        if(MeshList[i].mesh == mesh)
            break;
    }

    assert(i < MeshList.Num());
    if(i == MeshList.Num())
        return INVALID;

    return ++MeshList[i].refs;

    traceOutFast;
}


DWORD ResourceManager::ReleaseMesh(CTSTR lpName)
{
    traceInFast(ResourceManager::ReleaseMesh);

    assert(lpName);

    Mesh *mesh = NULL;
    DWORD i;

    for(i=0; i<MeshList.Num(); i++)
    {
        if(MeshList[i].name.CompareI(lpName))
        {
            mesh = MeshList[i].mesh;
            break;
        }
    }

    assert(i < MeshList.Num());
    if(i == MeshList.Num())
        return INVALID;

    DWORD ref = --MeshList[i].refs;

    if(!ref)
    {
        delete mesh;
        MeshList[i].name.Clear();
        MeshList.Remove(i);
    }

    return ref;

    traceOutFast;
}

DWORD ResourceManager::ReleaseMesh(Mesh *mesh)
{
    traceInFast(ResourceManager::ReleaseMesh);

    if(!mesh) return 0;

    DWORD i;

    for(i=0; i<MeshList.Num(); i++)
    {
        if(MeshList[i].mesh == mesh)
            break;
    }

    assert(i < MeshList.Num());
    if(i == MeshList.Num())
        return INVALID;

    DWORD ref = --MeshList[i].refs;

    if(!ref)
    {
        delete mesh;
        MeshList[i].name.Clear();
        MeshList.Remove(i);
    }

    return ref;

    traceOutFast;
}

void ResourceManager::ForceFreeMesh(CTSTR lpName)
{
    traceInFast(ResourceManager::ForceFreeMesh);

    assert(lpName);

    Mesh *mesh = NULL;
    DWORD i;

    for(i=0; i<MeshList.Num(); i++)
    {
        if(MeshList[i].name.CompareI(lpName))
        {
            mesh = MeshList[i].mesh;
            break;
        }
    }

    assert(i < MeshList.Num());
    if(i == MeshList.Num())
        return;

    delete mesh;
    MeshList[i].name.Clear();
    MeshList.Remove(i);

    traceOutFast;
}

void ResourceManager::ForceFreeMesh(Mesh *mesh)
{
    traceInFast(ResourceManager::ForceFreeMesh);

    if(!mesh) return;

    DWORD i;

    for(i=0; i<MeshList.Num(); i++)
    {
        if(MeshList[i].mesh == mesh)
            break;
    }

    assert(i < MeshList.Num());
    if(i == MeshList.Num())
        return;

    delete mesh;
    MeshList[i].name.Clear();
    MeshList.Remove(i);

    traceOutFast;
}



Shader* ResourceManager::GetPixelShader(CTSTR lpName)
{
    traceInFast(ResourceManager::GetPixelShader);

    assert(lpName);

    UINT val = PixelShaderNames.FindValueIndexI(lpName);
    if(val != INVALID)
        return PixelShaders[val];

    //----------------------------------------

    String path;
    if(!Engine::ConvertResourceName(lpName, TEXT("shaders"), path))
        return NULL;

    //----------------------------------------

    Shader *newPixelShader = GS->CreatePixelShaderFromFile(path);

    if(!newPixelShader)
    {
        CrashError(TEXT("Resource Manager: Could not find Pixel Shader %s"), lpName);
        return NULL;
    }

    PixelShaderNames << lpName;
    PixelShaders << newPixelShader;

    return newPixelShader;

    traceOutFast;
}

Shader* ResourceManager::GetVertexShader(CTSTR lpName)
{
    traceInFast(ResourceManager::GetVertexShader);

    assert(lpName);

    UINT val = VertexShaderNames.FindValueIndexI(lpName);
    if(val != INVALID)
        return VertexShaders[val];

    //----------------------------------------

    String path;
    if(!Engine::ConvertResourceName(lpName, TEXT("shaders"), path))
        return NULL;

    //----------------------------------------

    Shader *newVertexShader = GS->CreateVertexShaderFromFile(path);

    if(!newVertexShader)
    {
        CrashError(TEXT("Resource Manager: Could not find Vertex Shader %s"), lpName);
        return NULL;
    }

    VertexShaderNames << lpName;
    VertexShaders << newVertexShader;

    return newVertexShader;

    traceOutFast;
}

Effect* ResourceManager::GetEffect(CTSTR lpName)
{
    traceInFast(ResourceManager::GetEffect);

    assert(lpName);

    UINT val = EffectShaderNames.FindValueIndexI(lpName);
    if(val != INVALID)
        return EffectShaders[val];

    //----------------------------------------

    String path;
    if(!Engine::ConvertResourceName(lpName, TEXT("effects"), path))
        return NULL;

    //----------------------------------------

    Effect *newEffect = GS->CreateEffectFromFile(path);

    if(!newEffect)
    {
        AppWarning(TEXT("Resource Manager: Could not find Effect %s"), lpName);
        return NULL;
    }

    EffectShaderNames << lpName;
    EffectShaders << newEffect;

    return newEffect;

    traceOutFast;
}

Effect* ResourceManager::GetInternalEffect(CTSTR lpName)
{
    traceInFast(ResourceManager::GetInternalEffect);

    assert(lpName);

    UINT val = InternalEffectShaderNames.FindValueIndexI(lpName);
    if(val != INVALID)
        return InternalEffectShaders[val];

    //----------------------------------------

    String path;
    if(!Engine::ConvertResourceName(lpName, TEXT("internalEffects"), path))
        return NULL;

    //----------------------------------------

    Effect *newEffect = GS->CreateEffectFromFile(path);

    if(!newEffect)
    {
        AppWarning(TEXT("Resource Manager: Could not find Internal Effect %s"), lpName);
        return NULL;
    }

    InternalEffectShaderNames << lpName;
    InternalEffectShaders << newEffect;

    return newEffect;

    traceOutFast;
}

void ResourceManager::FreeEffect(CTSTR lpName)
{
    traceInFast(ResourceManager::FreeEffect);

    assert(lpName);

    UINT val = EffectShaderNames.FindValueIndexI(lpName);
    if(val != INVALID)
    {
        delete EffectShaders[val];

        EffectShaders.Remove(val);
        EffectShaderNames.Remove(val);
    }

    traceOutFast;
}

void ResourceManager::FreeInternalEffect(CTSTR lpName)
{
    traceInFast(ResourceManager::FreeInternalEffect);

    assert(lpName);

    UINT val = InternalEffectShaderNames.FindValueIndexI(lpName);
    if(val != INVALID)
    {
        delete InternalEffectShaders[val];

        InternalEffectShaders.Remove(val);
        InternalEffectShaderNames.Remove(val);
    }

    traceOutFast;
}

void ResourceManager::FreeEffect(Effect *effect)
{
    traceInFast(ResourceManager::FreeEffect);

    assert(effect);

    UINT val = EffectShaders.FindValueIndex(effect);
    if(val != INVALID)
    {
        delete EffectShaders[val];

        EffectShaders.Remove(val);
        EffectShaderNames.Remove(val);
    }

    traceOutFast;
}


LPBYTE ResourceManager::LoadSoundData(CTSTR lpName)
{
    traceIn(ResourceManager::LoadSoundData);

    XFile file;
    LPBYTE lpData = NULL;

    //----------------------------------------

    String path;
    if(!Engine::ConvertResourceName(lpName, TEXT("sounds"), path))
        return NULL;

    //----------------------------------------

    if(file.Open(path, XFILE_READ, XFILE_OPENEXISTING))
    {
        DWORD dwFileSize = file.GetFileSize();
        lpData = (LPBYTE)Allocate(dwFileSize);
        file.Read(lpData, dwFileSize);

        SoundList  << lpData;
        SoundNames << lpName;

        file.Close();
    }
    else
        AppWarning(TEXT("Resource Manager: Could not create sound from file \"%s\""), lpName);

    return lpData;

    traceOut;
}

Sound *ResourceManager::NewSound(CTSTR lpName, BOOL b3DSound, BOOL bSelfDestruct)
{
    traceInFast(ResourceManager::NewSound);

    assert(lpName);
    LPBYTE lpData = NULL;

    UINT val = SoundNames.FindValueIndexI(lpName);
    if(val != INVALID)
        lpData = SoundList[val];

    if(!lpData)
        lpData = LoadSoundData(lpName);

    if(!lpData)
    {
        AppWarning(TEXT("Resource Manager: Could not create sound"));
        return NULL;
    }

    Sound *sound = SS->CreateSound(lpData, b3DSound, bSelfDestruct);
    if(sound)
    {
        sound->soundName = lpName;
        sound->b3DSound = b3DSound;
    }

    return sound;

    traceOutFast;
}

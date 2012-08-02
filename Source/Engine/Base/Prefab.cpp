/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Prefab.h

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


DefineClass(Prefab);
DefineClass(AnimatedPrefab);


Prefab::Prefab()
{
    bUseLightmapping = TRUE;
    bStaticGeometry = TRUE;
}


void Prefab::Init()
{
    traceIn(Prefab::Init);

    if(prefabName.IsEmpty())
        return;

    String prefabPath;
    if(!Engine::ConvertResourceName(prefabName, TEXT("prefabs"), prefabPath))
    {
        SafeDestroy();
        return;
    }

    ConfigFile prefabConfig;
    if(!prefabConfig.Open(prefabPath))
    {
        AppWarning(TEXT("Could not load prefab '%s'"), prefabPath.Array());
        SafeDestroy();
        return;
    }

    String strMeshName = prefabConfig.GetString(TEXT("Prefab"), TEXT("ModelFile"));
    if(strMeshName.IsEmpty())
    {
        AppWarning(TEXT("Bad mesh for prefab '%s'"), prefabPath.Array());
        SafeDestroy();
        return;
    }

    StringList MaterialNames;
    prefabConfig.GetStringList(TEXT("Prefab"), TEXT("Materials"), MaterialNames);

    MaterialList.SetSize(MaterialNames.Num());

    for(int i=0; i<MaterialNames.Num(); i++)
        MaterialList[i] = GetMaterial(MaterialNames[i]);

    bRenderable = TRUE;

    SetMesh(strMeshName);

    Super::Init();

    phyShape = mesh->GetShape();
    phyObject = physics->CreateStaticObject(phyShape, GetWorldPos(), GetWorldRot());
    phyObject->SetEntityOwner(this);
    phyObject->EnableCollisionCallback(TRUE);

    wireframeColor = 0xFF007F00;

    traceOut;
}

void Prefab::Reinitialize()
{
    traceIn(Prefab::Reinitialize);

    for(int i=0; i<MaterialList.Num(); i++)
        ReleaseMaterial(MaterialList[i]);
    MaterialList.Clear();

    DestroyObject(phyObject);
    if(phyShape)
        phyShape->Release();

    //---------------------------

    String prefabPath;
    if(!Engine::ConvertResourceName(prefabName, TEXT("prefabs"), prefabPath))
    {
        Super::Init();
        SafeDestroy();
        return;
    }

    ConfigFile prefabConfig;
    if(!prefabConfig.Open(prefabPath))
    {
        AppWarning(TEXT("Could not load prefab '%s'"), prefabPath.Array());
        Super::Init();
        SafeDestroy();
        return;
    }

    String strMeshName = prefabConfig.GetString(TEXT("Prefab"), TEXT("ModelFile"));
    if(strMeshName.IsEmpty())
    {
        AppWarning(TEXT("Bad mesh for prefab '%s'"), prefabPath.Array());
        Super::Init();
        SafeDestroy();
        return;
    }

    StringList MaterialNames;
    prefabConfig.GetStringList(TEXT("Prefab"), TEXT("Materials"), MaterialNames);

    MaterialList.SetSize(MaterialNames.Num());

    for(int i=0; i<MaterialNames.Num(); i++)
        MaterialList[i] = GetMaterial(MaterialNames[i]);

    phyShape = mesh->GetShape();
    phyObject   = physics->CreateStaticObject(phyShape, GetWorldPos(), GetWorldRot());
    phyObject->SetEntityOwner(this);
    phyObject->EnableCollisionCallback(TRUE);

    traceOut;
}

void Prefab::Serialize(Serializer &s)
{
    Super::Serialize(s);

    s << bRenderable << prefabName;
}


void AnimatedPrefab::Init()
{
}

void AnimatedPrefab::Reinitialize()
{
}

void AnimatedPrefab::Serialize(Serializer &s)
{
    Super::Serialize(s);

    s << bRenderable << prefabName << defaultAnimation;
}


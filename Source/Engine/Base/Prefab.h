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


#ifndef PREFAB_HEADER
#define PREFAB_HEADER



class BASE_EXPORT Prefab : public MeshEntity
{
    friend class PrefabBrowser;
    friend class PrefabPlacer;

    DeclareClass(Prefab, MeshEntity);

    String prefabName;
    BOOL bPrefabLoaded;

public:
    Prefab();

    void PreRender()            {}
    void Render()               {}
    void PostRender()           {}

    void PreFrame()             {}
    void Tick(float fSeconds)   {}

    void Init();
    void Reinitialize();

    void Serialize(Serializer &s);
};

class BASE_EXPORT AnimatedPrefab : public AnimatedEntity
{
    friend class PrefabBrowser;
    friend class PrefabPlacer;

    DeclareClass(AnimatedPrefab, AnimatedEntity);

    String prefabName;
    BOOL bPrefabLoaded;
    String defaultAnimation;

public:
    void Init();
    void Reinitialize();

    void Serialize(Serializer &s);
};



#endif


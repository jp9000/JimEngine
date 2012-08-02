/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Light.h:  Dynamic Lighting

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


#ifndef LIGHT_HEADER
#define LIGHT_HEADER



/*=========================================================
    Base Light class
==========================================================*/

class BASE_EXPORT Light : public Entity
{
    friend class EditorEngine;
    friend class EditorLevelInfo;
    friend class Level;

    DeclareClass(Light, Entity);

protected:
    BOOL            bOff;
    BOOL            bLightmapped;

    static VertexBuffer *editorPointVB;
    static List<Light*> LightList;

    inline static void AddMeshLight(MeshEntity *meshEnt, MeshLightInfo &info)
    {
        meshEnt->MeshLights << info;
    }

    List<MeshEntity*> LitEntities;
    List<Brush*>      LitBrushes;

public:
    Light();
    virtual ~Light();

    inline UINT         NumLitEntities() const                      {return LitEntities.Num();}
    inline MeshEntity*  GetLitEntity(UINT id) const                 {return LitEntities[id];}

    inline UINT         NumLitBrushes() const                       {return LitBrushes.Num();}
    inline Brush*       GetLitBrush(UINT id) const                  {return LitBrushes[id];}


    inline BOOL     CanCastShadows() const {return !bStaticLight && bCastShadows && !bLightmapped;}


    virtual void    ProcessEntity(MeshEntity *ent)                  {}
    virtual void    RemoveEntity(MeshEntity *ent, int side=0)       {}

    void            SetColor(DWORD newColor);
    void            SetColor(Color3 &newColor);
    void            SetColor(float newR, float newG, float newB);

    inline void     SetEnabled(BOOL bEnabled=TRUE)                  {bOff = !bEnabled;}
    inline BOOL     IsEnabled()                                     {return !bOff;}

    inline BOOL     IsLightmapped() const                           {return bLightmapped;}
    inline BOOL     IsOff() const {return bOff;}

    static inline unsigned int  NumLights()                         {return LightList.Num();}
    static inline Light*        GetLight(unsigned int i)            {return LightList[i];}

    virtual Entity* DuplicateEntity();

    void Serialize(Serializer &s);

    //<Script module="Base" classdecs="Light">
    BOOL bCastShadows;
    DWORD color;
    int intensity;
    BOOL bStaticLight;
    BOOL bEnableEmission;
    float lightVolume;

    Declare_Internal_Member(native_SetColor);
    Declare_Internal_Member(native_SetColor_2);
    Declare_Internal_Member(native_SetEnabled);
    Declare_Internal_Member(native_IsEnabled);
    //</Script>
};



/*=========================================================
    Point Light
==========================================================*/

#define UPDATEFACE_PX   1  //positive X
#define UPDATEFACE_NX   2
#define UPDATEFACE_PY   4  //positive Y
#define UPDATEFACE_NY   8
#define UPDATEFACE_PZ   16 //positive Z
#define UPDATEFACE_NZ   32

class BASE_EXPORT PointLight : public Light
{
    friend class Level;
    friend class IndoorLevel;
    friend class OutdoorLevel;
    friend class OctLevel;
    friend class SpaceLevel;

    DeclareClass(PointLight, Light);

    BaseTexture *renderTexture;
    DWORD updateFaces;

public:
    //<Script module="Base" classdecs="PointLight">
    float lightRange;
    //</Script>

    PointLight();

    void Init();
    void Destroy();

    void Reinitialize();

    Bounds GetBounds() {return Bounds(Vect(-lightRange), Vect(lightRange));}

    void OnUpdatePosition();

    void ProcessEntity(MeshEntity *ent);
    void RemoveEntity(MeshEntity *ent, int side);

    BOOL InsideClip(const ViewClip &clip);
    void EditorRender();
};



/*=========================================================
    Spot Light
==========================================================*/

class BASE_EXPORT SpotLight : public Light
{
    friend class Level;
    friend class IndoorLevel;
    friend class OutdoorLevel;
    friend class OctLevel;
    friend class SpaceLevel;

    DeclareClass(SpotLight, Light);

    Texture *spotTexture;
    BaseTexture *renderTexture;
    DWORD updateFaces;

public:
    //<Script module="Base" classdecs="SpotLight">
    float lightRange;
    String SpotlightMap;
    float cutoff;
    //</Script>

    SpotLight();

    void Init();
    void Destroy();

    void Reinitialize();

    Bounds GetBounds();

    void OnUpdatePosition();

    void ProcessEntity(MeshEntity *ent);
    void RemoveEntity(MeshEntity *ent, int side);

    void EditorRender();
};



/*=========================================================
    Directional Light
==========================================================*/

class BASE_EXPORT DirectionalLight : public Light
{
    friend class Level;

    DeclareClass(DirectionalLight, Light);

public:
    void Init();

    void EditorRender();
};


#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  SpaceLevel.h:  OUTER SPACE Level

  Copyright (c) 2001-2009, Hugh Bailey
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


#ifndef SPACELEVEL_HEADER
#define SPACELEVEL_HEADER


/*=========================================================2w2  
    SpaceObjectRef
==========================================================*/

struct SpaceObjectRef
{
    String strClass;
    Vect Pos;
    Quat Rot;

    friend inline Serializer& operator<<(Serializer &s, SpaceObjectRef &sor)
    {
        s << sor.strClass << sor.Pos << sor.Rot;
    }

    inline Serializer& SerializeObject(Serializer &s)
    {
        s << Pos << Rot;
    }
};


/*=========================================================2w2  
    WarpPoint
==========================================================*/

struct WarpPoint
{
    
};


/*=========================================================2w2  
    SpaceEntityData
==========================================================*/

class SpaceEntityData : public LevelData
{
};

#define GetSpaceEntityData(ent) static_cast<SpaceEntityData*>(ent->levelData)


/*=========================================================
    SpaceLevel
==========================================================*/

//-----------------------------------------
// Space Level
class BASE_EXPORT SpaceLevel : public Level
{
    DeclareClass(SpaceLevel, Level);

    friend struct       Brush;
    friend struct       AddRef;
    friend class        MeshEntity;
    friend class        PointLight;
    friend class        SpotLight;
    friend class        DirectionalLight;

protected:
    virtual void UpdateEntityPositionData(Entity *ent, const Matrix &transform);

    virtual void CalculateRenderOrder(Camera *camera, const ViewClip &clip) {}
    virtual void GetStaticGeometry(const Bounds &bounds, List<Brush*> &Brushes, List<MeshEntity*> &StaticMeshes) {}
    virtual void GetObjects(const Bounds &bounds, List<LevelObject*> &objects) {}
    virtual void GetObjects(const Vect &eyePos, const ViewClip &clip, List<LevelObject*> &objects) {}

public:
    SpaceLevel();
    ~SpaceLevel();

    virtual void PreFrame();

    virtual BOOL Load(CTSTR lpName);
    virtual void Unload();

    virtual void AddEntity(Entity *ent);
    virtual void RemoveEntity(Entity *ent, BOOL bChildren=TRUE);

    virtual AIPath* GetEntityPath(Entity *ent, const Vect &targetPos) {return NULL;}

    virtual void Tick(float fSeconds);

    ViewClip            fullClip;

    List<AddBrush>      BrushList;

    Mesh                sunModel;
    Texture             *sunTexture;

    CubeTexture         *backgroundTexture;
};



#endif

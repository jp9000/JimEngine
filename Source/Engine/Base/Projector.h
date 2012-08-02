/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Projector:  Texture projector

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


#ifndef PROJECTOR_HEADER
#define PROJECTOR_HEADER


class Projector : public Entity
{
    friend class Level;

    DeclareClass(Projector, Entity);

protected:
    static List<Projector*> ProjectorList;

    ViewClip clip;

    BOOL bPerspective;
    float left, right, top, bottom, znear, zfar;
    float projMatrix[16];

    List<MeshEntity*> MeshTargets;
    List<Brush*> BrushTargets;

public:
    void Init();
    void Destroy();

    virtual void OnUpdatePosition();

    virtual Bounds GetBounds();

    virtual void LoadProjector() {scriptLoadProjector();}

    inline float* GetProjectionMatrix() const      {return (float*)projMatrix;}

    inline float GetStartDist() const              {return znear;}
    inline float GetEndDist() const                {return zfar;}

    static inline UINT NumProjectors()             {return ProjectorList.Num();}
    static inline Projector* GetProjector(UINT id) {return ProjectorList[id];}

    inline const ViewClip& GetViewClip() const     {return clip;}

    inline void SetPerspective(float fovy, float aspect, float znearIn, float zfarIn)
    {
        float xmin, xmax, ymin, ymax;

        ymax = znearIn * tan(RAD(fovy)*0.5f);
        ymin = -ymax;

        xmin = ymin * aspect;
        xmax = ymax * aspect;

        SetFrustum(xmin, xmax, ymin, ymax, znearIn, zfarIn);
    }

    inline void SetFrustum(float leftIn, float rightIn, float topIn, float bottomIn, float znearIn, float zfarIn)
    {
        clip.SetFrustum(leftIn, rightIn, topIn, bottomIn, znearIn, zfarIn);

        bPerspective = TRUE;
        left    = leftIn;
        right   = rightIn;
        top     = topIn;
        bottom  = bottomIn;
        znear   = znearIn;
        zfar    = zfarIn;

        Matrix4x4Frustum(projMatrix, left, right, top, bottom, znear, zfar);
    }

    inline void SetOrtho(float leftIn, float rightIn, float topIn, float bottomIn, float znearIn, float zfarIn)
    {
        clip.SetOrtho(leftIn, rightIn, topIn, bottomIn, znearIn, zfarIn);

        bPerspective = FALSE;
        left    = leftIn;
        right   = rightIn;
        top     = topIn;
        bottom  = bottomIn;
        znear   = znearIn;
        zfar    = zfarIn;

        Matrix4x4Ortho(projMatrix, left, right, top, bottom, znear, zfar);
    }

    inline void LoadProjectionTransform()
    {
        if(bPerspective)
            Frustum(left, right, top, bottom, znear, zfar);
        else
            Ortho(left, right, top, bottom, znear, zfar);
    }

    //<Script module="Base" classdecs="Projector">
    Effect* effect;
    Texture* texture;

    void scriptLoadProjector()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 0, cs);
    }

    Declare_Internal_Member(native_SetPerspective);
    Declare_Internal_Member(native_SetFrustum);
    Declare_Internal_Member(native_SetOrtho);
    //</Script>
};


class Decal : public Projector
{
    DeclareClass(Decal, Projector);

public:
    Decal() : decalColor(1.0f, 1.0f, 1.0f, 1.0f) {}

    void LoadProjector();

    void Init();

    void EditorRender();

    //<Script module="Base" classdecs="Decal">
    Color4 decalColor;
    //</Script>
};

class LitDecal : public Decal
{
    DeclareClass(LitDecal, Decal);
};

class ShadowDecal : public Decal
{
    DeclareClass(ShadowDecal, Decal);

    MeshEntity *meshEnt;

public:
    Quat shadowRot;

    ShadowDecal() {}
    ShadowDecal(MeshEntity *ent);

    virtual BOOL UpdatingPosition();
};


#endif

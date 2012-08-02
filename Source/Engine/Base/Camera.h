/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Camera.h:  Camera

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


#ifndef CAMERA_HEADER
#define CAMERA_HEADER


class Viewport;


/*=========================================================
    View clipping
==========================================================*/

struct BASE_EXPORT ViewClip
{
    List<Plane> planes;


    ViewClip()  {}
    ViewClip(const ViewClip &clip)  {planes.CopyList(clip.planes);}


    void Transform(const Matrix &m);
    ViewClip GetTransform(const Matrix &m) const   {ViewClip clip(*this); clip.Transform(m); return clip;}


    void SetFrustum(float left, float right, float top, float bottom, float znear, float zfar=0.0f);
    void SetOrtho(float left, float right, float top, float bottom, float znear, float zfar=0.0f);

    inline void SetPerspective(float fovy, float aspect, float znear, float zfar=0.0f)
    {
        float xmin, xmax, ymin, ymax;

        ymax = znear * tan(RAD(fovy)/2);
        ymin = -ymax;

        xmin = ymin * aspect;
        xmax = ymax * aspect;

        SetFrustum(xmin, xmax, ymin, ymax, znear, zfar);
    }

    ViewClip& operator=(const ViewClip &clip)       {planes.CopyList(clip.planes); return *this;}

    BOOL operator==(const ViewClip &clip) const
    {
        if(clip.planes.Num() != planes.Num()) return 0;

        for(DWORD i=0; i<planes.Num(); i++)
        {
            if(clip.planes[i] != planes[i])
                return 0;
        }

        return 1;
    }

    BOOL operator!=(const ViewClip &clip)           {return !(*this == clip);}

    BOOL PointWithin(const Vect &v, float failDist=0.0f) const;

    inline BOOL BoundsVisible(const Bounds &box) const
    {
        for(DWORD i=0; i<planes.Num(); i++)
        {
            if(box.OutsidePlane(planes[i]))
                return FALSE; 
        }

        return TRUE;
    }

    int  BoundsTest(const Bounds &box) const;

    BOOL SphereVisible(const Vect &pos, float radius) const;


    void CreateFromBounds(const Vect &eye, const Bounds &box);
    void TruncateWithBounds(const Vect &eye, const Bounds &box);
};



/*========================================================
    Camera Class
=========================================================*/

//world rendering is done using the camera of course!
class BASE_EXPORT Camera : public Entity
{
    friend class Viewport;
    friend class EditorLevelInfo;
    friend class Level;

    DeclareClass(Camera, Entity);

    Viewport *assignedViewport;

    ViewClip    clip;

    BOOL bPerspective;
    BOOL bSoundCamera;
    float left, right, top, bottom, znear, zfar;

public:
    virtual void Init();
    void SetPerspective(float fovy, float aspect, float znearIn, float zfarIn);
    void SetFrustum(float leftIn, float rightIn, float topIn, float bottomIn, float znearIn, float zfarIn);
    void SetOrtho(float leftIn, float rightIn, float topIn, float bottomIn, float znearIn, float zfarIn);

    void LoadProjectionTransform();

    inline const ViewClip& GetClip() const      {return clip;}

    inline Viewport *GetAssignedViewport()      {return assignedViewport;}

    inline BOOL IsPerspective()                 {return bPerspective;}
    inline BOOL IsSoundCamera()                 {return bSoundCamera;}

    inline void SetSoundCamera(BOOL bPlaySound) {bSoundCamera = bPlaySound;}

    inline float Left() const                   {return left;}
    inline float Right() const                  {return right;}
    inline float Top() const                    {return top;}
    inline float Bottom() const                 {return bottom;}
    inline float Near() const                   {return znear;}
    inline float Far() const                    {return zfar;}

    //<Script module="Base" classdecs="Camera">
    Declare_Internal_Member(native_SetPerspective);
    Declare_Internal_Member(native_SetFrustum);
    Declare_Internal_Member(native_SetOrtho);
    Declare_Internal_Member(native_GetAssignedViewport);
    Declare_Internal_Member(native_SetSoundCamera);
    Declare_Internal_Member(native_IsPerspective);
    Declare_Internal_Member(native_IsSoundCamera);
    Declare_Internal_Member(native_LoadProjectionTransform);
    //</Script>
};

inline void GraphicsSystem::DrawCubeBackdrop(Camera *cam, CubeTexture *cubetexture)
{
    assert(cam && cubetexture);
    if(!cam || (!cam->IsPerspective())) return;

    DrawCubeBackdrop(cubetexture, cam->GetWorldRot(), cam->Left(), cam->Right(), cam->Top(), cam->Bottom(), cam->Near());
}

inline void GraphicsSystem::DrawCubeBackdrop(Camera *cam, CubeTexture *cubetexture, const Quat &customRot)
{
    assert(cam && cubetexture);
    if(!cam || (!cam->IsPerspective())) return;

    DrawCubeBackdrop(cubetexture, customRot, cam->Left(), cam->Right(), cam->Top(), cam->Bottom(), cam->Near());
}


/*========================================================
    UserCamera Class
=========================================================*/

class BASE_EXPORT UserCamera : public Camera
{
    DeclareClass(UserCamera, Camera);

public:
    UserCamera();

    void Init();

    void Reinitialize();

    void EditorRender();

    //<Script module="Base" classdecs="UserCamera">
    float fFOV;
    float fAspect;
    //</Script>
};


/*========================================================
    Skybox Cam
=========================================================*/

class BASE_EXPORT SkyboxCam : public Camera
{
    DeclareClass(SkyboxCam, Camera);
};




#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Camera.h:  Main Camera/Viewport code

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


DefineClass(Camera);
DefineClass(UserCamera);
DefineClass(SkyboxCam);

/*=========================================================
    View clipping
==========================================================*/

void ViewClip::Transform(const Matrix &m)
{
    for(DWORD i=0; i<planes.Num(); i++)
        planes[i].Transform(m);
}

void ViewClip::SetFrustum(float left, float right, float top, float bottom, float znear, float zfar)
{
    Vect a,b,c;

    planes.SetSize(zfar == 0.0f ? 5 : 6);

    planes[0].Dir.Set(0.0f, 0.0f, 1.0f);
    planes[0].Dist = -znear;

    a = 0; //(0,0,0)
    c.z = b.z = -znear;
    c.x = b.x = left;
    b.y = bottom;
    c.y = top;
    planes[1].CreateFromTri(a, b, c);

    c.x = b.x = right;
    planes[2].CreateFromTri(a, c, b);

    b.x = left;
    c.x = right;
    c.y = b.y = top;
    planes[3].CreateFromTri(a, b, c);

    c.y = b.y = bottom;
    planes[4].CreateFromTri(a, c, b);

    if(zfar != 0.0f)
    {
        planes[5].Dir.Set(0.0f, 0.0f, -1.0f);
        planes[5].Dist = zfar;
    }
}

void ViewClip::SetOrtho(float left, float right, float top, float bottom, float znear, float zfar)
{
    Vect a,b,c;

    planes.SetSize(zfar == 0.0f ? 5 : 6);

    planes[0].Dir.Set(0.0f, 0.0f, 1.0f);
    planes[0].Dist = -znear;

    a.z = zfar;
    c.z = b.z = znear;

    a.x = c.x = b.x = left;
    a.y = b.y = bottom;
    c.y = top;
    planes[1].CreateFromTri(a, b, c);

    a.x = c.x = b.x = right;
    a.y = b.y = top;
    c.y = bottom;
    planes[2].CreateFromTri(a, b, c);

    a.x = b.x = left;
    c.x = right;
    a.y = c.y = b.y = top;
    planes[3].CreateFromTri(a, b, c);

    a.x = b.x = right;
    c.x = left;
    a.y = c.y = b.y = bottom;
    planes[4].CreateFromTri(a, b, c);

    if(zfar != 0.0f)
    {
        planes[5].Dir.Set(0.0f, 0.0f, -1.0f);
        planes[5].Dist = zfar;
    }
}

BOOL ViewClip::PointWithin(const Vect &v, float failDist) const
{
    for(unsigned int i=0; i<planes.Num(); i++)
    {
        if(v.DistFromPlane(planes[i]) >= failDist)
            return 0;
    }

    return 1;
}

BOOL ViewClip::SphereVisible(const Vect &pos, float radius) const
{
    for(DWORD i=0; i<planes.Num(); i++)
    {
        if(pos.DistFromPlane(planes[i]) > radius)
            return 0; 
    }

    return 1;
}

int  ViewClip::BoundsTest(const Bounds &box) const
{
    DWORD result=0;

    for(DWORD i=0; i<planes.Num(); i++)
        result |= box.PlaneTest(planes[i]); 

    return result;
}


struct AABBEdges
{
    char num;
    union
    {
        struct {char v1,v2,v3,v4,v5,v6;};
        char ptr[6];
    };
};

AABBEdges boxedges[3][3][3] =
{{{ {6, 1, 3, 2, 6, 4, 5},
    {6, 1, 3, 2, 0, 4, 5},
    {6, 5, 7, 3, 2, 0, 4}},

  { {6, 1, 3, 2, 6, 4, 0},
    {4, 1, 3, 2, 0 ,0,0},
    {6, 5, 7, 3, 2, 0, 1}},

  { {6, 1, 3, 7, 6, 4, 0},
    {6, 3, 7, 6, 2, 0, 1},
    {6, 5, 7, 6, 2, 0, 1}}},

 {{
    {6, 0, 2, 6, 4, 5, 1},
    {4, 0, 4, 5, 1 ,0,0},
    {6, 0, 4, 5, 7, 3, 1}},

  { {4, 0, 2, 6, 4 ,0,0},
    {0,0,0,0,0,0,0},
    {4, 5, 7, 3, 1 ,0,0}},

  { {6, 2, 3, 7, 6, 4, 0},
    {4, 2, 3, 7, 6 ,0,0},
    {6, 7, 6, 2, 3, 1, 5}}},

 {{ {6, 0, 2, 6, 7, 5, 1},
    {6, 4, 6, 7, 5, 1, 0},
    {6, 4, 6, 7, 3, 1, 0}},

  { {6, 0, 2, 6, 7, 5, 4},
    {4, 4, 6, 7, 5 ,0,0},
    {6, 4, 6, 7, 3, 1, 5}},

  { {6, 0, 2, 3, 7, 5, 4},
    {6, 6, 2, 3, 7, 5, 4},
    {6, 6, 2, 3, 1, 5, 4}}}};


/*
   a 3x3x3 is fast and gives us a perfect clipping volume.

xmin ymin zmin
xmin ymin zMid
xmin ymin zMax
xmin yMid zmin
xmin yMid zMid
xmin yMid zMax
xmin yMax zmin
xmin yMax zMin
xmin yMax zMax

xMid ymin zmin
xMid ymin zMid
xMid ymin zMax
xMid yMid zmin
xMid yMid zMid
xMid yMid zMax
xMid yMax zmin
xMid yMax zMin
xMid yMax zMax

xMax ymin zmin
xMax ymin zMid
xMax ymin zMax
xMax yMid zmin
xMax yMid zMid
xMax yMid zMax
xMax yMax zmin
xMax yMax zMin
xMax yMax zMax

*/

void ViewClip::CreateFromBounds(const Vect &eye, const Bounds &box)
{
    int xval, yval, zval;

    /*if(eye.x < box.Min.x)       xval = 0;   //xmin
    else if(eye.x > box.Max.x)  xval = 2;   //xmax
    else                        xval = 1;   //xmid

    if(eye.y < box.Min.y)       yval = 0;   //ymin
    else if(eye.y > box.Max.y)  yval = 2;   //ymax
    else                        yval = 1;   //ymid

    if(eye.z < box.Min.z)       zval = 0;   //zmin
    else if(eye.z > box.Max.z)  zval = 2;   //zmax
    else                        zval = 1;   //zmid*/

    xval = (eye.x < box.Min.x) ? 0 : ((eye.x > box.Max.x) ? 2 : 1);
    yval = (eye.y < box.Min.y) ? 0 : ((eye.y > box.Max.y) ? 2 : 1);
    zval = (eye.z < box.Min.z) ? 0 : ((eye.z > box.Max.z) ? 2 : 1);

    AABBEdges &edgeverts = (boxedges[xval][yval][zval]);

    assert(edgeverts.num);

    if((xval == 1) && (yval == 1) && (zval == 1))
        AppWarning(TEXT("ViewClip::CreateFromBounds:  Tried to create volume from inside bounds"));

    if(edgeverts.num)
    {
        int i;

        Vect v1 = box.GetPoint(edgeverts.v1),curPoint,nextPoint;
        curPoint = v1;

        planes.SetSize(edgeverts.num);

        for(i=1; i<edgeverts.num; i++)
        {
            nextPoint = box.GetPoint(edgeverts.ptr[i]);

            if(nextPoint != curPoint)
            {
                planes[i-1].CreateFromTri(eye, curPoint, nextPoint);

                curPoint = nextPoint;
            }
        }

        planes[i-1].CreateFromTri(eye, curPoint, v1);
    }
}


void ViewClip::TruncateWithBounds(const Vect &eye, const Bounds &box)
{
    List<Plane> oldplanes;
    zero(&oldplanes, 8);

    oldplanes.CopyList(planes);

    CreateFromBounds(eye, box);

    planes.AppendList(oldplanes);
    oldplanes.Clear();
}


/*=========================================================
    Camera functions
==========================================================*/

void Camera::Init()
{
    bSoundCamera = TRUE;
    editorSprite = NULL;

    Super::Init();
}

void Camera::SetPerspective(float fovy, float aspect, float znearIn, float zfarIn)
{
    float xmin, xmax, ymin, ymax;

    ymax = znearIn * tan(RAD(fovy)/2);
    ymin = -ymax;

    xmin = ymin * aspect;
    xmax = ymax * aspect;

    SetFrustum(xmin, xmax, ymin, ymax, znearIn, zfarIn);
}

void Camera::SetFrustum(float leftIn, float rightIn, float topIn, float bottomIn, float znearIn, float zfarIn)
{
    clip.SetFrustum(leftIn, rightIn, topIn, bottomIn, znearIn);

    bPerspective = 1;
    left = leftIn;
    right = rightIn;
    top = topIn;
    bottom = bottomIn;
    znear = znearIn;
    zfar = zfarIn;
}

void Camera::SetOrtho(float leftIn, float rightIn, float topIn, float bottomIn, float znearIn, float zfarIn)
{
    clip.SetOrtho(leftIn, rightIn, topIn, bottomIn, znearIn);

    bPerspective = 0;
    left = leftIn;
    right = rightIn;
    top = topIn;
    bottom = bottomIn;
    znear = znearIn;
    zfar = zfarIn;
}

void Camera::LoadProjectionTransform()
{
    if(bPerspective)
        Frustum(left, right, top, bottom, znear, zfar);
    else
        Ortho(left, right, top, bottom, znear, zfar);
}


/*=========================================================
    UserCamera functions
==========================================================*/

UserCamera::UserCamera()
{
    fFOV = 60.0f;
    fAspect = 1.0f;
}

void UserCamera::Init()
{
    Super::Init();
    Reinitialize();
}

void UserCamera::Reinitialize()
{
    SetPerspective(fFOV, fAspect, 0.4f, 4096.0f);
}

void UserCamera::EditorRender()
{
    Super::EditorRender();
}


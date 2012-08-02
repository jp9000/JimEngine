/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Camera

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


#include "..\Base.h"


//<Script module="Base" filedefs="Camera.xscript">
void Camera::native_SetPerspective(CallStruct &cs)
{
    float fovy = cs.GetFloat(0);
    float aspect = cs.GetFloat(1);
    float znearIn = cs.GetFloat(2);
    float zfarIn = cs.GetFloat(3);

    SetPerspective(fovy, aspect, znearIn, zfarIn);
}

void Camera::native_SetFrustum(CallStruct &cs)
{
    float leftIn = cs.GetFloat(0);
    float rightIn = cs.GetFloat(1);
    float topIn = cs.GetFloat(2);
    float bottomIn = cs.GetFloat(3);
    float znearIn = cs.GetFloat(4);
    float zfarIn = cs.GetFloat(5);

    SetFrustum(leftIn, rightIn, topIn, bottomIn, znearIn, zfarIn);
}

void Camera::native_SetOrtho(CallStruct &cs)
{
    float leftIn = cs.GetFloat(0);
    float rightIn = cs.GetFloat(1);
    float topIn = cs.GetFloat(2);
    float bottomIn = cs.GetFloat(3);
    float znearIn = cs.GetFloat(4);
    float zfarIn = cs.GetFloat(5);

    SetOrtho(leftIn, rightIn, topIn, bottomIn, znearIn, zfarIn);
}

void Camera::native_GetAssignedViewport(CallStruct &cs)
{
    Viewport*& returnVal = (Viewport*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetAssignedViewport();
}

void Camera::native_SetSoundCamera(CallStruct &cs)
{
    BOOL bPlaySound = (BOOL)cs.GetInt(0);

    SetSoundCamera(bPlaySound);
}

void Camera::native_IsPerspective(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = IsPerspective();
}

void Camera::native_IsSoundCamera(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = IsSoundCamera();
}

void Camera::native_LoadProjectionTransform(CallStruct &cs)
{
    LoadProjectionTransform();
}
//</Script>

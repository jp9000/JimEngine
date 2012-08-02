/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Projectors

  Copyright (c) 2009, Hugh Bailey
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


//<Script module="Base" filedefs="Projector.xscript">
void Projector::native_SetPerspective(CallStruct &cs)
{
    float fovy = cs.GetFloat(0);
    float aspect = cs.GetFloat(1);
    float znearIn = cs.GetFloat(2);
    float zfarIn = cs.GetFloat(3);

    SetPerspective(fovy, aspect, znearIn, zfarIn);
}

void Projector::native_SetFrustum(CallStruct &cs)
{
    float left = cs.GetFloat(0);
    float right = cs.GetFloat(1);
    float top = cs.GetFloat(2);
    float bottom = cs.GetFloat(3);
    float znear = cs.GetFloat(4);
    float zfar = cs.GetFloat(5);

    SetFrustum(left, right, top, bottom, znear, zfar);
}

void Projector::native_SetOrtho(CallStruct &cs)
{
    float left = cs.GetFloat(0);
    float right = cs.GetFloat(1);
    float top = cs.GetFloat(2);
    float bottom = cs.GetFloat(3);
    float znear = cs.GetFloat(4);
    float zfar = cs.GetFloat(5);

    SetOrtho(left, right, top, bottom, znear, zfar);
}
//</Script>

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Controls

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


//<Script module="Base" filedefs="ResourceManagement.xscript">
void ENGINEAPI NativeGlobal_GetMaterial(CallStruct &cs)
{
    Material*& returnVal = (Material*&)cs.GetObjectOut(RETURNVAL);
    String name = cs.GetString(0);

    returnVal = GetMaterial(name);
}

void ENGINEAPI NativeGlobal_GetTexture(CallStruct &cs)
{
    Texture*& returnVal = (Texture*&)cs.GetObjectOut(RETURNVAL);
    String name = cs.GetString(0);
    BOOL bGenMipMaps = (BOOL)cs.GetInt(1);

    returnVal = GetTexture(name, bGenMipMaps);
}

void ENGINEAPI NativeGlobal_GetCubeTexture(CallStruct &cs)
{
    CubeTexture*& returnVal = (CubeTexture*&)cs.GetObjectOut(RETURNVAL);
    String name = cs.GetString(0);
    BOOL bGenMipMaps = (BOOL)cs.GetInt(1);

    returnVal = GetCubeTexture(name, bGenMipMaps);
}

void ENGINEAPI NativeGlobal_AddTextureRef(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    BaseTexture* texture = (BaseTexture*)cs.GetObject(0);

    returnVal = AddTextureRef(texture);
}

void ENGINEAPI NativeGlobal_ReleaseMaterial(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    Material* material = (Material*)cs.GetObject(0);

    returnVal = ReleaseMaterial(material);
}

void ENGINEAPI NativeGlobal_ReleaseMaterial_2(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    String name = cs.GetString(0);

    returnVal = ReleaseMaterial(name);
}

void ENGINEAPI NativeGlobal_ReleaseTexture(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    BaseTexture* texture = (BaseTexture*)cs.GetObject(0);

    returnVal = ReleaseTexture(texture);
}

void ENGINEAPI NativeGlobal_ReleaseTexture_2(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    String name = cs.GetString(0);

    returnVal = ReleaseTexture(name);
}

void ENGINEAPI NativeGlobal_GetPixelShader(CallStruct &cs)
{
    Shader*& returnVal = (Shader*&)cs.GetObjectOut(RETURNVAL);
    String name = cs.GetString(0);

    returnVal = GetPixelShader(name);
}

void ENGINEAPI NativeGlobal_GetVertexShader(CallStruct &cs)
{
    Shader*& returnVal = (Shader*&)cs.GetObjectOut(RETURNVAL);
    String name = cs.GetString(0);

    returnVal = GetVertexShader(name);
}

void ENGINEAPI NativeGlobal_GetEffect(CallStruct &cs)
{
    Effect*& returnVal = (Effect*&)cs.GetObjectOut(RETURNVAL);
    String name = cs.GetString(0);

    returnVal = GetEffect(name);
}

void ENGINEAPI NativeGlobal_GetEffect_2(CallStruct &cs)
{
    Effect*& returnVal = (Effect*&)cs.GetObjectOut(RETURNVAL);
    int i = cs.GetInt(0);

    returnVal = GetEffect(i);
}

void ENGINEAPI NativeGlobal_NewSound(CallStruct &cs)
{
    Sound*& returnVal = (Sound*&)cs.GetObjectOut(RETURNVAL);
    String name = cs.GetString(0);
    BOOL b3DSound = (BOOL)cs.GetInt(1);
    BOOL bSelfDestruct = (BOOL)cs.GetInt(2);

    returnVal = NewSound(name, b3DSound, bSelfDestruct);
}
//</Script>

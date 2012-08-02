/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Material

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


//<Script module="Base" filedefs="Material.xscript">
void Material::native_LoadFromFile(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String file = cs.GetString(0);

    returnVal = LoadFromFile(file);
}

void Material::native_SetCurrentEffect(CallStruct &cs)
{
    Effect* effectIn = (Effect*)cs.GetObject(0);

    SetCurrentEffect(effectIn);
}

void Material::native_GetCurrentEffect(CallStruct &cs)
{
    Effect*& returnVal = (Effect*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetCurrentEffect();
}

void Material::native_SetFloat(CallStruct &cs)
{
    String paramName = cs.GetString(0);
    float fValue = cs.GetFloat(1);

    SetFloat(paramName, fValue);
}

void Material::native_SetColor(CallStruct &cs)
{
    String paramName = cs.GetString(0);
    DWORD color = (DWORD)cs.GetInt(1);

    SetColor(paramName, color);
}

void Material::native_SetCurrentTexture(CallStruct &cs)
{
    String paramName = cs.GetString(0);
    BaseTexture* texture = (BaseTexture*)cs.GetObject(1);

    SetCurrentTexture(paramName, texture);
}

void Material::native_GetFloat(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    String paramName = cs.GetString(0);

    returnVal = GetFloat(paramName);
}

void Material::native_GetColor(CallStruct &cs)
{
    DWORD& returnVal = (DWORD&)cs.GetIntOut(RETURNVAL);
    String paramName = cs.GetString(0);

    returnVal = GetColor(paramName);
}

void Material::native_GetCurrentTexture(CallStruct &cs)
{
    BaseTexture*& returnVal = (BaseTexture*&)cs.GetObjectOut(RETURNVAL);
    String paramName = cs.GetString(0);

    returnVal = GetCurrentTexture(paramName);
}

void Material::native_GetParamID(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    String paramName = cs.GetString(0);

    returnVal = GetParamID(paramName);
}

void Material::native_SetFloat_2(CallStruct &cs)
{
    int param = cs.GetInt(0);
    float fValue = cs.GetFloat(1);

    SetFloat(param, fValue);
}

void Material::native_SetColor_2(CallStruct &cs)
{
    int param = cs.GetInt(0);
    DWORD color = (DWORD)cs.GetInt(1);

    SetColor(param, color);
}

void Material::native_SetCurrentTexture_2(CallStruct &cs)
{
    int param = cs.GetInt(0);
    BaseTexture* texture = (BaseTexture*)cs.GetObject(1);

    SetCurrentTexture(param, texture);
}

void Material::native_GetFloat_2(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    int param = cs.GetInt(0);

    returnVal = GetFloat(param);
}

void Material::native_GetColor_2(CallStruct &cs)
{
    DWORD& returnVal = (DWORD&)cs.GetIntOut(RETURNVAL);
    int param = cs.GetInt(0);

    returnVal = GetColor(param);
}

void Material::native_GetCurrentTexture_2(CallStruct &cs)
{
    BaseTexture*& returnVal = (BaseTexture*&)cs.GetObjectOut(RETURNVAL);
    int param = cs.GetInt(0);

    returnVal = GetCurrentTexture(param);
}
//</Script>

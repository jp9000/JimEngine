/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Config

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


//<Script module="Base" filedefs="Config.xscript">
void ENGINEAPI NativeGlobal_GetConfigString(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    String section = cs.GetString(0);
    String key = cs.GetString(1);
    String defaultValue = cs.GetString(2);

    returnVal = AppConfig->GetString(section, key, defaultValue);
}

void ENGINEAPI NativeGlobal_GetConfigInt(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    String section = cs.GetString(0);
    String key = cs.GetString(1);
    int defaultValue = cs.GetInt(2);

    returnVal = AppConfig->GetInt(section, key, defaultValue);
}

void ENGINEAPI NativeGlobal_GetConfigFloat(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);
    String section = cs.GetString(0);
    String key = cs.GetString(1);
    float defaultValue = cs.GetFloat(2);

    returnVal = AppConfig->GetFloat(section, key, defaultValue);
}

void ENGINEAPI NativeGlobal_GetConfigColor(CallStruct &cs)
{
    Color4& returnVal = (Color4&)cs.GetStructOut(RETURNVAL);
    String section = cs.GetString(0);
    String key = cs.GetString(1);

    returnVal = AppConfig->GetColor(section, key);
}

void ENGINEAPI NativeGlobal_SetConfigString(CallStruct &cs)
{
    String section = cs.GetString(0);
    String key = cs.GetString(1);
    String value = cs.GetString(2);

    AppConfig->SetString(section, key, value);
}

void ENGINEAPI NativeGlobal_SetConfigInt(CallStruct &cs)
{
    String section = cs.GetString(0);
    String key = cs.GetString(1);
    int value = cs.GetInt(2);

    AppConfig->SetInt(section, key, value);
}

void ENGINEAPI NativeGlobal_SetConfigFloat(CallStruct &cs)
{
    String section = cs.GetString(0);
    String key = cs.GetString(1);
    float value = cs.GetFloat(2);

    AppConfig->SetFloat(section, key, value);
}

void ENGINEAPI NativeGlobal_SetConfigColor(CallStruct &cs)
{
    String section = cs.GetString(0);
    String key = cs.GetString(1);
    const Color4 &value = (const Color4&)cs.GetStruct(2);

    AppConfig->SetColor(section, key, value);
}
//</Script>

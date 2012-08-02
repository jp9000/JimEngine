/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Object/FrameObject

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


//<Script module="Base" filedefs="Object.xscript">
void Object::native_AddReference(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = AddReference();
}

void Object::native_GetRefCount(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = GetRefCount();
}

void Object::native_Release(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = Release();
}

void Object::native_IsOf(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String className = cs.GetString(0);

    returnVal = IsOf(className);
}

void Object::native_InitializeObject(CallStruct &cs)
{
    InitializeObject();
}

void ENGINEAPI NativeGlobal_CreateFactoryObject(CallStruct &cs)
{
    Object*& returnVal = (Object*&)cs.GetObjectOut(RETURNVAL);
    String objName = cs.GetString(0);
    BOOL bInitialize = (BOOL)cs.GetInt(1);

    returnVal = CreateFactoryObject(objName, bInitialize);
}
//</Script>

//<Script module="Base" filedefs="FrameObject.xscript">
void FrameObject::native_SafeRelease(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = SafeRelease();
}

void FrameObject::native_SafeDestroy(CallStruct &cs)
{
    SafeDestroy();
}
//</Script>

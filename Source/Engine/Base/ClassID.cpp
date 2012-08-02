/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ClassID.cpp:  Class Identification System

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
#include "ScriptDefs.h"


Class *Class::lpFirst = NULL;
Class *Class::lpLast = NULL;



Class::Class(const TCHAR *lpName, Class *parent_class, BOOL bAbstract, CREATENEWPROC createproc, DWORD dwClassSize)
    : name(lpName), Parent(parent_class),
    lpNext(NULL), lpPrev(NULL), pCreate(createproc),
    classSize(dwClassSize)
{
    isAbstract = bAbstract;

    scpy(module, Engine::loadingModule);

    if(lpLast)
    {
        lpPrev = lpLast;
        lpLast->lpNext = this;
    }
    else
    {
        lpFirst = this;
        lpPrev = lpNext = NULL;
    }

    lpLast = this;
}

Class::~Class()
{
    if(scriptClass)
        delete scriptClass;

    if(bFreeManually)
        Free((void*)name);

    if(lpPrev)
        lpPrev->lpNext = lpNext;
    if(lpNext)
        lpNext->lpPrev = lpPrev;

    if(!lpPrev && !lpNext)
        lpFirst = lpLast = NULL;
    else if(lpLast == this)
        lpLast = lpPrev;
    else if(lpFirst == this)
        lpFirst = lpNext;
}

Class *Class::_FindClass(const TCHAR *lpName, const TCHAR *lpModule)
{
    Class *lpTemp = lpFirst;

    do
    {
        if(lpModule && (scmpi(lpTemp->module, lpModule) != 0))
            continue;
        if(scmp(lpTemp->name, lpName) == 0)
            return lpTemp;
    }while(lpTemp = lpTemp->lpNext);

    return NULL;
}

void Class::_FindClassesOf(const TCHAR *lpName, List<Class*> &classes)
{
    Class *lpTemp = lpFirst;
    Class *lpBaseClass = _FindClass(lpName);

    classes.Clear();

    do
    {
        if(lpTemp->_IsOf(lpBaseClass) && (lpTemp != lpBaseClass))
            classes << lpTemp;
    }while(lpTemp = lpTemp->lpNext);
}


void Class::GetClassChildren(Class* cls, List<Class*> &classes)
{
    Class *lpTemp = lpFirst;

    do
    {
        if(lpTemp->Parent == cls)
            classes << lpTemp;
    }while(lpTemp = lpTemp->lpNext);
}


BOOL Class::_IsOf(Class *cls) const
{
    const Class *testClass = this;
    while(testClass != NULL)
    {
        if(testClass == cls) return 1;
        testClass = testClass->Parent;
    }

    return 0;
}


void* Class::GetScriptVarAddress(Object *obj, int pos)
{
    Class *curClass = this;

    do
    {
        if(!curClass->scriptClass)
            continue;

        ClassDefinition *classDef = curClass->scriptClass;

        if(!classDef->Variables.Num())
            continue;

        if(classDef->variableStartIndex > pos)
            continue;

        for(int i=0; i<classDef->Variables.Num(); i++)
        {
            if((classDef->variableStartIndex+i) == pos)
                return ((LPBYTE)obj)+classDef->Variables[i].offset;
        }
    }while(curClass = curClass->Parent);

    return NULL;
}

FunctionDefinition* Class::GetTopmostFunction(int pos)
{
    Class *curClass = this;

    do
    {
        if(!curClass->scriptClass)
            continue;

        ClassDefinition *classDef = curClass->scriptClass;

        if(!classDef->Functions.Num())
            continue;

        for(int i=0; i<classDef->Functions.Num(); i++)
        {
            FunctionDefinition &curFunc = classDef->Functions[i];
            if(curFunc.funcOffset == pos)
                return &curFunc;
        }
    }while(curClass = curClass->Parent);

    return NULL;
}

ClassDefinition* Class::GetScriptClass()
{
    Class *curClass = this;

    do
    {
        if(!curClass->scriptClass)
            continue;
        else
            return curClass->scriptClass;
    }while(curClass = curClass->Parent);

    return NULL;
}

void Class::CallScriptConstructors(Object *obj)
{
    ClassDefinition *classDef = GetScriptClass();
    if(!classDef)
        return;

    CallClassConstructor(obj, classDef);
}

void Class::CallClassConstructor(Object *obj, ClassDefinition *classDef)
{
    if(classDef->Parent)
        CallClassConstructor(obj, classDef->Parent);

    if(classDef->baseConstructor)
    {
        FunctionDefinition *constructor = classDef->baseConstructor;

        CallStruct cs;
        cs.SetNumParams(constructor->Params.Num());

        for(int i=0; i<constructor->Params.Num(); i++)
        {
            DefaultVariable *param = &constructor->Params[i];
            if(param->typeInfo.type == DataType_String)
            {
                BufferInputSerializer s(param->DefaultParamData);
                String str;
                s << str;

                cs.SetString(i, str);
            }
            else
                cs.SetStruct(i, param->DefaultParamData.Array(), param->DefaultParamData.Num());
        }

        constructor->Call(obj, cs);
    }
}

void Class::DefineNativeMember(OBJECTCALLBACK func, int id)
{
    assert(scriptClass);
    if(!scriptClass)
        return;
    assert(id < scriptClass->Functions.Num());
    if(id >= scriptClass->Functions.Num())
        return;
    assert(scriptClass->Functions[id].flags & FUNC_INTERNAL);
    scriptClass->Functions[id].ObjectFunc = func;
}

void Class::DefineNativeStaticMember(NATIVECALLBACK func, int id)
{
    assert(scriptClass);
    if(!scriptClass)
        return;
    assert(id < scriptClass->Functions.Num());
    if(id >= scriptClass->Functions.Num())
        return;
    assert(scriptClass->Functions[id].flags & FUNC_INTERNAL);
    scriptClass->Functions[id].NativeFunc = func;
}

void Class::DefineNativeVariable(size_t offset, int id)
{
    assert(scriptClass);
    if(!scriptClass)
        return;
    assert(id < scriptClass->Variables.Num());
    if(id >= scriptClass->Variables.Num())
        return;
    scriptClass->Variables[id].offset = (int)offset;
}

void Class::CallScriptMember(Object *obj, unsigned int id, CallStruct &cs)
{
    traceIn(Class::CallScriptMember);

    if(!obj || !scriptClass) return;

    int actualID = scriptClass->functionStartIndex+id;

    if(obj->objScript)
    {
        for(int i=0; i<obj->objScript->Functions.Num(); i++)
        {
            if(obj->objScript->Functions[i].funcOffset == actualID)
            {
                obj->objScript->Functions[i].Call(obj, cs);
                break;
            }
        }
    }

    FunctionDefinition *funcDef = obj->GetObjectClass()->GetTopmostFunction(actualID);
    assert(funcDef);
    if(funcDef)
        funcDef->Call(obj, cs);

    traceOut;
}

Object* Class::Create(BOOL bInitialize)
{
    Object *obj = pCreate(this);
    obj->topClass = this;

    if(bInitialize)
    {
        CallScriptConstructors(obj);
        obj->Init();
    }

    return obj;
}


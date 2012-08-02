/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Obj.cpp:  Object Engine

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
#include "ScriptCompiler.h"


Class Object::class_id(TEXT("Object"), NULL, TRUE, Object::CreateNew, sizeof(Object));
Object* ENGINEAPI Object::CreateNew(Class *targetClass) {void* test1234 = Allocate(targetClass->GetClassSize()); zero(test1234, targetClass->GetClassSize()); return new(test1234) Object;}

Object* Object::pFirstObject = NULL;
Object* Object::pLastObject = NULL;

DefineClass(SerializerObject);
DefineClass(FrameObject);

FrameObject* FrameObject::pFirstFrameObject = NULL;
FrameObject* FrameObject::pLastFrameObject = NULL;

#define GetVarAddress(obj, var) (((LPBYTE)(obj))+var->offset)

Object::Object()
{
    if(pLastObject)
    {
        pPrevObject = pLastObject;
        pLastObject->pNextObject = this;
    }
    else 
    {
        pPrevObject = pNextObject = NULL;
        pFirstObject = this;
    }

    pLastObject = this;
}

Object::~Object()
{
    if(pPrevObject)
        pPrevObject->pNextObject = pNextObject;
    if(pNextObject)
        pNextObject->pPrevObject = pPrevObject;

    if(!pPrevObject && !pNextObject)
        pFirstObject = pLastObject = NULL;
    else if(pLastObject == this)
        pLastObject = pPrevObject;
    else if(pFirstObject == this)
        pFirstObject = pNextObject;
}

void Object::DestroyAll()
{
    traceIn(Object::DestroyAll);

    if(pFirstObject)
    {
        String strWarning = TEXT("Some objects of the following classes were never destroyed:\r\n    ");

        BOOL bFirst = TRUE;
        while(pFirstObject)
        {
            if(!bFirst)
                strWarning << TEXT(",\r\n    ");
            else
                bFirst = FALSE;

            strWarning << pFirstObject->GetObjectClass()->GetName();
            delete pFirstObject;
        }

        AppWarning(strWarning);
    }

    traceOut;
}

void Object::Serialize(Serializer &s)
{
    traceIn(Object::Serialize);

    ClassDefinition *classDef = topClass->GetScriptClass();
    int i;

    if(s.IsLoading())
    {
        int num;
        s << num;

        for(i=0; i<num; i++)
        {
            String name, typeName;
            int size;

            s << name << typeName << size;

            PropertyVariable *var = classDef->GetVariable(name);

            //if the variable isn't found anymore or the type has changed then skip, that way we
            //don't break level files every time we make a change to property variables
            if(!var || (!typeName.Compare(var->typeInfo.name)))
                s.Seek(size, SERIALIZE_SEEK_CURRENT);
            else
            {
                if(var->typeInfo.type == DataType_String)
                {
                    String str;
                    s << str;

                    *(String*)GetVarAddress(this, var) = str;
                }
                else if(var->typeInfo.type == DataType_Object)
                {
                    if(var->flags & VAR_PROPERTY)
                    {
                        BOOL bSound         = (scmp(var->typeInfo.name, TEXT("Sound")) == 0);
                        BOOL bTexture       = (scmp(var->typeInfo.name, TEXT("Texture")) == 0);
                        BOOL bCubeTexture   = (scmp(var->typeInfo.name, TEXT("CubeTexture")) == 0);
                        BOOL bMaterial      = (scmp(var->typeInfo.name, TEXT("Material")) == 0);

                        if(bSound || bTexture || bCubeTexture || bMaterial)
                        {
                            String strResource;
                            s << strResource;

                            if(strResource.IsValid())
                            {
                                if(bSound)
                                    *((Sound**)GetVarAddress(this, var)) = NewSound(strResource, var->propertyType != PROPERTY_SOUND2D);
                                else if(bTexture)
                                {
                                    if(var->propertyType == PROPERTY_TEXTURE2D)
                                        *((Texture**)GetVarAddress(this, var)) = GetTexture(strResource, FALSE);
                                    else
                                        *((Texture**)GetVarAddress(this, var)) = GetTexture(strResource);
                                }
                                else if(bCubeTexture)
                                    *((CubeTexture**)GetVarAddress(this, var)) = GetCubeTexture(strResource);
                                else if(bMaterial)
                                    *((Material**)GetVarAddress(this, var)) = GetMaterial(strResource);
                            }
                        }
                    }
                }
                else if(size != var->typeInfo.size)
                    s.Seek(size, SERIALIZE_SEEK_CURRENT);
                else
                    s.Serialize(GetVarAddress(this, var), size);
            }
        }

        if(objScript)
        {
            delete objScript;
            objScript = NULL;
        }

        s << num;
        if(num)
        {
            objScript = new ObjectScript;

            objScript->ScriptList.SetSize(num);
            for(i=0; i<num; i++)
                s << objScript->ScriptList[i];

            s << objScript->FunctionIDs;

            String errorList;
            objScript->Compile(this, errorList);
        }
    }
    else //saving
    {
        int num = classDef->NumVariables();
        s << num;

        for(i=0; i<num; i++)
        {
            PropertyVariable *var = classDef->GetVariable(i);

            int varSize = 0;
            DWORD overWritePos = 0;

            s << var->name << String(var->typeInfo.name);
            overWritePos = s.GetPos();
            s << varSize;

            if(var->typeInfo.type == DataType_String)
            {
                String& str = *(String*)GetVarAddress(this, var);
                s << str;
            }
            else if(var->typeInfo.type == DataType_Object)
            {
                if(var->flags & VAR_PROPERTY)
                {
                    BOOL bSound         = (scmp(var->typeInfo.name, TEXT("Sound")) == 0);
                    BOOL bTexture       = (scmp(var->typeInfo.name, TEXT("Texture")) == 0);
                    BOOL bCubeTexture   = (scmp(var->typeInfo.name, TEXT("CubeTexture")) == 0);
                    BOOL bMaterial      = (scmp(var->typeInfo.name, TEXT("Material")) == 0);

                    if(bSound || bTexture || bCubeTexture || bMaterial)
                    {
                        if(bSound)
                        {
                            Sound *val = *((Sound**)GetVarAddress(this, var));

                            if(!val)
                                s << String();
                            else
                            {
                                String resName = val->GetResourceName();
                                s << resName;
                            }
                        }
                        else if(bTexture || bCubeTexture)
                        {
                            BaseTexture *val = *((BaseTexture**)GetVarAddress(this, var));

                            if(!val)
                                s << String();
                            else
                            {
                                String name = RM->GetTextureName(val);
                                s << name;
                            }
                        }
                        else if(bMaterial)
                        {
                            Material *val = *((Material**)GetVarAddress(this, var));

                            if(!val)
                                s << String();
                            else
                            {
                                String name = RM->GetMaterialName(val);
                                s << name;
                            }
                        }
                    }
                }
            }
            else
                s.Serialize(GetVarAddress(this, var), var->typeInfo.size);

            varSize = s.GetPos()-overWritePos-sizeof(varSize);

            s.Seek(overWritePos, SERIALIZE_SEEK_START);
            s << varSize;
            s.Seek(0, SERIALIZE_SEEK_END);
        }

        if(objScript)
        {
            num = objScript->ScriptList.Num();
            s << num;

            for(i=0; i<objScript->ScriptList.Num(); i++)
                s << objScript->ScriptList[i];

            s << objScript->FunctionIDs;
        }
        else
        {
            num = 0;
            s << num;
        }
    }

    traceOut;
}

void* Object::GetScriptPtr(CTSTR lpName)
{
    ClassDefinition *classDef = topClass->GetScriptClass();

    PropertyVariable *var = classDef->GetVariable(lpName);
    if(!var) return NULL;

    return GetVarAddress(this, var);
}


FrameObject::FrameObject()
{
    if(pLastFrameObject)
    {
        pPrevFrameObject = pLastFrameObject;
        pLastFrameObject->pNextFrameObject = this;
    }
    else 
    {
        pPrevFrameObject = pNextFrameObject = NULL;
        pFirstFrameObject = this;
    }

    pLastFrameObject = this;
}

FrameObject::~FrameObject()
{
    if(pPrevFrameObject)
        pPrevFrameObject->pNextFrameObject = pNextFrameObject;
    if(pNextFrameObject)
        pNextFrameObject->pPrevFrameObject = pPrevFrameObject;

    if(!pPrevFrameObject && !pNextFrameObject)
        pFirstFrameObject = pLastFrameObject = NULL;
    else if(pLastFrameObject == this)
        pLastFrameObject = pPrevFrameObject;
    else if(pFirstFrameObject == this)
        pFirstFrameObject = pNextFrameObject;
}

int FrameObject::SafeRelease()
{
    if(!refCounter)
    {
        SafeDestroy();
        return 0;
    }
    else
        return refCounter--;
}

void FrameObject::SafeDestroy()
{   
    bMarkedForDestruction = 1;
}


Object *Object::CreateFactoryObject(CTSTR lpClass, BOOL bInitialize)
{
    assert(lpClass);

    if(!lpClass)
        Log(TEXT("Warning: CreateFactoryObject - Dear lord you tried to create a class without specifying a class string?  ...seriously, are you mad?"));

    TSTR lpTemp = sdup(lpClass);
    TSTR lpSeperator = schr(lpTemp, ':');

    if(lpSeperator)
    {
        *lpSeperator = 0;
        lpClass = lpSeperator+1;

        if(level)
        {
            if(!level->LoadLevelModule(lpTemp))
                AppWarning(TEXT("Warning: CreateFactoryObject - Couldn't Find library %s for class %s"), lpTemp, lpClass);
        }
        else if(!LoadGameModule(lpTemp))
            AppWarning(TEXT("Warning: CreateFactoryObject - Couldn't Find library %s for class %s"), lpTemp, lpClass);
    }

    Class *cls = FindClass(lpClass);

    Free(lpTemp);

    if(!cls)
        return NULL;

    Object *obj = cls->Create(bInitialize);

    return obj;
}

Object *Object::InitializeObject(BOOL bCallScriptConstructors)
{
    if(!topClass)
        topClass = this->_GetInteralClass();
    if(bCallScriptConstructors)
        topClass->CallScriptConstructors(this);

    this->Init();

    return this;
}


void Object::DestroyObject(Object* obj)
{
    if(obj)
    {
        obj->Destroy();

        Class* classInfo = obj->GetObjectClass();

        if(classInfo->bPureScriptClass)
        {
            ClassDefinition *classDef = classInfo->scriptClass;

            assert(classDef);

            do
            {
                for(int i=0; i<classDef->Variables.Num(); i++)
                {
                    DefaultVariable *var = &classDef->Variables[i];
                    Scripting->FreeVarData(GetVarAddress(obj, var), var);
                }

                classDef = classDef->Parent;
            }while(classDef->classData->bPureScriptClass);
        }

        delete obj->objScript;
        delete obj;
    }
}

int Object::Release()
{
    if(!refCounter)
    {
        DestroyObject(this);
        return 0;
    }
    else
        return refCounter--;
}

//-----------------------------------------------------------------------------------------------------------

SerializerObject::SerializerObject(Serializer *sIn, BOOL bDestroyIn)
{
    s = sIn;
    bDestroy = bDestroyIn;
}

SerializerObject::~SerializerObject()
{
    if(bDestroy) delete s;
}

//-----------------------------------------------------------------------------------------------------------

BOOL ObjectScript::Compile(Object *obj, String &errorList, DWORD id)
{
    traceIn(ObjectScript::Compile);

    int i, j, funcCount;
    Class *cls = obj->GetObjectClass();

    if(id != INVALID)
    {
        assert(ScriptList.Num() > id);
        if(ScriptList.Num() <= id)
            return FALSE;

        if(id >= Functions.Num())
            Functions.SetSize(id+1);
        else
            Functions[id].FreeData();

        //----------------

        funcCount = id+1;
        i = id;
    }
    else
    {
        for(i=0; i<Functions.Num(); i++)
            Functions[i].FreeData();
        Functions.Clear();

        Functions.SetSize(ScriptList.Num());

        //----------------

        funcCount = ScriptList.Num();
        i = 0;
    }

    Compiler compiler;
    BOOL bCompileFailure = FALSE;

    for(; i<funcCount; i++)
    {
        String &script = ScriptList[i];
        FunctionDefinition *funcDef = &Functions[i];

        FunctionDefinition *copyFunc = cls->GetTopmostFunction(FunctionIDs[i]);
        assert(copyFunc);
        if(!copyFunc)
            continue;

        funcDef->name = copyFunc->name;
        funcDef->flags = copyFunc->flags | FUNC_OBJECTSCOPE;
        funcDef->classContext = copyFunc->classContext;
        funcDef->funcOffset = copyFunc->funcOffset;
        funcDef->returnType = copyFunc->returnType;
        funcDef->Params.SetSize(copyFunc->Params.Num());

        for(j=0; j<copyFunc->Params.Num(); j++)
        {
            DefaultVariable &copyVar = copyFunc->Params[j];
            DefaultVariable &var = funcDef->Params[j];

            //not doing a mcpy because name is a String, besides this function rarely gets called.
            var.flags = copyVar.flags;
            var.name = copyVar.name;
            var.numElements = copyVar.numElements;
            var.offset = copyVar.offset;
            var.scope = copyVar.scope;
            var.typeInfo = copyVar.typeInfo;
        }

        BOOL bFailed = !compiler.CompileObjectScopeFunction(*funcDef, script, obj, errorList);
        bCompileFailure |= bFailed;

        if(bFailed)
            funcDef->ByteCode.Clear();
    }

    return !bCompileFailure;

    traceOut;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Script

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


#include "Base.h"
#include "ScriptDefs.h"
#include "ScriptCompiler.h"
#include "ScriptBytecode.h"


ScriptSystem* Scripting = NULL;



void CallStruct::ClearParam(int param)
{
    CallParam &callParam = Params[param+1];

    if(callParam.type == DataType_String)
    {
        String &str = *(String*)callParam.lpData;
        str.Clear();
    }
    Free(callParam.lpData);

    callParam.lpData = NULL;
    callParam.type = DataType_Void;
}

void CallStruct::SetStruct(int param, const void* lpData, UINT size)
{
    CallParam &callParam = Params[param+1];

    Free(callParam.lpData);

    callParam.type = DataType_Struct;

    LPVOID &lpStructData = callParam.lpData;
    lpStructData = Allocate(size);
    mcpy(lpStructData, lpData, size);
}

const List<BYTE>& CallStruct::GetList(int param) const
{
    return *(const List<BYTE>*)Params[param+1].lpData;
}

List<BYTE>& CallStruct::GetListOut(int param)
{
    return *(List<BYTE>*)Params[param+1].lpData;
}


BOOL          CallStruct::GetBool     (int param) const {return *(BOOL*)Params[param+1].lpData;}
int           CallStruct::GetInt      (int param) const {return *(int*)Params[param+1].lpData;}
float         CallStruct::GetFloat    (int param) const {return *(float*)Params[param+1].lpData;}
HANDLE        CallStruct::GetHandle   (int param) const {return *(HANDLE*)Params[param+1].lpData;}
Object*       CallStruct::GetObject   (int param) const {return *(Object**)Params[param+1].lpData;}
const String& CallStruct::GetString   (int param) const {return *(const String*)Params[param+1].lpData;}
const BYTE&   CallStruct::GetStruct   (int param) const {return *(const LPBYTE)Params[param+1].lpData;}

BOOL&         CallStruct::GetBoolOut     (int param)    {return *(BOOL*)Params[param+1].lpData;}
int&          CallStruct::GetIntOut      (int param)    {return *(int*)Params[param+1].lpData;}
float&        CallStruct::GetFloatOut    (int param)    {return *(float*)Params[param+1].lpData;}
HANDLE&       CallStruct::GetHandleOut   (int param)    {return *(HANDLE*)Params[param+1].lpData;}
Object*&      CallStruct::GetObjectOut   (int param)    {return *(Object**)Params[param+1].lpData;}
String&       CallStruct::GetStringOut   (int param)    {return *(String*)Params[param+1].lpData;}
BYTE&         CallStruct::GetStructOut   (int param)    {return *(LPBYTE)Params[param+1].lpData;}


#define ResizeAndSetParam(typethingy, newval, datatype) \
    ClearParam(param);\
    *(typethingy*)(Params[param+1].lpData = Allocate(sizeof(typethingy))) = newval; \
    Params[param+1].type = datatype;


void CallStruct::SetBool     (int param,       BOOL      val)   {ResizeAndSetParam(BOOL,    val, DataType_Integer);}
void CallStruct::SetInt      (int param,       int       val)   {ResizeAndSetParam(int,     val, DataType_Integer);}
void CallStruct::SetFloat    (int param,       float     val)   {ResizeAndSetParam(float,   val, DataType_Float);}
void CallStruct::SetHandle   (int param,       HANDLE    val)   {ResizeAndSetParam(HANDLE,  val, DataType_Float);}
void CallStruct::SetObject   (int param,       Object*   val)   {ResizeAndSetParam(Object*, val, DataType_Object);}


void CallStruct::SetString(int param, CTSTR val)
{
    CallParam &callParam = Params[param+1];

    if(callParam.type != DataType_String)
    {
        Free(callParam.lpData);
        callParam.lpData = new String;
        callParam.type = DataType_String;
    }

    *(String*)callParam.lpData = val;
}

inline BOOL CallStruct::IsValid(int param) const
{
    return ((param+1) < Params.Num()) ? (Params[param+1].lpData != NULL) : FALSE;
}

inline BOOL CallStruct::HasParam(int param) const
{
    return (param+1) < Params.Num();
}

inline void CallStruct::SetNumParams(int num)
{
    if((num+1) < Params.Num())
    {
        for(int i=num+1; i<Params.Num(); i++)
            ClearParam(i-1);
    }

    Params.SetSize(num+1);
}

inline int CallStruct::NumParams() const {return Params.Num()-1;}

inline CallStruct::~CallStruct()
{
    for(int i=0; i<Params.Num(); i++)
        ClearParam(i-1);
    Params.Clear();
}

//--------------------------------------------------------

struct ScriptParam
{
    String strName;
    String strType;
    VariableType type;

    inline ~ScriptParam() {FreeData();}
    inline void FreeData()
    {
        strType.Clear();
        strName.Clear();
    }
};

struct ScriptReturn
{
    String strType;
    VariableType type;

    inline ~ScriptReturn() {FreeData();}
    inline void FreeData()
    {
        strType.Clear();
    }
};

#undef ResizeAndSetParam

//--------------------------------------------------------

ScriptSystem::ScriptSystem()
{
    Scripting = this;
    AddTypeFunctions();
}

ScriptSystem::~ScriptSystem()
{
    traceIn(ScriptSystem::~ScriptSystem);

    int i,j;
    for(j=0; j<Modules.Num(); j++)
    {
        ModuleScriptData &module = Modules[j];

        for(i=0; i<module.GlobalEnumList.Num(); i++)
            module.GlobalEnumList[i].FreeData();
        module.GlobalEnumList.Clear();

        for(i=0; i<module.GlobalStructList.Num(); i++)
            module.GlobalStructList[i].FreeData();
        module.GlobalStructList.Clear();

        for(i=0; i<module.GlobalFunctionList.Num(); i++)
            module.GlobalFunctionList[i].FreeData();
        module.GlobalFunctionList.Clear();

        for(i=0; i<module.GlobalVariableList.Num(); i++)
            module.GlobalVariableList[i].FreeData();
        module.GlobalVariableList.Clear();

        module.strModule.Clear();
    }

    for(i=0; i<GlobalDefineList.Num(); i++)
        GlobalDefineList[i].FreeData();
    GlobalDefineList.Clear();

    for(i=0; i<ScriptTypes.Num(); i++)
        ScriptTypes[i].FreeData();

    Class *curClass = Class::FirstClass();
    Class *lastClass = NULL;
    do
    {
        delete lastClass;
        delete curClass->scriptClass;
        curClass->scriptClass = NULL;
        lastClass = curClass->bFreeManually ? curClass : NULL;
    } while(curClass = curClass->lpNext);

    delete lastClass;

    Scripting = NULL;

    traceOut;
}

FunctionDefinition* ScriptSystem::GetGlobalFunction(CTSTR lpName, CTSTR lpModule)
{
    for(int moduleID=0; moduleID<Modules.Num(); moduleID++)
    {
        ModuleScriptData &module = Modules[moduleID];

        if(lpModule && !module.strModule.CompareI(lpModule))
            continue;

        for(int i=0; i<module.GlobalFunctionList.Num(); i++)
        {
            if(module.GlobalFunctionList[i].name.Compare(lpName))
                return &module.GlobalFunctionList[i];
        }
    }

    return NULL;
}

FunctionDefinition* ScriptSystem::GetNextGlobalFunction(CTSTR lpName, FunctionDefinition *curFunc, CTSTR lpModule)
{
    if(!curFunc)
        return GetGlobalFunction(lpName, lpModule);

    BOOL bFound = FALSE;
    for(int moduleID=0; moduleID<Modules.Num(); moduleID++)
    {
        ModuleScriptData &module = Modules[moduleID];

        if(lpModule && !module.strModule.CompareI(lpModule))
            continue;

        for(int i=0; i<module.GlobalFunctionList.Num(); i++)
        {
            if(bFound)
            {
                if(module.GlobalFunctionList[i].name.Compare(lpName))
                    return &module.GlobalFunctionList[i];
            }
            else
            {
                if(&module.GlobalFunctionList[i] == curFunc)
                    bFound = TRUE;
            }
        }
    }

    return NULL;
}

EnumDefinition* ScriptSystem::GetEnumDef(CTSTR lpName, CTSTR lpModule)
{
    for(int moduleID=0; moduleID<Modules.Num(); moduleID++)
    {
        ModuleScriptData &module = Modules[moduleID];

        if(lpModule && !module.strModule.CompareI(lpModule))
            continue;

        for(int i=0; i<module.GlobalEnumList.Num(); i++)
        {
            if(module.GlobalEnumList[i].name.Compare(lpName))
                return &module.GlobalEnumList[i];
        }
    }

    return NULL;
}

DefineDefinition* ScriptSystem::GetDefineDef(CTSTR lpName)
{
    for(int i=0; i<GlobalDefineList.Num(); i++)
    {
        if(GlobalDefineList[i].name.Compare(lpName))
            return &GlobalDefineList[i];
    }

    return NULL;
}

StructDefinition* ScriptSystem::GetStruct(CTSTR lpName, CTSTR lpModule)
{
    for(int moduleID=0; moduleID<Modules.Num(); moduleID++)
    {
        ModuleScriptData &module = Modules[moduleID];

        if(lpModule && !module.strModule.CompareI(lpModule))
            continue;

        for(int i=0; i<module.GlobalStructList.Num(); i++)
        {
            if(module.GlobalStructList[i].name.Compare(lpName))
                return &module.GlobalStructList[i];
        }
    }

    return NULL;
}

DefaultVariable* ScriptSystem::GetGlobalVariable(CTSTR lpName, CTSTR lpModule)
{
    for(int moduleID=0; moduleID<Modules.Num(); moduleID++)
    {
        ModuleScriptData &module = Modules[moduleID];

        if(lpModule && !module.strModule.CompareI(lpModule))
            continue;

        for(int i=0; i<module.GlobalVariableList.Num(); i++)
        {
            if(module.GlobalVariableList[i].name.Compare(lpName))
                return &module.GlobalVariableList[i];
        }
    }

    return NULL;
}

FunctionDefinition* ScriptSystem::GetOperator(CTSTR lpOperator, TypeInfo &srcType, TypeInfo *targetType, CTSTR lpModule)
{
    for(int h=0; h<Modules.Num(); h++)
    {
        ModuleScriptData &module = Modules[h];

        if(lpModule && !module.strModule.CompareI(lpModule))
            continue;

        for(unsigned int i=0; i<module.GlobalFunctionList.Num(); i++)
        {
            FunctionDefinition &funcDef = module.GlobalFunctionList[i];
            if(funcDef.name == lpOperator)
            {
                if(targetType == NULL)
                {
                    if((funcDef.Params.Num() == 1) && (funcDef.Params[0].typeInfo == srcType))
                        return &funcDef;
                }
                else
                {
                    if((funcDef.Params.Num() == 2) && (funcDef.Params[0].typeInfo == srcType) && (funcDef.Params[1].typeInfo == *targetType))
                        return &funcDef;
                }
            }
        }
    }

    return NULL;
}

ClassDefinition* ScriptSystem::GetClassDef(CTSTR lpName, CTSTR lpModule)
{
    Class* classInfo = FindClass(lpName, lpModule);
    if(!classInfo)
        return NULL;

    return classInfo->scriptClass;
}


BOOL ScriptSystem::LoadModuleScriptData(CTSTR lpModule, String &errorList, BOOL bGenerateOutput, BOOL bGenerateHeaders)
{
    traceIn(ScriptSystem::LoadModuleScriptData);

    String searchDir;
    searchDir << TEXT("data/") << lpModule << TEXT("/script/*.xscript");

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(searchDir, ofd);

    if(!hFind)
        return TRUE;

    Compiler compiler;

    StringList FileList;

    XFile scriptFile;

    compiler.bGeneratingBytecodeText = bGenerateOutput;

    if(bGenerateOutput)
    {
        String directories;
        directories << TEXT("ScriptOutput");
        OSCreateDirectory(directories);
        OSCreateDirectory(directories << TEXT("/") << lpModule);
        OSCreateDirectory(directories << TEXT("/Bytecode"));
    }

    StringList fileDataList;

    do
    {
        FileList << ofd.fileName;

        String strFile;
        strFile << TEXT("data/") << lpModule << TEXT("/script/") << ofd.fileName;
        if(!scriptFile.Open(strFile, XFILE_READ, XFILE_OPENEXISTING))
            AppWarning(TEXT("Could not open script file '%s'"), ofd.fileName);
        else
        {
            scriptFile.ReadFileToString(*fileDataList.CreateNew());
            scriptFile.Close();
        }
    }while(OSFindNextFile(hFind, ofd));

    OSFindClose(hFind);

    for(int i=0; i<fileDataList.Num(); i++)
    {
        compiler.SetCodeStart(fileDataList[i]);
        if(!compiler.ParsePreprocessor(fileDataList[i]))
            compiler.AddError(TEXT("'endif' preprocessor directive not found"));
        compiler.SetCodeStart(fileDataList[i]);
        compiler.CompileStage(lpModule, 0, errorList, FileList[i]);
    }

    for(int i=0; i<fileDataList.Num(); i++)
    {
        compiler.SetCodeStart(fileDataList[i]);
        compiler.CompileStage(lpModule, 1, errorList, FileList[i]);
    }

    compiler.InitializeStructures();
    compiler.InitializeClasses();
    compiler.InitializeClassFunctions();

    for(int i=0; i<fileDataList.Num(); i++)
    {
        compiler.SetCodeStart(fileDataList[i]);
        compiler.CompileStage(lpModule, 2, errorList, FileList[i]);
    }

    if(bGenerateHeaders)
    {
        String directories;
        directories << TEXT("ScriptOutput");
        OSCreateDirectory(directories);
        OSCreateDirectory(directories << TEXT("/") << lpModule);
        OSCreateDirectory(directories + TEXT("/Headers"));
        OSCreateDirectory(directories + TEXT("/Source"));

        for(int i=0; i<fileDataList.Num(); i++)
        {
            compiler.SetCodeStart(fileDataList[i]);
            compiler.CompileStage(lpModule, 3, errorList, FileList[i]);
            compiler.SaveCurrentSource();
        }

        compiler.compilerError = &errorList;
        compiler.curModule = lpModule;
        compiler.CreateMainSourceFiles();
    }

    FileList.Clear();

    return compiler.errorCount == 0;

    traceOut;
}

void ScriptSystem::UnloadModuleScriptData(CTSTR lpModule)
{
    traceIn(ScriptSystem::UnloadModuleScriptData);

    int i,j;
    for(j=0; j<Modules.Num(); j++)
    {
        ModuleScriptData &module = Modules[j];

        if(!module.strModule.CompareI(lpModule))
            continue;

        for(i=0; i<module.GlobalEnumList.Num(); i++)
            module.GlobalEnumList[i].FreeData();
        module.GlobalEnumList.Clear();

        for(i=0; i<module.GlobalStructList.Num(); i++)
            module.GlobalStructList[i].FreeData();
        module.GlobalStructList.Clear();

        for(i=0; i<module.GlobalFunctionList.Num(); i++)
            module.GlobalFunctionList[i].FreeData();
        module.GlobalFunctionList.Clear();

        for(i=0; i<module.GlobalVariableList.Num(); i++)
            module.GlobalVariableList[i].FreeData();
        module.GlobalVariableList.Clear();

        module.strModule.Clear();
    }

    Class *curClass = Class::FirstClass();
    Class *lastClass = NULL;
    do
    {
        if(scmpi(curClass->module, lpModule) != 0)
            continue;

        delete lastClass;
        lastClass = NULL;

        delete curClass->scriptClass;
        curClass->scriptClass = NULL;
        lastClass = curClass->bFreeManually ? curClass : NULL;
    } while(curClass = curClass->lpNext);

    delete lastClass;

    traceOut;
}

ModuleScriptData *ScriptSystem::GetModule(CTSTR lpModule, BOOL bCreateNew)
{
    for(int i=0; i<Modules.Num(); i++)
    {
        if(Modules[i].strModule.CompareI(lpModule))
            return &Modules[i];
    }

    if(bCreateNew)
    {
        ModuleScriptData *newModule = Modules.CreateNew();
        newModule->strModule = lpModule;
        newModule->moduleHash = StringCRC16I(lpModule);
        return newModule;
    }
    return NULL;
}

FunctionDefinition *ScriptSystem::GetFunctionByID(int id)
{
    int moduleHash = HIWORD(id);
    int funcID = LOWORD(id);

    for(int i=0; i<Modules.Num(); i++)
    {
        ModuleScriptData &module = Modules[i];

        if(module.moduleHash == moduleHash)
            return &module.GlobalFunctionList[funcID];
    }

    AppWarning(TEXT("Invalid module hash specified for global function.  Check to make sure your LoadModuleNatives function is up to date."));

    return NULL;
}

void* ScriptSystem::GetGlobalVariableAddress(int id)
{
    int moduleHash = HIWORD(id);
    int varID = LOWORD(id);

    for(int i=0; i<Modules.Num(); i++)
    {
        ModuleScriptData &module = Modules[i];

        if(module.moduleHash == moduleHash)
            return module.GlobalVariableList[varID].DefaultParamData.Array();
    }

    AppWarning(TEXT("Invalid module hash specified for global variable.  Check to make sure your LoadModuleNatives function is up to date."));

    return NULL;
}

void ScriptSystem::DefineNativeGlobal(NATIVECALLBACK func, int id)
{
    FunctionDefinition *funcDef = GetFunctionByID(id);
    if(!funcDef)
        return;
    funcDef->NativeFunc = func;
}

void ScriptSystem::DefineNativeStructMember(CTSTR lpName, OBJECTCALLBACK func, int id)
{
    StructDefinition *structDef = GetStruct(lpName);
    structDef->Functions[id].ObjectFunc = func;
}

void ScriptSystem::DefineNativeStructStaticMember(CTSTR lpName, NATIVECALLBACK func, int id)
{
    StructDefinition *structDef = GetStruct(lpName);
    structDef->Functions[id].NativeFunc = func;
}


BOOL ScriptSystem::CallFunction(Object *obj, CTSTR functionName, CallStruct &cs)
{
    traceIn(ScriptSystem::CallFunction);

    ClassDefinition *classDef = NULL;
    
    if(obj)
    {
        classDef = obj->GetObjectClass()->GetScriptClass();
        if(!classDef)
            return FALSE;
    }

    FunctionDefinition *funcDef = NULL;
    int i;

    BOOL bFoundFunction = FALSE;
    while(funcDef = (classDef) ? classDef->GetNextFunction(functionName, funcDef) : GetNextGlobalFunction(functionName, funcDef))
    {
        if(funcDef->Params.Num() < cs.NumParams())
            continue;

        BOOL bMatch=TRUE;

        for(i=0; i<funcDef->Params.Num(); i++)
        {
            if(!cs.IsValid(i))
            {
                if(!(funcDef->Params[i].flags & VAR_OUT))
                {
                    bMatch = FALSE;
                    break;
                }
            }
            else if(funcDef->Params[i].typeInfo.type != cs.Params[i+1].type)
            {
                bMatch = FALSE;
                break;
            }
        }

        if(!bMatch)
            continue;

        //found the function, now just set up any default parameters

        cs.SetNumParams(funcDef->Params.Num());

        for(i=0; i<funcDef->Params.Num(); i++)
        {
            DefaultVariable &var = funcDef->Params[i];
            if(!cs.IsValid(i))
            {
                BufferInputSerializer sDefault(var.DefaultParamData);

                if(var.typeInfo.type == DataType_String)
                {
                    String str;
                    sDefault << str;
                    cs.SetString(i, str);
                }
                else
                {
                    cs.Params[i+1].lpData = Allocate(var.typeInfo.size);
                    sDefault.Serialize(cs.Params[i+1].lpData, var.typeInfo.size);
                }
            }
        }

        bFoundFunction = TRUE;
        break;
    }

    if(!bFoundFunction)
        return FALSE;

    return funcDef->Call(obj, cs);

    traceOut;
}

struct TempListThingy
{
    BYTE *array;
    unsigned int num;
};

//these functions will recursively duplicate all data within any variables or lists and any data recusively within them and any data recusively in them and any--...
void ScriptSystem::DuplicateTypeData(LPVOID lpAddr, TypeInfo &type, TypeInfo *subType)
{
    LPBYTE lpBytes = (LPBYTE)lpAddr;
    if(type.type == DataType_String)
    {
        String &str = *((String*)lpBytes);
        String newStr = str;
        mcpy(&str, &newStr, sizeof(String));
        zero(&newStr, sizeof(String));
    }
    else if(type.type == DataType_Struct)
    {
        StructDefinition *structDef = GetStruct(type.name);

        while(structDef)
        {
            for(int i=0; i<structDef->Variables.Num(); i++)
            {
                Variable *subVar = &structDef->Variables[i];
                DuplicateVarData(lpBytes+subVar->offset, subVar);
            }

            structDef = structDef->Parent;
        }
    }
    else if(type.type == DataType_List)
    {
        assert(subType);

        int typeSize = subType->size;

        List<BYTE> &oldList = *((List<BYTE>*)lpBytes);
        TempListThingy newData;
        newData.array = (BYTE*)Allocate(oldList.Num()*typeSize);
        newData.num = oldList.Num();

        mcpy(newData.array, oldList.Array(), oldList.Num()*typeSize);
        mcpy(&oldList, &newData, sizeof(TempListThingy));

        for(int i=0; i<newData.num; i++)
            DuplicateTypeData(newData.array+(typeSize*i), *subType);
    }
}

void ScriptSystem::DuplicateStructData(LPVOID lpAddr, StructDefinition *structDef)
{
    while(structDef)
    {
        LPBYTE lpBytes = (LPBYTE)lpAddr;
        for(int i=0; i<structDef->Variables.Num(); i++)
        {
            Variable *subVar = &structDef->Variables[i];
            DuplicateVarData(lpBytes+subVar->offset, subVar);
        }

        structDef = structDef->Parent;
    }
}

void ScriptSystem::DuplicateVarData(LPVOID lpAddr, Variable *var)
{
    LPBYTE lpBytes = (LPBYTE)lpAddr;

    int numElements = MAX(1, var->numElements);

    for(int i=0; i<numElements; i++)
        DuplicateTypeData(lpBytes+(i*var->typeInfo.size), var->typeInfo, &var->subTypeInfo);
}

//these functions will recursively free all data within any variables or lists and any data recusively within them and any data recusively in them and any--...
void ScriptSystem::FreeTypeData(LPVOID lpAddr, TypeInfo &type, TypeInfo *subType)
{
    LPBYTE lpBytes = (LPBYTE)lpAddr;
    if(type.type == DataType_String)
    {
        String &str = *((String*)lpBytes);
        str.Clear();
    }
    else if(type.type == DataType_Struct)
    {
        StructDefinition *structDef = GetStruct(type.name);

        while(structDef)
        {
            for(int i=0; i<structDef->Variables.Num(); i++)
            {
                Variable *subVar = &structDef->Variables[i];
                FreeVarData(lpBytes+subVar->offset, subVar);
            }

            structDef = structDef->Parent;
        }
    }
    else if(type.type == DataType_List)
    {
        assert(subType);

        List<BYTE> &tempList = *((List<BYTE>*)lpBytes);

        LPBYTE listArray = tempList.Array();
        UINT num = tempList.Num();

        int typeSize = subType->size;

        assert(typeSize);

        for(int i=0; i<num; i++)
            FreeTypeData(listArray+(typeSize*i), *subType);

        tempList.Clear();
    }
}

void ScriptSystem::FreeStructData(LPVOID lpAddr, StructDefinition *structDef)
{
    while(structDef)
    {
        LPBYTE lpBytes = (LPBYTE)lpAddr;
        for(int i=0; i<structDef->Variables.Num(); i++)
        {
            Variable *subVar = &structDef->Variables[i];
            FreeVarData(lpBytes+subVar->offset, subVar);
        }

        structDef = structDef->Parent;
    }
}

void ScriptSystem::FreeVarData(LPVOID lpAddr, Variable *var)
{
    LPBYTE lpBytes = (LPBYTE)lpAddr;

    int numElements = MAX(1, var->numElements);

    for(int i=0; i<numElements; i++)
        FreeTypeData(lpBytes+(i*var->typeInfo.size), var->typeInfo, &var->subTypeInfo);
}


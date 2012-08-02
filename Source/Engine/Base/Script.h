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


#ifndef SCRIPT_HEADER
#define SCRIPT_HEADER


/*========================================================
    Class Script Structs/Macros
=========================================================*/

#define BeginScriptVars(cls)   public:
#define EndScriptVars()

#define NoScriptVars(cls)

#define Declare_Internal_Member(funcname) \
    void funcname(CallStruct &cs);

#define Declare_Internal_StaticMember(funcname) \
    static void ENGINEAPI funcname(CallStruct &cs);


/*========================================================
    Script Structs/Macros
=========================================================*/

enum VariableType
{
    DataType_Integer,
    DataType_Float,

    DataType_List,
    DataType_String,

    DataType_SubType,
    DataType_Null,
    DataType_Any,

    DataType_Type,

    DataType_Void,

    DataType_Handle,

    DataType_Object,
    DataType_Struct,

    DataType_DWORD = 0x7FFFFFFF
};

struct BaseTypeInfo
{
    VariableType type;
    int          size;
    int          align;
};

struct TypeDataInfo : BaseTypeInfo
{
    TCHAR name[64];
};

//----------------------------------------------------------------

#undef GetObject //this was screwing up my function calls

#define Declare_Native_Global(funcname) \
    void ENGINEAPI funcname(CallStruct &cs);

#define Load_Module_Natives() \
    void ENGINEAPI LoadModuleNatives(); \
    static int ThrowAway_Var = Engine::SetNativeDataLoader(LoadModuleNatives);

#define RETURNVAL -1

struct CallParam;

class BASE_EXPORT CallStruct
{
    friend struct FunctionDefinition;
    friend class ScriptSystem;

    List<CallParam> Params;

    void ClearParam(int param);

public:
    inline CallStruct() {SetNumParams(0);}
    inline ~CallStruct();

    BOOL          GetBool     (int param) const;
    int           GetInt      (int param) const;
    float         GetFloat    (int param) const;
    HANDLE        GetHandle   (int param) const;
    Object*       GetObject   (int param) const;
    const String& GetString   (int param) const;
    const BYTE&   GetStruct   (int param) const;

    BOOL&         GetBoolOut     (int param);
    int&          GetIntOut      (int param);
    float&        GetFloatOut    (int param);
    HANDLE&       GetHandleOut   (int param);
    Object*&      GetObjectOut   (int param);
    String&       GetStringOut   (int param);
    BYTE&         GetStructOut   (int param);

    void SetBool     (int param,       BOOL      val);
    void SetInt      (int param,       int       val);
    void SetFloat    (int param,       float     val);
    void SetHandle   (int param,       HANDLE    val);
    void SetObject   (int param,       Object*   val);
    void SetString   (int param,       CTSTR     val);
    void SetStruct   (int param, const void*     data, UINT size);

    const List<BYTE>& GetList(int param) const;
    List<BYTE>& GetListOut(int param);

    inline BOOL         IsValid     (int param) const;
    inline BOOL         HasParam    (int param) const;

    inline void         SetNumParams(int num);
    inline int          NumParams() const;
};


/*=========================================================
    ScriptSystem
==========================================================*/

struct FunctionDefinition;
struct StructDefinition;
struct ClassDefinition;
struct EnumDefinition;
struct DefineDefinition;
struct Variable;
struct DefaultVariable;
struct TypeInfo;
struct ScriptTypeData;

struct ModuleScriptData
{
    WORD moduleHash;

    String strModule;

    List<FunctionDefinition> GlobalFunctionList;
    List<StructDefinition> GlobalStructList;
    List<EnumDefinition> GlobalEnumList;
    List<DefaultVariable> GlobalVariableList;
};

//--------------------------------------------------------

class BASE_EXPORT ScriptSystem
{
    friend class Engine;
    friend class ScriptEditor;
    friend class ScriptCompilerEngine;
    friend class ObjectPropertiesEditor;
    friend class Object;
    friend class Class;
    friend struct ScriptList;
    friend struct Compiler;
    friend struct CodeSegment;
    friend struct StructDefinition;
    friend struct FunctionDefinition;

    List<ModuleScriptData> Modules;
    List<ScriptTypeData> ScriptTypes;

    List<DefineDefinition> GlobalDefineList;

    FunctionDefinition* GetGlobalFunction(CTSTR lpName, CTSTR lpModule=NULL);
    FunctionDefinition* GetNextGlobalFunction(CTSTR lpName, FunctionDefinition *curFunc, CTSTR lpModule=NULL);
    FunctionDefinition* GetOperator(CTSTR lpOperator, TypeInfo &srcType, TypeInfo *targetType, CTSTR lpModule=NULL);
    EnumDefinition* GetEnumDef(CTSTR lpName, CTSTR lpModule=NULL);
    DefineDefinition* GetDefineDef(CTSTR lpName);
    StructDefinition* GetStruct(CTSTR lpName, CTSTR lpModule=NULL);
    DefaultVariable* GetGlobalVariable(CTSTR lpName, CTSTR lpModule=NULL);
    ClassDefinition* GetClassDef(CTSTR lpName, CTSTR lpModule=NULL);

    FunctionDefinition *GetFunctionByID(int id);
    void* GetGlobalVariableAddress(int id);

    BOOL LoadModuleScriptData(CTSTR lpModule, String &errorList, BOOL bGenerateOutput=FALSE, BOOL bGenerateHeaders=FALSE);
    BOOL GenerateHeaders(CTSTR lpModule, String &errorList);
    void UnloadModuleScriptData(CTSTR lpModule);

    void PreprocessFileData(String &fileData);

    void LoadBaseScriptingDefinitions();

    ModuleScriptData *GetModule(CTSTR lpModule, BOOL bCreateNew=FALSE);

    void DuplicateTypeData(LPVOID lpAddr, TypeInfo &type, TypeInfo *subType=NULL);
    void DuplicateStructData(LPVOID lpAddr, StructDefinition *structDef);
    void DuplicateVarData(LPVOID lpAddr, Variable *var);

    void FreeTypeData(LPVOID lpAddr, TypeInfo &type, TypeInfo *subType=NULL);
    void FreeStructData(LPVOID lpAddr, StructDefinition *structDef);
    void FreeVarData(LPVOID lpAddr, Variable *var);

    void AddTypeFunction(CTSTR lpType, CTSTR lpFuncDec, DEFFUNCPROC func);

    void AddTypeFunctions();

public:
    ScriptSystem();
    ~ScriptSystem();

    void DefineNativeGlobal(NATIVECALLBACK func, int id);
    void DefineNativeStructMember(CTSTR lpName, OBJECTCALLBACK func, int id);
    void DefineNativeStructStaticMember(CTSTR lpName, NATIVECALLBACK func, int id);

    BOOL CallFunction(Object *obj, CTSTR functionName, CallStruct &cs);
};


BASE_EXPORT extern ScriptSystem* Scripting;


#endif

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


#ifndef SCRIPTDEFS_HEADER
#define SCRIPTDEFS_HEADER


#define NUMKEYWORDS         17
#define NUMAOPERATORS       12
#define NUMMOPERATORS       11
#define NUMCOPERATORS       11
#define NUMPREFIXOPERATORS  5
#define NUMPOSTFIXOPERATORS 3
#define NUMTYPES            10
#define NUMCLOSINGTOKENS    29

#define VALIDOPS            16
#define VALIDPREFIXOPS      3

static const TCHAR *KeywordNames[NUMKEYWORDS] = {TEXT("return"), TEXT("this"),   TEXT("if"),   TEXT("for"),         TEXT("while"),
                                                 TEXT("do"),     TEXT("switch"), TEXT("case"), TEXT("default"),     TEXT("break"),
                                                 TEXT("super"),  TEXT("null"),   TEXT("cast"), TEXT("static_cast"), TEXT("dynamic_cast"),
                                                 TEXT("create"), TEXT("destroy")};

static const TCHAR *AOperatorNames[NUMAOPERATORS] = {TEXT("="),   TEXT("+="), TEXT("-="), TEXT("/="), TEXT("*="),
                                                     TEXT("|="),  TEXT("~="), TEXT("^="), TEXT("%="), TEXT("<<="),
                                                     TEXT(">>="), TEXT("&=")};

static const TCHAR *MOperatorNames[NUMMOPERATORS] = {TEXT("+"), TEXT("-"), TEXT("/"), TEXT("*"),  TEXT("|"),
                                                     TEXT("~"), TEXT("^"), TEXT("%"), TEXT("<<"), TEXT(">>"),
                                                     TEXT("&")};

static const TCHAR *COperatorNames[NUMCOPERATORS] = {TEXT("=="), TEXT("!="), TEXT("<="), TEXT(">="), TEXT("<"),
                                                     TEXT(">"),  TEXT("||"), TEXT("&&"), TEXT("?"),  TEXT(":"),
                                                     TEXT("!")};

static const TCHAR *PrefixOperatorNames[NUMPREFIXOPERATORS] = {TEXT("++"), TEXT("--"), TEXT("-"), TEXT("!"), TEXT("~")};

static const TCHAR *PostfixOperatorNames[NUMPOSTFIXOPERATORS] = {TEXT("++"), TEXT("--"), TEXT(".")};

static const TCHAR *TypeNames[NUMTYPES] = {TEXT("bool"),   TEXT("icolor"), TEXT("char"), TEXT("int"),   TEXT("float"),
                                           TEXT("string"), TEXT("type"),   TEXT("void"), TEXT("list"),  TEXT("handle")};

static const TCHAR *ClosingTokens[NUMCLOSINGTOKENS] = {TEXT("}"),  TEXT(";"),  TEXT(")"),  TEXT(","),  TEXT("]"),
                                                       TEXT("?"),  TEXT(":"),  TEXT("&&"), TEXT("||"), TEXT("<"),  
                                                       TEXT(">"),  TEXT("<="), TEXT(">="), TEXT("=="), TEXT("!="), 
                                                       TEXT("="),  TEXT("-="), TEXT("+="), TEXT("/="), TEXT("*="), 
                                                       TEXT("<<"), TEXT(">>"), TEXT("-"),  TEXT("+"),  TEXT("/"),  
                                                       TEXT("%"),  TEXT("*"),  TEXT("|"),  TEXT("&")};

static const int TokenPrecedence[NUMCLOSINGTOKENS] = {0,  1,  2,  2,  2,
                                                      3,  3,  4,  5,  6,  
                                                      6,  7,  7,  7,  7,
                                                      8,  9,  9,  10, 10,
                                                      11, 11, 12, 12, 13,
                                                      13, 13, 14, 14};

static const TCHAR *ValidOperators[VALIDOPS] = {TEXT("+"),  TEXT("-"),  TEXT("/"),  TEXT("*"),  TEXT("<<"), TEXT(">>"),
                                                TEXT("=="), TEXT("!="), TEXT("<="), TEXT(">="), TEXT("<"),  TEXT(">"),
                                                TEXT("!"),  TEXT("~"),  TEXT("="),  TEXT("%")};

static const TCHAR *ValidPrefixOperators[VALIDPREFIXOPS] = {TEXT("-"),  TEXT("!"),  TEXT("~")};




enum TokenType
{
    TokenType_Unknown,

    TokenType_Keyword,

    TokenType_AssignmentOperator,
    TokenType_ModifyOperator,
    TokenType_ConditionalOperator,
    TokenType_PrefixOperator,
    TokenType_PostfixOperator,

    TokenType_Type,
    TokenType_Class,
    TokenType_EnumDef,
    TokenType_Struct,

    TokenType_Function,

    TokenType_Variable,

    TokenType_String,

    TokenType_Character,
    TokenType_Enum,
    TokenType_Number,

    TokenType_DWORD = 0x7FFFFFFF
};

enum VariableScope
{
    VarScope_Param,
    VarScope_Local,
    VarScope_Object,
    VarScope_Class,
    VarScope_Struct,
    VarScope_Global,
    VarScope_DWORD = 0x7FFFFFFF
};

struct TypeInfo : BaseTypeInfo
{
    CTSTR name;

    inline TypeInfo() {name = TEXT("void"); type = DataType_Void; size = 0;}

    inline BOOL operator==(const TypeInfo& ti)
    {
        if((type < DataType_Object) && (type == ti.type))
            return TRUE;

        if((type == DataType_Object) && (ti.type == DataType_Object))
        {
            Class *thisClass = FindClass(name);
            Class *thatClass = FindClass(ti.name);

            if(thatClass->_IsOf(thisClass))
                return TRUE;
        }
        return (name == ti.name);
    }

    inline BOOL Compatible(const TypeInfo& ti)
    {
        if((type < DataType_Object) && (type == ti.type))
            return TRUE;

        if((type == DataType_Object) && (ti.type == DataType_Object))
        {
            Class *thisClass = FindClass(name);
            Class *thatClass = FindClass(ti.name);

            if(thatClass->_IsOf(thisClass))
                return TRUE;
        }

        if(type <= DataType_Float && ti.type <= DataType_Float)
            return TRUE;

        if(type != ti.type) return FALSE;

        return (name == ti.name);
    }

    inline BOOL operator!=(const TypeInfo& ti)
    {
        return !(name == ti.name);
    }

    inline TypeInfo& operator=(const TypeInfo& ti)
    {
        name = ti.name;
        type = ti.type;
        size = ti.size;
        return *this;
    }
};


struct CallParam
{
    VariableType type;
    LPVOID       lpData;
};

struct CodeInfo
{
    TypeInfo typeReturned;
    TypeInfo subType;
    UINT inPos, outPos;
    List<BYTE> OutVarData;
    BOOL bErrorsOccured;

    inline CodeInfo() {zero(this, sizeof(CodeInfo)); typeReturned.type = DataType_Void; subType.type = DataType_Void;}
};

class CodeInfoList : public List<CodeInfo>
{
public:
    inline ~CodeInfoList()
    {
        FreeData();
    }

    inline void FreeData()
    {
        for(int i=0; i<num; i++)
            array[i].OutVarData.Clear();
        Clear();
    }
};

struct StructDefinition;

struct Variable
{
    inline void FreeData()
    {
        name.Clear();
    }

    String name;
    TypeInfo typeInfo;
    TypeInfo subTypeInfo; //used for lists
    VariableScope scope;
    StructDefinition *structDef; //this way there's no need to look it up

    unsigned int offset;
    int numElements;
};

#define VAR_OUT      1
#define VAR_DEFAULT  2
#define VAR_PROPERTY 4

struct DefaultVariable : Variable
{
    BYTE flags;
    List<BYTE> DefaultParamData;

    inline void FreeData()
    {
        Variable::FreeData();
        DefaultParamData.Clear();
    }
};

enum
{
    PROPERTY_SCROLLER=1,
    PROPERTY_TEXTURE,
    PROPERTY_SOUND,
    PROPERTY_MUSIC,
    PROPERTY_MATERIAL,
    PROPERTY_SOUND2D,
    PROPERTY_TEXTURE2D
};

struct PropertyVariable : DefaultVariable
{
    String section;
    String description;
    int propertyType;
    union
    {
        struct {float fMin, fMax, fInc;};
        struct {int   iMin, iMax, iInc;};
    };

    inline void FreeData()
    {
        DefaultVariable::FreeData();
        section.Clear();
        description.Clear();
    }
};


enum
{
    FUNC_IMPLEMENTABLE = 1<<0,
    FUNC_INTERNAL      = 1<<1,
    FUNC_OPERATOR      = 1<<2,
    FUNC_STATIC        = 1<<3,
    FUNC_OBJECTSCOPE   = 1<<4,
};

struct FunctionDefinition
{
    ~FunctionDefinition() {FreeData();}

    inline void FreeData()
    {
        DWORD i;

        for(i=0; i<Params.Num(); i++)
            Params[i].FreeData();
        Params.Clear();

        name.Clear();
        ByteCode.Clear();

        for(int i=0; i<LocalVars.Num(); i++)
            LocalVars[i].FreeData();
        LocalVars.Clear();
    }

    String name;

    ClassDefinition  *classContext;
    StructDefinition *structContext;

    unsigned int funcOffset;
    int localVarAlign;
    List<DefaultVariable> Params;
    TypeInfo returnType;
    BYTE flags;

    List<Variable> LocalVars;

    unsigned int LocalVariableStackSize;
    List<BYTE> ByteCode;

    union
    {
        NATIVECALLBACK NativeFunc;
        OBJECTCALLBACK ObjectFunc;
    };

    BOOL Call(Object *obj, CallStruct &callData);

    inline BOOL IsLocation(Object *curObj, CTSTR lpClass, CTSTR lpFunction)
    {
         if(name == lpFunction)
         {
             if(!lpClass || (curObj && scmp(curObj->GetObjectClass()->GetName(), lpClass) == 0))
                 return true;
         }

         return false;
    }

    BOOL FunctionsMatch(FunctionDefinition &funcDef);
};

struct EnumItem
{
    String name;
    int val;
};

struct EnumDefinition
{
    ~EnumDefinition() {FreeData();}

    inline void FreeData()
    {
        DWORD i;

        for(i=0; i<Items.Num(); i++)
            Items[i].name.Clear();
        Items.Clear();

        name.Clear();
    }

    String name;
    List<EnumItem> Items;
};

struct DefineDefinition
{
    String name, val;

    inline void FreeData()
    {
        name.Clear();
        val.Clear();
    }
};

struct StructDefinition
{
    ~StructDefinition() {FreeData();}

    inline void FreeData()
    {
        DWORD i;

        for(i=0; i<Variables.Num(); i++)
            Variables[i].FreeData();
        Variables.Clear();

        for(i=0; i<Functions.Num(); i++)
            Functions[i].FreeData();
        Functions.Clear();

        name.Clear();
    }

    inline TypeInfo GetType()
    {
        TypeInfo structType;
        structType.name = name;
        structType.size = size;
        structType.align = align;
        structType.type = DataType_Struct;
        return structType;
    }

    String name;
    BOOL bDataInitialized;
    int size, pad, align;
    List<Variable> Variables;
    StructDefinition *Parent;
    List<FunctionDefinition> Functions;

    Variable *GetVariableFromPos(DWORD varPosition);
    Variable *GetVariable(const String &variableName);

    BOOL NeedsInitialization()
    {
        for(int i=0; i<Variables.Num(); i++)
        {
            Variable &var = Variables[i];
            if(var.typeInfo.type == DataType_List)
                return TRUE;
            else if(var.typeInfo.type == DataType_String)
                return TRUE;
            else if(var.typeInfo.type == DataType_Struct)
            {
                if(var.structDef->NeedsInitialization())
                    return TRUE;
            }
        }

        return FALSE;
    }

    inline FunctionDefinition* GetFunction(CTSTR lpName, BOOL bThisStructOnly=FALSE)
    {
        for(unsigned int i=0; i<Functions.Num(); i++)
        {
            FunctionDefinition &funcDef = Functions[i];
            if(funcDef.name == lpName)
                return &funcDef;
        }

        if(Parent && !bThisStructOnly)
            return Parent->GetFunction(lpName);

        return NULL;
    }

    inline FunctionDefinition* GetNextFunction(CTSTR lpName, FunctionDefinition* curFunction, BOOL bThisStructOnly=FALSE)
    {
        if(!curFunction)
            return GetFunction(lpName, bThisStructOnly);

        BOOL bCheck = FALSE;
        for(unsigned int i=0; i<Functions.Num(); i++)
        {
            FunctionDefinition &funcDef = Functions[i];
            if(bCheck)
            {
                if(funcDef.name == lpName)
                    return &funcDef;
            }

            if(curFunction == &funcDef)
            {
                curFunction = NULL;
                bCheck = TRUE;
            }
        }

        if(Parent && !bThisStructOnly)
            return Parent->GetNextFunction(lpName, curFunction);

        return NULL;
    }
};

struct ClassDefinition
{
    ~ClassDefinition() {FreeData();}

    inline void FreeData()
    {
        DWORD i;

        for(i=0; i<Variables.Num(); i++)
            Variables[i].FreeData();
        Variables.Clear();

        for(i=0; i<Functions.Num(); i++)
            Functions[i].FreeData();
        Functions.Clear();
    }

    Class *classData;
    ClassDefinition *Parent;
    List<PropertyVariable> Variables;
    List<FunctionDefinition> Functions;
    FunctionDefinition *baseConstructor;

    int variableStartIndex;
    int functionStartIndex;

    inline FunctionDefinition* GetFunction(CTSTR lpName, BOOL bThisClassOnly=FALSE)
    {
        for(unsigned int i=0; i<Functions.Num(); i++)
        {
            FunctionDefinition &funcDef = Functions[i];
            if(funcDef.name == lpName)
                return &funcDef;
        }

        if(classData->Parent && !bThisClassOnly)
            return classData->Parent->scriptClass->GetFunction(lpName);

        return NULL;
    }

    inline FunctionDefinition* GetNextFunction(CTSTR lpName, FunctionDefinition* curFunction, BOOL bThisClassOnly=FALSE)
    {
        if(!curFunction)
            return GetFunction(lpName, bThisClassOnly);

        BOOL bCheck = FALSE;
        for(unsigned int i=0; i<Functions.Num(); i++)
        {
            FunctionDefinition &funcDef = Functions[i];
            if(bCheck)
            {
                if(funcDef.name == lpName)
                    return &funcDef;
            }

            if(curFunction == &funcDef)
                bCheck = TRUE;
        }

        if(Parent && !bThisClassOnly)
            return Parent->GetNextFunction(lpName, curFunction);

        return NULL;
    }

    void SetFunctionClassData(FunctionDefinition &funcDef);
    FunctionDefinition* FindSuper(FunctionDefinition &funcDef);
    inline int NumUniqueFunctions();

    PropertyVariable *GetVariable(CTSTR lpVariableName);
    inline PropertyVariable *GetVariable(int i);
    inline unsigned int GetVariableID(Variable *var);
    inline int NumVariables();
};

inline PropertyVariable *ClassDefinition::GetVariable(int i)
{
    if(Variables.Num())
    {
        if(variableStartIndex <= i)
        {
            for(int j=0; j<Variables.Num(); j++)
            {
                if((variableStartIndex+j) == i)
                    return &Variables[j];
            }
        }
    }

    return (!Parent) ? NULL : Parent->GetVariable(i);
}

inline unsigned int ClassDefinition::GetVariableID(Variable *var)
{
    if(Variables.Num())
    {
        for(int j=0; j<Variables.Num(); j++)
        {
            if(&Variables[j] == var)
                return variableStartIndex+j;
        }
    }

    return (!Parent) ? -1 : Parent->GetVariableID(var);
}

inline int ClassDefinition::NumUniqueFunctions()
{
    int curFuncCount = 0;
    for(int i=0; i<Functions.Num(); i++)
    {
        if(Functions[i].funcOffset >= functionStartIndex)
            ++curFuncCount;
    }

    return curFuncCount;
}

inline int ClassDefinition::NumVariables()
{
    return (!Parent) ? Variables.Num() : (Parent->NumVariables() + Variables.Num());
}

//--------------------------------------------------------

struct BASE_EXPORT ObjectScript
{
    List<UINT> FunctionIDs;
    StringList ScriptList;
    List<FunctionDefinition> Functions;

    ~ObjectScript()
    {
        int i;

        ScriptList.Clear();
        for(i=0; i<Functions.Num(); i++)
            Functions[i].FreeData();
        Functions.Clear();
    }

    BOOL Compile(Object *obj, String &errorList, DWORD id=INVALID);
};


//----------------------------------------------------------------

class DataStack// : public List<BYTE>
{
    BYTE* array;
    UINT num, fullSize;

public:
    inline DataStack()
    {
        array = (BYTE*)Allocate(fullSize = 4096);
        num = 0;
    }

    inline ~DataStack()
    {
        Free(array);
    }

    inline UINT Num() const {return num;}

    inline void Push(int size)
    {
        num += size;
        if(num>fullSize)
        {
            BYTE *newArray = (BYTE*)Allocate(fullSize+4096);
            mcpy(newArray+4096, array, fullSize);

            Free(array);
            array = newArray;
            fullSize += 4096;
        }
    }

    inline void Pop(int size)
    {
        assert(num >= size);

        num -= size;
        UINT sizeOffset = fullSize-num;
        if(sizeOffset >= 4096 && sizeOffset != 0)
        {
            sizeOffset = sizeOffset&0xFFFFF000; //((sizeOffset/4096)*4096)
            fullSize -= sizeOffset;
            BYTE *newArray = (BYTE*)Allocate(fullSize);
            mcpy(newArray, array+sizeOffset, fullSize);

            Free(array);
            array = newArray;
        }
    }

    inline BYTE* Array() {return array+(fullSize-num);}

    inline void PushData(LPVOID lpData, int size)
    {
        Push(size);
        mcpy(Array(), lpData, size);
    }

    inline void PopData(LPVOID lpData, int size)
    {
        mcpy(lpData, Array(), size);
        Pop(size);
    }

    inline void GetData(LPVOID lpData, unsigned int size)
    {
        assert(size <= num);
        mcpy(lpData, Array(), size);
    }

    inline void SetData(LPVOID lpData, unsigned int size)
    {
        assert(size <= num);
        mcpy(Array(), lpData, size);
    }

    inline void PushString(const String &str)
    {
        DWORD length = str.Length();
        if(length)
            PushData((TSTR)str, length*sizeof(TCHAR));
        PushData(&length, sizeof(DWORD));
    }

    inline void PopString(String &str)
    {
        str.Clear();
        DWORD length;
        PopData(&length, sizeof(DWORD));
        if(length)
        {
            str.SetLength(length);
            PopData((TSTR)str, length*sizeof(TCHAR));
        }
    }
};

//----------------------------------------------------------------

typedef void (ENGINEAPI *TYPEFUNCPROC)(LPVOID, CTSTR, DataStack&, DataStack&);

struct TypeFunction
{
    ~TypeFunction() {FreeData();}

    inline void FreeData()
    {
        DWORD i;

        for(i=0; i<Params.Num(); i++)
            Params[i].FreeData();
        Params.Clear();

        name.Clear();
    }

    String name;

    List<DefaultVariable> Params;
    unsigned int funcOffset;
    TypeInfo returnType;

    TYPEFUNCPROC typeFunc;
};

struct ScriptTypeData
{
    inline ~ScriptTypeData() {FreeData();}

    inline void FreeData()
    {
        for(int i=0; i<funcs.Num(); i++)
            funcs[i].FreeData();
        funcs.Clear();
    }

    TypeInfo type;
    List<TypeFunction> funcs;
};


#endif


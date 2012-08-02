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


#ifndef SCRIPTCOMPILER_HEADER
#define SCRIPTCOMPILER_HEADER


struct CodeSegment
{
    FunctionDefinition *functionParent;
    Compiler *curCompiler;

    List<int> LocalVariableIDs;
    List<DWORD> JumpPoints;

    CodeSegment *Parent;

    BOOL NameDefined(const String &token);
    TokenType GetTokenType(const String &token, BOOL bPostfix=TRUE);
    Variable *GetVariable(const String &variableName);
};

struct ErrorInfo
{
    String file;
    int stage, line;
    String error;

    inline void FreeData() {file.Clear(); error.Clear();}
};

struct ErrorInfoList : List<ErrorInfo>
{
    ~ErrorInfoList()
    {
        for(int i=0; i<Num(); i++)
            array[i].FreeData();
    }
};

struct BASE_EXPORT CodeTokenizer
{
    String *compilerError;
    ErrorInfoList errorList;
    int errorCount;
    int warningCount;

    int curStage;

    String curFile;

    String dupString;
    TSTR lpCode, lpTemp;

    inline CodeTokenizer() {errorCount = warningCount = curStage = 0; lpCode = lpTemp = NULL; compilerError = NULL;}
    inline ~CodeTokenizer() {CompileErrors();}

    void SetCodeStart(CTSTR lpCode);
    inline void ResetCodePosition() {lpTemp = lpCode;}

    void AddWarning(const TCHAR *format, ...);
    void AddError(const TCHAR *format, ...);
    void CompileErrors();

    BOOL GetNextToken(String &token, BOOL bPeek=FALSE);
    BOOL GetNextTokenEval(String &token, BOOL *bFloatOccurance=NULL, int curPrecedence=0);
    BOOL IsClosingToken(const String &token, CTSTR lpTokenPriority);

    BOOL PassBracers(TSTR lpCodePos);
    BOOL PassGreaterEqual(TSTR lpCodePos);
    BOOL PassParenthesis(TSTR lpCodePos);
    BOOL PassString(TSTR lpCodePos);
    BOOL PassCharacterThingy(TSTR lpCodePos);
    BOOL GotoToken(CTSTR lpTarget, BOOL bPassToken=FALSE);
    BOOL GotoClosestToken(CTSTR token1, CTSTR token2, BOOL bPassToken=FALSE);
    BOOL GotoClosingToken(CTSTR lpTokenPriority);

    int GetCodePos(TSTR lpPos) {return lpPos-lpCode;}

    static int GetTokenPrecedence(CTSTR lpToken);

    inline static String GetActualString(String &stringToken) {return String::RepresentationToString(stringToken);}
    static BOOL GetActualCharacter(String &stringToken, int &val);
};

enum CompileErrorType
{
    COMPILE_UNRECOVERABLE,
    COMPILE_SUCCESS,
    COMPILE_ERROR
};

struct Compiler : CodeTokenizer
{
    String compileStage;
    String curModule;

    ClassDefinition *curClass;
    StructDefinition *curStruct;
    Object *curObject;

    inline Compiler() {curClass = NULL; curStruct = NULL; enumCount = bGeneratingBytecodeText = 0;}

    List<DWORD> *CurBreakOffsets;
    BOOL bBreaksCurrentlyAllowed;

    //void AddRandomNumber(String &curToken);
    //void AddStartCharacter(String &curToken);

    BOOL ParsePreprocessor(String &fileDataOut, TSTR lpIfStart=NULL, BOOL bTrue=TRUE);

    //------------------------------------
    // Source code output for native functions

    String strNativeClassLinks;
    String strNativeStructLinks;
    String strNativeLinks;
    String strForwards;
    String strIncludes;

    String strCurNativeClassDefs;
    String strCurNativeStructDefs;
    String strCurNativeDefs;
    String strCurClassDefs;
    String strCurClasses;
    String strCurEnums;
    String strCurStructs;
    String strCurNativeDecs;

    int enumCount;
    XFile curSourceFile;

    void CreateSourceComment();
    void CreateSourceClass(ClassDefinition *classDef);
    void CreateSourceStruct(StructDefinition *structDef);
    void CreateSourceEnum(EnumDefinition *enumDef);
    void CreateSourceGlobalNative(FunctionDefinition *func);
    void CreateSourceNativeFuncDef(ClassDefinition *classDef, StructDefinition *structDef, FunctionDefinition *func, String& funcName);
    void SaveCurrentSource();
    BOOL CreateMainSourceFiles();

    //------------------------------------
    // Bytecode debug output

    String curBytecodeOutput;

    TSTR lpLastTextOffset;
    BOOL bGeneratingBytecodeText;
    int  curCodeOffset;

    bool WriteCurrentCodeOffset(FunctionDefinition *func);
    void WriteByteCode(FunctionDefinition *func);

    //------------------------------------
    // Primary compilation functions

    BOOL CompileObjectScopeFunction(FunctionDefinition &functionDef, CTSTR lpScript, Object *obj, String &errorList);

    void InitializeClasses();
    void InitializeStructures();
    void InitializeClassFunctions();
    BOOL CompileStage(CTSTR lpCurModule, int stage, String &errorList, CTSTR lpFile);
    BOOL Compile(String &strErrorList);
    BOOL CompileCode(FunctionDefinition &functionDef, CodeSegment *Parent=NULL, CTSTR lpDefaultEndToken=TEXT(";"), BOOL bSwitch=FALSE);
    BOOL CompileSubCode(FunctionDefinition &functionDef, CodeSegment *Parent, CodeInfo &returnInfo, CTSTR endTokenPriority=TEXT(","), VariableType preferredType=DataType_Void);

    CompileErrorType CompileParams(FunctionDefinition &functionDef, CodeSegment *curCodeSegment, CodeInfoList &paramData);
    CompileErrorType ProcessFuncOrVar(FunctionDefinition &functionDef, CodeSegment *curCodeSegment, Variable *&curVar, BOOL &bDoingSuper, BOOL bThis, BOOL &bWasFunction, BOOL &bLValue, DWORD varStartPos, List<TypeInfo> &popData, TypeInfo &returnType, BOOL bCallingStatic=FALSE);
    CompileErrorType ProcessTypeFunctions(FunctionDefinition &functionDef, CodeSegment *curCodeSegment, Variable *&curVar, BOOL &bDoingSuper, BOOL &bWasFunction, BOOL &bLValue, DWORD varStartPos, List<TypeInfo> &popData, TypeInfo &returnType);
    CompileErrorType ProcessConstructor(FunctionDefinition &functionDef, CodeSegment *curCodeSegment, ClassDefinition *classDef, StructDefinition *structDef, BOOL *lpFoundConstructor=NULL);
    CompileErrorType CallConstructor(FunctionDefinition &functionDef, CodeSegment *curCodeSegment, ClassDefinition *classDef, StructDefinition *structDef, CodeInfoList &paramData, BOOL *lpFoundConstructor=NULL, BOOL bTop=TRUE);

    //------------------------------------
    // Bytecode and other complimentary functions for compilation

    static BOOL GetBaseFloat(String &str)
    {
        if(str[str.Length()-1] == 'f')
        {
            str.SetLength(str.Length()-1);
            return TRUE;
        }

        return DefinitelyFloatString(str);
    }

    void Pop(List<BYTE> &ByteCode, TypeInfo &ti);

    void PushDup(List<BYTE> &ByteCode, TypeInfo &ti);

    void APushDup(List<BYTE> &ByteCode);
    void APush(List<BYTE> &ByteCode, Variable *var, int arrayIndex=0);
    void APushSVar(List<BYTE> &ByteCode, Variable *var, BOOL bTempStack, BOOL bArrayOffset=FALSE);
    void APushCVar(List<BYTE> &ByteCode, ClassDefinition *varClass, Variable *var, BOOL bTempStack, BOOL bArrayOffset=FALSE);
    void APushListItem(List<BYTE> &ByteCode, Variable *var);
    void PushAData(List<BYTE> &ByteCode, TypeInfo &type, BOOL bNoRemove, BOOL bInitialize=TRUE);
    void APopVar(List<BYTE> &ByteCode, Variable *var, BOOL bInitialize);

    void TPop(List<BYTE> &ByteCode, int size);
    void TPopFree(List<BYTE> &ByteCode, TypeInfo &type, TypeInfo *subType=NULL);
    void TPopS(List<BYTE> &ByteCode);
    void TPopTypes(List<BYTE> &ByteCode, List<TypeInfo> &typeList);
    void PushTData(List<BYTE> &ByteCode, TypeInfo &type);

    void TPushStack(List<BYTE> &ByteCode, unsigned int size);
    void TPushZero(List<BYTE> &ByteCode, unsigned int size);
    void PushInt(List<BYTE> &ByteCode, unsigned int intVal);
    void PushFloat(List<BYTE> &ByteCode, float floatVal);
    void PushType(List<BYTE> &ByteCode, TypeDataInfo &typeData);
    void PushString(List<BYTE> &ByteCode, String &stringVal);

    void CallFunc(List<BYTE> &ByteCode, FunctionDefinition &targetFunctionDef);
    void CallStructFunc(List<BYTE> &ByteCode, FunctionDefinition &targetFunctionDef, CTSTR structName);
    void CallStructOp(List<BYTE> &ByteCode, FunctionDefinition &targetFunctionDef, CTSTR structName);
    void CallTypeFunc(List<BYTE> &ByteCode, TypeFunction &targetFunctionDef, Variable *var);
    void CallClassStatic(List<BYTE> &ByteCode, FunctionDefinition &targetFunctionDef, ClassDefinition *classDef);
    void CallClassFunc(List<BYTE> &ByteCode, FunctionDefinition &targetFunctionDef, BOOL bSuper);

    void CreateObj(List<BYTE> &ByteCode, String &className);

    void DoReturn(FunctionDefinition &functionDef);

    void Jump(List<BYTE> &ByteCode, DWORD offset);
    void JumpUndefined(List<BYTE> &ByteCode, DWORD &pos);
    void JumpIf(List<BYTE> &ByteCode, DWORD &pos);
    void JumpIfNot(List<BYTE> &ByteCode, DWORD &pos);
    void JumpOffsetIf(List<BYTE> &ByteCode, DWORD offset);
    void JumpOffsetIfNot(List<BYTE> &ByteCode, DWORD offset);

    void DynamicCast(List<BYTE> &ByteCode, String &strClass);

    BOOL DoOperator(List<BYTE> &ByteCode, String &opName, TypeInfo &srcType, TypeInfo &targetType, BOOL &bCalledFunction);

    void DoInstruction(List<BYTE> &ByteCode, BYTE instruction);

    void ProcessFunctionParamters(FunctionDefinition &functionDef, List<DefaultVariable> &funcParams, CodeInfoList &paramData);

    //------------------------------------
    // Type/variable/function/token information functions

    static TokenType GetTokenType(ClassDefinition *classDef, StructDefinition *structDef, const String &token, BOOL bPostfix=TRUE);
    static VariableType GetVariableType(ClassDefinition *classDef, StructDefinition *structDef, CTSTR variableType);

    static BOOL GetTypeInfo(CTSTR lpType, TypeInfo& ti);

    static int GetEnumVal(CTSTR lpName);

    static BOOL NameDefined(ClassDefinition *classDef, StructDefinition *structDef, CTSTR lpName, BOOL bFunction=FALSE, CTSTR lpModule=NULL);

    static FunctionDefinition *GetFunction(ClassDefinition *classDef, StructDefinition *structDef, CTSTR lpName, BOOL bTopLevelOnly=FALSE);
    static FunctionDefinition *GetNextFunction(ClassDefinition *classDef, StructDefinition *structDef, CTSTR lpName, FunctionDefinition *curFunc, BOOL bTopLevelOnly=FALSE);
    static FunctionDefinition *GetMatchingFunction(ClassDefinition *classDef, StructDefinition *structDef, CTSTR lpName, CodeInfoList &paramMatchList, DWORD curCodePos=0, BOOL bLocalOnly=FALSE, BOOL bSuperFuncs=TRUE);
    static BOOL FunctionMatches(List<DefaultVariable> &funcParams, CodeInfoList &paramMatchList, DWORD curCodePos=0);
    static Variable *GetVariable(ClassDefinition *classDef, StructDefinition *structDef, CTSTR lpName);

    static BOOL ValidOperator(CTSTR lpOperator);
    static BOOL ValidPrefixOperator(CTSTR lpOperator);

    inline static void AlignOffset(unsigned int &pos, int align)
    {
        if(pos && align)
        {
            unsigned remainder = (pos % align);
            if(remainder)
                pos += (align - remainder);
        }
    }
};

inline void CopyStringRange(String &strTarget, TSTR lpStart, TSTR lpEnd)
{
    if(lpStart != lpEnd)
    {
        TCHAR lastChar = *lpEnd;
        *lpEnd = 0;
        strTarget << lpStart;
        *lpEnd = lastChar;
    }
}


#endif


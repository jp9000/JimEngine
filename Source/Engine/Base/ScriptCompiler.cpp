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
#include "ScriptByteCode.h"


int ENGINEAPI GetStringLine(const TCHAR *lpStart, const TCHAR *lpOffset);



#define AddErrorExpecting(expecting, got)   AddError(TEXT("Expecting '%s', but got '%s'"), (TSTR)expecting, (TSTR)got)
#define AddErrorEndOfCode()                 AddError(TEXT("Unexpected end of code"))
#define AddErrorUnrecoverable()             AddError(TEXT("Compiler terminated due to unrecoverable error"))
#define AddErrorRedefinition(redef)         AddError(TEXT("Redefinition: '%s'"), (TSTR)redef)
#define AddErrorNoClue(item)                AddError(TEXT("Unknown Token: '%s'"), (TSTR)curToken)


#define PeekAtAToken(str) \
    if(!GetNextToken(str, TRUE)) \
    { \
        if(bNewCodeSegment) \
            delete curCodeSegment; \
        return FALSE; \
    }

#define HandMeAToken(str) \
    if(!GetNextToken(str)) \
    { \
        if(bNewCodeSegment) \
            delete curCodeSegment; \
        return FALSE; \
    }

#define OWGetAway() \
    if(!GotoClosestToken(lpDefaultEndToken, TEXT("}"), FALSE)) \
    { \
        if(bNewCodeSegment) \
            delete curCodeSegment; \
        return FALSE; \
    }

BOOL Compiler::CompileCode(FunctionDefinition &functionDef, CodeSegment *Parent, CTSTR lpDefaultEndToken, BOOL bSwitch)
{
    String curToken, strEndToken;

    if(!GetNextToken(curToken, TRUE))
        return FALSE;

    BOOL bNewCodeSegment = FALSE;
    CodeSegment *curCodeSegment = Parent;

    BOOL bReturned = FALSE;

    if(curToken[0] == '{')
    {
        if(!GetNextToken(curToken))
            return FALSE;
        strEndToken = "}";

        bNewCodeSegment = TRUE;

        curCodeSegment = new CodeSegment;
        curCodeSegment->Parent = Parent;
        curCodeSegment->curCompiler = this;
        curCodeSegment->functionParent = &functionDef;
    }
    else
    {
        if(bSwitch)
        {
            AddErrorExpecting(TEXT("{"), curToken);
            return FALSE;
        }
        strEndToken = lpDefaultEndToken;

        bReturned = TRUE; //warning -- HACK!
    }

    BOOL bFirstVal = FALSE;
    BOOL bCaseFound = FALSE, bDefaultFound = FALSE;
    DWORD lastCaseOffset = 0, defaultCasePos = 0, lastCasePos = 0;

    BOOL bEndOfCode = TRUE;

    while((bGeneratingBytecodeText && WriteCurrentCodeOffset(&functionDef)) || GetNextToken(curToken))
    {
        DWORD IncrementOrDecrementPrefix = 0;
        BOOL bDoingSuper=FALSE;

        int line = GetStringLine(lpCode, lpTemp);

        if(curToken.Compare(strEndToken))
        {
            bEndOfCode = FALSE;
            break;
        }

        if(curToken[0] == lpDefaultEndToken[0])
            continue;

        if(curToken[0] == '{')
        {
            --lpTemp;
            CompileCode(functionDef, curCodeSegment);

            continue;
        }

        if(!bFirstVal)
        {
            if(bSwitch && (!curToken.Compare(TEXT("case")) && !curToken.Compare(TEXT("default"))))
            {
                AddErrorExpecting(TEXT("case' or 'default"), curToken);
                continue;
            }

            bFirstVal = TRUE;
        }

        String returnVar;

        TokenType type = curCodeSegment->GetTokenType(curToken, FALSE);

        DWORD varStartPos = functionDef.ByteCode.Num();
        TypeInfo lastReturnType;
        BOOL bThis = FALSE;

        if(type == TokenType_PrefixOperator)
        {
            if(curToken.Compare(TEXT("++")) || curToken.Compare(TEXT("--")))
                IncrementOrDecrementPrefix = (curToken[0] == '+') ? 1 : 2;

            HandMeAToken(curToken);
            type = curCodeSegment->GetTokenType(curToken, FALSE);
        }

        if((type == TokenType_Keyword) && (curToken == TEXT("super")))
        {
            if(!curClass && !curStruct)
            {
                AddError(TEXT("'%s' keyword can only be used in class/struct member functions"), TEXT("super"));
                OWGetAway();
                continue;
            }

            HandMeAToken(curToken);
            if(curToken[0] != '.')
            {
                AddErrorExpecting(TEXT("."), curToken);
                OWGetAway();
                continue;
            }

            HandMeAToken(curToken);
            type = curCodeSegment->GetTokenType(curToken, FALSE);

            if(type != TokenType_Function)
            {
                AddErrorExpecting(TEXT("function"), curToken);
                OWGetAway();
                continue;
            }

            bDoingSuper = TRUE;
        }

        if(type == TokenType_EnumDef)
        {
            type = TokenType_Type;
            curToken = TEXT("int");
        }

        String nextToken;
        PeekAtAToken(nextToken);

        BOOL bStaticFunc=FALSE;

        if(nextToken[0] == '.' && (type == TokenType_Class || type == TokenType_Struct)) //static member
        {
            GetTypeInfo(curToken, lastReturnType);
            bStaticFunc = TRUE;
        }
        else if((type == TokenType_Type) || (type == TokenType_EnumDef) || (type == TokenType_Class) || (type == TokenType_Struct)) //local variable definition
        {
            String strType = curToken;
            String subType;

            if(strType == TEXT("list"))
            {
                HandMeAToken(curToken);
                if(curToken == TEXT("<"))
                {
                    HandMeAToken(curToken);
                    if(curToken != TEXT(">"))
                    {
                        subType = curToken;

                        HandMeAToken(curToken);
                        if(curToken != TEXT(">"))
                        {
                            AddErrorExpecting(TEXT(">"), curToken);
                            if(!GotoToken(TEXT(">"), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                        }
                    }
                    else
                        AddError(TEXT("A type must be specified for a list variable"));
                }
                else
                    AddErrorExpecting(TEXT("<"), curToken);
            }

            String strName;

            if(bSwitch)
                AddError(TEXT("Cannot define a variable inside the main path of a switch statement"));

            BOOL bContinue = FALSE;
            BOOL bUghWhyDoIHaveToDoThis___First = TRUE;
            do
            {
                if(!bUghWhyDoIHaveToDoThis___First)
                {
                    HandMeAToken(curToken);
                }
                else
                    bUghWhyDoIHaveToDoThis___First = FALSE;

                HandMeAToken(strName);

                if(!iswalpha(strName[0]))
                    AddError(TEXT("Names must begin with valid characters: '%s'"), (TSTR)strName);
                else if(curCodeSegment->NameDefined(strName))
                    AddErrorRedefinition(strName);

                int varID = functionDef.LocalVars.Num();
                Variable *var = functionDef.LocalVars.CreateNew();
                if(!GetTypeInfo(strType, var->typeInfo))
                    AddError(TEXT("Unknown data type: '%s'"), strType.Array());
                else if(var->typeInfo.type == DataType_Void)
                    AddError(TEXT("Cannot use void as a data type for variables"));

                if(subType.IsValid())
                {
                    if(!GetTypeInfo(subType, var->subTypeInfo))
                        AddError(TEXT("Unknown data type: '%s'"), subType.Array());
                    else if(var->typeInfo.type == DataType_Void)
                        AddError(TEXT("Cannot use void as a data type for variables"));

                    if(var->typeInfo.type == DataType_Struct)
                        var->structDef = Scripting->GetStruct(var->typeInfo.name);
                }

                functionDef.localVarAlign = MAX(functionDef.localVarAlign, var->typeInfo.align);
                AlignOffset(functionDef.LocalVariableStackSize, var->typeInfo.align);

                var->scope = VarScope_Local;
                var->name = strName;
                var->numElements = 0;  //todo: number of elements thingy?
                var->offset = functionDef.LocalVariableStackSize;

                curCodeSegment->LocalVariableIDs << varID;

                PeekAtAToken(curToken);

                int elements = 1;

                if(curToken[0] == '[')
                {
                    HandMeAToken(curToken);
                    HandMeAToken(curToken);

                    if((curToken[0] != ']') && !ValidIntString(curToken))
                        AddError(TEXT("Invalid array count: %s"), curToken.Array());

                    if(curToken[0] == ']')
                        AddError(TEXT("Must specify an integer count of 1 or higher"));
                    else
                    {
                        elements = var->numElements = tstoi(curToken);

                        if(var->numElements == 0)
                            AddError(TEXT("Must specify an integer count of 1 or higher"));

                        HandMeAToken(curToken);

                        if(curToken[0] != ']')
                        {
                            AddErrorExpecting(TEXT("]"), curToken);
                            if(!GotoToken(TEXT("]"), TRUE))   {AddErrorEndOfCode(); return FALSE;}
                        }
                    }

                    PeekAtAToken(curToken);
                }
                else if(var->typeInfo.type == DataType_Struct && curToken != TEXT("="))
                {
                    StructDefinition *structDef = Scripting->GetStruct(var->typeInfo.name);
                    CompileErrorType errorType;
                    BOOL bFoundConstructor;

                    UINT lastPos = functionDef.ByteCode.Num();;

                    APush(functionDef.ByteCode, var);

                    TPushZero(functionDef.ByteCode, structDef->size);

                    if(curToken[0] == '(')
                        errorType = ProcessConstructor(functionDef, curCodeSegment, NULL, structDef, &bFoundConstructor);
                    else
                    {
                        CodeInfoList paramList;
                        errorType = CallConstructor(functionDef, curCodeSegment, NULL, structDef, paramList, &bFoundConstructor);
                    }

                    switch(errorType)
                    {
                        case COMPILE_UNRECOVERABLE:
                            return FALSE;
                        case COMPILE_ERROR:
                            GotoClosestToken(TEXT(","), TEXT(";"));
                    }

                    PushTData(functionDef.ByteCode, structDef->GetType());
                    APopVar(functionDef.ByteCode, var, TRUE);

                    if(!bFoundConstructor)
                        functionDef.ByteCode.SetSize(lastPos);
                }

                functionDef.LocalVariableStackSize += var->typeInfo.size*elements;

                if(curToken == TEXT("="))  //default value!
                {
                    APush(functionDef.ByteCode, var);

                    int startPos = functionDef.ByteCode.Num();

                    if(var->numElements)
                        AddError(TEXT("Default variables cannot be assigned to arrays"));
                    HandMeAToken(curToken);

                    CodeInfo codeInfo;
                    if(!CompileSubCode(functionDef, curCodeSegment, codeInfo))
                    {
                        if(bNewCodeSegment)
                            delete curCodeSegment;
                        return FALSE;
                    }

                    if(codeInfo.bErrorsOccured)
                    {
                        OWGetAway();
                        bContinue = TRUE;
                        continue;
                    }

                    if(codeInfo.typeReturned.type == DataType_Null)
                    {
                        if(var->typeInfo.type == DataType_Object || var->typeInfo.type == DataType_Handle)
                        {
                            DoInstruction(functionDef.ByteCode, BCPushNullObj);
                            codeInfo.typeReturned = var->typeInfo;
                        }
                        else if(var->typeInfo.type == DataType_String)
                        {
                            DoInstruction(functionDef.ByteCode, BCPushNullStr);
                            codeInfo.typeReturned = var->typeInfo;
                        }
                    }

                    BOOL bCalledFunction;
                    TypeInfo opReturnType = var->typeInfo;
                    if(!DoOperator(functionDef.ByteCode, curToken, opReturnType, codeInfo.typeReturned, bCalledFunction))
                    {
                        AddError(TEXT("Invalid types used for operator: '%s %s %s'"), opReturnType.name, curToken.Array(), codeInfo.typeReturned.name);
                        OWGetAway();
                        bContinue = TRUE;
                        continue;
                    }

                    if(bCalledFunction)
                    {
                        List<BYTE> pushVarData;
                        PushAData(pushVarData, var->typeInfo, TRUE);
                        functionDef.ByteCode.InsertList(startPos, pushVarData);

                        PushTData(functionDef.ByteCode, var->typeInfo);
                        //TPop(functionDef.ByteCode, opReturnType.size);
                    }

                    APopVar(functionDef.ByteCode, var, TRUE);

                    PeekAtAToken(curToken);
                }
            }while(curToken[0] == ',');

            if(bContinue)
                continue;
        }
        else if(type == TokenType_Keyword)
        {
            if(curToken == TEXT("break"))
            {
                if(!bBreaksCurrentlyAllowed)
                {
                    AddError(TEXT("breaks are not allowed outside of loop contexts: '%s'"), (TSTR)curToken);
                    OWGetAway();
                    continue;
                }

                DWORD breakPos;
                JumpUndefined(functionDef.ByteCode, breakPos);

                CurBreakOffsets->Add(breakPos);
            }
            else if(curToken == TEXT("this"))
            {
                if(!curClass && !curStruct)
                    AddError(TEXT("'%s' keyword can only be used in class/struct member functions"), TEXT("this"));

                if(curClass)
                    GetTypeInfo(functionDef.classContext->classData->GetName(), lastReturnType);
                else if(curStruct)
                    GetTypeInfo(functionDef.structContext->name, lastReturnType);

                bThis = TRUE;
            }
            else if(curToken == TEXT("destroy"))
            {
                CodeInfo ci;
                if(!CompileSubCode(functionDef, curCodeSegment, ci, TEXT(";")))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;
                    return FALSE;
                }

                if(ci.typeReturned.type != DataType_Object)
                    AddError(TEXT("'destroy' can only be used with objects"));

                DoInstruction(functionDef.ByteCode, BCDestroyObj);

                bThis = TRUE;
            }
            else if(curToken == TEXT("create"))
            {
                HandMeAToken(curToken);

                BOOL bInit = TRUE;
                if(curToken == TEXT("noinit"))
                {
                    HandMeAToken(curToken);
                    bInit = FALSE;
                }

                ClassDefinition *classDef = Scripting->GetClassDef(curToken);

                if(!classDef)
                {
                    AddError(TEXT("Unknown class '%s'"), curToken.Array());
                    OWGetAway();
                    continue;
                }

                TypeInfo classType;
                GetTypeInfo(curToken, classType);

                CreateObj(functionDef.ByteCode, curToken);

                BOOL bGetMeAway = FALSE;
                switch(ProcessConstructor(functionDef, curCodeSegment, classDef, NULL))
                {
                    case COMPILE_UNRECOVERABLE:
                        delete curCodeSegment;
                        return FALSE;

                    case COMPILE_ERROR:
                        bGetMeAway = TRUE;
                        break;
                }

                if(bGetMeAway)
                {
                    OWGetAway();
                    continue;
                }

                if(bInit)
                {
                    FunctionDefinition *func = Object::GetLocalClass()->scriptClass->GetFunction(TEXT("InitializeObject"));

                    DoInstruction(functionDef.ByteCode, BCAPushT);
                    CallClassFunc(functionDef.ByteCode, *func, FALSE);
                }

                TPop(functionDef.ByteCode, classType.size);
            }
            else if(curToken == TEXT("return"))
            {
                if(functionDef.returnType.type != DataType_Void)
                {
                    PeekAtAToken(curToken);

                    if(curToken[0] == ';')
                        AddError(TEXT("Function must return '%s' value"), functionDef.returnType.name);
                    else
                    {
                        CodeInfo ci;
                        if(!CompileSubCode(functionDef, curCodeSegment, ci, TEXT(";")))
                        {
                            if(bNewCodeSegment)
                                delete curCodeSegment;
                            return FALSE;
                        }

                        if(ci.typeReturned.type == DataType_Null)
                        {
                            if(functionDef.returnType.type == DataType_Object || functionDef.returnType.type == DataType_Handle)
                            {
                                DoInstruction(functionDef.ByteCode, BCPushNullObj);
                                ci.typeReturned = functionDef.returnType;
                            }
                            else if(functionDef.returnType.type == DataType_String)
                            {
                                DoInstruction(functionDef.ByteCode, BCPushNullStr);
                                ci.typeReturned = functionDef.returnType;
                            }
                        }
                        else if((functionDef.returnType.type == DataType_Float) && (ci.typeReturned.type == DataType_Integer))
                            DoInstruction(functionDef.ByteCode, BCCastFloat);
                        else if((functionDef.returnType.type == DataType_Integer) && (ci.typeReturned.type == DataType_Float))
                            DoInstruction(functionDef.ByteCode, BCCastInt);
                    }
                }

                DoReturn(functionDef);

                bReturned = TRUE;
            }
            else if(curToken == TEXT("if"))
            {
                HandMeAToken(curToken);

                if(curToken[0] != '(')
                {
                    AddErrorExpecting(TEXT("("), curToken);
                    continue;
                }

                CodeInfo ci;
                if(!CompileSubCode(functionDef, curCodeSegment, ci, TEXT(")")))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;
                    return FALSE;
                }

                if(!ci.bErrorsOccured)
                {
                    if(ci.typeReturned.type == DataType_Object || ci.typeReturned.type == DataType_Handle)
                    {
                        DoInstruction(functionDef.ByteCode, BCPushNullObj);
                        DoInstruction(functionDef.ByteCode, BCNotEqualO);
                        GetTypeInfo(TEXT("bool"), ci.typeReturned);
                    }
                    else if(ci.typeReturned.type != DataType_Integer)
                        AddError(TEXT("Expecting expression for '%s' statement"), TEXT("if"));
                }

                HandMeAToken(curToken);

                if(curToken[0] != ')')
                {
                    AddErrorExpecting(TEXT(")"), curToken);
                    continue;
                }

                DWORD jumpOverwritePos;
                JumpIfNot(functionDef.ByteCode, jumpOverwritePos);

                if(!CompileCode(functionDef, curCodeSegment))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;
                    return FALSE;
                }

                //--------------------

                PeekAtAToken(curToken);

                if(curToken == "else")
                {
                    DWORD exitIfPos;
                    JumpUndefined(functionDef.ByteCode, exitIfPos);

                    *(DWORD*)&functionDef.ByteCode[jumpOverwritePos] = functionDef.ByteCode.Num()-jumpOverwritePos-4;

                    GetNextToken(curToken);

                    if(!CompileCode(functionDef, curCodeSegment))
                    {
                        if(bNewCodeSegment)
                            delete curCodeSegment;
                        return FALSE;
                    }

                    *(DWORD*)&functionDef.ByteCode[exitIfPos] = functionDef.ByteCode.Num()-exitIfPos-4;
                }
                else
                    *(DWORD*)&functionDef.ByteCode[jumpOverwritePos] = functionDef.ByteCode.Num()-jumpOverwritePos-4;

                if(!bNewCodeSegment)
                {
                    bEndOfCode = FALSE;
                    break;
                }
                else
                    continue;
            }
            else if(curToken == TEXT("for"))
            {
                HandMeAToken(curToken);

                if(curToken[0] != '(')
                {
                    AddErrorExpecting(TEXT("("), curToken);
                    continue;
                }

                //--------------------

                CompileCode(functionDef, curCodeSegment);

                //--------------------

                DWORD jumpBackPos = functionDef.ByteCode.Num();

                CodeInfo ci;
                if(!CompileSubCode(functionDef, curCodeSegment, ci, TEXT(";")))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;
                    return FALSE;
                }

                if(!ci.bErrorsOccured)
                {
                    if(ci.typeReturned.type == DataType_Object || ci.typeReturned.type == DataType_Handle)
                    {
                        DoInstruction(functionDef.ByteCode, BCPushNullObj);
                        DoInstruction(functionDef.ByteCode, BCNotEqualO);
                        GetTypeInfo(TEXT("bool"), ci.typeReturned);
                    }
                    else if(ci.typeReturned.type != DataType_Integer)
                        AddError(TEXT("Expecting expression for '%s' statement"), TEXT("for"));
                }

                HandMeAToken(curToken);

                if(curToken[0] != ';')
                {
                    AddErrorExpecting(TEXT(";"), curToken);
                    continue;
                }

                DWORD jumpOverwritePos;
                JumpIfNot(functionDef.ByteCode, jumpOverwritePos);

                //--------------------

                DWORD copyCodeStart = functionDef.ByteCode.Num();
                List<BYTE> CopiedCode;

                if(!CompileCode(functionDef, curCodeSegment, TEXT(")")))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;
                    return FALSE;
                }

                CopiedCode.CopyArray(&functionDef.ByteCode[copyCodeStart], functionDef.ByteCode.Num()-copyCodeStart);
                functionDef.ByteCode.SetSize(copyCodeStart);

                //--------------------

                BOOL bBreaksWereAllowed = bBreaksCurrentlyAllowed;
                List<DWORD> *OldBreakOffsets = CurBreakOffsets;
                List<DWORD> BreakOffsets;
                CurBreakOffsets = &BreakOffsets;
                bBreaksCurrentlyAllowed = TRUE;

                if(!CompileCode(functionDef, curCodeSegment))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;
                    return FALSE;
                }

                //--------------------

                functionDef.ByteCode.AppendList(CopiedCode);

                Jump(functionDef.ByteCode, jumpBackPos-functionDef.ByteCode.Num()-5);

                *(DWORD*)&functionDef.ByteCode[jumpOverwritePos] = functionDef.ByteCode.Num()-jumpOverwritePos-4;

                for(DWORD i=0; i<BreakOffsets.Num(); i++)
                    *(DWORD*)&functionDef.ByteCode[BreakOffsets[i]] = functionDef.ByteCode.Num()-BreakOffsets[i]-4;
                BreakOffsets.Clear();

                bBreaksCurrentlyAllowed = bBreaksWereAllowed;
                CurBreakOffsets = OldBreakOffsets;

                if(!bNewCodeSegment)
                {
                    bEndOfCode = FALSE;
                    break;
                }
                else
                    continue;
            }
            else if(curToken == TEXT("while"))
            {
                HandMeAToken(curToken);

                if(curToken[0] != '(')
                {
                    AddErrorExpecting(TEXT("("), curToken);
                    continue;
                }

                DWORD jumpBackPos = functionDef.ByteCode.Num();

                CodeInfo ci;
                if(!CompileSubCode(functionDef, curCodeSegment, ci, TEXT(")")))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;
                    return FALSE;
                }

                if(!ci.bErrorsOccured)
                {
                    if(ci.typeReturned.type == DataType_Object || ci.typeReturned.type == DataType_Handle)
                    {
                        DoInstruction(functionDef.ByteCode, BCPushNullObj);
                        DoInstruction(functionDef.ByteCode, BCNotEqualO);
                        GetTypeInfo(TEXT("bool"), ci.typeReturned);
                    }
                    else if(ci.typeReturned.type != DataType_Integer)
                        AddError(TEXT("Expecting expression for '%s' statement"), TEXT("while"));
                }

                HandMeAToken(curToken);

                if(curToken[0] != ')')
                {
                    AddErrorExpecting(TEXT(")"), curToken);
                    continue;
                }

                //--------------------

                DWORD jumpOverwritePos;
                JumpIfNot(functionDef.ByteCode, jumpOverwritePos);

                BOOL bBreaksWereAllowed = bBreaksCurrentlyAllowed;
                List<DWORD> *OldBreakOffsets = CurBreakOffsets;
                List<DWORD> BreakOffsets;
                CurBreakOffsets = &BreakOffsets;
                bBreaksCurrentlyAllowed = TRUE;

                if(!CompileCode(functionDef, curCodeSegment))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;
                    bBreaksCurrentlyAllowed = bBreaksWereAllowed;
                    CurBreakOffsets = OldBreakOffsets;
                    return FALSE;
                }

                Jump(functionDef.ByteCode, jumpBackPos-functionDef.ByteCode.Num()-5);

                //--------------------

                *(DWORD*)&functionDef.ByteCode[jumpOverwritePos] = functionDef.ByteCode.Num()-jumpOverwritePos-4;

                for(DWORD i=0; i<BreakOffsets.Num(); i++)
                    *(DWORD*)&functionDef.ByteCode[BreakOffsets[i]] = functionDef.ByteCode.Num()-BreakOffsets[i]-4;
                BreakOffsets.Clear();

                bBreaksCurrentlyAllowed = bBreaksWereAllowed;
                CurBreakOffsets = OldBreakOffsets;

                if(!bNewCodeSegment)
                {
                    bEndOfCode = FALSE;
                    break;
                }
                else
                    continue;
            }
            else if(curToken == TEXT("do"))
            {
                DWORD jumpBackPos = functionDef.ByteCode.Num();

                BOOL bBreaksWereAllowed = bBreaksCurrentlyAllowed;
                List<DWORD> *OldBreakOffsets = CurBreakOffsets;
                List<DWORD> BreakOffsets;
                CurBreakOffsets = &BreakOffsets;
                bBreaksCurrentlyAllowed = TRUE;

                if(!CompileCode(functionDef, curCodeSegment))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;

                    bBreaksCurrentlyAllowed = bBreaksWereAllowed;
                    CurBreakOffsets = OldBreakOffsets;

                    return FALSE;
                }

                //--------------------

                HandMeAToken(curToken);

                if(!curToken.Compare(TEXT("while")))
                {
                    AddErrorExpecting(TEXT("while"), curToken);
                    continue;
                }

                HandMeAToken(curToken);

                if(curToken[0] != '(')
                {
                    AddErrorExpecting(TEXT("("), curToken);
                    continue;
                }

                CodeInfo ci;
                if(!CompileSubCode(functionDef, curCodeSegment, ci, TEXT(")")))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;

                    bBreaksCurrentlyAllowed = bBreaksWereAllowed;
                    CurBreakOffsets = OldBreakOffsets;

                    return FALSE;
                }

                if(!ci.bErrorsOccured)
                {
                    if(ci.typeReturned.type == DataType_Object || ci.typeReturned.type == DataType_Handle)
                    {
                        DoInstruction(functionDef.ByteCode, BCPushNullObj);
                        DoInstruction(functionDef.ByteCode, BCNotEqualO);
                        GetTypeInfo(TEXT("bool"), ci.typeReturned);
                    }
                    else if(ci.typeReturned.type != DataType_Integer)
                        AddError(TEXT("Expecting expression for '%s' statement"), TEXT("do/while"));
                }

                HandMeAToken(curToken);

                if(curToken[0] != ')')
                {
                    AddErrorExpecting(TEXT(")"), (TSTR)curToken);
                    continue;
                }

                //--------------------

                DWORD jumpOverwritePos;
                JumpIf(functionDef.ByteCode, jumpOverwritePos);

                *(DWORD*)&functionDef.ByteCode[jumpOverwritePos] = jumpBackPos-jumpOverwritePos-4;

                for(DWORD i=0; i<BreakOffsets.Num(); i++)
                    *(DWORD*)&functionDef.ByteCode[BreakOffsets[i]] = functionDef.ByteCode.Num()-BreakOffsets[i]-4;
                BreakOffsets.Clear();

                bBreaksCurrentlyAllowed = bBreaksWereAllowed;
                CurBreakOffsets = OldBreakOffsets;
            }
            else if(curToken == TEXT("switch"))
            {
                HandMeAToken(curToken);

                if(curToken[0] != '(')
                {
                    AddErrorExpecting(TEXT("("), curToken);
                    continue;
                }

                CodeInfo ci;
                if(!CompileSubCode(functionDef, curCodeSegment, ci, TEXT(")")))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;
                    return FALSE;
                }

                if(!ci.bErrorsOccured && ci.typeReturned.type != DataType_Integer)
                    AddError(TEXT("Expecting integral value for 'switch' statement"));

                HandMeAToken(curToken);

                if(curToken[0] != ')')
                {
                    AddErrorExpecting(TEXT(")"), curToken);
                    continue;
                }

                //--------------------

                BOOL bBreaksWereAllowed = bBreaksCurrentlyAllowed;
                List<DWORD> *OldBreakOffsets = CurBreakOffsets;
                List<DWORD> BreakOffsets;
                CurBreakOffsets = &BreakOffsets;
                bBreaksCurrentlyAllowed = TRUE;

                if(!CompileCode(functionDef, curCodeSegment, TEXT(";"), TRUE))
                {
                    if(bNewCodeSegment)
                        delete curCodeSegment;

                    bBreaksCurrentlyAllowed = bBreaksWereAllowed;
                    CurBreakOffsets = OldBreakOffsets;
                    return FALSE;
                }

                //--------------------

                for(DWORD i=0; i<BreakOffsets.Num(); i++)
                    *(DWORD*)&functionDef.ByteCode[BreakOffsets[i]] = functionDef.ByteCode.Num()-BreakOffsets[i]-4;
                BreakOffsets.Clear();

                bBreaksCurrentlyAllowed = bBreaksWereAllowed;
                CurBreakOffsets = OldBreakOffsets;

                TypeInfo intType;
                GetTypeInfo(TEXT("int"), intType);

                Pop(functionDef.ByteCode, intType);

                if(!bNewCodeSegment)
                {
                    bEndOfCode = FALSE;
                    break;
                }
                else
                    continue;
            }
            else if(curToken == TEXT("case"))
            {
                if(!bSwitch)
                {
                    AddError(TEXT("'case' cannot be used outside of a switch: '%s'"), (TSTR)curToken);
                    continue;
                }

                if(bDefaultFound)
                {
                    AddError(TEXT("'default' must be used last inside of a switch: '%s'"), (TSTR)curToken);
                    continue;
                }

                //--------------------

                BOOL bNonCaseCodeCalled = functionDef.ByteCode.Num()-lastCasePos != 0;

                DWORD skipPos;
                if(!bNonCaseCodeCalled)
                    JumpUndefined(functionDef.ByteCode, skipPos);

                //--------------------

                if(bCaseFound)
                    *(DWORD*)&functionDef.ByteCode[lastCaseOffset] = functionDef.ByteCode.Num()-lastCaseOffset-4;

                TypeInfo intType;
                GetTypeInfo(TEXT("int"), intType);

                PushDup(functionDef.ByteCode, intType);

                //--------------------

                DWORD curCodePos = functionDef.ByteCode.Num();
                
                HandMeAToken(curToken);

                TokenType curType = GetTokenType(NULL, NULL, curToken);
                if((curType == TokenType_Number) && !DefinitelyFloatString(curToken))
                    PushInt(functionDef.ByteCode, tstoi(curToken));
                else if(curType == TokenType_Enum)
                    PushInt(functionDef.ByteCode, GetEnumVal(curToken));
                else
                    AddError(TEXT("Only constant string or integral values are allowed for a case: '%s'"), (TSTR)curToken);

                HandMeAToken(curToken);

                if(curToken[0] != ':')
                {
                    AddErrorExpecting(TEXT(":"), curToken);
                    continue;
                }

                //--------------------

                DoInstruction(functionDef.ByteCode, BCEqual);
                JumpIfNot(functionDef.ByteCode, lastCaseOffset);

                bCaseFound = TRUE;

                //--------------------

                lastCasePos = functionDef.ByteCode.Num();

                if(!bNonCaseCodeCalled)
                    functionDef.ByteCode[skipPos] = functionDef.ByteCode.Num()-skipPos-4;

                continue;
            }
            else if(curToken == TEXT("default"))
            {
                if(!bSwitch)
                {
                    AddError(TEXT("'default' cannot be used outside of a switch: '%s'"), (TSTR)curToken);
                    continue;
                }

                if(bDefaultFound)
                {
                    AddError(TEXT("Cannot have more than one 'default' within a switch: '%s'"), (TSTR)curToken);
                    continue;
                }

                //--------------------

                BOOL bNonCaseCodeCalled = functionDef.ByteCode.Num()-lastCasePos != 0;

                DWORD skipPos;
                if(!bNonCaseCodeCalled)
                    JumpUndefined(functionDef.ByteCode, skipPos);

                //--------------------

                if(bCaseFound)
                    *(DWORD*)&functionDef.ByteCode[lastCaseOffset] = functionDef.ByteCode.Num()-lastCaseOffset-4;

                bDefaultFound = TRUE;
                bCaseFound = TRUE;

                JumpUndefined(functionDef.ByteCode, lastCaseOffset);

                defaultCasePos = functionDef.ByteCode.Num();

                //--------------------

                lastCasePos = functionDef.ByteCode.Num();

                if(!bNonCaseCodeCalled)
                    functionDef.ByteCode[skipPos] = functionDef.ByteCode.Num()-skipPos-4;

                continue;
            }
            else
            {
                AddError(TEXT("Invalid use of '%'"), (TSTR)curToken);
                OWGetAway();
                continue;
            }
        }
        else if((type != TokenType_Variable) && (type != TokenType_Function))
        {
            AddErrorNoClue(curToken);
            OWGetAway();
            continue;
        }

        PeekAtAToken(nextToken);
        BOOL bIsDot = (nextToken[0] == '.' && lastReturnType.type != DataType_Void);

        if(bIsDot || (type == TokenType_Variable) || (type == TokenType_Function))
        {
            Variable *curVar = NULL;

            BOOL bWasFunction = FALSE;
            BOOL bLValue = FALSE;
            BOOL bGetMeAway = FALSE;
            List<TypeInfo> popData;

            if(!bIsDot)
                lpTemp -= curToken.Length();

            switch(ProcessFuncOrVar(functionDef, curCodeSegment, curVar, bDoingSuper, bThis, bWasFunction, bLValue, varStartPos, popData, lastReturnType, bStaticFunc))
            {
                case COMPILE_UNRECOVERABLE:
                    delete curCodeSegment;
                    return FALSE;

                case COMPILE_ERROR:
                    bGetMeAway = TRUE;
                    break;
            }

            if(bGetMeAway)
            {
                OWGetAway();
                continue;
            }

            if(!bWasFunction && !bLValue)
            {
                AddError(TEXT("Cannot set structure values directly from a function return"));
                OWGetAway();
                continue;
            }

            TPopTypes(functionDef.ByteCode, popData);

            if(!bWasFunction)
            {
                PeekAtAToken(curToken);

                TokenType nextType = curCodeSegment->GetTokenType(curToken);

                if(nextType == TokenType_PostfixOperator)
                {
                    HandMeAToken(curToken);

                    if(IncrementOrDecrementPrefix)
                    {
                        AddError(TEXT("Cannot use a prefix operator and a postfix operator at the same time"));
                        OWGetAway();
                        continue;
                    }

                    PushAData(functionDef.ByteCode, curVar->typeInfo, TRUE);

                    if(curVar->typeInfo.type == DataType_Float)
                    {
                        PushFloat(functionDef.ByteCode, 1.0f);
                        DoInstruction(functionDef.ByteCode, (curToken == TEXT("++")) ? BCAddF : BCSubtractF);
                    }
                    else if(curVar->typeInfo.type == DataType_Integer)
                    {
                        PushInt(functionDef.ByteCode, 1);
                        DoInstruction(functionDef.ByteCode, (curToken == TEXT("++")) ? BCAdd : BCSubtract);
                    }
                    else
                    {
                        AddErrorExpecting(TEXT("Numeric variable"), curVar->typeInfo.name);
                        OWGetAway();
                        continue;
                    }

                    APopVar(functionDef.ByteCode, curVar, FALSE);
                }
                else if(nextType == TokenType_AssignmentOperator)
                {
                    DWORD startPos = functionDef.ByteCode.Num();

                    if(IncrementOrDecrementPrefix)
                    {
                        AddError(TEXT("Cannot use an assignment operator with a prefix operator: '%s'"), (TSTR)curToken);
                        OWGetAway();
                        continue;
                    }

                    HandMeAToken(curToken);

                    if(curToken.Length() > 1)
                        PushAData(functionDef.ByteCode, curVar->typeInfo, TRUE);

                    CodeInfo codeInfo;
                    if(!CompileSubCode(functionDef, curCodeSegment, codeInfo))
                    {
                        if(bNewCodeSegment)
                            delete curCodeSegment;
                        return FALSE;
                    }

                    if(codeInfo.bErrorsOccured)
                    {
                        OWGetAway();
                        continue;
                    }

                    if(codeInfo.typeReturned.type == DataType_Null)
                    {
                        if(curVar->typeInfo.type == DataType_Object || curVar->typeInfo.type == DataType_Handle)
                        {
                            DoInstruction(functionDef.ByteCode, BCPushNullObj);
                            codeInfo.typeReturned = curVar->typeInfo;
                        }
                        else if(curVar->typeInfo.type == DataType_String)
                        {
                            DoInstruction(functionDef.ByteCode, BCPushNullStr);
                            codeInfo.typeReturned = curVar->typeInfo;
                        }
                    }
                    else if((curVar->typeInfo.type == DataType_Float) && (codeInfo.typeReturned.type == DataType_Integer))
                    {
                        DoInstruction(functionDef.ByteCode, BCCastFloat);
                        codeInfo.typeReturned = curVar->typeInfo;
                    }
                    else if((curVar->typeInfo.type == DataType_Integer) && (codeInfo.typeReturned.type == DataType_Float))
                    {
                        DoInstruction(functionDef.ByteCode, BCCastInt);
                        codeInfo.typeReturned = curVar->typeInfo;
                    }

                    BOOL bCalledFunction = FALSE;
                    TypeInfo opReturnType = curVar->typeInfo;
                    if(!DoOperator(functionDef.ByteCode, curToken, opReturnType, codeInfo.typeReturned, bCalledFunction))
                    {
                        AddError(TEXT("Invalid types used for operator: '%s %s %s'"), opReturnType.name, (TSTR)curToken, codeInfo.typeReturned.name);
                        OWGetAway();
                        continue;
                    }

                    if(curToken.Length() > 1)
                    {
                        if(bCalledFunction)
                            PushTData(functionDef.ByteCode, opReturnType);
                        if(!DoOperator(functionDef.ByteCode, String(TEXT("=")), opReturnType, opReturnType, bCalledFunction))
                        {
                            AddError(TEXT("Invalid types used for operator: '%s %s %s'"), opReturnType.name, (TSTR)curToken, opReturnType.name);
                            OWGetAway();
                            continue;
                        }
                    }

                    if(bCalledFunction)
                    {
                        PushTData(functionDef.ByteCode, opReturnType);

                        List<BYTE> pushVarData;
                        PushAData(pushVarData, curVar->typeInfo, TRUE);
                        functionDef.ByteCode.InsertList(startPos, pushVarData);
                    }

                    APopVar(functionDef.ByteCode, curVar, TRUE);
                }
                else if((curToken[0] == ';') && IncrementOrDecrementPrefix)
                {
                    PushAData(functionDef.ByteCode, curVar->typeInfo, TRUE);

                    if(curVar->typeInfo.type == DataType_Float)
                    {
                        PushFloat(functionDef.ByteCode, 1.0f);
                        DoInstruction(functionDef.ByteCode, (IncrementOrDecrementPrefix == 1) ? BCAddF : BCSubtractF);
                    }
                    else if(curVar->typeInfo.type == DataType_Integer)
                    {
                        PushInt(functionDef.ByteCode, 1);
                        DoInstruction(functionDef.ByteCode, (IncrementOrDecrementPrefix == 1) ? BCAdd : BCSubtract);
                    }
                    else
                    {
                        AddErrorExpecting(TEXT("Numeric variable"), curVar->typeInfo.name);
                        OWGetAway();
                        continue;
                    }

                    APopVar(functionDef.ByteCode, curVar, FALSE);
                }
                else
                {
                    if(curToken == TEXT("->"))
                        AddError(TEXT("Did you mean to use '.' here?"));
                    else
                        AddErrorNoClue(curToken);
                    OWGetAway();
                    continue;
                }
            }
        }

        HandMeAToken(curToken);

        if(curToken[0] != lpDefaultEndToken[0])
        {
            AddErrorNoClue(curToken);
            OWGetAway();
            continue;
        }

        if(curToken[0] == strEndToken[0])
        {
            bEndOfCode = FALSE;
            break;
        }
    }

    if(bSwitch)
    {
        if(!bCaseFound && !bDefaultFound)
            AddError(TEXT("A switch must have at least one 'case' or 'default'"));
        else
        {
            if(bCaseFound)
                *(DWORD*)&functionDef.ByteCode[lastCaseOffset] = functionDef.ByteCode.Num()-lastCaseOffset-4;
            if(bDefaultFound)
                Jump(functionDef.ByteCode, defaultCasePos-functionDef.ByteCode.Num()-5);
        }
    }

    if(!curCodeSegment->Parent && (functionDef.returnType.type != DataType_Void) && !bReturned)
        AddWarning(TEXT("Main function path does not return a value"));

    if(bNewCodeSegment)
        delete curCodeSegment;
    return !bEndOfCode;
}


#undef PeekAtAToken
#define PeekAtAToken(str) \
    if(!GetNextToken(str, TRUE)) \
    { \
        delete curCodeSegment; \
        return FALSE; \
    }

#undef HandMeAToken
#define HandMeAToken(str) \
    if(!GetNextToken(str)) \
    { \
        delete curCodeSegment; \
        return FALSE; \
    }

BOOL Compiler::CompileSubCode(FunctionDefinition &functionDef, CodeSegment *Parent, CodeInfo &returnInfo, CTSTR endTokenPriority, VariableType preferredType)
{
    String curToken, curOperator;

    CodeSegment *curCodeSegment = new CodeSegment;
    curCodeSegment->functionParent = &functionDef;
    curCodeSegment->Parent = Parent;
    curCodeSegment->curCompiler = this;

    if(curFile.CompareI(TEXT("VideoMenu.xscript")) && GetStringLine(lpCode, lpTemp) == 170)
        nop();

    BOOL bEndOfCode = TRUE;

    int startErrorCount = errorCount;
    returnInfo.bErrorsOccured = TRUE;
    returnInfo.inPos = functionDef.ByteCode.Num();

    DWORD IncrementOrDecrementPrefix = 0;

    int line = GetStringLine(lpCode, lpTemp);
    CTSTR lpLastCodeThingy = lpTemp;

    int curPrecidence = GetTokenPrecedence(endTokenPriority);
    BOOL bGotActualFloat = FALSE;
    if(GetNextTokenEval(curToken, &bGotActualFloat, curPrecidence))
    {
        bEndOfCode = FALSE;

        BOOL bDoingSuper = FALSE;

        BOOL bPopTempObject = FALSE; //only used when doing a '.' after a cast

        String prefixOperator;

        TokenType type = curCodeSegment->GetTokenType(curToken, FALSE);

        DWORD varStartPos = functionDef.ByteCode.Num();
        TypeInfo lastReturnType;
        Variable *curVar = NULL;
        BOOL bThis = FALSE;

        if(type == TokenType_PrefixOperator)
        {
            if(curToken.Compare(TEXT("++")) || curToken.Compare(TEXT("--")))
                IncrementOrDecrementPrefix = (curToken[0] == '+') ? 1 : 2;
            else
                prefixOperator = curToken;

            HandMeAToken(curToken);
            type = curCodeSegment->GetTokenType(curToken, TRUE);
        }

        String nextToken;
        PeekAtAToken(nextToken);

        if((type == TokenType_Keyword) && (curToken == TEXT("super")))
        {
            if(!curClass && !curStruct)
            {
                AddError(TEXT("'%s' keyword can only be used in class/struct member functions"), TEXT("super"));
                delete curCodeSegment;
                return GotoClosingToken(endTokenPriority);
            }

            HandMeAToken(curToken);
            if(curToken[0] != '.')
            {
                AddErrorExpecting(TEXT("."), curToken);
                delete curCodeSegment;
                return GotoClosingToken(endTokenPriority);
            }

            HandMeAToken(curToken);
            type = curCodeSegment->GetTokenType(curToken, FALSE);

            if(type != TokenType_Function)
            {
                AddErrorExpecting(TEXT("function"), curToken);
                delete curCodeSegment;
                return GotoClosingToken(endTokenPriority);
            }

            bDoingSuper = TRUE;
        }

        BOOL bStaticFunc=FALSE;

        if(nextToken[0] == '.' && (type == TokenType_Class || type == TokenType_Struct)) //static member
        {
            GetTypeInfo(curToken, lastReturnType);
            bStaticFunc = TRUE;
        }
        else if(curToken[0] == '(')
        {
            PeekAtAToken(curToken);
            TokenType type = GetTokenType(NULL, NULL, curToken, FALSE);

            if((type == TokenType_Type) || (type == TokenType_Class)) //assume cast
            {
                String typeName;
                HandMeAToken(typeName);

                HandMeAToken(curToken);

                if(curToken[0] != ')')
                {
                    AddErrorExpecting(TEXT(")"), curToken);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                CodeInfo ci;
                if(!CompileSubCode(functionDef, curCodeSegment, ci, ClosingTokens[NUMCLOSINGTOKENS-1]))
                {
                    delete curCodeSegment;
                    return FALSE;
                }

                if(ci.bErrorsOccured)
                {
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                GetTypeInfo(typeName, returnInfo.typeReturned);
                lastReturnType = returnInfo.typeReturned;

                if(ci.typeReturned.type == DataType_Null)
                {
                    if(returnInfo.typeReturned.type == DataType_Object || ci.typeReturned.type == DataType_Handle)
                    {
                        DoInstruction(functionDef.ByteCode, BCPushNullObj);
                        ci.typeReturned = returnInfo.typeReturned;
                    }
                    else if(returnInfo.typeReturned.type == DataType_String)
                    {
                        DoInstruction(functionDef.ByteCode, BCPushNullStr);
                        ci.typeReturned = returnInfo.typeReturned;
                    }
                }
                else if((returnInfo.typeReturned.type <= DataType_Float) && (ci.typeReturned.type <= DataType_Float))
                {
                    if((ci.typeReturned.type == DataType_Float) && (returnInfo.typeReturned.type == DataType_Integer))
                    {
                        DoInstruction(functionDef.ByteCode, BCCastInt);
                        ci.typeReturned = returnInfo.typeReturned;
                    }
                    else if((ci.typeReturned.type == DataType_Integer) && (returnInfo.typeReturned.type == DataType_Float))
                    {
                        DoInstruction(functionDef.ByteCode, BCCastFloat);
                        ci.typeReturned = returnInfo.typeReturned;
                    }
                }
                else if((returnInfo.typeReturned.type == DataType_Object) && (ci.typeReturned.type == DataType_Object))
                {
                    //todo: ??  great, is this empty on purpose or was stuff supposed to go here?
                    //might be empty on purpose, but that's retarded if so.  please shoot me if that's the case.
                }
                else if(returnInfo.typeReturned.type == DataType_Struct)
                {
                    AddError(TEXT("Cannot cast to a structure"));
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }
                else
                {
                    AddError(TEXT("Cannot cast '%s' to '%s'"), ci.typeReturned.name, returnInfo.typeReturned.name);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }
            }
            else
            {
                CodeInfo ci;
                if(!CompileSubCode(functionDef, curCodeSegment, ci, TEXT(")")))
                {
                    delete curCodeSegment;
                    return FALSE;
                }

                lastReturnType = returnInfo.typeReturned = ci.typeReturned;

                HandMeAToken(curToken);

                if(curToken[0] != ')')
                {
                    AddErrorExpecting(TEXT(")"), curToken);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }
            }
        }
        else if(type == TokenType_Keyword)
        {
            BOOL bDynamicCast;

            if(curToken == TEXT("this"))
            {
                if(!curClass && !curStruct)
                    AddError(TEXT("'%s' keyword can only be used in class/struct member functions"), TEXT("this"));

                if(curClass)
                    GetTypeInfo(functionDef.classContext->classData->GetName(), returnInfo.typeReturned);
                else if(curStruct)
                    GetTypeInfo(functionDef.structContext->name, returnInfo.typeReturned);
                lastReturnType = returnInfo.typeReturned;

                PeekAtAToken(curToken);
                if(curToken[0] != '.')
                {
                    DoInstruction(functionDef.ByteCode, BCAPushThis);
                    PushAData(functionDef.ByteCode, returnInfo.typeReturned, FALSE);
                }

                bThis = TRUE;
            }
            else if(curToken == TEXT("create"))
            {
                HandMeAToken(curToken);

                BOOL bInit = TRUE;
                if(curToken == TEXT("noinit"))
                {
                    HandMeAToken(curToken);
                    bInit = FALSE;
                }

                ClassDefinition *classDef = Scripting->GetClassDef(curToken);

                if(!classDef)
                {
                    AddError(TEXT("Unknown class '%s'"), curToken.Array());
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                TypeInfo classType;
                GetTypeInfo(curToken, classType);

                CreateObj(functionDef.ByteCode, curToken);

                switch(ProcessConstructor(functionDef, curCodeSegment, classDef, NULL))
                {
                    case COMPILE_UNRECOVERABLE:
                        delete curCodeSegment;
                        return FALSE;

                    case COMPILE_ERROR:
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                }

                if(bInit)
                {
                    FunctionDefinition *func = Object::GetLocalClass()->scriptClass->GetFunction(TEXT("InitializeObject"));

                    DoInstruction(functionDef.ByteCode, BCAPushT);
                    CallClassFunc(functionDef.ByteCode, *func, FALSE);
                }

                lastReturnType = returnInfo.typeReturned = classType;
                PushTData(functionDef.ByteCode, classType);
            }
            else if(curToken == TEXT("null"))
            {
                returnInfo.typeReturned.type = DataType_Null;
                returnInfo.typeReturned.name = TEXT("null");
            }
            else if((bDynamicCast = (curToken == TEXT("cast") || curToken == TEXT("dynamic_cast"))) || curToken == TEXT("static_cast"))
            {
                HandMeAToken(curToken);

                if(curToken[0] != '<')
                {
                    AddErrorExpecting(TEXT("<"), curToken);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                String typeName;
                HandMeAToken(typeName);

                HandMeAToken(curToken);

                if(curToken[0] != '>')
                {
                    AddErrorExpecting(TEXT(">"), curToken);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                HandMeAToken(curToken);

                if(curToken[0] != '(')
                {
                    AddErrorExpecting(TEXT("("), curToken);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                CodeInfo ci;
                if(!CompileSubCode(functionDef, curCodeSegment, ci, TEXT(")")))
                {
                    delete curCodeSegment;
                    return FALSE;
                }

                if(ci.bErrorsOccured)
                {
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                GetTypeInfo(typeName, returnInfo.typeReturned);
                lastReturnType = returnInfo.typeReturned;

                if(ci.typeReturned.type == DataType_Null)
                {
                    if(returnInfo.typeReturned.type == DataType_Object || returnInfo.typeReturned.type == DataType_Handle)
                    {
                        DoInstruction(functionDef.ByteCode, BCPushNullObj);
                        ci.typeReturned = returnInfo.typeReturned;
                    }
                    else if(returnInfo.typeReturned.type == DataType_String)
                    {
                        DoInstruction(functionDef.ByteCode, BCPushNullStr);
                        ci.typeReturned = returnInfo.typeReturned;
                    }
                }
                else if((returnInfo.typeReturned.type <= DataType_Float) && (ci.typeReturned.type <= DataType_Float))
                {
                    if((ci.typeReturned.type == DataType_Float) && (returnInfo.typeReturned.type == DataType_Integer))
                    {
                        DoInstruction(functionDef.ByteCode, BCCastInt);
                        ci.typeReturned = returnInfo.typeReturned;
                    }
                    else if((ci.typeReturned.type == DataType_Integer) && (returnInfo.typeReturned.type == DataType_Float))
                    {
                        DoInstruction(functionDef.ByteCode, BCCastFloat);
                        ci.typeReturned = returnInfo.typeReturned;
                    }
                }
                else if((returnInfo.typeReturned.type == DataType_Object) && (ci.typeReturned.type == DataType_Object))
                {
                    if(bDynamicCast)
                        DynamicCast(functionDef.ByteCode, typeName);
                }
                else if(returnInfo.typeReturned.type == DataType_Struct)
                {
                    AddError(TEXT("Cannot cast to a structure"));
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }
                else
                {
                    AddError(TEXT("Cannot cast '%s' to '%s'"), ci.typeReturned.name, returnInfo.typeReturned.name);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                HandMeAToken(curToken);

                if(curToken[0] != ')')
                {
                    AddErrorExpecting(TEXT(")"), curToken);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                PeekAtAToken(nextToken);
                if(nextToken[0] == '.')
                {
                    TPushStack(functionDef.ByteCode, ci.typeReturned.size);
                    bPopTempObject = TRUE;
                }
            }
        }
        else if(type == TokenType_Enum)
        {
            int enumVal = GetEnumVal(curToken);

            if(preferredType == DataType_Float)
            {
                AddWarning(TEXT("Converting enum to floating point value"));
                GetTypeInfo(TEXT("float"), returnInfo.typeReturned);
                PushFloat(functionDef.ByteCode, (float)enumVal);
            }
            else
            {
                PushInt(functionDef.ByteCode, enumVal);
                GetTypeInfo(TEXT("int"), returnInfo.typeReturned);
            }
        }
        else if(type == TokenType_String)
        {
            GetTypeInfo(TEXT("string"), returnInfo.typeReturned);
            lastReturnType = returnInfo.typeReturned;

            PushString(functionDef.ByteCode, GetActualString(curToken));
        }
        else if(type == TokenType_Character)
        {
            GetTypeInfo(TEXT("char"), returnInfo.typeReturned);

            int val = 0;
            if(!GetActualCharacter(curToken, val))
                AddError(TEXT("Invalid character '%s'"), curToken);

            PushInt(functionDef.ByteCode, val);
        }
        else if((type == TokenType_Type) || (type == TokenType_EnumDef) || (type == TokenType_Class) || (type == TokenType_Struct))
        {
            String typeName = curToken;

            PeekAtAToken(curToken);

            if(curToken[0] == '(')
            {
                if(type == TokenType_Struct) //calling struct constructor
                {
                    StructDefinition *structDef = Scripting->GetStruct(typeName);

                    if(!structDef)
                    {
                        AddError(TEXT("Unknown struct '%s'"), curToken.Array());
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }

                    TypeInfo structType;
                    GetTypeInfo(typeName, structType);

                    TPushZero(functionDef.ByteCode, structType.size);

                    switch(ProcessConstructor(functionDef, curCodeSegment, NULL, structDef))
                    {
                        case COMPILE_UNRECOVERABLE:
                            delete curCodeSegment;
                            return FALSE;

                        case COMPILE_ERROR:
                            delete curCodeSegment;
                            return GotoClosingToken(endTokenPriority);
                    }

                    lastReturnType = returnInfo.typeReturned = structType;
                    PushTData(functionDef.ByteCode, structType);
                }
                else //regular cast
                {
                    HandMeAToken(curToken);

                    CodeInfo ci;
                    if(!CompileSubCode(functionDef, curCodeSegment, ci, TEXT(")")))
                    {
                        delete curCodeSegment;
                        return FALSE;
                    }

                    GetTypeInfo(typeName, returnInfo.typeReturned);

                    if(ci.typeReturned.type == DataType_Null)
                    {
                        if(returnInfo.typeReturned.type == DataType_Object || returnInfo.typeReturned.type == DataType_Handle)
                        {
                            DoInstruction(functionDef.ByteCode, BCPushNullObj);
                            ci.typeReturned = returnInfo.typeReturned;
                        }
                        else if(returnInfo.typeReturned.type == DataType_String)
                        {
                            DoInstruction(functionDef.ByteCode, BCPushNullStr);
                            ci.typeReturned = returnInfo.typeReturned;
                        }
                    }
                    else if((returnInfo.typeReturned.type <= DataType_Float) && (ci.typeReturned.type <= DataType_Float))
                    {
                        if((ci.typeReturned.type == DataType_Float) && (returnInfo.typeReturned.type == DataType_Integer))
                        {
                            DoInstruction(functionDef.ByteCode, BCCastInt);
                            ci.typeReturned = returnInfo.typeReturned;
                        }
                        else if((ci.typeReturned.type == DataType_Integer) && (returnInfo.typeReturned.type == DataType_Float))
                        {
                            DoInstruction(functionDef.ByteCode, BCCastFloat);
                            ci.typeReturned = returnInfo.typeReturned;
                        }
                    }
                    else
                    {
                        AddError(TEXT("Cannot cast '%s' to '%s'"), ci.typeReturned.name, returnInfo.typeReturned.name);
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }

                    HandMeAToken(curToken);

                    if(curToken[0] != ')')
                    {
                        AddErrorExpecting(TEXT(")"), curToken);
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }
                }
            }
            else //not casting/constructing, push type
            {
                GetTypeInfo(TEXT("type"), returnInfo.typeReturned);

                TypeInfo ti;
                GetTypeInfo(typeName, ti);

                TypeDataInfo tri;
                scpy_n(tri.name, ti.name, 63);
                PushType(functionDef.ByteCode, tri);
            }
        }
        else if(type == TokenType_Number)
        {
            if(prefixOperator == TEXT("-"))
            {
                curToken = prefixOperator + curToken;
                prefixOperator.Clear();
            }

            if(!ValidIntString(curToken))
            {
                float chi = (float)tstof(curToken);

                if(preferredType == DataType_Integer)
                {
                    PushInt(functionDef.ByteCode, (int)chi);
                    GetTypeInfo(TEXT("int"), returnInfo.typeReturned);
                }
                else
                {
                    PushFloat(functionDef.ByteCode, chi);
                    GetTypeInfo(TEXT("float"), returnInfo.typeReturned);
                }
            }
            else
            {
                int chi = tstoi(curToken);

                if(preferredType == DataType_Float)
                {
                    PushFloat(functionDef.ByteCode, (float)chi);
                    GetTypeInfo(TEXT("float"), returnInfo.typeReturned);
                }
                else
                {
                    PushInt(functionDef.ByteCode, chi);
                    GetTypeInfo(TEXT("int"), returnInfo.typeReturned);
                }
            }
        }
        else if(curToken == TEXT("true"))
        {
            if(preferredType == DataType_Float)
            {
                AddWarning(TEXT("Converting boolean value to floating point value"));
                PushFloat(functionDef.ByteCode, 1.0f);
                GetTypeInfo(TEXT("float"), returnInfo.typeReturned);
            }
            else
            {
                PushInt(functionDef.ByteCode, 1);
                GetTypeInfo(TEXT("bool"), returnInfo.typeReturned);
            }
        }
        else if(curToken == TEXT("false"))
        {
            if(preferredType == DataType_Float)
            {
                AddWarning(TEXT("Converting boolean value to floating point value"));
                PushFloat(functionDef.ByteCode, 0.0f);
                GetTypeInfo(TEXT("float"), returnInfo.typeReturned);
            }
            else
            {
                PushInt(functionDef.ByteCode, 0);
                GetTypeInfo(TEXT("bool"), returnInfo.typeReturned);
            }
        }
        else if((type != TokenType_Variable) && (type != TokenType_Function))
        {
            AddErrorNoClue(curToken);
            delete curCodeSegment;
            return GotoClosingToken(endTokenPriority);
        }

        PeekAtAToken(nextToken);
        BOOL bIsDot = (nextToken[0] == '.' && lastReturnType.type != DataType_Void);

        if(bIsDot || (type == TokenType_Variable) || (type == TokenType_Function))
        {
            List<TypeInfo> popData;
            BOOL bLValue = TRUE;
            BOOL bWasFunction = FALSE;

            if(!bIsDot)
                lpTemp -= curToken.Length();

            if(bPopTempObject)
            {
                popData << lastReturnType;
                DoInstruction(functionDef.ByteCode, BCAPushT);
            }

            switch(ProcessFuncOrVar(functionDef, curCodeSegment, curVar, bDoingSuper, bThis, bWasFunction, bLValue, varStartPos, popData, lastReturnType, bStaticFunc))
            {
                case COMPILE_UNRECOVERABLE:
                    delete curCodeSegment;
                    return FALSE;

                case COMPILE_ERROR:
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
            }

            if(!bWasFunction)
            {
                PeekAtAToken(curToken);

                if(bLValue)
                {
                    if(!returnInfo.OutVarData.Num())
                        returnInfo.OutVarData.CopyArray(functionDef.ByteCode+varStartPos, functionDef.ByteCode.Num()-varStartPos);

                    if(popData.Num())
                        TPopTypes(returnInfo.OutVarData, popData);
                }

                TokenType nextType = curCodeSegment->GetTokenType(curToken);

                BOOL bCalledIndDec = FALSE;

                if(nextType == TokenType_PostfixOperator)
                {
                    PushAData(functionDef.ByteCode, curVar->typeInfo, TRUE);

                    HandMeAToken(curToken);

                    if(IncrementOrDecrementPrefix)
                    {
                        AddError(TEXT("Cannot use a prefix operator and a postfix operator at the same time"));
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }

                    if(!bLValue)
                    {
                        AddError(TEXT("Structure data returned from a function cannot be used with increment/decrement operators"));
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }

                    PushAData(functionDef.ByteCode, curVar->typeInfo, TRUE);

                    if(curVar->typeInfo.type == DataType_Float)
                    {
                        PushFloat(functionDef.ByteCode, 1.0f);
                        DoInstruction(functionDef.ByteCode, (curToken == TEXT("++")) ? BCAddF : BCSubtractF);
                    }
                    else if(curVar->typeInfo.type == DataType_Integer)
                    {
                        PushInt(functionDef.ByteCode, 1);
                        DoInstruction(functionDef.ByteCode, (curToken == TEXT("++")) ? BCAdd : BCSubtract);
                    }
                    else
                    {
                        AddError(TEXT("Expecting Numeric Variable: '%s'"), (TSTR)curToken);
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }

                    APopVar(functionDef.ByteCode, curVar, FALSE);

                    PeekAtAToken(curToken);
                    nextType = curCodeSegment->GetTokenType(curToken);

                    bCalledIndDec = TRUE;
                }
                else if(IncrementOrDecrementPrefix)
                {
                    PushAData(functionDef.ByteCode, curVar->typeInfo, TRUE);

                    if(!bLValue)
                    {
                        AddError(TEXT("Structure data returned from a function cannot be used with increment/decrement operators"));
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }

                    if(curVar->typeInfo.type == DataType_Float)
                    {
                        PushFloat(functionDef.ByteCode, 1.0f);
                        DoInstruction(functionDef.ByteCode, (IncrementOrDecrementPrefix == 1) ? BCAddF : BCSubtractF);
                    }
                    else if(curVar->typeInfo.type == DataType_Integer)
                    {
                        PushInt(functionDef.ByteCode, 1);
                        DoInstruction(functionDef.ByteCode, (IncrementOrDecrementPrefix == 1) ? BCAdd : BCSubtract);
                    }
                    else
                    {
                        AddErrorExpecting(TEXT("Numeric variable"), curVar->typeInfo.name);
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }

                    APushDup(functionDef.ByteCode);
                    APopVar(functionDef.ByteCode, curVar, FALSE);

                    PushAData(functionDef.ByteCode, curVar->typeInfo, FALSE);

                    bCalledIndDec = TRUE;
                }
                else
                {
                    if((nextType != TokenType_AssignmentOperator) || curToken.Length() > 1)
                        PushAData(functionDef.ByteCode, lastReturnType, FALSE);
                }

                if(nextType == TokenType_AssignmentOperator)
                {
                    if(bCalledIndDec)
                    {
                        AddError(TEXT("Cannot use an assignment operator after having used an increment/decrement operator"));
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }
                }
            }
            else
            {
                if(IncrementOrDecrementPrefix)
                {
                    AddError(TEXT("Cannot use an increment/decrement on a function return value"));
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                PushTData(functionDef.ByteCode, lastReturnType);
                popData.Remove(0);
            }

            if(popData.Num())
                TPopTypes(functionDef.ByteCode, popData);

            returnInfo.typeReturned = lastReturnType;
            if(curVar)
                returnInfo.subType = curVar->subTypeInfo;
        }

        if(!prefixOperator.IsEmpty())
        {
            PeekAtAToken(curToken);
            TokenType nextType = curCodeSegment->GetTokenType(curToken);

            if(nextType == TokenType_AssignmentOperator)
            {
                AddError(TEXT("Cannot use an assignment operator after having used a prefix operator"));
                delete curCodeSegment;
                return GotoClosingToken(endTokenPriority);
            }

            if(returnInfo.typeReturned.type <= DataType_Float)
            {
                if(prefixOperator.Compare(TEXT("!")))
                {
                    if(returnInfo.typeReturned.type > DataType_Integer)
                    {
                        AddError(TEXT("Expecting Integer Variable: '%s'"), (TSTR)curToken);
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }

                    DoInstruction(functionDef.ByteCode, BCLogicalNot);
                    GetTypeInfo(TEXT("int"), returnInfo.typeReturned);
                }
                else if(prefixOperator.Compare(TEXT("~")))
                {
                    if(returnInfo.typeReturned.type > DataType_Integer)
                    {
                        AddError(TEXT("Expecting Integer Variable: '%s'"), (TSTR)curToken);
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }

                    DoInstruction(functionDef.ByteCode, BCBitwiseNot);
                }
                else if(prefixOperator.Compare(TEXT("-")))
                {
                    if(returnInfo.typeReturned.type > DataType_Float)
                    {
                        AddError(TEXT("Expecting Numeric Variable: '%s'"), (TSTR)curToken);
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }

                    DoInstruction(functionDef.ByteCode, (returnInfo.typeReturned.type == DataType_Float) ? BCNegateF : BCNegate);
                }
            }
            else if(returnInfo.typeReturned.type == DataType_Object || returnInfo.typeReturned.type == DataType_Handle)
            {
                if(prefixOperator == TEXT("!"))
                {
                    DoInstruction(functionDef.ByteCode, BCPushNullObj);
                    DoInstruction(functionDef.ByteCode, BCEqualO);

                    GetTypeInfo(TEXT("bool"), returnInfo.typeReturned);
                }
                else
                {
                    AddError(TEXT("Cannot use '%s' prefix operator on objects/handles"), (TSTR)prefixOperator);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }
            }
            else
            {
                FunctionDefinition *callFunc = Scripting->GetOperator(prefixOperator, returnInfo.typeReturned, NULL);
                if(callFunc)
                {
                    CallFunc(functionDef.ByteCode, *callFunc);
                    PushTData(functionDef.ByteCode, callFunc->returnType);
                    returnInfo.typeReturned = callFunc->returnType;
                }
                else
                {
                    if(returnInfo.typeReturned.type == DataType_Struct)
                    {
                        StructDefinition *structDef = Scripting->GetStruct(returnInfo.typeReturned.name);

                        while(callFunc = structDef->GetNextFunction(prefixOperator, callFunc, TRUE))
                        {
                            if(callFunc->Params.Num() == 0)
                            {
                                CallStructOp(functionDef.ByteCode, *callFunc, structDef->name);
                                PushTData(functionDef.ByteCode, callFunc->returnType);
                                returnInfo.typeReturned = callFunc->returnType;
                                break;
                            }
                        }
                    }

                    if(!callFunc)
                    {
                        AddError(TEXT("Operator '%s' for type '%s' was not found"), (TSTR)prefixOperator, (TSTR)lastReturnType.name);
                        delete curCodeSegment;
                        return GotoClosingToken(endTokenPriority);
                    }
                }
            }
        }

        DWORD firstOutPos = functionDef.ByteCode.Num();

        PeekAtAToken(curOperator);
        TokenType operatorType = curCodeSegment->GetTokenType(curOperator);

        List<DWORD> LogicalExitOffsets;

        while((operatorType == TokenType_AssignmentOperator) || !IsClosingToken(curOperator, endTokenPriority))
        {
            HandMeAToken(curOperator);

            if(curOperator[0] == '?')
            {
                CodeInfo ciTrue, ciFalse;

                if(returnInfo.typeReturned.type == DataType_Object || returnInfo.typeReturned.type == DataType_Handle)
                {
                    DoInstruction(functionDef.ByteCode, BCPushNullObj);
                    DoInstruction(functionDef.ByteCode, BCNotEqualO);
                    GetTypeInfo(TEXT("bool"), returnInfo.typeReturned);
                }
                else if(returnInfo.typeReturned.type != DataType_Integer)
                {
                    AddErrorExpecting(TEXT("expression"), returnInfo.typeReturned.name);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                DWORD jumpPos;
                JumpIfNot(functionDef.ByteCode, jumpPos);

                if(!CompileSubCode(functionDef, curCodeSegment, ciTrue, TEXT(":")))
                {
                    delete curCodeSegment;
                    return FALSE;
                }

                DWORD jumpOut;
                JumpUndefined(functionDef.ByteCode, jumpOut);

                *(DWORD*)(functionDef.ByteCode+jumpPos) = functionDef.ByteCode.Num()-jumpPos-4;

                HandMeAToken(curOperator);
                if(curOperator[0] != ':')
                {
                    AddErrorExpecting(TEXT(":"), curOperator);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                if(!CompileSubCode(functionDef, curCodeSegment, ciFalse))
                {
                    delete curCodeSegment;
                    return FALSE;
                }

                if(ciTrue.typeReturned.type == DataType_Null)
                {
                    if(ciFalse.typeReturned.type == DataType_String)
                    {
                        functionDef.ByteCode.Insert(ciTrue.outPos, BCPushNullStr);
                        ciTrue.typeReturned = ciFalse.typeReturned;
                    }
                    else if(ciFalse.typeReturned.type == DataType_Object || ciFalse.typeReturned.type == DataType_Handle)
                    {
                        functionDef.ByteCode.Insert(ciTrue.outPos, BCPushNullObj);
                        ciTrue.typeReturned = ciFalse.typeReturned;
                    }
                }
                else if(ciFalse.typeReturned.type == DataType_Null)
                {
                    if(ciTrue.typeReturned.type == DataType_String)
                    {
                        functionDef.ByteCode.Insert(ciFalse.outPos, BCPushNullStr);
                        ciFalse.typeReturned = ciTrue.typeReturned;
                    }
                    else if(ciTrue.typeReturned.type == DataType_Object || ciTrue.typeReturned.type == DataType_Handle)
                    {
                        functionDef.ByteCode.Insert(ciFalse.outPos, BCPushNullObj);
                        ciFalse.typeReturned = ciTrue.typeReturned;
                    }
                }

                if(ciTrue.typeReturned != ciFalse.typeReturned)
                {
                    AddError(TEXT("Two types given by conditional operator do not match or cannot be used together: '%s' and '%s'"), ciTrue.typeReturned.name, ciFalse.typeReturned.name);
                    delete curCodeSegment;
                    return GotoClosingToken(endTokenPriority);
                }

                *(DWORD*)(functionDef.ByteCode+jumpOut) = ciFalse.outPos-jumpOut-4;

                PeekAtAToken(curOperator);

                returnInfo.OutVarData.Clear();
                returnInfo.typeReturned = ciFalse.typeReturned;

                firstOutPos = functionDef.ByteCode.Num();

                bGotActualFloat = FALSE;
                continue;
            }

            if((operatorType < TokenType_AssignmentOperator) || (operatorType > TokenType_ConditionalOperator))
            {
                AddError(TEXT("Expecting an operator, instead got '%s'"), (TSTR)curOperator);
                delete curCodeSegment;
                return GotoClosingToken(endTokenPriority);
            }

            CodeInfo ci;
            if(!CompileSubCode(functionDef, curCodeSegment, ci, (operatorType == TokenType_AssignmentOperator) ? TEXT("!=") : curOperator))
            {
                delete curCodeSegment;
                return FALSE;
            }

            if(ci.bErrorsOccured)
            {
                delete curCodeSegment;
                return GotoClosingToken(endTokenPriority);
            }

            if(ci.typeReturned.type == DataType_Null)
            {
                if(returnInfo.typeReturned.type == DataType_Object || returnInfo.typeReturned.type == DataType_Handle)
                {
                    DoInstruction(functionDef.ByteCode, BCPushNullObj);
                    ci.typeReturned = returnInfo.typeReturned;
                }
                else if(returnInfo.typeReturned.type == DataType_String)
                {
                    DoInstruction(functionDef.ByteCode, BCPushNullStr);
                    ci.typeReturned = returnInfo.typeReturned;
                }
            }
            else if((ci.typeReturned.type == DataType_Float) && (returnInfo.typeReturned.type == DataType_Integer))
            {
                DoInstruction(functionDef.ByteCode, BCCastInt);
                ci.typeReturned = returnInfo.typeReturned;
            }
            else if((ci.typeReturned.type == DataType_Integer) && (returnInfo.typeReturned.type == DataType_Float))
            {
                DoInstruction(functionDef.ByteCode, BCCastFloat);
                ci.typeReturned = returnInfo.typeReturned;
            }
            //todo - ooooooo, cast conversion operator thingies! (later note: actually meh, don't need 'em, plus I'm too lazy for that)

            DWORD secondOutPos = functionDef.ByteCode.Num();

            if(curOperator.Compare(TEXT("||")))
            {
                if(returnInfo.typeReturned.type == DataType_Object || returnInfo.typeReturned.type == DataType_Handle)
                {
                    functionDef.ByteCode.Insert(firstOutPos++, BCPushNullObj);
                    functionDef.ByteCode.Insert(firstOutPos++, BCNotEqualO);
                    GetTypeInfo(TEXT("bool"), returnInfo.typeReturned);
                }

                if(ci.typeReturned.type == DataType_Object || ci.typeReturned.type == DataType_Handle)
                {
                    DoInstruction(functionDef.ByteCode, BCPushNullObj);
                    DoInstruction(functionDef.ByteCode, BCNotEqualO);
                    GetTypeInfo(TEXT("bool"), ci.typeReturned);
                }

                if((returnInfo.typeReturned.type != DataType_Integer) || (ci.typeReturned.type != DataType_Integer))
                    AddError(TEXT("Expecting expression value for operator '%s'"), curOperator.Array());

                List<BYTE> logicalOrCode;
                JumpOffsetIfNot(logicalOrCode, 14);
                PushInt(logicalOrCode, TRUE);
                JumpUndefined(logicalOrCode, *LogicalExitOffsets.CreateNew());

                LogicalExitOffsets.Last() += firstOutPos;

                functionDef.ByteCode.InsertList(firstOutPos, logicalOrCode);
            }
            else if(curOperator.Compare(TEXT("&&")))
            {
                if(returnInfo.typeReturned.type == DataType_Object || returnInfo.typeReturned.type == DataType_Handle)
                {
                    functionDef.ByteCode.Insert(firstOutPos++, BCPushNullObj);
                    functionDef.ByteCode.Insert(firstOutPos++, BCNotEqualO);
                    GetTypeInfo(TEXT("bool"), returnInfo.typeReturned);
                }

                if(ci.typeReturned.type == DataType_Object || ci.typeReturned.type == DataType_Handle)
                {
                    DoInstruction(functionDef.ByteCode, BCPushNullObj);
                    DoInstruction(functionDef.ByteCode, BCNotEqualO);
                    GetTypeInfo(TEXT("bool"), ci.typeReturned);
                }

                if((returnInfo.typeReturned.type != DataType_Integer) || (ci.typeReturned.type != DataType_Integer))
                    AddError(TEXT("Expecting expression value for operator '%s'"), curOperator.Array());

                List<BYTE> logicalAndCode;
                JumpOffsetIf(logicalAndCode, 14);
                PushInt(logicalAndCode, FALSE);
                JumpUndefined(logicalAndCode, *LogicalExitOffsets.CreateNew());

                LogicalExitOffsets.Last() += firstOutPos;

                functionDef.ByteCode.InsertList(firstOutPos, logicalAndCode);
            }
            else
            {
                BOOL bCalledFunction, bAlreadyErrored = FALSE;
                if(!DoOperator(functionDef.ByteCode, curOperator, returnInfo.typeReturned, ci.typeReturned, bCalledFunction))
                {
                    AddError(TEXT("Invalid types used for operator: '%s %s %s'"), returnInfo.typeReturned.name, (TSTR)curToken, ci.typeReturned.name);
                    bAlreadyErrored = TRUE;
                }

                if(bCalledFunction)
                    PushTData(functionDef.ByteCode, returnInfo.typeReturned);

                if(operatorType == TokenType_AssignmentOperator)
                {
                    if(curVar)
                    {
                        int startPos = functionDef.ByteCode.Num();

                        List<BYTE> pushVarData;

                        if(!returnInfo.OutVarData.Num())
                            AddError(TEXT("Assignment operator requires a variable destination"));

                        if(curOperator.Length() > 1)
                        {
                            if(!DoOperator(functionDef.ByteCode, String(TEXT("=")), returnInfo.typeReturned, returnInfo.typeReturned, bCalledFunction) && !bAlreadyErrored)
                                AddError(TEXT("Invalid types used for operator: '%s %s %s'"), returnInfo.typeReturned.name, (TSTR)curToken, returnInfo.typeReturned.name);
                        }

                        if(bCalledFunction)
                            PushTData(functionDef.ByteCode, returnInfo.typeReturned);

                        //PushDup(functionDef.ByteCode, returnInfo.typeReturned);
                        APushDup(functionDef.ByteCode);
                        APopVar(functionDef.ByteCode, curVar, TRUE);
                        PushAData(functionDef.ByteCode, curVar->typeInfo, FALSE);
                    }
                    else
                        AddError(TEXT("Assignment operator requires a variable destination"));
                }
            }

            returnInfo.OutVarData.Clear();

            firstOutPos = functionDef.ByteCode.Num();

            PeekAtAToken(curOperator);
            operatorType = curCodeSegment->GetTokenType(curOperator);
        }

        for(int i=0; i<LogicalExitOffsets.Num(); i++)
            *(DWORD*)&functionDef.ByteCode[LogicalExitOffsets[i]] = functionDef.ByteCode.Num()-(LogicalExitOffsets[i]+4);

        bGotActualFloat = FALSE;
    }

    returnInfo.outPos = functionDef.ByteCode.Num();
    returnInfo.bErrorsOccured = (errorCount > startErrorCount);

    delete curCodeSegment;
    return !bEndOfCode;
}



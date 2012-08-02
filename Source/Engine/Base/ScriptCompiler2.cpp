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
        return COMPILE_UNRECOVERABLE;

#define HandMeAToken(str) \
    if(!GetNextToken(str)) \
        return COMPILE_UNRECOVERABLE;


CompileErrorType Compiler::CompileParams(FunctionDefinition &functionDef, CodeSegment *curCodeSegment, CodeInfoList &paramData)
{
    String curToken;

    PeekAtAToken(curToken);
    if(curToken[0] == ')')
    {
        HandMeAToken(curToken);
        return COMPILE_SUCCESS;
    }

    while(curToken[0] != ')')
    {
        CodeInfo &ci = *paramData.CreateNew();
        ci.typeReturned.type = DataType_Void;

        PeekAtAToken(curToken);

        if((curToken[0] != ',') && (curToken[0] != ')')) //default param
        {
            if(!CompileSubCode(functionDef, curCodeSegment, ci))
                return COMPILE_UNRECOVERABLE;
        }
        else
            ci.outPos = functionDef.ByteCode.Num();

        if(ci.bErrorsOccured)
            return COMPILE_ERROR;

        HandMeAToken(curToken);

        if((curToken[0] != ',') && (curToken[0] != ')'))
        {
            AddErrorExpecting(TEXT(")' or ',"), curToken);
            return COMPILE_ERROR;
        }
    }

    return COMPILE_SUCCESS;
}


CompileErrorType Compiler::ProcessFuncOrVar(FunctionDefinition &functionDef, CodeSegment *curCodeSegment, Variable *&curVar, BOOL &bDoingSuper, BOOL bThis, BOOL &bWasFunction, BOOL &bLValue, DWORD varStartPos, List<TypeInfo> &popData, TypeInfo &returnType, BOOL bCallingStatic)
{
    String curToken;

    ClassDefinition *classDef = curClass;
    StructDefinition *structDef = curStruct;

    BOOL bMember = FALSE;
    TokenType type;
    BOOL bFirst;

    PeekAtAToken(curToken);

    if(bThis)
    {
        HandMeAToken(curToken);
        PeekAtAToken(curToken);
    }

    if(curToken[0] == '.')
    {
        bFirst = FALSE;

        HandMeAToken(curToken);

        if( (returnType.type == DataType_List)    ||
            (returnType.type == DataType_Integer) ||
            (returnType.type == DataType_Float)   ||
            (returnType.type == DataType_String)  )
        {
            return ProcessTypeFunctions(functionDef, curCodeSegment, curVar, bDoingSuper, bWasFunction, bLValue, varStartPos, popData, returnType);
        }

        HandMeAToken(curToken);

        if(bWasFunction)
            DoInstruction(functionDef.ByteCode, BCAPushT);

        if(returnType.type == DataType_Struct)
        {
            BOOL bPriorVar = curVar != NULL;

            structDef = Scripting->GetStruct(returnType.name);
            curVar = structDef->GetVariable(curToken);
            classDef = NULL;
            bMember = TRUE;

            if(bThis)
                bLValue = TRUE;
            else if(bWasFunction || !bPriorVar)
                bLValue = FALSE;

            if(!curVar)
                type = TokenType_Function;
            else
                type = TokenType_Variable;
        }
        else if(returnType.type == DataType_Object)
        {
            classDef = Scripting->GetClassDef(returnType.name);
            curVar = classDef->GetVariable(curToken);
            structDef = NULL;
            bMember = TRUE;

            bLValue = TRUE;

            if(!curVar)
                type = TokenType_Function;
            else
                type = TokenType_Variable;
        }
        else
        {
            AddError(TEXT("Cannot use a member operator on '%s'"), returnType.name);
            return COMPILE_ERROR;
        }
    }
    else if(bThis)
    {
        bLValue = bFirst = TRUE;
        varStartPos = functionDef.ByteCode.Num();
        HandMeAToken(curToken);

        returnType.type = DataType_Void;

        if(curClass)
            curVar = curClass->GetVariable(curToken);
        else if(curStruct)
            curVar = curStruct->GetVariable(curToken);

        if(!curVar)
        {
            AddError(TEXT("Could not find the variable '%s'"), curToken);
            return COMPILE_ERROR;
        }

        type = TokenType_Variable;
    }
    else
    {
        bLValue = bFirst = TRUE;
        varStartPos = functionDef.ByteCode.Num();
        HandMeAToken(curToken);

        type = curCodeSegment->GetTokenType(curToken);
    }

    if(type == TokenType_Variable)
    {
        if(!curVar && !bWasFunction)
            curVar = curCodeSegment->GetVariable(curToken);

        if(!curVar)
        {
            AddError(TEXT("Could not find the variable '%s'"), curToken);
            return COMPILE_ERROR;
        }

        if(bFirst && functionDef.flags & FUNC_STATIC)
        {
            if((curClass && curVar->scope == VarScope_Class) || (curStruct && curVar->scope == VarScope_Struct))
            {
                AddError(TEXT("Cannot access member variables from static functions"));
                return COMPILE_ERROR;
            }
        }

        if(bFirst && bDoingSuper)
        {
            AddError(TEXT("Super-scope access is restricted to functions because variables cannot be redefined"));
            return COMPILE_ERROR;
        }

        BOOL bArrayElement = FALSE;
        BOOL bGettingArrayNum = FALSE;

        if(curVar->numElements || curVar->typeInfo.type == DataType_List)
        {
            PeekAtAToken(curToken);

            if(curToken[0] != '[')
            {
                if(curToken[0] == '.')
                {
                    if(curVar->typeInfo.type != DataType_List)
                    {
                        HandMeAToken(curToken);
                        HandMeAToken(curToken);

                        if(curToken == TEXT("Num"))
                        {
                            bGettingArrayNum = TRUE;

                            PeekAtAToken(curToken);
                            if(curToken == TEXT("("))
                            {
                                HandMeAToken(curToken);
                                PeekAtAToken(curToken);
                                if(curToken == TEXT(")"))
                                {
                                    HandMeAToken(curToken);
                                }
                                else
                                {
                                     AddErrorExpecting(TEXT(")"), curToken);
                                     return COMPILE_ERROR;
                                }
                            }
                            else
                            {
                                AddErrorExpecting(TEXT("("), curToken);
                                return COMPILE_ERROR;
                            }
                        }
                    }
                }
                else if(curVar->typeInfo.type != DataType_List)
                {
                    AddErrorExpecting(TEXT("["), curToken);
                    return COMPILE_ERROR;
                }
            }
            else
            {
                HandMeAToken(curToken);

                CodeInfo ci;
                if(!CompileSubCode(functionDef, curCodeSegment, ci, TEXT("]")))
                    return COMPILE_UNRECOVERABLE;

                bArrayElement = TRUE;

                HandMeAToken(curToken);

                if(curToken[0] != ']')
                {
                    AddErrorExpecting(TEXT("]"), curToken);
                    return COMPILE_ERROR;
                }
            }
        }

        if(bGettingArrayNum)
        {
            PushInt(functionDef.ByteCode, curVar->numElements);
            GetTypeInfo(TEXT("int"), returnType);
            bWasFunction = TRUE;
            bLValue = FALSE;
        }
        else if(bArrayElement && curVar->typeInfo.type == DataType_List)
        {
            if(returnType.type == DataType_Struct)
                APushSVar(functionDef.ByteCode, curVar, bWasFunction, FALSE);
            else if(returnType.type == DataType_Object)
                APushCVar(functionDef.ByteCode, classDef, curVar, bWasFunction, FALSE);
            else
                APush(functionDef.ByteCode, curVar, FALSE);

            APushListItem(functionDef.ByteCode, curVar);

            bLValue = (bLValue || (curVar->subTypeInfo.type == DataType_Object));

            returnType = curVar->subTypeInfo;
            bWasFunction = FALSE;
        }
        else
        {
            if(returnType.type == DataType_Struct)
                APushSVar(functionDef.ByteCode, curVar, bWasFunction, bArrayElement);
            else if(returnType.type == DataType_Object)
                APushCVar(functionDef.ByteCode, classDef, curVar, bWasFunction, bArrayElement);
            else
                APush(functionDef.ByteCode, curVar, bArrayElement);

            bLValue = (bLValue || (curVar->typeInfo.type == DataType_Object));

            returnType = curVar->typeInfo;
            bWasFunction = FALSE;
        }
    }
    else if(type == TokenType_Function)
    {
        String strName = curToken;

        HandMeAToken(curToken);

        if(curToken[0] != '(')
        {
            AddErrorNoClue(strName);
            return COMPILE_ERROR;
        }

        BOOL bEvilUnknownStructItem = (structDef && !bFirst && !bLValue && !bWasFunction && !bCallingStatic);
        if(bEvilUnknownStructItem)
            DoInstruction(functionDef.ByteCode, BCAPushStack);

        CodeInfoList paramData;

        CompileErrorType errorType = CompileParams(functionDef, curCodeSegment, paramData);
        if(errorType != COMPILE_SUCCESS)
            return errorType;

        if(bFirst && bDoingSuper)
        {
            if(classDef && !classDef->Parent)
            {
                AddError(TEXT("...trying to do super inside of Object?  You must realize that such a thing is impossible"));
                return COMPILE_ERROR;
            }
            else if(structDef && !structDef->Parent)
            {
                AddError(TEXT("Cannot use super in a base structure"));
                return COMPILE_ERROR;
            }
        }

        ClassDefinition  *topClass = NULL;
        StructDefinition *topStruct = NULL;

        if(classDef)
            topClass = bDoingSuper ? classDef->Parent : classDef;
        else if(structDef)
            topStruct = bDoingSuper ? structDef->Parent : structDef;

        FunctionDefinition *callFunc = GetMatchingFunction(topClass, topStruct, strName, paramData, functionDef.ByteCode.Num(), bMember, !bCallingStatic);

        if(!callFunc)
        {
            if(callFunc = GetFunction(topClass, topStruct, strName))
                AddError(TEXT("'%s': Invalid parameters used or could not find matching function"), strName.Array());
            else
                AddError(TEXT("Could not find function '%s'"), strName.Array());

            return COMPILE_ERROR;
        }

        BOOL bStaticFunc = callFunc->flags & FUNC_STATIC;

        if(bFirst && functionDef.flags & FUNC_STATIC)
        {
            if((curClass || curStruct) && ~callFunc->flags & FUNC_STATIC)
            {
                AddError(TEXT("Cannot access non-static member functions from static functions"));
                return COMPILE_ERROR;
            }
        }

        if(bCallingStatic && !bStaticFunc)
        {
            AddError(TEXT("Function '%s' is not a static function"), strName.Array());
            return COMPILE_ERROR;
        }

        ProcessFunctionParamters(functionDef, callFunc->Params, paramData);

        BOOL bGlobalFunc = HIWORD(callFunc->funcOffset) != 0;

        if((classDef || structDef) && bFirst && !bGlobalFunc && !bStaticFunc)
            DoInstruction(functionDef.ByteCode, BCAPushThis);
        else if(bStaticFunc && !bFirst && !bCallingStatic)
            DoInstruction(functionDef.ByteCode, BCAPop);

        assert(bFirst || !bGlobalFunc || structDef);

        if(bGlobalFunc)
            CallFunc(functionDef.ByteCode, *callFunc);
        else if(structDef)
            CallStructFunc(functionDef.ByteCode, *callFunc, callFunc->structContext->name);
        else
        {
            if(bStaticFunc)
                CallClassStatic(functionDef.ByteCode, *callFunc, classDef);
            else
                CallClassFunc(functionDef.ByteCode, *callFunc, bDoingSuper);
        }

        for(int i=(callFunc->Params.Num()-1); i>=0; i--)
        {
            CodeInfo &match = paramData[i];
            DefaultVariable &param = callFunc->Params[i];

            if(param.flags & VAR_OUT)
            {
                if(!match.OutVarData.Num())
                {
                    AddError(TEXT("Parameter %d must have a valid variable for output"), i);
                    return COMPILE_ERROR;
                }
                else
                {
                    functionDef.ByteCode.AppendList(match.OutVarData);
                    APopVar(functionDef.ByteCode, &param, FALSE);
                }
            }
        }

        if(bEvilUnknownStructItem)
            Pop(functionDef.ByteCode, structDef->GetType());

        returnType = callFunc->returnType;

        popData.Insert(0, callFunc->returnType);

        bLValue = FALSE;

        bWasFunction = TRUE;
    }
    else
    {
        AddError(TEXT("Token '%s' is not a function or variable."), curToken.Array());
        return COMPILE_ERROR;
    }

    bDoingSuper = FALSE;

    PeekAtAToken(curToken);

    bFirst = FALSE;

    if(curToken[0] == '.')
        return ProcessFuncOrVar(functionDef, curCodeSegment, curVar, bDoingSuper, FALSE, bWasFunction, bLValue, varStartPos, popData, returnType);

    return COMPILE_SUCCESS;
}

CompileErrorType Compiler::ProcessTypeFunctions(FunctionDefinition &functionDef, CodeSegment *curCodeSegment, Variable *&curVar, BOOL &bDoingSuper, BOOL &bWasFunction, BOOL &bLValue, DWORD varStartPos, List<TypeInfo> &popData, TypeInfo &returnType)
{
    String curToken;

    String funcName;
    HandMeAToken(funcName);

    HandMeAToken(curToken);

    if(curToken[0] != '(')
    {
        AddErrorNoClue(strName);
        return COMPILE_ERROR;
    }

    CodeInfoList paramData;

    CompileErrorType errorType = CompileParams(functionDef, curCodeSegment, paramData);
    if(errorType != COMPILE_SUCCESS)
        return errorType;

    TypeFunction *callFunc = NULL;
    for(int i=0; i<Scripting->ScriptTypes.Num(); i++)
    {
        ScriptTypeData &scriptType = Scripting->ScriptTypes[i];

        if(scriptType.type == curVar->typeInfo)
        {
            BOOL bFoundFunc = FALSE;
            for(int j=0; j<scriptType.funcs.Num(); j++)
            {
                TypeFunction &typeFunc = scriptType.funcs[j];
                if(typeFunc.name == funcName)
                {
                    callFunc = &typeFunc;
                    bFoundFunc = TRUE;
                    break;
                }
            }

            if(bFoundFunc)
                break;
        }
    }

    if(!callFunc || !FunctionMatches(callFunc->Params, paramData, functionDef.ByteCode.Num()))
    {
        AddError(TEXT("Type function '%s' not found for type '%s'"), funcName.Array(), curVar->typeInfo.name);
        return COMPILE_ERROR;
    }

    ProcessFunctionParamters(functionDef, callFunc->Params, paramData);

    for(int i=0; i<callFunc->Params.Num(); i++)
    {
        DefaultVariable *param = &callFunc->Params[i];
        TypeInfo type = param->typeInfo;
        TypeInfo subType = param->subTypeInfo;

        if(type.type == DataType_SubType)
        {
            type = curVar->subTypeInfo;
            if(!type.Compatible(paramData[i].typeReturned))
            {
                AddError(TEXT("Parameter %d specified for list function must be of type '%s'"), i, type.name);
                return COMPILE_ERROR;
            }
        }
        else if(subType.type == DataType_SubType)
        {
            subType = curVar->subTypeInfo;
            if(!subType.Compatible(paramData[i].subType))
            {
                AddError(TEXT("Parameter %d specified for type function must be a list with the sub-type '%s'"), i, subType.name);
                return COMPILE_ERROR;
            }
        }
    }

    CallTypeFunc(functionDef.ByteCode, *callFunc, curVar);

    for(int i=(callFunc->Params.Num()-1); i>=0; i--)
    {
        CodeInfo &match = paramData[i];
        DefaultVariable &param = callFunc->Params[i];

        if(param.flags & VAR_OUT)
        {
            if(!match.OutVarData.Num())
            {
                AddError(TEXT("Parameter %d must be a valid variable for output"), i);
                return COMPILE_ERROR;
            }
            else
            {
                functionDef.ByteCode.AppendList(match.OutVarData);
                APopVar(functionDef.ByteCode, &param, FALSE);
            }
        }
    }

    returnType = callFunc->returnType;

    popData.Insert(0, callFunc->returnType);

    bLValue = FALSE;
    bWasFunction = TRUE;
    curVar = NULL;

    PeekAtAToken(curToken);
    if(curToken[0] == '.')
        return ProcessFuncOrVar(functionDef, curCodeSegment, curVar, bDoingSuper, FALSE, bWasFunction, bLValue, varStartPos, popData, returnType);

    return COMPILE_SUCCESS;
}

CompileErrorType Compiler::ProcessConstructor(FunctionDefinition &functionDef, CodeSegment *curCodeSegment, ClassDefinition *classDef, StructDefinition *structDef, BOOL *lpFoundConstructor)
{
    String curToken;

    CodeInfoList paramData;

    PeekAtAToken(curToken);
    if(curToken[0] == '(')
    {
        HandMeAToken(curToken);

        CompileErrorType errorType = CompileParams(functionDef, curCodeSegment, paramData);
        if(errorType != COMPILE_SUCCESS)
            return errorType;
    }

    return CallConstructor(functionDef, curCodeSegment, classDef, structDef, paramData, lpFoundConstructor);
}

CompileErrorType Compiler::CallConstructor(FunctionDefinition &functionDef, CodeSegment *curCodeSegment, ClassDefinition *classDef, StructDefinition *structDef, CodeInfoList &paramData, BOOL *lpFoundConstructor, BOOL bTop)
{
    if(bTop && lpFoundConstructor)
        *lpFoundConstructor = FALSE;

    CodeInfoList paramDataDup;
    paramDataDup.SetSize(paramData.Num());
    for(int i=0; i<paramData.Num(); i++)
    {
        paramDataDup[i].OutVarData.CopyList(paramData[i].OutVarData);
        paramDataDup[i].inPos = paramData[i].inPos;
        paramDataDup[i].outPos = paramData[i].outPos;
        paramDataDup[i].subType = paramData[i].subType;
        paramDataDup[i].typeReturned = paramData[i].typeReturned;

        if(!paramDataDup[i].OutVarData.Num())
        {
            UINT startPos = paramDataDup[i].inPos;
            UINT size = paramDataDup[i].outPos - startPos;
            paramDataDup[i].OutVarData.AppendArray(functionDef.ByteCode+startPos, size);
        }
    }

    FunctionDefinition *callFunc = GetMatchingFunction(classDef, structDef, NULL, paramDataDup, functionDef.ByteCode.Num(), TRUE, FALSE);

    if(!callFunc)
    {
        if(paramData.Num() && bTop)
        {
            if(callFunc = GetFunction(classDef, structDef, NULL))
                AddError(TEXT("Invalid parameters used or could not find matching constructor"));
            else
                AddError(TEXT("Could not find constructor"));

            return COMPILE_ERROR;
        }
        else
        {
            paramDataDup.FreeData();
            callFunc = GetMatchingFunction(classDef, structDef, NULL, paramDataDup, functionDef.ByteCode.Num(), TRUE, FALSE);
        }
    }

    DWORD lastOffset = functionDef.ByteCode.Num();
    /*if(structDef)
    {
        if(structDef->Parent)
            CallConstructor(functionDef, curCodeSegment, NULL, structDef->Parent, paramData, lpFoundConstructor, FALSE);
    }
    else
    {
        if(scmpi(classDef->classData->name, TEXT("Planet")) == 0)
            nop();
        //if(classDef->Parent)
        //    CallConstructor(functionDef, curCodeSegment, classDef->Parent, NULL, paramData, lpFoundConstructor, FALSE);
    }*/

    BOOL bCodeExecuted = (lastOffset < functionDef.ByteCode.Num());

    if(!callFunc)
        return COMPILE_SUCCESS;

    if(lpFoundConstructor)
        *lpFoundConstructor = TRUE;

    if(bCodeExecuted)
    {
        for(int i=0; i<paramDataDup.Num(); i++)
        {
            paramDataDup[i].inPos = functionDef.ByteCode.Num();
            functionDef.ByteCode.AppendList(paramDataDup[i].OutVarData);
            paramDataDup[i].outPos = functionDef.ByteCode.Num();
        }
    }

    ProcessFunctionParamters(functionDef, callFunc->Params, paramDataDup);

    DoInstruction(functionDef.ByteCode, BCAPushT);

    if(structDef)
        CallStructFunc(functionDef.ByteCode, *callFunc, structDef->name);
    else
        CallClassFunc(functionDef.ByteCode, *callFunc, FALSE);

    return COMPILE_SUCCESS;
}


void Compiler::ProcessFunctionParamters(FunctionDefinition &functionDef, List<DefaultVariable> &funcParams, CodeInfoList &paramData)
{
    for(int i=(paramData.Num()-1); i>=0; i--)
    {
        CodeInfo &match = paramData[i];
        DefaultVariable &param = funcParams[i];

        if(param.flags & VAR_OUT && param.typeInfo.type == DataType_Struct)
        {
            if(param.structDef->NeedsInitialization())  //woa woa, we don't want to send in an initialized out var.
            {                                           //slight hack because we don't know it's an out var when compiling the parameters
                List<BYTE> removeVal;
                PushAData(removeVal, match.typeReturned, FALSE);

                UINT startPos = match.outPos-removeVal.Num();
                functionDef.ByteCode.RemoveRange(startPos, match.outPos);

                removeVal.Clear();
                PushAData(removeVal, match.typeReturned, FALSE, FALSE);

                functionDef.ByteCode.InsertList(startPos, removeVal);
                continue;
            }
        }

        BufferInputSerializer defIn(param.DefaultParamData);

        List<BYTE> newStuff;

        if(match.typeReturned.type == DataType_Null)
        {
            if(param.typeInfo.type == DataType_Object || param.typeInfo.type == DataType_Handle)
                functionDef.ByteCode.Insert(match.outPos, BCPushNullObj);
            else if(param.typeInfo.type == DataType_String)
                functionDef.ByteCode.Insert(match.outPos, BCPushNullStr);
        }

        if((param.typeInfo.type == DataType_Float) && (match.typeReturned.type == DataType_Integer))
        {
            if(match.outPos-match.inPos == 9 && functionDef.ByteCode[match.inPos] == BCPush)
            {
                int iVal = *(int*)(functionDef.ByteCode.Array()+match.inPos+5);
                *(float*)(functionDef.ByteCode.Array()+match.inPos+5) = (float)iVal;
            }
            else
                functionDef.ByteCode.Insert(match.outPos, BCCastFloat);
        }
        else if((param.typeInfo.type == DataType_Integer) && (match.typeReturned.type == DataType_Float))
        {
            if(match.outPos-match.inPos == 9 && functionDef.ByteCode[match.inPos] == BCPush) //if it's a constant value, convert it right on the spot
            {
                float fVal = *(float*)(functionDef.ByteCode.Array()+match.inPos+5);
                *(int*)(functionDef.ByteCode.Array()+match.inPos+5) = (int)fVal;
            }
            else
                functionDef.ByteCode.Insert(match.outPos, BCCastInt);
        }
        else if(match.typeReturned.type == DataType_Void)
        {
            if(param.typeInfo.type == DataType_Integer)
            {
                int defInteger;
                defIn << defInteger;

                PushInt(newStuff, defInteger);
            }
            else if(param.typeInfo.type == DataType_Float)
            {
                float defFloat;
                defIn << defFloat;

                PushFloat(newStuff, defFloat);
            }
            else if(param.typeInfo.type == DataType_Object || param.typeInfo.type == DataType_Handle)
            {
                DoInstruction(newStuff, BCPushNullObj);
            }
            else if(param.typeInfo.type == DataType_String)
            {
                String defString;
                defIn << defString;

                PushString(newStuff, defString);
            }
        }

        if(newStuff.Num())
            functionDef.ByteCode.InsertList(match.outPos, newStuff);
    }
}

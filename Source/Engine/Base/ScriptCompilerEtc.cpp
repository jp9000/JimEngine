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


#define AddErrorExpecting(expecting, got)   AddError(TEXT("Expecting '%s', but got '%s'"), (TSTR)expecting, (TSTR)got)
#define AddErrorEndOfCode()                 AddError(TEXT("Unexpected end of code"))
#define AddErrorUnrecoverable()             AddError(TEXT("Compiler terminated due to unrecoverable error"))
#define AddErrorRedefinition(redef)         AddError(TEXT("Redefinition: '%s'"), (TSTR)redef)
#define AddErrorNoClue(item)                AddError(TEXT("Syntax Error: '%s'"), (TSTR)curToken)


void CodeTokenizer::SetCodeStart(CTSTR lpCodeIn)
{
    dupString = lpCodeIn;
    lpTemp = lpCode = dupString;
}


BOOL CodeTokenizer::GetNextToken(String &token, BOOL bPeek)
{
    TSTR lpStart = lpTemp;

    TSTR lpTokenStart = NULL;
    BOOL bAlphaNumeric = FALSE;

    while(*lpTemp)
    {
        if(mcmp(lpTemp, TEXT("//"), 2*sizeof(TCHAR)))
        {
            lpTemp = schr(lpTemp, '\n');

            if(!lpTemp)
                return FALSE;
        }
        else if(mcmp(lpTemp, TEXT("/*"), 2*sizeof(TCHAR)))
        {
            lpTemp = sstr(lpTemp+2, TEXT("*/"));

            if(!lpTemp)
                return FALSE;

            lpTemp += 2;
        }

        if((*lpTemp == '_') || iswalnum(*lpTemp))
        {
            if(lpTokenStart)
            {
                if(!bAlphaNumeric)
                    break;
            }
            else
            {
                lpTokenStart = lpTemp;
                bAlphaNumeric = TRUE;
            }
        }
        else
        {
            if(lpTokenStart)
            {
                if(bAlphaNumeric)
                    break;

                if(*lpTokenStart == '>' || *lpTokenStart == '<')
                {
                    if((*lpTemp != '=') && (*lpTemp != '>') && (*lpTemp != '<'))
                        break;
                }

                if( ((*lpTokenStart == '=') && (*lpTemp != '=')) ||
                    (*lpTokenStart == ';') ||
                    (*lpTemp == ' ')   ||
                    (*lpTemp == L'　') ||
                    (*lpTemp == '\'')  ||
                    (*lpTemp == '"')   ||
                    (*lpTemp == ';')   ||
                    (*lpTemp == '(')   ||
                    (*lpTemp == ')')   ||
                    (*lpTemp == '[')   ||
                    (*lpTemp == ']')   ||
                    (*lpTemp == '{')   ||
                    (*lpTemp == '}')   ||
                    (*lpTemp == '\r')  ||
                    (*lpTemp == '\t')  ||
                    (*lpTemp == '\n')  )
                {
                    break;
                }
            }
            else
            {
                if(*lpTemp == '"')
                {
                    lpTokenStart = lpTemp;

                    BOOL bFoundEnd = TRUE;
                    while(*++lpTemp != '"')
                    {
                        if(!*lpTemp)
                        {
                            bFoundEnd = FALSE;
                            break;
                        }
                    }

                    if(!bFoundEnd)
                        return FALSE;

                    ++lpTemp;
                    break;
                }
                if(*lpTemp == ';')
                {
                    lpTokenStart = lpTemp;
                    ++lpTemp;
                    break;
                }

                if(*lpTemp == '\'')
                {
                    lpTokenStart = lpTemp;

                    BOOL bFoundEnd = TRUE;
                    while(*++lpTemp != '\'')
                    {
                        if(!*lpTemp)
                        {
                            bFoundEnd = FALSE;
                            break;
                        }
                    }

                    if(!bFoundEnd)
                        return FALSE;

                    ++lpTemp;
                    break;
                }
                else if((*lpTemp == '(') ||
                        (*lpTemp == ')') ||
                        (*lpTemp == '[') ||
                        (*lpTemp == ']') ||
                        (*lpTemp == '{') ||
                        (*lpTemp == '}'))
                {
                    lpTokenStart = lpTemp++;
                    break;
                }

                if( (*lpTemp != ' ')   &&
                    (*lpTemp != L'　') &&
                    (*lpTemp != '\r')  &&
                    (*lpTemp != '\t')  &&
                    (*lpTemp != '\n')  )
                {
                    lpTokenStart = lpTemp;
                    bAlphaNumeric = FALSE;
                }
            }
        }

        ++lpTemp;
    }

    if(!lpTokenStart)
        return FALSE;

    TCHAR oldCH = *lpTemp;
    *lpTemp = 0;

    token = lpTokenStart;

    *lpTemp = oldCH;

    if(bAlphaNumeric && iswdigit(*lpTokenStart)) //handle floating points
    {
        if( (token.Length() > 2) && 
            (lpTokenStart[0] == '0') &&
            (lpTokenStart[1] == 'x')) //convert hex
        {
            unsigned int val = tstring_base_to_uint(lpTokenStart, NULL, 0);
            token = FormattedString(TEXT("%d"), val);
        }
        else
        {
            String nextToken;

            TSTR lpPos = lpTemp;
            if(!GetNextToken(nextToken)) return FALSE;
            if(nextToken[0] == '.')
            {
                lpPos = lpTemp;

                token << nextToken;
                if(!GetNextToken(nextToken)) return FALSE;
                if(iswdigit(nextToken[0]) || nextToken == TEXT("f"))
                    token << nextToken;
                else
                    lpTemp = lpPos;
            }
            else
                lpTemp = lpPos;

            if(token[token.Length()-1] == 'e')
            {
                if(*lpTemp == '-')
                {
                    TSTR lpPos = lpTemp++;

                    if(!GetNextToken(nextToken)) return FALSE;
                    if(!iswdigit(nextToken[0]))
                        lpTemp = lpPos;
                    else
                        token << TEXT("-") << nextToken;
                }
            }

            lpPos = lpTemp;
            if(!GetNextToken(nextToken)) return FALSE;
            if(nextToken[0] == '.')
            {
                lpPos = lpTemp;

                token << nextToken;
                if(!GetNextToken(nextToken)) return FALSE;
                if(iswdigit(nextToken[0]) || nextToken == TEXT("f"))
                    token << nextToken;
                else
                    lpTemp = lpPos;
            }
            else
                lpTemp = lpPos;
        }
    }

    if(bPeek)
        lpTemp = lpStart;

    return TRUE;
}

BOOL CodeTokenizer::GetNextTokenEval(String &token, BOOL *bFloatOccurance, int curPrecedence)
{
    TSTR lpLastSafePos = lpTemp;
    String curVal, curToken;
    BOOL bFoundNumber = FALSE;
    BOOL bUsedBracers = FALSE;

    if(!GetNextToken(curToken)) return FALSE;

    if(curToken == TEXT("("))
    {
        TSTR lpPrevPos = lpTemp;
        int newPrecedence = GetTokenPrecedence(curToken);
        if(!GetNextTokenEval(curToken, bFloatOccurance, newPrecedence)) return FALSE;

        if(!ValidFloatString(curToken))
        {
            lpTemp = lpPrevPos;
            token = TEXT("(");
            return TRUE;
        }

        String nextToken;
        if(!GetNextToken(nextToken)) return FALSE;
        if(nextToken != TEXT(")"))
        {
            lpTemp = lpPrevPos;
            token = TEXT("(");
            return TRUE;
        }

        bUsedBracers = TRUE;
    }

    if(curToken == TEXT("-") && iswdigit(*lpTemp))
    {
        String nextToken;
        if(!GetNextToken(nextToken)) return FALSE;
        curToken << nextToken;
    }

    if(ValidFloatString(curToken))
    {
        bFoundNumber = TRUE;
        curVal = curToken;

        if(!ValidIntString(curVal) && bFloatOccurance)
            *bFloatOccurance = TRUE;
    }
    else
    {
        if(bFoundNumber)
        {
            lpTemp = lpLastSafePos;
            token = curVal;
            return TRUE;
        }
        else
        {
            token = curToken;
            return TRUE;
        }
    }

    lpLastSafePos = lpTemp;

    String operatorToken;
    while(GetNextToken(operatorToken))
    {
        int newPrecedence = GetTokenPrecedence(operatorToken);
        if(newPrecedence <= curPrecedence)
        {
            lpTemp = lpLastSafePos;
            token = curVal;
            return TRUE;
        }

        String nextVal;
        if(!GetNextTokenEval(nextVal, bFloatOccurance, newPrecedence)) return FALSE;

        if(!ValidFloatString(nextVal))
        {
            lpTemp = lpLastSafePos;
            token = curVal;
            return TRUE;
        }

        if(operatorToken == TEXT("<<"))
        {
            int val1 = tstoi(curVal);
            int val2 = tstoi(nextVal);

            val1 <<= val2;
            curVal = IntString(val1);
        }
        else if(operatorToken == TEXT(">>"))
        {
            int val1 = tstoi(curVal);
            int val2 = tstoi(nextVal);

            val1 >>= val2;
            curVal = IntString(val1);
        }
        else if(operatorToken == TEXT("*"))
        {
            float val1 = tstof(curVal);
            float val2 = tstof(nextVal);

            val1 *= val2;
            curVal = FormattedString(TEXT("%g"), val1);
        }
        else if(operatorToken == TEXT("/"))
        {
            float val1 = tstof(curVal);
            float val2 = tstof(nextVal);

            val1 /= val2;
            curVal = FormattedString(TEXT("%g"), val1);
        }
        else if(operatorToken == TEXT("+"))
        {
            float val1 = tstof(curVal);
            float val2 = tstof(nextVal);

            val1 += val2;
            curVal = FormattedString(TEXT("%g"), val1);
        }
        else if(operatorToken == TEXT("-"))
        {
            float val1 = tstof(curVal);
            float val2 = tstof(nextVal);

            val1 -= val2;
            curVal = FormattedString(TEXT("%g"), val1);
        }
        else if(operatorToken == TEXT("|"))
        {
            int val1 = tstoi(curVal);
            int val2 = tstoi(nextVal);

            val1 |= val2;
            curVal = IntString(val1);
        }
        else if(operatorToken == TEXT("&"))
        {
            int val1 = tstoi(curVal);
            int val2 = tstoi(nextVal);

            val1 &= val2;
            curVal = IntString(val1);
        }
        else
        {
            lpTemp = lpLastSafePos;
            token = curVal;
            return TRUE;
        }

        lpLastSafePos = lpTemp;
    }

    return FALSE;
}

int CodeTokenizer::GetTokenPrecedence(CTSTR lpToken)
{
    for(int i=0; i<NUMCLOSINGTOKENS; i++)
    {
        if(!scmp(ClosingTokens[i], lpToken))
            return TokenPrecedence[i];
    }

    return 0;
}

BOOL CodeTokenizer::IsClosingToken(const String &token, CTSTR lpTokenPriority)
{
    DWORD i;

    String strPriority = lpTokenPriority;
    int priorityPrecedence = 0, tokenPrecedence = 14;

    for(i=0; i<NUMCLOSINGTOKENS; i++)
    {
        if(token.Compare(ClosingTokens[i]))
            tokenPrecedence = TokenPrecedence[i];

        if(strPriority.Compare(ClosingTokens[i]))
            priorityPrecedence = TokenPrecedence[i];
    }

    return tokenPrecedence <= priorityPrecedence;
}

BOOL CodeTokenizer::PassBracers(TSTR lpCodePos)
{
    lpTemp = lpCodePos;

    String curToken;

    if(!GetNextToken(curToken))
        return FALSE;
    if(curToken[0] != '{')
        return FALSE;

    while(GetNextToken(curToken, TRUE))
    {
        if(curToken[0] == '}')
        {
            GetNextToken(curToken);
            return TRUE;
        }
        else if(curToken[0] == '{')
        {
            PassBracers(lpTemp);
            continue;
        }
        else if(curToken[0] == '\'')
        {
            PassCharacterThingy(lpTemp);
            continue;
        }
        else if(curToken[0] == '"')
        {
            PassString(lpTemp);
            continue;
        }

        GetNextToken(curToken);
    };

    return FALSE;
}

BOOL CodeTokenizer::PassGreaterEqual(TSTR lpCodePos)
{
    lpTemp = lpCodePos;

    String curToken;

    if(!GetNextToken(curToken))
        return FALSE;
    if(curToken[0] != '<')
        return FALSE;

    while(GetNextToken(curToken, TRUE))
    {
        if(curToken[0] == '>')
        {
            GetNextToken(curToken);
            return TRUE;
        }
        else if(curToken[0] == '<')
        {
            PassGreaterEqual(lpTemp);
            continue;
        }
        else if(curToken[0] == '{')
        {
            PassBracers(lpTemp);
            continue;
        }
        else if(curToken[0] == '\'')
        {
            PassCharacterThingy(lpTemp);
            continue;
        }
        else if(curToken[0] == '"')
        {
            PassString(lpTemp);
            continue;
        }

        GetNextToken(curToken);
    };

    return FALSE;
}

BOOL CodeTokenizer::PassParenthesis(TSTR lpCodePos)
{
    lpTemp = lpCodePos;

    String curToken;

    if(!GetNextToken(curToken))
        return FALSE;
    if(curToken[0] != '(')
        return FALSE;

    while(GetNextToken(curToken, TRUE))
    {
        if(curToken[0] == ')')
        {
            GetNextToken(curToken);
            return TRUE;
        }
        else if(curToken[0] == '(')
        {
            PassParenthesis(lpTemp);
            continue;
        }
        else if(curToken[0] == '{')
        {
            PassBracers(lpTemp);
            continue;
        }
        else if(curToken[0] == '\'')
        {
            PassCharacterThingy(lpTemp);
            continue;
        }
        else if(curToken[0] == '"')
        {
            PassString(lpTemp);
            continue;
        }
        GetNextToken(curToken);
    }

    return FALSE;
}

BOOL CodeTokenizer::PassString(TSTR lpCodePos)
{
    TSTR lpNextLeft = schr(lpCodePos, '"');
    TSTR lpNextRight = lpNextLeft;

    for(;;)
    {
        lpNextRight = schr(lpNextRight+1, '"');
        if(!lpNextRight)
            return FALSE;

        if(lpNextRight[-1] != '\\')
            break;
    }

    lpTemp = lpNextRight+1;

    return TRUE;
}

BOOL CodeTokenizer::PassCharacterThingy(TSTR lpCodePos)
{
    TSTR lpNextLeft = schr(lpCodePos, '\'');
    TSTR lpNextRight = lpNextLeft;

    for(;;)
    {
        lpNextRight = schr(lpNextRight+1, '\'');
        if(!lpNextRight)
            return FALSE;

        if(lpNextRight[-1] != '\\')
            break;
    }

    lpTemp = lpNextRight+1;

    return TRUE;
}

BOOL CodeTokenizer::GotoToken(CTSTR lpTarget, BOOL bPassToken)
{
    String curToken;

    while(GetNextToken(curToken, TRUE))
    {
        if(curToken == lpTarget)
        {
            if(bPassToken)
                GetNextToken(curToken);
            return TRUE;
        }
        else if(curToken[0] == '{')
        {
            PassBracers(lpTemp);
            continue;
        }
        else if(curToken[0] == '(')
        {
            PassParenthesis(lpTemp);
            continue;
        }
        GetNextToken(curToken);
    }

    return FALSE;
}

BOOL CodeTokenizer::GotoClosestToken(CTSTR token1, CTSTR token2, BOOL bPassToken)
{
    CTSTR semicolenPos  = sstr(lpTemp, token1);
    CTSTR openBracerPos = sstr(lpTemp, token2);

    TCHAR jumpToken[2] = {0, 0};

    if(openBracerPos && semicolenPos)
    {
        jumpToken[0] = (openBracerPos < semicolenPos) ? *openBracerPos : *semicolenPos;
    }
    else if(semicolenPos)
        jumpToken[0] = ';';
    else
        jumpToken[0] = '{';

    return GotoToken(jumpToken, bPassToken);
}

BOOL CodeTokenizer::GotoClosingToken(CTSTR lpTokenPriority)
{
    String curToken;

    while(GetNextToken(curToken, TRUE))
    {
        if(IsClosingToken(curToken, lpTokenPriority))
            return TRUE;
        else if(curToken[0] == '{')
        {
            PassBracers(lpTemp);
            continue;
        }
        else if(curToken[0] == '(')
        {
            PassParenthesis(lpTemp);
            continue;
        }
        else if(curToken[0] == '\'')
        {
            PassCharacterThingy(lpTemp-1);
            continue;
        }
        else if(curToken[0] == '"')
        {
            PassString(lpTemp-1);
            continue;
        }

        GetNextToken(curToken);
    }

    return FALSE;
}


void CodeTokenizer::AddError(const TCHAR *format, ...)
{
    if(errorCount == 100)
        return;

    ErrorInfo &error = *errorList.CreateNew();
    error.file = curFile;
    error.line = GetStringLine(lpCode, lpTemp);
    error.stage = curStage;

    String strError;

    if(!curFile.IsEmpty())
        strError << curFile << TEXT(" : ");

    if(lpCode)
        strError << /*compileStage <<*/ FormattedString(TEXT("Error (%d): "), error.line);
    else
        strError << /*compileStage <<*/ TEXT("Error: ");

    ++errorCount;
    va_list arglist;

    va_start(arglist, format);

    strError << FormattedStringva(format, arglist) << TEXT("\r\n");

    if(errorCount == 100)
    {
        if(lpCode)
            strError << FormattedString(TEXT("Error (%d): "),error.line) << TEXT("100 error limit reached\r\n");
        else
            strError << TEXT("Error: ") << TEXT("100 error limit reached\r\n");
    }

    error.error = strError;
}

void CodeTokenizer::AddWarning(const TCHAR *format, ...)
{
    if(warningCount == 100)
        return;

    ErrorInfo &error = *errorList.CreateNew();
    error.file = curFile;
    error.line = GetStringLine(lpCode, lpTemp);
    error.stage = curStage;

    String strError;

    if(!curFile.IsEmpty())
        strError << curFile << TEXT(" : ");

    if(lpCode)
        strError << /*compileStage <<*/ FormattedString(TEXT("Warning (%d): "), error.line);
    else
        strError << /*compileStage <<*/ TEXT("Warning: ");

    ++warningCount;

    va_list arglist;

    va_start(arglist, format);

    strError << FormattedStringva(format, arglist) << TEXT("\r\n");

    if(warningCount == 100)
    {
        if(lpCode)
            strError << FormattedString(TEXT("Warning (%d): "), error.line) << TEXT("100 warning limit reached\r\n");
        else
            strError << TEXT("Warning: ") << TEXT("100 error limit reached\r\n");
    }

    error.error = strError;
}

void CodeTokenizer::CompileErrors()
{
    if(!compilerError) return;

    for(int i=0; i<errorList.Num(); i++)
    {
        for(int j=i+1; j<errorList.Num(); j++)
        {
            if(scmp(errorList[j].file, errorList[i].file) < 0)
            {
                errorList.SwapValues(j, i--);
                break;
            }
            else if(scmp(errorList[j].file, errorList[i].file) == 0)
            {
                if(errorList[j].line < errorList[i].line)
                {
                    errorList.SwapValues(j, i--);
                    break;
                }
                else if(errorList[j].line == errorList[i].line)
                {
                    if(errorList[j].stage < errorList[i].stage)
                    {
                        errorList.SwapValues(j, i--);
                        break;
                    }
                }
            }
        }
    }

    for(int i=0; i<errorList.Num(); i++)
        *compilerError << errorList[i].error;
}

/*void Compiler::AddRandomNumber(String &curToken)
{
    String randNum = FormattedString(TEXT("%lX"), rand());

    DWORD curPos = lpTemp-lpCode;

    dupString.InsertString(curPos, randNum);

    lpCode = dupString;
    lpTemp = lpCode+(curPos+randNum.Length());

    curToken += randNum;
}

void Compiler::AddStartCharacter(String &curToken)
{
    DWORD curPos = lpTemp-lpCode;

    dupString.InsertChar(curPos-curToken.Length(), 'A');

    lpCode = dupString;
    lpTemp = lpCode+(curPos+1);

    curToken = String(TEXT("A")) + curToken;
}*/

BOOL CodeTokenizer::GetActualCharacter(String &stringToken, int &val)
{
    String stringOut = stringToken.Mid(1, stringToken.Length()-1);

    if(stringOut[0] == '\\')
    {
        if(stringOut.Length() > 2)
            return FALSE;

        if(stringOut[1] == TEXT('\\'))
            return TEXT('\\');
        else if(stringOut[1] == TEXT('t'))
            return TEXT('\t');
        else if(stringOut[1] == TEXT('r'))
            return TEXT('\r');
        else if(stringOut[1] == TEXT('n'))
            return TEXT('\n');
        else if(stringOut[1] == TEXT('\''))
            return TEXT('\'');
        else
            return FALSE;
    }
    else if(stringOut.Length() > 1)
        return FALSE;
    else
        val = stringOut[0];

    return TRUE;
}



void Compiler::Pop(List<BYTE> &ByteCode, TypeInfo &ti)
{
    BYTE instruction = BCPop;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << ti.size;
}

void Compiler::PushDup(List<BYTE> &ByteCode, TypeInfo &ti)
{
    if(ti.type == DataType_String)
    {
        BYTE instruction = BCPushDupS;

        BufferOutputSerializer output(ByteCode);
        output << instruction;
    }
    else
    {
        BYTE instruction = BCPushDup;

        BufferOutputSerializer output(ByteCode);
        output << instruction;
        output << ti.size;
    }
}


void Compiler::APushDup(List<BYTE> &ByteCode)
{
    BYTE instruction = BCAPushDup;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
}

void Compiler::APush(List<BYTE> &ByteCode, Variable *var, int arrayIndex)
{
    BYTE instruction = BCAPush;
    int pos;
    if(var->scope == VarScope_Class)
        pos = curClass->GetVariableID(var);
    else
        pos = var->offset;

    arrayIndex *= var->typeInfo.size;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << (int&)var->scope;
    output << pos;
    output << arrayIndex;
}

void Compiler::APushSVar(List<BYTE> &ByteCode, Variable *var, BOOL bTempStack, BOOL bArrayOffset)
{
    BYTE instruction = BCAPushSVar;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << var->offset;
    output << bTempStack;
    output << bArrayOffset;
}

void Compiler::APushCVar(List<BYTE> &ByteCode, ClassDefinition *varClass, Variable *var, BOOL bTempStack, BOOL bArrayOffset)
{
    BYTE instruction = BCAPushCVar;
    int pos = varClass->GetVariableID(var);

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << pos;
    output << bTempStack;
    output << bArrayOffset;
}

void Compiler::APushListItem(List<BYTE> &ByteCode, Variable *var)
{
    BYTE instruction = BCAPushListItem;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << String(var->subTypeInfo.name);
}

void Compiler::PushAData(List<BYTE> &ByteCode, TypeInfo &type, BOOL bNoRemove, BOOL bInitialize)
{
    BufferOutputSerializer output(ByteCode);

    if(type.type == DataType_String)
    {
        BYTE instruction = BCPushADataS;
        output << instruction;
    }
    else if(type.type == DataType_Struct && bInitialize)
    {
        StructDefinition *structDef = Scripting->GetStruct(type.name);
        if(structDef->NeedsInitialization())
        {
            BYTE instruction = BCPushADataInit;
            output << instruction;
            output << structDef->name;
            output.OutputDword(0);
        }
        else
        {
            BYTE instruction = BCPushAData;
            output << instruction;
            output << type.size;
        }
    }
    else
    {
        BYTE instruction = BCPushAData;
        output << instruction;
        output << type.size;
    }

    output << bNoRemove;
}

void Compiler::APopVar(List<BYTE> &ByteCode, Variable *var, BOOL bInitialize)
{
    BYTE instruction;
    if(var->typeInfo.type == DataType_String)
        instruction = BCAPopVarS;
    else if(bInitialize)
    {
        if(var->typeInfo.type == DataType_List)
            instruction = BCAPopVarInit;
        else if(var->typeInfo.type == DataType_Struct)
        {
            StructDefinition *structDef = Scripting->GetStruct(var->typeInfo.name);
            instruction = structDef->NeedsInitialization() ? BCAPopVarInit : BCAPopVar;
        }
        else
            instruction = BCAPopVar;
    }
    else
        instruction = BCAPopVar;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    if(instruction == BCAPopVar)
        output << var->typeInfo.size;
    else if(instruction == BCAPopVarInit)
    {
        output << String(var->typeInfo.name);
        if(var->typeInfo.type == DataType_List)
            output << String(var->subTypeInfo.name);
        else
            output << String();
    }
}


void Compiler::TPop(List<BYTE> &ByteCode, int size)
{
    BYTE instruction = BCTPop;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << size;
}

void Compiler::TPopFree(List<BYTE> &ByteCode, TypeInfo &type, TypeInfo *subType)
{
    BYTE instruction = BCTPop;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << String(type.name);
    if(subType)
        output << String(subType->name);
    else
        output.OutputDword(0);
}

void Compiler::TPopS(List<BYTE> &ByteCode)
{
    BYTE instruction = BCTPopS;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
}

void Compiler::TPopTypes(List<BYTE> &ByteCode, List<TypeInfo> &popData)
{
    int curPopSize=0;
    for(int i=0; i<popData.Num(); i++)
    {
        TypeInfo &type = popData[i];
        if(type.type == DataType_Struct)
        {
            if(type.type == DataType_Struct)
            {
                StructDefinition *structDef = Scripting->GetStruct(type.name);
                if(!structDef->NeedsInitialization())
                    curPopSize += type.size;
                else
                {
                    if(curPopSize)
                        TPop(ByteCode, curPopSize);
                    curPopSize = 0;

                    TPopFree(ByteCode, structDef->GetType());
                }
            }
        }
        else if(type.type == DataType_String)
        {
            if(curPopSize)
                TPop(ByteCode, curPopSize);
            curPopSize = 0;

            TPopS(ByteCode);
        }
        else
            curPopSize += type.size;
    }

    if(curPopSize)
        TPop(ByteCode, curPopSize);
}

void Compiler::PushTData(List<BYTE> &ByteCode, TypeInfo &type)
{
    BYTE instruction = (type.type == DataType_String) ? BCPushTDataS : BCPushTData;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    if(type.type != DataType_String)
        output << type.size;
}


void Compiler::TPushStack(List<BYTE> &ByteCode, unsigned int size)
{
    BYTE instruction = BCTPushStack;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << size;
}

void Compiler::TPushZero(List<BYTE> &ByteCode, unsigned int size)
{
    BYTE instruction = BCTPushZero;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << size;
}

void Compiler::PushInt(List<BYTE> &ByteCode, unsigned int intVal)
{
    BYTE instruction = BCPush;
    int size = sizeof(unsigned int);

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << size;
    output << intVal;
}

void Compiler::PushFloat(List<BYTE> &ByteCode, float floatVal)
{
    BYTE instruction = BCPush;
    int size = sizeof(float);

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << size;
    output << floatVal;
}

void Compiler::PushType(List<BYTE> &ByteCode, TypeDataInfo &tri)
{
    BYTE instruction = BCPush;
    int size = sizeof(TypeDataInfo);

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << size;
    output.Serialize(&tri, sizeof(tri));
}

void Compiler::PushString(List<BYTE> &ByteCode, String &stringVal)
{
    BYTE instruction = BCPushS;

    BufferOutputSerializer output(ByteCode);

    if(stringVal.IsEmpty())
    {
        instruction = BCPushNullStr;
        output << instruction;
    }
    else
    {
        output << instruction;
        output << stringVal;
    }
}


void Compiler::CallFunc(List<BYTE> &ByteCode, FunctionDefinition &targetFunctionDef)
{
    BYTE instruction = 0;
    instruction = BCCallFunc;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << targetFunctionDef.funcOffset;
}

void Compiler::CallStructFunc(List<BYTE> &ByteCode, FunctionDefinition &targetFunctionDef, CTSTR structName)
{
    BYTE instruction = 0;
    instruction = BCCallStructFunc;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << String(structName);
    output << targetFunctionDef.funcOffset;
}

void Compiler::CallStructOp(List<BYTE> &ByteCode, FunctionDefinition &targetFunctionDef, CTSTR structName)
{
    BYTE instruction = 0;
    instruction = BCCallStructOp;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << String(structName);
    output << targetFunctionDef.funcOffset;
}

void Compiler::CallTypeFunc(List<BYTE> &ByteCode, TypeFunction &targetFunctionDef, Variable *var)
{
    BYTE instruction = BCCallTypeFunc;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << (int&)var->typeInfo.type;
    output << targetFunctionDef.funcOffset;
    output << String(var->subTypeInfo.name);
}

void Compiler::CallClassStatic(List<BYTE> &ByteCode, FunctionDefinition &targetFunctionDef, ClassDefinition *classDef)
{
    BYTE instruction = 0;
    instruction = BCCallClassStFn;

    int funcID;
    for(funcID=0; funcID<classDef->Functions.Num(); funcID++)
    {
        if(&classDef->Functions[funcID] == &targetFunctionDef)
            break;
    }

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << String(classDef->classData->name);
    output << funcID;
}

void Compiler::CallClassFunc(List<BYTE> &ByteCode, FunctionDefinition &targetFunctionDef, BOOL bSuper)
{
    BYTE instruction = 0;
    instruction = BCCallClassFunc;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << targetFunctionDef.funcOffset;
    output << bSuper;
}


void Compiler::CreateObj(List<BYTE> &ByteCode, String &className)
{
    BYTE instruction = BCCreateObj;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << className;
}


void Compiler::DoReturn(FunctionDefinition &functionDef)
{
    VariableType type = functionDef.returnType.type;

    if(type == DataType_String)
        DoInstruction(functionDef.ByteCode, BCReturnS);
    else
    {
        BYTE instruction = BCReturn;

        BufferOutputSerializer output(functionDef.ByteCode);
        output << instruction;
        output << functionDef.returnType.size;
    }
}


void Compiler::Jump(List<BYTE> &ByteCode, DWORD offset)
{
    BYTE instruction = BCJump;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << offset;
}

void Compiler::JumpUndefined(List<BYTE> &ByteCode, DWORD &pos)
{
    BYTE instruction = BCJump;
    DWORD blank = 0;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    pos = ByteCode.Num();
    output << blank;
}

void Compiler::JumpIf(List<BYTE> &ByteCode, DWORD &pos)
{
    BYTE instruction = BCJumpIf;
    DWORD blank = 0;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    pos = ByteCode.Num();
    output << blank;
}

void Compiler::JumpIfNot(List<BYTE> &ByteCode, DWORD &pos)
{
    BYTE instruction = BCJumpIfNot;
    DWORD blank = 0;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    pos = ByteCode.Num();
    output << blank;
}

void Compiler::JumpOffsetIf(List<BYTE> &ByteCode, DWORD offset)
{
    BYTE instruction = BCJumpIf;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << offset;
}

void Compiler::JumpOffsetIfNot(List<BYTE> &ByteCode, DWORD offset)
{
    BYTE instruction = BCJumpIfNot;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << offset;
}


void Compiler::DynamicCast(List<BYTE> &ByteCode, String &strClass)
{
    BYTE instruction = BCDynamicCast;

    BufferOutputSerializer output(ByteCode);
    output << instruction;
    output << strClass;
}


BOOL Compiler::DoOperator(List<BYTE> &ByteCode, String &opNameIn, TypeInfo &srcType, TypeInfo &targetType, BOOL &bCalledFunction)
{
    if((opNameIn == TEXT("=")) && (srcType == targetType))
    {
        bCalledFunction = FALSE;
        return TRUE;
    }

    String opName;
    TokenType opType = GetTokenType(NULL, NULL, opNameIn);

    if((opType == TokenType_AssignmentOperator) && (opNameIn.Length() > 1))
        opName = opNameIn.Left(opNameIn.Length()-1);
    else
        opName = opNameIn;

    do
    {
        BYTE instruction;

        if((srcType.type <= DataType_Float) && (targetType.type <= DataType_Float))
        {
            if(opName[0] == '+')
                instruction = (srcType.type == DataType_Float) ? BCAddF : BCAdd;
            else if(opName[0] == '-')
                instruction = (srcType.type == DataType_Float) ? BCSubtractF : BCSubtract;
            else if(opName[0] == '*')
                instruction = (srcType.type == DataType_Float) ? BCMultiplyF : BCMultiply;
            else if(opName[0] == '/')
                instruction = (srcType.type == DataType_Float) ? BCDivideF : BCDivide;
            else if(opName[0] == '%')
                instruction = (srcType.type == DataType_Float) ? BCModF : BCMod;
            else if(opName.Compare(TEXT("==")))
                instruction = (srcType.type == DataType_Float) ? BCEqualF : BCEqual;
            else if(opName.Compare(TEXT("!=")))
                instruction = (srcType.type == DataType_Float) ? BCNotEqualF : BCNotEqual;
            else if(opName.Compare(TEXT(">=")))
                instruction = (srcType.type == DataType_Float) ? BCGreaterEqualF : BCGreaterEqual;
            else if(opName.Compare(TEXT(">")))
                instruction = (srcType.type == DataType_Float) ? BCGreaterF : BCGreater;
            else if(opName.Compare(TEXT("<=")))
                instruction = (srcType.type == DataType_Float) ? BCLessEqualF : BCLessEqual;
            else if(opName.Compare(TEXT("<")))
                instruction = (srcType.type == DataType_Float) ? BCLessF : BCLess;
            else if((srcType.type == DataType_Integer) && (targetType.type == DataType_Integer))
            {
                if(opName.Compare(TEXT("<<")))
                    instruction = BCShiftLeft;
                else if(opName.Compare(TEXT(">>")))
                    instruction = BCShiftRight;
                else if(opName.Compare(TEXT("&")))
                    instruction = BCBitwiseAnd;
                else if(opName.Compare(TEXT("|")))
                    instruction = BCBitwiseOr;
                else
                    break;
            }
            else if(opName.Compare(TEXT("=")))
            {
                if((srcType.type == DataType_Integer) && (targetType.type == DataType_Float))
                    instruction = BCCastInt;
                else if((srcType.type == DataType_Float) && (targetType.type == DataType_Integer))
                    instruction = BCCastFloat;
                else
                    break; //actually getting here is impossible
            }
            else
                break;
        }
        else if((srcType.type == DataType_Object) && (targetType.type == DataType_Object))
        {
            if(opName == TEXT("=="))
                instruction = BCEqualO;
            else if(opName == TEXT("!="))
                instruction = BCNotEqualO;
            else
                break;
        }
        else if((srcType.type == DataType_Handle) && (targetType.type == DataType_Handle))
        {
            if(opName == TEXT("=="))
                instruction = BCEqualO;
            else if(opName == TEXT("!="))
                instruction = BCNotEqualO;
            else
                break;
        }
        else if((srcType.type == DataType_String) && (targetType.type == DataType_String))
        {
            if(opName[0] == '+')
                instruction = BCAddS;
            else if(opName.Compare(TEXT("==")))
                instruction = BCEqualS;
            else if(opName.Compare(TEXT("!=")))
                instruction = BCNotEqualS;
            else
                break;
        }
        else
            break;

        DoInstruction(ByteCode, instruction);
        if((opType == TokenType_ConditionalOperator) || (opName == TEXT("!")))
            GetTypeInfo(TEXT("int"), srcType);
        bCalledFunction = FALSE;
        return TRUE;
    }while(false);

    FunctionDefinition *operatorFunc = Scripting->GetOperator(opName, srcType, &targetType);
    if(!operatorFunc)
    {
        if(srcType.type == DataType_Struct)
        {
            StructDefinition *structDef = Scripting->GetStruct(srcType.name);

            while(operatorFunc = structDef->GetNextFunction(opName, operatorFunc, TRUE))
            {
                if(operatorFunc->Params.Num() == 1 && operatorFunc->Params[0].typeInfo == targetType)
                    break;
            }

            if(operatorFunc)
            {
                CallStructOp(ByteCode, *operatorFunc, srcType.name);
                bCalledFunction = TRUE;
                return TRUE;
            }
        }
        return FALSE;
    }

    CallFunc(ByteCode, *operatorFunc);
    bCalledFunction = TRUE;
    srcType = operatorFunc->returnType;
    return TRUE;
}

void Compiler::DoInstruction(List<BYTE> &ByteCode, BYTE instruction)
{
    BufferOutputSerializer output(ByteCode);
    output << instruction;
}


TokenType Compiler::GetTokenType(ClassDefinition *classDef, StructDefinition *structDef, const String &token, BOOL bPostfix)
{
    DWORD i;

    if(iswalpha(token[0]))
    {
        for(i=0; i<NUMKEYWORDS; i++)
        {
            if(token.Compare(KeywordNames[i]))
                return TokenType_Keyword;
        }

        for(i=0; i<NUMTYPES; i++)
        {
            if(token.Compare(TypeNames[i]))
                return TokenType_Type;
        }

        if(classDef)
        {
            if(classDef->GetFunction(token))
                return TokenType_Function;

            if(classDef->GetVariable(token))
                return TokenType_Variable;
        }
        else if(structDef)
        {
            if(structDef->GetFunction(token))
                return TokenType_Function;

            if(structDef->GetVariable(token))
                return TokenType_Variable;
        }

        for(int moduleID=0; moduleID<Scripting->Modules.Num(); moduleID++)
        {
            ModuleScriptData &module = Scripting->Modules[moduleID];

            for(i=0; i<module.GlobalEnumList.Num(); i++)
            {
                EnumDefinition &enumDef = module.GlobalEnumList[i];

                if(enumDef.name == token)
                    return TokenType_EnumDef;

                for(int j=0; j<enumDef.Items.Num(); j++)
                {
                    if(enumDef.Items[j].name == token)
                        return TokenType_Enum;
                }
            }
        }

        if(Scripting->GetGlobalFunction(token))
            return TokenType_Function;

        if(Scripting->GetGlobalVariable(token))
            return TokenType_Variable;

        if(Scripting->GetStruct(token))
            return TokenType_Struct;

        if(FindClass(token))
            return TokenType_Class;
    }
    else
    {
        if((token[0] == '"') && (token[token.Length()-1] == '"'))
            return TokenType_String;
    
        if((token[0] == '\'') && (token[token.Length()-1] == '\''))
            return TokenType_Character;
    
       if(ValidFloatString(token))
            return TokenType_Number;

        for(i=0; i<NUMAOPERATORS; i++)
        {
            if(token.Compare(AOperatorNames[i]))
                return TokenType_AssignmentOperator;
        }

        if(bPostfix)
        {
            for(i=0; i<NUMPOSTFIXOPERATORS; i++)
            {
                if(token.Compare(PostfixOperatorNames[i]))
                    return TokenType_PostfixOperator;
            }
        }
        else
        {
            for(i=0; i<NUMPREFIXOPERATORS; i++)
            {
                if(token.Compare(PrefixOperatorNames[i]))
                    return TokenType_PrefixOperator;
            }
        }

        for(i=0; i<NUMMOPERATORS; i++)
        {
            if(token.Compare(MOperatorNames[i]))
                return TokenType_ModifyOperator;
        }

        for(i=0; i<NUMCOPERATORS; i++)
        {
            if(token.Compare(COperatorNames[i]))
                return TokenType_ConditionalOperator;
        }
    }

    return TokenType_Unknown;
}


VariableType Compiler::GetVariableType(ClassDefinition *classDef, StructDefinition *structDef, CTSTR variableType)
{
    TokenType tokenType = GetTokenType(classDef, structDef, variableType);

    if(tokenType == TokenType_Type)
    {
        for(int i=0; i<NUMTYPES; i++)
        {
            if(scmp(variableType, TypeNames[i]) == 0)
            {
                i = (i < 3) ? 0 : (i-3);
                return (VariableType)i;
            }
        }
    }
    else if(tokenType == TokenType_Class)
        return DataType_Object;
    else if(tokenType == TokenType_Struct)
        return DataType_Struct;

    return DataType_Void;
}

BOOL Compiler::GetTypeInfo(CTSTR lpType, TypeInfo& ti)
{
    if(scmp(lpType, TEXT("list")) == 0)
    {
        ti.type  = DataType_List;
        ti.size  = sizeof(List<BYTE>);
        ti.align = sizeof(LPVOID);
        ti.name  = TEXT("list");
        return TRUE;
    }
    else if(scmp(lpType, TEXT("int")) == 0)
    {
        ti.type  = DataType_Integer;
        ti.size  = sizeof(int);
        ti.align = sizeof(int);
        ti.name  = TEXT("int");
        return TRUE;
    }
    else if(scmp(lpType, TEXT("void")) == 0)
    {
        ti.type  = DataType_Void;
        ti.size  = 0;
        ti.align = 0;
        ti.name  = TEXT("void");
        return TRUE;
    }
    else if(scmp(lpType, TEXT("bool")) == 0)
    {
        ti.type  = DataType_Integer;
        ti.size  = sizeof(int);
        ti.align = sizeof(int);
        ti.name  = TEXT("bool");
        return TRUE;
    }
    else if(scmp(lpType, TEXT("char")) == 0)
    {
        ti.type  = DataType_Integer;
        ti.size  = sizeof(int);
        ti.align = sizeof(int);
        ti.name  = TEXT("char");
        return TRUE;
    }
    else if(scmp(lpType, TEXT("icolor")) == 0)
    {
        ti.type  = DataType_Integer;
        ti.size  = sizeof(int);
        ti.align = sizeof(int);
        ti.name  = TEXT("icolor");
        return TRUE;
    }
    else if(scmp(lpType, TEXT("float")) == 0)
    {
        ti.type  = DataType_Float;
        ti.size  = sizeof(float);
        ti.align = sizeof(float);
        ti.name  = TEXT("float");
        return TRUE;
    }
    else if(scmp(lpType, TEXT("string")) == 0)
    {
        ti.type  = DataType_String;
        ti.size  = sizeof(String);
        ti.align = sizeof(LPVOID);
        ti.name  = TEXT("string");
        return TRUE;
    }
    else if(scmp(lpType, TEXT("handle")) == 0)
    {
        ti.type  = DataType_Handle;
        ti.size  = sizeof(HANDLE);
        ti.align = sizeof(HANDLE);
        ti.name  = TEXT("handle");
        return TRUE;
    }
    else if(scmp(lpType, TEXT("type")) == 0)
    {
        ti.type  = DataType_Type;
        ti.size  = sizeof(TypeDataInfo);
        ti.align = sizeof(int);
        ti.name  = TEXT("type");
        return TRUE;
    }

    EnumDefinition *enumDef = Scripting->GetEnumDef(lpType);
    if(enumDef)
    {
        ti.type  = DataType_Integer;
        ti.size  = sizeof(int);
        ti.align = sizeof(int);
        ti.name  = enumDef->name;
        return TRUE;
    }

    StructDefinition *structDef = Scripting->GetStruct(lpType);
    if(structDef)
    {
        ti.size  = structDef->size;
        ti.align = structDef->align;
        ti.type  = DataType_Struct;
        ti.name  = structDef->name;
        return TRUE;
    }

    Class* testClass;
    if(testClass = FindClass(lpType))
    {
        if(testClass->scriptClass)
        {
            ti.type  = DataType_Object;
            ti.size  = sizeof(Object*);
            ti.align = sizeof(Object*);
            ti.name  = testClass->name;
            return TRUE;
        }
    }

    ti.type  = DataType_Void;
    ti.size  = 0;
    ti.align = 0;
    ti.name  = TEXT("void");

    return FALSE;
}

BOOL Compiler::NameDefined(ClassDefinition *classDef, StructDefinition *structDef, CTSTR lpName, BOOL bFunction, CTSTR lpModule)
{
    if(!bFunction)
    {
        if(classDef && classDef->GetFunction(lpName))
            return TRUE;
        else if(structDef && structDef->GetFunction(lpName))
            return TRUE;
    }

    for(int moduleID=0; moduleID<Scripting->Modules.Num(); moduleID++)
    {
        ModuleScriptData &module = Scripting->Modules[moduleID];

        if(lpModule && !module.strModule.CompareI(lpModule))
            continue;

        for(int i=0; i<module.GlobalEnumList.Num(); i++)
        {
            EnumDefinition &enumDef = module.GlobalEnumList[i];

            if(enumDef.name == lpName)
                return TRUE;

            for(int j=0; j<enumDef.Items.Num(); j++)
            {
                if(enumDef.Items[j].name == lpName)
                    return TRUE;
            }
        }
    }

    if(Scripting->GetStruct(lpName, lpModule))
        return TRUE;

    if(!bFunction && Scripting->GetGlobalFunction(lpName, lpModule))
        return TRUE;

    if(Scripting->GetGlobalVariable(lpName, lpModule))
        return TRUE;

    Class *testClass = FindClass(lpName);
    if(testClass)
        return TRUE;

    return FALSE;
}

int Compiler::GetEnumVal(CTSTR lpName)
{
    for(int moduleID=0; moduleID<Scripting->Modules.Num(); moduleID++)
    {
        ModuleScriptData &module = Scripting->Modules[moduleID];

        for(int i=0; i<module.GlobalEnumList.Num(); i++)
        {
            EnumDefinition &enumDef = module.GlobalEnumList[i];

            for(int j=0; j<enumDef.Items.Num(); j++)
            {
                if(enumDef.Items[j].name == lpName)
                    return enumDef.Items[j].val;
            }
        }
    }

    return -1;
}

FunctionDefinition *Compiler::GetFunction(ClassDefinition *classDef, StructDefinition *structDef, CTSTR lpName, BOOL bTopLevelOnly)
{
    FunctionDefinition *funcDef = NULL;

    if(classDef)
        funcDef = classDef->GetFunction(lpName, bTopLevelOnly);
    else if(structDef)
        funcDef = structDef->GetFunction(lpName, bTopLevelOnly);

    if(!funcDef)
        funcDef = Scripting->GetGlobalFunction(lpName);

    return funcDef;
}

FunctionDefinition *Compiler::GetNextFunction(ClassDefinition *classDef, StructDefinition *structDef, CTSTR lpName, FunctionDefinition *curFunc, BOOL bTopLevelOnly)
{
    FunctionDefinition *funcDef = NULL;

    if(classDef)
        funcDef = classDef->GetNextFunction(lpName, curFunc, bTopLevelOnly);
    else if(structDef)
        funcDef = structDef->GetNextFunction(lpName, curFunc, bTopLevelOnly);

    if(!funcDef)
        funcDef = Scripting->GetNextGlobalFunction(lpName, curFunc);

    return funcDef;
}

FunctionDefinition *Compiler::GetMatchingFunction(ClassDefinition *classDef, StructDefinition *structDef, CTSTR lpName, CodeInfoList &paramMatchList, DWORD curCodePos, BOOL bLocalOnly, BOOL bSuperFuncs)
{
    FunctionDefinition *funcDef = NULL;

    if(classDef)
    {
        while(funcDef = classDef->GetNextFunction(lpName, funcDef, !bSuperFuncs))
        {
            if(FunctionMatches(funcDef->Params, paramMatchList, curCodePos))
                return funcDef;
        }
    }
    else if(structDef)
    {
        while(funcDef = structDef->GetNextFunction(lpName, funcDef, !bSuperFuncs))
        {
            if(FunctionMatches(funcDef->Params, paramMatchList, curCodePos))
                return funcDef;
        }
    }

    if(!bLocalOnly)
    {
        funcDef = NULL;

        while(funcDef = Scripting->GetNextGlobalFunction(lpName, funcDef))
        {
            if(FunctionMatches(funcDef->Params, paramMatchList, curCodePos))
                return funcDef;
        }
    }

    return NULL;
}

BOOL Compiler::FunctionMatches(List<DefaultVariable> &funcParams, CodeInfoList &paramMatchList, DWORD curCodePos)
{
    if(paramMatchList.Num() > funcParams.Num())
        return FALSE;

    int i;
    for(i=0; i<paramMatchList.Num(); i++)
    {
        DefaultVariable &param = funcParams[i];
        CodeInfo &match = paramMatchList[i];

        if((match.typeReturned.type == DataType_Void) && (param.flags & VAR_DEFAULT)) //if is default
            continue;

        if(match.typeReturned.type == DataType_Null)
        {
            if(param.typeInfo.type == DataType_Object || param.typeInfo.type == DataType_String)
                continue;
        }

        if((param.typeInfo.type <= DataType_Float) && (match.typeReturned.type <= DataType_Float)) //floats/ints can be converted
            continue;

        if(param.typeInfo.type == DataType_SubType)
            continue;

        if(param.typeInfo.type != match.typeReturned.type)
            return FALSE;

        if(param.typeInfo.type < DataType_Void) //if is not an class/struct
            continue;

        if(param.typeInfo.type == DataType_Object)
        {
            if(scmp(match.typeReturned.name, TEXT("Object")) == 0) //"Object" is impossible, so this would mean it's a null value
                continue;

            Class *classInfoParam = FindClass(param.typeInfo.name);
            Class *classInfoMatch = FindClass(match.typeReturned.name);

            if(!classInfoMatch->_IsOf(classInfoParam))
                return FALSE;
            else
                continue;
        }
        else if(param.typeInfo.name != match.typeReturned.name)
            return FALSE;

        if(param.typeInfo.type == DataType_List)
        {
            if(param.subTypeInfo.type != DataType_SubType && param.subTypeInfo.name != match.subType.name)
                return FALSE;
        }
    }

    if(i < funcParams.Num())
    {
        int num = funcParams.Num()-i;

        for(; i<funcParams.Num(); i++)
        {
            if(!(funcParams[i].flags & VAR_DEFAULT))
                return FALSE;
        }

        while(num--)
        {
            UINT lastPos = !paramMatchList.Num() ? curCodePos : paramMatchList.Last().outPos;

            CodeInfo &ci = *paramMatchList.CreateNew();
            ci.typeReturned.type = DataType_Void;
            ci.outPos = lastPos;
        }
    }

    return TRUE;
}

Variable *Compiler::GetVariable(ClassDefinition *classDef, StructDefinition *structDef, CTSTR lpName)
{
    Variable *var = NULL;

    if(classDef)
        var = classDef->GetVariable(lpName);
    else if(structDef)
        var = structDef->GetVariable(lpName);

    if(!var)
        var = Scripting->GetGlobalVariable(lpName);

    return var;
}

BOOL Compiler::ValidOperator(CTSTR lpOperator)
{
    for(int i=0; i<VALIDOPS; i++)
    {
        if(scmp(lpOperator, ValidOperators[i]) == 0)
            return TRUE;
    }

    return FALSE;
}

BOOL Compiler::ValidPrefixOperator(CTSTR lpOperator)
{
    for(int i=0; i<VALIDPREFIXOPS; i++)
    {
        if(scmp(lpOperator, ValidPrefixOperators[i]) == 0)
            return TRUE;
    }

    return FALSE;
}


//==================================================================
//  StructDefinition
//==================================================================

Variable *StructDefinition::GetVariable(const String &variableName)
{
    DWORD i;

    for(i=0; i<Variables.Num(); i++)
    {
        if(variableName.Compare(Variables[i].name))
            return &Variables[i];
    }

    if(Parent)
        return Parent->GetVariable(variableName);

    return NULL;
}

Variable *StructDefinition::GetVariableFromPos(DWORD varPosition)
{
    DWORD i;

    for(i=0; i<Variables.Num(); i++)
    {
        Variable *var = &Variables[i];
        if(var->offset == varPosition)
            return var;
    }

    if(Parent)
        return Parent->GetVariableFromPos(varPosition);

    return NULL;
}


//==================================================================
//  ClassDefinition
//==================================================================

void ClassDefinition::SetFunctionClassData(FunctionDefinition &funcDef)
{
    List<ClassDefinition*> Heirarchy;

    ClassDefinition *classDef = this;
    do
    {
        Heirarchy.Insert(0, classDef);
    }while(classDef = classDef->Parent);

    int totalOffset = 0;
    BOOL bFoundOriginal = FALSE;
    for(int i=0; i<Heirarchy.Num(); i++)
    {
        classDef = Heirarchy[i];
        for(int j=0; j<classDef->Functions.Num(); j++)
        {
            FunctionDefinition &curFunc = classDef->Functions[j];

            if(funcDef.name == curFunc.name)
            {
                if(funcDef.FunctionsMatch(curFunc))
                {
                    if(Parent && !functionStartIndex && (i == Heirarchy.Num()-1))
                        functionStartIndex = totalOffset;

                    funcDef.funcOffset = totalOffset;
                    bFoundOriginal = TRUE;
                    break;
                }
                else
                    ++totalOffset;
            }
            else if(!classDef->FindSuper(curFunc))
                ++totalOffset;
        }

        if(bFoundOriginal)
            break;
    }

    if(!bFoundOriginal)
        funcDef.funcOffset = totalOffset;
}

FunctionDefinition* ClassDefinition::FindSuper(FunctionDefinition &funcDef)
{
    if(!Parent)
        return NULL;

    ClassDefinition *classDef = Parent;
    FunctionDefinition *curFunc = NULL;

    while(curFunc = classDef->GetNextFunction(funcDef.name, curFunc))
    {
        if(funcDef.FunctionsMatch(*curFunc))
            return curFunc;
    }

    return NULL;
}

PropertyVariable *ClassDefinition::GetVariable(CTSTR lpVariableName)
{
    PropertyVariable *var = NULL;

    for(int i=0; i<Variables.Num(); i++)
    {
        if(Variables[i].name.Compare(lpVariableName))
            return &Variables[i];
    }

    return (Parent) ? Parent->GetVariable(lpVariableName) : NULL;
}


//==================================================================
//  FunctionDefinition
//==================================================================

BOOL FunctionDefinition::FunctionsMatch(FunctionDefinition &funcDef)
{
    if(Params.Num() != funcDef.Params.Num())
        return FALSE;

    if(returnType != funcDef.returnType)
        return FALSE;

    for(int i=0; i<Params.Num(); i++)
    {
        if(Params[i].typeInfo != funcDef.Params[i].typeInfo)
            return FALSE;
    }

    return TRUE;
}



//==================================================================
//  CodeSegment
//==================================================================

BOOL CodeSegment::NameDefined(const String &token)
{
    TokenType type = curCompiler->GetTokenType(curCompiler->curClass, curCompiler->curStruct, token);

    if(type != TokenType_Unknown)
        return TRUE;

    DWORD i;

    if(iswalpha(token[0]))
    {
        for(i=0; i<functionParent->Params.Num(); i++)
        {
            if(token.Compare(functionParent->Params[i].name))
                return TRUE;
        }

        CodeSegment *curSeg = this;

        do
        {
            for(i=0; i<curSeg->LocalVariableIDs.Num(); i++)
            {
                if(token.Compare(curSeg->functionParent->LocalVars[curSeg->LocalVariableIDs[i]].name))
                    return TRUE;
            }
        }while(curSeg = curSeg->Parent);
    }

    return FALSE;
}


TokenType CodeSegment::GetTokenType(const String &token, BOOL bPostfix)
{       
    DWORD i;

    if(iswalpha(token[0]))
    {
        for(i=0; i<functionParent->Params.Num(); i++)
        {
            if(token.Compare(functionParent->Params[i].name))
                return TokenType_Variable;
        }

        CodeSegment *curSeg = this;

        while(curSeg)
        {
            for(i=0; i<curSeg->LocalVariableIDs.Num(); i++)
            {
                if(token.Compare(curSeg->functionParent->LocalVars[curSeg->LocalVariableIDs[i]].name))
                    return TokenType_Variable;
            }

            curSeg = curSeg->Parent;
        }
    }

    return curCompiler->GetTokenType(curCompiler->curClass, curCompiler->curStruct, token, bPostfix);
}

Variable *CodeSegment::GetVariable(const String &variableName)
{
    Variable *var = NULL;

    int i;

    CodeSegment *curSeg = this;
    while(curSeg)
    {
        for(i=0; i<curSeg->LocalVariableIDs.Num(); i++)
        {
            if(variableName.Compare(curSeg->functionParent->LocalVars[curSeg->LocalVariableIDs[i]].name))
                return &curSeg->functionParent->LocalVars[curSeg->LocalVariableIDs[i]];
        }

        curSeg = curSeg->Parent;
    }

    for(i=0; i<functionParent->Params.Num(); i++)
    {
        if(variableName.Compare(functionParent->Params[i].name))
            return &functionParent->Params[i];
    }

    if(curCompiler->curClass)
    {
        var = curCompiler->curClass->GetVariable(variableName);
        if(var)
            return var;
    }
    else if(curCompiler->curStruct)
    {
        var = curCompiler->curStruct->GetVariable(variableName);
        if(var)
            return var;
    }

    return Scripting->GetGlobalVariable(variableName);
}

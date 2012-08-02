/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EffectProcessor.cpp

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
#include "EffectProcessor.h"

#define AddErrorExpecting(expecting, got)   AddError(TEXT("Expecting '%s', but got '%s'"), (TSTR)expecting, (TSTR)got)
#define AddErrorEndOfCode()                 AddError(TEXT("Unexpected end of code"))
#define AddErrorUnrecoverable()             AddError(TEXT("Compiler terminated due to unrecoverable error"))
#define AddErrorNoClue(item)                AddError(TEXT("Unexpected: '%s'"), (TSTR)curToken)

#define PeekAtAToken(str) if(!GetNextToken(str, TRUE)) {AddErrorEndOfCode(); return FALSE;}
#define HandMeAToken(str) if(!GetNextToken(str)) {AddErrorEndOfCode(); return FALSE;}

#define ExpectToken(expecting, gototoken) {if(!GetNextToken(curToken)) {AddErrorEndOfCode(); return FALSE;} if(curToken != expecting) {AddErrorExpecting(expecting, curToken); if(!GotoToken(gototoken, TRUE)) {AddErrorEndOfCode(); return FALSE;} continue;}}
#define ExpectTokenIgnore(expecting) {if(!GetNextToken(curToken)) {AddErrorEndOfCode(); return FALSE;} if(curToken != expecting) {AddErrorExpecting(expecting, curToken); continue;}}


BOOL EffectProcessor::ProcessEffect(Effect *effectIn, CTSTR lpFileName, CTSTR lpEffectString, String &errorString, BOOL bRecurse)
{
    TSTR lpOldTemp;
    String oldFileName;

    if(bRecurse)
    {
        lpOldTemp = lpTemp;
        lpTemp = (TSTR)lpEffectString;
        oldFileName = curFile;
    }
    else
    {
        effect = effectIn;

        compilerError = &errorString;
        SetCodeStart(lpEffectString);
    }

    curFile = lpFileName;

    String curToken;

    while(GetNextToken(curToken))
    {
        if(curToken == TEXT("#"))
        {
            CTSTR lpEndPos = schr(lpTemp, '\n');

            if(!lpEndPos)
                continue;

            DWORD endPos = (DWORD)(UPARAM)((lpEndPos+1)-lpCode);

            HandMeAToken(curToken);
            if(curToken == TEXT("include"))
            {
                HandMeAToken(curToken);
                if(curToken[0] != '"' || curToken.Right(1)[0] != '"')
                {
                    AddErrorExpecting(TEXT("String"), curToken);
                    GotoToken(TEXT("\n"), TRUE);
                    continue;
                }

                //---------------------------

                curToken = curToken.Mid(1, curToken.Length()-1);

                XFile incFile;
                if(!incFile.Open(effect->strEffectDir + TEXT("/") + curToken, XFILE_READ, XFILE_OPENEXISTING))
                {
                    AddError(TEXT("Couldn't open include file '%s'"), incFile);
                    GotoToken(TEXT("\n"), TRUE);
                    continue;
                }

                //---------------------------

                String strInclude;
                incFile.ReadFileToString(strInclude);
                ProcessEffect(effect, GetPathFileName(curToken, TRUE), strInclude, errorString, TRUE);
                incFile.Close();
            }
        }
        else if(scmpi(curToken, TEXT("struct")) == 0)
        {
            TSTR lpStructStart = lpTemp-curToken.Length();

            EPStruct *curStruct = Structs.CreateNew();

            HandMeAToken(curStruct->name);

            ExpectToken(TEXT("{"), TEXT(";"));

            do
            {
                String strType, strName, strMapping;

                HandMeAToken(strType);
                if(strType == TEXT(";")) continue;
                if(strType == TEXT("}")) break;

                HandMeAToken(strName);
                if(strName == TEXT(";")) {AddErrorNoClue(strName); continue;}
                if(strName == TEXT("}")) {AddErrorNoClue(strName); break;}

                HandMeAToken(curToken);
                if(curToken == TEXT(":"))
                {
                    HandMeAToken(strMapping);
                    if(strMapping == TEXT(";")) {AddErrorNoClue(strMapping); continue;}
                    if(strMapping == TEXT("}")) {AddErrorNoClue(strMapping); break;}
                }

                HandMeAToken(curToken);
                if(curToken != TEXT(";"))
                    AddErrorExpecting(TEXT(";"), curToken);

                EPVar *curVar = curStruct->Vars.CreateNew();
                curVar->name = strName;
                curVar->type = strType;
                curVar->mapping = strMapping;

                if(curToken == TEXT("}")) {AddErrorNoClue(curToken); break;}
            }while(true);

            ExpectTokenIgnore(TEXT(";"));

            TCHAR lastChar = *lpTemp;
            *lpTemp = 0;
            curStruct->contents = lpStructStart;
            *lpTemp = lastChar;
        }
        else if(scmpi(curToken, TEXT("technique")) == 0)
        {
            EPTechnique curTech;

            HandMeAToken(curTech.name);

            ExpectTokenIgnore(TEXT("{"));

            PeekAtAToken(curToken);

            while(curToken != TEXT("}"))
            {
                HandMeAToken(curToken);

                if(curToken != TEXT("pass"))
                {
                    AddErrorExpecting(TEXT("pass"), curToken);
                    break;
                }

                EPPass *curPass = curTech.Passes.CreateNew();

                HandMeAToken(curToken);
                if(curToken != TEXT("{"))
                {
                    curPass->name = curToken;
                    HandMeAToken(curToken);
                }

                PeekAtAToken(curToken);

                while(curToken != TEXT("}"))
                {
                    String *strCall;

                    HandMeAToken(curToken);

                    if((curToken == TEXT("VertexShader")) || (curToken == TEXT("VertexProgram")))
                        strCall = &curPass->vertexProgram;
                    else if((curToken == TEXT("GeometryShader")) || (curToken == TEXT("GeometryProgram")))
                    {
                        GotoToken(TEXT(";"), TRUE);
                        PeekAtAToken(curToken);
                        continue;
                    }
                    else if((curToken == TEXT("PixelShader")) || (curToken == TEXT("FragmentProgram")))
                        strCall = &curPass->fragmentProgram;
                    else
                        {AddErrorNoClue(curToken); GotoToken(TEXT(";"), TRUE); PeekAtAToken(curToken); continue;}

                    HandMeAToken(curToken);
                    if(curToken[0] != '=') {AddErrorExpecting(TEXT("="), curToken); GotoToken(TEXT(";"), TRUE); PeekAtAToken(curToken); continue;}

                    PeekAtAToken(curToken);
                    if(curToken.CompareI(TEXT("compile")))
                    {
                        AddError(TEXT("compile specifier not necessary."));

                        HandMeAToken(curToken);
                        HandMeAToken(curToken);
                    }

                    //--------------------------

                    TSTR curPos = lpTemp;
                    TSTR endPos = schr(lpTemp, ';');

                    if(!endPos) {AddErrorEndOfCode(); return FALSE;}

                    TCHAR lastChar = *(++endPos);
                    *endPos = 0;
                    *strCall = curPos;
                    *endPos = lastChar;

                    lpTemp = endPos+1;

                    PeekAtAToken(curToken);
                }

                HandMeAToken(curToken);

                PeekAtAToken(curToken);
            }

            ExpectToken(TEXT("}"), TEXT("}"));

            EPTechnique *actualTech = Techniques.CreateNew();
            mcpy(actualTech, &curTech, sizeof(EPTechnique));
            zero(&curTech, sizeof(EPTechnique));
        }
        else if(scmpi(curToken, TEXT("samplerstate")) == 0)
        {
            TSTR lpSamplerStart = lpTemp-curToken.Length();

            EPSampler curSampler;

            HandMeAToken(curSampler.name);

            ExpectToken(TEXT("{"), TEXT(";"));

            PeekAtAToken(curToken);

            while(curToken != TEXT("}"))
            {
                String strState;
                HandMeAToken(strState);

                ExpectToken(TEXT("="), TEXT(";"));

                String strValue;
                HandMeAToken(strValue);

                ExpectToken(TEXT(";"), TEXT(";"));

                *curSampler.states.CreateNew() = strState;
                *curSampler.values.CreateNew() = strValue;

                PeekAtAToken(curToken);
            }

            ExpectToken(TEXT("}"), TEXT("}"));
            ExpectTokenIgnore(TEXT(";"));

            TCHAR lastChar = *lpTemp;
            *lpTemp = 0;
            curSampler.contents = lpSamplerStart;
            *lpTemp = lastChar;

            //--------------------

            EPSampler *actualSampler = Samplers.CreateNew();
            mcpy(actualSampler, &curSampler, sizeof(EPSampler));
            zero(&curSampler, sizeof(EPSampler));
        }
        else if(curToken == TEXT("{"))
        {
            if(!PassBracers(lpTemp-1)) {AddErrorEndOfCode(); return FALSE;}
        }
        else if(curToken == TEXT(";"))
        {
        }
        else //params, samplers, functions
        {
            BOOL bProperty, bConst;
            if(curToken == TEXT("property"))
            {
                bProperty = TRUE;
                HandMeAToken(curToken);
            }
            else
                bProperty = FALSE;

            if(curToken == TEXT("const"))
            {
                bConst = TRUE;
                HandMeAToken(curToken);
            }
            else
                bConst = FALSE;
                
            if(curToken == TEXT("uniform"))
                HandMeAToken(curToken);

            String strType = curToken;

            String strName;
            HandMeAToken(strName);

            PeekAtAToken(curToken);

            if(curToken[0] == '(') //function
            {
                EPFunc curFunc;

                curFunc.name = strName;
                curFunc.retType = strType;

                if(!ProcessFuncParams(curFunc))
                    return FALSE;

                ExpectTokenIgnore(TEXT("{"));

                TSTR lpFuncStart = lpTemp-curToken.Length();

                int numBraces = 1;

                BOOL bFailed = FALSE;
                BOOL bProcessedReturn = FALSE;

                while(numBraces != 0)
                {
                    if(!bProcessedReturn)
                    {
                        curToken = strType;
                        bProcessedReturn = TRUE;
                    }
                    else
                        HandMeAToken(curToken);

                    if(curToken[0] == '{')
                        ++numBraces;
                    else if(curToken[0] == '}')
                        --numBraces;
                    else if(FindStruct(curToken))
                        curFunc.structDependancies.SafeAdd(curToken);
                    else if(FindFunc(curToken))
                        curFunc.funcDependancies.SafeAdd(curToken);
                    else if(FindSampler(curToken))
                        curFunc.samplerDependancies.SafeAdd(curToken);
                    else if(FindParam(curToken))
                        curFunc.paramDependancies.SafeAdd(curToken);
                }

                if(!bFailed)
                {
                    TCHAR lastChar = *lpTemp;
                    *lpTemp = 0;
                    curFunc.contents = lpFuncStart;
                    *lpTemp = lastChar;

                    EPFunc *actualFunc = Functions.CreateNew();
                    mcpy(actualFunc, &curFunc, sizeof(EPFunc));
                    zero(&curFunc, sizeof(EPFunc));
                }
            }
            else //param
            {
                EPParam curParam;
                zero(&curParam, sizeof(EPParam));
                curParam.name = strName;
                curParam.type = strType;
                curParam.bConst = bConst;
                curParam.bProperty = bProperty;

                if(curToken[0] == '[')
                {
                    HandMeAToken(curToken);

                    HandMeAToken(curToken);
                    curParam.arrayCount = tstoi(curToken);

                    ExpectToken(TEXT("]"), TEXT(";"));

                    PeekAtAToken(curToken);
                }

                if(curToken == TEXT("="))
                {
                    HandMeAToken(curToken);

                    BufferOutputSerializer sOut(curParam.DefaultValue);

                    if(scmpi_n(strType, TEXT("texture"), 7) == 0)
                    {
                        curParam.bTexture = TRUE;
                        HandMeAToken(curToken);

                        if((curToken[0] != '"') || (curToken.Right(1)[0] != '"'))
                        {
                            AddErrorExpecting(TEXT("string"), TEXT("unknown"));
                            GotoToken(TEXT(";"), TRUE);
                            continue;
                        }

                        sOut << GetActualString(curToken);
                    }
                    else if(scmp(strType, TEXT("float")) == 0)
                    {
                        HandMeAToken(curToken);

                        if(!ValidFloatString(curToken))
                            AddError(TEXT("Invalid floating point value: '%s'"), curToken.Array());

                        float fValue = tstof(curToken);

                        sOut << fValue;
                    }
                    else if(scmp(strType, TEXT("int")) == 0)
                    {
                        HandMeAToken(curToken);

                        if(!ValidIntString(curToken))
                            AddError(TEXT("Invalid integer value: '%s'"), curToken.Array());

                        int iValue = tstoi(curToken);

                        sOut << iValue;
                    }
                    else if(scmp_n(strType, TEXT("float"), 5) == 0)
                    {
                        CTSTR lpFloatType = strType.Array()+5;
                        int floatCount = 0;

                        if(lpFloatType[0] == '1') floatCount = 1.0f;
                        else if(lpFloatType[0] == '2') floatCount = 2.0f;
                        else if(lpFloatType[0] == '3') floatCount = 3.0f;
                        else if(lpFloatType[0] == '4') floatCount = 4.0f;
                        else
                            AddError(TEXT("Invalid row count '%c'"), lpFloatType[0]);

                        if(lpFloatType[1] == 'x')
                        {
                            if(lpFloatType[2] != '1')
                            {
                                if(lpFloatType[2] == '2') floatCount *= 2.0f;
                                else if(lpFloatType[2] == '3') floatCount *= 3.0f;
                                else if(lpFloatType[2] == '4') floatCount *= 4.0f;
                                else
                                    AddError(TEXT("Invalid column count '%c'"), lpFloatType[0]);
                            }
                        }

                        if(floatCount > 1)
                            {ExpectToken(TEXT("{"), TEXT(";"));}

                        int j;
                        for(j=0; j<floatCount; j++)
                        {
                            if(j)
                            {
                                HandMeAToken(curToken);
                                if(curToken[0] != ',')
                                {
                                    AddErrorExpecting(TEXT(","), curToken);
                                    break;
                                }
                            }

                            HandMeAToken(curToken);

                            if(!ValidFloatString(curToken))
                            {
                                AddError(TEXT("Invalid floating point value: %s"), curToken.Array());
                                break;
                            }

                            float fValue = tstof(curToken);
                            sOut << fValue;
                        }

                        if(j != floatCount) //processing error occured
                        {
                            GotoToken(TEXT(";"));
                            continue;
                        }

                        if(floatCount > 1)
                            {ExpectToken(TEXT("}"), TEXT(";"));}
                    }

                    PeekAtAToken(curToken);
                }

                if(bProperty)
                {
                    while(curToken[0] == '<')
                    {
                        HandMeAToken(curToken);

                        TSTR lpStart = lpTemp;
                        if(!GotoToken(TEXT(">"), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                        TSTR lpEnd = lpTemp-1;

                        TCHAR lastChar = *lpEnd;
                        *lpEnd = 0;
                        *curParam.Properties.CreateNew() = lpStart;
                        *lpEnd = lastChar;

                        PeekAtAToken(curToken);
                    }
                }

                ExpectTokenIgnore(TEXT(";"));

                EPParam *actualParam = Params.CreateNew();
                mcpy(actualParam, &curParam, sizeof(EPParam));
                zero(&curParam, sizeof(EPParam));
            }
        }
    }

    if(bRecurse)
    {
        lpTemp = lpOldTemp;
        curFile = oldFileName;
    }
    else
    {
        if(!errorCount)
            CompileEffect();
    }

    return !errorCount;
}

BOOL EffectProcessor::ProcessFuncParams(EPFunc &curFunc)
{
    String curToken;

    HandMeAToken(curToken);

    PeekAtAToken(curToken);
    while(curToken[0] != ')')
    {
        EPVar &var = *curFunc.Params.CreateNew();

        HandMeAToken(curToken);
        if(curToken == TEXT("uniform"))
        {
            var.bUniform = TRUE;
            HandMeAToken(curToken);
        }

        var.type = curToken;

        HandMeAToken(var.name);
        HandMeAToken(curToken);

        if(FindStruct(var.type))
            curFunc.structDependancies.SafeAdd(var.type);
        else if(FindSampler(var.type))
            curFunc.samplerDependancies.SafeAdd(var.type);

        if(curToken[0] == ':')
        {
            HandMeAToken(var.mapping);
            HandMeAToken(curToken);
        }

        if((curToken[0] != ',') && (curToken[0] != ')'))
        {
            AddErrorExpecting(TEXT(",' or ')"), curToken);
            GotoToken(TEXT(")"), TRUE);
            break;
        }
    }

    PeekAtAToken(curToken);

    if(curToken[0] == ':') //function is mapped to something, like COLOR
    {
        HandMeAToken(curToken);
        HandMeAToken(curFunc.mapping);
    }

    return TRUE;
}

void EffectProcessor::CompileEffect()
{
    int i;

    effect->Params.SetSize(Params.Num());
    effect->Techniques.SetSize(Techniques.Num());

    for(i=0; i<Params.Num(); i++)
    {
        EffectParam *param = effect->Params+i;
        EPParam &buildParam = Params[i];

        param->name = buildParam.name;
        param->dataType = Effect_Param;
        param->DefaultValue.CopyList(buildParam.DefaultValue);

        if(buildParam.type == TEXT("bool"))
            param->type = Parameter_Bool;
        else if(buildParam.type == TEXT("float"))
            param->type = Parameter_Float;
        else if(buildParam.type == TEXT("int"))
            param->type = Parameter_Int;
        else if(buildParam.type == TEXT("float2"))
            param->type = Parameter_Vector2;
        else if(buildParam.type == TEXT("float3"))
            param->type = Parameter_Vector;
        else if(buildParam.type == TEXT("float4"))
            param->type = Parameter_Vector4;
        else if(buildParam.type == TEXT("float3x3"))
            param->type = Parameter_Matrix3x3;
        else if(buildParam.type == TEXT("float4x4"))
            param->type = Parameter_Matrix;
        else if(scmp_n(buildParam.type, TEXT("texture"), 7) == 0)
            param->type = Parameter_Texture;

        if(buildParam.bProperty)
        {
            if(param->type == Parameter_Bool)
                param->propertyType = EffectProperty_Bool;
            else if(param->type == Parameter_Float)
            {
                param->propertyType = EffectProperty_Float;
                param->fMin = -10000.0f;
                param->fMax = 10000.0f;
                param->fInc = 0.1f;
                param->fMul = 1.0f;
            }
            else if(param->type == Parameter_Vector)
                param->propertyType = EffectProperty_Color;
            else if(param->type == Parameter_Texture)
                param->propertyType = EffectProperty_Texture;
        }
        else
            param->propertyType = EffectProperty_None;

        buildParam.param = param;

        for(int i=0; i<buildParam.Properties.Num(); i++)
            param->AddPropertyString(buildParam.Properties[i]);
    }

    for(i=0; i<Techniques.Num(); i++)
    {
        EffectTechnique *tech = effect->Techniques+i;
        EPTechnique &buildTech = Techniques[i];

        tech->name = buildTech.name;
        tech->Passes.SetSize(buildTech.Passes.Num());
        tech->dataType = Effect_Technique;
        tech->bValid = TRUE;

        for(int j=0; j<tech->Passes.Num(); j++)
        {
            EffectPass *pass = tech->Passes+j;
            EPPass &buildPass = buildTech.Passes[j];
            int k;

            buildPass.pass = pass;

            curPass = pass;
            curPass->name = buildPass.name;
            curPass->dataType = Effect_Pass;

            //-----------------------------

            StringList UsedParams;

            String strShader;
            MakeShaderString(buildPass.vertexProgram, strShader, UsedParams);

            effect->strProcessingInfo.Clear();
            effect->strProcessingInfo << TEXT("Vertex shader error for pass ") << IntString(j) << TEXT(" in technique ") << tech->name << TEXT(" of effect '") << effect->effectPath << TEXT("'");

            curPass->vertShader = (Shader*)effect->system->CreateVertexShader(strShader);
            if(curPass->vertShader)
            {
                curPass->VertParams.SetSize(UsedParams.Num());
                for(k=0; k<UsedParams.Num(); k++)
                {
                    String &strParam = UsedParams[k];

                    curPass->VertParams[k].param = (EffectParam*)effect->GetParameterByName(strParam);
                    curPass->VertParams[k].handle = curPass->vertShader->GetParameterByName(strParam);
                }
            }
            else
                tech->bValid = FALSE;

            //-----------------------------

            UsedParams.Clear(); strShader.Clear();
            MakeShaderString(buildPass.fragmentProgram, strShader, UsedParams);

            effect->strProcessingInfo.Clear();
            effect->strProcessingInfo << TEXT("Pixel shader error for pass ") << IntString(j) << TEXT(" in technique ") << tech->name << TEXT(" of effect '") << effect->effectPath << TEXT("'");

            curPass->pixelShader = effect->system->CreatePixelShader(strShader);

            if(curPass->pixelShader)
            {
                curPass->PixelParams.SetSize(UsedParams.Num());
                for(k=0; k<UsedParams.Num(); k++)
                {
                    String &strParam = UsedParams[k];

                    curPass->PixelParams[k].param = (EffectParam*)effect->GetParameterByName(strParam);
                    curPass->PixelParams[k].handle = curPass->pixelShader->GetParameterByName(strParam);
                }
            }
            else
                tech->bValid = FALSE;
        }
    }

    effect->strProcessingInfo.Clear();
}

void EffectProcessor::AddDependancies(EPFunc *func, String &strOutput, StringList &UsedParams)
{
    int i;

    for(i=0; i<func->paramDependancies.Num(); i++)
    {
        EPParam *param = FindParam(func->paramDependancies[i]);
        if(param->bWritten) continue;

        param->WriteVar(strOutput, UsedParams);
    }

    if(func->paramDependancies.Num())
        strOutput << TEXT("\r\n\r\n");

    for(i=0; i<func->samplerDependancies.Num(); i++)
    {
        EPSampler *sampler = FindSampler(func->samplerDependancies[i]);
        if(sampler->bWritten)
            continue;

        strOutput << sampler->contents << TEXT("\r\n\r\n");
        sampler->bWritten = TRUE;
    }

    for(i=0; i<func->funcDependancies.Num(); i++)
    {
        EPFunc *newFunc = FindFunc(func->funcDependancies[i]);
        if(newFunc->bWritten)
            continue;

        AddDependancies(newFunc, strOutput, UsedParams);
        strOutput << TEXT("\r\n");
        newFunc->bWritten = TRUE;
    }

    for(i=0; i<func->structDependancies.Num(); i++)
    {
        EPStruct *structVal = FindStruct(func->structDependancies[i]);
        if(structVal->bWritten)
            continue;

        strOutput << structVal->contents << TEXT("\r\n\r\n");
        structVal->bWritten = TRUE;
    }

    strOutput << func->retType << TEXT(" ") << func->name << TEXT("(");

    for(i=0; i<func->Params.Num(); i++)
    {
        if(i) strOutput << TEXT(", ");

        EPVar &var = func->Params[i];
        if(var.bUniform) strOutput << TEXT("uniform ");
        strOutput << var.type << TEXT(" ") << var.name;
    }
    
    strOutput << TEXT(")\r\n") << func->contents << TEXT("\r\n");
}

void EffectProcessor::MakeShaderString(String &shaderCall, String &shaderOut, StringList &UsedParams)
{
    CodeTokenizer ct;
    ct.lpCode = ct.lpTemp = shaderCall;

    String strFuncName, curToken;
    ct.GetNextToken(strFuncName);

    while(ct.GetNextToken(curToken))
    {
        EPParam *param = FindParam(curToken);
        if(param)
            param->WriteVar(shaderOut, UsedParams);
    }

    if(strFuncName.CompareI(TEXT("NULL")))
        return;

    EPFunc *func = FindFunc(strFuncName);
    if(!func)
        return;
    AddDependancies(func, shaderOut, UsedParams);

    shaderOut << TEXT("\r\n");
    shaderOut << func->retType << TEXT(" main(");

    String callAddition;

    BOOL bHasNormalParams = FALSE;

    for(int i=0; i<func->Params.Num(); i++)
    {
        EPVar &var = func->Params[i];
        EPStruct *epStruct = FindStruct(var.type);
        if(var.mapping.IsValid() || (epStruct && epStruct->IsMapped()))
        {
            shaderOut << var.type << TEXT(" ") << var.name;
            if(!epStruct) shaderOut << TEXT(" : ") << var.mapping;

            if(callAddition.IsValid()) callAddition << TEXT(", ");
            callAddition << var.name;
        }
        else
            bHasNormalParams = TRUE;
    }

    if(bHasNormalParams)
        callAddition << TEXT(", ");

    String adjustedCall = shaderCall;
    if(callAddition.IsValid())
    {
        TSTR lpStart = schr(adjustedCall, '(');
        if(!lpStart)
        {
            AddError(TEXT("Invalid function call '%s'"), adjustedCall);
            return;
        }

        ++lpStart;

        DWORD dwPos = (DWORD)(UPARAM)(lpStart-adjustedCall.Array());

        adjustedCall.InsertString(dwPos, callAddition);
    }

    shaderOut << TEXT(")");

    if(func->mapping.IsValid())
        shaderOut << TEXT(" : ") << func->mapping;
    shaderOut << TEXT("\r\n{\r\n    ");
    shaderOut << TEXT("return ");
    shaderOut << adjustedCall << TEXT("\r\n}\r\n");

    ResetEffectData();
}


void EffectParam::AddPropertyString(String &propString)
{
    CodeTokenizer ct;
    ct.lpCode = ct.lpTemp = propString;

    String curToken;
    ct.GetNextToken(curToken);

    if(curToken == TEXT("name"))
    {
        ct.GetNextToken(curToken);
        ct.GetNextToken(curToken);
        fullName = ct.GetActualString(curToken);
    }
    else if(curToken == TEXT("type"))
    {
        do
        {
            ct.GetNextToken(curToken);
            if(curToken[0] != '=') break;

            ct.GetNextToken(curToken);
            if(curToken != TEXT("scroller")) break;

            ct.GetNextToken(curToken);
            if(curToken[0] != '(') break;

            //-----------------

            ct.GetNextToken(curToken);
            fMin = tstof(curToken);

            ct.GetNextToken(curToken);
            if(curToken[0] != ',') break;

            //-----------------

            ct.GetNextToken(curToken);
            fMax = tstof(curToken);

            ct.GetNextToken(curToken);
            if(curToken[0] != ',') break;

            //-----------------

            ct.GetNextToken(curToken);
            fInc = tstof(curToken);

            ct.GetNextToken(curToken);
            if(curToken[0] != ',') break;

            //-----------------

            ct.GetNextToken(curToken);
            fMul = tstof(curToken);

            ct.GetNextToken(curToken);
            if(curToken[0] != ')') break;
        }while(false);
    }
}

void EffectProcessor::ResetEffectData()
{
    int i;

    for(i=0; i<Params.Num(); i++)
        Params[i].bWritten = FALSE;
    for(i=0; i<Structs.Num(); i++)
        Structs[i].bWritten = FALSE;
    for(i=0; i<Functions.Num(); i++)
        Functions[i].bWritten = FALSE;
    for(i=0; i<Samplers.Num(); i++)
        Samplers[i].bWritten = FALSE;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  D3DShaderProcessor.cpp

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


#include "D3DSystem.h"

#define AddErrorExpecting(expecting, got)   AddError(TEXT("Expecting '%s', but got '%s'"), (TSTR)expecting, (TSTR)got)
#define AddErrorEndOfCode()                 AddError(TEXT("Unexpected end of code"))
#define AddErrorUnrecoverable()             AddError(TEXT("Compiler terminated due to unrecoverable error"))
#define AddErrorNoClue(item)                AddError(TEXT("Unexpected: '%s'"), (TSTR)curToken)

#define PeekAtAToken(str) if(!GetNextToken(str, TRUE)) {AddErrorEndOfCode(); return FALSE;}
#define HandMeAToken(str) if(!GetNextToken(str)) {AddErrorEndOfCode(); return FALSE;}

#define EscapeLikeTheWind(gototoken) {if(!GotoToken(gototoken, TRUE)) {AddErrorEndOfCode(); return FALSE;} continue;}

#define ExpectToken(expecting, gototoken) {if(!GetNextToken(curToken)) {AddErrorEndOfCode(); return FALSE;} if(curToken != expecting) {AddErrorExpecting(expecting, curToken); if(!GotoToken(gototoken, TRUE)) {AddErrorEndOfCode(); return FALSE;} continue;}}
#define ExpectTokenIgnore(expecting) {if(!GetNextToken(curToken)) {AddErrorEndOfCode(); return FALSE;} if(curToken != expecting) {AddErrorExpecting(expecting, curToken); continue;}}


BOOL ShaderProcessor::ProcessShader(CTSTR input, String &output, String &errorString)
{
    compilerError = &errorString;

    String curToken;

    SetCodeStart(input);

    output.Clear();

    TSTR lpLastPos = lpTemp;

    DWORD curInsideCount = 0;
    BOOL  bNewCodeLine = TRUE;

    while(GetNextToken(curToken))
    {
        TSTR lpCurPos = lpTemp-curToken.Length();

        if(curToken.CompareI(TEXT("samplerstate")))
        {
            ShaderSampler &curSampler = *Samplers.CreateNew();
            HandMeAToken(curSampler.name);

            SamplerInfo info;

            ExpectToken(TEXT("{"), TEXT(";"));

            PeekAtAToken(curToken);

            while(curToken != TEXT("}"))
            {
                String strState;
                HandMeAToken(strState);

                ExpectToken(TEXT("="), TEXT(";"));

                String strValue;
                HandMeAToken(strValue);

                if(!AddState(info, strState, strValue))
                    EscapeLikeTheWind(TEXT(";"));

                ExpectToken(TEXT(";"), TEXT(";"));

                PeekAtAToken(curToken);
            }

            curSampler.sampler = CreateSamplerState(info);

            ExpectToken(TEXT("}"), TEXT("}"));
            ExpectTokenIgnore(TEXT(";"));

            //----------------------------------------

            lpLastPos = lpTemp;
            continue;
        }
        else if(curToken[0] == '{')
            ++curInsideCount;
        else if(curToken[0] == '}')
            --curInsideCount;
        else if(curToken[0] == '(')
            ++curInsideCount;
        else if(curToken[0] == ')')
            --curInsideCount;
        else if(curInsideCount) //inside a function or whatever.
        {
            ShaderTex* texInfo = FindTexture(curToken); //Tex

            TSTR lpSavedPos = lpTemp;

            String testToken;
            PeekAtAToken(testToken);
            if(testToken[0] == '.') //Tex.
            {
                HandMeAToken(testToken);

                String sampleType;
                HandMeAToken(sampleType);  //Tex.Sample[Type]

                if(scmp_n(sampleType, TEXT("Sample"), 6) == 0)
                {
                    CopyStringRange(output, lpLastPos, lpCurPos);

                    if(!texInfo)
                    {
                        AddError(TEXT("Trying to sample invalid texture object '%s'"), curToken);
                        continue;
                    }

                    ExpectTokenIgnore(TEXT("("));  //Tex.Sample[Type](

                    String strSampler;
                    HandMeAToken(strSampler);  //Tex.Sample[Type](SamplerState
                    Params[texInfo->paramID].samplerID = GetSamplerID(strSampler);

                    ExpectTokenIgnore(TEXT(","));  //Tex.Sample[Type](SamplerState, 

                    String strTexOutput;
                    DWORD dwPos;

                    if(sampleType == TEXT("Sample"))
                    {
                        strTexOutput << TEXT("tex") << texInfo->type << TEXT("(") << texInfo->name << TEXT(",");

                        TSTR lpParamStart = lpTemp;
                        if(!GotoToken(TEXT(")"), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                        CopyStringRange(strTexOutput, lpParamStart, lpTemp);  //Tex.SampleBias(SamplerState, [texcoordstuff]) 

                        dwPos = lpTemp-lpCode;
                    }
                    else if(sampleType == TEXT("SampleBias"))
                    {
                        strTexOutput << TEXT("tex") << texInfo->type << TEXT("bias(") << texInfo->name << TEXT(",");

                        TSTR lpParamStart = lpTemp;
                        if(!GotoToken(TEXT(","), TRUE))  {AddErrorEndOfCode(); return FALSE;}
                        CopyStringRange(strTexOutput, lpParamStart, lpTemp);  //Tex.SampleBias(SamplerState, [texcoordstuff], 

                        lpParamStart = lpTemp;
                        if(!GotoToken(TEXT(")"), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                        CopyStringRange(strTexOutput, lpParamStart, lpTemp);  //Tex.SampleBias(SamplerState, [texcoordstuff], [biasstuff])

                        dwPos = lpTemp-lpCode;
                    }
                    else if(sampleType == TEXT("SampleLevel"))
                    {
                        strTexOutput << TEXT("tex") << texInfo->type << TEXT("lod(") << texInfo->name << TEXT(",");

                        TSTR lpParamStart = lpTemp;
                        if(!GotoToken(TEXT(","), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                        CopyStringRange(strTexOutput, lpParamStart, lpTemp);  //Tex.SampleLevel(SamplerState, [texcoordstuff], 

                        lpParamStart = lpTemp;
                        if(!GotoToken(TEXT(")"), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                        CopyStringRange(strTexOutput, lpParamStart, lpTemp);  //Tex.SampleLevel(SamplerState, [texcoordstuff], [lodstuff])

                        dwPos = lpTemp-lpCode;
                    }
                    else if(sampleType == TEXT("SampleGrad"))
                    {
                        strTexOutput << TEXT("tex") << texInfo->type << TEXT("grad(") << texInfo->name << TEXT(",");

                        TSTR lpParamStart = lpTemp;
                        if(!GotoToken(TEXT(","), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                        CopyStringRange(strTexOutput, lpParamStart, lpTemp);  //Tex.SampleGrad(SamplerState, [texcoordstuff], 

                        lpParamStart = lpTemp;
                        if(!GotoToken(TEXT(","), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                        CopyStringRange(strTexOutput, lpParamStart, lpTemp);  //Tex.SampleGrad(SamplerState, [texcoordstuff], [gradstuff1],

                        lpParamStart = lpTemp;
                        if(!GotoToken(TEXT(")"), TRUE)) {AddErrorEndOfCode(); return FALSE;}
                        CopyStringRange(strTexOutput, lpParamStart, lpTemp);  //Tex.SampleGrad(SamplerState, [texcoordstuff], [gradstuff1], [gradstuff2])

                        dwPos = lpTemp-lpCode;
                    }
                    else
                    {
                        AddError(TEXT("Invalid sampler function '%s'"), sampleType.Array());
                        return FALSE;
                    }

                    DWORD lastPos = lpLastPos-lpCode;

                    dupString.InsertString(dwPos, strTexOutput);
                    lpCode = dupString;
                    lpTemp = lpLastPos = lpCode+dwPos;
                    continue;
                }

                lpTemp = lpSavedPos;
            }
        }
        else if(bNewCodeLine) //not inside any code, so this is some sort of declaration (function/struct/var)
        {
            if( (curToken != TEXT("const"))     &&
                (curToken != TEXT("struct"))    &&
                (curToken != TEXT("void"))      &&
                (curToken != TEXT(";"))         )
            {
                TSTR lpSavedPos = lpTemp;
                String savedToken = curToken;

                BOOL bUniform;
                if(bUniform = (curToken == TEXT("uniform")))
                    HandMeAToken(curToken);

                String strType = curToken;

                String strName;
                HandMeAToken(strName);

                PeekAtAToken(curToken);
                if(curToken[0] != '(') //verified variable
                {
                    UINT paramID = Params.Num();

                    ShaderParam *param = Params.CreateNew();
                    param->name = strName;

                    if(scmpi_n(strType, TEXT("texture"), 7) == 0)
                    {
                        TSTR lpType = strType.Array()+7;
                        supr(lpType);

                        if (!*lpType ||
                            (scmp(lpType, TEXT("1D")) && scmp(lpType, TEXT("2D")) && scmp(lpType, TEXT("3D")) && scmp(lpType, TEXT("CUBE")))
                           )
                        {
                            AddError(TEXT("Invalid texture type"));
                        }

                        param->textureID = Textures.Num();

                        ShaderTex* texInfo = Textures.CreateNew();
                        texInfo->type = lpType;
                        texInfo->name = strName;
                        texInfo->paramID = paramID;

                        param->type = Parameter_Texture;

                        strType = TEXT("sampler");
                    }
                    else if(scmp_n(strType, TEXT("float"), 5) == 0)
                    {
                        CTSTR lpType = strType.Array()+5;

                        if(*lpType == 0)
                            param->type = Parameter_Float;
                        else if(scmpi(lpType, TEXT("2")) == 0)
                            param->type = Parameter_Vector2;
                        else if(scmpi(lpType, TEXT("3")) == 0)
                            param->type = Parameter_Vector3;
                        else if(scmpi(lpType, TEXT("4")) == 0)
                            param->type = Parameter_Vector4;
                        else if(scmpi(lpType, TEXT("3x3")) == 0)
                            param->type = Parameter_Matrix3x3;
                        else if(scmpi(lpType, TEXT("4x4")) == 0)
                            param->type = Parameter_Matrix;
                    }
                    else if(scmp(strType, TEXT("int")) == 0)
                        param->type = Parameter_Int;
                    else if(scmp(strType, TEXT("bool")) == 0)
                        param->type = Parameter_Bool;

                    //--------------------------

                    CopyStringRange(output, lpLastPos, lpCurPos);
                    if(bUniform)
                        output << TEXT("uniform ");
                    output << strType << TEXT(" ") << strName;

                    lpLastPos = lpTemp;
                    bNewCodeLine = FALSE;
                    continue;
                }

                lpTemp = lpSavedPos;
                curToken = savedToken;
            }
        }

        CopyStringRange(output, lpLastPos, lpCurPos);
        output << curToken;

        lpLastPos = lpTemp;

        bNewCodeLine = (curToken.IsValid() && ((curToken[0] == ';') || (curToken[0] == '}')));
    }

    return !errorCount;
}

#undef  ExpectToken
#define ExpectToken(expecting) {if(!GetNextToken(curToken)) {AddErrorEndOfCode(); return FALSE;} if(curToken != expecting) {AddErrorExpecting(expecting, curToken); return FALSE;}}

BOOL ShaderProcessor::AddState(SamplerInfo &info, String &stateName, String &stateVal)
{
    if(scmpi_n(stateName, TEXT("Address"), 7) == 0)
    {
        int type = stateName[7]-'U';

        GSAddressMode *mode;
        switch(type)
        {
            case 0: mode = &info.addressU; break;
            case 1: mode = &info.addressV; break;
            case 2: mode = &info.addressW;
        }

        if(stateVal.CompareI(TEXT("Wrap")) || stateVal.CompareI(TEXT("Repeat")))
            *mode = GS_ADDRESS_WRAP;
        else if(stateVal.CompareI(TEXT("Clamp")) || stateVal.CompareI(TEXT("None")))
            *mode = GS_ADDRESS_CLAMP;
        else if(stateVal.CompareI(TEXT("Mirror")))
            *mode = GS_ADDRESS_MIRROR;
        else if(stateVal.CompareI(TEXT("Border")))
            *mode = GS_ADDRESS_BORDER;
        else if(stateVal.CompareI(TEXT("MirrorOnce")))
            *mode = GS_ADDRESS_MIRRORONCE;
        else
            AddWarning(TEXT("Invalid address mode specified in sampler state"));
    }
    else if(stateName.CompareI(TEXT("MaxAnisotropy")))
    {
        info.maxAnisotropy = tstoi(stateVal);
        if(info.maxAnisotropy < 1 || info.maxAnisotropy > 16)
            AddWarning(TEXT("Invalid max anisotrophy specified in sampler state, must be 1-16"));
    }
    else if(stateName.CompareI(TEXT("Filter")))
    {
        if(stateVal.CompareI(TEXT("Anisotropic")))
            info.filter = GS_FILTER_ANISOTROPIC;
        else if(stateVal.CompareI(TEXT("Point")) || stateVal.CompareI(TEXT("MIN_MAG_MIP_POINT")))
            info.filter = GS_FILTER_POINT;
        else if(stateVal.CompareI(TEXT("Linear")) || stateVal.CompareI(TEXT("MIN_MAG_MIP_LINEAR")))
            info.filter = GS_FILTER_LINEAR;
        else if(stateVal.CompareI(TEXT("MIN_MAG_POINT_MIP_LINEAR")))
            info.filter = GS_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        else if(stateVal.CompareI(TEXT("MIN_POINT_MAG_LINEAR_MIP_POINT")))
            info.filter = GS_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        else if(stateVal.CompareI(TEXT("MIN_POINT_MAG_MIP_LINEAR")))
            info.filter = GS_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        else if(stateVal.CompareI(TEXT("MIN_LINEAR_MAG_MIP_POINT")))
            info.filter = GS_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        else if(stateVal.CompareI(TEXT("MIN_LINEAR_MAG_POINT_MIP_LINEAR")))
            info.filter = GS_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        else if(stateVal.CompareI(TEXT("MIN_MAG_LINEAR_MIP_POINT")))
            info.filter = GS_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        else
            AddWarning(TEXT("Invalid filter specified in sampler state"));
    }
    else if(stateName.CompareI(TEXT("BorderColor")))
    {
        if(stateVal[0] == '{')
        {
            String curToken;

            HandMeAToken(curToken);
            if(!ValidFloatString(curToken)) {AddErrorExpecting(TEXT("floating point"), curToken); return FALSE;}
            info.borderColor.x = tstof(curToken);

            //-------------------------------

            ExpectToken(TEXT(","));

            //-------------------------------

            HandMeAToken(curToken);
            if(!ValidFloatString(curToken)) {AddErrorExpecting(TEXT("floating point"), curToken); return FALSE;}
            info.borderColor.y = tstof(curToken);

            //-------------------------------

            ExpectToken(TEXT(","));

            //-------------------------------

            HandMeAToken(curToken);
            if(!ValidFloatString(curToken)) {AddErrorExpecting(TEXT("floating point"), curToken); return FALSE;}
            info.borderColor.z = tstof(curToken);

            //-------------------------------

            ExpectToken(TEXT(","));

            //-------------------------------


            HandMeAToken(curToken);
            if(!ValidFloatString(curToken)) {AddErrorExpecting(TEXT("floating point"), curToken); return FALSE;}
            info.borderColor.w = tstof(curToken);

            //-------------------------------

            ExpectToken(TEXT("}"));
        }
        else if(ValidIntString(stateVal))
            info.borderColor = Color4().MakeFromRGBA(tstoi(stateVal));
    }

    return TRUE;
}

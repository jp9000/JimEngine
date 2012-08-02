/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EffectProcessor.h

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


#ifndef EFFECTPROCESSOR_HEADER
#define EFFECTPROCESSOR_HEADER

#include "ScriptDefs.h"
#include "ScriptCompiler.h"

#define NUMEPTYPES 7

static CTSTR lpEffectTypes[NUMEPTYPES] = {TEXT("samplerstate"), TEXT("texture"), TEXT("float"), TEXT("double"), TEXT("int"), TEXT("bool"), TEXT("string")};


enum EffectDataType
{
    Effect_Param,
    Effect_Technique,
    Effect_Sampler,
    Effect_Pass
};

struct EffectData
{
    EffectDataType dataType;
};

struct EffectParam : EffectData
{
    ShaderParameterType type;
    String name;

    List<BYTE> CurValue;
    List<BYTE> DefaultValue;

    String fullName;
    EffectPropertyType propertyType;

    BOOL bChanged;

    union
    {
        struct {float fMin, fMax, fInc, fMul;};
        struct {int   iMin, iMax, iInc, iMul;};
    };

    void AddPropertyString(String &propString);

    inline void FreeData()
    {
        CurValue.Clear(); DefaultValue.Clear();
        name.Clear(); fullName.Clear();
    }
};

struct PassParam
{
    EffectParam* param;
    HANDLE handle;
};

struct EffectPass : EffectData
{
    String name;

    Shader          *vertShader;
    List<PassParam> VertParams;

    Shader          *pixelShader;
    List<PassParam> PixelParams;

    inline void FreeData()
    {
        name.Clear(); VertParams.Clear(); PixelParams.Clear();
        DestroyObject((Object*)vertShader); DestroyObject((Object*)pixelShader);
    }
};

struct EffectTechnique : EffectData
{
    String name;
    List<EffectPass> Passes;

    BOOL bValid;

    inline void FreeData()
    {
        for(int i=0; i<Passes.Num(); i++)
            Passes[i].FreeData();
        Passes.Clear();

        name.Clear();
    }
};

//-------------------------------------------------

struct EPVar
{
    String type;
    String name;
    String mapping;

    BOOL bUniform;
    BOOL bWritten;

    inline EPVar() {zero(this, sizeof(EPVar));}
    inline ~EPVar() {FreeData();}

    inline void FreeData()
    {
        type.Clear(); name.Clear(); mapping.Clear();
    }
};

struct EPParam
{
    String type;
    String name;
    List<BYTE> DefaultValue;

    BOOL bConst;
    BOOL bProperty;
    BOOL bTexture;

    BOOL bWritten;
    int  writeOrder;

    int  arrayCount;

    StringList Properties;

    EffectParam *param;

    inline void WriteVar(String &strOutput, StringList &UsedParams)
    {
        if(bWritten)
            return;

        if(bConst)
            strOutput << TEXT("const ");
        else
            UsedParams << name;

        strOutput << type << TEXT(" ") << name;
        if(arrayCount)
            strOutput << TEXT("[") << IntString(arrayCount) << TEXT("]");
        strOutput << TEXT(";\r\n");

        bWritten = TRUE;
    }

    inline ~EPParam() {FreeData();}

    inline void FreeData()
    {
        type.Clear(); name.Clear(); DefaultValue.Clear(); Properties.Clear();
    }
};

struct EPStruct
{
    String name;
    String contents;

    List<EPVar> Vars;

    BOOL bWritten;

    inline BOOL IsMapped() {return (Vars.Num() > 0) ? Vars[0].mapping.IsValid() : FALSE;}

    inline ~EPStruct() {FreeData();}

    inline void FreeData()
    {
        name.Clear(); contents.Clear();

        for(int i=0; i<Vars.Num(); i++)
            Vars[i].FreeData();
        Vars.Clear();
    }
};

struct EPSampler
{
    String name;
    StringList states;
    StringList values;
    String contents;

    BOOL bWritten;

    inline ~EPSampler() {FreeData();}

    inline void FreeData()
    {
        name.Clear(); contents.Clear(); states.Clear(); values.Clear();
    }
};

struct EPPass
{
    String name;

    String vertexProgram;
    String fragmentProgram;

    EffectPass *pass;

    inline ~EPPass() {FreeData();}

    inline void FreeData()
    {
        name.Clear(); vertexProgram.Clear(); fragmentProgram.Clear();
    }
};

struct EPTechnique
{
    String name;

    List<EPPass> Passes;

    inline ~EPTechnique() {FreeData();}

    inline void FreeData()
    {
        name.Clear();

        for(int i=0; i<Passes.Num(); i++)
            Passes[i].FreeData();
        Passes.Clear();
    }
};

struct EPFunc
{
    String name;
    String retType;
    String mapping;
    String contents;

    List<EPVar> Params;

    StringList funcDependancies;
    StringList structDependancies;
    StringList paramDependancies;
    StringList samplerDependancies;

    BOOL bWritten;

    inline ~EPFunc() {FreeData();}

    inline void FreeData()
    {
        name.Clear(); retType.Clear(); contents.Clear(); mapping.Clear();

        for(int i=0; i<Params.Num(); i++)
            Params[i].FreeData();
        Params.Clear();

        funcDependancies.Clear();
        structDependancies.Clear();
        paramDependancies.Clear();
        samplerDependancies.Clear();
    }
};


struct EffectProcessor : CodeTokenizer
{
    inline EffectProcessor() {compilerError = NULL; effect = NULL; curType = -1;}
    ~EffectProcessor()
    {
        for(int i=0; i<Params.Num(); i++)
            Params[i].FreeData();
        Params.Clear();

        for(int i=0; i<Structs.Num(); i++)
            Structs[i].FreeData();
        Structs.Clear();

        for(int i=0; i<Functions.Num(); i++)
            Functions[i].FreeData();
        Functions.Clear();

        for(int i=0; i<Samplers.Num(); i++)
            Samplers[i].FreeData();
        Samplers.Clear();

        for(int i=0; i<Techniques.Num(); i++)
            Techniques[i].FreeData();
        Techniques.Clear();
    }

    BOOL ProcessEffect(Effect *effectIn, CTSTR lpFileName, CTSTR lpEffectString, String &errorString, BOOL bRecurse=FALSE);

    BOOL ProcessFuncParams(EPFunc &curFunc);

    void CompileEffect();

    void MakeShaderString(String &shaderCall, String &shaderOut, StringList &UsedParams);
    void AddDependancies(EPFunc *func, String &strOutput, StringList &UsedParams);

    void ResetEffectData();

    Effect      *effect;

    EffectPass  *curPass;
    DWORD       curType;

    List<EPParam>       Params;
    List<EPStruct>      Structs;
    List<EPFunc>        Functions;
    List<EPSampler>     Samplers;
    List<EPTechnique>   Techniques;

    inline EPFunc* FindFunc(CTSTR lpFunc)
    {
        for(int i=0; i<Functions.Num(); i++)
        {
            if(Functions[i].name == lpFunc)
                return Functions+i;
        }

        return NULL;
    }

    inline EPStruct* FindStruct(CTSTR lpStruct)
    {
        for(int i=0; i<Structs.Num(); i++)
        {
            if(Structs[i].name == lpStruct)
                return Structs+i;
        }

        return NULL;
    }

    inline EPSampler* FindSampler(CTSTR lpSampler)
    {
        for(int i=0; i<Samplers.Num(); i++)
        {
            if(Samplers[i].name == lpSampler)
                return Samplers+i;
        }

        return NULL;
    }

    inline EPParam* FindParam(CTSTR lpParam)
    {
        for(int i=0; i<Params.Num(); i++)
        {
            if(Params[i].name == lpParam)
                return Params+i;
        }

        return NULL;
    }
};


#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Effect.cpp

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


DefineClass(Effect);


inline EffectParam* ConvertParam(HANDLE hObject)
{
    EffectParam *param = (EffectParam*)hObject;
    assert(param && param->dataType == Effect_Param);
    //if(!param || (param->dataType != Effect_Param)) return NULL;

    return param;
}

inline EffectTechnique* ConvertTechnique(HANDLE hObject)
{
    EffectTechnique *param = (EffectTechnique*)hObject;
    assert(param && param->dataType == Effect_Technique);
    //if(!param || (param->dataType != Effect_Technique)) return NULL;

    return param;
}

inline EffectPass* ConvertPass(HANDLE hObject)
{
    EffectPass *param = (EffectPass*)hObject;
    assert(param && param->dataType == Effect_Pass);
    //if(!param || (param->dataType != Effect_Pass)) return NULL;

    return param;
}


//================================================================================
// Effect
//================================================================================

Effect::Effect(GraphicsSystem *curSystem, CTSTR lpEffectFile)
{
    traceIn(Effect::Effect);

    system = curSystem;

    String strEffectFile = lpEffectFile;
    strEffectFile.FindReplace(TEXT("\\"), TEXT("/"));
    strEffectDir = GetPathDirectory(strEffectFile);

    effectPath = strEffectFile;

    XFile chi;

    if(!chi.Open(strEffectFile, XFILE_READ, XFILE_OPENEXISTING))
        ErrOut(TEXT("Could not open effect file '%s'"), strEffectFile.Array());

    String strFile;
    chi.ReadFileToString(strFile);
    chi.Close();

    String errors;

    bProcessing = TRUE;
    system->curProcessingEffect = this;

    EffectProcessor test;
    if(!test.ProcessEffect(this, GetPathFileName(strEffectFile, TRUE), strFile, errors))
        ProgramBreak();

    bProcessing = FALSE;
    system->curProcessingEffect = NULL;

    hViewProj = GetParameterByName(TEXT("ViewProj"));
    hWorld = GetParameterByName(TEXT("World"));
    hScale = GetParameterByName(TEXT("World"));

    traceOut;
}

Effect::~Effect()
{
    traceIn(Effect::~Effect);

    int i;

    for(i=0; i<Params.Num(); i++)
        Params[i].FreeData();
    Params.FreeAll();
    for(i=0; i<Techniques.Num(); i++)
        Techniques[i].FreeData();
    Techniques.FreeAll();

    traceOut;
}

void   Effect::GetEffectParameterInfo(HANDLE hObject, EffectParameterInfo &paramInfo) const
{
    traceIn(Effect::GetEffectParameterInfo);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    paramInfo.name = param->name;
    paramInfo.propertyType = param->propertyType;
    paramInfo.type = param->type;
    paramInfo.fullName = param->fullName;

    if(paramInfo.propertyType == EffectProperty_Float)
    {
        paramInfo.fMin = param->fMin;
        paramInfo.fMax = param->fMax;
        paramInfo.fInc = param->fInc;
        paramInfo.fMul = param->fMul;
    }
    else if(paramInfo.propertyType == EffectProperty_Texture)
    {
        if(param->DefaultValue.Num())
        {
            BufferInputSerializer s(param->DefaultValue);
            s << paramInfo.strDefaultTexture;
        }
    }

    traceOut;
}


HANDLE Effect::GetTechnique(CTSTR lpTechnique) const
{
    traceInFast(Effect::GetTechnique);

    for(int i=0; i<Techniques.Num(); i++)
    {
        EffectTechnique *tech = Techniques+i;
        if(tech->name == lpTechnique)
            return (HANDLE)tech;
    }

    return NULL;

    traceOutFast;
}

BOOL   Effect::UsableTechnique(HANDLE hObject) const
{
    traceInFast(Effect::GetTechnique);

    EffectTechnique *tech = ConvertTechnique(hObject);
    if(!tech) return FALSE;

    return tech->bValid;

    traceOutFast;
}


HANDLE Effect::GetPass(HANDLE hTechnique, DWORD i) const
{
    traceInFast(Effect::GetPass);

    EffectTechnique *tech = ConvertTechnique(hTechnique);
    if(!tech) return NULL;

    return (HANDLE)(tech->Passes+i);

    traceOutFast;
}

HANDLE Effect::GetPassByName(HANDLE hTechnique, CTSTR lpName) const
{
    traceInFast(Effect::GetPassByName);

    EffectTechnique *tech = ConvertTechnique(hTechnique);
    if(!tech) return NULL;

    for(int i=0; i<tech->Passes.Num(); i++)
    {
        EffectPass *pPass = tech->Passes+i;
        if(pPass->name == lpName)
            return (HANDLE)pPass;
    }

    return NULL;

    traceOutFast;
}

DWORD  Effect::BeginTechnique(HANDLE hTechnique)
{
    traceInFast(Effect::BeginTechnique);

    EffectTechnique *tech = ConvertTechnique(hTechnique);
    if(!tech) return 0;

    curTech = tech;

    system->curEffect = this;
    system->ResetViewMatrix();

    return tech->Passes.Num();

    traceOutFast;
}

void   Effect::EndTechnique()
{
    traceInFast(Effect::EndTechnique);

    curTech = NULL;

    system->curEffect = NULL;

    for(int i=0; i<Params.Num(); i++)
    {
        Params[i].CurValue.FreeAll();
        Params[i].bChanged = FALSE;
    }

    traceOutFast;
}


void   Effect::BeginPass(DWORD i)
{
    traceInFast(Effect::BeginPass);

    assertmsg(!curPass, TEXT("Called BeginPass while already in a pass."));

    assert(curTech);
    if(!curTech) return;

    assert(i < curTech->Passes.Num());
    if(i >= curTech->Passes.Num()) return;

    curPass = curTech->Passes+i;
    system->LoadPixelShader(curPass->pixelShader);
    system->LoadVertexShader(curPass->vertShader);

    for(int j=0; j<curPass->PixelParams.Num(); j++)
    {
        PassParam &param = curPass->PixelParams[j];
        param.param->curPShaderParam = param.handle;
        param.param->bValid = TRUE;
    }
    for(int j=0; j<curPass->VertParams.Num(); j++)
    {
        PassParam &param = curPass->VertParams[j];
        param.param->curVShaderParam = param.handle;
        param.param->bValid = TRUE;
    }

    UploadAllShaderParams();

    traceOutFast;
}

void   Effect::BeginPassByHandle(HANDLE hPass)
{
    traceInFast(Effect::BeginPassByHandle);

    assertmsg(!curPass, TEXT("Called BeginPass while already in a pass."));

    assert(curTech);
    if(!curTech) return;

    curPass = ConvertPass(hPass);
    system->LoadPixelShader(curPass->pixelShader);
    system->LoadVertexShader(curPass->vertShader);

    for(int j=0; j<curPass->PixelParams.Num(); j++)
    {
        PassParam &param = curPass->PixelParams[j];
        param.param->curPShaderParam = param.handle;
        param.param->bValid = TRUE;
    }
    for(int j=0; j<curPass->VertParams.Num(); j++)
    {
        PassParam &param = curPass->VertParams[j];
        param.param->curVShaderParam = param.handle;
        param.param->bValid = TRUE;
    }

    UploadAllShaderParams();

    traceOutFast;
}

void   Effect::EndPass()
{
    traceInFast(Effect::EndPass);

    assertmsg(curPass, TEXT("Called EndPass while not in a pass."));

    for(int i=0; i<curPass->PixelParams.Num(); i++)
    {
        PassParam &param = curPass->PixelParams[i];
        param.param->curPShaderParam = NULL;
        param.param->bChanged = FALSE;
        param.param->bValid = FALSE;

        if(param.param->type == Parameter_Texture)
            curPass->pixelShader->SetTexture(param.handle, NULL);
    }
    for(int i=0; i<curPass->VertParams.Num(); i++)
    {
        PassParam &param = curPass->VertParams[i];
        param.param->curVShaderParam = NULL;
        param.param->bChanged = FALSE;
        param.param->bValid = FALSE;

        if(param.param->type == Parameter_Texture)
            curPass->vertShader->SetTexture(param.handle, NULL);
    }

    curPass = NULL;
    firstChanged = NULL;

    system->LoadPixelShader(NULL);
    system->LoadVertexShader(NULL);

    traceOutFast;
}

//void UploadParam(Shader *shader, EffectParam *param, HANDLE handle);
inline void UploadParam(Shader *shader, EffectParam *param, HANDLE handle)
{
    if(!param->CurValue.Num())
    {
        if(param->DefaultValue.Num() && (param->type != Parameter_Texture))
            param->CurValue.CopyFrom(param->DefaultValue);
        else
            return;
    }

    switch(param->type)
    {
        case Parameter_Bool:
            shader->SetBool(handle, *(BOOL*)param->CurValue.Array()); break;
        case Parameter_Texture:
            shader->SetTexture(handle, *(BaseTexture**)param->CurValue.Array()); break;
        default:
            shader->SetValue(handle, param->CurValue.Array(), param->CurValue.Num());
    }
}

inline void Effect::UploadAllShaderParams()
{
    for(int i=0; i<Params.Num(); i++)
    {
        EffectParam *param = &Params[i];

        if(param->curVShaderParam) UploadParam(curPass->vertShader,  param, param->curVShaderParam);
        if(param->curPShaderParam) UploadParam(curPass->pixelShader, param, param->curPShaderParam);
    }
}

inline void Effect::UpdateParams()
{
    assertmsg(curPass, TEXT("Called UpdateParams while not in a pass."));

    EffectParam *param = firstChanged;
    if(!param)
        return;

    while(param)
    {
        if(param->curVShaderParam) UploadParam(curPass->vertShader,  param, param->curVShaderParam);
        if(param->curPShaderParam) UploadParam(curPass->pixelShader, param, param->curPShaderParam);
        param->bChanged = FALSE;

        param = param->nextChanged;
    }

    firstChanged = NULL;
}

inline HANDLE Effect::GetParameter(int parameter) const
{
    if(parameter >= Params.Num()) return NULL;

    return (HANDLE)(Params+parameter);
}

inline HANDLE Effect::GetParameterByName(CTSTR lpName) const
{
    for(int i=0; i<Params.Num(); i++)
    {
        EffectParam *curParam = Params+i;
        if(curParam->name == lpName)
            return (HANDLE)curParam;
    }

    return NULL;
}


inline void   Effect::GetBool(HANDLE hObject, BOOL &bValue) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    if(param->CurValue.Num() >= sizeof(BOOL))
        bValue = *(BOOL*)param->CurValue.Array();
}

inline void   Effect::GetFloat(HANDLE hObject, float &fValue) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    if(param->CurValue.Num() >= sizeof(float))
        fValue = *(float*)param->CurValue.Array();
}

inline void   Effect::GetInt(HANDLE hObject, int &iValue) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    if(param->CurValue.Num() >= sizeof(int))
        iValue = *(int*)param->CurValue.Array();
}

inline void   Effect::GetMatrix(HANDLE hObject, float *matrix) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    if(param->CurValue.Num() == (sizeof(float)*16))
        mcpy(matrix, param->CurValue.Array(), (sizeof(float)*16));
}

inline void   Effect::GetVector(HANDLE hObject, Vect &value) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    if(param->CurValue.Num() >= sizeof(Vect))
        value = *(Vect*)param->CurValue.Array();
}

inline void   Effect::GetVector2(HANDLE hObject, Vect2 &value) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    if(param->CurValue.Num() >= sizeof(Vect2))
        value = *(Vect2*)param->CurValue.Array();
}

inline void   Effect::GetVector4(HANDLE hObject, Vect4 &value) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    if(param->CurValue.Num() >= sizeof(Vect4))
        value = *(Vect4*)param->CurValue.Array();
}

inline void   Effect::GetString(HANDLE hObject, String &value) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    if(param->CurValue.Num())
    {
        BufferInputSerializer s(param->CurValue);
        s << value;
    }
}


inline void   Effect::SetBool(HANDLE hObject, BOOL bValue)
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    BOOL bSizeChanged = param->CurValue.SetSize(sizeof(BOOL));
    BOOL &curVal = *(BOOL*)param->CurValue.Array();

    if(bSizeChanged || curVal != bValue)
    {
        curVal = bValue;

        if(param->bValid && !param->bChanged)
        {
            param->nextChanged = firstChanged;
            firstChanged = param;
            param->bChanged = TRUE;
        }
    }
}

inline void   Effect::SetFloat(HANDLE hObject, float fValue)
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    BOOL bSizeChanged = param->CurValue.SetSize(sizeof(float));
    float &curVal = *(float*)param->CurValue.Array();

    if(bSizeChanged || curVal != fValue)
    {
        curVal = fValue;

        if(param->bValid && !param->bChanged)
        {
            param->nextChanged = firstChanged;
            firstChanged = param;
            param->bChanged = TRUE;
        }
    }
}

inline void   Effect::SetInt(HANDLE hObject, int iValue)
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    BOOL bSizeChanged = param->CurValue.SetSize(sizeof(int));
    int &curVal = *(int*)param->CurValue.Array();

    if(bSizeChanged || curVal != iValue)
    {
        curVal = iValue;

        if(param->bValid && !param->bChanged)
        {
            param->nextChanged = firstChanged;
            firstChanged = param;
            param->bChanged = TRUE;
        }
    }
}

inline void   Effect::SetMatrix(HANDLE hObject, float *matrix)
{
    SetValue(hObject, matrix, sizeof(float)*4*4);
}

inline void   Effect::SetVector(HANDLE hObject, const Vect &value)
{
    SetValue(hObject, value.ptr, sizeof(Vect));
}

inline void   Effect::SetVector2(HANDLE hObject, const Vect2 &value)
{
    SetValue(hObject, value.ptr, sizeof(Vect2));
}

inline void   Effect::SetVector4(HANDLE hObject, const Vect4 &value)
{
    SetValue(hObject, value.ptr, sizeof(Vect4));
}

inline void   Effect::SetTexture(HANDLE hObject, const BaseTexture *texture)
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    BOOL bSizeChanged = param->CurValue.SetSize(sizeof(const BaseTexture*));
    const BaseTexture *&curVal = *(const BaseTexture**)param->CurValue.Array();

    if(bSizeChanged || curVal != texture)
    {
        curVal = texture;

        if(param->bValid && !param->bChanged)
        {
            param->nextChanged = firstChanged;
            firstChanged = param;
            param->bChanged = TRUE;
        }
    }
}

inline void   Effect::SetValue(HANDLE hObject, const void *val, DWORD dwSize)
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    //if(!param) return;

    BOOL bSizeChanged = param->CurValue.SetSize(dwSize);

    if(bSizeChanged || !mcmp(param->CurValue.Array(), val, dwSize))
    {
        mcpy(param->CurValue.Array(), val, dwSize);

        if(param->bValid && !param->bChanged)
        {
            param->nextChanged = firstChanged;
            firstChanged = param;
            param->bChanged = TRUE;
        }
    }
}


inline BOOL   Effect::GetDefaultBool(HANDLE hObject, BOOL &bValue) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    if(!param || (param->DefaultValue.Num() < sizeof(BOOL))) return FALSE;

    bValue = *(BOOL*)param->DefaultValue.Array();
    return TRUE;
}

inline BOOL   Effect::GetDefaultFloat(HANDLE hObject, float &fValue) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    if(!param || (param->DefaultValue.Num() < sizeof(float))) return FALSE;

    fValue = *(float*)param->DefaultValue.Array();
    return TRUE;
}

inline BOOL   Effect::GetDefaultInt(HANDLE hObject, int &iValue) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    if(!param || (param->DefaultValue.Num() < sizeof(int))) return FALSE;

    iValue = *(int*)param->DefaultValue.Array();
    return TRUE;
}

inline BOOL   Effect::GetDefaultMatrix(HANDLE hObject, float *matrix) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    if(!param || (param->DefaultValue.Num() < (sizeof(float)*4*4))) return FALSE;

    mcpy(matrix, param->DefaultValue.Array(), sizeof(float)*4*4);
    return TRUE;
}

inline BOOL   Effect::GetDefaultVector(HANDLE hObject, Vect &value) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    if(!param || (param->DefaultValue.Num() < sizeof(float)*3)) return FALSE;

    mcpy(value, param->DefaultValue.Array(), sizeof(float)*3);
    return TRUE;
}

inline BOOL   Effect::GetDefaultVector2(HANDLE hObject, Vect2 &value) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    if(!param || (param->DefaultValue.Num() < sizeof(Vect2))) return FALSE;

    value = *(Vect2*)param->DefaultValue.Array();
    return TRUE;
}

inline BOOL   Effect::GetDefaultVector4(HANDLE hObject, Vect4 &value) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    if(!param || (param->DefaultValue.Num() < sizeof(Vect4))) return FALSE;

    value = *(Vect4*)param->DefaultValue.Array();
    return TRUE;
}

inline BOOL   Effect::GetDefaultTexture(HANDLE hObject, String &value) const
{
    assert(hObject);

    EffectParam *param = ConvertParam(hObject);
    if(!param || !param->DefaultValue.Num()) return FALSE;

    BufferInputSerializer sIn(param->DefaultValue);
    sIn << value;
    return TRUE;
}


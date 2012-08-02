/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  D3DShader.cpp

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


D3DShader *D3DShader::CreateShader(D3DSystem *curSystem, ShaderType type, CTSTR lpShader)
{
    traceIn(D3DShader::CreateShader);

    LPCSTR lpTarget;
    switch(type)
    {
        case Shader_Pixel:
            lpTarget = "ps_3_0"; break;
        case Shader_Vertex:
            lpTarget = "vs_3_0"; break;
    }

    String strProcessedShader;
    String errorString;

    ShaderProcessor shaderProcessor;
    if(!shaderProcessor.ProcessShader(lpShader, strProcessedShader, errorString))
    {
        String strMessage;
        if(curSystem->CurrentProcessingEffect())
            strMessage << curSystem->CurrentProcessingEffect()->ProcessingInfo() << TEXT(":\r\n\r\n");

        if(errorString.IsValid())
            strMessage << TEXT("Error parsing shader: ") << errorString;
        else
            strMessage << TEXT("An unknown error has occured in the shader processor.");

        AppWarning(strMessage);
        return NULL;
    }

    LPSTR lpAnsiShader = strProcessedShader.CreateUTF8String();
    strProcessedShader.Clear();

    ID3DXBuffer *compiledShader, *shaderErrors;
    ID3DXConstantTable *hlslNewConstantTable;
    if(!SUCCEEDED(D3DXCompileShader(lpAnsiShader, (UINT)strlen(lpAnsiShader), NULL, NULL, "main", lpTarget, D3DXSHADER_OPTIMIZATION_LEVEL3, &compiledShader, &shaderErrors, &hlslNewConstantTable)))
    {
#ifdef _DEBUG
        XFile test123(TEXT("chi.txt"), XFILE_WRITE, XFILE_CREATEALWAYS);
        test123.Write(lpAnsiShader, (DWORD)strlen(lpAnsiShader));
        test123.Close();
#endif

        if(curSystem->CurrentProcessingEffect())
            AppWarning(TEXT("%s:\r\n\r\nError compiling shader:\r\n\r\n%S"), curSystem->CurrentProcessingEffect()->ProcessingInfo().Array(), (LPSTR)shaderErrors->GetBufferPointer());
        else
            AppWarning(TEXT("Error compiling shader:\r\n\r\n%S"), (LPSTR)shaderErrors->GetBufferPointer());

        if(shaderErrors)
            shaderErrors->Release();
        if(compiledShader)
            compiledShader->Release();
        Free(lpAnsiShader);
        return NULL;
    }

    IDirect3DVertexShader9 *vShader;
    IDirect3DPixelShader9  *pShader;

    HRESULT result;
    if(type == Shader_Vertex)
        result = curSystem->d3dDevice->CreateVertexShader((DWORD*)compiledShader->GetBufferPointer(), &vShader);
    else if(type == Shader_Pixel)
        result = curSystem->d3dDevice->CreatePixelShader((DWORD*)compiledShader->GetBufferPointer(), &pShader);

    if(!SUCCEEDED(result))
    {
#ifdef _DEBUG
        XFile test123(TEXT("chi.txt"), XFILE_WRITE, XFILE_CREATEALWAYS);
        test123.Write(lpAnsiShader, (DWORD)strlen(lpAnsiShader));
        test123.Close();
#endif

        if(result == D3DERR_INVALIDCALL)
            ErrOut(TEXT("CreateVertexShader, invalid call."));
        else if(result == D3DERR_OUTOFVIDEOMEMORY)
            ErrOut(TEXT("CreateVertexShader, out of video mem."));
        else if(result == E_OUTOFMEMORY)
            ErrOut(TEXT("CreateVertexShader, out of normal mem."));

        AppWarning(TEXT("Could not create vertex shader"));

        if(shaderErrors)
            shaderErrors->Release();
        if(compiledShader)
            compiledShader->Release();
        if(hlslNewConstantTable)
            hlslNewConstantTable->Release();

        Free(lpAnsiShader);
        return NULL;
    }

    Free(lpAnsiShader);

    //--------------------------------------------------------

    D3DShader *outShader = CreateObject(D3DShader);
    outShader->hlslConstantTable = hlslNewConstantTable;
    outShader->shaderType = type;
    outShader->d3d = curSystem;

    if(type == Shader_Vertex)
        outShader->vertexShader = vShader;
    else if(type == Shader_Pixel)
        outShader->pixelShader = pShader;

    outShader->Params = shaderProcessor.Params;
    outShader->Samplers = shaderProcessor.Samplers;
    zero(&shaderProcessor.Params,   sizeof(List<ShaderParam>));
    zero(&shaderProcessor.Samplers, sizeof(List<ShaderSampler>));

    int textureOffset = 0;

    for(int i=0; i<outShader->Params.Num(); i++)
    {
        ShaderParam &param = outShader->Params[i];

        char utfName[64];
        tstr_to_utf8(param.name, utfName, 63);
        param.hD3DObj = hlslNewConstantTable->GetConstantByName(NULL, utfName);

        if(param.type == Parameter_Texture)
        {
            if(param.hD3DObj)
                param.textureID = textureOffset++;
        }
    }

    if(shaderErrors)
        shaderErrors->Release();
    if(compiledShader)
        compiledShader->Release();

    return outShader;

    traceOut;
}

D3DShader::~D3DShader()
{
    traceIn(D3DShader::~D3DShader);

    D3DRelease(vertexShader);
    D3DRelease(hlslConstantTable);

    for(int i=0; i<Params.Num(); i++)
        Params[i].FreeData();
    Params.Clear();

    for(int i=0; i<Samplers.Num(); i++)
        Samplers[i].FreeData();
    Samplers.Clear();

    traceOut;
}

HANDLE D3DShader::GetParameter(int parameter)
{
    if(parameter >= Params.Num())
        return NULL;
    return (HANDLE)(Params+parameter);
}

HANDLE D3DShader::GetParameterByName(CTSTR lpName)
{
    for(int i=0; i<Params.Num(); i++)
    {
        ShaderParam &param = Params[i];
        if(param.name == lpName)
            return (HANDLE)&param;
    }

    return NULL;
}

void   D3DShader::GetParameterInfo(HANDLE hObject, ShaderParameterInfo &paramInfo)
{
    assert(hObject); //if(!hObject) return;
    ShaderParam *param = (ShaderParam*)hObject;

    paramInfo.type = param->type;
    paramInfo.name = param->name;
}


void   D3DShader::SetBool(HANDLE hObject, BOOL bValue)
{
    //profileSingularSegment("D3DShader::SetBool");

    assert(hObject); //if(!hObject) return;
    ShaderParam *param = (ShaderParam*)hObject;

    if(!bUsingShader || !param->hD3DObj)
        return;

    hlslConstantTable->SetBool(d3d->d3dDevice, param->hD3DObj, bValue);
}

void   D3DShader::SetFloat(HANDLE hObject, float fValue)
{
    //profileSingularSegment("D3DShader::SetFloat");

    assert(hObject); if(!hObject) return;
    ShaderParam *param = (ShaderParam*)hObject;

    if(!bUsingShader || !param->hD3DObj)
        return;

    hlslConstantTable->SetFloat(d3d->d3dDevice, param->hD3DObj, fValue);
}

void   D3DShader::SetInt(HANDLE hObject, int iValue)
{
    assert(hObject); //if(!hObject) return;
    ShaderParam *param = (ShaderParam*)hObject;

    if(!bUsingShader || !param->hD3DObj)
        return;

    hlslConstantTable->SetInt(d3d->d3dDevice, param->hD3DObj, iValue);
}

void   D3DShader::SetMatrix(HANDLE hObject, float *matrix)
{
    //profileSingularSegment("D3DShader::SetMatrix");

    assert(matrix);

    assert(hObject); //if(!hObject) return;
    ShaderParam *param = (ShaderParam*)hObject;

    if(!bUsingShader || !param->hD3DObj)
        return;

    hlslConstantTable->SetMatrix(d3d->d3dDevice, param->hD3DObj, (D3DXMATRIX*)matrix);
}

void   D3DShader::SetVector(HANDLE hObject, const Vect &value)
{
    //profileSingularSegment("D3DShader::SetVector");

    assert(hObject); //if(!hObject) return;
    ShaderParam *param = (ShaderParam*)hObject;

    if(!bUsingShader || !param->hD3DObj)
        return;

    hlslConstantTable->SetFloatArray(d3d->d3dDevice, param->hD3DObj, value.ptr, 3);
}

void   D3DShader::SetVector2(HANDLE hObject, const Vect2 &value)
{
    //profileSingularSegment("D3DShader::SetVector2");

    assert(hObject); //if(!hObject) return;
    ShaderParam *param = (ShaderParam*)hObject;

    if(!bUsingShader || !param->hD3DObj)
        return;

    hlslConstantTable->SetFloatArray(d3d->d3dDevice, param->hD3DObj, value.ptr, 2);
}

void   D3DShader::SetVector4(HANDLE hObject, const Vect4 &value)
{
    //profileSingularSegment("D3DShader::SetVector4");

    assert(hObject); //if(!hObject) return;
    ShaderParam *param = (ShaderParam*)hObject;

    if(!bUsingShader || !param->hD3DObj)
        return;

    hlslConstantTable->SetFloatArray(d3d->d3dDevice, param->hD3DObj, value.ptr, 4);
}

void   D3DShader::SetTexture(HANDLE hObject, BaseTexture *texture)
{
    //profileSingularSegment("D3DShader::SetTexture");

    assert(hObject); //if(!hObject) return;
    ShaderParam *param = (ShaderParam*)hObject;

    if(!bUsingShader || !param->hD3DObj)
        return;

    if(d3d->curTextures[param->textureID] == texture)
        return;

    if(!texture)
        d3d->d3dDevice->SetTexture(param->textureID, NULL);
    else
    {
        ShaderSampler *sampler = &Samplers[param->samplerID];
        if(d3d->curSamplers[param->textureID] != sampler->sampler)
            d3d->LoadSamplerState(sampler->sampler, param->textureID);

        d3d->d3dDevice->SetTexture(param->textureID, GetD3DTex(texture));
    }

    d3d->curTextures[param->textureID] = texture;
}

void   D3DShader::SetValue(HANDLE hObject, void *val, DWORD dwSize)
{
    assert(val && dwSize);

    assert(hObject); //if(!hObject) return;
    ShaderParam *param = (ShaderParam*)hObject;

    if(!bUsingShader || !param->hD3DObj)
        return;

    hlslConstantTable->SetValue(d3d->d3dDevice, param->hD3DObj, val, dwSize);
}

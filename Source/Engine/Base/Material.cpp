/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Material.cpp

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


DefineClass(Material);


Material::~Material()
{
    traceIn(Material::~Material);

    FreeParameters();

    traceOut;
}

void Material::FreeParameters()
{
    traceIn(Material::FreeParameters);

    for(DWORD i=0; i<Params.Num(); i++)
    {
        MaterialParameter &param = Params[i];

        if(param.type == Parameter_Texture)
        {
            BaseTexture* texture = *(BaseTexture**)param.data;

            if(flags & MATERIAL_EDITING)
                DestroyObject(texture);
            else
                RM->ReleaseTexture(texture);
        }
    }
    Params.Clear();

    traceOut;
}


BOOL Material::LoadParameters()
{
    traceInFast(Material::LoadParameters);

    for(DWORD i=0; i<Params.Num(); i++)
    {
        MaterialParameter &param = Params[i];

        switch(param.type)
        {
            case Parameter_Bool:
                effect->SetBool(param.handle, *(BOOL*)param.data); break;
            case Parameter_Float:
                effect->SetFloat(param.handle, *(float*)param.data); break;
            case Parameter_Int:
                effect->SetInt(param.handle, *(int*)param.data); break;
            case Parameter_Vector2:
                effect->SetVector2(param.handle, *(Vect2*)param.data); break;
            case Parameter_Vector3:
                effect->SetVector(param.handle, *(Vect*)param.data); break;
            case Parameter_Vector4:
                effect->SetVector4(param.handle, *(Vect4*)param.data); break;
            case Parameter_Matrix:
                effect->SetMatrix(param.handle, (float*)param.data); break;
            case Parameter_Texture:
                {
                    BaseTexture *tex = *(BaseTexture**)param.data;
                    if(!tex)
                        return FALSE;

                    effect->SetTexture(param.handle, tex);
                    break;
                }
        }
    }

    return TRUE;

    traceOutFast;
}

BOOL Material::LoadFromFile(CTSTR lpFile)
{
    traceIn(Material::LoadFromFile);

    String path;

    ConfigFile materialFile;
    if(!materialFile.Open(lpFile))
    {
        AppWarning(TEXT("Couldn't load material file '%s'"), lpFile);
        return FALSE;
    }

    effect = ::GetEffect(materialFile.GetString(TEXT("Material"), TEXT("Effect")));

    if(!effect)
    {
        AppWarning(TEXT("Invalid effect in material file '%s'"), lpFile);
        return FALSE;
    }

    String soundName = materialFile.GetString(TEXT("Material"), TEXT("SoftSound"));
    if(soundName.IsValid()) SetSoftHitSound(soundName);
    soundName = materialFile.GetString(TEXT("Material"), TEXT("HardSound"));
    if(soundName.IsValid()) SetHardHitSound(soundName);

    restitution = materialFile.GetFloat(TEXT("Material"), TEXT("Restitution"));
    friction    = materialFile.GetFloat(TEXT("Material"), TEXT("Friction"), 0.5f);

    DWORD curParamID = 0;

    HANDLE hCurParam;
    while(hCurParam = effect->GetParameter(curParamID++))
    {
        EffectParameterInfo paramInfo;
        effect->GetEffectParameterInfo(hCurParam, paramInfo);

        if(paramInfo.propertyType != EffectProperty_None)
        {
            if(paramInfo.propertyType == EffectProperty_Texture)
            {
                MaterialParameter *param = Params.CreateNew();
                param->type = Parameter_Texture;
                param->handle = hCurParam;
                *(BaseTexture**)param->data = GetTexture(materialFile.GetString(TEXT("Parameters"), paramInfo.name));
            }
            else if(paramInfo.propertyType == EffectProperty_Color)
            {
                MaterialParameter *param = Params.CreateNew();
                param->type = Parameter_Vector3;
                param->handle = hCurParam;
                Vect chi = materialFile.GetColor3(TEXT("Parameters"), paramInfo.name);
                mcpy(param->data, &chi, sizeof(Vect));
            }
            else if(paramInfo.propertyType == EffectProperty_Float)
            {
                MaterialParameter *param = Params.CreateNew();
                param->type = Parameter_Float;
                param->handle = hCurParam;
                *(float*)param->data = materialFile.GetFloat(TEXT("Parameters"), paramInfo.name)*paramInfo.fMul;
            }
        }
    }

    return TRUE;

    traceOut;
}

void Material::SetCurrentEffect(Effect *effectIn)
{
    traceIn(Material::SetCurrentEffect);

    if(effectIn == effect)
        return;

    Params.Clear();
    effect = effectIn;
    if(!effectIn)
        return;

    DWORD curParamID = 0;

    HANDLE hCurParam;
    while(hCurParam = effect->GetParameter(curParamID++))
    {
        EffectParameterInfo paramInfo;
        effect->GetEffectParameterInfo(hCurParam, paramInfo);

        if(paramInfo.propertyType != EffectProperty_None)
        {
            if(paramInfo.propertyType == EffectProperty_Texture)
            {
                MaterialParameter *param = Params.CreateNew();
                param->type = Parameter_Texture;
                param->handle = hCurParam;
            }
            else if(paramInfo.propertyType == EffectProperty_Color)
            {
                MaterialParameter *param = Params.CreateNew();
                param->type = Parameter_Vector3;
                param->handle = hCurParam;
            }
            else if(paramInfo.propertyType == EffectProperty_Float)
            {
                MaterialParameter *param = Params.CreateNew();
                param->type = Parameter_Float;
                param->handle = hCurParam;
            }
        }
    }

    traceOut;
}

Effect* Material::GetCurrentEffect()
{
    return effect;
}

MaterialParameter *Material::GetParam(CTSTR paramName)
{
    for(int i=0; i<Params.Num(); i++)
    {
        MaterialParameter &param = Params[i];

        EffectParameterInfo paramInfo;
        effect->GetEffectParameterInfo(param.handle, paramInfo);

        if(paramInfo.name.CompareI(paramName))
            return &param;
    }

    return NULL;
}

int Material::GetParamID(CTSTR paramName)
{
    for(int i=0; i<Params.Num(); i++)
    {
        MaterialParameter &param = Params[i];

        EffectParameterInfo paramInfo;
        effect->GetEffectParameterInfo(param.handle, paramInfo);

        if(paramInfo.name.CompareI(paramName))
            return i;
    }

    return INVALID;
}

void Material::SetFloat(CTSTR paramName, float fValue)
{
    MaterialParameter *param = GetParam(paramName);
    if(!param || param->type != Parameter_Float)
        return;

    *(float*)param->data = fValue;
}

void Material::SetColor(CTSTR paramName, DWORD color)
{
    MaterialParameter *param = GetParam(paramName);
    if(!param || param->type != Parameter_Vector3)
        return;

    ((Vect*)param->data)->MakeFromRGB(color);
}

void Material::SetCurrentTexture(CTSTR paramName, BaseTexture *texture)
{
    MaterialParameter *param = GetParam(paramName);
    if(!param || param->type != Parameter_Texture)
        return;

    *(BaseTexture**)param->data = texture;
}


float Material::GetFloat(CTSTR paramName)
{
    MaterialParameter *param = GetParam(paramName);
    if(!param || param->type != Parameter_Float)
        return 0.0f;

    return *(float*)param->data;
}

DWORD Material::GetColor(CTSTR paramName)
{
    MaterialParameter *param = GetParam(paramName);
    if(!param || param->type != Parameter_Vector3)
        return 0;

    return (*(Vect*)param->data).GetRGB();
}

BaseTexture* Material::GetCurrentTexture(CTSTR paramName)
{
    MaterialParameter *param = GetParam(paramName);
    if(!param || param->type != Parameter_Texture)
        return NULL;

    return *(BaseTexture**)param->data;
}


void Material::SetFloat(int id, float fValue)
{
    if(id > Params.Num())
        return;

    MaterialParameter *param = &Params[id];
    if(!param || param->type != Parameter_Float)
        return;

    EffectParameterInfo paramInfo;
    effect->GetEffectParameterInfo(param->handle, paramInfo);

    *(float*)param->data = fValue*paramInfo.fMul;
}

void Material::SetColor(int id, DWORD color)
{
    if(id > Params.Num())
        return;

    MaterialParameter *param = &Params[id];
    if(!param || param->type != Parameter_Vector3)
        return;

    ((Vect*)param->data)->MakeFromRGB(color);
}

void Material::SetCurrentTexture(int id, BaseTexture *texture)
{
    if(id > Params.Num())
        return;

    MaterialParameter *param = &Params[id];
    if(!param || param->type != Parameter_Texture)
        return;

    *(BaseTexture**)param->data = texture;
}


float Material::GetFloat(int id)
{
    if(id > Params.Num())
        return NULL;

    MaterialParameter *param = &Params[id];
    if(!param || param->type != Parameter_Float)
        return NULL;

    return *(float*)param->data;
}

DWORD Material::GetColor(int id)
{
    if(id > Params.Num())
        return NULL;

    MaterialParameter *param = &Params[id];
    if(!param || param->type != Parameter_Vector3)
        return NULL;

    return (*(Vect*)param->data).GetRGB();
}

BaseTexture* Material::GetCurrentTexture(int id)
{
    if(id > Params.Num())
        return NULL;

    MaterialParameter *param = &Params[id];
    if(!param || param->type != Parameter_Texture)
        return NULL;

    return *(BaseTexture**)param->data;
}

void Material::ProcessCollision(PhyObject *obj, PhyObject *collider, float appliedImpulse, const Vect &hitPos)
{
    traceIn(Material::ProcessCollision);

    if(appliedImpulse < 4.0f)
        return;

    //static int flong = 0;

    CTSTR soundName = (appliedImpulse > 100.0f) ? GetHardHitSound() : GetSoftHitSound();
    if(soundName)
    {
        float pitch = (RandomFloat()*0.05f)+(sinf(atanf(appliedImpulse*0.1f))*0.1f);

        Sound *sound = NewSound(soundName, TRUE, TRUE);
        sound->SetPitch(pitch);
        sound->SetPosition(hitPos);
        sound->SetRange(50.0f);
        sound->SetVolume(MAX(appliedImpulse*0.02f, 1.0f));
        sound->Play(FALSE);
        //Log(TEXT("flong: %d, appliedImpulse %f, pitch %f, hitSound: %s"), flong, appliedImpulse, pitch, soundName);

        //flong++;
    }

    traceOut;
}
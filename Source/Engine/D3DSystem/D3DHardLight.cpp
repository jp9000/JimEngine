/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  D3DHardwareLight.cpp:  Direct3D 9 Light Management

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

const DWORD LightTypes[] = {D3DLIGHT_POINT, D3DLIGHT_DIRECTIONAL, D3DLIGHT_SPOT};

D3DHardwareLight::D3DHardwareLight(D3DSystem *curSystem, DWORD dwNewType)
{
    d3d = curSystem;

    dwType = dwNewType;
    LightNum = d3d->curLightCount++;

    cutoff = 0.0;
    SetAmbientColor(0xFF333333);
    SetDiffuseColor(0x80FFFFFF);
    SetSpecularColor(0xFFFFFFFF);
    isOn = 0;
    //ResetLightData();
}

D3DHardwareLight::~D3DHardwareLight()
{
    d3d->curLightCount--;
}

void  D3DHardwareLight::ResetLightData()
{
    D3DLIGHT9 lightdata;
    zero(&lightdata, sizeof(D3DLIGHT9));

    mcpy(&lightdata.Ambient, &AmbientRGB, 16);
    mcpy(&lightdata.Diffuse, &DiffuseRGB, 16);
    mcpy(&lightdata.Specular, &SpecularRGB, 16);
    lightdata.Type    = (D3DLIGHTTYPE)LightTypes[dwType];
    switch(dwAttenuationType)
    {
        case 0:
            lightdata.Attenuation0 = 1.0f;
            lightdata.Attenuation1 = 0.0f;
            lightdata.Attenuation2 = 0.0f;
            break;
        case 1:
            lightdata.Attenuation0 = 0.0f;
            lightdata.Attenuation1 = attenuation;
            lightdata.Attenuation2 = 0.0f;
            break;
        case 2:
            lightdata.Attenuation0 = 0.0f;
            lightdata.Attenuation1 = 0.0f;
            lightdata.Attenuation2 = attenuation;
            break;
    }
    lightdata.Range = 2000.0f;
    mcpy(&lightdata.Position, &pos, 12);

    d3d->d3dDevice->SetLight(0, &lightdata);
}

void  D3DHardwareLight::Enable(BOOL bEnable)
{
    isOn = bEnable;
    ResetLightData();
    d3d->d3dDevice->LightEnable(0, bEnable);
}

void  D3DHardwareLight::SetIntensity(float intensity)
{
}

void  D3DHardwareLight::SetAmbientColor(DWORD dwRGB)
{
    Color4 color(RGB_Rf(dwRGB), RGB_Gf(dwRGB), RGB_Bf(dwRGB), 0.0);
    AmbientRGB = color;
    if(isOn)
        ResetLightData();
}

void  D3DHardwareLight::SetDiffuseColor(DWORD dwRGBA)
{
    Color4 color(RGB_Rf(dwRGBA), RGB_Gf(dwRGBA), RGB_Bf(dwRGBA), 0.0);
    DiffuseRGB = color;
    if(isOn)
        ResetLightData();
}

void  D3DHardwareLight::SetSpecularColor(DWORD dwRGB)
{
    Color4 color(RGB_Rf(dwRGB), RGB_Gf(dwRGB), RGB_Bf(dwRGB), 0.0);
    SpecularRGB = color;
    if(isOn)
        ResetLightData();
}

void  D3DHardwareLight::SetAttenuation(DWORD dwAttenuationTypeIn, float attenuationIn)
{
    dwAttenuationType = dwAttenuationTypeIn;
    attenuation = attenuationIn;

    if(isOn)
        ResetLightData();
}

void  D3DHardwareLight::Retransform()
{
    pos.x = d3d->MatrixStack[d3d->curMatrix].T.x;
    pos.y = d3d->MatrixStack[d3d->curMatrix].T.y;
    pos.z = -d3d->MatrixStack[d3d->curMatrix].T.z;

    if(isOn)
        ResetLightData();
}

void  D3DHardwareLight::SetCutoff(float degrees)
{
    cutoff = degrees;
    if(isOn)
        ResetLightData();
}

void  D3DHardwareLight::SetType(DWORD dwNewType)
{
    dwType = dwNewType;
    Retransform();
}

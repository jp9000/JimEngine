/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  OutdoorWeather.cpp

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


void OutdoorLevel::Tick(float fTime)
{
    float fT;

    curTimeOfDay += fTime*fDaySpeed;
    curTimeOfDay = fmodf(curTimeOfDay, 2400.0f);

    //-------------------------------------------------------------
    // Calculate water ripple
    Ripple1Value += Ripple1Speed*fTime;
    Ripple2Value += Ripple2Speed*fTime;

    for(int i=0; i<2; i++)
    {
        while(Ripple1Value.ptr[i] > 1.0f)
            Ripple1Value.ptr[i] -= 1.0f;
        while(Ripple1Value.ptr[i] < -1.0f)
            Ripple1Value.ptr[i] += 1.0f;

        while(Ripple2Value.ptr[i] > 1.0f)
            Ripple2Value.ptr[i] -= 1.0f;
        while(Ripple2Value.ptr[i] < -1.0f)
            Ripple2Value.ptr[i] += 1.0f;
    }

    //-------------------------------------------------------------
    // Calculate moon positions
    for(int i=0; i<Moons.Num(); i++)
    {
        MoonData &moon = Moons[i];
        moon.curPos = moon.fLatitude-((curTimeOfDay/2400.0f)*360.0f)-90.0f;
    }

    //-------------------------------------------------------------
    // Compute Transition
    /*if(bTransitioningWeather)
    {
        fWeatherTransition += newWeather->fTransitionSpeed*fTime;
        if(fWeatherTransition >= 1.0f)
        {
            bTransitioningWeather = FALSE;
            curWeather = newWeather;

            if(!FogTable) FogTable = CreateTexture(512, 1, GS_R32F, Allocate(512*sizeof(float)), FALSE, FALSE);
            float* lpFogTex = (float*)FogTable->GetImage();

            CreateFogTable(curWeather->FogType, curWeather->fFogDepth, 1.0f, lpFogTex);

            FogTable->SetImage(lpFogTex);
        }
        else
        {
            float lpNewTable[512];

            if(!FogTable) FogTable = CreateTexture(512, 1, GS_R32F, Allocate(512*sizeof(float)), FALSE, FALSE);
            float* lpFogTex = (float*)FogTable->GetImage();

            CreateFogTable(curWeather->FogType, curWeather->fFogDepth, 1.0f, lpFogTex);
            CreateFogTable(newWeather->FogType, newWeather->fFogDepth, 1.0f, lpNewTable);

            for(int i=0; i<128; i++)
            {
                float oldValue = lpFogTex[i];
                float newValue = lpNewTable[i];

                lpFogTex[i] = BYTE(Lerp(oldValue, newValue, fWeatherTransition)*128.0f);
            }

            FogTable->SetImage(lpFogTex);
        }
    }*/

    //-------------------------------------------------------------
    // Compute Sky color and Sun position/visibility

    float fMoonVisibility = 0.0f;

    fSunVisibility = 0.0f;

    if((curTimeOfDay > 800.0f) && (curTimeOfDay <= 1600.0f))
    {
        if(bTransitioningWeather)
        {
            curSkyColor = Lerp(curWeather->SkyColorDay, newWeather->SkyColorDay, fWeatherTransition);
            curSunColor = Lerp(curWeather->SunColorDay, newWeather->SunColorDay, fWeatherTransition);
            curFogColor = Lerp(curWeather->FogColorDay, newWeather->FogColorDay, fWeatherTransition);
            curAmbientColor = Lerp(curWeather->AmbientColorDay, newWeather->AmbientColorDay, fWeatherTransition);
        }
        else
        {
            curSkyColor = curWeather->SkyColorDay;
            curSunColor = curWeather->SunColorDay;
            curFogColor = curWeather->FogColorDay;
            curAmbientColor = curWeather->AmbientColorDay;
        }

        fSunVisibility = 1.0f;
    }
    else if((curTimeOfDay > 2000.0f) || (curTimeOfDay <= 400.0f))
    {
        if(bTransitioningWeather)
        {
            curSkyColor = Lerp(curWeather->SkyColorNight, newWeather->SkyColorNight, fWeatherTransition);
            curFogColor = Lerp(curWeather->FogColorNight, newWeather->FogColorNight, fWeatherTransition);
            curAmbientColor = Lerp(curWeather->AmbientColorNight, newWeather->AmbientColorNight, fWeatherTransition);
        }
        else
        {
            curSkyColor = curWeather->SkyColorNight;
            curFogColor = curWeather->FogColorNight;
            curAmbientColor = curWeather->AmbientColorNight;
        }

        fMoonVisibility = 1.0f;
    }
    else if((curTimeOfDay > 400.0f) && (curTimeOfDay <= 800.0f))
    {
        if(curTimeOfDay <= 500.0f)
        {
            fT = (curTimeOfDay-400.0f)/100.0f;
            if(bTransitioningWeather)
            {
                Color3 newColor;
                curSkyColor = Lerp(curWeather->SkyColorNight, curWeather->SkyColorDawn, fT);
                newColor    = Lerp(newWeather->SkyColorNight, newWeather->SkyColorDawn, fT);
                curSkyColor = Lerp(curSkyColor, newColor, fWeatherTransition);

                curSunColor = Lerp(curWeather->SkyColorDawn, newWeather->SkyColorDawn, fWeatherTransition);

                curFogColor = Lerp(curWeather->FogColorNight, curWeather->FogColorDawn, fT);
                newColor    = Lerp(newWeather->FogColorNight, newWeather->FogColorDawn, fT);
                curFogColor = Lerp(curFogColor, newColor, fWeatherTransition);

                curAmbientColor = Lerp(curWeather->AmbientColorNight, curWeather->AmbientColorDawn, fT);
                newColor        = Lerp(newWeather->AmbientColorNight, newWeather->AmbientColorDawn, fT);
                curAmbientColor = Lerp(curAmbientColor, newColor, fWeatherTransition);
            }
            else
            {
                curSkyColor = Lerp(curWeather->SkyColorNight, curWeather->SkyColorDawn, fT);
                curSunColor = curWeather->SkyColorDawn;
                curFogColor = Lerp(curWeather->FogColorNight, curWeather->FogColorDawn, fT);
                curAmbientColor = Lerp(curWeather->AmbientColorNight, curWeather->AmbientColorDawn, fT);
            }

            fMoonVisibility = 1.0f-fT;
        }
        else
        {
            fT = (curTimeOfDay-500.0f)/300.0f;
            if(bTransitioningWeather)
            {
                Color3 newColor;
                curSkyColor = Lerp(curWeather->SkyColorDawn, curWeather->SkyColorDay, fT);
                newColor    = Lerp(newWeather->SkyColorDawn, newWeather->SkyColorDay, fT);
                curSkyColor = Lerp(curSkyColor, newColor, fWeatherTransition);

                curSunColor = Lerp(curWeather->SunColorDawn, curWeather->SunColorDay, fT);
                newColor    = Lerp(newWeather->SunColorDawn, newWeather->SunColorDay, fT);
                curSunColor = Lerp(curSunColor, newColor, fWeatherTransition);

                curFogColor = Lerp(curWeather->FogColorDawn, curWeather->FogColorDay, fT);
                newColor    = Lerp(newWeather->FogColorDawn, newWeather->FogColorDay, fT);
                curFogColor = Lerp(curFogColor, newColor, fWeatherTransition);

                curAmbientColor = Lerp(curWeather->AmbientColorDawn, curWeather->AmbientColorDay, fT);
                newColor        = Lerp(newWeather->AmbientColorDawn, newWeather->AmbientColorDay, fT);
                curAmbientColor = Lerp(curAmbientColor, newColor, fWeatherTransition);
            }
            else
            {
                curSkyColor = Lerp(curWeather->SkyColorDawn, curWeather->SkyColorDay, fT);
                curSunColor = Lerp(curWeather->SunColorDawn, curWeather->SunColorDay, fT);
                curFogColor = Lerp(curWeather->FogColorDawn, curWeather->FogColorDay, fT);
                curAmbientColor = Lerp(curWeather->AmbientColorDawn, curWeather->AmbientColorDay, fT);
            }

            fSunVisibility = fT;
        }
    }
    else if((curTimeOfDay > 1600.0f) && (curTimeOfDay <= 2000.0f))
    {
        if(curTimeOfDay <= 1900.0f)
        {
            fT = (curTimeOfDay-1600.0f)/300.0f;
            if(bTransitioningWeather)
            {
                Color3 newColor;
                curSkyColor = Lerp(curWeather->SkyColorDay, curWeather->SkyColorDusk, fT);
                newColor    = Lerp(newWeather->SkyColorDay, newWeather->SkyColorDusk, fT);
                curSkyColor = Lerp(curSkyColor, newColor, fWeatherTransition);

                curSunColor = Lerp(curWeather->SunColorDay, curWeather->SunColorDusk, fT);
                newColor    = Lerp(newWeather->SunColorDay, newWeather->SunColorDusk, fT);
                curSunColor = Lerp(curSunColor, newColor, fWeatherTransition);

                curFogColor = Lerp(curWeather->FogColorDay, curWeather->FogColorDusk, fT);
                newColor    = Lerp(newWeather->FogColorDay, newWeather->FogColorDusk, fT);
                curFogColor = Lerp(curFogColor, newColor, fWeatherTransition);

                curAmbientColor = Lerp(curWeather->AmbientColorDay, curWeather->AmbientColorDusk, fT);
                newColor        = Lerp(newWeather->AmbientColorDay, newWeather->AmbientColorDusk, fT);
                curAmbientColor = Lerp(curAmbientColor, newColor, fWeatherTransition);
            }
            else
            {
                curSkyColor = Lerp(curWeather->SkyColorDay, curWeather->SkyColorDusk, fT);
                curSunColor = Lerp(curWeather->SunColorDay, curWeather->SunColorDusk, fT);
                curFogColor = Lerp(curWeather->FogColorDay, curWeather->FogColorDusk, fT);
                curAmbientColor = Lerp(curWeather->AmbientColorDay, curWeather->AmbientColorDusk, fT);
            }

            fSunVisibility = 1.0f-fT;
        }
        else
        {
            fT = (curTimeOfDay-1900.0f)/100.0f;
            if(bTransitioningWeather)
            {
                Color3 newColor;
                curSkyColor = Lerp(curWeather->SkyColorDusk, curWeather->SkyColorNight, fT);
                newColor    = Lerp(newWeather->SkyColorDusk, newWeather->SkyColorNight, fT);
                curSkyColor = Lerp(curSkyColor, newColor, fWeatherTransition);

                curSunColor = Lerp(curWeather->SunColorDusk, newWeather->SunColorDusk, fWeatherTransition);

                curFogColor = Lerp(curWeather->FogColorDusk, curWeather->FogColorNight, fT);
                newColor    = Lerp(newWeather->FogColorDusk, newWeather->FogColorNight, fT);
                curFogColor = Lerp(curFogColor, newColor, fWeatherTransition);

                curAmbientColor = Lerp(curWeather->AmbientColorDusk, curWeather->AmbientColorNight, fT);
                newColor        = Lerp(newWeather->AmbientColorDusk, newWeather->AmbientColorNight, fT);
                curAmbientColor = Lerp(curAmbientColor, newColor, fWeatherTransition);
            }
            else
            {
                curSkyColor = Lerp(curWeather->SkyColorDusk, curWeather->SkyColorNight, fT);
                curSunColor = curWeather->SunColorDusk;
                curFogColor = Lerp(curWeather->FogColorDusk, curWeather->FogColorNight, fT);
                curAmbientColor = Lerp(curWeather->AmbientColorDusk, curWeather->AmbientColorNight, fT);
            }

            fMoonVisibility = fT;
        }
    }

    if(sunLight && moonLight)
    {
        float angle = 90.0f+(curTimeOfDay/2400.0)*360.0;
        sunLight->SetRot(AxisAngle(1.0f, 0.0f, 0.0f, angle).GetQuat());

        sunLight->SetColor(curSunColor * fSunVisibility);

        moonLight->SetRot(AxisAngle(1.0f, 0.0f, 0.0f, angle-180.0f).GetQuat());

        Color3 curMoonShine;

        if(bTransitioningWeather)
            curMoonShine = Lerp(curWeather->MoonShineColor, newWeather->MoonShineColor, fWeatherTransition);
        else
            curMoonShine = curWeather->MoonShineColor;

        moonLight->SetColor(curMoonShine * fMoonVisibility);
    }

    float fSpeed = curWeather->fCloudSpeed;

    if(bTransitioningWeather)
        fSpeed += Lerp(fSpeed, newWeather->fCloudSpeed, fWeatherTransition);

    fCurCloudOffset += (fSpeed/60.0f)*(fTime/12.0f);

    while(fCurCloudOffset >= 1.0f) fCurCloudOffset -= 1.0f;

    skyOriantation.CreateFromQuat(AxisAngle(-1.0f, 0.0f, 0.0f, (curTimeOfDay/2400.0)*360.0).GetQuat());

    //-------------------------------------------------------------

    Super::Tick(fTime);
}


void OutdoorLevel::SetWeather(TSTR lpWeather, BOOL bInstant)
{
    int i;

    if(!lpWeather)
    {
        SetWeather(TEXT("Default"), bInstant);
        return;
    }

    if(curWeather && curWeather->weatherName.CompareI(lpWeather))
        return;

    for(i=0; i<WeatherList.Num(); i++)
    {
        if(WeatherList[i].weatherName.CompareI(lpWeather))
        {
            newWeather = &WeatherList[i];
            break;
        }
    }

    if(i == WeatherList.Num())
        return;

    /*if(bInstant)
    {
        bTransitioningWeather = FALSE;
        curWeather = newWeather;

        if(!FogTable) FogTable = CreateTexture(512, 1, GS_R32F, Allocate(512*sizeof(float)), FALSE, FALSE);
        float* lpFogTex = (float*)FogTable->GetImage();

        CreateFogTable(curWeather->FogType, curWeather->fFogDepth, 1.0f, lpFogTex);
        FogTable->SetImage(lpFogTex);
    }
    else
    {
        bTransitioningWeather = TRUE;
        fWeatherTransition = 0.0f;
    }*/
}

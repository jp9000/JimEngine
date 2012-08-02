/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  DSSystem.cpp

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


#include "DSSystem.h"



DefineClass(DSSoundStream);
DefineClass(DSSound);

DefineClass(DSSystem);


DSSystem::DSSystem()
{
    traceIn(DSSystem::DSSystem);

    BOOL bHighQuality = AppConfig->GetInt(TEXT("Sound"), TEXT("HighQuality"));
    BOOL bUseHardware = AppConfig->GetInt(TEXT("Sound"), TEXT("HardwareSound"));
    bGlobal = AppConfig->GetInt(TEXT("Sound"), TEXT("BackgroundSound"));

    WAVEFORMATEX wfe;
    DSBUFFERDESC dsbd;

    bHardware = bUseHardware;

    ////////////////////////////////////////////////////
    //initialize structures.
    zero(&dsbd, sizeof(dsbd));
    dsbd.dwSize = sizeof(dsbd);
    //dsbd.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME;
    /*if(bHardware)
        dsbd.dwFlags |= DSBCAPS_LOCHARDWARE;*/

    zero(&wfe, sizeof(wfe));
    wfe.cbSize = sizeof(WAVEFORMATEX);
    wfe.nChannels = 2;
    wfe.wBitsPerSample = 16;
    wfe.nBlockAlign = (wfe.nChannels*wfe.wBitsPerSample)/8;
    wfe.wFormatTag = WAVE_FORMAT_PCM;

    if(bHighQuality)
        wfe.nSamplesPerSec = 44100;
    else
        wfe.nSamplesPerSec = 22050;

    wfe.nAvgBytesPerSec = (wfe.nSamplesPerSec*wfe.nBlockAlign);


    /*wfe.nChannels = 1;
    wfe.nBlockAlign = wfe.wBitsPerSample/8;
    wfe.nAvgBytesPerSec = (wfe.nSamplesPerSec*wfe.nBlockAlign);*/

    if(!SUCCEEDED(DirectSoundCreate8(NULL, &dSound, NULL)))
        ErrOut(TEXT("DSSystem: Could not create the dSound Object"));
    if(!SUCCEEDED(dSound->SetCooperativeLevel((HWND)hwndMain, DSSCL_PRIORITY)))
        ErrOut(TEXT("DSSystem: Could not set the dSound primary cooperation level"));
    if(!SUCCEEDED(dSound->CreateSoundBuffer(&dsbd, &dSoundPBuffer, NULL)))
        ErrOut(TEXT("DSSystem: Could not create the dSound primary buffer"));
    if(!SUCCEEDED(dSoundPBuffer->SetFormat(&wfe)))
        ErrOut(TEXT("DSSystem: Could not set the dSoundPBuffer primary buffer format"));
    if(!SUCCEEDED(dSoundPBuffer->QueryInterface(IID_IDirectSound3DListener, (LPVOID*)&ds3DListener)))
        ErrOut(TEXT("DSSystem: Could not query the ds3DListener Object."));

    Log(TEXT("DirectSound Device Initialized"));

    traceOut;
}

DSSystem::~DSSystem()
{
    traceIn(DSSystem::~DSSystem);

    DWORD num = DSSound::EffectsList.Num();
    for(DWORD i=0; i<num; i++)
        delete DSSound::EffectsList[0];
    DSSound::EffectsList.Clear();

    if(ds3DListener)
        ds3DListener->Release();
    if(dSoundPBuffer)
        dSoundPBuffer->Release();

#ifndef _DEBUG
    if(dSound)
        dSound->Release();
#endif

    Log(TEXT("DirectSound Device Destroyed"));

    traceOut;
}

Sound *DSSystem::CreateSound(LPBYTE lpData, BOOL b3DSound, BOOL bSelfDestruct)
{
    traceIn(DSSystem::CreateSound);

    return (Sound*)InitializeObjectData(new DSSound(lpData, bHardware, b3DSound, bSelfDestruct));

    traceOut;
}

SoundStream *DSSystem::CreateSoundStream(const SoundStreamInfo &ssi)
{
    traceIn(DSSystem::CreateSoundStream);

    return (SoundStream*)InitializeObjectData(new DSSoundStream(ssi));

    traceOut;
}

void DSSystem::SetListenerPosition(const Vect &pos)
{
    assert(ds3DListener);

    curPos = pos;

    ds3DListener->SetPosition(pos.x, pos.y, -pos.z, DS3D_DEFERRED);

    bPositionSet = TRUE;
}

void DSSystem::SetListenerOriantation(const Quat &rot)
{
    assert(ds3DListener);

    Vect front(0.0f, 0.0f, -1.0f), top(0.0f, 1.0f, 0.0f);
    Matrix rotMatrix(rot);

    rotMatrix.Transpose();

    front.TransformVector(rotMatrix);
    top.TransformVector(rotMatrix);

    ds3DListener->SetOrientation(front.x, front.y, -front.z, top.x, top.y, -top.z, DS3D_DEFERRED);
}

void DSSystem::SetListenerVelocity(const Vect &vel)
{
    assert(ds3DListener);

    ds3DListener->SetVelocity(vel.x, vel.y, vel.z, DS3D_DEFERRED);
}

void DSSystem::UpdatePositionalData()
{
    assert(ds3DListener);

    ds3DListener->CommitDeferredSettings();
}

void DSSystem::SetEffectsVol(float vol)
{
    if(vol > 1.0f) vol = 1.0f;
    if(vol < 0.0f) vol = 0.0f;

    effectsVol = vol;

    for(int i=0; i<DSSound::EffectsList.Num(); i++)
    {
        DSSound *sound = DSSound::EffectsList[i];

        sound->ResetVolume(); //reset volume
    }
}

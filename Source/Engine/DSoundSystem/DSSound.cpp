/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  DSSound.cpp

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


List<DSSound*> DSSound::EffectsList;

DSSound::DSSound(LPBYTE lpData, BOOL bStatic, BOOL b3DSound, BOOL bSelfDestructIn)
{
    traceIn(DSSound::DSSound);

    WAVEHeader wave;
    WAVEFORMATEX wfe;
    DSBUFFERDESC dsbd;

    IDirectSoundBuffer *baseBuffer;
    DSSystem *dSS = static_cast<DSSystem*>(SS);
    IDirectSound8 *dSound = dSS->dSound;
    mcpy(&wave, lpData, sizeof(WAVEHeader));

    if( (wave.fmt != ' tmf') ||
        (wave.dataSigniture != 'atad') ||
        (wave.riff != 'FFIR') ||
        (wave.wave != 'EVAW') )
        ErrOut(TEXT("DSSystem: Invalid sound file format"));

    if((wave.Channels == 2) && b3DSound)
        ErrOut(TEXT("DSSystem: Tried to open stereo sound for 3D sound output"));

    assert(dSound);

    bIs3DSound = b3DSound;

    zero(&wfe, sizeof(wfe));
    wfe.cbSize = sizeof(WAVEFORMATEX);
    wfe.nAvgBytesPerSec = wave.BytesPerSec;
    wfe.nBlockAlign = wave.BlockAlign;
    wfe.nChannels = wave.Channels;
    wfe.nSamplesPerSec = wave.SamplesPerSec;
    wfe.wBitsPerSample = wave.BitsPerSample;
    wfe.wFormatTag = wave.Format;

    zero(&dsbd, sizeof(dsbd));
    dsbd.lpwfxFormat = &wfe;
    dsbd.dwBufferBytes = wave.dataSize;
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwReserved = NULL;
    dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPOSITIONNOTIFY;

    if(bStatic)
        dsbd.dwFlags |= DSBCAPS_STATIC;

    if(dSS->bGlobal)
        dsbd.dwFlags |= DSBCAPS_GLOBALFOCUS;

    if(b3DSound)
    {
        dsbd.dwFlags |= DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
        dsbd.guid3DAlgorithm = DS3DALG_HRTF_FULL;
    }

    HRESULT res = dSound->CreateSoundBuffer(&dsbd, &baseBuffer, NULL);

    if(!SUCCEEDED(res))
        ErrOut(TEXT("DSSystem: Could not create %s sound buffer, error code %d"), b3DSound ? TEXT("3D") : TEXT("normal"), res);

    if(!SUCCEEDED(baseBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&buffer)))
        ErrOut(TEXT("DSSystem: Could not query the main buffer object"));
    baseBuffer->Release();

    soundVol = 1.0f;

    //----------------------------------------

    if(b3DSound)
    {
        if(!SUCCEEDED(buffer->QueryInterface(IID_IDirectSound3DBuffer8, (LPVOID*)&buffer3D)))
            ErrOut(TEXT("DSSystem: Could not query 3D sound buffer interface"));

        res = buffer3D->SetMaxDistance(1000.0f, DS3D_DEFERRED);
        res = buffer3D->SetMinDistance(1000.0f, DS3D_DEFERRED);
        innerRange = 10.0f;
        outerRange = 1000.0f;

        curPos.Set(0.0f, 0.0f, 0.0f);
    }
    else
        rangeVol = 1.0f;

    //----------------------------------------
    // Notification stuff

    bSelfDestruct = bSelfDestructIn;

    IDirectSoundNotify8 *notify;
    if(!SUCCEEDED(buffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&notify)))
        ErrOut(TEXT("DSSystem: Could not query sound notification interface"));

    hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    DSBPOSITIONNOTIFY notifyPos;
    notifyPos.dwOffset = DSBPN_OFFSETSTOP;
    notifyPos.hEventNotify = hStopEvent;
    res = notify->SetNotificationPositions(1, &notifyPos);
    notify->Release();

    //----------------------------------------

    ResetVolume();

    res = buffer->GetFrequency(&mainFrequency);

    dwWaitToPlay = 0;

    //----------------------------------------

    LPBYTE lpWaveData;
    DWORD dwLockSize;
    res = buffer->Lock(0, 0, (LPVOID*)&lpWaveData, &dwLockSize, NULL, NULL, DSBLOCK_ENTIREBUFFER);
    mcpy(lpWaveData, lpData+sizeof(WAVEHeader), dwLockSize);
    res = buffer->Unlock(lpWaveData, dwLockSize, NULL, 0);

    EffectsList << this;

    traceOut;
}

DSSound::~DSSound()
{
    traceIn(DSSound::~DSSound);

    if(hStopEvent)
        CloseHandle(hStopEvent);

    if(buffer3D)
        buffer3D->Release();
    if(buffer)
        buffer->Release();

    EffectsList.RemoveItem(this);

    traceOut;
}

BOOL DSSound::RestoreBuffer()
{
    assert(buffer);

    DWORD status;
    buffer->GetStatus(&status);

    if(status & DSBSTATUS_BUFFERLOST)
    {
        if(buffer->Restore() == DSERR_BUFFERLOST)
            return 0;
    }

    return 1;
}

void DSSound::Play(BOOL bLoop)
{
    assert(buffer);

    bPlaying = TRUE;

    if(bIs3DSound)
    {
        if(!bPositionSet)
            Log(TEXT("DSSound: Warning, tried to play a 3D Sound without a set position"));

        if(!bPositionSet || !static_cast<DSSystem*>(SS)->bPositionSet)
        {
            dwWaitToPlay = 2;
            bPlayLooped = bLoop;
            return;
        }
    }

    if(RestoreBuffer())
        buffer->Play(0, 0, bLoop ? DSBPLAY_LOOPING : 0);
}

void DSSound::Stop()
{
    assert(buffer);

    buffer->Stop();
    buffer->SetCurrentPosition(0);

    bPlaying = FALSE;
}

void DSSound::SetPosition(const Vect &pos)
{
    assert(bIs3DSound);
    assert(buffer3D);

    if(bIs3DSound)
    {
        curPos = pos;
        buffer3D->SetPosition(pos.x, pos.y, -pos.z, DS3D_DEFERRED);

        bPositionSet = TRUE;
    }
}

void DSSound::SetVelocity(const Vect &vel)
{
    assert(bIs3DSound);
    assert(buffer3D);

    if(bIs3DSound)
        buffer3D->SetVelocity(vel.x, vel.y, -vel.z, DS3D_DEFERRED);
}

void DSSound::SetRange(float soundRange)
{
    assert(bIs3DSound);
    assert(buffer3D);

    if(bIs3DSound)
    {
        buffer3D->SetMinDistance(soundRange, DS3D_DEFERRED);
        buffer3D->SetMaxDistance(soundRange, DS3D_DEFERRED);

        outerRange = soundRange;
        innerRange = soundRange*0.01f;
    }
}


void DSSound::PreFrame()
{
    traceIn(DSSound::PreFrame);

    if(WaitForSingleObject(hStopEvent, 0) == WAIT_OBJECT_0)
    {
        if(!bPlayLooped)
        {
            bPlaying = FALSE;

            if(bSelfDestruct)
            {
                SafeDestroy();
                return;
            }
        }
    }

    if(dwWaitToPlay && bPositionSet && static_cast<DSSystem*>(SS)->bPositionSet)
    {
        if(dwWaitToPlay == 1)
        {
                if(bIs3DSound)
                {
                    buffer3D->SetPosition(curPos.x, curPos.y, -curPos.z, DS3D_DEFERRED);
                    buffer3D->SetMinDistance(outerRange, DS3D_DEFERRED);
                    buffer3D->SetMaxDistance(outerRange, DS3D_DEFERRED);
                }
            if(RestoreBuffer())
            {
                HRESULT test;
                if(!SUCCEEDED(test = buffer->Play(0, 0, bPlayLooped ? DSBPLAY_LOOPING : 0)))
                {
                    assert(0);
                    Log(TEXT("DSSoundSystem: Sound failed to play."));
                }
            }
        }

        --dwWaitToPlay;
    }

    AdjustDistanceVolume();

    traceOut;
}


void DSSound::AdjustDistanceVolume()
{
    if(bIs3DSound)
    {
        float dist = static_cast<DSSystem*>(SS)->curPos.Dist(curPos);

        if(dist <= innerRange)
            rangeVol = 1.0f;
        else if(dist <= outerRange)
            rangeVol = 1.0f-((dist-innerRange)/(outerRange-innerRange));
        else
            rangeVol = 0.0f;

        ResetVolume();
    }
}


float DSSound::GetRange()
{
    assert(bIs3DSound);
    assert(buffer3D);

    return outerRange;
}


void DSSound::SetVolume(float vol)
{
    if(vol < 0.0f)
        vol = 0.0f;
    else if(vol > 1.0f)
        vol = 1.0f;

    soundVol = vol;

    ResetVolume();
}


void DSSound::SetPitch(float pitchAdjust)
{
    if(pitchAdjust < -1.0f)
        pitchAdjust = -1.0f;
    else if(pitchAdjust > 1.0f)
        pitchAdjust = 1.0f;

    pitch = pitchAdjust;

    if(pitchAdjust < 0.0f)
        pitchAdjust *= 0.5f;

    pitchAdjust += 1.0f;

    buffer->SetFrequency((DWORD)((float)mainFrequency*pitchAdjust));
}

void DSSound::ResetVolume()
{
    float vol = soundVol*SS->GetEffectsVol();

    if(bIs3DSound)
    {
        vol *= rangeVol;
        vol *= 0.5f;
    }

    buffer->SetVolume(ConvertToDBVolume(vol));
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  DSSystem.h

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


#ifndef DXSOUND_HEADER
#define DXSOUND_HEADER

//#include <windows.h>

#define DIRECTSOUND_VERSION 0x800
#include <dsound.h>

#include <Base.h>



class DSSoundStream;


/*====================================================================
    Sound Stream class
=====================================================================*/

struct PlayThreadInfo
{
    DSSoundStream *stream;
    LPVOID param;
};

DWORD ENGINEAPI PlayThread(PlayThreadInfo *pti);

class DSSoundStream : public SoundStream
{
    DeclareClass(DSSoundStream, SoundStream);

    friend class DSSystem;
    friend DWORD ENGINEAPI PlayThread(PlayThreadInfo *pti);

    BOOL RestoreBuffer();

public:
    DSSoundStream() {}
    DSSoundStream(const SoundStreamInfo &ssi);
    ~DSSoundStream();

    void Play();
    void Stop();

    void SetVolume(float vol);
    float GetVolume() {return streamVol;}

    void AddStreamData(LPBYTE lpData);

    int GetCurBlockSize() {return curBlockSize;}

    void SetStreamCallback(NEXTSTREAMSEGMENTPROC proc, LPVOID param);

    static List<DSSoundStream*> StreamList;

private:
    IDirectSoundBuffer8 *buffer;

    HANDLE hStreamThread;
    NEXTSTREAMSEGMENTPROC StreamProc;
    LPVOID TSTReamProcParam;
    DWORD curWritePos, dwStreamDelay;
    BOOL bPlaying;
    HANDLE hStopEvent;

    float streamVol;

    DWORD dwStreamSize;

    int curBlockSize;
};



/*====================================================================
    Individual Sound class
=====================================================================*/

class DSSound : public Sound
{
    DeclareClass(DSSound, Sound);

    friend class DSSystem;

    BOOL RestoreBuffer();

public:
    DSSound()
        : soundVol(1.0f), rangeVol(0.0f)
    {}

    DSSound(LPBYTE lpData, BOOL bStatic, BOOL b3DSound, BOOL bSelfDestructIn);
    ~DSSound();

    void Play(BOOL bLoop);
    void Stop();

    void SetVolume(float vol);
    float GetVolume()         {return soundVol;}

    void SetPosition(const Vect &pos);
    void SetVelocity(const Vect &vel);

    void SetPitch(float pitchAdjust);
    float GetPitch()    {return pitch;}

    void PreFrame();

    void AdjustDistanceVolume();

    void SetRange(float soundRange);
    float GetRange();

    static List<DSSound*> EffectsList;

private:
    void ResetVolume();

    IDirectSoundBuffer8 *buffer;
    IDirectSound3DBuffer8 *buffer3D;
    BOOL bIs3DSound;

    HANDLE hStopEvent;

    Vect curPos;
    BOOL bPositionSet;

    BOOL bPlaying;
    BOOL bSelfDestruct;

    BOOL bPlayLooped;
    DWORD dwWaitToPlay;

    float soundVol;
    float rangeVol;

    float innerRange;
    float outerRange;

    DWORD mainFrequency;
    float pitch;
};


/*====================================================================
    Sound System class
=====================================================================*/

class DSSystem : public SoundSystem
{
    DeclareClass(DSSystem, SoundSystem);

    friend class DSSound;
    friend class DSSoundStream;

public:
    DSSystem();
    ~DSSystem();

    Sound *CreateSound(LPBYTE lpData, BOOL b3DSound, BOOL bSelfDestruct);
    SoundStream *CreateSoundStream(const SoundStreamInfo &ssi);

    void SetListenerPosition(const Vect &pos);
    void SetListenerVelocity(const Vect &vel);
    void SetListenerOriantation(const Quat &rot);
    void UpdatePositionalData();

    void SetEffectsVol(float vol);

private:
    BOOL bHardware;
    BOOL bGlobal;

    Vect curPos;
    BOOL bPositionSet;

    IDirectSound8 *dSound;
    IDirectSoundBuffer *dSoundPBuffer;
    IDirectSound3DListener8 *ds3DListener;
};


inline int ConvertToDBVolume(float vol) //0.0 to 1.0
{
    return (int)((vol <= 1e-5f) ? -10000.0f : (20.0f * log10(vol))*100.0f);
}


#endif
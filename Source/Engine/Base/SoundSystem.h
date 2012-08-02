/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  SoundSystem.h:  Sound Base class

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


#ifndef SOUNDDRIVER_HEADER
#define SOUNDDRIVER_HEADER



class SoundStream;


/*====================================================================
    Wave Header
=====================================================================*/

typedef struct
{
    DWORD       riff;           // 'FFIR'
    DWORD       riffSize;
    DWORD       wave;           // 'EVAW'
    DWORD       fmt;            // ' tmf'
    DWORD       fmtSize;
    WORD        Format;                              
    WORD        Channels;
    DWORD       SamplesPerSec;
    DWORD       BytesPerSec;
    WORD        BlockAlign;
    WORD        BitsPerSample;
    DWORD       dataSigniture;  // 'atad'
    DWORD       dataSize;
} WAVEHeader;


/*====================================================================
    Sound Stream class
=====================================================================*/

typedef DWORD (ENGINEAPI* NEXTSTREAMSEGMENTPROC)(SoundStream*, LPVOID);


//////////////////////////////
// Sound Stream Info Struct
typedef struct
{
    WORD        Channels;
    WORD        BitsPerSample;
    DWORD       SamplesPerSecond;
} SoundStreamInfo;


//////////////////////////////
// Sound Stream
class BASE_EXPORT SoundStream : public Object
{
    DeclareClass(SoundStream, Object);

public:
    virtual ~SoundStream() {}

    virtual void Play()=0;
    virtual void Stop()=0;

    virtual void SetVolume(float vol)=0;
    virtual float GetVolume()=0;

    virtual void AddStreamData(LPBYTE lpData)=0;

    virtual int GetCurBlockSize()=0;

    virtual void SetStreamCallback(NEXTSTREAMSEGMENTPROC proc, LPVOID param)=0;
};


/*====================================================================
    Individual Sound class
=====================================================================*/

class BASE_EXPORT Sound : public FrameObject
{
    friend class ResourceManager;

    DeclareClass(Sound, FrameObject);

    String soundName; //great jim!  just great.  a hack.  just how I love things.
    BOOL b3DSound;

public:
    virtual ~Sound() {}

    virtual void  Play(BOOL bLoop)=0;
    virtual void  Stop()=0;

    virtual void  SetVolume(float vol)=0;
    virtual float GetVolume()=0;

    virtual void  SetPosition(const Vect &pos)=0;
    virtual void  SetVelocity(const Vect &vel)=0;

    virtual void  SetRange(float soundRange)=0;
    virtual float GetRange()=0;

    virtual void  SetPitch(float pitchAdjust)=0; //-1.0 to 1.0
    virtual float GetPitch()=0;                  //-1.0 = one octave lower
                                                 // 0.0 = normal pitch
                                                 // 1.0 = one octave higher

    String GetResourceName() {return soundName;}

    BOOL Is3DSound() {return b3DSound;}

    //<Script module="Base" classdecs="Sound">
    Declare_Internal_Member(native_Play);
    Declare_Internal_Member(native_Stop);
    Declare_Internal_Member(native_SetVolume);
    Declare_Internal_Member(native_GetVolume);
    Declare_Internal_Member(native_SetPosition);
    Declare_Internal_Member(native_SetVelocity);
    Declare_Internal_Member(native_SetRange);
    Declare_Internal_Member(native_GetRange);
    Declare_Internal_Member(native_SetPitch);
    Declare_Internal_Member(native_GetPitch);
    //</Script>
};


/*====================================================================
    Sound System class
=====================================================================*/

class BASE_EXPORT SoundSystem : public FrameObject
{
    DeclareClass(SoundSystem, FrameObject);

protected:
    float effectsVol;

public:
    SoundSystem() : effectsVol(1.0f) {}
    virtual ~SoundSystem() {}

    virtual Sound *CreateSound(LPBYTE lpData, BOOL b3DSound, BOOL bSelfDestruct=FALSE)=0;
    virtual SoundStream *CreateSoundStream(const SoundStreamInfo &ssi)=0;

    virtual void SetListenerPosition(const Vect &pos)=0;
    virtual void SetListenerVelocity(const Vect &vel)=0;
    virtual void SetListenerOriantation(const Quat &rot)=0;
    virtual void UpdatePositionalData()=0;

    virtual void SetEffectsVol(float vol)=0;

    inline float GetEffectsVol()   {return effectsVol;}

    virtual Sound*  CreateSoundFromFile(TSTR lpFile, BOOL b3DSound);

    //<Script module="Base" classdecs="SoundSystem">
    Declare_Internal_Member(native_SetEffectsVol);
    Declare_Internal_Member(native_GetEffectsVol);
    //</Script>
};

//<Script module="Base" globaldecs="SoundSystem.xscript">
Declare_Native_Global(NativeGlobal_SS);
//</Script>

BASE_EXPORT extern SoundSystem *SS;


#endif

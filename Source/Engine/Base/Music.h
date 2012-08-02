/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Music.h:  Music class

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


#ifndef MUSIC_HEADER
#define MUSIC_HEADER


enum MusicTransition
{
    MusicTransition_Instant,
    MusicTransition_FastFade,
    MusicTransition_Fade,
    MusicTransition_SlowFade
};

class BASE_EXPORT MusicManager : public FrameObject
{
    DeclareClass(MusicManager, FrameObject);

    float curVol;

    XFile *oggIntroFile;
    XFile *oggLoopFile;
    HANDLE vorbisIntroData;  //OggVorbis_File *
    HANDLE vorbisLoopData;   //OggVorbis_File *

    HANDLE curVorbisData;

    String nextIntroFile, nextLoopFile;
    BOOL bHasIntro;
    float nextPlayPos;
    double introLength;

    float transitionSpeed, transitionTime;

    SoundStream *stream;
    LPBYTE overBuffer;
    DWORD overBufferSize;

    BOOL bFadingIn;

    void PlayForRealPlz();

    static DWORD ENGINEAPI NextMusicSegment(SoundStream *stream, MusicManager *music);

public:
    MusicManager();
    ~MusicManager();

    void Destroy();

    void Tick(float fSeconds);

    BOOL Play(CTSTR lpFileIntro, CTSTR lpFileLoop, float playPos=0.0f, MusicTransition transitionType=MusicTransition_Instant);
    void Stop();

    float GetCurrentPos();

    void SetVolume(float volume);
    inline float GetVolume() const {return curVol;}

    //<Script module="Base" classdecs="MusicManager">
    Declare_Internal_Member(native_Play);
    Declare_Internal_Member(native_Stop);
    Declare_Internal_Member(native_GetCurrentPos);
    Declare_Internal_Member(native_SetVolume);
    Declare_Internal_Member(native_GetVolume);
    //</Script>
};

//<Script module="Base" globaldecs="MusicManager.xscript">
Declare_Native_Global(NativeGlobal_MM);
//</Script>


BASE_EXPORT extern MusicManager *MM;



#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Controls

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


#include "..\Base.h"

//<Script module="Base" filedefs="SoundSystem.xscript">
void Sound::native_Play(CallStruct &cs)
{
    BOOL bLoop = (BOOL)cs.GetInt(0);

    Play(bLoop);
}

void Sound::native_Stop(CallStruct &cs)
{
    Stop();
}

void Sound::native_SetVolume(CallStruct &cs)
{
    float vol = cs.GetFloat(0);

    SetVolume(vol);
}

void Sound::native_GetVolume(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetVolume();
}

void Sound::native_SetPosition(CallStruct &cs)
{
    const Vect &pos = (const Vect&)cs.GetStruct(0);

    SetPosition(pos);
}

void Sound::native_SetVelocity(CallStruct &cs)
{
    const Vect &vel = (const Vect&)cs.GetStruct(0);

    SetVelocity(vel);
}

void Sound::native_SetRange(CallStruct &cs)
{
    float soundRange = cs.GetFloat(0);

    SetRange(soundRange);
}

void Sound::native_GetRange(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetRange();
}

void Sound::native_SetPitch(CallStruct &cs)
{
    float pitchAdjust = cs.GetFloat(0);

    SetPitch(pitchAdjust);
}

void Sound::native_GetPitch(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetPitch();
}

void SoundSystem::native_SetEffectsVol(CallStruct &cs)
{
    float vol = cs.GetFloat(0);

    SetEffectsVol(vol);
}

void SoundSystem::native_GetEffectsVol(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetEffectsVol();
}

void ENGINEAPI NativeGlobal_SS(CallStruct &cs)
{
    SoundSystem*& returnVal = (SoundSystem*&)cs.GetObjectOut(RETURNVAL);

    returnVal = SS;
}
//</Script>

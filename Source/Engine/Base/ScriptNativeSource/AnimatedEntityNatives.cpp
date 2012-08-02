/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  AnimatedEntity

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

//<Script module="Base" filedefs="AnimatedEntity.xscript">
void AnimatedEntity::native_PlayAnimation(CallStruct &cs)
{
    int id = cs.GetInt(0);
    AnimationType animType = (AnimationType)cs.GetInt(1);
    float speed = cs.GetFloat(2);
    float transitionSpeed = cs.GetFloat(3);

    PlayAnimation(id, animType, speed, transitionSpeed);
}

void AnimatedEntity::native_PlayAnimation_2(CallStruct &cs)
{
    String seqName = cs.GetString(0);
    AnimationType animType = (AnimationType)cs.GetInt(1);
    float speed = cs.GetFloat(2);
    float transitionSpeed = cs.GetFloat(3);

    PlayAnimation(seqName, animType, speed, transitionSpeed);
}

void AnimatedEntity::native_TransitionAnimation(CallStruct &cs)
{
    String curSeq = cs.GetString(0);
    String nextSeq = cs.GetString(1);
    AnimationType animType = (AnimationType)cs.GetInt(2);
    float speed = cs.GetFloat(3);
    float transitionSpeed = cs.GetFloat(4);

    TransitionAnimation(curSeq, nextSeq, animType, speed, transitionSpeed);
}

void AnimatedEntity::native_SetAnimationSpeed(CallStruct &cs)
{
    int id = cs.GetInt(0);
    float speed = cs.GetFloat(1);

    SetAnimationSpeed(id, speed);
}

void AnimatedEntity::native_SetAnimationSpeed_2(CallStruct &cs)
{
    String seqName = cs.GetString(0);
    float speed = cs.GetFloat(1);

    SetAnimationSpeed(seqName, speed);
}

void AnimatedEntity::native_StopAnimation(CallStruct &cs)
{
    int id = cs.GetInt(0);
    float transitionSpeed = cs.GetFloat(1);

    StopAnimation(id, transitionSpeed);
}

void AnimatedEntity::native_StopAnimation_2(CallStruct &cs)
{
    String seqName = cs.GetString(0);
    float transitionSpeed = cs.GetFloat(1);

    StopAnimation(seqName, transitionSpeed);
}

void AnimatedEntity::native_StopAllAnimation(CallStruct &cs)
{
    StopAllAnimation();
}

void AnimatedEntity::native_IsAnimationPlaying(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    int id = cs.GetInt(0);

    returnVal = IsAnimationPlaying(id);
}

void AnimatedEntity::native_IsAnimationPlaying_2(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String seqName = cs.GetString(0);

    returnVal = IsAnimationPlaying(seqName);
}

void AnimatedEntity::native_NumActiveAnimations(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = NumActiveAnimations();
}

void AnimatedEntity::native_NumBoneExtensions(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = NumBoneExtensions();
}

void AnimatedEntity::native_GetBoneExtension(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String name = cs.GetString(0);
    BoneExtension &boneExt = (BoneExtension&)cs.GetStructOut(1);

    returnVal = GetBoneExtension(name, boneExt);
}

void AnimatedEntity::native_NumBones(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = NumBones();
}

void AnimatedEntity::native_GetBone(CallStruct &cs)
{
    DBone*& returnVal = (DBone*&)cs.GetObjectOut(RETURNVAL);
    int i = cs.GetInt(0);

    returnVal = GetBone(i);
}

void AnimatedEntity::native_GetAnimationID(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    String seqName = cs.GetString(0);

    returnVal = GetAnimationID(seqName);
}

void AnimatedEntity::native_AddCustomAnimation(CallStruct &cs)
{
    const Anim &animData = (const Anim&)cs.GetStruct(0);

    AddCustomAnimation(animData);
}

void AnimatedEntity::native_GetAnimationData(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    int idSequence = cs.GetInt(0);
    Anim &anim = (Anim&)cs.GetStructOut(1);

    returnVal = GetAnimationData(idSequence, anim);
}
//</Script>

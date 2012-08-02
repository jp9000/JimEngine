/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  AnimatedEntity.h:  Animated Entities

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


#ifndef ANIMATEDENTITY_HEADER
#define ANIMATEDENTITY_HEADER


/*=========================================================
    AnimatedEntity
==========================================================*/


//-----------------------------------------
//Deformable Bone (works really nice too)
class BASE_EXPORT DBone : public Entity
{
    friend class AnimatedEntity;
    friend class MeshBrowser;

    DeclareClass(DBone, Entity);

    Bone  *lpBone;

    Vect    ObjPos;
    Quat    ObjRot;
    Matrix  curMatrix;

public:
    DBone();

    void PreRender()            {}
    void Render()               {}
    void PostRender()           {}

    void PreFrame()             {}
    void Tick(float fSeconds)   {}

    virtual BOOL UpdatingPosition() {return FALSE;}
};


//-----------------------------------------
//Animation flags
enum
{
    ANIMATION_HALT              =1,
    ANIMATION_HALTING           =2,
    ANIMATION_TRANSITION        =4,
};


//-----------------------------------------
//Animation type
enum AnimationType {ANIM_PLAY_AND_STOP, ANIM_HALTED, ANIM_PLAY_AND_HALT, ANIM_LOOP};


//-----------------------------------------
//Animation data
struct Anim
{
    UINT          idSequence;
    UINT          nextSequence;
    DWORD         endFrame;
    DWORD         curFrame;
    DWORD         nextFrame;
    DWORD         flags;
    float         speed;
    AnimationType type;

    float         fT;
    float         curFrameTime;
    float         transitionVal;
    float         transitionCurTime;
    float         transitionLength;
};


//-----------------------------------------
//Animatable object
class BASE_EXPORT AnimatedEntity : public MeshEntity
{
    DeclareClass(AnimatedEntity, MeshEntity);

    DWORD nActiveAnimations;

    List<DBone*> BoneList;
    List<DBone*> BoneRootList;

    //animation data
    List<Anim> AnimList;

    //deformable data
    VBData *vbDeform;
    VBData *vbOrigin;

    Vect  animationPos;
    Quat  animationRot;

    void  RotateBone(DBone *bone);
    void  Skin(DBone *bone);
    void  RecreateBounds();

public:
    virtual void Destroy();

    virtual void Tick(float fSeconds);

    virtual void SetMesh(CTSTR lpMesh);

    const SkinMesh* GetSkinMesh() const {return static_cast<const SkinMesh*>(GetMesh());}

    virtual void SetMeshAdjustPos(const Vect &posAdj) {animationPos = posAdj;}
    virtual void SetMeshAdjustRot(const Quat &rotAdj) {animationRot = rotAdj;}

    virtual const Vect& GetMeshAdjustPos() const {return animationPos;}
    virtual const Quat& GetMeshAdjustRot() const {return animationRot;}

    //rendering functions
    virtual void Render();

    void RenderBones();

    virtual void WorldRender();
    virtual void QuickRender();
    virtual void RenderInitialPass();

    //animation functions
    virtual void  PlayAnimation(int id, AnimationType type=ANIM_LOOP, float speed=1.0f, float transitionSpeed=0.2f);
    virtual void  PlayAnimation(CTSTR lpSeqName, AnimationType type=ANIM_LOOP, float speed=1.0f, float transitionSpeed=0.2f);

    virtual void  TransitionAnimation(CTSTR lpCurSeq, CTSTR lpNextSeq, AnimationType type=ANIM_LOOP, float speed=1.0f, float transitionSpeed=0.2f);

    virtual void  SetAnimationSpeed(int id, float speed);
    virtual void  SetAnimationSpeed(CTSTR lpSeqName, float speed);

    virtual void  StopAnimation(int id, float transitionSpeed=0.2f);
    virtual void  StopAnimation(CTSTR lpSeqName, float transitionSpeed=0.2f);

    virtual void  OnStopAnimation(CTSTR lpSeqName) {scriptOnStopAnimation(lpSeqName);}
    virtual void  OnHaltAnimation(CTSTR lpSeqName) {scriptOnHaltAnimation(lpSeqName);}

    virtual void  StopAllAnimation();

    BOOL IsAnimationPlaying(int id);
    BOOL IsAnimationPlaying(CTSTR lpSeqName);

    inline UINT NumActiveAnimations() const {return nActiveAnimations;}

    inline UINT NumBoneExtensions() const {return static_cast<SkinMesh*>(mesh)->BoneExtensions.Num();}
    BOOL GetBoneExtension(CTSTR lpName, BoneExtension &boneExtension) const;

    inline UINT NumBones() const {return BoneList.Num();}
    inline DBone* GetBone(UINT i) {return BoneList[i];}

    inline UINT NumRootBones() const {return BoneRootList.Num();}
    inline DBone* GetRootBone(UINT i) {return BoneRootList[i];}

    UINT GetAnimationID(CTSTR lpSeqName);
    CTSTR GetAnimationName(UINT idSequence);

    void AddCustomAnimation(const Anim& animData);
    BOOL GetAnimationData(int idSequence, Anim &anim);

    //------------------------------------------------
    // script stuff

    //<Script module="Base" classdecs="AnimatedEntity">
    void scriptOnStopAnimation(CTSTR seqName)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetString(0, seqName);

        GetLocalClass()->CallScriptMember(this, 7, cs);
    }

    void scriptOnHaltAnimation(CTSTR seqName)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetString(0, seqName);

        GetLocalClass()->CallScriptMember(this, 8, cs);
    }

    Declare_Internal_Member(native_PlayAnimation);
    Declare_Internal_Member(native_PlayAnimation_2);
    Declare_Internal_Member(native_TransitionAnimation);
    Declare_Internal_Member(native_SetAnimationSpeed);
    Declare_Internal_Member(native_SetAnimationSpeed_2);
    Declare_Internal_Member(native_StopAnimation);
    Declare_Internal_Member(native_StopAnimation_2);
    Declare_Internal_Member(native_StopAllAnimation);
    Declare_Internal_Member(native_IsAnimationPlaying);
    Declare_Internal_Member(native_IsAnimationPlaying_2);
    Declare_Internal_Member(native_NumActiveAnimations);
    Declare_Internal_Member(native_NumBoneExtensions);
    Declare_Internal_Member(native_GetBoneExtension);
    Declare_Internal_Member(native_NumBones);
    Declare_Internal_Member(native_GetBone);
    Declare_Internal_Member(native_GetAnimationID);
    Declare_Internal_Member(native_AddCustomAnimation);
    Declare_Internal_Member(native_GetAnimationData);
    //</Script>
};


#endif


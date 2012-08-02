/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  AnimatedEntity.cpp:  Animated Entities

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


DefineClass(DBone);
DefineClass(AnimatedEntity);



DBone::DBone()
{
    bPositionalOnly = TRUE;
}


void AnimatedEntity::Destroy()
{
    traceIn(AnimatedEntity::Destroy);

    //int i;
    AnimList.Clear();

    /*for(i=0; i<BoneRootList.Num(); i++)
        DestroyObject(BoneRootList[i]);*/

    BoneRootList.Clear();
    BoneList.Clear();

    if(!GS->UseHardwareAnimation())
    {
        if(VertBuffer)
            delete VertBuffer;
        VertBuffer = NULL;
    }

    //Free(FaceNormalList);

    Super::Destroy();

    traceOut;
}

void AnimatedEntity::SetMesh(CTSTR lpMesh)
{
    traceIn(AnimatedEntity::SetMesh);

    MeshEntity::SetMesh(lpMesh);

    if(GetSkinMesh()->bHasAnimation)
    {
        for(DWORD i=0; i<GetSkinMesh()->BoneList.Num(); i++)
        {
            DBone *bone = CreateObject(DBone);
            bone->SetPos(bone->ObjPos = GetSkinMesh()->BoneList[i].Pos);
            bone->SetRot(bone->ObjRot = Quat(0, 0, 0, 1));
            bone->lpBone = GetSkinMesh()->BoneList+i;

            if(GetSkinMesh()->BoneList[i].idParent != 0xFFFFFFFF)
            {
                bone->Attach(BoneList[GetSkinMesh()->BoneList[i].idParent]);
                bone->SetPos(animationPos + (bone->GetLocalPos() - ((DBone*)bone->GetParent())->ObjPos));
            }

            BoneList << bone;
            if(GetSkinMesh()->BoneList[i].flags & BONE_ROOT)
            {
                BoneRootList << bone;
                bone->Attach(this);
            }
        }

        if(!GS->UseHardwareAnimation())
        {
            VertBuffer = CloneVertexBuffer(GetSkinMesh()->VertBuffer, FALSE);
            vbDeform = VertBuffer->GetData();
            vbOrigin = GetSkinMesh()->VertBuffer->GetData();

            VertList = vbDeform->VertList.Array();
        }
    }

    traceOut;
}

void AnimatedEntity::WorldRender()
{
    traceInFast(AnimatedEntity::WorldRender);

    if(!GS->UseHardwareAnimation() || !GetSkinMesh()->bHasAnimation)
    {
        Super::WorldRender();
        return;
    }

    if(!mesh)
        return;

    LoadEffectData();

    //render mesh
    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(mesh->IdxBuffer);

    HANDLE boneTransformsHandle = level->GetBoneTransforms();
    float boneTransforms[16*4];

    for(DWORD i=0;i<GetSkinMesh()->AnimatedSections.Num();i++)
    {
        DrawSection &section        = GetSkinMesh()->SectionList[i];
        AnimSection &animSection    = GetSkinMesh()->AnimatedSections[i];
        Material    *material       = MaterialList[i];

        if(material && material->GetEffect() == GetActiveEffect())
        {
            material->LoadParameters();

            for(int j=0; j<animSection.SubSections.Num(); j++)
            {
                AnimSubSection &subSection = animSection.SubSections[j];

                for(int k=0; k<subSection.numBones; k++)
                    Matrix4x4Convert(boneTransforms+(k*16), BoneList[subSection.bones[k]]->curMatrix);
                GetActiveEffect()->SetValue(boneTransformsHandle, boneTransforms, 64*subSection.numBones);

                Draw(GS_TRIANGLES, 0, subSection.startFace*3, subSection.numFaces*3);
            }
        }
    }

    ResetEffectData();

    traceOutFast;
}

void AnimatedEntity::RenderInitialPass()
{
    traceInFast(AnimatedEntity::RenderInitialPass);

    if(!GS->UseHardwareAnimation() || !GetSkinMesh()->bHasAnimation)
    {
        Super::RenderInitialPass();
        return;
    }

    if(!mesh)
        return;

    LoadEffectData();

    LoadVertexBuffer(VertBuffer);       
    LoadIndexBuffer(mesh->IdxBuffer);

    HANDLE boneTransformsHandle = level->GetBoneTransforms();
    float boneTransforms[16*4];

    for(DWORD i=0;i<mesh->nSections;i++)
    {
        DrawSection &section        = mesh->SectionList[i];
        AnimSection &animSection    = GetSkinMesh()->AnimatedSections[i];
        Material    *material       = MaterialList[i];

        if(material && section.numFaces)
        {
            if(material->GetEffect() == GetActiveEffect())
            {
                material->LoadParameters();

                for(int j=0; j<animSection.SubSections.Num(); j++)
                {
                    AnimSubSection &subSection = animSection.SubSections[j];

                    for(int k=0; k<subSection.numBones; k++)
                        Matrix4x4Convert(boneTransforms+(k*16), BoneList[subSection.bones[k]]->curMatrix);
                    GetActiveEffect()->SetValue(boneTransformsHandle, boneTransforms, 64*subSection.numBones);

                    Draw(GS_TRIANGLES, 0, subSection.startFace*3, subSection.numFaces*3);
                }
            }
        }
    }

    ResetEffectData();

    traceOutFast;
}

void AnimatedEntity::QuickRender()
{
    traceInFast(AnimatedEntity::QuickRender);

    if(!GS->UseHardwareAnimation() || !GetSkinMesh()->bHasAnimation)
    {
        Super::QuickRender();
        return;
    }

    if(!mesh)
        return;

    if(GetActiveEffect())
        LoadEffectData();
    else
    {
        MatrixPush();
        MatrixMultiply(invTransform);
        if(bHasScale) MatrixScale(scale);
    }

    LoadVertexBuffer(VertBuffer);
    LoadIndexBuffer(mesh->IdxBuffer);

    HANDLE boneTransformsHandle = level->GetBoneTransforms();
    float boneTransforms[16*4];

    for(DWORD i=0;i<mesh->nSections;i++)
    {
        DrawSection &section        = mesh->SectionList[i];
        AnimSection &animSection    = GetSkinMesh()->AnimatedSections[i];
        Material    *material       = MaterialList[i];

        if(!material)
            continue;

        if(material->GetEffect() == GetActiveEffect())
            material->LoadParameters();

        for(int j=0; j<animSection.SubSections.Num(); j++)
        {
            AnimSubSection &subSection = animSection.SubSections[j];

            for(int k=0; k<subSection.numBones; k++)
                Matrix4x4Convert(boneTransforms+(k*16), BoneList[subSection.bones[k]]->curMatrix);
            GetActiveEffect()->SetValue(boneTransformsHandle, boneTransforms, 64*subSection.numBones);

            Draw(GS_TRIANGLES, 0, subSection.startFace*3, subSection.numFaces*3);
        }
    }

    if(GetActiveEffect())
        ResetEffectData();
    else
        MatrixPop();

    traceOutFast;
}


void AnimatedEntity::Tick(float fSeconds)
{
    traceIn(AnimatedEntity::Tick);

    Super::Tick(fSeconds);

    //if(InCinematic())
    //    UpdatePositionalData();

    for(DWORD i=0; i<AnimList.Num(); i++)
    {
        Anim &anim = AnimList[i];
        BOOL bContinue = FALSE;

        if(fabs(anim.speed) <= 0.01f)
            continue;

        if(anim.flags & ANIMATION_TRANSITION)
        {
            anim.transitionCurTime += fSeconds;
            anim.transitionVal = anim.transitionCurTime/anim.transitionLength;

            if(anim.transitionCurTime >= anim.transitionLength)
            {
                if(anim.idSequence != INVALID)
                    OnStopAnimation(GetAnimationName(anim.idSequence));
                anim.idSequence = anim.nextSequence;

                fSeconds -= (anim.transitionCurTime-anim.transitionLength);

                if(anim.idSequence == INVALID)
                {
                    AnimList.Remove(i--);
                    if(!AnimList.Num())
                    {
                        bounds = mesh->bounds;
                        ResetScale();

                        if(!GS->UseHardwareAnimation())
                        {
                            mcpy(VertList, vbOrigin->VertList.Array(), sizeof(Vect)*GetSkinMesh()->nVerts);
                            mcpy(vbDeform->NormalList.Array(), vbOrigin->NormalList.Array(), sizeof(Vect)*GetSkinMesh()->nVerts);
                            mcpy(vbDeform->TangentList.Array(), vbOrigin->TangentList.Array(), sizeof(Vect)*GetSkinMesh()->nVerts);
                            VertBuffer->FlushBuffers();
                        }
                    }
                    continue;
                }
                else
                {
                    anim.flags          &= ~ANIMATION_TRANSITION;
                    anim.endFrame        = GetSkinMesh()->SequenceList[anim.idSequence].nFrames-1;
                    anim.curFrame        = 0;
                    anim.curFrameTime    = 0.0f;
                    anim.fT              = 0.0f;
                    if(!anim.endFrame)
                    {
                        anim.nextFrame    = 0;
                        anim.flags       |= ANIMATION_HALT;
                    }
                    else
                        anim.nextFrame  = 1;
                }
            }
            else
                continue;
        }

        if(anim.flags & ANIMATION_HALT)
            continue;

        BOOL bReverse = anim.speed < 0.0f;
        float speed = fabs(anim.speed);

        float fKeyframeTime = GetSkinMesh()->SequenceList[anim.idSequence].fKeyframeTime;

        if(bReverse)
        {
            anim.curFrameTime -= fSeconds*speed;

            while(anim.curFrameTime < 0.0f)
            {
                anim.curFrameTime += fKeyframeTime;

                if(anim.curFrame == 1)
                {
                    if(anim.type == ANIM_PLAY_AND_HALT)
                    {
                        anim.fT = 0.0f;
                        anim.flags |= ANIMATION_HALTING;
                        anim.nextFrame = anim.curFrame;
                        bContinue = TRUE;
                        break;
                    }
                }

                if(anim.curFrame == 0)
                {
                    if(anim.type == ANIM_PLAY_AND_STOP)
                    {
                        if(anim.transitionLength != 0.0f)
                        {
                            anim.transitionCurTime = 0.0f;
                            anim.nextSequence = INVALID;
                            anim.flags |= ANIMATION_TRANSITION;
                        }
                        else
                        {
                            OnStopAnimation(GetAnimationName(anim.idSequence));
                            AnimList.Remove(i--);
                        }

                        bContinue = TRUE;
                        break;
                    }
                    else if(anim.type == ANIM_LOOP)
                        anim.curFrame = anim.endFrame;
                }
                else
                    anim.curFrame = anim.curFrame-1;

                anim.nextFrame = (anim.nextFrame == 0) ? anim.endFrame : anim.nextFrame-1;
            }

            if(bContinue)
                continue;

            anim.fT = anim.curFrameTime/fKeyframeTime;

            if(anim.fT < 0.0f)
                anim.fT = 0.0f;
        }
        else
        {
            anim.curFrameTime += fSeconds*speed;

            while(anim.curFrameTime >= fKeyframeTime)
            {
                anim.curFrameTime -= fKeyframeTime;

                if(anim.curFrame == (anim.endFrame-1))
                {
                    if(anim.type == ANIM_PLAY_AND_HALT)
                    {
                        anim.fT = 0.0f;
                        anim.flags |= ANIMATION_HALTING;
                        anim.nextFrame = anim.curFrame;
                        bContinue = TRUE;
                        break;
                    }
                }

                if(anim.curFrame == anim.endFrame)
                {
                    if(anim.type == ANIM_PLAY_AND_STOP)
                    {
                        if(anim.transitionLength != 0.0f)
                        {
                            anim.transitionCurTime = 0.0f;
                            anim.nextSequence = INVALID;
                            anim.flags |= ANIMATION_TRANSITION;
                        }
                        else
                        {
                            OnStopAnimation(GetAnimationName(anim.idSequence));
                            AnimList.Remove(i--);
                        }

                        bContinue = TRUE;
                        break;
                    }
                    else if(anim.type == ANIM_LOOP)
                        anim.curFrame = 0;
                }
                else
                    anim.curFrame = anim.curFrame+1;

                anim.nextFrame = (anim.nextFrame == anim.endFrame) ? 0 : anim.nextFrame+1;
            }

            if(bContinue)
                continue;

            anim.fT = anim.curFrameTime/fKeyframeTime;

            if(anim.fT > 1.0f)
                anim.fT = 1.0f;
        }
    }

    if(AnimList.Num())
    {
        DWORD i;

        nActiveAnimations = 0;

        for(i=0; i<AnimList.Num(); i++)
        {
            Anim &anim = AnimList[i];
            if(!(anim.flags & ANIMATION_HALT))
                ++nActiveAnimations;

            if(anim.flags & ANIMATION_HALTING)
            {
                anim.flags = (anim.flags&~ANIMATION_HALTING)|ANIMATION_HALT;
                OnHaltAnimation(GetAnimationName(anim.idSequence));
            }
        }

        if(!nActiveAnimations)
            return;

        UpdatePositionalData();

        if(AnimList.Num()) //no animations removed
        {
            for(i=0; i<BoneRootList.Num(); i++)
            {
                BoneRootList[i]->SetPos(BoneRootList[i]->lpBone->Pos + animationPos);
                RotateBone(BoneRootList[i]);
            }

            if(!GS->UseHardwareAnimation())
            {
                Vect *NormalList = vbDeform->NormalList.Array();

                //mcpy(VertList, vbOrigin->VertList.Array(), sizeof(Vect)*GetSkinMesh()->nVerts);
                zero(VertList, sizeof(Vect)*GetSkinMesh()->nVerts);
                zero(NormalList, sizeof(Vect)*GetSkinMesh()->nVerts);
                zero(vbDeform->TangentList.Array(), sizeof(Vect)*GetSkinMesh()->nVerts);

                for(i=0; i<BoneRootList.Num(); i++)
                    Skin(BoneRootList[i]);

                VertBuffer->FlushBuffers();

                //prevBounds = bounds;
            }
            RecreateBounds();
        }
    }

    traceOut;
}

//quickly jim!
void AnimatedEntity::RenderBones()
{
    traceIn(AnimatedEntity::RenderBones);

    Vect pos;
    const Vect *parent;
    DWORD nBones=GetSkinMesh()->BoneList.Num(),i;

    EnableDepthTest(FALSE);
    for(i=0;i<nBones;i++)
    {
        RenderStart();
            pos = BoneList[i]->GetWorldPos();
            //pos.x += 8;
            if(!(BoneList[i]->lpBone->flags & BONE_ROOT))
            {
                Color(1.0f, 1.0f, 1.0f, 1.0f);
                parent = &reinterpret_cast<DBone*>(BoneList[i]->GetParent())->GetWorldPos();
                Vertex(pos.x,   pos.y,   pos.z);
                Color(1.0f, 1.0f, 1.0f, 1.0f);
                Vertex(parent->x/*+8*/, parent->y, parent->z);
            }
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x,   pos.y,   pos.z);
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x,   pos.y,   pos.z+0.5);
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x,   pos.y,   pos.z);
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x,   pos.y,   pos.z-0.5);
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x,   pos.y,   pos.z);
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x+0.5, pos.y,   pos.z);
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x,   pos.y,   pos.z);
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x,   pos.y+0.5, pos.z);
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x,   pos.y,   pos.z);
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x-0.5, pos.y,   pos.z);
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x,   pos.y,   pos.z);
            Color(0.0f, 1.0f, 0.0f, 1.0f);
            Vertex(pos.x,   pos.y-0.5, pos.z);
        RenderStop(GS_LINES);
    }
    EnableDepthTest(TRUE);

    traceOut;
}

void AnimatedEntity::Render()
{
    //RenderBones();
    Super::Render();
}

void AnimatedEntity::PlayAnimation(int id, AnimationType type, float speed, float transitionSpeed)
{
    traceIn(AnimatedEntity::PlayAnimation);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::PlayAnimation - Invalid mesh")); return;}

    for(DWORD i=0;i<AnimList.Num();i++)
    {
        Anim &anim = AnimList[i];
        if(anim.idSequence == id)
        {
            if(anim.flags & ANIMATION_TRANSITION)
            {
                anim.idSequence = anim.nextSequence;
                anim.nextSequence = id;
                anim.transitionVal = 1.0f-anim.transitionVal;
                anim.transitionCurTime = anim.transitionVal*transitionSpeed;
                anim.transitionLength = transitionSpeed;
            }
            return;
        }
    }

    for(DWORD i=0;i<AnimList.Num();i++)
    {
        Anim &anim = AnimList[i];
        if((anim.flags & ANIMATION_TRANSITION) && (anim.nextSequence == id))
            return;
    }

    Anim newanim;

    zero(&newanim, sizeof(newanim));

    newanim.endFrame          = GetSkinMesh()->SequenceList[id].nFrames-1;
    newanim.curFrame          = 0;
    newanim.curFrameTime      = 0.0f;
    newanim.fT                = 0.0f;
    newanim.speed             = speed;
    newanim.type              = type;

    if(transitionSpeed != 0.0f)
    {
        newanim.idSequence = -1;
        newanim.nextSequence = id;
        newanim.flags = ANIMATION_TRANSITION;
        newanim.transitionCurTime = 0.0f;
        newanim.transitionLength  = transitionSpeed;
    }
    else
        newanim.idSequence = id;

    if(!newanim.endFrame)
    {
        newanim.nextFrame     = 0;
        newanim.flags        |= ANIMATION_HALT;
    }
    else
        newanim.nextFrame     = 1;

    AnimList << newanim;

    traceOut;
}

void AnimatedEntity::TransitionAnimation(CTSTR lpCurSeq, CTSTR lpNextSeq, AnimationType type, float speed, float transitionSpeed)
{
    traceIn(AnimatedEntity::TransitionAnimation);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::TransitionAnimation - Invalid mesh")); return;}

    UINT id = GetAnimationID(lpCurSeq), id2 = GetAnimationID(lpNextSeq);
    if(id == INVALID)
    {
        AppWarning(TEXT("Could not find sequence name '%s'"), lpCurSeq);
        if(id2 != INVALID) return;
    }
    if(id2 == INVALID)
    {
        AppWarning(TEXT("Could not find sequence name '%s'"), lpNextSeq);
        return;
    }

    for(DWORD i=0;i<AnimList.Num();i++)
    {
        Anim &anim = AnimList[i];
        if(anim.idSequence == id)
        {
            anim.flags |= ANIMATION_TRANSITION;
            anim.nextSequence = id2;
            anim.transitionCurTime = 0.0f;
            anim.transitionLength = transitionSpeed;
            anim.speed = speed;
            anim.type = type;
            return;
        }
    }

    AppWarning(TEXT("Sequence '%s' is not currently playing"), lpCurSeq);

    traceOut;
}

void AnimatedEntity::SetAnimationSpeed(int id, float speed)
{
    traceIn(AnimatedEntity::SetAnimationSpeed);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::SetAnimationSpeed - Invalid mesh")); return;}

    for(DWORD i=0;i<AnimList.Num();i++)
    {
        if(AnimList[i].idSequence == id)
        {
            AnimList[i].speed = speed;
            return;
        }
    }
    AppWarning(TEXT("Sequence '%s' is not currently playing"), GetSkinMesh()->SequenceList[id].strName.Array());

    traceOut;
}

void AnimatedEntity::StopAnimation(int id, float transitionSpeed)
{
    traceIn(AnimatedEntity::StopAnimation);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::StopAnimation - Invalid mesh")); return;}

    if(transitionSpeed == 0.0f)
    {
        for(DWORD i=0;i<AnimList.Num();i++)
        {
            Anim &anim = AnimList[i];

            if(anim.idSequence == id)
            {
                AnimList.Remove(i);
                if(!AnimList.Num())
                {
                    bounds = mesh->bounds;
                    ResetScale();

                    if(!GS->UseHardwareAnimation())
                    {
                        mcpy(VertList, vbOrigin->VertList.Array(), sizeof(Vect)*GetSkinMesh()->nVerts);
                        mcpy(vbDeform->NormalList.Array(), vbOrigin->NormalList.Array(), sizeof(Vect)*GetSkinMesh()->nVerts);
                        mcpy(vbDeform->TangentList.Array(), vbOrigin->TangentList.Array(), sizeof(Vect)*GetSkinMesh()->nVerts);
                        VertBuffer->FlushBuffers();
                    }

                    OnStopAnimation(GetAnimationName(id));
                }

                return;
            }
        }
    }
    else
    {
        for(DWORD i=0;i<AnimList.Num();i++)
        {
            Anim &anim = AnimList[i];

            if((anim.nextSequence == id) && (anim.flags & ANIMATION_TRANSITION))
            {
                anim.nextSequence = anim.idSequence;
                anim.idSequence = id;
                anim.transitionVal = 1.0f-anim.transitionVal;
                anim.transitionCurTime = anim.transitionVal*transitionSpeed;
                anim.transitionLength = transitionSpeed;
                return;
            }
        }

        for(DWORD i=0;i<AnimList.Num();i++)
        {
            Anim &anim = AnimList[i];

            if(anim.idSequence == id)
            {
                if(anim.flags & ANIMATION_TRANSITION)
                    return;

                anim.transitionLength = transitionSpeed;
                anim.transitionCurTime = 0.0f;
                anim.flags |= ANIMATION_TRANSITION;
                anim.nextSequence = -1;
                return;
            }
        }
    }

    AppWarning(TEXT("Could not stop sequence '%s' because it is not currently playing"), GetSkinMesh()->SequenceList[id].strName.Array());

    traceOut;
}

void AnimatedEntity::PlayAnimation(CTSTR lpSeqName, AnimationType type, float speed, float transitionSpeed)
{
    traceIn(AnimatedEntity::PlayAnimation);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::PlayAnimation - Invalid mesh")); return;}

    UINT id = GetAnimationID(lpSeqName);
    if(id != INVALID)
        PlayAnimation(id, type, speed, transitionSpeed);
    else
        AppWarning(TEXT("Could not find sequence name '%s'"), lpSeqName);

    traceOut;
}

void AnimatedEntity::SetAnimationSpeed(CTSTR lpSeqName, float speed)
{
    traceIn(AnimatedEntity::SetAnimationSpeed);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::SetAnimationSpeed - Invalid mesh")); return;}

    UINT id = GetAnimationID(lpSeqName);
    if(id != INVALID)
        SetAnimationSpeed(id, speed);
    else
        AppWarning(TEXT("could not find sequence name '%s'"), lpSeqName);

    traceOut;
}

BOOL AnimatedEntity::IsAnimationPlaying(int id)
{
    traceIn(AnimatedEntity::IsAnimationPlaying);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::IsAnimationPlaying - Invalid mesh")); return FALSE;}

    for(int i=0; i<AnimList.Num(); i++)
    {
        Anim &anim = AnimList[i];
        if( (anim.idSequence == id) ||
            ( (anim.flags & ANIMATION_TRANSITION) && (anim.nextSequence == id) )
          )
        {
            return TRUE;
        }
    }

    return FALSE;

    traceOut;
}

BOOL AnimatedEntity::IsAnimationPlaying(CTSTR lpSeqName)
{
    traceIn(AnimatedEntity::IsAnimationPlaying);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::IsAnimationPlaying - Invalid mesh")); return FALSE;}

    int i;

    for(i=0; i<AnimList.Num(); i++)
    {
        if(scmpi(GetSkinMesh()->SequenceList[AnimList[i].idSequence].strName, lpSeqName) == 0)
            return 1;
    }

    return 0;

    traceOut;
}

void AnimatedEntity::StopAnimation(CTSTR lpSeqName, float transitionSpeed)
{
    traceIn(AnimatedEntity::StopAnimation);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::StopAnimation - Invalid mesh")); return;}

    UINT id = GetAnimationID(lpSeqName);
    if(id != INVALID)
        StopAnimation(id, transitionSpeed);
    else
        AppWarning(TEXT("could not find sequence name '%s'"), lpSeqName);

    traceOut;
}

void AnimatedEntity::StopAllAnimation()
{
    traceIn(AnimatedEntity::StopAllAnimation);

    //DWORD i;

    AnimList.Clear();
    bounds = mesh->bounds;
    ResetScale();

    if(!GS->UseHardwareAnimation())
    {
        mcpy(VertList, vbOrigin->VertList.Array(), sizeof(Vect)*GetSkinMesh()->nVerts);
        mcpy(vbDeform->NormalList.Array(), vbOrigin->NormalList.Array(), sizeof(Vect)*GetSkinMesh()->nVerts);
        mcpy(vbDeform->TangentList.Array(), vbOrigin->TangentList.Array(), sizeof(Vect)*GetSkinMesh()->nVerts);
        VertBuffer->FlushBuffers();
    }

    /*for(i=0; i<mesh->nFaces; i++)
    {
        Face &face = mesh->FaceList[i];
        Vect NewFaceNormal((VertList[face.B] - VertList[face.A]) ^ (VertList[face.C] - VertList[face.A]));

        NewFaceNormal.Norm();
        FaceNormalList[i] = NewFaceNormal;
    }*/

    traceOut;
}

Quat identQuat = Quat(0.0f, 0.0f, 0.0f, 1.0f);

void AnimatedEntity::RotateBone(DBone *bone)
{
    traceInFast(AnimatedEntity::RotateBone);

    //static VWeight *lpWeight;
    static Vect    vTransformed;
    static Quat    curRot,newLocalRot;
    static Vect    curPos, startPos;
    Bone    *boneDef;
    DWORD i;

    boneDef = bone->lpBone; //get default bone info so we don't have to get it constantly
    curRot.SetIdentity();
    startPos = curPos = (boneDef->flags & BONE_ROOT) ? (boneDef->Pos+animationPos) : bone->ObjPos;

    newLocalRot.SetIdentity();

    for(i=0;i<AnimList.Num();i++)
    {
        Anim &anim = AnimList[i];

        BOOL bTransitioning = anim.flags & ANIMATION_TRANSITION;
        BOOL bTransitioningIn = (anim.idSequence == INVALID);

        UINT sequence = bTransitioningIn ? anim.nextSequence : anim.idSequence;
        SeqKeys *lpKeys = &bone->lpBone->seqKeys[sequence];

        if(lpKeys->hasRotKeys)
        {
            Quat curKey;

            curKey = CubicInterpolateQuat(lpKeys->lpRotKeys[anim.curFrame],
                lpKeys->lpRotKeys[anim.nextFrame],
                lpKeys->lpRotTans[anim.curFrame],
                lpKeys->lpRotTans[anim.nextFrame],
                anim.fT);

            /*curKey = InterpolateQuat(lpKeys->lpRotKeys[anim.curFrame],
                lpKeys->lpRotKeys[anim.nextFrame],
                anim.fT);*/

            if(bTransitioning)
            {
                if(bTransitioningIn)
                    curKey = InterpolateQuat(Quat::Identity(), curKey, anim.transitionVal);
                else
                {
                    Quat nextKey = Quat::Identity();
                    if(anim.nextSequence != INVALID)
                    {
                        SeqKeys *lpTransitionKeys = &bone->lpBone->seqKeys[anim.nextSequence];
                        if(lpTransitionKeys->hasRotKeys)
                            nextKey = lpTransitionKeys->lpRotKeys[0];
                    }
                    curKey = InterpolateQuat(curKey, nextKey, anim.transitionVal);
                }
            }

            if(curRot == identQuat)
                curRot = curKey;
            else
                curRot *= curKey;
        }
        if(!InCinematic() && lpKeys->hasPosKeys)
        {
            Vect curKey;

            curKey = GetHSpline(lpKeys->lpPosKeys[anim.curFrame],
                lpKeys->lpPosKeys[anim.nextFrame],
                lpKeys->lpPosTans[anim.curFrame],
                lpKeys->lpPosTans[anim.nextFrame],
                anim.fT);

            /*curKey = Lerp<Vect>(lpKeys->lpPosKeys[anim.curFrame],
                lpKeys->lpPosKeys[anim.nextFrame],
                anim.fT);*/

            if(bTransitioning)
            {
                if(bTransitioningIn)
                    curKey = curKey*anim.transitionVal;
                else
                {
                    Vect nextKey = Vect::Zero();
                    if(anim.nextSequence != INVALID)
                    {
                        SeqKeys *lpTransitionKeys = &bone->lpBone->seqKeys[anim.nextSequence];
                        if(lpTransitionKeys->hasPosKeys)
                            nextKey = lpTransitionKeys->lpPosKeys[0];
                    }
                    curKey = Lerp<Vect>(curKey, nextKey, anim.transitionVal);
                }
            }

            curPos += curKey;
        }
    }

    newLocalRot = -curRot;

    if(curRot == identQuat)
        curRot = bone->ObjRot;
    else
        curRot *= bone->ObjRot;

    Matrix m;

    m.SetIdentity();
    m.Translate(curPos);
    m.Rotate(-curRot);

    bone->curMatrix = m;
    bone->curMatrix.Translate(-boneDef->Pos);
    bone->curMatrix.Transpose();

    for(i=0;i<bone->NumChildren();i++)
    {
        if(bone->GetChild(i)->IsOf(GetClass(DBone)))
        {
            DBone *child = (DBone*)bone->GetChild(i);
            child->ObjPos = child->lpBone->LocalPos;
            child->ObjPos.TransformPoint(m);
            child->ObjRot = curRot;
        }
    }

    bone->SetPos(curPos);
    bone->SetRot(newLocalRot);
    if(boneDef->flags & BONE_ROOT)
        bone->ObjPos = curPos;

    //kept seperate from above so that we don't have to allocate massive stack per call
    for(i=0;i<bone->NumChildren();i++)
    {
        Entity *child = bone->GetChild(i);
        if(child->IsOf(GetClass(DBone)))
            RotateBone((DBone*)child);
    }

    traceOutFast;
}

void AnimatedEntity::Skin(DBone *bone)
{
    traceInFast(AnimatedEntity::Skin);

    //static VWeight *lpWeight;
    static Vect    vTransformed;
    static Quat    curRot;
    static Vect    curPos;
    static Matrix  m;
    Bone    *boneDef;
    DWORD i;

    boneDef = bone->lpBone; //get default bone info so we don't have to get it constantly
    curRot  = -bone->GetLocalRot() * bone->ObjRot;
    curPos  = bone->ObjPos;

    m.SetIdentity();
    m.Translate(curPos);
    m.Rotate(-curRot);
    m.Translate(-boneDef->Pos);

    for(i=0;i<boneDef->Weights.Num();i++)
    {
        DWORD  vert  = boneDef->Weights[i].vert;
        float weight = boneDef->Weights[i].weight;

        //Transform the normal of the vertex
        vTransformed = vbOrigin->NormalList[vert];
        vTransformed.TransformVector(m);
        vbDeform->NormalList[vert] += (vTransformed * weight);

        //Transform the tangent U normal of the vertex
        vTransformed = vbOrigin->TangentList[vert];
        vTransformed.TransformVector(m);
        vbDeform->TangentList[vert] += (vTransformed * weight);

        //Transform the vertex
        vTransformed = vbOrigin->VertList[vert];
        vTransformed.TransformPoint(m);
        vbDeform->VertList[vert] += (vTransformed * weight);
    }

    for(i=0;i<bone->NumChildren();i++)
    {
        Entity *child = bone->GetChild(i);
        if(child->IsOf(GetClass(DBone)))
            Skin((DBone*)child);
    }

    traceOutFast;
}

void  AnimatedEntity::RecreateBounds()
{
    traceIn(AnimatedEntity::RecreateBounds);

    bounds.Min = M_INFINITE;
    bounds.Max = -M_INFINITE;

    for(int i=0; i<BoneList.Num(); i++)
    {
        Vect &pos = BoneList[i]->ObjPos;

        bounds.Max.ClampMin(pos);
        bounds.Min.ClampMax(pos);
    }

    bounds.Min -= 1.0f;
    bounds.Max += 1.0f;

    bounds.Min *= scale;
    bounds.Max *= scale;

    traceOut;
}

BOOL AnimatedEntity::GetBoneExtension(CTSTR lpName, BoneExtension &boneExtension) const
{
    traceIn(AnimatedEntity::GetBoneExtension);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::GetBoneExtension - Invalid mesh")); return FALSE;}

    for(UINT id=0; id<GetSkinMesh()->BoneExtensions.Num(); id++)
    {
        String &name = GetSkinMesh()->BoneExtensionNames[id];

        if(name.CompareI(lpName))
        {
            mcpy(&boneExtension, GetSkinMesh()->BoneExtensions+id, sizeof(BoneExtension));
            return TRUE;
        }
    }

    return FALSE;

    traceOut;
}


UINT AnimatedEntity::GetAnimationID(CTSTR lpSeqName)
{
    traceIn(AnimatedEntity::GetAnimationID);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::GetAnimationID - Invalid mesh")); return INVALID;}

    for(UINT id=0; id<GetSkinMesh()->SequenceList.Num(); id++)
    {
        if(scmpi(GetSkinMesh()->SequenceList[id].strName, lpSeqName) == 0)
            return id;
    }

    return INVALID;

    traceOut;
}

CTSTR AnimatedEntity::GetAnimationName(UINT idSequence)
{
    traceIn(AnimatedEntity::GetAnimationID);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::GetAnimationName - Invalid mesh")); return NULL;}

    if(idSequence < GetSkinMesh()->SequenceList.Num())
        return GetSkinMesh()->SequenceList[idSequence].strName.Array();

    return NULL;

    traceOut;
}

void AnimatedEntity::AddCustomAnimation(const Anim& animData)
{
    traceIn(AnimatedEntity::AddCustomAnimation);

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::AddCustomAnimation - Invalid mesh")); return;}

    AnimList << animData;

    traceOut;
}

BOOL AnimatedEntity::GetAnimationData(int idSequence, Anim& anim)
{
    traceIn(AnimatedEntity::GetAnimationData);

    int i;

    if(!mesh) {AppWarning(TEXT("AnimatedEntity::GetAnimationData - Invalid mesh")); return FALSE;}

    for(i=0; i<AnimList.Num(); i++)
    {
        if(AnimList[i].idSequence = idSequence)
        {
            mcpy(&anim, &AnimList[i], sizeof(anim));
            return TRUE;
        }
    }

    return FALSE;

    traceOut;
}

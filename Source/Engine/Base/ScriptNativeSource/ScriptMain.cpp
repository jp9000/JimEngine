/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ScriptMain

  Copyright (c) 2009, Hugh Bailey
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
#include "GlobalNatives.h"


void ScriptSystem::LoadBaseScriptingDefinitions()
{
    //<Script module="Base" nativeloader>
    Class* curClass;

    curClass = GetClass(AnimatedEntity);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_PlayAnimation, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_PlayAnimation_2, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_TransitionAnimation, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_SetAnimationSpeed, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_SetAnimationSpeed_2, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_StopAnimation, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_StopAnimation_2, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_StopAllAnimation, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_IsAnimationPlaying, 0xA);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_IsAnimationPlaying_2, 0xB);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_NumActiveAnimations, 0xC);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_NumBoneExtensions, 0xD);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_GetBoneExtension, 0xE);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_NumBones, 0xF);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_GetBone, 0x10);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_GetAnimationID, 0x11);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_AddCustomAnimation, 0x12);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AnimatedEntity::native_GetAnimationData, 0x13);
    }

    curClass = GetClass(Button);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(Button, bDisabled), 0);
        curClass->DefineNativeVariable(offsetof(Button, upSound), 1);
        curClass->DefineNativeVariable(offsetof(Button, downSound), 2);
        curClass->DefineNativeVariable(offsetof(Button, overSound), 3);
    }

    curClass = GetClass(TextureButton);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(TextureButton, upTex), 0);
        curClass->DefineNativeVariable(offsetof(TextureButton, downTex), 1);
        curClass->DefineNativeVariable(offsetof(TextureButton, overTex), 2);
        curClass->DefineNativeVariable(offsetof(TextureButton, disabledTex), 3);
    }

    curClass = GetClass(PushButton);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(PushButton, strText), 0);
        curClass->DefineNativeVariable(offsetof(PushButton, strFont), 1);
        curClass->DefineNativeVariable(offsetof(PushButton, fontColor), 2);
        curClass->DefineNativeVariable(offsetof(PushButton, bgColor), 3);
    }

    curClass = GetClass(CheckBox);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(CheckBox, strText), 0);
        curClass->DefineNativeVariable(offsetof(CheckBox, strFont), 1);
        curClass->DefineNativeVariable(offsetof(CheckBox, fontColor), 2);
        curClass->DefineNativeVariable(offsetof(CheckBox, bgColor), 3);
        curClass->DefineNativeVariable(offsetof(CheckBox, bDisabled), 4);
        curClass->DefineNativeVariable(offsetof(CheckBox, upSound), 5);
        curClass->DefineNativeVariable(offsetof(CheckBox, downSound), 6);
    }

    curClass = GetClass(EditBox);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(EditBox, strFont), 0);
        curClass->DefineNativeVariable(offsetof(EditBox, fontColor), 1);
        curClass->DefineNativeVariable(offsetof(EditBox, bgColor), 2);
        curClass->DefineNativeVariable(offsetof(EditBox, bMultiline), 3);
        curClass->DefineNativeVariable(offsetof(EditBox, bPassword), 4);
        curClass->DefineNativeVariable(offsetof(EditBox, bDisabled), 5);
        curClass->DefineNativeVariable(offsetof(EditBox, strBuffer), 6);
        curClass->DefineNativeVariable(offsetof(EditBox, maxLength), 7);
    }

    curClass = GetClass(ListBox);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(ListBox, strFont), 0);
        curClass->DefineNativeVariable(offsetof(ListBox, fontColor), 1);
        curClass->DefineNativeVariable(offsetof(ListBox, bgColor), 2);

        curClass->DefineNativeMember((OBJECTCALLBACK)&ListBox::native_AddItem, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&ListBox::native_InsertItem, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&ListBox::native_Remove, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&ListBox::native_RemoveItem, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&ListBox::native_GetCurSel, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&ListBox::native_SetCurSel, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&ListBox::native_NumItems, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&ListBox::native_GetItemName, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&ListBox::native_GetItemData, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&ListBox::native_GetItemByName, 0xA);
    }

    curClass = GetClass(SlideInfo);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(SlideInfo, bEndSlide), 0);
    }

    curClass = GetClass(Slider);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(Slider, color), 0);
        curClass->DefineNativeVariable(offsetof(Slider, minValue), 1);
        curClass->DefineNativeVariable(offsetof(Slider, maxValue), 2);
        curClass->DefineNativeVariable(offsetof(Slider, clampValue), 3);
        curClass->DefineNativeVariable(offsetof(Slider, bClampNumber), 4);
        curClass->DefineNativeVariable(offsetof(Slider, moveSound), 5);

        curClass->DefineNativeMember((OBJECTCALLBACK)&Slider::native_GetValue, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Slider::native_SetValue, 0x2);
    }

    curClass = GetClass(StaticText);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(StaticText, textOffset), 0);
        curClass->DefineNativeVariable(offsetof(StaticText, strText), 1);
        curClass->DefineNativeVariable(offsetof(StaticText, strFont), 2);
        curClass->DefineNativeVariable(offsetof(StaticText, fontColor), 3);
    }

    curClass = GetClass(Camera);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Camera::native_SetPerspective, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Camera::native_SetFrustum, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Camera::native_SetOrtho, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Camera::native_GetAssignedViewport, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Camera::native_SetSoundCamera, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Camera::native_IsPerspective, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Camera::native_IsSoundCamera, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Camera::native_LoadProjectionTransform, 0x7);
    }

    curClass = GetClass(UserCamera);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(UserCamera, fFOV), 0);
        curClass->DefineNativeVariable(offsetof(UserCamera, fAspect), 1);
    }

    curClass = GetClass(Character);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Character::native_GetController, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Character::native_SetController, 0x1);
    }

    curClass = GetClass(Controller);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Controller::native_NumCharacters, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Controller::native_GetCharacter, 0x1);
    }

    curClass = GetClass(Engine);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Engine::native_GetCurFPS, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Engine::native_NumPolys, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Engine::native_NumLights, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Engine::native_InEditor, 0x3);
    }

    curClass = GetClass(Entity);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(Entity, phyShape), 0);
        curClass->DefineNativeVariable(offsetof(Entity, phyObject), 1);
        curClass->DefineNativeVariable(offsetof(Entity, bPositionalOnly), 2);
        curClass->DefineNativeVariable(offsetof(Entity, bAlwaysVisible), 3);
        curClass->DefineNativeVariable(offsetof(Entity, bPlacable), 4);

        curClass->DefineNativeStaticMember((NATIVECALLBACK)&Entity::native_FindByClass, 0x0);
        curClass->DefineNativeStaticMember((NATIVECALLBACK)&Entity::native_FindByName, 0x1);
        curClass->DefineNativeStaticMember((NATIVECALLBACK)&Entity::native_FindByID, 0x2);
        curClass->DefineNativeStaticMember((NATIVECALLBACK)&Entity::native_FindAllByClass, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_SetPos, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_SetRot, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetLocalPos, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetLocalRot, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetWorldPos, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetWorldRot, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetEntityTransform, 0xA);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetEntityInvTransform, 0xB);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_Attach, 0xC);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_Detach, 0xD);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_NumChildren, 0xE);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetChild, 0xF);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetParent, 0x10);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetLineCollision, 0x11);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetRayCollision, 0x12);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetConvexCollision, 0x13);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetName, 0x14);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_DuplicateEntity, 0x15);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_SetAsSavable, 0x16);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_IsSavable, 0x17);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetEntityID, 0x18);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_SetState, 0x19);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_GetState, 0x1A);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Entity::native_UpdatePositionalData, 0x1B);
    }

    curClass = GetClass(Font);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Font::native_LetterWidth, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Font::native_WordWidth, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Font::native_TextWidth, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Font::native_GetFontHeight, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Font::native_DrawLetter, 0x4);
    }

    curClass = GetClass(FrameObject);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(FrameObject, bRenderable), 0);

        curClass->DefineNativeMember((OBJECTCALLBACK)&FrameObject::native_SafeRelease, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&FrameObject::native_SafeDestroy, 0x6);
    }

    curClass = GetClass(Game);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Game::native_SetGameSpeed, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Game::native_GetGameSpeed, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Game::native_Pause, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Game::native_IsPaused, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Game::native_Save, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Game::native_GetMainViewport, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Game::native_SetMainViewport, 0x6);
    }

    curClass = GetClass(Texture);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Texture::native_Width, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Texture::native_Height, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Texture::native_HasAlpha, 0x2);
    }

    curClass = GetClass(CubeTexture);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&CubeTexture::native_Size, 0x0);
    }

    curClass = GetClass(Shader);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_NumParams, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_GetParameter, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_GetParameterByName, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetBool, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetFloat, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetInt, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetVector, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetVector2, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetVector4, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetTexture, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetColor, 0xA);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetColor_2, 0xB);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetColor_3, 0xC);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetColor3, 0xD);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetColor3_2, 0xE);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetColor3_3, 0xF);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetVector4_2, 0x10);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetVector_2, 0x11);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetMatrix, 0x12);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Shader::native_SetMatrixIdentity, 0x13);
    }

    curClass = GetClass(Effect);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetTechnique, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_UsableTechnique, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetPass, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetPassByName, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_BeginTechnique, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_EndTechnique, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_BeginPass, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_BeginPassByName, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_BeginPassByHandle, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_EndPass, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_NumParams, 0xA);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetParameter, 0xB);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetParameterByName, 0xC);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetBool, 0xD);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetFloat, 0xE);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetInt, 0xF);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetVector, 0x10);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetVector2, 0x11);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetVector4, 0x12);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetString, 0x13);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetDefaultBool, 0x14);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetDefaultFloat, 0x15);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetDefaultInt, 0x16);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetDefaultVector, 0x17);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetDefaultVector2, 0x18);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetDefaultVector4, 0x19);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetDefaultTexture, 0x1A);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetBool, 0x1B);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetFloat, 0x1C);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetInt, 0x1D);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetVector, 0x1E);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetVector2, 0x1F);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetVector4, 0x20);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetTexture, 0x21);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetViewProj, 0x22);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetWorld, 0x23);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetScale, 0x24);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetColor, 0x25);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetColor_2, 0x26);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetDefaultColor, 0x27);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetDefaultColor_2, 0x28);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetColor, 0x29);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetColor_2, 0x2A);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetColor_3, 0x2B);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetColor3, 0x2C);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetColor3_2, 0x2D);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetDefaultColor3, 0x2E);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_GetDefaultColor3_2, 0x2F);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetColor3, 0x30);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetColor3_2, 0x31);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetColor3_3, 0x32);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetVector4_2, 0x33);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetVector_2, 0x34);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetMatrix, 0x35);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Effect::native_SetMatrixIdentity, 0x36);
    }

    curClass = GetClass(MeshObject);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshObject::native_MeshObject, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshObject::native_Render, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshObject::native_RenderBare, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshObject::native_SetMaterial, 0x3);
    }

    curClass = GetClass(GraphicsSystem);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_GetSize, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_GetSizeX, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_GetSizeY, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixPush, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixPop, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixSet, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixMultiply, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixRotate, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixRotate_2, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixRotate_3, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixTranslate, 0xA);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixTranslate_2, 0xB);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixTranslate_3, 0xC);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixScale, 0xD);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixScale_2, 0xE);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixTranspose, 0xF);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixInverse, 0x10);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_MatrixIdentity, 0x11);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_SetCurFont, 0x12);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_SetCurFont_2, 0x13);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_GetFont, 0x14);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_SetFontColor, 0x15);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_SetFontColor_2, 0x16);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_DrawText, 0x17);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_DrawTextCenter, 0x18);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_RenderStartNew, 0x19);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_RenderStart, 0x1A);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_RenderStop, 0x1B);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_RenderSave, 0x1C);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Vertex, 0x1D);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Vertex_2, 0x1E);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Normal, 0x1F);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Normal_2, 0x20);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Color, 0x21);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Color_2, 0x22);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_TexCoord, 0x23);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_TexCoord_2, 0x24);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_LoadVertexBuffer, 0x25);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_LoadTexture, 0x26);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_LoadSamplerState, 0x27);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_LoadIndexBuffer, 0x28);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_LoadDefault2DSampler, 0x29);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_LoadDefault3DSampler, 0x2A);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_LoadVertexShader, 0x2B);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_LoadPixelShader, 0x2C);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_SetFrameBufferTarget, 0x2D);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Draw, 0x2E);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_DrawBare, 0x2F);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_ReverseCullMode, 0x30);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_SetCullMode, 0x31);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_GetCullMode, 0x32);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_EnableBlending, 0x33);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_BlendFunction, 0x34);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_ClearDepthBuffer, 0x35);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_EnableDepthTest, 0x36);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_DepthWriteEnable, 0x37);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_DepthFunction, 0x38);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_GetDepthFunction, 0x39);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_ClearColorBuffer, 0x3A);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_ColorWriteEnable, 0x3B);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_ClearStencilBuffer, 0x3C);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_EnableStencilTest, 0x3D);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_StencilWriteEnable, 0x3E);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_StencilFunction, 0x3F);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_StencilOp, 0x40);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_SetPointSize, 0x41);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_ToggleFullScreen, 0x42);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_SetResolution, 0x43);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_AdjustDisplayColors, 0x44);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_GetCurrentDisplayMode, 0x45);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_IsFullscreen, 0x46);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Set2DMode, 0x47);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Set3DMode, 0x48);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Perspective, 0x49);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Ortho, 0x4A);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_Frustum, 0x4B);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_DrawSprite, 0x4C);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_DrawSprite3D, 0x4D);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_DrawSpriteCenter, 0x4E);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_DrawCubeBackdrop, 0x4F);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_DrawCubeBackdrop_2, 0x50);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_DrawSpriteEx, 0x51);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_GetInput, 0x52);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_GetMouseOver, 0x53);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_GetLocalMousePos, 0x54);
        curClass->DefineNativeMember((OBJECTCALLBACK)&GraphicsSystem::native_SetLocalMousePos, 0x55);
    }

    curClass = GetClass(KeyboardInputHandler);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(KeyboardInputHandler, bCharInput), 0);
    }

    curClass = GetClass(Input);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Input::native_PushKBHandler, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Input::native_PushMouseHandler, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Input::native_GetCurKBHandler, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Input::native_GetCurMouseHandler, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Input::native_EmulateMouseInput, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Input::native_EmulateKBInput, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Input::native_GetButtonState, 0x6);
    }

    curClass = GetClass(Level);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Level::native_GetEntityPath, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Level::native_GetCurrentCamera, 0x1);
    }

    curClass = GetClass(Light);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(Light, bCastShadows), 0);
        curClass->DefineNativeVariable(offsetof(Light, color), 1);
        curClass->DefineNativeVariable(offsetof(Light, intensity), 2);
        curClass->DefineNativeVariable(offsetof(Light, bStaticLight), 3);
        curClass->DefineNativeVariable(offsetof(Light, bEnableEmission), 4);
        curClass->DefineNativeVariable(offsetof(Light, lightVolume), 5);

        curClass->DefineNativeMember((OBJECTCALLBACK)&Light::native_SetColor, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Light::native_SetColor_2, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Light::native_SetEnabled, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Light::native_IsEnabled, 0x3);
    }

    curClass = GetClass(PointLight);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(PointLight, lightRange), 0);
    }

    curClass = GetClass(SpotLight);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(SpotLight, lightRange), 0);
        curClass->DefineNativeVariable(offsetof(SpotLight, SpotlightMap), 1);
        curClass->DefineNativeVariable(offsetof(SpotLight, cutoff), 2);
    }

    curClass = GetClass(Material);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_LoadFromFile, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_SetCurrentEffect, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_GetCurrentEffect, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_SetFloat, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_SetColor, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_SetCurrentTexture, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_GetFloat, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_GetColor, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_GetCurrentTexture, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_GetParamID, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_SetFloat_2, 0xA);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_SetColor_2, 0xB);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_SetCurrentTexture_2, 0xC);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_GetFloat_2, 0xD);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_GetColor_2, 0xE);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Material::native_GetCurrentTexture_2, 0xF);
    }

    curClass = GetClass(MeshEntity);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(MeshEntity, bUseLightmapping), 0);
        curClass->DefineNativeVariable(offsetof(MeshEntity, lightmapResolution), 1);
        curClass->DefineNativeVariable(offsetof(MeshEntity, bStaticGeometry), 2);
        curClass->DefineNativeVariable(offsetof(MeshEntity, bCastCompositeShadow), 3);

        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_SetMesh, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_SetMeshAdjustPos, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_SetMeshAdjustRot, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_GetMeshAdjustPos, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_GetMeshAdjustRot, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_SetScale, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_SetScale_2, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_GetMeshBounds, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_GetInitialBounds, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_GetTransform, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_GetInvTransform, 0xA);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_HasValidMesh, 0xB);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MeshEntity::native_SetMaterial, 0xC);
    }

    curClass = GetClass(MusicManager);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&MusicManager::native_Play, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MusicManager::native_Stop, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MusicManager::native_GetCurrentPos, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MusicManager::native_SetVolume, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&MusicManager::native_GetVolume, 0x4);
    }

    curClass = GetClass(Object);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Object::native_AddReference, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Object::native_GetRefCount, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Object::native_Release, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Object::native_IsOf, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Object::native_InitializeObject, 0x6);
    }

    curClass = GetClass(AIPath);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_AdjustedTarget, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_GetTargetPos, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_ResetPath, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_GetCurDist, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_GetTotalDist, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_GetCurPos, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_GetCurNode, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_GetNextNode, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_PathEnded, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_NumNodes, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_GetNodePos, 0xA);
        curClass->DefineNativeMember((OBJECTCALLBACK)&AIPath::native_TraversePath, 0xB);
    }

    curClass = GetClass(PhySphere);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhySphere::native_GetRadius, 0x0);
    }

    curClass = GetClass(PhyBox);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyBox::native_GetHalfExtents, 0x0);
    }

    curClass = GetClass(PhyCylinder);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCylinder::native_GetHalfHeight, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCylinder::native_GetRadius, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCylinder::native_GetAxis, 0x2);
    }

    curClass = GetClass(PhyCapsule);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCapsule::native_GetHalfHeight, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCapsule::native_GetRadius, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCapsule::native_GetAxis, 0x2);
    }

    curClass = GetClass(PhyCone);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCone::native_GetHeight, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCone::native_GetRadius, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCone::native_GetAxis, 0x2);
    }

    curClass = GetClass(PhyCompound);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCompound::native_AddShape, 0x0);
    }

    curClass = GetClass(PhyObject);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_SetPos, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_SetRot, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_GetPos, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_GetRot, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_GetCurrentTransform, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_SetShape, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_GetShape, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_SetEntityOwner, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_GetEntityOwner, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_SetFilter, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_GetLineCollision, 0xA);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_GetRayCollision, 0xB);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_GetConvexCollision, 0xC);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_Activate, 0xD);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_Deactivate, 0xE);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_IsActive, 0xF);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_EnableCollisionCallback, 0x10);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_ApplyImpulse, 0x11);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_ApplyRelativeImpulse, 0x12);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_SetVelocity, 0x13);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_GetVelocity, 0x14);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyObject::native_GetBounds, 0x15);
    }

    curClass = GetClass(PhyGhost);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyGhost::native_UpdatePositionalData, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyGhost::native_NumOverlappingObjects, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyGhost::native_GetOverlappingObject, 0x2);
    }

    curClass = GetClass(PhyRigid);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyRigid::native_GetType, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyRigid::native_MakeStatic, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyRigid::native_MakeKinematic, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyRigid::native_MakeDynamic, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyRigid::native_SetRestitution, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyRigid::native_GetRestitution, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyRigid::native_SetFriction, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyRigid::native_GetFriction, 0x7);
    }

    curClass = GetClass(PhyCharacter);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCharacter::native_SetMoveDirection, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCharacter::native_GetMoveDirection, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCharacter::native_IsFalling, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCharacter::native_Jump, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCharacter::native_IsMoving, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCharacter::native_SetFriction, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCharacter::native_GetFriction, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhyCharacter::native_Stop, 0x7);
    }

    curClass = GetClass(PhysicsSystem);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_MakeSphere, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_MakeBox, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_MakeBox_2, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_MakeCylinder, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_MakeCapsule, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_MakeCone, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_MakeCompound, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_CreateStaticObject, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_CreateKinematicObject, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_CreateDynamicObject, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_CreateCharacterObject, 0xA);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_CreateGhost, 0xB);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_GetLineCollision, 0xC);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_GetRayCollision, 0xD);
        curClass->DefineNativeMember((OBJECTCALLBACK)&PhysicsSystem::native_GetConvexCollision, 0xE);
    }

    curClass = GetClass(Projector);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(Projector, effect), 0);
        curClass->DefineNativeVariable(offsetof(Projector, texture), 1);

        curClass->DefineNativeMember((OBJECTCALLBACK)&Projector::native_SetPerspective, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Projector::native_SetFrustum, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Projector::native_SetOrtho, 0x3);
    }

    curClass = GetClass(Decal);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(Decal, decalColor), 0);
    }

    curClass = GetClass(SerializerObject);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_SerializeChar, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_SerializeByte, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_SerializeShort, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_SerializeWord, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_SerializeInt, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_SerializeFloat, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_SerializeDouble, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_SerializeString, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_SerializeObject, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_IsLoading, 0x9);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_GetPos, 0xA);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SerializerObject::native_Seek, 0xB);
    }

    curClass = GetClass(Sound);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Sound::native_Play, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Sound::native_Stop, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Sound::native_SetVolume, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Sound::native_GetVolume, 0x3);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Sound::native_SetPosition, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Sound::native_SetVelocity, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Sound::native_SetRange, 0x6);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Sound::native_GetRange, 0x7);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Sound::native_SetPitch, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Sound::native_GetPitch, 0x9);
    }

    curClass = GetClass(SoundSystem);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&SoundSystem::native_SetEffectsVol, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&SoundSystem::native_GetEffectsVol, 0x1);
    }

    curClass = GetClass(Sprite3D);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(Sprite3D, spriteTextureName), 0);
        curClass->DefineNativeVariable(offsetof(Sprite3D, spriteColor), 1);
        curClass->DefineNativeVariable(offsetof(Sprite3D, spriteSize), 2);
        curClass->DefineNativeVariable(offsetof(Sprite3D, spriteRotation), 3);
        curClass->DefineNativeVariable(offsetof(Sprite3D, spriteRotationRate), 4);
    }

    curClass = GetClass(TexturedLine);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(TexturedLine, pointA), 0);
        curClass->DefineNativeVariable(offsetof(TexturedLine, pointB), 1);
        curClass->DefineNativeVariable(offsetof(TexturedLine, width), 2);
        curClass->DefineNativeVariable(offsetof(TexturedLine, color), 3);

        curClass->DefineNativeMember((OBJECTCALLBACK)&TexturedLine::native_SetLineTexture, 0x0);
    }

    curClass = GetClass(Viewport);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeMember((OBJECTCALLBACK)&Viewport::native_SetCamera, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Viewport::native_GetCamera, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Viewport::native_GetMouseRay, 0x2);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Viewport::native_GetPointViewportPos, 0x3);
    }

    curClass = GetClass(Window);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(Window, id), 0);

        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_SetPos, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_SetSize, 0x1);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_GetRealPos, 0x4);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_GetLocalPos, 0x5);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_GetSize, 0x8);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_SetOffsetType, 0xB);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_GetOffsetType, 0xC);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_SetPosOffset, 0xD);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_GetOffsetPoint, 0xF);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_SetFullScreen, 0x10);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_Attach, 0x11);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_Detach, 0x12);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_GetParent, 0x13);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_NumChildren, 0x14);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_GetChild, 0x15);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_SetTopLevel, 0x16);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_SetSystem, 0x17);
        curClass->DefineNativeMember((OBJECTCALLBACK)&Window::native_GetSystem, 0x18);
    }

    curClass = GetClass(ControlWindow);
    assert(curClass);

    if(curClass)
    {
        curClass->DefineNativeVariable(offsetof(ControlWindow, bKeepFocus), 0);

        curClass->DefineNativeMember((OBJECTCALLBACK)&ControlWindow::native_TakeInputControl, 0x0);
        curClass->DefineNativeMember((OBJECTCALLBACK)&ControlWindow::native_HasFocus, 0x1);
    }

    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::Native_operator_Add_Vect2, 0x2);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::Native_operator_Sub_Vect2, 0x3);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::Native_operator_Div_Vect2, 0x4);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::Native_operator_Mul_Vect2, 0x5);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::Native_operator_Add_float, 0x6);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::Native_operator_Sub_float, 0x7);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::Native_operator_Div_float, 0x8);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::Native_operator_Mul_float, 0x9);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::Native_operator_Negate, 0xA);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::Native_operator_Equal_Vect2, 0xB);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::Native_operator_NotEqual_Vect2, 0xC);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::native_Set, 0xD);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::native_Norm, 0xE);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::native_GetNorm, 0xF);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::native_Len, 0x10);
    DefineNativeStructMember(TEXT("Vect2"), (OBJECTCALLBACK)&Vect2::native_Dist, 0x11);

    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::Native_operator_Add_Vect, 0x2);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::Native_operator_Sub_Vect, 0x3);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::Native_operator_Div_Vect, 0x4);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::Native_operator_Mul_Vect, 0x5);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::Native_operator_Add_float, 0x6);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::Native_operator_Sub_float, 0x7);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::Native_operator_Div_float, 0x8);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::Native_operator_Mul_float, 0x9);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::Native_operator_Negate, 0xA);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::Native_operator_Equal_Vect, 0xB);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::Native_operator_NotEqual_Vect, 0xC);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_Dot, 0xD);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_Cross, 0xE);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_Set, 0xF);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_CloseTo, 0x10);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_DistFromPlane, 0x11);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_ClampMin, 0x12);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_ClampMin_2, 0x13);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_ClampMax, 0x14);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_ClampMax_2, 0x15);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_Len, 0x16);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_Dist, 0x17);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_Norm, 0x18);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_GetNorm, 0x19);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_Abs, 0x1A);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_GetAbs, 0x1B);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_SetZero, 0x1C);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_TransformPoint, 0x1D);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_TransformVector, 0x1E);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_GetTransformedPoint, 0x1F);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_GetTransformedVector, 0x20);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_MakeFromRGB, 0x21);
    DefineNativeStructMember(TEXT("Vect"), (OBJECTCALLBACK)&Vect::native_GetRGB, 0x22);
    DefineNativeStructStaticMember(TEXT("Vect"), (NATIVECALLBACK)&Vect::native_Min, 0x23);
    DefineNativeStructStaticMember(TEXT("Vect"), (NATIVECALLBACK)&Vect::native_Min_2, 0x24);
    DefineNativeStructStaticMember(TEXT("Vect"), (NATIVECALLBACK)&Vect::native_Max, 0x25);
    DefineNativeStructStaticMember(TEXT("Vect"), (NATIVECALLBACK)&Vect::native_Max_2, 0x26);
    DefineNativeStructStaticMember(TEXT("Vect"), (NATIVECALLBACK)&Vect::native_Zero, 0x27);

    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::Native_operator_Add_Vect4, 0x2);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::Native_operator_Sub_Vect4, 0x3);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::Native_operator_Div_Vect4, 0x4);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::Native_operator_Mul_Vect4, 0x5);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::Native_operator_Add_float, 0x6);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::Native_operator_Sub_float, 0x7);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::Native_operator_Div_float, 0x8);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::Native_operator_Mul_float, 0x9);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::Native_operator_Equal_Vect4, 0xA);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::Native_operator_NotEqual_Vect4, 0xB);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_CloseTo, 0xC);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_Abs, 0xD);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_GetAbs, 0xE);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_NormXYZ, 0xF);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_GetNormXYZ, 0x10);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_NormFull, 0x11);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_GetNormFull, 0x12);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_DesaturateColor, 0x13);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_ClampColor, 0x14);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_GetDesaturatedColor, 0x15);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_GetClampedColor, 0x16);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_MakeFromRGBA, 0x17);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_GetRGBA, 0x18);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_SetZero, 0x19);
    DefineNativeStructStaticMember(TEXT("Vect4"), (NATIVECALLBACK)&Vect4::native_Zero, 0x1A);
    DefineNativeStructMember(TEXT("Vect4"), (OBJECTCALLBACK)&Vect4::native_Set, 0x1B);

    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::Native_operator_Mul_Quat, 0x1);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::Native_operator_Mul_AxisAngle, 0x2);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::Native_operator_Negate, 0x3);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::Native_operator_Assign_AxisAngle, 0x4);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_SetIdentity, 0x5);
    DefineNativeStructStaticMember(TEXT("Quat"), (NATIVECALLBACK)&Quat::native_Identity, 0x6);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_Inv, 0x7);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_GetInv, 0x8);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_Reverse, 0x9);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_GetReverse, 0xA);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_Norm, 0xB);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_GetNorm, 0xC);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_Len, 0xD);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_Dist, 0xE);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_MakeFromDirection, 0xF);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_SetLookDirection, 0x10);
    DefineNativeStructStaticMember(TEXT("Quat"), (NATIVECALLBACK)&Quat::native_GetLookDirection, 0x11);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_CloseTo, 0x12);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_GetDirectionVector, 0x13);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_GetInterpolationTangent, 0x14);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_CreateFromMatrix, 0x15);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_MakeFromAxisAngle, 0x16);
    DefineNativeStructMember(TEXT("Quat"), (OBJECTCALLBACK)&Quat::native_GetAxisAngle, 0x17);

    DefineNativeStructMember(TEXT("AxisAngle"), (OBJECTCALLBACK)&AxisAngle::Native_operator_Assign_Quat, 0x3);
    DefineNativeStructMember(TEXT("AxisAngle"), (OBJECTCALLBACK)&AxisAngle::native_CloseTo, 0x4);
    DefineNativeStructMember(TEXT("AxisAngle"), (OBJECTCALLBACK)&AxisAngle::native_Set, 0x5);
    DefineNativeStructMember(TEXT("AxisAngle"), (OBJECTCALLBACK)&AxisAngle::native_Clear, 0x6);
    DefineNativeStructMember(TEXT("AxisAngle"), (OBJECTCALLBACK)&AxisAngle::native_MakeFromQuat, 0x7);
    DefineNativeStructMember(TEXT("AxisAngle"), (OBJECTCALLBACK)&AxisAngle::native_GetQuat, 0x8);

    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_GetPoint, 0x1);
    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_GetTransformedBounds, 0x2);
    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_Transform, 0x3);
    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_GetCenter, 0x4);
    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_Intersects, 0x5);
    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_PointInside, 0x6);
    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_GetDiamater, 0x7);
    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_RayIntersection, 0x8);
    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_LineIntersection, 0x9);
    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_RayIntersects, 0xA);
    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_LineIntersects, 0xB);
    DefineNativeStructMember(TEXT("Bounds"), (OBJECTCALLBACK)&Bounds::native_PlaneTest, 0xC);

    DefineNativeStructMember(TEXT("Plane"), (OBJECTCALLBACK)&Plane::native_Transform, 0x2);
    DefineNativeStructMember(TEXT("Plane"), (OBJECTCALLBACK)&Plane::native_GetTransform, 0x3);
    DefineNativeStructMember(TEXT("Plane"), (OBJECTCALLBACK)&Plane::native_GetRayIntersection, 0x4);
    DefineNativeStructMember(TEXT("Plane"), (OBJECTCALLBACK)&Plane::native_GetIntersection, 0x5);

    DefineNativeStructMember(TEXT("Matrix"), (OBJECTCALLBACK)&Matrix::Native_operator_Assign_Quat, 0x2);
    DefineNativeStructMember(TEXT("Matrix"), (OBJECTCALLBACK)&Matrix::Native_operator_Mul_Matrix, 0x3);
    DefineNativeStructMember(TEXT("Matrix"), (OBJECTCALLBACK)&Matrix::Native_operator_Mul_Vect, 0x4);
    DefineNativeStructMember(TEXT("Matrix"), (OBJECTCALLBACK)&Matrix::Native_operator_Mul_Quat, 0x5);
    DefineNativeStructMember(TEXT("Matrix"), (OBJECTCALLBACK)&Matrix::Native_operator_Mul_AxisAngle, 0x6);
    DefineNativeStructMember(TEXT("Matrix"), (OBJECTCALLBACK)&Matrix::native_SetIdentity, 0x7);
    DefineNativeStructStaticMember(TEXT("Matrix"), (NATIVECALLBACK)&Matrix::native_Identity, 0x8);
    DefineNativeStructMember(TEXT("Matrix"), (OBJECTCALLBACK)&Matrix::native_Translate, 0x9);
    DefineNativeStructMember(TEXT("Matrix"), (OBJECTCALLBACK)&Matrix::native_Rotate, 0xA);
    DefineNativeStructMember(TEXT("Matrix"), (OBJECTCALLBACK)&Matrix::native_Rotate_2, 0xB);
    DefineNativeStructMember(TEXT("Matrix"), (OBJECTCALLBACK)&Matrix::native_Transpose, 0xC);
    DefineNativeStructMember(TEXT("Matrix"), (OBJECTCALLBACK)&Matrix::native_GetTranspose, 0xD);

    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetConfigString, 0xc4c80000);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetConfigInt, 0xc4c80001);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetConfigFloat, 0xc4c80002);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetConfigColor, 0xc4c80003);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetConfigString, 0xc4c80004);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetConfigInt, 0xc4c80005);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetConfigFloat, 0xc4c80006);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetConfigColor, 0xc4c80007);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetEngine, 0xc4c80008);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_UnloadGame, 0xc4c80009);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LoadGame, 0xc4c8000a);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CurrentGame, 0xc4c8000b);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GS, 0xc4c8000c);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixPush, 0xc4c8000d);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixPop, 0xc4c8000e);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixSet, 0xc4c8000f);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixMultiply, 0xc4c80010);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixRotate, 0xc4c80011);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixRotate_2, 0xc4c80012);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixRotate_3, 0xc4c80013);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixTranslate, 0xc4c80014);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixTranslate_2, 0xc4c80015);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixTranslate_3, 0xc4c80016);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixScale, 0xc4c80017);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixScale_2, 0xc4c80018);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixTranspose, 0xc4c80019);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixInverse, 0xc4c8001a);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MatrixIdentity, 0xc4c8001b);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetCurFont, 0xc4c8001c);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetCurFont_2, 0xc4c8001d);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetFont, 0xc4c8001e);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetFontColor, 0xc4c8001f);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetFontColor_2, 0xc4c80020);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DrawText, 0xc4c80021);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DrawTextCenter, 0xc4c80022);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_RenderStartNew, 0xc4c80023);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_RenderStart, 0xc4c80024);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_RenderStop, 0xc4c80025);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_RenderSave, 0xc4c80026);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_Vertex, 0xc4c80027);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_Vertex_2, 0xc4c80028);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_Normal, 0xc4c80029);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_Normal_2, 0xc4c8002a);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_Color, 0xc4c8002b);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_Color_2, 0xc4c8002c);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_TexCoord, 0xc4c8002d);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_TexCoord_2, 0xc4c8002e);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LoadVertexBuffer, 0xc4c8002f);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LoadTexture, 0xc4c80030);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LoadSamplerState, 0xc4c80031);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LoadIndexBuffer, 0xc4c80032);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LoadDefault2DSampler, 0xc4c80033);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LoadDefault3DSampler, 0xc4c80034);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LoadVertexShader, 0xc4c80035);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LoadPixelShader, 0xc4c80036);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetFrameBufferTarget, 0xc4c80037);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_Draw, 0xc4c80038);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DrawBare, 0xc4c80039);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetCullMode, 0xc4c8003a);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetCullMode, 0xc4c8003b);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_EnableBlending, 0xc4c8003c);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_BlendFunction, 0xc4c8003d);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_ClearDepthBuffer, 0xc4c8003e);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_EnableDepthTest, 0xc4c8003f);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DepthWriteEnable, 0xc4c80040);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DepthFunction, 0xc4c80041);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetDepthFunction, 0xc4c80042);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_ClearColorBuffer, 0xc4c80043);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_ColorWriteEnable, 0xc4c80044);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_ClearStencilBuffer, 0xc4c80045);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_EnableStencilTest, 0xc4c80046);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_StencilWriteEnable, 0xc4c80047);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_StencilFunction, 0xc4c80048);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_StencilOp, 0xc4c80049);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetPointSize, 0xc4c8004a);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_ToggleFullScreen, 0xc4c8004b);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetResolution, 0xc4c8004c);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_AdjustDisplayColors, 0xc4c8004d);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_EnumDisplayModes, 0xc4c8004e);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetCurrentDisplayMode, 0xc4c8004f);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_IsFullscreen, 0xc4c80050);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_Set2DMode, 0xc4c80051);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DrawSprite, 0xc4c80052);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DrawSprite3D, 0xc4c80053);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DrawSpriteCenter, 0xc4c80054);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DrawCubeBackdrop, 0xc4c80055);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DrawCubeBackdrop_2, 0xc4c80056);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DrawSpriteEx, 0xc4c80057);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetMouseOver, 0xc4c80058);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetLocalMousePos, 0xc4c80059);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetLocalMousePos, 0xc4c8005a);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetInputCodeName, 0xc4c8005c);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_PushKBHandler, 0xc4c8005d);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_PushMouseHandler, 0xc4c8005e);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CreateBareLevel, 0xc4c8005f);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LoadLevel, 0xc4c80060);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetLevel, 0xc4c80061);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_DEG, 0xc4c80062);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_RAD, 0xc4c80063);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_acos, 0xc4c80064);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_asin, 0xc4c80065);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_atan, 0xc4c80066);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_atan2, 0xc4c80067);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_cos, 0xc4c80068);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_sin, 0xc4c80069);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_tan, 0xc4c8006a);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_tanh, 0xc4c8006b);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_atof, 0xc4c8006c);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_atoi, 0xc4c8006d);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_finite, 0xc4c8006e);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_fabs, 0xc4c8006f);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_floor, 0xc4c80070);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_ceil, 0xc4c80071);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_fmod, 0xc4c80072);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MIN, 0xc4c80073);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MAX, 0xc4c80074);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MIN_2, 0xc4c80075);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MAX_2, 0xc4c80076);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_pow, 0xc4c80077);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_rand, 0xc4c80078);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_sqrt, 0xc4c80079);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CloseFloat, 0xc4c8007a);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_FloatString, 0xc4c8007b);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_FormattedFloat, 0xc4c8007c);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_IntString, 0xc4c8007d);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_UIntString, 0xc4c8007e);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LerpFloat, 0xc4c8007f);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LerpVect2, 0xc4c80080);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_LerpVect, 0xc4c80081);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_InterpolateQuat, 0xc4c80082);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CubicInterpolateQuat, 0xc4c80083);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_RandomFloat, 0xc4c80084);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_RandomVect, 0xc4c80085);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetHSpline, 0xc4c80086);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SphereRayCollision, 0xc4c80087);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CylinderRayCollision, 0xc4c80088);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CalcTorque, 0xc4c80089);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CalcTorque_2, 0xc4c8008a);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_MM, 0xc4c8008b);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CreateFactoryObject, 0xc4c8008c);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&Native_operator_OSTimeInfo_Less_OSTimeInfo, 0xc4c8008d);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&Native_operator_OSTimeInfo_Greater_OSTimeInfo, 0xc4c8008e);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&Native_operator_OSTimeInfo_Equal_OSTimeInfo, 0xc4c8008f);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_OSCreateDirectory, 0xc4c80090);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_OSFileExists, 0xc4c80091);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_OSGetFileTime, 0xc4c80092);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_OSAppHasFocus, 0xc4c80093);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_OSSetCursorPos, 0xc4c80094);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_OSGetCursorPos, 0xc4c80095);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_OSShowCursor, 0xc4c80096);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_EnumResources, 0xc4c80097);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_EnumFiles, 0xc4c80098);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetPathFileName, 0xc4c80099);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetPathDirectory, 0xc4c8009a);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetPathWithoutExtension, 0xc4c8009b);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetPathExtension, 0xc4c8009c);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_Log, 0xc4c8009d);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_EnableMemoryTracking, 0xc4c8009e);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_EndProgram, 0xc4c8009f);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_Physics, 0xc4c800a0);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetMaterial, 0xc4c800a1);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetTexture, 0xc4c800a2);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetCubeTexture, 0xc4c800a3);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_AddTextureRef, 0xc4c800a4);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_ReleaseMaterial, 0xc4c800a5);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_ReleaseMaterial_2, 0xc4c800a6);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_ReleaseTexture, 0xc4c800a7);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_ReleaseTexture_2, 0xc4c800a8);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetPixelShader, 0xc4c800a9);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetVertexShader, 0xc4c800aa);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetEffect, 0xc4c800ab);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_GetEffect_2, 0xc4c800ac);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_NewSound, 0xc4c800ad);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CreateFileInputSerializer, 0xc4c800ae);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CreateFileOutputSerializer, 0xc4c800af);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SS, 0xc4c800b0);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_PushCursorPos, 0xc4c800b5);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_PopCursorPos, 0xc4c800b6);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CurrentInputWindow, 0xc4c800b7);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_CurrentFocusWindow, 0xc4c800b8);
    Scripting->DefineNativeGlobal((NATIVECALLBACK)&NativeGlobal_SetUnechoedCursorPos, 0xc4c800b9);
    //</Script>
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  MaterialWindow.h

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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#pragma once


class MaterialItem;


class MaterialWindow : public ControlWindow
{
    DeclareClass(MaterialWindow, ControlWindow);

public:
    void Init();
    void Destroy();

    void SetDirectory(CTSTR lpDir);

    Material* SetEditMode(DWORD dwDrawType);

    void DeleteMaterial();

    void MouseDown(DWORD button);

    void CleanupItems();

    BOOL                bCanDraw;

    MaterialItem        *selectedMaterial;

    List<MaterialItem*> Items;
    List<Effect*>       Effects;
    List<String*>       EffectNames;

    Texture             *BlankAttenuation;
    Effect              *curEffect;

    VertexBuffer        *sphereVertBuffer;
    IndexBuffer         *sphereIdxBuffer;

    VertexBuffer        *boxVertBuffer;
    IndexBuffer         *boxIdxBuffer;
};


#define DRAWMATERIAL_SPHERE  0
#define DRAWMATERIAL_BOX     1
#define DRAWMATERIAL_FLAT    2


class MaterialItem : public ControlWindow
{
    DeclareClass(MaterialItem, ControlWindow);

public:
    MaterialItem() : mainTextureParam(-1) {bRenderable = TRUE;}
    void LoadFile(Effect *effect, ConfigFile &materialFile);

    void Init();
    void Destroy();

    void Render();

    void MouseDown(DWORD button);

    void EditMaterial();

    void CleanupParams();

    int mainTextureParam;

    DWORD dwDrawType;

    DWORD clickTime;

    String strName;

    Material *curMaterial;
};

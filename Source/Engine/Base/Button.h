/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Button.h:  Button

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


#ifndef BUTTON_HEADER
#define BUTTON_HEADER


class BASE_EXPORT Button : public ControlWindow
{
    DeclareClass(Button, ControlWindow);

protected:
    BOOL bButtonDown;

public:
    void Destroy();

    //<Script module="Base" classdecs="Button">
    BOOL bDisabled;
    Sound* upSound;
    Sound* downSound;
    Sound* overSound;
    //</Script>

    virtual void GotFocus();
    virtual void LostFocus();

    virtual void MouseDown(DWORD button);
    virtual void MouseUp(DWORD button);
};

class BASE_EXPORT TextureButton : public Button
{
    DeclareClass(TextureButton, Button);

    Texture *curTexture;

public:
    inline TextureButton() {bRenderable = TRUE;}
    TextureButton(int idIn) {bRenderable = TRUE; id = idIn;}

    //<Script module="Base" classdecs="TextureButton">
    Texture* upTex;
    Texture* downTex;
    Texture* overTex;
    Texture* disabledTex;
    //</Script>

    virtual void Destroy();

    virtual void GotFocus();
    virtual void LostFocus();

    virtual void MouseDown(DWORD button);
    virtual void MouseUp(DWORD button);

    virtual void Render();
};

class BASE_EXPORT PushButton : public Button
{
    DeclareClass(PushButton, Button);

    Texture *bmp;
    BOOL  bUseBitmap;

public:
    inline PushButton()
    {
        bgColor = Color_Black;
        fontColor = Color_White;
        strFont = TEXT("Base:Arial Medium.xft");
        bRenderable = TRUE;
    }

    inline PushButton(int idIn)
    {
        id = idIn;
        bgColor = Color_Black;
        fontColor = Color_White;
        strFont = TEXT("Base:Arial Medium.xft");
        bRenderable = TRUE;
    }

    inline void UseBitmap(Texture *tex)
    {
        if(tex)
        {
            bUseBitmap = TRUE;
            bmp = tex;
        }
        else if(bmp)
        {
            RM->ReleaseTexture(bmp);
            bmp = NULL;
            bUseBitmap = FALSE;
        }
    }

    //<Script module="Base" classdecs="PushButton">
    String strText;
    String strFont;
    DWORD fontColor;
    DWORD bgColor;
    //</Script>

    virtual void Init();

    virtual void Render();
};


#endif
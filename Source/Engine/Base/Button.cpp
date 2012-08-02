/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Button.cpp:  Button

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

DefineClass(Button);
DefineClass(TextureButton);
DefineClass(PushButton);


void Button::Destroy()
{
    traceIn(Button::Destroy);

    Super::Destroy();

    DestroyObject(upSound);
    DestroyObject(downSound);
    DestroyObject(overSound);

    traceOut;
}

void Button::GotFocus()
{
    traceIn(Button::GotFocus);

    if(!bDisabled)
    {
        if(overSound)
            overSound->Play(FALSE);
    }

    traceOut;
}

void Button::LostFocus()
{
    traceIn(Button::LostFocus);

    if(!bDisabled)
        bButtonDown = FALSE;

    traceOut;
}

void Button::MouseDown(DWORD button)
{
    traceIn(Button::MouseDown);

    if((button == MOUSE_LEFTBUTTON) && !bDisabled)
    {
        if(downSound)
            downSound->Play(FALSE);
        bButtonDown = TRUE;
    }

    traceOut;
}

void Button::MouseUp(DWORD button)
{
    traceIn(Button::MouseUp);

    if((button == MOUSE_LEFTBUTTON) && bButtonDown)
    {
        if(upSound)
            upSound->Play(FALSE);

        bButtonDown = FALSE;

        SendParentMessage(Window_Command);
    }

    traceOut;
}


void TextureButton::Destroy()
{
    traceIn(TextureButton::Destroy);

    Super::Destroy();

    traceOut;
}

void TextureButton::GotFocus()
{
    traceIn(TextureButton::GotFocus);

    Super::GotFocus();

    if(!bDisabled && overTex)
        curTexture = overTex;

    traceOut;
}

void TextureButton::LostFocus()
{
    traceIn(TextureButton::LostFocus);

    Super::LostFocus();

    if(!bDisabled)
        curTexture = upTex;

    traceOut;
}

void TextureButton::MouseDown(DWORD button)
{
    traceIn(TextureButton::MouseDown);

    Super::MouseDown(button);

    if((button == MOUSE_LEFTBUTTON) && !bDisabled && downTex)
        curTexture = downTex;

    traceOut;
}

void TextureButton::MouseUp(DWORD button)
{
    traceIn(TextureButton::MouseUp);

    Super::MouseUp(button);

    if((button == MOUSE_LEFTBUTTON) && bButtonDown && upTex)
        curTexture = upTex;

    traceOut;
}

void TextureButton::Render()
{
    traceIn(TextureButton::Render);

    if(!curTexture)
    {
        if(!bDisabled)
        {
            if(!upTex)
                return;

            curTexture = upTex;
        }
        else
        {
            if(!disabledTex && !upTex)
                return;

            curTexture = disabledTex ? disabledTex : upTex;
        }
    }

    GS->DrawSprite(curTexture, 0.0f, 0.0f, GetSizeX(), GetSizeY());

    traceOut;
}


void PushButton::Init()
{
    traceIn(PushButton::Init);

    Super::Init();

    traceOut;
}

void PushButton::Render()
{
    traceIn(PushButton::Render);

    Vect2 LwR = GetSize();  //lower right

    Vect2 UL1(1.0f);        //upper right+1
    Vect2 LR1 = LwR-1.0f;   //lower right-1

    Vect2 UL2(2.0f);        //upper right+2
    Vect2 LR2 = LwR-2.0f;   //lower right-2

    Vect2 Middle = GetSize()*0.5f;

    Color4 mainColor;
    mainColor.MakeFromRGBA(bgColor);
    mainColor.SquarifyColor();

    EnableBlending(TRUE);
    EnableDepthTest(FALSE);
    DepthWriteEnable(FALSE);
    EnableTexturing(FALSE);
    BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);

    float textLength=0.0f,textLengthD2=0.0f,textHeightD2=0.0f;

    Color4 bmpColor = mainColor;

    if(!bUseBitmap)
    {
        GS->SetCurFont(strFont);
        if(strText.IsValid() && strFont.IsValid())
        {
            textLength = (float)GS->GetCurFont()->TextWidth(strText);
            textLengthD2 = textLength*0.5f;

            textHeightD2 = ((float)GS->GetCurFont()->GetFontHeight())*0.5f;
        }

        if(bDisabled)
            GS->SetFontColor(Color4().MakeFromRGBA(fontColor).DesaturateColor()*Color4(0.7f, 0.7f, 0.7f, 1.0f));
        else
            GS->SetFontColor(fontColor);
    }
    else
    {
        if(bDisabled)
            bmpColor.DesaturateColor();
    }

    RenderStart();
        Color(bgColor);
        Vertex(UL2.x, UL2.y);
        Vertex(UL2.x, LR2.y);
        Vertex(LR2.x, UL2.y);
        Vertex(LR2.x, LR2.y);
    RenderStop(GS_TRIANGLESTRIP);

    if(bButtonDown)
    {
        RenderStart();

            Color((mainColor - Color4(0.8f, 0.8f, 0.8f, 0.0f)).ClampColor());
            Vertex(0.0f, 0.0f); Vertex(LR1.x, 0.0f);
            Vertex(0.0f, 0.0f); Vertex(0.0f, LR1.y);

            Color(mainColor);
            Vertex(LR1.x, LR1.y); Vertex(LR1.x, 0.0f);
            Vertex(LR1.x, LR1.y); Vertex(0.0f, LR1.y);

            Color((mainColor - Color4(0.6f, 0.6f, 0.6f, 0.0f)).ClampColor());
            Vertex(UL1.x, UL1.y); Vertex(LR2.x, UL1.y);
            Vertex(UL1.x, UL1.y); Vertex(UL1.x, LR2.y);

            Color((mainColor - Color4(0.2f, 0.2f, 0.2f, 0.0f)).ClampColor());
            Vertex(LR2.x, LR2.y); Vertex(LR2.x, UL1.y);
            Vertex(LR2.x, LR2.y); Vertex(UL1.x, LR2.y);

        RenderStop(GS_LINES);

        if(bUseBitmap)
        {
            if(bmp)
                GS->DrawSpriteEx(bmp, bmpColor.GetRGBA(), UL2.x, UL2.y, LR2.x, LR2.y);
        }
        else if(!strText.IsEmpty() && (textLength < (LR2.x-UL2.x)))
            GS->DrawText((Middle.x-textLengthD2)+1, (Middle.y-textHeightD2), (Middle.x+textLengthD2)+1, Middle.y+textHeightD2, FALSE, strText);
    }
    else
    {
        RenderStart();

            Color(mainColor);
            Vertex(0.0f, 0.0f); Vertex(LR1.x, 0.0f);
            Vertex(0.0f, 0.0f); Vertex(0.0f, LR1.y);

            Color((mainColor - Color4(0.8f, 0.8f, 0.8f, 0.0f)).ClampColor());
            Vertex(LR1.x, LR1.y); Vertex(LR1.x, 0.0f);
            Vertex(LR1.x, LR1.y); Vertex(0.0f, LR1.y);

            Color((mainColor - Color4(0.2f, 0.2f, 0.2f, 0.0f)).ClampColor());
            Vertex(UL1.x, UL1.y); Vertex(LR2.x, UL1.y);
            Vertex(UL1.x, UL1.y); Vertex(UL1.x, LR2.y);

            Color((mainColor - Color4(0.6f, 0.6f, 0.6f, 0.0f)).ClampColor());
            Vertex(LR2.x, LR2.y); Vertex(LR2.x, UL1.y);
            Vertex(LR2.x, LR2.y); Vertex(UL1.x, LR2.y);

        RenderStop(GS_LINES);

        if(bUseBitmap)
        {
            if(bmp)
                GS->DrawSpriteEx(bmp, bmpColor.GetRGBA(), UL2.x, UL2.y, LR2.x, LR2.y);
        }
        else if(!strText.IsEmpty() && (textLength < (LR2.x-UL2.x)))
        {
            GS->DrawText(Middle.x-textLengthD2, (Middle.y-textHeightD2)-1.0f, Middle.x+textLengthD2, (Middle.y+textHeightD2)-1.0f, FALSE, strText);
        }
    }

    traceOut;
}

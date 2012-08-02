/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  CheckBox.cpp:  CheckBox

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

DefineClass(CheckBox);


void CheckBox::Init()
{
    traceIn(CheckBox::Init);

    Super::Init();

    SetSize(16.0f, 16.0f);

    state = CheckState_Unchecked;

    traceOut;
}

void CheckBox::Destroy()
{
    traceIn(CheckBox::Destroy);

    Super::Destroy();

    DestroyObject(upSound);
    DestroyObject(downSound);

    traceOut;
}

void CheckBox::MouseDown(DWORD button)
{
    traceIn(CheckBox::MouseDown);

    if((button == MOUSE_LEFTBUTTON) && !bDisabled)
    {
        if(downSound)
            downSound->Play(FALSE);
    }

    traceOut;
}

void CheckBox::MouseUp(DWORD button)
{
    traceIn(CheckBox::MouseUp);

    if((button == MOUSE_LEFTBUTTON))
    {
        if(upSound)
            upSound->Play(FALSE);

        state = (state == CheckState_Unchecked) ? CheckState_Checked : CheckState_Unchecked;

        SendParentMessage(Window_Command);
    }

    traceOut;
}

void CheckBox::Render()
{
    Vect2 LwR = GetSize();       //lower right

    Vect2 UL1(1.0f);        //upper right+1
    Vect2 LR1 = LwR-1.0f;   //lower right-1

    Vect2 UL2(2.0f);        //upper right+2
    Vect2 LR2 = LwR-2.0f;   //lower right-2

    float textLength=0.0f,textLengthD2=0.0f,textHeightD2=0.0f;

    Color4 mainColor;
    mainColor.MakeFromRGBA(bgColor);
    mainColor.SquarifyColor();

    RenderStart();
        Color(bgColor);
        Vertex(UL1.x, UL1.y);
        Vertex(UL1.x, LR1.y);
        Vertex(LR1.x, UL1.y);
        Vertex(LR1.x, LR1.y);
    RenderStop(GS_TRIANGLESTRIP);

    RenderStart();
        Color(mainColor);
        Vertex(0.0f, 0.0f); Vertex(LR1.x, 0.0f);
        Vertex(0.0f, 0.0f); Vertex(0.0f, LR1.y);
        Vertex(LR1.x, 0.0f);Vertex(LR1.x, LwR.y); 
        Vertex(0.0f, LR1.y);Vertex(LwR.x, LR1.y); 
    RenderStop(GS_LINES);

    if(state == CheckState_Checked)
    {
        Vect2 halfSize = GetSize()*0.5f;
        Vect2 halfSizeM2 = halfSize-2.0f;

        MatrixPush();
            MatrixTranslate(GetOffsetPoint(Offset_Center));
            MatrixRotate(AxisAngle(0.0f, 0.0f, 1.0f, RAD(45.0f)));

            RenderStart();
                Color(mainColor);
                Vertex(-1.0f, -halfSizeM2.y);
                Vertex(-1.0f,  halfSizeM2.y);
                Vertex( 1.0f, -halfSizeM2.y);
                Vertex( 1.0f,  halfSizeM2.y);
            RenderStop(GS_TRIANGLESTRIP);

            RenderStart();
                Color(mainColor);
                Vertex(-halfSizeM2.x, -1.0f);
                Vertex(-halfSizeM2.x,  1.0f);
                Vertex( halfSizeM2.x, -1.0f);
                Vertex( halfSizeM2.x,  1.0f);
            RenderStop(GS_TRIANGLESTRIP);
        MatrixPop();
    }

    if(bDisabled)
        GS->SetFontColor(Color4().MakeFromRGBA(fontColor).DesaturateColor()*Color4(0.8f, 0.8f, 0.8f, 1.0f));
    else
        GS->SetFontColor(fontColor);

    GS->SetCurFont(strFont);
    if(!strText.IsEmpty())
    {
        textLength = (float)GS->GetCurFont()->TextWidth(strText);
        textLengthD2 = textLength*0.5f;

        textHeightD2 = ((float)GS->GetCurFont()->GetFontHeight())*0.5f;

        Vect2 textOffset = GetOffsetPoint(Offset_CenterRight)+Vect2(5.0f, 0.0f);
        GS->DrawText(textOffset.x, (textOffset.y-textHeightD2), (textOffset.x+textLength), textOffset.y+textHeightD2, FALSE, strText);
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  StaticText.cpp:  Static Text Control

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

DefineClass(StaticText);


void StaticText::Render()
{
    if(strText.IsEmpty())
        return;

    Font *font = GS->GetFont(strFont);

    float fontHeight = font->GetFontHeight();
    float fontHeightD2 = fontHeight*0.5f;
    float textWidth = font->TextWidth(strText);
    float textWidthD2 = textWidth*0.5f;

    GS->SetFontColor(fontColor);
    GS->SetCurFont(font);

    Vect2 offsetPoint = GetOffsetPoint(textOffset);

    switch(textOffset)
    {
        case Offset_TopLeft:
            GS->DrawText(0.0f, 0.0f, textWidth, fontHeight, FALSE, strText);
            break;
        case Offset_TopCenter:
            GS->DrawText(offsetPoint.x-textWidthD2, 0.0f, textWidth, fontHeight, FALSE, strText);
            break;
        case Offset_TopRight:
            GS->DrawText(offsetPoint.x-textWidth, 0.0f, textWidth, fontHeight, FALSE, strText);
            break;

        case Offset_CenterLeft:
            GS->DrawText(0.0f, offsetPoint.y-fontHeightD2, textWidth, fontHeight, FALSE, strText);
            break;
        case Offset_Center:
            GS->DrawText(offsetPoint.x-textWidthD2, offsetPoint.y-fontHeightD2, textWidth, fontHeight, FALSE, strText);
            break;
        case Offset_CenterRight:
            GS->DrawText(offsetPoint.x-textWidth, offsetPoint.y-fontHeightD2, textWidth, fontHeight, FALSE, strText);
            break;

        case Offset_BottomLeft:
            GS->DrawText(0.0f, offsetPoint.y-fontHeight, textWidth, fontHeight, FALSE, strText);
            break;
        case Offset_BottomCenter:
            GS->DrawText(offsetPoint.x-textWidthD2, offsetPoint.y-fontHeight, textWidth, fontHeight, FALSE, strText);
            break;
        case Offset_BottomRight:
            GS->DrawText(offsetPoint.x-textWidth, offsetPoint.y-fontHeight, textWidth, fontHeight, FALSE, strText);
            break;
    }
}

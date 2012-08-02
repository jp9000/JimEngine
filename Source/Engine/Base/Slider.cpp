/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Slider.cpp:  Slider Control

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


DefineClass(SlideInfo);
DefineClass(Slider);


void Slider::Init()
{
    traceIn(Slider::Init);

    Super::Init();

    color      = 0xFFFFFFFF;

    minValue = 0.0f;
    maxValue = 1.0f;
    clampValue = 0.0f;
    bClampNumber = FALSE;

    traceOut;
}

void Slider::MouseDown(DWORD button)
{
    traceIn(Slider::MouseDown);

    if(button == MOUSE_LEFTBUTTON)
    {
        if(bMouseOverSlider)
            bKeepFocus = bHoldingSlider = TRUE;
    }

    traceOut;
}

void Slider::MouseUp(DWORD button)
{
    traceIn(Slider::MouseUp);

    if(button == MOUSE_LEFTBUTTON)
    {
        if(bHoldingSlider)
        {
            bKeepFocus = bHoldingSlider = FALSE;

            if(moveSound)
                moveSound->Play(FALSE);

            SlideInfo *si = CreateObject(SlideInfo);
            si->bEndSlide = TRUE;

            SendParentMessage(Window_Command, si);

            DestroyObject(si);
        }
    }

    traceOut;
}

void Slider::MouseMove(int x, int y, short x_offset, short y_offset)
{
    traceIn(Slider::MouseMove);

    Vect2 truePos = GetRealPos();

    if(!bHoldingSlider)
    {
        float SLPos = Lerp(truePos.x+2.0f, (truePos.x+GetSizeX())-2.0f, curPos);
        float SLPm2 = SLPos-2.0f;
        float SLPp2 = SLPos+2.0f;

        bMouseOverSlider = ((x <= SLPp2) && (x >= SLPm2));
    }
    else
    {
        float stretch = GetSizeX()-8.0f;

        float lastPos = curPos;

        curPos = ((float(x)-(truePos.x+2.0f))/stretch);
        if(curPos < 0.0f)
            curPos = 0.0f;
        if(curPos > 1.0f)
            curPos = 1.0f;

        if(clampValue != 0.0f)
        {
            if(!bClampNumber)
            {
                float trueClamp = clampValue/(maxValue-minValue);
                curPos += (trueClamp*0.5f);
                curPos = floor(curPos/trueClamp)*trueClamp;
            }
            else
            {
                float trueClamp = (clampValue-minValue)/(maxValue-minValue);
                if( (curPos >= (trueClamp-0.03f)) &&
                    (curPos <= (trueClamp+0.03f)) )
                {
                    curPos = trueClamp;
                }
            }
        }

        SlideInfo *si = CreateObject(SlideInfo);
        si->bEndSlide = FALSE;

        SendParentMessage(Window_Command, si);

        DestroyObject(si);
    }

    traceOut;
}

void Slider::Render()
{
    Vect2 LwR = GetSize();       //lower right

    Vect2 UL1(1.0f);       //upper right+1
    Vect2 LR1 = LwR-1.0f;   //lower right-1

    Vect2 UL2(2.0f);       //upper right+2
    Vect2 LR2 = LwR-2.0f;   //lower right-2

    Vect2 Mid = LwR*0.5f;
    Mid.Set(floor(Mid.x+0.5f), floor(Mid.y+0.5f));

    Vect2 Mp1 = Mid+1.0f;   //mid+1
    Vect2 Mm1 = Mid-1.0f;   //mid-1

    Color4 mainColor = Color4().MakeFromRGBA(color).SquarifyColor();

    RenderStart();
        Color(mainColor * Color4(0.5f, 0.5f, 0.5f, 1.0f));
        Vertex(0.0f, Mm1.y); Vertex(LR1.x, Mm1.y);

        Color(mainColor);
        Vertex(0.0f, Mid.y); Vertex(LR1.x, Mid.y);
    RenderStop(GS_LINES);

    //----------------------------

    float SLPos = floorf(LR1.x*curPos);
    float SLPm1 = SLPos-1.0f;
    float SLPp1 = SLPos+1.0f;
    float SLPm2 = SLPos-2.0f;
    float SLPp2 = SLPos+2.0f;

    RenderStart();
        Color(color);
        Vertex(SLPos, 0.0f); Vertex(SLPos, LR1.y);

        Color(mainColor * Color4(0.8f, 0.8f, 0.8f, 1.0f));
        Vertex(SLPm1, 1.0f); Vertex(SLPm1, LR2.y);
        Vertex(SLPm1, 1.0f); Vertex(SLPos, 1.0f);

        Color(mainColor);
        Vertex(SLPm2, 0.0f); Vertex(SLPm2, LR1.y);
        Vertex(SLPm2, 0.0f); Vertex(SLPp1, 0.0f);

        Color(mainColor * Color4(0.6f, 0.6f, 0.6f, 1.0f));
        Vertex(SLPp1, 1.0f); Vertex(SLPp1, LR2.y);
        Vertex(SLPos, LR2.y);Vertex(SLPp1, LR2.y); 

        Color(mainColor * Color4(0.3f, 0.3f, 0.3f, 1.0f));
        Vertex(SLPp2, 0.0f); Vertex(SLPp2, LR1.y);
        Vertex(SLPm1, LR1.y);Vertex(SLPp2, LR1.y); 
    RenderStop(GS_LINES);
}

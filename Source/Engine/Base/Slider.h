/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Slider.h:  Slider Control

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


#ifndef SLIDER_HEADER
#define SLIDER_HEADER


class SlideInfo : public Object
{
    DeclareClass(SlideInfo, Object);

public:
    //<Script module="Base" classdecs="SlideInfo">
    BOOL bEndSlide;
    //</Script>
};


class BASE_EXPORT Slider : public ControlWindow
{
    DeclareClass(Slider, ControlWindow);

protected:
    float curPos;
    BOOL bHoldingSlider;
    BOOL bMouseOverSlider;

public:
    inline Slider()
    {
        bRenderable = TRUE;
        color = Color_Black;
    }

    inline Slider(int idIn)
    {
        bRenderable = TRUE;
        id = idIn;
        color = Color_Black;
    }

    virtual void Init();

    virtual float GetValue() const {return Lerp(minValue, maxValue, curPos);}
    virtual void  SetValue(float val) {curPos = (val-minValue)/(maxValue-minValue);}

    virtual void MouseDown(DWORD button);
    virtual void MouseUp(DWORD button);

    virtual void MouseMove(int x, int y, short x_offset, short y_offset);

    virtual void Render();

    //<Script module="Base" classdecs="Slider">
    DWORD color;
    float minValue;
    float maxValue;
    float clampValue;
    BOOL bClampNumber;
    Sound* moveSound;

    Declare_Internal_Member(native_GetValue);
    Declare_Internal_Member(native_SetValue);
    //</Script>
};


#endif

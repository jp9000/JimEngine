/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditBox.cpp:  Edit Box

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

DefineClass(EditBox);


class EditBoxKBHandler : public KeyboardInputHandler
{
    DeclareClass(EditBoxKBHandler, KeyboardInputHandler);

    void KeyboardHandler(unsigned int code, BOOL keydown)
    {
        traceIn(EditBoxKBHandler::KeyboardHandler);

        if(!keydown)
            return;

        if(code == 27)
        {
            editBox->LostFocus();
        }
        else if(code == 8)
        {
            if(editBox->strBuffer.Length())
                editBox->strBuffer.SetLength(editBox->strBuffer.Length()-1);
        }
        else if(code == 13)
        {
            if(editBox->bMultiline)
            {
                if(editBox->strBuffer.Length() < editBox->maxLength)
                    editBox->strBuffer += (TCHAR)code;
            }
            else
                editBox->LostFocus();
        }
        else if((code < 127) && (code > 31))
        {
            if(editBox->strBuffer.Length() < editBox->maxLength)
                editBox->strBuffer += (TCHAR)code;
        }

        traceOut;
    }

public:
    EditBox *editBox;
};

DefineClass(EditBoxKBHandler);


void EditBox::Init()
{
    traceIn(EditBox::Init);

    Super::Init();

    maxLength = 0xFF;

    traceOut;
}

void EditBox::Destroy()
{
    traceIn(EditBox::Destroy);

    if(bSelected)
    {
        DestroyObject(kbHandler);
        kbHandler = NULL;
    }

    Super::Destroy();

    traceOut;
}

void EditBox::Render()
{
    Vect2 LwR = GetSize();  //lower right

    Vect2 UL1(1.0f);        //upper right+1
    Vect2 LR1 = LwR-1.0f;   //lower right-1

    Vect2 UL2(2.0f);        //upper right+2
    Vect2 LR2 = LwR-2.0f;   //lower right-2

    Vect2 UL8(8.0f);        //upper right+8
    Vect2 LR8 = LwR-8.0f;   //lower right-8

    Color4 mainColor = Color4().MakeFromRGBA(bgColor).SquarifyColor();

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

    GS->SetCurFont(strFont);
    if(bDisabled)
        GS->SetFontColor(Color4().MakeFromRGBA(fontColor).DesaturateColor()*0.8f);
    else
        GS->SetFontColor(fontColor);

    String visString;
    if(bPassword)
    {
        String strMask;

        for(int i=0; i<strBuffer.Length(); i++)
            strMask += (TCHAR)'*';

        visString = strMask + (TCHAR)(bShowTextCursor * '|');
    }
    else
        visString = strBuffer + (TCHAR)(bShowTextCursor * '|');

    if(visString.IsValid())
        GS->DrawText(5, 4, GetSizeX()-10, GetSizeY()-10, FALSE, visString.Array());
}

void EditBox::Tick(float fSeconds)
{
    fCursorTime += fSeconds;
    if(fCursorTime >= 0.5f)
    {
        fCursorTime -= 0.5f;
        if(bSelected)
            bShowTextCursor = !bShowTextCursor;
        else
            bShowTextCursor = FALSE;
    }
}


void EditBox::MouseDown(DWORD button)
{
    traceIn(EditBox::MouseDown);

    if(bSelected)
    {
        if(GetSystem()->GetMouseOver() != this)
        {
            bKeepFocus = FALSE;

            //emulate mouse input so we can seamlessly click whatever the user was trying to click on.
            GS->GetInput()->EmulateMouseInput(MOUSE_MOVE, 0, 0);
            GS->GetInput()->EmulateMouseInput(button, 0, TRUE);
        }
    }
    else if((button == MOUSE_LEFTBUTTON) && !bDisabled)
    {
        kbHandler = CreateObject(EditBoxKBHandler);
        kbHandler->editBox = this;
        PushKBHandler(kbHandler, TRUE);
        bKeepFocus = bSelected = TRUE;
    }

    traceOut;
}

void EditBox::LostFocus()
{
    traceIn(EditBox::LostFocus);

    if(bSelected)
    {
        DestroyObject(kbHandler);
        kbHandler = NULL;
    }
    bSelected = bKeepFocus = FALSE;

    traceOut;
}


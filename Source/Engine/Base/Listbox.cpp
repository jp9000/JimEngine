/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Listbox.cpp:  List Box

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

DefineClass(ListBox);

class ListBoxArrowButton : public PushButton
{
    DeclareClass(ListBoxArrowButton, PushButton);

public:
    void Render();

    BOOL bUp;
};

DefineClass(ListBoxArrowButton);


void ListBoxArrowButton::Render()
{
    Super::Render();

    Color4 mainColor = Color4().MakeFromRGBA(bgColor).SquarifyColor();

    if(bDisabled)
        mainColor *= Color4(0.5f, 0.5f, 0.5f, 1.0f);

    Vect2 offset(0.0f), v1, v2, v3;

    offset = (bButtonDown) ? 1.0f : 0.0f;

    if(bUp)
    {
        v1 = GetOffsetPoint(Offset_TopCenter)+Vect2(0.0f, 5.0f);
        v2 = GetOffsetPoint(Offset_BottomLeft)+Vect2(5.0f, -5.0f);
        v3 = GetOffsetPoint(Offset_BottomRight)+Vect2(-5.0f, -5.0f);
    }
    else
    {
        v1 = GetOffsetPoint(Offset_BottomCenter)+Vect2(0.0f, -5.0f);
        v2 = GetOffsetPoint(Offset_TopRight)+Vect2(-5.0f, 5.0f);
        v3 = GetOffsetPoint(Offset_TopLeft)+Vect2(5.0f, 5.0f);
    }

    RenderStart();
        Color(mainColor);
        Vertex(v1+offset);
        Vertex(v2+offset);
        Vertex(v3+offset);
    RenderStop(GS_TRIANGLES);
}


void ListBox::Init()
{
    traceIn(ListBox::Init);

    Super::Init();

    font = GetSystem()->GetFont(strFont);

    TopScroller = CreateObject(ListBoxArrowButton);
    TopScroller->SetOffsetType(Offset_TopRight);
    TopScroller->Attach(this);
    TopScroller->SetSize(18.0f, 18.0f);
    TopScroller->bgColor = bgColor;
    TopScroller->bUp = TRUE;
    TopScroller->SetPosOffset(-2.0f, 2.0f, FALSE);

    BottomScroller = CreateObject(ListBoxArrowButton);
    BottomScroller->SetOffsetType(Offset_BottomRight);
    BottomScroller->bDisabled = FALSE;
    BottomScroller->Attach(this);
    BottomScroller->SetSize(18.0f, 18.0f);
    BottomScroller->SetPosOffset(-2.0f, -2.0f, FALSE);
    BottomScroller->bgColor = bgColor;
    BottomScroller->bUp = FALSE;

    curItem = -1;

    traceOut;
}

void ListBox::Destroy()
{
    traceIn(ListBox::Destroy);

    Super::Destroy();

    for(int i=0; i<ItemList.Num(); i++)
        ItemList[i].FreeData();
    ItemList.Clear();

    traceOut;
}

void ListBox::Render()
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


    GS->SetCurFont(font);

    DWORD dwFontHeight = font->GetFontHeight();
    DWORD dwListSize = (((DWORD)GetSizeY())-10)/dwFontHeight;

    if(ItemList.Num() < dwListSize)
        dwListSize = ItemList.Num();

    DWORD dwEndPos = dwScrollPos+dwListSize;

    if((curItem >= dwScrollPos) && (curItem < dwEndPos))
    {
        DWORD curPixelPos = (curItem-dwScrollPos)*dwFontHeight;

        EnableDepthTest(0);
        SetCullMode(GS_NEITHER);

        DWORD selectColor = (mainColor*0.65f).GetRGBA();

        RenderStart();
            Color(selectColor);
            Vertex(5, curPixelPos+5);
            Vertex(LwR.x-23, curPixelPos+5);
            Vertex(5, curPixelPos+dwFontHeight+5);
            Vertex(LwR.x-23, curPixelPos+dwFontHeight+5);
        RenderStop(GS_TRIANGLESTRIP);

        SetCullMode(GS_BACK);
    }

    for(int i=dwScrollPos; i<dwEndPos; i++)
    {
        DWORD curPixelPos = (i-dwScrollPos)*dwFontHeight;
        GS->SetFontColor((curItem == i) ? (0xFF000000|(fontColor^bgColor)) : fontColor);
        GS->DrawText(5, curPixelPos+5, LwR.x-23, curPixelPos+dwFontHeight+5, FALSE, ItemList[i].name);
    }
}


int ListBox::AddItem(CTSTR name, Object *data, BOOL bDestroy)
{
    traceIn(ListBox::AddItem);

    assert(name && *name);

    if(!name || !*name)
        return INVALID;

    for(int i=0; i<ItemList.Num(); i++)
    {
        if(ItemList[i].name.CompareI(name))
        {
            assert(0);
            return INVALID;
        }
    }

    int id = ItemList.Num();;
    ListBoxItem &item = *ItemList.CreateNew();
    item.data = data;
    item.name = name;
    item.bDestroy = bDestroy;

    return id;

    traceOut;
}

void ListBox::InsertItem(int index, CTSTR name, Object *data, BOOL bDestroy)
{
    traceIn(ListBox::InsertItem);

    assert(name && *name && index <= ItemList.Num());

    if(!name || !*name || index > ItemList.Num())
        return;

    for(int i=0; i<ItemList.Num(); i++)
    {
        if(ItemList[i].name.CompareI(name))
        {
            assert(0);
            return;
        }
    }

    ListBoxItem &item = *ItemList.InsertNew(index);
    item.data = data;
    item.name = name;
    item.bDestroy = bDestroy;

    traceOut;
}

void ListBox::Remove(int index)
{
    traceIn(ListBox::Remove);

    assert(index < ItemList.Num());

    if(index >= ItemList.Num())
        return;

    ItemList[index].FreeData();
    ItemList.Remove(index);

    DWORD dwListSize = (((DWORD)GetSizeY())-10)/font->GetFontHeight();
    if( (ItemList.Num() > dwListSize) &&
        ((ItemList.Num()-dwScrollPos) < dwListSize) )
        --dwScrollPos;

    if(index == curItem)
        curItem = -1;

    traceOut;
}

void ListBox::RemoveItem(CTSTR name)
{
    traceIn(ListBox::RemoveItem);

    assert(name);

    for(int i=0; i<ItemList.Num(); i++)
    {
        if(ItemList[i].name.CompareI(name))
        {
            Remove(i);
            return;
        }
    }

    traceOut;
}


void ListBox::PreFrame()
{
    DWORD dwListSize = (((DWORD)GetSizeY())-10)/font->GetFontHeight();

    if(ItemList.Num() <= dwListSize)
    {
        dwScrollPos = 0;
        TopScroller->bDisabled = BottomScroller->bDisabled = TRUE;
    }
    else
    {
        TopScroller->bDisabled      = !dwScrollPos;
        BottomScroller->bDisabled   = ((ItemList.Num()-dwScrollPos) == dwListSize);
    }
}

void ListBox::MouseDown(DWORD button)
{
    traceIn(ListBox::MouseDown);

    if(button == MOUSE_LEFTBUTTON)
    {
        int mouse_x, mouse_y;
        GetSystem()->GetLocalMousePos(mouse_x, mouse_y);
        Vect2 mousePos(mouse_x, mouse_y);
        mousePos -= GetRealPos();

        if( (mousePos.x < 5.0f) || (mousePos.y < 5.0f) ||
            (mousePos.x > (GetSizeX()-23.0f)) || (mousePos.y > (GetSizeY()-5.0f)) )
            return;

        mousePos -= 5.0f;

        DWORD dwFontHeight = font->GetFontHeight();
        DWORD dwListSize = (((DWORD)GetSizeY())-10)/font->GetFontHeight();
        DWORD pos = mousePos.y;

        pos /= dwFontHeight;

        if((pos < ItemList.Num()) && (pos < dwListSize))
        {
            curItem = pos+dwScrollPos;

            SendParentMessage(Window_Command);
        }
    }

    traceOut;
}

UINT ListBox::OnMessage(Window *child, UINT message, Object *param)
{
    if(message == Window_Command)
    {
        if(child == TopScroller)
        {
            --dwScrollPos;
            TopScroller->bDisabled = !dwScrollPos;
            return 0;
        }
        else if(child == BottomScroller)
        {
            ++dwScrollPos;

            DWORD dwListSize = (((DWORD)GetSize().y)-10)/font->GetFontHeight();
            BottomScroller->bDisabled = ((ItemList.Num()-dwScrollPos) == dwListSize);
            return 0;
        }
    }

    return 0;
}


int ListBox::GetItemByName(CTSTR lpName)
{
    traceIn(ListBox::GetItemByName);

    for(int i=0; i<ItemList.Num(); i++)
    {
        if(ItemList[i].name.CompareI(lpName))
            return i;
    }

    return -1;

    traceOut;
}

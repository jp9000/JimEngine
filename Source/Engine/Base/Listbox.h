/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Listbox.h:  List Box

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


#ifndef LISTBOX_HEADER
#define LISTBOX_HEADER


class ListBoxArrowButton;

struct ListBoxItem
{
    String name;
    Object *data;
    BOOL bDestroy;

    inline void FreeData()
    {
        name.Clear();
        if(bDestroy)
            DestroyObject(data);
    }
};


class BASE_EXPORT ListBox : public ControlWindow
{
    friend class ListBoxButtonHandler;
    DeclareClass(ListBox, ControlWindow);

    //scroll pos
    DWORD dwScrollPos;

    //item data
    int curItem;
    List<ListBoxItem> ItemList;

    //font to use
    Font *font;

    //scrollbar buttons
    ListBoxArrowButton *TopScroller, *BottomScroller;

public:
    inline ListBox()
    {
        strFont = TEXT("Base:Arial Medium.xft");
        bgColor = Color_Black;
        fontColor = Color_White;
        bRenderable = TRUE;
    }

    inline ListBox(int idIn)
    {
        id = idIn;
        strFont = TEXT("Base:Arial Medium.xft");
        bgColor = Color_Black;
        fontColor = Color_White;
        bRenderable = TRUE;
    }

    virtual void Init();
    virtual void Destroy();

    virtual void Render();

    virtual int AddItem(CTSTR name, Object *data=NULL, BOOL bDestroy=TRUE);
    virtual void InsertItem(int index, CTSTR name, Object *data=NULL, BOOL bDestroy=TRUE);
    virtual void Remove(int index);
    virtual void RemoveItem(CTSTR name);

    virtual void PreFrame();

    virtual void MouseDown(DWORD button);

    virtual UINT OnMessage(Window *child, UINT message, Object *param);

    inline int NumItems() {return ItemList.Num();}

    inline int GetCurSel() {return curItem;}
    inline void SetCurSel(int id) {curItem = id;}

    inline Object* GetItemData(int id) {return ItemList[id].data;}
    inline String GetItemName(int id) {return String(ItemList[id].name);}

    int GetItemByName(CTSTR lpName);

    //<Script module="Base" classdecs="ListBox">
    String strFont;
    DWORD fontColor;
    DWORD bgColor;

    Declare_Internal_Member(native_AddItem);
    Declare_Internal_Member(native_InsertItem);
    Declare_Internal_Member(native_Remove);
    Declare_Internal_Member(native_RemoveItem);
    Declare_Internal_Member(native_GetCurSel);
    Declare_Internal_Member(native_SetCurSel);
    Declare_Internal_Member(native_NumItems);
    Declare_Internal_Member(native_GetItemName);
    Declare_Internal_Member(native_GetItemData);
    Declare_Internal_Member(native_GetItemByName);
    //</Script>
};


#endif
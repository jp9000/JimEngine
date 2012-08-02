/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Controls

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


#include "..\Base.h"


//<Script module="Base" filedefs="BasicControls.xscript">
void ListBox::native_AddItem(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    String name = cs.GetString(0);
    Object* data = (Object*)cs.GetObject(1);
    BOOL bDestroy = (BOOL)cs.GetInt(2);

    returnVal = AddItem(name, data, bDestroy);
}

void ListBox::native_InsertItem(CallStruct &cs)
{
    int index = cs.GetInt(0);
    String name = cs.GetString(1);
    Object* data = (Object*)cs.GetObject(2);
    BOOL bDestroy = (BOOL)cs.GetInt(3);

    InsertItem(index, name, data, bDestroy);
}

void ListBox::native_Remove(CallStruct &cs)
{
    int index = cs.GetInt(0);

    Remove(index);
}

void ListBox::native_RemoveItem(CallStruct &cs)
{
    String name = cs.GetString(0);

    RemoveItem(name);
}

void ListBox::native_GetCurSel(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = GetCurSel();
}

void ListBox::native_SetCurSel(CallStruct &cs)
{
    int index = cs.GetInt(0);

    SetCurSel(index);
}

void ListBox::native_NumItems(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = NumItems();
}

void ListBox::native_GetItemName(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    int index = cs.GetInt(0);

    returnVal = GetItemName(index);
}

void ListBox::native_GetItemData(CallStruct &cs)
{
    Object*& returnVal = (Object*&)cs.GetObjectOut(RETURNVAL);
    int index = cs.GetInt(0);

    returnVal = GetItemData(index);
}

void ListBox::native_GetItemByName(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    String name = cs.GetString(0);

    returnVal = GetItemByName(name);
}

void Slider::native_GetValue(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetValue();
}

void Slider::native_SetValue(CallStruct &cs)
{
    float val = cs.GetFloat(0);

    SetValue(val);
}
//</Script>

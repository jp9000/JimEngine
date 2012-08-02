/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ScriptLists

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


DefineClass(ObjectList);
DefineClass(StringListObject);
DefineClass(FileList);
DefineClass(DisplayModeList);


void ObjectList::native_Num(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = Objects.Num();
}

void ObjectList::native_GetItem(CallStruct &cs)
{
    Object*& returnVal = (Object*&)cs.GetObjectOut(RETURNVAL);
    int index = cs.GetInt(0);

    returnVal = Objects[index];
}

void ObjectList::native_Last(CallStruct &cs)
{
    Object*& returnVal = (Object*&)cs.GetObjectOut(RETURNVAL);

    returnVal = Objects.Last();
}

void ObjectList::native_Add(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    Object* obj = (Object*)cs.GetObject(0);

    returnVal = Objects.Add(obj);
}

void ObjectList::native_Insert(CallStruct &cs)
{
    int index = cs.GetInt(0);
    Object* obj = (Object*)cs.GetObject(1);

    Objects.Insert(index, obj);
}

void ObjectList::native_Remove(CallStruct &cs)
{
    int index = cs.GetInt(0);
    BOOL bDestroyObject = (BOOL)cs.GetInt(1);

    if(bDestroyObject)
        DestroyObject(Objects[index]);
    Objects.Remove(index);
}

void ObjectList::native_RemoveItem(CallStruct &cs)
{
    Object* obj = (Object*)cs.GetObject(0);
    BOOL bDestroyObject = (BOOL)cs.GetInt(1);

    if(bDestroyObject)
        DestroyObject(obj);
    Objects.RemoveItem(obj);
}

void ObjectList::native_Clear(CallStruct &cs)
{
    BOOL bDestroyObjects = (BOOL)cs.GetInt(0);

    if(bDestroyObjects)
    {
        for(int i=0; i<Objects.Num(); i++)
            DestroyObject(Objects[i]);
    }

    Objects.Clear();
}


StringListObject::StringListObject(StringList &strListIn)
{
    strList.TransferFrom(strListIn);
}

void StringListObject::native_Num(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = strList.Num();
}

void StringListObject::native_GetItem(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    int index = cs.GetInt(0);

    returnVal = strList[index];
}

void StringListObject::native_Last(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);

    returnVal = strList.Last();
}

void StringListObject::native_Add(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    String str = cs.GetString(0);

    returnVal = strList.Add(str);
}

void StringListObject::native_Insert(CallStruct &cs)
{
    int index = cs.GetInt(0);
    String str = cs.GetString(1);

    strList.Insert(index, str);
}

void StringListObject::native_Remove(CallStruct &cs)
{
    int index = cs.GetInt(0);

    strList.Remove(index);
}

void StringListObject::native_Clear(CallStruct &cs)
{
    strList.Clear();
}


FileList::FileList(StringList &filesIn, StringList &directoriesIn)
{
    files.TransferFrom(filesIn);
    directories.TransferFrom(directoriesIn);
}

void FileList::native_NumFiles(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    returnVal = files.Num();
}

void FileList::native_NumDirectories(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);
    returnVal = directories.Num();
}

void FileList::native_GetFile(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    int index = cs.GetInt(0);

    if(index < files.Num())
        returnVal = files[index];
}

void FileList::native_GetDirectory(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    int index = cs.GetInt(0);

    if(index < directories.Num())
        returnVal = directories[index];
}

DisplayModeList::DisplayModeList(List<DisplayMode> &displayModesIn)
{
    displayModes.TransferFrom(displayModesIn);
}

void DisplayModeList::native_Num(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = displayModes.Num();
}

void DisplayModeList::native_GetDisplayMode(CallStruct &cs)
{
    DisplayMode& returnVal = (DisplayMode&)cs.GetStructOut(RETURNVAL);
    int index = cs.GetInt(0);

    mcpy(&returnVal, &displayModes[index], sizeof(DisplayMode));
}

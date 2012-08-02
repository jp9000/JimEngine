/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ScriptLists.h:  Script Lists

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


#ifndef SCRIPTLISTS_HEADER
#define SCRIPTLISTS_HEADER


class ObjectList : public Object
{
    DeclareClass(ObjectList, Object);

    List<Object*> Objects;

public:
    //<Script classdec="ObjectList">
    Declare_Internal_Member(native_Num);
    Declare_Internal_Member(native_GetItem);
    Declare_Internal_Member(native_Last);
    Declare_Internal_Member(native_Add);
    Declare_Internal_Member(native_Insert);
    Declare_Internal_Member(native_Remove);
    Declare_Internal_Member(native_RemoveItem);
    Declare_Internal_Member(native_Clear);
    //</Script>
};


class StringListObject : public Object
{
    DeclareClass(StringListObject, Object);

    StringList strList;

public:
    inline StringListObject() {}
    StringListObject(StringList &strListIn);

    //<Script classdec="StringListObject">
    Declare_Internal_Member(native_Num);
    Declare_Internal_Member(native_GetItem);
    Declare_Internal_Member(native_Last);
    Declare_Internal_Member(native_Add);
    Declare_Internal_Member(native_Insert);
    Declare_Internal_Member(native_Remove);
    Declare_Internal_Member(native_Clear);
    //</Script>
};

class FileList : public Object
{
    DeclareClass(FileList, Object);

    StringList directories;
    StringList files;

public:
    inline FileList() {}
    FileList(StringList &filesIn, StringList &directoriesIn);

    //<Script classdec="FileList">
    Declare_Internal_Member(native_NumFiles);
    Declare_Internal_Member(native_NumDirectories);
    Declare_Internal_Member(native_GetFile);
    Declare_Internal_Member(native_GetDirectory);
    //</Script>
};

class DisplayModeList : public Object
{
    DeclareClass(DisplayModeList, Object);

    List<DisplayMode> displayModes;

public:
    inline DisplayModeList() {}
    DisplayModeList(List<DisplayMode> &displayModesIn);

    //<Script classdec="DisplayModeList">
    Declare_Internal_Member(native_Num);
    Declare_Internal_Member(native_GetDisplayMode);
    //</Script>
};


#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Other Script Stuff

  Copyright (c) 2009, Hugh Bailey
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
#include "GlobalNatives.h"


//<Script module="Base" filedefs="Other.xscript">
void ENGINEAPI Native_operator_OSTimeInfo_Less_OSTimeInfo(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const OSTimeInfo &time1 = (const OSTimeInfo&)cs.GetStruct(0);
    const OSTimeInfo &time2 = (const OSTimeInfo&)cs.GetStruct(1);

    returnVal = (time1 < time2);
}

void ENGINEAPI Native_operator_OSTimeInfo_Greater_OSTimeInfo(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const OSTimeInfo &time1 = (const OSTimeInfo&)cs.GetStruct(0);
    const OSTimeInfo &time2 = (const OSTimeInfo&)cs.GetStruct(1);

    returnVal = (time1 > time2);
}

void ENGINEAPI Native_operator_OSTimeInfo_Equal_OSTimeInfo(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const OSTimeInfo &time1 = (const OSTimeInfo&)cs.GetStruct(0);
    const OSTimeInfo &time2 = (const OSTimeInfo&)cs.GetStruct(1);

    returnVal = (time1 == time2);
}

void ENGINEAPI NativeGlobal_OSCreateDirectory(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String strDirectory = cs.GetString(0);

    returnVal = OSCreateDirectory(strDirectory);
}

void ENGINEAPI NativeGlobal_OSFileExists(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String strFile = cs.GetString(0);

    returnVal = OSFileExists(strFile);
}

void ENGINEAPI NativeGlobal_OSGetFileTime(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String strFile = cs.GetString(0);
    OSFileTime &fileTime = (OSFileTime&)cs.GetStructOut(1);

    returnVal = OSGetFileTime(strFile, fileTime);
}

void ENGINEAPI NativeGlobal_OSAppHasFocus(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = OSAppHasFocus();
}

void ENGINEAPI NativeGlobal_OSSetCursorPos(CallStruct &cs)
{
    int x = cs.GetInt(0);
    int y = cs.GetInt(1);

    OSSetCursorPos(x, y);
}

void ENGINEAPI NativeGlobal_OSGetCursorPos(CallStruct &cs)
{
    int &x = cs.GetIntOut(0);
    int &y = cs.GetIntOut(1);

    OSGetCursorPos(x, y);
}

void ENGINEAPI NativeGlobal_OSShowCursor(CallStruct &cs)
{
    BOOL bShow = (BOOL)cs.GetInt(0);

    OSShowCursor(bShow);
}

struct EnumFileInfo
{
    String fileName;
    BOOL bDirectory;
};

static BOOL RemovePathMarkers(String &str)
{
    str.FindReplace(TEXT("\\"), TEXT("/"));

    if(str.IsEmpty()) return FALSE;

    if(str.Right(1) == TEXT("/"))
        str.RemoveChar(str.Length()-1);

    if(str.IsEmpty()) return FALSE;

    if(str[0] == '/')
        str.RemoveChar(0);

    if(str.IsEmpty()) return FALSE;

    return TRUE;
}

void ENGINEAPI NativeGlobal_EnumResources(CallStruct &cs)
{
    List<EnumFileInfo> &fileList = (List<EnumFileInfo>&)cs.GetListOut(0);
    String module = cs.GetString(1);
    String subdir = cs.GetString(2);
    String filter = cs.GetString(3);

    if(!RemovePathMarkers(module))  {AppWarning(TEXT("Invalid module specified in script function EnumResources")); return;}
    if(!RemovePathMarkers(subdir))  {AppWarning(TEXT("Invalid subdirrectory specified in script function EnumResources")); return;}
    if(!RemovePathMarkers(filter))  {AppWarning(TEXT("Invalid filter specified in script function EnumResources")); return;}

    fileList.Clear();

    String strPath;
    strPath << TEXT("data/") << module << TEXT("/") << subdir << TEXT("/") << filter;

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(strPath, ofd);
    if(hFind)
    {
        do
        {
            if(ofd.bHidden) continue;

            EnumFileInfo &efi = *fileList.CreateNew();
            efi.bDirectory = ofd.bDirectory;
            efi.fileName = ofd.fileName;
        }while(OSFindNextFile(hFind, ofd));

        OSFindClose(hFind);
    }
}

void ENGINEAPI NativeGlobal_EnumFiles(CallStruct &cs)
{
    List<EnumFileInfo> &fileList = (List<EnumFileInfo>&)cs.GetListOut(0);
    String path = cs.GetString(1);

    if(path.IsEmpty()) {AppWarning(TEXT("Invalid path specified in script function EnumResources")); return;}

    fileList.Clear();

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(path, ofd);
    if(hFind)
    {
        do
        {
            if(ofd.bHidden) continue;

            EnumFileInfo &efi = *fileList.CreateNew();
            efi.bDirectory = ofd.bDirectory;
            efi.fileName = ofd.fileName;
        }while(OSFindNextFile(hFind, ofd));

        OSFindClose(hFind);
    }
}

void ENGINEAPI NativeGlobal_GetPathFileName(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    String path = cs.GetString(0);
    BOOL bExtention = (BOOL)cs.GetInt(1);

    returnVal = GetPathFileName(path, bExtention);
}

void ENGINEAPI NativeGlobal_GetPathDirectory(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    String path = cs.GetString(0);

    returnVal = GetPathDirectory(path);
}

void ENGINEAPI NativeGlobal_GetPathWithoutExtension(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    String path = cs.GetString(0);

    returnVal = GetPathWithoutExtension(path);
}

void ENGINEAPI NativeGlobal_GetPathExtension(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);
    String path = cs.GetString(0);

    returnVal = GetPathExtension(path);
}

void ENGINEAPI NativeGlobal_Log(CallStruct &cs)
{
    String strLine = cs.GetString(0);

    Log(strLine);
}

void ENGINEAPI NativeGlobal_EnableMemoryTracking(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);
    int id = cs.GetInt(1);

    EnableMemoryTracking(bEnable, id);
}

void ENGINEAPI NativeGlobal_EndProgram(CallStruct &cs)
{
    EndProgram();
}
//</Script>

//<Script module="Base" filedefs="TexturedLine.xscript">
void TexturedLine::native_SetLineTexture(CallStruct &cs)
{
    Texture* texture = (Texture*)cs.GetObject(0);

    SetLineTexture(texture);
}
//</Script>

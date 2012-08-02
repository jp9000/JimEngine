/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  XFileWindows.cpp:  The truth is out there

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

#ifdef __UNIX__

XFile::XFile(CTSTR lpFile, DWORD dwAccess, DWORD dwCreationDisposition)
    : dwPos(0)
{
    assert(lpFile);
    Open(lpFile, dwAccess, dwCreationDisposition);
}

BOOL XFile::Open(CTSTR lpFile, DWORD dwAccess, DWORD dwCreationDisposition)
{
    assert(lpFile);

    switch(dwCreationDisposition)
    {
        case XFILE_CREATENEW:
            hFile = fopen(lpFile, TEXT("rb"));
            if(hFile)
            {
                fclose(hFile);
                hFile = NULL;
                break;
            }
            else
                hFile = fopen(lpFile, TEXT("w+b"));
            break;
        case XFILE_CREATEALWAYS:
            hFile = fopen(lpFile, TEXT("w+b"));
            break;
        case XFILE_OPENEXISTING:
            hFile = fopen(lpFile, TEXT("r+b"));


            break;
        case XFILE_OPENALWAYS:
            hFile = fopen(lpFile, TEXT("r+b"));
            if(!hFile)
                hFile = fopen(lpFile, TEXT("w+b"));
            break;
    }


    return (hFile != 0);
}

DWORD XFile::Read(LPVOID lpBuffer, DWORD dwBytes)
{
    assert(lpBuffer);

    if(!lpBuffer) return XFILE_ERROR;

    if(!hFile) return XFILE_ERROR;

    dwPos += dwBytes;

    return fread(lpBuffer, dwBytes, 1, hFile);;
}

DWORD XFile::Write(LPVOID lpBuffer, DWORD dwBytes)
{
    assert(lpBuffer);

    if(!hFile) return XFILE_ERROR;

    dwPos += dwBytes;

    return fwrite(lpBuffer, dwBytes, 1, hFile);;
}

DWORD XFile::WriteStr(CTSTR lpBuffer)
{
    DWORD dwBytes = slen(lpBuffer);

    assert(lpBuffer);

    if(!hFile) return XFILE_ERROR;

    dwPos += dwBytes;

    return fwrite(lpBuffer, dwBytes, 1, hFile);;
}

BOOL XFile::SetFileSize(DWORD dwSize)
{
    assert(hFile);
    CrashError(TEXT("not implemented yet"));

    /*if(!hFile) return 0;


    return SetEndOfFile(hFile);*/
    return 0;
}

DWORD XFile::SetPos(int iPos, DWORD dwMoveMethod)  //uses the SetFilePointer 4th parameter flags
{
    DWORD dwNewPos;
    assert(hFile);

    if(!hFile) return 0;

    if(dwMoveMethod == XFILE_BEGIN)
        dwNewPos = iPos;
    else if(dwMoveMethod == XFILE_CURPOS)
        dwNewPos = dwPos + iPos;
    else
    {
        fseek(hFile, 0, SEEK_END);
        dwNewPos = ftell(hFile);
        dwNewPos += iPos;
        fseek(hFile, dwPos, SEEK_SET);
    }



    if(!fseek(hFile, dwNewPos, SEEK_SET))
        return dwPos = dwNewPos;
    else
        return XFILE_ERROR;
}

void XFile::Close()
{
    if(hFile)
    {
        fclose(hFile);
        hFile = NULL;
    }
}

#endif

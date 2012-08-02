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



#ifdef WIN32

#define _WIN32_WINDOWS 0x0410
#define _WIN32_WINNT   0x0403
#include <windows.h>
#include "Base.h"


XFile::XFile(CTSTR lpFile, DWORD dwAccess, DWORD dwCreationDisposition)
{
    traceInFast(XFile::XFile);

    assert(lpFile);
    Open(lpFile, dwAccess, dwCreationDisposition);

    traceOutFast;
}

BOOL XFile::Open(CTSTR lpFile, DWORD dwAccess, DWORD dwCreationDisposition)
{
    traceInFast(XFile::Open);

    dwPos = 0;

    assert(lpFile);
    if((hFile = CreateFile(lpFile, dwAccess, 0, NULL, dwCreationDisposition, 0, NULL)) == INVALID_HANDLE_VALUE)
    {
        hFile = NULL;
        return 0;
    }
    return 1;

    traceOutFast;
}

DWORD XFile::Read(LPVOID lpBuffer, DWORD dwBytes)
{
    traceInFast(XFile::Read);

    DWORD dwRet;

    assert(lpBuffer);

    if(!lpBuffer) return XFILE_ERROR;

    if(!hFile) return XFILE_ERROR;

    dwPos += dwBytes;

    ReadFile(hFile, lpBuffer, dwBytes, &dwRet, NULL);
    return dwRet;

    traceOutFast;
}

DWORD XFile::Write(LPVOID lpBuffer, DWORD dwBytes)
{
    traceInFast(XFile::Write);

    DWORD dwRet;

    assert(lpBuffer);

    if(!hFile) return XFILE_ERROR;

    dwPos += dwBytes;

    WriteFile(hFile, lpBuffer, dwBytes, &dwRet, NULL);

    if(dwRet)
        bHasWritten = TRUE;

    return dwRet;

    traceOutFast;
}

DWORD XFile::WriteStr(WSTR lpBuffer)
{
    traceInFast(XFile::WriteStr);

    assert(lpBuffer);

    if(!hFile) return XFILE_ERROR;

    DWORD dwElements = slen(lpBuffer);

    char lpDest[4096];
    DWORD dwBytes = (DWORD)wchar_to_utf8(lpBuffer, dwElements, lpDest, 4095, 0);
    DWORD retVal = Write(lpDest, dwBytes);

    return retVal;

    traceOutFast;
}

void XFile::FlushWorthlessPieceOfGarbageFileBuffers()
{
    FlushFileBuffers(hFile);
}

DWORD XFile::WriteStr(LPSTR lpBuffer)
{
    traceInFast(XFile::WriteStr);

    assert(lpBuffer);

    if(!hFile) return XFILE_ERROR;

    DWORD dwElements = (DWORD)strlen(lpBuffer);

    return Write(lpBuffer, dwElements);

    traceOutFast;
}

DWORD XFile::WriteAsUTF8(CTSTR lpBuffer, DWORD dwElements)
{
    traceInFast(XFile::WriteAsUTF8);

    if(!lpBuffer)
        return 0;

    if(!hFile) return XFILE_ERROR;

    if(!dwElements)
        dwElements = slen(lpBuffer);

    DWORD dwBytes = (DWORD)wchar_to_utf8_len(lpBuffer, dwElements, 0);
    LPSTR lpDest = (LPSTR)Allocate(dwBytes+1);
    wchar_to_utf8(lpBuffer, dwElements, lpDest, dwBytes, 0);
    DWORD retVal = Write(lpDest, dwBytes);
    Free(lpDest);

    return retVal;

    traceOutFast;
}

BOOL XFile::SetFileSize(DWORD dwSize)
{
    traceInFast(XFile::SetFileSize);

    assert(hFile);

    if(!hFile) return 0;

    if(dwPos > dwSize)
        dwPos = dwSize;

    SetPos(dwSize, FILE_BEGIN);
    return SetEndOfFile(hFile);

    traceOutFast;
}

DWORD XFile::GetFileSize()
{
    traceInFast(XFile::GetFileSize);

    return ::GetFileSize(hFile, NULL);

    traceOutFast;
}

DWORD XFile::SetPos(int iPos, DWORD dwMoveMethod)  //uses the SetFilePointer 4th parameter flags
{
    traceInFast(XFile::SetPos);

    assert(hFile);

    if(!hFile) return 0;

    DWORD moveValue;

    switch(dwMoveMethod)
    {
        case XFILE_CURPOS:
            moveValue = FILE_CURRENT;
            break;

        case XFILE_END:
            moveValue = FILE_END;
            break;

        case XFILE_BEGIN:
            moveValue = FILE_BEGIN;
            break;
    }

    return (dwPos = SetFilePointer(hFile, iPos, NULL, moveValue));

    traceOutFast;
}

void XFile::Close()
{
    traceInFast(XFile::Close);

    if(hFile)
    {
        if(bHasWritten)
            FlushFileBuffers(hFile);
        CloseHandle(hFile);
        hFile = NULL;
    }

    traceOutFast;
}

String GetPathFileName(CTSTR lpPath, BOOL bExtension)
{
    assert(lpPath);
    if(!lpPath)
        return String();

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(lpPath, ofd);

    if(!hFind)
        ofd.bDirectory = FALSE;
    else
        OSFindClose(hFind);

    if(!ofd.bDirectory)
    {
        CTSTR lpDirectoryEnd = srchr(lpPath, '/');

        if(!lpDirectoryEnd)
            lpDirectoryEnd = srchr(lpPath, '/');

        if(lpDirectoryEnd)
            lpPath = lpDirectoryEnd+1;
    }

    String newPath = lpPath;

    if(!bExtension)
    {
        TSTR pDot = srchr(newPath, '.');
        if(pDot)
            newPath.SetLength((((UPARAM)pDot)-((UPARAM)newPath.Array()))/sizeof(TCHAR));
    }

    return newPath;
}

String GetPathDirectory(CTSTR lpPath)
{
    assert(lpPath);
    if(!lpPath)
        return String();

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(lpPath, ofd);

    if(!hFind)
        ofd.bDirectory = FALSE;
    else
        OSFindClose(hFind);

    int nDirectoryEnd;

    if(!ofd.bDirectory)
    {
        CTSTR lpDirectoryEnd = srchr(lpPath, '/');

        if(!lpDirectoryEnd)
            lpDirectoryEnd = srchr(lpPath, '/');

        if(lpDirectoryEnd)
            nDirectoryEnd = (((UPARAM)lpDirectoryEnd)-((UPARAM)lpPath))/sizeof(TCHAR);
        else
            nDirectoryEnd = slen(lpPath);
    }
    else
        nDirectoryEnd = slen(lpPath);

    String newPath = lpPath;

    newPath[nDirectoryEnd] = 0;
    return newPath;
}

String GetPathWithoutExtension(CTSTR lpPath)
{
    assert(lpPath);
    if(!lpPath)
        return String();

    TSTR lpExtensionStart = srchr(lpPath, '.');
    if(lpExtensionStart)
    {
        UINT newLength = (UINT)(UPARAM)(lpExtensionStart-lpPath);
        if(!newLength)
            return String();

        String newString;
        newString.SetLength(newLength);
        scpy_n(newString, lpPath, newLength);

        return newString;
    }
    else
        return String(lpPath);
}

String GetPathExtension(CTSTR lpPath)
{
    assert(lpPath);
    if(!lpPath)
        return String();

    TSTR lpExtensionStart = srchr(lpPath, '.');
    if(lpExtensionStart)
        return String(lpExtensionStart+1);
    else
        return String();
}




#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  XFile.h:  The truth is out there

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

#ifndef XFILE_HEADER
#define XFILE_HEADER



#define XFILE_ERROR         0xFFFFFFFF

#define XFILE_READ          0x80000000
#define XFILE_WRITE         0x40000000

#define XFILE_CREATENEW     1
#define XFILE_CREATEALWAYS  2
#define XFILE_OPENEXISTING  3
#define XFILE_OPENALWAYS    4

#define XFILE_BEGIN         1
#define XFILE_CURPOS        2
#define XFILE_END           3


class BASE_EXPORT XFile
{
    HANDLE hFile;
    DWORD dwPos;
    BOOL bHasWritten;

public:
    XFile() {zero(this, sizeof(XFile));}
    XFile(CTSTR lpFile, DWORD dwAccess, DWORD dwCreationDisposition);

    ~XFile() {Close();}

    BOOL Open(CTSTR lpFile, DWORD dwAccess, DWORD dwCreationDisposition);
    BOOL IsOpen()   {return hFile != NULL;}
    DWORD Read(LPVOID lpBuffer, DWORD dwBytes);
    DWORD Write(LPVOID lpBuffer, DWORD dwBytes);
    DWORD WriteStr(LPSTR lpBuffer);
    DWORD WriteStr(WSTR lpBuffer);
    DWORD WriteAsUTF8(CTSTR lpBuffer, DWORD dwElements=0);

    void FlushWorthlessPieceOfGarbageFileBuffers();

    inline void ReadFileToString(String &strIn)
    {
        if(!IsOpen())
            return;

        SetPos(0, XFILE_BEGIN);
        DWORD dwFileSize = GetFileSize();
        LPSTR lpFileDataUTF8 = (LPSTR)Allocate(dwFileSize+1);
        lpFileDataUTF8[dwFileSize] = 0;
        Read(lpFileDataUTF8, dwFileSize);
        Close();

        strIn = lpFileDataUTF8;
        Free(lpFileDataUTF8);
    }

    BOOL SetFileSize(DWORD dwSize);
    DWORD GetFileSize();
    DWORD SetPos(int iPos, DWORD dwMoveMethod);
    inline DWORD GetPos() {return dwPos;}
    void Close();
};

BASE_EXPORT String GetPathFileName(CTSTR lpPath, BOOL bExtention=FALSE);
BASE_EXPORT String GetPathDirectory(CTSTR lpPath);
BASE_EXPORT String GetPathWithoutExtension(CTSTR lpPath);
BASE_EXPORT String GetPathExtension(CTSTR lpPath);

class BASE_EXPORT XFileInputSerializer : public Serializer
{
public:
    XFileInputSerializer() {bufferSize = 0; bufferPos = 0;}
    ~XFileInputSerializer() {Close();}

    BOOL IsLoading() {return TRUE;}

    BOOL Open(CTSTR lpFile)
    {
        bufferPos = bufferSize = 0;
        return file.Open(lpFile, XFILE_READ, XFILE_OPENEXISTING);
    }

    void Close()
    {
        file.Close();
    }

    void Serialize(LPVOID lpData, DWORD length)
    {
        assert(lpData);

        LPBYTE lpTemp = (LPBYTE)lpData;

        while(length)
        {
            if(!bufferSize || (bufferSize == bufferPos))
                Cache();

            DWORD dwReadSize = MIN(length, bufferSize-bufferPos);

            if(!dwReadSize)
                return;

            mcpy(lpTemp, Buffer+bufferPos, dwReadSize);

            lpTemp += dwReadSize;
            bufferPos += dwReadSize;

            length -= dwReadSize;
        }
    }

    DWORD Seek(long offset, DWORD seekType=SERIALIZE_SEEK_START)
    {
        DWORD ret;

        if(seekType == SERIALIZE_SEEK_START)
            ret = file.SetPos(offset, XFILE_BEGIN);
        else if(seekType == SERIALIZE_SEEK_CURRENT)
            ret = file.SetPos(long(GetPos())+offset, XFILE_BEGIN);
        else if(seekType == SERIALIZE_SEEK_END)
            ret = file.SetPos(offset, XFILE_END);

        Cache();

        return ret;
    }

    DWORD GetPos()
    {
        return (file.GetPos()-bufferSize)+bufferPos;
    }

private:
    void Cache()
    {
        bufferSize = file.Read(Buffer, 2048);
        bufferPos = 0;
    }

    XFile file;

    DWORD bufferPos, bufferSize;
    BYTE Buffer[2048];
};


class XFileOutputSerializer : public Serializer
{
public:
    XFileOutputSerializer() {bufferPos = 0;}

    BOOL IsLoading() {return FALSE;}

    BOOL Open(CTSTR lpFile, DWORD dwCreationDisposition)
    {
        totalWritten = bufferPos = 0;
        return file.Open(lpFile, XFILE_WRITE, dwCreationDisposition);
    }

    void Close()
    {
        Flush();
        file.Close();
    }

    void Serialize(LPVOID lpData, DWORD length)
    {
        assert(lpData);

        LPBYTE lpTemp = (LPBYTE)lpData;

        totalWritten += length;

        while(length)
        {
            if(bufferPos == 2048)
                Flush();
                
            DWORD dwWriteSize = MIN(length, (2048-bufferPos));

            assert(dwWriteSize);

            if(!dwWriteSize)
                return;

            mcpy(Buffer+bufferPos, lpTemp, dwWriteSize);

            lpTemp += dwWriteSize;
            bufferPos += dwWriteSize;

            length -= dwWriteSize;
        }
    }

    DWORD Seek(long offset, DWORD seekType=SERIALIZE_SEEK_START)
    {
        Flush();
        if(seekType == SERIALIZE_SEEK_START)
            seekType = XFILE_BEGIN;
        else if(seekType == SERIALIZE_SEEK_CURRENT)
            seekType = XFILE_CURPOS;
        else if(seekType == SERIALIZE_SEEK_END)
            seekType = XFILE_END;

        return file.SetPos(offset, seekType);;
    }

    DWORD GetPos()
    {
        return file.GetPos()+bufferPos;
    }

    inline DWORD GetTotalWritten() {return totalWritten;}

private:
    void Flush()
    {
        if(bufferPos)
        {
            file.Write(Buffer, bufferPos);
            bufferPos = 0;
        }
    }

    XFile file;

    DWORD bufferPos, totalWritten;
    BYTE Buffer[2048];
};


#endif

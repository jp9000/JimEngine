/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Serializer.h

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

#ifndef SERIALIZER_HEADER
#define SERIALIZER_HEADER


#define SERIALIZE_SEEK_START    0
#define SERIALIZE_SEEK_CURRENT  1
#define SERIALIZE_SEEK_END      2


class BASE_EXPORT Serializer
{
public:
    Serializer()                        {}
    virtual ~Serializer()               {}

    virtual BOOL IsLoading()=0;

    virtual void Serialize(LPVOID lpData, DWORD length)=0;

    virtual DWORD Seek(long offset, DWORD seekType=SERIALIZE_SEEK_START)=0;

    virtual DWORD GetPos()=0;

    virtual BOOL DataPending() {return FALSE;}

    friend inline Serializer& operator<<(Serializer &s, BYTE &b)       {s.Serialize(&b, 1);  return s;}
    friend inline Serializer& operator<<(Serializer &s, char &b)       {s.Serialize(&b, 1);  return s;}

    friend inline Serializer& operator<<(Serializer &s, WORD  &w)      {s.Serialize(&w, 2);  return s;}
    friend inline Serializer& operator<<(Serializer &s, short &w)      {s.Serialize(&w, 2);  return s;}

    friend inline Serializer& operator<<(Serializer &s, DWORD &dw)     {s.Serialize(&dw, 4); return s;}
    friend inline Serializer& operator<<(Serializer &s, LONG  &dw)     {s.Serialize(&dw, 4); return s;}

    friend inline Serializer& operator<<(Serializer &s, UINT  &dw)     {s.Serialize(&dw, 4); return s;}
    friend inline Serializer& operator<<(Serializer &s, int   &dw)     {s.Serialize(&dw, 4); return s;}

    friend inline Serializer& operator<<(Serializer &s, bool  &bv)     {BOOL bVal = (BOOL)bv; s.Serialize(&bVal, 4); return s;}

    friend inline Serializer& operator<<(Serializer &s, LONG64 &qw)    {s.Serialize(&qw, 8); return s;}
    friend inline Serializer& operator<<(Serializer &s, QWORD &qw)     {s.Serialize(&qw, 8); return s;}

    friend inline Serializer& operator<<(Serializer &s, float &f)      {s.Serialize(&f, 4);  return s;}
    friend inline Serializer& operator<<(Serializer &s, double &d)     {s.Serialize(&d, 8);  return s;}

    inline Serializer& OutputByte (BYTE  cVal)       {if(!IsLoading()) Serialize(&cVal,  1); return *this;}
    inline Serializer& OutputChar (char  cVal)       {if(!IsLoading()) Serialize(&cVal,  1); return *this;}
    inline Serializer& OutputWord (WORD  sVal)       {if(!IsLoading()) Serialize(&sVal,  2); return *this;}
    inline Serializer& OutputShort(short sVal)       {if(!IsLoading()) Serialize(&sVal,  2); return *this;}
    inline Serializer& OutputDword(DWORD lVal)       {if(!IsLoading()) Serialize(&lVal,  4); return *this;}
    inline Serializer& OutputLong (long  lVal)       {if(!IsLoading()) Serialize(&lVal,  4); return *this;}
    inline Serializer& OutputQword(QWORD qVal)       {if(!IsLoading()) Serialize(&qVal,  8); return *this;}
    inline Serializer& OutputInt64(INT64 qVal)       {if(!IsLoading()) Serialize(&qVal,  8); return *this;}

    inline Serializer& OutputFloat(float fVal)       {if(!IsLoading()) Serialize(&fVal,  4); return *this;}
    inline Serializer& OutputDouble(double dVal)     {if(!IsLoading()) Serialize(&dVal,  8); return *this;}
};


#endif



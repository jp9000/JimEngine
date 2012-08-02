/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EngineDefs.h:  Engine Default Definitions

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


#ifndef ENGINEDEF_HEADER
#define ENGINEDEF_HEADER

#undef UINT
#undef LPUINT
#undef ULONG
#undef DWORD
#undef LPDWORD
#undef USHORT
#undef WORD
#undef LPWORD
#undef UCHAR
#undef BYTE
#undef LPBYTE
#undef BOOL
#undef INT
#undef LPINT
#undef LONG
#undef LPLONG
#undef SHORT
#undef LPSHORT
#undef CHAR
#undef TCHAR
#undef TSTR
#undef LPCHAR
#undef VOID
#undef HANDLE
#undef LONG64
#undef ULONG64

#undef TRUE
#undef FALSE

#undef HIWORD
#undef LOWORD
#undef MAKEDWORD

#undef TEXT2
#undef TEXT
#undef T


#ifdef WIN32
typedef unsigned long       ULONG,DWORD,*LPDWORD;
typedef long                LONG,*LPLONG;
#else
#endif

typedef int                 BOOL,INT,*LPINT;
typedef unsigned int        UINT,*LPUINT;

typedef short               SHORT,*LPSHORT;
typedef unsigned short      USHORT,WORD,*LPWORD;

typedef char                CHAR,*LPCHAR;
typedef unsigned char       UCHAR,BYTE,*LPBYTE;

typedef long long           LONG64,INT64,LONGLONG;
typedef unsigned long long  QWORD,ULONG64,UINT64,ULONGLONG;

#ifdef C64
typedef long long           PARAM;
typedef unsigned long long  UPARAM;
#else
typedef long                PARAM;
typedef unsigned long       UPARAM;
#endif


#ifdef UNICODE
typedef wchar_t TCHAR;
#define TEXT2(val)  L ## val
#define TEXT(val)   TEXT2(val)
#else
typedef char TCHAR;
#define TEXT2(val)  val
#define TEXT(val)   TEXT2(val)
#endif

typedef char                *LPSTR;
typedef const char          *LPCSTR;
typedef wchar_t             *WSTR;
typedef const wchar_t       *CWSTR;
typedef TCHAR               *TSTR;
typedef const TCHAR         *CTSTR;

typedef void                VOID,*LPVOID,*HANDLE;


#define TRUE  1
#define FALSE 0

#define INVALID     0xFFFFFFFF


#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  inline.h:  greetings and salutations

  Copyright (c) 2001-2007, Hugh Bailey
  All rights reserved.

  Redipibution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redipibutions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redipibutions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the dipibution.
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
  on any theory of liability, whether in contract, puct liability, or tort
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Blah blah blah blah.  To the fricken' code already!
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef INLINE_HEADER
#define INLINE_HEADER

//disable this warning just for this file
#pragma warning(disable : 4035)


#ifdef WIN32
    #pragma intrinsic(memcpy, memset, memcmp)
#endif


#ifndef USE_CUSTOM_MEMORY_FUNCTIONS

inline void ENGINEAPI mcpy(void *pDest, const void *pSrc, size_t iLen)
{
    memcpy(pDest, pSrc, iLen);
}

#endif


#ifdef C64


#ifdef USE_CUSTOM_MEMORY_FUNCTIONS

inline void ENGINEAPI mcpy(void *pDest, const void *pSrc, size_t iLen)
{
    assert(pDest);
    assert(pSrc);

    register size_t iLenMod8 = iLen%8;
    register size_t iLenDiv8 = iLen/8;

    register QWORD *srcQW = (QWORD*)pSrc, *destQW = (QWORD*)pDest;
    while(iLenDiv8--)
        *(destQW++) = *(srcQW++);

    register BYTE *srcB = (BYTE*)srcQW, *destB = (BYTE*)destQW;
    while(iLenMod8--)
        *(destB++) = *(srcB++);
}

#endif

inline BOOL ENGINEAPI mcmp(const void *pDest, const void *pSrc, size_t iLen)
{
    assert(pDest);
    assert(pSrc);

    register size_t iLenMod8 = iLen%8;
    register size_t iLenDiv8 = iLen/8;

    register QWORD *srcQW = (QWORD*)pSrc, *destQW = (QWORD*)pDest;
    while(iLenDiv8--)
    {
        if(*(srcQW++) != *(destQW++))
            return FALSE;
    }

    register BYTE *srcB = (BYTE*)srcQW, *destB = (BYTE*)destQW;
    while(iLenMod8--)
    {
        if(*(srcB++) != *(destB++))
            return FALSE;
    }

    return TRUE;
}

inline void ENGINEAPI zero(void *pDest, size_t iLen)
{
    register size_t iLenMod8 = iLen%8;
    register size_t iLenDiv8 = iLen/8;

    register QWORD *destQW = (QWORD*)pDest;
    while(iLenDiv8--)
        *(destQW++) = 0;

    register BYTE *destB = (BYTE*)destQW;
    while(iLenMod8--)
        *(destB++) = 0;
}

inline void ENGINEAPI msetd(void *pDest, DWORD val, size_t iLen)
{
    assert(pDest);

    QWORD newVal = ((QWORD)val)|(((QWORD)val)<<32);

    register size_t iLenMod8 = iLen%8;
    register size_t iLenDiv8 = iLen/8;

    register QWORD *destQW = (QWORD*)pDest;
    while(iLenDiv8--)
        *(destQW++) = newVal;

    register BYTE *destB = (BYTE*)destQW;
    register BYTE *pVal = (BYTE*)&newVal;
    while(iLenMod8--)
        *(destB++) = *(pVal++);
}

inline void ENGINEAPI mswap(void *pDest, void *pSrc, size_t iLen)
{
    assert(pDest);
    assert(pSrc);

    register size_t iLenMod8 = iLen%8;
    register size_t iLenDiv8 = iLen/8;

    register QWORD *srcQW = (QWORD*)pSrc, *destQW = (QWORD*)pDest;
    while(iLenDiv8--)
    {
        QWORD qw = *destQW;
        *(destQW++) = *srcQW;
        *(srcQW++) = qw;
    }

    register BYTE *srcB = (BYTE*)srcQW, *destB = (BYTE*)destQW;
    while(iLenMod8--)
    {
        BYTE b = *destB;
        *(destB++) = *srcB;
        *(srcB++) = b;
    }
}

inline void ENGINEAPI mcpyrev(void *pDest, const void *pSrc, size_t iLen)
{
    assert(pDest);
    assert(pSrc);

    register size_t iLenMod8 = iLen%8;
    register size_t iLenDiv8 = iLen/8;

    register BYTE *srcB = (BYTE*)pSrc, *destB = (BYTE*)pDest;
    register QWORD *srcQW = (QWORD*)(srcB+iLen), *destQW = (QWORD*)(destB+iLen);
    while(iLenDiv8--)
        *(--destQW) = *(--srcQW);

    srcB = (BYTE*)srcQW;
    destB = (BYTE*)destQW;
    while(iLenMod8--)
        *(--destB) = *(--srcB);
}

inline unsigned int PtrTo32(void *ptr)
{
    return ((unsigned int)((unsigned long long)ptr));
}

#else

#pragma warning(disable : 4311)

inline unsigned int PtrTo32(void *ptr)
{
    return ((unsigned int)ptr);
}

#pragma warning(default : 4311)

#ifdef USE_CUSTOM_MEMORY_FUNCTIONS
inline void ENGINEAPI mcpy(void *pDest, const void *pSrc, size_t iLen)
{
    assert(pDest);
    assert(pSrc);

    register size_t iLenMod4 = iLen%4;
    register size_t iLenDiv4 = iLen/4;

    register DWORD *srcDW = (DWORD*)pSrc, *destDW = (DWORD*)pDest;
    while(iLenDiv4--)
        *(destDW++) = *(srcDW++);

    register BYTE *srcB = (BYTE*)srcDW, *destB = (BYTE*)destDW;
    while(iLenMod4--)
        *(destB++) = *(srcB++);
}
#endif

inline void ENGINEAPI zero(void *pDest, size_t iLen)
{
    assert(pDest);

    register size_t iLenMod4 = iLen%4;
    register size_t iLenDiv4 = iLen/4;

    register DWORD *destDW = (DWORD*)pDest;
    while(iLenDiv4--)
        *(destDW++) = 0;

    register BYTE *destB = (BYTE*)destDW;
    while(iLenMod4--)
        *(destB++) = 0;
}

inline BOOL ENGINEAPI mcmp(const void *pDest, const void *pSrc, size_t iLen)
{
    assert(pDest);
    assert(pSrc);

    register size_t iLenMod4 = iLen%4;
    register size_t iLenDiv4 = iLen/4;

    register DWORD *srcDW = (DWORD*)pSrc, *destDW = (DWORD*)pDest;
    while(iLenDiv4--)
    {
        if(*(srcDW++) != *(destDW++))
            return FALSE;
    }

    register BYTE *srcB = (BYTE*)srcDW, *destB = (BYTE*)destDW;
    while(iLenMod4--)
    {
        if(*(srcB++) != *(destB++))
            return FALSE;
    }

    return TRUE;
}

inline void ENGINEAPI msetd(void *pDest, DWORD val, size_t iLen)
{
    assert(pDest);

    register size_t iLenMod4 = iLen%4;
    register size_t iLenDiv4 = iLen/4;

    register DWORD *destDW = (DWORD*)pDest;
    while(iLenDiv4--)
        *(destDW++) = val;

    register BYTE *destB = (BYTE*)destDW;
    register BYTE *pVal = (BYTE*)&val;
    while(iLenMod4--)
        *(destB++) = *(pVal++);
}

inline void ENGINEAPI mswap(void *pDest, const void *pSrc, size_t iLen)
{
    assert(pDest);
    assert(pSrc);

    register size_t iLenMod4 = iLen%4;
    register size_t iLenDiv4 = iLen/4;

    register DWORD *srcDW = (DWORD*)pSrc, *destDW = (DWORD*)pDest;
    while(iLenDiv4--)
    {
        DWORD dw = *destDW;
        *(destDW++) = *srcDW;
        *(srcDW++) = dw;
    }

    register BYTE *srcB = (BYTE*)srcDW, *destB = (BYTE*)destDW;
    while(iLenMod4--)
    {
        BYTE b = *destB;
        *(destB++) = *srcB;
        *(srcB++) = b;
    }
}

inline void ENGINEAPI mcpyrev(void *pDest, const void *pSrc, size_t iLen)
{
    assert(pDest);
    assert(pSrc);

    register size_t iLenMod4 = iLen%4;
    register size_t iLenDiv4 = iLen/4;

    register BYTE *srcB = (BYTE*)pSrc, *destB = (BYTE*)pDest;
    register DWORD *srcDW = (DWORD*)(srcB+iLen), *destDW = (DWORD*)(destB+iLen);
    while(iLenDiv4--)
        *(--destDW) = *(--srcDW);

    srcB = (BYTE*)srcDW;
    destB = (BYTE*)destDW;
    while(iLenMod4--)
        *(--destB) = *(--srcB);
}

#endif

inline void ENGINEAPI mset(void *pDest, unsigned char val, size_t iLen)
{
    msetd(pDest, (val)|(val<<8)|(val<<16)|(val<<24), iLen);
}

inline void nop()
{
}

#pragma warning(default : 4035)

#endif

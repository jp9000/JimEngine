/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Alloc.h:  Base Allocation Class

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


#ifndef ALLOC_HEADER
#define ALLOC_HEADER


#include <new>



//========================================================
//Allocation class
class BASE_EXPORT Alloc
{
public:
    virtual ~Alloc()  {}

    virtual void * _Allocate(size_t dwSize)=0;
    virtual void * _ReAllocate(LPVOID lpData, size_t dwSize)=0;
    virtual void   _Free(LPVOID lpData)=0;
    virtual void   ErrorTermination()=0;

    virtual size_t GetCurrentMemoryUsage() {return 0;}

    inline void *operator new(size_t dwSize)
    {
        return malloc(dwSize);
    }

    inline void operator delete(void *lpData)
    {
        free(lpData);
    }

#ifdef _DEBUG
    inline void *operator new(size_t dwSize, TCHAR *lpFile, unsigned int lpLine)
    {
        return malloc(dwSize);
    }

    inline void operator delete(void *lpData, TCHAR *lpFile, unsigned int lpLine)
    {
        free(lpData);
    }
#endif

};


//========================================================
BASE_EXPORT extern   Alloc          *MainAllocator;

BASE_EXPORT extern unsigned int dwAllocCurLine;
BASE_EXPORT extern TCHAR *       lpAllocCurFile;


#define Free                        MainAllocator->_Free
//#define GetSize                     MainAllocator->_GetSize



//========================================================
#ifdef _DEBUG

    inline void* _debug_Allocate(size_t size, TCHAR *lpFile, unsigned int dwLine)                    {dwAllocCurLine = dwLine;lpAllocCurFile = lpFile;return MainAllocator->_Allocate(size);}
    inline void* _debug_ReAllocate(void* lpData, size_t size, TCHAR *lpFile, unsigned int dwLine)    {dwAllocCurLine = dwLine;lpAllocCurFile = lpFile;return MainAllocator->_ReAllocate(lpData, size);}

    #define Allocate(size)              _debug_Allocate(size, TEXT(__FILE__), __LINE__)
    #define ReAllocate(lpData, size)    _debug_ReAllocate(lpData, size, TEXT(__FILE__), __LINE__)

//========================================================
#else //!_DEBUG

    #define Allocate(size)             MainAllocator->_Allocate(size)
    #define ReAllocate(lpData, size)   MainAllocator->_ReAllocate(lpData, size)

#endif


inline void* operator new(size_t dwSize)
{
    void* val = Allocate((DWORD)dwSize);
    zero(val, (unsigned int)dwSize);

    return val;
}

inline void operator delete(void* lpData)
{
    Free(lpData);
}

inline void* operator new[](size_t dwSize)
{
    void* val = Allocate((DWORD)dwSize);
    zero(val, (unsigned int)dwSize);

    return val;
}

inline void operator delete[](void* lpData)
{
    Free(lpData);
}


#endif


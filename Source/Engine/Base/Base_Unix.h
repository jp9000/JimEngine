/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    UnixBase.h:  Unix header
  
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


#ifndef BASE_UNIX_HEADER
#define BASE_UNIX_HEADER

//-----------------------------------------
//warnings we don't want to hear over and over that have no relevance
//-----------------------------------------
/*#pragma warning(disable : 4018)
#pragma warning(disable : 4200)
#pragma warning(disable : 4244)
//#pragma warning(disable : 4699)
//#pragma warning(disable : 4245)
#pragma warning(disable : 4305)
#pragma warning(disable : 4711)
#pragma warning(disable : 4251)  //class '' needs to have dll-interface to be used by clients of class ''*/


//-----------------------------------------
//defines
//-----------------------------------------
#if !defined(BASE_EXPORTING)
    #define BASE_EXPORT     extern "C"
#else
    #define BASE_EXPORT
#endif


#define ENGINEAPI
#define __cdecl
#define DWORD (*ENGINETHREAD)(LPVOID);
#define _msize  malloc_usable_size


#if defined(ENGINE_MAIN_PROGRAM)

    int ENGINEAPI EngineMain(TSTR lpCommandLine, int c);

    int main(int argc, char *argv[])
    {
        return EngineMain(argv, argc);
    }

#elif defined(DLL_EXTENSION)
    BOOL ENGINEAPI EngineDLL(DWORD dwWhy);
    void _init() {EngineDLL(DLL_LOADING);}
    void _fini() {EngineDLL(DLL_UNLOADING);}
#endif



//-----------------------------------------
//typedefs
//-----------------------------------------
typedef LPVOID  DEFPROC;


//-----------------------------------------
//variables
//-----------------------------------------
BASE_EXPORT extern LPVOID   xdisplay;


#endif

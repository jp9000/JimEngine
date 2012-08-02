/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  DebugAlloc.h:  Tracked Allocation Class

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
#include "malloc.h"
#include <crtdbg.h>

//-----------------------------------------
//Allocation tracking struct
struct Allocation
{
    DWORD allocationID;
    LPVOID Address;
    TCHAR lpFile[1024];
    DWORD dwLine;
};

Allocation *AllocationList = NULL;
DWORD numAllocations = 0;
DWORD totalAllocations = 0;

DWORD allocationCounter = 0;

unsigned int dwAllocCurLine;
TCHAR *       lpAllocCurFile;

BOOL bEnableTracking = FALSE;

extern XFile LogFile;
extern int memoryBreakID;


void EnableMemoryTracking(BOOL bEnable, int id)
{
    bEnableTracking = bEnable;
    memoryBreakID = id;
}

void SetMemoryBreakID(int id)
{
    memoryBreakID = id;
}


DebugAlloc::DebugAlloc()
{
    LogFile.WriteStr(TEXT("Tracking Allocations\r\n"));
}

DebugAlloc::~DebugAlloc()
{
    if(numAllocations)
    {
        TCHAR temp[4096];

        tsprintf_s(temp, 4095, TEXT("%d Memory leaks detected on exit!\r\n"), numAllocations);
        LogFile.WriteStr(temp);

        LogFile.WriteStr(TEXT("Allocation Tracking Results: Memory Leaks:\r\n=========================================================\r\n"));
        for(DWORD i=0;i<numAllocations;i++)
        {
            if(AllocationList[i].allocationID != INVALID)
                tsprintf_s(temp, 4095, TEXT("\tID: %d\r\n\tAddress: 0x%lX\r\n\tDeclared in file %s on line %d\r\n"), AllocationList[i].allocationID, AllocationList[i].Address, AllocationList[i].lpFile, AllocationList[i].dwLine);
            else
                tsprintf_s(temp, 4095, TEXT("\tID: Track point was not enabled when allocation was made\r\n\tAddress: 0x%lX\r\n\tDeclared in file %s on line %d\r\n"), AllocationList[i].Address, AllocationList[i].lpFile, AllocationList[i].dwLine);
            LogFile.WriteStr(temp);
        }
        LogFile.WriteStr(TEXT("=========================================================\r\n"));
        return;
    }
}

void * DebugAlloc::_Allocate(size_t dwSize)
{
    if(!dwSize) return NULL;

    LPVOID lpRet;

    if(bEnableTracking)
    {
        ++allocationCounter;
        ++totalAllocations;
    }

    if(allocationCounter == memoryBreakID)
        ProgramBreak();

    if((lpRet=FastAlloc::_Allocate(dwSize)) && bEnableTracking)
    {
        Allocation allocTemp;

        Allocation *new_array = (Allocation*)FastAlloc::_Allocate(sizeof(Allocation)*++numAllocations);
        zero(new_array, sizeof(Allocation)*numAllocations);

        allocTemp.Address = lpRet;
        if(lpAllocCurFile)
            scpy(allocTemp.lpFile, lpAllocCurFile);
        allocTemp.dwLine = dwAllocCurLine;

        if(bEnableTracking)
            allocTemp.allocationID = allocationCounter;
        else
            allocTemp.allocationID = INVALID;

        if(AllocationList)
            mcpy(new_array, AllocationList, sizeof(Allocation)*(numAllocations-1));

        FastAlloc::_Free(AllocationList);

        AllocationList = new_array;

        mcpy(&AllocationList[numAllocations-1], &allocTemp, sizeof(Allocation));
    }

    return lpRet;
}

void * DebugAlloc::_ReAllocate(LPVOID lpData, size_t dwSize)
{
    LPVOID lpRet;

    if(!lpData)
    {
        lpRet = _Allocate(dwSize);
        return lpRet;
    }

    if(bEnableTracking)
    {
        ++allocationCounter;
        ++totalAllocations;
    }

    if(allocationCounter == memoryBreakID)
        ProgramBreak();

    lpRet = FastAlloc::_ReAllocate(lpData, dwSize);

    /*if(bEnableTracking)
    {*/
        for(DWORD i=0;i<numAllocations;i++)
        {
            if(AllocationList[i].Address == lpData)
            {
                if(bEnableTracking)
                    AllocationList[i].allocationID = allocationCounter;
                else
                    AllocationList[i].allocationID = INVALID;

                AllocationList[i].Address = lpRet;
                break;
            }
        }
    //}

    return lpRet;
}

DWORD freeCount = 0;

void DebugAlloc::_Free(LPVOID lpData)
{
    //assert(lpData);

    if(lpData)
    {
        FastAlloc::_Free(lpData);

        /*if(!bEnableTracking)
            return;*/

        for(DWORD i=0;i<numAllocations;i++)
        {
            if(AllocationList[i].Address == lpData)
            {
                if(!--numAllocations) {FastAlloc::_Free(AllocationList); AllocationList=NULL; return;}

                if(freeCount++ == 40)
                {
                    Allocation *new_array = (Allocation*)FastAlloc::_Allocate(sizeof(Allocation)*numAllocations);
                    zero(new_array, sizeof(Allocation)*numAllocations);
                    mcpy(new_array, AllocationList, sizeof(Allocation)*i);
                    mcpy(new_array+i, AllocationList+i+1, sizeof(Allocation)*(numAllocations-i));

                    FastAlloc::_Free(AllocationList);
                    AllocationList = new_array;

                    freeCount = 0;
                }
                else
                    mcpy(AllocationList+i, AllocationList+i+1, sizeof(Allocation)*(numAllocations-i));
                break;
            }
        }
    }
}


void* DefaultAlloc::_Allocate(size_t dwSize)
{
    return malloc(dwSize);
}

void* DefaultAlloc::_ReAllocate(LPVOID lpData, size_t dwSize)
{
    return (!lpData) ? malloc(dwSize) : realloc(lpData, dwSize);
}

void DefaultAlloc::_Free(LPVOID lpData)
{
    free(lpData);
}

void DefaultAlloc::ErrorTermination()
{
}


SeriousMemoryDebuggingAlloc::SeriousMemoryDebuggingAlloc()
{
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_ALLOC_MEM_DF);
}


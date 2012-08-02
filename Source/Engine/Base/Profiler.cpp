/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  DebugStuff.cpp:  Debug/Optimization Stuff

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



inline double MicroToMS(DWORD microseconds)
{
    return double(microseconds)*0.001;
}

float minPercentage, minTime;


struct ProfileNodeInfo
{
    inline ~ProfileNodeInfo()
    {
        FreeData();
    }

    void FreeData()
    {
        for(int i=0; i<Children.Num(); i++)
            delete Children[i];
        Children.Clear();
    }

    CTSTR lpName;

    DWORD numCalls;
    DWORD avgTimeElapsed;
    double avgPercentage;
    double childPercentage;
    double unaccountedPercentage;

    bool bSingular;

    QWORD totalTimeElapsed;

    ProfileNodeInfo *parent;
    List<ProfileNodeInfo*> Children;

    void calculateProfileData(int rootCallCount)
    {
        avgTimeElapsed = totalTimeElapsed/rootCallCount;
        
        if(parent)  avgPercentage = (double(avgTimeElapsed)/double(parent->avgTimeElapsed))*parent->avgPercentage;
        else        avgPercentage = 100.0f;

        childPercentage = 0.0;

        if(Children.Num())
        {
            for(int i=0; i<Children.Num(); i++)
            {
                Children[i]->parent = this;
                Children[i]->calculateProfileData(rootCallCount);

                if(!Children[i]->bSingular)
                    childPercentage += Children[i]->avgPercentage;
            }

            unaccountedPercentage = avgPercentage-childPercentage;
        }
    }

    void dumpData(int rootCallCount, int indent=0)
    {
        if(indent == 0)
            calculateProfileData(rootCallCount);

        String indentStr;
        for(int i=0; i<indent; i++)
            indentStr << TEXT("| ");

        CTSTR lpIndent = indent == 0 ? TEXT("") : indentStr.Array();

        int perFrameCalls = numCalls/rootCallCount;

        float fTimeTaken = MicroToMS(avgTimeElapsed);

        if(avgPercentage >= minPercentage && fTimeTaken >= minTime)
        {
            if(Children.Num())
                Log(TEXT("%s%s - [%.3g%%] [avg time: %g ms] [avg calls per frame: %d] [children: %.3g%%] [unaccounted: %.3g%%]"), lpIndent, lpName, avgPercentage, fTimeTaken, perFrameCalls, childPercentage, unaccountedPercentage);
            else
                Log(TEXT("%s%s - [%.3g%%] [avg time: %g ms] [avg calls per frame: %d]"), lpIndent, lpName, avgPercentage, fTimeTaken, perFrameCalls);
        }

        for(int i=0; i<Children.Num(); i++)
            Children[i]->dumpData(rootCallCount, indent+1);
    }

    ProfileNodeInfo* FindSubProfile(CTSTR lpName)
    {
        for(int i=0; i<Children.Num(); i++)
        {
            if(Children[i]->lpName == lpName)
                return Children[i];
        }

        return NULL;
    }

    static ProfileNodeInfo* FindProfile(CTSTR lpName)
    {
        for(int i=0; i<profilerData.Num(); i++)
        {
            if(profilerData[i].lpName == lpName)
                return profilerData+i;
        }

        return NULL;
    }

    static List<ProfileNodeInfo> profilerData;
};


ProfilerNode *__curProfilerNode = NULL;
List<ProfileNodeInfo> ProfileNodeInfo::profilerData;
BOOL bProfilingEnabled = FALSE;


void ENGINEAPI EnableProfiling(BOOL bEnable)
{
    //if(engine && !engine->InEditor())
    bProfilingEnabled = bEnable;

    minPercentage = AppConfig->GetFloat(TEXT("Profiler"), TEXT("MinPercentage"));
    minTime = AppConfig->GetFloat(TEXT("Profiler"), TEXT("MinTime"));
}

void ENGINEAPI DumpProfileData()
{
    if(ProfileNodeInfo::profilerData.Num())
    {
        Log(TEXT("\r\nProfiler results:\r\n"));
        Log(TEXT("=============================================================="));
        for(int i=0; i<ProfileNodeInfo::profilerData.Num(); i++)
            ProfileNodeInfo::profilerData[i].dumpData(ProfileNodeInfo::profilerData[i].numCalls);
        Log(TEXT("==============================================================\r\n"));
    }
}

void ENGINEAPI FreeProfileData()
{
    for(int i=0; i<ProfileNodeInfo::profilerData.Num(); i++)
        ProfileNodeInfo::profilerData[i].FreeData();
    ProfileNodeInfo::profilerData.Clear();
}

inline ProfilerNode::ProfilerNode(CTSTR lpName, bool bSingularize)
{
    info = NULL;
    this->lpName = NULL;

    parent = __curProfilerNode;

    if(bSingularNode = bSingularize)
    {
        if(!parent)
            return;

        while(parent->parent != NULL)
            parent = parent->parent;
    }
    else
        __curProfilerNode = this;

    if(parent)
    {
        if(!parent->lpName) return; //profiling was disabled when parent was created, so exit to avoid inconsistent results

        ProfileNodeInfo *parentInfo = parent->info;
        info = parentInfo->FindSubProfile(lpName);
        if(!info)
        {
            info = new ProfileNodeInfo;
            parentInfo->Children << info;
            info->lpName = lpName;
            info->bSingular = bSingularize;
        }
    }
    else if(bProfilingEnabled)
    {
        info = ProfileNodeInfo::FindProfile(lpName);
        if(!info)
        {
            info = ProfileNodeInfo::profilerData.CreateNew();
            info->lpName = lpName;
        }
    }
    else
        return;

    ++info->numCalls;

    this->lpName = lpName;
    startTime = OSGetTimeMicroseconds();
}

inline ProfilerNode::~ProfilerNode()
{
    QWORD newTime = OSGetTimeMicroseconds();

    //profiling was diabled when created
    if(lpName)
    {
        DWORD curTime = (DWORD)(newTime-startTime);
        info->totalTimeElapsed += curTime;
    }

    if(!bSingularNode)
        __curProfilerNode = parent;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Profiler:  Profiling/Optimization Stuff

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


#ifndef PROFILING_HEADER
#define PROFILING_HEADER


struct ProfileNodeInfo;

class BASE_EXPORT ProfilerNode
{
    CTSTR lpName;
    QWORD startTime;
    ProfilerNode *parent;
    bool bSingularNode;
    ProfileNodeInfo *info;

public:
    inline ProfilerNode(CTSTR name, bool bSingularize=false);
    inline ~ProfilerNode();
};

BASE_EXPORT extern ProfilerNode *__curProfilerNode;
BASE_EXPORT extern BOOL bProfilingEnabled;

#define ENABLE_PROFILING 1

#ifdef ENABLE_PROFILING
    #define profileSingularSegment(name)    ProfilerNode _curProfiler(TEXT(name), true);
    #define profileSingularIn(name)         {ProfilerNode _curProfiler(TEXT(name), true);
    #define profileSegment(name)            ProfilerNode _curProfiler(TEXT(name));
    #define profileIn(name)                 {ProfilerNode _curProfiler(TEXT(name));
    #define profileOut                      }
#else
    #define profileSingularSegment(name)
    #define profileSingularIn(name)
    #define profileSegment(name)
    #define profileIn(name)
    #define profileOut
#endif

BASE_EXPORT void ENGINEAPI EnableProfiling(BOOL bEnable);
BASE_EXPORT void ENGINEAPI DumpProfileData();
BASE_EXPORT void ENGINEAPI FreeProfileData();

#endif

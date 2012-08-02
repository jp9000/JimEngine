/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  PathFindingNatives

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


#include "..\Base.h"

//<Script module="Base" filedefs="PathSystem.xscript">
void AIPath::native_AdjustedTarget(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = AdjustedTarget();
}

void AIPath::native_GetTargetPos(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetTargetPos();
}

void AIPath::native_ResetPath(CallStruct &cs)
{
    ResetPath();
}

void AIPath::native_GetCurDist(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetCurDist();
}

void AIPath::native_GetTotalDist(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetTotalDist();
}

void AIPath::native_GetCurPos(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetCurPos();
}

void AIPath::native_GetCurNode(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = GetCurNode();
}

void AIPath::native_GetNextNode(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = GetNextNode();
}

void AIPath::native_PathEnded(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = PathEnded();
}

void AIPath::native_NumNodes(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = NumNodes();
}

void AIPath::native_GetNodePos(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    int nodeID = cs.GetInt(0);
    Vect &pos = (Vect&)cs.GetStructOut(1);

    returnVal = GetNodePos(nodeID, pos);
}

void AIPath::native_TraversePath(CallStruct &cs)
{
    TraverseType& returnVal = (TraverseType&)cs.GetIntOut(RETURNVAL);
    float moveDist = cs.GetFloat(0);
    int targetNodeID = cs.GetInt(1);

    returnVal = TraversePath(moveDist, targetNodeID);
}
//</Script>

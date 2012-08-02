/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  PathFinding

  Copyright (c) 2009, Hugh Bailey
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


#ifndef PATHFINDING_HEADER
#define PATHFINDING_HEADER



/*=========================================================
    AIPath - Base AI Path information class
==========================================================*/

enum TraverseType
{
    Traverse_Recalculate,
    Traverse_Continue,
    Traverse_Stop,
    Traverse_End
};

class BASE_EXPORT AIPath : public Object
{
    DeclareClass(AIPath, Object);

public:
    virtual ~AIPath() {}

    virtual void ResetPath()=0;
    virtual float GetCurDist() const=0;
    virtual float GetTotalDist() const=0;
    virtual Vect GetCurPos() const=0;
    virtual int GetCurNode() const=0;
    virtual int GetNextNode() const=0;
    virtual BOOL PathEnded() const=0;

    virtual void GetPlannedPath(List<Vect> &path) const=0;

    virtual int NumNodes() const=0;
    virtual BOOL GetNodePos(int nodeID, Vect &v) const=0;

    virtual BOOL AdjustedTarget() const=0;
    virtual Vect GetTargetPos() const=0;

    virtual TraverseType TraversePath(float moveDist, int destNodeID=-1)=0;

    //<Script module="Base" classdecs="AIPath">
    Declare_Internal_Member(native_AdjustedTarget);
    Declare_Internal_Member(native_GetTargetPos);
    Declare_Internal_Member(native_ResetPath);
    Declare_Internal_Member(native_GetCurDist);
    Declare_Internal_Member(native_GetTotalDist);
    Declare_Internal_Member(native_GetCurPos);
    Declare_Internal_Member(native_GetCurNode);
    Declare_Internal_Member(native_GetNextNode);
    Declare_Internal_Member(native_PathEnded);
    Declare_Internal_Member(native_NumNodes);
    Declare_Internal_Member(native_GetNodePos);
    Declare_Internal_Member(native_TraversePath);
    //</Script>
};

/*=========================================================
    PathSystem - Base Pathfinding System class
==========================================================*/

class BASE_EXPORT PathSystem : public Object
{
    DeclareClass(PathSystem, Object);

protected:
    inline static PathfindingData *GetEntityPathData(Entity *ent) {return ent->pathfindingData;}
    inline static void SetEntityPathData(Entity *ent, PathfindingData *data) {DestroyObject(ent->pathfindingData); ent->pathfindingData = data;}

public:
    virtual void AddBlocker(Entity *ent)=0;
    virtual void RemoveBlocker(Entity *ent)=0;

    virtual AIPath* GetEntityPath(Entity *ent, const Vect &targetPath)=0;
};

struct TraversedGridNode;


/*=========================================================
    GridPathSystem - Two-dimensional path system
==========================================================*/

class BASE_EXPORT GridPathSystem : public PathSystem
{
    DeclareClass(GridPathSystem, PathSystem);

    friend class GridAIPath;
    friend class EditorEngine;
    friend class OctLevel;

    Vect gridOffset;
    float nodeSize, nodeSizeSquared;
    int xSize, ySize;
    List<BYTE> NodeList;
    BitList BlockedNodes;

    //-----------------------------------

    int ConvertPosToNode(const Vect pos) const;
    Vect ConvertNodeToPos(int id) const;
    Vect ConvertNodeToPos(int x, int y) const;

    void GetNodeXY(int nodeID, int &x, int &y) const;

    int GetLinkNodeID(int nodeID, int link) const;

    //-----------------------------------

    float CalcH(int idSrc, int idTarget) const;

    unsigned int SearchList(List<TraversedGridNode> &list, int id) const;

    GridAIPath* AStar(Entity *ent, const Vect &targetPos);

    bool GetClosestOpenNode(int &blocker, const Vect &sourcePos, Vect &targetPos);

    //-----------------------------------

    void OptimizePath(GridAIPath *path, List<int> &PathList, const Vect &startVec, const Vect &endVec) const;
    bool IsUncollapsable(Entity *ent, const List<int> &PathList, int pathListID) const;
    unsigned int IsCollapsableCorner(Entity *ent, const List<int> &PathList, int pathListID, int &centerID) const;

    BOOL IsBlockedNode(Entity* ent, int id) const;

    //-----------------------------------

public:
    GridPathSystem() {}
    GridPathSystem(const Vect &mainOffset, float nodeLength, int width, int length);

    virtual void AddBlocker(Entity *ent);
    virtual void RemoveBlocker(Entity *ent);

    virtual AIPath* GetEntityPath(Entity *ent, const Vect &targetPos);

    void Serialize(Serializer &s);
};


#endif

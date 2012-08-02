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


#include "Base.h"


DefineClass(GridPathSystem);


enum
{
    UncollapsableItem,
    StraightPath,
    CollapsableCorner
};


struct TraversedGridNode
{
    int id;
    int parent;
    float G;
};

class GridPathfindingData : public PathfindingData
{
public:
    List<int> BlockIDs;

    virtual ~GridPathfindingData()
    {
        BlockIDs.Clear();
    }
};


#define GetEntityGridData(ent) static_cast<GridPathfindingData*>(GetEntityPathData(ent))



class GridAIPath : public AIPath
{
public:
    GridPathSystem *system;

    Entity *ent;

    List<Vect> Path;

    BOOL bPathEnded;
    BOOL bAdjustedTarget;

    float curFT;
    int curPathNode;
    Vect curPos;

    virtual BOOL AdjustedTarget() const {return bAdjustedTarget;}
    virtual Vect GetTargetPos() const {return Path.Last();}

    virtual void ResetPath()
    {
        curFT = 0.0f;
        curPathNode = 0;
    }

    virtual float GetCurDist() const
    {
        if((curFT == 0.0f) && (curPathNode == 0))
            return 0.0f;

        float totalPath = Path[curPathNode+1].Dist(Path[curPathNode])*curFT;

        for(int i=0; i<curPathNode; i++)
            totalPath += Path[i+1].Dist(Path[i]);

        return totalPath;
    }

    virtual float GetTotalDist() const
    {
        if((curFT == 0.0f) && (curPathNode == 0))
            return 0.0f;

        float totalPath = 0.0f;
        for(int i=0; i<Path.Num(); i++)
            totalPath += Path[i+1].Dist(Path[i]);

        return totalPath;
    }

    virtual Vect GetCurPos() const
    {
        return curPos;
    }

    virtual BOOL PathEnded() const
    {
        return bPathEnded;
    }

    virtual void GetPlannedPath(List<Vect> &path) const
    {
        path.CopyList(Path);
    }

    int GetCurNode() const
    {
        return curPathNode;
    }

    int GetNextNode() const
    {
        return (bPathEnded) ? curPathNode : curPathNode+1;
    }

    int NumNodes() const
    {
        return Path.Num();
    }

    BOOL GetNodePos(int nodeID, Vect &v) const
    {
        if(nodeID >= Path.Num())
            return FALSE;
        else
        {
            v = Path[nodeID];
            return TRUE;
        }
    }

    TraverseType TraversePath(float moveDist, int targetNode=-1)
    {
        if(targetNode == -1)
            targetNode = Path.Num()-1;

        if(targetNode <= curPathNode)
            return Traverse_Stop;

        if(bPathEnded)
            return Traverse_End;

        float dist = Path[curPathNode+1].Dist(Path[curPathNode]);
        curFT += moveDist/dist;

        while(curFT > 1.0f)
        {
            curFT -= 1.0f;
            ++curPathNode;

            if(curPathNode < targetNode)
            {
                dist = dist/Path[curPathNode+1].Dist(Path[curPathNode]);
                curFT *= dist;
            }
            else if(targetNode == Path.Num()-1)
            {
                curFT = 0.0f;
                bPathEnded = TRUE;
                return Traverse_End;
            }
            else 
            {
                curFT = 0.0f;
                return Traverse_Stop;
            }
        }

        curPos = Lerp<Vect>(Path[curPathNode], Path[curPathNode+1], curFT);

        return Traverse_Continue;
    }
};


int GridPathSystem::ConvertPosToNode(const Vect pos) const
{
    Vect gridPos = pos-gridOffset;
    Vect offset = (gridPos/nodeSize)+0.5f;
    int x = floorf(offset.x);
    int y = floorf(offset.z);

    return ((y*xSize)+x);
}

Vect GridPathSystem::ConvertNodeToPos(int id) const
{
    return gridOffset+Vect(float(id%xSize)*nodeSize, 0.0f, float(id/xSize)*nodeSize);
}

Vect GridPathSystem::ConvertNodeToPos(int x, int y) const
{
    return gridOffset+Vect(float(x)*nodeSize, 0.0f, float(y)*nodeSize);
}

void GridPathSystem::GetNodeXY(int nodeID, int &x, int &y) const
{
    y = nodeID/xSize;
    x = nodeID%xSize;
}

int GridPathSystem::GetLinkNodeID(int nodeID, int link) const
{
    switch(link)
    {
        case 0:
            return nodeID-xSize-1;
        case 1:
            return nodeID-xSize;
        case 2:
            return (nodeID-xSize)+1;
        case 3:
            return nodeID+1;
        case 4:
            return nodeID+xSize+1;
        case 5:
            return nodeID+xSize;
        case 6:
            return (nodeID+xSize)-1;
        case 7:
            return nodeID-1;
    }

    return -1;
}

float GridPathSystem::CalcH(int idSrc, int idTarget) const
{
    int srcX, srcY, targetX, targetY;
    GetNodeXY(idSrc, srcX, srcY);
    GetNodeXY(idTarget, targetX, targetY);

    return float(abs(targetX-srcX)+abs(targetY-srcY))*nodeSize;
}

unsigned int GridPathSystem::SearchList(List<TraversedGridNode> &list, int id) const
{
    for(int i=0; i<list.Num(); i++)
    {
        if(list[i].id == id)
            return i;
    }
    return INVALID;
}

BOOL GridPathSystem::IsBlockedNode(Entity* ent, int id) const
{
    if(ent)
    {
        GridPathfindingData *pathData = GetEntityGridData(ent);
        if(pathData && pathData->BlockIDs.HasValue(id))
            return FALSE;
    }

    return BlockedNodes[id];
}

bool GridPathSystem::GetClosestOpenNode(int &blocker, const Vect &sourcePos, Vect &targetPos)
{
    traceInFast(GridPathSystem::GetClosestOpenNode);

    BitList CheckedIDs;

    int x, y, i;
    GetNodeXY(blocker, x, y);

    Vect closestPos;
    float closestDist = M_INFINITE;
    bool bFound = false;

    int xStop = MIN(x+4, xSize);
    for(i=MAX(x-4, 0); i<xStop; i++)
    {
        int curID = ((y*xSize)+i);
        if(BlockedNodes[curID] || NodeList[curID] == 0)
            continue;

        Vect pos = ConvertNodeToPos(i, y);
        float dist = pos.Dist(targetPos) + pos.Dist(sourcePos);

        if(dist < closestDist)
        {
            closestDist = dist;
            closestPos  = pos;
            blocker = curID;
            bFound = TRUE;
        }
    }

    int yStop = MIN(y+4, ySize);
    for(i=MAX(y-4, 0); i<yStop; i++)
    {
        int curID = ((i*xSize)+x);
        if(BlockedNodes[curID] || NodeList[curID] == 0)
            continue;

        Vect pos = ConvertNodeToPos(x, i);
        float dist = pos.Dist(targetPos) + pos.Dist(sourcePos);

        if(dist < closestDist)
        {
            closestDist = dist;
            closestPos  = pos;
            blocker = curID;
            bFound = TRUE;
        }
    }

    if(bFound) targetPos = closestPos;

    return bFound;

    traceOutFast;
}

GridAIPath* GridPathSystem::AStar(Entity *ent, const Vect &curTargetPos)
{
    traceIn(GridPathSystem::AStar);

    List<TraversedGridNode> OpenListThingy, ClosedListThingy;
    BitList CheckedIDs;
    Vect targetPos;
    long i;

    //-----------------------------------
    // get start node

    long nNodes = NodeList.Num();
    CheckedIDs.SetSize(nNodes);

    int startID = -1, endID = -1;

    startID = ConvertPosToNode(ent->GetWorldPos());
    endID = ConvertPosToNode(curTargetPos);

    int x, y;
    GetNodeXY(endID, x, y);

    BOOL bAdjustedPos = FALSE;

    if(x >= xSize) {x = xSize-1; bAdjustedPos = TRUE;}
    else if(x < 0) {x = 0;       bAdjustedPos = TRUE;}

    if(y >= ySize) {y = ySize-1; bAdjustedPos = TRUE;}
    else if(y < 0) {y = 0;       bAdjustedPos = TRUE;}

    if(bAdjustedPos)
        targetPos = ConvertNodeToPos(x, y);
    else
        targetPos = curTargetPos;

    endID = ((y*xSize)+x);

    if((startID >= NodeList.Num()) || (endID >= NodeList.Num()))
        return NULL;

    if(BlockedNodes[endID] || NodeList[endID] == 0)
    {
        if(!GetClosestOpenNode(endID, ent->GetWorldPos(), targetPos))
            return NULL;

        bAdjustedPos = TRUE;
    }

    BYTE &startNode = NodeList[startID];
    BYTE &endNode = NodeList[endID];

    //-----------------------------------

    TraversedGridNode &curNode = *OpenListThingy.CreateNew();
    curNode.id = startID;
    curNode.parent = -1;
    curNode.G = 0.0f;

    CheckedIDs.Set(startID);

    BOOL bPathFound = FALSE;

    while(OpenListThingy.Num())
    {
        float bestF = 9e10;
        int bestNodeID = -1;
        TraversedGridNode bestNode;

        for(i=0; i<OpenListThingy.Num(); i++)
        {
            TraversedGridNode &linkNode = OpenListThingy[i];

            float f = linkNode.G + CalcH(linkNode.id, endID);
            if(f < bestF)
            {
                bestF = f;
                bestNodeID = i;
            }
        }

        mcpy(&bestNode, OpenListThingy+bestNodeID, sizeof(TraversedGridNode));

        OpenListThingy.Remove(bestNodeID);
        ClosedListThingy.Add(bestNode);

        BYTE nodeLinks = NodeList[bestNode.id];

        for(i=0; i<8; i++)
        {
            if(!(nodeLinks & (1<<i)))
                continue;

            int linkNodeID = GetLinkNodeID(bestNode.id, i);

            if(linkNodeID == endID)
            {
                TraversedGridNode &newNode = *ClosedListThingy.CreateNew();
                bPathFound = TRUE;
                newNode.id = endID;
                newNode.parent = bestNode.id;
                newNode.G = 0.0f;
                break;
            }

            if(!CheckedIDs[linkNodeID] && !IsBlockedNode(ent, linkNodeID))
            {
                TraversedGridNode &newNode = *OpenListThingy.CreateNew();
                newNode.id = linkNodeID;
                newNode.parent = bestNode.id;
                newNode.G = bestNode.G + ((i&1) ? nodeSize : nodeSizeSquared);

                CheckedIDs.Set(linkNodeID);
            }
            else
            {
                int validLinkNodeID = SearchList(OpenListThingy, linkNodeID);
                if(validLinkNodeID != INVALID)
                {
                    TraversedGridNode &linkNode = OpenListThingy[validLinkNodeID];
                    float cost = ((i&1) ? nodeSize : nodeSizeSquared) + bestNode.G;

                    if(cost < linkNode.G)
                    {
                        linkNode.G = cost;
                        linkNode.parent = bestNode.id;
                    }
                }
            }
        }

        if(bPathFound)
            break;
    }

    CheckedIDs.Clear();
    OpenListThingy.Clear();

    if(bPathFound)
    {
        GridAIPath *newPath = CreateBare(GridAIPath);
        newPath->system = this;
        newPath->ent = ent;
        newPath->bAdjustedTarget = bAdjustedPos;

        TraversedGridNode *curNode = &ClosedListThingy.Last();

        List<int> PathList;

        PathList.Insert(0, curNode->id);

        while(curNode->parent != -1)
        {
            int nextNodeID = SearchList(ClosedListThingy, curNode->parent);
            assert(nextNodeID != INVALID);

            curNode = ClosedListThingy+nextNodeID;

            PathList.Insert(0, curNode->id);
        }

        ClosedListThingy.Clear();

        OptimizePath(newPath, PathList, ent->GetWorldPos(), targetPos);
        newPath->curPos = ent->GetWorldPos();

        return newPath;
    }

    ClosedListThingy.Clear();
    return NULL;

    traceOut;
}

void GridPathSystem::OptimizePath(GridAIPath *path, List<int> &PathList, const Vect &startVec, const Vect &endVec) const
{
    traceIn(GridPathSystem::OptimizePath);

    int i;

    BitList Untouchables;
    Untouchables.SetSize(PathList.Num());

    for(i=0; i<PathList.Num(); i++)
        Untouchables.SetVal(i, IsUncollapsable(path->ent, PathList, i));

    int lastUntouchable;
    int boundsSizeX, boundsSizeY;

    for(i=0; i<PathList.Num(); i++)
    {
        if(Untouchables[i])
            continue;

        int nextUntouchable;
        int tempID = i;

        for(int j=i+1; j<Untouchables.Num(); j++)
        {
            if(Untouchables[j])
            {
                nextUntouchable = PathList[j];
                break;
            }
        }

        for(int j=i-1; j>=0; j--)
        {
            if(Untouchables[j])
            {
                lastUntouchable = PathList[j];
                break;
            }
        }

        int boundsStartX, boundsStartY;
        int boundsEndX, boundsEndY;
        GetNodeXY(lastUntouchable, boundsStartX, boundsStartY);
        GetNodeXY(nextUntouchable, boundsEndX, boundsEndY);

        boundsSizeX = boundsEndX-boundsStartX;
        boundsSizeY = boundsEndY-boundsStartY;

        bool bUnder = false;

        do
        {
            if(Untouchables[tempID])
                break;

            int cornerID;
            bool bBreakOut = false;
            switch(IsCollapsableCorner(path->ent, PathList, tempID, cornerID))
            {
                case UncollapsableItem:
                    bBreakOut = true;
                    Untouchables.Set(tempID);
                    break;
                case StraightPath:
                    break;
                case CollapsableCorner:
                    {
                        if(boundsSizeX == 0)
                        {
                            bBreakOut = true;
                            break;
                        }

                        int x, y;
                        GetNodeXY(cornerID, x, y);
                        x -= boundsStartX; y -= boundsStartY;
                        int maxY = (x*boundsSizeY)/boundsSizeX;
                        if(tempID == i)
                        {
                            bUnder = (y <= maxY);
                            PathList[tempID] = cornerID;
                        }
                        else if((y <= maxY) == bUnder)
                            PathList[tempID] = cornerID;
                        else
                        {
                            bBreakOut = true;
                            break;
                        }
                    }
            }

            if(bBreakOut)
                break;

        }while(tempID--);
    }

    for(i=0; i<Untouchables.Num(); i++)
    {
        if(i == 0)
            path->Path << startVec;
        else if(i == (Untouchables.Num()-1))
        {
            Vect adjEndVec = endVec;
            adjEndVec.y = startVec.y;
            path->Path << adjEndVec;
        }
        else if(Untouchables[i])
        {
            Vect pathNode = ConvertNodeToPos(PathList[i]);
            pathNode.y = startVec.y;
            path->Path << pathNode;
        }
    }

    traceOut;
}

bool GridPathSystem::IsUncollapsable(Entity *ent, const List<int> &PathList, int pathListID) const
{
    traceInFast(GridPathSystem::IsUncollapsable);

    if((pathListID == 0) || (pathListID == (PathList.Num()-1))) //end nodes should always be added
        return true;

    int id0 = PathList[pathListID-1];
    int id1 = PathList[pathListID];
    int id2 = PathList[pathListID+1];

    int ID1mID0 = (id1-id0);
    int ID2mID1 = (id2-id1);

    if(ID1mID0 == ID2mID1) //straight line.  weird, I know, but they're IDs for a rectangular array so it works
        return false;

    int centerID = INVALID;

    if((abs(ID1mID0) == 1) || (abs(ID1mID0) == xSize))
        centerID = (id2-ID1mID0);
    else if((abs(ID2mID1) == 1) || (abs(ID2mID1) == xSize))
        centerID = (id0+ID2mID1);

    if(centerID == INVALID || centerID >= NodeList.Num())
        return true;

    if(IsBlockedNode(ent, centerID))
        return true;

    BYTE centerVal = NodeList[centerID];

    for(int i=0; i<8; i++)
    {
        if(centerVal & (1<<i))
        {
            int linkNodeID = GetLinkNodeID(centerID, i);

            if(IsBlockedNode(ent, linkNodeID))
                continue;

            if(linkNodeID == id0)
                id0 = INVALID;
            else if(linkNodeID == id1)
                id1 = INVALID;
            else if(linkNodeID == id2)
                id2 = INVALID;
        }
    }

    return ((id0 & id1 & id2) != INVALID);

    traceOutFast;
}

unsigned int GridPathSystem::IsCollapsableCorner(Entity *ent, const List<int> &PathList, int pathListID, int &centerID) const //returns swap id or INVALID if failed
{
    traceInFast(GridPathSystem::IsCollapsableCorner);

    if((pathListID == 0) || (pathListID == (PathList.Num()-1)))
        return UncollapsableItem;

    int id0 = PathList[pathListID-1];
    int id1 = PathList[pathListID];
    int id2 = PathList[pathListID+1];

    int ID1mID0 = (id1-id0);
    int ID2mID1 = (id2-id1);

    if(ID1mID0 == ID2mID1) //straight line.  weird, I know, but they're IDs for a rectangular array so it works
        return StraightPath;

    int centerIDTemp = INVALID;

    if((abs(ID1mID0) == 1) || (abs(ID1mID0) == xSize))
        centerIDTemp = (id2-ID1mID0);
    else if((abs(ID2mID1) == 1) || (abs(ID2mID1) == xSize))
        centerIDTemp = (id0+ID2mID1);

    if(centerIDTemp == INVALID)
        return UncollapsableItem;

    if(IsBlockedNode(ent, centerIDTemp))
        return UncollapsableItem;

    BYTE centerVal = NodeList[centerIDTemp];

    for(int i=0; i<8; i++)
    {
        if(centerVal & (1<<i))
        {
            int linkNodeID = GetLinkNodeID(centerIDTemp, i);

            if(IsBlockedNode(ent, linkNodeID))
                continue;

            if(linkNodeID == id0)
                id0 = INVALID;
            else if(linkNodeID == id1)
                id1 = INVALID;
            else if(linkNodeID == id2)
                id2 = INVALID;
        }
    }

    if((id0 & id1 & id2) == INVALID)
    {
        centerID = centerIDTemp;
        return CollapsableCorner;
    }
    else
        return UncollapsableItem;

    traceOutFast;
}


GridPathSystem::GridPathSystem(const Vect &mainOffset, float nodeLength, int width, int length)
{
    gridOffset = mainOffset;
    nodeSize = nodeLength;
    nodeSizeSquared = Vect2(nodeSize, nodeSize).Len();
    xSize = width; ySize = length;
}

void GridPathSystem::AddBlocker(Entity *ent)
{
    traceInFast(GridPathSystem::AddBlocker);

    /*int i=0;

    if(ent->IsOf(GetClass(Prefab)))
        return;

    int closestID = ConvertPosToNode(ent->GetWorldPos());
    if(closestID >= NodeList.Num())
        return;

    GridPathfindingData *data = CreateObject(GridPathfindingData);

    data->BlockIDs << closestID;

    List<int> CheckList;
    CheckList << closestID;
    BlockedNodes.Set(CheckList[0]);
    data->BlockIDs << CheckList[0];

    //Implement Collision (fix this stuff somehow)
    while(CheckList.Num())
    {
        int checkCount = CheckList.Num();

        while(checkCount--)
        {
            for(int i=0; i<8; i++)
            {
                if(NodeList[CheckList[0]] & (1<<i))
                {
                    int linkID = GetLinkNodeID(CheckList[0], i);
                    if(BlockedNodes[linkID])
                        continue;

                    Vect pos = ConvertNodeToPos(linkID);
                    if(ent->PrimitiveInside(pos+Vect(0.0f, 3.01f, 0.0f), CylinderCollision, nodeSize*0.5f, 3.0f))
                    {
                        CheckList << linkID;
                        BlockedNodes.Set(linkID);
                        data->BlockIDs << linkID;
                    }
                }
            }
            CheckList.Remove(0);
        }
    }

    SetEntityPathData(ent, data);*/

    traceOutFast;
}

void GridPathSystem::RemoveBlocker(Entity *ent)
{
    traceInFast(GridPathSystem::RemoveBlocker);

    /*GridPathfindingData *data = GetEntityGridData(ent);

    if(ent->IsOf(GetClass(Prefab)))
        return;

    if(data)
    {
        for(int i=0; i<data->BlockIDs.Num(); i++)
            BlockedNodes.Clear(data->BlockIDs[i]);

        SetEntityPathData(ent, NULL);
    }*/

    traceOutFast;
}

AIPath* GridPathSystem::GetEntityPath(Entity *ent, const Vect &targetPos)
{
    return AStar(ent, targetPos);
}

void GridPathSystem::Serialize(Serializer &s)
{
    s << gridOffset
      << nodeSize << nodeSizeSquared
      << xSize << ySize
      << NodeList;
    BlockedNodes.SetSize(NodeList.Num());
}

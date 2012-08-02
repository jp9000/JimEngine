/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Triangulator.cpp

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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#include "Xed.h"


inline float GetAngleValue(float angle)
{
    float val = acos(fabsf(angle))/M_PI;
    
    return val;
}

inline Vect2 Convert3DTo2D(Vect &p, DWORD axis, BOOL bNegative)
{
    Vect2 val;

    switch(axis)
    {
        case 0:
            val.Set(p.z, p.y);
            break;

        case 1:
            val.Set(p.x, p.z);
            break;

        case 2:
            val.Set(-p.x, p.y);
            break;
    }

    if(bNegative)
        val.x = -val.x;

    return val;
}

extern int blablabla;

void  Triangulator::Trianglulate3DData(const Plane &plane, const List<PolyLine> &Lines3D, const List<Vect> &Verts3D)
{
    DWORD i, j;

    float bestAngle = -1.0f;
    DWORD axis;
    BOOL  isNegative;

    for(i=0; i<3; i++)
    {
        Vect curDir;
        zero(&curDir, sizeof(Vect));

        for(j=0; j<2; j++)
        {
            curDir.ptr[i] = j ? -1.0f : 1.0f;
            float angle = plane.Dir.Dot(curDir);

            if(angle > bestAngle)
            {
                axis = i;
                isNegative = j;

                bestAngle = angle;
            }
        }
    }

    List<DWORD> OldIDs;

    Lines.CopyList(Lines3D);

    for(i=0; i<Lines.Num(); i++)
    {
        DWORD &v1 = Lines[i].v1;
        DWORD &v2 = Lines[i].v2;

        DWORD newV1 = OldIDs.FindValueIndex(v1);
        DWORD newV2 = OldIDs.FindValueIndex(v2);

        if(newV1 == INVALID)
        {
            newV1 = Verts.Add(Convert3DTo2D(Verts3D[v1], axis, isNegative));
            OldIDs.Add(v1);
        }
        if(newV2 == INVALID)
        {
            newV2 = Verts.Add(Convert3DTo2D(Verts3D[v2], axis, isNegative));
            OldIDs.Add(v2);
        }

        v1 = newV1;
        v2 = newV2;
    }

    Triangulate();

    for(i=0; i<Faces.Num(); i++)
    {
        Face &face = Faces[i];
        face.A = OldIDs[face.A];
        face.B = OldIDs[face.B];
        face.C = OldIDs[face.C];
    }
}

void  Triangulator::Triangulate()
{
    assert(Verts.Num());

    if(!Verts.Num())
        return;

    DWORD i;

    //if no lines available, build lines list from vert list
    if(!Lines.Num())
    {
        Lines.SetSize(Verts.Num());

        for(i=0; i<Verts.Num(); i++)
        {
            PolyLine &line = Lines[i];
            int ip1 = (i == Verts.Num()-1) ? 0 : (i+1);

            line.v1 = i;
            line.v2 = ip1;
        }
    }

    List<ChibiLoopNode> ChibiLoopTree;
    List<LoopVerts> LoopList;

    DWORD curLoop=0;

    //-----------------------------------------------------------
    // find loops!

	if(blablabla == 1)
		blablabla = 0;

    List<DWORD> LineIDs;
    LineIDs.SetSize(Lines.Num());
    for(i=0; i<LineIDs.Num(); i++) LineIDs[i] = i;

    while(LineIDs.Num())
    {
        LoopVerts &CurLoop = *LoopList.CreateNew();
        DWORD lastLine = 0;

        CurLoop.Clear();

        while(TRUE)
        {
            PolyLine &line1 = Lines[LineIDs[lastLine]];
            LineIDs.Remove(lastLine);

            CurLoop << line1;

            BOOL bNoResults = TRUE;

            if(line1.v2 == CurLoop[0].v1)
                break;

            for(i=0; i<LineIDs.Num(); i++)
            {
                PolyLine &line2 = Lines[LineIDs[i]];
                if((line2.v1 == line1.v2) && (line2.v2 != line1.v1) && (!line2.lineData || (line1.lineData == line2.lineData)))
                {
                    lastLine = i;
                    line2.lineData = line1.lineData;
                    bNoResults = FALSE;
                    break;
                }
            }

            if(bNoResults)
            {
                AppWarning(TEXT("Invalid Loop data sent to triangulation."));
                LineIDs.Clear();
                break;
            }
        }
    }

    //-----------------------------------------------------------
    // sort loops!

    List<DWORD> LoopRefs;

    LoopRefs.SetSize(LoopList.Num());
    for(i=0; i<LoopList.Num(); i++) LoopRefs[i] = i;

    CreateChibiLoopTree(ChibiLoopTree, LoopRefs, LoopList);

    LoopRefs.Clear();

    //-----------------------------------------------------------
    // connect loops

    //SpreadLoopData(ChibiLoopTree, LoopList, 0);

    List<LoopVerts> NewLoopList;

    for(i=0; i<ChibiLoopTree.Num(); i++)
        ConnectChibiLoops(ChibiLoopTree[i], LoopList, NewLoopList);

    DestroyChibiLoopTree(ChibiLoopTree);

    for(int i=0; i<LoopList.Num(); i++)
        LoopList[i].Clear();
    LoopList.Clear();

    LoopList.SetSize(NewLoopList.Num());
    for(i=0; i<NewLoopList.Num(); i++)
    {
        LoopList[i].CopyList(NewLoopList[i]);
        NewLoopList[i].Clear();
    }
    NewLoopList.Clear();

    for(i=0; i<LoopList.Num(); i++)
    {
        LoopVerts &loop = LoopList[i];
        PolyLine &firstLine = loop[0];

        for(int j=1; j<loop.Num(); j++)
        {
            PolyLine &line = loop[j];
            line.lineData = firstLine.lineData;
        }
    }

    //-----------------------------------------------------------
    // triangulate!

    for(i=0; i<LoopList.Num(); i++)
        TriangulateShape(LoopList[i]);

    //-----------------------------------------------------------
    // clean up!

    Lines.Clear();

    for(i=0; i<LoopList.Num(); i++)
    {
        Lines.AppendList(LoopList[i]);
        LoopList[i].Clear();
    }
    LoopList.Clear();
}

void  Triangulator::TriangulateShape(List<PolyLine> &ShapeData)
{
    List<int> shapeRefs;
    int i;

    shapeRefs.SetSize(ShapeData.Num());
    for(i=0; i<shapeRefs.Num(); i++)
    {
        shapeRefs[i] = i;
        /*String chong;
        chong << long(i) << TEXT(": x: ") << formattedNumber(Verts[ShapeData[i].v1].x, TEXT("%0.4f") << TEXT("\ty: ") << formattedNumber(Verts[ShapeData[i].v1].y, TEXT("%0.4f") << TEXT("\r\n");
        OutputDebugString(chong);*/
    }

    //Faces.Clear();

    //------------------------------------
    // find best triangle

    while(shapeRefs.Num() > 3) //(shapeRefs.Num() > 3)
    {
        int bestTri = INVALID;
        float bestValue = 0.0f;

        Vect2 vP = Verts[ShapeData[shapeRefs[0]].v1];
        Vect2 v1 = Verts[ShapeData[shapeRefs[1]].v1];
        Vect2 v2 = Verts[ShapeData[shapeRefs[2]].v1];

        BOOL bBreak = FALSE;

        i = 3;

        while(!bBreak)
        {
            bBreak = (i == 1);

            DWORD tri = i-1;

            if(i == shapeRefs.Num())
                i = 0;

            DWORD ip1 = (i == shapeRefs.Num()-1) ? 0 : (i+1);

            Vect2 v3 = Verts[ShapeData[shapeRefs[i]].v1];
            Vect2 v4 = Verts[ShapeData[shapeRefs[ip1]].v1];
 
            Vect2 line1Norm = (v1-v2).Norm().GetCross();
            Vect2 line2Norm = (v2-v3).Norm();

            //triangle candidate
            if(line1Norm.Dot(line2Norm) > 0.0f)
            {
                if(!LineIntersectsRefs(v1, v3, shapeRefs, ShapeData))
                {
                    if(!PointInsideTri(v1, v2, v3, vP) && !PointInsideTri(v1, v2, v3, v4))
                    {
                        Vect2 n1 = (v1-v2).Norm();
                        Vect2 n2 = (v3-v1).Norm();
                        Vect2 n3 = (v2-v3).Norm();
                       // Vect2 n4 = (vP-v1).Norm();
                        //Vect2 n5 = (v4-v3).Norm();

                        float angle1 = n1.Dot(n3);
                        float angle2 = n2.Dot(n1);
                        float angle3 = n3.Dot(n2);
                        //float angle4 = n4.Dot(n2);
                        //float angle5 = n5.Dot(-n2);

                        float val = GetAngleValue(angle1)*
                                    GetAngleValue(angle2)*
                                    GetAngleValue(angle3);

                        if(val > bestValue)
                        {
                            bestTri = tri;
                            bestValue = val;
                        }
                    }
                }
            }

            //------
            vP = v1;
            v1 = v2;
            v2 = v3;
            ++i;
        }

        if(bestTri == INVALID)
            return;

        int prevID = (bestTri == 0) ? (shapeRefs.Num()-1) : bestTri-1;
        int nextID = (bestTri == (shapeRefs.Num()-1)) ? 0 : bestTri+1;

        Face face;
        face.A = ShapeData[shapeRefs[prevID]].v1;
        face.B = ShapeData[shapeRefs[bestTri]].v1;
        face.C = ShapeData[shapeRefs[nextID]].v1;

        Faces << face;
        FaceData << ShapeData[0].lineData;

        shapeRefs.Remove(bestTri);
    }

    Face lastFace = {ShapeData[shapeRefs[0]].v1, ShapeData[shapeRefs[1]].v1, ShapeData[shapeRefs[2]].v1};
    Faces << lastFace;
    FaceData << ShapeData[0].lineData;
}

BOOL  Triangulator::LineIntersectsRefs(Vect2 &v1, Vect2 &v2, List<int> &FaceRefs, List<PolyLine> &ShapeData)
{
    Line2 line1(v1, v2);

    if(LineIntersectsShape(v1, v2, ShapeData))
        return TRUE;

    for(int i=0; i<FaceRefs.Num(); i++)
    {
        Line2 line2(Verts[ShapeData[FaceRefs[i]].v1], Verts[ShapeData[FaceRefs[i]].v2]);

        if(line1.LinesIntersect(line2))
            return TRUE;
    }

    return FALSE;
}

BOOL  Triangulator::LineIntersectsShape(Vect2 &v1, Vect2 &v2, List<PolyLine> &CurShapeRefs)
{
    Line2 line1(v1, v2);

    for(int i=0; i<CurShapeRefs.Num(); i++)
    {
        PolyLine &PolyLine = CurShapeRefs[i];
        Line2 line2(Verts[PolyLine.v1], Verts[PolyLine.v2]);

        if(line1.LinesIntersect(line2))
            return TRUE;
    }

    return FALSE;
}

void  Triangulator::CreateChibiLoopTree(List<ChibiLoopNode> &LoopNodeList, List<DWORD> &LoopRefs, List<LoopVerts> &LoopList)
{
    int i, j;

    // find top loops

    /*for(i=0; i<LoopRefs.Num(); i++)
    {
        LoopVerts &loop1 = LoopList[LoopRefs[i]];
        BOOL bTopLoop = TRUE;
        //Vect2 &testVert = Verts[LoopList[LoopRefs[i]][0].v1];

        List<DWORD> ChildRefs;

        for(j=0; j<LoopRefs.Num(); j++)
        {
            if(j == i)
                continue;

            LoopVerts &loop2 = LoopList[LoopRefs[j]];

            DWORD val = LoopInsideLoop(loop2, loop1);

            if(val == 2)
            {
                if(!ChibiLoopFacingUp(loop2))
                    val = 1;
            }

            if(val == 1)
            {
                bTopLoop = FALSE;
                break;
            }
            else if(LoopInsideLoop(loop1, loop2))
                ChildRefs << LoopRefs[j];
        }

        if(bTopLoop)
        {
            ChibiLoopNode &chibiLoopNode = *LoopNodeList.CreateNew();
            chibiLoopNode.loop = LoopRefs[i];

            CreateChibiLoopTree(chibiLoopNode.Children, ChildRefs, LoopList);
        }
    }*/

    for(i=0; LoopRefs.Num(); i++)  //works like a while(LoopRefs.Num())
    {
        LoopVerts &loop1 = LoopList[LoopRefs[i]];
        BOOL bTopLoop = TRUE;

        List<DWORD> ChildRefs;

        ChildRefs.Clear();

        for(j=0; j<LoopRefs.Num(); j++)
        {
            if(j == i)
                continue;

            LoopVerts &loop2 = LoopList[LoopRefs[j]];

            DWORD val = LoopInsideLoop(loop2, loop1);

            if(val == 2)
            {
                if(!ChibiLoopFacingUp(loop2))
                    val = 1;
            }

            if(val == 1)
            {
                bTopLoop = FALSE;
                break;
            }
            else
            {
                DWORD val2 = LoopInsideLoop(loop1, loop2);
                if((val2 == 1) || ((val2 & val) == 2))
                    ChildRefs << LoopRefs[j];
            }
        }

        if(bTopLoop)
        {
            ChibiLoopNode &chibiLoopNode = *LoopNodeList.CreateNew();
            chibiLoopNode.loop = LoopRefs[i];

            LoopRefs.Remove(i);

            if(ChildRefs.Num())
            {
                for(j=0; j<ChildRefs.Num(); j++)
                    LoopRefs.RemoveItem(ChildRefs[j]);

                CreateChibiLoopTree(chibiLoopNode.Children, ChildRefs, LoopList);
            }

            i=-1; //because it'll do the i++
        }
    }
}

void  Triangulator::ConnectChibiLoops(ChibiLoopNode &loopNode, List<LoopVerts> &LoopList, List<LoopVerts> &NewLoopList)
{
    DWORD i, j, k, l;

    List<PolyLine> CurLoop;
    CurLoop.CopyList(LoopList[loopNode.loop]);

    if(!ChibiLoopFacingUp(CurLoop))
        ReverseLoop(CurLoop);

    List<DWORD> AlreadyUsed;

    for(i=0; i<loopNode.Children.Num(); i++)
    {
        ChibiLoopNode &childNode = loopNode.Children[i];
        List<PolyLine> &childLoop = LoopList[childNode.loop];

        if(ChibiLoopFacingUp(childLoop))
            ReverseLoop(childLoop);
    }

    List<DWORD> CurChildren;
    CurChildren.SetSize(loopNode.Children.Num());
    for(i=0; i<loopNode.Children.Num(); i++)
        CurChildren[i] = i;

    i = 0;

    while(CurChildren.Num())
    {
        ChibiLoopNode &childNode = loopNode.Children[CurChildren[i]];
        List<PolyLine> &childLoop = LoopList[childNode.loop];

        //---------------------------------------------
        // find closest points between loops

        DWORD curBestChoiceLoop1 = INVALID;
        DWORD curBestChoiceLoop2 = INVALID;
        float closestPos = 0.0f;

        for(j=0; j<CurLoop.Num(); j++)
        {
            if(AlreadyUsed.FindValueIndex(CurLoop[j].v1) != INVALID)
                continue;

            Vect2 &v1 = Verts[CurLoop[j].v1];

            for(k=0; k<childLoop.Num(); k++)
            {
                if(AlreadyUsed.FindValueIndex(childLoop[k].v1) != INVALID)
                    continue;

                Vect2 &v2 = Verts[childLoop[k].v1];

                BOOL bBadChoice = LineIntersectsShape(v1, v2, CurLoop);

                if(bBadChoice)
                    continue;

                for(l=0; l<CurChildren.Num(); l++)
                {
                    if(LineIntersectsShape(v1, v2, LoopList[loopNode.Children[CurChildren[l]].loop]))
                    {
                        bBadChoice = TRUE;
                        break;
                    }
                }

                if(bBadChoice)
                    continue;

                float dist = v2.Dist(v1);

                if((curBestChoiceLoop1 != INVALID) && (dist > closestPos))
                    continue;

                closestPos = dist;
                curBestChoiceLoop1 = j;
                curBestChoiceLoop2 = k;
            }
        }

        if(curBestChoiceLoop1 == INVALID)
        {
            i = (i == CurChildren.Num()-1) ? 0 : (i+1);
            if(i == 0)
            {
                AppWarning(TEXT("...almost infinitely looped there.  Fortunately I have measures against such devious things."));
                break;
            }
            continue;
        }

        AlreadyUsed << CurLoop[curBestChoiceLoop1].v1;
        AlreadyUsed << childLoop[curBestChoiceLoop2].v1;

        //---------------------------------------------
        // connect loops

        if(CurLoop[0].lineData && !childLoop[0].lineData)
        {
            for(j=0; j<childLoop.Num(); j++)
                childLoop[j].lineData = CurLoop[0].lineData;
        }

        SetNewLoopStartPosition(CurLoop,   curBestChoiceLoop1);
        SetNewLoopStartPosition(childLoop, curBestChoiceLoop2);

        if(!CurLoop[0].lineData && childLoop[0].lineData)
        {
            for(j=0; j<CurLoop.Num(); j++)
                CurLoop[j].lineData = childLoop[0].lineData;
        }

        PolyLine &lastLine = CurLoop.Last();
        PolyLine *pLine = CurLoop.CreateNew();
        pLine->v1 = lastLine.v2;
        pLine->v2 = childLoop[0].v1;

        CurLoop.AppendList(childLoop);
        pLine = CurLoop.CreateNew();
        pLine->v1 = childLoop[0].v1;
        pLine->v2 = CurLoop[0].v1;

        /*for(j=0; j<CurLoop.Num(); j++)
        {
            String chong;
            chong << long(j) << TEXT(": x: ") << FormattedString(TEXT("%0.4f"), Verts[CurLoop[j].v1].x) << TEXT("\ty: ") << FormattedString(TEXT("%0.4f"), Verts[CurLoop[j].v1].y) << TEXT("\r\n");
            OutputDebugString(chong);
        }
        OutputDebugString(TEXT("====================================\r\n"));*/

        CurChildren.Remove(i);
        i = 0;
    }

    NewLoopList.CreateNew()->CopyList(CurLoop);
    CurLoop.Clear();

    for(i=0; i<loopNode.Children.Num(); i++)
    {
        ChibiLoopNode &childNode = loopNode.Children[i];

        for(j=0; j<childNode.Children.Num(); j++)
            ConnectChibiLoops(childNode.Children[j], LoopList, NewLoopList);
    }
}

void  Triangulator::SetNewLoopStartPosition(List<PolyLine> &Loop, DWORD pos)
{
    List<PolyLine> NewLoop;
    NewLoop.SetSize(Loop.Num());

    for(DWORD i=0; i<Loop.Num(); i++)
    {
        DWORD newPos = i+pos;

        if(newPos >= Loop.Num())
            newPos -= Loop.Num();

        NewLoop[i] = Loop[newPos];
    }

    Loop.CopyList(NewLoop);
}

void  Triangulator::ReverseLoop(List<PolyLine> &Loop)
{
    List<PolyLine> NewLoop;
    NewLoop.SetSize(Loop.Num());

    for(int i=Loop.Num()-1, j=0; i>=0; i--, j++)
    {
        NewLoop[i].v1 = Loop[j].v2;
        NewLoop[i].v2 = Loop[j].v1;
        NewLoop[i].lineData = Loop[j].lineData;
    }

    Loop.CopyList(NewLoop);
}

BOOL  Triangulator::ChibiLoopFacingUp(List<PolyLine> &Loop)
{
    DWORD i;

    /*PolyLine line1 = Loop.Last();

    long val = 0;

    for(i=0; i<Loop.Num(); i++)
    {
        PolyLine &line2 = Loop[i];

        Vect2 v1 = (Verts[line1.v1]-Verts[line1.v2]);
        Vect2 v2 = (Verts[line2.v2]-Verts[line2.v1]);

        float lineVal = (v1.x*v2.y)-(v1.y*v2.x);

        if(lineVal > 0.0f)
            val++;
        else if(lineVal < 0.0f)
            val--;

        line1 = line2;
    }*/

    /*float bestX = Verts[Loop[0].v1].x;
    DWORD bestLine = 0;

    for(i=1; i<Loop.Num(); i++)
    {
        float testX = Verts[Loop[i].v1].x;
        if(testX < bestX)
        {
            bestX = testX;
            bestLine = i;
        }
    }

    PolyLine line1 = (bestLine == 0) ? Loop.Last() : Loop[bestLine-1];
    PolyLine line2 = Loop[bestLine];
    
    //----------------------------------

    Vect2 vec1 = (Verts[line1.v1]-Verts[line1.v2]);
    Vect2 vec2 = (Verts[line2.v2]-Verts[line2.v1]);

    return ((vec1.x*vec2.y)-(vec1.y*vec2.x)) > 0.0f;*/
    

    //----------------------------------

    List<DWORD> ISAUSELESSPEICEOFCRAP;

    ISAUSELESSPEICEOFCRAP.SetSize(Loop.Num());

    while(1)
    {
        float bestX = Verts[Loop[0].v1].x;
        DWORD bestLine = INVALID;

        for(i=0; i<Loop.Num(); i++)
        {
            float testX = Verts[Loop[i].v1].x;
            if((bestLine == INVALID) || (testX < bestX))
            {
                if(!ISAUSELESSPEICEOFCRAP[i])
                {
                    bestX = testX;
                    bestLine = i;
                }
            }
        }

        if(bestLine == INVALID)
        {
            AppWarning(TEXT("Scream."));
            return FALSE;
        }

        PolyLine line1 = (bestLine == 0) ? Loop.Last() : Loop[bestLine-1];
        PolyLine line2 = Loop[bestLine];

        //----------------------------------

        Vect2 vec1 = (Verts[line1.v1]-Verts[line1.v2]);
        Vect2 vec2 = (Verts[line2.v2]-Verts[line2.v1]);

        float theresult = ((vec1.x*vec2.y)-(vec1.y*vec2.x));

        if(CloseFloat(theresult, 0.0f, EPSILON))
            ISAUSELESSPEICEOFCRAP[bestLine] = TRUE;
        else
            return theresult > 0.0f;
    }
}

void  Triangulator::DestroyChibiLoopTree(List<ChibiLoopNode> &LoopNodeList)
{
    for(int i=0; i<LoopNodeList.Num(); i++)
    {
        ChibiLoopNode &loopNode = LoopNodeList[i];

        if(loopNode.Children.Num())
            DestroyChibiLoopTree(loopNode.Children);
    }
    LoopNodeList.Clear();
}

void Triangulator::SpreadLoopData(List<ChibiLoopNode> &LoopNodeList, List<LoopVerts> &LoopList, DWORD data)
{
    for(int i=0; i<LoopNodeList.Num(); i++)
    {
        List<PolyLine> &loop = LoopList[LoopNodeList[i].loop];
        if(!loop[0].lineData)
        {
            for(int j=0; j<loop.Num(); j++)
                loop[j].lineData = data;
        }

        SpreadLoopData(LoopNodeList[i].Children, LoopList, loop[0].lineData);
    }
}

//0 = OUTSIDE, 1 = INSIDE, 2 = EQUAL

DWORD Triangulator::LoopInsideLoop(List<PolyLine> &Loop, List<PolyLine> &LoopInside) const
{
    DWORD unsharedVert = FindUnsharedVert(LoopInside, Loop);

    if(unsharedVert == INVALID)
        return 2;

    return PointInsideLoop(Loop, Verts[unsharedVert]);
}

DWORD Triangulator::FindUnsharedVert(List<PolyLine> &Loop1, List<PolyLine> &Loop2) const
{
    for(int i=0; i<Loop1.Num(); i++)
    {
        DWORD vert1 = Loop1[i].v1;
        BOOL bFoundVert = FALSE;

        for(int j=0; j<Loop2.Num(); j++)
        {
            if(vert1 == Loop2[j].v1)
            {
                bFoundVert = TRUE;
                break;
            }
        }

        if(!bFoundVert)
            return vert1;
    }

    return INVALID;
}

BOOL  Triangulator::PointInsideLoop(List<PolyLine> &Loop, const Vect2 &point) const
{
    BOOL bInside = FALSE;

    for(int i=0; i<Loop.Num(); i++)
    {
        Line2 edge(Verts[Loop[i].v1], Verts[Loop[i].v2]);

        BOOL yAbove1 = (edge.A.y >= point.y);
        BOOL yAbove2 = (edge.B.y >= point.y);

        if(yAbove1 != yAbove2)
        {
            float c0 = (edge.B.y - point.y) * (edge.A.x - edge.B.x);
            float c1 = (edge.B.x - point.x) * (edge.A.y - edge.B.y);

            if((c0 >= c1) == yAbove2)
                bInside = !bInside;
        }
    }

    return bInside;
}

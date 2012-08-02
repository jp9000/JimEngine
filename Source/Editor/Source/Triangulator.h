/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Triangulator.h

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


#pragma once

struct PolyLine
{
    union
    {
        struct {DWORD v1, v2;};
        DWORD p[2];
    };

    DWORD lineData;
};

class Triangulator
{
public:
    List<PolyLine>  Lines;
    List<Vect2>     Verts;
    List<Face>      Faces;
    List<DWORD>     FaceData;

    typedef List<PolyLine> LoopVerts;

    struct ChibiLoopNode
    {
        List<ChibiLoopNode> Children;
        DWORD loop;
    };

    void Trianglulate3DData(const Plane &plane, const List<PolyLine> &Lines3D, const List<Vect> &Verts3D);
    void Triangulate();

private:
    void TriangulateShape(List<PolyLine> &ShapeData);
    BOOL LineIntersectsRefs(Vect2 &v1, Vect2 &v2, List<int> &FaceRefs, List<PolyLine> &ShapeData);
    BOOL LineIntersectsShape(Vect2 &v1, Vect2 &v2, List<PolyLine> &CurShapeRefs);
    void CreateChibiLoopTree(List<ChibiLoopNode> &LoopNodeList, List<DWORD> &LoopRefs, List<LoopVerts> &LoopList);
    void ConnectChibiLoops(ChibiLoopNode &loopNode, List<LoopVerts> &LoopList, List<LoopVerts> &NewLoopList);
    void SetNewLoopStartPosition(List<PolyLine> &Loop, DWORD pos);
    void ReverseLoop(List<PolyLine> &Loop);
    BOOL ChibiLoopFacingUp(List<PolyLine> &Loop);
    void DestroyChibiLoopTree(List<ChibiLoopNode> &LoopNodeList);
    //0 = OUTSIDE, 1 = INSIDE, 2 = EQUAL

    void SpreadLoopData(List<ChibiLoopNode> &LoopNodeList, List<LoopVerts> &LoopList, DWORD data);

    DWORD LoopInsideLoop(List<PolyLine> &Loop, List<PolyLine> &LoopInside) const;
    DWORD FindUnsharedVert(List<PolyLine> &Loop1, List<PolyLine> &Loop2) const;
    BOOL PointInsideLoop(List<PolyLine> &Loop, const Vect2 &point) const;
};




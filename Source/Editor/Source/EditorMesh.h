/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditorMesh.h

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


#define LINEINSIDE  1
#define LINEOUTSIDE 2

struct PolyData
{
    //List<DWORD>     Verts;
    List<PolyLine>  Lines;
    List<PolyLine>  NewLines;
    List<Vect>      NewVerts;
    List<UVCoord>   NewUVs;
    List<Vect>      NewNorms;

    inline void Clear()
    {
        Lines.Clear();
        NewLines.Clear();
        NewVerts.Clear();
        NewNorms.Clear();
        NewUVs.Clear();
    }

    inline DWORD FindNewLine(DWORD v1, DWORD v2, DWORD startLine=0)
    {
        for(DWORD i=startLine; i<NewLines.Num(); i++)
        {
            PolyLine &line = NewLines[i];

            if( ((line.v1 == v1) && (line.v2 == v2)) ||
                ((line.v1 == v2) && (line.v2 == v1)) )
            {
                return i;
            }
        }

        return INVALID;
    }

    inline DWORD FindLine(DWORD v1, DWORD v2)
    {
        for(DWORD i=0; i<Lines.Num(); i++)
        {
            PolyLine &line = Lines[i];

            if( ((line.v1 == v1) && (line.v2 == v2)) ||
                ((line.v1 == v2) && (line.v2 == v1)) )
            {
                return i;
            }
        }

        return INVALID;
    }
};

struct PolyEdgeData
{
    List<DWORD> PolyEdges;
};


struct PolyInfo
{
    DWORD originalPoly;
    BOOL bDoPlanerTest;
    BOOL bLiesOnEdge;
};


struct PolyIntersectData
{
    DWORD polyID;
    float dist;
    Vect  vert;
    DWORD lineSides;
};


struct AddLMPolyInfo
{
    inline AddLMPolyInfo(UINT curPolyIn, float maxAngleIn, Vect &mapDirIn, List<UINT> &SectionPolysIn, List<Vect> &PolyLinesIn, List<UINT> &UnprocessedPolysIn, BitList &ProcessedPolysIn, BitList &ProcessedEdgesIn)
    : curPoly(curPolyIn), maxAngle(maxAngleIn), mapDir(mapDirIn), SectionPolys(SectionPolysIn), PolyLines(PolyLinesIn), UnprocessedPolys(UnprocessedPolysIn), ProcessedPolys(ProcessedPolysIn), ProcessedEdges(ProcessedEdgesIn)
    {}

    UINT curPoly;
    float maxAngle;
    Vect &mapDir;
    List<UINT> &SectionPolys;
    List<UINT> &UnprocessedPolys;
    List<Vect> &PolyLines;
    BitList &ProcessedPolys;
    BitList &ProcessedEdges;
};



struct EditorMesh
{
    List<Plane>     PlaneList;

    List<Vect>      VertList;
    List<UVCoord>   UVList;
    List<UVCoord>   LMUVList;
    List<Vect>      NormalList;
    List<Vect>      TangentList; //if any

    List<Face>      FaceList;
    List<DWORD>     FacePlaneList;
    List<DWORD>     FacePolyList;  //this will cause an inoptimal mesh if used unwisely
    List<DWORD>     FaceSmoothList;

    List<IDList>    FaceCuts;

    List<Bounds>    PolyBounds;

    List<PolyFace>  PolyList;

    List<PolyInfo>  PolyInfoList;  //god I make myself seem so retarded.  probably for a good reason.
    List<DWORD>     FaceSideList;

    List<Edge>      EdgeList;
    List<Edge>      PolyEdgeList;
    List<PolyEdgeData> PolyEdgeDataList;

    DWORD           originalVertNum;
    DWORD           originalFaceNum;
    DWORD           originalPolyNum;

    EditorMesh      *intersectingMesh;
    Plane           splitPlane;

    List<DWORD>     IntersectingFaces;
    List<DWORD>     NonIntersectingFaces;

    List<Vect>      NewVertList;
    List<Vect>      PHVertList;

    BitList         DeleteFaces;
    BitList         DeleteVerts;
    BitList         DeleteEdges;
    List<IDList>    VertFaces;
    List<IDList>    VertEdges;

    List<PolyData>  PolyDataList;

    BOOL            bDefaultCollisionOver;
    BOOL            bSideCollisionEnabled;

    BOOL            bAlmostIntersectedSoMakeAllOutside;

    Bounds          bounds;


    EditorBrush     *brush;


    EditorMesh()  {}
    EditorMesh(const EditorMesh &mesh);

    ~EditorMesh() {Clear();}

    //void  MakeFromBrush(Brush *brush);
    void  MakeFromMesh(Mesh *mesh);
    void  SaveToMesh(Mesh *mesh);
    void  Clear();

    void  CopyEditorMesh(const EditorMesh &mesh);

    BOOL  RayTriangleTest(DWORD triangle, const Vect &rayOrig, const Vect &rayDir, BOOL bFrontOnly=TRUE, float *fT=NULL);
    DWORD RayMeshTest(const Vect &rayOrig, const Vect &rayDir, float *fT=NULL, Plane *collisionPlane=NULL);
    BOOL  PointInsideMesh(const Vect &p);

    Vect GetFaceTangent(DWORD face);

    //Boolean operations
    void  InitPolyOperationData();
    void  LevelSubtract(EditorMesh &eb, EditorMesh &portalMesh);
    void  EndLevelSubtract();
    bool  GeometrySubtract(EditorMesh &eb);
    bool  SplitByPlane(const Plane &plane, EditorMesh *&newMesh);

    BOOL  PolyPolyIntersection(DWORD poly1, DWORD poly2);
    BOOL  PolyPlaneIntersection(DWORD polyID, PolyData &polyData, const Plane &plane, const Vect &crossVect, List<PolyIntersectData> &IntData);
    void  SeperateInsideOutsidePolys();
    void  ProcessSplits(BOOL bExcludePlanarPolys=FALSE);
    void  ProcessSplit1FindExtraSplits(PolyData &data);
    void  ProcessSplit2AddVertData(PolyData &data);
    void  ProcessSplit3RemoveCoincidingLines(PolyData &data);
    void  ProcessSplit4AddNewLines(PolyData &data);
    void  ProcessSplit5RetriangulatePoly(DWORD polyID);

    void  RemoveMarkedFaces(bool bRebuildPolys=true, bool bRemoveVerts=true);

    void  RemoveExcessFaces();
    void  RemoveExcessVerts();

    //Other
    void  RemoveEmptyPolys();
    void  RemoveUnassignedPolyFaces();

    void  BuildLightmapUVs(float maxAngle=45.0f, float adjVal=0.06f, int seperationUnits=128);
    void  AddLightmapUVPoly(AddLMPolyInfo &info);
    Vect  GetPolyDir(UINT poly);

    void  BuildPlaneList();

    void  BuildPolyData();

    void  RemoveFace(DWORD face);

    void  MakeEdges(BOOL bCombineEdges=TRUE);
    void  MakePolyEdges();
    void  FreePolyEdges();

    void  RemoveDuplicateVertices(BOOL bIgnorePolys=FALSE);
    BOOL  MergeExcessEdges();
    void  MergeExcessFaces();
    BOOL  MergeEdgeVerts(DWORD edgeNum, DWORD v1, DWORD v2);

    void  AddBareFace(const Vect &v1, const Vect &v2, const Vect &v3, const Plane &plane);

    BOOL  TriangleTriangleTest(DWORD face1ID, DWORD face2ID);

    void  MakeBounds();

    void  RebuildPolyBounds();
    void  RebuildPolyList(BOOL bSortFaces=TRUE);

    void  RebuildPlaneListThingy();

    BOOL  IsEmpty() {return VertList.Num() > 0;}

    void  DumpMesh();

    BOOL  bIAmADebugVariable;

    friend Serializer& operator<<(Serializer &s, EditorMesh &mesh);

private:
    void  RemoveMarkedVerts();
};

void GetBaryCoordsFromPoint(Vect *face, const Plane &plane, const Vect &p, float *coords);

BOOL PointInsideTriangle(Vect *tri, const Vect &point);

BOOL GetIntersectingPoints(const Plane &plane, Vect *tri, Vect *line);

DWORD PlaneListHasValue(List<Plane> &list, const Plane &val, float epsilon=EPSILON);
DWORD VectListHasValue (List<Vect>  &list, const Vect  &val, float epsilon=EPSILON);

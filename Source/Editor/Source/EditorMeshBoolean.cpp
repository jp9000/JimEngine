/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditorMeshBoolean.cpp

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



void  EditorMesh::InitPolyOperationData()
{
    traceIn(EditorMesh::InitPolyOperationData);

    FaceSideList.Clear();

    IntersectingFaces.Clear();
    NonIntersectingFaces.Clear();

    RebuildPolyList(FALSE);
    BuildPolyData();
    RebuildPolyBounds();

    originalVertNum = VertList.Num();
    originalFaceNum = FaceList.Num();
    originalPolyNum = PolyList.Num();

    PolyInfoList.SetSize(PolyList.Num());

    traceOut;
}

void  EditorMesh::LevelSubtract(EditorMesh &eb, EditorMesh &portalMesh)
{
    traceIn(EditorMesh::LevelSubtract);

    int i, j;

    intersectingMesh = &eb;

    InitPolyOperationData();

    //-----------------------------------------------------

    for(i=0; i<PolyList.Num(); i++)
    {
        for(j=0; j<eb.PolyList.Num(); j++)
            PolyPolyIntersection(i, j);
    }

    //-----------------------------------------------------

    ProcessSplits();

    //-----------------------------------------------------

    if(!IntersectingFaces.Num() && !NonIntersectingFaces.Num())
    {
        if(!bAlmostIntersectedSoMakeAllOutside && eb.PointInsideMesh(VertList[0]))
        {
            IntersectingFaces.SetSize(FaceList.Num());
            for(i=0; i<IntersectingFaces.Num(); i++)
                IntersectingFaces[i] = i;
        }
    }
    else
    {
        for(i=IntersectingFaces.Num()-1; i>=0; i--)
        {
            DWORD faceID = IntersectingFaces[i];
            Face &face = FaceList[faceID];

            portalMesh.AddBareFace(VertList[face.A], VertList[face.B], VertList[face.C], PlaneList[FacePlaneList[faceID]]);
        }

        portalMesh.FacePolyList.SetSize(portalMesh.FaceList.Num());
        portalMesh.FaceSmoothList.SetSize(portalMesh.FaceList.Num());

        portalMesh.NormalList.SetSize(portalMesh.VertList.Num());
        portalMesh.UVList.SetSize(portalMesh.VertList.Num());

        portalMesh.MakeBounds();

        portalMesh.RemoveDuplicateVertices(TRUE);
    }

    //-----------------------------------------------------

    EndLevelSubtract();

    traceOut;
}

int blablabla = 0;

void  EditorMesh::ProcessSplits(BOOL bExcludePlanarPolys)
{
    traceIn(EditorMesh::ProcessSplits);

    int i;

    BOOL bAnythingProcessed = FALSE;

    bAlmostIntersectedSoMakeAllOutside = FALSE;

    for(i=0; i<PolyDataList.Num(); i++)
    {
        PolyData &data = PolyDataList[i];

        if(!data.NewVerts.Num())
            continue;

        if(bExcludePlanarPolys)
        {
            ProcessSplit1FindExtraSplits(data);
            ProcessSplit2AddVertData(data);
            ProcessSplit3RemoveCoincidingLines(data);

            if(PolyInfoList[i].bDoPlanerTest && !data.NewLines.Num())
                data.Lines[0].lineData = LINEINSIDE;

            PolyInfoList[i].bDoPlanerTest = FALSE;

            ProcessSplit4AddNewLines(data);
            ProcessSplit5RetriangulatePoly(i);

            bAnythingProcessed = TRUE;
        }
        else
        {
            PolyInfoList[i].bDoPlanerTest = FALSE;

            if(i == 80)
                blablabla = 1;

            ProcessSplit1FindExtraSplits(data);
            ProcessSplit2AddVertData(data);
            ProcessSplit3RemoveCoincidingLines(data);
            if(!data.NewLines.Num())
                bAlmostIntersectedSoMakeAllOutside = TRUE;

            ProcessSplit4AddNewLines(data);
            ProcessSplit5RetriangulatePoly(i);

            bAnythingProcessed = TRUE;
        }
    }

    if(bAnythingProcessed && intersectingMesh)
        SeperateInsideOutsidePolys();
        

    PolyInfoList.Clear();

    traceOut;
}

void  EditorMesh::EndLevelSubtract()
{
    traceIn(EditorMesh::EndLevelSubtract);

    int i;

    if(IntersectingFaces.Num() == FaceList.Num())
        Clear();
    else
    {
        for(i=IntersectingFaces.Num()-1; i>=0; i--)
            RemoveFace(IntersectingFaces[i]);
    }

    for(i=0; i<PolyDataList.Num(); i++)
        PolyDataList[i].Clear();
    PolyDataList.Clear();

    IntersectingFaces.Clear();
    NonIntersectingFaces.Clear();

    FaceSideList.Clear();

    PolyList.SetSize(originalPolyNum);

    traceOut;
}


bool  EditorMesh::GeometrySubtract(EditorMesh &eb)
{
    traceIn(EditorMesh::GeometrySubtract);

    int i, j;

    intersectingMesh = &eb;

    InitPolyOperationData();
    eb.InitPolyOperationData();

    BOOL bFoundSplittingPoly = FALSE;

    for(i=0; i<PolyList.Num(); i++)
    {
        for(j=0; j<eb.PolyList.Num(); j++)
            bFoundSplittingPoly |= PolyPolyIntersection(i, j);
    }

    if(bFoundSplittingPoly)
    {
        ProcessSplits(FALSE);
        eb.ProcessSplits(TRUE);

        //-----------------------------------------------

        if(IntersectingFaces.Num() == FaceList.Num())
            Clear();
        else
        {
            for(i=IntersectingFaces.Num()-1; i>=0; i--)
                RemoveFace(IntersectingFaces[i]);
        }

        //-----------------------------------------------

        if(eb.IntersectingFaces.Num() == eb.FaceList.Num())
            eb.Clear();
        else
        {
            for(i=eb.IntersectingFaces.Num()-1; i>=0; i--)
                eb.RemoveFace(eb.IntersectingFaces[i]);
        }

        //-----------------------------------------------

        for(i=0; i<PolyDataList.Num(); i++)
            PolyDataList[i].Clear();
        PolyDataList.Clear();

        //-----------------------------------------------

        DWORD usedGroups1 = 0;
        for(i=0; i<FaceList.Num(); i++)
            usedGroups1 |= FaceSmoothList[i];

        DWORD usedGroups2 = 0;
        for(i=0; i<eb.FaceList.Num(); i++)
            usedGroups2 |= eb.FaceSmoothList[i];

        DWORD sharedGroups = (usedGroups1 & usedGroups2);
        if(sharedGroups)
        {
            DWORD usedGroups = (usedGroups1 | usedGroups2);

            for(i=0; i<32; i++)
            {
                DWORD curGroup = (1<<i);
                if(curGroup & sharedGroups)
                {
                    DWORD newGroup = 0;
                    while((newGroup < 32) && ((1<<newGroup) & usedGroups)) ++newGroup;

                    if(newGroup == 32)
                        break;

                    newGroup = (1<<newGroup);

                    usedGroups |= newGroup;

                    for(int j=0; j<eb.FaceList.Num(); j++)
                    {
                        DWORD &smooth = eb.FaceSmoothList[j];

                        if(smooth & curGroup)
                        {
                            smooth &= ~curGroup;
                            smooth |= newGroup;
                        }
                    }
                }
            }
        }

        //-----------------------------------------------

        for(i=0; i<eb.FaceList.Num(); i++)
        {
            Face &face = eb.FaceList[i];
            Plane newPlane = eb.PlaneList[eb.FacePlaneList[i]];
            int j;

            for(j=0; j<PlaneList.Num(); j++)
            {
                if(PlaneList[j].CloseTo(newPlane, 1e-3))
                    break;
            }

            DWORD planeNum = ((j != PlaneList.Num()) ? j : PlaneList.Add(newPlane));

            eb.FacePlaneList[i] = planeNum;
            eb.FacePolyList[i] += PolyList.Num();

            face.A += VertList.Num();
            face.B += VertList.Num();
            face.C += VertList.Num();
        }

        FaceList.AppendList(eb.FaceList);
        FacePolyList.AppendList(eb.FacePolyList);
        FacePlaneList.AppendList(eb.FacePlaneList);
        NormalList.AppendList(eb.NormalList);
        FaceSmoothList.AppendList(eb.FaceSmoothList);

        VertList.AppendList(eb.VertList);
        UVList.AppendList(eb.UVList);

        //-----------------------------------------------

        IntersectingFaces.Clear();
        NonIntersectingFaces.Clear();
        FaceSideList.Clear();

        MakeEdges();
        RemoveExcessFaces();
        RebuildPolyList(FALSE);
        RemoveEmptyPolys();
        return true;
    }

    return false;

    traceOut;
}

bool  EditorMesh::SplitByPlane(const Plane &plane, EditorMesh *&newMesh)
{
    traceIn(EditorMesh::SplitByPlane);

    InitPolyOperationData();

    splitPlane = plane;

    //------------------------------------------

    bool bFoundSplittingPoly = false;
    for(int i=0; i<PolyList.Num(); i++)
    {
        if(!PolyList[i].Faces.Num())
            continue;

        PolyData &polyData = PolyDataList[i];
        DWORD polyFaceID = PolyList[i].Faces[0];
        Plane &polyPlane = PlaneList[FacePlaneList[polyFaceID]];
        List<PolyIntersectData> IntData;

        Vect crossVect = plane.Dir.Cross(polyPlane.Dir);

        Face &face = FaceList[polyFaceID];
        if(i == 80 && CloseFloat(VertList[face.A].y, 0.0f) && CloseFloat(VertList[face.B].y, 0.0f) && CloseFloat(VertList[face.C].y, 0.0f))
            nop();

        if(!PolyPlaneIntersection(i, polyData, plane, crossVect, IntData))
            continue;

        if(i == 80 && CloseFloat(VertList[face.A].y, 0.0f) && CloseFloat(VertList[face.B].y, 0.0f) && CloseFloat(VertList[face.C].y, 0.0f))
            nop();

        bFoundSplittingPoly = true;

        //------------------------------

        Vect points[3];
        UVCoord uvCoords[3];
        Vect norms[3];
        float baryCoords[3];

        Face &polyFace = FaceList[polyFaceID];

        points[0] = VertList[polyFace.A];
        points[1] = VertList[polyFace.B];
        points[2] = VertList[polyFace.C];

        uvCoords[0] = UVList[polyFace.A];
        uvCoords[1] = UVList[polyFace.B];
        uvCoords[2] = UVList[polyFace.C];

        norms[0] = NormalList[polyFace.A];
        norms[1] = NormalList[polyFace.B];
        norms[2] = NormalList[polyFace.C];

        for(int j=0; j<IntData.Num(); j++)
        {
            GetBaryCoordsFromPoint(points, polyPlane, IntData[j].vert, baryCoords);

            UVCoord coord = (uvCoords[0]*baryCoords[0]) +
                            (uvCoords[1]*baryCoords[1]) +
                            (uvCoords[2]*baryCoords[2]);

            Vect norm     = (norms[0]*baryCoords[0]) +
                            (norms[1]*baryCoords[1]) +
                            (norms[2]*baryCoords[2]);

            norm.Norm();

            polyData.NewVerts << IntData[j].vert;
            polyData.NewNorms << norm;
            polyData.NewUVs << coord;

            if(j % 2)
            {
                PolyLine newLine;
                newLine.lineData = LINEINSIDE;
                newLine.v1 = j-1;
                newLine.v2 = j;

                polyData.NewLines << newLine;
            }
        }
    }

    if(!bFoundSplittingPoly)
        return false;

    //------------------------------------------

    ProcessSplits();

	//------------------------------------------
	//separate polygons

	RemoveUnassignedPolyFaces();
	RebuildPolyList(FALSE);

	for(int i=0; i<FaceList.Num(); i++)
	{
		Face &face = FaceList[i];
		Vect &testVert = VertList[face.A];

		float planeDist = testVert.DistFromPlane(splitPlane);

		if(CloseFloat(planeDist, 0.0f))
		{
			int side = 0;
			for(int j=1; j<3; j++)
			{
				planeDist = VertList[face.ptr[j]].DistFromPlane(splitPlane);
				if(planeDist > EPSILON)
				{
					side = LINEOUTSIDE;
					break;
				}
				else if(planeDist < -EPSILON)
				{
					side = LINEINSIDE;
					break;
				}
			}

			if(side == LINEOUTSIDE)
				IntersectingFaces << i;
			else if(side == LINEINSIDE)
				NonIntersectingFaces << i;
			else
			{
				Plane &plane = PlaneList[FacePlaneList[i]];
				if(!plane.Dir.CloseTo(splitPlane.Dir))
					IntersectingFaces << i;
				else
					NonIntersectingFaces << i;
			}
		}
		else if(planeDist > 0.0f)
			IntersectingFaces << i;
		else
			NonIntersectingFaces << i;
	}

	//------------------------------------------

    newMesh = new EditorMesh(*this);

    newMesh->DeleteFaces.Clear();
    newMesh->DeleteFaces.SetSize(newMesh->FaceList.Num());

    for(int i=0; i<IntersectingFaces.Num(); i++)
        newMesh->DeleteFaces.Set(IntersectingFaces[i]);

    newMesh->RemoveMarkedFaces();
    newMesh->MakeEdges();

    //------------------------------------------

    DeleteFaces.Clear();
    DeleteFaces.SetSize(FaceList.Num());

    for(int i=0; i<NonIntersectingFaces.Num(); i++)
        DeleteFaces.Set(NonIntersectingFaces[i]);

    RemoveMarkedFaces();
    MakeEdges();

    return true;

    traceOut;
}




void  EditorMesh::SeperateInsideOutsidePolys()
{
    DWORD i, j, k;

    RemoveUnassignedPolyFaces();
    RebuildPolyList(FALSE);

    MakeEdges(TRUE);

    typedef List<DWORD> LinkedFaceList;
    List<LinkedFaceList> LinkedFaces;

    LinkedFaces.SetSize(FaceList.Num());

    for(i=0; i<EdgeList.Num(); i++)
    {
        Edge &edge = EdgeList[i];

        if(edge.f2 != INVALID)
        {
            LinkedFaces[edge.f1] << edge.f2;
            LinkedFaces[edge.f2] << edge.f1;
        }
    }

    List<DWORD> TempOutsideFaces;
    List<DWORD> TempInsideFaces;

    for(i=0; i<PolyInfoList.Num(); i++)
    {
        PolyInfo &info = PolyInfoList[i];

        if(!info.bDoPlanerTest)
            continue;

        PolyFace &poly = PolyList[i];

        BOOL bLiesOnEdges = FALSE;
        DWORD newSide = 0;

        for(j=0; j<poly.Faces.Num(); j++)
        {
            List<DWORD> &ConnectedFaces = LinkedFaces[poly.Faces[j]];

            for(k=0; k<ConnectedFaces.Num(); k++)
            {
                DWORD testPoly = FacePolyList[ConnectedFaces[k]];
                if(testPoly == i)
                    continue;

                PolyInfo &infoTest = PolyInfoList[testPoly];

                if(infoTest.bLiesOnEdge)
                {
                    newSide = ~(FaceSideList[ConnectedFaces[k]]) & 0x3;
                    bLiesOnEdges = TRUE;
                    break;
                }
            }

            if(bLiesOnEdges)
                break;
        }

        if(bLiesOnEdges)
        {
            for(j=0; j<poly.Faces.Num(); j++)
            {
                FaceSideList[poly.Faces[j]] = newSide;
            }
        }
    }

    for(i=0; i<FaceSideList.Num(); i++)
    {
        if(FaceSideList[i] == LINEOUTSIDE)
            TempOutsideFaces << i;
        else if(FaceSideList[i] == LINEINSIDE)
            TempInsideFaces << i;
    }

    while(TempOutsideFaces.Num())
    {
        DWORD faceID = TempOutsideFaces[0];
        List<DWORD> &ConnectedFaces = LinkedFaces[faceID];

        for(i=0; i<ConnectedFaces.Num(); i++)
        {
            DWORD testFace = ConnectedFaces[i];
            DWORD &side = FaceSideList[testFace];

            if(!side)
            {
                side = LINEOUTSIDE;
                TempOutsideFaces << testFace;
            }
        }

        TempOutsideFaces.Remove(0);
    }

    while(TempInsideFaces.Num())
    {
        DWORD faceID = TempInsideFaces[0];
        List<DWORD> &ConnectedFaces = LinkedFaces[faceID];

        for(i=0; i<ConnectedFaces.Num(); i++)
        {
            DWORD testFace = ConnectedFaces[i];
            DWORD &side = FaceSideList[testFace];

            if(!side)
            {
                side = LINEINSIDE;
                TempInsideFaces << testFace;
            }
        }

        TempInsideFaces.Remove(0);
    }

    for(i=0; i<LinkedFaces.Num(); i++)
        LinkedFaces[i].Clear();
    LinkedFaces.Clear();

    List<DWORD> DazedAndConfusedFaces;

    for(i=0; i<FaceSideList.Num(); i++)
    {
        DWORD side = FaceSideList[i];

        if(side == LINEINSIDE)
            IntersectingFaces << i;
        else if(side == LINEOUTSIDE)
            NonIntersectingFaces << i;
        else
            DazedAndConfusedFaces << i;
    }

    if(DazedAndConfusedFaces.Num())
    {
        for(i=0; i<DazedAndConfusedFaces.Num(); i++)
        {
			Face &face = FaceList[DazedAndConfusedFaces[i]];
            Vect &testVert = VertList[face.A];

            if(intersectingMesh->PointInsideMesh(testVert))
                IntersectingFaces << DazedAndConfusedFaces[i];
            else
                NonIntersectingFaces << DazedAndConfusedFaces[i];
        }
    }
}

//if new verts lay on edges, split those edges
void  EditorMesh::ProcessSplit1FindExtraSplits(PolyData &data)
{
    DWORD j,k;

    BitList AlreadyAdded;
    AlreadyAdded.SetSize(data.NewVerts.Num());

    for(j=0; j<data.Lines.Num(); j++)
    {
        PolyLine line = data.Lines[j];
        Vect v1 = VertList[line.v1];
        Vect v2 = VertList[line.v2];

        for(k=0; k<data.NewVerts.Num(); k++)
        {
            Vect &vert = data.NewVerts[k];

            //if it's not a newer vert, then it's already connected to an existing split.
            if(!AlreadyAdded[k])
            {
                if(PointOnFiniteLine(v1, v2, vert))
                {
                    if(!vert.CloseTo(v1, LARGE_EPSILON) && !vert.CloseTo(v2, LARGE_EPSILON))
                    {
                        PolyLine &newLine = *data.Lines.InsertNew(j+1);
                        DWORD oldV2 = line.v2;

                        DWORD vertID = VertList.Add(vert);
                        UVList.Add(data.NewUVs[k]);
                        NormalList.Add(data.NewNorms[k]);

                        v2 = vert;
                        line.v2 = vertID;

                        newLine.v1 = vertID;
                        newLine.v2 = oldV2;
                        //newLine.lineData = LINEINSIDE;  //warning: removed this for plane splitting, check this if problems with boolean ops

                        data.Lines[j] = line;

                        AlreadyAdded.Set(k);
                    }
                }
            }
        }
    }

    AlreadyAdded.Clear();

    AlreadyAdded.SetSize(data.Lines.Num());

    for(j=0; j<data.NewLines.Num(); j++)
    {
        PolyLine line = data.NewLines[j];
        Vect v1 = data.NewVerts[line.v1];
        Vect v2 = data.NewVerts[line.v2];

        for(k=0; k<data.Lines.Num(); k++)
        {
            DWORD vertID = data.Lines[k].v1;
            Vect &vert = VertList[vertID];

            //if it's not a newer vert, then it's already connected to an existing split.
            if(!AlreadyAdded[k])
            {
                if(PointOnFiniteLine(v1, v2, vert))
                {
                    if(!vert.CloseTo(v1, LARGE_EPSILON) && !vert.CloseTo(v2, LARGE_EPSILON))
                    {
                        PolyLine &newLine = *data.NewLines.InsertNew(j+1);
                        DWORD oldV2 = line.v2;

                        DWORD newVertID = data.NewVerts.FindValueIndex(vert);
                        if(newVertID == INVALID)
                        {
                            newVertID = data.NewVerts.Add(vert);
                            data.NewUVs.Add(UVList[vertID]);
                            data.NewNorms.Add(NormalList[vertID]);
                        }

                        v2 = vert;
                        line.v2 = newVertID;

                        newLine.v1 = newVertID;
                        newLine.v2 = oldV2;
                        newLine.lineData = LINEINSIDE;

                        data.NewLines[j] = line;

                        AlreadyAdded.Set(k);
                    }
                }
            }
        }
    }

    for(j=0; j<data.NewLines.Num(); j++)
    {
        PolyLine &line1 = data.NewLines[j];
        BOOL bFoundExisting = FALSE;

        for(k=j+1; k<data.NewLines.Num(); k++)
        {
            PolyLine &line2 = data.NewLines[k];

            if((line1.v1 == line2.v1) && (line1.v2 == line2.v2))
            {
                bFoundExisting = TRUE;
                break;
            }
        }

        if(bFoundExisting)
            data.NewLines.Remove(j--);
    }

    AlreadyAdded.Clear();
}

//add verts to the actual mesh from the temporary storage
void  EditorMesh::ProcessSplit2AddVertData(PolyData &data)
{
    DWORD j,k;

    List<DWORD> NewVertIndices;
    NewVertIndices.SetSize(data.NewVerts.Num());

    for(j=0; j<data.NewVerts.Num(); j++)
    {
        BOOL bDeadVert = TRUE;

        for(k=0; k<data.NewLines.Num(); k++)
        {
            PolyLine &line = data.NewLines[k];
            if((line.v1 == j) || (line.v2 == j))
            {
                bDeadVert = FALSE;
                break;
            }
        }

        if(bDeadVert)
            continue;

        Vect &vert = data.NewVerts[j];
        DWORD foundVert = INVALID;

        for(k=0; k<data.Lines.Num(); k++)
        {
            PolyLine &line = data.Lines[k];
            Vect &v1 = VertList[line.v1];

            if(vert.CloseTo(v1))
            {
                foundVert = line.v1;
                break;
            }
        }

        if(foundVert == INVALID)
        {
            NewVertIndices[j] = VertList.Add(vert);
            UVList.Add(data.NewUVs[j]);
            NormalList.Add(data.NewNorms[j]);
        }
        else
            NewVertIndices[j] = foundVert;
    }

    data.NewVerts.Clear();
    data.NewUVs.Clear();
    data.NewNorms.Clear();

    for(j=0; j<data.NewLines.Num(); j++)
    {
        PolyLine &line = data.NewLines[j];

        line.v1 = NewVertIndices[line.v1];
        line.v2 = NewVertIndices[line.v2];
    }

    NewVertIndices.Clear();
}

void  EditorMesh::ProcessSplit3RemoveCoincidingLines(PolyData &data)
{
    DWORD j,k;
    DWORD lastLineData = LINEOUTSIDE;

    for(j=0; j<data.NewLines.Num(); j++)
    {
        PolyLine &line = data.NewLines[j];

        DWORD foundLine = data.FindLine(line.v1, line.v2);

        if(foundLine != INVALID)
        {
            List<DWORD> DupeLines;
            DWORD curDupe = j;

            while((curDupe = data.FindNewLine(line.v1, line.v2, curDupe+1)) != INVALID)
                DupeLines.Insert(0, curDupe);

            if(!DupeLines.Num() && (line.v1 == data.Lines[foundLine].v1))
                lastLineData = LINEINSIDE;

            for(k=0; k<DupeLines.Num(); k++)
                data.NewLines.Remove(DupeLines[k]);

            data.NewLines.Remove(j--);
        }
    }

    if(!data.NewLines.Num())
        data.Lines[0].lineData = /*LINEOUTSIDE;//*/lastLineData;
}

void  EditorMesh::ProcessSplit4AddNewLines(PolyData &data)
{
    DWORD j;

    if(data.NewLines.Num() && data.Lines.Num())
    {
        List<PolyLine> TempLines;
        TempLines.CopyList(data.NewLines);

        for(j=0; j<data.NewLines.Num(); j++)
        {
            PolyLine &line = data.NewLines[j];

            line.lineData = LINEOUTSIDE;

            DWORD oldV2 = line.v2;
            line.v2 = line.v1;
            line.v1 = oldV2;
        }

        ///inverted --> uninverted --> main
        data.NewLines.AppendList(TempLines);
        data.NewLines.AppendList(data.Lines);

        data.Lines.CopyList(data.NewLines);

        data.NewLines.Clear();
        TempLines.Clear();
    }
}

void  EditorMesh::ProcessSplit5RetriangulatePoly(DWORD polyID)
{
    PolyFace &poly = PolyList[polyID];

    PolyData &data    = PolyDataList[polyID];
    DWORD polyFace1ID = PolyList[polyID].Faces[0];
    DWORD polyPlaneID = FacePlaneList[polyFace1ID];
    DWORD smoothID    = FaceSmoothList[polyFace1ID];
    Plane &polyPlane  = PlaneList[polyPlaneID];

    DWORD i;

    for(i=0; i<poly.Faces.Num(); i++)
        FacePolyList[poly.Faces[i]] = INVALID;
    poly.Faces.Clear();

    Triangulator triangulator;
    triangulator.Trianglulate3DData(polyPlane, data.Lines, VertList);

    DWORD prevFaceListSize = FaceList.Num();

    PolyFace *insidePoly = NULL;
    DWORD newPolyID = INVALID;

    FaceList.AppendList(triangulator.Faces);
    FacePlaneList.SetSize(FaceList.Num());
    FacePolyList.SetSize(FaceList.Num());
    FaceSmoothList.SetSize(FaceList.Num());

    FaceSideList.SetSize(FaceList.Num());

    DWORD totalSidesVal = 0;

    for(i=0; i<triangulator.Faces.Num(); i++)
    {
        DWORD side = triangulator.FaceData[i];
        DWORD faceListOffset = prevFaceListSize+i;

        FacePlaneList[faceListOffset]  = polyPlaneID;
        FaceSmoothList[faceListOffset] = smoothID;

        poly.Faces << faceListOffset;
        FacePolyList[faceListOffset] = polyID;

        FaceSideList[faceListOffset] = side;
        totalSidesVal |= side;
    }

    if(totalSidesVal != 3)
        PolyInfoList[polyID].bLiesOnEdge = TRUE;
}

//oh dear god please don't make me write thi -- ..*HRK*!!!  OKAY FINE!!!
//(note: why do I even have a return value?)
BOOL  EditorMesh::PolyPolyIntersection(DWORD poly1, DWORD poly2)
{
    PolyData &poly1Data = PolyDataList[poly1];
    PolyData &poly2Data = intersectingMesh->PolyDataList[poly2];

    if(!poly1Data.Lines.Num() || !poly2Data.Lines.Num())
        return FALSE;

    Bounds &poly1Bounds = PolyBounds[poly1];
    DWORD poly1Face1ID = PolyList[poly1].Faces[0];
    Plane &poly1Plane = PlaneList[FacePlaneList[poly1Face1ID]];

    Bounds &poly2Bounds = intersectingMesh->PolyBounds[poly2];
    DWORD poly2Face1ID = intersectingMesh->PolyList[poly2].Faces[0];
    Plane &poly2Plane = intersectingMesh->PlaneList[intersectingMesh->FacePlaneList[poly2Face1ID]];

    if(!poly1Bounds.Intersects(poly2Bounds))
        return FALSE;

    if(poly1Plane.Coplanar(poly2Plane))
    {
        PolyInfoList[poly1].bDoPlanerTest = TRUE;
        intersectingMesh->PolyInfoList[poly2].bDoPlanerTest = TRUE;
        return FALSE;
    }

    List<PolyIntersectData> IntData;
    int i, j;

    Vect &crossVect = poly2Plane.Dir^poly1Plane.Dir;

    IntData.Clear();

    if(!PolyPlaneIntersection(0, poly1Data, poly2Plane, crossVect, IntData))
        return FALSE;
    if(!intersectingMesh->PolyPlaneIntersection(1, poly2Data, poly1Plane, crossVect, IntData))
        return FALSE;

    if((IntData.Num()%2) != 0)
        AppWarning(TEXT("hrm.  not good."));

    BOOL bInsideCoincidingLine[2] = {FALSE, FALSE};
    BOOL bIsInside=FALSE;

    List<Vect>  NewVerts;
    List<float> NewDists;
    List<DWORD> Poly2Sides;

    DWORD lastPoly=INVALID;
    float lastDist=0.0f;

    DWORD curPoly2Side;

    for(i=0; i<IntData.Num(); i++)
    {
        PolyIntersectData &intersection = IntData[i];
        if((lastPoly != intersection.polyID) || (intersection.dist != lastDist))
        {
            BOOL &polyInsideCoinciding = bInsideCoincidingLine[intersection.polyID];
            polyInsideCoinciding = !polyInsideCoinciding;

            if(intersection.polyID == 1)
                curPoly2Side = intersection.lineSides;
        }

        if(bInsideCoincidingLine[0] && bInsideCoincidingLine[1])
        {
            if(!bIsInside)
            {
                bIsInside = TRUE;
                NewVerts << intersection.vert;
                NewDists << intersection.dist;
                Poly2Sides << curPoly2Side;
            }
        }
        else if(bIsInside)
        {
            bIsInside = FALSE;
            NewVerts << intersection.vert;
            NewDists << intersection.dist;
            Poly2Sides << curPoly2Side;
        }

        lastPoly = intersection.polyID;
        lastDist = intersection.dist;
    }

    for(i=0; i<NewVerts.Num(); i+=2)
    {
        float &v1 = NewDists[i];
        float &v2 = NewDists[i+1];

        if(CloseFloat(v1, v2, EPSILON))
        {
            NewVerts.Remove(i); NewVerts.Remove(i);
            NewDists.Remove(i); NewDists.Remove(i);
            Poly2Sides.Remove(i); Poly2Sides.Remove(i);

            i-=2;
        }
    }

    IntData.Clear();
    NewDists.Clear();

    if(!NewVerts.Num())
        return FALSE;

    //---------------------------------------------------------

    Vect points[3];
    UVCoord uvCoords[3];
    Vect norms[3];
    float baryCoords[3];

    Face &poly1Face1 = FaceList[poly1Face1ID];

    points[0] = VertList[poly1Face1.A];
    points[1] = VertList[poly1Face1.B];
    points[2] = VertList[poly1Face1.C];

    uvCoords[0] = UVList[poly1Face1.A];
    uvCoords[1] = UVList[poly1Face1.B];
    uvCoords[2] = UVList[poly1Face1.C];

    norms[0] = NormalList[poly1Face1.A];
    norms[1] = NormalList[poly1Face1.B];
    norms[2] = NormalList[poly1Face1.C];

    for(i=0; i<NewVerts.Num(); i+=2)
    {
        Vect *newVert[2] = {&NewVerts[i], &NewVerts[i+1]};
        PolyLine line;
        BOOL bFoundNewVerts = FALSE;

        line.lineData = LINEINSIDE;

        BOOL bCoplanerUselessLine = ((Poly2Sides[i] == 2) && (Poly2Sides[i+1] == 2));

        for(j=0; j<2; j++)
        {
            line.p[j] = poly1Data.NewVerts.FindValueIndex(*newVert[j]);

            if(line.p[j] == INVALID)
            {
                GetBaryCoordsFromPoint(points, poly1Plane, *newVert[j], baryCoords);

                UVCoord coord = (uvCoords[0]*baryCoords[0]) +
                                (uvCoords[1]*baryCoords[1]) +
                                (uvCoords[2]*baryCoords[2]);

                Vect norm     = (norms[0]*baryCoords[0]) +
                                (norms[1]*baryCoords[1]) +
                                (norms[2]*baryCoords[2]);

                norm.Norm();

                line.p[j] = poly1Data.NewVerts.Add(*newVert[j]);
                poly1Data.NewUVs << coord;
                poly1Data.NewNorms << norm;
            }
        }

        BOOL bFoundLine = FALSE;

        if(!bFoundNewVerts)
        {
            for(j=0; j<poly1Data.NewLines.Num(); j++)
            {
                PolyLine &testLine = poly1Data.NewLines[j];
                if( (testLine.v1 == line.v1) &&
                    (testLine.v2 == line.v2) )
                {
                    bFoundLine = TRUE;
                    break;
                }
            }
        }

        if(!bFoundLine && !bCoplanerUselessLine)
            poly1Data.NewLines << line;
    }

    Face &poly2Face1 = intersectingMesh->FaceList[poly2Face1ID];
    points[0] = intersectingMesh->VertList[poly2Face1.A];
    points[1] = intersectingMesh->VertList[poly2Face1.B];
    points[2] = intersectingMesh->VertList[poly2Face1.C];

    uvCoords[0] = intersectingMesh->UVList[poly2Face1.A];
    uvCoords[1] = intersectingMesh->UVList[poly2Face1.B];
    uvCoords[2] = intersectingMesh->UVList[poly2Face1.C];

    norms[0] = intersectingMesh->NormalList[poly2Face1.A];
    norms[1] = intersectingMesh->NormalList[poly2Face1.B];
    norms[2] = intersectingMesh->NormalList[poly2Face1.C];

    for(i=NewVerts.Num()-1; i>=0; i-=2)
    {
        Vect *newVert[2] = {&NewVerts[i], &NewVerts[i-1]};
        PolyLine line;// = *poly2Data.NewLines.CreateNew();
        BOOL bFoundNewVerts = FALSE;

        line.lineData = LINEINSIDE;

        for(j=0; j<2; j++)
        {
            line.p[j] = poly2Data.NewVerts.FindValueIndex(*newVert[j]);

            if(line.p[j] == INVALID)
            {
                GetBaryCoordsFromPoint(points, poly2Plane, *newVert[j], baryCoords);

                UVCoord coord = (uvCoords[0]*baryCoords[0]) +
                                (uvCoords[1]*baryCoords[1]) +
                                (uvCoords[2]*baryCoords[2]);

                Vect norm     = (norms[0]*baryCoords[0]) +
                                (norms[1]*baryCoords[1]) +
                                (norms[2]*baryCoords[2]);

                norm.Norm();

                line.p[j] = poly2Data.NewVerts.Add(*newVert[j]);
                poly2Data.NewUVs << coord;
                poly2Data.NewNorms << norm;

                bFoundNewVerts = TRUE;
            }
        }

        BOOL bFoundLine = FALSE;

        if(!bFoundNewVerts)
        {
            for(j=0; j<poly2Data.NewLines.Num(); j++)
            {
                PolyLine &testLine = poly2Data.NewLines[j];
                if( (testLine.v1 == line.v1) &&
                    (testLine.v2 == line.v2) )
                {
                    bFoundLine = TRUE;
                    break;
                }
            }
        }

        if(!bFoundLine)
            poly2Data.NewLines << line;
    }

    return TRUE;
}

BOOL  EditorMesh::PolyPlaneIntersection(DWORD polyID, PolyData &polyData, const Plane &plane, const Vect &crossVect, List<PolyIntersectData> &IntData)
{
    DWORD i, j;
    //DWORD numOnCorners=0;

    List<PolyIntersectData> IntDataTemp;

    for(i=0; i<polyData.Lines.Num(); i++)
    {
        PolyLine &line = polyData.Lines[i];
        Vect &v1 = VertList[line.v1];
        Vect &v2 = VertList[line.v2];
        float fT;

        if(plane.GetIntersection(v1, v2, fT))
        {
            //this stuff here is for when the splitting plane lies on a vertex.
            //we don't want to create duplicated vertices unless we're on the
            //corner of an opening.
            BOOL bOnVert = ((fT == 1.0f) || (fT == 0.0f));
            DWORD lineSides = 3;

            if(bOnVert)
                lineSides = plane.LineInside(v1, v2);

            //-----------------------------

            Vect vNewPoint = Lerp(v1, v2, fT);
            float dist = vNewPoint.Dot(crossVect);
            BOOL bCancelInsert = FALSE;

            for(j=0; j<IntDataTemp.Num(); j++)
            {
                PolyIntersectData &intersect = IntDataTemp[j];

                if((dist == intersect.dist) && (intersect.polyID == polyID) && bOnVert)
                {
                    if(lineSides != intersect.lineSides)
                        intersect.lineSides = 3;
                    else
                        IntDataTemp.Remove(j);

                    bCancelInsert = TRUE;
                    break;
                }
                else if(dist < intersect.dist)
                    break;
            }

            if(bCancelInsert)
                continue;

            PolyIntersectData intersection;
            intersection.polyID = polyID;
            intersection.dist = dist;
            intersection.vert = vNewPoint;
            intersection.lineSides = lineSides;

            IntDataTemp.Insert(j, intersection);
        }
    }

    //now, here, what we do, is we find points that do not need to be
    //BOOL bInvert = FALSE;

    if(IntDataTemp.Num() > 1)
    {
        for(i=0; i<(IntDataTemp.Num()-1); i++)
        {
            PolyIntersectData &intersect1 = IntDataTemp[i];
            if(intersect1.lineSides != 3)
            {
                PolyIntersectData &intersect2 = IntDataTemp[i+1];

                if((intersect2.lineSides & intersect1.lineSides) == 0) //if one is 1 and the other is 2
                {
                    if(i%2)
                        IntDataTemp.Remove(i--);
                    else
                        IntDataTemp.Remove(i+1);
                }
            }
        }
    }

    //reorder them
    if(IntDataTemp.Num() > 1)
    {
        for(i=0; i<IntDataTemp.Num(); i++)
        {
            PolyIntersectData &intersectTemp = IntDataTemp[i];
            for(j=0; j<IntData.Num(); j++)
            {
                PolyIntersectData &intersect = IntData[j];

                if(intersectTemp.dist < intersect.dist)
                    break;
            }

            IntData.Insert(j, intersectTemp);
        }

        return TRUE;
    }

    return FALSE;

    //return (IntDataTemp.Num > 2) && (numOnCorners != (num/2));  //if they all sit on corners, then, well, no actual splits
}


void  EditorMesh::BuildPolyData()
{
    DWORD i, j, k, diseasedLine;

    for(i=0; i<PolyDataList.Num(); i++)
        PolyDataList[i].Clear();

    PolyDataList.SetSize(PolyList.Num());

    //get lines
    for(i=0; i<PolyList.Num(); i++)
    {
        PolyFace &poly = PolyList[i];
        PolyData &polyData = PolyDataList[i];

        for(j=0; j<poly.Faces.Num(); j++)
        {
            Face &face = FaceList[poly.Faces[j]];

            for(k=0; k<3; k++)
            {
                DWORD kp1 = (k == 2) ? 0 : (k+1);

                diseasedLine = polyData.FindLine(face.ptr[k], face.ptr[kp1]);

                if(diseasedLine == INVALID)
                {
                    PolyLine line = {face.ptr[k], face.ptr[kp1]};
                    polyData.Lines << line;
                }
                else
                    polyData.Lines.Remove(diseasedLine);
            }
        }
    }

    //order the lines, build the verts
    for(i=0; i<PolyList.Num(); i++)
    {
        PolyData &polyData = PolyDataList[i];

        if(polyData.Lines.Num())
        {
            for(j=0; j<(polyData.Lines.Num()-1); j++)
            {
                PolyLine &line1 = polyData.Lines[j];
                PolyLine &nextLine = polyData.Lines[j+1];

                for(k=j+1; k<polyData.Lines.Num(); k++)
                {
                    PolyLine &line2 = polyData.Lines[k];

                    if(line1.v2 == line2.v1)
                    {
                        PolyLine backup = {line2.v1, line2.v2};

                        line2.v1 = nextLine.v1;
                        line2.v2 = nextLine.v2;

                        nextLine.v1 = backup.v1;
                        nextLine.v2 = backup.v2;

                        break;
                    }
                }
            }
        }
    }
}

BOOL PointInsideTriangle(Vect *tri, const Vect &point)
{
    float angle = 0.0f;
    Vect dirs[3];
    int i;

    for(i=0; i<3; i++)
        dirs[i] = (tri[i]-point).Norm();

    for(i=0; i<3; i++)
    {
        int ip1 = (i == 2) ? 0 : (i+1);

        angle += acosf(dirs[i].Dot(dirs[ip1]));
    }

    return CloseFloat(angle, 6.283185f, EPSILON);
}

BOOL GetIntersectingPoints(const Plane &plane, Vect *tri, Vect *line)
{
    float fDists[3];
    int sideVals[3];
    int i, zeroCount = 0;

    for(i=0; i<3; i++)
    {
        fDists[i] = tri[i].DistFromPlane(plane);
        if(fabsf(fDists[i]) <= EPSILON)
            fDists[i] = 0.0f;

        sideVals[i] = 0;

        if(fDists[i] < 0.0f)
            sideVals[i] = 1;
        else if(fDists[i] > 0.0f)
            sideVals[i] = 2;
        else if(fDists[i] == 0.0f)
        {
            sideVals[i] = 3;
            ++zeroCount;
        }

        fDists[i] = fabsf(fDists[i]);
    }

    if( (zeroCount != 2) &&
        (sideVals[0] & sideVals[1]) &&
        (sideVals[0] & sideVals[2]) &&
        (sideVals[1] & sideVals[2]) )
        return FALSE;

    DWORD count = 0;

    for(i=0; i<3; i++)
    {
        int ip1 = (i == 2) ? 0 : (i+1);

        if(sideVals[i] == 3)
            line[count++] = tri[i];
        else if(!(sideVals[i] & sideVals[ip1]))
            line[count++] = Lerp(tri[i], tri[ip1], fDists[i]/(fDists[i]+fDists[ip1]));
    }

    return TRUE;
}


void EditorMesh::RemoveFace(DWORD face)
{
    DWORD k;

    if(IntersectingFaces.Num())
    {
        for(k=0; k<IntersectingFaces.Num(); k++)
        {
            DWORD &face2 = IntersectingFaces[k];

            if(face2 == face)
                IntersectingFaces.Remove(k--);
            else if(face2 > face)
                --face2;
        }
    }
    if(NonIntersectingFaces.Num())
    {
        for(k=0; k<NonIntersectingFaces.Num(); k++)
        {
            DWORD &face2 = NonIntersectingFaces[k];

            if(face2 == face)
                NonIntersectingFaces.Remove(k--);
            else if(face2 > face)
                --face2;
        }
    }

    if(FaceCuts.Num())
    {
        FaceCuts[face].Clear();
        FaceCuts.Remove(face);
    }

    FaceList.Remove(face);
    FacePlaneList.Remove(face);
    FacePolyList.Remove(face);
    FaceSmoothList.Remove(face);
    if(FaceSideList.Num())
        FaceSideList.Remove(face);
}

void EditorMesh::RemoveDuplicateVertices(BOOL bIgnorePolys)
{
    long i, j;

    List<DWORD> VertPolyIDs;
    VertPolyIDs.SetSize(VertList.Num());

    if(!bIgnorePolys)
    {
        for(i=0; i<FaceList.Num(); i++)
        {
            Face &f = FaceList[i];
            DWORD polyID = FacePolyList[i];

            VertPolyIDs[f.A] = polyID;
            VertPolyIDs[f.B] = polyID;
            VertPolyIDs[f.C] = polyID;
        }
    }

    List<DWORD> NewVertIDs;
    List<DWORD> DeleteVertIDs;
    BitList     DeleteVerts;

    NewVertIDs.SetSize(VertList.Num());
    DeleteVertIDs.SetSize(VertList.Num());
    DeleteVerts.SetSize(VertList.Num());

    for(i=0; i<VertList.Num(); i++)
    {
        if(DeleteVerts[i])
            continue;

        Vect &vert1  = VertList[i];
        DWORD vertPoly1 = VertPolyIDs[i];

        NewVertIDs[i] = i;

        for(j=i+1; j<VertList.Num(); j++)
        {
            if(DeleteVerts[j])
                continue;

            Vect &vert2  = VertList[j];
            DWORD vertPoly2 = VertPolyIDs[j];

            if( vert1.CloseTo(vert2) &&
                vertPoly1 == vertPoly2)
            {
                DeleteVerts.Set(j);
                NewVertIDs[j] = i;
            }
        }
    }

    DWORD adjustID=0;

    for(i=0; i<VertList.Num(); i++)
    {
        if(i == originalVertNum)
            originalVertNum -= adjustID;  //removing original verts, adjust original vert count

        if(DeleteVerts[i])
        {
            ++adjustID;
            continue;
        }

        DeleteVertIDs[i] = i-adjustID;
    }

    for(i=0; i<FaceList.Num(); i++)
    {
        Face &face = FaceList[i];

        face.A = DeleteVertIDs[NewVertIDs[face.A]];
        face.B = DeleteVertIDs[NewVertIDs[face.B]];
        face.C = DeleteVertIDs[NewVertIDs[face.C]];
    }

    for(i=VertList.Num()-1; i>=0; i--)
    {
        if(DeleteVerts[i])
        {
            VertList.Remove(i);
            UVList.Remove(i);
            NormalList.Remove(i);
        }
    }
}

BOOL EditorMesh::MergeExcessEdges()
{
    long i,j;

    MakeEdges(FALSE);

    BitList LinkedEdges;
    LinkedEdges.SetSize(EdgeList.Num());

    List<DWORD> OpenVerts;
    OpenVerts.SetSize(VertList.Num());

    DeleteFaces.SetSize(FaceList.Num());
    DeleteVerts.SetSize(VertList.Num());
    DeleteEdges.SetSize(EdgeList.Num());
    VertFaces.SetSize(VertList.Num());
    VertEdges.SetSize(VertList.Num());

    BOOL bNewPass = FALSE;

    //Get Open Verticies
    for(i=0; i<EdgeList.Num(); i++)
    {
        Edge &edge = EdgeList[i];

        if(edge.f2 == INVALID)
        {
            OpenVerts[edge.v1]++;
            OpenVerts[edge.v2]++;
        }
        else
        {
            VertFaces[edge.v1].SafeAdd(edge.f2);
            VertFaces[edge.v2].SafeAdd(edge.f2);
        }

        VertFaces[edge.v1].SafeAdd(edge.f1);
        VertFaces[edge.v2].SafeAdd(edge.f1);

        VertEdges[edge.v1].SafeAdd(i);
        VertEdges[edge.v2].SafeAdd(i);

        Vect *e1[2] = {&VertList[edge.v1], &VertList[edge.v2]};

        for(j=i+1; j<EdgeList.Num(); j++)
        {
            if(LinkedEdges[j])
                continue;

            Edge &edge2 = EdgeList[j];
            Vect *e2[2] = {&VertList[edge2.v1], &VertList[edge2.v2]};

            if( (e1[0]->CloseTo(*e2[0]) && e1[1]->CloseTo(*e2[1])) ||
                (e1[0]->CloseTo(*e2[1]) && e1[1]->CloseTo(*e2[0])) )
            {
                LinkedEdges.Set(i);
                LinkedEdges.Set(j);
                break;
            }
        }
    }

    for(i=0; i<EdgeList.Num(); i++)
    {
        if(DeleteEdges[i])
            continue;

        Edge &edge1 = EdgeList[i];
        BOOL bEdge1Open = (edge1.f2 == INVALID);
        BOOL bEdge1Linked = LinkedEdges[i];

        for(j=i+1; j<EdgeList.Num(); j++)
        {
            if(DeleteEdges[j])
                continue;

            if(bEdge1Linked != LinkedEdges[j])
                continue;

            Edge &edge2 = EdgeList[j];
            DWORD middleVert = INVALID;

            //---------------------------------------
            // Check to see whether these edges are connected

            if( (edge1.v1 == edge2.v1) ||
                (edge1.v1 == edge2.v2) )
            {
                middleVert = edge1.v1;
            }
            else if( (edge1.v2 == edge2.v1) ||
                     (edge1.v2 == edge2.v2) )
            {
                middleVert = edge1.v2;
            }

            if(middleVert == INVALID)
                continue;

            if(middleVert < originalVertNum)
                continue;

            BOOL bEdge2Open = (edge2.f2 == INVALID);

            //if edges are not both open or closed
            if( (bEdge1Open && !bEdge2Open) ||
                (bEdge2Open && !bEdge1Open) )
                continue;

            //if both edges are open, check to see if they're both on the same sides.
            if(bEdge1Open)
            {
                if( (edge1.v1 == edge2.v1) || (edge1.v2 == edge2.v2) )
                    continue;

                //check to make sure the middle vertex is only on two open edges
                //if(OpenVerts[middleVert] > 2)
                //    continue;
            }
            //if both edges are closed, check to make sure the middle vert is also closed.
            else if(OpenVerts[middleVert])
                continue;

            //---------------------------------------

            DWORD v[3];
            Vect verts[3];

            v[0] = (edge1.v1 != middleVert) ? edge1.v1 : edge1.v2;
            v[1] = middleVert;
            v[2] = (edge2.v1 != middleVert) ? edge2.v1 : edge2.v2;

            verts[0] = VertList[v[0]];
            verts[1] = VertList[v[1]];
            verts[2] = VertList[v[2]];

            Vect Edge1Dir = (verts[2]-verts[1]).Norm();
            Vect Edge2Dir = (verts[1]-verts[0]).Norm();

            //check to see whether these edges are the same direction
            if(!Edge1Dir.CloseTo(Edge2Dir, TINY_EPSILON))
                continue;

            if(!MergeEdgeVerts(i, v[0], v[1]))
            {
                if(MergeEdgeVerts(j, v[2], v[1]))
                    --i;

                bNewPass = TRUE;
            }

            break;
        }
    }

    for(i=0; i<VertFaces.Num(); i++)
        VertFaces[i].Clear();
    VertFaces.Clear();

    for(i=0; i<VertEdges.Num(); i++)
        VertEdges[i].Clear();
    VertEdges.Clear();

    //---------------------------------------
    // Cleanup:  Delete Faces/Edges/Verts

    DWORD adjustID;

    List<DWORD> NewVertIDs;
    List<DWORD> NewFaceIDs;

    NewVertIDs.SetSize(VertList.Num());
    NewFaceIDs.SetSize(FaceList.Num());

    adjustID=0;
    for(i=0; i<VertList.Num(); i++)
    {
        if(DeleteVerts[i])
        {
            ++adjustID;
            continue;
        }

        NewVertIDs[i] = i-adjustID;
    }

    adjustID=0;
    for(i=0; i<FaceList.Num(); i++)
    {
        if(DeleteFaces[i])
        {
            ++adjustID;
            continue;
        }

        Face &face = FaceList[i];

        face.A = NewVertIDs[face.A];
        face.B = NewVertIDs[face.B];
        face.C = NewVertIDs[face.C];

        NewFaceIDs[i] = i-adjustID;
    }

    for(i=VertList.Num()-1; i>=0; i--)
    {
        if(DeleteVerts[i])
        {
            VertList.Remove(i);
            UVList.Remove(i);
            NormalList.Remove(i);
        }
    }

    for(i=FaceList.Num()-1; i>=0; i--)
    {
        if(DeleteFaces[i])
        {
            FaceList.Remove(i);
            FacePlaneList.Remove(i);
            FacePolyList.Remove(i);
            FaceSmoothList.Remove(i);
        }
    }

    DeleteEdges.Clear();
    DeleteFaces.Clear();
    DeleteVerts.Clear();

    EdgeList.Clear();

    return bNewPass;
}

void EditorMesh::MergeExcessFaces()
{
    long i,j;

    MakeEdges(FALSE);

    List<DWORD> OpenVerts;
    OpenVerts.SetSize(VertList.Num());

    DeleteFaces.SetSize(FaceList.Num());
    DeleteVerts.SetSize(VertList.Num());
    DeleteEdges.SetSize(EdgeList.Num());
    VertFaces.SetSize(VertList.Num());
    VertEdges.SetSize(VertList.Num());

    //Get Open Verticies
    for(i=0; i<EdgeList.Num(); i++)
    {
        Edge &edge = EdgeList[i];

        if(edge.f2 == INVALID)
        {
            OpenVerts[edge.v1]++;
            OpenVerts[edge.v2]++;
        }
        else
        {
            VertFaces[edge.v1].SafeAdd(edge.f2);
            VertFaces[edge.v2].SafeAdd(edge.f2);
        }

        VertFaces[edge.v1].SafeAdd(edge.f1);
        VertFaces[edge.v2].SafeAdd(edge.f1);

        VertEdges[edge.v1].SafeAdd(i);
        VertEdges[edge.v2].SafeAdd(i);
    }


    for(i=0; i<VertList.Num(); i++)
    {
        if(OpenVerts[i])
            continue;

        IDList &Faces = VertFaces[i];

        if(!Faces.Num())
        {
            DeleteVerts.Set(i);
            continue;
        }

        DWORD firstPlane = FacePlaneList[Faces[0]];
        BOOL bCoplanar = TRUE;

        for(j=1; j<Faces.Num(); j++)
        {
            if(FacePlaneList[Faces[j]] != firstPlane)
            {
                bCoplanar = FALSE;
                break;
            }
        }

        if(!bCoplanar)
            continue;

        IDList &Edges = VertEdges[i];
        long chosenEdgeID = -1;

        for(j=0; j<Edges.Num(); j++)
        {
            long edgeID = Edges[j];

            if(DeleteEdges[edgeID])
                continue;

            Edge &edge = EdgeList[edgeID];
            DWORD oppositeVert = (edge.v1 == i) ? edge.v2 : edge.v1;
            
            if(MergeEdgeVerts(edgeID, oppositeVert, i))
                break;
        }
    }

    for(i=0; i<VertFaces.Num(); i++)
        VertFaces[i].Clear();
    VertFaces.Clear();

    for(i=0; i<VertEdges.Num(); i++)
        VertEdges[i].Clear();
    VertEdges.Clear();

    //---------------------------------------
    // Cleanup:  Delete Faces/Edges/Verts

    DWORD adjustID;

    List<DWORD> NewVertIDs;
    List<DWORD> NewFaceIDs;

    NewVertIDs.SetSize(VertList.Num());
    NewFaceIDs.SetSize(FaceList.Num());

    adjustID=0;
    for(i=0; i<VertList.Num(); i++)
    {
        if(DeleteVerts[i])
        {
            ++adjustID;
            continue;
        }

        NewVertIDs[i] = i-adjustID;
    }

    adjustID=0;
    for(i=0; i<FaceList.Num(); i++)
    {
        if(DeleteFaces[i])
        {
            ++adjustID;
            continue;
        }

        Face &face = FaceList[i];

        face.A = NewVertIDs[face.A];
        face.B = NewVertIDs[face.B];
        face.C = NewVertIDs[face.C];

        NewFaceIDs[i] = i-adjustID;
    }

    for(i=VertList.Num()-1; i>=0; i--)
    {
        if(DeleteVerts[i])
        {
            VertList.Remove(i);
            UVList.Remove(i);
            NormalList.Remove(i);
        }
    }

    for(i=FaceList.Num()-1; i>=0; i--)
    {
        if(DeleteFaces[i])
        {
            FaceList.Remove(i);
            FacePlaneList.Remove(i);
            FacePolyList.Remove(i);
            FaceSmoothList.Remove(i);
        }
    }

    DeleteEdges.Clear();
    DeleteFaces.Clear();
    DeleteVerts.Clear();

    EdgeList.Clear();
}

BOOL EditorMesh::MergeEdgeVerts(DWORD mergingEdge, DWORD v1, DWORD v2)
{
    DWORD i,j,k;

    Edge &edge = EdgeList[mergingEdge];

    IDList &Vert1Faces = VertFaces[v1];
    IDList &Vert2Faces = VertFaces[v2];

    BOOL bGoodEdge = TRUE;

    for(k=0; k<Vert2Faces.Num(); k++)
    {
        long faceID = Vert2Faces[k];

        if(DeleteFaces[faceID])
            continue;

        if((faceID == edge.f1) || (faceID == edge.f2))
            continue;

        Face &face = FaceList[faceID];

        if((face.A == face.B) || (face.A == face.C) || (face.B == face.C))
            continue;

        Plane &plane = PlaneList[FacePlaneList[faceID]];

        Vect &fA = VertList[(face.A != v2) ? face.A : v1];
        Vect &fB = VertList[(face.B != v2) ? face.B : v1];
        Vect &fC = VertList[(face.C != v2) ? face.C : v1];

        Vect newDir = ((fB-fA)^(fC-fA)).Norm();



        if(!newDir.CloseTo(plane.Dir))
        {
            bGoodEdge = FALSE;
            break;
        }
    }

    if(!bGoodEdge)
        return FALSE;

    IDList &Vert1Edges = VertEdges[v1];
    IDList &Vert2Edges = VertEdges[v2];

    DeleteVerts.Set(v2);
    DeleteEdges.Set(mergingEdge);
    DeleteFaces.Set(edge.f1);

    if(edge.f2 != INVALID)
        DeleteFaces.Set(edge.f2);

    //---------------------------------------
    // Find Faces Connected to middleVert and make them use vert1 instead.

    for(i=0; i<Vert2Faces.Num(); i++)
    {
        DWORD faceID = Vert2Faces[i];

        if(DeleteFaces[faceID])
            continue;

        if( (faceID == edge.f1) ||
            (faceID == edge.f2) )
        {
            continue;
        }

        Face &face = FaceList[faceID];

        for(j=0; j<3; j++)
        {
            if(face.ptr[j] == v2)
            {
                face.ptr[j] = v1;
                break;
            }
        }
    }

    for(i=0; i<Vert1Faces.Num(); i++)
    {
        DWORD faceID = Vert1Faces[i];

        if(DeleteFaces[faceID])
        {
            Vert1Faces.Remove(i--);
            continue;
        }

        if( (faceID == edge.f1) ||
            (faceID == edge.f2) )
        {
            Vert1Faces.Remove(i--);
            continue;
        }
    }

    Vert1Faces.AppendList(Vert2Faces);

    //---------------------------------------
    // Remove the offset edges from connected to the middlevert from the deleted faces

    for(i=0; i<Vert2Edges.Num(); i++)
    {
        DWORD edgeID = Vert2Edges[i];

        if(edgeID == mergingEdge)
            continue;

        if(DeleteEdges[edgeID])
        {
            Vert2Edges.Remove(i--);
            continue;
        }

        Edge &vert2Edge = EdgeList[edgeID];

        for(j=0; j<2; j++)
        {
            if(vert2Edge.vptr[j] == v2)
            {
                vert2Edge.vptr[j] = v1;

                for(k=0; k<Vert1Edges.Num(); k++)
                {
                    Edge &vert1Edge = EdgeList[Vert1Edges[k]];

                    if(vert1Edge.EdgeConnects(vert2Edge))
                    {
                        DeleteEdges.Set(edgeID);
                        Vert2Edges.Remove(i--);

                        if(vert1Edge.f1 == vert2Edge.f1)
                            vert1Edge.f1 = vert2Edge.f2;
                        else if(vert1Edge.f1 == vert2Edge.f2)
                            vert1Edge.f1 = vert2Edge.f1;
                        else if(vert1Edge.f2 == vert2Edge.f1)
                            vert1Edge.f2 = vert2Edge.f2;
                        else if(vert1Edge.f2 == vert2Edge.f2)
                            vert1Edge.f2 = vert2Edge.f1;

                        if(vert1Edge.f1 == INVALID)
                        {
                            DWORD v1=vert1Edge.v2, v2=vert1Edge.v1;
                            DWORD f1=vert1Edge.f2, f2=vert1Edge.f1;

                            vert1Edge.v1 = v1;  vert1Edge.v2 = v2;
                            vert1Edge.f1 = f1;  vert1Edge.f2 = f2;
                        }
                        break;
                    }
                }
                break;
            }
        }
    }

    Vert1Edges.AppendList(Vert2Edges);

    return TRUE;
}

void EditorMesh::AddBareFace(const Vect &v1, const Vect &v2, const Vect &v3, const Plane &plane)
{
    Face newFace;

    newFace.A = VertList.SafeAdd(v1);
    newFace.B = VertList.SafeAdd(v2);
    newFace.C = VertList.SafeAdd(v3);

    FaceList << newFace;

    FacePlaneList << PlaneList.Add(plane);
}

void  EditorMesh::RemoveExcessFaces()
{
    bool bRemovedStuff;

    do
    {
        bRemovedStuff = false;
        for(UINT i=0; i<EdgeList.Num(); i++)
        {
            Edge &edge = EdgeList[i];

            if((edge.f1 == INVALID) || (FacePolyList[edge.f1] == INVALID))
            {
                if((edge.f2 != INVALID) && (FacePolyList[edge.f2] != INVALID))
                {
                    bRemovedStuff = true;
                    FacePolyList[edge.f2] = INVALID;
                    edge.f2 = INVALID;
                }
            }
            else if((edge.f2 == INVALID) || (FacePolyList[edge.f2] == INVALID))
            {
                if((edge.f1 != INVALID) && (FacePolyList[edge.f1] != INVALID))
                {
                    bRemovedStuff = true;
                    FacePolyList[edge.f1] = INVALID;
                    edge.f1 = INVALID;
                }
            }
        }
    }while(bRemovedStuff);

    RemoveUnassignedPolyFaces();
}

void  EditorMesh::RemoveExcessVerts()
{
    DeleteVerts.SetSize(VertList.Num());
    DeleteVerts.SetAll();

    for(int i=0; i<FaceList.Num(); i++)
    {
        Face &face = FaceList[i];

        DeleteVerts.Clear(face.A);
        DeleteVerts.Clear(face.B);
        DeleteVerts.Clear(face.C);
    }

    RemoveMarkedVerts();
}


void GetBaryCoordsFromPoint(Vect *face, const Plane &plane, const Vect &p, float *coords)
{
    Plane ACrossPlane, BCrossPlane, CCrossPlane;
    float maxDistA, maxDistB, maxDistC;

    ACrossPlane.Dir = (plane.Dir^(face[2]-face[1])).Norm();
    BCrossPlane.Dir = (plane.Dir^(face[0]-face[2])).Norm();
    CCrossPlane.Dir = (plane.Dir^(face[1]-face[0])).Norm();

    ACrossPlane.Dist = ACrossPlane.Dir|face[2];
    BCrossPlane.Dist = BCrossPlane.Dir|face[0];
    CCrossPlane.Dist = CCrossPlane.Dir|face[1];

    maxDistA = face[0].DistFromPlane(ACrossPlane);
    maxDistB = face[1].DistFromPlane(BCrossPlane);
    maxDistC = face[2].DistFromPlane(CCrossPlane);

    coords[0] = p.DistFromPlane(ACrossPlane)/maxDistA;
    coords[1] = p.DistFromPlane(BCrossPlane)/maxDistB;
    coords[2] = p.DistFromPlane(CCrossPlane)/maxDistC;
}

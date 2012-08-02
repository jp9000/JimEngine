/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditorMesh.cpp

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


EditorMesh::EditorMesh(const EditorMesh &mesh)
{
    traceIn(EditorMesh::EditorMesh);

    CopyEditorMesh(mesh);

    traceOut;
}

void EditorMesh::CopyEditorMesh(const EditorMesh &mesh)
{
    traceIn(EditorMesh::CopyEditorMesh);

    int i;

    PlaneList.CopyList(mesh.PlaneList);

    VertList.CopyList(mesh.VertList);
    UVList.CopyList(mesh.UVList);
    NormalList.CopyList(mesh.NormalList);

    FaceList.CopyList(mesh.FaceList);
    FacePlaneList.CopyList(mesh.FacePlaneList);
    FacePolyList.CopyList(mesh.FacePolyList);
    FaceSmoothList.CopyList(mesh.FaceSmoothList);

    FaceCuts.CopyList(mesh.FaceCuts);

    PolyBounds.CopyList(mesh.PolyBounds);

    PolyList.SetSize(mesh.PolyList.Num());
    for(i=0; i<PolyList.Num(); i++)
        PolyList[i].Faces.CopyList(mesh.PolyList[i].Faces);

    EdgeList.CopyList(mesh.EdgeList);
    originalVertNum = mesh.originalVertNum;
    originalFaceNum = mesh.originalFaceNum;

    IntersectingFaces.CopyList(mesh.IntersectingFaces);
    NonIntersectingFaces.CopyList(mesh.NonIntersectingFaces);

    NewVertList.CopyList(mesh.NewVertList);
    PHVertList.CopyList(mesh.PHVertList);

    DeleteFaces.CopyList(mesh.DeleteFaces);
    DeleteVerts.CopyList(mesh.DeleteVerts);
    DeleteEdges.CopyList(mesh.DeleteEdges);

    VertFaces.SetSize(mesh.VertFaces.Num());
    for(i=0; i<VertFaces.Num(); i++)
        VertFaces[i].CopyList(mesh.VertFaces[i]);
    VertEdges.SetSize(mesh.VertEdges.Num());
    for(i=0; i<VertEdges.Num(); i++)
        VertEdges[i].CopyList(mesh.VertEdges[i]);

    bDefaultCollisionOver = mesh.bDefaultCollisionOver;
    bSideCollisionEnabled = mesh.bSideCollisionEnabled;

    bounds = mesh.bounds;

    traceOut;
}

void EditorMesh::Clear()
{
    traceIn(EditorMesh::Clear);

    DWORD i;

    PlaneList.Clear();

    PHVertList.Clear();

    VertList.Clear();
    UVList.Clear();
    NormalList.Clear();
    FaceList.Clear();
    FacePlaneList.Clear();
    FacePolyList.Clear();
    FaceSmoothList.Clear();

    NonIntersectingFaces.Clear();
    IntersectingFaces.Clear();

    for(i=0; i<FaceCuts.Num(); i++)
        FaceCuts[i].Clear();
    FaceCuts.Clear();

    for(i=0; i<VertFaces.Num(); i++)
        VertFaces[i].Clear();
    VertFaces.Clear();

    for(i=0; i<VertEdges.Num(); i++)
        VertEdges[i].Clear();
    VertEdges.Clear();

    for(i=0; i<PolyList.Num(); i++)
        PolyList[i].Faces.Clear();
    PolyList.Clear();

    NewVertList.Clear();
    EdgeList.Clear();

    PolyBounds.Clear();

    for(i=0; i<PolyDataList.Num(); i++)
        PolyDataList[i].Clear();

    PolyDataList.Clear();

    traceOut;
}


void EditorMesh::MakeFromMesh(Mesh *mesh)
{
    traceIn(EditorMesh::MakeFromMesh);

    VBData *vbd = mesh->VertBuffer->GetData();
    VertList.CopyList(vbd->VertList);
    NormalList.CopyList(vbd->NormalList);
    UVList.CopyList(*vbd->TVList[0].GetV2());
    if(vbd->TVList.Num() == 2)
        LMUVList.CopyList(*vbd->TVList[1].GetV2());
    TangentList.CopyList(vbd->TangentList);

    FaceList.CopyArray(mesh->FaceList, mesh->nFaces);
    BuildPlaneList();

    PolyList.SetSize(FaceList.Num());
    FacePolyList.SetSize(FaceList.Num());
    for(int i=0; i<PolyList.Num(); i++)
    {
        PolyFace &poly = PolyList[i];
        poly.Faces << i;
        FacePolyList[i] = i;
    }

    MakeBounds();

    traceOut;
}

void EditorMesh::SaveToMesh(Mesh *mesh)
{
    traceIn(EditorMesh::SaveToMesh);

    mesh->nVerts = VertList.Num();
    mesh->nFaces = FaceList.Num();

    VBData *vbd = mesh->VertBuffer->GetData();
    vbd->VertList.CopyList(VertList);

    if(LMUVList.Num())
    {
        vbd->TVList.SetSize(2);
        vbd->TVList[1].SetWidth(2);
        vbd->TVList[1].GetV2()->CopyList(LMUVList);
    }

    vbd->TVList[0].SetSize(2);
    vbd->TVList[0].GetV2()->CopyList(UVList);
    vbd->NormalList.CopyList(NormalList);
    vbd->TangentList.CopyList(TangentList);

    DestroyObject(mesh->IdxBuffer);
    mesh->FaceList = (Face*)Allocate(FaceList.Num()*sizeof(Face));
    mcpy(mesh->FaceList, FaceList.Array(), FaceList.Num()*sizeof(Face));
    mesh->IdxBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, mesh->FaceList, FaceList.Num()*3);

    mesh->VertBuffer->FlushBuffers(TRUE);

    traceOut;
}

void EditorMesh::BuildPlaneList()
{
    PlaneList.Clear();
    FacePlaneList.SetSize(FaceList.Num());

    for(int i=0; i<FaceList.Num(); i++)
    {
        Face &f = FaceList[i];
        Plane p(VertList[f.A], VertList[f.B], VertList[f.C]);
        FacePlaneList[i] = PlaneList.SafeAdd(p);
    }
}


DWORD PlaneListHasValue(List<Plane> &list, const Plane &val, float epsilon)
{
    for(DWORD i=0; i<list.Num(); i++)
    {
        if(list[i].CloseTo(val, epsilon))
            return i;
    }

    return INVALID;
}

DWORD VectListHasValue(List<Vect> &list, const Vect &val, float epsilon)
{
    for(DWORD i=0; i<list.Num(); i++)
    {
        if(list[i].CloseTo(val, epsilon))
            return i;
    }

    return INVALID;
}


void EditorMesh::MakeEdges(BOOL bCombineEdges)
{
    DWORD i,j,k;

    EdgeList.Clear();

    for(i=0; i<FaceList.Num(); i++)
    {
        Face &face = FaceList[i];

        for(j=0; j<3; j++)
        {
            DWORD jp1 = (j == 2) ? 0 : j+1;
            DWORD v1 = face.ptr[j], v2 = face.ptr[jp1];
            BOOL bFoundEdge=FALSE;

            for(k=0; k<EdgeList.Num(); k++)
            {
                Edge &edge = EdgeList[k];

                if(bCombineEdges)
                {
                    Vect &e1v1 = VertList[v1],      &e1v2 = VertList[v2];
                    Vect &e2v1 = VertList[edge.v1], &e2v2 = VertList[edge.v2];

                    if( //(e1v1.CloseTo(e2v1) && e1v2.CloseTo(e2v2)) ||  //yes, this line was clearly retardation once again on my part
                        (e1v1.CloseTo(e2v2) && e1v2.CloseTo(e2v1)) )
                    {
                        bFoundEdge = TRUE;
                        edge.f2 = i;
                        break;
                    }
                }
                else
                {
                    if(edge.EdgeConnects(v1, v2))
                    {
                        bFoundEdge = TRUE;
                        edge.f2 = i;
                        break;
                    }
                }
            }

            if(!bFoundEdge)
            {
                Edge &newEdge = *EdgeList.CreateNew();

                newEdge.f1 = i;
                newEdge.f2 = INVALID;
                newEdge.v1 = v1;
                newEdge.v2 = v2;
            }
        }
    }
}


void EditorMesh::MakePolyEdges()
{
    MakeEdges();

    PolyEdgeDataList.SetSize(PolyList.Num());

    for(int i=0; i<EdgeList.Num(); i++)
    {
        Edge &edge = EdgeList[i];
        DWORD poly1 = FacePolyList[edge.f1];
        DWORD poly2 = (edge.f2 != INVALID) ? FacePolyList[edge.f2] : INVALID;

        if(poly1 != poly2)
        {
            Edge polyEdge;
            polyEdge.f1 = poly1;   polyEdge.f2 = poly2;
            polyEdge.v1 = edge.v1; polyEdge.v2 = edge.v2;

            int edgeID = PolyEdgeList.Add(polyEdge);

            PolyEdgeDataList[polyEdge.f1].PolyEdges << edgeID;
            if(poly2 != INVALID)
                PolyEdgeDataList[polyEdge.f2].PolyEdges << edgeID;
        }
    }
}

void EditorMesh::FreePolyEdges()
{
    for(int i=0; i<PolyEdgeDataList.Num(); i++)
        PolyEdgeDataList[i].PolyEdges.Clear();
    PolyEdgeDataList.Clear();
    PolyEdgeList.Clear();
}


// sort so that a<=b
#define SORT(a,b)       \
             if(a>b)    \
             {          \
               float c; \
               c=a;     \
               a=b;     \
               b=c;     \
             }


__forceinline void compute_interavals(float *axisVerts,
                                      float *triDists,
                                      BOOL bEdge1Intersects, BOOL bEdge2Intersects,
                                      Vect &interval,
                                      float *intervalDists)
{
    if(!bEdge1Intersects)
    {
        interval.x = axisVerts[2];
        interval.y = (axisVerts[0]-axisVerts[2]) * triDists[2];
        interval.z = (axisVerts[1]-axisVerts[2]) * triDists[2];

        intervalDists[0] = triDists[2]-triDists[0];
        intervalDists[1] = triDists[2]-triDists[1];
    }
    else if(!bEdge2Intersects)
    {
        interval.x = axisVerts[1];
        interval.y = (axisVerts[0]-axisVerts[1]) * triDists[1];
        interval.z = (axisVerts[2]-axisVerts[1]) * triDists[1];

        intervalDists[0] = triDists[1]-triDists[0];
        intervalDists[1] = triDists[1]-triDists[2];
    }
    else if((triDists[1]*triDists[2] > 0.0f) || (triDists[0] != 0.0f))
    {
        interval.x = axisVerts[0];
        interval.y = (axisVerts[1]-axisVerts[0]) * triDists[0];
        interval.z = (axisVerts[2]-axisVerts[0]) * triDists[0];

        intervalDists[0] = triDists[0]-triDists[1];
        intervalDists[1] = triDists[0]-triDists[2];
    }
    else if(triDists[1] != 0.0f)
    {
        interval.x = axisVerts[1];
        interval.y = (axisVerts[0]-axisVerts[1]) * triDists[1];
        interval.z = (axisVerts[2]-axisVerts[1]) * triDists[1];
        
        intervalDists[0] = triDists[1]-triDists[0];
        intervalDists[1] = triDists[1]-triDists[2];
    }
    else if(triDists[2] != 0.0f)
    {
        interval.x = axisVerts[2];
        interval.y = (axisVerts[0]-axisVerts[2]) * triDists[2];
        interval.z = (axisVerts[1]-axisVerts[2]) * triDists[2];

        intervalDists[0] = triDists[2]-triDists[0];
        intervalDists[1] = triDists[2]-triDists[1];
    }
}


BOOL EditorMesh::TriangleTriangleTest(DWORD face1ID, DWORD face2ID)
{
    Face &face1 = FaceList[face1ID];
    Face &face2 = intersectingMesh->FaceList[face2ID];

    Vect *F1Verts[3] = {&VertList[face1.A],
                        &VertList[face1.B],
                        &VertList[face1.C]};

    Vect *F2Verts[3] = {&intersectingMesh->VertList[face2.A],
                        &intersectingMesh->VertList[face2.B],
                        &intersectingMesh->VertList[face2.C]};

    //--------------------------------------------------------------------

    Plane &F1Plane = PlaneList[FacePlaneList[face1ID]];

    // calculate the distance of face 2's verticies from plane 1
    float F2Dists[3] = {F2Verts[0]->DistFromPlane(F1Plane),
                        F2Verts[1]->DistFromPlane(F1Plane),
                        F2Verts[2]->DistFromPlane(F1Plane)};

    // percision adjustment
    if(fabsf(F2Dists[0]) < EPSILON) F2Dists[0] = 0.0f;
    if(fabsf(F2Dists[1]) < EPSILON) F2Dists[1] = 0.0f;
    if(fabsf(F2Dists[2]) < EPSILON) F2Dists[2] = 0.0f;

    // no idea what this stuff is for.  perhaps that comment about me being retarded was correct because I can't remember
    BOOL F2Edge1Intersects = (F2Dists[0]*F2Dists[1]) <= 0.0f;
    BOOL F2Edge2Intersects = (F2Dists[0]*F2Dists[2]) <= 0.0f;

    // if both edges are only on one side of plane 1, then the triangle is not intersecting.
    if(!F2Edge1Intersects && !F2Edge2Intersects)
        return FALSE;

    //--------------------------------------------------------------------

    Plane &F2Plane = intersectingMesh->PlaneList[intersectingMesh->FacePlaneList[face2ID]];

    if(CloseFloat(fabsf(F1Plane.Dir.Dot(F2Plane.Dir)), 1.0f))
        return FALSE;

    // put V0,V1,V2 into plane equation 2
    float F1Dists[3] = {F1Verts[0]->DistFromPlane(F2Plane),
                        F1Verts[1]->DistFromPlane(F2Plane),
                        F1Verts[2]->DistFromPlane(F2Plane)};

    // percision adjustment
    if(fabsf(F1Dists[0]) < EPSILON) F1Dists[0] = 0.0;
    if(fabsf(F1Dists[1]) < EPSILON) F1Dists[1] = 0.0;
    if(fabsf(F1Dists[2]) < EPSILON) F1Dists[2] = 0.0;

    BOOL F1Edge1Intersects = (F1Dists[0]*F1Dists[1]) <= 0.0f;
    BOOL F1Edge2Intersects = (F1Dists[0]*F1Dists[2]) <= 0.0f;

    // if both edges are only on one side of plane 2, then the triangle is not intersecting.
    if(!F1Edge1Intersects && !F1Edge2Intersects)
        return FALSE;

    //--------------------------------------------------------------------

    // compute direction of intersection line
    Vect &LineDir = F1Plane.Dir.Cross(F2Plane.Dir);

    // choose the largest axis of the line direction
    int axisIndex = 0;
    float axisX   = fabsf(LineDir.x);
    float axisY   = fabsf(LineDir.y);
    float axisZ   = fabsf(LineDir.z);

    if(axisY > axisX) 
    {
        axisX = axisY;
        axisIndex = 1;
    }
    if(axisZ > axisX)
        axisIndex = 2;

    // get the verticies of both triangles from the chosen aligned axis
    Vect F1AA(F1Verts[0]->ptr[axisIndex], F1Verts[1]->ptr[axisIndex], F1Verts[2]->ptr[axisIndex]);
    Vect F2AA(F2Verts[0]->ptr[axisIndex], F2Verts[1]->ptr[axisIndex], F2Verts[2]->ptr[axisIndex]);

    //--------------------------------------------------------------------
    // at this point, I'm lost

    Vect interval1, interval2;
    float interval1Dists[2], interval2Dists[2];

    // compute interval for triangle 1
    compute_interavals(F1AA, F1Dists, F1Edge1Intersects, F1Edge2Intersects, interval1, interval1Dists);

    // compute interval for triangle 2
    compute_interavals(F2AA, F2Dists, F2Edge1Intersects, F2Edge2Intersects, interval2, interval2Dists);

    //--------------------------------------------------------------------
    // uh, what's this stuff do again?

    float xx,yy,xxyy,tmp;
    float isect1[2], isect2[2];

    xx   = interval1Dists[0] * interval1Dists[1];
    yy   = interval2Dists[0] * interval2Dists[1];
    xxyy = xx*yy;

    tmp = interval1.x*xxyy;
    isect1[0] = tmp + (interval1.y*yy*interval1Dists[1]);
    isect1[1] = tmp + (interval1.z*yy*interval1Dists[0]);

    tmp = interval2.x*xxyy;
    isect2[0] = tmp + (interval2.y*xx*interval2Dists[1]);
    isect2[1] = tmp + (interval2.z*xx*interval2Dists[0]);

    SORT(isect1[0], isect1[1]);
    SORT(isect2[0], isect2[1]);

    if((isect1[1] <= isect2[0]) || (isect2[1] < isect1[0]))
        return FALSE;

    return TRUE;
}


void EditorMesh::MakeBounds()
{
    if(!VertList.Num())
        return;

    DWORD i;

    bounds.Min = VertList[0];
    for(i=1; i<VertList.Num(); i++)
    {
        bounds.Min.x = MIN(bounds.Min.x, VertList[i].x);
        bounds.Min.y = MIN(bounds.Min.y, VertList[i].y);
        bounds.Min.z = MIN(bounds.Min.z, VertList[i].z);
    }

    bounds.Max = VertList[0];
    for(i=1; i<VertList.Num(); i++)
    {
        bounds.Max.x = MAX(bounds.Max.x, VertList[i].x);
        bounds.Max.y = MAX(bounds.Max.y, VertList[i].y);
        bounds.Max.z = MAX(bounds.Max.z, VertList[i].z);
    }
}


void EditorMesh::RebuildPolyBounds()
{
    DWORD i, j, k;

    PolyBounds.SetSize(PolyList.Num());

    for(i=0; i<PolyList.Num(); i++)
    {
        PolyFace &poly = PolyList[i];
        BOOL bInitializedBounds = FALSE;

        for(j=0; j<poly.Faces.Num(); j++)
        {
            Face &face = FaceList[poly.Faces[j]];

            Vect *v[3] = {&VertList[face.A], &VertList[face.B], &VertList[face.C]};

            Bounds &bounds = PolyBounds[i];

            if(!bInitializedBounds)
            {
                bounds.Min = *v[0];
                for(k=1; k<3; k++)
                {
                    bounds.Min.x = MIN(bounds.Min.x, v[k]->x);
                    bounds.Min.y = MIN(bounds.Min.y, v[k]->y);
                    bounds.Min.z = MIN(bounds.Min.z, v[k]->z);
                }
                bounds.Max = *v[0];
                for(k=1; k<3; k++)
                {
                    bounds.Max.x = MAX(bounds.Max.x, v[k]->x);
                    bounds.Max.y = MAX(bounds.Max.y, v[k]->y);
                    bounds.Max.z = MAX(bounds.Max.z, v[k]->z);
                }

                bInitializedBounds = TRUE;
            }
            else
            {
                for(k=0; k<3; k++)
                {
                    bounds.Min.x = MIN(bounds.Min.x, v[k]->x);
                    bounds.Min.y = MIN(bounds.Min.y, v[k]->y);
                    bounds.Min.z = MIN(bounds.Min.z, v[k]->z);
                }
                for(k=0; k<3; k++)
                {
                    bounds.Max.x = MAX(bounds.Max.x, v[k]->x);
                    bounds.Max.y = MAX(bounds.Max.y, v[k]->y);
                    bounds.Max.z = MAX(bounds.Max.z, v[k]->z);
                }
            }
        }
    }
}


void EditorMesh::RebuildPolyList(BOOL bSortFaces)
{
    DWORD i,j;

    for(i=0; i<PolyList.Num(); i++)
        PolyList[i].Faces.Clear();
    PolyList.Clear();

    for(i=0; i<FaceList.Num(); i++)
    {
        DWORD polyID = FacePolyList[i];

        if(polyID >= PolyList.Num())
            PolyList.SetSize(polyID+1);
        PolyList[polyID].Faces << i;
    }

    if(bSortFaces)
    {
        List<Face>  NewFaceList;
        List<DWORD> NewFacePlaneList;
        List<DWORD> NewFacePolyList;
        List<DWORD> NewFaceSideList;
        List<DWORD> NewFaceSmoothList;

        DWORD faceCount = 0;

        for(i=0; i<PolyList.Num(); i++)
        {
            PolyFace &poly = PolyList[i];

            for(j=0; j<poly.Faces.Num(); j++)
            {
                DWORD &faceID = poly.Faces[j];

                NewFaceList << FaceList[faceID];
                NewFacePlaneList << FacePlaneList[faceID];
                NewFaceSmoothList << FaceSmoothList[faceID];
                if(FaceSideList.Num())
                    NewFaceSideList << FaceSideList[faceID];
                NewFacePolyList << i;

                faceID = faceCount++;
            }
        }

        FaceList.Clear();
        FacePlaneList.Clear();
        FaceSmoothList.Clear();
        FaceSideList.Clear();
        FacePolyList.Clear();

        FaceList.CopyList(NewFaceList);
        FacePlaneList.CopyList(NewFacePlaneList);
        FaceSmoothList.CopyList(NewFaceSmoothList);
        FaceSideList.CopyList(NewFaceSideList);
        FacePolyList.CopyList(NewFacePolyList);
    }
}


BOOL EditorMesh::RayTriangleTest(DWORD triangle, const Vect &rayOrig, const Vect &rayDir, BOOL bFrontOnly, float *fT)
{
    Plane &plane = PlaneList[FacePlaneList[triangle]];

    BOOL bInverted = FALSE;

    float fDot = plane.Dir.Dot(rayDir);

    if(bFrontOnly)
    {
        if(fDot >= -0.001f)
            return FALSE;
    }
    else
    {
        if(fabs(fDot) <= 0.001f)
            return FALSE;

        if(fDot > 0.0f)
            bInverted = TRUE;
    }

    Face &face = FaceList[triangle];
    Vect *v[3] = {&VertList[face.A], &VertList[face.B], &VertList[face.C]};

    Vect edgeVec1 = (bInverted) ? (*v[2] - *v[0]) : (*v[1] - *v[0]);
    Vect edgeVec2 = (bInverted) ? (*v[1] - *v[0]) : (*v[2] - *v[0]);

    Vect pVec = rayDir.Cross(edgeVec2);

    float fDet = pVec.Dot(edgeVec1);

    //----------

    Vect  tVec = rayOrig - *v[0];

    float fA = tVec.Dot(pVec);
    if((fA < 0.0f) || (fA > fDet))
        return FALSE;

    Vect qVec = tVec.Cross(edgeVec1);

    float fB = rayDir.Dot(qVec);
    if((fB < 0.0f) || ((fA+fB) > fDet))
        return FALSE;

    if(fT)
        *fT = edgeVec2.Dot(qVec)/fDet;

    return TRUE;
}

void  EditorMesh::RemoveEmptyPolys()
{
    int adjust = 0;

    for(int i=0; i<PolyList.Num(); i++)
    {
        PolyFace &poly = PolyList[i];

        if(poly.Faces.Num())
        {
            if(adjust)
            {
                for(int j=0; j<poly.Faces.Num(); j++)
                    FacePolyList[poly.Faces[j]] += adjust;
            }
        }
        else
        {
            PolyList.Remove(i);

            if(PolyInfoList.Num())
                PolyInfoList.Remove(i);

            --adjust;
            --i;
        }
    }
}

void  EditorMesh::RemoveUnassignedPolyFaces()
{
    for(int i=FaceList.Num()-1; i>=0; i--)
    {
        if(FacePolyList[i] == INVALID)
            RemoveFace(i);
    }
}


BOOL  EditorMesh::PointInsideMesh(const Vect &p)
{
    DWORD i;

    Vect rayDir(0.0f, 1.0f, 0.0f);

    DWORD bestPoly = INVALID;
    float bestDist = M_INFINITE;

    DWORD numHits = 0;

    for(i=0; i<FaceList.Num(); i++)
    {
        float curFT;

        if(RayTriangleTest(i, p, rayDir, FALSE, &curFT))
        {
            if(curFT > 0.0f)
                ++numHits;
        }
    }

    return (numHits%2);
}

DWORD EditorMesh::RayMeshTest(const Vect &rayOrig, const Vect &rayDir, float *fT, Plane *collisionPlane)
{
    DWORD i;

    DWORD bestPoly = INVALID;
    float bestDist = M_INFINITE;

    for(i=0; i<FaceList.Num(); i++)
    {
        float curFT;

        if(RayTriangleTest(i, rayOrig, rayDir, TRUE, &curFT))
        {
            if((curFT > 0.0f) && (curFT < bestDist))
            {
                bestDist = curFT;
                bestPoly = FacePolyList[i];

                if(fT)
                    *fT = curFT;

                if(collisionPlane)
                    *collisionPlane = PlaneList[FacePlaneList[i]];
            }
        }
    }

    return bestPoly;
}


void EditorMesh::DumpMesh()
{
	int i;

    for(i=0; i<VertList.Num(); i++)
    {
        Vect &v = VertList[i];
        UVCoord &uv = UVList[i];
        Vect &n = NormalList[i];

        OSDebugOut(TEXT("Vert %02d:\t%f, %f, %f\tUV: %f, %f\tNorm: %f, %f, %f\r\n"),
            i,
            v.x, v.y, v.z,
            uv.x, uv.y,
            n.x, n.y, n.z);
    }

    OSDebugOut(TEXT("\r\n\r\n"));

    for(i=0; i<FaceList.Num(); i++)
    {
        Face &face = FaceList[i];

        OSDebugOut(TEXT("Face %02d: %02d, %02d, %02d\tPoly: %02d\tSmooth: 0x%08lX\r\n"),
            i,
            face.A, face.B, face.C,
            FacePolyList[i],
            FaceSmoothList[i]);
    }
}


void EditorMesh::RebuildPlaneListThingy()
{
    int i;

    PlaneList.Clear();

    for(i=0; i<FaceList.Num(); i++)
    {
        Face &face = FaceList[i];

        Plane chi(VertList[face.A], VertList[face.B], VertList[face.C]);

        FacePlaneList[i] = PlaneList.SafeAdd(chi);
    }
}

void EditorMesh::RemoveMarkedFaces(bool bRebuildPolys, bool bRemoveVerts)
{
    if(DeleteFaces.Num() != FaceList.Num())
    {
        AppWarning(TEXT("EditorMesh::DeleteFaces must be the same size as the number of faces in the mesh"));
        return;
    }

    for(int i=DeleteFaces.Num()-1; i>=0; i--)
    {
        if(DeleteFaces[i])
            RemoveFace(i);
    }

    if(bRebuildPolys)
        RebuildPolyList(false);

    if(bRemoveVerts)
        RemoveExcessVerts();

    DeleteFaces.Clear();
}

void EditorMesh::RemoveMarkedVerts()
{
    if(DeleteVerts.Num() != VertList.Num())
    {
        AppWarning(TEXT("EditorMesh::DeleteVerts must be the same size as the number of verts in the mesh"));
        return;
    }

    List<DWORD> NewIndices;
    NewIndices.SetSize(VertList.Num());

    int adjustVert=0;
    for(int i=0; i<DeleteVerts.Num(); i++)
    {
        if(DeleteVerts[i])
            --adjustVert;
        NewIndices[i] = i+adjustVert;
    }

    for(int i=DeleteVerts.Num()-1; i>=0; i--)
    {
        if(DeleteVerts[i])
        {
            VertList.Remove(i);
            if(UVList.Num())
                UVList.Remove(i);
            if(NormalList.Num())
                NormalList.Remove(i);
            if(TangentList.Num())
                TangentList.Remove(i);
            if(LMUVList.Num())
                LMUVList.Remove(i);
        }
    }

    for(int i=0; i<FaceList.Num(); i++)
    {
        Face &face = FaceList[i];

        for(int j=0; j<3; j++)
            face.ptr[j] = NewIndices[face.ptr[j]];
    }

    DeleteVerts.Clear();
}


Vect EditorMesh::GetFaceTangent(DWORD face)
{
    Face &f = FaceList[face];
    Plane &plane = PlaneList[FacePlaneList[face]];

    Vect vec1 = VertList[f.B] - VertList[f.A];
    Vect vec2 = VertList[f.C] - VertList[f.A];

    double deltaU1 = (UVList[f.B].x - UVList[f.A].x);
    double deltaU2 = (UVList[f.C].x - UVList[f.A].x);
    double deltaV1 = (UVList[f.B].y - UVList[f.A].y);
    double deltaV2 = (UVList[f.C].y - UVList[f.A].y);

    //---------------------

    double a = (deltaU1 - deltaV1*deltaU2/deltaV2);
    if(a != 0.0) a = 1.0/a;
    double b = (deltaU2 - deltaV2*deltaU1/deltaV1);
    if(b != 0.0) b = 1.0/b;

    Vect duTemp = ((vec1*a) + (vec2*b));
    double tempf = 1.0 / sqrt(duTemp|duTemp);
    duTemp *= tempf;

    Vect norm1 = plane.Dir;
    tempf = duTemp|norm1;
    return (duTemp - (norm1*tempf)).Norm();
}

Serializer& operator<<(Serializer &s, EditorMesh &mesh)
{
    traceIn(operator<<(Serializer&, EditorMesh&));

    Vect::SerializeList(s, mesh.VertList);//s   << mesh.VertList
    s   << mesh.UVList
        << mesh.FaceList
        << mesh.FacePolyList
        << mesh.FaceSmoothList
        << mesh.FacePlaneList;
    Plane::SerializeList(s, mesh.PlaneList);
    s   << mesh.bounds;

    if(s.IsLoading())
    {
        DWORD numPolys = mesh.PolyList.Num();
        for(int i=0; i<numPolys; i++)
            mesh.PolyList[i].Faces.Clear();

        s << numPolys;

        mesh.PolyList.SetSize(numPolys);

        for(int i=0; i<numPolys; i++)
            s << mesh.PolyList[i].Faces;
    }
    else
    {
        DWORD numPolys = mesh.PolyList.Num();
        s << numPolys;

        for(int i=0; i<numPolys; i++)
            s << mesh.PolyList[i].Faces;
    }

    return s;

    traceOut;
}

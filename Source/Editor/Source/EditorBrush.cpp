/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditorBrush.cpp

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


DefineClass(EditorBrush);


struct VertDupe
{
    DWORD vert;
    DWORD uvVert;
    DWORD polyID;
    DWORD smooth;
    BOOL  bTVMirrored;
};
typedef List<VertDupe> VertDupeList;


bool EditFace::FacesConnect(const EditFace &face2, DWORD &e1, DWORD &e2) const
{
    DWORD i, j;

    for(i=0; i<3; i++)
    {
        DWORD ip1 = (i == 2) ? 0 : i+1;

        for(j=0; j<3; j++)
        {
            DWORD jp1 = (j == 2) ? 0 : j+1;

            if( (pointFace.ptr[i]   == face2.pointFace.ptr[jp1]) &&
                (pointFace.ptr[ip1] == face2.pointFace.ptr[j])   )
            {
                e1 = i;
                e2 = j;
                return true;
            }
        }
    }

    return false;
}



EditorBrush::EditorBrush()
{
    traceIn(EditorBrush::EditorBrush);

    brushID = -1;

    bRenderable = TRUE;

    mesh.brush = this;

    lightmapResolution = 256;
    bUseLightmapping = TRUE;

    bCanSubtract = TRUE;

    traceOut;
}

EditorBrush::~EditorBrush()
{
    traceIn(EditorBrush::~EditorBrush);

    levelInfo->Deselect(this);

    Clear();

    if(levelInfo->WorkBrush == this)
        levelInfo->WorkBrush = NULL;
    else
        levelInfo->BrushList.RemoveItem(this);

    traceOut;
}


void EditorBrush::Clear()
{
    traceIn(EditorBrush::Clear);

    DWORD i;

    FaceList.Clear();
    PointList.Clear();
    UVList.Clear();
    VisibleEdgeList.Clear();
    SelectedPolys.Clear();

    for(i=0; i<PolyList.Num(); i++)
    {
        PolyList[i].Faces.Clear();
        if(Materials[i])
            RM->ReleaseMaterial(Materials[i]);
    }
    PolyList.Clear();
    Materials.Clear();

    delete SelectionIdxBuffer;

    FreeWireframeBuffers();

    traceOut;
}


void EditorBrush::ProcessBasicMeshData()
{
    traceInFast(EditorBrush::ProcessBasicMeshData);

    DWORD i,j,k;

    for(i=0; i<PolyList.Num(); i++)
        PolyList[i].Faces.Clear();
    PolyList.Clear();

    for(i=0; i<FaceList.Num(); i++)
    {
        EditFace &face = FaceList[i];

        mset(face.edgeVisible, 0xFF, sizeof(bool)*3);

        if(face.polyID >= PolyList.Num())
            PolyList.SetSize(face.polyID+1);
        PolyList[face.polyID].Faces << i;
    }

    for(int i=PolyList.Num(); i<Materials.Num(); i++)
    {
        if(Materials[i])
            RM->ReleaseMaterial(Materials[i]);
    }
    Materials.SetSize(PolyList.Num());

    for(i=0; i<PolyList.Num(); i++)
    {
        PolyFace &poly = PolyList[i];

        if(poly.Faces.Num() < 2)
            continue;

        for(j=0; j<poly.Faces.Num(); j++)
        {
            EditFace &face1 = FaceList[poly.Faces[j]];

            for(k=j+1; k<poly.Faces.Num(); k++)
            {
                EditFace &face2 = FaceList[poly.Faces[k]];

                DWORD e1, e2;
                if(face1.FacesConnect(face2, e1, e2))
                {
                    face1.edgeVisible[e1] = false;
                    face2.edgeVisible[e2] = false;
                }
            }
        }
    }

    RebuildVisibleEdges();
    RebuildWireframeBuffers();

    traceOutFast;
}

void EditorBrush::RebuildVisibleEdges()
{
    traceInFast(EditorBrush::RebuildVisibleEdges);

    DWORD i, j;

    VisibleEdgeList.Clear();

    for(i=0; i<FaceList.Num(); i++)
    {
        EditFace &face = FaceList[i];

        for(j=0; j<3; j++)
        {
            if(face.edgeVisible[j])
            {
                DWORD jp1 = (j == 2) ? 0 : j+1;
                SimpleEdge edge = {face.pointFace.ptr[j], face.pointFace.ptr[jp1]};
                VisibleEdgeList.SafeAdd(edge);
            }
        }
    }

    traceOutFast;
}


void EditorBrush::RebuildWireframeBuffers()
{
    traceInFast(EditorBrush::RebuildWireframeBuffers);

    FreeWireframeBuffers();

    VBData *vbd = new VBData;
    vbd->VertList.CopyList(PointList);
    WFVertBuffer = CreateVertexBuffer(vbd);

    DWORD *indexList = (DWORD*)Allocate(VisibleEdgeList.Num()*sizeof(SimpleEdge));
    mcpy(indexList, VisibleEdgeList.Array(), VisibleEdgeList.Num()*sizeof(SimpleEdge));

    WFIdxBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, indexList, VisibleEdgeList.Num()*2);

    traceOutFast;
}

void EditorBrush::RebuildFaceNormals()
{
    traceInFast(EditorBrush::RebuildFaceNormals);

    DWORD i;

    FaceNormals.SetSize(FaceList.Num());

    for(i=0; i<FaceList.Num(); i++)
    {
        EditFace &face = FaceList[i];

        Vect &A = PointList[face.pointFace.A];
        Vect &B = PointList[face.pointFace.B];
        Vect &C = PointList[face.pointFace.C];

        FaceNormals[i] = ((B-A)^(C-A)).Norm();
    }

    traceOutFast;
}

void EditorBrush::RebuildNormals()
{
    traceInFast(EditorBrush::RebuildNormals);

    DWORD i, j, k;

    NormalList.Clear();

    DWORD smoothVals = 0;

    for(i=0; i<FaceList.Num(); i++)
    {
        EditFace &face = FaceList[i];
        smoothVals |= face.smoothFlags;

        face.normalFace.A = 
        face.normalFace.B = 
        face.normalFace.C = INVALID;

        if(!face.smoothFlags)
        {
            Vect &faceNorm = FaceNormals[i];

            DWORD normID = NormalList.Add(faceNorm);

            for(k=0; k<3; k++)
                face.normalFace.ptr[k] = normID;
        }
    }

    /*for(i=0; i<32; i++)
    {
        DWORD smooth = 1<<i;
        if(smoothVals & smooth)
        {
            for(j=0; j<FaceList.Num(); j++)
            {
                EditFace &face = FaceList[j];

                if(face.smoothFlags & smooth)
                {
                    Vect &faceNorm = FaceNormals[j];

                    for(k=0; k<3; k++)
                    {
                        if(face.normalFace.ptr[k] == INVALID)
                            face.normalFace.ptr[k] = NormalList.Add(faceNorm);
                        else
                            NormalList[face.normalFace.ptr[k]] += faceNorm;
                    }
                }
            }
        }
    }*/

    for(int smoothID=0; smoothID<32; smoothID++)
    {
        DWORD smooth = 1<<smoothID;
        if(smoothVals & smooth)
        {
            for(i=0; i<PointList.Num(); i++)
            {
                DWORD vertNum = INVALID;

                for(j=0; j<FaceList.Num(); j++)
                {
                    EditFace &face = FaceList[j];

                    if(face.smoothFlags & smooth)
                    {
                        Vect &faceNorm = FaceNormals[j];

                        for(k=0; k<3; k++)
                        {
                            if(face.pointFace.ptr[k] == i)
                            {
                                if(vertNum == INVALID)
                                    vertNum = NormalList.Add(faceNorm);
                                else
                                    NormalList[vertNum] += faceNorm;

                                face.normalFace.ptr[k] = vertNum;
                            }
                        }
                    }
                }
            }
        }
    }

    for(i=0; i<NormalList.Num(); i++)
        NormalList[i].Norm();

    traceOutFast;
}

void EditorBrush::MergeAllDuplicateVertices()
{
    traceInFast(EditorBrush::MergeAllDuplicateVertices);

    int i,j;
    List<DWORD>  PointDupeList;
    BitList      PointKillList;
    List<DWORD>  PointAdjustList;
    List<DWORD>  NormalDupeList;
    BitList      NormalKillList;
    List<DWORD>  NormalAdjustList;
    BitList      UVKillList;
    List<DWORD>  UVAdjustList;
    DWORD pointAdjust=0,normalAdjust=0,uvAdjust=0;

    PointDupeList.SetSize(PointList.Num());
    PointKillList.SetSize(PointList.Num());
    PointAdjustList.SetSize(PointList.Num());

    for(i=0; i<PointList.Num(); i++)
    {
        if(PointKillList[i])
        {
            ++pointAdjust;
            continue;
        }

        PointAdjustList[i] = i-pointAdjust;

        Vect &p1 = PointList[i];

        for(j=i+1; j<PointList.Num(); j++)
        {
            if(PointKillList[j])
                continue;

            Vect &p2 = PointList[j];

            if(p1.CloseTo(p2))
            {
                PointDupeList[j] = i+1;
                PointKillList.Set(j);
            }
        }
    }

    NormalDupeList.SetSize(NormalList.Num());
    NormalKillList.SetSize(NormalList.Num());
    NormalAdjustList.SetSize(NormalList.Num());

    for(i=0; i<NormalList.Num(); i++)
    {
        if(NormalKillList[i])
        {
            ++normalAdjust;
            continue;
        }

        NormalAdjustList[i] = i-normalAdjust;

        Vect &p1 = NormalList[i];

        for(j=i+1; j<NormalList.Num(); j++)
        {
            if(NormalKillList[j])
                continue;

            Vect &p2 = NormalList[j];

            if(p1.CloseTo(p2))
            {
                NormalDupeList[j] = i+1;
                NormalKillList.Set(j);
            }
        }
    }

    for(i=0; i<FaceList.Num(); i++)
    {
        EditFace &face = FaceList[i];

        for(j=0; j<3; j++)
        {
            if(PointDupeList.Num())
            {
                DWORD pointDupe       = PointDupeList[face.pointFace.ptr[j]];
                face.pointFace.ptr[j] = PointAdjustList[pointDupe ? (pointDupe-1) : face.pointFace.ptr[j]];
            }

            if(NormalDupeList.Num())
            {
                DWORD normalDupe       = NormalDupeList[face.normalFace.ptr[j]];
                face.normalFace.ptr[j] = NormalAdjustList[normalDupe ? (normalDupe-1) : face.normalFace.ptr[j]];
            }
        }
    }

    for(i=PointList.Num()-1; i>=0; i--)
    {
        if(PointKillList[i])
            PointList.Remove(i);
    }

    for(i=NormalList.Num()-1; i>=0; i--)
    {
        if(NormalKillList[i])
            NormalList.Remove(i);
    }

    //-----------------------------------------------------

    UVKillList.SetSize(UVList.Num());
    UVAdjustList.SetSize(UVList.Num());

    for(int polyID=0; polyID<PolyList.Num(); polyID++)
    {
        PolyFace &poly = PolyList[polyID];
        for(int face1ID=0; face1ID<poly.Faces.Num(); face1ID++)
        {
            EditFace &face1 = FaceList[poly.Faces[face1ID]];

            for(int face2ID=face1ID+1; face2ID<poly.Faces.Num(); face2ID++)
            {
                EditFace &face2 = FaceList[poly.Faces[face2ID]];

                for(i=0; i<3; i++)
                {
                    if(UVKillList[face1.uvFace.ptr[i]]) continue;

                    for(j=0; j<3; j++)
                    {
                        if(UVKillList[face1.uvFace.ptr[j]]) continue;

                        if(face1.pointFace.ptr[i] == face2.pointFace.ptr[j])
                        {
                            if(face1.uvFace.ptr[i] != face2.uvFace.ptr[j])
                            {
                                if(face1.uvFace.ptr[i] > face2.uvFace.ptr[j])
                                {
                                    NormalDupeList[face1.uvFace.ptr[i]] = face2.uvFace.ptr[j]+1;
                                    UVKillList.Set(face1.uvFace.ptr[i]);
                                }
                                else
                                {
                                    NormalDupeList[face2.uvFace.ptr[j]] = face1.uvFace.ptr[i]+1;
                                    UVKillList.Set(face2.uvFace.ptr[j]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for(i=0; i<UVList.Num(); i++)
    {
        if(UVKillList[i])
        {
            ++uvAdjust;
            continue;
        }

        UVAdjustList[i] = i-uvAdjust;
    }

    for(i=0; i<FaceList.Num(); i++)
    {
        EditFace &face = FaceList[i];

        for(j=0; j<3; j++)
            face.uvFace.ptr[j] = UVAdjustList[face.uvFace.ptr[j]];
    }

    for(i=UVList.Num()-1; i>=0; i--)
    {
        if(UVKillList[i])
            UVList.Remove(i);
    }

    traceOutFast;
}


void EditorBrush::FreeWireframeBuffers()
{
    traceIn(EditorBrush::FreeWireframeBuffers);

    delete WFVertBuffer;
    delete WFIdxBuffer;

    WFVertBuffer = NULL;
    WFIdxBuffer  = NULL;

    traceOut;
}

void EditorBrush::BuildEditorMesh(bool bTransform)
{
    traceIn(EditorBrush::BuildEditorMesh);

    mesh.Clear();

    Matrix mat;
    mat.SetIdentity();
    mat.Translate(GetLocalPos());
    mat.Rotate(GetLocalRot());

    List<VertDupeList> Dupes;
    DWORD i,j,k,l;

    mesh.PolyList.SetSize(PolyList.Num());

    Dupes.SetSize(PointList.Num());

    for(i=0; i<PolyList.Num(); i++)
    {
        PolyFace &face = PolyList[i];

        for(j=0; j<FaceList.Num(); j++)
        {
            EditFace &editFace = FaceList[j];

            if(editFace.polyID == i)
            {
                Face newFace;

                newFace.A = newFace.B = newFace.C = INVALID;

                Vect uv1(UVList[editFace.uvFace.A]);
                Vect uv2(UVList[editFace.uvFace.B]);
                Vect uv3(UVList[editFace.uvFace.C]);

                Vect temp1(uv1-uv3), temp2(uv2-uv3);
                Vect cross = (temp1^temp2).Norm();

                BOOL bTVMirrored = (cross.z <= 0.0f);

                for(k=0; k<3; k++)
                {
                    VertDupeList &dupes = Dupes[editFace.pointFace.ptr[k]];
                    for(l=0; l<dupes.Num(); l++)
                    {
                        VertDupe &dupe = dupes[l];
                        if( (dupe.uvVert == editFace.uvFace.ptr[k]) &&
                            (dupe.smooth == editFace.smoothFlags)   &&
                            (dupe.polyID == editFace.polyID)        &&
                            (dupe.bTVMirrored == bTVMirrored)       )
                        {
                            newFace.ptr[k] = dupe.vert;
                        }
                    }

                    if(newFace.ptr[k] == INVALID)
                    {
                        VertDupe dupe;
                        newFace.ptr[k] = dupe.vert = mesh.VertList.Num();
                        dupe.smooth = editFace.smoothFlags;
                        dupe.uvVert = editFace.uvFace.ptr[k];
                        dupe.polyID = editFace.polyID;
                        dupe.bTVMirrored = bTVMirrored;

                        dupes << dupe;

                        Vect newVert = PointList[editFace.pointFace.ptr[k]];
                        Vect newNorm = NormalList[editFace.normalFace.ptr[k]];

                        if(bTransform)
                        {
                            newVert.TransformPoint(mat);
                            newNorm.TransformVector(mat);
                        }

                        mesh.VertList   << newVert;
                        mesh.NormalList << newNorm;
                        mesh.UVList     << UVList[editFace.uvFace.ptr[k]];
                    }
                }

                DWORD index = mesh.FaceList.Num();

                mesh.FaceList << newFace;
                mesh.FacePolyList << editFace.polyID;
                mesh.FaceSmoothList << editFace.smoothFlags;

                Plane newPlane(mesh.VertList[newFace.A],
                               mesh.VertList[newFace.B],
                               mesh.VertList[newFace.C]);

                for(k=0; k<mesh.PlaneList.Num(); k++)
                {
                    if(mesh.PlaneList[k].CloseTo(newPlane, 1e-3))
                        break;
                }

                DWORD planeNum = ((k != mesh.PlaneList.Num()) ? k : mesh.PlaneList.Add(newPlane));

                mesh.FacePlaneList << planeNum;
            }
        }
    }

    for(i=0; i<Dupes.Num(); i++)
        Dupes[i].Clear();
    Dupes.Clear();

    traceOut;
}

void EditorBrush::SubtractBrush(BOOL bRebuilding)
{
    traceIn(EditorBrush::SubtractBrush);

    int i;

    if(!bCanSubtract)
        return;

    if(!level->IsOf(GetClass(IndoorLevel)))
    {
        CrashError(TEXT("Huh?  Are you saying that this isn't an indoor level?  Weird.  WAIT NOT WEIRD HOW DID YOU EXECUTE THIS FUNCTION!?!"));
        return;
    }

    //------------------------------------------------------------
    //Save Undo Data

    Action action;
    action.strName    = TEXT("Hollify brush");
    action.actionProc = EditorBrush::UndoRedoSubtract;
    BufferOutputSerializer sUndo(action.data);

    if(!bRebuilding)
    {
        sUndo << GetName();
        Serialize(sUndo);

        i = -1;
        sUndo << i;
    }

    //------------------------------------------------------------

    IndoorLevel *indoorLevel = (IndoorLevel*)level;

    if(!NormalList.Num())
    {
        if(!FaceNormals.Num())
            RebuildFaceNormals();
        RebuildNormals();
    }
    MergeAllDuplicateVertices();

    if(!bRebuilding)
        InvertBrush();

    //------------------------------------------------------------

    BuildEditorMesh(true);
    mesh.MakeBounds();

    mesh.MakeEdges();

    brushID = indoorLevel->PVSList.Num();
    PVS &newPVS = *indoorLevel->PVSList.CreateNew();

    mesh.InitPolyOperationData();

    for(i=0; i<levelInfo->BrushList.Num(); i++)
    {
        EditorBrush *brush = levelInfo->BrushList[i];

        if(brush == this)
            continue;

        if(brush->brushType != BrushType_Subtraction)
            continue;

        if(brush->brushID == INVALID)
            continue;

        PVS &curPVS = indoorLevel->PVSList[brush->brushID];

        brush->SelectedPolys.Clear();
        brush->RebuildSelectionIndices();

        if(mesh.bounds.Intersects(brush->mesh.bounds))
        {
            if(!bRebuilding)
            {
                sUndo << brush->GetName();
                brush->Serialize(sUndo);
            }

            EditorMesh portalMesh;
            brush->mesh.LevelSubtract(mesh, portalMesh);

            if(!bRebuilding)
                brush->UpdateLevelBrush();

            int portalID = -1;

            if(portalMesh.VertList.Num())
            {
                Portal portal;

                portal.nVerts = portalMesh.VertList.Num();
                portal.nFaces = portalMesh.FaceList.Num();

                VBData *vbd = new VBData;
                vbd->VertList.CopyList(portalMesh.VertList);
                portal.VertBuffer = CreateVertexBuffer(vbd);

                portal.bounds = portalMesh.bounds;

                portal.FaceList = (Face*)Allocate(sizeof(Face)*portal.nFaces);
                mcpy(portal.FaceList, portalMesh.FaceList.Array(), sizeof(Face)*portal.nFaces);
                portal.IdxBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, portal.FaceList, portal.nFaces*3);

                portal.PVSRefs[1] = brush->brushID;
                portal.PVSRefs[0] = brushID;

                portalID = indoorLevel->PortalList.Add(portal);
                curPVS.PortalRefs << portalID;
                newPVS.PortalRefs << portalID;
            }

            if(!bRebuilding)
                sUndo << portalID;
        }

        curPVS.entities.Clear();
        curPVS.lights.Clear();
        curPVS.visEntities.Clear();
        curPVS.visLights.Clear();
        curPVS.visMeshEntities.Clear();
    }

    mesh.ProcessSplits();
    mesh.EndLevelSubtract();

    if(!bRebuilding)
    {
        sUndo << String(); //empty string signals the end of the brush data
        editor->undoStack->Push(action);
    }

    mesh.MakeBounds();
    newPVS.bounds = mesh.bounds;

    if(!bRebuilding)
    {
        brushType = BrushType_Subtraction;
        levelInfo->WorkBrush = NULL;
        levelInfo->BrushList.Add(this);

        UpdateLevelBrush();
    }

    if(IsSelected())
        levelInfo->Deselect(this);

    levelInfo->ResetEntityLevelData();

    traceOut;
}

struct AddSegmentedDialogData
{
    int type;
    float segmentSize;
};

BOOL CALLBACK AddSegmentedGeomDialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static AddSegmentedDialogData *data = NULL;

    switch(message)
    {
        case WM_INITDIALOG:
            SendMessage(GetDlgItem(hwnd, IDC_CUTCUBES), BM_SETCHECK, BST_CHECKED, 0);
            SendMessage(GetDlgItem(hwnd, IDC_CUTTERRAIN), BM_SETCHECK, BST_UNCHECKED, 0);

            LinkUpDown(GetDlgItem(hwnd, IDC_SEGMENTSIZE), GetDlgItem(hwnd, IDC_SEGMENTSIZE_EDIT));
            InitUpDownFloatData(GetDlgItem(hwnd, IDC_SEGMENTSIZE), 10.0f, 5.0f, 100.0f);

            data = (AddSegmentedDialogData*)lParam;
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    data->segmentSize = GetUpDownFloat(GetDlgItem(hwnd, IDC_SEGMENTSIZE));

                    if(SendMessage(GetDlgItem(hwnd, IDC_CUTCUBES), BM_GETCHECK, 0, 0) == BST_CHECKED)
                        data->type = IDC_CUTCUBES;
                    else
                        data->type = IDC_CUTTERRAIN;

                case IDCANCEL:
                    EndDialog(hwnd, LOWORD(wParam));
            }
    }
    return FALSE;
}

void EditorBrush::AddSegmentedGeometry()
{
    traceIn(EditorBrush::AddSegmentedGeometry);

    AddSegmentedDialogData data;
    if(DialogBoxParam(hinstMain, MAKEINTRESOURCE(IDD_ADDSEGMENTED), hwndEditor, (DLGPROC)AddSegmentedGeomDialog, (LPARAM)&data) == IDCANCEL)
        return;

    //------------------------------------------------------------
    //Save Undo Data

    Action action;
    action.strName    = TEXT("Add segmented geometry");
    action.actionProc = EditorBrush::UndoRedoAddSegmented;
    BufferOutputSerializer sUndo(action.data);

    sUndo << GetName();
    Serialize(sUndo);

    //------------------------------------------------------------
    // Calculate mesh cutting data

    if(!NormalList.Num())
    {
        if(!FaceNormals.Num())
            RebuildFaceNormals();
        RebuildNormals();
    }
    MergeAllDuplicateVertices();

    //----------

    BuildEditorMesh(true);
    mesh.MakeBounds();
    mesh.MakeEdges();

    //----------

    Vect segmentStart = (mesh.bounds.Min+0.1f)/data.segmentSize;
    segmentStart.x = ceilf(segmentStart.x);
    segmentStart.y = ceilf(segmentStart.y);
    segmentStart.z = ceilf(segmentStart.z);

    int startX = (int)segmentStart.x;
    int startY = (int)segmentStart.y;
    int startZ = (int)segmentStart.z;

    segmentStart *= data.segmentSize;

    //----------

    Vect segmentEnd = (mesh.bounds.Max-0.1f)/data.segmentSize;
    segmentEnd.x = ceilf(segmentEnd.x);
    segmentEnd.y = ceilf(segmentEnd.y);
    segmentEnd.z = ceilf(segmentEnd.z);

    int endX = (int)segmentEnd.x;
    int endY = (int)segmentEnd.y;
    int endZ = (int)segmentEnd.z;

    segmentEnd *= data.segmentSize;

    //----------

    List<EditorMesh*> NewMeshList;
    List<EditorMesh*> TempMeshList;
    Plane cutPlane;

    StringList SegmentedObjcetNames;

    BitList InvalidPolys;
    InvalidPolys.SetSize(PolyList.Num());

    int sizeX = endX-startX;
    int sizeY = endY-startY;
    int sizeZ = endZ-startZ;

    //------------------------------------------------------------
    // X cut
    cutPlane.Dir.Set(1.0f, 0.0f, 0.0f);

    //for(int i=1; i<2; i++)
    for(int i=0; i<sizeX; i++)
    {
        cutPlane.Dist = segmentStart.x + (data.segmentSize*float(i));

        EditorMesh *newMesh = NULL;

        if(mesh.SplitByPlane(cutPlane, newMesh))
            NewMeshList << newMesh;
    }

    //------------------------------------------------------------
    // Y cut
    if(data.type == IDC_CUTCUBES)
    {
        cutPlane.Dir.Set(0.0f, 1.0f, 0.0f);
        TempMeshList.CopyList(NewMeshList);
        TempMeshList << &mesh;

        for(int i=0; i<sizeY; i++)
        {
            cutPlane.Dist = segmentStart.y + (data.segmentSize*float(i));

            for(int j=0; j<TempMeshList.Num(); j++)
            {
                EditorMesh *newMesh = NULL;
                if(TempMeshList[j]->SplitByPlane(cutPlane, newMesh))
                    NewMeshList << newMesh;
            }
        }
    }

    //------------------------------------------------------------
    // Z cut
    cutPlane.Dir.Set(0.0f, 0.0f, 1.0f);
    TempMeshList.CopyList(NewMeshList);
    TempMeshList << &mesh;

    for(int i=0; i<sizeZ; i++)
    {
        cutPlane.Dist = segmentStart.z + (data.segmentSize*float(i));

        for(int j=0; j<TempMeshList.Num(); j++)
        {
            EditorMesh *newMesh = NULL;
            if(TempMeshList[j]->SplitByPlane(cutPlane, newMesh))
                NewMeshList << newMesh;
        }
    }

    //------------------------------------------------------------
    // Create new brushes
    levelInfo->WorkBrush = NULL;

    for(int i=0; i<NewMeshList.Num(); i++)
    {
        EditorMesh *newMesh = NewMeshList[i];

        if(!newMesh->VertList.Num())
        {
            delete newMesh;
            continue;
        }

        InvalidPolys.SetAll();
        for(int j=0; j<newMesh->PolyList.Num(); j++)
        {
            if(newMesh->PolyList[j].Faces.Num())
                InvalidPolys.Clear(j);
        }

        newMesh->RemoveEmptyPolys();

        EditorBrush *brush = CreateObject(EditorBrush);
        brush->mesh.CopyEditorMesh(*newMesh);
        delete newMesh;

        for(int i=0; i<InvalidPolys.Num(); i++)
        {
            if(!InvalidPolys[i])
            {
                RM->AddMaterialRef(Materials[i]);
                brush->Materials << Materials[i];
            }
        }

        brush->GenerateUniqueName(TEXT("Segment"));
        SegmentedObjcetNames << brush->GetName();

        brush->CopyFromMesh();

        brush->mesh.MakeEdges();
        brush->mesh.MakeBounds();

        brush->RebuildBounds();

        Brush *addBrush;

        OctLevel *octLevel = (OctLevel*)level;
        brush->brushID = octLevel->BrushList.Num();
        OctBrush *octBrush = *octLevel->BrushList.CreateNew() = new OctBrush;

        octBrush->bounds = brush->mesh.bounds;
        octBrush->leaf = new LevelObject(octBrush);

        octBrush->node = octLevel->objectTree->Add(octBrush->leaf);
        addBrush = octBrush;

        addBrush->bounds = brush->mesh.bounds;

        brush->brushType = BrushType_Addition;
        levelInfo->BrushList.Add(brush);

        brush->UpdateLevelBrush();
    }

	if(mesh.VertList.Num())
	{
		InvalidPolys.SetAll();
		for(int i=0; i<mesh.PolyList.Num(); i++)
		{
			if(mesh.PolyList[i].Faces.Num())
				InvalidPolys.Clear(i);
		}

		mesh.RemoveEmptyPolys();

		for(int i=InvalidPolys.Num()-1; i>=0; i--)
		{
			if(InvalidPolys[i])
			{
				RM->ReleaseMaterial(Materials[i]);
				Materials.Remove(i);
			}
		}

		CopyFromMesh();

		mesh.MakeEdges();
		mesh.MakeBounds();
		RebuildBounds();

		Brush *addBrush;

		OctLevel *octLevel = (OctLevel*)level;
		brushID = octLevel->BrushList.Num();
		OctBrush *octBrush = *octLevel->BrushList.CreateNew() = new OctBrush;

		octBrush->bounds = mesh.bounds;
		octBrush->leaf = new LevelObject(octBrush);

		octBrush->node = octLevel->objectTree->Add(octBrush->leaf);
		addBrush = octBrush;

		addBrush->bounds = mesh.bounds;

		brushType = BrushType_Addition;
		levelInfo->BrushList.Add(this);

		UpdateLevelBrush();

		SegmentedObjcetNames << GetName();
	}
	else
		this->SafeDestroy();

	//------------------------------------------------------------

	sUndo << SegmentedObjcetNames;

    editor->undoStack->Push(action);

    traceOut;
}

void EditorBrush::AddGeometry(BOOL bRebuilding)
{
    traceIn(EditorBrush::AddGeometry);

    //------------------------------------------------------------
    //Save Undo Data

    Action action;
    action.strName    = TEXT("Add geometry brush");
    action.actionProc = EditorBrush::UndoRedoAddGeometry;
    BufferOutputSerializer sUndo(action.data);

    if(!bRebuilding)
    {
        sUndo << GetName();
        Serialize(sUndo);

        editor->undoStack->Push(action);
    }

    //------------------------------------------------------------

    if(!NormalList.Num())
    {
        if(!FaceNormals.Num())
            RebuildFaceNormals();
        RebuildNormals();
    }
    MergeAllDuplicateVertices();

    //------------------------------------------------------------

    BuildEditorMesh(true);

    mesh.MakeEdges();
    mesh.MakeBounds();

    Brush *addBrush;

    if(level->IsOf(GetClass(IndoorLevel)))
    {
        IndoorLevel *indoorLevel = (IndoorLevel*)level;
        brushID = indoorLevel->BrushList.Num();
        addBrush = indoorLevel->BrushList.CreateNew();

        for(int i=0; i<indoorLevel->PVSList.Num(); i++)
        {
            PVS *pvs = &indoorLevel->PVSList[i];

            if(pvs->bounds.Intersects(mesh.bounds))
                pvs->BrushRefs << brushID;
        }
    }
    else if(level->IsOf(GetClass(OutdoorLevel)))
    {
        OutdoorLevel *outdoorLevel = (OutdoorLevel*)level;
        brushID = outdoorLevel->BrushList.Num();
        addBrush = outdoorLevel->BrushList.CreateNew();

        for(int i=0; i<outdoorLevel->TerrainBlocks.Num(); i++)
        {
            TerrainBlock *block = &outdoorLevel->TerrainBlocks[i];

            if(block->bounds.Intersects(mesh.bounds))
                block->BrushRefs << brushID;
        }
    }
    else if(level->IsOf(GetClass(OctLevel)))
    {
        OctLevel *octLevel = (OctLevel*)level;
        brushID = octLevel->BrushList.Num();
        OctBrush *octBrush = *octLevel->BrushList.CreateNew() = new OctBrush;

        octBrush->bounds = mesh.bounds;
        octBrush->leaf = new LevelObject(octBrush);

        octBrush->node = octLevel->objectTree->Add(octBrush->leaf);
        addBrush = octBrush;
    }

    addBrush->bounds = mesh.bounds;

    if(!bRebuilding)
    {
        brushType = BrushType_Addition;
        levelInfo->WorkBrush = NULL;
        levelInfo->BrushList.Add(this);

        UpdateLevelBrush();
    }

    if(IsSelected())
        levelInfo->Deselect(this);

    //levelInfo->ResetEntityLevelData();

    traceOut;
}

void EditorBrush::SelectAllFaces()
{
    traceIn(EditorBrush::SelectAllFaces);

    SelectedPolys.SetSize(mesh.PolyList.Num());

    for(int i=0; i<SelectedPolys.Num(); i++)
        SelectedPolys[i] = i;

    traceOut;
}

void EditorBrush::SubtractGeometry()
{
    traceIn(EditorBrush::SubtractGeometry);

    if(!bCanSubtract)
        return;

    if(brushType != BrushType_WorkBrush)
        return;

    int i;

    //------------------------------------------------------------
    //Save Undo Data

    Action action;
    action.strName    = TEXT("Subtract geometry");
    action.actionProc = EditorBrush::UndoRedoSubtractGeometry;
    BufferOutputSerializer sUndo(action.data);

    sUndo << GetName();
    Serialize(sUndo);

    //------------------------------------------------------------

    InvertBrush();

    BuildEditorMesh(true);
    mesh.MakeBounds();
    mesh.MakeEdges();

    for(i=0; i<levelInfo->BrushList.Num(); i++)
    {
        EditorBrush *brush = levelInfo->BrushList[i];

        if(brush->brushType == BrushType_Addition)
        {
            sUndo << brush->GetName();
            brush->Serialize(sUndo);

            if(brush->mesh.GeometrySubtract(mesh))
            {
                brush->Clear();

                brush->CopyFromMesh();
                brush->MergeAllDuplicateVertices();
                brush->ProcessBasicMeshData();

                brush->UpdateLevelBrush();

                brush->RebuildBounds();

                AddBrush *addBrush = (AddBrush*)brush->GetLevelBrush();
            }

            break;
        }
    }

    editor->undoStack->Push(action);

    SafeDestroy();

    //levelInfo->ResetEntityLevelData();
    UpdateViewports();

    traceOut;
}


Entity* EditorBrush::DuplicateEntity()
{
    traceInFast(EditorBrush::DuplicateEntity);

    if(brushType == BrushType_WorkBrush)
        return NULL;

    EditorBrush *brush = (EditorBrush*)this->GetObjectClass()->Create();
    List<BYTE> data;

    BufferOutputSerializer sSave(data);
    SerializeBareBrush(sSave);

    BufferInputSerializer sLoad(data);
    brush->SerializeBareBrush(sLoad);
    brush->GenerateUniqueName();
    brush->InitializeObject();

    brush->Materials.CopyList(Materials);
    for(int i=0; i<Materials.Num(); i++)
    {
        if(Materials[i])
            RM->AddMaterialRef(Materials[i]);
    }

    levelInfo->BrushList << brush;

    brush->brushID = -1;
    brush->ProcessBasicMeshData();

    return brush;

    traceOutFast;
}


void EditorBrush::CopyFromMesh()
{
    traceInFast(EditorBrush::CopyFromMesh);

    int i;

    if(!mesh.FaceList.Num())
    {
        SafeDestroy();
        return;
    }

    FaceList.SetSize(mesh.FaceList.Num());

    for(i=0; i<mesh.FaceList.Num(); i++)
    {
        EditFace &mainFace = FaceList[i];
        Face &face = mesh.FaceList[i];

        mcpy(mainFace.pointFace.ptr, face.ptr, sizeof(Face));
        mcpy(mainFace.uvFace.ptr, face.ptr, sizeof(Face));

        mainFace.polyID = mesh.FacePolyList[i];
        mainFace.smoothFlags = mesh.FaceSmoothList[i];
    }

    UVList.CopyList(mesh.UVList);
    PointList.CopyList(mesh.VertList);

    Matrix mat;
    mat.SetIdentity();
    mat.Translate(GetLocalPos());
    mat.Rotate(GetLocalRot());
    mat.Transpose();

    for(i=0; i<PointList.Num(); i++)
        PointList[i].TransformPoint(mat);

    ProcessBasicMeshData();

    RebuildFaceNormals();
    RebuildNormals();

    traceOutFast;
}


void EditorBrush::InvertBrush()
{
    traceInFast(EditorBrush::InvertBrush);

    int i;

    for(i=0; i<FaceList.Num(); i++)
    {
        EditFace &face = FaceList[i];

        DWORD B = face.pointFace.B;
        face.pointFace.B = face.pointFace.C;
        face.pointFace.C = B;

        B = face.uvFace.B;
        face.uvFace.B = face.uvFace.C;
        face.uvFace.C = B;
    }

    for(i=0; i<UVList.Num(); i++)
        UVList[i].x = -UVList[i].x;

    RebuildFaceNormals();
    RebuildNormals();

    traceOutFast;
}


Brush* EditorBrush::GetLevelBrush()
{
    traceInFast(EditorBrush::GetLevelBrush);

    if(brushType == BrushType_WorkBrush || brushID == -1)
        return NULL;

    Brush *levelBrush;
    
    if(level->IsOf(GetClass(IndoorLevel)))
    {
        IndoorLevel *indoorLevel = (IndoorLevel*)level;

        if(brushType == BrushType_Subtraction)
            levelBrush = &indoorLevel->PVSList[brushID];
        else if(brushType == BrushType_Addition)
            levelBrush = &indoorLevel->BrushList[brushID];
    }
    else if(level->IsOf(GetClass(OutdoorLevel)))
    {
        OutdoorLevel *outdoorLevel = (OutdoorLevel*)level;

        if(brushType != BrushType_Addition)
            CrashError(TEXT("...uhm, wha?  (...Outdoor level, EditorBrush::GetBrush, non-addition..?)"));

        levelBrush = &outdoorLevel->BrushList[brushID];
    }
    else if(level->IsOf(GetClass(OctLevel)))
    {
        OctLevel *octLevel = (OctLevel*)level;

        if(brushType != BrushType_Addition)
            CrashError(TEXT("...uhm, wha?  (...Oct level, EditorBrush::GetBrush, non-addition..?)"));

        levelBrush = octLevel->BrushList[brushID];
    }

    return levelBrush;

    traceOutFast;
}

void EditorBrush::ReorderMaterialPolys()
{
    traceInFast(EditorBrush::ReorderMaterialPolys);

    int i, j, k;
    List<Material*> UniqueMaterials;
    List<UINT>      UniqueMaterialPolyCount;

    for(i=0; i<Materials.Num(); i++)
    {
        UINT id = UniqueMaterials.FindValueIndex(Materials[i]);
        if(id == INVALID)
        {
            id = UniqueMaterials.Add(Materials[i]);
            UniqueMaterialPolyCount.Add(1);
        }
        else
            ++UniqueMaterialPolyCount[id];
    }

    UINT curPoly = 0;

    for(i=0; i<UniqueMaterials.Num(); i++)
    {
        Material *mat = UniqueMaterials[i];
        UINT numPolys = UniqueMaterialPolyCount[i];
        UINT startPoly = curPoly;

        for(j=curPoly; j<Materials.Num(); j++)
        {
            if((curPoly-startPoly) == numPolys)
                break;

            if(Materials[j] == mat)
            {
                if(j == curPoly)
                {
                    ++curPoly;
                    continue;
                }

                List<DWORD> &FacesA = PolyList[j].Faces;
                List<DWORD> &FacesB = PolyList[curPoly].Faces;
                for(k=0; k<FacesA.Num(); k++)
                {
                    EditFace &face = FaceList[FacesA[k]];
                    face.polyID = curPoly;
                }

                for(k=0; k<FacesB.Num(); k++)
                {
                    EditFace &face = FaceList[FacesB[k]];
                    face.polyID = j;
                }

                Materials.SwapValues(j, curPoly);
                PolyList.SwapValues(j, curPoly);

                for(k=0; k<mesh.FacePolyList.Num(); k++)
                {
                    DWORD &poly = mesh.FacePolyList[k];
                    if(poly == j) poly = curPoly;
                    else if(poly == curPoly) poly = j;
                }

                ++curPoly;
            }
        }
    }

    mesh.RebuildPolyList();
    mesh.RebuildPolyBounds();

    traceOutFast;
}

void EditorBrush::UpdateLevelBrush()
{
    traceInFast(EditorBrush::UpdateLevelBrush);

    if((brushID == -1) || (brushType == BrushType_WorkBrush))
    {
        AppWarning(TEXT("ACK!  No level brush assigned to editor brush, cannot rebuild"));
        return;
    }

    ReorderMaterialPolys();
    mesh.MakeEdges();

    Brush *levelBrush = GetLevelBrush();

    DWORD i;

    /*if(brushType == BrushType_Subtraction)
    {
        PVS *pvs = (PVS*)levelBrush;
        pvs->Clear();
    }
    else if(brushType == BrushType_Addition)
    {
        AddBrush *addBrush = (AddBrush*)levelBrush;
        addBrush->Clear();
    }*/

    levelBrush->Clear();

    // get materials and sections
    if(mesh.VertList.Num())
    {
        List<Material*> UniqueMaterials;
        List<DrawSection> NewSections;

        DWORD curFace = 0;
        DrawSection *curSection = NULL;
        for(i=0; i<Materials.Num(); i++)
        {
            UINT id = UniqueMaterials.FindValueIndex(Materials[i]);
            if(id == INVALID)
            {
                id = UniqueMaterials.Add(Materials[i]);

                if(Materials[i])
                    RM->AddMaterialRef(Materials[i]);

                curSection = NewSections.CreateNew();
                curSection->startFace = mesh.PolyList[i].Faces[0];
            }

            curSection->numFaces += mesh.PolyList[i].Faces.Num();
        }

        NewSections.TransferTo(levelBrush->SectionList, (UINT&)levelBrush->nSections);
        UniqueMaterials.TransferTo(levelBrush->Materials);

        /*levelBrush->Materials.SetSize(mesh.PolyList.Num());
        levelBrush->SectionList = (DrawSection*)Allocate(sizeof(DrawSection)*mesh.PolyList.Num());

        DWORD curFace = 0;

        for(i=0; i<mesh.PolyList.Num(); i++)
        {
            PolyFace &poly = mesh.PolyList[i];

            levelBrush->Materials[i] = Materials[i];

            if(Materials[i])
                RM->AddMaterialRef(Materials[i]);

            DrawSection &curSection = levelBrush->SectionList[i];
            curSection.startFace = curFace;

            if(poly.Faces.Num())
            {
                Plane &plane = mesh.PlaneList[mesh.FacePlaneList[poly.Faces[0]]];

                /*if(level->IsOf(GetClass(IndoorLevel)) && (brushType == BrushType_Subtraction))
                {
                    IndoorLevel *indoorLevel = (IndoorLevel*)level;
                    DWORD planeID = indoorLevel->PlaneList.SafeAdd(plane);
                }
                else if(brushType == BrushType_Addition)
                {
                    AddBrush *addBrush = (AddBrush*)levelBrush;

                    DWORD planeID = addBrush->PlaneList.SafeAdd(plane);

                    for(j=0; j<poly.Faces.Num(); j++)
                        levelBrush->FacePlaneList[curFace+j] = planeID;
                }*//*
            }

            curFace += (curSection.numFaces = poly.Faces.Num());
        }*/

        // get edges
        mesh.MakeEdges();
        if(mesh.EdgeList.Num())
        {
            levelBrush->EdgeList = (Edge*)Allocate(sizeof(Edge)*mesh.EdgeList.Num());
            mcpy(levelBrush->EdgeList, mesh.EdgeList.Array(), sizeof(Edge)*mesh.EdgeList.Num());
        }

        // get verts/uvs/norms
        VBData *vbd = new VBData;

        vbd->TVList.SetSize(1);
        vbd->TVList[0].SetWidth(2);
        vbd->TVList[0].SetSize(mesh.VertList.Num());
        vbd->TangentList.SetSize(mesh.VertList.Num());

        vbd->VertList.CopyList(mesh.VertList);
        vbd->NormalList.CopyList(mesh.NormalList);
        vbd->TVList[0].GetV2()->CopyList(mesh.UVList);

        levelBrush->VertList    = vbd->VertList.Array();
        levelBrush->UVList      = (UVCoord*)vbd->TVList[0].Array();
        levelBrush->TangentList = vbd->TangentList.Array();

        // get faces
        levelBrush->FaceList = (Face*)Allocate(sizeof(Face)*mesh.FaceList.Num());
        mcpy(levelBrush->FaceList, mesh.FaceList.Array(), sizeof(Face)*mesh.FaceList.Num());
        levelBrush->IdxBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, levelBrush->FaceList, mesh.FaceList.Num()*3);

        // get tangent coordinates
        List<Vect> FaceTangents;
        FaceTangents.SetSize(mesh.FaceList.Num());

        for(i=0; i<mesh.FaceList.Num(); i++)
        {
            /*Face &f = mesh.FaceList[i];

            Vect vec1 = mesh.VertList[f.B] - mesh.VertList[f.A];
            Vect vec2 = mesh.VertList[f.C] - mesh.VertList[f.A];

            double deltaU1 = (mesh.UVList[f.B].x - mesh.UVList[f.A].x);
            double deltaU2 = (mesh.UVList[f.C].x - mesh.UVList[f.A].x);
            double deltaV1 = (mesh.UVList[f.B].y - mesh.UVList[f.A].y);
            double deltaV2 = (mesh.UVList[f.C].y - mesh.UVList[f.A].y);

            //---------------------

            double a = (deltaU1 - deltaV1*deltaU2/deltaV2);
            if(a != 0.0) a = 1.0/a;
            double b = (deltaU2 - deltaV2*deltaU1/deltaV1);
            if(b != 0.0) b = 1.0/b;

            Vect duTemp = ((vec1*a) + (vec2*b));
            double tempf = 1.0 / sqrt(duTemp|duTemp);
            duTemp *= tempf;

            Vect norm1 = mesh.PlaneList[mesh.FacePlaneList[i]].Dir;
            tempf = duTemp|norm1;
            FaceTangents[i] = (duTemp - (norm1*tempf)).Norm();*/

            FaceTangents[i] = mesh.GetFaceTangent(i);
        }

        for(i=0; i<mesh.VertList.Num(); i++)
        {
            Vect UNormal(0, 0, 0);

            for(DWORD j=0; j<mesh.FaceList.Num(); j++)
            {
                Face &curFace = mesh.FaceList[j];
                for(int k=0; k<3; k++)
                {
                    if(curFace.ptr[k] == i)
                    {
                        UNormal += FaceTangents[j];
                        break;
                    }
                }
            }

            UNormal.Norm();

            //Readjust to be orthogonal with vertex normal
            Vect vertNorm = mesh.NormalList[i];

            double tempf = UNormal|vertNorm;
            UNormal = -(UNormal - (vertNorm*tempf)).Norm();

            vbd->TangentList[i] = UNormal;
        }

        FaceTangents.Clear();

        // create buffer
        levelBrush->VertBuffer = CreateVertexBuffer(vbd);
    }

    levelBrush->nVerts      = mesh.VertList.Num();
    levelBrush->nFaces      = mesh.FaceList.Num();
    levelBrush->nEdges      = mesh.EdgeList.Num();

    // wireframe
    if(mesh.EdgeList.Num())
    {
        DWORD *wireframeIndices = (DWORD*)Allocate(mesh.EdgeList.Num()*2*sizeof(DWORD));

        for(i=0; i<mesh.EdgeList.Num(); i++)
        {
            DWORD curPos = i*2;
            wireframeIndices[curPos]   = levelBrush->EdgeList[i].v1;
            wireframeIndices[curPos+1] = levelBrush->EdgeList[i].v2;
        }

        levelBrush->WireframeBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, wireframeIndices, mesh.EdgeList.Num()*2);
    }
    levelBrush->bounds = mesh.bounds;

    levelBrush->meshShape = physics->MakeStaticMeshShape(levelBrush->VertList, levelBrush->nVerts, (UINT*)levelBrush->FaceList, levelBrush->nFaces*3);
    levelBrush->phyObj    = physics->CreateStaticObject(levelBrush->meshShape, Vect(0.0f, 0.0f, 0.0f), Quat::Identity());
    levelBrush->phyObj->EnableCollisionCallback(TRUE);
    levelBrush->phyObj->SetBrushOwner(levelBrush);

    UpdateViewports();

    traceOutFast;
}

void EditorBrush::UpdateTangents()
{
    traceInFast(EditorBrush::UpdateTangents);

    assert(brushID != -1);
    if(brushID == -1)
        return;

    Brush *levelBrush = GetLevelBrush();
    DWORD i;

    List<Vect> FaceTangents;
    FaceTangents.SetSize(mesh.FaceList.Num());

    for(i=0; i<mesh.FaceList.Num(); i++)
    {
        Face &f = mesh.FaceList[i];

        Vect vec1 = mesh.VertList[f.B] - mesh.VertList[f.A];
        Vect vec2 = mesh.VertList[f.C] - mesh.VertList[f.A];

        double deltaU1 = (mesh.UVList[f.B].x - mesh.UVList[f.A].x);
        double deltaU2 = (mesh.UVList[f.C].x - mesh.UVList[f.A].x);
        double deltaV1 = (mesh.UVList[f.B].y - mesh.UVList[f.A].y);
        double deltaV2 = (mesh.UVList[f.C].y - mesh.UVList[f.A].y);

        //---------------------

        double a = (deltaU1 - deltaV1*deltaU2/deltaV2);
        if(a != 0.0) a = 1.0/a;
        double b = (deltaU2 - deltaV2*deltaU1/deltaV1);
        if(b != 0.0) b = 1.0/b;

        Vect duTemp = ((vec1*a) + (vec2*b));
        double tempf = 1.0 / sqrt(duTemp|duTemp);
        duTemp *= tempf;

        Vect norm1 = mesh.PlaneList[mesh.FacePlaneList[i]].Dir;
        tempf = duTemp|norm1;
        FaceTangents[i] = (duTemp - (norm1*tempf)).Norm();
    }

    for(i=0; i<mesh.VertList.Num(); i++)
    {
        Vect UNormal(0, 0, 0);

        for(DWORD j=0; j<mesh.FaceList.Num(); j++)
        {
            Face &curFace = mesh.FaceList[j];
            for(int k=0; k<3; k++)
            {
                if(curFace.ptr[k] == i)
                {
                    UNormal += FaceTangents[j];
                    break;
                }
            }
        }

        UNormal.Norm();

        //Readjust to be orthogonal with vertex normal
        Vect vertNorm = mesh.NormalList[i];

        double tempf = UNormal|vertNorm;
        UNormal = -(UNormal - (vertNorm*tempf)).Norm();

        levelBrush->TangentList[i] = UNormal;
    }

    FaceTangents.Clear();

    traceOutFast;
}

void EditorBrush::UpdateNormals(BOOL bUpdateLevelBrush)
{
    traceInFast(EditorBrush::UpdateNormals);

    RebuildNormals();

    //IndoorLevel *indoorLevel = (IndoorLevel*)level;

    DWORD i, j, k, l;

    mesh.NormalList.SetSize(mesh.VertList.Num());

    zero(mesh.NormalList.Array(), sizeof(Vect)*mesh.NormalList.Num());

    //-------------------------------------

    /*List<IDList> Dupes;
    List<DWORD>  VertSmooth;
    Dupes.SetSize(mesh.VertList.Num());
    VertSmooth.SetSize(mesh.VertList.Num());

    for(i=0; i<mesh.VertList.Num(); i++)
    {
        if(Dupes[i].Num())
            continue;

        Dupes[i] << i;

        Vect &v1 = mesh.VertList[i];

        for(j=i+1; j<mesh.VertList.Num(); j++)
        {
            if(Dupes[j].Num())
                continue;

            Vect &v2 = mesh.VertList[j];

            if(v1.CloseTo(v2))
                Dupes[i] << j;
        }

        for(j=1; j<Dupes[i].Num(); j++)
            Dupes[Dupes[i][j]].CopyList(Dupes[i]);
    }

    //-------------------------------------

    List<IDList> SmoothPolys;
    SmoothPolys.SetSize(32);

    for(i=0; i<PolyList.Num(); i++)
    {
        List<DWORD> &BrushFaces = PolyList[i].Faces;
        List<DWORD> &MeshFaces  = mesh.PolyList[i].Faces;

        EditFace &brushFace = FaceList[BrushFaces[0]];

        for(j=0; j<MeshFaces.Num(); j++)
        {
            Face &meshFace = mesh.FaceList[MeshFaces[j]];
            mesh.FaceSmoothList[MeshFaces[j]] = brushFace.smoothFlags;

            if(!brushFace.smoothFlags)
            {
                Vect &faceNorm = FaceNormals[BrushFaces[0]];

                mesh.NormalList[meshFace.A] =
                mesh.NormalList[meshFace.B] =
                mesh.NormalList[meshFace.C] = faceNorm;
            }

            for(k=0; k<3; k++)
            {
                DWORD vertID = meshFace.ptr[k];
                for(int dupeID=0; dupeID<Dupes[vertID].Num(); dupeID++)
                {
                    DWORD dupe = Dupes[vertID][dupeID];

                    if(dupe == vertID)
                        VertSmooth[dupe] |= brushFace.smoothFlags;
                }
            }
        }

        for(j=0; j<32; j++)
        {
            if(brushFace.smoothFlags & (1<<j))
                SmoothPolys[j] << i;
        }
    }

    //-------------------------------------

    for(i=0; i<32; i++)
    {
        DWORD smoothFlag = 1<<i;

        for(j=0; j<SmoothPolys[i].Num(); j++)
        {
            DWORD polyID = SmoothPolys[i][j];

            List<DWORD> &MeshFaces  = mesh.PolyList[polyID].Faces;
            List<DWORD> &BrushFaces = PolyList[polyID].Faces;
            Vect &faceNorm = FaceNormals[BrushFaces[0]];

            for(k=0; k<MeshFaces.Num(); k++)
            {
                Face &face = mesh.FaceList[MeshFaces[k]];
                for(int vert=0; vert<3; vert++)
                {
                    DWORD vertID = face.ptr[vert];

                    for(int dupeID=0; dupeID<Dupes[vertID].Num(); dupeID++)
                    {
                        DWORD dupe = Dupes[vertID][dupeID];

                        if(VertSmooth[dupe] & smoothFlag)
                            mesh.NormalList[dupe] += faceNorm;
                    }
                }
            }
        }

        SmoothPolys[i].Clear();
    }*/

    DWORD smoothVals = 0;

    Matrix invRot = -GetLocalRot();

    for(i=0; i<mesh.FaceList.Num(); i++)
    {
        DWORD polyFace = PolyList[mesh.FacePolyList[i]].Faces[0];

        EditFace &brushPoly = FaceList[polyFace];
        mesh.FaceSmoothList[i] = brushPoly.smoothFlags;

        smoothVals |= brushPoly.smoothFlags;

        if(!brushPoly.smoothFlags)
        {
            Face &face = mesh.FaceList[i];
            Vect faceNorm = FaceNormals[polyFace];

            faceNorm.TransformVector(invRot);

            for(j=0; j<3; j++)
                mesh.NormalList[face.ptr[j]] = faceNorm;
        }
    }

    for(int smoothID=0; smoothID<32; smoothID++)
    {
        DWORD smooth = 1<<smoothID;
        if(smoothVals & smooth)
        {
            BitList FoundVerts;  //found verts is just like found cake.  it's litter.
            FoundVerts.SetSize(mesh.VertList.Num());

            for(i=0; i<mesh.VertList.Num(); i++)
            {
                if(FoundVerts[i])
                    continue;

                Vect &v1 = mesh.VertList[i];

                List<DWORD> Verts;
                Verts << i;

                FoundVerts.Set(i);

                for(j=i+1; j<mesh.VertList.Num(); j++)
                {
                    if(FoundVerts[j])
                        continue;

                    Vect &v2 = mesh.VertList[j];

                    if(v1.CloseTo(v2))
                    {
                        Verts << j;
                        FoundVerts.Set(j);
                    }
                }

                List<DWORD> NormVerts;
                Vect newNorm(0.0f, 0.0f, 0.0f);

                for(j=0; j<Verts.Num(); j++)
                {
                    DWORD curVert = Verts[j];

                    for(k=0; k<mesh.FaceList.Num(); k++)
                    {
                        if(mesh.FaceSmoothList[k] & smooth)
                        {
                            DWORD polyID = mesh.FacePolyList[k];
                            Vect faceNorm = FaceNormals[PolyList[polyID].Faces[0]];

                            faceNorm.TransformVector(invRot);

                            Face &face = mesh.FaceList[k];

                            for(l=0; l<3; l++)
                            {
                                if(face.ptr[l] == curVert)
                                {
                                    newNorm += faceNorm;
                                    NormVerts.SafeAdd(face.ptr[l]);
                                    break;
                                }
                            }
                        }
                    }
                }

                newNorm.Norm();

                for(j=0; j<NormVerts.Num(); j++)
                    mesh.NormalList[NormVerts[j]] = newNorm;
            }
        }
    }

    //-------------------------------------

    //for(i=0; i<mesh.NormalList.Num(); i++)
    //    mesh.NormalList[i].Norm();

    if(bUpdateLevelBrush && (brushID != -1))
    {
        Brush *levelBrush = GetLevelBrush();
        VBData *vbd = levelBrush->VertBuffer->GetData();
        vbd->NormalList.CopyList(mesh.NormalList);
    }

    //-------------------------------------

    /*SmoothPolys.Clear();

    for(i=0; i<Dupes.Num(); i++)
        Dupes[i].Clear();
    Dupes.Clear();*/

    traceOutFast;
}



BOOL EditorBrush::CanSelect(const Vect &rayOrig, const Vect &rayDir)
{
    traceInFast(EditorBrush::CanSelect);

    DWORD i;

    Matrix transform;
    transform.SetIdentity();
    transform *= -GetWorldRot();
    transform *= -GetWorldPos();

    Vect newRayOrig = rayOrig;
    Vect newRayDir  = rayDir;

    newRayOrig.TransformPoint(transform);
    newRayDir.TransformVector(transform);

    for(i=0; i<VisibleEdgeList.Num(); i++)
    {
        SimpleEdge &edge = VisibleEdgeList[i];

        Vect *e[2] = {&PointList[edge.v1], &PointList[edge.v2]};

        Vect edgeVec  = *e[1]-*e[0];
        float edgeLen = edgeVec.Len();

        Vect edgeRayDir  = edgeVec*(1.0f/edgeLen);

        float fT1, fT2;
        if(ClosestLinePoints(newRayOrig, newRayDir, *e[0], edgeRayDir, fT1, fT2))
        {
            if((fT2 < 0.0f) || (fT2 > edgeLen))
                continue;

            Vect closestPoint1 = newRayOrig+(newRayDir*fT1);
            Vect closestPoint2 = *e[0]+(edgeRayDir*fT2);

            if(closestPoint1.Dist(closestPoint2) < 0.4f)
                return TRUE;
        }
    }

    return FALSE;

    traceOutFast;
}


void EditorBrush::RebuildSelectionIndices()
{
    traceInFast(EditorBrush::RebuildSelectionIndices);

    delete SelectionIdxBuffer;
    SelectionIdxBuffer = NULL;

    List<Face> IndexFaceList;

    for(DWORD i=0; i<SelectedPolys.Num(); i++)
    {
        PolyFace &poly = mesh.PolyList[SelectedPolys[i]];

        for(DWORD j=0; j<poly.Faces.Num(); j++)
            IndexFaceList << mesh.FaceList[poly.Faces[j]];
    }

    if(IndexFaceList.Num())
    {
        Face *indices = (Face*)Allocate(IndexFaceList.Num()*sizeof(Face));
        mcpy(indices, IndexFaceList.Array(), IndexFaceList.Num()*sizeof(Face));

        SelectionIdxBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, indices, IndexFaceList.Num()*3);
    }

    traceOutFast;
}

void EditorBrush::RebuildBounds()
{
    bounds.Min = Vect( M_INFINITE);
    bounds.Max = Vect(-M_INFINITE);

    for(int i=0; i<PointList.Num(); i++)
    {
        bounds.Min.ClampMax(PointList[i]);
        bounds.Max.ClampMin(PointList[i]);
    }
}


void EditorBrush::SerializeUVData(Serializer &s)
{
    traceIn(EditorBrush::SerializeUVData);

    s << UVList;
    s << mesh.UVList;

    if(s.IsLoading())
    {
        if(brushID != -1)
        {
            Brush *levelBrush = GetLevelBrush();

            mcpy(levelBrush->UVList, mesh.UVList.Array(), mesh.UVList.Num()*sizeof(UVCoord));
            levelBrush->VertBuffer->FlushBuffers();
        }
    }

    traceOut;
}


void EditorBrush::Serialize(Serializer &s)
{
    traceIn(EditorBrush::Serialize);

    SerializeBrushData(s, TRUE);

    traceOut;
}

void EditorBrush::SerializeBareBrush(Serializer &s)
{
    traceIn(EditorBrush::SerializeBareBrush);

    Super::Serialize(s);

    DWORD dwObsolete = 0;

    s   << dwObsolete
        << FaceList;
    Vect::SerializeList(s, PointList);
    s   << UVList
        << brushID
        << brushType
        << SelectedPolys;

    RebuildBounds();

    traceOut;
}


void EditorBrush::SerializeBrushData(Serializer &s, BOOL bUndoRedoData)
{
    traceIn(EditorBrush::SerializeBrushData);

    Super::Serialize(s);

    int orderID = -1;

    if(brushType != BrushType_WorkBrush)
    {
        orderID = levelInfo->BrushList.FindValueIndex(this);

        if(s.IsLoading())
            levelInfo->BrushList.Remove(orderID);
    }

    BrushType prevBrushType = brushType;
    DWORD prevBrushID = brushID;

    Bounds prevMeshBounds = mesh.bounds;
    DWORD dwObsolete = 0;

    s   << orderID
        << dwObsolete
        << FaceList;
    Vect::SerializeList(s, PointList);
    s   << UVList
        << brushID
        << brushType
        << bCanSubtract
        << SelectedPolys;

    BOOL bWorkBrush = (brushType == BrushType_WorkBrush);

    if(!bWorkBrush)
        s << mesh;

    if(s.IsLoading())
    {
        if(bWorkBrush)
        {
            if(levelInfo->WorkBrush && (prevBrushID != -1))
                DestroyObject(levelInfo->WorkBrush);
            levelInfo->WorkBrush = this;
        }
        else
            levelInfo->BrushList.Insert(orderID, this);

        //---------------------------------------------------------

        if(bUndoRedoData)
        {
            if(bWorkBrush && (prevBrushID != -1)) //undoing a subtraction/addition
                RemoveLevelBrush(prevBrushID, prevBrushType);
            if(!bWorkBrush && (prevBrushID == -1)) //redoing subtraction/addition
            {
                levelInfo->WorkBrush = NULL;
                AddLevelBrush();
            }
        }
        //---------------------------------------------------------

        ProcessBasicMeshData();
        RebuildFaceNormals();

        RebuildBounds();

        if(bWorkBrush)
        {
            mesh.Clear();
            RebuildNormals();
        }
        else
        {
            RebuildSelectionIndices();
            UpdateNormals(FALSE);

            /*if(bUndoRedoData)
                UpdateLevelBrush();*/
        }

        for(int i=0; i<Materials.Num(); i++)
        {
            String materialName;
            s << materialName;

            if(!materialName.IsEmpty())
                Materials[i] = RM->GetMaterial(materialName);
        }

		if(!bWorkBrush && bUndoRedoData)
			UpdateLevelBrush();
    }
    else
    {
        String emptyString;

        for(int i=0; i<Materials.Num(); i++)
        {
            if(Materials[i])
                s << RM->GetMaterialName(Materials[i]);
            else
                s << emptyString;
        }
    }

    traceOut;
}

void EditorBrush::AddLevelBrush()
{
    if(level->IsOf(GetClass(IndoorLevel)))
    {
        IndoorLevel *indoorLevel = (IndoorLevel*)level;

        if(brushType == BrushType_Subtraction)
        {
            brushID = indoorLevel->PVSList.Num();
            indoorLevel->PVSList.SetSize(indoorLevel->PVSList.Num()+1);
        }
        else if(brushType == BrushType_Addition)
        {
            brushID = indoorLevel->BrushList.Num();
            indoorLevel->BrushList.SetSize(indoorLevel->BrushList.Num()+1);

            for(int i=0; i<indoorLevel->PVSList.Num(); i++)
            {
                PVS *pvs = &indoorLevel->PVSList[i];
                if(pvs->bounds.Intersects(mesh.bounds))
                    pvs->BrushRefs << brushID;
            }
        }
    }
    else if(level->IsOf(GetClass(OutdoorLevel)))
    {
        OutdoorLevel *outdoorLevel = (OutdoorLevel*)level;

        if(brushType == BrushType_Addition)
        {
            brushID = outdoorLevel->BrushList.Num();
            outdoorLevel->BrushList.SetSize(outdoorLevel->BrushList.Num()+1);

            for(int i=0; i<outdoorLevel->TerrainBlocks.Num(); i++)
            {
                TerrainBlock *block = &outdoorLevel->TerrainBlocks[i];
                if(block->bounds.Intersects(mesh.bounds))
                    block->BrushRefs << brushID;
            }
        }
    }   
    else if(level->IsOf(GetClass(OctLevel)))
    {
        OctLevel *octLevel = (OctLevel*)level;

        if(brushType == BrushType_Addition)
        {
            brushID = octLevel->BrushList.Num();
            octLevel->BrushList << new OctBrush;

            OctBrush *octBrush = octLevel->BrushList[brushID];
            octBrush->bounds = mesh.bounds;
            octBrush->leaf = new LevelObject(octBrush);
            octBrush->node = octLevel->objectTree->Add(octBrush->leaf);
        }
    }   
}

void EditorBrush::RemoveLevelBrush(int removeBrushID, int removeBrushType)
{
    if(removeBrushID == -1)
        removeBrushID = brushID;
    if(removeBrushType == -1)
        removeBrushType = brushType;

    if(level->IsOf(GetClass(IndoorLevel)))
    {
        IndoorLevel *indoorLevel = (IndoorLevel*)level;

        if(removeBrushType == BrushType_Subtraction)
        {
            PVS *levelBrush = (PVS*)&indoorLevel->PVSList[removeBrushID];
            levelBrush->RemoveEntities();
            levelBrush->Clear();
            indoorLevel->PVSList.Remove(removeBrushID);
        }
        else if(removeBrushType == BrushType_Addition)
        {
            Brush *levelBrush = (Brush*)&indoorLevel->BrushList[removeBrushID];
            levelBrush->Clear();
            indoorLevel->BrushList.Remove(removeBrushID);

            for(int i=0; i<indoorLevel->PVSList.Num(); i++)
            {
                PVS *pvs = &indoorLevel->PVSList[i];
                pvs->BrushRefs.RemoveItem(removeBrushID);
            }
        }
    }
    else if(level->IsOf(GetClass(OutdoorLevel)))
    {
        OutdoorLevel *outdoorLevel = (OutdoorLevel*)level;

        if(removeBrushType == BrushType_Addition)
        {
            Brush *levelBrush = (Brush*)&outdoorLevel->BrushList[removeBrushID];
            levelBrush->Clear();
            outdoorLevel->BrushList.Remove(removeBrushID);

            for(int i=0; i<outdoorLevel->TerrainBlocks.Num(); i++)
            {
                TerrainBlock *block = &outdoorLevel->TerrainBlocks[i];
                block->BrushRefs.RemoveItem(removeBrushID);
            }
        }
    }
    else if(level->IsOf(GetClass(OctLevel)))
    {
        OctLevel *octLevel = (OctLevel*)level;

        if(removeBrushType == BrushType_Addition)
        {
            OctBrush *levelBrush = (OctBrush*)octLevel->BrushList[removeBrushID];
            octLevel->BrushList.Remove(removeBrushID);
            octLevel->objectTree->Remove(levelBrush->node, levelBrush->leaf);
            delete levelBrush;
        }
    }
}



void ENGINEAPI EditorBrush::UndoRedoSubtract(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorBrush::UndoRedoSubtract);

    IndoorLevel *indoorLevel = (IndoorLevel*)level;

    int i;

    List<int> PortalRefs;

    while(s.DataPending())
    {
        String strName;
        s    << strName;
        sOut << strName;

        if(strName.IsEmpty())  //end of brushes
            break;

        EditorBrush *brush = (EditorBrush*)Entity::FindByName(strName);
        PVS *pvs;

        if(!brush)
        {
            brush = CreateObject(EditorBrush);
            brush->SetName(strName);
        }

        List<DWORD> PrevRefs;

        if(bUndo)
            pvs = (PVS*)brush->GetLevelBrush();

        brush->Serialize(sOut);
        brush->Serialize(s);

        if(!bUndo)
            pvs = (PVS*)brush->GetLevelBrush();

        int portalID;

        s    << portalID;
        sOut << portalID;

        if(portalID != -1)  //remember, first brush gets killed/recreated in brush->SerializeBrush
        {
            if(bUndo)
                pvs->PortalRefs.RemoveItem(portalID);

            PortalRefs << portalID;
        }
    }

    if(PortalRefs.Num())
    {
        if(bUndo)
        {
            for(i=0; i<PortalRefs.Num(); i++)
            {
                Portal &portal = indoorLevel->PortalList[PortalRefs[i]];
                portal.Serialize(sOut);
                portal.FreeBuffers();
            }

            indoorLevel->PortalList.SetSize(indoorLevel->PortalList.Num()-PortalRefs.Num());
        }
        else
        {
            for(i=0; i<PortalRefs.Num(); i++)
            {
                DWORD portalID = indoorLevel->PortalList.Num();
                Portal &portal = *indoorLevel->PortalList.CreateNew();
                portal.Serialize(s);

                indoorLevel->PVSList[portal.PVSRefs[0]].PortalRefs << portalID;
                indoorLevel->PVSList[portal.PVSRefs[1]].PortalRefs << portalID;
            }
        }
    }

    levelInfo->ResetEntityLevelData();

    traceOut;
}

void ENGINEAPI EditorBrush::UndoRedoAddGeometry(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorBrush::UndoRedoAddGeometry);

    String strName;

    s    << strName;
    sOut << strName;

    EditorBrush *brush = (EditorBrush*)Entity::FindByName(strName);

    brush->Serialize(sOut);
    brush->Serialize(s);

    //levelInfo->ResetEntityLevelData();

    traceOut;
}

void ENGINEAPI EditorBrush::UndoRedoAddSegmented(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorBrush::UndoRedoAddGeometry);

    String strName;
    s    << strName;
    sOut << strName;

    if(bUndo)
    {
        EditorBrush *edBrush = CreateObject(EditorBrush);
        edBrush->SetName(strName);
        edBrush->Serialize(s);

        StringList SegmentedItems;
        s << SegmentedItems;
        sOut << SegmentedItems;

        List<EditorBrush*> SortedList;
        for(int i=0; i<SegmentedItems.Num(); i++)
        {
            EditorBrush *brush = (EditorBrush*)Entity::FindByName(SegmentedItems[i]);
            int j;

            for(j=0; j<SortedList.Num(); j++)
            {
                if(SortedList[j]->brushID < brush->brushID)
                    break;
            }

            SortedList.Insert(j, brush);
            brush->Serialize(sOut);
        }

        for(int i=0; i<SortedList.Num(); i++)
        {
            EditorBrush *brush = (EditorBrush*)SortedList[i];
            brush->RemoveLevelBrush();
            brush->SafeDestroy();
        }
    }
    else
    {
        EditorBrush *edBrush = (EditorBrush*)Entity::FindByName(strName);
        edBrush->Serialize(sOut);
        edBrush->SafeDestroy();

        StringList SegmentedItems;
        s << SegmentedItems;
        sOut << SegmentedItems;

        for(int i=0; i<SegmentedItems.Num(); i++)
        {
            EditorBrush *brush = CreateObject(EditorBrush);
            brush->SetName(SegmentedItems[i]);
            brush->Serialize(s);
            //brush->AddLevelBrush();
        }
    }

    traceOut;
}

void ENGINEAPI EditorBrush::UndoRedoSubtractGeometry(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorBrush::UndoRedoSubtractGeometry);

    IndoorLevel *indoorLevel = (IndoorLevel*)level;

    BOOL bFirstBrush = TRUE;
    while(s.DataPending())
    {
        String strName;
        s    << strName;
        sOut << strName;

        if(strName.IsEmpty())  //end of brushes
            break;

        EditorBrush *edBrush = (EditorBrush*)Entity::FindByName(strName);

        if(!edBrush)
        {
            edBrush = CreateObject(EditorBrush);
            edBrush->SetName(strName);
        }

        if(bUndo)
        {
            if(bFirstBrush)
            {
                edBrush->Serialize(s);
                edBrush->Serialize(sOut);
            }
            else
            {
                edBrush->Serialize(sOut);
                edBrush->Serialize(s);
            }
        }
        else
        {
            edBrush->Serialize(sOut);
            edBrush->Serialize(s);

            if(bFirstBrush)
                edBrush->SafeDestroy();
        }

        bFirstBrush = FALSE;
    }

    //levelInfo->ResetEntityLevelData();

    traceOut;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  TextureAdjust.cpp

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





TCHAR lpPan[12];
TCHAR lpVerticalPan[12];
TCHAR lpRotateAngle[12];
TCHAR lpScaleAmount[12];
TCHAR lpHorizontalTile[12];
TCHAR lpVerticalTile[12];


BOOL WINAPI TextureAdjustProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(TextureAdjustProc);

    HWND hwndControl;

    switch(message)
    {
        case WM_INITDIALOG:
            hwndControl = GetDlgItem(hwnd, IDC_PAN);
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.0625"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.125"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.25"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.5"));
            SendMessage(hwndControl, CB_SETCURSEL, 2, 0);
            SendMessage(hwndControl, CB_LIMITTEXT, 11, 0);
            scpy(lpPan, TEXT("0.25"));

            hwndControl = GetDlgItem(hwnd, IDC_ROTATEANGLE);
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("45"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("90"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("180"));
            SendMessage(hwndControl, CB_SETCURSEL, 1, 0);
            SendMessage(hwndControl, CB_LIMITTEXT, 11, 0);
            scpy(lpRotateAngle, TEXT("90"));

            hwndControl = GetDlgItem(hwnd, IDC_SCALEAMOUNT);
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.0625"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.125"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.25"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.5"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("2.0"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("4.0"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("8.0"));
            SendMessage(hwndControl, CB_SETCURSEL, 3, 0);
            SendMessage(hwndControl, CB_LIMITTEXT, 11, 0);
            scpy(lpScaleAmount, TEXT("0.5"));

            hwndControl = GetDlgItem(hwnd, IDC_HORIZONTALTILE);
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.0625"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.125"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.25"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.5"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("1.0"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("2.0"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("4.0"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("8.0"));
            SendMessage(hwndControl, CB_SETCURSEL, 4, 0);
            SendMessage(hwndControl, CB_LIMITTEXT, 11, 0);
            scpy(lpHorizontalTile, TEXT("1.0"));

            hwndControl = GetDlgItem(hwnd, IDC_VERTICALTILE);
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.0625"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.125"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.25"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("0.5"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("1.0"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("2.0"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("4.0"));
            SendMessage(hwndControl, CB_ADDSTRING, 0, (LPARAM)TEXT("8.0"));
            SendMessage(hwndControl, CB_SETCURSEL, 4, 0);
            SendMessage(hwndControl, CB_LIMITTEXT, 11, 0);
            scpy(lpVerticalTile, TEXT("1.0"));

            hwndControl = GetDlgItem(hwnd, IDC_ALIGNFLOORWALL);
            LinkUpDown(GetDlgItem(hwnd, IDC_ALIGNFLOORWALLSCROLLER), GetDlgItem(hwnd, IDC_ALIGNFLOORWALL));
            InitUpDownFloatData(GetDlgItem(hwnd, IDC_ALIGNFLOORWALLSCROLLER), 10.0f, 1.0f, 100.0f, 0.1f);
            SendMessage(hwndControl, CB_LIMITTEXT, 11, 0);

            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    DestroyWindow(hwnd);
                    editor->hwndUVEdit = NULL;
                    break;

                case IDC_PANUP:
                    levelInfo->PanUVs(FALSE, tstof(lpPan));
                    break;

                case IDC_PANDOWN:
                    levelInfo->PanUVs(FALSE, -tstof(lpPan));
                    break;

                case IDC_PANLEFT:
                    levelInfo->PanUVs(TRUE, tstof(lpPan));
                    break;

                case IDC_PANRIGHT:
                    levelInfo->PanUVs(TRUE, -tstof(lpPan));
                    break;

                case IDC_ROTATECLOCKWISE:
                    levelInfo->RotateUVs(tstof(lpRotateAngle));
                    break;

                case IDC_ROTATECCLOCKWISE:
                    levelInfo->RotateUVs(-tstof(lpRotateAngle));
                    break;

                case IDC_SCALEU:
                    levelInfo->ScaleUVs(SCALE_U, 1.0f/tstof(lpScaleAmount));
                    break;

                case IDC_SCALEV:
                    levelInfo->ScaleUVs(SCALE_V, 1.0f/tstof(lpScaleAmount));
                    break;

                case IDC_SCALEUV:
                    levelInfo->ScaleUVs(SCALE_U|SCALE_V, 1.0f/tstof(lpScaleAmount));
                    break;

                case IDC_FITTEXTURE:
                    levelInfo->FitUVs(tstof(lpHorizontalTile), tstof(lpVerticalTile));
                    break;

                case IDC_ALIGNWALL:
                    levelInfo->WallAlignUVs(hwnd);
                    break;

                case IDC_ALIGNFLOOR:
                    levelInfo->FloorAlignUVs(hwnd);
                    break;

                case IDC_PAN:
                case IDC_ROTATEANGLE:
                case IDC_SCALEAMOUNT:
                case IDC_HORIZONTALTILE:
                case IDC_VERTICALTILE:
                    {
                        if( (HIWORD(wParam) == CBN_EDITUPDATE) ||
                            (HIWORD(wParam) == CBN_SELCHANGE)  )
                        {
                            COMBOBOXINFO cbi;

                            cbi.cbSize = sizeof(cbi);

                            TCHAR lpText[12];
                            if(HIWORD(wParam) == CBN_EDITUPDATE)
                            {
                                GetComboBoxInfo(GetDlgItem(hwnd, LOWORD(wParam)), &cbi);
                                GetWindowText(cbi.hwndItem, lpText, 11);
                            }
                            else
                            {
                                DWORD dwCurSel = SendMessage(GetDlgItem(hwnd, LOWORD(wParam)), CB_GETCURSEL, 0, 0);
                                SendMessage(GetDlgItem(hwnd, LOWORD(wParam)), CB_GETLBTEXT, dwCurSel, (LPARAM)lpText);
                            }
                            double val = tstof(lpText);

                            BOOL bUpdateControl = FALSE;

                            if(*lpText)
                            {
                                TSTR lpTemp = lpText;
                                do
                                {
                                    if(isalpha(*lpTemp))
                                    {
                                        bUpdateControl = TRUE;
                                        break;
                                    }
                                }while(*++lpTemp);
                            }
                            else
                                bUpdateControl = TRUE;

                            tsprintf_s(lpText, 11, TEXT("%g"), val);

                            if(_finite(val))
                            {
                                switch(LOWORD(wParam))
                                {
                                    case IDC_PAN:
                                        scpy(lpPan, lpText); break;
                                    case IDC_ROTATEANGLE:
                                        scpy(lpRotateAngle, lpText); break;
                                    case IDC_SCALEAMOUNT:
                                        scpy(lpScaleAmount, lpText); break;
                                    case IDC_HORIZONTALTILE:
                                        scpy(lpHorizontalTile, lpText); break;
                                    case IDC_VERTICALTILE:
                                        scpy(lpVerticalTile, lpText); break;
                                }
                            }
                            else
                                bUpdateControl = TRUE;

                            if(bUpdateControl)
                            {
                                switch(LOWORD(wParam))
                                {
                                    case IDC_PAN:
                                        SetWindowText(cbi.hwndItem, lpPan); break;
                                    case IDC_ROTATEANGLE:
                                        SetWindowText(cbi.hwndItem, lpRotateAngle); break;
                                    case IDC_SCALEAMOUNT:
                                        SetWindowText(cbi.hwndItem, lpScaleAmount); break;
                                    case IDC_HORIZONTALTILE:
                                        SetWindowText(cbi.hwndItem, lpHorizontalTile); break;
                                    case IDC_VERTICALTILE:
                                        SetWindowText(cbi.hwndItem, lpVerticalTile); break;
                                }
                            }
                        }
                        break;
                    }
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            editor->hwndUVEdit = NULL;
            break;

        default:;
    }
    return FALSE;

    traceOut;
}


void EditorLevelInfo::PanUVs(BOOL bPanU, float amount)
{
    traceIn(EditorLevelInfo::PanUVs);

    DWORD i,j,k,l;

    SaveUVUndoData();

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        EditorMesh  *mesh  = &brush->mesh;
        Brush       *levelBrush = brush->GetLevelBrush();

        BitList ModifiedBrushUVs;
        BitList ModifiedMeshUVs;

        ModifiedBrushUVs.SetSize(brush->UVList.Num());
        ModifiedMeshUVs.SetSize(mesh->UVList.Num());

        for(j=0; j<brush->SelectedPolys.Num(); j++)
        {
            DWORD polyID = brush->SelectedPolys[j];

            PolyFace &meshPoly = mesh->PolyList[polyID];
            for(k=0; k<meshPoly.Faces.Num(); k++)
            {
                Face &face = mesh->FaceList[meshPoly.Faces[k]];

                for(l=0; l<3; l++)
                {
                    DWORD vert = face.ptr[l];
                    if(!ModifiedMeshUVs[vert])
                    {
                        if(bPanU)
                        {
                            mesh->UVList[vert].x += amount;
                            levelBrush->UVList[vert].x += amount;
                        }
                        else
                        {
                            mesh->UVList[vert].y += amount;
                            levelBrush->UVList[vert].y += amount;
                        }
                        ModifiedMeshUVs.Set(vert);
                    }
                }
            }

            PolyFace &brushPoly = brush->PolyList[polyID];
            for(k=0; k<brushPoly.Faces.Num(); k++)
            {
                EditFace &face = brush->FaceList[brushPoly.Faces[k]];

                for(l=0; l<3; l++)
                {
                    DWORD vert = face.uvFace.ptr[l];
                    if(!ModifiedBrushUVs[vert])
                    {
                        if(bPanU)
                            brush->UVList[vert].x += amount;
                        else
                            brush->UVList[vert].y += amount;

                        ModifiedBrushUVs.Set(vert);
                    }
                }
            }
        }

        levelBrush->VertBuffer->FlushBuffers();
    }

    UpdateViewports();

    traceOut;
}

void EditorLevelInfo::RotateUVs(float angle)
{
    traceIn(EditorLevelInfo::RotateUVs);

    DWORD i,j,k,l;

    SaveUVUndoData();

    float radAngle = RAD(angle);

    UVCoord UVec( cos(radAngle), sin(radAngle));
    UVCoord VVec(-sin(radAngle), cos(radAngle));

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        EditorMesh  *mesh  = &brush->mesh;
        Brush       *levelBrush = brush->GetLevelBrush();

        BitList ModifiedBrushUVs;
        BitList ModifiedMeshUVs;

        ModifiedBrushUVs.SetSize(brush->UVList.Num());
        ModifiedMeshUVs.SetSize(mesh->UVList.Num());

        for(j=0; j<brush->SelectedPolys.Num(); j++)
        {
            DWORD polyID = brush->SelectedPolys[j];

            PolyFace &meshPoly = mesh->PolyList[polyID];
            for(k=0; k<meshPoly.Faces.Num(); k++)
            {
                Face &face = mesh->FaceList[meshPoly.Faces[k]];

                for(l=0; l<3; l++)
                {
                    DWORD vert = face.ptr[l];
                    if(!ModifiedMeshUVs[vert])
                    {
                        UVCoord uv(mesh->UVList[vert]);

                        levelBrush->UVList[vert].x  =
                        mesh->UVList[vert].x = uv|UVec;

                        levelBrush->UVList[vert].y  =
                        mesh->UVList[vert].y = uv|VVec;

                        ModifiedMeshUVs.Set(vert);
                    }
                }
            }

            PolyFace &brushPoly = brush->PolyList[polyID];
            for(k=0; k<brushPoly.Faces.Num(); k++)
            {
                EditFace &face = brush->FaceList[brushPoly.Faces[k]];

                for(l=0; l<3; l++)
                {
                    DWORD vert = face.uvFace.ptr[l];
                    if(!ModifiedBrushUVs[vert])
                    {
                        UVCoord uv(brush->UVList[vert]);

                        brush->UVList[vert].x = uv|UVec;
                        brush->UVList[vert].y = uv|VVec;

                        ModifiedBrushUVs.Set(vert);
                    }
                }
            }
        }

        brush->UpdateTangents();

        levelBrush->VertBuffer->FlushBuffers();
    }

    UpdateViewports();

    traceOut;
}

void EditorLevelInfo::ScaleUVs(DWORD dwScale, float amount)
{
    traceIn(EditorLevelInfo::ScaleUVs);

    DWORD i,j,k,l;

    SaveUVUndoData();

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        EditorMesh  *mesh  = &brush->mesh;
        Brush       *levelBrush = brush->GetLevelBrush();

        BitList ModifiedBrushUVs;
        BitList ModifiedMeshUVs;

        ModifiedBrushUVs.SetSize(brush->UVList.Num());
        ModifiedMeshUVs.SetSize(mesh->UVList.Num());

        for(j=0; j<brush->SelectedPolys.Num(); j++)
        {
            DWORD polyID = brush->SelectedPolys[j];

            PolyFace &meshPoly = mesh->PolyList[polyID];
            for(k=0; k<meshPoly.Faces.Num(); k++)
            {
                Face &face = mesh->FaceList[meshPoly.Faces[k]];

                for(l=0; l<3; l++)
                {
                    DWORD vert = face.ptr[l];
                    if(!ModifiedMeshUVs[vert])
                    {
                        UVCoord &uv = mesh->UVList[vert];

                        if(dwScale & SCALE_U)
                        {
                            levelBrush->UVList[vert].x =
                            mesh->UVList[vert].x = uv.x*amount;
                        }

                        if(dwScale & SCALE_V)
                        {
                            levelBrush->UVList[vert].y =
                            mesh->UVList[vert].y = uv.y*amount;
                        }

                        ModifiedMeshUVs.Set(vert);
                    }
                }
            }

            PolyFace &brushPoly = brush->PolyList[polyID];
            for(k=0; k<brushPoly.Faces.Num(); k++)
            {
                EditFace &face = brush->FaceList[brushPoly.Faces[k]];

                for(l=0; l<3; l++)
                {
                    DWORD vert = face.uvFace.ptr[l];
                    if(!ModifiedBrushUVs[vert])
                    {
                        UVCoord &uv = brush->UVList[vert];

                        if(dwScale & SCALE_U)
                            uv.x *= amount;
                        if(dwScale & SCALE_V)
                            uv.y *= amount;

                        ModifiedBrushUVs.Set(vert);
                    }
                }
            }
        }

        levelBrush->VertBuffer->FlushBuffers();
    }

    UpdateViewports();

    traceOut;
}

void EditorLevelInfo::FitUVs(float uTile, float vTile)
{
    traceIn(EditorLevelInfo::FitUVs);

    DWORD i,j,k,l;

    SaveUVUndoData();

    //--------------------------------------------
    // Get the map direction

    Vect direction(0.0f, 0.0f, 0.0f);

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        EditorMesh  *mesh  = &brush->mesh;

        Matrix invRot = brush->GetLocalRot().GetInv();

        for(j=0; j<brush->SelectedPolys.Num(); j++)
        {
            DWORD polyID = brush->SelectedPolys[j];

            PolyFace &poly = brush->PolyList[polyID];

            for(k=0; k<poly.Faces.Num(); k++)
            {
                Vect faceDir = brush->FaceNormals[poly.Faces[k]];
                faceDir.TransformVector(invRot);
                direction += faceDir;
            }
        }
    }

    direction.Norm();

    Vect directionX, directionY;

    if(CloseFloat(direction.y, 1.0f, EPSILON))
    {
        directionX.Set(1.0f, 0.0f, 0.0f);
        directionY.Set(0.0f, 0.0f, 1.0f);
    }
    else if(CloseFloat(direction.y, -1.0f, EPSILON))
    {
        directionX.Set(1.0f, 0.0f, 0.0f);
        directionY.Set(0.0f, 0.0f, -1.0f);
    }
    else
    {
        Vect yAxis(0.0f, 1.0f, 0.0f);
        directionX = -(direction^yAxis).Norm();
        directionY = -(direction^directionX);
    }

    //--------------------------------------------
    // Get the map boundries

    float minX, maxX, minY, maxY;
    BOOL bHaveValues = FALSE;

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        EditorMesh  *mesh  = &brush->mesh;

        for(j=0; j<brush->SelectedPolys.Num(); j++)
        {
            DWORD polyID = brush->SelectedPolys[j];

            PolyFace &poly = mesh->PolyList[polyID];

            for(k=0; k<poly.Faces.Num(); k++)
            {
                Face &face = mesh->FaceList[poly.Faces[k]];

                for(l=0; l<3; l++)
                {
                    Vect &v = mesh->VertList[face.ptr[l]];

                    float xVal = directionX|v;
                    float yVal = directionY|v;

                    if(!bHaveValues)
                    {
                        minX = maxX = xVal;
                        minY = maxY = yVal;
                        bHaveValues = TRUE;
                    }
                    else
                    {
                        minX = MIN(minX, xVal);  maxX = MAX(maxX, xVal);
                        minY = MIN(minY, yVal);  maxY = MAX(maxY, yVal);
                    }
                }
            }
        }
    }

    float xLenI = 1.0f/((maxX-minX)/uTile);
    float yLenI = 1.0f/((maxY-minY)/vTile);

    //--------------------------------------------
    // Get the UVs

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        EditorMesh  *mesh  = &brush->mesh;
        Brush       *levelBrush = brush->GetLevelBrush();

        Matrix mat;
        mat.SetIdentity();
        mat.Translate(brush->Pos);
        mat.Rotate(brush->GetLocalRot());

        for(j=0; j<brush->SelectedPolys.Num(); j++)
        {
            DWORD polyID = brush->SelectedPolys[j];

            PolyFace &brushPoly = brush->PolyList[polyID];
            for(k=0; k<brushPoly.Faces.Num(); k++)
            {
                EditFace &face = brush->FaceList[brushPoly.Faces[k]];

                for(l=0; l<3; l++)
                {
                    Vect v = brush->PointList[face.pointFace.ptr[l]];
                    UVCoord &uv = brush->UVList[face.uvFace.ptr[l]];

                    v.TransformPoint(mat);

                    uv.x = ((directionX|v)-minX)*xLenI;
                    uv.y = ((directionY|v)-minY)*yLenI;
                }
            }


            PolyFace &meshPoly = mesh->PolyList[polyID];
            for(k=0; k<meshPoly.Faces.Num(); k++)
            {
                Face &face = mesh->FaceList[meshPoly.Faces[k]];

                for(l=0; l<3; l++)
                {
                    Vect &v = mesh->VertList[face.ptr[l]];
                    UVCoord &uv = mesh->UVList[face.ptr[l]];
                    UVCoord &brushUV = levelBrush->UVList[face.ptr[l]];
                    Vect &tangent = levelBrush->TangentList[face.ptr[l]];

                    uv.x = ((directionX|v)-minX)*xLenI;
                    uv.y = ((directionY|v)-minY)*yLenI;

                    brushUV = uv;

                    tangent = -directionX;
                }
            }
        }

        levelBrush->VertBuffer->FlushBuffers();

    }

    UpdateViewports();

    traceOut;
}

void EditorLevelInfo::FloorAlignUVs(HWND hwndAlignWindow)
{
    traceIn(EditorLevelInfo::FloorAlignUVs);

    DWORD i,j,k,l;

    SaveUVUndoData();

    float divValue = 1.0f/GetUpDownFloat(GetDlgItem(hwndAlignWindow, IDC_ALIGNFLOORWALLSCROLLER));

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        EditorMesh  *mesh  = &brush->mesh;
        Brush       *levelBrush = brush->GetLevelBrush();

        Matrix mat;
        mat.SetIdentity();
        mat.Translate(brush->GetLocalPos());
        mat.Rotate(brush->GetLocalRot());

        BitList ModifiedBrushUVs;
        BitList ModifiedMeshUVs;

        ModifiedBrushUVs.SetSize(brush->UVList.Num());
        ModifiedMeshUVs.SetSize(mesh->UVList.Num());

        for(j=0; j<brush->SelectedPolys.Num(); j++)
        {
            DWORD polyID = brush->SelectedPolys[j];
            DWORD firstFace = brush->PolyList[polyID].Faces[0];
            BOOL bCheckPoly = FALSE;
            BOOL bIgnorePoly = FALSE;

            float uMultiply = 1.0f, vMultiply = 1.0f;

            Material *material = brush->Materials[polyID];

            if(material)
            {
                Effect *effect = material->effect;

                for(k=0; k<material->Params.Num(); k++)
                {
                    MaterialParameter &param = material->Params[k];
                    if(param.type == Parameter_Texture)
                    {
                        float fWidth  = (*(Texture**)param.data)->Width();
                        float fHeight = (*(Texture**)param.data)->Height();

                        if(fWidth > fHeight)
                            uMultiply = fHeight/fWidth;
                        else
                            vMultiply = fWidth/fHeight;
                    }
                }
            }

            //uMultiply *= 0.1f;
            //vMultiply *= 0.1f;

            uMultiply *= divValue;
            vMultiply *= divValue;

            PolyFace &meshPoly = mesh->PolyList[polyID];
            for(k=0; k<meshPoly.Faces.Num(); k++)
            {
                if(!bCheckPoly)
                {
                    if(fabs(mesh->PlaneList[mesh->FacePlaneList[meshPoly.Faces[k]]].Dir.y) < 0.707)
                    {
                        bIgnorePoly = TRUE;
                        break;
                    }
                    bCheckPoly = TRUE;
                }

                Face &face = mesh->FaceList[meshPoly.Faces[k]];
                for(l=0; l<3; l++)
                {
                    DWORD vert = face.ptr[l];
                    if(!ModifiedMeshUVs[vert])
                    {
                        levelBrush->UVList[vert].x = mesh->UVList[vert].x = mesh->VertList[vert].x*uMultiply;
                        levelBrush->UVList[vert].y = mesh->UVList[vert].y = mesh->VertList[vert].z*vMultiply;
                        levelBrush->TangentList[vert].Set(-1.0f, 0.0f, 0.0f);

                        ModifiedMeshUVs.Set(vert);
                    }
                }
            }

            if(bIgnorePoly)
                continue;

            PolyFace &brushPoly = brush->PolyList[polyID];
            for(k=0; k<brushPoly.Faces.Num(); k++)
            {
                EditFace &face = brush->FaceList[brushPoly.Faces[k]];

                for(l=0; l<3; l++)
                {
                    DWORD uvVert = face.uvFace.ptr[l];
                    DWORD pointVert = face.pointFace.ptr[l];

                    Vect vert = brush->PointList[pointVert].GetTransformedPoint(mat);

                    if(!ModifiedBrushUVs[uvVert])
                    {
                        UVCoord &uv = brush->UVList[uvVert];

                        uv.x = vert.x*uMultiply;
                        uv.y = vert.z*vMultiply;

                        ModifiedBrushUVs.Set(uvVert);
                    }
                }
            }
        }

        levelBrush->VertBuffer->FlushBuffers();
    }

    UpdateViewports();

    traceOut;
}


void EditorLevelInfo::WallAlignUVs(HWND hwndAlignWindow)
{
    traceIn(EditorLevelInfo::WallAlignUVs);

    DWORD i,j,k,l;

    SaveUVUndoData();

    Vect UDir(1.0f, 0.0f, 0.0f), VDir(0.0f, 1.0f, 0.0f);

    float lowestV = 0.0f;
    BOOL bFirst = TRUE;

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        EditorMesh  *mesh  = &brush->mesh;
        Brush       *levelBrush = brush->GetLevelBrush();

        for(j=0; j<brush->SelectedPolys.Num(); j++)
        {
            DWORD polyID = brush->SelectedPolys[j];
            PolyFace &meshPoly = mesh->PolyList[polyID];

            for(k=0; k<meshPoly.Faces.Num(); k++)
            {
                Face &face = mesh->FaceList[meshPoly.Faces[k]];
                for(l=0; l<3; l++)
                {
                    float curV = mesh->VertList[face.ptr[l]].y;

                    if(bFirst)
                    {
                        lowestV = curV;
                        bFirst = FALSE;
                    }
                    else if(curV < lowestV)
                        lowestV = curV;
                }
            }
        }
    }

    //lowestV *= 0.1;

    float divValue = 1.0f/GetUpDownFloat(GetDlgItem(hwndAlignWindow, IDC_ALIGNFLOORWALLSCROLLER));
    lowestV *= divValue;

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        EditorMesh  *mesh  = &brush->mesh;
        Brush       *levelBrush = brush->GetLevelBrush();

        Matrix mat;
        mat.SetIdentity();
        mat.Translate(brush->GetLocalPos());
        mat.Rotate(brush->GetLocalRot());

        BitList ModifiedBrushUVs;
        BitList ModifiedMeshUVs;

        ModifiedBrushUVs.SetSize(brush->UVList.Num());
        ModifiedMeshUVs.SetSize(mesh->UVList.Num());

        for(j=0; j<brush->SelectedPolys.Num(); j++)
        {
            DWORD polyID = brush->SelectedPolys[j];
            DWORD firstFace = brush->PolyList[polyID].Faces[0];
            BOOL bCheckPoly = FALSE;
            BOOL bIgnorePoly = FALSE;

            float uMultiply = 1.0f, vMultiply = 1.0f;

            Material *material = brush->Materials[polyID];
            if(material)
            {
                Effect *effect = material->effect;

                for(k=0; k<material->Params.Num(); k++)
                {
                    MaterialParameter &param = material->Params[k];
                    if(param.type == Parameter_Texture)
                    {
                        float fWidth  = (*(Texture**)param.data)->Width();
                        float fHeight = (*(Texture**)param.data)->Height();

                        if(fWidth > fHeight)
                            uMultiply = fHeight/fWidth;
                        else
                            vMultiply = fWidth/fHeight;
                    }
                }
            }

            //uMultiply *= 0.1f;
            //vMultiply *= 0.1f;

            uMultiply *= divValue;
            vMultiply *= divValue;

            PolyFace &meshPoly = mesh->PolyList[polyID];
            for(k=0; k<meshPoly.Faces.Num(); k++)
            {
                if(!bCheckPoly)
                {
                    Plane &plane = mesh->PlaneList[mesh->FacePlaneList[meshPoly.Faces[k]]];
                    if(fabsf(plane.Dir.y) > 0.3f)
                    {
                        bIgnorePoly = TRUE;
                        break;
                    }
                    bCheckPoly = TRUE;

                    UDir = plane.Dir.Cross(Vect(0.0f, -1.0f, 0.0f));
                    VDir = -plane.Dir.Cross(UDir);
                }

                Face &face = mesh->FaceList[meshPoly.Faces[k]];
                for(l=0; l<3; l++)
                {
                    DWORD vert = face.ptr[l];
                    if(!ModifiedMeshUVs[vert])
                    {
                        levelBrush->UVList[vert].x = mesh->UVList[vert].x = (mesh->VertList[vert].Dot(UDir)*uMultiply);
                        levelBrush->UVList[vert].y = mesh->UVList[vert].y = lowestV + (mesh->VertList[vert].Dot(VDir)*vMultiply);
                        levelBrush->TangentList[vert] = -UDir;

                        ModifiedMeshUVs.Set(vert);
                    }
                }
            }

            if(bIgnorePoly)
                continue;

            PolyFace &brushPoly = brush->PolyList[polyID];
            for(k=0; k<brushPoly.Faces.Num(); k++)
            {
                EditFace &face = brush->FaceList[brushPoly.Faces[k]];

                for(l=0; l<3; l++)
                {
                    DWORD uvVert = face.uvFace.ptr[l];
                    DWORD pointVert = face.pointFace.ptr[l];

                    Vect vert = brush->PointList[pointVert].GetTransformedPoint(mat);

                    if(!ModifiedBrushUVs[uvVert])
                    {
                        UVCoord &uv = brush->UVList[uvVert];

                        uv.x = (vert.Dot(UDir)*uMultiply);
                        uv.y = lowestV + (vert.Dot(VDir)*vMultiply);

                        ModifiedBrushUVs.Set(uvVert);
                    }
                }
            }
        }

        levelBrush->VertBuffer->FlushBuffers();
    }

    UpdateViewports();

    traceOut;
}


void EditorLevelInfo::SaveUVUndoData()
{
    traceIn(EditorLevelInfo::SaveUVUndoData);

    DWORD dwNumBrushes = 0;
    for(int i=0; i<BrushList.Num(); i++)
    {
        if(BrushList[i]->SelectedPolys.Num())
            ++dwNumBrushes;
    }

    if(dwNumBrushes)
    {
        Action action;
        action.strName = TEXT("UV Operation");
        action.actionProc = EditorLevelInfo::UndoRedoUVOperation;
        BufferOutputSerializer s(action.data);

        s << dwNumBrushes;

        for(int i=0; i<BrushList.Num(); i++)
        {
            EditorBrush *brush = BrushList[i];

            if(brush->SelectedPolys.Num())
            {
                s << brush->name;
                brush->SerializeUVData(s);
            }
        }

        editor->undoStack->Push(action);
    }

    traceOut;
}

void ENGINEAPI EditorLevelInfo::UndoRedoUVOperation(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoUVOperation);

    DWORD dwNumBrushes;
    s    << dwNumBrushes;
    sOut << dwNumBrushes;

    for(int i=0; i<dwNumBrushes; i++)
    {
        String strBrushName;
        s    << strBrushName;
        sOut << strBrushName;

        EditorBrush *brush = (EditorBrush*)Entity::FindByName(strBrushName);

        brush->SerializeUVData(sOut);
        brush->SerializeUVData(s);
    }

    UpdateViewports();

    traceOut;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  SurfacePropertiesEditor.cpp

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


BOOL WINAPI SurfacePropertiesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

SurfacePropertiesEditor *surfaceProperties = NULL;



SurfacePropertiesEditor::SurfacePropertiesEditor()
{
    hwndSurfaceProperties = CreateDialog(hinstMain, MAKEINTRESOURCE(IDD_SURFACEPROPERTIES), hwndEditor, (DLGPROC)SurfacePropertiesProc);
    surfaceProperties = this;

    UpdateProperties();
}

SurfacePropertiesEditor::~SurfacePropertiesEditor()
{
    DestroyWindow(hwndSurfaceProperties);
    surfaceProperties = NULL;
}


void SurfacePropertiesEditor::UpdateProperties()
{
    DWORD numPolys = 0;
    DWORD ButtonStates[32];
	int i;

    zero(ButtonStates, sizeof(DWORD)*32);

    for(i=0; i<levelInfo->BrushList.Num(); i++)
    {
        EditorBrush *brush = levelInfo->BrushList[i];

        for(int j=0; j<brush->SelectedPolys.Num(); j++)
        {
            DWORD polyID = brush->SelectedPolys[j];
            PolyFace &meshPoly = brush->PolyList[polyID];
            EditFace &editFace = brush->FaceList[meshPoly.Faces[0]];

            for(int k=0; k<32; k++)
            {
                DWORD val = ((editFace.smoothFlags & (1<<k)) > 0);
                ButtonStates[k] |= ++val;
            }

            ++numPolys;
        }
    }

    for(i=0; i<32; i++)
    {
        HWND hwndSGControl = GetDlgItem(hwndSurfaceProperties, IDC_SG1+i);

        TCHAR controlName[7];
        itots_s(i+1, controlName, 6, 10);

        switch(ButtonStates[i])
        {
            case 0:
            case 1:
                SendMessage(hwndSGControl, BM_SETCHECK, BST_UNCHECKED, 0);
                SetWindowText(hwndSGControl, controlName);
                break;

            case 2:
                SendMessage(hwndSGControl, BM_SETCHECK, BST_CHECKED, 0);
                SetWindowText(hwndSGControl, controlName);
                break;

            case 3:
                SendMessage(hwndSGControl, BM_SETCHECK, BST_CHECKED, 0);
                SetWindowText(hwndSGControl, NULL);
                break;
        }
    }

    String strTitle;

    strTitle << TEXT("Surface Properties (") << (long)numPolys << TEXT(" selected)");
    SetWindowText(hwndSurfaceProperties, strTitle);
}


BOOL WINAPI SurfacePropertiesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_COMMAND:
            {
                DWORD controlID = LOWORD(wParam);

                if((controlID >= IDC_SG1) && (controlID <= IDC_SG32))
                {
                    HWND hwndSGControl = GetDlgItem(hwnd, controlID);

                    DWORD smoothGroup = controlID-IDC_SG1;
                    DWORD smoothFlag = 1<<smoothGroup;

                    TCHAR controlName[3];
                    itots_s((controlID-IDC_SG1)+1, controlName, 2, 10);

                    DWORD state = SendMessage(hwndSGControl, BM_GETCHECK, 0, 0);
                    BOOL bSet = (state != BST_CHECKED);

                    SendMessage(hwndSGControl, BM_SETCHECK, bSet, 0);
                    SetWindowText(hwndSGControl, controlName);

                    DWORD i,j,k;

                    for(i=0; i<levelInfo->BrushList.Num(); i++)
                    {
                        EditorBrush *brush = levelInfo->BrushList[i];
                        Brush       *levelBrush = brush->GetLevelBrush();

                        if(brush->SelectedPolys.Num())
                        {
                            for(j=0; j<brush->SelectedPolys.Num(); j++)
                            {
                                PolyFace &poly = brush->PolyList[brush->SelectedPolys[j]];

                                for(k=0; k<poly.Faces.Num(); k++)
                                {
                                    EditFace &face = brush->FaceList[poly.Faces[k]];

                                    if(bSet)
                                        face.smoothFlags |= smoothFlag;
                                    else
                                        face.smoothFlags &= ~smoothFlag;
                                }
                            }
                            brush->UpdateNormals();
                            brush->UpdateTangents();
                            levelBrush->VertBuffer->FlushBuffers();
                        }
                    }

                    UpdateViewports();
                }
                else if(controlID == IDOK)
                    delete surfaceProperties;
                break;
            }

        case WM_CLOSE:
            delete surfaceProperties;
    }

    return FALSE;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ViewportConfig.cpp [EditorEngine]

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


BOOL CALLBACK ConfigureVPsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void ENGINEAPI SplitterProc(Vect2 &newPos, DWORD param1, DWORD param2);



void EditorEngine::KillViewports()
{
    traceIn(EditorEngine::KillViewports);

    DWORD i;

    for(i=0; i<Splitters.Num(); i++)
        DestroyObject(Splitters[i]);
    Splitters.Clear();

    for(i=0; i<Viewports.Num(); i++)
        DestroyObject(Viewports[i]);
    Viewports.Clear();

    currentLayout = (Layout)-1;

    traceOut;
}


void EditorEngine::SetViewportLayout(Layout newLayout, Vect2 *splitterPos)
{
    traceIn(EditorEngine::SetViewportLayout);

    EditorViewport *vp;
    Splitter *splitter;
    DWORD i;

    Vect2 gdSize = GS->GetSize();

    float posX = float(gdSize.x/2);
    float posY = float(gdSize.y/2);

    if(splitterPos)
    {
        posX = splitterPos->x;
        posY = splitterPos->y;
    }
    else
    {
        splitPos.x = posX;
        splitPos.y = posY;
    }

    float rightSize  = gdSize.x-posX;
    float bottomSize = gdSize.y-posY;

    //reset viewports
    if(newLayout != currentLayout)
    {
        KillViewports();

        DWORD newViewports, newSplitters;

        switch(newLayout)
        {
            case Layout_SinglePanel:
                newViewports = 1;
                newSplitters = 0;
                break;
            case Layout_VerticalPair:
            case Layout_HorizontalPair:
                newViewports = 2;
                newSplitters = 1;
                break;
            case Layout_TopPair_SingleBottom:
            case Layout_LeftPair_SingleRight:
            case Layout_BottomPair_SingleTop:
            case Layout_RightPair_SingleLeft:
                newViewports = 3;
                newSplitters = 3;
                break;
            case Layout_FourPanel:
                newViewports = newSplitters = 4;
                break;
        }

        for(i=0; i<newViewports; i++)
            Viewports << CreateObject(EditorViewport);

        for(i=0; i<newSplitters; i++)
        {
            Splitters << CreateObject(Splitter);
            Splitters[i]->SetSplitterHandler((SPLITTERHANDLER)SplitterProc, (UPARAM)this, 0);
        }

        switch(newLayout)
        {
            case Layout_SinglePanel:
                {
                    Viewports[0]->SetViewportType(ViewportType_Main);
                    break;
                }
            case Layout_VerticalPair:
            case Layout_HorizontalPair:
                {
                    Viewports[0]->SetViewportType(ViewportType_Top);
                    Viewports[1]->SetViewportType(ViewportType_Main);
                    break;
                }
            case Layout_TopPair_SingleBottom:
            case Layout_LeftPair_SingleRight:
            case Layout_BottomPair_SingleTop:
            case Layout_RightPair_SingleLeft:
                {
                    Viewports[0]->SetViewportType(ViewportType_Top);
                    Viewports[1]->SetViewportType(ViewportType_Front);
                    Viewports[2]->SetViewportType(ViewportType_Main);
                    break;
                }
            case Layout_FourPanel:
                {
                    Viewports[0]->SetViewportType(ViewportType_Top);
                    Viewports[1]->SetViewportType(ViewportType_Front);
                    Viewports[2]->SetViewportType(ViewportType_Right);
                    Viewports[3]->SetViewportType(ViewportType_Main);
                    break;
                }
        }
    }

    switch(newLayout)
    {
        case Layout_SinglePanel:
            {
                vp = Viewports[0];
                vp->SetPos(0.0f, 0.0f);
                vp->SetSize(gdSize);
                break;
            }
        case Layout_VerticalPair:
            {
                vp = Viewports[0];          //left
                vp->SetPos(0.0f, 0.0f);
                vp->SetSize(posX-2.0f, gdSize.y);

                vp = Viewports[1];          //right
                vp->SetPos(posX+2.0f, 0.0f);
                vp->SetSize(rightSize-2.0f, gdSize.y);

                //-------------------------------
                //splitters
                splitter = Splitters[0];    //vertical
                splitter->SetPos (posX-2.0f, 0.0f);
                splitter->SetSize(5.0f, gdSize.y);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_VERTICAL;
                break;
            }
        case Layout_HorizontalPair:
            {
                vp = Viewports[0];          //left
                vp->SetPos(0.0f, 0.0f);
                vp->SetSize(gdSize.x, posY-2.0f);

                vp = Viewports[1];          //right
                vp->SetPos(0.0f, posY+2.0f);
                vp->SetSize(gdSize.x, bottomSize-2.0f);

                //-------------------------------
                //splitters
                splitter = Splitters[0];    //horizontal
                splitter->SetPos (0.0f, posY-2.0f);
                splitter->SetSize(gdSize.x, 5.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_HORIZONTAL;
                break;
            }
        case Layout_TopPair_SingleBottom:
            {
                vp = Viewports[0];          //top left
                vp->SetPos(0.0f, 0.0f);
                vp->SetSize(posX-2.0f, posY-2.0f);

                vp = Viewports[1];          //top right
                vp->SetPos(posX+2.0f, 0.0f);
                vp->SetSize(rightSize-2.0f, posY-2.0f);

                vp = Viewports[2];          //bottom
                vp->SetPos(0.0f, posY+2.0f);
                vp->SetSize(gdSize.x, bottomSize-2.0f);

                //-------------------------------
                //splitters
                splitter = Splitters[0];    //horizontal
                splitter->SetPos (0.0f, posY-2.0f);
                splitter->SetSize(gdSize.x, 5.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_HORIZONTAL;

                splitter = Splitters[1];    //vertical
                splitter->SetPos (posX-2.0f, 0.0f);
                splitter->SetSize(5.0f, posY-1.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_VERTICAL;

                splitter = Splitters[2];    //center
                splitter->SetPos (posX-5.0f,  posY-5.0f);
                splitter->SetSize(9.0f,        9.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_CENTER;
                break;
            }
        case Layout_LeftPair_SingleRight:
            {
                vp = Viewports[0];          //top left
                vp->SetPos(0.0f, 0.0f);
                vp->SetSize(posX-2.0f, posY-2.0f);

                vp = Viewports[1];          //bottom left
                vp->SetPos(0.0f, posY+2.0f);
                vp->SetSize(posX-2.0f, bottomSize-2.0f);

                vp = Viewports[2];          //right
                vp->SetPos(posX+2.0f, 0.0f);
                vp->SetSize(rightSize-2.0f, gdSize.y);

                //-------------------------------
                //splitters
                splitter = Splitters[0];    //vertical
                splitter->SetPos (posX-2.0f, 0.0f);
                splitter->SetSize(5.0f, gdSize.y-1.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_VERTICAL;

                splitter = Splitters[1];    //horizontal
                splitter->SetPos (0.0f, posY-2.0f);
                splitter->SetSize(posX-1.0f, 5.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_HORIZONTAL;

                splitter = Splitters[2];    //center
                splitter->SetPos (posX-5.0f,  posY-5.0f);
                splitter->SetSize(9.0f,        9.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_CENTER;
                break;
            }
        case Layout_BottomPair_SingleTop:
            {
                vp = Viewports[0];          //bottom left
                vp->SetPos(0.0f, posY+2.0f);
                vp->SetSize(posX-2.0f, bottomSize-2.0f);

                vp = Viewports[1];          //bottom right
                vp->SetPos(posX+2.0f, posY+2.0f);
                vp->SetSize(rightSize-2.0f, bottomSize-2.0f);

                vp = Viewports[2];          //top
                vp->SetPos(0.0f, 0.0f);
                vp->SetSize(gdSize.x, posY-2.0f);

                //-------------------------------
                //splitters
                splitter = Splitters[0];    //horizontal
                splitter->SetPos (0.0f, posY-2.0f);
                splitter->SetSize(gdSize.x, 5.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_HORIZONTAL;

                splitter = Splitters[1];    //vertical
                splitter->SetPos (posX-2.0f, posY+2.0f);
                splitter->SetSize(5.0f, bottomSize-1.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_VERTICAL;

                splitter = Splitters[2];    //center
                splitter->SetPos (posX-5.0f,  posY-5.0f);
                splitter->SetSize(9.0f,        9.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_CENTER;
                break;
            }
        case Layout_RightPair_SingleLeft:
            {
                vp = Viewports[0];          //top right
                vp->SetPos(posX+2.0f, 0.0f);
                vp->SetSize(rightSize-2.0f, posY-2.0f);

                vp = Viewports[1];          //bottom right
                vp->SetPos(posX+2.0f, posY+2.0f);
                vp->SetSize(rightSize-2.0f, bottomSize-2.0f);

                vp = Viewports[2];          //left
                vp->SetPos(0.0f, 0.0f);
                vp->SetSize(posX-2.0f, gdSize.y);

                //-------------------------------
                //splitters
                splitter = Splitters[0];    //vertical
                splitter->SetPos (posX-2.0f, 0.0f);
                splitter->SetSize(5.0f, gdSize.y-1.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_VERTICAL;

                splitter = Splitters[1];    //horizontal
                splitter->SetPos (posX+2.0, posY-2.0f);
                splitter->SetSize(rightSize-1.0f, 5.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_HORIZONTAL;

                splitter = Splitters[2];    //center
                splitter->SetPos (posX-5.0f,  posY-5.0f);
                splitter->SetSize(9.0f,        9.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_CENTER;
                break;
            }
            break;
        case Layout_FourPanel:
            {
                vp = Viewports[0];          //upper left
                vp->SetPos (0.0f,                  0.0f);
                vp->SetSize(posX-2.0f,             posY-2.0f);

                vp = Viewports[1];          //upper right
                vp->SetPos (posX+2.0f,             0.0f);
                vp->SetSize(rightSize-2.0f,        posY-2.0f);

                vp = Viewports[2];          //lower left
                vp->SetPos (0.0f,                  posY+2.0f);
                vp->SetSize(posX-2.0f,             bottomSize-2.0f);

                vp = Viewports[3];          //lower right
                vp->SetPos (posX+2.0f,             posY+2.0f);
                vp->SetSize(rightSize-2.0f,        bottomSize-2.0f);

                //-------------------------------
                //splitters
                splitter = Splitters[0];    //horizontal
                splitter->SetPos (0.0f,        posY-2.0f);
                splitter->SetSize(gdSize.x,  5.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_HORIZONTAL;

                splitter = Splitters[1];    //vertical 1
                splitter->SetPos (posX-2.0f,   0.0f);
                splitter->SetSize(5.0f,        posY-1.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_VERTICAL;

                splitter = Splitters[2];    //vertical 2
                splitter->SetPos (posX-2.0f,   posY+2.0f);
                splitter->SetSize(5.0f,        gdSize.y-posY-2.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_VERTICAL;

                splitter = Splitters[3];    //center
                splitter->SetPos (posX-5.0f,  posY-5.0f);
                splitter->SetSize(9.0f,        9.0f);
                splitter->splitterPos.Set(posX, posY);
                splitter->splitterType = SPLITTER_CENTER;
                break;
            }
    }

    for(i=0; i<Splitters.Num(); i++)
        Splitters[i]->ResetBuffers();

    currentLayout = newLayout;

    UpdateViewports();

    traceOut;
}


void EditorEngine::ResetSound()
{
    traceIn(EditorEngine::ResetSound);

    bool bFoundSoundCamera = false;
    for(DWORD i=0; i<Viewports.Num(); i++)
    {
        if(Viewports[i]->GetCamera()->IsSoundCamera())
        {
            bFoundSoundCamera = true;
            break;
        }
    }

    if(!bFoundSoundCamera)
        SS->SetEffectsVol(0.0f);
    else
        SS->SetEffectsVol(AppConfig->GetFloat(TEXT("Sound"), TEXT("EffectsVolume"), 0.3f));

    traceOut;
}


void EditorEngine::ConfigureViewports()
{
    traceIn(EditorEngine::ConfigureViewports);

    DialogBox(hinstMain, MAKEINTRESOURCE(IDD_VIEWPORTCONFIG), hwndMain, (DLGPROC)ConfigureVPsProc);

    traceOut;
}


BOOL CALLBACK ConfigureVPsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(ConfigureVPsProc);

    switch(message)
    {
        case WM_INITDIALOG:
            SendMessage(GetDlgItem(hwnd, IDC_FP),   BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)editor->hbmpFP);
            SendMessage(GetDlgItem(hwnd, IDC_VP),   BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)editor->hbmpVP);
            SendMessage(GetDlgItem(hwnd, IDC_HP),   BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)editor->hbmpHP);
            SendMessage(GetDlgItem(hwnd, IDC_SP),   BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)editor->hbmpSP);
            SendMessage(GetDlgItem(hwnd, IDC_SBTP), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)editor->hbmpSBTP);
            SendMessage(GetDlgItem(hwnd, IDC_STBP), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)editor->hbmpSTBP);
            SendMessage(GetDlgItem(hwnd, IDC_SRLP), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)editor->hbmpSRLP);
            SendMessage(GetDlgItem(hwnd, IDC_SLRP), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)editor->hbmpSLRP);
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_FP:
                    editor->SetViewportLayout(Layout_FourPanel);
                    break;
                case IDC_VP:
                    editor->SetViewportLayout(Layout_VerticalPair);
                    break;
                case IDC_HP:
                    editor->SetViewportLayout(Layout_HorizontalPair);
                    break;
                case IDC_SP:
                    editor->SetViewportLayout(Layout_SinglePanel);
                    break;
                case IDC_SBTP:
                    editor->SetViewportLayout(Layout_TopPair_SingleBottom);
                    break;
                case IDC_STBP:
                    editor->SetViewportLayout(Layout_BottomPair_SingleTop);
                    break;
                case IDC_SRLP:
                    editor->SetViewportLayout(Layout_LeftPair_SingleRight);
                    break;
                case IDC_SLRP:
                    editor->SetViewportLayout(Layout_RightPair_SingleLeft);
                    break;
            }
            EndDialog(hwnd, 0);
    }

    return FALSE;

    traceOut;
}


void ENGINEAPI SplitterProc(Vect2 &newPos, DWORD param1, DWORD param2)
{
    traceIn(SplitterProc);

    Vect2 max = GS->GetSize()-40.0f;

    if(newPos.x < 40.0f)
        newPos.x = 40.0f;
    else if(newPos.x > max.x)
        newPos.x = max.x;

    if(newPos.y < 40.0f)
        newPos.y = 40.0f;
    else if(newPos.y > max.y)
        newPos.y = max.y;

    editor->splitPos = newPos;

    for(DWORD i=0; i<editor->Splitters.Num(); i++)
        editor->Splitters[i]->splitterPos = newPos;

    editor->SetViewportLayout(editor->currentLayout, &newPos);

    traceOut;
}

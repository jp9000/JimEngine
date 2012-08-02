/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditorEngine.h

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


class UndoRedoStack;


struct DIB
{
    BITMAPINFO bi;
    LPBYTE lpBitmap;

    DIB()   {zero(&bi, sizeof(bi)); bi.bmiHeader.biSize = sizeof(bi);}
    ~DIB()  {FreeBitmap();}

    void FreeBitmap() {Free(lpBitmap); lpBitmap = NULL;}
};


/*==============================================================
  Editor Engine
===============================================================*/

//--------------------------------------------------
// Layout enum
enum Layout
{
    Layout_SinglePanel,
    Layout_VerticalPair,
    Layout_HorizontalPair,
    Layout_TopPair_SingleBottom,
    Layout_LeftPair_SingleRight,
    Layout_BottomPair_SingleTop,
    Layout_RightPair_SingleLeft,
    Layout_FourPanel
};

class EditorKBHandler;

//--------------------------------------------------
// EditorEngine class
class EditorEngine : public Engine
{
    DeclareClass(EditorEngine, Engine);

    EditorKBHandler *editorKBHandler;

public:
    void Init();
    void Destroy();

    void PreFrame();

    void KillViewports();
    void SetViewportLayout(Layout newLayout, Vect2 *splitterPos=NULL);

    void SetStatusText(DWORD statusNum, CTSTR lpText);

    void ResetSound();

    void ConfigureViewports();
    EditorViewport* GetMainViewport();

    void CreateIndoorSidebar();
    void CreateOctLevelSidebar(DWORD dwTab=0, bool bSwitching=false);
    void CreateOutdoorSidebar();

    BYTE* EditorEngine::ProcessTargaData(LPBYTE lpTargaData, BOOL bInvert, BOOL bBGR);

    void NewIndoorLevel(BOOL bDisplaySaveMessage=TRUE);
    void NewOctLevel(BOOL bDisplaySaveMessage=TRUE);
    void NewOutdoorLevel(BOOL bDisplaySaveMessage=TRUE);

    void SaveLevelAs();
    void SaveIndoorLevel();
    void SaveOctLevel();
    void SaveOutdoorLevel();

    void OpenLevel();
    BOOL OpenEditableLevel(CTSTR lpName, BOOL bShowMessages=TRUE);

    Vect& SnapPoint(Vect &v) const;

    void EnableRealTimeRendering(bool bEnable);
    void HideEditorObjects(bool bHide);

    bool EditBoxDialog(HWND hwndParent, CTSTR lpName, String &result);

    UndoRedoStack *undoStack, *redoStack;

    bool        bRealTimeEnabled;
    bool        bHideEditorObjects;

    StringList  DependantModules;
    BOOL        bAddonModule;
    String      curWorkingModule;

    float       moveSpeed;

    HWND        hwndUVEdit;
    HWND        hwndObjectBrowser;

    Class       *selectedEntityClass;

    Effect      *editorEffects;

    SelectionBox *selBoxThing;

    //Draw info
    Layout currentLayout;
    List<EditorViewport*> Viewports;
    List<Splitter*> Splitters;

    VertexBuffer *vbGridLineV, *vbGridLineH;
    VertexBuffer *snapGuide;

    Vect2       splitPos;

    HBITMAP     hbmpFP, hbmpHP, hbmpVP, hbmpSP,
                hbmpSBTP, hbmpSTBP, hbmpSRLP, hbmpSLRP;

    static BOOL CALLBACK OctLevelSideBarProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK SetActiveModulesDialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK SideBarDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

void GetPossibleModules(StringList &modules);
BOOL GetResourceStringFromPath(CTSTR lpFilePath, CTSTR lpExpectedType, String &strResource);

extern HWND    hwndEditor;
extern HWND    hwndSidebar;
extern HWND    hwndStatusBar;
extern HFONT   hWindowFont;
extern HFONT   hCourierFont;

extern EditorEngine *editor;

#define        WM_UPDATEVIEWPORTS      WM_USER+177
#define        WM_PROCESSACCELERATORS  WM_USER+178

inline void UpdateViewports(HWND hWindow=NULL)
{
    PostMessage(hWindow ? hWindow : hwndMain, WM_UPDATEVIEWPORTS, 0, 0);
}

Vect& SnapToSpacing(Vect &v, float spacing);

void SetRadioButtonCheck(HWND hwndParent, DWORD buttonID);

void WriteTargaFile(CTSTR lpFileName, DWORD width, DWORD height, DWORD channels, LPBYTE lpData, DWORD cropX=0, DWORD cropY=0, DWORD cropCX=0, DWORD cropCY=0);

inline void Saturate(float &f) {if(f > 1.0f) f = 1.0f; else if(f < 0.0f) f = 0.0f;}

void InitSqrtTable();
float SqrtTable(float f);

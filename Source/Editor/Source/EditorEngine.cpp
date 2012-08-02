/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditorEngine.cpp

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


DefineClass(EditorEngine);


EditorEngine *editor;


LRESULT WINAPI GraphicsWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI EditorWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK OpenLevelDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EditorSettingsDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK EditBoxDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK LightmapSettingsDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


void ENGINEAPI UpdateEditMenu();

class EditorKBHandler : public KeyboardInputHandler
{
    DeclareClass(EditorKBHandler, KeyboardInputHandler);

public:
    void KeyboardHandler(unsigned int kbc, BOOL bDown);
};

DefineClass(EditorKBHandler);



void CreateToolbars(int clientX, int clientY);

extern ScriptErrorWindow *scriptErrorWindow;


HWND    hwndEditor      = NULL;
HWND    hwndSidebar     = NULL;
HWND    hwndTabControl  = NULL;
HWND    hwndStatusBar   = NULL;
HFONT   hWindowFont     = NULL;
HFONT   hCourierFont    = NULL;

int     statusBarHeight = 0;


#define IDC_MAINTOOLAR      4000
#define IDC_TABCONTROLTHING 4001
#define IDC_SUBTRACT        5001
#define IDC_ADD             5002


#define TOOLBARSIZEX        200
#define TOOLBARSIZEY        28


void EditorEngine::Init()
{
    traceIn(EditorEngine::Init);

    editor = this;
    WNDCLASS wc;

    bEditor = true;

    LOGFONT lf;
    zero(&lf, sizeof(lf));

    lf.lfHeight = -11;
    scpy(lf.lfFaceName, TEXT("Arial"));
    hWindowFont = CreateFontIndirect(&lf);

    lf.lfHeight = -11;
    scpy(lf.lfFaceName, TEXT("Courier New"));
    hCourierFont = CreateFontIndirect(&lf);

    InitColorControl();
    InitUpDownControl();
    MaterialEditor::RegisterWindowClasses();
    ShapeEditor::RegisterWindowClasses();
    ObjectPropertiesEditor::RegisterWindowClasses();
    MeshBrowser::RegisterWindowClasses();
    PrefabBrowser::RegisterWindowClasses();
    ScriptEditor::RegisterWindowClasses();

    int borderXSize = 0;
    int borderYSize = 0;

    BOOL bMaximized = AppConfig->GetInt(TEXT("Window"), TEXT("Maximized"), FALSE);

    int fullscreenX = GetSystemMetrics(SM_CXFULLSCREEN);
    int fullscreenY = GetSystemMetrics(SM_CYFULLSCREEN);

    borderXSize += GetSystemMetrics(SM_CXSIZEFRAME)*2;
    borderYSize += GetSystemMetrics(SM_CYSIZEFRAME)*2;
    borderYSize += GetSystemMetrics(SM_CYCAPTION);
    borderYSize += GetSystemMetrics(SM_CYMENU);

    //------------------------------------------------------------
    // Register Other Window Classes

    zero(&wc, sizeof(wc));
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)ObjectBrowserProc;
    wc.hInstance = hinstMain;
    wc.hIcon = LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_OBJECTBROWSERMENU);
    wc.lpszClassName = TEXT("ObjectBrowser");

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register a window class"));

    //------------------------------------------------------------
    // Create Main Window
    zero(&wc, sizeof(wc));
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)EditorWndProc;
    wc.hInstance = hinstMain;
    wc.hIcon = LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    wc.lpszClassName = ENGINE_WINDOW_CLASS_NAME;

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register a window class"));

    //int cx = 1024+borderXSize, cy = 768+borderYSize;
    int cx = AppConfig->GetInt(TEXT("Window"), TEXT("SizeX"), 1032);
    int cy = AppConfig->GetInt(TEXT("Window"), TEXT("SizeY"), 814);

    int x = (fullscreenX/2)-(cx/2);
    int y = (fullscreenY/2)-(cy/2);

    hwndEditor = CreateWindow(ENGINE_WINDOW_CLASS_NAME,
        TEXT("Xed"), WS_OVERLAPPEDWINDOW,
        x, y, cx, cy, NULL, NULL, hinstMain, NULL);

    ShowWindow(hwndEditor, bMaximized ? SW_SHOWMAXIMIZED : SW_SHOW);

    if(bMaximized)
    {
        cx = fullscreenX;
        cy = fullscreenY+TOOLBARSIZEY;
    }

    //--------------------------------------------------------
    // Create Status Window
    hwndStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD, 0, 0, 0, 0, hwndEditor, NULL, hinstMain, NULL);
    RECT statusRect;
    GetClientRect(hwndStatusBar, &statusRect);
    statusBarHeight = statusRect.bottom;

    int parts[3];

    int nWidth = statusRect.right / 2;
    for(DWORD i=0; i<2; i++)
    { 
       parts[i] = nWidth;
       nWidth += nWidth;
    }

    SendMessage(hwndStatusBar, SB_SETPARTS, 2, (LPARAM)parts);

    ShowWindow(hwndStatusBar, SW_SHOW);

    //------------------------------------------------------------
    // Create Editor Child Window
    wc.lpfnWndProc = (WNDPROC)GraphicsWndProc;
    wc.lpszMenuName = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = TEXT("XR3DClass");
    wc.cbWndExtra = sizeof(LPVOID);

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register a window class"));

    int clientX = (cx-borderXSize);
    int clientY = (cy-borderYSize);

    int d3dSizeX = clientX-TOOLBARSIZEX;
    int d3dSizeY = clientY-TOOLBARSIZEY-statusBarHeight;

    hwndMain = (HANDLE)CreateWindow(TEXT("XR3DClass"),
        NULL, WS_VISIBLE|WS_CHILD,
        0, TOOLBARSIZEY, d3dSizeX, d3dSizeY,
        hwndEditor, NULL, hinstMain, NULL);

    Super::Init();

    String errList;
    LoadGameModule(TEXT("Editor"));
    GetClass(EditorBrush)->DefineNativeVariable(offsetof(EditorBrush, bUseLightmapping), 0);
    GetClass(EditorBrush)->DefineNativeVariable(offsetof(EditorBrush, lightmapResolution), 1);

    /*PhySphere *chong  = physics->MakeSphere(100.0f);
    PhySphere *ching  = physics->MakeSphere(10.0f);
    PhyObject *bigObj = physics->CreateStaticObject(chong, Vect(0.0f, 0.0f, 0.0f), Quat::Identity());
    if(physics->GetConvexCollision(ching, Vect(0.0f, 0.0f, 0.0f), Vect(0.0f, 0.0001f, 0.0f)))
        nop();*/

    CreateToolbars(clientX, clientY);

    //--------------------------------------------------------
    // Setup graphics and viewports

    OSSetWindowSize(d3dSizeX, d3dSizeY);
    GS->SetSize(d3dSizeX, d3dSizeY);

    String lastLevel;

    //--------------------------------------------------------
    // Load last module used or bring up the editor startup dialog if none or not found, and then load the last level that module was on

    curWorkingModule = AppConfig->GetString(TEXT("Settings"), TEXT("LastModule"));
    BOOL bModLoaded = (!curWorkingModule.IsEmpty() && OSFileExists(String() << TEXT("data/") << curWorkingModule));

    if(!bModLoaded)
    {
        if(DialogBox(hinstMain, MAKEINTRESOURCE(IDD_STARTUPDIALOG), hwndEditor, (DLGPROC)EditorStartupDialog) == IDCANCEL)
            PostQuitMessage(0);
        else
            bModLoaded = TRUE;
    }

    if(bModLoaded)
    {
        String moduleInfoPath;
        moduleInfoPath << TEXT("data/") << curWorkingModule << TEXT("/Module.ini");

        ConfigFile moduleInfo;
        moduleInfo.Open(moduleInfoPath);

        StringList ExcludeStrings;
        AppConfig->GetStringList(TEXT("Engine"), TEXT("GameModule"), ExcludeStrings);

        moduleInfo.GetStringList(TEXT("Module"), TEXT("Dependancies"), DependantModules);

        for(int i=0; i<DependantModules.Num(); i++)
        {
            if(ExcludeStrings.HasValueI(DependantModules[i]))
                DependantModules.Remove(i--);
            else
                LoadGameModule(DependantModules[i]);
        }

        LoadGameModule(curWorkingModule);

        bAddonModule = moduleInfo.GetInt(TEXT("Module"), TEXT("Type"), 1);

        lastLevel = AppConfig->GetString(TEXT("Settings"), TEXT("LastLevel"));
        if(lastLevel.IsEmpty() || !editor->OpenEditableLevel(lastLevel, FALSE))
        {
            if(AppConfig->GetInt(TEXT("Settings"), TEXT("DefaultLevel"), 0) == 0)
                editor->NewIndoorLevel();
            else
                editor->NewOctLevel(); 
        }
    }

    //--------------------------------------------------------
    // Etc

    selBoxThing = new SelectionBox;

    RenderStartNew();
        Vertex(-1.0f, 0.0f, 0.0f);
        Vertex( 1.0f, 0.0f, 0.0f);
    vbGridLineV = RenderSave();

    RenderStartNew();
        Vertex(0.0f, -1.0f, 0.0f);
        Vertex(0.0f,  1.0f, 0.0f);
    vbGridLineH = RenderSave();

    VBData *vbd = new VBData;

    vbd->VertList.SetSize(30);

    vbd->VertList[0].Set(-1.5f, 0.0f, 0.0f);
    vbd->VertList[1].Set( 1.5f, 0.0f, 0.0f);

    vbd->VertList[2].Set(-0.5f, 0.5f, 0.0f);
    vbd->VertList[3].Set( 0.5f, 0.5f, 0.0f);
    vbd->VertList[4].Set(-0.5f, -0.5f, 0.0f);
    vbd->VertList[5].Set( 0.5f, -0.5f, 0.0f);
    vbd->VertList[6].Set(-0.5f, 0.0f, 0.5f);
    vbd->VertList[7].Set( 0.5f, 0.0f, 0.5f);
    vbd->VertList[8].Set(-0.5f, 0.0f, -0.5f);
    vbd->VertList[9].Set( 0.5f, 0.0f, -0.5f);

    int curPos = 0;
    for(int i=1; i<3; i++)
    {
        int nextPos = (i*10);
        for(int j=0; j<10; j++)
        {
            Vect &curVert = vbd->VertList[curPos+j];
            vbd->VertList[nextPos+j].Set(curVert.y, curVert.z, curVert.x);
        }
        curPos = nextPos;
    }

    snapGuide = CreateVertexBuffer(vbd);

    moveSpeed = 0.04f;

    hbmpFP      = LoadBitmap(hinstMain, MAKEINTRESOURCE(IDB_FP));
    hbmpHP      = LoadBitmap(hinstMain, MAKEINTRESOURCE(IDB_HP));
    hbmpVP      = LoadBitmap(hinstMain, MAKEINTRESOURCE(IDB_VP));
    hbmpSP      = LoadBitmap(hinstMain, MAKEINTRESOURCE(IDB_SP));
    hbmpSBTP    = LoadBitmap(hinstMain, MAKEINTRESOURCE(IDB_SBTP));
    hbmpSTBP    = LoadBitmap(hinstMain, MAKEINTRESOURCE(IDB_STBP));
    hbmpSLRP    = LoadBitmap(hinstMain, MAKEINTRESOURCE(IDB_SLRP));
    hbmpSRLP    = LoadBitmap(hinstMain, MAKEINTRESOURCE(IDB_SRLP));

    editorKBHandler = CreateObject(EditorKBHandler);
    PushKBHandler(editorKBHandler);

    selectedEntityClass = GetClass(Entity);

    CreateUndoRedoStacks(UpdateEditMenu, undoStack, redoStack);

    editorEffects = GS->CreateEffectFromFile(TEXT("data/Editor/effects/EditorEffectsBecauseImCheap.effect"));

    traceOut;
}

void EditorEngine::Destroy()
{
    traceIn(EditorEngine::Destroy);

    ConfigFile curModuleConfig;
    if( levelInfo && levelInfo->strLevelName.IsValid() &&
        editor->curWorkingModule.IsValid() &&
        curModuleConfig.Open(String() << TEXT("data/") << editor->curWorkingModule << TEXT("/Module.ini"), TRUE))
    {
        AppConfig->SetString(TEXT("Settings"), TEXT("LastLevel"), levelInfo->strLevelName);
    }

    AppConfig->SetString(TEXT("Settings"), TEXT("LastModule"), curWorkingModule);

    delete scriptErrorWindow;

    while(ScriptEditor::OpenScripts.Num())
        delete ScriptEditor::OpenScripts[0];
    delete meshBrowser;
    delete materialEditor;
    delete shapeEditor;

    DestroyObject(editorKBHandler);

    DeleteObject(hbmpFP);
    DeleteObject(hbmpHP);
    DeleteObject(hbmpVP);
    DeleteObject(hbmpSP);
    DeleteObject(hbmpSBTP);
    DeleteObject(hbmpSTBP);
    DeleteObject(hbmpSLRP);
    DeleteObject(hbmpSRLP);

    delete snapGuide;

    delete vbGridLineH;
    delete vbGridLineV;

    delete selBoxThing;

    //--------------------------------------------------------

    KillViewports();

    delete editorEffects;

    DestroyObject(levelInfo);

    delete undoStack;
    delete redoStack;

    Super::Destroy();

    OSDestroyMainWindow();
    DestroyWindow(hwndEditor);

    editor = NULL;

    DeleteObject(hCourierFont);
    DeleteObject(hWindowFont);

    traceOut;
}


void EditorEngine::PreFrame()
{
    //SetViewportLayout(currentLayout, &splitPos);
}


BYTE* EditorEngine::ProcessTargaData(LPBYTE lpTargaData, BOOL bInvert, BOOL bBGR)
{
    TargaHeader *th = (TargaHeader*)lpTargaData;
    Texture *texOut=NULL;
    BYTE *lpData = NULL;
    int i;

    LPBYTE lpFile = (lpTargaData+18);

    assert(!th->bColorMapped);
    assert(!th->IDSize);
    assert(!(th->depth%8));


    if(!th->bColorMapped && !th->IDSize && !(th->depth%8))
    {
        DWORD nBPS = 3;
        DWORD nFileBPS = th->depth/8;
        DWORD dwPixels = th->width*th->height;
        DWORD dwSize = dwPixels*nBPS;

        lpData = (LPBYTE)Allocate(dwSize+1);

        if(th->bUsesRLE)
        {
            struct {BYTE num:7; BYTE bUsingRLE:1;} repeat;
            DWORD count=0;
            BYTE *lpTemp = lpFile;

            do
            {
                (*(LPBYTE)&repeat) = (*(LPBYTE)lpTemp);
                ++lpTemp;

                for(i=0; i<=repeat.num; i++)
                {
                    DWORD bytePos = ((count+i)*nBPS);
                    DWORD filePos = ((count+i)*nFileBPS);

                    if(nFileBPS > 2)
                    {
                        if(bBGR)
                        {
                            lpData[bytePos+2] = *lpTemp;
                            lpData[bytePos+1] = lpTemp[1];
                            lpData[bytePos]   = lpTemp[2];
                        }
                        else
                        {
                            lpData[bytePos]   = *lpTemp;
                            lpData[bytePos+1] = lpTemp[1];
                            lpData[bytePos+2] = lpTemp[2];
                        }
                    }
                    else
                    {
                        lpData[bytePos]   = *lpTemp;
                        lpData[bytePos+1] = *lpTemp;
                        lpData[bytePos+2] = *lpTemp;
                    }
                    
                    if(!repeat.bUsingRLE)
                        lpTemp += nFileBPS;
                }

                if(repeat.bUsingRLE)
                    lpTemp += nFileBPS;

                count += repeat.num;
                ++count;
            }while(count < dwPixels);
        }
        else
        {
            for(i=0;i<dwPixels;i++)
            {
                DWORD bytePos = (i*nBPS);
                DWORD filePos = (i*nFileBPS);
                if(nFileBPS > 2)
                {
                    if(bBGR)
                    {
                        lpData[bytePos+2] = lpFile[filePos];
                        lpData[bytePos+1] = lpFile[filePos+1];
                        lpData[bytePos]   = lpFile[filePos+2];
                    }
                    else
                    {
                        lpData[bytePos]   = lpFile[filePos];
                        lpData[bytePos+1] = lpFile[filePos+1];
                        lpData[bytePos+2] = lpFile[filePos+2];
                    }
                }
                else
                {
                    lpData[bytePos]   = lpFile[filePos];
                    lpData[bytePos+1] = lpFile[filePos];
                    lpData[bytePos+2] = lpFile[filePos];
                }
            }
        }
    }

    if(bInvert)
    {
        DWORD rowLength = th->width*3;
        DWORD halfHeight = th->height/2;

        LPBYTE row = (LPBYTE)Allocate(rowLength);

        for(i=0; i<halfHeight; i++)
        {
            DWORD curPos = i*rowLength;
            DWORD invPos = (th->height-(i+1))*rowLength;

            mcpy(row, &lpData[invPos], rowLength);
            mcpy(&lpData[invPos], &lpData[curPos], rowLength);
            mcpy(&lpData[curPos], row, rowLength);
        }

        Free(row);
    }

    return lpData;
}


LRESULT WINAPI GraphicsWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(GraphicsWndProc);

    if(message == WM_PAINT)
    {
        RECT rect;
        PAINTSTRUCT ps;

        if(!editor->bRealTimeEnabled && GetUpdateRect(hwnd, &rect, FALSE))
        {
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            if(engine && !bErrorMessage)
                engine->Update(FALSE);
        }
        return 0;
    }
    else if(message == WM_UPDATEVIEWPORTS)
    {
        if(!editor->bRealTimeEnabled)
        {
            if(engine)
                engine->Update(editor->bRealTimeEnabled);
        }
        return 0;
    }

    return CallWindowProc((FARPROC)MainWndProc, hwnd, message, wParam, lParam);

    traceOut;
}


//-----------------------------------------------------
// Top Buttons
#define NUMTOPBUTTONS 5
TBBUTTON topButtons[NUMTOPBUTTONS] = 
{
    {STD_FILEOPEN, ID_FILE_OPENLEVEL, TBSTATE_ENABLED, BTNS_BUTTON, {0, 0}, NULL, NULL},
    {STD_FILESAVE, ID_FILE_SAVELEVEL, TBSTATE_ENABLED, BTNS_BUTTON, {0, 0}, NULL, NULL},
    {10, 0, 0, BTNS_SEP, {0, 0}, NULL, NULL},
    {15+0, ID_SCENE_ENABLESCENE, TBSTATE_ENABLED, BTNS_CHECK, {0, 0}, NULL, NULL},
    {15+1, ID_SCENE_HIDEEDITOROBJECTS, TBSTATE_ENABLED, BTNS_CHECK, {0, 0}, NULL, NULL},
};



TSTR topButtonDescriptions[NUMTOPBUTTONS] =
{
    TEXT("Open a level"),
    TEXT("Save level"),
    NULL,
    TEXT("Enable real-time scene processing"),
    TEXT("Hide Editor Objects - View the scene in it's natural form")
};


void CreateToolbars(int clientX, int clientY)
{
    traceIn(EditorEngine::CreateToolbars);

    HWND newToolbar;
    TBADDBITMAP tbab;

    newToolbar = CreateWindow(TOOLBARCLASSNAME,
        NULL,
        WS_VISIBLE|WS_CHILD|TBSTYLE_WRAPABLE|TBSTYLE_FLAT,
        0, 0, clientX, TOOLBARSIZEY,
        hwndEditor,
        (HMENU)IDC_MAINTOOLAR,
        hinstMain,
        NULL);

    tbab.hInst = HINST_COMMCTRL;
    tbab.nID = IDB_STD_SMALL_COLOR;
    SendMessage(newToolbar, TB_ADDBITMAP, 0, (LPARAM)&tbab);

    tbab.hInst = hinstMain;
    tbab.nID = IDB_TOOLBAR_REALTIME;
    SendMessage(newToolbar, TB_ADDBITMAP, 1, (LPARAM)&tbab);

    tbab.hInst = hinstMain;
    tbab.nID = IDB_TOOLBAR_HIDEEDITOROBJS;
    SendMessage(newToolbar, TB_ADDBITMAP, 1, (LPARAM)&tbab);

    SendMessage(newToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SendMessage(newToolbar, TB_ADDBUTTONS, NUMTOPBUTTONS, (LPARAM)topButtons);

    traceOut;
}


void EditorEngine::CreateIndoorSidebar()
{
    traceIn(EditorEngine::CreateIndoorSidebar);

    RECT editorRect;
    GetClientRect(hwndEditor, &editorRect);

    if(hwndTabControl)
        DestroyWindow(hwndTabControl);
    if(hwndSidebar)
        DestroyWindow(hwndSidebar);
    hwndSidebar = CreateDialog(hinstMain, MAKEINTRESOURCE(IDD_INDOORSIDEBAR), hwndEditor, (DLGPROC)EditorEngine::SideBarDlgProc);
    SetWindowPos(hwndSidebar, NULL, editorRect.right-TOOLBARSIZEX, TOOLBARSIZEY, TOOLBARSIZEX, editorRect.bottom-TOOLBARSIZEY-statusBarHeight, 0);

    ShowWindow(hwndSidebar, SW_SHOW);
    //ShowWindow(hwndTabControl, SW_SHOW);

    traceOut;
}


void EditorEngine::CreateOctLevelSidebar(DWORD dwTab, bool bSwitching)
{
    traceIn(EditorEngine::CreateOctLevelSidebar);

    RECT editorRect;
    GetClientRect(hwndEditor, &editorRect);

    if(hwndTabControl && !bSwitching)
        DestroyWindow(hwndTabControl);
    if(hwndSidebar)
        DestroyWindow(hwndSidebar);

    /*if(!bSwitching)
    {
        hwndTabControl = CreateWindow(WC_TABCONTROL, TEXT(""), WS_CHILD|WS_CLIPSIBLINGS,
            editorRect.right-TOOLBARSIZEX, TOOLBARSIZEY, TOOLBARSIZEX, TOOLBARSIZEY+20,
            hwndEditor, (HMENU)IDC_TABCONTROLTHING, hinstMain, NULL);
        SendMessage(hwndTabControl, WM_SETFONT, (WPARAM)hWindowFont, (LPARAM)FALSE);

        TCITEM ibching;

        ibching.mask = TCIF_TEXT;
        ibching.pszText = TEXT("Main");
        TabCtrl_InsertItem(hwndTabControl, 0, &ibching);
        ibching.pszText = TEXT("Prefabs");
        TabCtrl_InsertItem(hwndTabControl, 1, &ibching);
    }*/

    if(levelInfo)
        levelInfo->curSidebarTab = dwTab;

    if(dwTab == 0)
    {
        hwndSidebar = CreateDialog(hinstMain, MAKEINTRESOURCE(IDD_OCTLEVELSIDEBAR), hwndEditor, (DLGPROC)EditorEngine::OctLevelSideBarProc);
        //SetWindowPos(hwndSidebar, NULL, editorRect.right-TOOLBARSIZEX, TOOLBARSIZEY+20, TOOLBARSIZEX, editorRect.bottom-TOOLBARSIZEY-statusBarHeight, 0);
        SetWindowPos(hwndSidebar, NULL, editorRect.right-TOOLBARSIZEX, TOOLBARSIZEY, TOOLBARSIZEX, editorRect.bottom-TOOLBARSIZEY-statusBarHeight, 0);
    }
    else if(dwTab == 1)
    {
        hwndSidebar = CreateDialog(hinstMain, MAKEINTRESOURCE(IDD_PREFABSIDEBAR), hwndEditor, (DLGPROC)EditorEngine::OctLevelSideBarProc);
        SetWindowPos(hwndSidebar, NULL, editorRect.right-TOOLBARSIZEX, TOOLBARSIZEY+20, TOOLBARSIZEX, editorRect.bottom-TOOLBARSIZEY-statusBarHeight, 0);
    }

    ShowWindow(hwndSidebar, SW_SHOW);
    //ShowWindow(hwndTabControl, SW_SHOW);

    traceOut;
}


void EditorEngine::CreateOutdoorSidebar()
{
    traceIn(EditorEngine::CreateOutdoorSidebar);

    RECT editorRect;
    GetClientRect(hwndEditor, &editorRect);

    if(hwndTabControl)
        DestroyWindow(hwndTabControl);
    if(hwndSidebar)
        DestroyWindow(hwndSidebar);
    hwndSidebar = CreateDialog(hinstMain, MAKEINTRESOURCE(IDD_OUTDOORSIDEBAR), hwndEditor, (DLGPROC)EditorEngine::SideBarDlgProc);
    SetWindowPos(hwndSidebar, NULL, editorRect.right-TOOLBARSIZEX, TOOLBARSIZEY, TOOLBARSIZEX, editorRect.bottom-TOOLBARSIZEY-statusBarHeight, 0);

    ShowWindow(hwndSidebar, SW_SHOW);
    //ShowWindow(hwndTabControl, SW_SHOW);

    traceOut;
}



LRESULT WINAPI EditorWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(EditorWndProc);

    switch(message)
    {
        case WM_ACTIVATEAPP:
            SendMessage(hwndMain, WM_ACTIVATE, wParam ? WA_ACTIVE : WA_INACTIVE , 0);
            return 0;

        case WM_SETFOCUS:
            SetFocus(hwndMain);
            break;

        case WM_NOTIFY:
            {
                NMHDR *pNotify = (NMHDR*)lParam;

                if(pNotify->idFrom == IDC_MAINTOOLAR)
                {
                    if(pNotify->code == TBN_HOTITEMCHANGE)
                    {
                        NMTBHOTITEM *pItem = (NMTBHOTITEM*)pNotify;

                        if(pItem->dwFlags & HICF_LEAVING)
                            editor->SetStatusText(0, NULL);
                        else
                        {
							int i;

                            for(i=0; i<NUMTOPBUTTONS; i++)
                            {
                                if(topButtons[i].idCommand == pItem->idNew)
                                    break;
                            }
                            if(i != NUMTOPBUTTONS)
                                editor->SetStatusText(0, topButtonDescriptions[i]);
                        }
                    }
                }
                else if(pNotify->idFrom == IDC_TABCONTROLTHING)
                {
                    if(pNotify->code == TCN_SELCHANGE)
                    {
                        int curSel = TabCtrl_GetCurSel(pNotify->hwndFrom);

                        editor->CreateOctLevelSidebar(curSel, true);
                    }
                }

                break;
            }

        case WM_PROCESSACCELERATORS:
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_SCENE_SELECT:
                    levelInfo->SetCurModifyMode(ModifyMode_Select);
                    break;

                case ID_SCENE_MOVE:
                    levelInfo->SetCurModifyMode(ModifyMode_Move);
                    break;

                case ID_SCENE_ROTATE:
                    levelInfo->SetCurModifyMode(ModifyMode_Rotate);
                    break;

                case ID_SCENE_SELECTOBJECTS:
                    levelInfo->SetCurSelectMode(SelectMode_ObjectsAndDetails);
                    break;

                case ID_SCENE_SELECTPREFABS:
                    levelInfo->SetCurSelectMode(SelectMode_WorldPrefabs);
                    break;

                case ID_SCENE_SELECTBRUSHES:
                    levelInfo->SetCurSelectMode(SelectMode_Brushes);
                    break;

                case ID_SCENE_SELECTTEXTURES:
                    levelInfo->SetCurSelectMode(SelectMode_Textures);
                    break;


                case ID_LEVEL_REBUILDBRUSHES:
                    levelInfo->RebuildScene();
                    break;
                case ID_LEVEL_REBUILDADDITIONBRUSHES:
                    levelInfo->RebuildAdditions();
                    break;
                case ID_LEVEL_REBUILDSUBTRACTIONBRUSHES:
                    levelInfo->RebuildSubtractions();
                    break;


                case ID_SCENE_ENABLESCENE:
                    editor->EnableRealTimeRendering(!editor->bRealTimeEnabled);
                    break;

                case ID_SCENE_HIDEEDITOROBJECTS:
                    editor->HideEditorObjects(!editor->bHideEditorObjects);
                    break;

                case ID_FILE_NEWMODULE:
                    DialogBox(hinstMain, MAKEINTRESOURCE(IDD_CREATEMODULE), hwndEditor, (DLGPROC)CreateModuleDialog);
                    break;

                case ID_FILE_SETACTIVEMODULES:
                    DialogBox(hinstMain, MAKEINTRESOURCE(IDD_LOADMODULES), hwndEditor, (DLGPROC)EditorEngine::SetActiveModulesDialog);
                    break;

                case ID_FILE_NEWINDOORLEVEL:
                    editor->NewIndoorLevel();
                    break;

                case ID_FILE_NEWOCTLEVEL:
                    editor->NewOctLevel();
                    break;

                case ID_FILE_NEWOUTDOORLEVEL:
                    editor->NewOutdoorLevel();
                    break;

                case ID_FILE_OPENLEVEL:
                    if(levelInfo->bModified)
                    {
                        switch(MessageBox(hwndEditor, TEXT("You have unsaved changes.\r\n\r\nWould you like to save your changes?"), TEXT("Save?"), MB_YESNOCANCEL))
                        {
                            case IDYES:
                                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_FILE_SAVELEVEL, 0), NULL);
                            case IDNO:
                                //DialogBox(hinstMain, MAKEINTRESOURCE(IDD_OPENLEVEL), hwndMain, (DLGPROC)OpenLevelDialogProc);
                                editor->OpenLevel();
                            default:
                                break;
                        }
                    }
                    else
                        //DialogBox(hinstMain, MAKEINTRESOURCE(IDD_OPENLEVEL), hwndMain, (DLGPROC)OpenLevelDialogProc);
                        editor->OpenLevel();
                    break;

                case ID_FILE_SAVELEVEL:
                    if(levelInfo->strLevelName.IsEmpty())
                        editor->SaveLevelAs();
                    else
                    {
                        if(level->IsOf(GetClass(IndoorLevel)))
                            editor->SaveIndoorLevel();
                        else if(level->IsOf(GetClass(OutdoorLevel)))
                            editor->SaveOutdoorLevel();
                        else if(level->IsOf(GetClass(OctLevel)))
                            editor->SaveOctLevel();
                    }
                    break;

                case ID_FILE_SAVELEVELAS:
                    editor->SaveLevelAs();
                    break;

                case ID_FILE_SETTINGS:
                    DialogBox(hinstMain, MAKEINTRESOURCE(IDD_EDITORSETTINGS), hwndEditor, (DLGPROC)EditorSettingsDialogProc);
                    break;

                case ID_FILE_EXIT:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;

                case ID_MYEDIT_UNDO:
                    if(editor->undoStack->IsValid())
                    {
                        editor->undoStack->Pop();
                        UpdateViewports();
                    }
                    break;

                case ID_MYEDIT_REDO:
                    if(editor->redoStack->IsValid())
                    {
                        editor->redoStack->Pop();
                        UpdateViewports();
                    }
                    break;

                case ID_MYEDIT_DELETE:
                    if(levelInfo)
                        levelInfo->DeleteSelectedObjects();
                    break;

                case ID_VIEW_VIEWPORTLAYOUT:
                    editor->ConfigureViewports();
                    break;

                case ID_SCENE_MATERIALS:
                    if(!materialEditor)
                        new MaterialEditor;
                    else
                        ShowWindow(materialEditor->hwndMaterialEditor, SW_RESTORE);
                    break;

                case ID_SCENE_MESHBROWSER:
                    if(!meshBrowser)
                        new MeshBrowser;
                    else
                        ShowWindow(meshBrowser->hwndMeshBrowser, SW_RESTORE);
                    break;

                case ID_SCENE_PREFABBROWSER:
                    if(!prefabBrowser)
                        new PrefabBrowser;
                    else
                        ShowWindow(prefabBrowser->hwndPrefabBrowser, SW_RESTORE);
                    break;

                case ID_SCENE_OBJECTPROPERTIES:
                    if(!objectProperties)
                        new ObjectPropertiesEditor;
                    else
                        ShowWindow(objectProperties->hwndObjectProperties, SW_RESTORE);
                    break;

                case ID_SCENE_SURFACEPROPERTIES:
                    if(!surfaceProperties)
                        new SurfacePropertiesEditor;
                    else
                        ShowWindow(surfaceProperties->hwndSurfaceProperties, SW_RESTORE);
                    break;

                case ID_SCENE_BASICTEXTUREALIGNMENT:
                    if(!editor->hwndUVEdit)
                        editor->hwndUVEdit = CreateDialog(hinstMain, MAKEINTRESOURCE(IDD_UVEDIT), hwndMain, (DLGPROC)TextureAdjustProc);
                    else
                        ShowWindow(editor->hwndUVEdit, SW_RESTORE);
                    break;

                case ID_SCENE_OBJECTBROWSER:
                    if(!editor->hwndObjectBrowser)
                        CreateObjectBrowser();
                    else
                        ShowWindow(editor->hwndObjectBrowser, SW_RESTORE);
                    break;

                case ID_SCENE_SHAPEEDITOR:
                    if(!shapeEditor)
                        new ShapeEditor;
                    else
                        ShowWindow(shapeEditor->hwndShapeEditor, SW_RESTORE);
                    break;

                case ID_BUILD_REBUILDSCENE:
                    levelInfo->RebuildScene();
                    break;
            }
            break;

        case WM_CONTEXTMENU:
            return TRUE;

        case WM_SIZE:
            {
                if(wParam != SIZE_MINIMIZED)
                {
                    int x = LOWORD(lParam);
                    int y = HIWORD(lParam);

                    if(GS)
                    {
                        Vect2 oldSize = GS->GetSize();
                        GS->SetSize(x-TOOLBARSIZEX, y-TOOLBARSIZEY-statusBarHeight);

                        Vect2 offsetSize = GS->GetSize()/oldSize;
                        editor->splitPos *= offsetSize;
                        editor->splitPos.x = floor(editor->splitPos.x+0.5f);
                        editor->splitPos.y = floor(editor->splitPos.y+0.5f);

                        editor->SetViewportLayout(editor->currentLayout, &editor->splitPos);
                    }

                    SendMessage(hwndStatusBar, WM_SIZE, SIZE_RESTORED, 0);

                    int parts[3];

                    int nWidth = x / 2;
                    for(DWORD i=0; i<2; i++)
                    { 
                        parts[i] = nWidth;
                        nWidth += nWidth;
                    }

                    SendMessage(hwndStatusBar, SB_SETPARTS, 2, (LPARAM)parts);

                    OSSetWindowSize(x-TOOLBARSIZEX, y-TOOLBARSIZEY-statusBarHeight);

                    SetWindowPos(GetDlgItem(hwnd, IDC_MAINTOOLAR), NULL, 0, 0, x, TOOLBARSIZEY, SWP_NOMOVE);
                    SetWindowPos(hwndSidebar, NULL,
                        x-TOOLBARSIZEX, TOOLBARSIZEY,
                        TOOLBARSIZEX, y-TOOLBARSIZEY-statusBarHeight, 0);

                    ShowWindow(hwndMain, SW_RESTORE);

                    UpdateViewports();
                }
                else
                    ShowWindow(hwndMain, SW_MINIMIZE);

                return TRUE;
            }

        case WM_SIZING:
            {
                int borderXSize = 400;
                int borderYSize = 300;

                borderXSize += GetSystemMetrics(SM_CXSIZEFRAME)*2;
                borderYSize += GetSystemMetrics(SM_CYSIZEFRAME)*2;
                borderYSize += GetSystemMetrics(SM_CYCAPTION);
                borderYSize += GetSystemMetrics(SM_CYMENU);

                RECT *pRect = (RECT*)lParam;

                if((pRect->right - pRect->left) <= borderXSize)
                    pRect->right = pRect->left + borderXSize;

                if((pRect->bottom - pRect->top) <= borderYSize)
                    pRect->bottom = pRect->top + borderYSize;

                return TRUE;
            }

        case WM_CLOSE:
            if(levelInfo->bModified)
            {
                switch(MessageBox(hwndEditor, TEXT("You have unsaved changes.\r\n\r\nWould you like to save your changes?"), TEXT("Save?"), MB_YESNOCANCEL))
                {
                    case IDYES:
                        SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_FILE_SAVELEVEL, 0), NULL);
                        break;

                    case IDCANCEL:
                        return FALSE;
                }
            }

            if(meshBrowser)
                delete meshBrowser;

            if(materialEditor)
                delete materialEditor;

            if(surfaceProperties)
                delete surfaceProperties;

            if(objectProperties)
                delete objectProperties;

            PostQuitMessage(0);
            break;

        case WM_DESTROY:
            {
                WINDOWPLACEMENT wp;
                wp.length = sizeof(WINDOWPLACEMENT);
                GetWindowPlacement(hwndEditor, &wp);

                AppConfig->SetInt(TEXT("Window"), TEXT("SizeX"), wp.rcNormalPosition.right-wp.rcNormalPosition.left);
                AppConfig->SetInt(TEXT("Window"), TEXT("SizeY"), wp.rcNormalPosition.bottom-wp.rcNormalPosition.top);
                AppConfig->SetInt(TEXT("Window"), TEXT("Maximized"), IsZoomed(hwndEditor));
                break;
            }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);

    traceOut;
}


BOOL CALLBACK EditorEngine::SideBarDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(EditorEngine::SideBarDlgProc);

    switch(message)
    {
        case WM_INITDIALOG:
            {
                HWND hwndControl;

                hwndControl = GetDlgItem(hwnd, ID_MODE_SELECT);
                SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);

                hwndControl = GetDlgItem(hwnd, ID_GRID_SNAP);
                SendMessage(hwndControl, BM_SETCHECK, levelInfo->bSnapToGrid ? BST_CHECKED : BST_UNCHECKED, 0);

                hwndControl = GetDlgItem(hwnd, IDC_PREFABLIST);
                if(hwndControl)
                {
                    LVCOLUMN lvc;
                    zero(&lvc, sizeof(lvc));
                    lvc.mask = LVCF_TEXT | LVCF_WIDTH;

                    lvc.pszText = TEXT("Filters");
                    lvc.cx = 120;
                    SendMessage(hwndControl, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

                    lvc.pszText = TEXT("Name");
                    lvc.cx = 120;
                    SendMessage(hwndControl, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
                }
                break;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_MODIFY_SELECT:
                    levelInfo->SetCurModifyMode(ModifyMode_Select);
                    break;

                case ID_MODIFY_MOVE:
                    levelInfo->SetCurModifyMode(ModifyMode_Move);
                    break;

                case ID_MODIFY_ROTATE:
                    levelInfo->SetCurModifyMode(ModifyMode_Rotate);
                    break;

                /*case ID_TERRAIN_EDITOR:
                    if(levelInfo->newObject)
                    {
                        delete levelInfo->newObject;
                        levelInfo->newObject = NULL;
                    }
                    levelInfo->curEditMode = EditMode_Terrain; break;*/

                case ID_CREATE_BOX:
                    if(levelInfo->newObject)
                        delete levelInfo->newObject;
                    levelInfo->newObject = NULL;

                    levelInfo->SetCurEditMode(EditMode_Create);
                    levelInfo->newObject = CreateBare(BoxPrimitive);
                    break;

                case ID_BRUSH_IMPORT:
                    if(levelInfo->curEditMode != EditMode_Modify)
                        SendMessage(GetDlgItem(hwnd, ID_MODIFY_SELECT), BM_CLICK, 0, 0);
                    levelInfo->ImportBrush();
                    break;

                case ID_BRUSH_SUBTRACT:
                    if(levelInfo->WorkBrush)
                        levelInfo->WorkBrush->SubtractBrush();
                    break;

                case ID_BRUSH_ADD_GEOMETRY:
                    if(levelInfo->WorkBrush)
                        levelInfo->WorkBrush->AddGeometry();
                    break;

                case ID_BRUSH_SUBTRACT_GEOMETRY:
                    if(levelInfo->WorkBrush)
                        levelInfo->WorkBrush->SubtractGeometry();
                    break;

                case ID_GRID_SNAP:
                    levelInfo->bSnapToGrid = !levelInfo->bSnapToGrid; break;

                case ID_GRID_INCREASE:
                    if(levelInfo->gridSpacing > 0.5f)
                        levelInfo->gridSpacing *= 0.5f;
                    UpdateViewports();
                    break;

                case ID_GRID_DECREASE:
                    if(levelInfo->gridSpacing < 50.0f)
                        levelInfo->gridSpacing *= 2.0f;
                    UpdateViewports();
                    break;

                case ID_LIGHTMAPS_REMOVE:
                    {
                        if(MessageBox(hwndEditor, TEXT("Are you sure you want to remove all lightmaps?"), TEXT("Lightmap Removal"), MB_YESNO) == IDNO)
                            break;

                        for(int i=0; i<Light::LightList.Num(); i++)
                        {
                            Light *light = Light::LightList[i];

                            if(light->bLightmapped)
                            {
                                light->bLightmapped = FALSE;
                                light->UpdatePositionalData();
                                light->Reinitialize();
                            }
                        }

                        for(int i=0; i<levelInfo->BrushList.Num(); i++)
                        {
                            Brush *levelBrush = levelInfo->BrushList[i]->GetLevelBrush();
                            levelBrush->bLightmapped = FALSE;

                            levelBrush->lightmap.FreeData();
                        }

                        Entity *ent = Entity::FirstEntity();
                        while(ent)
                        {
                            MeshEntity *meshEnt = ObjectCast<MeshEntity>(ent);

                            if(meshEnt)
                            {
                                meshEnt->bLightmapped = FALSE;
                                meshEnt->lightmap.FreeData();
                            }

                            ent = ent->NextEntity();
                        }

                        levelInfo->bModified = true;
                        level->bHasLightmaps = FALSE;

                        break;
                    }
            }
    }

    return FALSE;

    traceOut;
}


BOOL CALLBACK EditorEngine::OctLevelSideBarProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(EditorEngine::OctLevelSideBarProc);

    switch(message)
    {
        case WM_INITDIALOG:
            {
                OctLevel *octLevel = (OctLevel*)level;

                HWND hwndControl;

                hwndControl = GetDlgItem(hwnd, ID_MODE_SELECT);
                SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);

                hwndControl = GetDlgItem(hwnd, ID_SELECTMODE_OBJECTS);
                SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);

                hwndControl = GetDlgItem(hwnd, ID_GRID_SNAP);
                SendMessage(hwndControl, BM_SETCHECK, levelInfo->bSnapToGrid ? BST_CHECKED : BST_UNCHECKED, 0);

                hwndControl = GetDlgItem(hwnd, IDC_PREFABLIST);
                if(hwndControl)
                {
                    LVCOLUMN lvc;
                    zero(&lvc, sizeof(lvc));
                    lvc.mask = LVCF_TEXT | LVCF_WIDTH;

                    lvc.pszText = TEXT("Filters");
                    lvc.cx = 120;
                    SendMessage(hwndControl, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

                    lvc.pszText = TEXT("Name");
                    lvc.cx = 120;
                    SendMessage(hwndControl, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
                }

                break;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_MODIFY_SELECT:
                    levelInfo->SetCurModifyMode(ModifyMode_Select);
                    break;

                case ID_MODIFY_MOVE:
                    levelInfo->SetCurModifyMode(ModifyMode_Move);
                    break;

                case ID_MODIFY_ROTATE:
                    levelInfo->SetCurModifyMode(ModifyMode_Rotate);
                    break;

                case ID_SELECTMODE_OBJECTS:
                    levelInfo->SetCurSelectMode(SelectMode_ObjectsAndDetails);
                    break;

                case ID_SELECTMODE_WORLD:
                    levelInfo->SetCurSelectMode(SelectMode_WorldPrefabs);
                    break;

                case ID_SELECTMODE_BRUSHES:
                    levelInfo->SetCurSelectMode(SelectMode_Brushes);
                    break;

                case ID_SELECTMODE_TEXTURES:
                    levelInfo->SetCurSelectMode(SelectMode_Textures);
                    break;

                case ID_GRID_ADJUSTY:
                    if(levelInfo->newObject)
                        delete levelInfo->newObject;
                    levelInfo->newObject = CreateBare(YPlaneAdjuster);
                    levelInfo->SetCurEditMode(EditMode_Create);
                    break;

                case ID_GRID_RESETY:
                    if(!CloseFloat(levelInfo->curYPlanePosition, 0.0f, 0.1f))
                    {
                        EditorLevelInfo::SaveModifyYPlaneUndoData(TEXT("Change Grid Position"), levelInfo->curYPlanePosition);

                        levelInfo->curYPlanePosition = 0.0f;
                        if(levelInfo->newObject && levelInfo->newObject->IsOf(GetClass(YPlaneAdjuster)))
                        {
                            YPlaneAdjuster *adjuster = (YPlaneAdjuster*)levelInfo->newObject;
                            adjuster->realPos = adjuster->curPos = adjuster->lastPos = levelInfo->curYPlanePosition;
                        }

                        UpdateViewports();
                    }
                    break;

                case ID_BRUSH_IMPORT:
                    if(levelInfo->curEditMode != EditMode_Modify)
                        SendMessage(GetDlgItem(hwnd, ID_MODIFY_SELECT), BM_CLICK, 0, 0);
                    levelInfo->ImportBrush();
                    break;

                case ID_BRUSH_FLOOR:
                    if(levelInfo->newObject)
                        delete levelInfo->newObject;
                    levelInfo->newObject = CreateBare(FloorPrimitive);
                    levelInfo->SetCurEditMode(EditMode_Create);
                    break;

                case ID_BRUSH_PLANE:
                    if(levelInfo->newObject)
                        delete levelInfo->newObject;
                    levelInfo->newObject = CreateBare(PlanePrimitive);
                    levelInfo->SetCurEditMode(EditMode_Create);
                    break;


                case ID_PLACEPREFAB_DETAIL:
                    if(levelInfo->newObject)
                        delete levelInfo->newObject;
                    levelInfo->newObject = CreateObjectParam2(PrefabPlacer, levelInfo->curEditMode, PrefabDetail);
                    levelInfo->SetCurEditMode(EditMode_Create);
                    break;

                case ID_PLACEPREFAB_ALIGNED:
                    if(levelInfo->newObject)
                        delete levelInfo->newObject;
                    levelInfo->newObject = CreateObjectParam2(PrefabPlacer, levelInfo->curEditMode, PrefabYAlign);
                    levelInfo->SetCurEditMode(EditMode_Create);
                    break;

                case ID_PLACEPREFAB_FLOOR:
                    if(levelInfo->newObject)
                        delete levelInfo->newObject;
                    levelInfo->newObject = CreateObjectParam2(PrefabPlacer, levelInfo->curEditMode, PrefabFloor);
                    levelInfo->SetCurEditMode(EditMode_Create);
                    break;

                case ID_PLACEOBJECT_DETAIL:
                    if(levelInfo->newObject)
                        delete levelInfo->newObject;
                    levelInfo->newObject = CreateObjectParam2(EntityPlacer, levelInfo->curEditMode, EntityDetail);
                    levelInfo->SetCurEditMode(EditMode_Create);
                    break;

                case ID_PLACEOBJECT_ALIGNED:
                    if(levelInfo->newObject)
                        delete levelInfo->newObject;
                    levelInfo->newObject = CreateObjectParam2(EntityPlacer, levelInfo->curEditMode, EntityYAlign);
                    levelInfo->SetCurEditMode(EditMode_Create);
                    break;


                case ID_BRUSH_ADD_GEOMETRY:
                    if(levelInfo->WorkBrush)
                        levelInfo->WorkBrush->AddGeometry();
                    break;

                case ID_BRUSH_SUBTRACT_GEOMETRY:
                    if(levelInfo->WorkBrush)
                        levelInfo->WorkBrush->SubtractGeometry();
                    break;

                case ID_BRUSH_ADD_SEGMENTED:
                    if(levelInfo->WorkBrush)
                        levelInfo->WorkBrush->AddSegmentedGeometry();
                    break;

                case ID_GRID_SNAP:
                    levelInfo->bSnapToGrid = !levelInfo->bSnapToGrid; break;

                case ID_GRID_INCREASE:
                    if(levelInfo->gridSpacing > 0.5f)
                        levelInfo->gridSpacing *= 0.5f;
                    UpdateViewports();
                    break;

                case ID_GRID_DECREASE:
                    if(levelInfo->gridSpacing < 50.0f)
                        levelInfo->gridSpacing *= 2.0f;
                    UpdateViewports();
                    break;

                case ID_GRID_CUSTOMGRID:
                    break;

                case ID_GRID_DEFAULTGRID:
                    levelInfo->gridSpacing = 1.0f;
                    UpdateViewports();
                    break;

                case ID_LIGHTMAPS_BUILD:
                    levelInfo->BuildLightmaps(levelInfo->lightmapSettings);
                    break;

                case ID_LIGHTMAPS_DRAFT:
                    {
                        LightmapSettings settings;
                        settings.bUseDXT1 = TRUE;
                        levelInfo->BuildLightmaps(settings);
                        break;
                    }

                case ID_LIGHTMAPS_SETTINGS:
                    DialogBox(hinstMain, MAKEINTRESOURCE(IDD_LIGHTMAPSETTINGS), hwndEditor, (DLGPROC)LightmapSettingsDialogProc);
                    break;

                case ID_LIGHTMAPS_REMOVE:
                    {
                        if(MessageBox(hwndEditor, TEXT("Are you sure you want to remove all lightmaps?"), TEXT("Lightmap Removal"), MB_YESNO) == IDNO)
                            break;

                        for(DWORD i=0; i<Light::LightList.Num(); i++)
                        {
                            Light *light = Light::LightList[i];

                            if(light->bLightmapped)
                            {
                                light->bLightmapped = FALSE;
                                light->UpdatePositionalData();
                                light->Reinitialize();
                            }
                        }

                        for(int i=0; i<levelInfo->BrushList.Num(); i++)
                        {
                            Brush *levelBrush = levelInfo->BrushList[i]->GetLevelBrush();
                            levelBrush->bLightmapped = FALSE;

                            levelBrush->lightmap.FreeData();
                        }

                        Entity *ent = Entity::FirstEntity();
                        while(ent)
                        {
                            MeshEntity *meshEnt = ObjectCast<MeshEntity>(ent);

                            if(meshEnt)
                            {
                                meshEnt->bLightmapped = FALSE;
                                meshEnt->lightmap.FreeData();
                            }

                            ent = ent->NextEntity();
                        }

                        levelInfo->bModified = true;
                        level->bHasLightmaps = FALSE;

                        UpdateViewports();
                        break;
                    }
            }
    }

    return FALSE;

    traceOut;
}


BOOL CALLBACK EditorSettingsDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(EditorEngine::EditorSettingsDialogProc);

    switch(message)
    {
        case WM_INITDIALOG:
            {
                float fGamma        = AppConfig->GetFloat(TEXT("Display"), TEXT("Gamma"), 1.0f);
                float fBrightness   = AppConfig->GetFloat(TEXT("Display"), TEXT("Brightness"), 1.0f);
                float fContrast     = AppConfig->GetFloat(TEXT("Display"), TEXT("Contrast"), 1.0f);

                InitUpDownFloatData(GetDlgItem(hwnd, IDC_GAMMASCROLL), fGamma, 0.5f, 2.0f, 0.01f);
                LinkUpDown(GetDlgItem(hwnd, IDC_GAMMASCROLL), GetDlgItem(hwnd, IDC_GAMMAEDIT));

                InitUpDownFloatData(GetDlgItem(hwnd, IDC_BRIGHTNESSSCROLL), fBrightness, 0.75f, 1.75f, 0.01f);
                LinkUpDown(GetDlgItem(hwnd, IDC_BRIGHTNESSSCROLL), GetDlgItem(hwnd, IDC_BRIGHTNESSEDIT));

                InitUpDownFloatData(GetDlgItem(hwnd, IDC_CONTRASTSCROLL), fContrast, 0.75f, 1.75f, 0.01f);
                LinkUpDown(GetDlgItem(hwnd, IDC_CONTRASTSCROLL), GetDlgItem(hwnd, IDC_CONTRASTEDIT));

                int defLevel = AppConfig->GetInt(TEXT("Settings"), TEXT("DefaultLevel"), 0);

                if((defLevel < 0) || (defLevel > 1))
                    defLevel = 0;

                if(defLevel == 0)
                    SendMessage(GetDlgItem(hwnd, IDC_DEFAULT_INDOOR), BM_SETCHECK, BST_CHECKED, 0);
                else if(defLevel == 1)
                    SendMessage(GetDlgItem(hwnd, IDC_DEFAULT_OCT), BM_SETCHECK, BST_CHECKED, 0);
                //else if(defLevel == 2)
                //    SendMessage(GetDlgItem(hwnd, IDC_DEFAULT_INDOOR), BM_SETCHECK, BST_CHECKED, 0);

                break;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_GAMMASCROLL:
                case IDC_BRIGHTNESSSCROLL:
                case IDC_CONTRASTSCROLL:
                    OSColorAdjust(GetUpDownFloat(GetDlgItem(hwnd, IDC_GAMMASCROLL)),
                                  GetUpDownFloat(GetDlgItem(hwnd, IDC_BRIGHTNESSSCROLL)),
                                  GetUpDownFloat(GetDlgItem(hwnd, IDC_CONTRASTSCROLL)));
                    break;

                case IDOK:
                    AppConfig->SetFloat(TEXT("Display"), TEXT("Gamma"), GetUpDownFloat(GetDlgItem(hwnd, IDC_GAMMASCROLL)));
                    AppConfig->SetFloat(TEXT("Display"), TEXT("Brightness"), GetUpDownFloat(GetDlgItem(hwnd, IDC_BRIGHTNESSSCROLL)));
                    AppConfig->SetFloat(TEXT("Display"), TEXT("Contrast"), GetUpDownFloat(GetDlgItem(hwnd, IDC_CONTRASTSCROLL)));
                    if(SendMessage(GetDlgItem(hwnd, IDC_DEFAULT_INDOOR), BM_GETCHECK, 0, 0) == BST_CHECKED)
                        AppConfig->SetInt(TEXT("Settings"), TEXT("DefaultLevel"), 0);
                    else if(SendMessage(GetDlgItem(hwnd, IDC_DEFAULT_OCT), BM_GETCHECK, 0, 0) == BST_CHECKED)
                        AppConfig->SetInt(TEXT("Settings"), TEXT("DefaultLevel"), 1);
                    EndDialog(hwnd, IDOK);
                    break;

                case IDCANCEL:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
            }
            break;

        case WM_CLOSE:
            {
                float fGamma        = AppConfig->GetFloat(TEXT("Display"), TEXT("Gamma"), 1.0f);
                float fBrightness   = AppConfig->GetFloat(TEXT("Display"), TEXT("Brightness"), 1.0f);
                float fContrast     = AppConfig->GetFloat(TEXT("Display"), TEXT("Contrast"), 1.0f);

                OSColorAdjust(fGamma, fBrightness, fContrast);
                EndDialog(hwnd, IDCANCEL);
                break;
            }
    }

    return FALSE;

    traceOut;
}


BOOL CALLBACK OpenLevelDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(EditorEngine::OpenLevelDialogProc);

    switch(message)
    {
        case WM_INITDIALOG:
            {
                OSFindData findData;

                HANDLE hFind = OSFindFirstFile(TEXT("data/levels/*.xlv"), findData);

                if(hFind)
                {
                    do
                    {
                        findData.fileName[slen(findData.fileName)-4] = 0;
                        SendMessage(GetDlgItem(hwnd, IDC_LEVELLIST), LB_ADDSTRING, 0, (LPARAM)findData.fileName);
                    }while(OSFindNextFile(hFind, findData));

                    OSFindClose(hFind);

                    SendMessage(GetDlgItem(hwnd, IDC_LEVELLIST), LB_SETCURSEL, 0, 0);
                }
                break;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_LEVELLIST:
                    if(HIWORD(wParam) == LBN_DBLCLK)
                        PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), lParam);
                    break;

                case IDOK:
                    {
                        DWORD dwCurSel = SendMessage(GetDlgItem(hwnd, IDC_LEVELLIST), LB_GETCURSEL, 0, 0);

                        if(dwCurSel != LB_ERR)
                        {
                            String strLevelName;
                            strLevelName.SetLength(255);
                            SendMessage(GetDlgItem(hwnd, IDC_LEVELLIST), LB_GETTEXT, dwCurSel, (LPARAM)(CTSTR)strLevelName);

                            editor->OpenEditableLevel(strLevelName);
                        }

                        EndDialog(hwnd, IDOK);
                        break;
                    }

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwnd, IDCANCEL);
            break;
    }
    return FALSE;

    traceOut;
}

EditorViewport* EditorEngine::GetMainViewport()
{
    for(DWORD i=0; i<editor->Viewports.Num(); i++)
    {
        EditorViewport *viewport = editor->Viewports[i];

        if(viewport->GetViewportType() == ViewportType_Main)
            return viewport;
    }

    return NULL;
}

Vect& SnapToSpacing(Vect &v, float spacing)
{
    float spacingD2 = spacing*0.5f;

    v.x += (v.x > 0.0f) ? spacingD2 : -spacingD2;
    v.y += (v.y > 0.0f) ? spacingD2 : -spacingD2;
    v.z += (v.z > 0.0f) ? spacingD2 : -spacingD2;

    v.x -= fmodf(v.x, spacing);
    v.y -= fmodf(v.y, spacing);
    v.z -= fmodf(v.z, spacing);

    return v;
}

Vect& EditorEngine::SnapPoint(Vect &v) const
{
    if(!levelInfo || !levelInfo->bSnapToGrid)
        return v;

    return SnapToSpacing(v, levelInfo->gridSpacing);
}

void EditorKBHandler::KeyboardHandler(unsigned int kbc, BOOL bKeydown)
{
    traceIn(EditorKBHandler::KeyboardHandler);

    EditorViewport *viewport = editor->GetMainViewport();

    if(levelInfo->curEditMode == EditMode_Create)
    {
        if(levelInfo && levelInfo->newObject)
        {
            if(levelInfo->newObject->ProcessKeyStroke(kbc, bKeydown))
                return;
        }
    }

    if(viewport)
    {
        switch(kbc)
        {
            case KBC_W:
                viewport->bMovingForward    = bKeydown;
                break;
            case KBC_S:
                viewport->bMovingBackward   = bKeydown;
                break;
            case KBC_A:
                viewport->bMovingLeft       = bKeydown;
                break;
            case KBC_D:
                viewport->bMovingRight      = bKeydown;
                break;

            case KBC_DELETE:
                if(bKeydown)
                {
                    if(levelInfo)
                        levelInfo->DeleteSelectedObjects();
                }
                break;

            case KBC_Z:
                if(bKeydown && GetButtonState(KBC_CONTROL))
                    SendMessage(hwndEditor, WM_COMMAND, ID_MYEDIT_UNDO, 0);
                break;

            case KBC_Y:
                if(bKeydown && GetButtonState(KBC_CONTROL))
                    SendMessage(hwndEditor, WM_COMMAND, ID_MYEDIT_REDO, 0);
                break;
        }
    }

    traceOut;
}

void EditorEngine::SetStatusText(DWORD statusNum, CTSTR lpText)
{
    traceIn(EditorEngine::SetStatusText);

    SendMessage(hwndStatusBar, SB_SETTEXT, statusNum, (LPARAM)lpText);

    traceOut;
}

void EditorEngine::EnableRealTimeRendering(bool bEnable)
{
    traceIn(EditorEngine::EnableRealTimeRendering);

    bRealTimeEnabled = bEnable;
    PostQuitMessage(1);

    //------------------------------------

    HMENU hMainMenu = GetMenu(hwndEditor);
    HMENU hSceneMenu = GetSubMenu(hMainMenu, 2);

    MENUITEMINFO mii;
    zero(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);

    mii.fMask  = MIIM_STATE;
    mii.fState = bEnable ? MFS_CHECKED : MFS_UNCHECKED;

    SetMenuItemInfo(hSceneMenu, ID_SCENE_ENABLESCENE, FALSE, &mii);

    //------------------------------------

    TBBUTTONINFO bi;
    zero(&bi, sizeof(bi));
    bi.cbSize  = sizeof(bi);

    bi.dwMask  = TBIF_STATE;
    bi.fsState = TBSTATE_ENABLED | (bEnable ? TBSTATE_CHECKED : 0);

    SendMessage(GetDlgItem(hwndEditor, IDC_MAINTOOLAR), TB_SETBUTTONINFO, ID_SCENE_ENABLESCENE, (LPARAM)&bi);

    traceOut;
}

void EditorEngine::HideEditorObjects(bool bHide)
{
    traceIn(EditorEngine::HideEditorObjects);

    bHideEditorObjects = bHide;

    //------------------------------------

    HMENU hMainMenu = GetMenu(hwndEditor);
    HMENU hSceneMenu = GetSubMenu(hMainMenu, 2);

    MENUITEMINFO mii;
    zero(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);

    mii.fMask  = MIIM_STATE;
    mii.fState = bHide ? MFS_CHECKED : MFS_UNCHECKED;

    SetMenuItemInfo(hSceneMenu, ID_SCENE_HIDEEDITOROBJECTS, FALSE, &mii);

    //------------------------------------

    TBBUTTONINFO bi;
    zero(&bi, sizeof(bi));
    bi.cbSize  = sizeof(bi);

    bi.dwMask  = TBIF_STATE;
    bi.fsState = TBSTATE_ENABLED | (bHide ? TBSTATE_CHECKED : 0);

    SendMessage(GetDlgItem(hwndEditor, IDC_MAINTOOLAR), TB_SETBUTTONINFO, ID_SCENE_HIDEEDITOROBJECTS, (LPARAM)&bi);

    UpdateViewports();

    traceOut;
}


void ENGINEAPI UpdateEditMenu()
{
    HMENU hMainMenu = GetMenu(hwndEditor);
    HMENU hEditMenu = GetSubMenu(hMainMenu, 1);

    String strName;

    MENUITEMINFO mii;
    zero(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);

    strName = TEXT("&Undo");
    if(editor->undoStack->IsValid())
        strName << TEXT(" ") << editor->undoStack->GetCurrentName();
    strName << TEXT("\tCtrl-Z");

    mii.fMask  = MIIM_STATE|MIIM_STRING;
    mii.dwTypeData = (TSTR)strName;
    mii.fState = editor->undoStack->IsValid() ? MFS_ENABLED : MFS_GRAYED;
    SetMenuItemInfo(hEditMenu, ID_MYEDIT_UNDO, FALSE, &mii);

    strName = TEXT("&Redo");
    if(editor->redoStack->IsValid())
        strName << TEXT(" ") << editor->redoStack->GetCurrentName();
    strName << TEXT("\tCtrl-Y");

    mii.fMask  = MIIM_STATE|MIIM_STRING;
    mii.dwTypeData = (TSTR)strName;
    mii.fState = editor->redoStack->IsValid() ? MFS_ENABLED : MFS_GRAYED;

    SetMenuItemInfo(hEditMenu, ID_MYEDIT_REDO, FALSE, &mii);
}


struct EditBoxDialogInfo
{
    CTSTR lpWindowName;
    String &data;
};

bool EditorEngine::EditBoxDialog(HWND hwndParent, CTSTR lpName, String &result)
{
    EditBoxDialogInfo newInfo = {lpName, result};
    return DialogBoxParam(hinstMain, MAKEINTRESOURCE(IDD_EDITBOXDIALOG), hwndParent, (DLGPROC)EditBoxDialogProc, (LPARAM)&newInfo) == IDOK;
}


BOOL CALLBACK EditBoxDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(EditBoxDialogProc);

    static EditBoxDialogInfo *curInfo = NULL;

    switch(message)
    {
        case WM_INITDIALOG:
            curInfo = (EditBoxDialogInfo*)lParam;
            SetWindowText(hwnd, curInfo->lpWindowName);

            curInfo->data.Clear();
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    {
                        HWND hwndControl = GetDlgItem(hwnd, IDC_EDITBOX);
                        int size = SendMessage(hwndControl, WM_GETTEXTLENGTH, 0, 0)+1;

                        curInfo->data.SetLength(size);
                        SendMessage(hwndControl, WM_GETTEXT, (WPARAM)size, (LPARAM)(TSTR)curInfo->data);

                        EndDialog(hwnd, IDOK);
                        break;
                    }

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;
            }
    }

    return FALSE;

    traceOut;
}

void SetRadioButtonCheck(HWND hwndParent, DWORD buttonID)
{
    if(!hwndParent)
        return;

    DWORD firstButton = buttonID;
    DWORD lastButton = buttonID;

    HWND curWindow = GetDlgItem(hwndParent, buttonID);
    while(curWindow = GetWindow(curWindow, GW_HWNDPREV))
    {
        DWORD style = (DWORD)GetWindowLongPtr(curWindow, GWL_STYLE);
        DWORD id = (DWORD)GetWindowLongPtr(curWindow, GWLP_ID);

        if(id != INVALID)
        {
            firstButton = id;
            TCHAR className[8];
            GetClassName(GetDlgItem(hwndParent, firstButton), className, 7);

            if((scmpi(className, TEXT("button")) == 0) && (style & BS_AUTORADIOBUTTON))
                SendMessage(GetDlgItem(hwndParent, firstButton), BM_SETCHECK, BST_UNCHECKED, 0);
        }

        if(style & WS_GROUP)
            break;
    }

    curWindow = GetDlgItem(hwndParent, buttonID);
    while(curWindow = GetWindow(curWindow, GW_HWNDNEXT))
    {
        DWORD style = (DWORD)GetWindowLongPtr(curWindow, GWL_STYLE);
        DWORD id = (DWORD)GetWindowLongPtr(curWindow, GWLP_ID);

        if(id != INVALID)
        {
            lastButton = id;
            TCHAR className[8];
            GetClassName(GetDlgItem(hwndParent, lastButton), className, 7);

            if((scmpi(className, TEXT("button")) == 0) && (style & BS_AUTORADIOBUTTON))
                SendMessage(GetDlgItem(hwndParent, lastButton), BM_SETCHECK, BST_UNCHECKED, 0);
        }

        if(style & WS_GROUP)
            break;
    }

    SendMessage(GetDlgItem(hwndParent, buttonID), BM_SETCHECK, BST_CHECKED, 0);
}

void GetPossibleModules(StringList &modules)
{
    modules.Clear();

    OSFindData ofdModule;
    HANDLE hFindModule = OSFindFirstFile(TEXT("data/*."), ofdModule);
    if(!hFindModule)
        return;

    do
    {
        if(ofdModule.bDirectory)
            modules << ofdModule.fileName;
    }while(OSFindNextFile(hFindModule, ofdModule));

    OSFindClose(hFindModule);
}

BOOL GetResourceStringFromPath(CTSTR lpFilePath, CTSTR lpExpectedType, String &strResource)
{
    String strFile = lpFilePath;
    TSTR lpModule = sstr(strFile, TEXT("/data/"));
    if(!lpModule) return FALSE;

    lpModule += 6;

    TSTR lpEndMarker = schr(lpModule, '/');
    if(!lpEndMarker) return FALSE;

    *lpEndMarker = 0;
    String strModule = lpModule;
    *lpEndMarker = '/';

    String strType;
    strType << TEXT("/") << lpExpectedType << TEXT("/");

    if(scmpi_n(lpEndMarker, strType, strType.Length()) != 0)
        return FALSE;

    lpEndMarker += strType.Length();

    strResource.Clear();
    strResource << strModule << TEXT(":") << lpEndMarker;

    return TRUE;
}

void WriteTargaFile(CTSTR lpFileName, DWORD width, DWORD height, DWORD channels, LPBYTE lpData, DWORD cropX, DWORD cropY, DWORD cropCX, DWORD cropCY)
{
    TargaHeader th;

    if(!cropCX) cropCX = width;
    if(!cropCY) cropCY = height;

    zero(&th, sizeof(th));

    int cropWidth  = cropCX-cropX;
    int cropHeight = cropCY-cropY;

    th.depth    = BYTE(channels*8);
    th.width    = WORD(cropWidth);
    th.height   = WORD(cropHeight);
    th.ImageType = 2;

    XFile targaFile(lpFileName, XFILE_WRITE, XFILE_CREATEALWAYS);
    targaFile.Write(&th, sizeof(th));
    for(int i=height-1; i>=0; i--)
    {
        if((i >= cropY) && (i < cropCY))
            targaFile.Write(lpData+(i*width*channels)+(cropX*channels), cropWidth*channels);
    }
    targaFile.Close();
}

float sqrtTableVals[32768];

void InitSqrtTable()
{
    sqrtTableVals[0] = 0.0f;

    for(int i=1; i<32768; i++)
    {
        double d = double(i);
        d /= 32767.0;
        d = sqrt(d);
        sqrtTableVals[i] = (float)d;
    }
}

float SqrtTable(float f)
{
    Saturate(f);
    UINT lookup = UINT(double(f) * 32767.0);
    return sqrtTableVals[lookup];
}


BOOL CALLBACK LightmapSettingsDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(LightmapSettingsDialogProc);

    switch(message)
    {
        case WM_INITDIALOG:
            {
                HWND hwndTemp;

                hwndTemp = GetDlgItem(hwnd, levelInfo->lightmapSettings.bUseIBL ? IDC_IBL : IDC_BASICLIGHTING);
                SendMessage(hwndTemp, BM_SETCHECK, BST_CHECKED, 0);

                if(!levelInfo->lightmapSettings.nGIPasses)
                    levelInfo->lightmapSettings.nGIPasses = 1;

                SendMessage(GetDlgItem(hwnd, IDC_ENABLEGI), BM_SETCHECK, levelInfo->lightmapSettings.bUseGI ? BST_CHECKED : BST_UNCHECKED, 0);
                hwndTemp = GetDlgItem(hwnd, IDC_PASSCOUNT_SCROLLER);
                LinkUpDown(hwndTemp, GetDlgItem(hwnd, IDC_PASSCOUNT));
                InitUpDownIntData(hwndTemp, levelInfo->lightmapSettings.nGIPasses, 1, 6);

                SendMessage(GetDlgItem(hwnd, IDC_BLURSHADOWS), BM_SETCHECK, levelInfo->lightmapSettings.bBlurShadows ? BST_CHECKED : BST_UNCHECKED, 0);

                if(!levelInfo->lightmapSettings.hemicubeResolution)
                    levelInfo->lightmapSettings.hemicubeResolution = 32;
                if(levelInfo->lightmapSettings.maxPhotonDist < 40.0f)
                    levelInfo->lightmapSettings.maxPhotonDist = 50.0f;


                hwndTemp = GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER);
                LinkUpDown(hwndTemp, GetDlgItem(hwnd, IDC_MAXPHOTONDIST));
                InitUpDownFloatData(hwndTemp, levelInfo->lightmapSettings.maxPhotonDist, 40.0f, 1000.0f);
                hwndTemp = GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION);
                SendMessage(hwndTemp, CB_ADDSTRING, 0, (LPARAM)TEXT("32"));
                SendMessage(hwndTemp, CB_ADDSTRING, 0, (LPARAM)TEXT("64"));
                SendMessage(hwndTemp, CB_ADDSTRING, 0, (LPARAM)TEXT("128"));

                int curResVal = levelInfo->lightmapSettings.hemicubeResolution>>6;
                SendMessage(hwndTemp, CB_SETCURSEL, curResVal, 0);

                if(levelInfo->lightmapSettings.gamma < 0.5f)
                    levelInfo->lightmapSettings.gamma = 1.0f;
                if(levelInfo->lightmapSettings.brightness < 0.5f)
                    levelInfo->lightmapSettings.brightness = 1.0f;
                if(levelInfo->lightmapSettings.contrast < 0.5f)
                    levelInfo->lightmapSettings.contrast = 1.0f;

                SendMessage(GetDlgItem(hwnd, IDC_COMPRESS), BM_SETCHECK, levelInfo->lightmapSettings.bUseDXT1 ? BST_CHECKED : BST_UNCHECKED, 0);

                SendMessage(GetDlgItem(hwnd, IDC_ENABLECF), BM_SETCHECK, levelInfo->lightmapSettings.bFilter ? BST_CHECKED : BST_UNCHECKED, 0);
                hwndTemp = GetDlgItem(hwnd, IDC_GAMMASCROLL);
                LinkUpDown(hwndTemp, GetDlgItem(hwnd, IDC_GAMMAEDIT));
                InitUpDownFloatData(hwndTemp, levelInfo->lightmapSettings.gamma, 0.5f, 2.0f, 0.1f);
                hwndTemp = GetDlgItem(hwnd, IDC_BRIGHTNESSSCROLL);
                LinkUpDown(hwndTemp, GetDlgItem(hwnd, IDC_BRIGHTNESSEDIT));
                InitUpDownFloatData(hwndTemp, levelInfo->lightmapSettings.brightness, 0.5f, 1.5f, 0.1f);
                hwndTemp = GetDlgItem(hwnd, IDC_CONTRASTSCROLL);
                LinkUpDown(hwndTemp, GetDlgItem(hwnd, IDC_CONTRASTEDIT));
                InitUpDownFloatData(hwndTemp, levelInfo->lightmapSettings.contrast, 0.5f, 1.5f, 0.1f);

                SendMessage(GetDlgItem(hwnd, IDC_ENABLEAO), BM_SETCHECK, levelInfo->lightmapSettings.bUseAO ? BST_CHECKED : BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hwnd, IDC_RENDERAO), BM_SETCHECK, levelInfo->lightmapSettings.bVisualizeAO ? BST_CHECKED : BST_UNCHECKED, 0);
                hwndTemp = GetDlgItem(hwnd, IDC_MAXAODIST_SCROLLER);
                LinkUpDown(hwndTemp, GetDlgItem(hwnd, IDC_MAXAODIST));
                InitUpDownFloatData(hwndTemp, MIN(levelInfo->lightmapSettings.aoDist, levelInfo->lightmapSettings.maxPhotonDist), 1.0f, levelInfo->lightmapSettings.maxPhotonDist);
                hwndTemp = GetDlgItem(hwnd, IDC_AOEXPONENT_SCROLLER);
                LinkUpDown(hwndTemp, GetDlgItem(hwnd, IDC_AOEXPONENT));
                InitUpDownFloatData(hwndTemp, levelInfo->lightmapSettings.aoExponent, 0.1f, 4.0f, 0.1f);
                hwndTemp = GetDlgItem(hwnd, IDC_AODARKCUTOFF_SCROLLER);
                LinkUpDown(hwndTemp, GetDlgItem(hwnd, IDC_AODARKCUTOFF));
                InitUpDownFloatData(hwndTemp, levelInfo->lightmapSettings.aoDarknessCutoff, 0.0f, 0.5f, 0.01f);

                hwndTemp = GetDlgItem(hwnd, IDC_LMMULTIPLIER);
                SendMessage(hwndTemp, CB_ADDSTRING, 0, (LPARAM)TEXT("Half"));
                SendMessage(hwndTemp, CB_ADDSTRING, 0, (LPARAM)TEXT("None"));
                SendMessage(hwndTemp, CB_ADDSTRING, 0, (LPARAM)TEXT("Double"));
                SendMessage(hwndTemp, CB_SETCURSEL, levelInfo->lightmapSettings.resMultiplier, 0);

                BOOL bEnableGI = SendMessage(GetDlgItem(hwnd, IDC_ENABLEGI), BM_GETCHECK, 0, 0) == BST_CHECKED;
                BOOL bIBL = SendMessage(GetDlgItem(hwnd, IDC_IBL), BM_GETCHECK, 0, 0) == BST_CHECKED;
                BOOL bAOEnable = SendMessage(GetDlgItem(hwnd, IDC_ENABLEAO), BM_GETCHECK, 0, 0) == BST_CHECKED;
                BOOL bEnable = bEnableGI || bIBL || bAOEnable;
                BOOL bCFEnable = SendMessage(GetDlgItem(hwnd, IDC_ENABLECF), BM_GETCHECK, 0, 0) == BST_CHECKED;

                EnableWindow(GetDlgItem(hwnd, IDC_GAMMAEDIT), bCFEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_GAMMASCROLL), bCFEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_BRIGHTNESSEDIT), bCFEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_BRIGHTNESSSCROLL), bCFEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_CONTRASTEDIT), bCFEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_CONTRASTSCROLL), bCFEnable);

                EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT), bEnableGI);
                EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT_SCROLLER), bEnableGI);

                EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST), bEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), bEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), bEnable);

                EnableWindow(GetDlgItem(hwnd, IDC_RENDERAO), bAOEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_MAXAODIST), bAOEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_MAXAODIST_SCROLLER), bAOEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_AOEXPONENT), bAOEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_AOEXPONENT_SCROLLER), bAOEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_AODARKCUTOFF), bAOEnable);
                EnableWindow(GetDlgItem(hwnd, IDC_AODARKCUTOFF_SCROLLER), bAOEnable);

                EnableWindow(GetDlgItem(hwnd, IDC_BLURSHADOWS), SendMessage(GetDlgItem(hwnd, IDC_BASICLIGHTING), BM_GETCHECK, 0, 0) == BST_CHECKED);
                break;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    {
                        levelInfo->lightmapSettings.bUseIBL = SendMessage(GetDlgItem(hwnd, IDC_IBL), BM_GETCHECK, 0, 0) == BST_CHECKED;

                        levelInfo->lightmapSettings.bUseGI = SendMessage(GetDlgItem(hwnd, IDC_ENABLEGI), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        levelInfo->lightmapSettings.nGIPasses = GetUpDownInt(GetDlgItem(hwnd, IDC_PASSCOUNT_SCROLLER));

                        levelInfo->lightmapSettings.bBlurShadows = SendMessage(GetDlgItem(hwnd, IDC_BLURSHADOWS), BM_GETCHECK, 0, 0) == BST_CHECKED;

                        levelInfo->lightmapSettings.maxPhotonDist = GetUpDownFloat(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER));

                        int hemicubeRes;
                        hemicubeRes = 1<<SendMessage(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), CB_GETCURSEL, 0, 0);
                        levelInfo->lightmapSettings.hemicubeResolution = 32*hemicubeRes;

                        levelInfo->lightmapSettings.bUseDXT1 = SendMessage(GetDlgItem(hwnd, IDC_COMPRESS), BM_GETCHECK, 0, 0) == BST_CHECKED;

                        levelInfo->lightmapSettings.bFilter = SendMessage(GetDlgItem(hwnd, IDC_ENABLECF), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        levelInfo->lightmapSettings.gamma = GetUpDownFloat(GetDlgItem(hwnd, IDC_GAMMASCROLL));
                        levelInfo->lightmapSettings.brightness = GetUpDownFloat(GetDlgItem(hwnd, IDC_BRIGHTNESSSCROLL));
                        levelInfo->lightmapSettings.contrast = GetUpDownFloat(GetDlgItem(hwnd, IDC_CONTRASTSCROLL));

                        levelInfo->lightmapSettings.bUseAO = SendMessage(GetDlgItem(hwnd, IDC_ENABLEAO), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        levelInfo->lightmapSettings.bVisualizeAO = SendMessage(GetDlgItem(hwnd, IDC_RENDERAO), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        levelInfo->lightmapSettings.aoDist = GetUpDownFloat(GetDlgItem(hwnd, IDC_MAXAODIST_SCROLLER));
                        levelInfo->lightmapSettings.aoExponent = GetUpDownFloat(GetDlgItem(hwnd, IDC_AOEXPONENT_SCROLLER));
                        levelInfo->lightmapSettings.aoDarknessCutoff = GetUpDownFloat(GetDlgItem(hwnd, IDC_AODARKCUTOFF_SCROLLER));

                        levelInfo->lightmapSettings.resMultiplier = SendMessage(GetDlgItem(hwnd, IDC_LMMULTIPLIER), CB_GETCURSEL, 0, 0);

                        EndDialog(hwnd, IDOK);
                        break;
                    }

                case IDC_BASICLIGHTING:
                    {
                        EnableWindow(GetDlgItem(hwnd, IDC_BLURSHADOWS), TRUE);
                        if(SendMessage(GetDlgItem(hwnd, IDC_ENABLEGI), BM_GETCHECK, 0, 0) == BST_UNCHECKED)
                        {
                            EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), FALSE);
                        }
                        break;
                    }

                case IDC_MAXPHOTONDIST_SCROLLER:
                    {
                        if(HIWORD(wParam) == JUDN_UPDATE)
                        {
                            HWND hwndAODist = GetDlgItem(hwnd, IDC_MAXAODIST_SCROLLER);
                            HWND hwndPhotonDist = GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER);
                            float aoDist = GetUpDownFloat(hwndAODist);
                            float photonDist = GetUpDownFloat(hwndPhotonDist);

                            InitUpDownFloatData(hwndAODist, MIN(aoDist, photonDist), 1.0f, photonDist);
                        }
                        break;
                    }

                case IDC_IBL:
                    {
                        EnableWindow(GetDlgItem(hwnd, IDC_BLURSHADOWS), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), TRUE);
                        break;
                    }

                case IDC_ENABLEGI:
                    {
                        BOOL bEnableGI = SendMessage(GetDlgItem(hwnd, IDC_ENABLEGI), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        BOOL bIBL = SendMessage(GetDlgItem(hwnd, IDC_IBL), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        BOOL bAOEnable = SendMessage(GetDlgItem(hwnd, IDC_ENABLEAO), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        BOOL bEnable = bEnableGI || bIBL || bAOEnable;

                        EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT), bEnableGI);
                        EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT_SCROLLER), bEnableGI);

                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST), bEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), bEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), bEnable);
                        break;
                    }

                case IDC_ENABLEAO:
                    {
                        BOOL bEnableGI = SendMessage(GetDlgItem(hwnd, IDC_ENABLEGI), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        BOOL bIBL = SendMessage(GetDlgItem(hwnd, IDC_IBL), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        BOOL bAOEnable = SendMessage(GetDlgItem(hwnd, IDC_ENABLEAO), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        BOOL bEnable = bEnableGI || bIBL || bAOEnable;

                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST), bEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), bEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), bEnable);

                        EnableWindow(GetDlgItem(hwnd, IDC_RENDERAO), bAOEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_MAXAODIST), bAOEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_MAXAODIST_SCROLLER), bAOEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_AOEXPONENT), bAOEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_AOEXPONENT_SCROLLER), bAOEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_AODARKCUTOFF), bAOEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_AODARKCUTOFF_SCROLLER), bAOEnable);
                        break;
                    }

                case IDC_ENABLECF:
                    {
                        BOOL bCFEnable = SendMessage(GetDlgItem(hwnd, IDC_ENABLECF), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        EnableWindow(GetDlgItem(hwnd, IDC_GAMMAEDIT), bCFEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_GAMMASCROLL), bCFEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_BRIGHTNESSEDIT), bCFEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_BRIGHTNESSSCROLL), bCFEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_CONTRASTEDIT), bCFEnable);
                        EnableWindow(GetDlgItem(hwnd, IDC_CONTRASTSCROLL), bCFEnable);
                        break;
                    }

                case IDC_HIGHQUALITY:
                    {
                        SendMessage(GetDlgItem(hwnd, IDC_IBL), BM_SETCHECK, BST_CHECKED, 0);
                        SendMessage(GetDlgItem(hwnd, IDC_BASICLIGHTING), BM_SETCHECK, BST_UNCHECKED, 0);

                        SendMessage(GetDlgItem(hwnd, IDC_ENABLEGI), BM_SETCHECK, BST_CHECKED, 0);
                        SetUpDownInt(GetDlgItem(hwnd, IDC_PASSCOUNT_SCROLLER), 3);

                        SetUpDownFloat(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), 50.0f);
                        SendMessage(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), CB_SETCURSEL, 1, 0);

                        EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT_SCROLLER), TRUE);

                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), TRUE);

                        EnableWindow(GetDlgItem(hwnd, IDC_BLURSHADOWS), FALSE);
                        break;
                    }

                case IDC_DECENTQUALITY:
                    {
                        SendMessage(GetDlgItem(hwnd, IDC_IBL), BM_SETCHECK, BST_CHECKED, 0);
                        SendMessage(GetDlgItem(hwnd, IDC_BASICLIGHTING), BM_SETCHECK, BST_UNCHECKED, 0);

                        SendMessage(GetDlgItem(hwnd, IDC_ENABLEGI), BM_SETCHECK, BST_CHECKED, 0);
                        SetUpDownInt(GetDlgItem(hwnd, IDC_PASSCOUNT_SCROLLER), 3);

                        SetUpDownFloat(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), 50.0f);
                        SendMessage(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), CB_SETCURSEL, 0, 0);

                        EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT_SCROLLER), TRUE);

                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), TRUE);

                        EnableWindow(GetDlgItem(hwnd, IDC_BLURSHADOWS), FALSE);
                        break;
                    }

                case IDC_BASICQUALITY:
                    {
                        SendMessage(GetDlgItem(hwnd, IDC_IBL), BM_SETCHECK, BST_UNCHECKED, 0);
                        SendMessage(GetDlgItem(hwnd, IDC_BASICLIGHTING), BM_SETCHECK, BST_CHECKED, 0);

                        SendMessage(GetDlgItem(hwnd, IDC_ENABLEGI), BM_SETCHECK, BST_CHECKED, 0);
                        SetUpDownInt(GetDlgItem(hwnd, IDC_PASSCOUNT_SCROLLER), 1);

                        SetUpDownFloat(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), 50.0f);
                        SendMessage(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), CB_SETCURSEL, 0, 0);

                        SendMessage(GetDlgItem(hwnd, IDC_BLURSHADOWS), BM_SETCHECK, BST_CHECKED, 0);

                        EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT_SCROLLER), TRUE);

                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), TRUE);
                        EnableWindow(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), TRUE);

                        EnableWindow(GetDlgItem(hwnd, IDC_BLURSHADOWS), TRUE);
                        break;
                    }

                case IDC_FASTQUALITY:
                    {
                        SendMessage(GetDlgItem(hwnd, IDC_IBL), BM_SETCHECK, BST_UNCHECKED, 0);
                        SendMessage(GetDlgItem(hwnd, IDC_BASICLIGHTING), BM_SETCHECK, BST_CHECKED, 0);

                        SendMessage(GetDlgItem(hwnd, IDC_ENABLEGI), BM_SETCHECK, BST_UNCHECKED, 0);

                        SendMessage(GetDlgItem(hwnd, IDC_BLURSHADOWS), BM_SETCHECK, BST_UNCHECKED, 0);

                        EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_PASSCOUNT_SCROLLER), FALSE);

                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_MAXPHOTONDIST_SCROLLER), FALSE);
                        EnableWindow(GetDlgItem(hwnd, IDC_PHOTONMAPRESOLUTION), FALSE);

                        EnableWindow(GetDlgItem(hwnd, IDC_BLURSHADOWS), TRUE);
                        break;
                    }

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwnd, IDCANCEL);
            break;
    }

    return FALSE;

    traceOut;
}

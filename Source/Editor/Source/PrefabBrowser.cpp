/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  PrefabBrowser.cpp

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


DefineClass(PrefabWindow);


LRESULT WINAPI PrefabBrowserProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI PrefabViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI PrefabControlContainerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


PrefabBrowser *prefabBrowser = NULL;


#define ID_ANIMOFFSET           4000


#define PARAM_ID                5000
#define PARAMSELECT_ID          5100
#define PARAMLMRES_ID           5200
#define PARAMLMSCROLL_ID        5201
#define PARAMMAX_ID             5300

TCHAR lpLastPrefabModule[250] = TEXT("");
TCHAR lpLastPrefabDir[250] = TEXT("");


void PrefabBrowser::RegisterWindowClasses()
{
    traceIn(PrefabBrowser::RegisterWindowClasses);

    WNDCLASS wc;
    zero(&wc, sizeof(wc));

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)PrefabBrowserProc;
    wc.hInstance = hinstMain;
    wc.hIcon = LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_PREFABBROWSERMENU);
    wc.lpszClassName = TEXT("PrefabBrowser");

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the prefab browser window class"));

    wc.lpfnWndProc = (WNDPROC)PrefabViewProc;
    wc.lpszMenuName = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = TEXT("XR3DPrefabView");
    wc.cbWndExtra = sizeof(LPVOID);

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the prefab view class"));

    wc.style = CS_PARENTDC;
    wc.lpfnWndProc = (WNDPROC)PrefabControlContainerProc;
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
    wc.lpszClassName = TEXT("XR3DPrefabControlContainer");
    wc.cbWndExtra = 0;

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the material view class"));

    traceOut;
}


#define clientXSize   900
#define clientYSize   600

PrefabBrowser::PrefabBrowser()
{
    traceIn(PrefabBrowser::PrefabBrowser);

    prefabBrowser = this;

    int borderXSize = clientXSize;
    int borderYSize = clientYSize;

    borderXSize += GetSystemMetrics(SM_CXDLGFRAME)*2;

    borderYSize += GetSystemMetrics(SM_CYDLGFRAME)*2;
    borderYSize += GetSystemMetrics(SM_CYCAPTION);
    borderYSize += GetSystemMetrics(SM_CYMENU);

    hwndPrefabBrowser = CreateWindow(TEXT("PrefabBrowser"), TEXT("Prefab Browser"),
                                   WS_VISIBLE|WS_DLGFRAME|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   borderXSize, borderYSize,
                                   hwndEditor, NULL, hinstMain, NULL);

    //-----------------------------------------

    hwndModuleList = CreateWindow(TEXT("combobox"), NULL,
                                  WS_VISIBLE|WS_CHILDWINDOW|CBS_DROPDOWNLIST|CBS_SORT,
                                  0, 0, 300, 190,
                                  hwndPrefabBrowser, NULL, hinstMain, NULL);
    SendMessage(hwndModuleList, WM_SETFONT, (WPARAM)hWindowFont, 0);

    RECT sizeOfWorthlessComboBox;
    GetWindowRect(hwndModuleList, &sizeOfWorthlessComboBox);
    int comboBoxSize = sizeOfWorthlessComboBox.bottom-sizeOfWorthlessComboBox.top;

    //-----------------------------------------

    hwndTreeControl = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, NULL,
                                    WS_VISIBLE|WS_CHILDWINDOW|TVS_EDITLABELS|TVS_HASLINES|TVS_LINESATROOT|TVS_SHOWSELALWAYS|TVS_HASBUTTONS,
                                    0, comboBoxSize, 300, clientYSize,
                                    hwndPrefabBrowser, NULL, hinstMain, NULL);

    SendMessage(hwndTreeControl, WM_SETFONT, (WPARAM)hWindowFont, 0);

    HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK, 4, 0);

    SHSTOCKICONINFO shsii;
    zero(&shsii, sizeof(shsii));
    shsii.cbSize = sizeof(shsii);

    HINSTANCE hShell = GetModuleHandle(TEXT("shell32"));

    HICON hIcon = LoadIcon(hShell, MAKEINTRESOURCE(4));
    ImageList_AddIcon(hImageList, hIcon);

    hIcon = LoadIcon(hShell, MAKEINTRESOURCE(1));
    ImageList_AddIcon(hImageList, hIcon);

    TreeView_SetImageList(hwndTreeControl, hImageList, TVSIL_NORMAL);

    //-----------------------------------------

    hwndPrefabView = (HANDLE)CreateWindow(TEXT("XR3DPrefabView"), NULL,
                                        WS_VISIBLE|WS_CHILD,
                                        300, 0, 600, 600,
                                        hwndPrefabBrowser, NULL, hinstMain, NULL);

    String strGraphicsSystem = AppConfig->GetString(TEXT("Engine"), TEXT("GraphicsSystem"));
    prefabView = (GraphicsSystem*)CreateFactoryObject(strGraphicsSystem, FALSE);

    if(!prefabView)
        CrashError(TEXT("Bad Graphics System: %s"), strGraphicsSystem);

    prefabView->InitializeDevice(hwndPrefabView);
    prefabView->InitializeObject();
    prefabView->SetSize(600, 600);

    //-----------------------------------------

    List<CTSTR> curModules;
    Engine::GetGameModules(curModules);
    //GetPossibleModules(possibleModules);

    for(int i=0; i<curModules.Num(); i++)
    {
        int id = SendMessage(hwndModuleList, CB_ADDSTRING, 0, (LPARAM)curModules[i]);

        if(scmpi(curModules[i], lpLastPrefabModule) == 0)
        {
            curModule = curModules[i];
            SendMessage(hwndModuleList, CB_SETCURSEL, id, 0);
        }
    }

    if(curModule.IsEmpty())
    {
        curModule = TEXT("Base");
        SendMessage(hwndModuleList, CB_SETCURSEL, SendMessage(hwndModuleList, CB_FINDSTRING, -1, (LPARAM)TEXT("Base")), 0);
    }

    //-----------------------------------------

    prefabManager = new ResourceManager;

    mainGD = GS;
    mainResourceMan = RM;

    prefabWindow = CreateObject(PrefabWindow);

    //-----------------------------------------

    GS = prefabView;
    RM = prefabManager;

    defaultMaterial = CreateObject(Material);
    defaultMaterial->effect = GetEffect(TEXT("Base:Bump.effect"));

    MaterialParameter parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("diffuseTexture"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:default/TEST.dds"));
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("normalMap"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:default/TEST_bm.dds"));
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("specularMap"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:default/white.tga"), FALSE);
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("illuminationMap"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:default/white.tga"), FALSE);
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("specularLevel"));
    parameter.type = Parameter_Vector3;
    *(float*)parameter.data = 0.0f;
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("specularPower"));
    parameter.type = Parameter_Float;
    *(float*)parameter.data = 32.0f;
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("illuminationColor"));
    parameter.type = Parameter_Vector3;
    zero(parameter.data, sizeof(Vect));
    defaultMaterial->Params << parameter;

    DWORD chi = 0xFFFFFFFF;
    blankAttenuation = prefabView->CreateTexture(1, 1, 3, &chi, FALSE, TRUE);

    GS = mainGD;
    RM = mainResourceMan;

    //prefabView->SetSize(200, 200);
    //SetWindowPos(hwndPrefabView, NULL, 0, 0, 200, 200, SWP_NOMOVE);

    //-----------------------------------------

    hCurTreeItem = NULL;
    curMesh = NULL;

    String curDir;
    curDir << TEXT("data/") << curModule << TEXT("/prefabs/");
    ProcessDirectory(curDir, NULL, curModule);

    TreeView_Expand(hwndTreeControl, TreeView_GetRoot(hwndTreeControl), TVE_EXPAND);

    traceOut;
}

PrefabBrowser::~PrefabBrowser()
{
    traceIn(PrefabBrowser::~PrefabBrowser);

    String strModuleDir;
    strModuleDir << TEXT("data/") << curModule << TEXT("/prefabs");
    if(!OSFileExists(strModuleDir + TEXT("/*.*")))
        RemoveDirectory(strModuleDir);

    //--------------------------------

    String curPath;
    GetItemPath(hCurTreeItem, curPath, TRUE);
    if(!curMesh)
        curPath << TEXT("/");

    scpy_n(lpLastPrefabModule, curModule, 240);
    scpy_n(lpLastPrefabDir, curPath, 240);

    HIMAGELIST hImageList = TreeView_GetImageList(hwndTreeControl, TVSIL_NORMAL);

    DeleteMesh();

    GS = prefabView;
    RM = prefabManager;

    DestroyObject(defaultMaterial);
    DestroyObject(blankAttenuation);

    GS = mainGD;
    RM = mainResourceMan;

    delete prefabManager;

    for(int i=0; i<EditingControls.Num(); i++)
        DestroyWindow(EditingControls[i]);

    DestroyObject(prefabWindow);
    DestroyObject(prefabView);

    DestroyWindow(hwndTreeControl);
    DestroyWindow(hwndPrefabBrowser);

    ImageList_Destroy(hImageList);
    prefabBrowser = NULL;

    traceOut;
}


void PrefabBrowser::ProcessDirectory(TSTR lpBaseDir, HTREEITEM hParent, TSTR lpName)
{
    traceIn(PrefabBrowser::ProcessDirectory);

    if((scmpi(lpName, TEXT("Editor")) == 0) || (scmpi(lpName, TEXT(".svn")) == 0))
        return;

    HTREEITEM hItem;

    TVINSERTSTRUCT tvis;

    zero(&tvis, sizeof(TVINSERTSTRUCT));
    tvis.hParent                = hParent;
    tvis.hInsertAfter           = TVI_SORT;
    tvis.itemex.mask            = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
    tvis.itemex.iImage          = 0;
    tvis.itemex.iSelectedImage  = 0;
    tvis.itemex.pszText         = lpName;
    hItem = TreeView_InsertItem(hwndTreeControl, &tvis);

    tvis.hParent                = hItem;
    tvis.hInsertAfter           = TVI_LAST;
    tvis.itemex.iImage          = 1;
    tvis.itemex.iSelectedImage  = 1;

    String strFindPath = lpBaseDir;
    strFindPath << TEXT("*.*");

    WIN32_FIND_DATA wfd;
    HANDLE hFind = FindFirstFile(strFindPath, &wfd);

    if(hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if((scmp(wfd.cFileName, TEXT(".")) == 0) || (scmp(wfd.cFileName, TEXT("..")) == 0))
                    continue;
                strFindPath.Clear() << lpBaseDir << wfd.cFileName << TEXT("/");
                ProcessDirectory(strFindPath, hItem, wfd.cFileName);
            }
        }while(FindNextFile(hFind, &wfd));

        FindClose(hFind);
    }

    strFindPath.Clear() << lpBaseDir << TEXT("*.pfb");
    hFind = FindFirstFile(strFindPath, &wfd);

    if(hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            String fileName = wfd.cFileName;
            fileName.SetLength(fileName.Length()-4);
            tvis.itemex.pszText = fileName;
            HTREEITEM hTest = TreeView_InsertItem(hwndTreeControl, &tvis);

            String curItemPath = lpBaseDir;
            curItemPath << fileName;
            if(lpLastPrefabDir[0] && scmpi(curItemPath, lpLastPrefabDir) == 0)
            {
                TreeView_SelectItem(hwndTreeControl, hTest);
            }
        }while(FindNextFile(hFind, &wfd));

        FindClose(hFind);
    }

    if( (!lpLastPrefabDir[0] && !hParent) ||
        (lpLastPrefabDir[0] && scmpi(lpBaseDir, lpLastPrefabDir) == 0) )
    {
        TreeView_SelectItem(hwndTreeControl, hItem);
    }

    traceOut;
}

void PrefabBrowser::InsertTreeViewItem(HTREEITEM hParent, CTSTR lpName, BOOL bFolder)
{
    traceIn(PrefabBrowser::InsertTreeViewItem);

    TVINSERTSTRUCT tvis;
    zero(&tvis, sizeof(TVINSERTSTRUCT));
    tvis.hParent                = hParent;
    tvis.itemex.mask            = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
    tvis.itemex.iImage          = !bFolder;
    tvis.itemex.iSelectedImage  = !bFolder;
    tvis.itemex.pszText         = (TSTR)lpName;

    HTREEITEM hLastChild = TVI_FIRST;
    HTREEITEM hCurChild = TreeView_GetChild(hwndTreeControl, hParent);

    String sortName = lpName;

    while(hCurChild)
    {
        TVITEMEX tvi;

        TCHAR name[256];
        zero(name, 256);

        tvi.mask = TVIF_TEXT|TVIF_HANDLE|TVIF_IMAGE;
        tvi.cchTextMax = 255;
        tvi.hItem = hCurChild;
        tvi.pszText = name;
        TreeView_GetItem(hwndTreeControl, &tvi);

        if((tvi.iImage == 1) && bFolder)
            break;
        else if((tvi.iImage == 0) && !bFolder)
        {
            hLastChild = hCurChild;
            hCurChild = TreeView_GetNextSibling(hwndTreeControl, hCurChild);
            continue;
        }

        if(scmpi(name, lpName) > 0)
            break;

        hLastChild = hCurChild;
        hCurChild = TreeView_GetNextSibling(hwndTreeControl, hCurChild);
    }

    tvis.hInsertAfter = hLastChild;

    TreeView_InsertItem(hwndTreeControl, &tvis);

    traceOut;
}


void PrefabBrowser::GetItemPath(HTREEITEM hItem, String &path, BOOL bFullPath)
{
    traceIn(PrefabBrowser::GetItemPath);

    HTREEITEM hParent = TreeView_GetParent(hwndTreeControl, hItem);

    if(hParent)
    {
        GetItemPath(hParent, path, bFullPath);

        TVITEMEX tvi;

        TCHAR name[256];
        zero(name, 256);

        tvi.mask = TVIF_TEXT|TVIF_HANDLE;
        tvi.cchTextMax = 255;
        tvi.hItem = hItem;
        tvi.pszText = name;
        TreeView_GetItem(hwndTreeControl, &tvi);

        if(!bFullPath && path.IsEmpty())
            path << name;
        else
            path << TEXT("/") << name;
    }
    else if(bFullPath)
        path << TEXT("data/") << curModule << TEXT("/prefabs");

    traceOut;
}

HTREEITEM PrefabBrowser::GetCurrentDir(HTREEITEM hItem, String &path, BOOL bFullPath)
{
    traceIn(PrefabBrowser::GetCurrentDir);

    HTREEITEM hParent = hItem ? TreeView_GetParent(hwndTreeControl, hItem) : NULL;

    if(hParent)
    {
        GetItemPath(hParent, path, bFullPath);

        TVITEMEX tvi;

        TCHAR name[256];
        zero(name, 256);

        tvi.mask = TVIF_TEXT|TVIF_HANDLE|TVIF_IMAGE;
        tvi.cchTextMax = 255;
        tvi.hItem = hItem;
        tvi.pszText = name;
        TreeView_GetItem(hwndTreeControl, &tvi);

        if(tvi.iImage == 0)
        {
            if(!bFullPath && path.IsEmpty())
                path << name;
            else
                path << TEXT("/") << name;
        }
    }
    else if(bFullPath)
        path << TEXT("data/") << curModule << TEXT("/prefabs");

    return IsItemDirectory(hItem) ? hItem : hParent;

    traceOut;
}

BOOL PrefabBrowser::IsItemDirectory(HTREEITEM hItem)
{
    traceIn(PrefabBrowser::IsItemDirectory);

    if(!hItem)
        return TRUE;

    TVITEMEX tvi;
    tvi.mask = TVIF_HANDLE|TVIF_IMAGE;
    tvi.hItem = hItem;
    TreeView_GetItem(hwndTreeControl, &tvi);

    return tvi.iImage == 0;

    traceOut;
}


void PrefabBrowser::SetCurrentModule()
{
    traceIn(PrefabBrowser::SetCurrentModule);

    String strModuleDir;
    strModuleDir << TEXT("data/") << curModule << TEXT("/prefabs");
    if(!OSFileExists(strModuleDir + TEXT("/*.*")))
        RemoveDirectory(strModuleDir);

    //--------------------------------

    int id = SendMessage(hwndModuleList, CB_GETCURSEL, 0, 0);
    curModule.SetLength(SendMessage(hwndModuleList, CB_GETLBTEXTLEN, id, 0));
    SendMessage(hwndModuleList, CB_GETLBTEXT, id, (LPARAM)curModule.Array());

    String curDir;
    curDir << TEXT("data/") << curModule << TEXT("/prefabs/");

    TreeView_DeleteAllItems(hwndTreeControl);
    ProcessDirectory(curDir, NULL, curModule);
    TreeView_Expand(hwndTreeControl, TreeView_GetRoot(hwndTreeControl), TVE_EXPAND);

    DeleteMesh();
    hCurTreeItem = NULL;
    curMesh = NULL;

    UpdateMeshView();

    traceOut;
}

void PrefabBrowser::SetCurrentPrefab(bool bResetCurrent)
{
    traceIn(PrefabBrowser::SetCurrentPrefab);

    HTREEITEM hItem = TreeView_GetSelection(hwndTreeControl);
    if(!bResetCurrent && (hItem == hCurTreeItem))
        return;

    //--------------------------------

    if(!hItem)
        return;

    TCHAR name[256];
    zero(name, 256);

    TVITEM tvi;
    tvi.mask = TVIF_TEXT|TVIF_IMAGE;
    tvi.cchTextMax = 255;
    tvi.hItem = hItem;
    tvi.pszText = name;
    TreeView_GetItem(hwndTreeControl, &tvi);

    if(tvi.iImage == 0)
        return;

    String strPrefabPath;
    strPrefabPath << curModule << TEXT(":");

    String strPrefab;
    GetItemPath(hItem, strPrefab, FALSE);
    strPrefabPath << strPrefab;

    strPrefabPath << TEXT(".pfb");

    Engine::ConvertResourceName(strPrefabPath, TEXT("prefabs"), strPrefabPath);

    ConfigFile prefabInfo;
    if(!prefabInfo.Open(strPrefabPath))
            return;

    //--------------------------------

    String strTempMeshName;
    strTempMeshName = prefabInfo.GetString(TEXT("Prefab"), TEXT("ModelFile"));

    hCurTreeItem = hItem;

    String strMeshPath;
    if(!Engine::ConvertResourceName(strTempMeshName, TEXT("models"), strMeshPath))
        return;

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(strMeshPath, ofd);
    if(!hFind) return;

    OSFindClose(hFind);

    //--------------------------------

    levelInfo->selectedPrefab.Clear();

    strPrefabName = name;
    if(levelInfo)
    {
        String prefabPath;
        GetItemPath(hItem, prefabPath, FALSE);

        levelInfo->selectedPrefab = curModule + TEXT(":") + prefabPath + TEXT(".pfb");
    }

    strMeshName = strTempMeshName;
    DeleteMesh();

    hFind = OSFindFirstFile(GetPathWithoutExtension(strMeshPath) << TEXT(".xan"), ofd);
    if(hFind) OSFindClose(hFind);

    bHasAnimation = (hFind != NULL);

    strAnimationName = prefabInfo.GetString(TEXT("Prefab"), TEXT("DefaultAnimation"));

    prefabLightmapRes = prefabInfo.GetInt(TEXT("Prefab"), TEXT("LightmapRes"), 128);

    //--------------------------------

    GS = prefabView;
    RM = prefabManager;

    curMesh = (bHasAnimation) ? new SkinMesh : new Mesh;
    curMesh->LoadMesh(strMeshName);

    prefabInfo.GetStringList(TEXT("Prefab"), TEXT("Materials"), MaterialNames);

    MeshMaterials.SetSize(curMesh->DefaultMaterialList.Num());
    for(int i=0; i<MeshMaterials.Num(); i++)
    {
        Material *& material = MeshMaterials[i];

        CTSTR lpName;
        if(i >= MaterialNames.Num())
        {
            material = defaultMaterial;
            continue;
        }
        else
            lpName = MaterialNames[i];

        String resourcePath;
        if(!Engine::ConvertResourceName(lpName, TEXT("materials"), resourcePath))
        {
            material = defaultMaterial;
            continue;
        }

        hFind = OSFindFirstFile(resourcePath, ofd);
        if(hFind)
        {
            FindClose(hFind);

            material = prefabManager->GetMaterial(lpName);
        }
        else
            material = defaultMaterial;
    }

    GS = mainGD;
    RM = mainResourceMan;
    float meshRadius = curMesh->bounds.GetDiamater()*0.5f;

    prefabWindow->viewDist = Vect2(tanf(RAD(90.0f-30.0f))*meshRadius, meshRadius).Len();

    UpdateMeshView();

    //--------------------------------

    /*if(bHasAnimation)
    {
        SkinMesh *skinMesh = (SkinMesh*)curMesh;

        HMENU hMenu = GetMenu(hwndPrefabBrowser);
        HMENU hAnimationMenu = CreatePopupMenu();

        for(int i=0; i<skinMesh->SequenceList.Num(); i++)
            AppendMenu(hAnimationMenu, MF_STRING, ID_ANIMOFFSET+i, skinMesh->SequenceList[i].name);

        AppendMenu(hMenu, MF_POPUP|MF_STRING, (UINT_PTR)hAnimationMenu, TEXT("Animation");
        DrawMenuBar(hwndPrefabBrowser);
    }*/

    //--------------------------------

    prefabInfo.Close();

    if((levelInfo->curEditMode == EditMode_Create) && levelInfo->newObject && levelInfo->newObject->IsOf(GetClass(PrefabPlacer)))
    {
        PrefabPlacer *pfp = (PrefabPlacer*)levelInfo->newObject;
        EditMode em = pfp->prevEditMode;
        PrefabType pf = pfp->prefabType;

        delete levelInfo->newObject;
        levelInfo->newObject = CreateObjectParam2(PrefabPlacer, em, pf);
    }

    traceOut;
}


void PrefabBrowser::DeleteMesh()
{
    traceIn(PrefabBrowser::DeleteMesh);

    HMENU hMenu = GetMenu(hwndPrefabBrowser);
    DeleteMenu(hMenu, 1, MF_BYPOSITION);
    DrawMenuBar(hwndPrefabBrowser);

    if(curMesh)
    {
        GS = prefabView;
        RM = prefabManager;

        delete curMesh;

        for(int i=0; i<MeshMaterials.Num(); i++)
        {
            Material *material = MeshMaterials[i];

            if(material != defaultMaterial)
                prefabManager->ReleaseMaterial(material);
        }
        MeshMaterials.Clear();

        GS = mainGD;
        RM = mainResourceMan;

        curMesh = NULL;

        MaterialNames.Clear();
    }

    traceOut;
}


void PrefabBrowser::CreatePrefab()
{
    traceIn(PrefabBrowser::CreatePrefab);

    if(editor->bAddonModule && !curModule.CompareI(editor->curWorkingModule))
    {
        String strMessage;
        strMessage << TEXT("You can only create new prefabs inside of the \"") << editor->curWorkingModule << TEXT("\" module.");
        MessageBox(hwndPrefabBrowser, strMessage, NULL, 0);
        return;
    }

    if(!meshBrowser || meshBrowser->strMeshName.IsEmpty())
    {
        MessageBox(hwndPrefabBrowser, TEXT("You must have a model selected in the model browser to create a new prefab template."), TEXT("Please choose a model to use first"), MB_OK);
        return;
    }

    if(meshBrowser->bHasAnimation)
    {
        MessageBox(hwndPrefabBrowser, TEXT("Animated Prefabs are currently unsupported."), TEXT("Please choose a model to use first"), MB_OK);
        return;
    }


    bool bYouFailJimTryAgain;

    String fullPath;
    String strDirectory;
    String newPrefabName;

    HTREEITEM hItem = TreeView_GetSelection(hwndTreeControl);
    HTREEITEM hItemDir = GetCurrentDir(hItem, strDirectory, TRUE);

    do
    {
        bYouFailJimTryAgain = false;

        if(!editor->EditBoxDialog(hwndPrefabBrowser, TEXT("Please enter the prefab name"), newPrefabName))
            return;

        fullPath.Clear() << strDirectory << TEXT("/") << newPrefabName << TEXT(".pfb");

        OSFindData ofd;
        HANDLE hFind = OSFindFirstFile(fullPath, ofd);

        if(hFind)
        {
            OSFindClose(hFind);
            if(MessageBox(hwndPrefabBrowser, String() << TEXT("\"") << newPrefabName << TEXT("\" already exists.  Please choose a different name."), TEXT("File already exists"), MB_OKCANCEL) == IDCANCEL)
                return;

            bYouFailJimTryAgain = true;
        }
    }while(bYouFailJimTryAgain);

    String strModuleDir;
    strModuleDir << TEXT("data/") << curModule << TEXT("/prefabs");
    OSCreateDirectory(strModuleDir);

    ConfigFile newPrefab;
    newPrefab.Open(fullPath, TRUE);

    newPrefab.SetString(TEXT("Prefab"), TEXT("ModelFile"), meshBrowser->strMeshName);

    for(int i=0; i<meshBrowser->curMesh->DefaultMaterialList.Num(); i++)
    {
        NameStruct &mat = meshBrowser->curMesh->DefaultMaterialList[i];
        newPrefab.AddString(TEXT("Prefab"), TEXT("Materials"), String(mat.name));
    }

    newPrefab.Close();

    InsertTreeViewItem(hItemDir, newPrefabName, FALSE);

    traceOut;
}

void PrefabBrowser::ResetView()
{
    traceIn(PrefabBrowser::ResetView);

    if(bEditMode)
    {
        for(int i=0; i<EditingControls.Num(); i++)
            DestroyWindow(EditingControls[i]);

        NumParams = 0;

        ShowWindow(hwndTreeControl, SW_SHOW);

        bEditMode = FALSE;

        SelectItem(strPrefabName);

        SetCurrentPrefab(true);
    }

    traceOut;
}


void PrefabBrowser::NewFolder()
{
    traceIn(PrefabBrowser::NewFolder);

    if(editor->bAddonModule && !curModule.CompareI(editor->curWorkingModule))
    {
        String strMessage;
        strMessage << TEXT("You can only create new resource folders inside of the \"") << editor->curWorkingModule << TEXT("\" module.");
        MessageBox(hwndPrefabBrowser, strMessage, NULL, 0);
        return;
    }

    if(bEditMode)
        return;
    String fullPath;
    String strDirectory;
    String folderName;

    HTREEITEM hItem = TreeView_GetSelection(hwndTreeControl);
    HTREEITEM hItemDir = GetCurrentDir(hItem, strDirectory, TRUE);

    bool bYouFailJimTryAgain;

    do
    {
        bYouFailJimTryAgain = false;

        editor->EditBoxDialog(hwndTreeControl, TEXT("Please enter the name of the new folder"), folderName);

        fullPath.Clear() << strDirectory << TEXT("/") << folderName;

        OSFindData ofd;
        HANDLE hFind = OSFindFirstFile(fullPath, ofd);

        if(hFind)
        {
            OSFindClose(hFind);
            if(MessageBox(hwndPrefabBrowser, String() << TEXT("\"") << folderName << TEXT("\" already exists.  Please choose a different name."), TEXT("Folder already exists"), MB_OKCANCEL) == IDCANCEL)
                return;

            bYouFailJimTryAgain = true;
        }
    }while(bYouFailJimTryAgain);

    String strModuleDir;
    strModuleDir << TEXT("data/") << curModule << TEXT("/prefabs");
    OSCreateDirectory(strModuleDir);

    OSCreateDirectory(fullPath);

    InsertTreeViewItem(hItemDir, folderName, TRUE);

    traceOut;
}

void PrefabBrowser::DeleteItem()
{
    traceIn(PrefabBrowser::DeleteItem);

    if(editor->bAddonModule && !curModule.CompareI(editor->curWorkingModule))
    {
        String strMessage;
        strMessage << TEXT("You can only delete items inside of the \"") << editor->curWorkingModule << TEXT("\" module.");
        MessageBox(hwndPrefabBrowser, strMessage, NULL, 0);
        return;
    }

    if(!hCurTreeItem)
        return;

    List<Prefab*> Prefabs;
    GetLevelPrefabs(Prefabs);
    
    String path;
    GetItemPath(hCurTreeItem, path, TRUE);
    path << TEXT(".pfb");

    if(DeleteFile(path))
    {
        TreeView_DeleteItem(hwndTreeControl, hCurTreeItem);

        for(int i=0; i<Prefabs.Num(); i++)
            DestroyObject(Prefabs[i]);

        UpdateViewports();
    }

    traceOut;
}


void PrefabBrowser::EditPrefab()
{
    traceIn(PrefabBrowser::EditPrefab);

    if(!hCurTreeItem)
        return;

    HANDLE hSelectedItem = TreeView_GetSelection(hwndTreeControl);

    if(hSelectedItem != hCurTreeItem)
        return;

    HWND hwndTemp;

    NumParams = 0;
    bEditMode = TRUE;

    ShowWindow(hwndTreeControl, SW_HIDE);

    hwndTemp = CreateWindowEx(0, TEXT("Static"), TEXT("Prefab Name:"), SS_RIGHT|WS_CHILD|WS_VISIBLE, 2, 20, 78, 18, hwndPrefabBrowser, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndName = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 82, 16, 150, 22, hwndPrefabBrowser, NULL, hinstMain, NULL);
    SendMessage(hwndName, WM_SETFONT, (WPARAM)hWindowFont, 0);
    SetFocus(hwndName);
    EditingControls << hwndName;
    if(editor->bAddonModule && !editor->curWorkingModule.CompareI(curModule))
        SendMessage(hwndName, EM_SETREADONLY, TRUE, 0);

    hwndSaveChanges = CreateWindowEx(0, TEXT("Button"), TEXT("Save"), WS_CHILD|WS_VISIBLE|WS_TABSTOP, 82, 46, 60, 20, hwndPrefabBrowser, NULL, hinstMain, NULL);
    SendMessage(hwndSaveChanges, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndSaveChanges;

    hwndClose = CreateWindowEx(0, TEXT("Button"), TEXT("Close"), WS_CHILD|WS_VISIBLE|WS_TABSTOP, 146, 46, 60, 20, hwndPrefabBrowser, NULL, hinstMain, NULL);
    SendMessage(hwndClose, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndClose;

    hwndControlContainer = CreateWindowEx(0, TEXT("XR3DPrefabControlContainer"), NULL, WS_CHILD|WS_VISIBLE, 0, 110, 300, 490, hwndPrefabBrowser, NULL, hinstMain, NULL);
    EditingControls << hwndControlContainer;

    UpdateViewports(hwndPrefabView);

    SetWindowText(hwndName, strPrefabName);

    DWORD numParams = 0;

    if(bHasAnimation)
        AddPlainEditParam(TEXT("Default Animation: "), strAnimationName);

    AddLightmapScrollerParam(TEXT("Lightmap Resolution: "), prefabLightmapRes);

    MaterialStart = NumParams;
    for(int i=0; i<MaterialNames.Num(); i++)
        AddSelectableEditParam(String(TEXT("Material ")) << long(i+1) << TEXT(": "), MaterialNames[i]);

    AdjustContainer();

    bUnsavedChanges = FALSE;

    traceOut;
}

void PrefabBrowser::DuplicatePrefab()
{
    traceIn(PrefabBrowser::DuplicatePrefab);

    if(!hCurTreeItem)
        return;

    String strDirectory, fullPath, newPrefabName;

    HTREEITEM hItem = TreeView_GetSelection(hwndTreeControl);
    HTREEITEM hItemDir = GetCurrentDir(hItem, strDirectory, TRUE);

    bool bYouFailJimTryAgain;

    do
    {
        bYouFailJimTryAgain = false;

        if(!editor->EditBoxDialog(hwndPrefabBrowser, TEXT("Please enter the duplicate prefab name"), newPrefabName))
            return;

        fullPath.Clear() << strDirectory << TEXT("/") << newPrefabName << TEXT(".pfb");

        OSFindData ofd;
        HANDLE hFind = OSFindFirstFile(fullPath, ofd);

        if(hFind)
        {
            OSFindClose(hFind);
            if(MessageBox(hwndPrefabBrowser, String() << TEXT("\"") << newPrefabName << TEXT("\" already exists.  Please choose a different name."), TEXT("File already exists"), MB_OKCANCEL) == IDCANCEL)
                return;

            bYouFailJimTryAgain = true;
        }
    }while(bYouFailJimTryAgain);

    ConfigFile newPrefab;
    newPrefab.Open(fullPath, TRUE);

    newPrefab.SetString(TEXT("Prefab"), TEXT("ModelFile"), strMeshName);

    for(int i=0; i<MaterialNames.Num(); i++)
        newPrefab.AddString(TEXT("Prefab"), TEXT("Materials"), MaterialNames[i]);

    newPrefab.Close();

    InsertTreeViewItem(hItemDir, newPrefabName, FALSE);

    traceOut;
}

void PrefabBrowser::GetLevelPrefabs(List<Prefab*> &PrefabList)
{
    traceIn(PrefabBrowser::GetLevelPrefabs);

    Entity *ent = Entity::FirstEntity();
    while(ent)
    {
        if(ent->IsOf(GetClass(Prefab)))
        {
            Prefab *pf = (Prefab*)ent;
            if(pf->prefabName.CompareI(levelInfo->selectedPrefab))
                PrefabList << pf;
        }

        ent = ent->NextEntity();
    }

    traceOut;
}


void PrefabBrowser::AddSelectableEditParam(CTSTR lpName, CTSTR defaultValue)
{
    traceIn(PrefabBrowser::AddSelectableEditParam);

    DWORD yAdjust = NumParams*52;

    HWND hwndTemp;

    hwndTemp = CreateWindowEx(0, TEXT("Static"), lpName, SS_LEFT|WS_CHILD|WS_VISIBLE, 10, 10+yAdjust, 150, 18, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);

    hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), defaultValue, WS_CHILD|WS_VISIBLE|ES_READONLY|WS_TABSTOP, 20, 32+yAdjust, 200, 22, hwndControlContainer, (HMENU)(UPARAM)(PARAM_ID+NumParams), hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);

    hwndTemp = CreateWindowEx(0, TEXT("Button"), TEXT("Apply"), WS_CHILD|WS_VISIBLE|WS_TABSTOP, 225, 32+yAdjust, 60, 22, hwndControlContainer, (HMENU)(UPARAM)(PARAMSELECT_ID+NumParams), hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);

    ++NumParams;

    traceOut;
}

void PrefabBrowser::AddLightmapScrollerParam(CTSTR lpName, int defValue)
{
    traceIn(PrefabBrowser::AddLightmapScrollerParam);

    DWORD yAdjust = NumParams*52;

    HWND hwndTemp;

    hwndTemp = CreateWindowEx(0, TEXT("Static"), lpName, SS_LEFT|WS_CHILD|WS_VISIBLE, 10, 10+yAdjust, 150, 18, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);

    HWND hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 20, 32+yAdjust, 50, 22, hwndControlContainer, (HMENU)(UPARAM)PARAMLMRES_ID, hinstMain, NULL);
    SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hWindowFont, 0);

    hwndTemp = CreateWindowEx(0,TEXT("UpDownControl"), NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 74, 32+yAdjust, 14, 22, hwndControlContainer, (HMENU)(UPARAM)PARAMLMSCROLL_ID, hinstMain, NULL);
    LinkUpDown(hwndTemp, hwndEdit);
    InitUpDownIntData(hwndTemp, defValue, 16, 1024, 2);

    ++NumParams;

    traceOut;
}

void PrefabBrowser::AddPlainEditParam(CTSTR lpName, CTSTR defaultValue)
{
    traceIn(PrefabBrowser::AddPlainEditParam);

    DWORD yAdjust = NumParams*52;

    HWND hwndTemp;

    hwndTemp = CreateWindowEx(0, TEXT("Static"), lpName, SS_LEFT|WS_CHILD|WS_VISIBLE, 10, 10+yAdjust, 150, 18, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);

    hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), defaultValue, WS_CHILD|WS_VISIBLE|WS_TABSTOP, 20, 32+yAdjust, 200, 22, hwndControlContainer, (HMENU)(UPARAM)(PARAM_ID+NumParams), hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);

    ++NumParams;

    traceOut;
}




void PrefabBrowser::UpdateMeshView()
{
    traceIn(PrefabBrowser::UpdateMeshView);

    if(prefabView && prefabManager)
    {
        GS = prefabView;
        RM = prefabManager;

        prefabView->PreRenderScene();
        prefabView->RenderScene(TRUE, 0xFF101040);
        prefabView->PostRenderScene();

        GS = mainGD;
        RM = mainResourceMan;
    }

    traceOut;
}

void PrefabBrowser::AdjustContainer()
{
    traceIn(PrefabBrowser::AdjustContainer);

    if(NumParams > 11)
    {
        SCROLLINFO si;

        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        si.nMin = 0;
        si.nMax = NumParams*26;
        si.nPage = 11*26;
        si.nPos = 0;
        si.nTrackPos = 0;

        ShowScrollBar(hwndControlContainer, SB_VERT, TRUE);
        SetScrollInfo(hwndControlContainer, SB_VERT, &si, TRUE);
    }
    else
        ShowScrollBar(hwndControlContainer, SB_VERT, FALSE);

    traceOut;
}

void PrefabBrowser::UpdateParam(int paramID, int notification)
{
    traceIn(PrefabBrowser::UpdateParam);

    if(bHasAnimation && (paramID == 0))
    {
        HWND hwndControl = GetDlgItem(hwndControlContainer, PARAM_ID+paramID);
        if(notification == EN_KILLFOCUS)
        {
            String newAnimationName;
            newAnimationName.SetLength(GetWindowTextLength(hwndControl)+1);
            GetWindowText(hwndControl, newAnimationName, newAnimationName.Length()+1);

            if(newAnimationName.CompareI(strAnimationName))
            {
                bUnsavedChanges = true;
                strAnimationName = newAnimationName;

                //todo - stuff related to changing animation here
            }
        }
    }
    else if(paramID == PARAMLMSCROLL_ID)
    {
        if(notification == JUDN_UPDATE)
            bUnsavedChanges = true;
    }

    traceOut;
}

void PrefabBrowser::ApplyItem(int paramID)
{
    traceIn(PrefabBrowser::ApplyItem);

    if(!materialEditor || !materialEditor->materialWindow->selectedMaterial)
    {
        MessageBox(hwndPrefabBrowser, TEXT("You must select a material selected in the material editor."), TEXT("Hsss!  Unable to do as asked, massster!"), MB_OK);
        return;
    }

    HWND hwndControl = GetDlgItem(hwndControlContainer, PARAM_ID+paramID);
    DWORD materialOffset = paramID-MaterialStart;

    String itemName;
    materialEditor->GetCurItemResourceName(itemName);

    if(!itemName.CompareI(MaterialNames[materialOffset]))
    {
        bUnsavedChanges = true;
        MaterialNames[materialOffset].Clear();
        MaterialNames[materialOffset] = itemName;

        SetWindowText(hwndControl, itemName);

        GS = prefabView;
        RM = prefabManager;

        if(MeshMaterials[materialOffset] != defaultMaterial)
            prefabManager->ReleaseMaterial(MeshMaterials[materialOffset]);
        MeshMaterials[materialOffset] = prefabManager->GetMaterial(itemName);

        GS = mainGD;
        RM = mainResourceMan;

        UpdateViewports(hwndPrefabView);
    }

    traceOut;
}

bool PrefabBrowser::SavePrefab()
{
    traceIn(PrefabBrowser::SavePrefab);

    DWORD i;

    TCHAR name[255];
    GetWindowText(hwndName, name, 254);

    if(!*name)
    {
        MessageBox(hwndPrefabBrowser, TEXT("You must enter a name before you can save your changes."), NULL, 0);
        return true;
    }

    if( schr(name, '\\') ||
        schr(name, '/')  ||
        schr(name, '"')  ||
        schr(name, '?')  ||
        schr(name, ':')  ||
        schr(name, '|')  ||
        schr(name, '*')  ||
        schr(name, '<')  ||
        schr(name, '>')  )
    {
        MessageBox(hwndPrefabBrowser, TEXT("Your prefab name contains invalid characters (\\ / | * ? : \" < >)"), NULL, 0);
        return false;
    }

    //-------------------------------------------------

    String baseDirectory;
    BOOL bOverride = FALSE;
    if(editor->bAddonModule && !curModule.CompareI(editor->curWorkingModule))
    {
        baseDirectory << TEXT("data/") << editor->curWorkingModule << TEXT("/override");
        OSCreateDirectory(baseDirectory);
        baseDirectory << TEXT("/") << curModule;
        OSCreateDirectory(baseDirectory);
        baseDirectory << TEXT("/prefabs");
        bOverride = TRUE;
    }
    else
        baseDirectory << TEXT("data/") << curModule << TEXT("/prefabs");
    OSCreateDirectory(baseDirectory);

    //-------------------------------------------------

    bUnsavedChanges = FALSE;

    List<Prefab*> Prefabs;
    GetLevelPrefabs(Prefabs);

    //-------------------------------------------------

    for(int i=0; i<Prefabs.Num(); i++)
        Prefabs[i]->Reinitialize();

    String path, newPath, previousPath;

    HTREEITEM hCurDir;
    bool bNewName = false;
    if(!bOverride)
    {
        hCurDir = GetCurrentDir(hCurTreeItem, path, TRUE);
        newPath << path << TEXT("/") << name << TEXT(".pfb");
        previousPath << path << TEXT("/") << strPrefabName << TEXT(".pfb");

        if(!previousPath.CompareI(newPath))
        {
            MoveFile(previousPath, newPath);
            bNewName = true;
        }
    }
    else
    {
        GetItemPath(hCurTreeItem, path, FALSE);
        newPath << baseDirectory << TEXT("/") << path << TEXT(".pfb");

        String strName = path;
        int numDirs = strName.NumTokens('/');
        String newDir = baseDirectory;
        for(int i=0; i<numDirs-1; i++)
        {
            newDir << TEXT("/") << strName.GetToken('/');
            OSCreateDirectory(newDir);
        }
    }

    //-------------------------------------------------

    ConfigFile prefabFile;
    prefabFile.Create(newPath);

    prefabFile.SetString(TEXT("Prefab"), TEXT("ModelFile"), strMeshName);

    TCHAR value[255];

    HWND hwndControl;

    if(bHasAnimation)
    {
        hwndControl = GetDlgItem(hwndControlContainer, PARAM_ID);
        GetWindowText(hwndControl, value, 254);
        prefabFile.SetString(TEXT("Prefab"), TEXT("DefaultAnimation"), value);
    }

    hwndControl = GetDlgItem(hwndControlContainer, PARAMLMSCROLL_ID);
    prefabFile.SetInt(TEXT("Prefab"), TEXT("LightmapRes"), GetUpDownInt(hwndControl));

    for(i=bHasAnimation+1; i<NumParams; i++)
    {
        hwndControl = GetDlgItem(hwndControlContainer, PARAM_ID+i);
        GetWindowText(hwndControl, value, 254);

        prefabFile.AddString(TEXT("Prefab"), TEXT("Materials"), value);
    }

    prefabFile.Close();

    //-------------------------------------------------

    if(bNewName)
    {
        TreeView_DeleteItem(hwndTreeControl, hCurTreeItem);
        strPrefabName = name;

        InsertTreeViewItem(hCurDir, strPrefabName, false);

        for(int i=0; i<Prefabs.Num(); i++)
            Prefabs[i]->prefabName = strPrefabName;
    }

    for(i=0; i<Prefabs.Num(); i++)
        Prefabs[i]->Reinitialize();

    UpdateViewports();

    return true;

    traceOut;
}

void PrefabBrowser::SelectItem(CTSTR lpItem)
{
    traceIn(PrefabBrowser::SelectItem);

    String strNewItem = lpItem;

    DWORD numTokens = strNewItem.NumTokens('/');

    HTREEITEM hLastItem = TreeView_GetRoot(hwndTreeControl);

    int curItemID = 0;

    while(numTokens)
    {
        TreeView_Expand(hwndTreeControl, hLastItem, TVE_EXPAND);
        String curItem = strNewItem.GetToken(curItemID++, '/');
        --numTokens;

        HTREEITEM hCurItem = TreeView_GetChild(hwndTreeControl, hLastItem);
        do
        {
            TCHAR name[256];
            zero(name, 256);

            TVITEMEX tvi;
            tvi.mask = TVIF_TEXT|TVIF_HANDLE;
            tvi.cchTextMax = 255;
            tvi.hItem = hCurItem;
            tvi.pszText = name;
            TreeView_GetItem(hwndTreeControl, &tvi);

            if(curItem.CompareI(name))
            {
                hLastItem = hCurItem;
                break;
            }
        }while(hCurItem = TreeView_GetNextItem(hwndTreeControl, hCurItem, TVGN_NEXT));
    }

    SetFocus(hwndTreeControl);
    TreeView_Select(hwndTreeControl, hLastItem, TVGN_CARET);

    hCurTreeItem = hLastItem;

    traceOut;
}

BOOL PrefabBrowser::RenameItem(HTREEITEM hItem, CTSTR lpText)
{
    traceIn(PrefabBrowser::RenameItem);

    if(editor->bAddonModule && !curModule.CompareI(editor->curWorkingModule))
    {
        String strMessage;
        strMessage << TEXT("You can only rename items inside of the \"") << editor->curWorkingModule << TEXT("\" module.");
        MessageBox(hwndPrefabBrowser, strMessage, NULL, 0);
        return FALSE;
    }

    if(!lpText)
        return FALSE;

    if( schr(lpText, '\\') ||
        schr(lpText, '/')  ||
        schr(lpText, '*')  ||
        schr(lpText, '?')  ||
        schr(lpText, ':')  ||
        schr(lpText, '"')  ||
        schr(lpText, '<')  ||
        schr(lpText, '>')  )
    {
        return FALSE;
    }

    HTREEITEM hParent = TreeView_GetParent(hwndTreeControl, hItem);

    List<Prefab*> PrefabList;
    prefabBrowser->GetLevelPrefabs(PrefabList);

    TVITEM tvi;
    tvi.mask = TVIF_IMAGE;
    tvi.hItem = hItem;
    TreeView_GetItem(hwndTreeControl, &tvi);

    bool bFolder = (tvi.iImage == 0);

    String path, parentPath;
    prefabBrowser->GetItemPath(hItem, path, TRUE);
    prefabBrowser->GetItemPath(hParent, parentPath, TRUE);

    parentPath << TEXT("/") << lpText;

    if(!bFolder)
    {
        path << TEXT(".pfb");
        parentPath << TEXT(".pfb");
    }

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(parentPath, ofd);
    if(hFind)
    {
        OSFindClose(hFind);
        return FALSE;
    }

    if(!bFolder)
    {
        if(MessageBox(hwndPrefabBrowser, TEXT("Make sure you've removed this prefab from any other levels before renaming.\r\n\r\nAre you sure you want to rename?"), TEXT("Renaming item"), MB_YESNO) == IDYES)
        {
            if(!MoveFile(path, parentPath))
                return FALSE;
            else 
            {
                if(!bFolder)
                {
                    for(int i=0; i<PrefabList.Num(); i++)
                        PrefabList[i]->prefabName = lpText;
                }
                return TRUE;
            }
        }
    }
    else
    {
        if(MessageBox(hwndPrefabBrowser, TEXT("Make sure you've removed any prefabs of this folder from any levels before renaming.\r\n\r\nAre you sure you want to rename?"), TEXT("Renaming item"), MB_YESNO) == IDYES)
        {
            if(!MoveFile(path, parentPath))
                return FALSE;
            else
                return TRUE;
        }
    }
    return FALSE;

    traceOut;
}



LRESULT WINAPI PrefabBrowserProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_CLOSE:
            if(prefabBrowser->bEditMode && prefabBrowser->bUnsavedChanges)
            {
                int query = MessageBox(prefabBrowser->hwndPrefabBrowser, TEXT("You have unsaved changes.\r\n\r\nDo you want to save your changes?"), TEXT("Just wondering."), MB_YESNOCANCEL);

                if(query == IDCANCEL)
                    break;
                else if(query == IDYES)
                {
                    if(!prefabBrowser->SavePrefab())
                        break;
                }
            }
            delete prefabBrowser;
            break;

        case WM_COMMAND:
            {
                WORD commandID = LOWORD(wParam);

                /*if((commandID >= 5000) && (commandID < 6000))
                {
                    SkinMesh *mesh = (SkinMesh*)prefabBrowser->curMesh;

                    mesh->
                    break;
                }*/

                if(prefabBrowser->bEditMode)
                {
                    if((HWND)lParam == prefabBrowser->hwndSaveChanges)
                    {
                        prefabBrowser->SavePrefab();
                        break;
                    }
                    else if((HWND)lParam == prefabBrowser->hwndClose)
                    {
                        if(prefabBrowser->bEditMode && prefabBrowser->bUnsavedChanges)
                        {
                            int query = MessageBox(prefabBrowser->hwndPrefabBrowser, TEXT("You have unsaved changes.\r\n\r\nDo you want to save your changes?"), TEXT("Just wondering."), MB_YESNOCANCEL);

                            if(query == IDCANCEL)
                                break;
                            else if(query == IDYES)
                            {
                                if(!prefabBrowser->SavePrefab())
                                    break;
                            }
                        }

                        prefabBrowser->ResetView();
                        break;
                    }
                    else if((HWND)lParam == prefabBrowser->hwndName)
                    {
                        TCHAR name[255];
                        GetWindowText(prefabBrowser->hwndName, name, 254);

                        if(!prefabBrowser->strPrefabName.CompareI(name))
                            prefabBrowser->bUnsavedChanges = TRUE;

                        break;
                    }
                }

                if((HWND)lParam == prefabBrowser->hwndModuleList)
                {
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        if(prefabBrowser->bEditMode && prefabBrowser->bUnsavedChanges)
                        {
                            int query = MessageBox(prefabBrowser->hwndPrefabBrowser, TEXT("You have unsaved changes.\r\n\r\nDo you want to save your changes?"), TEXT("Just wondering."), MB_YESNOCANCEL);

                            if(query == IDCANCEL)
                            {
                                SendMessage(prefabBrowser->hwndModuleList, CB_SETCURSEL, SendMessage(prefabBrowser->hwndModuleList, CB_FINDSTRING, -1, (LPARAM)prefabBrowser->curModule.Array()), 0);
                                break;
                            }
                            else if(query == IDYES)
                            {
                                if(!prefabBrowser->SavePrefab())
                                {
                                    SendMessage(prefabBrowser->hwndModuleList, CB_SETCURSEL, SendMessage(prefabBrowser->hwndModuleList, CB_FINDSTRING, -1, (LPARAM)prefabBrowser->curModule.Array()), 0);
                                    break;
                                }
                            }
                        }

                        prefabBrowser->SetCurrentModule();
                        break;
                    }
                }

                switch(commandID)
                {
                    case ID_PREFAB_NEWPREFABTEMPLATE:
                        prefabBrowser->CreatePrefab();
                        break;

                    case ID_PREFAB_CLOSE:
                        CloseWindow(hwnd);
                        break;

                    case ID_PREFAB_NEWFOLDER:
                        prefabBrowser->NewFolder();
                        break;

                    case ID_PREFAB_DUPLICATETEMPLATE:
                        prefabBrowser->DuplicatePrefab();
                        break;

                    case ID_PREFAB_DELETE:
                        prefabBrowser->DeleteItem();
                        break;
                }

                break;
            }

        case WM_NOTIFY:
            if(prefabBrowser)
            {
                NMTREEVIEW *info = (NMTREEVIEW*)lParam;

                if(info->hdr.hwndFrom == prefabBrowser->hwndTreeControl)
                {
                    HWND hwndTreeControl = info->hdr.hwndFrom;

                    if(info->hdr.code == TVN_SELCHANGED)
                        prefabBrowser->SetCurrentPrefab();
                    else if(info->hdr.code == TVN_BEGINLABELEDIT)
                    {
                        HTREEITEM hRoot = TreeView_GetRoot(hwndTreeControl);
                        HTREEITEM hItem = TreeView_GetSelection(hwndTreeControl);

                        if(hRoot == hItem)
                            return TRUE;
                    }
                    else if(info->hdr.code == TVN_ENDLABELEDIT)
                    {
                        NMTVDISPINFO *dispInfo = (NMTVDISPINFO*)lParam;
                        HTREEITEM hItem   = dispInfo->item.hItem;
                        TSTR lpText = dispInfo->item.pszText;

                        return prefabBrowser->RenameItem(hItem, lpText);
                    }
                    else if(info->hdr.code == NM_DBLCLK)
                    {
                        prefabBrowser->EditPrefab();
                    }
                }
            }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT WINAPI PrefabViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_PAINT)
    {
        RECT rect;
        PAINTSTRUCT ps;

        if(GetUpdateRect(hwnd, &rect, FALSE))
        {
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            if(prefabBrowser && !bErrorMessage)
                prefabBrowser->UpdateMeshView();
        }
        return 0;
    }
    else if(message == WM_UPDATEVIEWPORTS)
    {
        if(prefabBrowser)
            prefabBrowser->UpdateMeshView();

        return 0;
    }

    return CallWindowProc((FARPROC)MainWndProc, hwnd, message, wParam, lParam);
}

LRESULT WINAPI PrefabControlContainerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_COMMAND:
            {
                int controlID = LOWORD(wParam);

                if(prefabBrowser->bEditMode)
                {
                    if(controlID < PARAMMAX_ID)
                    {
                        if(controlID >= PARAMLMRES_ID)
                        {
                            prefabBrowser->UpdateParam(PARAMLMSCROLL_ID, HIWORD(wParam));
                        }
                        else if(controlID >= PARAMSELECT_ID)
                        {
                            prefabBrowser->ApplyItem(controlID-PARAMSELECT_ID);
                            break;
                        }
                        else if(controlID >= PARAM_ID)
                        {
                            prefabBrowser->UpdateParam(controlID-PARAM_ID, HIWORD(wParam));
                            break;
                        }
                    }

                    if((HWND)lParam == prefabBrowser->hwndSaveChanges)
                    {
                        prefabBrowser->SavePrefab();
                    }
                    else if((HWND)lParam == prefabBrowser->hwndClose)
                    {
                        if(prefabBrowser->bEditMode && prefabBrowser->bUnsavedChanges)
                        {
                            int query = MessageBox(prefabBrowser->hwndPrefabBrowser, TEXT("You have unsaved changes.\r\n\r\nDo you want to save your changes?"), TEXT("Just wondering."), MB_YESNOCANCEL);

                            if(query == IDCANCEL)
                                break;
                            if(query == IDYES)
                            {
                                if(!prefabBrowser->SavePrefab())
                                    break;
                            }
                        }

                        prefabBrowser->ResetView();
                    }
                }
            }
            break;

        case WM_VSCROLL:
            {
                if(!lParam)
                {
                    SCROLLINFO si;
                    si.cbSize = sizeof(si);
                    si.fMask  = SIF_ALL;
                    GetScrollInfo(hwnd, SB_VERT, &si);

                    int curPos = si.nPos;

                    switch(LOWORD(wParam))
                    {
                        case SB_TOP:
                            si.nPos = si.nMax; break;
                        case SB_BOTTOM:
                            si.nPos = si.nMin; break;
                        case SB_LINEUP:
                            si.nPos -= 26; break;
                        case SB_LINEDOWN:
                            si.nPos += 26; break;
                        case SB_PAGEUP:
                            si.nPos -= si.nPage; break;
                        case SB_PAGEDOWN:
                            si.nPos += si.nPage; break;
                        case SB_THUMBTRACK:
                            si.nPos = si.nTrackPos; break;
                    }

                    if(si.nPos > int(si.nMax-si.nPage))
                        si.nPos = (si.nMax-si.nPage);
                    else if(si.nPos < si.nMin)
                        si.nPos = si.nMin;

                    if(si.nPos == curPos)
                        return 0;

                    long offset = (si.nPos-curPos);

                    RECT parentPos;
                    GetWindowRect(hwnd, &parentPos);

                    HWND hwndChild = GetWindow(hwnd, GW_CHILD);

                    do
                    {
                        RECT childPos;
                        GetWindowRect(hwndChild, &childPos);

                        childPos.left -= parentPos.left;
                        childPos.top  -= parentPos.top;

                        childPos.top -= offset;

                        SetWindowPos(hwndChild, NULL, childPos.left, childPos.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);

                    }while(hwndChild = GetWindow(hwndChild, GW_HWNDNEXT));

                    //redraw the windows all at once
                    RedrawWindow(hwnd, NULL, NULL, RDW_ERASE|RDW_INVALIDATE);

                    si.fMask = SIF_POS;
                    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
                }
            }
            break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}


void PrefabWindow::Init()
{
    traceIn(PrefabWindow::Init);

    Super::Init();

    SetSystem(prefabBrowser->prefabView);

    Pos.Set(0.0f, 0.0f);
    Size.Set(600.0f, 600.0f);

    viewDist = 20.0f;
    tilt = 20.0f;
    spin = 220.0f;

    traceOut;
}

void PrefabWindow::Render()
{
    traceInFast(PrefabWindow::Render);

    Mesh *mesh = prefabBrowser->curMesh;

    if(!mesh)
        return;

    Perspective(60.0f, Size.x/Size.y, 0.4f, 1000.0f);

    Vect eyePos(0.0f, 0.0f, viewDist);
    Vect lightPos = Vect(-6.0f, 6.0f, 13.0f).Norm()*viewDist;

    MatrixPush();
    MatrixIdentity();
    MatrixTranslate(0.0f, 0.0f, -viewDist);

        Quat adjustRot;
        adjustRot = AxisAngle(1.0f, 0.0f, 0.0f, RAD(tilt));
        adjustRot *= AxisAngle(0.0f, 1.0f, 0.0f, RAD(spin));

        LoadVertexBuffer(mesh->VertBuffer);
        LoadIndexBuffer(mesh->IdxBuffer);

        Matrix rotMatrix(adjustRot);
        eyePos.TransformVector(rotMatrix);
        lightPos.TransformVector(rotMatrix);

        MatrixRotate(adjustRot);
        MatrixTranslate(-mesh->bounds.GetCenter());

        ColorWriteEnable(1, 1, 1, 1);
        EnableBlending(FALSE);
        BlendFunction(GS_BLEND_ONE, GS_BLEND_ONE);

        EnableDepthTest(TRUE);
        DepthWriteEnable(TRUE);
        DepthFunction(GS_LESS);

        HANDLE hTechnique;

        for(int i=0; i<prefabBrowser->MeshMaterials.Num(); i++)
        {
            Material *material = prefabBrowser->MeshMaterials[i];
            DrawSection &section = mesh->SectionList[i];

            Effect *effect = material->effect;

            hTechnique = effect->GetTechnique(TEXT("InitialPass"));
            if(hTechnique)
            {
                effect->BeginTechnique(hTechnique);
                if(effect->BeginPassByName(TEXT("Normal")))
                {
                    if(material->LoadParameters())
                        Draw(GS_TRIANGLES, 0, (section.startFace*3), (section.numFaces*3));

                    effect->EndPass();
                }
                effect->EndTechnique();
            }
        }

        EnableBlending(TRUE);
        DepthWriteEnable(FALSE);
        DepthFunction(GS_LEQUAL);

        for(int i=0; i<prefabBrowser->MeshMaterials.Num(); i++)
        {
            Material *material = prefabBrowser->MeshMaterials[i];
            DrawSection &section = mesh->SectionList[i];

            Effect *effect = material->effect;

            hTechnique = effect->GetTechnique(TEXT("PointLight"));

            if(hTechnique)
            {
                HANDLE hParam;

                hParam = effect->GetParameterByName(TEXT("lightRange"));
                effect->SetFloat(hParam, 1.0f);

                hParam = effect->GetParameterByName(TEXT("lightColor"));
                effect->SetColor(hParam, INVALID);

                hParam = effect->GetParameterByName(TEXT("attenuationMap"));
                effect->SetTexture(hParam, prefabBrowser->blankAttenuation);

                hParam = effect->GetParameterByName(TEXT("eyePos"));
                effect->SetVector(hParam, eyePos);

                hParam = effect->GetParameterByName(TEXT("lightPos"));
                effect->SetVector(hParam, lightPos);

                effect->BeginTechnique(hTechnique);
                if(effect->BeginPassByName(TEXT("Normal")))
                {
                    if(material->LoadParameters())
                        Draw(GS_TRIANGLES, 0, (section.startFace*3), (section.numFaces*3));

                    effect->EndPass();
                }
                effect->EndTechnique();
            }
        }

    MatrixPop();

    traceOutFast;
}

void PrefabWindow::MouseDown(DWORD button)
{
    traceIn(PrefabWindow::MouseDown);

    switch(button)
    {
        case MOUSE_LEFTBUTTON:
            buttonsDown |= 1;
            break;

        case MOUSE_RIGHTBUTTON:
            buttonsDown |= 2;
            break;
    }

    if(buttonsDown != 3)
    {
        ShowCursor(FALSE);

        GetSystem()->GetLocalMousePos(lastMouseX, lastMouseY);

        bKeepFocus = TRUE;
        SetCapture(prefabBrowser->hwndPrefabView);
    }

    traceOut;
}

void PrefabWindow::MouseUp(DWORD button)
{
    traceIn(PrefabWindow::MouseUp);

    switch(button)
    {
        case MOUSE_LEFTBUTTON:
            buttonsDown &= ~1;
            break;

        case MOUSE_RIGHTBUTTON:
            buttonsDown &= ~2;
            break;
    }

    if(!buttonsDown)
    {
        bKeepFocus = FALSE;
        ReleaseCapture();

        ShowCursor(TRUE);
    }

    traceOut;
}

void PrefabWindow::MouseMove(int x, int y, short x_offset, short y_offset)
{
    traceIn(PrefabWindow::MouseMove);

    if(bIgnoreNextMove)
    {
        bIgnoreNextMove = FALSE;
        return;
    }

    float xMove = float(x_offset)*0.1f;
    float yMove = float(y_offset)*0.1f;

    if(buttonsDown)
    {
        if(buttonsDown != 3)
            tilt += yMove;
        else 
            viewDist += yMove;

        spin += xMove;

        if(spin > 360.0f) spin -= 360.0f;
        if(spin < 0.0f) spin += 360.0f;

        if(tilt > 360.0f) tilt -= 360.0f;
        if(tilt < 0.0f) tilt += 360.0f;

        if(viewDist < 0.5f) viewDist = 0.5f;

        prefabBrowser->UpdateMeshView();

        GetSystem()->SetLocalMousePos(lastMouseX, lastMouseY);
        bIgnoreNextMove = TRUE;
    }

    traceOut;
}

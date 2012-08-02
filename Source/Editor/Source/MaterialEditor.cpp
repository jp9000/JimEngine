/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  MaterialEditor.cpp

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


BOOL WINAPI AddFolderProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI MaterialEditorProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI MaterialViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI MaterialControlContainerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


MaterialEditor *materialEditor = NULL;

#define PARAM_ID            5000
#define PARAMBROWSE_ID      5100
#define PARAMSCROLLER_ID    5200
#define PARAMMAX_ID         5300

TCHAR lpLastMaterialModule[250] = TEXT("");
TCHAR lpLastMaterialDir[250] = TEXT("");



void MaterialEditor::RegisterWindowClasses()
{
    traceIn(MaterialEditor::RegisterWindowClasses);

    WNDCLASS wc;
    zero(&wc, sizeof(wc));

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)MaterialEditorProc;
    wc.hInstance = hinstMain;
    wc.hIcon = LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MATERIALSMENU);
    wc.lpszClassName = TEXT("MaterialEditor");

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the material editor window class"));

    wc.lpfnWndProc = (WNDPROC)MaterialViewProc;
    wc.lpszMenuName = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = TEXT("XR3DMaterialView");
    wc.cbWndExtra = sizeof(LPVOID);

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the material view class"));

    wc.style = CS_PARENTDC;
    wc.lpfnWndProc = (WNDPROC)MaterialControlContainerProc;
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
    wc.lpszClassName = TEXT("XR3DMaterialControlContainer");
    wc.cbWndExtra = 0;

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the material view class"));

    traceOut;
}

MaterialEditor::MaterialEditor()
{
    traceIn(MaterialEditor::RegisterWindowClasses);

    materialEditor = this;

    int borderXSize = 600;
    int borderYSize = 500;

    borderXSize += GetSystemMetrics(SM_CXDLGFRAME)*2;
    borderXSize += GetSystemMetrics(SM_CXVSCROLL);

    borderYSize += GetSystemMetrics(SM_CYDLGFRAME)*2;
    borderYSize += GetSystemMetrics(SM_CYCAPTION);
    borderYSize += GetSystemMetrics(SM_CYMENU);

    hwndMaterialEditor = CreateWindow(TEXT("MaterialEditor"), TEXT("Material Browser"),
                                      WS_VISIBLE|WS_DLGFRAME|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      borderXSize, borderYSize,
                                      hwndEditor, NULL, hinstMain, NULL);

    //-----------------------------------------

    hwndScrollBar = CreateWindowEx(0, TEXT("SCROLLBAR"), NULL,
                                   WS_CHILD|SBS_VERT|SBS_LEFTALIGN|WS_DISABLED,
                                   600, 0, 100, 500,
                                   hwndMaterialEditor, NULL, hinstMain, NULL);

    //-----------------------------------------

    hwndMaterialView = (HANDLE)CreateWindow(TEXT("XR3DMaterialView"),
            NULL, WS_VISIBLE|WS_CHILD,
            150, 0, 450, 500,
            hwndMaterialEditor, NULL, hinstMain, NULL);

    String strGraphicsSystem = AppConfig->GetString(TEXT("Engine"), TEXT("GraphicsSystem"));
    materialView = (GraphicsSystem*)CreateFactoryObject(strGraphicsSystem, FALSE);

    if(!materialView)
        CrashError(TEXT("Bad Graphics System: %s"), strGraphicsSystem);

    materialView->InitializeDevice(hwndMaterialView);
    materialView->InitializeObject();
    materialView->SetSize(450, 500);

    //-----------------------------------------

    materialWindow = CreateObject(MaterialWindow);

    hwndModuleList = CreateWindow(TEXT("combobox"), NULL,
                                  WS_VISIBLE|WS_CHILDWINDOW|CBS_DROPDOWNLIST|CBS_SORT,
                                  0, 0, 150, 190,
                                  hwndMaterialEditor, NULL, hinstMain, NULL);
    SendMessage(hwndModuleList, WM_SETFONT, (WPARAM)hWindowFont, 0);

    RECT sizeOfWorthlessComboBox;
    GetWindowRect(hwndModuleList, &sizeOfWorthlessComboBox);
    int comboBoxSize = sizeOfWorthlessComboBox.bottom-sizeOfWorthlessComboBox.top;

    hwndTreeControl = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, NULL,
                                     WS_VISIBLE|WS_CHILDWINDOW|TVS_EDITLABELS|TVS_HASLINES|TVS_LINESATROOT|TVS_SHOWSELALWAYS|TVS_HASBUTTONS,
                                     0, comboBoxSize, 150, 500,
                                     hwndMaterialEditor, NULL, hinstMain, NULL);

    List<CTSTR> ActiveModules;
    Engine::GetGameModules(ActiveModules);

    curModule = lpLastMaterialModule;

    BOOL bFoundLastModule = FALSE;

    for(int i=0; i<ActiveModules.Num(); i++)
    {
        CTSTR lpModule = ActiveModules[i];
        /*OSFindData ofd;
        String strCurMatDir;
        strCurMatDir << TEXT("data/") << ofdModule.fileName << TEXT("/materials");

        HANDLE hFindMaterials = OSFindFirstFile(strCurMatDir, ofd);
        if(!hFindMaterials)
            continue;

        OSFindClose(hFindMaterials);

        if(!ofd.bDirectory)
            continue;*/

        int id = SendMessage(hwndModuleList, CB_ADDSTRING, 0, (LPARAM)lpModule);

        if(curModule.IsValid() && scmpi(lpModule, curModule) == 0)
        {
            bFoundLastModule = TRUE;
            SendMessage(hwndModuleList, CB_SETCURSEL, id, 0);
        }
    }

    if(!bFoundLastModule)
    {
        curModule = TEXT("Base");
        SendMessage(hwndModuleList, CB_SETCURSEL, SendMessage(hwndModuleList, CB_FINDSTRING, -1, (LPARAM)TEXT("Base")), 0);
    }

    String strMatDir;
    strMatDir << TEXT("data/") << curModule << TEXT("/materials/");
    ProcessDirectory(strMatDir, NULL, curModule);

    traceOut;
}

MaterialEditor::~MaterialEditor()
{
    traceIn(MaterialEditor::MaterialEditor);

    String strOldDir;
    strOldDir << TEXT("data/") << curModule << TEXT("/materials");
    if(!OSFileExists(strOldDir + TEXT("/*.*")))
        RemoveDirectory(strOldDir);

    //--------------------------------------

    String curPath;
    GetCurItemPath(curPath, TRUE);
    curPath << TEXT("/");

    scpy_n(lpLastMaterialModule, curModule, 240);
    scpy_n(lpLastMaterialDir, curPath, 240);
    DestroyObject(materialWindow);

    CleanupEditMode();

    DestroyObject(materialView);
    DestroyWindow(hwndMaterialEditor);

    materialEditor = NULL;

    traceOut;
}

void MaterialEditor::UpdateMaterialView()
{
    traceIn(MaterialEditor::UpdateMaterialView);

    if(engine->bBlockViewUpdates)
        return;

    materialView->PreRenderScene();
    materialView->RenderScene(TRUE, 0xFF303030);
    materialView->PostRenderScene();

    traceOut;
}


void MaterialEditor::ProcessDirectory(TSTR lpBaseDir, HTREEITEM hParent, TSTR lpName)
{
    traceIn(MaterialEditor::ProcessDirectory);

    if((scmpi(lpName, TEXT("Editor")) == 0) || (scmpi(lpName, TEXT(".svn")) == 0))
        return;

    HTREEITEM hItem;

    static TVINSERTSTRUCT tvis;

    zero(&tvis, sizeof(TVINSERTSTRUCT));
    tvis.hParent          = hParent;
    tvis.hInsertAfter     = TVI_SORT;
    tvis.itemex.mask      = TVIF_TEXT;
    tvis.itemex.pszText   = lpName;
    hItem = TreeView_InsertItem(hwndTreeControl, &tvis);

    String strFindPath = lpBaseDir;
    strFindPath << TEXT("*.*");

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(strFindPath, ofd);

    if(hFind)
    {
        do
        {
            if(ofd.bDirectory)
            {
                strFindPath.Clear() << lpBaseDir << ofd.fileName << TEXT("/");
                ProcessDirectory(strFindPath, hItem, ofd.fileName);
            }
        }while(OSFindNextFile(hFind, ofd));

        OSFindClose(hFind);
    }

    TreeView_Expand(hwndTreeControl, hParent, TVE_EXPAND);

    if( (!lpLastMaterialDir[0] && !hParent) || 
        (lpLastMaterialDir[0] && scmpi(lpBaseDir, lpLastMaterialDir) == 0) )
    {
        TreeView_SelectItem(hwndTreeControl, hItem);
    }

    traceOut;
}

void MaterialEditor::GetItemPath(HTREEITEM hItem, String &path, BOOL bFullPath)
{
    traceIn(MaterialEditor::GetItemPath);

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
        path << TEXT("data/") << curModule << TEXT("/materials");

    traceOut;
}

void MaterialEditor::GetCurItemResourceName(String &name)
{
    traceIn(MaterialEditor::GetCurItemResourceName);

    name << curModule << TEXT(":");

    String relativePath;
    GetItemPath(TreeView_GetSelection(hwndTreeControl), relativePath, FALSE);

    if(relativePath.IsValid())
        name << relativePath << TEXT("/");

    name << materialWindow->selectedMaterial->strName << TEXT(".mtl");

    traceOut;
}


void MaterialEditor::EditMaterial(CTSTR lpMaterial)
{
    traceIn(MaterialEditor::EditMaterial);

    DWORD i;

    String strResourceName;
    GetCurItemResourceName(strResourceName);

    bEditingExisting = TRUE;

    CreateNewMaterial();

    SetWindowText(hwndName, lpMaterial);

    bUnsavedChanges = FALSE;

    strPreviousName = lpMaterial;

    editLevelMaterial = RM->UsingMaterial(strResourceName);
    if(editLevelMaterial)
    {
        SendMessage(hwndName, EM_SETREADONLY, TRUE, 0);
        EnableWindow(hwndEffectList, FALSE);
        BackupParameters.CopyList(editLevelMaterial->Params);

        for(i=0; i<BackupParameters.Num(); i++)
        {
            MaterialParameter &param = editLevelMaterial->Params[i];

            if(param.type == Parameter_Texture)
                RM->AddTextureRef(*(BaseTexture**)param.data);
        }
    }

    //-------------------------------------------

    String path;
    path << curModule << TEXT(":");

    String strItemPath;
    GetCurItemPath(strItemPath, FALSE);
    if(strItemPath.IsValid())
        path << strItemPath << TEXT("/");
    path << lpMaterial << TEXT(".mtl");

    Engine::ConvertResourceName(path, TEXT("materials"), path, editor->bAddonModule);

    ConfigFile materialFile;

    materialFile.Open(path);

    String strEffect = materialFile.GetString(TEXT("Material"), TEXT("Effect"));
    editMaterial->restitution = materialFile.GetFloat(TEXT("Material"), TEXT("Restitution"), 0.0f);
    editMaterial->friction = materialFile.GetFloat(TEXT("Material"), TEXT("Friction"), 0.5f);
    editMaterial->strSoundSoft = materialFile.GetString(TEXT("Material"), TEXT("SoftSound"));
    editMaterial->strSoundHard = materialFile.GetString(TEXT("Material"), TEXT("HardSound"));

    SetUpDownFloat(hwndRestitution,  editMaterial->restitution);
    SetUpDownFloat(hwndFriction,     editMaterial->friction);
    SetWindowText(hwndSoftCollision, editMaterial->strSoundSoft);
    SetWindowText(hwndHardCollision, editMaterial->strSoundHard);

    BOOL bFoundEffect = FALSE;

    for(i=0; i<materialWindow->Effects.Num(); i++)
    {
        if(materialWindow->EffectNames[i]->CompareI(strEffect))
        {
            bFoundEffect = TRUE;
            break;
        }
    }

    assert(bFoundEffect);
    if(!bFoundEffect)
        return;

    Effect *effect;
    materialWindow->curEffect =
    editMaterial->effect =
    effect = materialWindow->Effects[i];

    SendMessage(hwndEffectList, CB_SETCURSEL, i, 0);

    DWORD curParamID = 0;

    HANDLE hCurParam;
    while(hCurParam = effect->GetParameter(curParamID++))
    {
        EffectParameterInfo paramInfo;
        effect->GetEffectParameterInfo(hCurParam, paramInfo);

        if(paramInfo.propertyType != EffectProperty_None)
        {
            String strTextName;
            strTextName << paramInfo.fullName << TEXT(":");

            if(paramInfo.propertyType == EffectProperty_Texture)
            {
                String strDefault = materialFile.GetString(TEXT("Parameters"), paramInfo.name);

                if((scmp(paramInfo.name, TEXT("diffuseTexture")) == 0))
                {
                    EnableWindow(hwndDrawFlat, TRUE);
                    materialWindow->Items[0]->mainTextureParam = NumParams;
                }

                AddTextureParam(hCurParam, strTextName, strDefault);
            }
            else if(paramInfo.propertyType == EffectProperty_Color)
            {
                DWORD color = Vect_to_RGB(materialFile.GetColor(TEXT("Parameters"), paramInfo.name));
                AddColorParam(hCurParam, strTextName, color);
            }
            else if(paramInfo.propertyType == EffectProperty_Float)
            {
                float value = materialFile.GetFloat(TEXT("Parameters"), paramInfo.name);
                AddFloatScrollerParam(hCurParam, strTextName, paramInfo.fMin, paramInfo.fMax, paramInfo.fInc, value);
            }
            //else if(strType.CompareI("Bool"))
            //{
            //}
        }
    }

    AdjustContainer();

    int dwDrawType = materialFile.GetInt(TEXT("Material"), TEXT("DrawType"));

    materialWindow->Items[0]->dwDrawType = dwDrawType;

    SendMessage(hwndDrawSphere, BM_SETCHECK, (dwDrawType == DRAWMATERIAL_SPHERE) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(hwndDrawBox,    BM_SETCHECK, (dwDrawType == DRAWMATERIAL_BOX)    ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(hwndDrawFlat,   BM_SETCHECK, (dwDrawType == DRAWMATERIAL_FLAT)   ? BST_CHECKED : BST_UNCHECKED, 0);

    materialFile.Close();

    traceOut;
}


void MaterialEditor::CreateNewMaterial()
{
    traceIn(MaterialEditor::CreateNewMaterial);

    if(!bEditingExisting && editor->bAddonModule && !curModule.CompareI(editor->curWorkingModule))
    {
        String strMessage;
        strMessage << TEXT("You can only create new materials inside of the \"") << editor->curWorkingModule << TEXT("\" module.");
        MessageBox(hwndMaterialEditor, strMessage, NULL, 0);
        return;
    }

    HWND hwndTemp;

    if(bEditMode)
    {
        if(bUnsavedChanges)
        {
            int query = MessageBox(hwndMaterialEditor, TEXT("You have unsaved changes.\r\n\r\nDo you want to save your changes?"), TEXT("Just wondering."), MB_YESNOCANCEL);

            if(query == IDCANCEL)
                return;
            else if(query == IDYES)
            {
                if(!SaveMaterial())
                    return;
            }
        }

        CleanupEditMode();
    }

    ShowWindow(hwndScrollBar, SW_HIDE);

    NumParams = 0;
    yAdjust = 0;
    numMainControls = 0;
    numExtraParams = 0;

    editMaterial = materialWindow->SetEditMode(DRAWMATERIAL_SPHERE);

    hEditPath = TreeView_GetSelection(hwndTreeControl);

    bEditMode = TRUE;

    materialView->SetSize(200, 200);
    SetWindowPos(hwndMaterialView, NULL, 0, 0, 200, 200, SWP_NOMOVE);

    hwndTemp = CreateWindowEx(0, TEXT("Static"), TEXT("Material Name:"), SS_RIGHT|WS_CHILD|WS_VISIBLE, 350, 60, 80, 18, hwndMaterialEditor, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndTemp = CreateWindowEx(0, TEXT("Static"), TEXT("Effect Shader:"), SS_RIGHT|WS_CHILD|WS_VISIBLE, 350, 90, 80, 18, hwndMaterialEditor, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndName = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL, WS_CHILD|WS_VISIBLE, 432, 56, 150, 22, hwndMaterialEditor, NULL, hinstMain, NULL);
    SendMessage(hwndName, WM_SETFONT, (WPARAM)hWindowFont, 0);
    SetFocus(hwndName);
    EditingControls << hwndName;
    if(editor->bAddonModule && !curModule.CompareI(editor->curWorkingModule))
        SendMessage(hwndName, EM_SETREADONLY, TRUE, 0);

    hwndEffectList = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("ComboBox"), NULL, WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST, 432, 86, 150, 300, hwndMaterialEditor, NULL, hinstMain, NULL);
    SendMessage(hwndEffectList, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndEffectList;

    hwndSaveChanges = CreateWindowEx(0, TEXT("Button"), TEXT("Save"), WS_CHILD|WS_VISIBLE, 432, 116, 60, 20, hwndMaterialEditor, NULL, hinstMain, NULL);
    SendMessage(hwndSaveChanges, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndSaveChanges;

    hwndClose = CreateWindowEx(0, TEXT("Button"), TEXT("Close"), WS_CHILD|WS_VISIBLE, 496, 116, 60, 20, hwndMaterialEditor, NULL, hinstMain, NULL);
    SendMessage(hwndClose, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndClose;

    hwndDrawSphere = CreateWindowEx(0, TEXT("Button"), TEXT("Sphere"), WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON|WS_GROUP, 432, 144, 200, 18, hwndMaterialEditor, NULL, hinstMain, NULL);
    SendMessage(hwndDrawSphere, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndDrawSphere;

    hwndDrawBox = CreateWindowEx(0, TEXT("Button"), TEXT("Box"), WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON, 432, 162, 200, 18, hwndMaterialEditor, NULL, hinstMain, NULL);
    SendMessage(hwndDrawBox, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndDrawBox;

    hwndDrawFlat = CreateWindowEx(0, TEXT("Button"), TEXT("Flat"), WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON|WS_DISABLED, 432, 180, 200, 18, hwndMaterialEditor, NULL, hinstMain, NULL);
    SendMessage(hwndDrawFlat, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndDrawFlat;

    //-----------------------------------------

    hwndOuterContainer = CreateWindowEx(0, TEXT("XR3DMaterialControlContainer"), NULL, WS_CHILD|WS_VISIBLE, 150, 210, 466, 290, hwndMaterialEditor, NULL, hinstMain, NULL);
    EditingControls << hwndOuterContainer;

    hwndControlContainer = CreateWindowEx(0, TEXT("XR3DMaterialControlContainer"), NULL, WS_CHILD|WS_VISIBLE, 0, 0, 466, 290, hwndOuterContainer, NULL, hinstMain, NULL);
    EditingControls << hwndControlContainer;

    //-----------------------------------------
    // Restitution
    hwndTemp = CreateWindowEx(0, TEXT("Static"), TEXT("Resitution (Bounce):"), SS_RIGHT|WS_CHILD|WS_VISIBLE, 5, 4+yAdjust, 150, 18, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL, WS_CHILD|WS_VISIBLE, 160, yAdjust, 70, 22, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndRestitution = CreateWindowEx(0, TEXT("UpDownControl"), NULL, WS_CHILD|WS_VISIBLE, 232, yAdjust, 20, 22, hwndControlContainer, NULL, hinstMain, NULL);
    InitUpDownFloatData(hwndRestitution, 0.0f, 0.0f, 1.0f, 0.1f);
    LinkUpDown(hwndRestitution, hwndTemp);
    EditingControls << hwndRestitution;

    yAdjust += 26;
    numExtraParams++;

    //-----------------------------------------
    // Friction
    hwndTemp = CreateWindowEx(0, TEXT("Static"), TEXT("Friction:"), SS_RIGHT|WS_CHILD|WS_VISIBLE, 5, 4+yAdjust, 150, 18, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL, WS_CHILD|WS_VISIBLE, 160, yAdjust, 70, 22, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndFriction = CreateWindowEx(0, TEXT("UpDownControl"), NULL, WS_CHILD|WS_VISIBLE, 232, yAdjust, 20, 22, hwndControlContainer, NULL, hinstMain, NULL);
    InitUpDownFloatData(hwndFriction, 0.5f, 0.0f, 5.0f, 0.1f);
    LinkUpDown(hwndFriction, hwndTemp);
    EditingControls << hwndFriction;

    yAdjust += 26;
    numExtraParams++;

    //-----------------------------------------
    // Soft Collision Sound
    hwndTemp = CreateWindowEx(0, TEXT("Static"), TEXT("Soft Collision Sound:"), SS_RIGHT|WS_CHILD|WS_VISIBLE, 5, 4+yAdjust, 150, 18, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndSoftCollision = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL, WS_CHILD|WS_VISIBLE|ES_READONLY, 160, yAdjust, 150, 22, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndSoftCollision, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndSoftCollision;

    hwndSoftCollisionBrowse = CreateWindowEx(0, TEXT("Button"), TEXT("Browse..."), WS_CHILD|WS_VISIBLE, 313, yAdjust, 65, 22, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndSoftCollisionBrowse, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndSoftCollisionBrowse;

    hwndSoftCollisionClear = CreateWindowEx(0, TEXT("Button"), TEXT("Clear"), WS_CHILD|WS_VISIBLE, 380, yAdjust, 45, 22, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndSoftCollisionClear, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndSoftCollisionClear;

    yAdjust += 26;
    numExtraParams++;

    //-----------------------------------------
    // Hard Collision Sound
    hwndTemp = CreateWindowEx(0, TEXT("Static"), TEXT("Hard Collision Sound:"), SS_RIGHT|WS_CHILD|WS_VISIBLE, 5, 4+yAdjust, 150, 18, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndHardCollision = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL, WS_CHILD|WS_VISIBLE|ES_READONLY, 160, yAdjust, 150, 22, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndHardCollision, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndHardCollision;

    hwndHardCollisionBrowse = CreateWindowEx(0, TEXT("Button"), TEXT("Browse..."), WS_CHILD|WS_VISIBLE, 313, yAdjust, 65, 22, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndHardCollisionBrowse, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndHardCollisionBrowse;

    hwndHardCollisionClear = CreateWindowEx(0, TEXT("Button"), TEXT("Clear"), WS_CHILD|WS_VISIBLE, 380, yAdjust, 45, 22, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndHardCollisionClear, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndHardCollisionClear;

    yAdjust += 26;
    numExtraParams++;

    numMainControls = EditingControls.Num();

    //-----------------------------------------

    SendMessage(hwndDrawSphere, BM_SETCHECK, BST_CHECKED, 0);

    for(DWORD i=0; i<materialWindow->EffectNames.Num(); i++)
        SendMessage(hwndEffectList, CB_ADDSTRING, 0, (LPARAM)(CTSTR)*materialWindow->EffectNames[i]);

    UpdateViewports(hwndMaterialView);

    bUnsavedChanges = TRUE;

    traceOut;
}


void MaterialEditor::AddFolder(CTSTR lpFolder)
{
    traceIn(MaterialEditor::AddFolder);

    if(editor->bAddonModule && !curModule.CompareI(editor->curWorkingModule))
    {
        String strMessage;
        strMessage << TEXT("You can only create new folders inside of the \"") << editor->curWorkingModule << TEXT("\" module.");
        MessageBox(hwndMaterialEditor, strMessage, NULL, 0);
        return;
    }

    String baseDirectory;
    baseDirectory << TEXT("data/") << curModule << TEXT("/materials");
    OSCreateDirectory(baseDirectory);

    HTREEITEM hParent;
    if(hCurItem)
        hParent = hCurItem;
    else
        hParent = TreeView_GetSelection(hwndTreeControl);

    String path;
    GetItemPath(hParent, path);

    path << TEXT("/") << lpFolder;

    CreateDirectory(path, NULL);

    TVINSERTSTRUCT tvis;
    zero(&tvis, sizeof(TVINSERTSTRUCT));
    tvis.hParent          = hParent;
    tvis.hInsertAfter     = TVI_SORT;
    tvis.itemex.mask      = TVIF_TEXT;
    tvis.itemex.pszText   = (TSTR)lpFolder;
    HTREEITEM hItem = TreeView_InsertItem(hwndTreeControl, &tvis);
    TreeView_SelectItem(hwndTreeControl, hItem);

    traceOut;
}

void MaterialEditor::RemoveFolder()
{
    traceIn(MaterialEditor::RemoveFolder);

    if(editor->bAddonModule && !curModule.CompareI(editor->curWorkingModule))
    {
        String strMessage;
        strMessage << TEXT("You can only remove folders inside of the \"") << editor->curWorkingModule << TEXT("\" module.");
        MessageBox(hwndMaterialEditor, strMessage, NULL, 0);
        return;
    }

    HTREEITEM hItem;
    if(hCurItem)
        hItem = hCurItem;
    else
        hItem = TreeView_GetSelection(hwndTreeControl);

    HTREEITEM hParent = TreeView_GetParent(hwndTreeControl, hItem);

    if(hParent)
    {
        if(MessageBox(hwndMaterialEditor, TEXT("Removing a folder will remove all the materials and subfolders.  Are you really sure you want to do this?"), TEXT("You know..  Just making sure."), MB_YESNO) == IDYES)
        {
            String path;
            GetItemPath(hItem, path);

            KillDirectory(path);
            TreeView_SelectItem(hwndTreeControl, hParent);
            TreeView_DeleteItem(hwndTreeControl, hItem);
        }
    }

    hCurItem = NULL;

    traceOut;
}

void MaterialEditor::ChangeDirectory()
{
    traceIn(MaterialEditor::ChangeDirectory);

    if(bEditMode)
    {
        CleanupEditMode();
        bEditMode = FALSE;

        materialView->SetSize(450, 500);
        SetWindowPos(hwndMaterialView, NULL, 0, 0, 450, 500, SWP_NOMOVE);
    }

    ShowWindow(hwndScrollBar, SW_SHOW);

    HTREEITEM hItem = TreeView_GetSelection(hwndTreeControl);

    String path;
    GetItemPath(hItem, path, FALSE);

    materialWindow->SetDirectory(path);

    UpdateViewports(hwndMaterialView);

    traceOut;
}


void MaterialEditor::KillDirectory(CTSTR lpDir)
{
    traceIn(MaterialEditor::KillDirectory);

    String strFindPath;

    strFindPath << lpDir << TEXT("/*.*");

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

                strFindPath.Clear() << lpDir << TEXT("/") << wfd.cFileName;
                KillDirectory(strFindPath);
            }
            else
            {
                strFindPath.Clear() << lpDir << TEXT("/") << wfd.cFileName;
                DeleteFile(strFindPath);
            }
        }while(FindNextFile(hFind, &wfd));

        FindClose(hFind);
    }

    RemoveDirectory(lpDir);

    traceOut;
}


void MaterialEditor::ChangeEffect()
{
    traceIn(MaterialEditor::ChangeEffect);

    CleanupEditMode(FALSE);
    editMaterial = materialWindow->SetEditMode(DRAWMATERIAL_SPHERE);

    int effectID = SendMessage(hwndEffectList, CB_GETCURSEL, 0, 0);

    assert(effectID != CB_ERR);
    if(effectID == CB_ERR)
        return;

    Effect *effect;
    materialWindow->curEffect =
    editMaterial->effect =
    effect = materialWindow->Effects[effectID];

    DWORD curParamID = 0;

    yAdjust = numExtraParams*26;
    NumParams = 0;

    HANDLE hCurParam;
    while(hCurParam = effect->GetParameter(curParamID++))
    {
        EffectParameterInfo paramInfo;
        effect->GetEffectParameterInfo(hCurParam, paramInfo);

        if(paramInfo.propertyType != EffectProperty_None)
        {
            String strName;
            strName << paramInfo.fullName << TEXT(":");

            if(paramInfo.propertyType == EffectProperty_Texture)
            {
                if(scmp(paramInfo.name, TEXT("diffuseTexture")) == 0)
                {
                    materialWindow->Items[0]->mainTextureParam = NumParams;
                    EnableWindow(hwndDrawFlat, TRUE);
                }

                AddTextureParam(hCurParam, strName, paramInfo.strDefaultTexture);
            }
            else if(paramInfo.propertyType == EffectProperty_Color)
            {
                DWORD color = 0xFFFFFFFF;
                effect->GetDefaultColor3(hCurParam, color);
                AddColorParam(hCurParam, strName, color);
            }
            else if(paramInfo.propertyType == EffectProperty_Float)
            {
                float value = 0.0f;
                effect->GetDefaultFloat(hCurParam, value);
                AddFloatScrollerParam(hCurParam, strName, paramInfo.fMin, paramInfo.fMax, paramInfo.fInc, value);
            }
            //else if(strType.CompareI("Bool"))
            //{
            //}
        }
    }

    AdjustContainer();

    traceOut;
}

void MaterialEditor::AddTextureParam(HANDLE hParam, CTSTR lpName, CTSTR defaultValue)
{
    traceIn(MaterialEditor::AddTextureParam);

    HWND hwndTemp;

    hwndTemp = CreateWindowEx(0, TEXT("Static"), lpName, SS_RIGHT|WS_CHILD|WS_VISIBLE, 5, 4+yAdjust, 150, 18, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), defaultValue, WS_CHILD|WS_VISIBLE|ES_READONLY, 160, yAdjust, 200, 22, hwndControlContainer, (HMENU)(UPARAM)(PARAM_ID+NumParams), hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndTemp = CreateWindowEx(0, TEXT("Button"), TEXT("Browse..."), WS_CHILD|WS_VISIBLE, 365, yAdjust, 60, 22, hwndControlContainer, (HMENU)(UPARAM)(PARAMBROWSE_ID+NumParams), hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    MaterialParameter materialParam;
    zero(&materialParam, sizeof(materialParam));
    materialParam.handle = hParam;
    materialParam.type   = Parameter_Texture;

    editMaterial->Params << materialParam;

    if(defaultValue && *defaultValue)
    {
        String path;
        Engine::ConvertResourceName(defaultValue, TEXT("textures"), path);
        Texture *texture = materialView->CreateTextureFromFile(path, TRUE);

        *(Texture**)editMaterial->Params[NumParams].data = texture;
        editMaterial->effect->SetTexture(hParam, texture);
    }

    ++NumParams;
    yAdjust += 26;

    traceOut;
}

void MaterialEditor::AddColorParam(HANDLE hParam, CTSTR lpName, DWORD defaultValue)
{
    traceIn(MaterialEditor::AddColorParam);

    HWND hwndTemp;

    hwndTemp = CreateWindowEx(0, TEXT("Static"), lpName, SS_RIGHT|WS_CHILD|WS_VISIBLE, 5, 4+yAdjust, 150, 18, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndTemp = CreateWindowEx(0, TEXT("ColorControl"), NULL, WS_CHILD|WS_VISIBLE, 160, yAdjust, 40, 22, hwndControlContainer, (HMENU)(UPARAM)(PARAM_ID+NumParams), hinstMain, NULL);
    CCSetColor(hwndTemp, defaultValue);
    EditingControls << hwndTemp;

    Color3 vColor = RGB_to_Vect(defaultValue);

    MaterialParameter materialParam;
    zero(&materialParam, sizeof(materialParam));
    materialParam.handle = hParam;
    materialParam.type   = Parameter_Vector3;
    mcpy(materialParam.data, &vColor, sizeof(Vect));

    editMaterial->Params << materialParam;

    ++NumParams;
    yAdjust += 26;

    traceOut;
}

void MaterialEditor::AddFloatParam(HANDLE hParam, CTSTR lpName, float defaultValue)
{
    traceIn(MaterialEditor::AddFloatParam);

    HWND hwndTemp;

    hwndTemp = CreateWindowEx(0, TEXT("Static"), lpName, SS_RIGHT|WS_CHILD|WS_VISIBLE, 5, 4+yAdjust, 150, 18, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL, WS_CHILD|WS_VISIBLE, 160, yAdjust, 50, 22, hwndControlContainer, (HMENU)(UPARAM)(PARAM_ID+NumParams), hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    MaterialParameter materialParam;
    zero(&materialParam, sizeof(materialParam));
    materialParam.handle = hParam;
    materialParam.type   = Parameter_Float;
    *(float*)materialParam.data = defaultValue;

    editMaterial->Params << materialParam;

    ++NumParams;
    yAdjust += 26;

    traceOut;
}

void MaterialEditor::AddFloatScrollerParam(HANDLE hParam, CTSTR lpName, float minValue, float maxValue, float precision, float defaultValue)
{
    traceIn(MaterialEditor::AddFloatScrollerParam);

    HWND hwndTemp;

    if((defaultValue < minValue) || (defaultValue > maxValue))
        defaultValue = minValue;

    EffectParameterInfo paramInfo;
    editMaterial->effect->GetEffectParameterInfo(hParam, paramInfo);

    TCHAR floatText[64];
    tsprintf_s(floatText, 63, TEXT("%g"), defaultValue);

    hwndTemp = CreateWindowEx(0, TEXT("Static"), lpName, SS_RIGHT|WS_CHILD|WS_VISIBLE, 5, 4+yAdjust, 150, 18, hwndControlContainer, NULL, hinstMain, NULL);
    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndTemp;

    HWND hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), floatText, WS_CHILD|WS_VISIBLE, 160, yAdjust, 70, 22, hwndControlContainer, (HMENU)(UPARAM)(PARAM_ID+NumParams), hinstMain, NULL);
    SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hWindowFont, 0);
    EditingControls << hwndEdit;

    hwndTemp = CreateWindowEx(0, TEXT("UpDownControl"), NULL, WS_CHILD|WS_VISIBLE, 232, yAdjust, 20, 22, hwndControlContainer, (HMENU)(UPARAM)(PARAMSCROLLER_ID+NumParams), hinstMain, NULL);
    DWORD floong = GetLastError();
    InitUpDownFloatData(hwndTemp, defaultValue, minValue, maxValue, precision);
    LinkUpDown(hwndTemp, hwndEdit);
    EditingControls << hwndTemp;

    MaterialParameter materialParam;
    zero(&materialParam, sizeof(materialParam));
    materialParam.handle = hParam;
    materialParam.type   = Parameter_Float;
    *(float*)materialParam.data = defaultValue*paramInfo.fMul;

    editMaterial->Params << materialParam;

    ++NumParams;
    yAdjust += 26;

    traceOut;
}

void MaterialEditor::AdjustScroller(int paramID)
{
    traceIn(MaterialEditor::AdjustScroller);

    HWND hwndScroller = GetDlgItem(hwndControlContainer, PARAMSCROLLER_ID+paramID);
    HANDLE hParam = editMaterial->Params[paramID].handle;

    EffectParameterInfo paramInfo;
    editMaterial->effect->GetEffectParameterInfo(hParam, paramInfo);

    float fPosition = GetUpDownFloat(hwndScroller);

    *(float*)editMaterial->Params[paramID].data = fPosition*paramInfo.fMul;

    bUnsavedChanges = TRUE;

    UpdateViewports(hwndMaterialView);

    if(bEditingExisting && editLevelMaterial)
    {
        *(float*)editLevelMaterial->Params[paramID].data = fPosition*paramInfo.fMul;
        UpdateViewports();
    }

    traceOut;
}

void MaterialEditor::TextureBrowse(int paramID)
{
    traceIn(MaterialEditor::TextureBrowse);

    HWND hwndEdit = GetDlgItem(hwndControlContainer, PARAM_ID+paramID);
    HANDLE hParam = editMaterial->Params[paramID].handle;

    TCHAR lpFile[256];
    OPENFILENAME ofn;
    zero(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndMaterialEditor;
    ofn.lpstrFile = lpFile;
    ofn.nMaxFile = 255;

    String initialDir;
    initialDir << TEXT(".\\data\\") << curModule << TEXT("\\textures");

    if(!OSFileExists(initialDir))
        initialDir = TEXT(".\\data\\");
    else 
        initialDir << TEXT("\\");

    ofn.lpstrFile[0] = 0;
    ofn.lpstrFilter = TEXT("All Formats\0*.bmp;*.dds;*.dib;*.jpg;*.png;*.tga\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    TCHAR curDirectory[256];
    GetCurrentDirectory(255, curDirectory);

    if(GetOpenFileName(&ofn))
    {
        SetCurrentDirectory(curDirectory);

        String strCurDirectory;
        strCurDirectory << curDirectory << TEXT("/data/");
        strCurDirectory.FindReplace(TEXT("\\"), TEXT("/"));

        String strDirectory = lpFile;
        strDirectory.FindReplace(TEXT("\\"), TEXT("/"));

        StringList possibleModules;
        GetPossibleModules(possibleModules);

        String strMatchingModule;
        String strTruncatedFilePath;
        for(int i=0; i<possibleModules.Num(); i++)
        {
            String strModuleDir;
            strModuleDir << strCurDirectory << possibleModules[i] << TEXT("/textures/");

            if(scmpi_n(strModuleDir, strDirectory, strModuleDir.Length()) == 0)
            {
                strMatchingModule = possibleModules[i];
                strTruncatedFilePath = strDirectory.Array()+strModuleDir.Length();
                break;
            }
        }

        if(strMatchingModule.IsValid())
        {
            String fileName;
            String filePath;
            fileName << strMatchingModule << TEXT(":") << strTruncatedFilePath;
            Engine::ConvertResourceName(fileName, TEXT("textures"), filePath);

            SetWindowText(hwndEdit, fileName);

            MaterialParameter &param = editMaterial->Params[paramID];

            Texture *curTexture = (*(Texture**)param.data);
            delete curTexture;

            Texture *texture = materialView->CreateTextureFromFile(filePath, TRUE);

            *(Texture**)param.data = texture;

            bUnsavedChanges = TRUE;

            UpdateViewports(hwndMaterialView);

            if(bEditingExisting && editLevelMaterial)
            {
                MaterialParameter &levelParam = editLevelMaterial->Params[paramID];
                RM->ReleaseTexture(*(BaseTexture**)levelParam.data);
                *(Texture**)levelParam.data = GetTexture(fileName);
                UpdateViewports();
            }
        }
        else
            MessageBox(hwndMaterialEditor, TEXT("The chosen texture must reside within the 'textures' directory of a module (example data\\MyModule\\textures) or a subdirectory of that path."), NULL, 0);
    }
    else
        SetCurrentDirectory(curDirectory);

    traceOut;
}

void MaterialEditor::SoundBrowse(HWND hwndEdit)
{
    traceIn(MaterialEditor::SoundBrowse);

    TCHAR lpFile[256];
    OPENFILENAME ofn;
    zero(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndMaterialEditor;
    ofn.lpstrFile = lpFile;
    ofn.nMaxFile = 255;

    String initialDir;
    initialDir << TEXT(".\\data\\") << curModule << TEXT("\\sounds");

    if(!OSFileExists(initialDir))
        initialDir = TEXT(".\\data\\");
    else 
        initialDir << TEXT("\\");

    ofn.lpstrFile[0] = 0;
    ofn.lpstrFilter = TEXT("Sound Files\0*.wav\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    TCHAR curDirectory[256];
    GetCurrentDirectory(255, curDirectory);

    if(GetOpenFileName(&ofn))
    {
        SetCurrentDirectory(curDirectory);

        String strCurDirectory;
        strCurDirectory << curDirectory << TEXT("/data/");
        strCurDirectory.FindReplace(TEXT("\\"), TEXT("/"));

        String strDirectory = lpFile;
        strDirectory.FindReplace(TEXT("\\"), TEXT("/"));

        StringList possibleModules;
        GetPossibleModules(possibleModules);

        String strMatchingModule;
        String strTruncatedFilePath;
        for(int i=0; i<possibleModules.Num(); i++)
        {
            String strModuleDir;
            strModuleDir << strCurDirectory << possibleModules[i] << TEXT("/sounds/");

            if(scmpi_n(strModuleDir, strDirectory, strModuleDir.Length()) == 0)
            {
                strMatchingModule = possibleModules[i];
                strTruncatedFilePath = strDirectory.Array()+strModuleDir.Length();
                break;
            }
        }

        if(strMatchingModule.IsValid())
        {
            String fileName;
            String filePath;
            fileName << strMatchingModule << TEXT(":") << strTruncatedFilePath;
            Engine::ConvertResourceName(fileName, TEXT("sounds"), filePath);

            SetWindowText(hwndEdit, fileName);

            bUnsavedChanges = TRUE;

            if(hwndEdit == hwndSoftCollision)
                editMaterial->strSoundSoft = fileName;
            else
                editMaterial->strSoundHard = fileName;
        }
        else
            MessageBox(hwndMaterialEditor, TEXT("The chosen sound must reside within the 'sounds' directory of a module (example data\\MyModule\\sounds) or a subdirectory of that path."), NULL, 0);
    }
    else
        SetCurrentDirectory(curDirectory);

    traceOut;
}

void MaterialEditor::SoundClear(HWND hwndEdit)
{
    traceIn(MaterialEditor::SoundClear);

    SetWindowText(hwndEdit, NULL);

    if(hwndEdit == hwndSoftCollision)
    {
        if(editMaterial->strSoundSoft.IsValid())
        {
            editMaterial->strSoundSoft.Clear();
            bUnsavedChanges = TRUE;
        }
    }
    else
    {
        if(editMaterial->strSoundHard.IsValid())
        {
            editMaterial->strSoundHard.Clear();
            bUnsavedChanges = TRUE;
        }
    }

    traceOut;
}

void MaterialEditor::UpdateParam(int paramID, int notification)
{
    traceIn(MaterialEditor::UpdateParam);

    HWND hwndControl = GetDlgItem(hwndControlContainer, PARAM_ID+paramID);

    if(paramID >= editMaterial->Params.Num())
        return;

    HANDLE hParam = editMaterial->Params[paramID].handle;

    EffectParameterInfo paramInfo;
    editMaterial->effect->GetEffectParameterInfo(hParam, paramInfo);

    if(paramInfo.propertyType == EffectProperty_Float)
    {
        if(notification == EN_KILLFOCUS)
        {
            TCHAR floatText[64];
            GetWindowText(hwndControl, floatText, 63);
            float fValue = tstof(floatText);

            if(!ValidFloatString(floatText) || !_finite(fValue)) //if not a valid number
            {
                editMaterial->effect->GetFloat(hParam, fValue);

                tsprintf_s(floatText, 63, TEXT("%g"), fValue);
                SetWindowText(hwndControl, floatText);
            }
            else
            {
                *(float*)editMaterial->Params[paramID].data = fValue;

                bUnsavedChanges = TRUE;

                UpdateViewports(hwndMaterialView);

                if(bEditingExisting && editLevelMaterial)
                {
                    *(float*)editLevelMaterial->Params[paramID].data = fValue;
                    UpdateViewports();
                }
            }
        }
    }
    else if(paramInfo.propertyType == EffectProperty_Color)
    {
        if(notification == CCN_CHANGED)
        {
            Color3 vColor = RGB_to_Vect(CCGetColor(hwndControl));

            mcpy(editMaterial->Params[paramID].data, vColor, sizeof(Vect));
            bUnsavedChanges = TRUE;

            UpdateViewports(hwndMaterialView);

            if(bEditingExisting && editLevelMaterial)
            {
                mcpy(editLevelMaterial->Params[paramID].data, vColor.ptr, sizeof(Vect));
                UpdateViewports();
            }
        }
    }

    traceOut;
}


void MaterialEditor::CleanupEditMode(BOOL bRemoveAllControls)
{
    traceIn(MaterialEditor::CleanupEditMode);

    DWORD i;

    if(bEditMode)
    {
        if(bRemoveAllControls)
        {
            for(i=0; i<EditingControls.Num(); i++)
                DestroyWindow(EditingControls[i]);
            EditingControls.Clear();
        }
        else
        {
            for(i=numMainControls; i<EditingControls.Num(); i++)
                DestroyWindow(EditingControls[i]);
            EditingControls.SetSize(numMainControls);

            EnableWindow(hwndDrawFlat, FALSE);
        }

        materialWindow->CleanupItems();

        if(editLevelMaterial)
        {
            for(i=0; i<BackupParameters.Num(); i++)
            {
                MaterialParameter &oldParam = BackupParameters[i];
                MaterialParameter &newParam = editLevelMaterial->Params[i];

                if(oldParam.type == Parameter_Texture)
                {
                    if(bUnsavedChanges)
                    {
                        RM->ReleaseTexture(*(BaseTexture**)newParam.data);
                        *(BaseTexture**)newParam.data = *(BaseTexture**)oldParam.data;
                    }
                    else
                        RM->ReleaseTexture(*(BaseTexture**)oldParam.data);
                }
                else if(bUnsavedChanges)
                    mcpy(newParam.data, oldParam.data, 256);
            }

            BackupParameters.Clear();

            UpdateViewports();
        }
    }

    traceOut;
}


BOOL MaterialEditor::SaveMaterial()
{
    traceIn(MaterialEditor::SaveMaterial);

    DWORD i;

    TCHAR name[255];
    GetWindowText(hwndName, name, 254);

    if(!*name)
    {
        MessageBox(hwndMaterialEditor, TEXT("You must enter a name before you can save your changes."), NULL, 0);
        return FALSE;
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
        MessageBox(hwndMaterialEditor, TEXT("Your material name contains invalid characters (\\ / | * ? : \" < >)"), NULL, 0);
        return FALSE;
    }

    //-------------------------------------------------

    int effectID = SendMessage(hwndEffectList, CB_GETCURSEL, 0, 0);

    assert(effectID != CB_ERR);
    if(effectID == CB_ERR)
    {
        MessageBox(hwndMaterialEditor, TEXT("You must select an effect for your material before you can save your changes"), NULL, 0);
        return FALSE;
    }

    if(!materialWindow->bCanDraw)
    {
        MessageBox(hwndMaterialEditor, TEXT("You must assign textures to your object before you can save your changes."), NULL, 0);
        return FALSE;
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
        baseDirectory << TEXT("/materials");
        bOverride = TRUE;
    }
    else
        baseDirectory << TEXT("data/") << curModule << TEXT("/materials");
    OSCreateDirectory(baseDirectory);

    //-------------------------------------------------

    bUnsavedChanges = FALSE;

    String strEffect;
    int nameSize = SendMessage(hwndEffectList, CB_GETLBTEXTLEN, effectID, 0);

    strEffect.SetLength(nameSize);
    SendMessage(hwndEffectList, CB_GETLBTEXT, effectID, (LPARAM)(TSTR)strEffect);

    //-------------------------------------------------

    String path;
    GetItemPath(hEditPath, path);
    path << TEXT("/") << name << TEXT(".mtl");

    if(bEditingExisting)
    {
        if(!bOverride)
        {
            String previousPath;
            GetItemPath(hEditPath, previousPath);
            previousPath << TEXT("/") << strPreviousName << TEXT(".mtl");

            MoveFile(previousPath, path);
        }

        if(editLevelMaterial)
        {
            for(i=0; i<BackupParameters.Num(); i++)
            {
                MaterialParameter &oldParam = BackupParameters[i];
                MaterialParameter &newParam = editLevelMaterial->Params[i];

                if(oldParam.type == Parameter_Texture)
                {
                    RM->ReleaseTexture(*(BaseTexture**)oldParam.data);
                    RM->AddTextureRef(*(BaseTexture**)newParam.data);
                }
            }
            BackupParameters.CopyList(editLevelMaterial->Params);
        }
    }

    if(bOverride)
    {
        String newPath;
        GetItemPath(hEditPath, newPath, FALSE);

        path.Clear();
        path << baseDirectory << TEXT("/");
        if(newPath.IsValid())
            path << newPath << TEXT("/");
        path << name << TEXT(".mtl");

        if(newPath.IsValid())
        {
            String strName = newPath;
            int numDirs = strName.NumTokens('/');
            String newDir = baseDirectory;
            for(int i=0; i<numDirs; i++)
            {
                newDir << TEXT("/") << strName.GetToken(i, '/');
                OSCreateDirectory(newDir);
            }
        }
    }

    ConfigFile materialFile;
    materialFile.Create(path);

    materialFile.SetString(TEXT("Material"), TEXT("Effect"), strEffect);
    materialFile.SetInt(TEXT("Material"), TEXT("DrawType"), materialWindow->Items[0]->dwDrawType);
    materialFile.SetFloat(TEXT("Material"), TEXT("Restitution"), editMaterial->restitution);
    materialFile.SetFloat(TEXT("Material"), TEXT("Friction"), editMaterial->friction);
    materialFile.SetString(TEXT("Material"), TEXT("SoftSound"), editMaterial->strSoundSoft);
    if(!editMaterial->strSoundHard.CompareI(editMaterial->strSoundSoft))
        materialFile.SetString(TEXT("Material"), TEXT("HardSound"), editMaterial->strSoundHard);

    for(i=0; i<NumParams; i++)
    {
        HWND hwndControl = GetDlgItem(hwndControlContainer, PARAM_ID+i);
        HANDLE hCurParam = editMaterial->Params[i].handle;

        EffectParameterInfo paramInfo;
        editMaterial->effect->GetEffectParameterInfo(hCurParam, paramInfo);

        if(paramInfo.propertyType == EffectProperty_Texture)
        {
            TCHAR texture[255];
            GetWindowText(hwndControl, texture, 254);
            materialFile.SetString(TEXT("Parameters"), paramInfo.name, texture);
        }
        else if(paramInfo.propertyType == EffectProperty_Float)
        {
            TCHAR floatText[255];
            GetWindowText(hwndControl, floatText, 254);
            materialFile.SetString(TEXT("Parameters"), paramInfo.name, floatText);
        }
        else if(paramInfo.propertyType == EffectProperty_Color)
        {
            Color3 color = RGB_to_Vect(CCGetColor(hwndControl));
            materialFile.SetColor3(TEXT("Parameters"), paramInfo.name, color);
        }
    }

    materialFile.Close();

    return TRUE;

    traceOut;
}

void MaterialEditor::ChangeModule()
{
    traceIn(MaterialEditor::ChangeModule);

    String strOldDir;
    strOldDir << TEXT("data/") << curModule << TEXT("/materials");
    if(!OSFileExists(strOldDir + TEXT("/*.*")))
        RemoveDirectory(strOldDir);

    //--------------------------------------

    int id = SendMessage(hwndModuleList, CB_GETCURSEL, 0, 0);
    curModule.SetLength(SendMessage(hwndModuleList, CB_GETLBTEXTLEN, id, 0));
    SendMessage(hwndModuleList, CB_GETLBTEXT, id, (LPARAM)curModule.Array());

    TreeView_DeleteAllItems(hwndTreeControl);

    //--------------------------------------

    String strMatDir;
    strMatDir << TEXT("data/") << curModule << TEXT("/materials/");
    ProcessDirectory(strMatDir, NULL, curModule);

    traceOut;
}


BOOL WINAPI AddFolderProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    {
                        TCHAR input[256];

                        SendMessage(GetDlgItem(hwnd, IDC_FOLDERNAME), WM_GETTEXT, (WPARAM)255, (LPARAM)input);
                        materialEditor->AddFolder(input);
                    }
                case IDCANCEL:
                    EndDialog(hwnd, 0);
            }
            break;

        case WM_CLOSE:
            EndDialog(hwnd, 0);
    }

    return 0;
}


LRESULT WINAPI MaterialEditorProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_CLOSE:
            delete materialEditor;
            break;

        /*(case WM_SIZE:
            {
                if(wParam != SIZE_MINIMIZED)
                {
                    int x = LOWORD(lParam);
                    int y = HIWORD(lParam);

                    if(materialEditor)
                    {
                        if(!materialEditor->bEditMode && materialEditor->materialView)
                            materialEditor->materialView->SetSize(x-150, y);

                        SetWindowPos(materialEditor->hwndMaterialView, NULL, 150, 0, x-150, y, 0);
                        SetWindowPos(materialEditor->hwndTreeControl, NULL, 0, 0, 150, y, 0);

                        ShowWindow(materialEditor->hwndMaterialView, SW_RESTORE);

                        UpdateViewports(hwnd);
                    }
                }
                else
                    ShowWindow(materialEditor->hwndMaterialView, SW_MINIMIZE);

                return TRUE;
            }

        case WM_SIZING:
            {
                int borderXSize = 600;
                int borderYSize = 500;

                borderXSize += GetSystemMetrics(SM_CXSIZEFRAME)*2;
                borderXSize += GetSystemMetrics(SM_CXVSCROLL);

                borderYSize += GetSystemMetrics(SM_CYSIZEFRAME)*2;
                borderYSize += GetSystemMetrics(SM_CYCAPTION);
                borderYSize += GetSystemMetrics(SM_CYMENU);

                RECT *pRect = (RECT*)lParam;

                if((pRect->right - pRect->left) <= borderXSize)
                    pRect->right = pRect->left + borderXSize;

                if((pRect->bottom - pRect->top) <= borderYSize)
                    pRect->bottom = pRect->top + borderYSize;

                return TRUE;
            }*/

        case WM_COMMAND:
            {
                int controlID = LOWORD(wParam);

                if(materialEditor->bEditMode)
                {
                    if((HWND)lParam == materialEditor->hwndEffectList)
                    {
                        if(HIWORD(wParam) == CBN_SELCHANGE)
                        {
                            materialEditor->ChangeEffect();
                            materialEditor->bUnsavedChanges = TRUE;
                            break;
                        }
                    }
                    else if((HWND)lParam == materialEditor->hwndSaveChanges)
                    {
                        materialEditor->SaveMaterial();
                        break;
                    }
                    else if((HWND)lParam == materialEditor->hwndClose)
                    {
                        if(materialEditor->bUnsavedChanges)
                        {
                            int query = MessageBox(materialEditor->hwndMaterialEditor, TEXT("You have unsaved changes.\r\n\r\nDo you want to save your changes?"), TEXT("Just wondering."), MB_YESNOCANCEL);

                            if(query == IDCANCEL)
                                break;
                            else if(query == IDYES)
                            {
                                if(!materialEditor->SaveMaterial())
                                    break;
                            }
                        }

                        materialEditor->ChangeDirectory();
                        break;
                    }
                    else if((HWND)lParam == materialEditor->hwndDrawSphere)
                    {
                        MaterialItem *item = materialEditor->materialWindow->Items[0];
                        item->dwDrawType = DRAWMATERIAL_SPHERE;
                        materialEditor->bUnsavedChanges = TRUE;
                        UpdateViewports(materialEditor->hwndMaterialView);
                        break;
                    }
                    else if((HWND)lParam == materialEditor->hwndDrawBox)
                    {
                        MaterialItem *item = materialEditor->materialWindow->Items[0];
                        item->dwDrawType = DRAWMATERIAL_BOX;
                        materialEditor->bUnsavedChanges = TRUE;
                        UpdateViewports(materialEditor->hwndMaterialView);
                        break;
                    }
                    else if((HWND)lParam == materialEditor->hwndDrawFlat)
                    {
                        MaterialItem *item = materialEditor->materialWindow->Items[0];
                        item->dwDrawType = DRAWMATERIAL_FLAT;
                        materialEditor->bUnsavedChanges = TRUE;
                        UpdateViewports(materialEditor->hwndMaterialView);
                        break;
                    }
                    else if((HWND)lParam == materialEditor->hwndName)
                    {
                        TCHAR name[255];
                        GetWindowText(materialEditor->hwndName, name, 254);

                        if(!materialEditor->strPreviousName.CompareI(name))
                            materialEditor->bUnsavedChanges = TRUE;
                        break;
                    }
                }

                if((HWND)lParam == materialEditor->hwndModuleList)
                {
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        if(materialEditor->bEditMode && materialEditor->bUnsavedChanges)
                        {
                            int query = MessageBox(materialEditor->hwndMaterialEditor, TEXT("You have unsaved changes.\r\n\r\nDo you want to save your changes?"), TEXT("Just wondering."), MB_YESNOCANCEL);

                            if(query == IDCANCEL)
                            {
                                SendMessage(materialEditor->hwndModuleList, CB_SETCURSEL, SendMessage(materialEditor->hwndModuleList, CB_FINDSTRING, -1, (LPARAM)materialEditor->curModule.Array()), 0);
                                break;
                            }
                            else if(query == IDYES)
                            {
                                if(!materialEditor->SaveMaterial())
                                {
                                    SendMessage(materialEditor->hwndModuleList, CB_SETCURSEL, SendMessage(materialEditor->hwndModuleList, CB_FINDSTRING, -1, (LPARAM)materialEditor->curModule.Array()), 0);
                                    break;
                                }
                            }
                        }

                        materialEditor->ChangeModule();
                        break;
                    }
                }

                switch(controlID)
                {
                    case ID_MATERIAL_CREATE:
                        materialEditor->bEditingExisting = FALSE;
                        materialEditor->CreateNewMaterial();
                        break;

                    case ID_MATERIAL_EDIT:
                        {
                            MaterialItem *item = materialEditor->materialWindow->selectedMaterial;

                            if(item)
                                item->EditMaterial();
                            break;
                        }

                    case ID_MATERIAL_DELETE:
                        materialEditor->materialWindow->DeleteMaterial();
                        break;

                    case ID_MATERIAL_NEWFOLDER:
                        DialogBox(hinstMain, MAKEINTRESOURCE(IDD_NEWFOLDER), materialEditor->hwndMaterialEditor, (DLGPROC)AddFolderProc);
                        break;

                    case ID_MATERIAL_REMOVEFOLDER:
                        materialEditor->RemoveFolder();
                        break;

                    case ID_MATERIAL_CLOSE:
                        delete materialEditor;
                        break;
                };
                break;
            }

        case WM_NOTIFY:
            if(materialEditor)
            {
                NMTREEVIEW *info = (NMTREEVIEW*)lParam;

                if(info->hdr.hwndFrom == materialEditor->hwndTreeControl)
                {
                    HWND hwndTreeControl = info->hdr.hwndFrom;

                    if(info->hdr.code == TVN_SELCHANGING)
                    {
                        if(materialEditor->bEditMode && materialEditor->bUnsavedChanges)
                        {
                            int query = MessageBox(materialEditor->hwndMaterialEditor, TEXT("You have unsaved changes.\r\n\r\nDo you want to save your changes?"), TEXT("Just wondering."), MB_YESNOCANCEL);

                            if(query == IDCANCEL)
                                return TRUE;
                            else if(query == IDYES)
                            {
                                if(!materialEditor->SaveMaterial())
                                    return TRUE;
                            }
                        }
                    }
                    else if(info->hdr.code == TVN_SELCHANGED)
                    {
                        materialEditor->ChangeDirectory();
                    }
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
                        HTREEITEM hParent = TreeView_GetParent(hwndTreeControl, dispInfo->item.hItem);
                        TSTR lpText = dispInfo->item.pszText;

                        if(!lpText)
                            break;

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

                        String path, parentPath;
                        materialEditor->GetItemPath(hItem, path);
                        materialEditor->GetItemPath(hParent, parentPath);

                        parentPath << TEXT("/") << lpText;
                        if(!MoveFile(path, parentPath))
                            return FALSE;
                        else 
                            return TRUE;
                    }
                    else if(info->hdr.code == NM_RCLICK)
                    {
                        materialEditor->hCurItem = TreeView_GetDropHilight(hwndTreeControl);

                        if(!materialEditor->hCurItem)
                            materialEditor->hCurItem = TreeView_GetSelection(hwndTreeControl);

                        HTREEITEM hRoot = TreeView_GetRoot(hwndTreeControl);

                        HMENU hmenu = LoadMenu(hinstMain, MAKEINTRESOURCE(IDR_POPUPS));
                        HMENU hmenuPopup = GetSubMenu(hmenu, 3);

                        MENUITEMINFO mii;
                        zero(&mii, sizeof(mii));
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_STATE;

                        mii.fState = (materialEditor->hCurItem == hRoot) ? MFS_DISABLED : 0;
                        SetMenuItemInfo(hmenu, ID_MATERIAL_REMOVEFOLDER, FALSE, &mii);

                        POINT p;
                        GetCursorPos(&p);
                        TrackPopupMenuEx(hmenuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, hwnd, NULL);

                        DestroyMenu(hmenu);
                    }
                }
                break;
            }

        case WM_VSCROLL:
            {
                if(lParam)
                {
                    if(!materialEditor->bEditMode)
                    {
                        HWND hScroll = (HWND)lParam;

                        if(hScroll == materialEditor->hwndScrollBar)
                        {
                            SCROLLINFO si;
                            si.cbSize = sizeof(si);
                            si.fMask  = SIF_ALL;
                            GetScrollInfo(hScroll, SB_CTL, &si);

                            int curPos = si.nPos;

                            switch(LOWORD(wParam))
                            {
                                case SB_TOP:
                                    si.nPos = si.nMax; break;
                                case SB_BOTTOM:
                                    si.nPos = si.nMin; break;
                                case SB_LINEUP:
                                    si.nPos -= 10; break;
                                case SB_LINEDOWN:
                                    si.nPos += 10; break;
                                case SB_PAGEUP:
                                    si.nPos -= si.nPage; break;
                                case SB_PAGEDOWN:
                                    si.nPos += si.nPage; break;
                                case SB_THUMBTRACK:
                                    si.nPos = si.nTrackPos; break;
                            }

                            if(si.nPos > si.nMax)
                                si.nPos = si.nMax;
                            else if(si.nPos < si.nMin)
                                si.nPos = si.nMin;

                            if(si.nPos == curPos)
                                return 0;

                            materialEditor->materialWindow->SetPosY(float(-si.nPos));

                            si.fMask = SIF_POS;
                            SetScrollInfo(hScroll, SB_CTL, &si, TRUE);

                            UpdateViewports(materialEditor->hwndMaterialView);

                            return 0;
                        }
                    }
                }
                break;
            }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT WINAPI MaterialViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_PAINT)
    {
        RECT rect;
        PAINTSTRUCT ps;

        if(GetUpdateRect(hwnd, &rect, FALSE))
        {
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            if(materialEditor && !bErrorMessage)
                materialEditor->UpdateMaterialView();
        }
        return 0;
    }
    else if(message == WM_UPDATEVIEWPORTS)
    {
        if(materialEditor)
            materialEditor->UpdateMaterialView();

        return 0;
    }

    return CallWindowProc((FARPROC)MainWndProc, hwnd, message, wParam, lParam);
}

LRESULT WINAPI MaterialControlContainerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_COMMAND:
            {
                int controlID = LOWORD(wParam);

                if(materialEditor->bEditMode)
                {
                    if(controlID < PARAMMAX_ID)
                    {
                        if(controlID >= PARAMSCROLLER_ID)
                        {
                            materialEditor->AdjustScroller(controlID-PARAMSCROLLER_ID);
                            break;
                        }
                        else if(controlID >= PARAMBROWSE_ID)
                        {
                            materialEditor->TextureBrowse(controlID-PARAMBROWSE_ID);
                            break;
                        }
                        else if(controlID >= PARAM_ID)
                        {
                            materialEditor->UpdateParam(controlID-PARAM_ID, HIWORD(wParam));
                            break;
                        }
                    }

                    if((HWND)lParam == materialEditor->hwndEffectList)
                    {
                        if(HIWORD(wParam) == CBN_SELCHANGE)
                        {
                            if(!materialEditor->bEditingExisting)
                            {
                                materialEditor->ChangeEffect();
                                materialEditor->bUnsavedChanges = TRUE;
                            }
                            break;
                        }
                    }
                    else if((HWND)lParam == materialEditor->hwndSaveChanges)
                    {
                        materialEditor->SaveMaterial();
                    }
                    else if((HWND)lParam == materialEditor->hwndSoftCollisionBrowse)
                        materialEditor->SoundBrowse(materialEditor->hwndSoftCollision);
                    else if((HWND)lParam == materialEditor->hwndSoftCollisionClear)
                        materialEditor->SoundClear(materialEditor->hwndSoftCollision);
                    else if((HWND)lParam == materialEditor->hwndHardCollisionBrowse)
                        materialEditor->SoundBrowse(materialEditor->hwndHardCollision);
                    else if((HWND)lParam == materialEditor->hwndHardCollisionBrowse)
                        materialEditor->SoundClear(materialEditor->hwndHardCollision);
                    else if((HWND)lParam == materialEditor->hwndRestitution)
                    {
                        float restitution = GetUpDownFloat(materialEditor->hwndRestitution);
                        if(!CloseFloat(restitution, materialEditor->editMaterial->GetRestitution()))
                        {
                            materialEditor->editMaterial->SetRestitution(restitution);
                            materialEditor->bUnsavedChanges = TRUE;
                        }
                    }
                    else if((HWND)lParam == materialEditor->hwndFriction)
                    {
                        float friction = GetUpDownFloat(materialEditor->hwndFriction);
                        if(!CloseFloat(friction, materialEditor->editMaterial->GetFriction()))
                        {
                            materialEditor->editMaterial->SetFriction(friction);
                            materialEditor->bUnsavedChanges = TRUE;
                        }
                    }
                    else if((HWND)lParam == materialEditor->hwndClose)
                    {
                        if(materialEditor->bEditMode && materialEditor->bUnsavedChanges)
                        {
                            int query = MessageBox(materialEditor->hwndMaterialEditor, TEXT("You have unsaved changes.\r\n\r\nDo you want to save your changes?"), TEXT("Just wondering."), MB_YESNOCANCEL);

                            if(query == IDCANCEL)
                                break;
                            if(query == IDYES)
                            {
                                if(!materialEditor->SaveMaterial())
                                    break;
                            }
                        }

                        materialEditor->ChangeDirectory();
                    }
                    else if((HWND)lParam == materialEditor->hwndDrawSphere)
                    {
                        MaterialItem *item = materialEditor->materialWindow->Items[0];
                        item->dwDrawType = DRAWMATERIAL_SPHERE;
                        materialEditor->bUnsavedChanges = TRUE;
                        UpdateViewports(materialEditor->hwndMaterialView);
                    }
                    else if((HWND)lParam == materialEditor->hwndDrawBox)
                    {
                        MaterialItem *item = materialEditor->materialWindow->Items[0];
                        item->dwDrawType = DRAWMATERIAL_BOX;
                        materialEditor->bUnsavedChanges = TRUE;
                        UpdateViewports(materialEditor->hwndMaterialView);
                    }
                    else if((HWND)lParam == materialEditor->hwndDrawFlat)
                    {
                        MaterialItem *item = materialEditor->materialWindow->Items[0];
                        item->dwDrawType = DRAWMATERIAL_FLAT;
                        materialEditor->bUnsavedChanges = TRUE;
                        UpdateViewports(materialEditor->hwndMaterialView);
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

                        SetWindowPos(hwndChild, NULL, childPos.left, childPos.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
                    }while(hwndChild = GetWindow(hwndChild, GW_HWNDNEXT));

                    //redraw the windows all at once
                    //RedrawWindow(hwnd, NULL, NULL, RDW_ERASE|RDW_INVALIDATE);

                    si.fMask = SIF_POS;
                    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
                }
            }
            break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

void MaterialEditor::AdjustContainer()
{
    traceIn(MaterialEditor::AdjustContainer);

    SetWindowPos(hwndControlContainer, NULL, 0, 0, 466, yAdjust+26, SWP_NOZORDER);
    if(NumParams > (11-numExtraParams))
    {
        SCROLLINFO si;

        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        si.nMin = 0;
        si.nMax = (numExtraParams+NumParams+2)*26;
        si.nPage = 11*26;
        si.nPos = 0;
        si.nTrackPos = 0;

        ShowScrollBar(hwndOuterContainer, SB_VERT, TRUE);
        SetScrollInfo(hwndOuterContainer, SB_VERT, &si, TRUE);
    }
    else
        ShowScrollBar(hwndOuterContainer, SB_VERT, FALSE);

    traceOut;
}


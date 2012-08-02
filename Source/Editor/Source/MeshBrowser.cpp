/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  MeshBrowser.cpp

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


DefineClass(MeshWindow);


LRESULT WINAPI MeshBrowserProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI MeshViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


MeshBrowser *meshBrowser = NULL;


#define ID_STOPANIMATION 5000
#define ID_ANIMOFFSET 5001


#define skinmesh static_cast<SkinMesh*>(curMesh)

TCHAR lpLastModelModule[250] = TEXT("");


void MeshBrowser::RegisterWindowClasses()
{
    traceIn(MeshBrowser::RegisterWindowClasses);

    WNDCLASS wc;
    zero(&wc, sizeof(wc));

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)MeshBrowserProc;
    wc.hInstance = hinstMain;
    wc.hIcon = LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MESHBROWSERMENU);
    wc.lpszClassName = TEXT("MeshBrowser");

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the mesh browser window class"));

    wc.lpfnWndProc = (WNDPROC)MeshViewProc;
    wc.lpszMenuName = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = TEXT("XR3DMeshView");
    wc.cbWndExtra = sizeof(LPVOID);

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the mesh view class"));

    traceOut;
}


#define clientXSize   600
#define clientYSize   450

MeshBrowser::MeshBrowser()
{
    traceIn(MeshBrowser::MeshBrowser);

    meshBrowser = this;

    int borderXSize = clientXSize;
    int borderYSize = clientYSize;

    borderXSize += GetSystemMetrics(SM_CXDLGFRAME)*2;

    borderYSize += GetSystemMetrics(SM_CYDLGFRAME)*2;
    borderYSize += GetSystemMetrics(SM_CYCAPTION);
    borderYSize += GetSystemMetrics(SM_CYMENU);

    hwndMeshBrowser = CreateWindow(TEXT("MeshBrowser"), TEXT("Mesh/Prefab Browser"),
                                   WS_VISIBLE|WS_DLGFRAME|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   borderXSize, borderYSize,
                                   hwndEditor, NULL, hinstMain, NULL);

    //-----------------------------------------

    hwndModuleList = CreateWindow(TEXT("combobox"), NULL,
                                  WS_VISIBLE|WS_CHILDWINDOW|CBS_DROPDOWNLIST|CBS_SORT,
                                  0, 0, 150, 190,
                                  hwndMeshBrowser, NULL, hinstMain, NULL);
    SendMessage(hwndModuleList, WM_SETFONT, (WPARAM)hWindowFont, 0);

    RECT sizeOfWorthlessComboBox;
    GetWindowRect(hwndModuleList, &sizeOfWorthlessComboBox);
    int comboBoxSize = sizeOfWorthlessComboBox.bottom-sizeOfWorthlessComboBox.top;

    //-----------------------------------------

    hwndTreeControl = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, NULL,
                                    WS_VISIBLE|WS_CHILDWINDOW|TVS_HASLINES|TVS_LINESATROOT|TVS_SHOWSELALWAYS|TVS_HASBUTTONS,
                                    0, comboBoxSize, 150, clientYSize,
                                    hwndMeshBrowser, NULL, hinstMain, NULL);

    SendMessage(hwndTreeControl, WM_SETFONT, (WPARAM)hWindowFont, 0);

    HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK, 4, 0);
    /*HICON hIcon;

    HINSTANCE hShell = GetModuleHandle("shell32");

    hIcon = LoadIcon(hShell, MAKEINTRESOURCE(4));
    ImageList_AddIcon(hImageList, hIcon);

    hIcon = LoadIcon(hShell, MAKEINTRESOURCE(1));
    ImageList_AddIcon(hImageList, hIcon);*/

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

    hwndMeshView = (HANDLE)CreateWindow(TEXT("XR3DMeshView"), NULL,
                                        WS_VISIBLE|WS_CHILD,
                                        150, 0, 450, 500,
                                        hwndMeshBrowser, NULL, hinstMain, NULL);

    String strGraphicsSystem = AppConfig->GetString(TEXT("Engine"), TEXT("GraphicsSystem"));
    meshView = (GraphicsSystem*)CreateFactoryObject(strGraphicsSystem, FALSE);

    if(!meshView)
        CrashError(TEXT("Bad Graphics System: %s"), strGraphicsSystem);

    meshView->InitializeDevice(hwndMeshView);
    meshView->InitializeObject();
    meshView->SetSize(450, 450);

    //-----------------------------------------

    List<CTSTR> curModules;
    Engine::GetGameModules(curModules);

    for(int i=0; i<curModules.Num(); i++)
    {
        CTSTR moduleName = curModules[i];

        if(OSFileExists(String() << TEXT("data/") << moduleName << TEXT("/models")))
        {
            int id = SendMessage(hwndModuleList, CB_ADDSTRING, 0, (LPARAM)moduleName);

            if(scmpi(moduleName, lpLastModelModule) == 0)
            {
                curModule = moduleName;
                SendMessage(hwndModuleList, CB_SETCURSEL, id, 0);
            }
        }
    }

    if(curModule.IsEmpty())
    {
        curModule = TEXT("Base");
        SendMessage(hwndModuleList, CB_SETCURSEL, SendMessage(hwndModuleList, CB_FINDSTRING, -1, (LPARAM)TEXT("Base")), 0);
    }

    //-----------------------------------------

    String strModelPath;
    strModelPath << TEXT("data/") << curModule << TEXT("/models/");

    //OSFindData ofd;
    //HANDLE hFind = OSFindFirstFile(strModelPath + TEXT("*.xmd"), ofd);

    /*do
    {
        TSTR lpName = ofd.fileName;
        lpName[slen(lpName)-4] = 0;

        SendMessage(hwndMeshList, LB_ADDSTRING, 0, (LPARAM)lpName);
    }while(OSFindNextFile(hFind, ofd));*/

    ProcessDirectory(strModelPath, NULL, curModule);

    TreeView_Expand(hwndTreeControl, TreeView_GetRoot(hwndTreeControl), TVE_EXPAND);

    //OSFindClose(hFind);

    hCurTreeItem = NULL;
    curMesh = NULL;

    //-----------------------------------------

    meshManager = new ResourceManager;

    mainGD = GS;
    mainResourceMan = RM;

    meshWindow = CreateObject(MeshWindow);

    //-----------------------------------------

    GS = meshView;
    RM = meshManager;

    defaultMaterial = CreateObject(Material);
    defaultMaterial->effect = GetEffect(TEXT("Base:Bump.effect"));

    MaterialParameter parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("diffuseTexture"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:Default/TEST.dds"));
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("normalMap"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:Default/TEST_bm.dds"));
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("specularMap"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:Default/white.tga"), FALSE);
    defaultMaterial->Params << parameter;

    parameter.handle = defaultMaterial->effect->GetParameterByName(TEXT("illuminationMap"));
    parameter.type = Parameter_Texture;
    *(Texture**)parameter.data = GetTexture(TEXT("Base:Default/white.tga"), FALSE);
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
    blankAttenuation = meshView->CreateTexture(1, 1, GS_RGB, &chi, FALSE, TRUE);

    GS = mainGD;
    RM = mainResourceMan;

    traceOut;
}

MeshBrowser::~MeshBrowser()
{
    traceIn(MeshBrowser::~MeshBrowser);

    HIMAGELIST hImageList = TreeView_GetImageList(hwndTreeControl, TVSIL_NORMAL);

    DeleteMesh();

    GS = meshView;
    RM = meshManager;

    DestroyObject(defaultMaterial);
    DestroyObject(blankAttenuation);

    GS = mainGD;
    RM = mainResourceMan;

    delete meshManager;

    DestroyObject(meshWindow);
    DestroyObject(meshView);

    DestroyWindow(hwndTreeControl);
    DestroyWindow(hwndMeshBrowser);

    ImageList_Destroy(hImageList);
    meshBrowser = NULL;

    traceOut;
}



void MeshBrowser::ProcessDirectory(TSTR lpBaseDir, HTREEITEM hParent, TSTR lpName)
{
    traceIn(MeshBrowser::ProcessDirectory);

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

    strFindPath.Clear() << lpBaseDir << TEXT("*.xmd");
    hFind = FindFirstFile(strFindPath, &wfd);

    if(hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            String fileName = wfd.cFileName;
            fileName.SetLength(fileName.Length()-4);
            tvis.itemex.pszText = fileName;
            HTREEITEM hTest = TreeView_InsertItem(hwndTreeControl, &tvis);
        }while(FindNextFile(hFind, &wfd));

        FindClose(hFind);
    }

    if(!hParent)
        TreeView_SelectItem(hwndTreeControl, hItem);

    traceOut;
}

void MeshBrowser::GetItemPath(HTREEITEM hItem, String &path, BOOL bFullPath)
{
    traceIn(MeshBrowser::GetItemPath);

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
        path << TEXT("data") << curModule << TEXT("/models");

    traceOut;
}


void MeshBrowser::SetCurrentModule()
{
    traceIn(MeshBrowser::SetCurrentModule);

    int id = SendMessage(hwndModuleList, CB_GETCURSEL, 0, 0);
    curModule.SetLength(SendMessage(hwndModuleList, CB_GETLBTEXTLEN, id, 0));
    SendMessage(hwndModuleList, CB_GETLBTEXT, id, (LPARAM)curModule.Array());

    String strModelPath;
    strModelPath << TEXT("data/") << curModule << TEXT("/models/");

    TreeView_DeleteAllItems(hwndTreeControl);
    ProcessDirectory(strModelPath, NULL, curModule);
    TreeView_Expand(hwndTreeControl, TreeView_GetRoot(hwndTreeControl), TVE_EXPAND);

    DeleteMesh();

    hCurTreeItem = NULL;
    curMesh = NULL;

    UpdateMeshView();

    traceOut;
}


void MeshBrowser::SetCurrentMesh()
{
    traceIn(MeshBrowser::SetCurrentMesh);

    HTREEITEM hItem = TreeView_GetSelection(hwndTreeControl);
    if(hItem == hCurTreeItem)
        return;

    //--------------------------------

    if(!hItem)
        return;

    TVITEM tvi;
    tvi.mask = TVIF_IMAGE;
    tvi.hItem = hItem;
    TreeView_GetItem(hwndTreeControl, &tvi);

    if(tvi.iImage == 0)
        return;

    strMeshName.Clear();

    hCurTreeItem = hItem;

    DeleteMesh();

    strMeshName = GetMeshResourceName(hCurTreeItem);

    //--------------------------------

    String strMeshPath;
    Engine::ConvertResourceName(strMeshName, TEXT("models"), strMeshPath);

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(GetPathWithoutExtension(strMeshPath) << TEXT(".xan"), ofd);

    bHasAnimation = (hFind != NULL);

    if(hFind)
        OSFindClose(hFind);

    //--------------------------------

    GS = meshView;
    RM = meshManager;

    //--------------------------------

    curMesh = (bHasAnimation) ? new SkinMesh : new Mesh;
    curMesh->LoadMesh(strMeshName);

    MeshMaterials.SetSize(curMesh->DefaultMaterialList.Num());
    for(int i=0; i<MeshMaterials.Num(); i++)
    {
        TSTR lpName = utf8_createTstr(curMesh->DefaultMaterialList[i].name);
        Material *& material = MeshMaterials[i];

        String strPath;
        Engine::ConvertResourceName(lpName, TEXT("materials"), strPath);

        hFind = OSFindFirstFile(strPath, ofd);
        if(hFind)
        {
            OSFindClose(hFind);

            material = meshManager->GetMaterial(lpName);
        }
        else
            material = defaultMaterial;

        Free(lpName);
    }

    //--------------------------------

    if(bHasAnimation)
    {
        HMENU hMenu = GetMenu(hwndMeshBrowser);
        HMENU hAnimationMenu = CreatePopupMenu();

        AppendMenu(hAnimationMenu, MF_STRING, ID_STOPANIMATION, TEXT("No Animation"));
        AppendMenu(hAnimationMenu, MF_SEPARATOR, 0, NULL);

        for(int i=0; i<skinmesh->SequenceList.Num(); i++)
            AppendMenu(hAnimationMenu, MF_STRING, ID_ANIMOFFSET+i, skinmesh->SequenceList[i].strName);

        AppendMenu(hMenu, MF_POPUP|MF_STRING, (UINT_PTR)hAnimationMenu, TEXT("Animation"));
        DrawMenuBar(hwndMeshBrowser);

        //------------------------------

        for(int i=0; i<skinmesh->BoneList.Num(); i++)
        {
            Bone *baseBone = &skinmesh->BoneList[i];
            MeshBone *newBone = new MeshBone;
            newBone->LocPos = newBone->ObjPos = baseBone->Pos;
            newBone->LocRot = newBone->ObjRot = baseBone->Rot;
            newBone->lpBone = baseBone;

            if(baseBone->idParent != INVALID)
            {
                MeshBone *parentBone = BoneList[baseBone->idParent];
                parentBone->Children << newBone;
                newBone->LocPos -= parentBone->ObjPos;
            }

            BoneList << newBone;

            if(baseBone->flags & BONE_ROOT)
                BoneRootList << newBone;
        }

        //------------------------------

        if(!GS->UseHardwareAnimation())
        {
            MainVertBuffer = CloneVertexBuffer(skinmesh->VertBuffer);

            vbDeform = MainVertBuffer->GetData();
            vbOrigin = skinmesh->VertBuffer->GetData();

            VertList = vbDeform->VertList.Array();
        }
        else
            MainVertBuffer = skinmesh->VertBuffer;
    }
    else
        MainVertBuffer = curMesh->VertBuffer;

    //--------------------------------

    GS = mainGD;
    RM = mainResourceMan;

    //--------------------------------

    float meshRadius = curMesh->bounds.GetDiamater()*0.5f;
    meshWindow->viewDist = Vect2(tanf(RAD(90.0f-30.0f))*meshRadius, meshRadius).Len();
    UpdateMeshView();

    traceOut;
}

String MeshBrowser::GetMeshResourceName(HTREEITEM hItem)
{
    traceIn(MeshBrowser::GetMeshResourceName);

    String strItemPath;
    GetItemPath(hItem, strItemPath, FALSE);
    return curModule + TEXT(":") + strItemPath + TEXT(".xmd");

    traceOut;
}


void MeshBrowser::DeleteMesh()
{
    traceIn(MeshBrowser::DeleteMesh);

    HMENU hMenu = GetMenu(hwndMeshBrowser);
    DeleteMenu(hMenu, 1, MF_BYPOSITION);
    DrawMenuBar(hwndMeshBrowser);

    if(curMesh)
    {
        GS = meshView;
        RM = meshManager;

        //--------------------------------

        if(bHasAnimation)
        {
            if(bPlayingAnimation)
                StopAnimation(TRUE);

            for(int i=0; i<BoneList.Num(); i++)
                delete BoneList[i];
            BoneList.Clear();
            BoneRootList.Clear();

            if(!GS->UseHardwareAnimation())
                delete MainVertBuffer;
        }

        //--------------------------------

        delete curMesh;

        for(int i=0; i<MeshMaterials.Num(); i++)
        {
            Material *material = MeshMaterials[i];

            if(material != defaultMaterial)
                meshManager->ReleaseMaterial(material);
        }
        MeshMaterials.Clear();

        //--------------------------------

        GS = mainGD;
        RM = mainResourceMan;

        curMesh = NULL;
    }

    traceOut;
}


void MeshBrowser::UpdateMeshView()
{
    traceIn(MeshBrowser::UpdateMeshView);

    if(meshView && meshManager && !engine->bBlockViewUpdates)
    {
        GS = meshView;
        RM = meshManager;

        if(bHasAnimation && bPlayingAnimation)
            AnimationTick();

        meshView->PreRenderScene();
        meshView->RenderScene(TRUE, 0xFF101040);
        meshView->PostRenderScene();

        GS = mainGD;
        RM = mainResourceMan;
    }

    traceOut;
}


void MeshBrowser::AnimationTick()
{
    traceIn(MeshBrowser::AnimationTick);

    if(!bPlayingAnimation)
        return;

    float fTime = float(TrackTimeRestart(animTimer))*0.001f;

    float fKeyframeTime = skinmesh->SequenceList[curAnim.idSequence].fKeyframeTime;

    curAnim.curFrameTime += fTime;

    while(curAnim.curFrameTime >= fKeyframeTime)
    {
        curAnim.curFrameTime -= fKeyframeTime;

        if(curAnim.curFrame == (curAnim.endFrame-1))
        {
            if(curAnim.type == ANIM_PLAY_AND_HALT)
            {
                curAnim.fT = 0.0f;
                curAnim.flags |= ANIMATION_HALTING;
                curAnim.nextFrame = curAnim.curFrame;
                break;
            }
        }

        if(curAnim.curFrame == curAnim.endFrame)
        {
            if(curAnim.type == ANIM_PLAY_AND_STOP)
            {
                StopAnimation();
                return;
            }
            else if(curAnim.type == ANIM_LOOP)
                curAnim.curFrame = 0;
        }
        else
            curAnim.curFrame = curAnim.curFrame+1;

        curAnim.nextFrame = (curAnim.nextFrame == curAnim.endFrame) ? 0 : curAnim.nextFrame+1;
    }

    curAnim.fT = curAnim.curFrameTime/fKeyframeTime;

    if(curAnim.fT > 1.0f)
        curAnim.fT = 1.0f;

    for(int i=0; i<BoneRootList.Num(); i++)
    {
        BoneRootList[i]->LocPos = BoneRootList[i]->lpBone->Pos;
        RotateBones(BoneRootList[i]);
    }

    if(!GS->UseHardwareAnimation())
    {
        zero(VertList, sizeof(Vect)*skinmesh->nVerts);
        zero(vbDeform->NormalList.Array(), sizeof(Vect)*skinmesh->nVerts);
        zero(vbDeform->TangentList.Array(), sizeof(Vect)*skinmesh->nVerts);

        for(int i=0; i<BoneRootList.Num(); i++)
            Skin(BoneRootList[i]);

        MainVertBuffer->FlushBuffers();
    }

    if(curAnim.flags & ANIMATION_HALT)
    {
        StopAnimation(TRUE);
        return;
    }

    if(curAnim.flags & ANIMATION_HALTING)
        curAnim.flags = (curAnim.flags&~ANIMATION_HALTING)|ANIMATION_HALT;

    traceOut;
}

void MeshBrowser::PlayAnimation(int index)
{
    traceIn(MeshBrowser::PlayAnimation);

    curAnim.idSequence  = index;
    curAnim.endFrame    = skinmesh->SequenceList[index].nFrames-1;
    curAnim.curFrame    = 0;
    curAnim.curFrameTime= 0.0f;
    curAnim.fT          = 0.0f;
    curAnim.speed       = 1.0f;
    curAnim.type        = ANIM_LOOP;
    curAnim.nextFrame   = (curAnim.endFrame != 0) ? 1 : 0;
    curAnim.flags       = (curAnim.endFrame != 0) ? 0 : ANIMATION_HALT;

    if(!bPlayingAnimation)
    {
        bPlayingAnimation = TRUE;
        animTimer = TrackTimeBegin(FALSE);
        PostQuitMessage(1);
    }

    traceOut;
}

void MeshBrowser::StopAnimation(bool bHalt)
{
    traceIn(MeshBrowser::StopAnimation);

    if(bPlayingAnimation)
    {
        PostQuitMessage(1);
        TrackTimeEnd(animTimer);
    }

    bPlayingAnimation = FALSE;

    if(!bHalt)
    {
        if(!GS->UseHardwareAnimation())
        {
            mcpy(VertList, vbOrigin->VertList.Array(), sizeof(Vect)*skinmesh->nVerts);
            mcpy(vbDeform->NormalList.Array(), vbOrigin->NormalList.Array(), sizeof(Vect)*skinmesh->nVerts);
            mcpy(vbDeform->TangentList.Array(), vbOrigin->TangentList.Array(), sizeof(Vect)*skinmesh->nVerts);
            MainVertBuffer->FlushBuffers();
        }

        if(GS != meshView)
            UpdateMeshView();
    }

    traceOut;
}

Quat identQuat = Quat(0.0f, 0.0f, 0.0f, 1.0f);

void MeshBrowser::RotateBones(MeshBone *bone)
{
    //static VWeight *lpWeight;
    static Vect    vTransformed;
    static Quat    curRot,newLocalRot;
    static Vect    curPos;
    static Matrix  m;
    Bone    *boneDef;
    DWORD i;

    m.SetIdentity();

    boneDef = bone->lpBone; //get default bone info so we don't have to get it constantly
    curRot.SetIdentity();
    curPos  = (boneDef->flags & BONE_ROOT) ? boneDef->Pos : bone->ObjPos;

    newLocalRot.SetIdentity();

    SeqKeys *lpKeys = &bone->lpBone->seqKeys[curAnim.idSequence];
    if(lpKeys->hasRotKeys)
    {
        Quat curKey;

        curKey = CubicInterpolateQuat(lpKeys->lpRotKeys[curAnim.curFrame],
            lpKeys->lpRotKeys[curAnim.nextFrame],
            lpKeys->lpRotTans[curAnim.curFrame],
            lpKeys->lpRotTans[curAnim.nextFrame],
            curAnim.fT);
        /*curKey = InterpolateQuat(lpKeys->lpRotKeys[curAnim.curFrame],
            lpKeys->lpRotKeys[curAnim.nextFrame],
            curAnim.fT);*/

        if(curRot == identQuat)
            curRot = curKey;
        else
            curRot *= curKey;
    }
    if(lpKeys->hasPosKeys)
    {
        curPos += GetHSpline(lpKeys->lpPosKeys[curAnim.curFrame],
            lpKeys->lpPosKeys[curAnim.nextFrame],
            lpKeys->lpPosTans[curAnim.curFrame],
            lpKeys->lpPosTans[curAnim.nextFrame],
            curAnim.fT);
        /*curPos += Lerp<Vect>(lpKeys->lpPosKeys[curAnim.curFrame],
            lpKeys->lpPosKeys[curAnim.nextFrame],
            curAnim.fT);*/

        bone->LocPos = curPos;
        if(boneDef->flags & BONE_ROOT)
            bone->ObjPos = curPos;
    }

    newLocalRot = -curRot;

    if(curRot == identQuat)
        curRot = bone->ObjRot;
    else
        curRot *= bone->ObjRot;

    m.SetIdentity();
    m.Translate(curPos);
    m.Rotate(-curRot);

    bone->curMatrix = m;
    bone->curMatrix.Translate(-boneDef->Pos);
    bone->curMatrix.Transpose();

    for(i=0;i<bone->Children.Num();i++)
    {
        MeshBone *child = bone->Children[i];
        child->ObjPos = child->lpBone->LocalPos;
        child->ObjPos.TransformPoint(m);
        child->ObjRot = curRot;
    }

    bone->LocRot = newLocalRot;

    //kept seperate from above so that we don't have to allocate massive stack per call
    for(i=0;i<bone->Children.Num();i++)
    {
        MeshBone *child = bone->Children[i];
        RotateBones(child);
    }
}

void MeshBrowser::Skin(MeshBone *bone)
{
    static Vect    vTransformed;
    static Quat    curRot;
    static Vect    curPos;
    static Matrix  m;
    Bone    *boneDef;
    DWORD i;

    boneDef = bone->lpBone; //get default bone info so we don't have to get it constantly
    curRot  = -bone->LocRot * bone->ObjRot;
    curPos  = bone->ObjPos;

    m.SetIdentity();
    m.Translate(curPos);
    m.Rotate(-curRot);
    m.Translate(-boneDef->Pos);

    for(i=0;i<boneDef->Weights.Num();i++)
    {
        DWORD  vert  = boneDef->Weights[i].vert;
        float weight = boneDef->Weights[i].weight;

        //Transform the normal of the vertex
        vTransformed = vbOrigin->NormalList[vert];
        vTransformed.TransformVector(m);
        vbDeform->NormalList[vert] += (vTransformed * weight);

        //Transform the tangent U normal of the vertex
        vTransformed = vbOrigin->TangentList[vert];
        vTransformed.TransformVector(m);
        vbDeform->TangentList[vert] += (vTransformed * weight);

        //Transform the vertex
        vTransformed = vbOrigin->VertList[vert];
        vTransformed.TransformPoint(m);
        vbDeform->VertList[vert] += (vTransformed * weight);
    }

    //kept seperate from above so that we don't have to allocate massive stack per call
    for(i=0;i<bone->Children.Num();i++)
    {
        MeshBone *child = bone->Children[i];
        Skin(child);
    }
}



LRESULT WINAPI MeshBrowserProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_CLOSE:
            delete meshBrowser;
            break;

        case WM_COMMAND:
            {
                WORD commandID = LOWORD(wParam);

                if((commandID >= ID_ANIMOFFSET) && (commandID < 6000))
                {
                    meshBrowser->PlayAnimation(commandID-ID_ANIMOFFSET);
                    break;
                }

                if((HWND)lParam == meshBrowser->hwndModuleList)
                {
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        meshBrowser->SetCurrentModule();
                        break;
                    }
                }

                switch(commandID)
                {
                    case ID_STOPANIMATION:
                        meshBrowser->StopAnimation();
                        break;

                    case ID_MESH_CLOSE:
                        delete meshBrowser;
                        break;
                }

                break;
            }

        case WM_NOTIFY:
            if(meshBrowser)
            {
                NMTREEVIEW *info = (NMTREEVIEW*)lParam;

                if(info->hdr.hwndFrom == meshBrowser->hwndTreeControl)
                {
                    HWND hwndTreeControl = info->hdr.hwndFrom;

                    if(info->hdr.code == TVN_SELCHANGED)
                        meshBrowser->SetCurrentMesh();
                }
            }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT WINAPI MeshViewProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_PAINT)
    {
        RECT rect;
        PAINTSTRUCT ps;

        if(GetUpdateRect(hwnd, &rect, FALSE))
        {
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            if(meshBrowser && !bErrorMessage && !meshBrowser->bPlayingAnimation)
                meshBrowser->UpdateMeshView();
        }
        return 0;
    }
    else if(message == WM_UPDATEVIEWPORTS)
    {
        if(meshBrowser && !meshBrowser->bPlayingAnimation)
            meshBrowser->UpdateMeshView();

        return 0;
    }

    return CallWindowProc((FARPROC)MainWndProc, hwnd, message, wParam, lParam);
}



void MeshWindow::Init()
{
    traceIn(MeshWindow::Init);

    Super::Init();

    SetSystem(meshBrowser->meshView);

    Pos.Set(0.0f, 0.0f);
    Size.Set(450.0f, 450.0f);

    viewDist = 20.0f;
    tilt = 20.0f;
    spin = 220.0f;

    traceOut;
}

void MeshWindow::Render()
{
    traceIn(MeshWindow::Render);

    Mesh *mesh = meshBrowser->curMesh;

    if(!mesh)
        return;

    Perspective(60.0f, 1.0f, 0.4f, 1000.0f);

    Vect eyePos(0.0f, 0.0f, viewDist);
    Vect lightPos = Vect(-6.0f, 6.0f, 13.0f).Norm()*viewDist;

    MatrixPush();
    MatrixIdentity();
    MatrixTranslate(0.0f, 0.0f, -viewDist);

        Quat adjustRot;
        adjustRot = AxisAngle(1.0f, 0.0f, 0.0f, RAD(tilt));
        adjustRot *= AxisAngle(0.0f, 1.0f, 0.0f, RAD(spin));

        LoadVertexBuffer(meshBrowser->MainVertBuffer);
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

        BOOL bHardwareAnim = meshBrowser->bPlayingAnimation && GS->UseHardwareAnimation();

        for(int i=0; i<meshBrowser->MeshMaterials.Num(); i++)
        {
            Material *material = meshBrowser->MeshMaterials[i];
            DrawSection &section = mesh->SectionList[i];

            Effect *effect = material->effect;

            hTechnique = effect->GetTechnique(TEXT("InitialPass"));
            if(hTechnique)
            {
                effect->BeginTechnique(hTechnique);
                if(effect->BeginPassByName(bHardwareAnim ? TEXT("Animated") : TEXT("Normal")))
                {
                    if(material->LoadParameters())
                    {
                        if(bHardwareAnim)
                        {
                            HANDLE boneTransformsHandle = effect->GetParameterByName(TEXT("boneTransforms"));
                            float boneTransforms[16*4];

                            AnimSection &animSection = meshBrowser->GetSkinMesh()->AnimatedSections[i];
                            for(int j=0; j<animSection.SubSections.Num(); j++)
                            {
                                AnimSubSection &subSection = animSection.SubSections[j];

                                for(int k=0; k<subSection.numBones; k++)
                                    Matrix4x4Convert(boneTransforms+(k*16), meshBrowser->BoneList[subSection.bones[k]]->curMatrix);
                                effect->SetValue(boneTransformsHandle, boneTransforms, 64*subSection.numBones);

                                Draw(GS_TRIANGLES, 0, (subSection.startFace*3), (subSection.numFaces*3));
                            }
                        }
                        else
                            Draw(GS_TRIANGLES, 0, (section.startFace*3), (section.numFaces*3));
                    }

                    effect->EndPass();
                }
                effect->EndTechnique();
            }
        }

        EnableBlending(TRUE);
        DepthWriteEnable(FALSE);
        DepthFunction(GS_LEQUAL);

        for(int i=0; i<meshBrowser->MeshMaterials.Num(); i++)
        {
            Material *material = meshBrowser->MeshMaterials[i];
            DrawSection &section = mesh->SectionList[i];

            Effect *effect = material->effect;

            hTechnique = effect->GetTechnique(TEXT("PointLight"));

            if(hTechnique)
            {
                HANDLE hParam;

                hParam = effect->GetParameterByName(TEXT("lightRange"));
                effect->SetFloat(hParam, 10000.0f);

                hParam = effect->GetParameterByName(TEXT("lightColor"));
                effect->SetColor(hParam, 0xFFFFFFFF);

                hParam = effect->GetParameterByName(TEXT("attenuationMap"));
                effect->SetTexture(hParam, meshBrowser->blankAttenuation);

                hParam = effect->GetParameterByName(TEXT("eyePos"));
                effect->SetVector(hParam, eyePos);

                hParam = effect->GetParameterByName(TEXT("lightPos"));
                effect->SetVector(hParam, lightPos);

                effect->BeginTechnique(hTechnique);

                if(effect->BeginPassByName(bHardwareAnim ? TEXT("Animated") : TEXT("Normal")))
                {
                    if(material->LoadParameters())
                    {
                        if(bHardwareAnim)
                        {
                            HANDLE boneTransformsHandle = effect->GetParameterByName(TEXT("boneTransforms"));
                            float boneTransforms[16*4];

                            AnimSection &animSection = meshBrowser->GetSkinMesh()->AnimatedSections[i];
                            for(int j=0; j<animSection.SubSections.Num(); j++)
                            {
                                AnimSubSection &subSection = animSection.SubSections[j];

                                for(int k=0; k<subSection.numBones; k++)
                                    Matrix4x4Convert(boneTransforms+(k*16), meshBrowser->BoneList[subSection.bones[k]]->curMatrix);
                                effect->SetValue(boneTransformsHandle, boneTransforms, 64*subSection.numBones);

                                Draw(GS_TRIANGLES, 0, (subSection.startFace*3), (subSection.numFaces*3));
                            }
                        }
                        else
                            Draw(GS_TRIANGLES, 0, (section.startFace*3), (section.numFaces*3));
                    }

                    effect->EndPass();
                }
                effect->EndTechnique();
            }
        }

    MatrixPop();

    traceOut;
}

void MeshWindow::MouseDown(DWORD button)
{
    traceIn(MeshWindow::MouseDown);

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
        SetCapture(meshBrowser->hwndMeshView);
    }

    traceOut;
}

void MeshWindow::MouseUp(DWORD button)
{
    traceIn(MeshWindow::MouseUp);

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

void MeshWindow::MouseMove(int x, int y, short x_offset, short y_offset)
{
    traceIn(MeshWindow::MouseMove);

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

        meshBrowser->UpdateMeshView();

        GetSystem()->SetLocalMousePos(lastMouseX, lastMouseY);
        bIgnoreNextMove = TRUE;
    }

    traceOut;
}

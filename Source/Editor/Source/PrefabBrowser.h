/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  PrefabBrowser.h

  Copyright (c) 2001-2009, Hugh Bailey
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


struct PrefabWindow : public ControlWindow
{
    DeclareClass(PrefabWindow, ControlWindow);

public:
    PrefabWindow() {bRenderable = TRUE;}

    void Init();

    void Render();

    void MouseDown(DWORD button);
    void MouseUp(DWORD button);
    void MouseMove(int x, int y, short x_offset, short y_offset);

    float viewDist;
    float tilt, spin;

    int lastMouseX, lastMouseY;
    BOOL bIgnoreNextMove;

    DWORD buttonsDown;
};


class PrefabBrowser
{
public:
    PrefabBrowser();
    ~PrefabBrowser();

    static void RegisterWindowClasses();

    void UpdateMeshView();

    void ProcessDirectory(TSTR lpBaseDir, HTREEITEM hParent, TSTR lpName);
    void GetItemPath(HTREEITEM hItem, String &path, BOOL bFullPath);
    HTREEITEM GetCurrentDir(HTREEITEM hItem, String &path, BOOL bFullPath);

    void InsertTreeViewItem(HTREEITEM hParent, CTSTR lpName, BOOL bFolder);

    BOOL IsItemDirectory(HTREEITEM hItem);

    void SetCurrentModule();

    void SetCurrentPrefab(bool bResetCurrent=false);
    void DeleteMesh();

    void ResetView();

    void NewFolder();
    void DeleteItem();

    void CreatePrefab();
    void EditPrefab();
    void DuplicatePrefab();

    void GetLevelPrefabs(List<Prefab*> &PrefabList);

    void SelectItem(CTSTR lpItem);

    BOOL RenameItem(HTREEITEM hItem, CTSTR lpName);

    GraphicsSystem *prefabView;
    ResourceManager *prefabManager;

    String strPrefabName;
    String strMeshName;
    String strAnimationName;
    StringList MaterialNames;

    HWND hwndPrefabBrowser;
    HWND hwndTreeControl;
    HWND hwndModuleList;
    HWND hwndPrefabView;

    HTREEITEM hCurTreeItem;

    String curModule;

    Mesh *curMesh;
    List<Material*> MeshMaterials;

    GraphicsSystem *mainGD;
    ResourceManager *mainResourceMan;

    Texture *blankAttenuation;

    bool bHasAnimation;

    int prefabLightmapRes;

    PrefabWindow *prefabWindow;

    Material *defaultMaterial;

    //editing related stuff

    HWND hwndName;
    HWND hwndSaveChanges;
    HWND hwndClose;
    HWND hwndControlContainer;

    bool bEditMode;
    bool bUnsavedChanges;

    DWORD NumParams;
    DWORD MaterialStart;
    List<HWND> EditingControls;

    void AddSelectableEditParam(CTSTR lpName, CTSTR defaultValue);
    void AddPlainEditParam(CTSTR lpName, CTSTR defaultValue);
    void AddLightmapScrollerParam(CTSTR lpName, int defValue);

    void AdjustContainer();

    void UpdateParam(int paramID, int notification);
    void ApplyItem(int paramID);

    bool SavePrefab();
};


extern PrefabBrowser *prefabBrowser;


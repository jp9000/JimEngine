/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  MaterialEditor.h

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


class MaterialEditor
{
public:
    MaterialEditor();
    ~MaterialEditor();

    static void RegisterWindowClasses();

    void UpdateMaterialView();

    void ProcessDirectory(TSTR lpBaseDir, HTREEITEM hParent, TSTR lpName);

    void GetItemPath(HTREEITEM hItem, String &path, BOOL bFullPath=TRUE);
    inline void GetCurItemPath(String &path, BOOL bFullPath=TRUE)
    {
        GetItemPath(TreeView_GetSelection(hwndTreeControl), path, bFullPath);
    }
    void GetCurItemResourceName(String &name);

    void AddFolder(CTSTR lpFolder);
    void RemoveFolder();

    void KillDirectory(CTSTR lpDir);

    void ChangeDirectory();

    //-----------------

    HWND hwndMaterialEditor;
    HWND hwndModuleList;
    HWND hwndTreeControl;
    HWND hwndMaterialView;
    HWND hwndScrollBar;

    GraphicsSystem *materialView;
    MaterialWindow *materialWindow;

    //-----------------

    void EditMaterial(CTSTR lpMaterial);
    void CreateNewMaterial();
    void ChangeEffect();

    void AddTextureParam(HANDLE hParam, CTSTR lpName, CTSTR defaultValue);
    void AddColorParam(HANDLE hParam, CTSTR lpName, DWORD defaultValue);
    void AddFloatParam(HANDLE hParam, CTSTR lpName, float defaultValue);
    void AddFloatScrollerParam(HANDLE hParam, CTSTR lpName, float minValue, float maxValue, float precision, float defaultValue);

    void AdjustScroller(int paramID);
    void TextureBrowse(int paramID);
    void UpdateParam(int paramID, int notification);

    void AdjustContainer();

    void CleanupEditMode(BOOL bRemoveAllControls=TRUE);

    void ChangeModule();

    BOOL SaveMaterial();

    void SoundBrowse(HWND hwndEdit);
    void SoundClear(HWND hwndEdit);

    BOOL bEditMode;
    BOOL bUnsavedChanges;

    BOOL bEditingExisting;
    String strPreviousName;

    String curModule;

    Material *editMaterial;

    Material *editLevelMaterial;
    List<MaterialParameter> BackupParameters;

    HTREEITEM hEditPath;
    HTREEITEM hCurItem;

    HWND hwndName;
    HWND hwndEffectList;
    HWND hwndSaveChanges;
    HWND hwndClose;
    HWND hwndDrawSphere;
    HWND hwndDrawBox;
    HWND hwndDrawFlat;
    HWND hwndRestitution, hwndFriction;
    HWND hwndSoftCollision, hwndSoftCollisionBrowse, hwndSoftCollisionClear;
    HWND hwndHardCollision, hwndHardCollisionBrowse, hwndHardCollisionClear;
    HWND hwndControlContainer, hwndOuterContainer;

    int numMainControls, numExtraParams, NumParams;
    DWORD yAdjust;
    List<HWND> EditingControls;
};

extern MaterialEditor *materialEditor;


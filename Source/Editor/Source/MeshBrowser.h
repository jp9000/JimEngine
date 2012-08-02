/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  MeshBrowser.h

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


struct MeshBone
{
    Vect    LocPos;
    Quat    LocRot;
    Vect    ObjPos;
    Quat    ObjRot;

    Matrix  curMatrix;

    Bone    *lpBone;
    List<MeshBone*> Children;
};


struct MeshWindow : public ControlWindow
{
    DeclareClass(MeshWindow, ControlWindow);

public:
    MeshWindow() {bRenderable = TRUE;}
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


class MeshBrowser
{
public:
    MeshBrowser();
    ~MeshBrowser();

    static void RegisterWindowClasses();

    void UpdateMeshView();

    void ProcessDirectory(TSTR lpBaseDir, HTREEITEM hParent, TSTR lpName);
    void GetItemPath(HTREEITEM hItem, String &path, BOOL bFullPath);

    void SetCurrentModule();

    void SetCurrentMesh();
    void DeleteMesh();

    inline SkinMesh* GetSkinMesh() const {return static_cast<SkinMesh*>(curMesh);}

    String GetMeshResourceName(HTREEITEM hItem);

    GraphicsSystem *meshView;
    ResourceManager *meshManager;

    String strMeshName;

    HWND hwndMeshBrowser;
    HWND hwndTreeControl;
    HWND hwndModuleList;
    HWND hwndMeshView;

    HTREEITEM hCurTreeItem;

    String curModule;

    Mesh *curMesh;
    List<Material*> MeshMaterials;

    GraphicsSystem *mainGD;
    ResourceManager *mainResourceMan;

    Texture *blankAttenuation;

    BOOL bHasAnimation;

    MeshWindow *meshWindow;

    Material *defaultMaterial;

    //-----------------------------------------------
    // animation stuff

    BOOL bPlayingAnimation;
    Anim curAnim;
    float animationPoint;
    DWORD animTimer;

    Vect *VertList;
    VertexBuffer *MainVertBuffer;
    VBData *vbDeform;
    VBData *vbOrigin;

    List<MeshBone*> BoneList;
    List<MeshBone*> BoneRootList;

    void AnimationTick();
    void PlayAnimation(int index);
    void StopAnimation(bool bHalt=false);
    void RotateBones(MeshBone *curBone);
    void Skin(MeshBone *curBone);
};


extern MeshBrowser *meshBrowser;


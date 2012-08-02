/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ObjectBrowser.cpp

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



BOOL CALLBACK OpenModuleDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


void UpdateObjectTreeAndModules();
void AddToObjectTree(HTREEITEM hParent, Class *cls);

void LoadNewModule();


HWND hwndObjectTree = NULL;
HWND hwndModuleList = NULL;


void CreateObjectBrowser()
{
    traceIn(CreateObjectBrowser);

    int borderXSize = 450;
    int borderYSize = 300;

    borderXSize += GetSystemMetrics(SM_CXSIZEFRAME)*2;
    borderYSize += GetSystemMetrics(SM_CYSIZEFRAME)*2;
    borderYSize += GetSystemMetrics(SM_CYCAPTION);
    borderYSize += GetSystemMetrics(SM_CYMENU);

    editor->hwndObjectBrowser = CreateWindow(TEXT("ObjectBrowser"), TEXT("Object/Module Browser"),
                                             WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                             CW_USEDEFAULT, CW_USEDEFAULT, borderXSize, borderYSize,
                                             hwndEditor, NULL, hinstMain, NULL);

    RECT rect;
    GetClientRect(editor->hwndObjectBrowser, &rect);

    hwndObjectTree = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, NULL,
                                    WS_VISIBLE|WS_CHILDWINDOW|TVS_EDITLABELS|TVS_HASLINES|
                                    TVS_LINESATROOT|TVS_SHOWSELALWAYS|TVS_HASBUTTONS,
                                    150, 0, rect.right-150, rect.bottom,
                                    editor->hwndObjectBrowser, NULL, hinstMain, NULL);

    hwndModuleList = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Listbox"), NULL,
                                    WS_VISIBLE|WS_CHILDWINDOW|LBS_SORT|LBS_HASSTRINGS|LBS_NOINTEGRALHEIGHT,
                                    0, 0, 150, rect.bottom,
                                    editor->hwndObjectBrowser, NULL, hinstMain, NULL);

    SendMessage(hwndModuleList, WM_SETFONT, (WPARAM)hWindowFont, 0);

    UpdateObjectTreeAndModules();

    traceOut;
}



LRESULT WINAPI ObjectBrowserProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(ObjectBrowserProc);

    switch(message)
    {
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_OBJECTS_OPENMODULE:
                    LoadNewModule();
                    break;

                case ID_OBJECTS_UNLOADMODULE:
                    {
                        DWORD dwCurSel = SendMessage(hwndModuleList, LB_GETCURSEL, 0, 0);

                        if(dwCurSel != LB_ERR)
                        {
                            if(MessageBox(hwndModuleList, TEXT("Unloading a module will remove all objects associated with this module from this level.\r\n\r\nAre you sure you want to do this?"), TEXT("Unload Module"), MB_YESNO|MB_ICONQUESTION) == IDYES)
                            {
                                String strModuleName;
                                strModuleName.SetLength(255);
                                SendMessage(hwndModuleList, LB_GETTEXT, dwCurSel, (LPARAM)(CTSTR)strModuleName);

                                SendMessage(hwndModuleList, LB_DELETESTRING, dwCurSel, 0);

                                level->UnloadLevelModule(strModuleName);

                                UpdateObjectTreeAndModules();
                            }
                        }
                        else
                            MessageBox(hwnd, TEXT("You have to select a module to unload."), NULL, MB_OK);

                        break;
                    }

                case ID_OBJECTS_CLOSE:
                    DestroyWindow(hwnd);
                    editor->hwndObjectBrowser = NULL;
                    break;
            }
            break;

        case WM_NOTIFY:
            {
                NMTREEVIEW *info = (NMTREEVIEW*)lParam;

                if(info->hdr.code == TVN_SELCHANGED)
                {
                    HTREEITEM hItem = TreeView_GetSelection(hwndObjectTree);

                    TVITEMEX tvi;

                    zero(&tvi, sizeof(tvi));
                    tvi.mask  = TVIF_PARAM|TVIF_HANDLE;
                    tvi.hItem = hItem;
                    TreeView_GetItem(hwndObjectTree, &tvi);

                    Class *cls = (Class*)tvi.lParam;

                    editor->selectedEntityClass = cls;

                    if((levelInfo->curEditMode == EditMode_Create) && levelInfo->newObject && levelInfo->newObject->IsOf(GetClass(EntityPlacer)))
                    {
                        EntityPlacer *ep = (EntityPlacer*)levelInfo->newObject;
                        EditMode em = ep->prevEditMode;
                        EntityType et = ep->entityType;

                        delete levelInfo->newObject;
                        levelInfo->newObject = CreateObjectParam2(EntityPlacer, em, et);
                    }
                }

                break;
            }

        case WM_SIZE:
            {
                int cx = LOWORD(lParam);
                int cy = HIWORD(lParam);

                SetWindowPos(hwndObjectTree, NULL, 150, 0, cx-150, cy, SWP_NOMOVE);
                SetWindowPos(hwndModuleList, NULL, 0, 0, 150, cy, SWP_NOMOVE);
                break;
            }

        case WM_SIZING:
            {
                int borderXSize = 300;
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
            DestroyWindow(hwnd);
            editor->hwndObjectBrowser = NULL;
            break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);

    traceOut;
}

void LoadNewModule()
{
    traceIn(LoadNewModule);

    DialogBox(hinstMain, MAKEINTRESOURCE(IDD_OPENMODULE), editor->hwndObjectBrowser, (DLGPROC)OpenModuleDialogProc);

    traceOut;
}


void UpdateObjectTreeAndModules()
{
    traceIn(UpdateObjectTreeAndModules);

    if(TreeView_GetRoot(hwndObjectTree))
        TreeView_DeleteItem(hwndObjectTree, TVI_ROOT);

    AddToObjectTree(NULL, GetClass(Entity));

    SendMessage(hwndModuleList, LB_RESETCONTENT, 0, 0);
    StringList LevelModules;
    level->GetLoadedModules(LevelModules);
    for(DWORD i=0; i<LevelModules.Num(); i++)
        SendMessage(hwndModuleList, LB_ADDSTRING, 0, (LPARAM)(CTSTR)LevelModules[i]);

    traceOut;
}


void AddToObjectTree(HTREEITEM hParent, Class *cls)
{
    traceIn(AddToObjectTree);

    static TVINSERTSTRUCT tvis;

    if(cls == GetClass(EditorObject))
        return;

    zero(&tvis, sizeof(TVINSERTSTRUCT));
    tvis.hParent          = hParent;
    tvis.hInsertAfter     = TVI_SORT;
    tvis.itemex.mask      = TVIF_TEXT|TVIF_PARAM;
    tvis.itemex.pszText   = (TSTR)cls->GetName();
    tvis.itemex.lParam    = (LPARAM)cls;
    HTREEITEM hItem = TreeView_InsertItem(hwndObjectTree, &tvis);

    List<Class*> Children;
    Class::GetClassChildren(cls, Children);

    for(DWORD i=0; i<Children.Num(); i++)
        AddToObjectTree(hItem, Children[i]);

    if(hParent == NULL)
        TreeView_Expand(hwndObjectTree, hItem, TVE_EXPAND);

    traceOut;
}


BOOL CALLBACK OpenModuleDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(OpenModuleDialogProc);

    switch(message)
    {
        case WM_INITDIALOG:
            {
                OSFindData findData;

                //todo: 
                AppWarning(TEXT("fix this thingy please"));
                HANDLE hFind = OSFindFirstFile(TEXT("*.xxt"), findData);

                if(hFind)
                {
                    do
                    {
                        findData.fileName[slen(findData.fileName)-4] = 0;
                        if(!IsModuleLoaded(findData.fileName))
                            SendMessage(GetDlgItem(hwnd, IDC_MODULELIST), LB_ADDSTRING, 0, (LPARAM)findData.fileName);
                    }while(OSFindNextFile(hFind, findData));

                    OSFindClose(hFind);

                    SendMessage(GetDlgItem(hwnd, IDC_MODULELIST), LB_SETCURSEL, 0, 0);
                    SendMessage(GetDlgItem(hwnd, IDC_MODULELIST), EM_LIMITTEXT, 250, 0);
                }
                break;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_MODULELIST:
                    if(HIWORD(wParam) == LBN_DBLCLK)
                        PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), lParam);
                    break;

                case IDOK:
                    {
                        DWORD dwCurSel = SendMessage(GetDlgItem(hwnd, IDC_MODULELIST), LB_GETCURSEL, 0, 0);

                        if(dwCurSel != LB_ERR)
                        {
                            String strModuleName;
                            strModuleName.SetLength(255);
                            SendMessage(GetDlgItem(hwnd, IDC_MODULELIST), LB_GETTEXT, dwCurSel, (LPARAM)(CTSTR)strModuleName);

                            if(!level->LoadLevelModule(strModuleName))
                                MessageBox(hwnd, String() << TEXT("Unable to open module '") << strModuleName << TEXT("'."), NULL, MB_OK);
                            else
                            {
                                EndDialog(hwnd, IDOK);
                                UpdateObjectTreeAndModules();
                            }
                        }
                        else
                            MessageBox(hwnd, TEXT("You need to select a module first"), NULL, MB_OK);

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

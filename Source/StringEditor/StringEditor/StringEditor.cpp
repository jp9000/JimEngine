/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Copyright (c) 2010-2012, Hugh Bailey
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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#include "Main.h"

StringEditor *App = NULL;

BOOL CALLBACK GetTextDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


struct NameDialogInfo
{
    String val;
    HTREEITEM hItem;
};


static HTREEITEM TreeView_FindItemStringI(HWND hwndTree, HTREEITEM hParent, CTSTR lpStr)
{
    if(!lpStr || !*lpStr) return NULL;

    if(!hParent)
        hParent = TreeView_GetRoot(hwndTree);
    HTREEITEM hCurChild = TreeView_GetChild(hwndTree, hParent);
    if(!hCurChild) return NULL;

    String val = lpStr;
    UINT textLen = val.Length()+1;
    val.SetLength(textLen);

    do
    {

        TVITEM tvi;
        zero(&tvi, sizeof(tvi));
        tvi.mask = TVIF_TEXT|TVIF_HANDLE;
        tvi.cchTextMax = textLen+1;
        tvi.pszText = val.Array();
        tvi.hItem = hCurChild;
        TreeView_GetItem(hwndTree, &tvi);

        if(val.CompareI(lpStr))
            break;
    }while(hCurChild = TreeView_GetNextSibling(hwndTree, hCurChild));

    return hCurChild;
}

static void TreeView_GetItemLookupName(HWND hwndTree, HTREEITEM hItem, String &lookup, BOOL bFirst=TRUE)
{
    if(!hItem) return;

    if(hItem == TreeView_GetRoot(hwndTree))
        return;

    TCHAR textVal[256];

    TVITEM tvi;
    zero(&tvi, sizeof(tvi));
    tvi.mask = TVIF_TEXT|TVIF_HANDLE;
    tvi.cchTextMax = 255;
    tvi.pszText = textVal;
    tvi.hItem = hItem;
    TreeView_GetItem(hwndTree, &tvi);

    if(bFirst)
        lookup.Clear();
    else
        lookup.InsertChar(0, '.');

    lookup.InsertString(0, textVal);

    HTREEITEM hParent = TreeView_GetParent(hwndTree, hItem);
    TreeView_GetItemLookupName(hwndTree, hParent, lookup, FALSE);
}


StringEditor::StringEditor()
{
    LOGFONT lf;
    zero(&lf, sizeof(lf));

    lf.lfHeight = -14;
    scpy(lf.lfFaceName, TEXT("Arial"));
    hWindowFont = CreateFontIndirect(&lf);

    //-----------------------------------

    WNDCLASS wc;

    zero(&wc, sizeof(wc));
    wc.style = 0;
    wc.lpfnWndProc = StringEditor::MainWindowProc;
    wc.hInstance = hinstMain;
    wc.hIcon = 0;//LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    wc.lpszClassName = TEXT("StringEditor");

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register main window class"));

    appDirectory.SetLength(255);
    GetCurrentDirectory(256, appDirectory.Array());

    ConfigFile config;
    config.Open(TEXT("StringConfig.ini"));
    int width  = config.GetInt(TEXT("Window"), TEXT("Width"), CW_USEDEFAULT);
    int height = config.GetInt(TEXT("Window"), TEXT("Height"), CW_USEDEFAULT);
    config.Close();

    hwndMain = CreateWindowEx(0, TEXT("StringEditor"), TEXT("String Editor - Untitled"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                              CW_USEDEFAULT, CW_USEDEFAULT, width, height, //, 1000, 600, 
                              NULL, NULL, hinstMain, this);

    /*RECT client, desktop, rect;
    GetWindowRect(GetDesktopWindow(), &desktop);
    GetWindowRect(hwndMain, &rect);

    SetWindowPos(hwndMain, NULL, desktop.right/2 - (rect.right-rect.left)/2, desktop.bottom/2 - (rect.bottom-rect.top)/2, 0, 0, SWP_NOSIZE);*/

    RECT client;
    GetClientRect(hwndMain, &client);

    //-----------------------------------

    hwndTree = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, NULL,
                              WS_VISIBLE|WS_CHILDWINDOW|TVS_EDITLABELS|TVS_HASLINES|TVS_LINESATROOT|TVS_SHOWSELALWAYS|TVS_HASBUTTONS,
                              0, 0, 300, client.bottom,
                              hwndMain, NULL, hinstMain, NULL);

    SendMessage(hwndTree, WM_SETFONT, (WPARAM)hWindowFont, 0);

    HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32|ILC_MASK, 4, 0);

    SHSTOCKICONINFO shsii;
    zero(&shsii, sizeof(shsii));
    shsii.cbSize = sizeof(shsii);

    HINSTANCE hShell = GetModuleHandle(TEXT("shell32"));

    HICON hIcon = LoadIcon(hShell, MAKEINTRESOURCE(1));
    ImageList_AddIcon(hImageList, hIcon);

    hIcon = LoadIcon(hShell, MAKEINTRESOURCE(4));
    ImageList_AddIcon(hImageList, hIcon);

    TreeView_SetImageList(hwndTree, hImageList, TVSIL_NORMAL);

    TVINSERTSTRUCT tvis;
    zero(&tvis, sizeof(TVINSERTSTRUCT));
    tvis.hParent          = NULL;
    tvis.hInsertAfter     = TVI_SORT;
    tvis.itemex.mask      = TVIF_TEXT;
    tvis.itemex.pszText   = TEXT("Untitled");
    HTREEITEM hItem = TreeView_InsertItem(hwndTree, &tvis);

    //-----------------------------------

    hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL,
                              WS_VISIBLE|WS_CHILDWINDOW|ES_MULTILINE|ES_AUTOHSCROLL|ES_AUTOVSCROLL|WS_HSCROLL|WS_VSCROLL|WS_DISABLED,
                              300, 0, client.right-300, client.bottom,
                              hwndMain, NULL, hinstMain, NULL);

    SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hWindowFont, 0);

    //-----------------------------------

    ShowWindow(hwndMain, SW_SHOW);

    lookup = new LocaleStringLookup;

    bUntitled = true;
}

StringEditor::~StringEditor()
{
    delete lookup;

    SetCurrentDirectory(appDirectory);

    ConfigFile config;
    config.Open(TEXT("StringConfig.ini"), TRUE);

    RECT rect;
    GetWindowRect(hwndMain, &rect);
    config.SetInt(TEXT("Window"), TEXT("Width"), rect.right-rect.left);
    config.SetInt(TEXT("Window"), TEXT("Height"), rect.bottom-rect.top);
    config.Close();

    DestroyWindow(hwndMain);
    DeleteObject(hWindowFont);
}


void StringEditor::ClearData(CTSTR lpName)
{
    delete lookup;
    lookup = new LocaleStringLookup;
    bFileChanged = false;
    bUntitled = true;

    SetWindowText(hwndEdit, NULL);
    EnableWindow(hwndEdit, FALSE);

    TreeView_DeleteAllItems(hwndTree);

    TVINSERTSTRUCT tvis;
    zero(&tvis, sizeof(TVINSERTSTRUCT));
    tvis.hParent          = NULL;
    tvis.hInsertAfter     = TVI_SORT;
    tvis.itemex.mask      = TVIF_TEXT;
    tvis.itemex.pszText   = (TSTR)lpName;
    HTREEITEM hItem = TreeView_InsertItem(hwndTree, &tvis);

    String strWindowName;
    strWindowName << TEXT("String Editor - ") << lpName;
    SetWindowText(hwndMain, strWindowName);
}

void StringEditor::SetName(CTSTR lpName)
{
    TVITEM tvItem;
    tvItem.mask = TVIF_TEXT | TVIF_HANDLE;
    tvItem.pszText = (TSTR)lpName;
    tvItem.hItem = TreeView_GetRoot(hwndTree);
    TreeView_SetItem(hwndTree, &tvItem);

    String strWindowName;
    strWindowName << TEXT("String Editor - ") << lpName;
    SetWindowText(hwndMain, strWindowName);
}

void StringEditor::InsertItem(CTSTR lpLookup)
{
    String lookupStr = lpLookup;
    String curName;

    HTREEITEM hParent = TreeView_GetRoot(hwndTree);

    int nTokens = lookupStr.NumTokens('.');
    for(int i=0; i<nTokens; i++)
    {
        String itemName = lookupStr.GetToken(i, '.');

        HTREEITEM hChild = TreeView_FindItemStringI(hwndTree, hParent, itemName);
        if(!hChild)
        {
            TVINSERTSTRUCT tvis;
            zero(&tvis, sizeof(TVINSERTSTRUCT));
            tvis.hParent            = hParent;
            tvis.hInsertAfter       = TVI_SORT;
            tvis.itemex.mask        = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
            tvis.itemex.iImage      = tvis.itemex.iSelectedImage = 1;
            tvis.itemex.pszText     = itemName.Array();
            hChild = TreeView_InsertItem(hwndTree, &tvis);
        }

        hParent = hChild;
    }
}

void StringEditor::New()
{
    if(bFileChanged)
    {
        switch(MessageBox(hwndMain, TEXT("You have unsaved changes.  Would you like to save your file?"), TEXT("Unsaved Changes"), MB_YESNOCANCEL))
        {
            case IDYES:
                Save();
            case IDNO:
                break;

            default:
                return;
        }
    }

    ClearData(TEXT("Untitled"));
}

void StringEditor::Open()
{
    if(bFileChanged)
    {
        switch(MessageBox(hwndMain, TEXT("You have unsaved changes.  Would you like to save your file?"), TEXT("Unsaved Changes"), MB_YESNOCANCEL))
        {
            case IDYES:
                Save();
            case IDNO:
                break;

            default:
                return;
        }
    }

    TCHAR lpFile[512];
    OPENFILENAME ofn;
    zero(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndMain;
    ofn.lpstrFile = lpFile;
    ofn.nMaxFile = 511;

    TCHAR curDirectory[512];
    GetCurrentDirectory(511, curDirectory);

    ofn.lpstrFile[0] = 0;
    ofn.lpstrFilter = TEXT("String Files\0*.txt\0");
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = curDirectory;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if(GetOpenFileName(&ofn))
    {
        curFile = lpFile;
        curFile.FindReplace(TEXT("\\"), TEXT("/"));

        ClearData(GetPathFileName(curFile));
        lookup->LoadStringFile(curFile);

        bUntitled = false;

        const LocaleStringCache &cache = lookup->GetCache();
        for(int i=0; i<cache.Num(); i++)
            InsertItem(cache[i]->lookup);

        //TreeView_SelectItem(hwndTree, TreeView_GetRoot(hwndTree));
        TreeView_Expand(hwndTree, TreeView_GetRoot(hwndTree), TVM_EXPAND);
    }
}

void StringEditor::Save()
{
    if(bUntitled)
    {
        SaveAs();
        return;
    }

    SaveItem(TreeView_GetSelection(hwndTree));

    String saveData;
    SaveChildren(TreeView_GetRoot(hwndTree), saveData);

    XFile stringFile;
    if(!stringFile.Open(curFile, XFILE_WRITE, XFILE_CREATEALWAYS))
    {
        MessageBox(hwndMain, TEXT("Unable to save file.  Check to make sure the file is not already in use."), NULL, MB_ICONERROR);
        return;
    }

    if(saveData.Right(4) == TEXT("\r\n\r\n"))
        saveData.SetLength(saveData.Length()-2);

    stringFile.WriteAsUTF8(saveData);
    stringFile.Close();

    bFileChanged = false;
}

void StringEditor::SaveChildren(HTREEITEM hParent, String &saveData)
{
    HTREEITEM hChild = TreeView_GetChild(hwndTree, hParent);
    if(!hChild)
        return;

    StringList lookups;
    StringList values;

    int largestLookupLen = 0;

    do
    {
        String lookupName;
        TreeView_GetItemLookupName(hwndTree, hChild, lookupName);

        if(lookup->HasLookup(lookupName))
        {
            lookups << lookupName;
            values  << lookup->LookupString(lookupName);

            largestLookupLen = MAX(largestLookupLen, lookupName.Length());
        }
    }while(hChild = TreeView_GetNextSibling(hwndTree, hChild));

    largestLookupLen += 4;
    largestLookupLen &= 0xFFFFFFFC;

    String strLookup;
    strLookup.SetLength(largestLookupLen);

    for(int i=0; i<lookups.Num(); i++)
    {
        int endPos = lookups[i].Length();
        scpy(strLookup, lookups[i]);

        for(int j=endPos; j<largestLookupLen; j++)
            strLookup[j] = ' ';

        saveData << strLookup << String::StringToRepresentation(values[i]) << TEXT("\r\n");
    }

    if(saveData.Right(4) != TEXT("\r\n\r\n"))
        saveData << TEXT("\r\n");

    hChild = TreeView_GetChild(hwndTree, hParent);
    do
    {
        SaveChildren(hChild, saveData);
    }while(hChild = TreeView_GetNextSibling(hwndTree, hChild));
}


void StringEditor::SaveAs()
{
    TCHAR lpFile[512];
    OPENFILENAME ofn;
    zero(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndMain;
    ofn.lpstrFile = lpFile;
    ofn.nMaxFile = 511;

    TCHAR curDirectory[512];
    GetCurrentDirectory(511, curDirectory);

    ofn.lpstrFile[0] = 0;
    ofn.lpstrFilter = TEXT("String Files\0*.txt\0");
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = curDirectory;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if(GetSaveFileName(&ofn))
    {
        if(!GetPathExtension(lpFile).CompareI(TEXT("txt")))
            scat(lpFile, TEXT(".txt"));

        curFile = lpFile;
        curFile.FindReplace(TEXT("\\"), TEXT("/"));

        SetName(GetPathFileName(curFile));
        bUntitled = false;

        Save();
    }
}

void StringEditor::NewString()
{
    HTREEITEM hItem = TreeView_GetSelection(hwndTree);
    if(!hItem || hItem == TreeView_GetRoot(hwndTree))
    {
        NewSubString();
        return;
    }

    HTREEITEM hParent = TreeView_GetParent(hwndTree, hItem);

    NameDialogInfo info;
    info.hItem = hParent;

    if(DialogBoxParam(hinstMain, MAKEINTRESOURCE(IDD_TEXTDLG), hwndMain, GetTextDlgProc, (LPARAM)&info) == IDCANCEL)
        return;

    TVINSERTSTRUCT tvis;
    zero(&tvis, sizeof(TVINSERTSTRUCT));
    tvis.hParent            = hParent;
    tvis.hInsertAfter       = TVI_SORT;
    tvis.itemex.mask        = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
    tvis.itemex.iImage      = tvis.itemex.iSelectedImage = 1;
    tvis.itemex.pszText     = info.val.Array();
    HTREEITEM hNewItem = TreeView_InsertItem(hwndTree, &tvis);

    String lookupName;
    TreeView_GetItemLookupName(hwndTree, hNewItem, lookupName);
    lookup->AddLookupString(lookupName, NULL);

    TreeView_SelectItem(hwndTree, hNewItem);

    SetFocus(hwndEdit);

    bFileChanged = TRUE;
}

void StringEditor::NewSubString()
{
    HTREEITEM hItem = TreeView_GetSelection(hwndTree);
    if(!hItem)
        hItem = TreeView_GetRoot(hwndTree);

    NameDialogInfo info;
    info.hItem = hItem;

    if(DialogBoxParam(hinstMain, MAKEINTRESOURCE(IDD_TEXTDLG), hwndMain, GetTextDlgProc, (LPARAM)&info) == IDCANCEL)
        return;

    TVINSERTSTRUCT tvis;
    zero(&tvis, sizeof(TVINSERTSTRUCT));
    tvis.hParent            = hItem;
    tvis.hInsertAfter       = TVI_SORT;
    tvis.itemex.mask        = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
    tvis.itemex.iImage      = tvis.itemex.iSelectedImage = 1;
    tvis.itemex.pszText     = info.val.Array();
    HTREEITEM hNewItem = TreeView_InsertItem(hwndTree, &tvis);

    String lookupName;
    TreeView_GetItemLookupName(hwndTree, hNewItem, lookupName);
    lookup->AddLookupString(lookupName, NULL);

    TreeView_SelectItem(hwndTree, hNewItem);

    SetFocus(hwndEdit);

    bFileChanged = TRUE;
}

void StringEditor::DeleteString(HTREEITEM hItem)
{
    BOOL bTopItem = (hItem == NULL);
    if(bTopItem) hItem = TreeView_GetSelection(hwndTree);

    if(hItem == TreeView_GetRoot(hwndTree) || !hItem)
        return;

    HTREEITEM hChild = TreeView_GetChild(hwndTree, hItem);
    if(hChild)
    {
        if(bTopItem && MessageBox(hwndMain, TEXT("Deleting this item will delete all sub-items.  Are you sure?"), TEXT("Delete"), MB_ICONWARNING|MB_YESNO) == IDNO)
            return;

        do
        {
            DeleteString(hChild);
        }while(hChild = TreeView_GetNextSibling(hwndTree, hChild));
    }
    else
    {
        if(bTopItem && MessageBox(hwndMain, TEXT("Are you sure you want to delete this item?"), TEXT("Delete"), MB_YESNO) == IDNO)
            return;
    }

    String lookupName;
    TreeView_GetItemLookupName(hwndTree, hItem, lookupName);

    if(lookup->HasLookup(lookupName))
        lookup->RemoveLookupString(lookupName);

    if(bTopItem)
    {
        HTREEITEM hParent = TreeView_GetParent(hwndTree, hItem);

        while(true)
        {
            TreeView_DeleteItem(hwndTree, hItem);

            if(hParent == TreeView_GetRoot(hwndTree) || TreeView_GetChild(hwndTree, hParent) != NULL)
                break;

            TreeView_GetItemLookupName(hwndTree, hParent, lookupName);
            if(lookup->HasLookup(lookupName))
                break;

            hItem = hParent;
            hParent = TreeView_GetParent(hwndTree, hItem);
        }

        bFileChanged = true;
    }
}

void StringEditor::ToggleActive()
{
    HTREEITEM hItem = TreeView_GetSelection(hwndTree);
    if(hItem == TreeView_GetRoot(hwndTree) || !TreeView_GetChild(hwndTree, hItem))
        return;

    String lookupName;
    TreeView_GetItemLookupName(hwndTree, hItem, lookupName);

    if(lookup->HasLookup(lookupName))
    {
        CTSTR lpStr = lookup->LookupString(lookupName);
        if(lpStr && MessageBox(hwndMain, TEXT("Making this item inactive will remove any data it contains.  Are you sure?"), TEXT("Make Inactive"), MB_ICONWARNING|MB_YESNO) == IDNO)
            return;

        lookup->RemoveLookupString(lookupName);
    }
    else
        lookup->AddLookupString(lookupName, NULL);

    SelectItem();

    bFileChanged = true;
}

void StringEditor::SelectItem()
{
    SetWindowText(hwndEdit, NULL);

    HTREEITEM hItem = TreeView_GetSelection(hwndTree);

    //------------------------------------

    HMENU hMenu = GetMenu(hwndMain);
    hMenu = GetSubMenu(hMenu, 1);

    BOOL bRoot = hItem == TreeView_GetRoot(hwndTree);
    BOOL bCanToggleActive = TreeView_GetChild(hwndTree, hItem) != NULL && !bRoot;

    MENUITEMINFO mii;
    zero(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fState = !bCanToggleActive ? MFS_DISABLED : 0;
    SetMenuItemInfo(hMenu, 4, TRUE, &mii);

    mii.fState = bRoot ? MFS_DISABLED : 0;
    SetMenuItemInfo(hMenu, 1, TRUE, &mii);
    SetMenuItemInfo(hMenu, 2, TRUE, &mii);

    //------------------------------------

    if(bRoot)
    {
        EnableWindow(hwndEdit, FALSE);
        return;
    }

    String lookupName;
    TreeView_GetItemLookupName(hwndTree, hItem, lookupName);

    if(!lookup->HasLookup(lookupName))
    {
        mii.fMask = MIIM_STRING;
        mii.dwTypeData = TEXT("Make Active");
        SetMenuItemInfo(hMenu, 4, TRUE, &mii);

        EnableWindow(hwndEdit, FALSE);
    }
    else
    {
        mii.fMask = MIIM_STRING;
        mii.dwTypeData = TEXT("Make Inactive");
        SetMenuItemInfo(hMenu, 4, TRUE, &mii);

        CTSTR lpStr = lookup->LookupString(lookupName);
        SetWindowText(hwndEdit, lpStr);
        EnableWindow(hwndEdit, TRUE);
    }
}

void StringEditor::SaveItem(HTREEITEM hItem)
{
    if(!hItem || hItem == TreeView_GetRoot(hwndTree))
        return;

    String lookupName;
    TreeView_GetItemLookupName(hwndTree, hItem, lookupName);

    if(!lookup->HasLookup(lookupName))
        return;

    int textLen = SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0);
    String strText;
    strText.SetLength(textLen);
    GetWindowText(hwndEdit, strText.Array(), textLen+1);

    if(scmp(lookup->LookupString(lookupName), strText) != 0)
    {
        bFileChanged = true;
        lookup->AddLookupString(lookupName, strText);
    }
}

void StringEditor::RenameItem(HTREEITEM hItem, CTSTR lpNewName)
{
    String oldName, newName = lpNewName;
    TreeView_GetItemLookupName(hwndTree, hItem, oldName);
    TreeView_GetItemLookupName(hwndTree, TreeView_GetParent(hwndTree, hItem), newName, FALSE);

    if(scmpi(newName, oldName) == 0)
        return;

    RenameSubItems(hItem, newName);

    if(lookup->HasLookup(oldName))
    {
        lookup->AddLookupString(newName, lookup->LookupString(oldName));
        lookup->RemoveLookupString(oldName);
    }

    bFileChanged = true;
}

void StringEditor::RenameSubItems(HTREEITEM hItem, CTSTR lpParentName)
{
    HTREEITEM hChild = TreeView_GetChild(hwndTree, hItem);

    while(hChild)
    {
        TCHAR name[256];

        TVITEM tvi;
        zero(&tvi, sizeof(tvi));
        tvi.mask = TVIF_TEXT|TVIF_HANDLE;
        tvi.cchTextMax = 255;
        tvi.pszText = name;
        tvi.hItem = hChild;
        TreeView_GetItem(hwndTree, &tvi);

        String newName;
        newName << lpParentName << TEXT(".") << name;

        String oldName;
        TreeView_GetItemLookupName(hwndTree, hChild, oldName);

        RenameSubItems(hChild, newName);

        if(lookup->HasLookup(oldName))
        {
            lookup->AddLookupString(newName, lookup->LookupString(oldName));
            lookup->RemoveLookupString(oldName);
        }

        hChild = TreeView_GetNextSibling(hwndTree, hChild);
    }
}


LRESULT STDCALL StringEditor::MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_SETFOCUS:
            SetFocus(App->hwndEdit);
            break;

        case WM_ACTIVATE:
            if(LOWORD(wParam) != WA_INACTIVE)
                SetFocus(App->hwndEdit);
            break;

        case WM_ACTIVATEAPP:
            if(wParam == TRUE)
                SetFocus(App->hwndEdit);
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_FILE_OPEN:
                    App->SaveItem(TreeView_GetSelection(App->hwndTree));
                    App->Open();
                    break;

                case ID_FILE_NEW:
                    App->SaveItem(TreeView_GetSelection(App->hwndTree));
                    App->New();
                    break;

                case ID_FILE_SAVE:
                    App->SaveItem(TreeView_GetSelection(App->hwndTree));
                    App->Save();
                    break;

                case ID_FILE_SAVE_AS:
                    App->SaveItem(TreeView_GetSelection(App->hwndTree));
                    App->SaveAs();
                    break;

                case ID_FILE_EXIT:
                    App->SaveItem(TreeView_GetSelection(App->hwndTree));
                    PostQuitMessage(0);
                    break;

                case ID_NEW_STRING:
                    App->SaveItem(TreeView_GetSelection(App->hwndTree));
                    App->NewString();
                    break;

                case ID_NEW_SUBSTRING:
                    App->SaveItem(TreeView_GetSelection(App->hwndTree));
                    App->NewSubString();
                    break;

                case ID_EDIT_DELETE:
                    App->SaveItem(TreeView_GetSelection(App->hwndTree));
                    App->DeleteString();
                    break;

                case ID_EDIT_RENAME:
                    {
                        HTREEITEM hItem = TreeView_GetSelection(App->hwndTree);
                        if(hItem && hItem != TreeView_GetRoot(App->hwndTree))
                        {
                            App->SaveItem(hItem);
                            TreeView_EditLabel(App->hwndTree, hItem);
                        }
                        break;
                    }

                case ID_EDIT_TOGGLEACTIVE:
                    App->SaveItem(TreeView_GetSelection(App->hwndTree));
                    App->ToggleActive();
                    break;
            }
            break;

        case WM_NOTIFY:
            {
                NMHDR *nm = (NMHDR*)lParam;
                if(nm->hwndFrom == App->hwndTree)
                {
                    NMTREEVIEW *info = (NMTREEVIEW*)lParam;
                    if(nm->code == TVN_SELCHANGED)
                    {
                        if(info->itemNew.hItem != info->itemOld.hItem)
                        {
                            App->SaveItem(info->itemOld.hItem);
                            App->SelectItem();
                        }
                    }
                    else if(nm->code == TVN_BEGINLABELEDIT)
                    {
                        HTREEITEM hRoot = TreeView_GetRoot(App->hwndTree);
                        HTREEITEM hItem = TreeView_GetSelection(App->hwndTree);

                        if(hRoot == hItem)
                            return TRUE;
                    }
                    else if(nm->code == TVN_ENDLABELEDIT)
                    {
                        App->SaveItem(TreeView_GetSelection(App->hwndTree));

                        NMTVDISPINFO *dispInfo = (NMTVDISPINFO*)lParam;
                        if(!dispInfo->item.pszText || !*dispInfo->item.pszText)
                            return FALSE;

                        if( schr(dispInfo->item.pszText, '.')      != NULL ||
                            schr(dispInfo->item.pszText, ' ')      != NULL ||
                            schr(dispInfo->item.pszText, '\t')     != NULL ||
                            schr(dispInfo->item.pszText, L'@')    != NULL)
                        {
                            MessageBox(hwnd, TEXT("Name must not contain periods/spaces/tabs"), NULL, MB_ICONERROR);
                            return FALSE;
                        }

                        HTREEITEM hParent = TreeView_GetParent(App->GetTreeControl(), dispInfo->item.hItem);
                        if(TreeView_FindItemStringI(App->GetTreeControl(), hParent, dispInfo->item.pszText) != NULL)
                        {
                            MessageBox(hwnd, TEXT("That item name is already in use"), NULL, 0);
                            return FALSE;
                        }

                        App->RenameItem(dispInfo->item.hItem, dispInfo->item.pszText);
                        return TRUE;
                    }
                    else if(nm->code == NM_RCLICK)
                    {
                        HTREEITEM hItem = TreeView_GetDropHilight(App->hwndTree);
                        if(hItem)
                            TreeView_SelectItem(App->hwndTree, hItem);
                        else
                            hItem = TreeView_GetSelection(App->hwndTree);

                        if(hItem)
                        {
                            HMENU hMenu = GetMenu(App->hwndMain);
                            hMenu = GetSubMenu(hMenu, 1);

                            POINT p;
                            GetCursorPos(&p);
                            TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, hwnd, NULL);
                        }
                    }
                }
                break;
            }

        case WM_SIZING:
            {
                RECT &screenSize = *(RECT*)lParam;

                if(screenSize.bottom-screenSize.top < 600)
                    screenSize.bottom = screenSize.top+600;
                if(screenSize.right-screenSize.left < 800)
                    screenSize.right = screenSize.left+800;

                break;
            }

        case WM_SIZE:
            {
                RECT client;
                GetClientRect(hwnd, &client);

                SetWindowPos(App->hwndTree, NULL, 0, 0, 300, client.bottom, SWP_NOMOVE);
                SetWindowPos(App->hwndEdit, NULL, 300, 0, client.right-300, client.bottom, SWP_NOMOVE);
                break;
            }

        case WM_CLOSE:
            {
                App->SaveItem(TreeView_GetSelection(App->hwndTree));

                if(App->bFileChanged)
                {
                    switch(MessageBox(hwnd, TEXT("You have unsaved changes.  Would you like to save your file?"), TEXT("Unsaved Changes"), MB_YESNOCANCEL))
                    {
                        case IDYES:
                            App->Save();
                        case IDNO:
                            break;

                        default:
                            return 0;
                    }
                }

                PostQuitMessage(0);
                break;
            }
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

BOOL CALLBACK GetTextDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static NameDialogInfo *pInfo = NULL;

    switch(message)
    {
        case WM_INITDIALOG:
            pInfo = (NameDialogInfo*)lParam;
            SetFocus(GetDlgItem(hwnd, IDC_NAME));
            SendMessage(GetDlgItem(hwnd, IDC_NAME), EM_SETLIMITTEXT, 255, 0);
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    {
                        UINT len = SendMessage(GetDlgItem(hwnd, IDC_NAME), WM_GETTEXTLENGTH, 0, 0);
                        String val;
                        if(len)
                        {
                            val.SetLength(len);
                            SendMessage(GetDlgItem(hwnd, IDC_NAME), WM_GETTEXT, len+1, (LPARAM)val.Array());
                            val.KillSpaces();
                            len = val.Length();
                        }

                        if(!len)
                            MessageBox(hwnd, TEXT("Please enter a name"), NULL, MB_ICONERROR);
                        else
                        {
                            if( schr(val, '.')      != NULL ||
                                schr(val, ' ')      != NULL ||
                                schr(val, '\t')     != NULL ||
                                schr(val, L'@')    != NULL)
                            {
                                MessageBox(hwnd, TEXT("Name must not contain periods/spaces/tabs"), NULL, MB_ICONERROR);
                                break;
                            }

                            if(pInfo)
                            {
                                if(TreeView_FindItemStringI(App->GetTreeControl(), pInfo->hItem, val) != NULL)
                                {
                                    MessageBox(hwnd, TEXT("That item name is already in use"), NULL, 0);
                                    break;
                                }

                                pInfo->val = val;
                            }

                            EndDialog(hwnd, IDOK);
                        }
                        break;
                    }

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
            }
            break;

        case WM_CLOSE:
            EndDialog(hwnd, IDCANCEL);
    }

    return FALSE;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ModuleManagement

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


struct ModuleDialogInfo
{
    StringList InactiveModules;
    StringList ActiveModules;
    String strWorkingModule;
    int selectedList;
    int selectedItem;
};


ModuleDialogInfo *mdi = NULL;


void AddModuleToList(HWND hwndList, String &module, int insertPos);

void AddModuleToList(HWND hwndList, String &module, int insertPos)
{
    String strModuleConfig;
    strModuleConfig << TEXT("data/") << module << TEXT("/Module.ini");

    ConfigFile moduleInfo;
    moduleInfo.Open(strModuleConfig);

    LVITEM lvi;
    lvi.mask = LVIF_TEXT;
    lvi.iItem = insertPos;
    lvi.iSubItem = 0;
    lvi.pszText = module;
    int id = ListView_InsertItem(hwndList, &lvi);

    TSTR lpType;
    if(module.CompareI(mdi->strWorkingModule))
        lpType = TEXT("Work Module");
    else
        lpType = (moduleInfo.GetInt(TEXT("Module"), TEXT("Type"), 1) ? TEXT("Addon Module") : TEXT("Master Module"));
    ListView_SetItemText(hwndList, id, 1, lpType);
}

BOOL CALLBACK EditorEngine::SetActiveModulesDialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(EditorEngine::SetActiveModulesDialog);

    int i;

    switch(message)
    {
        case WM_INITDIALOG:
            {
                HWND activeList = GetDlgItem(hwnd, IDC_LOADINGMODULES);
                HWND inactiveList = GetDlgItem(hwnd, IDC_AVAILABLEMODULES);

                LVCOLUMN lvc;
                lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

                lvc.iSubItem = 0;
                lvc.pszText = TEXT("Module");
                lvc.cx = 80;

                ListView_InsertColumn(activeList, 0, &lvc);
                ListView_InsertColumn(inactiveList, 0, &lvc);

                //---------------------------------

                lvc.iSubItem = 1;
                lvc.pszText = TEXT("Type");
                lvc.cx = 100;

                ListView_InsertColumn(inactiveList, 1, &lvc);
                ListView_InsertColumn(activeList, 1, &lvc);

                ListView_SetExtendedListViewStyle(inactiveList, LVS_EX_FULLROWSELECT);
                ListView_SetExtendedListViewStyle(activeList, LVS_EX_FULLROWSELECT);

                //---------------------------------

                mdi = new ModuleDialogInfo;
                GetPossibleModules(mdi->InactiveModules);

                for(i=0; i<mdi->InactiveModules.Num(); i++)
                {
                    if(scmpi(mdi->InactiveModules[i], TEXT("Base")) != 0)
                    {
                        ConfigFile config;
                        String strModuleConfig;
                        strModuleConfig << TEXT("data/") << mdi->InactiveModules[i] << TEXT("/Module.ini");
                        config.Open(strModuleConfig);
                        if(!config.GetInt(TEXT("Module"), TEXT("Editable"), 1))
                            mdi->InactiveModules.Remove(i--);
                    }
                }

                mdi->InactiveModules.Remove(mdi->InactiveModules.FindValueIndexI(TEXT("Base")));
                AppConfig->GetStringList(TEXT("Engine"), TEXT("GameModule"), mdi->ActiveModules);

                List<CTSTR> curModules;
                Engine::GetGameModules(curModules);
                for(i=0; i<curModules.Num(); i++)
                {
                    if(scmpi(curModules[i], TEXT("Base")) != 0)
                    {
                        ConfigFile config;
                        String strModuleConfig;
                        strModuleConfig << TEXT("data/") << curModules[i] << TEXT("/Module.ini");
                        config.Open(strModuleConfig);

                        if(!config.GetInt(TEXT("Module"), TEXT("Editable"), 1))
                            continue;

                        mdi->ActiveModules.SafeAddI(curModules[i]);
                    }
                }
                curModules.Clear();

                for(i=0; i<mdi->ActiveModules.Num(); i++)
                {
                    int id = mdi->InactiveModules.FindValueIndexI(mdi->ActiveModules[i]);
                    if(id != INVALID)
                        mdi->InactiveModules.Remove(id);
                }

                //---------------------------------

                mdi->strWorkingModule = editor->curWorkingModule;

                LVITEM lvi;
                lvi.mask = LVIF_TEXT;

                for(i=0; i<mdi->InactiveModules.Num(); i++)
                {
                    String strModuleConfig;
                    strModuleConfig << TEXT("data/") << mdi->InactiveModules[i] << TEXT("/Module.ini");

                    ConfigFile moduleInfo;
                    moduleInfo.Open(strModuleConfig);

                    lvi.pszText = mdi->InactiveModules[i];
                    lvi.iItem = i;
                    lvi.iSubItem = 0;
                    int id = ListView_InsertItem(inactiveList, &lvi);

                    TSTR lpType = (moduleInfo.GetInt(TEXT("Module"), TEXT("Type"), 1) ? TEXT("Addon Module") : TEXT("Master Module"));
                    ListView_SetItemText(inactiveList, id, 1, lpType);
                }

                for(i=0; i<mdi->ActiveModules.Num(); i++)
                {
                    String strModuleConfig;
                    strModuleConfig << TEXT("data/") << mdi->ActiveModules[i] << TEXT("/Module.ini");

                    ConfigFile moduleInfo;
                    moduleInfo.Open(strModuleConfig);

                    lvi.pszText = mdi->ActiveModules[i];
                    lvi.iItem = i;
                    lvi.iSubItem = 0;
                    ListView_InsertItem(activeList, &lvi);

                    TSTR lpType;
                    if(mdi->ActiveModules[i].CompareI(mdi->strWorkingModule))
                        lpType = TEXT("Work Module");
                    else
                        lpType = (moduleInfo.GetInt(TEXT("Module"), TEXT("Type"), 1) ? TEXT("Addon Module") : TEXT("Master Module"));
                    ListView_SetItemText(activeList, i, 1, lpType);
                }

                break;
            }

        case WM_COMMAND:
            {
                switch(LOWORD(wParam))
                {
                    case IDC_INCLUDE:
                        {
                            String val = mdi->InactiveModules[mdi->selectedItem];

                            ListView_DeleteItem(GetDlgItem(hwnd, IDC_AVAILABLEMODULES), mdi->selectedItem);
                            AddModuleToList(GetDlgItem(hwnd, IDC_LOADINGMODULES), val, mdi->ActiveModules.Num());

                            mdi->InactiveModules.Remove(mdi->selectedItem);
                            mdi->ActiveModules.Add(val);

                            EnableWindow(GetDlgItem(hwnd, IDC_UP), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_DOWN), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_INCLUDE), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_EXCLUDE), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_SETASWORKINGMODULE), FALSE);
                            break;
                        }

                    case IDC_EXCLUDE:
                        {
                            StringList ExcludeStrings;
                            AppConfig->GetStringList(TEXT("Engine"), TEXT("GameModule"), ExcludeStrings);

                            String val = mdi->ActiveModules[mdi->selectedItem];

                            if(ExcludeStrings.HasValueI(val))
                            {
                                MessageBox(hwnd, TEXT("Cannot remove this module because it is defined as a game module in the editor ini file."), NULL, 0);
                                break;
                            }

                            if(mdi->strWorkingModule.CompareI(val))
                                mdi->strWorkingModule.Clear();

                            ListView_DeleteItem(GetDlgItem(hwnd, IDC_LOADINGMODULES), mdi->selectedItem);
                            AddModuleToList(GetDlgItem(hwnd, IDC_AVAILABLEMODULES), val, mdi->InactiveModules.Num());

                            mdi->ActiveModules.Remove(mdi->selectedItem);
                            mdi->InactiveModules.Add(val);

                            EnableWindow(GetDlgItem(hwnd, IDC_UP), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_DOWN), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_INCLUDE), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_EXCLUDE), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDC_SETASWORKINGMODULE), FALSE);
                            break;
                        }

                    case IDC_UP:
                        {
                            if(mdi->selectedItem > 0)
                            {
                                String val = mdi->ActiveModules[mdi->selectedItem];

                                ListView_DeleteItem(GetDlgItem(hwnd, IDC_LOADINGMODULES), mdi->selectedItem);

                                mdi->ActiveModules.Remove(mdi->selectedItem);
                                mdi->ActiveModules.Insert(--mdi->selectedItem, val);

                                AddModuleToList(GetDlgItem(hwnd, IDC_LOADINGMODULES), val, mdi->selectedItem);
                            }
                            break;
                        }

                    case IDC_DOWN:
                        {
                            if( (mdi->ActiveModules.Num() > 1)                      &&
                                (mdi->selectedItem < (mdi->ActiveModules.Num()-1))  )
                            {
                                String val = mdi->ActiveModules[mdi->selectedItem];

                                ListView_DeleteItem(GetDlgItem(hwnd, IDC_LOADINGMODULES), mdi->selectedItem);

                                mdi->ActiveModules.Remove(mdi->selectedItem);
                                mdi->ActiveModules.Insert(++mdi->selectedItem, val);

                                AddModuleToList(GetDlgItem(hwnd, IDC_LOADINGMODULES), val, mdi->selectedItem);
                            }
                            break;
                        }

                    case IDC_SETASWORKINGMODULE:
                        {
                            if(!mdi->strWorkingModule.CompareI(mdi->ActiveModules[mdi->selectedItem]))
                            {
                                if(mdi->strWorkingModule.IsValid())
                                {
                                    LVFINDINFO lvfi;
                                    lvfi.flags = LVFI_STRING;
                                    lvfi.psz = mdi->strWorkingModule;

                                    int id = ListView_FindItem(GetDlgItem(hwnd, IDC_LOADINGMODULES), -1, &lvfi);

                                    if(id != -1)
                                    {
                                        ConfigFile moduleInfo;

                                        String strModuleConfig;
                                        strModuleConfig << TEXT("data/") << mdi->strWorkingModule << TEXT("/Module.ini");
                                        moduleInfo.Open(strModuleConfig);

                                        TSTR lpType = (moduleInfo.GetInt(TEXT("Module"), TEXT("Type"), 1) ? TEXT("Addon Module") : TEXT("Master Module"));
                                        ListView_SetItemText(GetDlgItem(hwnd, IDC_LOADINGMODULES), id, 1, lpType);
                                    }
                                }

                                mdi->strWorkingModule = mdi->ActiveModules[mdi->selectedItem];
                                ListView_SetItemText(GetDlgItem(hwnd, IDC_LOADINGMODULES), mdi->selectedItem, 1, TEXT("Work Module"));
                            }
                            break;
                        }

                    case IDOK:
                        {
                            if(mdi->strWorkingModule.IsEmpty())
                            {
                                MessageBox(hwnd, TEXT("You must set a working module to load"), NULL, 0);
                                break;
                            }

                            if(editor->curWorkingModule.IsValid() && !mdi->strWorkingModule.CompareI(editor->curWorkingModule))
                            {
                                if(levelInfo->bModified)
                                {
                                    switch(MessageBox(hwnd, TEXT("Changing the current working module will unload the current level.  Would you like to save your changes?"), TEXT("Are you sure?"), MB_YESNOCANCEL))
                                    {
                                        case IDYES:
                                            SendMessage(hwndEditor, WM_COMMAND, MAKEWPARAM(ID_FILE_SAVELEVEL, 0), NULL);
                                            break;

                                        case IDCANCEL:
                                            return FALSE;
                                    }
                                }

                                String strModuleConfig;
                                strModuleConfig << TEXT("data/") << mdi->strWorkingModule << TEXT("/Module.ini");

                                ConfigFile moduleInfo;
                                moduleInfo.Open(strModuleConfig);

                                if(moduleInfo.GetInt(TEXT("Module"), TEXT("Type"), 1) == 0)
                                {
                                    if(MessageBox(hwnd, TEXT("Warning: Changing the current working module to a master module allows the ability to modify critical data directly.  Are you sure you want to do this?"), TEXT("Are you sure?"), MB_YESNO) == IDNO)
                                        return FALSE;
                                }

                                //------------------------------------------------------

                                ConfigFile curModuleConfig;
                                if( levelInfo && levelInfo->strLevelName.IsValid() &&
                                    editor->curWorkingModule.IsValid() &&
                                    curModuleConfig.Open(String() << TEXT("data/") << editor->curWorkingModule << TEXT("/Module.ini"), TRUE))
                                {
                                    AppConfig->SetString(TEXT("Settings"), TEXT("LastLevel"), levelInfo->strLevelName);
                                }

                                String lastLevel = AppConfig->GetString(TEXT("Settings"), TEXT("LastLevel"));

                                editor->KillViewports();

                                DestroyObject(levelInfo);
                                DestroyObject(level);

                                levelInfo = NULL;
                                level = NULL;

                                //-----------------------------------------------------

                                int id = mdi->ActiveModules.FindValueIndexI(mdi->strWorkingModule);
                                if(id != INVALID) mdi->ActiveModules.Remove(id);

                                StringList ExcludeStrings;
                                AppConfig->GetStringList(TEXT("Engine"), TEXT("GameModule"), ExcludeStrings);
                                moduleInfo.SetStringList(TEXT("Module"), TEXT("Dependancies"), mdi->ActiveModules);

                                if(!ExcludeStrings.HasValueI(editor->curWorkingModule))
                                    UnloadGameModule(editor->curWorkingModule);

                                for(i=editor->DependantModules.Num()-1; i>=0; i--)
                                    UnloadGameModule(editor->DependantModules[i]);

                                //-----------------------------------------------------

                                for(i=0; i<mdi->ActiveModules.Num(); i++)
                                {
                                    String &strModule = mdi->ActiveModules[i];

                                    LoadGameModule(strModule);

                                    if(ExcludeStrings.HasValueI(strModule))
                                        mdi->ActiveModules.Remove(i--);
                                }

                                editor->bAddonModule = moduleInfo.GetInt(TEXT("Module"), TEXT("Type"), 1);
                                editor->DependantModules.CopyList(mdi->ActiveModules);

                                LoadGameModule(mdi->strWorkingModule);

                                if(lastLevel.IsEmpty() || !editor->OpenEditableLevel(lastLevel, FALSE))
                                {
                                    if(AppConfig->GetInt(TEXT("Settings"), TEXT("DefaultLevel"), 0) == 0)
                                        editor->NewIndoorLevel(FALSE);
                                    else
                                        editor->NewOctLevel(FALSE);
                                }
                            }
                            else
                            {
                                String strModuleConfig;
                                strModuleConfig << TEXT("data/") << mdi->strWorkingModule << TEXT("/Module.ini");

                                ConfigFile moduleInfo;
                                moduleInfo.Open(strModuleConfig);

                                int id = mdi->ActiveModules.FindValueIndexI(mdi->strWorkingModule);
                                if(id != INVALID) mdi->ActiveModules.Remove(id);

                                StringList ExcludeStrings;
                                AppConfig->GetStringList(TEXT("Engine"), TEXT("GameModule"), ExcludeStrings);
                                moduleInfo.SetStringList(TEXT("Module"), TEXT("Dependancies"), mdi->ActiveModules);

                                for(i=0; i<mdi->ActiveModules.Num(); i++)
                                {
                                    String &strModule = mdi->ActiveModules[i];

                                    LoadGameModule(strModule);

                                    if(ExcludeStrings.HasValueI(strModule))
                                        mdi->ActiveModules.Remove(i--);
                                }

                                for(i=0; i<editor->DependantModules.Num(); i++)
                                {
                                    if(!mdi->ActiveModules.HasValueI(editor->DependantModules[i]))
                                        UnloadGameModule(editor->DependantModules[i]);
                                }

                                editor->DependantModules.CopyList(mdi->ActiveModules);

                                LoadGameModule(mdi->strWorkingModule);
                            }

                            editor->curWorkingModule = mdi->strWorkingModule;

                            EndDialog(hwnd, IDOK);
                            break;
                        }

                    case IDCANCEL:
                        EndDialog(hwnd, IDCANCEL);
                }
                break;
            }

        case WM_NOTIFY:
            {
                NMITEMACTIVATE *nmia = (NMITEMACTIVATE*)lParam;

                switch(nmia->hdr.code)
                {
                    case NM_CLICK:
                        {
                            if(wParam == IDC_AVAILABLEMODULES)
                            {
                                if(nmia->iItem == -1)
                                    break;

                                ListView_SetCheckState(GetDlgItem(hwnd, IDC_LOADINGMODULES), -1, FALSE);
                                mdi->selectedList = 0;

                                EnableWindow(GetDlgItem(hwnd, IDC_UP), FALSE);
                                EnableWindow(GetDlgItem(hwnd, IDC_DOWN), FALSE);

                                EnableWindow(GetDlgItem(hwnd, IDC_INCLUDE), TRUE);
                                EnableWindow(GetDlgItem(hwnd, IDC_EXCLUDE), FALSE);

                                EnableWindow(GetDlgItem(hwnd, IDC_SETASWORKINGMODULE), FALSE);
                            }
                            else if(wParam == IDC_LOADINGMODULES)
                            {
                                if(nmia->iItem == -1)
                                    break;

                                ListView_SetCheckState(GetDlgItem(hwnd, IDC_AVAILABLEMODULES), -1, FALSE);
                                mdi->selectedList = 1;

                                EnableWindow(GetDlgItem(hwnd, IDC_UP), TRUE);
                                EnableWindow(GetDlgItem(hwnd, IDC_DOWN), TRUE);

                                EnableWindow(GetDlgItem(hwnd, IDC_INCLUDE), FALSE);
                                EnableWindow(GetDlgItem(hwnd, IDC_EXCLUDE), TRUE);

                                EnableWindow(GetDlgItem(hwnd, IDC_SETASWORKINGMODULE), TRUE);
                            }
                            else
                                break;

                            mdi->selectedItem = nmia->iItem;

                            String moduleName;
                            if(mdi->selectedList == 0)
                                moduleName = mdi->InactiveModules[mdi->selectedItem];
                            else
                                moduleName = mdi->ActiveModules[mdi->selectedItem];

                            String strTemp;
                            strTemp << TEXT("data/") << moduleName << TEXT("/Module.ini");

                            ConfigFile moduleInfo;
                            moduleInfo.Open(strTemp);

                            strTemp = moduleInfo.GetString(TEXT("Module"), TEXT("Name"));
                            SetWindowText(GetDlgItem(hwnd, IDC_NAME), strTemp);

                            strTemp = moduleInfo.GetString(TEXT("Module"), TEXT("Description"));
                            strTemp.FindReplace(TEXT("\\r\\n"), TEXT("\r\n"));
                            SetWindowText(GetDlgItem(hwnd, IDC_DESCRIPTION), strTemp);

                            break;
                        }
                }
                break;
            }

        case WM_CLOSE:
            EndDialog(hwnd, IDCANCEL);
            break;

        case WM_DESTROY:
            {
                delete mdi;
                mdi = NULL;
                break;
            }

    }

    return FALSE;

    traceOut;
}

BOOL CALLBACK CreateModuleDialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(CreateModuleDialog);

    if(message == WM_INITDIALOG)
    {
        SendMessage(GetDlgItem(hwnd, IDC_ADDON), BM_SETCHECK, BST_CHECKED, 0);
    }
    else if(message == WM_COMMAND)
    {
        switch(LOWORD(wParam))
        {
            case IDOK:
                {
                    String strName, strFullName, strDescription;
                    strName.SetLength(SendMessage(GetDlgItem(hwnd, IDC_NAME), WM_GETTEXTLENGTH, 0, 0));
                    if(!strName.Length())
                    {
                        MessageBox(hwnd, TEXT("You must enter a name for your module."), NULL, 0);
                        break;
                    }

                    SendMessage(GetDlgItem(hwnd, IDC_NAME), WM_GETTEXT, strName.Length()+1, (LPARAM)strName.Array());

                    if( schr(strName, '\\') ||
                        schr(strName, '/')  ||
                        schr(strName, '"')  ||
                        schr(strName, '?')  ||
                        schr(strName, ':')  ||
                        schr(strName, '|')  ||
                        schr(strName, '*')  ||
                        schr(strName, '<')  ||
                        schr(strName, '>')  )
                    {
                        MessageBox(hwnd, TEXT("Your module name contains invalid characters (\\ / | * ? : \" < >)"), NULL, 0);
                        break;
                    }

                    strName.KillSpaces();

                    BOOL bAddon = (SendMessage(GetDlgItem(hwnd, IDC_MASTER), BM_GETCHECK, 0, 0) != BST_CHECKED);

                    strFullName.SetLength(SendMessage(GetDlgItem(hwnd, IDC_FULLNAME), WM_GETTEXTLENGTH, 0, 0));
                    strDescription.SetLength(SendMessage(GetDlgItem(hwnd, IDC_DESCRIPTION), WM_GETTEXTLENGTH, 0, 0));

                    if(strFullName.Length())
                        SendMessage(GetDlgItem(hwnd, IDC_FULLNAME), WM_GETTEXT, strFullName.Length()+1, (LPARAM)strFullName.Array());
                    if(strDescription.Length())
                    {
                        SendMessage(GetDlgItem(hwnd, IDC_DESCRIPTION), WM_GETTEXT, strDescription.Length()+1, (LPARAM)strDescription.Array());
                        strDescription.FindReplace(TEXT("\r\n"), TEXT("\\r\\n"));
                    }

                    //------------------------------------------------------

                    if(levelInfo->bModified)
                    {
                        switch(MessageBox(hwnd, TEXT("Changing the current working module will unload the current level.  Would you like to save your changes?"), TEXT("Are you sure?"), MB_YESNOCANCEL))
                        {
                            case IDYES:
                                SendMessage(hwndEditor, WM_COMMAND, MAKEWPARAM(ID_FILE_SAVELEVEL, 0), NULL);
                                break;

                            case IDCANCEL:
                                EndDialog(hwnd, IDCANCEL);
                                return FALSE;
                        }
                    }

                    String strModulePath;
                    strModulePath << TEXT("data/") << strName;

                    if(OSFileExists(strModulePath))
                    {
                        MessageBox(hwnd, TEXT("A module of that name already exists"), NULL, 0);
                        break;
                    }

                    OSCreateDirectory(strModulePath);

                    ConfigFile moduleInfo;
                    if(!moduleInfo.Open(strModulePath + TEXT("/Module.ini"), TRUE))
                    {
                        MessageBox(hwnd, TEXT("Could not create module for some unknown..  unexplained..  mysterious reason."), NULL, 0);
                        break;
                    }

                    ConfigFile curModuleConfig;
                    if( levelInfo && levelInfo->strLevelName.IsValid() &&
                        editor->curWorkingModule.IsValid() &&
                        curModuleConfig.Open(String() << TEXT("data/") << editor->curWorkingModule << TEXT("/Module.ini"), TRUE))
                    {
                        AppConfig->SetString(TEXT("Settings"), TEXT("LastLevel"), levelInfo->strLevelName);
                    }

                    //------------------------------------------------------

                    moduleInfo.SetInt(TEXT("Module"), TEXT("Type"), bAddon);
                    if(strFullName.IsValid())
                        moduleInfo.SetString(TEXT("Module"), TEXT("Name"), strFullName);
                    if(strDescription.IsValid())
                        moduleInfo.SetString(TEXT("Module"), TEXT("Description"), strDescription);

                    //------------------------------------------------------

                    StringList ExcludeStrings;
                    AppConfig->GetStringList(TEXT("Engine"), TEXT("GameModule"), ExcludeStrings);
                    moduleInfo.SetStringList(TEXT("Module"), TEXT("Dependancies"), ExcludeStrings);

                    moduleInfo.SetString(TEXT("Module"), TEXT("Language"), GetLanguage());

                    if(!ExcludeStrings.HasValueI(editor->curWorkingModule))
                        UnloadGameModule(editor->curWorkingModule);

                    for(int i=editor->DependantModules.Num()-1; i>=0; i--)
                        UnloadGameModule(editor->DependantModules[i]);

                    //------------------------------------------------------

                    editor->bAddonModule = bAddon;
                    editor->curWorkingModule = strName;
                    editor->DependantModules.Clear();

                    LoadGameModule(editor->curWorkingModule);

                    if(AppConfig->GetInt(TEXT("Settings"), TEXT("DefaultLevel"), 0) == 0)
                        editor->NewIndoorLevel(FALSE);
                    else
                        editor->NewOctLevel(FALSE);

                    EndDialog(hwnd, IDOK);
                    break;
                }

            case IDCANCEL:
                EndDialog(hwnd, IDCANCEL);
        }
    }

    return FALSE;

    traceOut;
}

BOOL CALLBACK EditorStartupDialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(EditorStartupDialog);

    if(message == WM_COMMAND)
    {
        switch(LOWORD(wParam))
        {
            case IDC_CREATEMODULE:
                {
                    if(DialogBox(hinstMain, MAKEINTRESOURCE(IDD_CREATEMODULE), hwnd, (DLGPROC)CreateModuleDialog) != IDCANCEL)
                        EndDialog(hwnd, IDOK);
                    break;
                }

            case IDC_OPENEXISTINGMODULE:
                {
                    if(DialogBox(hinstMain, MAKEINTRESOURCE(IDD_LOADMODULES), hwnd, (DLGPROC)EditorEngine::SetActiveModulesDialog) != IDCANCEL)
                        EndDialog(hwnd, IDOK);
                    break;
                }

            case IDCANCEL:
                EndDialog(hwnd, IDCANCEL);
        }
    }

    return FALSE;

    traceOut;
}

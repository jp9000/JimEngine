/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ScriptEditor

  Copyright (c) 2009, Hugh Bailey
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

#include <ScriptDefs.h>
#include <ScriptCompiler.h>
#include <ScriptByteCode.h>


LRESULT ENGINEAPI ScriptEditorWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT ENGINEAPI ScriptEditControlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT ENGINEAPI ScriptErrorWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


List<ScriptEditor*> ScriptEditor::OpenScripts;

ScriptErrorWindow *scriptErrorWindow = NULL;

WNDPROC editProc = NULL;
//int dataPos = 0;


void ScriptEditor::RegisterWindowClasses()
{
    WNDCLASS wc;
    zero(&wc, sizeof(wc));

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)ScriptEditorWndProc;
    wc.hInstance = hinstMain;
    wc.hIcon = LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_SCRIPTMENU);
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
    wc.lpszClassName = TEXT("ScriptEditor");
    wc.cbWndExtra = sizeof(LPVOID);

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the script editor window classes"));

    wc.lpfnWndProc = (WNDPROC)ScriptErrorWndProc;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("ScriptErrorWindow");
    wc.cbWndExtra = 0;

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the script editor window classes"));
}

ScriptEditor::ScriptEditor()
{
    int borderXSize = 500;
    int borderYSize = 400;

    borderXSize += GetSystemMetrics(SM_CXSIZEFRAME)*2;

    borderYSize += GetSystemMetrics(SM_CYSIZEFRAME)*2;
    borderYSize += GetSystemMetrics(SM_CYMENU);
    borderYSize += GetSystemMetrics(SM_CYCAPTION);

    //------------------------------------------

    hwndScriptEditor = CreateWindow(TEXT("ScriptEditor"), TEXT("Script Editor"), 
                                    WS_VISIBLE|WS_OVERLAPPEDWINDOW,
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    borderXSize, borderYSize,
                                    hwndEditor, NULL, hinstMain, NULL);

    SetWindowLongPtr(hwndScriptEditor, 0, (LONG_PTR)this);

    //------------------------------------------

    //todo: create own rich edit control later because windows rich edit control is a piece of total garbage - and I mean *total* garbage.  I'd sooner use a normal edit control.
    hwndEdit = CreateWindowEx(WS_EX_RIGHTSCROLLBAR, TEXT("Edit"), NULL, WS_CHILDWINDOW|WS_VISIBLE|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|WS_VSCROLL|WS_HSCROLL, 0, 0, 500, 400, hwndScriptEditor, NULL, hinstMain, NULL);
    SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hCourierFont, 0);

    if(!editProc)
        editProc = (WNDPROC)GetWindowLongPtr(hwndEdit, GWLP_WNDPROC);
    SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)ScriptEditControlProc);

    SetFocus(hwndEdit);

    //------------------------------------------

    OpenScripts << this;
}

ScriptEditor::~ScriptEditor()
{
    DestroyWindow(hwndScriptEditor);

    OpenScripts.RemoveItem(this);
}

void ScriptEditor::InitData()
{
    String strTitle;

    if(!curEntity->objScript)
        curEntity->objScript = new ObjectScript;

    DWORD id = curEntity->objScript->FunctionIDs.FindValueIndex(functionID);

    if(id != INVALID)
    {
        SendMessage(hwndEdit, WM_SETTEXT, (WPARAM)0, (LPARAM)curEntity->objScript->ScriptList[id].Array());
        bModified = FALSE;
    }
    else
    {
        bWasInvalid = TRUE;

        FunctionDefinition *functionDef = curEntity->GetObjectClass()->GetTopmostFunction(functionID);
        String &strScript = *curEntity->objScript->ScriptList.CreateNew();
        curEntity->objScript->FunctionIDs.Add(functionID);

        strScript << functionDef->returnType.name << TEXT(" ") << functionDef->name << TEXT("(");

        for(int i=0; i<functionDef->Params.Num(); i++)
        {
            if(i > 0)
                strScript << TEXT(", ");

            DefaultVariable &var = functionDef->Params[i];

            if(var.flags & VAR_OUT) strScript << TEXT("out ");
            strScript << var.typeInfo.name << TEXT(" ") << var.name;
        }

        strScript << TEXT(")\r\n{\r\n    ");
        int pos = strScript.Length();
        strScript << TEXT("\r\n}\r\n");

        SendMessage(hwndEdit, WM_SETTEXT, (WPARAM)0, (LPARAM)strScript.Array());
        SendMessage(hwndEdit, EM_SETSEL, (WPARAM)pos, (LPARAM)pos);

        bModified = TRUE;
    }

    UpdateTitle();
}

void ScriptEditor::Save()
{
    int len = SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0);

    if(len < 7)
    {
        if(MessageBox(hwndScriptEditor, TEXT("Would you like to clear this script and close?"), TEXT("Clear and close?"), MB_YESNO) == IDYES)
            ClearAndClose();
        return;
    }

    String strBuffer;
    strBuffer.SetLength(len);

    SendMessage(hwndEdit, WM_GETTEXT, (WPARAM)len+1, (LPARAM)strBuffer.Array());

    DWORD id = curEntity->objScript->FunctionIDs.FindValueIndex(functionID);
    assert(id != INVALID);
    curEntity->objScript->ScriptList[id] = strBuffer;
    bModified = FALSE;
    bWasInvalid = FALSE;

    UpdateTitle();

    if(objectProperties && objectProperties->bScriptsTab && (levelInfo->SelectedObjects.Num() == 1) && (levelInfo->SelectedObjects[0] == curEntity))
        objectProperties->UpdateProperties();

    String strErrors;
    curEntity->objScript->Compile(curEntity, strErrors, id);

    if(!strErrors.IsEmpty())
        ScriptErrorWindow::ShowErrors(strErrors);
}

void ScriptEditor::ClearAndClose()
{
    if(MessageBox(hwndScriptEditor, TEXT("The current script data will be lost.  Are you sure you wish to clear this script event?"), TEXT("Discard current script?"), MB_YESNO) == IDNO)
        return;

    DWORD id = curEntity->objScript->FunctionIDs.FindValueIndex(functionID);
    if(id != INVALID)
    {
        curEntity->objScript->FunctionIDs.Remove(id);
        curEntity->objScript->ScriptList.Remove(id);
        if(curEntity->objScript->Functions.Num() > id)
        {
            curEntity->objScript->Functions[id].FreeData();
            curEntity->objScript->Functions.Remove(id);
        }

        if(!curEntity->objScript->FunctionIDs.Num())
        {
            delete curEntity->objScript;
            curEntity->objScript = NULL;
        }
    }

    if(objectProperties && objectProperties->bScriptsTab && (levelInfo->SelectedObjects.Num() == 1) && (levelInfo->SelectedObjects[0] == curEntity))
        objectProperties->UpdateProperties();

    CloseScript(FALSE);
}


void ScriptEditor::CheckForErrors()
{
    DWORD id = curEntity->objScript->FunctionIDs.FindValueIndex(functionID);
    assert(id != INVALID);
    if(id == INVALID)
        return;

    String &curScript = curEntity->objScript->ScriptList[id];
    String strPrevScriptData = curScript;

    int len = SendMessage(hwndEdit, WM_GETTEXTLENGTH, 0, 0);
    curScript.SetLength(len);
    SendMessage(hwndEdit, WM_GETTEXT, (WPARAM)len+1, (LPARAM)curScript.Array());

    String strErrors;
    curEntity->objScript->Compile(curEntity, strErrors, id);

    if(!strErrors.IsEmpty())
        ScriptErrorWindow::ShowErrors(strErrors);
    else
        ScriptErrorWindow::ShowErrors(String(TEXT("Compile Successful.  No Errors/Warnings")));

    curScript = strPrevScriptData;
}


void ScriptEditor::CloseScript(bool bAskToSave)
{
    if(bModified && bAskToSave)
    {
        switch(MessageBox(hwndScriptEditor, TEXT("You have unsaved changes to this script.  Would you like to save your data?"), TEXT("Save Changes?"), MB_YESNOCANCEL))
        {
            case IDYES:
                Save();
                break;
            case IDCANCEL:
                return;
        }
    }

    if(bWasInvalid)
    {
        DWORD id = curEntity->objScript->FunctionIDs.FindValueIndex(functionID);
        assert(id != INVALID);

        curEntity->objScript->FunctionIDs.Remove(id);
        curEntity->objScript->ScriptList.Remove(id);
        if(curEntity->objScript->Functions.Num() > id)
        {
            curEntity->objScript->Functions[id].FreeData();
            curEntity->objScript->Functions.Remove(id);
        }

        if(!curEntity->objScript->FunctionIDs.Num())
        {
            delete curEntity->objScript;
            curEntity->objScript = NULL;
        }
    }
    delete this;
}

void ScriptEditor::SetModified(bool bSet)
{
    if(bSet != bModified)
    {
        bModified = bSet;
        UpdateTitle();
    }
}

void ScriptEditor::UpdateTitle()
{
    String strTitle;

    FunctionDefinition *funcDef = curEntity->GetObjectClass()->GetTopmostFunction(functionID);
    strTitle << curEntity->GetName() << TEXT(":") << funcDef->name; 

    if(bModified) strTitle << TEXT("*");
    strTitle << TEXT(" - Script Editor");

    SendMessage(hwndScriptEditor, WM_SETTEXT, (WPARAM)0, (LPARAM)strTitle.Array());
}



LRESULT ENGINEAPI ScriptEditorWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_COMMAND:
            {
                ScriptEditor *scriptEditor = (ScriptEditor*)GetWindowLongPtr(hwnd, 0);

                switch(LOWORD(wParam))
                {
                    case ID_SCRIPT_SAVE:
                        scriptEditor->Save();
                        break;
                    case ID_SCRIPT_CLEAREVENTANDCLOSE:
                        scriptEditor->ClearAndClose();
                        break;

                    case ID_MYEDIT_UNDO:
                        break;
                    case ID_MYEDIT_REDO:
                        break;
                    case ID_MYEDIT_CUT:
                        break;
                    case ID_MYEDIT_COPY:
                        break;
                    case ID_MYEDIT_PASTE:
                        break;
                    case ID_MYEDIT_DELETE:
                        break;

                    case ID_SCRIPTCHECK:
                        scriptEditor->CheckForErrors();
                        break;

                    case ID_SCRIPT_CLOSE:
                        scriptEditor->CloseScript();
                }
                break;
            }

        case WM_SIZE:
            {
                ScriptEditor *scriptEditor = (ScriptEditor*)GetWindowLongPtr(hwnd, 0);

                if(!scriptEditor)
                    break;

                int cx = LOWORD(lParam);
                int cy = HIWORD(lParam);

                SetWindowPos(scriptEditor->hwndEdit, NULL, 0, 0, cx, cy, SWP_NOMOVE);
                break;
            }

        case WM_CLOSE:
            {
                ScriptEditor *scriptEditor = (ScriptEditor*)GetWindowLongPtr(hwnd, 0);
                scriptEditor->CloseScript();
                break;
            }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int GetIndentCount(TSTR lpStr);

int GetIndentCount(TSTR lpStr)
{
    TSTR lpTemp = lpStr;
    int count = 0;

    if(lpTemp)
    {
        do
        {
            if(*lpTemp == '{')
                ++count;
            else if(*lpTemp == '}' && count)
                --count;
        }while(*++lpTemp);
    }
    return count;
}

//simple sublcass of the edit control that auto-indents text.  later on I'm going to have to write a custom control for coloring
LRESULT ENGINEAPI ScriptEditControlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_CHAR)
    {
        ScriptEditor *scriptEditor = (ScriptEditor*)GetWindowLongPtr(GetParent(hwnd), 0);
        scriptEditor->SetModified(TRUE);

        if(wParam == '\r')
        {
            LRESULT ret = CallWindowProc((FARPROC)editProc, hwnd, message, wParam, lParam);

            String strText;

            int curPos = 0;
            SendMessage(hwnd, EM_GETSEL, (WPARAM)&curPos, 0);

            strText.SetLength(curPos);
            SendMessage(hwnd, WM_GETTEXT, curPos+1, (LPARAM)strText.Array());

            int indents = GetIndentCount(strText);

            for(int i=0; i<indents; i++)
            {
                for(int j=0; j<4; j++)
                    CallWindowProc((FARPROC)editProc, hwnd, message, ' ', 0);
            }

            return ret;
        }
        else if(wParam == '\t')
        {
            for(int j=0; j<4; j++)
                CallWindowProc((FARPROC)editProc, hwnd, message, ' ', 0);

            return 0;
        }
        else if(wParam == '}')
        {
            String strText;

            int curPos = 0;
            SendMessage(hwnd, EM_GETSEL, (WPARAM)&curPos, 0);

            strText.SetLength(curPos);
            SendMessage(hwnd, WM_GETTEXT, curPos+1, (LPARAM)strText.Array());

            int indents = GetIndentCount(strText);

            if(indents)
            {
                CTSTR lpTemp = strText.Array()+strText.Length();

                BOOL bFoundNonSpace = FALSE;
                int maxSpaces = 0;
                while(lpTemp-- != strText.Array())
                {
                    if(*lpTemp == '\n')
                        break;
                    else if(*lpTemp == 0)
                        continue;
                    else if(*lpTemp != ' ')
                    {
                        bFoundNonSpace = TRUE;
                        break;
                    }
                    else
                        ++maxSpaces;
                }

                if(!bFoundNonSpace)
                {
                    maxSpaces = MIN(maxSpaces, 4);
                    for(int j=0; j<maxSpaces; j++)
                        CallWindowProc((FARPROC)editProc, hwnd, message, 8, 0);
                }
            }
        }
    }
    return CallWindowProc((FARPROC)editProc, hwnd, message, wParam, lParam);
}


ScriptErrorWindow::ScriptErrorWindow()
{
    int borderXSize = 400;
    int borderYSize = 300;

    borderXSize += GetSystemMetrics(SM_CXSIZEFRAME)*2;

    borderYSize += GetSystemMetrics(SM_CYSIZEFRAME)*2;
    borderYSize += GetSystemMetrics(SM_CYCAPTION);

    //------------------------------------------

    hwndScriptErrors = CreateWindow(TEXT("ScriptErrorWindow"), TEXT("Script Errors"), 
                                    WS_VISIBLE|WS_OVERLAPPEDWINDOW,
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    borderXSize, borderYSize,
                                    hwndEditor, NULL, hinstMain, NULL);

    hwndEdit = CreateWindowEx(WS_EX_RIGHTSCROLLBAR, TEXT("Edit"), NULL, WS_CHILDWINDOW|WS_VISIBLE|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|WS_VSCROLL|WS_HSCROLL|ES_READONLY, 0, 0, 400, 300, hwndScriptErrors, NULL, hinstMain, NULL);
    SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hCourierFont, 0);

    scriptErrorWindow = this;
}

ScriptErrorWindow::~ScriptErrorWindow()
{
    DestroyWindow(hwndScriptErrors);

    scriptErrorWindow = NULL;
}

void ScriptErrorWindow::ShowErrors(String &strErrors)
{
    if(!scriptErrorWindow)
        new ScriptErrorWindow;
    else
    {
        ShowWindow(scriptErrorWindow->hwndScriptErrors, SW_RESTORE);
        SetWindowPos(scriptErrorWindow->hwndScriptErrors, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
    }

    SetWindowText(scriptErrorWindow->hwndEdit, strErrors);
}

LRESULT ENGINEAPI ScriptErrorWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_SIZE)
    {
        if(scriptErrorWindow)
        {
            int cx = LOWORD(lParam);
            int cy = HIWORD(lParam);

            SetWindowPos(scriptErrorWindow->hwndEdit, NULL, 0, 0, cx, cy, SWP_NOMOVE);
        }
    }
    else if(message == WM_CLOSE)
        delete scriptErrorWindow;

    return DefWindowProc(hwnd, message, wParam, lParam);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ObjectPropertiesEditor.h

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
#include <ScriptDefs.h>
#include <ScriptCompiler.h>
#include <ScriptByteCode.h>


ObjectPropertiesEditor *objectProperties;

LRESULT WINAPI ObjectPropertiesWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI ObjectPropertiesContainerWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

String GetPropertyStringVal(Object *obj, PropertyVariable *propVar);
void SetPropertyStringVal(Object *obj, PropertyVariable *propVar, CTSTR lpString);



#define GetVarAddress(obj, var) (((LPBYTE)(obj))+var->offset)

BOOL bSaveScollerUndo = TRUE;


#define VAR_ID              5000
#define VARBROWSE_ID        5100
#define VARSCROLLER_ID      5200
#define VAREDITSCRIPT_ID    5300
#define VARCLEARSCRIPT_ID   5400
#define VARENUM_ID          5500
#define VARMAX_ID           5600

#define VARSCRIPTSTATUS_ID  6000

#define IDC_TABCONTROLTHING 4000


struct SectionInfo
{
    String name;
    List<PropertyVariable*> Vars;
};


void ObjectPropertiesEditor::RegisterWindowClasses()
{
    traceIn(ObjectPropertiesEditor::RegisterWindowClasses);

    WNDCLASS wc;
    zero(&wc, sizeof(wc));

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)ObjectPropertiesWindowProc;
    wc.hInstance = hinstMain;
    wc.hIcon = LoadIcon(hinstMain, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
    wc.lpszClassName = TEXT("ObjectProperties");

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the material editor window class"));

    zero(&wc, sizeof(wc));
    wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = (WNDPROC)ObjectPropertiesContainerWindowProc;
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_3DFACE);
    wc.lpszClassName = TEXT("XR3DObjectPropertiesControlContainer");
    wc.cbWndExtra = 0;

    if(!RegisterClass(&wc))
        CrashError(TEXT("Could not register the material view class"));

    traceOut;
}

#define OP_WIDTH  400
#define OP_HEIGHT 450

#define CONTAINER_HEIGHT 425


ObjectPropertiesEditor::ObjectPropertiesEditor()
{
    traceIn(ObjectPropertiesEditor::ObjectPropertiesEditor);

    objectProperties = this;

    int borderXSize = OP_WIDTH;
    int borderYSize = OP_HEIGHT;

    borderXSize += GetSystemMetrics(SM_CXDLGFRAME)*2;

    borderYSize += GetSystemMetrics(SM_CYDLGFRAME)*2;
    borderYSize += GetSystemMetrics(SM_CYCAPTION);

    //-----------------------------------------

    hwndObjectProperties = CreateWindow(TEXT("ObjectProperties"), TEXT("Hi! - Object Properties"), 
                                        WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_DLGFRAME|WS_MINIMIZEBOX,
                                        CW_USEDEFAULT, CW_USEDEFAULT,
                                        borderXSize, borderYSize,
                                        hwndEditor, NULL, hinstMain, NULL);

    hwndOuterContainer = CreateWindow(TEXT("XR3DObjectPropertiesControlContainer"), NULL, WS_CHILD|WS_VISIBLE|WS_VSCROLL, 0, 25, OP_WIDTH, CONTAINER_HEIGHT, hwndObjectProperties, NULL, hinstMain, NULL);
    hwndContainer = CreateWindow(TEXT("XR3DObjectPropertiesControlContainer"), NULL, WS_CHILD|WS_VISIBLE, 0, 0, 0, 0, hwndOuterContainer, NULL, hinstMain, NULL);

    hwndTabControl = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD|WS_CLIPSIBLINGS,
        0, 0, OP_WIDTH, 20,
        hwndObjectProperties, (HMENU)IDC_TABCONTROLTHING, hinstMain, NULL);
    SendMessage(hwndTabControl, WM_SETFONT, (WPARAM)hWindowFont, 0);

    TCITEM ibching;

    ibching.mask = TCIF_TEXT;
    ibching.pszText = TEXT("Properties");
    TabCtrl_InsertItem(hwndTabControl, 0, &ibching);
    ibching.pszText = TEXT("Events");
    TabCtrl_InsertItem(hwndTabControl, 1, &ibching);

    ShowWindow(hwndTabControl, SW_SHOW);

    //-----------------------------------------

    bScriptsTab = FALSE;

    UpdateProperties();

    traceOut;
}

ObjectPropertiesEditor::~ObjectPropertiesEditor()
{
    traceIn(ObjectPropertiesEditor::~ObjectPropertiesEditor);

    Cleanup();

    DestroyWindow(hwndObjectProperties);

    objectProperties = NULL;

    traceOut;
}


void ObjectPropertiesEditor::UpdateProperties()
{
    traceIn(ObjectPropertiesEditor::UpdateProperties);

    Cleanup();

    BOOL bMultipleTypes = FALSE;
    Class *firstClass = NULL;
    for(int i=0; i<levelInfo->SelectedObjects.Num(); i++)
    {
        if(!firstClass)
            firstClass = levelInfo->SelectedObjects[i]->GetObjectClass();
        else if(levelInfo->SelectedObjects[i]->GetObjectClass() != firstClass)
        {
            bMultipleTypes = TRUE;
            break;
        }
    }

    if(bScriptsTab)
    {
        if(levelInfo->SelectedObjects.Num() == 0)
            SetWindowText(hwndObjectProperties, TEXT("[No objects selected] - Object Events"));
        else if(bMultipleTypes)
            SetWindowText(hwndObjectProperties, TEXT("[Multiple objects of different types] - Object Events"));
        else
        {
            bEditingMultipleObjects = levelInfo->SelectedObjects.Num() > 1;

            EditObjects();
        }
    }
    else
    {
        if(levelInfo->SelectedObjects.Num() == 0)
            SetWindowText(hwndObjectProperties, TEXT("[No objects selected] - Object Properties"));
        else if(bMultipleTypes)
            SetWindowText(hwndObjectProperties, TEXT("[Multiple objects of different types] - Object Properties"));
        else
        {
            bEditingMultipleObjects = levelInfo->SelectedObjects.Num() > 1;

            EditObjects();
        }
    }

    traceOut;
}


void ObjectPropertiesEditor::EditObjects()
{
    traceIn(ObjectPropertiesEditor::EditObjects);

    Entity *obj = levelInfo->SelectedObjects[0];

    assert(obj);

    if(!obj)
        return;

    int i, j;
    int curPos = 5;
    HWND hwndTemp;

    if(bScriptsTab)
    {
        String title;
        if(bEditingMultipleObjects)
            title << TEXT("[Multiple Objects] - Object Events");
        else
            title << obj->GetName() << TEXT(" - Object Events");

        SetWindowText(hwndObjectProperties, title);

        Class *curClass = obj->GetObjectClass();
        List<Class*> Heirarchy;
        do
        {
            ClassDefinition *classDef = curClass->scriptClass;
            if(!classDef)
                continue;

            BOOL bHasImplementableFunctions = FALSE;
            for(i=0; i<classDef->Functions.Num(); i++)
            {
                if(classDef->Functions[i].flags & FUNC_IMPLEMENTABLE)
                {
                    bHasImplementableFunctions = TRUE;
                    break;
                }
            }

            if(bHasImplementableFunctions)
                Heirarchy.Insert(0, curClass);
        }while(curClass = curClass->Parent);

        for(i=0; i<Heirarchy.Num(); i++)
        {
            curClass = Heirarchy[i];
            ClassDefinition *classDef = curClass->scriptClass;

            String classStr;
            classStr << TEXT("  ") << curClass->name;
            hwndTemp = CreateWindowEx(WS_EX_STATICEDGE, TEXT("Static"), classStr, SS_LEFT|WS_CHILD|WS_VISIBLE, 5, curPos, 390, 18, hwndContainer, NULL, NULL, NULL);
            SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
            Controls << hwndTemp;

            curPos += 24;

            for(j=0; j<classDef->Functions.Num(); j++)
            {
                FunctionDefinition &funcDef = classDef->Functions[j];

                if(!(funcDef.flags & FUNC_IMPLEMENTABLE))
                    continue;

                int curID = ScriptFunctions.Add(funcDef.funcOffset);

                String displayName;
                displayName << funcDef.name << ":   ";

                hwndTemp = CreateWindowEx(0, TEXT("Static"), displayName, SS_RIGHT|WS_CHILD|WS_VISIBLE, 10, curPos+1, 135, 18, hwndContainer, NULL, NULL, NULL);
                SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
                Controls << hwndTemp;

                HWND hwndTemp = CreateWindowEx(0, TEXT("Button"), TEXT("Edit..."), WS_CHILD|WS_VISIBLE, 192, curPos-2, 60, 22, hwndContainer, (HMENU)(UPARAM)(VAREDITSCRIPT_ID+curID), hinstMain, NULL);
                SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
                Controls << hwndTemp;

                hwndTemp = CreateWindowEx(0, TEXT("Button"), TEXT("Clear"), WS_CHILD|WS_VISIBLE, 256, curPos-2, 60, 22, hwndContainer, (HMENU)(UPARAM)(VARCLEARSCRIPT_ID+curID), hinstMain, NULL);
                SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
                Controls << hwndTemp;

                if(!bEditingMultipleObjects)
                {
                    String strStatus;

                    if(!obj->objScript)
                        strStatus = TEXT("None");
                    else
                    {
                        int scriptID = obj->objScript->FunctionIDs.FindValueIndex(funcDef.funcOffset);
                        if(scriptID == INVALID)
                            strStatus = TEXT("None");
                        else
                        {
                            String &script = obj->objScript->ScriptList[scriptID];

                            if(script.IsEmpty())
                                strStatus = TEXT("None");
                            else
                                strStatus = TEXT("Valid");
                        }
                    }

                    hwndTemp = CreateWindowEx(0, TEXT("Static"), strStatus, WS_CHILD|WS_VISIBLE, 150, curPos+1, 38, 18, hwndContainer, (HMENU)(UPARAM)(VARSCRIPTSTATUS_ID+curID), hinstMain, NULL);
                    SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
                    Controls << hwndTemp;
                }

                curPos += 24;
            }

            curPos += 10;
        }
    }
    else
    {
        String title;
        if(bEditingMultipleObjects)
            title << TEXT("[Multiple Objects] - Object Properties");
        else
            title << obj->GetName() << TEXT(" - Object Properties");

        SetWindowText(hwndObjectProperties, title);

        ClassDefinition *classDef = obj->GetObjectClass()->GetScriptClass();
        if(!classDef) return;

        int numVars = classDef->NumVariables();
        if(!numVars) return;

        int curVarID = 0;

        for(i=0; i<numVars; i++)
        {
            PropertyVariable *curVar = classDef->GetVariable(i);

            if(curVar->flags & VAR_PROPERTY)
            {
                String sectionName;
                if(curVar->section.IsEmpty())
                    sectionName = classDef->classData->name;
                else
                    sectionName = curVar->section;

                //--------------------

                UINT id;
                BOOL bFoundValue = FALSE;
                for(id=0; id<SectionList.Num(); id++)
                {
                    int compare = scmpi(SectionList[id].name, sectionName);
                    if(compare == 0)
                    {
                        bFoundValue = TRUE;
                        break;
                    }
                    else if(compare > 0)
                        break;
                }

                if(!bFoundValue)
                    SectionList.InsertNew(id)->name = sectionName;

                //--------------------

                SectionList[id].Vars.Add(curVar);
            }
        }

        curVarID = 0;

        for(i=0; i<SectionList.Num(); i++)
        {
            SectionInfo &section = SectionList[i];

            String classStr;
            classStr << TEXT("  ") << section.name;
            hwndTemp = CreateWindowEx(WS_EX_STATICEDGE, TEXT("Static"), classStr, SS_LEFT|WS_CHILD|WS_VISIBLE, 5, curPos, 390, 18, hwndContainer, NULL, NULL, NULL);
            SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
            Controls << hwndTemp;

            curPos += 24;

            for(j=0; j<section.Vars.Num(); j++)
            {
                PropertyVariable *curVar = section.Vars[j];
                String displayName;
                displayName << curVar->name << ":   ";

                hwndTemp = CreateWindowEx(0, TEXT("Static"), displayName, SS_RIGHT|WS_CHILD|WS_VISIBLE, 10, curPos+1, 135, 18, hwndContainer, NULL, NULL, NULL);
                SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
                Controls << hwndTemp;

                if(curVar->typeInfo.type == DataType_Integer)
                {
                    if(scmp(curVar->typeInfo.name, TEXT("int")) == 0)
                    {
                        hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL, ES_NUMBER|WS_CHILD|WS_VISIBLE, 150, curPos-1, 70, 20, hwndContainer, (HMENU)(UPARAM)(VAR_ID+curVarID), NULL, NULL);
                        int &iValue = *(int*)GetVarAddress(obj, curVar);

                        if(curVar->propertyType == PROPERTY_SCROLLER)
                        {
                            HWND hwndScroller = CreateWindowEx(0, TEXT("UpDownControl"), NULL, WS_CHILD|WS_VISIBLE, 222, curPos-1, 16, 20, hwndContainer, (HMENU)(UPARAM)(VARSCROLLER_ID+curVarID), hinstMain, NULL);
                            LinkUpDown(hwndScroller, hwndTemp);
                            InitUpDownIntData(hwndScroller, iValue, curVar->iMin, curVar->iMax, curVar->iInc);
                            Controls << hwndScroller;
                        }
                    }
                    else if(scmp(curVar->typeInfo.name, TEXT("bool")) == 0)
                    {
                        hwndTemp = CreateWindowEx(0, TEXT("Button"), NULL, BS_3STATE|WS_CHILD|WS_VISIBLE, 150, curPos, 150, 18, hwndContainer, (HMENU)(UPARAM)(VAR_ID+curVarID), NULL, NULL);
                        if(bEditingMultipleObjects)
                            SendMessage(hwndTemp, BM_SETCHECK, BST_INDETERMINATE, 0);
                        else
                        {
                            BOOL &bValue = *(BOOL*)GetVarAddress(obj, curVar);
                            SendMessage(hwndTemp, BM_SETCHECK, bValue ? BST_CHECKED : BST_UNCHECKED, 0);
                        }
                    }
                    else if(scmp(curVar->typeInfo.name, TEXT("icolor")) == 0)
                    {
                        hwndTemp = CreateWindowEx(0, TEXT("ColorControl"), NULL, WS_CHILD|WS_VISIBLE, 150, curPos-2, 40, 22, hwndContainer, (HMENU)(UPARAM)(VAR_ID+curVarID), NULL, NULL);
                        if(!bEditingMultipleObjects)
                        {
                            DWORD &cValue = *(DWORD*)GetVarAddress(obj, curVar);
                            CCSetColor(hwndTemp, cValue);
                        }
                        else
                            CCSetColor(hwndTemp, 0xFFFFFFFF);
                    }
                    else //enums
                    {
                        EnumDefinition *enumDef = Scripting->GetEnumDef(curVar->typeInfo.name);
                        int &iValue = *(int*)GetVarAddress(obj, curVar);

                        hwndTemp = CreateWindowEx(0, TEXT("ComboBox"), NULL, WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST, 150, curPos-1, 150, 80, hwndContainer, (HMENU)(UPARAM)(VARENUM_ID+curVarID), NULL, NULL);

                        for(int k=0; k<enumDef->Items.Num(); k++)
                        {
                            SendMessage(hwndTemp, CB_ADDSTRING, 0, (LPARAM)enumDef->Items[k].name.Array());
                            SendMessage(hwndTemp, CB_SETITEMDATA, k, (LPARAM)enumDef->Items[k].val);

                            if(iValue == enumDef->Items[k].val)
                                SendMessage(hwndTemp, CB_SETCURSEL, k, 0);
                        }
                    }
                }
                else if(curVar->typeInfo.type == DataType_Float)
                {
                    hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), NULL, WS_CHILD|WS_VISIBLE, 150, curPos-1, 70, 20, hwndContainer, (HMENU)(UPARAM)(VAR_ID+curVarID), NULL, NULL);
                    float &fValue = *(float*)GetVarAddress(obj, curVar);

                    if(curVar->propertyType == PROPERTY_SCROLLER)
                    {
                        HWND hwndScroller = CreateWindowEx(0, TEXT("UpDownControl"), NULL, WS_CHILD|WS_VISIBLE, 222, curPos-1, 16, 20, hwndContainer, (HMENU)(UPARAM)(VARSCROLLER_ID+curVarID), hinstMain, NULL);
                        LinkUpDown(hwndScroller, hwndTemp);
                        InitUpDownFloatData(hwndScroller, fValue, curVar->fMin, curVar->fMax, curVar->fInc);
                        Controls << hwndScroller;
                    }
                }
                else if(curVar->typeInfo.type == DataType_String)
                {
                    DWORD flags = WS_CHILD|WS_VISIBLE;

                    if(curVar->propertyType != 0)
                        flags |= ES_READONLY;

                    hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("PropertyEdit"), NULL, flags, 150, curPos-1, 150, 20, hwndContainer, (HMENU)(UPARAM)(VAR_ID+curVarID), NULL, NULL);
                    if(!bEditingMultipleObjects)
                    {
                        String &strVal = *(String*)GetVarAddress(obj, curVar);
                        SetWindowText(hwndTemp, strVal);
                    }

                    if(curVar->propertyType != 0)
                    {
                        HWND hwndBrowse = CreateWindowEx(0, TEXT("Button"), TEXT("Browse..."), WS_CHILD|WS_VISIBLE, 304, curPos-2, 60, 22, hwndContainer, (HMENU)(UPARAM)(VARBROWSE_ID+curVarID), hinstMain, NULL);
                        SendMessage(hwndBrowse, WM_SETFONT, (WPARAM)hWindowFont, 0);
                        Controls << hwndBrowse;
                    }
                }
                else if(curVar->typeInfo.type == DataType_Object)
                {
                    BOOL bSound         = (scmp(curVar->typeInfo.name, TEXT("Sound")) == 0);
                    BOOL bTexture       = (scmp(curVar->typeInfo.name, TEXT("Texture")) == 0);
                    BOOL bCubeTexture   = (scmp(curVar->typeInfo.name, TEXT("CubeTexture")) == 0);
                    BOOL bMaterial      = (scmp(curVar->typeInfo.name, TEXT("Material")) == 0);

                    if(bSound || bTexture || bCubeTexture || bMaterial)
                    {
                        hwndTemp = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("PropertyEdit"), NULL, WS_CHILD|WS_VISIBLE|ES_READONLY, 150, curPos-1, 150, 20, hwndContainer, (HMENU)(UPARAM)(VAR_ID+curVarID), NULL, NULL);
                        if(!bEditingMultipleObjects)
                        {
                            String strVal = GetPropertyStringVal(obj, curVar);

                            SetWindowText(hwndTemp, strVal);
                        }

                        HWND hwndBrowse = CreateWindowEx(0, TEXT("Button"), TEXT("Browse..."), WS_CHILD|WS_VISIBLE, 304, curPos-2, 60, 22, hwndContainer, (HMENU)(UPARAM)(VARBROWSE_ID+curVarID), hinstMain, NULL);
                        SendMessage(hwndBrowse, WM_SETFONT, (WPARAM)hWindowFont, 0);
                        Controls << hwndBrowse;
                    }
                }

                SendMessage(hwndTemp, WM_SETFONT, (WPARAM)hWindowFont, 0);
                Controls << hwndTemp;

                curPos += 24;

                ++curVarID;
            }

            curPos += 10;
        }

        SetWindowPos(hwndContainer, NULL, 0, 0, OP_WIDTH, curPos, SWP_NOZORDER);

        for(i=0; i<SectionList.Num(); i++)
        {
            VarList.AppendList(SectionList[i].Vars);
            SectionList[i].Vars.Clear();
        }

        ModifiedVars.SetSize(VarList.Num());
        if(!bEditingMultipleObjects)
            ModifiedVars.SetAll();
    }

    curPos -= 10;

    SCROLLINFO si;
    zero(&si, sizeof(si));

    si.cbSize = sizeof(si);
    si.fMask  = SIF_ALL;
    si.nMin   = 0;
    si.nMax   = curPos;
    si.nPage  = CONTAINER_HEIGHT;

    SetScrollInfo(hwndOuterContainer, SB_VERT, &si, TRUE);
    ShowScrollBar(hwndOuterContainer, SB_VERT, (curPos > OP_HEIGHT));

    traceOut;
}

void ObjectPropertiesEditor::UpdateVar(int paramID, int notification)
{
    traceIn(ObjectPropertiesEditor::UpdateVar);

    if(!SectionList.Num() || (paramID >= VarList.Num()))
        return;

    HWND hwndControl = GetDlgItem(hwndContainer, VAR_ID+paramID);

    Object *obj = levelInfo->SelectedObjects[0];

    BOOL bSomethingChanged = FALSE;

    PropertyVariable *param = VarList[paramID];

    if(scmp(param->typeInfo.name, TEXT("bool")) == 0)
        SendMessage(hwndControl, BM_SETCHECK, (SendMessage(hwndControl, BM_GETCHECK, 0, 0) != BST_CHECKED) ? BST_CHECKED : BST_UNCHECKED, 0);

    for(int i=levelInfo->SelectedObjects.Num()-1; i>=0; i--)
    {
        if(scmp(param->typeInfo.name, TEXT("bool")) == 0)
        {
            if(notification == BN_CLICKED)
            {
                SaveUndoData();

                int checkType = SendMessage(hwndControl, BM_GETCHECK, 0, 0);

                BOOL bNewVal = (checkType == BST_CHECKED);

                BOOL &bValue = *(BOOL*)GetVarAddress(obj, param);
                if( (!ModifiedVars[paramID] && (checkType != BST_INDETERMINATE)) ||
                    (ModifiedVars[paramID]  && (bNewVal != bValue))              )
                {
                    bSomethingChanged = TRUE;
                    BOOL &bObjValue = *(BOOL*)GetVarAddress(levelInfo->SelectedObjects[i], param);
                    bObjValue = bNewVal;
                    levelInfo->SelectedObjects[i]->Reinitialize();
                }

                UpdateViewports();
            }
        }
        else if(scmp(param->typeInfo.name, TEXT("icolor")) == 0)
        {
            if(notification == CCN_CHANGED)
            {
                DWORD &cValue = *(DWORD*)GetVarAddress(obj, param);
                DWORD cNewValue = CCGetColor(hwndControl);

                if( (!ModifiedVars[paramID] && (cNewValue != 0xFFFFFFFF)) ||
                    (ModifiedVars[paramID]  && (cNewValue != cValue))     )
                {
                    bSomethingChanged = TRUE;
                    DWORD &dwObjValue = *(DWORD*)GetVarAddress(levelInfo->SelectedObjects[i], param);
                    dwObjValue = cNewValue;
                    levelInfo->SelectedObjects[i]->Reinitialize();
                }

                UpdateViewports();
            }
        }
        else if(param->typeInfo.type == DataType_String)
        {
            if((notification == EN_ENTERHIT) || (notification == EN_KILLFOCUS))
            {
                String &strValue = *(String*)GetVarAddress(obj, param);

                String strNewValue;
                strNewValue.SetLength(SendMessage(hwndControl, WM_GETTEXTLENGTH, 0, 0));
                GetWindowText(hwndControl, strNewValue, strNewValue.Length()+1);

                if( (!ModifiedVars[paramID] && !strNewValue.IsEmpty())         ||
                    (ModifiedVars[paramID]  && !strNewValue.Compare(strValue)) )
                {
                    bSomethingChanged = TRUE;
                    String &strObjValue = *(String*)GetVarAddress(levelInfo->SelectedObjects[i], param);
                    strObjValue = strNewValue;
                    levelInfo->SelectedObjects[i]->Reinitialize();
                }

                UpdateViewports();
            }
        }
    }

    if(bSomethingChanged)
    {
        SaveUndoData();
        ModifiedVars.Set(paramID);
    }

    traceOut;
}

void ObjectPropertiesEditor::Browse(int paramID)
{
    traceIn(ObjectPropertiesEditor::Browse);

    if(!VarList.Num() || (paramID >= VarList.Num()))
        return;

    HWND hwndEdit = GetDlgItem(hwndContainer, VAR_ID+paramID);
    Object *obj = levelInfo->SelectedObjects[0];

    TSTR lpType;
    TSTR lpFilter;
    String strFullDir;

    strFullDir << TEXT(".\\data\\") << editor->curWorkingModule << TEXT("\\textures");

    PropertyVariable *param = VarList[paramID];

    switch(param->propertyType)
    {
        case PROPERTY_TEXTURE:
            lpType = TEXT("textures");
            lpFilter = TEXT("All Formats\0*.bmp;*.dds;*.jpg;*.png;*.tga\0");
            break;

        case PROPERTY_SOUND:
            lpType = TEXT("sounds");
            lpFilter = TEXT("Sound Files\0*.wav\0");
            break;

        case PROPERTY_MUSIC:
            lpType = TEXT("music");
            lpFilter = TEXT("OGG Files\0*.ogg\0");
            break;
    }

    TCHAR lpFile[512];
    OPENFILENAME ofn;
    zero(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndObjectProperties;
    ofn.lpstrFile = lpFile;
    ofn.nMaxFile = 511;

    ofn.lpstrFile[0] = 0;
    ofn.lpstrFilter = lpFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = OSFileExists(strFullDir) ? strFullDir.Array() : TEXT(".\\data\\");
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    TCHAR curDirectory[512];
    GetCurrentDirectory(511, curDirectory);

    if(GetOpenFileName(&ofn))
    {
        SetCurrentDirectory(curDirectory);

        String strFile = lpFile;
        strFile.FindReplace(TEXT("\\"), TEXT("/"));

        String strResource;
        if(GetResourceStringFromPath(strFile, lpType, strResource))
        {
            BOOL bSomethingChanged = FALSE;

            for(int i=levelInfo->SelectedObjects.Num()-1; i>=0; i--)
            {
                Entity *curObj = levelInfo->SelectedObjects[i];
                String strValue = GetPropertyStringVal(curObj, param);
                strValue.FindReplace(TEXT("\\"), TEXT("/"));

                if(!ModifiedVars[paramID] || !strValue.CompareI(strResource))
                {
                    SetWindowText(hwndEdit, strResource);
                    bSomethingChanged = TRUE;
                    SetPropertyStringVal(curObj, param, strResource);
                    curObj->Reinitialize();
                }
            }

            if(bSomethingChanged)
            {
                ModifiedVars.Set(paramID);
                SaveUndoData();
            }
        }
        else
        {
            if(param->propertyType == PROPERTY_TEXTURE)
                MessageBox(hwndObjectProperties, TEXT("The chosen texture must reside within a module's \"textures\" directory or subdirectories."), NULL, 0);
            else if(param->propertyType == PROPERTY_SOUND)
                MessageBox(hwndObjectProperties, TEXT("The chosen sound must reside within a module's \"sounds\" directory or subdirectories."), NULL, 0);
            else if(param->propertyType == PROPERTY_MUSIC)
                MessageBox(hwndObjectProperties, TEXT("The chosen ogg file must reside within a module's \"music\" directory or subdirectories."), NULL, 0);
        }
    }
    else
        SetCurrentDirectory(curDirectory);

    UpdateViewports();

    traceOut;
}

void ObjectPropertiesEditor::AdjustScroller(int paramID, int notification)
{
    traceIn(ObjectPropertiesEditor::AdjustScroller);

    if(!VarList.Num() || (paramID >= VarList.Num()))
        return;

    HWND hwndControl = GetDlgItem(hwndContainer, VARSCROLLER_ID+paramID);

    BOOL bSomethingChanged = FALSE;

    PropertyVariable *param = VarList[paramID];

    for(int i=0; i<levelInfo->SelectedObjects.Num(); i++)
    {
        Object *obj = levelInfo->SelectedObjects[i];
        switch(param->typeInfo.type)
        {
            case DataType_Integer:
                {
                    int &iValue = *(int*)GetVarAddress(obj, param);

                    int iOldVal = GetPrevUpDownInt(hwndControl);

                    iValue = GetUpDownInt(hwndControl);

                    if((iOldVal != iValue) && (notification == JUDN_UPDATE))
                        bSomethingChanged = TRUE;

                    obj->Reinitialize();
                    UpdateViewports();
                    break;
                }

            case DataType_Float:
                {
                    float &fValue = *(float*)GetVarAddress(obj, param);

                    float fOldVal = GetPrevUpDownFloat(hwndControl);

                    if((fOldVal != fValue) && (notification == JUDN_UPDATE))
                        bSomethingChanged = TRUE;

                    fValue = GetUpDownFloat(hwndControl);
                    obj->Reinitialize();
                    UpdateViewports();
                    break;
                }
        }
    }

    if(bSomethingChanged)
    {
        ModifiedVars.Set(paramID);
        SaveUndoData();
    }

    traceOut;
}

void ObjectPropertiesEditor::ChangeEnum(int varID)
{
    traceIn(ObjectPropertiesEditor::ChangeEnum);

    if(!VarList.Num() || (varID >= VarList.Num()))
        return;

    HWND hwndControl = GetDlgItem(hwndContainer, VARENUM_ID+varID);

    int selID = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
    int newVal = SendMessage(hwndControl, CB_GETITEMDATA, selID, 0);

    BOOL bSomethingChanged = FALSE;

    PropertyVariable *param = VarList[varID];

    for(int i=0; i<levelInfo->SelectedObjects.Num(); i++)
    {
        Object *obj = levelInfo->SelectedObjects[i];

        int& val = *(int*)GetVarAddress(obj, param);

        if(newVal != val)
        {
            bSomethingChanged = TRUE;
            val = newVal;
        }
    }

    if(bSomethingChanged)
    {
        ModifiedVars.Set(varID);
        SaveUndoData();
    }

    traceOut;
}


void ObjectPropertiesEditor::EditScript(int paramID)
{
    traceIn(ObjectPropertiesEditor::EditScript);

    if(!ScriptFunctions.Num() || (paramID >= ScriptFunctions.Num()))
        return;

    if(bEditingMultipleObjects)
    {
        if(MessageBox(hwndObjectProperties, TEXT("You have multiple objects selected.  Are you sure you wish to open script windows for all of them?"), TEXT("Are you positive?"), MB_YESNO) == IDNO)
            return;
    }

    for(int i=0; i<levelInfo->SelectedObjects.Num(); i++)
    {
        Entity *ent = levelInfo->SelectedObjects[i];
        DWORD functionID = ScriptFunctions[paramID];

        BOOL bScriptAlreadyOpen = FALSE;

        for(int j=0; j<ScriptEditor::OpenScripts.Num(); j++)
        {
            ScriptEditor *scriptEditor = ScriptEditor::OpenScripts[j];
            if((scriptEditor->curEntity == ent) && (scriptEditor->functionID == functionID))
            {
                bScriptAlreadyOpen = TRUE;

                if(!bEditingMultipleObjects)
                {
                    ShowWindow(scriptEditor->hwndScriptEditor, SW_RESTORE);
                    SetWindowPos(scriptEditor->hwndScriptEditor, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
                }

                break;
            }
        }

        if(bScriptAlreadyOpen)
            continue;

        ScriptEditor *scriptWindow = new ScriptEditor;
        scriptWindow->curEntity = ent;
        scriptWindow->functionID = functionID;

        scriptWindow->InitData();
    }

    traceOut;
}

void ObjectPropertiesEditor::ClearScript(int paramID)
{
    traceIn(ObjectPropertiesEditor::ClearScript);

    if(!ScriptFunctions.Num() || (paramID >= ScriptFunctions.Num()))
        return;

    List<ScriptEditor*> AssociatedScriptEditors;

    BOOL bAnyDataToClear = FALSE;

    for(int i=0; i<levelInfo->SelectedObjects.Num(); i++)
    {
        Entity *ent = levelInfo->SelectedObjects[i];
        DWORD functionID = ScriptFunctions[paramID];

        if(!ent->objScript)
            continue;

        if(ent->objScript->FunctionIDs.HasValue(functionID))
            bAnyDataToClear = TRUE;

        for(int j=0; j<ScriptEditor::OpenScripts.Num(); j++)
        {
            ScriptEditor *scriptEditor = ScriptEditor::OpenScripts[j];
            if((scriptEditor->curEntity == ent) && (scriptEditor->functionID == functionID))
            {
                AssociatedScriptEditors << scriptEditor;
                break;
            }
        }
    }

    if(!bAnyDataToClear)
        return;

    if(MessageBox(hwndObjectProperties, TEXT("All script data assigned to this event for the selected objects will be cleared and any associated script windows will be closed.  Are you sure you want to do this?"), TEXT("Are you positive?"), MB_YESNO) == IDNO)
        return;

    for(int i=0; i<levelInfo->SelectedObjects.Num(); i++)
    {
        Entity *ent = levelInfo->SelectedObjects[i];
        DWORD functionID = ScriptFunctions[paramID];

        DWORD id = ent->objScript->FunctionIDs.FindValueIndex(functionID);
        if(id != INVALID)
        {
            ent->objScript->FunctionIDs.Remove(id);
            ent->objScript->ScriptList.Remove(id);
            if(ent->objScript->Functions.Num() > id)
            {
                ent->objScript->Functions[id].FreeData();
                ent->objScript->Functions.Remove(id);
            }
        }
    }

    for(int i=0; i<AssociatedScriptEditors.Num(); i++)
        AssociatedScriptEditors[i]->CloseScript(FALSE);

    SetWindowText(GetDlgItem(hwndContainer, VARSCRIPTSTATUS_ID+paramID), TEXT("None"));

    traceOut;
}


void ObjectPropertiesEditor::Cleanup()
{
    traceIn(ObjectPropertiesEditor::Cleanup);

    VarList.Clear();
    ModifiedVars.Clear();
    ScriptFunctions.Clear();

    for(int i=0; i<SectionList.Num(); i++)
        SectionList[i].name.Clear();
    SectionList.Clear();

    for(int i=0; i<Controls.Num(); i++)
        DestroyWindow(Controls[i]);
    Controls.Clear();

    SCROLLINFO si;
    zero(&si, sizeof(si));

    si.cbSize = sizeof(si);
    si.fMask  = SIF_ALL;
    si.nMin   = 0;
    si.nMax   = 0;
    si.nPage  = CONTAINER_HEIGHT;

    SetScrollInfo(hwndOuterContainer, SB_VERT, &si, TRUE);
    ShowScrollBar(hwndOuterContainer, SB_VERT, FALSE);

    traceOut;
}


void ObjectPropertiesEditor::SaveUndoData()
{
    traceIn(ObjectPropertiesEditor::SaveUndoData);

    Action action;
    action.actionProc = EditorLevelInfo::UndoRedoChangeProperties;
    action.strName = TEXT("Change Properties");

    BufferOutputSerializer s(action.data);

    DWORD numObjects = levelInfo->SelectedObjects.Num();
    s << numObjects;

    for(int i=0; i<numObjects; i++)
    {
        s << levelInfo->SelectedObjects[i]->name;
        levelInfo->SelectedObjects[i]->Serialize(s);
    }

    editor->undoStack->Push(action);

    traceOut;
}


String GetPropertyStringVal(Object *obj, PropertyVariable *curVar)
{
    if(curVar->typeInfo.type == DataType_String)
        return *(String*)GetVarAddress(obj, curVar);
    else if(curVar->typeInfo.type == DataType_Object)
    {
        BOOL bSound         = (scmp(curVar->typeInfo.name, TEXT("Sound")) == 0);
        BOOL bTexture       = (scmp(curVar->typeInfo.name, TEXT("Texture")) == 0);
        BOOL bCubeTexture   = (scmp(curVar->typeInfo.name, TEXT("CubeTexture")) == 0);
        BOOL bMaterial      = (scmp(curVar->typeInfo.name, TEXT("Material")) == 0);

        if(bSound || bTexture || bCubeTexture || bMaterial)
        {
            String strVal;
            if(bSound)
            {
                Sound *sound = *(Sound**)GetVarAddress(obj, curVar);
                if(sound)
                    return sound->GetResourceName();
            }
            else if(bTexture || bCubeTexture)
                return RM->GetTextureName(*(BaseTexture**)GetVarAddress(obj, curVar));
            else if(bMaterial)
                return RM->GetMaterialName(*(Material**)GetVarAddress(obj, curVar));
        }
    }

    return String();
}

void SetPropertyStringVal(Object *obj, PropertyVariable *curVar, CTSTR lpString)
{
    if(curVar->typeInfo.type == DataType_String)
        *(String*)GetVarAddress(obj, curVar) = lpString;
    else if(curVar->typeInfo.type == DataType_Object)
    {
        BOOL bSound         = (scmp(curVar->typeInfo.name, TEXT("Sound")) == 0);
        BOOL bTexture       = (scmp(curVar->typeInfo.name, TEXT("Texture")) == 0);
        BOOL bCubeTexture   = (scmp(curVar->typeInfo.name, TEXT("CubeTexture")) == 0);
        BOOL bMaterial      = (scmp(curVar->typeInfo.name, TEXT("Material")) == 0);

        if(bSound || bTexture || bCubeTexture || bMaterial)
        {
            String strVal;
            if(bSound)
            {
                Sound **sound = (Sound**)GetVarAddress(obj, curVar);
                if(sound)
                    DestroyObject(*sound);

                *sound = NewSound(lpString, (curVar->propertyType != PROPERTY_SOUND2D));
            }
            else if(bTexture)
            {
                Texture **tex = (Texture**)GetVarAddress(obj, curVar);
                if(tex)
                    ReleaseTexture(*tex);

                if(curVar->propertyType == PROPERTY_TEXTURE2D)
                    *tex = GetTexture(lpString, FALSE);
                else
                    *tex = GetTexture(lpString);
            }
            else if(bCubeTexture)
            {
                CubeTexture **tex = (CubeTexture**)GetVarAddress(obj, curVar);
                if(tex)
                    ReleaseTexture(*tex);

                *tex = GetCubeTexture(lpString);
            }
            else if(bMaterial)
            {
                Material **mat = (Material**)GetVarAddress(obj, curVar);
                if(mat)
                    ReleaseMaterial(*mat);

                *mat = GetMaterial(lpString);
            }
        }
    }
}


LRESULT WINAPI ObjectPropertiesWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_NOTIFY:
            {
                NMHDR *pNotify = (NMHDR*)lParam;

                if(pNotify->idFrom == IDC_TABCONTROLTHING)
                {
                    if(pNotify->code == TCN_SELCHANGE)
                    {
                        int curSel = TabCtrl_GetCurSel(pNotify->hwndFrom);

                        objectProperties->bScriptsTab = curSel;
                        objectProperties->UpdateProperties();
                    }
                }
                break;
            };

        case WM_CLOSE:
            delete objectProperties;
            break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT WINAPI ObjectPropertiesContainerWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_COMMAND:
            {
                int controlID = LOWORD(wParam);
                int notifyID  = HIWORD(wParam);

                if(controlID < VARMAX_ID)
                {
                    if(controlID >= VARENUM_ID)
                    {
                        if(notifyID == CBN_SELCHANGE)
                            objectProperties->ChangeEnum(controlID-VARENUM_ID);
                        break;
                    }
                    else if(controlID >= VARCLEARSCRIPT_ID)
                    {
                        objectProperties->ClearScript(controlID-VARCLEARSCRIPT_ID);
                        break;
                    }
                    else if(controlID >= VAREDITSCRIPT_ID)
                    {
                        objectProperties->EditScript(controlID-VAREDITSCRIPT_ID);
                        break;
                    }
                    else if(controlID >= VARSCROLLER_ID)
                    {
                        objectProperties->AdjustScroller(controlID-VARSCROLLER_ID, HIWORD(wParam));
                        break;
                    }
                    else if(controlID >= VARBROWSE_ID)
                    {
                        objectProperties->Browse(controlID-VARBROWSE_ID);
                        break;
                    }
                    else if(controlID >= VAR_ID)
                    {
                        objectProperties->UpdateVar(controlID-VAR_ID, HIWORD(wParam));
                        break;
                    }
                }

                break;
            }

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

                    //SetWindowPos(hwnd, 

                    si.fMask = SIF_POS;
                    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
                }
                break;
            }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void ENGINEAPI EditorLevelInfo::UndoRedoChangeProperties(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoChangeProperties);

    DWORD numObjects;

    s    << numObjects;
    sOut << numObjects;

    for(int i=0; i<numObjects; i++)
    {
        String strName;
        s    << strName;
        sOut << strName;

        Entity *ent = Entity::FindByName(strName);

        ent->Serialize(sOut);
        ent->Serialize(s);
    }

    UpdateViewports();

    if(objectProperties)
        objectProperties->UpdateProperties();

    traceOut;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  LevelData.cpp

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


BOOL CALLBACK CreateOutdoorDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK CreateOctDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SaveLevelDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

struct CreateOutdoorData
{
    int size;
    float scale;
    BOOL bHasWater;
};

CreateOutdoorData outdoorCreationData;

void EditorEngine::NewIndoorLevel(BOOL bDisplaySaveMessage)
{
    traceIn(EditorEngine::NewIndoorLevel);

    BOOL bCancel = FALSE;
    if(bDisplaySaveMessage && levelInfo && levelInfo->bModified)
    {
        switch(MessageBox(hwndEditor, TEXT("You have unsaved changes.\r\n\r\nWould you like to save your changes?"), TEXT("Save?"), MB_YESNOCANCEL))
        {
            case IDYES:
                SendMessage(hwndEditor, WM_COMMAND, MAKEWPARAM(ID_FILE_SAVELEVEL, 0), NULL);
                break;

            case IDCANCEL:
                return;
        }
    }

    KillViewports();
    currentLayout = (Layout)-1;

    if(level)
    {
        DestroyObject(levelInfo);
        DestroyObject(level);
    }
    level = CreateObject(IndoorLevel);
    levelInfo = CreateObject(EditorLevelInfo);
    level->bLoaded = TRUE;

    SendMessage(GetDlgItem(hwndSidebar, ID_GRID_SNAP), BM_SETCHECK, BST_CHECKED, 0);

    SetViewportLayout(Layout_FourPanel);

    String windowTitle;
    if(curWorkingModule.IsEmpty())
        windowTitle << TEXT("No Module Loaded");
    else
        windowTitle << curWorkingModule;

    windowTitle << TEXT(": Untitled Indoor Level - Xed");
    SetWindowText(hwndEditor, windowTitle);

    editor->CreateIndoorSidebar();

    levelInfo->SetCurModifyMode(ModifyMode_Move);

    traceOut;
}

void EditorEngine::NewOctLevel(BOOL bDisplaySaveMessage)
{
    traceIn(EditorEngine::NewOctLevel);

    BOOL bCancel = FALSE;
    if(bDisplaySaveMessage && levelInfo && levelInfo->bModified)
    {
        switch(MessageBox(hwndEditor, TEXT("You have unsaved changes.\r\n\r\nWould you like to save your changes?"), TEXT("Save?"), MB_YESNOCANCEL))
        {
            case IDYES:
                SendMessage(hwndEditor, WM_COMMAND, MAKEWPARAM(ID_FILE_SAVELEVEL, 0), NULL);
                break;

            case IDCANCEL:
                return;
        }
    }

    KillViewports();
    currentLayout = (Layout)-1;

    if(level)
    {
        DestroyObject(levelInfo);
        DestroyObject(level);
    }

    OctLevel *octLevel;
    level = octLevel = CreateObject(OctLevel);
    levelInfo = CreateObject(EditorLevelInfo);
    level->bLoaded = TRUE;

    octLevel->objectTree = new OctBVH;

    //---------------------------------------------------

    SendMessage(GetDlgItem(hwndSidebar, ID_GRID_SNAP), BM_SETCHECK, BST_CHECKED, 0);

    SetViewportLayout(Layout_SinglePanel);

    String windowTitle;
    if(curWorkingModule.IsEmpty())
        windowTitle << TEXT("No Module Loaded");
    else
        windowTitle << curWorkingModule;

    windowTitle << TEXT(": Untitled Open Level - Xed");
    SetWindowText(hwndEditor, windowTitle);

    editor->CreateOctLevelSidebar();

    //---------------------------------------------------

    levelInfo->SetCurModifyMode(ModifyMode_Move);

    traceOut;
}

void EditorEngine::NewOutdoorLevel(BOOL bDisplaySaveMessage)
{
    traceIn(EditorEngine::NewOutdoorLevel);

    BOOL bCancel = FALSE;
    if(bDisplaySaveMessage && levelInfo && levelInfo->bModified)
    {
        switch(MessageBox(hwndEditor, TEXT("You have unsaved changes.\r\n\r\nWould you like to save your changes?"), TEXT("Save?"), MB_YESNOCANCEL))
        {
            case IDYES:
                SendMessage(hwndEditor, WM_COMMAND, MAKEWPARAM(ID_FILE_SAVELEVEL, 0), NULL);
                break;

            case IDCANCEL:
                return;
        }
    }

    //---------------------------------------------------

    if(DialogBox(hinstMain, MAKEINTRESOURCE(IDD_CREATEOUTDOOR), hwndEditor, (DLGPROC)CreateOutdoorDialogProc) == IDCANCEL)
        return;

    int rowCount = 8*(1<<outdoorCreationData.size);
    float scale = outdoorCreationData.scale;

    //---------------------------------------------------

    KillViewports();
    currentLayout = (Layout)-1;

    if(level)
    {
        DestroyObject(levelInfo);
        DestroyObject(level);
    }

    OutdoorLevel *outdoorLevel;
    level = outdoorLevel = CreateObject(OutdoorLevel);
    levelInfo = CreateObject(EditorLevelInfo);
    level->bLoaded = TRUE;

    //---------------------------------------------------

    outdoorLevel->rowCount = rowCount;
    outdoorLevel->TerrainBlocks.SetSize(rowCount*rowCount);

    float blockScale = 16.0f*scale;
    float minVal = -(blockScale*float(rowCount)) * 0.5f;

    for(int y=0; y<rowCount; y++)
    {
        DWORD yOffset = (y*rowCount);
        float yPos = minVal + (blockScale*float(y));

        for(int x=0; x<rowCount; x++)
        {
            DWORD blockID = yOffset+x;
            float xPos = minVal + (blockScale*float(x));

            //---------------------------------

            TerrainBlock &block = outdoorLevel->TerrainBlocks[blockID];

            block.Pos.Set(xPos, 0.0f, yPos);

            block.bounds.Min.Set(0.0f, 0.0f, 0.0f);
            block.bounds.Max.Set(blockScale, 0.0f, blockScale);

            block.bounds.Min += block.Pos;
            block.bounds.Max += block.Pos;

            //---------------------------------

            VBData *vbd = new VBData;
            vbd->VertList.SetSize(17*17);
            vbd->NormalList.SetSize(17*17);
            vbd->TangentList.SetSize(17*17);

            for(int i=0; i<289; i++)
            {
                Vect &vert = vbd->VertList[i];

                float fOffsetX = (float(i%17)*scale)+xPos;
                float fOffsetZ = (float(i/17)*scale)+yPos;

                vbd->VertList[i].Set(fOffsetX, 0.0f, fOffsetZ);
                vbd->NormalList[i].Set(0.0f, 1.0f, 0.0f);
                vbd->TangentList[i].Set(1.0f, 0.0f, 0.0f);
            }

            block.VertBuffer = CreateVertexBuffer(vbd);

            block.curLOD = 0;
            block.LODFlags = 0;
        }
    }

    

    //---------------------------------------------------

    WeatherData *curWeather = outdoorLevel->WeatherList.CreateNew();

    curWeather->weatherName         = TEXT("Default");

    curWeather->SkyColorDawn        = RGB_to_Vect(0x758DA4);
    curWeather->SkyColorDay         = RGB_to_Vect(0x5F87CB);
    curWeather->SkyColorDusk        = RGB_to_Vect(0x385981);
    curWeather->SkyColorNight       = RGB_to_Vect(0x090A0B);

    curWeather->AmbientColorDawn    = RGB_to_Vect(0x2F4260);
    curWeather->AmbientColorDay     = RGB_to_Vect(0x898CA0);
    curWeather->AmbientColorDusk    = RGB_to_Vect(0x444B60);
    curWeather->AmbientColorNight   = RGB_to_Vect(0x20232A);

    curWeather->SunColorDawn        .Set(0.950f, 0.624f, 0.467f);
    curWeather->SunColorDay         .Set(1.000f, 0.989f, 0.834f);
    curWeather->SunColorDusk        .Set(1.000f, 0.448f, 0.310f);
    curWeather->MoonShineColor      .Set(0.232f, 0.381f, 0.691f);

    curWeather->bHasGlare           = TRUE;

    curWeather->FogColorDawn        = RGB_to_Vect(0xFFBD9D);
    curWeather->FogColorDay         = RGB_to_Vect(0xCEE3FF);
    curWeather->FogColorDusk        = RGB_to_Vect(0xFFBD9D);
    curWeather->FogColorNight       = RGB_to_Vect(0x090A0B);
    curWeather->fFogDepth           = 0.69f;
    curWeather->FogType             = FOG_LINEAR;

    curWeather->fTransitionSpeed    = 0.15f;

    curWeather->fWindSpeed          = 0.1f;

    curWeather->fCloudSpeed         = 1.25f;
    curWeather->CloudTexture        = GetTexture(TEXT("Base:Default/TX_Sky_Clear.tga"));

    curWeather->AmbientSound        = NULL;

    outdoorLevel->SetWeather(NULL, TRUE);

    outdoorLevel->fVisibilityDistance = 1400.0f;
    outdoorLevel->bLimitVisDistance = TRUE;

    //---------------------------------------------------

    SendMessage(GetDlgItem(hwndSidebar, ID_GRID_SNAP), BM_SETCHECK, BST_CHECKED, 0);

    SetViewportLayout(Layout_HorizontalPair);

    String windowTitle;
    if(curWorkingModule.IsEmpty())
        windowTitle << TEXT("No Module Loaded");
    else
        windowTitle << curWorkingModule;

    windowTitle << TEXT(": Untitled Outdoor Level - Xed");
    SetWindowText(hwndEditor, windowTitle);

    editor->CreateOutdoorSidebar();

    levelInfo->SetCurModifyMode(ModifyMode_Move);

    traceOut;
}

void EditorEngine::SaveLevelAs()
{
    traceIn(EditorEngine::SaveLevelAs);

    assert(level);
    if(!level)
        return;

    //DialogBox(hinstMain, MAKEINTRESOURCE(IDD_SAVELEVEL), hwndMain, (DLGPROC)SaveLevelDialogProc);

    TCHAR lpFile[512];
    OPENFILENAME ofn;
    zero(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndEditor;
    ofn.lpstrFile = lpFile;
    ofn.nMaxFile = 511;

    String startDir;
    startDir << TEXT(".\\data\\") << curWorkingModule << TEXT("\\levels\\");
    OSCreateDirectory(startDir);

    ofn.lpstrFile[0] = 0;
    ofn.lpstrFilter = TEXT("Level Files\0*.xlv\0");
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = startDir;
    ofn.Flags = OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;

    TCHAR curDirectory[512];
    GetCurrentDirectory(511, curDirectory);

    if(GetSaveFileName(&ofn))
    {
        SetCurrentDirectory(curDirectory);

        if(!GetPathExtension(lpFile).CompareI(TEXT("xlv")))
            scat(lpFile, TEXT(".xlv"));

        levelInfo->strLevelName = lpFile;
        levelInfo->strLevelName.FindReplace(TEXT("\\"), TEXT("/"));

        String newTitle;
        if(curWorkingModule.IsEmpty())
            newTitle << TEXT("No Module Loaded");
        else
            newTitle << curWorkingModule;

        if(level->IsOf(GetClass(IndoorLevel)))
        {
            newTitle << TEXT(": [") << GetPathFileName(levelInfo->strLevelName) << TEXT("] - Indoor Level - Xed");
            SetWindowText(hwndEditor, newTitle);

            SaveIndoorLevel();
        }
        else if(level->IsOf(GetClass(OutdoorLevel)))
        {
            newTitle << TEXT(": [") << GetPathFileName(levelInfo->strLevelName) << TEXT("] - Outdoor Level - Xed");
            SetWindowText(hwndEditor, newTitle);

            SaveOutdoorLevel();
        }
        else if(level->IsOf(GetClass(OctLevel)))
        {
            newTitle << TEXT(": [") << GetPathFileName(levelInfo->strLevelName) << TEXT("] - Open Level - Xed");
            SetWindowText(hwndEditor, newTitle);

            SaveOctLevel();
        }
    }
    else
        SetCurrentDirectory(curDirectory);

    RemoveDirectory(startDir);

    traceOut;
}

void EditorEngine::OpenLevel()
{
    traceIn(EditorEngine::OpenLevel);

    TCHAR lpFile[512];
    OPENFILENAME ofn;
    zero(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndEditor;
    ofn.lpstrFile = lpFile;
    ofn.nMaxFile = 511;

    TCHAR curDirectory[512];
    GetCurrentDirectory(511, curDirectory);

    String startDir;
    startDir << curDirectory << TEXT("\\data\\") << curWorkingModule << TEXT("\\levels");
    if(!OSFileExists(startDir))
        startDir << curDirectory << TEXT("\\data");

    ofn.lpstrFile[0] = 0;
    ofn.lpstrFilter = TEXT("Level Files\0*.xlv\0");
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = startDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if(GetOpenFileName(&ofn))
    {
        SetCurrentDirectory(curDirectory);

        String strFile = lpFile;
        strFile.FindReplace(TEXT("\\"), TEXT("/"));

        OpenEditableLevel(strFile);
    }
    else
        SetCurrentDirectory(curDirectory);

    traceOut;
}

BOOL EditorEngine::OpenEditableLevel(CTSTR lpName, BOOL bShowMessages)
{
    traceIn(EditorEngine::OpenEditableLevel);

    assert(lpName);

    XFileInputSerializer levelData;

    //--------------------------------------------------

    DWORD dwSigniture, dwEditorOffset;
    DWORD i;
    WORD wVersion;

    String convertedPath, resourceName;
    GetResourceStringFromPath(lpName, TEXT("levels"), resourceName);
    Engine::ConvertResourceName(resourceName, TEXT("levels"), convertedPath, editor->bAddonModule);

    if(!levelData.Open(convertedPath))
    {
        if(bShowMessages)
            OSMessageBox(TEXT("Unable to open the level: \"%s\""), lpName);
        return FALSE;
    }

    levelData << dwSigniture;
    levelData << wVersion;
    levelData << dwEditorOffset;

    if((dwSigniture != '\0lix') && (dwSigniture != '\0lox') && (dwSigniture != '\0ltx'))
    {
        if(bShowMessages)
            OSMessageBox(TEXT("'%s', Bad Level file"), lpName);
        return FALSE;
    }

    if( ((dwSigniture == '\0ltx') && (wVersion != OCTLEVEL_VERSION)) ||
        ((dwSigniture == '\0lix') && (wVersion != INDOOR_VERSION)) ||
        ((dwSigniture == '\0lox') && (wVersion != OUTDOOR_VERSION)) )
    {
        if(bShowMessages)
            OSMessageBox(TEXT("'%s', Outdated file format version"), lpName);
        return FALSE;
    }

    if(!dwEditorOffset)
    {
        if(bShowMessages)
            OSMessageBox(TEXT("'%s', No editor data."), lpName);
        return FALSE;
    }

    //--------------------------------------------------

    KillViewports();

    if(level)
    {
        DestroyObject(levelInfo);
        DestroyObject(level);
        level = NULL;
    }

    levelData.Close();

    //--------------------------------------------------

    if(!LoadLevel(resourceName))
        return FALSE;

    levelInfo = CreateObject(EditorLevelInfo);
    levelInfo->strLevelName = lpName;

    //--------------------------------------------------

    String newTitle;
    if(curWorkingModule.IsEmpty())
        newTitle << TEXT("No Module Loaded");
    else
        newTitle << curWorkingModule;

    if(level->IsOf(GetClass(IndoorLevel)))
    {
        newTitle << TEXT(": [") << GetPathFileName(lpName) << TEXT("] - Indoor Level - Xed");
        SetWindowText(hwndEditor, newTitle);
        editor->CreateIndoorSidebar();
    }
    else if(level->IsOf(GetClass(OutdoorLevel)))
    {
        newTitle << TEXT(": [") << GetPathFileName(lpName) << TEXT("] - Outdoor Level - Xed");
        SetWindowText(hwndEditor, newTitle);
        editor->CreateOutdoorSidebar();
    }
    else if(level->IsOf(GetClass(OctLevel)))
    {
        newTitle << TEXT(": [") << GetPathFileName(lpName) << TEXT("] - Open Level - Xed");
        SetWindowText(hwndEditor, newTitle);
        editor->CreateOctLevelSidebar();
    }

    //--------------------------------------------------

    levelData.Open(lpName);
    levelData.Seek(dwEditorOffset, SERIALIZE_SEEK_START);

    //--------------------------------------------------

    BOOL bHasWorkbrush;
    DWORD nEditorBrushes;

    levelData << bHasWorkbrush;

    if(bHasWorkbrush)
    {
        levelInfo->WorkBrush = CreateObject(EditorBrush);
        levelInfo->WorkBrush->SerializeBrushData(levelData, FALSE);
    }

    levelData << nEditorBrushes;
    for(i=0; i<nEditorBrushes; i++)
    {
        EditorBrush *brush = CreateObject(EditorBrush);
        brush->SerializeBrushData(levelData, FALSE);
    }

    levelData << levelInfo->bSnapToGrid;
    levelData << levelInfo->gridSpacing;

    SendMessage(GetDlgItem(hwndSidebar, ID_GRID_SNAP), BM_SETCHECK, levelInfo->bSnapToGrid ? BST_CHECKED : BST_UNCHECKED, 0);

    Layout newViewportLayout;
    Vect2 newSplitPos;

    levelData << (DWORD&)newViewportLayout;
    levelData << newSplitPos;

    SetViewportLayout(newViewportLayout, &newSplitPos);

    for(i=0; i<Viewports.Num(); i++)
        Viewports[i]->SerializeSettings(levelData);

    levelData << levelInfo->curYPlanePosition;

    ModifyMode newModifyMode = ModifyMode_Move;
    levelData << (int&)newModifyMode;
    levelInfo->SetCurModifyMode(newModifyMode);

    SelectMode newSelectMode = SelectMode_ObjectsAndDetails;
    levelData << (int&)newSelectMode;
    levelInfo->SetCurSelectMode(newSelectMode);

    levelData << levelInfo->lightmapSettings;

    return TRUE;

    traceOut;
}


inline void WriteDW(Serializer &s, DWORD val)
{
    s << val;
}

inline void WriteW(Serializer &s, WORD val)
{
    s << val;
}


void EditorEngine::SaveIndoorLevel()
{
    traceIn(EditorEngine::SaveIndoorLevel);

    IndoorLevel *indoorLevel = (IndoorLevel*)level;

    XFileOutputSerializer levelData;

    DWORD i;

    String fileName = levelInfo->strLevelName;
    if(bAddonModule)
    {
        TSTR lpModuleName = sstr(levelInfo->strLevelName, TEXT("data/"))+5;
        if(lpModuleName)
        {
            TSTR lpSlash = schr(lpModuleName, '/');
            if(lpSlash)
            {
                *lpSlash = 0;
                String strModule = lpModuleName;
                *lpSlash = '/';

                if(!strModule.CompareI(editor->curWorkingModule))
                {
                    fileName.Clear();
                    fileName << TEXT("data/") << curWorkingModule << TEXT("/override");
                    OSCreateDirectory(fileName);
                    fileName << TEXT("/") << strModule;
                    OSCreateDirectory(fileName);
                    fileName << TEXT("/levels");
                    OSCreateDirectory(fileName);
                    fileName << TEXT("/") << GetPathFileName(levelInfo->strLevelName, TRUE);
                }
            }
        }
    }

    if(!levelData.Open(fileName, XFILE_CREATEALWAYS))
    {
        OSMessageBox(TEXT("Could not open file '%s'"), fileName.Array());
        return;
    }

    levelInfo->bModified = FALSE;

    List<Entity*> LevelEntList;

    Entity *ent = Entity::FirstEntity();
    while(ent)
    {
        if(ent->GetEditType())
            LevelEntList << ent;

        ent = ent->NextEntity();
    }

    //----------------------------------------------

    WriteDW(levelData, '\0lix'); //signiture
    WriteW(levelData, INDOOR_VERSION);   //file version
    WriteDW(levelData, 0);       //editor offset, which comes after.

    //----------------------------------------------

    WriteDW(levelData, indoorLevel->LevelModules.Num());
    WriteDW(levelData, indoorLevel->PVSList.Num());
    WriteDW(levelData, indoorLevel->PortalList.Num());
    WriteDW(levelData, indoorLevel->BrushList.Num());
    WriteDW(levelData, LevelEntList.Num());
    WriteDW(levelData, indoorLevel->CinematicList.Num()); //cinematics, currently not implemented.  TODO: implement cinematics, jerk.
    levelData << indoorLevel->LightmapTechnique;   //lightmapping type.  none/bare/bump.  currently not implemented.  TODO: implement.
    levelData << indoorLevel->bGlobalFog;
    levelData << indoorLevel->dwGlobalFogColor;

    //----------------------------------------------

    for(i=0; i<indoorLevel->LevelModules.Num(); i++)
        levelData << indoorLevel->LevelModules[i];

    //----------------------------------------------

    for(i=0; i<LevelEntList.Num(); i++)
    {
        Entity *ent = LevelEntList[i];

        Class *classInfo = ent->GetObjectClass();

        String strClassName = classInfo->GetName();
        levelData << strClassName;

        ent->Serialize(levelData);
    }

    //----------------------------------------------

    //uhm..  cinematics...  hi.
    for(i=0; i<indoorLevel->CinematicList.Num(); i++)
        levelData << indoorLevel->CinematicList[i].Keys;

    //----------------------------------------------

    for(i=0; i<indoorLevel->PortalList.Num(); i++)
        indoorLevel->PortalList[i].Serialize(levelData);

    //----------------------------------------------

    for(i=0; i<indoorLevel->BrushList.Num(); i++)
        indoorLevel->BrushList[i].Serialize(levelData);

    //----------------------------------------------

    for(i=0; i<indoorLevel->PVSList.Num(); i++)
        indoorLevel->PVSList[i].Serialize(levelData);

    levelData << indoorLevel->entityIDCounter;

    levelData << level->bHasLightmaps;

    //----------------------------------------------
    //----------------------------------------------

    DWORD dwEditorOffset = levelData.GetPos();

    BOOL bHasWorkBrush = UPARAM(levelInfo->WorkBrush) > 0;

    levelData << bHasWorkBrush;

    if(bHasWorkBrush)
        levelInfo->WorkBrush->SerializeBrushData(levelData, FALSE);

    WriteDW(levelData, levelInfo->BrushList.Num());

    for(i=0; i<levelInfo->BrushList.Num(); i++)
        levelInfo->BrushList[i]->SerializeBrushData(levelData, FALSE);

    levelData << levelInfo->bSnapToGrid;
    levelData << levelInfo->gridSpacing;

    levelData << (DWORD&)currentLayout;
    levelData << splitPos;

    for(i=0; i<Viewports.Num(); i++)
        Viewports[i]->SerializeSettings(levelData);

    levelData << levelInfo->curYPlanePosition;

    levelData << (int&)levelInfo->curModifyMode;
    levelData << (int&)levelInfo->curSelectMode;

    levelData << levelInfo->lightmapSettings;

    //----------------------------------------------

    levelData.Close();

    XFile levelFile;
    levelFile.Open(fileName, XFILE_READ|XFILE_WRITE, XFILE_OPENEXISTING);
    levelFile.SetPos(6, XFILE_BEGIN);
    levelFile.Write(&dwEditorOffset, 4);
    levelFile.Close();

    traceOut;
}

void EditorEngine::SaveOutdoorLevel()
{
    OutdoorLevel *outdoorLevel = (OutdoorLevel*)level;
}

void EditorEngine::SaveOctLevel()
{
    traceIn(EditorEngine::SaveOctLevel);

    OctLevel *octLevel = (OctLevel*)level;

    XFileOutputSerializer levelData;

    String fileName = levelInfo->strLevelName;
    if(bAddonModule)
    {
        TSTR lpModuleName = sstr(levelInfo->strLevelName, TEXT("data/"))+5;
        if(lpModuleName)
        {
            TSTR lpSlash = schr(lpModuleName, '/');
            if(lpSlash)
            {
                *lpSlash = 0;
                String strModule = lpModuleName;
                *lpSlash = '/';

                if(!strModule.CompareI(editor->curWorkingModule))
                {
                    fileName.Clear();
                    fileName << TEXT("data/") << curWorkingModule << TEXT("/override");
                    OSCreateDirectory(fileName);
                    fileName << TEXT("/") << strModule;
                    OSCreateDirectory(fileName);
                    fileName << TEXT("/levels");
                    OSCreateDirectory(fileName);
                    fileName << TEXT("/") << GetPathFileName(levelInfo->strLevelName, TRUE);
                }
            }
        }
    }

    if(!levelData.Open(fileName, XFILE_CREATEALWAYS))
    {
        OSMessageBox(TEXT("Could not open file '%s'"), fileName.Array());
        return;
    }

    levelInfo->bModified = FALSE;

    DWORD i;

    List<Entity*> LevelEntList;

    Entity *ent = Entity::FirstEntity();
    while(ent)
    {
        if(ent->GetEditType())
            LevelEntList << ent;

        ent = ent->NextEntity();
    }

    //----------------------------------------------

    WriteDW(levelData, '\0ltx'); //signiture
    WriteW(levelData, OCTLEVEL_VERSION);    //file version
    WriteDW(levelData, 0);       //editor offset, which comes after.

    //----------------------------------------------

    BOOL bObsolete = 0;
    DWORD dwObsolete = 0;

    WriteDW(levelData, octLevel->LevelModules.Num());
    WriteDW(levelData, octLevel->BrushList.Num());
    WriteDW(levelData, LevelEntList.Num());
    WriteDW(levelData, octLevel->CinematicList.Num()); //cinematics, currently not implemented.  TODO: implement cinematics, jerk.
    levelData << dwObsolete;   //lightmapping type.  none/bare/bump.  currently not implemented.  TODO: implement.

    levelData << bObsolete;
    levelData << dwObsolete;

    //----------------------------------------------

    for(i=0; i<octLevel->LevelModules.Num(); i++)
        levelData << octLevel->LevelModules[i];

    //----------------------------------------------

    for(i=0; i<LevelEntList.Num(); i++)
    {
        Entity *ent = LevelEntList[i];

        /*Class *classInfo = ent->GetObjectClass();

        String strClassName = classInfo->GetName();
        levelData << strClassName;

        int offsetPos = 0;
        int savePos = levelData.GetPos();
        levelData << offsetPos;

        ent->Serialize(levelData);

        offsetPos = levelData.GetPos();
        levelData.Seek(savePos, SERIALIZE_SEEK_START);
        levelData << offsetPos;
        levelData.Seek(0, SERIALIZE_SEEK_END);*/

        Entity::SaveEntity(levelData, ent);
    }

    //----------------------------------------------

    //uhm..  cinematics...  hi.
    for(i=0; i<octLevel->CinematicList.Num(); i++)
        levelData << octLevel->CinematicList[i].Keys;

    //----------------------------------------------

    levelData << dwObsolete << dwObsolete;

    //----------------------------------------------

    for(i=0; i<octLevel->BrushList.Num(); i++)
        octLevel->BrushList[i]->Serialize(levelData);

    //----------------------------------------------

    levelData << octLevel->entityIDCounter;

    //----------------------------------------------
    //----------------------------------------------

    DWORD dwEditorOffset = levelData.GetPos();

    BOOL bHasWorkBrush = UPARAM(levelInfo->WorkBrush) > 0;

    levelData << bHasWorkBrush;

    if(bHasWorkBrush)
        levelInfo->WorkBrush->SerializeBrushData(levelData, FALSE);

    WriteDW(levelData, levelInfo->BrushList.Num());

    for(i=0; i<levelInfo->BrushList.Num(); i++)
        levelInfo->BrushList[i]->SerializeBrushData(levelData, FALSE);

    levelData << levelInfo->bSnapToGrid;
    levelData << levelInfo->gridSpacing;

    levelData << (DWORD&)currentLayout;
    levelData << splitPos;

    for(i=0; i<Viewports.Num(); i++)
        Viewports[i]->SerializeSettings(levelData);

    levelData << levelInfo->curYPlanePosition;

    levelData << (int&)levelInfo->curModifyMode;
    levelData << (int&)levelInfo->curSelectMode;

    levelData << levelInfo->lightmapSettings;

    //----------------------------------------------

    levelData.Close();

    XFile levelFile;
    levelFile.Open(fileName, XFILE_READ|XFILE_WRITE, XFILE_OPENEXISTING);
    levelFile.SetPos(6, XFILE_BEGIN);
    levelFile.Write(&dwEditorOffset, 4);
    levelFile.Close();

    traceOut;
}


#define PATH_TOPLEFT     (1)
#define PATH_TOP         (1<<1)
#define PATH_TOPRIGHT    (1<<2)
#define PATH_RIGHT       (1<<3)
#define PATH_BOTTOMRIGHT (1<<4)
#define PATH_BOTTOM      (1<<5)
#define PATH_BOTTOMLEFT  (1<<6)
#define PATH_LEFT        (1<<7)


BOOL CALLBACK CreateOutdoorDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(CreateOutdoorDialogProc);

    switch(message)
    {
        case WM_INITDIALOG:
            {
                SendMessage(GetDlgItem(hwnd, IDC_TERRAINSIZE), CB_ADDSTRING, 0, (LPARAM)TEXT("128x128"));
                SendMessage(GetDlgItem(hwnd, IDC_TERRAINSIZE), CB_ADDSTRING, 0, (LPARAM)TEXT("256x256"));
                SendMessage(GetDlgItem(hwnd, IDC_TERRAINSIZE), CB_ADDSTRING, 0, (LPARAM)TEXT("512x512"));
                //SendMessage(GetDlgItem(hwnd, IDC_TERRAINSIZE), CB_ADDSTRING, 0, (LPARAM)TEXT("1024x1024"));
                //SendMessage(GetDlgItem(hwnd, IDC_TERRAINSIZE), CB_ADDSTRING, 0, (LPARAM)TEXT("2048x2048"));
                SendMessage(GetDlgItem(hwnd, IDC_TERRAINSIZE), CB_SETCURSEL, 1, 0);

                LinkUpDown(GetDlgItem(hwnd, IDC_BLOCKSIZESCROLLER), GetDlgItem(hwnd, IDC_BLOCKSIZE));
                InitUpDownFloatData(GetDlgItem(hwnd, IDC_BLOCKSIZESCROLLER), 8.0f, 2.0f, 20.0f, 0.1f);

                SendMessage(GetDlgItem(hwnd, IDC_TEXSIZE), CB_ADDSTRING, 0, (LPARAM)TEXT("8 - Low Detail"));
                SendMessage(GetDlgItem(hwnd, IDC_TEXSIZE), CB_ADDSTRING, 0, (LPARAM)TEXT("16 - Normal Detail"));
                SendMessage(GetDlgItem(hwnd, IDC_TEXSIZE), CB_ADDSTRING, 0, (LPARAM)TEXT("32 - High Detail"));
                SendMessage(GetDlgItem(hwnd, IDC_TEXSIZE), CB_SETCURSEL, 1, 0);
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_TEXSIZE, CBN_SELCHANGE), 0);
                break;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    outdoorCreationData.bHasWater = SendMessage(GetDlgItem(hwnd, IDC_HASWATER), BM_GETCHECK, 0, 0) == BST_CHECKED;
                    outdoorCreationData.scale = GetUpDownFloat(GetDlgItem(hwnd, IDC_BLOCKSIZESCROLLER));
                    outdoorCreationData.size = SendMessage(GetDlgItem(hwnd, IDC_TERRAINSIZE), CB_GETCURSEL, 0, 0);
                    EndDialog(hwnd, IDOK);
                    break;

                case IDC_TERRAINSIZE:
                case IDC_TEXSIZE:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        DWORD curDetail = (SendMessage(GetDlgItem(hwnd, IDC_TEXSIZE), CB_GETCURSEL, 0, 0)+1)*8;
                        DWORD curBlockSize = (SendMessage(GetDlgItem(hwnd, IDC_TERRAINSIZE), CB_GETCURSEL, 0, 0)+1)*128;

                        DWORD totalSize = (curBlockSize*curDetail);
                        totalSize *= totalSize;
                        totalSize *= 2;

                        String strTotalSize;
                        strTotalSize << (unsigned int)totalSize;
                        strTotalSize.GroupDigits();
                        SendMessage(GetDlgItem(hwnd, IDC_ESTTEXSIZE), WM_SETTEXT, 0, (LPARAM)(CTSTR)strTotalSize);
                    }
                    break;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
            }
    }

    return FALSE;

    traceOut;
}

BOOL CALLBACK SaveLevelDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    traceIn(SaveLevelDialogProc);

    switch(message)
    {
        case WM_INITDIALOG:
            {
                OSFindData findData;

                HANDLE hFind = OSFindFirstFile(TEXT("data/levels/*.xlv"), findData);

                if(hFind)
                {
                    do
                    {
                        findData.fileName[slen(findData.fileName)-4] = 0;
                        SendMessage(GetDlgItem(hwnd, IDC_LEVELLIST), LB_ADDSTRING, 0, (LPARAM)findData.fileName);
                    }while(OSFindNextFile(hFind, findData));

                    OSFindClose(hFind);

                    SendMessage(GetDlgItem(hwnd, IDC_LEVELLIST), LB_SETCURSEL, 0, 0);
                    SendMessage(GetDlgItem(hwnd, IDC_LEVELNAME), EM_LIMITTEXT, 250, 0);
                }
                break;
            }

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_LEVELLIST:
                    if(HIWORD(wParam) == LBN_SELCHANGE)
                    {
                        DWORD dwCurSel = SendMessage(GetDlgItem(hwnd, IDC_LEVELLIST), LB_GETCURSEL, 0, 0);

                        if(dwCurSel != LB_ERR)
                        {
                            String strLevelName;
                            strLevelName.SetLength(255);
                            SendMessage(GetDlgItem(hwnd, IDC_LEVELLIST), LB_GETTEXT, dwCurSel, (LPARAM)(CTSTR)strLevelName);

                            SendMessage(GetDlgItem(hwnd, IDC_LEVELNAME), WM_SETTEXT, 0, (LPARAM)(CTSTR)strLevelName);
                        }
                    }
                    else if(HIWORD(wParam) == LBN_DBLCLK)
                        PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), lParam);
                    break;

                case IDOK:
                    {
                        String strLevelName;
                        strLevelName.SetLength(255);
                        SendMessage(GetDlgItem(hwnd, IDC_LEVELNAME), WM_GETTEXT, 255, (LPARAM)(CTSTR)strLevelName);

                        if(!strLevelName.Length())
                        {
                            MessageBox(hwnd, TEXT("Hi!  ..er, oh.  You forgot to enter a name for your level.\r\n\r\n...Sorry, didn't think anyone would ever invoke this message box."), NULL, MB_OK);
                            break;
                        }

                        if(SendMessage(GetDlgItem(hwnd, IDC_LEVELLIST), LB_FINDSTRINGEXACT, -1, (LPARAM)(CTSTR)strLevelName) != LB_ERR)
                        {
                            String errorStringThing;
                            errorStringThing << TEXT("\"") << strLevelName << TEXT("\" already exists.\r\nDo you want to replace it?");
                            if(MessageBox(hwnd, errorStringThing, TEXT("*Cough*.  I sense a fire."), MB_YESNO) == IDNO)
                                break;
                        }

                        levelInfo->strLevelName = strLevelName;

                        String newTitle;
                        if(editor->curWorkingModule.IsEmpty())
                            newTitle << TEXT("No Module Loaded");
                        else
                            newTitle << editor->curWorkingModule;

                        if(level->IsOf(GetClass(IndoorLevel)))
                        {
                            newTitle << TEXT(": [") << strLevelName << TEXT("] - Indoor Level - Xed");
                            SetWindowText(hwndEditor, newTitle);
                            editor->SaveIndoorLevel();
                        }
                        else if(level->IsOf(GetClass(OutdoorLevel)))
                        {
                            newTitle << TEXT(": [") << strLevelName << TEXT("] - Outdoor Level - Xed");
                            SetWindowText(hwndEditor, newTitle);
                            editor->SaveOutdoorLevel();
                        }
                        else if(level->IsOf(GetClass(OctLevel)))
                        {
                            newTitle << TEXT(": [") << strLevelName << TEXT("] - Open Level - Xed");
                            SetWindowText(hwndEditor, newTitle);
                            editor->SaveOctLevel();
                        }

                        EndDialog(hwnd, IDOK);
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

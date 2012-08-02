/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Engine.cpp:  Engine

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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "Base.h"


DefineClass(Engine);



void ENGINEAPI TempMouseHandler(int action, LONG param1, LONG param2);
void ENGINEAPI TempKBHandler(unsigned int vk, BOOL keydown);


GameModule *LoadGameModuleObject(CTSTR lpModule);


NATIVELOADERCALLBACK Engine::curNativeLoader = NULL;
int Engine::lastModuleID = 1;
TCHAR Engine::loadingModule[64] = TEXT("");
List<ModuleInfo> Engine::Modules;

BOOL bFirstTick         = 1;



void Engine::Init()
{
    traceIn(Engine::Init);
    //Super::Init();

    ModuleInfo *mi = Modules.CreateNew();
    scpy(mi->name, TEXT("Base"));
    mi->moduleHash = StringCRC16I(mi->name);
    mi->bEditable = TRUE;

    engine = this;

    //-----------------------------------------

    locale = new LocaleStringLookup;

    String langModule;
    BOOL bLanguageModuleLoaded = FALSE;

    ConfigFile baseConfig;
    baseConfig.Open(TEXT("data/Base/Module.ini"));
    if(baseConfig.GetString(TEXT("Module"), TEXT("Language"), locale->GetLanguage()) != locale->GetLanguage())
    {
        langModule << TEXT("Base.") << locale->GetLanguage();
        bLanguageModuleLoaded = LoadGameModule(langModule, FALSE);
    }
    baseConfig.Close();

    locale->LoadStringFile(TEXT("Base:default.txt"));

    //-----------------------------------------

    Scripting = new ScriptSystem;
    String errorList;

    if(!Scripting->LoadModuleScriptData(TEXT("Base"), errorList))
        CrashError(TEXT("Compilation of base engine script files failed"));
    Scripting->LoadBaseScriptingDefinitions();
    errorList.Clear();

    if(bLanguageModuleLoaded)
        InitModule(langModule);

    //-----------------------------------------

    Log(TEXT("Initializing Base Engine"));

    if(!hwndMain)
    {
        String strGameName = AppConfig->GetString(TEXT("Engine"), TEXT("GameName"), TEXT("Game"));
        if(!(hwndMain = OSCreateMainWindow(strGameName, 640, 480, FALSE)))
            CrashError(TEXT("Could not create main window"));
    }

    //-----------------------------------------

    String strGraphicsSystem = AppConfig->GetString(TEXT("Engine"), TEXT("GraphicsSystem"));
    GS = (GraphicsSystem*)CreateFactoryObject(strGraphicsSystem, FALSE);

    if(!GS)
        CrashError(TEXT("Bad Graphics System: %s"), (TSTR)strGraphicsSystem);

    GS->InitializeDevice(hwndMain);
    GS->Init();

    //-----------------------------------------

    String strSoundSystem = AppConfig->GetString(TEXT("Engine"), TEXT("SoundSystem"), TEXT("NullSound"));
    SS = (SoundSystem*)CreateFactoryObject(strSoundSystem);

    if(!SS)
        CrashError(TEXT("Bad Sound System: %s"), (TSTR)strSoundSystem);

    //-----------------------------------------

    if(AppConfig->GetInt(TEXT("Engine"), TEXT("InitializeGameSystems"), TRUE))
    {
        physics = (PhysicsSystem*)CreateFactoryObject(TEXT("BulletPhysics"));
        MM = CreateObject(MusicManager);
    }

    //-----------------------------------------

    RM = new ResourceManager;

    TickID = TrackTimeBegin(FALSE);

    traceOut;
}

void Engine::Destroy()
{
    traceIn(Engine::Destroy);

    if(level)
        DestroyObject(level);

    if(MM)
    {
        MM->Destroy();
        delete MM;
    }

    delete RM;

    DestroyObject(physics);

    TrackTimeEnd(TickID);

    DestroyObject(SS);
    SS = NULL;

    DestroyObject(GS);
    GS = NULL;

    //Log(TEXT("Average FPS: %d"), dwAverageFPS);

    Log(TEXT("Freed Base Engine"));

    //OSDestroyMainWindow();

    //Super::Destroy();

    traceOut;
}

void Engine::Update(BOOL bProcessTime)
{
    traceIn(Engine::Update);

    profileSegment("main engine loop");

    if(bUpdating || bBlockViewUpdates)
        return;

    bUpdating = TRUE;
    frameCounter++;

    totalPolys = nPolys;
    nPolys = 0;
    nPVSs = 0;
    nLights = 0;

    //---------------------------

    FrameObject *curObj = FrameObject::FirstFrameObject();
    while(curObj)
    {
        curObj->PreFrame();
        curObj = curObj->NextFrameObject();
    }

    //---------------------------

    DWORD dwTime = TrackTimeRestart(TickID);
    float fSeconds = MSToSeconds(dwTime);

    if(!bProcessTime || !bPreviouslyProcessedTime)
    {
        dwTime = 0;
        fSeconds = 0.0f;
    }

    if(bFirstTick) bFirstTick = dwTime = 0;

    //---------------------------

    fpsTimer += dwTime;
    if(fpsTimer >= 1000)
    {
        curFPS = frameCounter;
        fpsTimer = frameCounter = 0;
    }

    //---------------------------

    float f3DSeconds = fSeconds;
    if(CurrentGame)
    {
        if(CurrentGame->IsPaused())
            f3DSeconds = 0.0f;
        else
        {
            float speed = CurrentGame->GetGameSpeed();
            if(speed != 1.0f)
                f3DSeconds *= speed;
        }
    }

    //---------------------------

    if(dwTime)
    {
        profileIn("Global Tick Processing");

        curObj = FrameObject::FirstFrameObject();
        while(curObj)
        {
            if(curObj->IsOf(GetClass(Entity)))
                curObj->Tick(f3DSeconds);
            else
                curObj->Tick(fSeconds);

            curObj = curObj->NextFrameObject();
        }

        profileOut;

        if(physics)
            physics->UpdatePhysics(f3DSeconds);
    }

    //---------------------------

    curObj = FrameObject::LastFrameObject();
    while(curObj)
    {
        FrameObject *prevObj = curObj->PrevFrameObject();

        if(curObj->bMarkedForDestruction)
            DestroyObject(curObj);

        curObj = prevObj;
    }

    //---------------------------

    SS->UpdatePositionalData();

    GS->PreRenderScene();

    if(level)
        level->ArrangeEntities();

    GS->RenderScene(TRUE, 0xFF000000 | bgColor);
    GS->PostRenderScene();

    bPreviouslyProcessedTime = bProcessTime;

    bUpdating = FALSE;

    traceOut;
}


BOOL Engine::ConvertResourceName(CTSTR lpName, CTSTR lpDir, String &str, BOOL bOverridable)
{
    traceIn(Engine::ConvertResourceName);

    String name = lpName;
    name.FindReplace(TEXT("\\"), TEXT("/"));

    if(name.NumTokens(':') != 2)
    {
        //AppWarning(TEXT("Invalid name '%s', name must be [Module]:[File]"), lpName);
        return FALSE;
    }

    String module = name.GetToken(0, ':');

    int moduleID = GetModuleID(module);
    if(moduleID == -1)
    {
        if(level)
        {
            if(!level->LoadLevelModule(module))
            {
                AppWarning(TEXT("Could not find module '%s'"), module.Array());
                return FALSE;
            }
        }
        else
        {
            if(!LoadGameModule(module))
            {
                AppWarning(TEXT("Could not find module '%s'"), module.Array());
                return FALSE;
            }
        }

        int moduleID = GetModuleID(lpName);
    }

    String resName = name.GetToken(1, ':');
    String resourceStr, overrideStr;
    if(lpDir && *lpDir)
        resourceStr << module << TEXT("/") << lpDir << TEXT("/") << resName;
    else
        resourceStr << module << TEXT("/") << resName;

    if(bOverridable)
    {
        overrideStr << TEXT("/override/") << resourceStr;

        for(int i=Modules.Num()-1; i>=0; i--)
        {
            ModuleInfo &curModule = Modules[i];

            if(curModule.bAddonModule && curModule.ModuleOverrides.HasValue(moduleID))
            {
                str.Clear();
                str << TEXT("data/") << curModule.name << overrideStr;

                if(OSFileExists(str))
                    return TRUE;
            }
        }
    }

    str.Clear();
    str << TEXT("data/") << resourceStr;

    return TRUE;

    traceOut;
}

void Engine::GetGameModules(List<CTSTR> &ModuleList)
{
    ModuleList.Clear();

    for(int i=0; i<Modules.Num(); i++)
    {
        if(!Modules[i].bEditable)
            continue;

        //also exclude modules that are pure dll
        String strModuleDir;
        strModuleDir << TEXT("data/") << Modules[i].name;
        if(!OSFileExists(strModuleDir))
            continue;

        ModuleList << Modules[i].name;
    }
}

GameModule *LoadGameModuleObject(CTSTR lpModule)
{
    traceIn(LoadGameModuleObject);

    if(scmpi(lpModule, TEXT("main")) == 0)
        return NULL;

    String strModuleClass;
    strModuleClass << lpModule;
    for(int i=0; i<strModuleClass.Length(); i++)
    {
        if(!i)
        {
            if((strModuleClass[i] >= 'a') && (strModuleClass[i] <= 'z'))
                strModuleClass[i] -= 0x20;
        }
        else
        {
            if((strModuleClass[i] >= 'A') && (strModuleClass[i] <= 'Z'))
                strModuleClass[i] += 0x20;
        }
    }

    strModuleClass << TEXT("GameModule");

    return (GameModule*)CreateFactoryObject(strModuleClass);

    traceOut;
}

BOOL Engine::LoadGameModule(CTSTR lpModule, BOOL bInit)
{
    traceIn(Engine::LoadGameModule);

    int i;

    for(i=0; i<Modules.Num(); i++)
    {
        if(scmpi(lpModule, Modules[i].name) == 0)
            return TRUE;
    }

    String strDLL;
    strDLL << GetBinDir() << lpModule << TEXT(".*");

    //-----------------------------------------

    String strModuleDir;
    strModuleDir << TEXT("data/") << lpModule;

    if(!OSFileExists(strModuleDir) && !OSFileExists(strDLL))
        return FALSE;

    ConfigFile moduleConfig;
    moduleConfig.Open(strModuleDir + TEXT("/Module.ini"));
    BOOL bAddon = moduleConfig.GetInt(TEXT("Module"), TEXT("Type"), 1);
    BOOL bEditable = moduleConfig.GetInt(TEXT("Module"), TEXT("Editable"), 1);

    List<int> Overrides;
    if(bAddon)
    {
        OSFindData ofd;
        HANDLE hFind = OSFindFirstFile(strModuleDir + TEXT("/override/*"), ofd);

        if(hFind)
        {
            do
            {
                LoadGameModule(ofd.fileName);
                int modID = GetModuleID(ofd.fileName);
                if(modID != -1)
                    Overrides << modID;
            }while(OSFindNextFile(hFind, ofd));

            OSFindClose(hFind);
        }
    }

    //-----------------------------------------

    UINT moduleIndex = Modules.Num();

    ModuleInfo &modInfo = *Modules.CreateNew();
    scpy(modInfo.name, lpModule);
    modInfo.moduleHash = StringCRC16I(lpModule);
    modInfo.bAddonModule = bAddon;
    modInfo.bEditable = bEditable;
    modInfo.moduleID = lastModuleID++;
    if(bAddon)
        modInfo.ModuleOverrides.CopyList(Overrides);

    //-----------------------------------------

    String langModule;
    BOOL bLanguageModuleLoaded = FALSE;

    if(moduleConfig.GetString(TEXT("Module"), TEXT("Language"), locale->GetLanguage()) != locale->GetLanguage())
    {
        langModule << lpModule << TEXT(".") << locale->GetLanguage();
        bLanguageModuleLoaded = LoadGameModule(langModule, FALSE);
    }

    String stringFile;
    stringFile << lpModule << TEXT(":default.txt");

    locale->LoadStringFile(stringFile);

    BOOL bSuccess = TRUE;
    if(bInit)
        bSuccess = InitModule(lpModule);

    if(bLanguageModuleLoaded)
        InitModule(langModule);

    return bSuccess;

    traceOut;
}

BOOL Engine::InitModule(CTSTR lpModule)
{
    ModuleInfo *mi = GetModule(lpModule);
    assert(mi);
    if(!mi) return FALSE;

    //-----------------------------------------

    String strDLL;
    strDLL << GetBinDir() << lpModule;

    String strModuleDir;
    strModuleDir << TEXT("data/") << lpModule;

    scpy(loadingModule, lpModule);
    HANDLE hModule = OSLoadLibrary(strDLL);

    if(!hModule && !OSFileExists(strModuleDir))
    {
        AppWarning(TEXT("Could not find any data for module '%s'"), loadingModule);
        return FALSE;
    }

    if(Scripting)
    {
        String errorList;
        if(!Scripting->LoadModuleScriptData(loadingModule, errorList))
            Log(TEXT("Warning: Compilation of module '%s' failed"), loadingModule);
    }

    if(curNativeLoader)
    {
        curNativeLoader();
        curNativeLoader = NULL;
    }

    //-----------------------------------------

    GameModule *gameModule = LoadGameModuleObject(lpModule);
    if(gameModule)
    {
        int numArgs;
        TSTR *lpCommands = OSGetCommandLine(numArgs);
        gameModule->ModuleStartup(lpCommands, numArgs);
    }

    loadingModule[0] = 0;

    mi->gameModule = gameModule;
    mi->hModule = hModule;

    Log(TEXT("Loaded Module: %s"), lpModule);

    return TRUE;
}

int Engine::SetNativeDataLoader(NATIVELOADERCALLBACK loader)
{
    curNativeLoader = loader;
    return 0;
}

BOOL Engine::IsModuleLoaded(CTSTR lpModule)
{
    for(DWORD i=0; i<Modules.Num(); i++)
    {
        if(scmpi(lpModule, Modules[i].name) == 0)
            return TRUE;
    }

    return FALSE;
}

void Engine::UnloadGameModule(CTSTR lpModule)
{
    traceIn(Engine::UnloadGameModule);

    for(int i=1; i<Modules.Num(); i++)
    {
        if(scmpi(lpModule, Modules[i].name) == 0)
        {
            BOOL bFoundObjects = FALSE;
            ModuleInfo &mi = Modules[i];

            if(mi.gameModule)
            {
                mi.gameModule->ModuleExit();
                DestroyObject(mi.gameModule);
            }

            Object *nextObj = Object::FirstObject();

            while(nextObj)
            {
                Object *curObj = nextObj;
                nextObj = nextObj->NextObject();

                if(scmpi(lpModule, curObj->GetObjectClass()->GetModule()) == 0)
                {
                    DestroyObject(curObj);
                    bFoundObjects = TRUE;
                }
            }

            if(Scripting)
                Scripting->UnloadModuleScriptData(lpModule);

            if(bFoundObjects)
                Log(TEXT("Warning: Some objects from module '%s' were never freed before the module was unloaded."), lpModule);

            OSFreeLibrary(mi.hModule);
            mi.ModuleOverrides.Clear();

            String stringFile;
            stringFile << lpModule << TEXT(":default.txt");
            locale->UnloadStringFile(stringFile);

            Log(TEXT("Unloaded Module: %s"), lpModule);

            Modules.Remove(i);
            return;
        }
    }

    traceOut;
}

void Engine::UnloadAllModules()
{
    traceIn(Engine::UnloadAllModules);

    for(DWORD i=1; i<Modules.Num(); i++)
    {
        OSFreeLibrary(Modules[i].hModule);
        Modules[i].ModuleOverrides.Clear();
    }
    Modules.Clear();

    Log(TEXT("Unloaded all modules."));

    traceOut;
}

int Engine::GetModuleID(CTSTR lpName)
{
    WORD hash = StringCRC16I(lpName);
    for(int i=0; i<Modules.Num(); i++)
    {
        ModuleInfo &mi = Modules[i];
        if(hash == mi.moduleHash)
            return mi.moduleID;
    }

    return -1;
}

ModuleInfo* Engine::GetModule(CTSTR lpName)
{
    WORD hash = StringCRC16I(lpName);
    for(int i=0; i<Modules.Num(); i++)
    {
        ModuleInfo &mi = Modules[i];
        if(mi.moduleHash == hash)
            return &mi;
    }

    return NULL;
}


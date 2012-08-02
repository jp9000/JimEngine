/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Engine.h:  Engine

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

#ifndef ENGINE_HEADER
#define ENGINE_HEADER


struct ModuleInfo
{
    int moduleID;
    WORD moduleHash;

    TCHAR name[64];
    GameModule *gameModule;
    HANDLE hModule;
    BOOL bAddonModule, bEditable;

    List<int> ModuleOverrides;
};



class BASE_EXPORT Engine : public Object 
{
    friend class Class;
    friend class Level;
    friend class ScriptCompilerEngine;
    friend class D3DSystem;
    friend class GLSystem;
    friend class EditorEngine;

    DeclareClass(Engine, Object);

private:
    //engine variables
    DWORD TickID;

    BOOL  bEditor;                          //build wireframe buffers

    BOOL  bUpdating;

    BOOL  bPreviouslyProcessedTime;

    //debug info variables
    DWORD frameCounter, curFPS, fpsTimer;   //frames per second
    DWORD totalPolys, nPolys;               //current polygon count
    DWORD nPVSs, nLights;

    static TCHAR loadingModule[64];
    static List<ModuleInfo> Modules;
    static int GetModuleID(CTSTR lpName);
    static ModuleInfo* GetModule(CTSTR lpModule);

    static int lastModuleID;
    static NATIVELOADERCALLBACK curNativeLoader;

    static BOOL InitModule(CTSTR lpModule);

protected:
    DWORD bgColor;

public:
    BOOL bBlockViewUpdates;

    virtual void Init();
    virtual void Destroy();

    //engine loop
    virtual void Update(BOOL bProcessTime=TRUE);

    inline DWORD GetCurFPS() const          {return curFPS;}
    inline DWORD NumLights() const          {return nLights;}
    inline DWORD NumPolys() const           {return totalPolys;}
    inline DWORD NumVisibleAreas() const    {return nPVSs;}

    inline void ResetTime() {TrackTimeRestart(TickID);}

    BOOL InEditor() const {return bEditor;}

    static BOOL ConvertResourceName(CTSTR lpName, CTSTR lpDir, String &str, BOOL bOverridable=TRUE);

    static void GetGameModules(List<CTSTR> &ModuleList);

    static BOOL LoadGameModule(CTSTR lpModule, BOOL bInit=TRUE);
    static BOOL IsModuleLoaded(CTSTR lpModule);
    static void UnloadGameModule(CTSTR lpModule);
    static void UnloadAllModules();

    static int SetNativeDataLoader(NATIVELOADERCALLBACK loader);

    //<Script module="Base" classdecs="Engine">
    Declare_Internal_Member(native_GetCurFPS);
    Declare_Internal_Member(native_NumPolys);
    Declare_Internal_Member(native_NumLights);
    Declare_Internal_Member(native_InEditor);
    //</Script>
};

BASE_EXPORT extern Engine *engine;


inline BOOL LoadGameModule(CTSTR lpModule)           {return Engine::LoadGameModule(lpModule);}
inline BOOL IsModuleLoaded(CTSTR lpModule)           {return Engine::IsModuleLoaded(lpModule);}
inline void UnloadGameModule(CTSTR lpModule)         {Engine::UnloadGameModule(lpModule);}
inline void UnloadAllModules()                       {Engine::UnloadAllModules();}


//<Script module="Base" globaldecs="Engine.xscript">
Declare_Native_Global(NativeGlobal_GetEngine);
//</Script>


#endif

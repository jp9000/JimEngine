/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Game.h:  Base Game class

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


#ifndef GAME_HEADER
#define GAME_HEADER


class BASE_EXPORT Game : public Object
{
    DeclareClass(Game, Object);

public:
    Game();
    virtual ~Game();

    void Destroy();

    inline void SetGameSpeed(float speed)               {GameSpeed = speed;}
    inline float GetGameSpeed() const                   {return GameSpeed;}

    inline BOOL Pause(BOOL bPause)                      {BOOL bPreviouslyPaused = bGamePaused; bGamePaused = bPause; return bPreviouslyPaused;}
    inline BOOL IsPaused()                              {return bGamePaused;}

    inline void SetMainViewport(Viewport *newViewport)  {mainViewport = newViewport;}
    inline Viewport* GetMainViewport() const            {return mainViewport;}

    BOOL Save(CTSTR lpFile);
    static BOOL Load(CTSTR lpFile);

    //<Script module="Base" classdecs="Game">
    void scriptSerializeGameData(SerializerObject* s)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetObject(0, (Object*)s);

        GetLocalClass()->CallScriptMember(this, 7, cs);
    }

    Declare_Internal_Member(native_SetGameSpeed);
    Declare_Internal_Member(native_GetGameSpeed);
    Declare_Internal_Member(native_Pause);
    Declare_Internal_Member(native_IsPaused);
    Declare_Internal_Member(native_Save);
    Declare_Internal_Member(native_GetMainViewport);
    Declare_Internal_Member(native_SetMainViewport);
    //</Script>

protected:
    float GameSpeed;
    BOOL bGamePaused;

    virtual void SerializeGameData(Serializer &s);

    Viewport *mainViewport;

private:
    void SerializeGameObjects(Serializer &s);
};

//<Script module="Base" globaldecs="Game.xscript">
Declare_Native_Global(NativeGlobal_UnloadGame);
Declare_Native_Global(NativeGlobal_LoadGame);
Declare_Native_Global(NativeGlobal_CurrentGame);
//</Script>


inline BOOL ENGINEAPI LoadGame(CTSTR lpFile) {return Game::Load(lpFile);}


BASE_EXPORT extern Game *CurrentGame;


#endif

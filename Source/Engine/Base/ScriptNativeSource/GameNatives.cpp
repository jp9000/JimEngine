/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Game

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


#include "..\Base.h"


//<Script module="Base" filedefs="Game.xscript">
void Game::native_SetGameSpeed(CallStruct &cs)
{
    float speed = cs.GetFloat(0);

    SetGameSpeed(speed);
}

void Game::native_GetGameSpeed(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetGameSpeed();
}

void Game::native_Pause(CallStruct &cs)
{
    BOOL bPause = (BOOL)cs.GetInt(0);

    Pause(bPause);
}

void Game::native_IsPaused(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = IsPaused();
}

void Game::native_Save(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String strFile = cs.GetString(0);

    returnVal = Save(strFile);
}

void Game::native_GetMainViewport(CallStruct &cs)
{
    Viewport*& returnVal = (Viewport*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetMainViewport();
}

void Game::native_SetMainViewport(CallStruct &cs)
{
    Viewport* viewport = (Viewport*)cs.GetObject(0);

    SetMainViewport(viewport);
}

void ENGINEAPI NativeGlobal_UnloadGame(CallStruct &cs)
{
    DestroyObject(CurrentGame);
}

void ENGINEAPI NativeGlobal_LoadGame(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String strFile = cs.GetString(0);

    returnVal = LoadGame(strFile);
}

void ENGINEAPI NativeGlobal_CurrentGame(CallStruct &cs)
{
    Game*& returnVal = (Game*&)cs.GetObjectOut(RETURNVAL);

    returnVal = CurrentGame;
}
//</Script>

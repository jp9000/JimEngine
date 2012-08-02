/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Game.cpp:  Base Game class

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



DefineClass(Game);


Game::Game()
: GameSpeed(1.0f)
{
    traceIn(Game::Game);

    if(CurrentGame) DestroyObject(CurrentGame);
    CurrentGame = this;

    traceOut;
}

Game::~Game()
{
    CurrentGame = NULL;
}

void Game::Destroy()
{
    Super::Destroy();

    DestroyObject(mainViewport);
}

BOOL Game::Save(CTSTR lpFile)
{
    traceIn(Game::Save);

    XFileOutputSerializer saveData;

    String strSaveLocation = AppConfig->GetString(TEXT("Game"), TEXT("SaveLocation"), TEXT("saves/"));

    strSaveLocation.FindReplace(TEXT("\\"), TEXT("/"));

    if(strSaveLocation.Right(1)[0] != '/')
        strSaveLocation.AppendChar('/');

    if(!OSFileExists(strSaveLocation.Left(strSaveLocation.Length()-1)))
    {
        String curDir;
        int numDirs = strSaveLocation.NumTokens('/')-1;
        for(int i=0; i<numDirs; i++)
        {
            curDir += strSaveLocation.GetToken(i, '/');
            OSCreateDirectory(curDir);
            curDir += TEXT("/");
        }
    }

    if(!saveData.Open(strSaveLocation + lpFile, XFILE_CREATEALWAYS))
    {
        AppWarning(TEXT("Could not create save file '%s'"), lpFile);
        return FALSE;
    }

    saveData << level->GetLevelName();
    saveData << String(GetObjectClass()->GetName());

    SerializeGameObjects(saveData);
    SerializeGameData(saveData);

    saveData << Level::entityIDCounter;

    saveData.Close();

    return TRUE;

    traceOut;
}

BOOL Game::Load(CTSTR lpFile)
{
    traceIn(Game::Load);

    assert(lpFile && *lpFile);

    XFileInputSerializer loadData;

    String strSaveLocation = AppConfig->GetString(TEXT("Game"), TEXT("SaveLocation"), TEXT("saves/"));

    if(!loadData.Open(strSaveLocation + lpFile))
    {
        AppWarning(TEXT("Could not load save file '%s'"), lpFile);
        return FALSE;
    }

    //-----------------------------------------------

    String strLevelName;
    loadData << strLevelName;

    DestroyObject(CurrentGame);

    if(!level || !level->GetLevelName().CompareI(strLevelName))
        LoadLevel(strLevelName);

    //-----------------------------------------------

    String strGameClass;
    loadData << strGameClass;

    Entity *ent = Entity::LastEntity();
    while(ent)
    {
        Entity *prevEnt = ent->PrevEntity();

        if(!ent->UserCreatedObjectType)
            DestroyObject(ent);

        ent = prevEnt;
    }

    Class* gameClass = FindClass(strGameClass);
    if(gameClass)
        gameClass->Create();
    else
        AppWarning(TEXT("Could not find game class '%s' while loading save file '%s'"), strGameClass, lpFile);

    //-----------------------------------------------

    CurrentGame->SerializeGameObjects(loadData);
    CurrentGame->SerializeGameData(loadData);

    //-----------------------------------------------

    int lastIDCounter;
    loadData << lastIDCounter;
    
    if(lastIDCounter > Level::entityIDCounter)
        Level::entityIDCounter = lastIDCounter;

    loadData.Close();

    return TRUE;

    traceOut;
}

void Game::SerializeGameObjects(Serializer &s)
{
    traceIn(Game::SerializeGameObjects);

    assert(level);
    if(!level)
        return;

    int i;

    int numRemoved, numChanged;

    if(s.IsLoading())
    {
        //remove any user-placed objects that are supposed to be removed
        s << numRemoved;
        for(i=0; i<numRemoved; i++)
        {
            UINT entID;
            s << entID;

            Entity *ent = FindEntityByID(entID);
            DestroyObject(ent);
        }

        //serialize any objects that have changed
        s << numChanged;
        for(i=0; i<numChanged; i++)
        {
            UINT entID;
            s << entID;

            Entity *ent = FindEntityByID(entID);
            if(ent)
            {
                ent->UserCreatedObjectType = 0; //very slight hack.  See Entity::Destroy().
                DestroyObject(ent);
            }

            ent = Entity::LoadEntity(s);

            int skipPos;
            s << skipPos;

            if(!ent)
                s.Seek(skipPos);
            else
                ent->SerializeGameData(s);
        }
    }
    else //saving
    {
        numRemoved = level->DestroyedUserObjects.Num();
        s << numRemoved;

        for(i=0; i<numRemoved; i++)
        {
            UINT entID = level->DestroyedUserObjects[i];
            s << entID;
        }

        int saveSpot = s.GetPos();

        numChanged = 0;
        s << numChanged;

        Entity *ent = Entity::FirstEntity();
        while(ent)
        {
            if(ent->IsSavable())
            {
                UINT entID = ent->GetEntityID();
                s << entID;

                ++numChanged;

                Entity::SaveEntity(s, ent);

                int offsetPos = 0;
                int savePos = s.GetPos();
                s << offsetPos;

                ent->SerializeGameData(s);

                offsetPos = s.GetPos();
                s.Seek(savePos, SERIALIZE_SEEK_START);
                s << offsetPos;
                s.Seek(0, SERIALIZE_SEEK_END);
            }

            ent = ent->NextEntity();
        }

        s.Seek(saveSpot, SERIALIZE_SEEK_START);
        s << numChanged;
        s.Seek(0, SERIALIZE_SEEK_END);
    }

    traceOut;
}

void Game::SerializeGameData(Serializer &s)
{
    SerializerObject *sObj = CreateObjectParam2(SerializerObject, &s, FALSE);
    scriptSerializeGameData(sObj);
    DestroyObject(sObj);
}

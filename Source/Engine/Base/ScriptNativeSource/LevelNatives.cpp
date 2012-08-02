/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  GameModule

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


//<Script module="Base" filedefs="Level.xscript">
void Level::native_GetEntityPath(CallStruct &cs)
{
    AIPath*& returnVal = (AIPath*&)cs.GetObjectOut(RETURNVAL);
    Entity* ent = (Entity*)cs.GetObject(0);
    const Vect &targetPos = (const Vect&)cs.GetStruct(1);

    returnVal = GetEntityPath(ent, targetPos);
}

void Level::native_GetCurrentCamera(CallStruct &cs)
{
    Camera*& returnVal = (Camera*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetCurrentCamera();
}

void ENGINEAPI NativeGlobal_CreateBareLevel(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String strLevelClass = cs.GetString(0);

    Class *levelClass = FindClass(strLevelClass);
    if(levelClass->_IsOf(GetClass(Level)))
    {
        if(level)
            DestroyObject(level);

        level = (Level*)levelClass->Create();
        returnVal = (level != NULL);
    }
    else
        returnVal = FALSE;
}

void ENGINEAPI NativeGlobal_LoadLevel(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String levelPath = cs.GetString(0);

    returnVal = LoadLevel(levelPath);
}

void ENGINEAPI NativeGlobal_GetLevel(CallStruct &cs)
{
    Level*& returnVal = (Level*&)cs.GetObjectOut(RETURNVAL);

    returnVal = level;
}
//</Script>

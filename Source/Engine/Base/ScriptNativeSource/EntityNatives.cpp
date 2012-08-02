/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Entity

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

//<Script module="Base" filedefs="Entity.xscript">
void ENGINEAPI Entity::native_FindByClass(CallStruct &cs)
{
    Entity*& returnVal = (Entity*&)cs.GetObjectOut(RETURNVAL);
    String name = cs.GetString(0);
    Entity* prevEnt = (Entity*)cs.GetObject(1);
    Entity* parent = (Entity*)cs.GetObject(2);

    returnVal = FindByClass(name, prevEnt, parent);
}

void ENGINEAPI Entity::native_FindByName(CallStruct &cs)
{
    Entity*& returnVal = (Entity*&)cs.GetObjectOut(RETURNVAL);
    String name = cs.GetString(0);
    Entity* parent = (Entity*)cs.GetObject(1);

    returnVal = FindByName(name, parent);
}

void ENGINEAPI Entity::native_FindByID(CallStruct &cs)
{
    Entity*& returnVal = (Entity*&)cs.GetObjectOut(RETURNVAL);
    int id = cs.GetInt(0);
    Entity* parent = (Entity*)cs.GetObject(1);

    returnVal = FindByID(id, parent);
}

void ENGINEAPI Entity::native_FindAllByClass(CallStruct &cs)
{
    String name = cs.GetString(0);
    List<Entity*> &ents = (List<Entity*>&)cs.GetListOut(1);
    Entity* parent = (Entity*)cs.GetObject(2);

    FindAllByClass(name, ents, parent);
}

void Entity::native_SetPos(CallStruct &cs)
{
    const Vect &pos = (const Vect&)cs.GetStruct(0);

    SetPos(pos);
}

void Entity::native_SetRot(CallStruct &cs)
{
    const Quat &rot = (const Quat&)cs.GetStruct(0);

    SetRot(rot);
}

void Entity::native_GetLocalPos(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetLocalPos();
}

void Entity::native_GetLocalRot(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = GetLocalRot();
}

void Entity::native_GetWorldPos(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetWorldPos();
}

void Entity::native_GetWorldRot(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = GetWorldRot();
}

void Entity::native_GetEntityTransform(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);

    returnVal = GetEntityTransform();
}

void Entity::native_GetEntityInvTransform(CallStruct &cs)
{
    Matrix& returnVal = (Matrix&)cs.GetStructOut(RETURNVAL);

    returnVal = GetEntityInvTransform();
}

void Entity::native_Attach(CallStruct &cs)
{
    Entity* new_parent = (Entity*)cs.GetObject(0);

    Attach(new_parent);
}

void Entity::native_Detach(CallStruct &cs)
{
    Detach();
}

void Entity::native_NumChildren(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = NumChildren();
}

void Entity::native_GetChild(CallStruct &cs)
{
    Entity*& returnVal = (Entity*&)cs.GetObjectOut(RETURNVAL);
    int child = cs.GetInt(0);

    returnVal = GetChild(child);
}

void Entity::native_GetParent(CallStruct &cs)
{
    Entity*& returnVal = (Entity*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetParent();
}

void Entity::native_GetLineCollision(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &vStart = (const Vect&)cs.GetStruct(0);
    const Vect &vEnd = (const Vect&)cs.GetStruct(1);
    PhyCollisionInfo &info = (PhyCollisionInfo&)cs.GetStructOut(2);

    returnVal = GetLineCollision(vStart, vEnd, &info);
}

void Entity::native_GetRayCollision(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &vStart = (const Vect&)cs.GetStruct(0);
    const Vect &vEnd = (const Vect&)cs.GetStruct(1);
    PhyCollisionInfo &info = (PhyCollisionInfo&)cs.GetStructOut(2);
    float rayLength = cs.GetFloat(3);

    returnVal = GetRayCollision(vStart, vEnd, &info, rayLength);
}

void Entity::native_GetConvexCollision(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    PhyShape* shape = (PhyShape*)cs.GetObject(0);
    const Vect &vStart = (const Vect&)cs.GetStruct(1);
    const Vect &vEnd = (const Vect&)cs.GetStruct(2);
    PhyCollisionInfo &info = (PhyCollisionInfo&)cs.GetStructOut(3);

    returnVal = GetConvexCollision(shape, vStart, vEnd, &info);
}

void Entity::native_GetName(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);

    returnVal = GetName();
}

void Entity::native_DuplicateEntity(CallStruct &cs)
{
    Entity*& returnVal = (Entity*&)cs.GetObjectOut(RETURNVAL);

    returnVal = DuplicateEntity();
}

void Entity::native_SetAsSavable(CallStruct &cs)
{
    BOOL bSavable = (BOOL)cs.GetInt(0);

    SetAsSavable(bSavable);
}

void Entity::native_IsSavable(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = IsSavable();
}

void Entity::native_GetEntityID(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = GetEntityID();
}

void Entity::native_SetState(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    String newState = cs.GetString(0);

    returnVal = SetState(newState);
}

void Entity::native_GetState(CallStruct &cs)
{
    String& returnVal = cs.GetStringOut(RETURNVAL);

    returnVal = GetState();
}

void Entity::native_UpdatePositionalData(CallStruct &cs)
{
    UpdatePositionalData();
}
//</Script>

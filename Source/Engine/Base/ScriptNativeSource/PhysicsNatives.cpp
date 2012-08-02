/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Physics

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


//<Script module="Base" filedefs="Physics.xscript">
void PhySphere::native_GetRadius(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetRadius();
}

void PhyBox::native_GetHalfExtents(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetHalfExtents();
}

void PhyCylinder::native_GetHalfHeight(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetHalfHeight();
}

void PhyCylinder::native_GetRadius(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetRadius();
}

void PhyCylinder::native_GetAxis(CallStruct &cs)
{
    PhyAxis& returnVal = (PhyAxis&)cs.GetIntOut(RETURNVAL);

    returnVal = GetAxis();
}

void PhyCapsule::native_GetHalfHeight(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetHalfHeight();
}

void PhyCapsule::native_GetRadius(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetRadius();
}

void PhyCapsule::native_GetAxis(CallStruct &cs)
{
    PhyAxis& returnVal = (PhyAxis&)cs.GetIntOut(RETURNVAL);

    returnVal = GetAxis();
}

void PhyCone::native_GetHeight(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetHeight();
}

void PhyCone::native_GetRadius(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetRadius();
}

void PhyCone::native_GetAxis(CallStruct &cs)
{
    PhyAxis& returnVal = (PhyAxis&)cs.GetIntOut(RETURNVAL);

    returnVal = GetAxis();
}

void PhyCompound::native_AddShape(CallStruct &cs)
{
    PhyShape* shape = (PhyShape*)cs.GetObject(0);
    const Vect &pos = (const Vect&)cs.GetStruct(1);
    const Quat &rot = (const Quat&)cs.GetStruct(2);

    AddShape(shape, pos, rot);
}

void PhyObject::native_SetPos(CallStruct &cs)
{
    const Vect &pos = (const Vect&)cs.GetStruct(0);

    SetPos(pos);
}

void PhyObject::native_SetRot(CallStruct &cs)
{
    const Quat &rot = (const Quat&)cs.GetStruct(0);

    SetRot(rot);
}

void PhyObject::native_GetPos(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetPos();
}

void PhyObject::native_GetRot(CallStruct &cs)
{
    Quat& returnVal = (Quat&)cs.GetStructOut(RETURNVAL);

    returnVal = GetRot();
}

void PhyObject::native_GetCurrentTransform(CallStruct &cs)
{
    Vect &pos = (Vect&)cs.GetStructOut(0);
    Quat &rot = (Quat&)cs.GetStructOut(1);

    GetCurrentTransform(pos, rot);
}

void PhyObject::native_SetShape(CallStruct &cs)
{
    PhyShape* shape = (PhyShape*)cs.GetObject(0);

    SetShape(shape);
}

void PhyObject::native_GetShape(CallStruct &cs)
{
    PhyShape*& returnVal = (PhyShape*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetShape();
}

void PhyObject::native_SetEntityOwner(CallStruct &cs)
{
    Entity* ent = (Entity*)cs.GetObject(0);

    SetEntityOwner(ent);
}

void PhyObject::native_GetEntityOwner(CallStruct &cs)
{
    Entity*& returnVal = (Entity*&)cs.GetObjectOut(RETURNVAL);

    returnVal = GetEntityOwner();
}

void PhyObject::native_SetFilter(CallStruct &cs)
{
    int collideGroups = cs.GetInt(0);
    int collideMask = cs.GetInt(1);

    SetFilter(collideGroups, collideMask);
}

void PhyObject::native_GetLineCollision(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &p1 = (const Vect&)cs.GetStruct(0);
    const Vect &p2 = (const Vect&)cs.GetStruct(1);
    PhyCollisionInfo &collisionInfo = (PhyCollisionInfo&)cs.GetStructOut(2);

    returnVal = GetLineCollision(p1, p2, &collisionInfo);
}

void PhyObject::native_GetRayCollision(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &rayOrigin = (const Vect&)cs.GetStruct(0);
    const Vect &rayDirection = (const Vect&)cs.GetStruct(1);
    PhyCollisionInfo &collisionInfo = (PhyCollisionInfo&)cs.GetStructOut(2);
    float rayLength = cs.GetFloat(3);

    returnVal = GetRayCollision(rayOrigin, rayDirection, &collisionInfo, rayLength);
}

void PhyObject::native_GetConvexCollision(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    PhyShape* shape = (PhyShape*)cs.GetObject(0);
    const Vect &p1 = (const Vect&)cs.GetStruct(1);
    const Vect &p2 = (const Vect&)cs.GetStruct(2);
    PhyCollisionInfo &collisionInfo = (PhyCollisionInfo&)cs.GetStructOut(3);

    returnVal = GetConvexCollision(shape, p1, p2, &collisionInfo);
}

void PhyObject::native_Activate(CallStruct &cs)
{
    Activate();
}

void PhyObject::native_Deactivate(CallStruct &cs)
{
    Deactivate();
}

void PhyObject::native_IsActive(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = IsActive();
}

void PhyObject::native_EnableCollisionCallback(CallStruct &cs)
{
    BOOL bEnable = (BOOL)cs.GetInt(0);

    EnableCollisionCallback(bEnable);
}

void PhyObject::native_ApplyImpulse(CallStruct &cs)
{
    const Vect &impulse = (const Vect&)cs.GetStruct(0);

    ApplyImpulse(impulse);
}

void PhyObject::native_ApplyRelativeImpulse(CallStruct &cs)
{
    const Vect &relativePos = (const Vect&)cs.GetStruct(0);
    const Vect &impulse = (const Vect&)cs.GetStruct(1);

    ApplyRelativeImpulse(relativePos, impulse);
}

void PhyObject::native_SetVelocity(CallStruct &cs)
{
    const Vect &linearVelocity = (const Vect&)cs.GetStruct(0);

    SetVelocity(linearVelocity);
}

void PhyObject::native_GetVelocity(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetVelocity();
}

void PhyObject::native_GetBounds(CallStruct &cs)
{
    Bounds& returnVal = (Bounds&)cs.GetStructOut(RETURNVAL);

    returnVal = GetBounds();
}

void PhyGhost::native_UpdatePositionalData(CallStruct &cs)
{
    UpdatePositionalData();
}

void PhyGhost::native_NumOverlappingObjects(CallStruct &cs)
{
    int& returnVal = cs.GetIntOut(RETURNVAL);

    returnVal = NumOverlappingObjects();
}

void PhyGhost::native_GetOverlappingObject(CallStruct &cs)
{
    PhyObject*& returnVal = (PhyObject*&)cs.GetObjectOut(RETURNVAL);
    int index = cs.GetInt(0);

    returnVal = GetOverlappingObject(index);
}

void PhyRigid::native_GetType(CallStruct &cs)
{
    RigidType& returnVal = (RigidType&)cs.GetIntOut(RETURNVAL);

    returnVal = GetType();
}

void PhyRigid::native_MakeStatic(CallStruct &cs)
{
    MakeStatic();
}

void PhyRigid::native_MakeKinematic(CallStruct &cs)
{
    MakeKinematic();
}

void PhyRigid::native_MakeDynamic(CallStruct &cs)
{
    float mass = cs.GetFloat(0);

    MakeDynamic(mass);
}

void PhyRigid::native_SetRestitution(CallStruct &cs)
{
    float restitution = cs.GetFloat(0);

    SetRestitution(restitution);
}

void PhyRigid::native_GetRestitution(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetRestitution();
}

void PhyRigid::native_SetFriction(CallStruct &cs)
{
    float friction = cs.GetFloat(0);

    SetFriction(friction);
}

void PhyRigid::native_GetFriction(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetFriction();
}

void PhyCharacter::native_SetMoveDirection(CallStruct &cs)
{
    const Vect &nextPos = (const Vect&)cs.GetStruct(0);

    SetMoveDirection(nextPos);
}

void PhyCharacter::native_GetMoveDirection(CallStruct &cs)
{
    Vect& returnVal = (Vect&)cs.GetStructOut(RETURNVAL);

    returnVal = GetMoveDirection();
}

void PhyCharacter::native_IsFalling(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = IsFalling();
}

void PhyCharacter::native_Jump(CallStruct &cs)
{
    float speed = cs.GetFloat(0);

    Jump(speed);
}

void PhyCharacter::native_IsMoving(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);

    returnVal = IsMoving();
}

void PhyCharacter::native_SetFriction(CallStruct &cs)
{
    float friction = cs.GetFloat(0);

    SetFriction(friction);
}

void PhyCharacter::native_GetFriction(CallStruct &cs)
{
    float& returnVal = cs.GetFloatOut(RETURNVAL);

    returnVal = GetFriction();
}

void PhyCharacter::native_Stop(CallStruct &cs)
{
    Stop();
}

void PhysicsSystem::native_MakeSphere(CallStruct &cs)
{
    PhySphere*& returnVal = (PhySphere*&)cs.GetObjectOut(RETURNVAL);
    float radius = cs.GetFloat(0);

    returnVal = MakeSphere(radius);
}

void PhysicsSystem::native_MakeBox(CallStruct &cs)
{
    PhyBox*& returnVal = (PhyBox*&)cs.GetObjectOut(RETURNVAL);
    const Vect &halfExtents = (const Vect&)cs.GetStruct(0);

    returnVal = MakeBox(halfExtents);
}

void PhysicsSystem::native_MakeBox_2(CallStruct &cs)
{
    PhyBox*& returnVal = (PhyBox*&)cs.GetObjectOut(RETURNVAL);
    float halfX = cs.GetFloat(0);
    float halfY = cs.GetFloat(1);
    float halfZ = cs.GetFloat(2);

    returnVal = MakeBox(halfX, halfY, halfZ);
}

void PhysicsSystem::native_MakeCylinder(CallStruct &cs)
{
    PhyCylinder*& returnVal = (PhyCylinder*&)cs.GetObjectOut(RETURNVAL);
    float halfHeight = cs.GetFloat(0);
    float radius = cs.GetFloat(1);
    PhyAxis axis = (PhyAxis)cs.GetInt(2);

    returnVal = MakeCylinder(halfHeight, radius, axis);
}

void PhysicsSystem::native_MakeCapsule(CallStruct &cs)
{
    PhyCapsule*& returnVal = (PhyCapsule*&)cs.GetObjectOut(RETURNVAL);
    float halfHeight = cs.GetFloat(0);
    float radius = cs.GetFloat(1);
    PhyAxis axis = (PhyAxis)cs.GetInt(2);

    returnVal = MakeCapsule(halfHeight, radius, axis);
}

void PhysicsSystem::native_MakeCone(CallStruct &cs)
{
    PhyCone*& returnVal = (PhyCone*&)cs.GetObjectOut(RETURNVAL);
    float height = cs.GetFloat(0);
    float radius = cs.GetFloat(1);
    PhyAxis axis = (PhyAxis)cs.GetInt(2);

    returnVal = MakeCone(height, radius, axis);
}

void PhysicsSystem::native_MakeCompound(CallStruct &cs)
{
    PhyCompound*& returnVal = (PhyCompound*&)cs.GetObjectOut(RETURNVAL);

    returnVal = MakeCompound();
}

void PhysicsSystem::native_CreateStaticObject(CallStruct &cs)
{
    PhyRigid*& returnVal = (PhyRigid*&)cs.GetObjectOut(RETURNVAL);
    PhyShape* shape = (PhyShape*)cs.GetObject(0);
    const Vect &pos = (const Vect&)cs.GetStruct(1);
    const Quat &rot = (const Quat&)cs.GetStruct(2);
    int collideGroups = cs.GetInt(3);
    int collideMask = cs.GetInt(4);

    returnVal = CreateStaticObject(shape, pos, rot, collideGroups, collideMask);
}

void PhysicsSystem::native_CreateKinematicObject(CallStruct &cs)
{
    PhyRigid*& returnVal = (PhyRigid*&)cs.GetObjectOut(RETURNVAL);
    PhyShape* shape = (PhyShape*)cs.GetObject(0);
    const Vect &pos = (const Vect&)cs.GetStruct(1);
    const Quat &rot = (const Quat&)cs.GetStruct(2);
    int collideGroups = cs.GetInt(3);
    int collideMask = cs.GetInt(4);

    returnVal = CreateKinematicObject(shape, pos, rot, collideGroups, collideMask);
}

void PhysicsSystem::native_CreateDynamicObject(CallStruct &cs)
{
    PhyRigid*& returnVal = (PhyRigid*&)cs.GetObjectOut(RETURNVAL);
    PhyShape* shape = (PhyShape*)cs.GetObject(0);
    const Vect &pos = (const Vect&)cs.GetStruct(1);
    const Quat &rot = (const Quat&)cs.GetStruct(2);
    float mass = cs.GetFloat(3);
    int collideGroups = cs.GetInt(4);
    int collideMask = cs.GetInt(5);

    returnVal = CreateDynamicObject(shape, pos, rot, mass, collideGroups, collideMask);
}

void PhysicsSystem::native_CreateCharacterObject(CallStruct &cs)
{
    PhyCharacter*& returnVal = (PhyCharacter*&)cs.GetObjectOut(RETURNVAL);
    PhyShape* shape = (PhyShape*)cs.GetObject(0);
    const Vect &pos = (const Vect&)cs.GetStruct(1);
    const Quat &rot = (const Quat&)cs.GetStruct(2);
    float mass = cs.GetFloat(3);
    int collideGroups = cs.GetInt(4);
    int collideMask = cs.GetInt(5);

    returnVal = CreateCharacterObject(shape, pos, rot, mass, collideGroups, collideMask);
}

void PhysicsSystem::native_CreateGhost(CallStruct &cs)
{
    PhyGhost*& returnVal = (PhyGhost*&)cs.GetObjectOut(RETURNVAL);
    int collideGroups = cs.GetInt(0);
    int collideMask = cs.GetInt(1);

    returnVal = CreateGhost(collideGroups, collideMask);
}

void PhysicsSystem::native_GetLineCollision(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &p1 = (const Vect&)cs.GetStruct(0);
    const Vect &p2 = (const Vect&)cs.GetStruct(1);
    PhyCollisionInfo &collisionInfo = (PhyCollisionInfo&)cs.GetStructOut(2);
    int filter = cs.GetInt(3);

    returnVal = GetLineCollision(p1, p2, &collisionInfo, filter);
}

void PhysicsSystem::native_GetRayCollision(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    const Vect &rayOrigin = (const Vect&)cs.GetStruct(0);
    const Vect &rayDirection = (const Vect&)cs.GetStruct(1);
    PhyCollisionInfo &collisionInfo = (PhyCollisionInfo&)cs.GetStructOut(2);
    int filter = cs.GetInt(3);
    float rayLength = cs.GetFloat(4);

    returnVal = GetRayCollision(rayOrigin, rayDirection, &collisionInfo, filter, rayLength);
}

void PhysicsSystem::native_GetConvexCollision(CallStruct &cs)
{
    BOOL& returnVal = (BOOL&)cs.GetIntOut(RETURNVAL);
    PhyShape* shape = (PhyShape*)cs.GetObject(0);
    const Vect &p1 = (const Vect&)cs.GetStruct(1);
    const Vect &p2 = (const Vect&)cs.GetStruct(2);
    PhyCollisionInfo &collisionInfo = (PhyCollisionInfo&)cs.GetStructOut(3);
    int filter = cs.GetInt(4);

    returnVal = GetConvexCollision(shape, p1, p2, &collisionInfo, filter);
}

void ENGINEAPI NativeGlobal_Physics(CallStruct &cs)
{
    PhysicsSystem*& returnVal = (PhysicsSystem*&)cs.GetObjectOut(RETURNVAL);

    returnVal = physics;
}
//</Script>

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


#ifndef PHYSICS_HEADER
#define PHYSICS_HEADER


class PhyObject;

enum
{
    PHY_GHOST      = (1<<11),
    PHY_KINEMATIC  = (1<<12),
    PHY_DYNAMIC    = (1<<13),
    PHY_STATIC     = (1<<14),
    PHY_ALL        = -1
};

enum PhyAxis
{
    PhyAxis_X,
    PhyAxis_Y,
    PhyAxis_Z
};

struct PhyCollisionInfo
{
    PhyCollisionInfo() {zero(this, sizeof(PhyCollisionInfo));}

    PhyObject *hitObj;

    Vect hitPos;
    Vect hitNorm;

    inline Plane HitPlane() {return Plane(hitNorm, hitNorm.Dot(hitPos));}
};


/*==========================================================================
  Shapes
===========================================================================*/

class BASE_EXPORT PhyShape : public Object
{
    DeclareClass(PhyShape, Object);

protected:
    LPVOID data;

    static PhyShape *firstShape;
    PhyShape *nextShape, *prevShape;

public:
    static inline PhyShape* FirstShape() {return firstShape;}
    inline PhyShape* PrevShape() const {return prevShape;}
    inline PhyShape* NextShape() const {return nextShape;}

    virtual float GetNormalOffset(const Vect &norm) const=0;
};

class BASE_EXPORT PhySphere : public PhyShape
{
    DeclareClass(PhySphere, PhyShape);

public:
    virtual float GetRadius() const=0;

    //<Script module="Base" classdecs="PhySphere">
    Declare_Internal_Member(native_GetRadius);
    //</Script>
};

class BASE_EXPORT PhyBox : public PhyShape
{
    DeclareClass(PhyBox, PhyShape);

public:
    virtual Vect GetHalfExtents() const=0;

    //<Script module="Base" classdecs="PhyBox">
    Declare_Internal_Member(native_GetHalfExtents);
    //</Script>
};

class BASE_EXPORT PhyCylinder : public PhyShape
{
    DeclareClass(PhyCylinder, PhyShape);

public:
    virtual float GetRadius() const=0;
    virtual float GetHalfHeight() const=0;
    virtual PhyAxis GetAxis() const=0;

    //<Script module="Base" classdecs="PhyCylinder">
    Declare_Internal_Member(native_GetHalfHeight);
    Declare_Internal_Member(native_GetRadius);
    Declare_Internal_Member(native_GetAxis);
    //</Script>
};

class BASE_EXPORT PhyCapsule : public PhyShape
{
    DeclareClass(PhyCapsule, PhyShape);

public:
    virtual float GetRadius() const=0;
    virtual float GetHalfHeight() const=0;
    virtual PhyAxis GetAxis() const=0;

    //<Script module="Base" classdecs="PhyCapsule">
    Declare_Internal_Member(native_GetHalfHeight);
    Declare_Internal_Member(native_GetRadius);
    Declare_Internal_Member(native_GetAxis);
    //</Script>
};

class BASE_EXPORT PhyCone : public PhyShape
{
    DeclareClass(PhyCone, PhyShape);

public:
    virtual float GetRadius() const=0;
    virtual float GetHeight() const=0;
    virtual PhyAxis GetAxis() const=0;

    //<Script module="Base" classdecs="PhyCone">
    Declare_Internal_Member(native_GetHeight);
    Declare_Internal_Member(native_GetRadius);
    Declare_Internal_Member(native_GetAxis);
    //</Script>
};

class BASE_EXPORT PhyCompound : public PhyShape
{
    DeclareClass(PhyCompound, PhyShape);

public:
    ///Note: this will automatically delete the shape object
    virtual void AddShape(PhyShape *shape, const Vect &pos, const Quat &rot)=0;

    //<Script module="Base" classdecs="PhyCompound">
    Declare_Internal_Member(native_AddShape);
    //</Script>
};

class BASE_EXPORT PhyStaticMesh : public PhyShape
{
    DeclareClass(PhyStaticMesh, PhyShape);
};

class BASE_EXPORT PhyDynamicMesh : public PhyShape
{
    DeclareClass(PhyDynamicMesh, PhyShape);
};

/*==========================================================================
  Physics Objects
===========================================================================*/

enum PhyOwner
{
    PhyOwner_None,
    PhyOwner_Entity,
    PhyOwner_Brush
};

class BASE_EXPORT PhyObject : public Object
{
    DeclareClass(PhyObject, Object);

protected:
    LPVOID data;

    union {Entity *ent; Brush  *brush;};

    static PhyObject *firstPhyObject;
    PhyObject *nextPhyObject, *prevPhyObject;

    PhyOwner ownerType;
    BOOL bCallbackEnabled;

public:
    static inline PhyObject* FirstPhyObject() {return firstPhyObject;}
    inline PhyObject* PrevPhyObject() const {return prevPhyObject;}
    inline PhyObject* NextPhyObject() const {return nextPhyObject;}

    virtual void SetPos(const Vect &pos)=0;
    virtual void SetRot(const Quat &rot)=0;

    virtual Vect GetPos() const=0;
    virtual Quat GetRot() const=0;
    virtual void GetCurrentTransform(Vect &pos, Quat &rot) const=0;

    virtual void SetShape(PhyShape *shape)=0;
    virtual PhyShape* GetShape() const=0;

    inline void     SetEntityOwner(Entity *curEnt) {ent = curEnt; ownerType = PhyOwner_Entity;}
    inline Entity*  GetEntityOwner() const {return ownerType == PhyOwner_Entity ? ent : NULL;}

    inline void     SetBrushOwner(Brush *curBrush) {brush = curBrush; ownerType = PhyOwner_Brush;}
    inline Brush*   GetBrushOwner() const {return ownerType == PhyOwner_Brush ? brush : NULL;}

    inline PhyOwner GetOwnerType() const {return ownerType;}

    ///Sets the collision group this object belongs to and collision mask of groups it can collide with
    virtual void SetFilter(short collideGroups, short collideMask)=0;

    virtual BOOL GetLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo=NULL) const=0;
    virtual BOOL GetConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo=NULL) const=0;

    inline  BOOL GetRayCollision(const Vect &rayPos, const Vect &rayDir, PhyCollisionInfo *collisionInfo=NULL, float rayLength=1000.0f) const
    {
        Vect endPos = rayPos+(rayDir*rayLength);
        return GetLineCollision(rayPos, endPos, collisionInfo);
    }

    virtual void    Activate()=0;
    virtual void    Deactivate()=0;
    virtual BOOL    IsActive()=0;

    virtual void    EnableCollisionCallback(BOOL bEnable) {bCallbackEnabled = bEnable;}

    virtual void    ApplyImpulse(const Vect &impulse)=0;
    virtual void    ApplyRelativeImpulse(const Vect &relativePos, const Vect &impulse)=0;

    virtual void    SetVelocity(const Vect &linearVelocity)=0;
    virtual Vect    GetVelocity() const=0;

    virtual Bounds  GetBounds() const=0;

    //<Script module="Base" classdecs="PhyObject">
    Declare_Internal_Member(native_SetPos);
    Declare_Internal_Member(native_SetRot);
    Declare_Internal_Member(native_GetPos);
    Declare_Internal_Member(native_GetRot);
    Declare_Internal_Member(native_GetCurrentTransform);
    Declare_Internal_Member(native_SetShape);
    Declare_Internal_Member(native_GetShape);
    Declare_Internal_Member(native_SetEntityOwner);
    Declare_Internal_Member(native_GetEntityOwner);
    Declare_Internal_Member(native_SetFilter);
    Declare_Internal_Member(native_GetLineCollision);
    Declare_Internal_Member(native_GetRayCollision);
    Declare_Internal_Member(native_GetConvexCollision);
    Declare_Internal_Member(native_Activate);
    Declare_Internal_Member(native_Deactivate);
    Declare_Internal_Member(native_IsActive);
    Declare_Internal_Member(native_EnableCollisionCallback);
    Declare_Internal_Member(native_ApplyImpulse);
    Declare_Internal_Member(native_ApplyRelativeImpulse);
    Declare_Internal_Member(native_SetVelocity);
    Declare_Internal_Member(native_GetVelocity);
    Declare_Internal_Member(native_GetBounds);
    //</Script>
};

//a ghost object is basically a list of objects that are inside a certain area defined by a shape
class BASE_EXPORT PhyGhost : public PhyObject
{
    DeclareClass(PhyGhost, PhyObject);

public:
    virtual void UpdatePositionalData()=0;

    virtual UINT NumOverlappingObjects() const=0;
    virtual PhyObject* GetOverlappingObject(UINT index) const=0;

    virtual BOOL GetAreaLineObjects(const Vect &p1, const Vect &p2, List<PhyObject*> &Objects, short filter=PHY_ALL) const=0;
    virtual BOOL GetAreaLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo=NULL, short filter=PHY_ALL) const=0;
    virtual BOOL GetAreaConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo=NULL, short filter=PHY_ALL) const=0;

    inline  BOOL GetAreaRayObjects(const Vect &rayPos, const Vect &rayDir, List<PhyObject*> &Objects, short filter=PHY_ALL, float rayLength=1000.0f) const
    {
        Vect endPos = rayPos+(rayDir*rayLength);
        return GetAreaLineObjects(rayPos, endPos, Objects, filter);
    }

    inline  BOOL GetAreaRayCollision(const Vect &rayPos, const Vect &rayDir, PhyCollisionInfo *collisionInfo=NULL, short filter=PHY_ALL, float rayLength=1000.0f) const
    {
        Vect endPos = rayPos+(rayDir*rayLength);
        return GetAreaLineCollision(rayPos, endPos, collisionInfo, filter);
    }

    virtual void ApplyRelativeImpulse(const Vect &relativePos, const Vect &impulse) {AppWarning(TEXT("Cannot apply force to a ghost object"));}
    virtual void ApplyImpulse(const Vect& impulse)       {AppWarning(TEXT("Cannot apply force to a ghost object"));}
    virtual void SetVelocity(const Vect &linearVelocity) {AppWarning(TEXT("Cannot use velocity with a ghost object"));}
    virtual Vect GetVelocity() const                     {AppWarning(TEXT("Cannot use velocity with a ghost object")); return Vect(0.0f, 0.0f, 0.0f);}

    //<Script module="Base" classdecs="PhyGhost">
    Declare_Internal_Member(native_UpdatePositionalData);
    Declare_Internal_Member(native_NumOverlappingObjects);
    Declare_Internal_Member(native_GetOverlappingObject);
    //</Script>
};

enum RigidType
{
    RigidType_Static,
    RigidType_Kinematic,
    RigidType_Dynamic
};

class BASE_EXPORT PhyRigid : public PhyObject
{
    DeclareClass(PhyRigid, PhyObject);

public:
    virtual RigidType GetType() const=0;
    virtual void MakeStatic()=0;
    virtual void MakeKinematic()=0;
    virtual void MakeDynamic(float mass)=0;

    //restitution is the 'bounce'
    virtual void  SetRestitution(float restitution)=0;
    virtual float GetRestitution() const=0;

    virtual void  SetFriction(float friction)=0;
    virtual float GetFriction() const=0;

    //<Script module="Base" classdecs="PhyRigid">
    Declare_Internal_Member(native_GetType);
    Declare_Internal_Member(native_MakeStatic);
    Declare_Internal_Member(native_MakeKinematic);
    Declare_Internal_Member(native_MakeDynamic);
    Declare_Internal_Member(native_SetRestitution);
    Declare_Internal_Member(native_GetRestitution);
    Declare_Internal_Member(native_SetFriction);
    Declare_Internal_Member(native_GetFriction);
    //</Script>
};


//the character class is a physics objects which essentially is made to "walk" on a surface rather than just dynamically collide with it.
class BASE_EXPORT PhyCharacter : public PhyObject
{
    DeclareClass(PhyCharacter, PhyObject);

public:
    virtual void SetMoveDirection(const Vect &nextPos)=0;
    virtual Vect GetMoveDirection() const=0;
    virtual BOOL IsFalling() const=0;
    virtual void Jump(float speed)=0;
    virtual BOOL IsMoving() const=0;

    virtual void  SetFriction(float friction)=0;
    virtual float GetFriction() const=0;

    inline void Stop() {SetMoveDirection(Vect::Zero());}

    //<Script module="Base" classdecs="PhyCharacter">
    Declare_Internal_Member(native_SetMoveDirection);
    Declare_Internal_Member(native_GetMoveDirection);
    Declare_Internal_Member(native_IsFalling);
    Declare_Internal_Member(native_Jump);
    Declare_Internal_Member(native_IsMoving);
    Declare_Internal_Member(native_SetFriction);
    Declare_Internal_Member(native_GetFriction);
    Declare_Internal_Member(native_Stop);
    //</Script>
};

/*class BASE_EXPORT PhySoftObject : public PhyObject
{
    DeclareClass(PhySoftObject, PhyObject);

public:
};*/


/*==========================================================================
  Physics Constraints
===========================================================================*/

class BASE_EXPORT PhyConstraint : public Object
{
    DeclareClass(PhyConstraint, Object);

protected:
    LPVOID data;

    static PhyConstraint *firstConstraint;
    PhyConstraint *nextConstraint, *prevConstraint;

public:
    static inline PhyConstraint* FirstConstraint() {return firstConstraint;}
    inline PhyConstraint* PrevConstraint() const {return prevConstraint;}
    inline PhyConstraint* NextConstraint() const {return nextConstraint;}
};


/*==========================================================================
  Physics System
===========================================================================*/

class BASE_EXPORT PhysicsSystem : public FrameObject
{
    DeclareClass(PhysicsSystem, FrameObject);

protected:
    void SetEntTransform(Entity *ent, const Vect &pos, const Quat &rot);

public:
    virtual void DestroyAllObjects()=0;

    virtual void UpdatePhysics(float fTime)=0;
    virtual void UpdateWorldData()=0;

    virtual PhySphere*          MakeSphere(float radius)=0;
    virtual PhyBox*             MakeBox(const Vect &halfExtents)=0;
    virtual PhyCylinder*        MakeCylinder(float halfHeight, float radius, PhyAxis axis=PhyAxis_Y)=0;
    virtual PhyCapsule*         MakeCapsule(float halfHeight, float radius, PhyAxis axis=PhyAxis_Y)=0;
    virtual PhyCone*            MakeCone(float height, float radius, PhyAxis axis=PhyAxis_Y)=0;
    virtual PhyCompound*        MakeCompound()=0;
    virtual PhyStaticMesh*      MakeStaticMeshShape(Vect *verts, UINT numVerts, UINT *indices, UINT numIndices)=0;
    virtual PhyDynamicMesh*     MakeDynamicMeshShape(Vect *verts, UINT numVerts, UINT *indices, UINT numIndices)=0;

    inline  PhyBox*             MakeBox(float halfX, float halfY, float halfZ) {return MakeBox(Vect(halfX, halfY, halfZ));}

    ///A static object is simply used for collision and does not move.
    virtual PhyRigid* CreateStaticObject(PhyShape *shape, const Vect &pos, const Quat &rot, short collideGroups=PHY_STATIC, short collideMask=PHY_ALL)=0;
    ///A kinematic object is an object which can influence the world, but does not have any dynamic response to collision.
    virtual PhyRigid* CreateKinematicObject(PhyShape *shape, const Vect &pos, const Quat &rot, short collideGroups=PHY_KINEMATIC, short collideMask=PHY_ALL)=0;
    ///A kinematic character object with the ability to walk around the world
    virtual PhyCharacter* CreateCharacterObject(PhyShape *shape, const Vect &pos, const Quat &rot, float mass, short collideGroups=PHY_DYNAMIC, short collideMask=PHY_ALL)=0;
    ///A dynamic object is an object which fully collides with and responds to collision from other objects.
    virtual PhyRigid* CreateDynamicObject(PhyShape *shape, const Vect &pos, const Quat &rot, float mass, short collideGroups=PHY_DYNAMIC, short collideMask=PHY_ALL)=0;
    ///A ghost object is essentially a list of objects within an area.
    virtual PhyGhost* CreateGhost(short collideGroups=PHY_GHOST, short collideMask=PHY_ALL)=0;

    virtual BOOL GetLineObjects(const Vect &p1, const Vect &p2, List<PhyObject*> &Objects, short filter=PHY_ALL) const=0;
    virtual BOOL GetLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo=NULL, short filter=PHY_ALL) const=0;
    virtual BOOL GetConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo=NULL, short filter=PHY_ALL) const=0;

    inline  BOOL GetRayObjects(const Vect &rayPos, const Vect &rayDir, List<PhyObject*> &Objects, short filter=PHY_ALL, float rayLength=1000.0f) const
    {
        Vect endPos = rayPos+(rayDir*rayLength);
        return GetLineObjects(rayPos, endPos, Objects, filter);
    }

    inline  BOOL GetRayCollision(const Vect &rayPos, const Vect &rayDir, PhyCollisionInfo *collisionInfo=NULL, short filter=PHY_ALL, float rayLength=1000.0f) const
    {
        Vect endPos = rayPos+(rayDir*rayLength);
        return GetLineCollision(rayPos, endPos, collisionInfo, filter);
    }

    //<Script module="Base" classdecs="PhysicsSystem">
    Declare_Internal_Member(native_MakeSphere);
    Declare_Internal_Member(native_MakeBox);
    Declare_Internal_Member(native_MakeBox_2);
    Declare_Internal_Member(native_MakeCylinder);
    Declare_Internal_Member(native_MakeCapsule);
    Declare_Internal_Member(native_MakeCone);
    Declare_Internal_Member(native_MakeCompound);
    Declare_Internal_Member(native_CreateStaticObject);
    Declare_Internal_Member(native_CreateKinematicObject);
    Declare_Internal_Member(native_CreateDynamicObject);
    Declare_Internal_Member(native_CreateCharacterObject);
    Declare_Internal_Member(native_CreateGhost);
    Declare_Internal_Member(native_GetLineCollision);
    Declare_Internal_Member(native_GetRayCollision);
    Declare_Internal_Member(native_GetConvexCollision);
    //</Script>
};

//<Script module="Base" globaldecs="Physics.xscript">
Declare_Native_Global(NativeGlobal_Physics);
//</Script>


BASE_EXPORT extern PhysicsSystem *physics;


#endif

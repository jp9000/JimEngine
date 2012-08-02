/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  BulletPhysics

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


#ifndef BULLETPHYSICS_HEADER
#define BULLETPHYSICS_HEADER

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <BulletMultiThreaded/SpuGatheringCollisionDispatcher.h>
#include <BulletMultiThreaded/PlatformDefinitions.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btSoftBody.h>
//#include <BulletSoftBody/btSoftBodyHelpers.h>


class BulletPhysics;


/*inline const btVector3& VectToBT(const Vect &v)    {return *reinterpret_cast<const btVector3*>(&v);}
inline const Vect& BTToVect(const btVector3 &v)    {return *reinterpret_cast<const Vect*>(&v);}

inline const btQuaternion& QuatToBT(const Quat &q) {return *reinterpret_cast<const btQuaternion*>(&q);}
inline const Quat& BTToQuat(const btQuaternion &q) {return *reinterpret_cast<const Quat*>(&q);}*/

inline btVector3 VectToBT(const Vect &v)    {return btVector3(v.x, v.y, v.z);}
inline Vect BTToVect(const btVector3 &v)    {return Vect(v.getX(), v.getY(), v.getZ());}

inline btQuaternion QuatToBT(const Quat &q) {return btQuaternion(q.x, q.y, q.z, q.w);}
inline Quat BTToQuat(const btQuaternion &q) {return Quat(q.x(), q.y(), q.z(), q.w());}


struct EngineMotionState;


#define LinkedListAdd(cls, firstobj, prevobj, nextobj) \
    if(firstobj) \
    { \
        nextobj = firstobj; \
        prevobj = NULL; \
        static_cast<cls*>(firstobj)->prevobj = this; \
    } \
    else \
        prevobj = nextobj = NULL; \
    firstobj = this;


#define LinkedListRemove(cls, firstobj, prevobj, nextobj) \
    if(!nextobj && !prevobj) \
        firstobj = NULL; \
    else \
    { \
        if(nextobj) static_cast<cls*>(nextobj)->prevobj = prevobj; \
        if(prevobj) \
            static_cast<cls*>(prevobj)->nextobj = nextobj; \
        else \
            firstobj = nextobj; \
    }

#define LinkedListAddShape(cls) LinkedListAdd(cls, firstShape, prevShape, nextShape)
#define LinkedListRemoveShape(cls) LinkedListRemove(cls, firstShape, prevShape, nextShape)

#define LinkedListAddPhyObj(cls) LinkedListAdd(cls, firstPhyObject, prevPhyObject, nextPhyObject)
#define LinkedListRemovePhyObj(cls) LinkedListRemove(cls, firstPhyObject, prevPhyObject, nextPhyObject)

#define LinkedListAddConstraint(cls) LinkedListAdd(cls, firstConstraint, prevPConstraint, nextConstraint)
#define LinkedListRemoveConstraint(cls) LinkedListRemove(cls, firstConstraint, prevConstraint, nextConstraint)



/*==========================================================================
  Bullet Shapes
===========================================================================*/

class BulletShape : public PhyShape
{
public:
    inline btCollisionShape* GetShape() const {return static_cast<btCollisionShape*>(data);}
};

inline btCollisionShape* GetBTShape(PhyShape *shape) {return static_cast<BulletShape*>(shape)->GetShape();}


class BulletSphere : public PhySphere
{
    friend class BulletPhysics;
    DeclareClass(BulletSphere, PhySphere);

    BulletPhysics *system;

public:
    BulletSphere() {}
    BulletSphere(BulletPhysics *bullet, float radius);

    ~BulletSphere();

    inline btSphereShape* GetShape() const {return static_cast<btSphereShape*>(data);}

    float GetRadius() const   {return GetShape()->getRadius();}

    virtual float GetNormalOffset(const Vect &norm) const;
};

class BulletBox : public PhyBox
{
    friend class BulletPhysics;
    DeclareClass(BulletBox, PhyBox);

    BulletPhysics *system;

public:
    BulletBox() {}
    BulletBox(BulletPhysics *bullet, const Vect &halfExtents);
    
    ~BulletBox();

    inline btBoxShape* GetShape() const {return static_cast<btBoxShape*>(data);}

    Vect GetHalfExtents() const
    {
        return BTToVect(GetShape()->getHalfExtentsWithoutMargin());
    }

    virtual float GetNormalOffset(const Vect &norm) const;
};

class BulletCylinder : public PhyCylinder
{
    friend class BulletPhysics;
    DeclareClass(BulletCylinder, PhyCylinder);

    BulletPhysics *system;

    PhyAxis axis;

public:
    BulletCylinder() {}
    BulletCylinder(BulletPhysics *bullet, float radius, float halfHeight, PhyAxis axisIn);

    ~BulletCylinder();

    inline btCylinderShape* GetShape() const {return static_cast<btCylinderShape*>(data);}

    virtual float GetRadius() const   {return GetShape()->getRadius();}

    virtual float GetHalfHeight() const
    {
        btVector3 extents = GetShape()->getHalfExtentsWithMargin();
        switch(axis)
        {
            case PhyAxis_X:
                return extents.getX();
            case PhyAxis_Y:
                return extents.getY();
            case PhyAxis_Z:
                return extents.getZ();
        }

        return 0.0f;
    }

    virtual PhyAxis GetAxis() const {return axis;}

    virtual float GetNormalOffset(const Vect &norm) const;
};

class BulletCapsule : public PhyCapsule
{
    friend class BulletPhysics;
    DeclareClass(BulletCapsule, PhyCapsule);

    BulletPhysics *system;

    PhyAxis axis;

public:
    BulletCapsule() {}
    BulletCapsule(BulletPhysics *bullet, float radius, float halfHeight, PhyAxis axisIn);

    ~BulletCapsule();

    inline btCapsuleShape* GetShape() const   {return static_cast<btCapsuleShape*>(data);}

    virtual float GetRadius() const           {return GetShape()->getRadius();}
    virtual float GetHalfHeight() const       {return GetShape()->getHalfHeight();}
    virtual PhyAxis GetAxis() const           {return axis;}

    virtual float GetNormalOffset(const Vect &norm) const;
};

class BulletCone : public PhyCone
{
    friend class BulletPhysics;
    DeclareClass(BulletCone, PhyCone);

    BulletPhysics *system;

    PhyAxis axis;

public:
    BulletCone() {}
    BulletCone(BulletPhysics *bullet, float radius, float height, PhyAxis axisIn);

    ~BulletCone();

    inline btConeShape* GetShape() const      {return static_cast<btConeShape*>(data);}

    virtual float GetRadius() const           {return GetShape()->getRadius();}
    virtual float GetHeight() const           {return GetShape()->getHeight();}
    virtual PhyAxis GetAxis() const           {return axis;}

    virtual float GetNormalOffset(const Vect &norm) const;
};

class BulletCompound : public PhyCompound
{
    friend class BulletPhysics;
    DeclareClass(BulletCompound, PhyCompound);

    BulletPhysics *system;

    List<PhyShape*> Shapes;

public:
    BulletCompound() {}
    BulletCompound(BulletPhysics *bullet);

    ~BulletCompound();

    inline btCompoundShape* GetShape() const  {return static_cast<btCompoundShape*>(data);}

    virtual void AddShape(PhyShape *shape, const Vect &pos, const Quat &rot);

    //todo: implement this?  screw it, I'm way too lazy
    virtual float GetNormalOffset(const Vect &norm) const {return 0.0f;}
};


class BulletMesh : public PhyStaticMesh
{
    friend class BulletPhysics;
    DeclareClass(BulletMesh, PhyStaticMesh);

    btTriangleIndexVertexArray *meshData;
    Vect *vertList;
    UINT *indexList;

    BulletPhysics *system;

public:
    BulletMesh() {}
    BulletMesh(BulletPhysics *bullet, Vect *verts, UINT numVerts, UINT *indices, UINT numIndices);

    ~BulletMesh();

    inline Vect* GetVertList() const  {return vertList;}
    inline UINT* GetIndexList() const {return indexList;}

    inline btBvhTriangleMeshShape* GetShape() const {return static_cast<btBvhTriangleMeshShape*>(data);}

    virtual float GetNormalOffset(const Vect &norm) const;
};

class BulletDynamicMesh : public PhyDynamicMesh
{
    friend class BulletPhysics;
    DeclareClass(BulletDynamicMesh, PhyDynamicMesh);

    btTriangleIndexVertexArray *meshData;

    BulletPhysics *system;

public:
    BulletDynamicMesh() {}
    BulletDynamicMesh(BulletPhysics *bullet, Vect *verts, UINT numVerts, UINT *indices, UINT numIndices);

    ~BulletDynamicMesh();

    inline btGImpactMeshShape* GetShape() const {return static_cast<btGImpactMeshShape*>(data);}

    virtual float GetNormalOffset(const Vect &norm) const;
};


/*==========================================================================
  Bullet Objects
===========================================================================*/

class BulletObject : public PhyObject
{
    friend class BulletPhysics;
public:
    inline btCollisionObject* GetObj() const {return static_cast<btCollisionObject*>(data);}

    inline BOOL CallbacksEnabled() const {return bCallbackEnabled;}
};

inline btCollisionObject* GetBTObject(PhyObject *obj) {return static_cast<BulletObject*>(obj)->GetObj();}


class BulletGhost : public PhyGhost
{
    friend class BulletPhysics;
    DeclareClass(BulletGhost, PhyGhost);

    BulletPhysics *system;
    PhyShape *curShape;

    short ghostGroups, ghostMask;

public:
    BulletGhost() {}
    BulletGhost(BulletPhysics *bullet, short collideGroups, short collideMask);

    ~BulletGhost();

    inline btGhostObject* GetObj() const {return static_cast<btGhostObject*>(data);}

    virtual void SetPos(const Vect &pos);
    virtual void SetRot(const Quat &rot);
    virtual Vect GetPos() const;
    virtual Quat GetRot() const;
    virtual void GetCurrentTransform(Vect &pos, Quat &rot) const;

    virtual void UpdatePositionalData();

    virtual void SetShape(PhyShape *shape);
    virtual PhyShape* GetShape() const {return curShape;}

    virtual UINT NumOverlappingObjects() const {return (UINT)GetObj()->getNumOverlappingObjects();}
    virtual PhyObject* GetOverlappingObject(UINT index) const;

    virtual BOOL GetAreaLineObjects(const Vect &p1, const Vect &p2, List<PhyObject*> &Objects, short filter) const;
    virtual BOOL GetAreaLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo, short filter) const;
    virtual BOOL GetAreaConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo, short filter) const;

    virtual void EnableCollisionCallback(BOOL bEnable);
    virtual void SetFilter(short collideGroups, short collideMask);

    virtual Bounds GetBounds() const;

    virtual void Activate() {}
    virtual void Deactivate() {}
    virtual BOOL IsActive() {return TRUE;}

    virtual BOOL GetLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const;
    virtual BOOL GetConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const;
};

class BulletRigid : public PhyRigid
{
    friend class BulletPhysics;
    DeclareClass(BulletRigid, PhyRigid);

    BulletPhysics *system;
    EngineMotionState* motionState;

    PhyShape *curShape;
    RigidType type;

public:
    BulletRigid() {}
    BulletRigid(BulletPhysics *bullet, PhyShape *shape, const Vect &pos, const Quat &rot, float mass, short collideGroups, short collideMask);

    ~BulletRigid();

    inline btRigidBody* GetObj() const {return static_cast<btRigidBody*>(data);}

    virtual void SetShape(PhyShape *shape);
    virtual PhyShape* GetShape() const {return curShape;}

    virtual void SetFilter(short collideGroups, short collideMask);

    virtual void SetPos(const Vect &pos);
    virtual void SetRot(const Quat &rot);
    virtual Vect GetPos() const;
    virtual Quat GetRot() const;
    virtual void GetCurrentTransform(Vect &pos, Quat &rot) const;

    virtual BOOL GetLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const;
    virtual BOOL GetConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const;

    virtual void ApplyRelativeImpulse(const Vect &relativePos, const Vect &impulse);
    virtual void ApplyImpulse(const Vect& impulse);

    virtual void SetVelocity(const Vect &linearVelocity);
    virtual Vect GetVelocity() const;

    virtual Bounds GetBounds() const;

    virtual void EnableCollisionCallback(BOOL bEnable);

    virtual RigidType GetType() const {return type;}
    virtual void MakeStatic();
    virtual void MakeKinematic();
    virtual void MakeDynamic(float mass);

    virtual void  SetRestitution(float restitution);
    virtual float GetRestitution() const;

    virtual void  SetFriction(float friction);
    virtual float GetFriction() const;

    virtual void Activate();
    virtual void Deactivate();
    virtual BOOL IsActive();
};


/*==========================================================================
  Bullet Character
===========================================================================*/

class BulletCharacter;

class CharacterActionHandler : public btActionInterface
{
public:
    CharacterActionHandler(btCollisionWorld *collisionWorld, BulletCharacter *character);
    ~CharacterActionHandler();

    virtual void updateAction(btCollisionWorld *collisionWorld, btScalar deltaTimeStep);

    virtual void debugDraw(btIDebugDraw* debugDrawer) {}

    void Jump(float speed);

    BOOL BlockedPos(const Vect &pos);
    BOOL PentrationRecovery(const Vect &v1, const Vect &v2);

    inline BOOL IsMoving() const {return movement != Vect::Zero();}

    btRigidBody *body;
    btPairCachingGhostObject *ghost;
    Plane curPlane;
    BOOL bFalling;
    Vect movement;
    float maxSlope, stepHeight, curTime;
    btCollisionWorld *world;

};

class BASE_EXPORT BulletCharacter : public PhyCharacter
{
    DeclareClass(BulletCharacter, PhyCharacter);

    BulletPhysics *system;
    EngineMotionState* motionState;

    PhyShape *curShape;

    CharacterActionHandler *handler;

public:
    BulletCharacter() {}
    BulletCharacter(BulletPhysics *bullet, PhyShape *shape, const Vect &pos, const Quat &rot, float mass, short collideGroups, short collideMask);

    ~BulletCharacter();

    inline btRigidBody* GetObj() const {return static_cast<btRigidBody*>(data);}

    virtual void SetMoveDirection(const Vect &movement) {handler->movement = movement;}
    virtual Vect GetMoveDirection() const {return handler->movement;}
    virtual BOOL IsFalling() const {return handler->bFalling;}
    virtual void Jump(float speed) {handler->Jump(speed);}
    virtual BOOL IsMoving() const {return handler->IsMoving();}

    //--------------------------------

    virtual void      SetPos(const Vect &pos);
    virtual void      SetRot(const Quat &rot);

    virtual Vect      GetPos() const;
    virtual Quat      GetRot() const;
    virtual void      GetCurrentTransform(Vect &pos, Quat &rot) const;

    virtual void      SetShape(PhyShape *shape);
    virtual PhyShape* GetShape() const {return curShape;}

    virtual void      SetFilter(short collideGroups, short collideMask);

    virtual BOOL      GetLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo=NULL) const;
    virtual BOOL      GetConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo=NULL) const;

    virtual void      Activate();
    virtual void      Deactivate();
    virtual BOOL      IsActive();

    virtual void      ApplyImpulse(const Vect &impulse);
    virtual void      ApplyRelativeImpulse(const Vect &relativePos, const Vect &impulse);

    virtual void      SetVelocity(const Vect &linearVelocity);
    virtual Vect      GetVelocity() const;

    virtual Bounds    GetBounds() const;

    virtual void      SetFriction(float friction);
    virtual float     GetFriction() const;
};


/*==========================================================================
  Bullet Physics System
===========================================================================*/

const int maxNumOutstandingTasks = 2;

class BulletPhysics : public PhysicsSystem
{
    DeclareClass(BulletPhysics, PhysicsSystem);

public:
    BulletPhysics();
    ~BulletPhysics();

    void UpdatePhysics(float fTime);
    void UpdateWorldData();

    virtual void DestroyAllObjects();

    void InitializeOSDispatcher();
    void InitializeOtherOSStuff();
    void DestroyOSDispatcher();
    void DestroyOtherOSStuff();

    virtual PhySphere*          MakeSphere(float radius);
    virtual PhyBox*             MakeBox(const Vect &halfExtents);
    virtual PhyCylinder*        MakeCylinder(float halfHeight, float radius, PhyAxis axis);
    virtual PhyCapsule*         MakeCapsule(float halfHeight, float radius, PhyAxis axis);
    virtual PhyCone*            MakeCone(float height, float radius, PhyAxis axis);
    virtual PhyCompound*        MakeCompound();
    virtual PhyStaticMesh*      MakeStaticMeshShape(Vect *verts, UINT numVerts, UINT *indices, UINT numIndices);
    virtual PhyDynamicMesh*     MakeDynamicMeshShape(Vect *verts, UINT numVerts, UINT *indices, UINT numIndices);

    virtual PhyRigid*           CreateStaticObject(PhyShape *shape, const Vect &pos, const Quat &rot, short collideGroups, short collideMask);
    virtual PhyRigid*           CreateKinematicObject(PhyShape *shape, const Vect &pos, const Quat &rot, short collideGroups, short collideMask);
    virtual PhyCharacter*       CreateCharacterObject(PhyShape *shape, const Vect &pos, const Quat &rot, float mass, short collideGroups, short collideMask);
    virtual PhyRigid*           CreateDynamicObject(PhyShape *shape, const Vect &pos, const Quat &rot, float mass, short collideGroups, short collideMask);
    virtual PhyGhost*           CreateGhost(short collideGroups, short collideMask);

    virtual BOOL GetLineObjects(const Vect &p1, const Vect &p2, List<PhyObject*> &Objects, short filter) const;
    virtual BOOL GetLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo, short filter) const;
    virtual BOOL GetConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo, short filter) const;

    inline void UpdateEntityTransform(Entity *ent, const Vect &pos, const Quat &rot) {SetEntTransform(ent, pos, rot);}

    BOOL GetLineCollisionSingle(PhyObject *obj, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const;
    BOOL GetConvexCollisionSingle(PhyObject *obj, PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const;

    btDefaultCollisionConfiguration     *configuration;
    btCollisionDispatcher               *dispatcher;
    btBroadphaseInterface               *broadPhase;
    btConstraintSolver                  *solver;
    btDiscreteDynamicsWorld             *world;

    btThreadSupportInterface            *dispatcherThread;

    btGhostPairCallback                 *ghostPairCallback;
};


/*==========================================================================
  Engine Motion State
===========================================================================*/

struct EngineMotionState : btMotionState
{
    EngineMotionState(const btTransform& startTrans, PhyObject *curObj) : transform(startTrans), phyObj(curObj) {}

    void SetTransformNoUpdate(const btTransform &trans)
    {
        transform = trans;
    }

    virtual void getWorldTransform(btTransform& trans) const    {trans = transform;}
    virtual void setWorldTransform(const btTransform& trans);

    btTransform transform;
    PhyObject *phyObj;
};



#endif

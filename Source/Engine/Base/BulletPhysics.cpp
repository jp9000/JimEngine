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


#include "Base.h"
#include "BulletPhysics.h"


DefineClass(BulletSphere);
DefineClass(BulletBox);
DefineClass(BulletCylinder);
DefineClass(BulletCapsule);
DefineClass(BulletCone);
DefineClass(BulletCompound);
DefineClass(BulletMesh);
DefineClass(BulletDynamicMesh);

DefineClass(BulletGhost);
DefineClass(BulletRigid);
DefineClass(BulletCharacter);

DefineClass(BulletPhysics);


void* enginebtAlloc(size_t size);
void  enginebtFree(void* mem);

void* enginebtAlloc(size_t size)    {return Allocate(size);}
void  enginebtFree(void* mem)       {Free(mem);}


bool EngineCollisionCallback(btManifoldPoint& cp,
                             const btCollisionObject* colObj0, int partId0, int index0,
                             const btCollisionObject* colObj1, int partId1, int index1);


struct ConvexNearestCallback : btCollisionWorld::ClosestConvexResultCallback
{
    ConvexNearestCallback(const btVector3& convexFromWorld, const btVector3& convexToWorld, short filterIn)
        : btCollisionWorld::ClosestConvexResultCallback(convexFromWorld, convexToWorld), filter(filterIn)
    {}

    short filter;

    virtual bool needsCollision(btBroadphaseProxy* proxy0) const
    {
        if(filter & proxy0->m_collisionFilterGroup)
            return btCollisionWorld::ClosestConvexResultCallback::needsCollision(proxy0);

        return false;
    }
};


struct RayNearestCallback : btCollisionWorld::ClosestRayResultCallback
{
    RayNearestCallback(const btVector3& rayFromWorld, const btVector3& rayToWorld, short filterIn)
        : btCollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld), filter(filterIn)
    {}

    short filter;

    virtual bool needsCollision(btBroadphaseProxy* proxy0) const
    {
        if(filter & proxy0->m_collisionFilterGroup)
            return btCollisionWorld::ClosestRayResultCallback::needsCollision(proxy0);

        return false;
    }
};

struct RayObjectsCallback : RayNearestCallback
{
    List<PhyObject*> &objects;

	RayObjectsCallback(const btVector3& rayFromWorld, const btVector3& rayToWorld, List<PhyObject*> &objList, short filterIn)
	    : RayNearestCallback(rayFromWorld, rayToWorld, filter), objects(objList)
	{}

    virtual	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult,bool normalInWorldSpace)
    {
        PhyObject *phyObj = static_cast<PhyObject*>(rayResult.m_collisionObject->getUserPointer());
        if(phyObj->IsOf(GetClass(BulletRigid)))
            objects.SafeAdd(static_cast<PhyRigid*>(phyObj));
        return 1.0f;//rayResult.m_hitFraction;
    }
};


void EngineMotionState::setWorldTransform(const btTransform& trans)
{
    Entity *ent = phyObj->GetEntityOwner();
    if(ent)
    {
        btVector3 pos = trans.getOrigin();
        btQuaternion rot = trans.getRotation();
        static_cast<BulletPhysics*>(physics)->UpdateEntityTransform(ent, BTToVect(pos), BTToQuat(rot));
    }
    transform = trans;
}

/*==========================================================================
  BulletSphere
===========================================================================*/

BulletSphere::BulletSphere(BulletPhysics *bullet, float radius)
{
    traceIn(BulletSphere::BulletSphere);

    system = bullet;

    data = (LPVOID)new btSphereShape(radius);
    GetShape()->setUserPointer(this);

    prevShape = NULL;

    LinkedListAddShape(BulletSphere);

    traceOut;
}

BulletSphere::~BulletSphere()
{
    traceIn(BulletSphere::~BulletSphere);

    LinkedListRemoveShape(BulletSphere);

    delete GetShape();

    traceOut;
}

float BulletSphere::GetNormalOffset(const Vect &norm) const
{
    traceIn(BulletSphere::GetNormalOffset);

    return GetRadius()+0.04f;

    traceOut;
}


/*==========================================================================
  BulletBox
===========================================================================*/

BulletBox::BulletBox(BulletPhysics *bullet, const Vect &halfExtents)
{
    traceIn(BulletBox::BulletBox);

    system = bullet;

    data = (LPVOID)new btBoxShape(VectToBT(halfExtents));
    GetShape()->setUserPointer(this);

    LinkedListAddShape(BulletBox);

    traceOut;
}

BulletBox::~BulletBox()
{
    traceIn(BulletBox::~BulletBox);

    LinkedListRemoveShape(BulletBox);

    delete GetShape();

    traceOut;
}

float BulletBox::GetNormalOffset(const Vect &norm) const
{
    traceIn(BulletBox::GetNormalOffset);

    Vect halfExtents = GetHalfExtents();
    return halfExtents.Abs().Dot(norm.GetAbs())+0.04f;

    traceOut;
}


/*==========================================================================
  BulletCylinder
===========================================================================*/

BulletCylinder::BulletCylinder(BulletPhysics *bullet, float radius, float halfHeight, PhyAxis axisIn)
{
    traceIn(BulletCylinder::BulletCylinder);

    system = bullet;

    axis = axisIn;

    btVector3 extents;
    switch(axisIn)
    {
        case PhyAxis_X:
            extents.setValue(halfHeight, radius, radius);
            data = (LPVOID)new btCylinderShapeX(extents);
            break;
        case PhyAxis_Y:
            extents.setValue(radius, halfHeight, radius);
            data = (LPVOID)new btCylinderShape (extents);
            break;
        case PhyAxis_Z:
            extents.setValue(radius, radius, halfHeight);
            data = (LPVOID)new btCylinderShapeZ(extents);
            break;
    }

    GetShape()->setUserPointer(this);

    LinkedListAddShape(BulletCylinder);

    traceOut;
}

BulletCylinder::~BulletCylinder()
{
    traceIn(BulletCylinder::~BulletCylinder);

    LinkedListRemoveShape(BulletCylinder);

    delete GetShape();

    traceOut;
}

float BulletCylinder::GetNormalOffset(const Vect &norm) const
{
    traceIn(BulletCylinder::GetNormalOffset);

    Vect newNorm;
    switch(axis)
    {
        case PhyAxis_X:
            newNorm.Set(norm.y, -norm.x, norm.z); break;
        case PhyAxis_Y:
            newNorm = norm; break;
        case PhyAxis_Z:
            newNorm.Set(norm.x, -norm.z, norm.y);
    }

    return GetPlaneCylinderOffset(newNorm, GetHalfHeight(), GetRadius())+0.04f;

    traceOut;
}


/*==========================================================================
  BulletCapsule
===========================================================================*/

BulletCapsule::BulletCapsule(BulletPhysics *bullet, float radius, float halfHeight, PhyAxis axisIn)
{
    traceIn(BulletCapsule::BulletCapsule);

    system = bullet;

    axis = axisIn;

    switch(axisIn)
    {
        case PhyAxis_X:
            data = (LPVOID)new btCapsuleShapeX(radius, halfHeight);
            break;
        case PhyAxis_Y:
            data = (LPVOID)new btCapsuleShape (radius, halfHeight);
            break;
        case PhyAxis_Z:
            data = (LPVOID)new btCapsuleShapeZ(radius, halfHeight);
            break;
    }

    GetShape()->setUserPointer(this);

    LinkedListAddShape(BulletCapsule);

    traceOut;
}

BulletCapsule::~BulletCapsule()
{
    traceIn(BulletCapsule::~BulletCapsule);

    LinkedListRemoveShape(BulletCapsule);

    delete GetShape();

    traceOut;
}

float BulletCapsule::GetNormalOffset(const Vect &norm) const
{
    traceIn(BulletCapsule::GetNormalOffset);

    Vect newNorm;
    switch(axis)
    {
        case PhyAxis_X:
            newNorm.Set(norm.y, -norm.x, norm.z); break;
        case PhyAxis_Y:
            newNorm = norm; break;
        case PhyAxis_Z:
            newNorm.Set(norm.x, -norm.z, norm.y);
    }

    return GetPlaneCapsuleOffset(newNorm, GetHalfHeight(), GetRadius());

    traceOut;
}


/*==========================================================================
  BulletCone
===========================================================================*/

BulletCone::BulletCone(BulletPhysics *bullet, float radius, float height, PhyAxis axisIn)
{
    traceIn(BulletCone::BulletCone);

    system = bullet;

    axis = axisIn;

    switch(axisIn)
    {
        case PhyAxis_X:
            data = (LPVOID)new btConeShapeX(radius, height);
            break;
        case PhyAxis_Y:
            data = (LPVOID)new btConeShape (radius, height);
            break;
        case PhyAxis_Z:
            data = (LPVOID)new btConeShapeZ(radius, height);
            break;
    }

    GetShape()->setUserPointer(this);

    LinkedListAddShape(BulletCone);

    traceOut;
}

BulletCone::~BulletCone()
{
    traceIn(BulletCone::~BulletCone);

    LinkedListRemoveShape(BulletCone);

    delete GetShape();

    traceOut;
}

float BulletCone::GetNormalOffset(const Vect &norm) const
{
    traceIn(BulletCone::GetNormalOffset);

    Vect newNorm;
    switch(axis)
    {
        case PhyAxis_X:
            newNorm.Set(norm.y, -norm.x, norm.z); break;
        case PhyAxis_Y:
            newNorm = norm; break;
        case PhyAxis_Z:
            newNorm.Set(norm.x, -norm.z, norm.y);
    }

    return GetPlaneConeOffset(newNorm, GetHeight(), GetRadius());

    traceOut;
}

/*==========================================================================
  BulletCone
===========================================================================*/

BulletCompound::BulletCompound(BulletPhysics *bullet)
{
    traceIn(BulletCompound::BulletCompound);

    system = bullet;

    data = (LPVOID)new btCompoundShape;
    GetShape()->setUserPointer(this);

    LinkedListAddShape(BulletCompound);

    traceOut;
}

BulletCompound::~BulletCompound()
{
    traceIn(BulletCompound::BulletCompound);

    LinkedListRemoveShape(BulletCompound);

    delete GetShape();

    for(int i=0; i<Shapes.Num(); i++)
        DestroyObject(Shapes[i]);

    traceOut;
}

void BulletCompound::AddShape(PhyShape *shape, const Vect &pos, const Quat &rot)
{
    traceIn(BulletCompound::AddShape);

    assertmsg(!Shapes.HasValue(shape), TEXT("Compound already contains the shape being added"));
    Shapes << shape;

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(VectToBT(pos));
    transform.setRotation(QuatToBT(rot));

    GetShape()->addChildShape(transform, GetBTShape(shape));

    traceOut;
}


/*==========================================================================
  BulletMesh
===========================================================================*/

BulletMesh::BulletMesh(BulletPhysics *bullet, Vect *verts, UINT numVerts, UINT *indices, UINT numIndices)
{
    traceIn(BulletMesh::BulletMesh);

    system = bullet;

    meshData = new btTriangleIndexVertexArray((int)(numIndices/3), (int*)indices, sizeof(UINT)*3, (int)numVerts, (btScalar*)verts, sizeof(Vect));
    data = (LPVOID)new btBvhTriangleMeshShape(meshData, true);
    GetShape()->setUserPointer(this);

    vertList = verts;
    indexList = indices;

    LinkedListAddShape(BulletMesh);

    traceOut;
}

BulletMesh::~BulletMesh()
{
    traceIn(BulletMesh::~BulletMesh);

    LinkedListRemoveShape(BulletMesh);

    delete GetShape();
    delete meshData;

    traceOut;
}

float BulletMesh::GetNormalOffset(const Vect &norm) const
{
    traceIn(BulletMesh::GetNormalOffset);

    Bounds bounds(BTToVect(GetShape()->getLocalAabbMax()), BTToVect(GetShape()->getLocalAabbMin()));
    return fabs(bounds.MinDistFrom(Plane(norm, 0.0f)));

    traceOut;
}


/*==========================================================================
  BulletDynamicMesh
===========================================================================*/

BulletDynamicMesh::BulletDynamicMesh(BulletPhysics *bullet, Vect *verts, UINT numVerts, UINT *indices, UINT numIndices)
{
    traceIn(BulletDynamicMesh::BulletDynamicMesh);

    system = bullet;

    meshData = new btTriangleIndexVertexArray((int)(numIndices/3), (int*)indices, sizeof(UINT)*3, (int)numVerts, (btScalar*)verts, sizeof(Vect));
    data = (LPVOID)new btGImpactMeshShape(meshData);
    GetShape()->setUserPointer(this);
    GetShape()->updateBound();

    LinkedListAddShape(BulletDynamicMesh);

    traceOut;
}

BulletDynamicMesh::~BulletDynamicMesh()
{
    traceIn(BulletDynamicMesh::~BulletDynamicMesh);

    LinkedListRemoveShape(BulletDynamicMesh);

    delete GetShape();
    delete meshData;

    traceOut;
}

float BulletDynamicMesh::GetNormalOffset(const Vect &norm) const
{
    traceIn(BulletDynamicMesh::GetNormalOffset);

    const btAABB &btbounds = GetShape()->getLocalBox();
    Bounds bounds(BTToVect(btbounds.m_min), BTToVect(btbounds.m_max));
    return fabs(bounds.MinDistFrom(Plane(norm, 0.0f)));

    traceOut;
}


/*==========================================================================
  BulletGhost
===========================================================================*/

BulletGhost::BulletGhost(BulletPhysics *bullet, short collideGroups, short collideMask)
{
    traceIn(BulletGhost::BulletGhost);

    system = bullet;

    ghostGroups = collideGroups;
    ghostMask = collideMask;

    data = (LPVOID)new btGhostObject();
    GetObj()->getWorldTransform().setIdentity();
    GetObj()->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    GetObj()->setUserPointer(this);

    LinkedListAddPhyObj(BulletGhost);

    traceOut;
}

BulletGhost::~BulletGhost()
{
    traceIn(BulletGhost::~BulletGhost);

    if(curShape)
        system->world->removeCollisionObject(GetObj());

    LinkedListRemovePhyObj(BulletGhost);

    delete GetObj();

    traceOut;
}


void BulletGhost::SetPos(const Vect &pos)
{
    traceIn(BulletGhost::SetPos);

    btTransform &transform = GetObj()->getWorldTransform();
    transform.setOrigin(VectToBT(pos));

    traceOut;
}

void BulletGhost::SetRot(const Quat &rot)
{
    traceIn(BulletGhost::SetRot);

    btTransform &transform = GetObj()->getWorldTransform();
    transform.setRotation(QuatToBT(rot));

    traceOut;
}

Vect BulletGhost::GetPos() const
{
    traceIn(BulletGhost::GetPos);

    btTransform &transform = GetObj()->getWorldTransform();
    return BTToVect(transform.getOrigin());

    traceOut;
}

Quat BulletGhost::GetRot() const
{
    traceIn(BulletGhost::GetRot);

    btTransform &transform = GetObj()->getWorldTransform();
    return BTToQuat(transform.getRotation());

    traceOut;
}

void BulletGhost::GetCurrentTransform(Vect &pos, Quat &rot) const
{
    traceIn(BulletGhost::GetCurrentTransform);

    btTransform &transform = GetObj()->getWorldTransform();
    pos = BTToVect(transform.getOrigin());
    rot = BTToQuat(transform.getRotation());

    traceOut;
}

void BulletGhost::UpdatePositionalData()
{
    traceIn(BulletGhost::UpdatePositionalData);

    if(!curShape)
    {
        AppWarning(TEXT("BulletGhost::UpdatePositional called but not associated with a shape"));
        return;
    }
    system->world->removeCollisionObject(GetObj());
    system->world->addCollisionObject(GetObj(), ghostGroups, ghostMask);
    //system->dispatcher->dispatchAllCollisionPairs(GetObj()->getOverlappingPairCache(), system->world->getDispatchInfo(), system->dispatcher);

    traceOut;
}


void BulletGhost::SetShape(PhyShape *shape)
{
    traceIn(BulletGhost::SetShape);

    GetObj()->setCollisionShape(GetBTShape(shape));

    if(!curShape)
        system->world->addCollisionObject(GetObj(), ghostGroups, ghostMask);
    else if(!shape)
        system->world->removeCollisionObject(GetObj());

    curShape = shape;

    traceOut;
}


PhyObject* BulletGhost::GetOverlappingObject(UINT index) const
{
    traceIn(BulletGhost::GetOverlappingObject);

    btCollisionObject *obj = GetObj()->getOverlappingObject((int)index);
    return static_cast<PhyObject*>(obj->getUserPointer());

    traceOut;
}


BOOL BulletGhost::GetAreaLineObjects(const Vect &p1, const Vect &p2, List<PhyObject*> &Objects, short filter) const
{
    traceIn(BulletGhost::GetAreaLineObjects);

    btVector3 btP1 = VectToBT(p1), btP2 = VectToBT(p2);

    RayObjectsCallback rayCallback(btP1, btP2, Objects, filter);

    GetObj()->rayTest(btP1, btP2, rayCallback);

    return rayCallback.hasHit();

    traceOut;
}

BOOL BulletGhost::GetAreaLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo, short filter) const
{
    traceIn(BulletGhost::GetAreaLineCollision);

    btVector3 btP1 = VectToBT(p1), btP2 = VectToBT(p2);

    RayNearestCallback rayCallback(btP1, btP2, filter);

    GetObj()->rayTest(btP1, btP2, rayCallback);

    if(!rayCallback.hasHit())
        return FALSE;

    if(collisionInfo)
    {
        collisionInfo->hitPos  = BTToVect(rayCallback.m_hitPointWorld);
        collisionInfo->hitNorm = BTToVect(rayCallback.m_hitNormalWorld);
        collisionInfo->hitObj  = static_cast<PhyRigid*>(rayCallback.m_collisionObject->getUserPointer());
    }

    return TRUE;

    traceOut;
}

BOOL BulletGhost::GetAreaConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo, short filter) const
{
    traceIn(BulletGhost::GetAreaConvexCollision);

    if(shape->IsOf(GetClass(PhyStaticMesh)))
    {
        AppWarning(TEXT("Cannot use convex collision tests on convex shapes"));
        return FALSE;
    }

    btVector3 btP1 = VectToBT(p1), btP2 = VectToBT(p2);

    btTransform btT1, btT2;
    btT1.setIdentity();
    btT2.setIdentity();

    btT1.setOrigin(btP1);
    btT2.setOrigin(btP2);

    ConvexNearestCallback convexCallback(btP1, btP2, filter);

    GetObj()->convexSweepTest(static_cast<btConvexShape*>(GetBTShape(shape)), btT1, btT2, convexCallback);

    if(!convexCallback.hasHit())
        return FALSE;

    if(collisionInfo)
    {
        collisionInfo->hitPos  = BTToVect(convexCallback.m_hitPointWorld);
        collisionInfo->hitNorm = BTToVect(convexCallback.m_hitNormalWorld);
        collisionInfo->hitObj  = static_cast<PhyRigid*>(convexCallback.m_hitCollisionObject->getUserPointer());
    }

    return TRUE;

    traceOut;
}

BOOL BulletGhost::GetLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const
{
    traceIn(BulletGhost::GetLineCollision);

    return system->GetLineCollisionSingle((PhyObject*)this, p1, p2, collisionInfo);

    traceOut;
}

BOOL BulletGhost::GetConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const
{
    traceIn(BulletGhost::GetConvexCollision);

    return system->GetConvexCollisionSingle((PhyObject*)this, shape, p1, p2, collisionInfo);

    traceOut;
}

void BulletGhost::SetFilter(short collideGroups, short collideMask)
{
    traceIn(BulletGhost::SetFilter);

    ghostGroups = collideGroups;
    ghostMask = collideMask;

    if(curShape)
    {
        system->world->removeCollisionObject(GetObj());
        system->world->addCollisionObject(GetObj(), ghostGroups, ghostMask);
    }

    traceOut;
}

void BulletGhost::EnableCollisionCallback(BOOL bEnable)
{
    traceIn(BulletGhost::EnableCollisionCallback);

    bCallbackEnabled = bEnable;

    if(bEnable)
        GetObj()->setCollisionFlags(GetObj()->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    else
        GetObj()->setCollisionFlags(GetObj()->getCollisionFlags() & ~btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

    traceOut;
}

Bounds BulletGhost::GetBounds() const
{
    traceIn(BulletGhost::GetBounds);

    if(curShape)
    {
        btTransform btT = GetObj()->getWorldTransform();

        btVector3 min, max;
        GetBTShape(curShape)->getAabb(btT, min, max);

        return Bounds(BTToVect(min), BTToVect(max));
    }

    return Bounds(Vect(0.0f, 0.0f, 0.0f), Vect(0.0f, 0.0f, 0.0f));

    traceOut;
}


/*==========================================================================
  BulletRigid
===========================================================================*/

BulletRigid::BulletRigid(BulletPhysics *bullet, PhyShape *shape, const Vect &pos, const Quat &rot, float mass, short collideGroups, short collideMask)
{
    traceIn(BulletRigid::BulletRigid);

    system = bullet;

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(VectToBT(pos));
    transform.setRotation(QuatToBT(rot));

    motionState = new EngineMotionState(transform, this);

    btVector3 intertia(0.0f, 0.0f, 0.0f);
    if(CloseFloat(mass, 0.0f))
        mass = 0.0f;
    else
        GetBTShape(shape)->calculateLocalInertia(mass, intertia);

    curShape = shape;

    btRigidBody::btRigidBodyConstructionInfo rigidInfo(mass, motionState, GetBTShape(shape), intertia);
    data = (LPVOID)new btRigidBody(rigidInfo);

    GetObj()->setUserPointer(this);

    system->world->addRigidBody(GetObj(), collideGroups, collideMask);

    LinkedListAddPhyObj(BulletRigid);

    traceOut;
}

BulletRigid::~BulletRigid()
{
    traceIn(BulletRigid::~BulletRigid);

    SetEntityOwner(NULL);

    LinkedListRemovePhyObj(BulletRigid);

    system->world->removeRigidBody(GetObj());

    delete GetObj();
    delete motionState;

    traceOut;
}


void BulletRigid::SetFilter(short collideGroups, short collideMask)
{
    traceIn(BulletRigid::SetFilter);

    system->world->removeRigidBody(GetObj());
    system->world->addRigidBody(GetObj(), collideGroups, collideMask);

    traceOut;
}


void BulletRigid::SetPos(const Vect &pos)
{
    traceIn(BulletRigid::SetPos);

    btTransform &transform = GetObj()->getWorldTransform();
    transform.setOrigin(VectToBT(pos));
    GetObj()->activate();

    if(type == RigidType_Kinematic || type == RigidType_Static)
        motionState->SetTransformNoUpdate(transform);

    traceOut;
}

void BulletRigid::SetRot(const Quat &rot)
{
    traceIn(BulletRigid::SetRot);

    btTransform &transform = GetObj()->getWorldTransform();
    transform.setRotation(QuatToBT(rot));
    GetObj()->activate();

    traceOut;
}

void BulletRigid::Activate()
{
    traceIn(BulletRigid::Activate);

    GetObj()->activate();

    traceOut;
}

void BulletRigid::Deactivate()
{
    traceIn(BulletRigid::Deactivate);

    GetObj()->setActivationState(WANTS_DEACTIVATION);

    traceOut;
}

BOOL BulletRigid::IsActive()
{
    traceIn(BulletRigid::IsActive);

    return ((GetObj()->getActivationState() != ISLAND_SLEEPING) && (GetObj()->getActivationState() != WANTS_DEACTIVATION));

    traceOut;
}

Vect BulletRigid::GetPos() const
{
    traceIn(BulletRigid::GetPos);

    btTransform transform;
    motionState->getWorldTransform(transform);
    return BTToVect(transform.getOrigin());

    traceOut;
}

Quat BulletRigid::GetRot() const
{
    traceIn(BulletRigid::GetRot);

    btTransform transform;
    motionState->getWorldTransform(transform);
    return BTToQuat(transform.getRotation());

    traceOut;
}

void BulletRigid::GetCurrentTransform(Vect &pos, Quat &rot) const
{
    traceIn(BulletRigid::GetCurrentTransform);

    btTransform transform;
    motionState->getWorldTransform(transform);
    pos = BTToVect(transform.getOrigin());
    rot = BTToQuat(transform.getRotation());

    traceOut;
}

void BulletRigid::MakeStatic()
{
    traceIn(BulletRigid::MakeStatic);

    if(type == RigidType_Kinematic)
    {
        GetObj()->setCollisionFlags(GetObj()->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
        GetObj()->setActivationState(ACTIVE_TAG);
    }

    GetObj()->setMassProps(0.0f, btVector3(0,0,0));

    type = RigidType_Static;

    traceOut;
}

void BulletRigid::MakeKinematic()
{
    traceIn(BulletRigid::MakeKinematic);

    GetObj()->setCollisionFlags(GetObj()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    GetObj()->setActivationState(DISABLE_DEACTIVATION);

    type = RigidType_Kinematic;

    traceOut;
}

void BulletRigid::MakeDynamic(float mass)
{
    traceIn(BulletRigid::MakeDynamic);

    system->world->removeRigidBody(GetObj());

    if(type == RigidType_Kinematic)
    {
        GetObj()->setCollisionFlags(GetObj()->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
        GetObj()->activate();
    }

    type = RigidType_Dynamic;

    btVector3 intertia(0.0f, 0.0f, 0.0f);
    if(CloseFloat(mass, 0.0f))
        mass = 0.0f;
    else
        GetBTShape(curShape)->calculateLocalInertia(mass, intertia);

    GetObj()->setMassProps(mass, intertia);

    system->world->addRigidBody(GetObj());

    traceOut;
}

BOOL BulletRigid::GetLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const
{
    traceIn(BulletRigid::GetLineCollision);

    return system->GetLineCollisionSingle((PhyObject*)this, p1, p2, collisionInfo);

    traceOut;
}

BOOL BulletRigid::GetConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const
{
    traceIn(BulletRigid::GetConvexCollision);

    return system->GetConvexCollisionSingle((PhyObject*)this, shape, p1, p2, collisionInfo);

    traceOut;
}

void BulletRigid::ApplyRelativeImpulse(const Vect &relativePos, const Vect &impulse)
{
    traceIn(BulletRigid::ApplyRelativeImpulse);

    if(type == RigidType_Dynamic)
    {
        GetObj()->applyImpulse(VectToBT(impulse), VectToBT(relativePos));
        GetObj()->activate();
    }

    traceOut;
}

void BulletRigid::ApplyImpulse(const Vect& impulse)
{
    traceIn(BulletRigid::ApplyImpulse);

    if(type == RigidType_Dynamic)
    {
        GetObj()->applyCentralImpulse(VectToBT(impulse));
        GetObj()->activate();
    }

    traceOut;
}


void BulletRigid::SetVelocity(const Vect &linearVelocity)
{
    traceIn(BulletRigid::SetVelocity);

    if(type == RigidType_Dynamic)
    {
        GetObj()->setLinearVelocity(VectToBT(linearVelocity));
        GetObj()->activate();
    }

    traceOut;
}

Vect BulletRigid::GetVelocity() const
{
    traceIn(BulletRigid::GetVelocity);

    if(type == RigidType_Dynamic)
        return BTToVect(GetObj()->getLinearVelocity());
    return Vect(0.0f, 0.0f, 0.0f);

    traceOut;
}


Bounds BulletRigid::GetBounds() const
{
    traceIn(BulletRigid::GetBounds);

    btVector3 min, max;
    GetObj()->getAabb(min, max);

    return Bounds(BTToVect(min), BTToVect(max));

    traceOut;
}

void BulletRigid::SetShape(PhyShape *shape)
{
    traceIn(BulletRigid::SetShape);

    GetObj()->setCollisionShape(GetBTShape(shape));
    GetObj()->activate();

    traceOut;
}

void BulletRigid::EnableCollisionCallback(BOOL bEnable)
{
    traceIn(BulletRigid::EnableCollisionCallback);

    bCallbackEnabled = bEnable;
    if(bEnable)
        GetObj()->setCollisionFlags(GetObj()->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
    else
        GetObj()->setCollisionFlags(GetObj()->getCollisionFlags() & ~btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

    traceOut;
}

void  BulletRigid::SetRestitution(float restitution)
{
    traceIn(BulletRigid::SetRestitution);

    GetObj()->setRestitution(restitution);

    traceOut;
}

float BulletRigid::GetRestitution() const
{
    traceIn(BulletRigid::GetRestitution);

    return GetObj()->getRestitution();

    traceOut;
}

void  BulletRigid::SetFriction(float friction)
{
    traceIn(BulletRigid::SetFriction);

    GetObj()->setFriction(friction);

    traceOut;
}

float BulletRigid::GetFriction() const
{
    traceIn(BulletRigid::GetFriction);

    return GetObj()->getFriction();

    traceOut;
}


/*==========================================================================
  BulletPhysics
===========================================================================*/

bool EngineCollisionCallback(btManifoldPoint& cp,
                             const btCollisionObject* colObj0, int partId0, int index0,
                             const btCollisionObject* colObj1, int partId1, int index1)
{
    if(!colObj0->getUserPointer() || !colObj1->getUserPointer())
        return false;

    const btCollisionObject *btObjs[2] = {colObj0, colObj1};
    const btCollisionObject *btColliders[2] = {colObj1, colObj0};
    const int triIndexes[2] = {index0, index1};

    float frictionVals[2];
    float restitutionVals[2];

    Vect hitPos     = BTToVect(cp.m_positionWorldOnB);
    Vect hitNorm    = BTToVect(cp.m_normalWorldOnB);

    for(int i=0; i<2; i++)
    {
        BulletObject *collider = static_cast<BulletObject*>(btColliders[i]->getUserPointer());
        BulletObject *obj = static_cast<BulletObject*>(btObjs[i]->getUserPointer());

        frictionVals[i] = btObjs[i]->getFriction();
        restitutionVals[i] = btObjs[i]->getRestitution();

        if(obj->CallbacksEnabled())
        {
            switch(obj->GetOwnerType())
            {
                case PhyOwner_Entity:
                    {
                        Entity *ent = obj->GetEntityOwner();
                        if(cp.getAppliedImpulse() > 4.0f && cp.getLifeTime() == 1)
                            ent->OnCollision(collider->GetEntityOwner(), hitPos, hitNorm, cp.getAppliedImpulse());

                        MeshEntity *meshEnt = ObjectCast<MeshEntity>(ent);//dynamic_cast<MeshEntity*>(ent);
                        if(meshEnt && ent->phyShape->IsOf(GetClass(PhyStaticMesh)))
                            meshEnt->MeshCollisionCallback(collider, triIndexes[i], cp.getAppliedImpulse(), cp.getLifeTime(), hitPos, frictionVals[i], restitutionVals[i]);

                        break;
                    }

                case PhyOwner_Brush:
                    {
                        Brush *brush = obj->GetBrushOwner();
                        brush->BrushCollisionCallback(collider, triIndexes[i], cp.getAppliedImpulse(), cp.getLifeTime(), hitPos, frictionVals[i], restitutionVals[i]);
                    }
            }
        }
    }

    cp.m_combinedFriction = frictionVals[0]*frictionVals[1];
    if(cp.m_combinedFriction > 10.0f)
        cp.m_combinedFriction = 10.0f;
    else if(cp.m_combinedFriction < -10.0f)
        cp.m_combinedFriction = -10.0f;

    cp.m_combinedRestitution = restitutionVals[0]*restitutionVals[1];

    return true;
}


BulletPhysics::BulletPhysics()
{
    traceIn(BulletPhysics::BulletPhysics);

    //--------------------------------------------
    // set default allocators
    btAlignedAllocSetCustom(enginebtAlloc, enginebtFree);

    //--------------------------------------------
    // create default bullet configuration (mostly internal bullet stuff I guess)
    configuration = new btSoftBodyRigidBodyCollisionConfiguration();

    //--------------------------------------------
    // create dispatcher (the method by which physics is updated every frame)
    InitializeOSDispatcher();

    //--------------------------------------------
    // create broadphase (determines object interaction algorithm, for example an AABB tree for testing whether objects are interacting)
    broadPhase = new btDbvtBroadphase();
    ghostPairCallback = new btGhostPairCallback();
    broadPhase->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback);

    //--------------------------------------------
    // create solver (determines how constraints are handled)
    solver = new btSequentialImpulseConstraintSolver();

    //--------------------------------------------
    // create physics world
    world = new btSoftRigidDynamicsWorld(dispatcher, broadPhase, solver, configuration);

    InitializeOtherOSStuff();

    world->setGravity(btVector3(0,-10,0));

    btGImpactCollisionAlgorithm::registerAlgorithm(static_cast<btCollisionDispatcher*>(world->getDispatcher()));

    //gContactAddedCallback = EngineCollisionCallback;

    traceOut;
}

BulletPhysics::~BulletPhysics()
{
    traceIn(BulletPhysics::~BulletPhysics);

    DestroyAllObjects();

    DestroyOtherOSStuff();

    delete world;
    delete solver;
    delete broadPhase;
    delete ghostPairCallback;
    DestroyOSDispatcher();
    delete configuration;

    traceOut;
}

void BulletPhysics::UpdatePhysics(float fSeconds)
{
    traceIn(BulletPhysics::UpdatePhysics);
    profileSegment("Physics Update");

    if(fSeconds > 0.5f)
        fSeconds = 0.5f;
    world->stepSimulation(fSeconds, 0);

    /*int numManifolds = world->getDispatcher()->getNumManifolds();
    bool bFound = false;
    for (int i=0;i<numManifolds;i++)
    {
        btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
        btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
        btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());

        for(int j=0; j<contactManifold->getNumContacts(); j++)
        {
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            if((pt.getDistance() < -0.01f) && (pt.getLifeTime() == 1))
                EngineCollisionCallback(pt, obA, pt.m_partId0, pt.m_index0, obB, pt.m_partId1, pt.m_index1);
        }
    }*/

    traceOut;
}

void BulletPhysics::UpdateWorldData()
{
    traceIn(BulletPhysics::UpdateWorldData);

    world->performDiscreteCollisionDetection();

    traceOut;
}


void BulletPhysics::DestroyAllObjects()
{
    traceIn(BulletPhysics::DestroyAllObjects);

    while(PhyConstraint::FirstConstraint())
        DestroyObject(PhyConstraint::FirstConstraint());

    while(PhyObject::FirstPhyObject())
        DestroyObject(PhyObject::FirstPhyObject());

    while(PhyShape::FirstShape())
        DestroyObject(PhyShape::FirstShape());

    traceOut;
}

PhySphere* BulletPhysics::MakeSphere(float radius)
{
    traceIn(BulletPhysics::MakeSphere);

    return CreateObjectParam2(BulletSphere, this, radius);

    traceOut;
}

PhyBox* BulletPhysics::MakeBox(const Vect &halfExtents)
{
    traceIn(BulletPhysics::MakeBox);

    return CreateObjectParam2(BulletBox, this, halfExtents);

    traceOut;
}

PhyCylinder* BulletPhysics::MakeCylinder(float halfHeight, float radius, PhyAxis axis)
{
    traceIn(BulletPhysics::MakeCylinder);

    return CreateObjectParam4(BulletCylinder, this, radius, halfHeight, axis);

    traceOut;
}

PhyCapsule* BulletPhysics::MakeCapsule(float halfHeight, float radius, PhyAxis axis)
{
    traceIn(BulletPhysics::MakeCapsule);

    return CreateObjectParam4(BulletCapsule, this, radius, halfHeight, axis);

    traceOut;
}

PhyCone* BulletPhysics::MakeCone(float height, float radius, PhyAxis axis)
{
    traceIn(BulletPhysics::MakeCone);

    return CreateObjectParam4(BulletCone, this, radius, height, axis);

    traceOut;
}

PhyCompound* BulletPhysics::MakeCompound()
{
    traceIn(BulletPhysics::MakeCompound);

    return CreateObjectParam(BulletCompound, this);

    traceOut;
}

PhyStaticMesh* BulletPhysics::MakeStaticMeshShape(Vect *verts, UINT numVerts, UINT *indices, UINT numIndices)
{
    traceIn(BulletPhysics::MakeStaticMeshShape);

    return CreateObjectParam5(BulletMesh, this, verts, numVerts, indices, numIndices);

    traceOut;
}

PhyDynamicMesh* BulletPhysics::MakeDynamicMeshShape(Vect *verts, UINT numVerts, UINT *indices, UINT numIndices)
{
    traceIn(BulletPhysics::MakeDynamicMeshShape);

    return CreateObjectParam5(BulletDynamicMesh, this, verts, numVerts, indices, numIndices);

    traceOut;
}


PhyRigid* BulletPhysics::CreateStaticObject(PhyShape *shape, const Vect &pos, const Quat &rot, short collideGroups, short collideMask)
{
    traceIn(BulletPhysics::CreateStaticObject);

    BulletRigid *staticObj = (BulletRigid*)InitializeObjectData(new BulletRigid(this, shape, pos, rot, 0.0f, collideGroups, collideMask));
    staticObj->type = RigidType_Static;
    staticObj->GetObj()->setFriction(3.0f);
    return staticObj;

    traceOut;
}

PhyRigid* BulletPhysics::CreateKinematicObject(PhyShape *shape, const Vect &pos, const Quat &rot, short collideGroups, short collideMask)
{
    traceIn(BulletPhysics::CreateKinematicObject);

    BulletRigid *kinematicObj = (BulletRigid*)InitializeObjectData(new BulletRigid(this, shape, pos, rot, 0.0f, collideGroups, collideMask));
    kinematicObj->GetObj()->setCollisionFlags(kinematicObj->GetObj()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    kinematicObj->GetObj()->setActivationState(DISABLE_DEACTIVATION);
    kinematicObj->type = RigidType_Kinematic;

    return kinematicObj;

    traceOut;
}

PhyCharacter* BulletPhysics::CreateCharacterObject(PhyShape *shape, const Vect &pos, const Quat &rot, float mass, short collideGroups, short collideMask)
{
    traceIn(BulletPhysics::CreateCharacterObject);

    BulletCharacter *characterObj = (BulletCharacter*)InitializeObjectData(new BulletCharacter(this, shape, pos, rot, mass, collideGroups, collideMask));

    return characterObj;

    traceOut;
}

PhyRigid* BulletPhysics::CreateDynamicObject(PhyShape *shape, const Vect &pos, const Quat &rot, float mass, short collideGroups, short collideMask)
{
    traceIn(BulletPhysics::CreateDynamicObject);

    assertmsg(!shape->IsOf(GetClass(PhyStaticMesh)), TEXT("Cannot use static meshes for dynamic collision objects.  Use a dynamic mesh or decompose into convex shapes first."));

    BulletRigid *dynamicObj = (BulletRigid*)InitializeObjectData(new BulletRigid(this, shape, pos, rot, mass, collideGroups, collideMask));
    dynamicObj->type = RigidType_Dynamic;

    return dynamicObj;

    traceOut;
}

PhyGhost* BulletPhysics::CreateGhost(short collideGroups, short collideMask)
{
    traceIn(BulletPhysics::CreateGhost);

    return (PhyGhost*)InitializeObjectData(new BulletGhost(this, collideGroups, collideMask));

    traceOut;
}


BOOL BulletPhysics::GetLineObjects(const Vect &p1, const Vect &p2, List<PhyObject*> &Objects, short filter) const
{
    traceIn(BulletPhysics::GetLineObjects);

    btVector3 btP1 = VectToBT(p1), btP2 = VectToBT(p2);

    RayObjectsCallback rayCallback(btP1, btP2, Objects, filter);

    world->rayTest(btP1, btP2, rayCallback);

    return rayCallback.hasHit();

    traceOut;
}

BOOL BulletPhysics::GetLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo, short filter) const
{
    traceIn(BulletPhysics::GetLineCollision);

    btVector3 btP1 = VectToBT(p1), btP2 = VectToBT(p2);

    RayNearestCallback rayCallback(btP1, btP2, filter);

    world->rayTest(btP1, btP2, rayCallback);

    if(!rayCallback.hasHit())
        return FALSE;

    if(collisionInfo)
    {
        collisionInfo->hitPos  = BTToVect(rayCallback.m_hitPointWorld);
        collisionInfo->hitNorm = BTToVect(rayCallback.m_hitNormalWorld);
        collisionInfo->hitObj  = static_cast<PhyRigid*>(rayCallback.m_collisionObject->getUserPointer());
    }

    return TRUE;

    traceOut;
}

BOOL BulletPhysics::GetConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo, short filter) const
{
    traceIn(BulletPhysics::GetConvexCollision);

    if(shape->IsOf(GetClass(PhyStaticMesh)) || shape->IsOf(GetClass(PhyDynamicMesh)))
    {
        AppWarning(TEXT("Cannot use convex collision tests on convex shapes"));
        return FALSE;
    }

    btVector3 btP1 = VectToBT(p1), btP2 = VectToBT(p2);

    btTransform btT1, btT2;
    btT1.setIdentity();
    btT2.setIdentity();

    btT1.setOrigin(btP1);
    btT2.setOrigin(btP2);

    ConvexNearestCallback convexCallback(btP1, btP2, filter);

    world->convexSweepTest((btConvexShape*)GetBTShape(shape), btT1, btT2, convexCallback);

    if(!convexCallback.hasHit())
        return FALSE;

    if(collisionInfo)
    {
        collisionInfo->hitPos  = Lerp<Vect>(p1, p2, convexCallback.m_closestHitFraction);
        collisionInfo->hitNorm = BTToVect(convexCallback.m_hitNormalWorld);
        collisionInfo->hitObj  = static_cast<PhyRigid*>(convexCallback.m_hitCollisionObject->getUserPointer());
    }

    return TRUE;

    traceOut;
}

BOOL BulletPhysics::GetLineCollisionSingle(PhyObject *obj, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const
{
    traceIn(BulletPhysics::GetLineCollisionSingle);

    btVector3 btP1 = VectToBT(p1), btP2 = VectToBT(p2);

    btCollisionWorld::ClosestRayResultCallback rayCallback(btP1, btP2);

    btTransform btT1, btT2;
    btT1.setIdentity(); btT1.setOrigin(btP1);
    btT2.setIdentity(); btT2.setOrigin(btP2);

    world->rayTestSingle(btT1, btT2, GetBTObject(obj), GetBTObject(obj)->getCollisionShape(), GetBTObject(obj)->getWorldTransform(), rayCallback);

    if(!rayCallback.hasHit())
        return FALSE;

    if(collisionInfo)
    {
        collisionInfo->hitPos  = BTToVect(rayCallback.m_hitPointWorld);
        collisionInfo->hitNorm = BTToVect(rayCallback.m_hitNormalWorld);
        collisionInfo->hitObj  = static_cast<PhyRigid*>(rayCallback.m_collisionObject->getUserPointer());
    }

    return TRUE;

    traceOut;
}

BOOL BulletPhysics::GetConvexCollisionSingle(PhyObject *obj, PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const
{
    traceIn(BulletPhysics::GetConvexCollisionSingle);

    if(shape->IsOf(GetClass(PhyStaticMesh)))
    {
        AppWarning(TEXT("Cannot use convex collision tests on convex shapes"));
        return FALSE;
    }

    btVector3 btP1 = VectToBT(p1), btP2 = VectToBT(p2);

    btCollisionWorld::ClosestConvexResultCallback convexCallback(btP1, btP2);

    btTransform btT1, btT2;
    btT1.setIdentity(); btT1.setOrigin(btP1);
    btT2.setIdentity(); btT2.setOrigin(btP2);

    world->objectQuerySingle(static_cast<btConvexShape*>(GetBTShape(shape)), btT1, btT2, GetBTObject(obj), GetBTObject(obj)->getCollisionShape(), GetBTObject(obj)->getWorldTransform(), convexCallback, 0.0f);

    if(!convexCallback.hasHit())
        return FALSE;

    if(collisionInfo)
    {
        collisionInfo->hitPos  = BTToVect(convexCallback.m_hitPointWorld);
        collisionInfo->hitNorm = BTToVect(convexCallback.m_hitNormalWorld);
        collisionInfo->hitObj  = static_cast<PhyRigid*>(convexCallback.m_hitCollisionObject->getUserPointer());
    }

    return TRUE;

    traceOut;
}

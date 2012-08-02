/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  BulletCharacter

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


struct NonSelfFloorCallback : btCollisionWorld::ClosestConvexResultCallback
{
    NonSelfFloorCallback(const btVector3& convexFromWorld, const btVector3& convexToWorld, btRigidBody *self_, short flags, float slope)
        : btCollisionWorld::ClosestConvexResultCallback(convexFromWorld, convexToWorld), self(self_), collisionFlags(flags), maxSlope(slope)
    {}

    btRigidBody *self;
    short collisionFlags;
    float maxSlope;

    virtual bool needsCollision(btBroadphaseProxy* proxy0) const
    {
        if(proxy0->m_collisionFilterGroup & collisionFlags)
            return btCollisionWorld::ClosestConvexResultCallback::needsCollision(proxy0);

        return false;
    }

    virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult &convexResult, bool normalInWorldSpace)
    {
        if(convexResult.m_hitCollisionObject == self || convexResult.m_hitNormalLocal.getY() < maxSlope)
            return 1.0;

        //AAAAAAAAAAAARRRRRRRRRGGGGGGGGGGHHHHHHHHHH BULLET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        btCollisionShape *shape = convexResult.m_hitCollisionObject->getCollisionShape();
        if(shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
        {
            BulletMesh *targetMesh = (BulletMesh*)shape->getUserPointer();

            btMatrix3x3 trans = convexResult.m_hitCollisionObject->getWorldTransform().getBasis();
            Matrix transform;

            transform.X = BTToVect(trans.getRow(0));
            transform.Y = BTToVect(trans.getRow(1));
            transform.Z = BTToVect(trans.getRow(2));
            transform.T.SetZero();

            int indexStart = convexResult.m_localShapeInfo->m_triangleIndex*3;
            const UINT *triIDs = targetMesh->GetIndexList()+indexStart;
            const Vect *VertList = targetMesh->GetVertList();

            const Vect &vA = VertList[triIDs[0]],
                       &vB = VertList[triIDs[1]],
                       &vC = VertList[triIDs[2]];

            Vect triNorm = (vB-vA).Cross(vC-vA).Norm().TransformVector(transform);

            if(triNorm.y < maxSlope)
                return 1.0f;
        }

        return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
    }
};

struct CharacterMotionState : EngineMotionState
{
    CharacterMotionState(const btTransform& startTrans, PhyObject *curObj) : EngineMotionState(startTrans, curObj) {}

    void SetTransformNoUpdate(const btTransform &trans)
    {
        transform = trans;
    }

    virtual void setWorldTransform(const btTransform& trans)
    {
        Entity *ent = phyObj->GetEntityOwner();
        if(ent)
        {
            Vect pos = BTToVect(transform.getOrigin());
            Quat rot = BTToQuat(transform.getRotation());

            if(bInterpolating)
            {
                float fTime = float(TrackTimeRestart(timerID))*0.001f;
                fT += (fTime/fInterpolationTime);

                if(fT < 1.0f)
                    pos.y = Lerp<float>(startPos.y, pos.y, fT);
                else
                {
                    TrackTimeEnd(timerID);
                    bInterpolating = FALSE;
                }
            }

            curPos = pos;

            static_cast<BulletPhysics*>(physics)->UpdateEntityTransform(ent, pos, rot);
        }
        transform = trans;
    }

    DWORD timerID;
    Vect startPos, curPos;
    float fT, fInterpolationTime;
    BOOL bInterpolating;
};


CharacterActionHandler::CharacterActionHandler(btCollisionWorld *collisionWorld, BulletCharacter *character)
{
    traceIn(CharacterActionHandler::CharacterActionHandler);

    bFalling = TRUE;

    maxSlope = 0.8f;
    stepHeight = 1.5f;

    world = collisionWorld;

    body = character->GetObj();

    ghost = new btPairCachingGhostObject;
    ghost->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    ghost->setCollisionShape(body->getCollisionShape());
    world->addCollisionObject(ghost, PHY_GHOST, PHY_STATIC|PHY_DYNAMIC|PHY_KINEMATIC);

    traceOut;
}

CharacterActionHandler::~CharacterActionHandler()
{
    traceIn(CharacterActionHandler::~CharacterActionHandler);

    world->removeCollisionObject(ghost);
    delete ghost;

    traceOut;
}


BOOL CharacterActionHandler::BlockedPos(const Vect &pos)
{
    traceIn(CharacterActionHandler::BlockedPos);

    btTransform transform = ghost->getWorldTransform();
    transform.setOrigin(VectToBT(pos));
    ghost->setWorldTransform(transform);

    world->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), world->getDispatchInfo(), world->getDispatcher());

    for(int i=0; i<ghost->getOverlappingPairCache()->getNumOverlappingPairs(); i++)
    {
        btManifoldArray manifoldArray;

        btBroadphasePair* collisionPair = &ghost->getOverlappingPairCache()->getOverlappingPairArray()[i];
        
        if(collisionPair->m_algorithm)
            collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

        for(int j=0; j<manifoldArray.size(); j++)
        {
            btPersistentManifold* manifold = manifoldArray[j];
            BOOL bReverse = (manifold->getBody0() != ghost);
            float fFlip;

            if(!bReverse)
            {
                if(manifold->getBody1() == body)
                    continue;

                fFlip = 1.0f;
            }
            else
            {
                if(manifold->getBody0() == body)
                    continue;

                fFlip = -1.0;
            }

            for(int p=0; p<manifold->getNumContacts(); p++)
            {
                const btManifoldPoint &pt = manifold->getContactPoint(p);

                if( pt.getDistance() < -LARGE_EPSILON)
                    return TRUE;
            }
        }
    }

    return FALSE;

    traceOut;
}


BOOL CharacterActionHandler::PentrationRecovery(const Vect &v1, const Vect &v2)
{
    traceIn(CharacterActionHandler::PentrationRecovery);

    BOOL bPenetrated = FALSE;

    world->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), world->getDispatchInfo(), world->getDispatcher());

    Vect moveDir = (v2-v1).Norm();
    Vect curPos;
    curPos.SetZero();
    
    for(int i=0; i<ghost->getOverlappingPairCache()->getNumOverlappingPairs(); i++)
    {
        btManifoldArray manifoldArray;

        btBroadphasePair* collisionPair = &ghost->getOverlappingPairCache()->getOverlappingPairArray()[i];
        
        if(collisionPair->m_algorithm)
            collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

        for(int j=0; j<manifoldArray.size(); j++)
        {
            btPersistentManifold* manifold = manifoldArray[j];
            BOOL bReverse = (manifold->getBody0() != ghost);
            BulletMesh *targetMesh = NULL;
            Matrix transform;
            float fFlip;

            if(!bReverse)
            {
                if(manifold->getBody1() == body)
                    continue;

                btCollisionShape *shape = ((btCollisionObject*)manifold->getBody1())->getCollisionShape();
                if(shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
                {
                    targetMesh = (BulletMesh*)shape->getUserPointer();

                    btMatrix3x3 trans = ((btCollisionObject*)manifold->getBody1())->getWorldTransform().getBasis();
                    transform.X = BTToVect(trans.getRow(0));
                    transform.Y = BTToVect(trans.getRow(1));
                    transform.Z = BTToVect(trans.getRow(2));
                    transform.T.SetZero();
                }

                fFlip = 1.0f;
            }
            else
            {
                if(manifold->getBody0() == body)
                    continue;

                btCollisionShape *shape = ((btCollisionObject*)manifold->getBody0())->getCollisionShape();
                if(shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
                {
                    targetMesh = (BulletMesh*)shape->getUserPointer();

                    btMatrix3x3 trans = ((btCollisionObject*)manifold->getBody0())->getWorldTransform().getBasis();
                    transform.X = BTToVect(trans.getRow(0));
                    transform.Y = BTToVect(trans.getRow(1));
                    transform.Z = BTToVect(trans.getRow(2));
                    transform.T.SetZero();
                }

                fFlip = -1.0;
            }

            for(int p=0; p<manifold->getNumContacts(); p++)
            {
                const btManifoldPoint &pt = manifold->getContactPoint(p);
                Vect norm = BTToVect(pt.m_normalWorldOnB*fFlip);

                //if a mesh, pass on back faces.  kind of an annoying hack because bullet collides with back or front mesh faces
                //when it collided with those faces it caused collision fighting jitters
                if(targetMesh)
                {
                    int indexStart = (!bReverse) ? pt.m_index1*3 : pt.m_index0*3;
                    const UINT *triIDs = targetMesh->GetIndexList()+indexStart;
                    const Vect *VertList = targetMesh->GetVertList();

                    const Vect &vA = VertList[triIDs[0]],
                               &vB = VertList[triIDs[1]],
                               &vC = VertList[triIDs[2]];

                    Vect triNorm = (vB-vA).Cross(vC-vA).Norm().TransformVector(transform);

                    if(triNorm.Dot(moveDir) >= -EPSILON)
                        continue;
                }

                if( pt.getDistance() < -LARGE_EPSILON &&
                    norm.Dot(moveDir) < 0.0f )
                {
                    Plane contactOffsetPlane;
                    contactOffsetPlane.Dir = norm;
                    contactOffsetPlane.Dist = -pt.getDistance();

                    float dist = curPos.DistFromPlane(contactOffsetPlane);
                    if(dist < 0.0f)
                        curPos += norm * -dist;

                    bPenetrated = TRUE;
                }
            }
        }
    }

    btTransform newTrans = ghost->getWorldTransform();
    newTrans.setOrigin(newTrans.getOrigin()+VectToBT(curPos));
    ghost->setWorldTransform(newTrans);
    body->setWorldTransform(newTrans);

    return bPenetrated;

    traceOut;
}


void CharacterActionHandler::updateAction(btCollisionWorld *world, btScalar deltaTimeStep)
{
    traceIn(CharacterActionHandler::updateAction);

    btTransform transA, transB;
    Vect startPos, nextPos, v1, v2;
    Vect curMovement = movement;

    btConvexShape *shape = (btConvexShape*)body->getCollisionShape();

    curTime = deltaTimeStep;

    transA.setIdentity();
    transA.setOrigin(body->getWorldTransform().getOrigin());
    transB = transA;
    startPos = BTToVect(transA.getOrigin());
    nextPos = startPos+(curMovement*float(deltaTimeStep));

    //=================================================================
    // test to see whether falling or not
    //=================================================================

    v2 = v1 = startPos;
    v1.y += 0.01f;
    v2.y -= 0.1f;

    transA.setOrigin(VectToBT(v1));
    transB.setOrigin(VectToBT(v2));

    NonSelfFloorCallback hitInfo(transA.getOrigin(), transB.getOrigin(), body, PHY_STATIC|PHY_KINEMATIC|PHY_DYNAMIC, maxSlope);
    world->convexSweepTest(shape, transA, transB, hitInfo);
    //ghost->convexSweepTest(shape, transA, transB, hitInfo, world->getDispatchInfo().m_allowedCcdPenetration);

    if(hitInfo.m_closestHitFraction < 1.0f)
    {
        Vect stepPos = Lerp<Vect>(v1, v2, hitInfo.m_closestHitFraction);

        bFalling = FALSE;
        curPlane.Dir = BTToVect(hitInfo.m_hitNormalWorld);
        curPlane.Dist = curPlane.Dir.Dot(stepPos);

        body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    }
    else
    {
        if(!bFalling) //kind of a hack to cause it to fall realistically
        {
            if(curMovement.GetNorm().y < (1.0f-maxSlope))
                body->setLinearVelocity(body->getLinearVelocity()+VectToBT(curMovement));
        }
        bFalling = TRUE;
    }

    //=================================================================
    // if on the ground, step forward
    //=================================================================

    if(!bFalling && IsMoving())
    {
        //-----------------------------------------------------------------
        // get next step position according to current plane
        //-----------------------------------------------------------------

        float fT;
        if(curPlane.GetRayIntersection(nextPos, Vect(0.0f, 1.0f, 0.0f), fT)) //this actually should never return false
            nextPos.y += fT;

        v1 = v2 = nextPos;
        v1.y += (stepHeight+0.04f);
        v2.y -= (stepHeight+0.2f);
        transA.setOrigin(VectToBT(v1));
        transB.setOrigin(VectToBT(v2));

        hitInfo.m_convexFromWorld = transA.getOrigin();
        hitInfo.m_convexToWorld   = transB.getOrigin();
        hitInfo.collisionFlags = PHY_STATIC|PHY_KINEMATIC;
        hitInfo.m_closestHitFraction = 1.0f;
        world->convexSweepTest(shape, transA, transB, hitInfo);

        //-----------------------------------------------------------------
        // if stepped up onto something, and the step was significant, interpolate
        //-----------------------------------------------------------------

        BOOL bValidStep = FALSE;
        if(hitInfo.m_closestHitFraction < 1.0f)
        {
            Vect stepPos = Lerp<Vect>(v1, v2, hitInfo.m_closestHitFraction);
            if(!BlockedPos(stepPos))
            {
                bValidStep = TRUE;
                transB.setOrigin(VectToBT(stepPos));
                v2 = stepPos;

                float stepAdj = fabs(stepPos.y-nextPos.y);

                if(stepAdj > 0.1f)
                {
                    CharacterMotionState *motion = (CharacterMotionState*)body->getMotionState();
                    motion->bInterpolating = TRUE;
                    motion->timerID = TrackTimeBegin();
                    motion->startPos = motion->curPos;
                    motion->fInterpolationTime = stepAdj*0.2f;
                    motion->fT = 0.0f;
                }
            }
        }

        if(!bValidStep)
        {
            transB.setOrigin(VectToBT(nextPos));
            v2 = nextPos;
        }

        //-----------------------------------------------------------------
        // recover from penetration
        //-----------------------------------------------------------------

        transB.setBasis(body->getWorldTransform().getBasis());
        ghost->setWorldTransform(transB);
        body->setWorldTransform(transB);
        body->activate(true);

        PentrationRecovery(startPos, v2);
    }

    traceOut;
}

void CharacterActionHandler::Jump(float speed)
{
}


//---------------------------------------------------------------------------------------------------------------------------

BulletCharacter::BulletCharacter(BulletPhysics *bullet, PhyShape *shape, const Vect &pos, const Quat &rot, float mass, short collideGroups, short collideMask)
{
    traceIn(BulletCharacter::BulletCharacter);

    system = bullet;

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(VectToBT(pos));
    transform.setRotation(QuatToBT(rot));

    motionState = new CharacterMotionState(transform, this);

    btVector3 intertia(0.0f, 0.0f, 0.0f);
    /*if(CloseFloat(mass, 0.0f))
        mass = 0.0f;
    else
        GetBTShape(shape)->calculateLocalInertia(mass, intertia);*/

    curShape = shape;

    btRigidBody::btRigidBodyConstructionInfo rigidInfo(mass, motionState, GetBTShape(shape), intertia);
    data = (LPVOID)new btRigidBody(rigidInfo);

    GetObj()->setUserPointer(this);
    //GetObj()->setFriction(1.0f);

    system->world->addRigidBody(GetObj(), collideGroups, collideMask);

    handler = new CharacterActionHandler(system->world, this);
    system->world->addAction(handler);

    LinkedListAddPhyObj(BulletCharacter);

    traceOut;
}

BulletCharacter::~BulletCharacter()
{
    traceIn(BulletCharacter::~BulletCharacter);

    SetEntityOwner(NULL);

    LinkedListRemovePhyObj(BulletCharacter);

    system->world->removeAction(handler);
    delete handler;

    system->world->removeRigidBody(GetObj());

    delete GetObj();
    delete motionState;

    traceOut;
}


void BulletCharacter::SetPos(const Vect &pos)
{
    traceIn(BulletCharacter::SetPos);

    btTransform &transform = GetObj()->getWorldTransform();
    transform.setOrigin(VectToBT(pos));
    GetObj()->activate();

    motionState->SetTransformNoUpdate(transform);

    traceOut;
}

void BulletCharacter::SetRot(const Quat &rot)
{
    traceIn(BulletCharacter::SetRot);

    btTransform &transform = GetObj()->getWorldTransform();
    transform.setRotation(QuatToBT(rot));
    GetObj()->activate();

    traceOut;
}

Vect BulletCharacter::GetPos() const
{
    traceIn(BulletCharacter::GetPos);

    btTransform transform;
    motionState->getWorldTransform(transform);
    return BTToVect(transform.getOrigin());

    traceOut;
}

Quat BulletCharacter::GetRot() const
{
    traceIn(BulletCharacter::GetRot);

    btTransform transform;
    motionState->getWorldTransform(transform);
    return BTToQuat(transform.getRotation());

    traceOut;
}

void BulletCharacter::GetCurrentTransform(Vect &pos, Quat &rot) const
{
    traceIn(BulletCharacter::GetCurrentTransform);

    btTransform transform;
    motionState->getWorldTransform(transform);
    pos = BTToVect(transform.getOrigin());
    rot = BTToQuat(transform.getRotation());

    traceOut;
}


void BulletCharacter::SetShape(PhyShape *shape)
{
    traceIn(BulletCharacter::SetShape);

    GetObj()->setCollisionShape(GetBTShape(shape));
    GetObj()->activate();

    traceOut;
}


void BulletCharacter::SetFilter(short collideGroups, short collideMask)
{
    traceIn(BulletCharacter::SetFilter);

    system->world->removeRigidBody(GetObj());
    system->world->addRigidBody(GetObj(), collideGroups, collideMask);

    traceOut;
}


BOOL BulletCharacter::GetLineCollision(const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const
{
    traceIn(BulletCharacter::GetLineCollision);

    return system->GetLineCollisionSingle((PhyObject*)this, p1, p2, collisionInfo);

    traceOut;
}

BOOL BulletCharacter::GetConvexCollision(PhyShape *shape, const Vect &p1, const Vect &p2, PhyCollisionInfo *collisionInfo) const
{
    traceIn(BulletCharacter::GetConvexCollision);

    return system->GetConvexCollisionSingle((PhyObject*)this, shape, p1, p2, collisionInfo);

    traceOut;
}


void BulletCharacter::Activate()
{
    traceIn(BulletCharacter::Activate);

    GetObj()->activate();

    traceOut;
}

void BulletCharacter::Deactivate()
{
    traceIn(BulletCharacter::Deactivate);

    GetObj()->setActivationState(WANTS_DEACTIVATION);

    traceOut;
}

BOOL BulletCharacter::IsActive()
{
    traceIn(BulletCharacter::IsActive);

    return ((GetObj()->getActivationState() != ISLAND_SLEEPING) && (GetObj()->getActivationState() != WANTS_DEACTIVATION));

    traceOut;
}


void BulletCharacter::ApplyImpulse(const Vect& impulse)
{
    traceIn(BulletCharacter::ApplyImpulse);

    traceOut;
}

void BulletCharacter::ApplyRelativeImpulse(const Vect &relativePos, const Vect &impulse)
{
    traceIn(BulletCharacter::ApplyRelativeImpulse);

    traceOut;
}


void BulletCharacter::SetVelocity(const Vect &linearVelocity)
{
    traceIn(BulletCharacter::SetVelocity);

    traceOut;
}

Vect BulletCharacter::GetVelocity() const
{
    traceIn(BulletCharacter::GetVelocity);

    return Vect(0.0f, 0.0f, 0.0f);

    traceOut;
}


Bounds BulletCharacter::GetBounds() const
{
    traceIn(BulletCharacter::GetBounds);

    btVector3 min, max;
    GetObj()->getAabb(min, max);

    return Bounds(BTToVect(min), BTToVect(max));

    traceOut;
}


void  BulletCharacter::SetFriction(float friction)
{
    traceIn(BulletCharacter::SetFriction);

    GetObj()->setFriction(friction);

    traceOut;
}

float BulletCharacter::GetFriction() const
{
    traceIn(BulletCharacter::GetFriction);

    return GetObj()->getFriction();

    traceOut;
}

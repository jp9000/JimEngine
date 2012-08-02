/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Entity.h

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


#ifndef ENTITY_HEADER
#define ENTITY_HEADER


/*=========================================================
    LevelData
==========================================================*/

class LevelData : public Object
{
public:
    virtual ~LevelData() {}
};


/*=========================================================
    PathData
==========================================================*/

class PathfindingData : public Object
{
public:
    virtual ~PathfindingData() {}
};


/*=========================================================
    Entity
==========================================================*/

#define TYPE_INTERNAL   0
#define TYPE_OBJECT     1
#define TYPE_PREFAB     2

//-----------------------------------------
//Renderable entity object
class BASE_EXPORT Entity : public FrameObject
{
    friend class Level;
    friend class PathSystem;
    friend class PhysicsSystem;
    friend class EditorEngine;
    friend class EditorLevelInfo;
    friend class ObjectPropertiesEditor;
    friend class Game;

    DeclareClass(Entity, FrameObject);

    static  void DestroyAll();

    static Entity* pFirstEntity;
    static Entity* pLastEntity;
    Entity *pNextEntity, *pPrevEntity;

    inline void AddToList();
    inline void RemoveFromList();


    //linked list for updating entities
    static Entity* pFirstUpdatingEntity;
    static Entity* pLastUpdatingEntity;
    Entity *pNextUpdatingEntity, *pPrevUpdatingEntity;

    inline void AddToUpdateList();
    inline void RemoveFromUpdateList();


    Entity          *Parent;
    List<Entity*>   Children;

    //status
    String          state;

    //cinematic
    BOOL            bInCinematic;
    Vect            savedPos;

    //rendering stuff
    BOOL            bVisible;
    BOOL            bAlreadyUsed;

    //positional stuff
    Vect            Pos;
    Quat            Rot;
    BOOL            bHasMoved, bInUpdateList;
    BOOL            bCalculateLocalPosition;

    Vect            worldPos;
    Quat            worldRot;

    virtual BOOL    InsideClip(const ViewClip &clip);

    //editor stuff
    BOOL            bSelected;
    String          name;
    DWORD           UserCreatedObjectType;

    //other temporary data
    Plane           curCamPlane;
    LevelData       *levelData;
    PathfindingData *pathfindingData;
    BOOL            bSavable;
    UINT            entityID;

    //collision/physics info
    Plane           curCollisionPlane;

protected:
    //editor stuff
    Texture         *editorSprite;

public:
    Entity();
    virtual ~Entity();

    void Destroy();

    //base list for all unattached objects
    static Entity* FindByClass(Class *cls, Entity *prevEntity=NULL, Entity *parent=NULL);
    static Entity* FindByClass(TSTR lpName, Entity *prevEntity=NULL, Entity *parent=NULL);
    static Entity* FindByName(TSTR lpName, Entity *parent=NULL);
    static Entity* FindByID(UINT id, Entity *parent=NULL);
    static void    FindAllByClass(Class *cls, List<Entity*> &ents, Entity *parent=NULL);
    static void    FindAllByClass(TSTR lpName, List<Entity*> &ents, Entity *parent=NULL);

    static Entity* FirstEntity() {return pFirstEntity;}
    static Entity* LastEntity()  {return pLastEntity;}
    Entity* PrevEntity()         {return pPrevEntity;}
    Entity* NextEntity()         {return pNextEntity;}

    //status
    BOOL SetState(CTSTR newStatus);
    inline const String& GetState() const {return state;}

    virtual BOOL StateChanging(CTSTR newState) {return scriptStateChanged(newState);}

    //positional data
    virtual void SetPos(const Vect &pos);        //always local.  This will set the position, and no collision detection will occur
    virtual void SetRot(const Quat &rot);        //always local.

    virtual void UpdatePositionalData();
    virtual BOOL UpdatingPosition() {return scriptUpdatingPosition();}
    virtual void OnUpdatePosition() {scriptOnUpdatePosition();}

    inline const Vect& GetLocalPos() const {return Pos;}
    inline const Quat& GetLocalRot() const {return Rot;}
    inline const Vect& GetWorldPos() const {return worldPos;}
    inline const Quat& GetWorldRot() const {return worldRot;}

    inline Matrix GetEntityTransform() const
    {
        Matrix transform;
        transform.SetIdentity();
        transform.Translate(worldPos);
        transform.Rotate(worldRot);

        return transform;
    }

    inline Matrix GetEntityInvTransform() const
    {
        Matrix transform;
        transform.SetIdentity();
        transform.Rotate(worldRot.GetInv());
        transform.Translate(-worldPos);

        return transform;
    }

    //entity attachment data
    virtual void Attach(Entity *new_parent);
    virtual void Detach();

    inline unsigned int  NumChildren() const {return Children.Num();}
    inline Entity*       GetChild(unsigned int i) const {return Children[i];}
    inline Entity*       GetParent() const {return Parent;}

    //collision/physics info
    inline BOOL GetLineCollision(const Vect &vStart, const Vect &vEnd, PhyCollisionInfo *collisionInfo=NULL) const;
    inline BOOL GetConvexCollision(PhyShape *shape, const Vect &vStart, const Vect &vEnd, PhyCollisionInfo *collisionInfo=NULL) const;
    inline BOOL GetRayCollision(const Vect &rayPos, const Vect &rayDir, PhyCollisionInfo *collisionInfo=NULL, float rayLength=1000.0f) const;

    virtual Bounds GetBounds() {return scriptGetBounds();}

    virtual void OnCollision(Entity *collider, const Vect &hitPos, const Vect &hitNormal, float impulse) {scriptOnCollision(collider, hitPos, hitNormal, impulse);}

    //other stuff
    virtual Entity* DuplicateEntity();

    virtual void    SetAsSavable(BOOL bCanSave);
    inline  BOOL    IsSavable() {return bSavable || UserCreatedObjectType == TYPE_OBJECT;}

    virtual BOOL    CanFrob()  {return scriptCanFrob();}
    virtual void    OnFrob()   {scriptOnFrob();}

    //editor stuff
    virtual void    EditorRender();

    virtual BOOL    CanSelect(const Vect &rayOrig, const Vect &rayDir);
    inline  BOOL    IsSelected() {return bSelected;}
    inline  void    SetSelected(BOOL bSelect) {bSelected = bSelect;}

    inline DWORD    GetEditType() const {return UserCreatedObjectType;}
    inline void     SetEditType(DWORD editType) {UserCreatedObjectType = editType;}

    inline String   GetName() const {return String(name);}
    inline void     SetName(CTSTR lpName)   {name = lpName;}

    void            GenerateUniqueName(CTSTR lpPrefix=NULL);

    //cinematic
    inline BOOL     InCinematic() const {return bInCinematic;}

    //serialization
    inline UINT     GetEntityID() const {return entityID;}

    void            Serialize(Serializer &s);
    virtual void    SerializeGameData(Serializer &s);

    static  Entity* LoadEntity(Serializer &s);
    static  void    SaveEntity(Serializer &s, Entity *ent=NULL);

    //<Script module="Base" classdecs="Entity">
    PhyShape* phyShape;
    PhyObject* phyObject;
    BOOL bPositionalOnly;
    BOOL bAlwaysVisible;
    BOOL bPlacable;

    BOOL scriptUpdatingPosition()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 28, cs);

        return cs.GetInt(RETURNVAL);
    }

    void scriptOnUpdatePosition()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 29, cs);
    }

    Bounds scriptGetBounds()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 30, cs);

        return (Bounds&)cs.GetStruct(RETURNVAL);
    }

    BOOL scriptCanFrob()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 31, cs);

        return cs.GetInt(RETURNVAL);
    }

    void scriptOnFrob()
    {
        CallStruct cs;
        GetLocalClass()->CallScriptMember(this, 32, cs);
    }

    void scriptOnCollision(Entity* collider, const Vect& hitPos, const Vect& hitNorm, float impulse)
    {
        CallStruct cs;
        cs.SetNumParams(4);
        cs.SetObject(0, (Object*)collider);
        cs.SetStruct(1, &hitPos, 16);
        cs.SetStruct(2, &hitNorm, 16);
        cs.SetFloat(3, impulse);

        GetLocalClass()->CallScriptMember(this, 33, cs);
    }

    BOOL scriptStateChanged(CTSTR newState)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetString(0, newState);

        GetLocalClass()->CallScriptMember(this, 34, cs);

        return cs.GetInt(RETURNVAL);
    }

    void scriptSerializeGameData(SerializerObject* s)
    {
        CallStruct cs;
        cs.SetNumParams(1);
        cs.SetObject(0, (Object*)s);

        GetLocalClass()->CallScriptMember(this, 35, cs);
    }

    Declare_Internal_StaticMember(native_FindByClass);
    Declare_Internal_StaticMember(native_FindByName);
    Declare_Internal_StaticMember(native_FindByID);
    Declare_Internal_StaticMember(native_FindAllByClass);
    Declare_Internal_Member(native_SetPos);
    Declare_Internal_Member(native_SetRot);
    Declare_Internal_Member(native_GetLocalPos);
    Declare_Internal_Member(native_GetLocalRot);
    Declare_Internal_Member(native_GetWorldPos);
    Declare_Internal_Member(native_GetWorldRot);
    Declare_Internal_Member(native_GetEntityTransform);
    Declare_Internal_Member(native_GetEntityInvTransform);
    Declare_Internal_Member(native_Attach);
    Declare_Internal_Member(native_Detach);
    Declare_Internal_Member(native_NumChildren);
    Declare_Internal_Member(native_GetChild);
    Declare_Internal_Member(native_GetParent);
    Declare_Internal_Member(native_GetLineCollision);
    Declare_Internal_Member(native_GetRayCollision);
    Declare_Internal_Member(native_GetConvexCollision);
    Declare_Internal_Member(native_GetName);
    Declare_Internal_Member(native_DuplicateEntity);
    Declare_Internal_Member(native_SetAsSavable);
    Declare_Internal_Member(native_IsSavable);
    Declare_Internal_Member(native_GetEntityID);
    Declare_Internal_Member(native_SetState);
    Declare_Internal_Member(native_GetState);
    Declare_Internal_Member(native_UpdatePositionalData);
    //</Script>
};

inline Entity* FindEntityByClass(Class *cls, Entity *prevEntity=NULL, Entity *parent=NULL)  {return Entity::FindByClass(cls, parent);}
inline Entity* FindEntityByClass(TSTR lpName, Entity *prevEntity=NULL, Entity *parent=NULL) {return Entity::FindByClass(lpName);}
inline Entity* FindEntityByName(TSTR lpName)                                                {return Entity::FindByName(lpName);}
inline Entity* FindEntityByID(UINT id)                                                      {return Entity::FindByID(id);}


Declare_Native_Global(NativeGlobal_FindEntityByClass);
Declare_Native_Global(NativeGlobal_FindEntityByName);
Declare_Native_Global(NativeGlobal_FindEntityByID);


inline BOOL Entity::GetLineCollision(const Vect &vStart, const Vect &vEnd, PhyCollisionInfo *collisionInfo) const
{
    if(!phyObject || !phyShape) return FALSE;
    return phyObject->GetLineCollision(vStart, vEnd, collisionInfo);
}

inline BOOL Entity::GetConvexCollision(PhyShape *shape, const Vect &vStart, const Vect &vEnd, PhyCollisionInfo *collisionInfo) const
{
    if(!phyObject || !phyShape) return FALSE;
    return phyObject->GetConvexCollision(shape, vStart, vEnd, collisionInfo);
}

inline BOOL Entity::GetRayCollision(const Vect &rayPos, const Vect &rayDir, PhyCollisionInfo *collisionInfo, float rayLength) const
{
    Vect endPos = rayPos+(rayDir*rayLength);
    return GetLineCollision(rayPos, endPos, collisionInfo);
}


inline void Entity::AddToList()
{
    if(pLastEntity)
    {
        pPrevEntity = pLastEntity;
        pLastEntity->pNextEntity = this;
        pNextEntity = NULL;
    }
    else 
    {
        pPrevEntity = pNextEntity = NULL;
        pFirstEntity = this;
    }

    pLastEntity = this;
}

inline void Entity::RemoveFromList()
{
    if(!pPrevEntity && !pNextEntity)
        pFirstEntity = pLastEntity = NULL;
    else
    {
        if(pPrevEntity)
            pPrevEntity->pNextEntity = pNextEntity;
        if(pNextEntity)
            pNextEntity->pPrevEntity = pPrevEntity;

        if(pLastEntity == this)
            pLastEntity = pPrevEntity;
        else if(pFirstEntity == this)
            pFirstEntity = pNextEntity;
    }
}

inline void Entity::AddToUpdateList()
{
    if(!bInUpdateList)
    {
        if(pLastUpdatingEntity)
        {
            pPrevUpdatingEntity = pLastUpdatingEntity;
            pLastUpdatingEntity->pNextUpdatingEntity = this;
            pNextUpdatingEntity = NULL;
        }
        else 
        {
            pPrevUpdatingEntity = pNextUpdatingEntity = NULL;
            pFirstUpdatingEntity = this;
        }

        pLastUpdatingEntity = this;

        bInUpdateList = TRUE;
    }
}

inline void Entity::RemoveFromUpdateList()
{
    if(bInUpdateList)
    {
        if(!pPrevUpdatingEntity && !pNextUpdatingEntity)
            pFirstUpdatingEntity = pLastUpdatingEntity = NULL;
        else
        {
            if(pPrevUpdatingEntity)
                pPrevUpdatingEntity->pNextUpdatingEntity = pNextUpdatingEntity;
            if(pNextUpdatingEntity)
                pNextUpdatingEntity->pPrevUpdatingEntity = pPrevUpdatingEntity;

            if(pLastUpdatingEntity == this)
                pLastUpdatingEntity = pPrevUpdatingEntity;
            else if(pFirstUpdatingEntity == this)
                pFirstUpdatingEntity = pNextUpdatingEntity;
        }

        pPrevUpdatingEntity = pNextUpdatingEntity = NULL;

        bInUpdateList = FALSE;
    }
}


#endif

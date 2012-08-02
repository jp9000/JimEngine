/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Entity.cpp:  Bsae Entity class

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


DefineClass(Entity);

Entity* Entity::pFirstEntity = NULL;
Entity* Entity::pLastEntity = NULL;

Entity* Entity::pFirstUpdatingEntity = NULL;
Entity* Entity::pLastUpdatingEntity = NULL;


/*========================================================
  Entity
=========================================================*/

Entity::Entity()
{
    traceInFast(Entity::Entity);

    AddToList();

    Rot.SetIdentity();
    worldRot.SetIdentity();

    if(engine && engine->InEditor())
        editorSprite = GetTexture(TEXT("Base:Editor/Killme.tga"));

    levelData = NULL;

    entityID = Level::entityIDCounter++;

    UpdatePositionalData();

    traceOutFast;
}

Entity::~Entity()
{
    traceInFast(Entity::~Entity);

    DestroyObject(phyObject);
    if(phyShape) phyShape->Release();

    if(!Parent)
    {
        RemoveFromList();
        RemoveFromUpdateList();
    }
    else
        Parent->Children.RemoveItem(this);

    int nChildren = Children.Num();

    for(int i=0; i<nChildren; i++)
        DestroyObject(Children[0]);

    traceOutFast;
}

void Entity::Destroy()
{
    traceInFast(Entity::Destroy);

    if(level && !engine->InEditor())
    {
        if(UserCreatedObjectType == TYPE_OBJECT)
            level->DestroyedUserObjects << entityID;
    }

    if(level && levelData)
        level->RemoveEntity(this);

    Super::Destroy();

    traceOutFast;
}

void Entity::DestroyAll()
{
    traceIn(Entity::DestroyAll);

    while(pFirstEntity)
        DestroyObject(pFirstEntity);

    traceOut;
}

void Entity::SetPos(const Vect &pos)
{
    traceIn(Entity::SetPos);

    Pos = pos;
    if(!bHasMoved)
        UpdatePositionalData();

    if(!Parent && phyObject)
        phyObject->SetPos(Pos);

    traceOut;
}

void Entity::SetRot(const Quat &rot)
{
    traceIn(Entity::SetRot);

    Rot = rot;
    if(!bHasMoved)
        UpdatePositionalData();

    /*if(Parent)
        Parent->SetRot(Parent->Rot);
    else if(phyObject)
        phyObject->SetRot(Rot);*/

    if(!Parent && phyObject)
        phyObject->SetRot(Rot);

    traceOut;
}

void Entity::Attach(Entity *new_parent)
{
    traceIn(Entity::Attach);

    if(Parent)
    {
        SetPos(worldPos);
        Rot = worldRot;

        Parent->Children.RemoveItem(this);
    }

    new_parent->Children << this;
    Parent = new_parent;

    RemoveFromList();

    UpdatePositionalData();

    traceOut;
}

void Entity::UpdatePositionalData()
{
    if(Parent && !Parent->bHasMoved)
        Parent->UpdatePositionalData();
    else if(!Parent)
        AddToUpdateList();

    bHasMoved = TRUE;
}

void Entity::Detach()
{
    traceIn(Entity::Detach);

    if(Parent)
    {
        SetPos(worldPos);
        Rot = worldRot;

        Parent->Children.RemoveItem(this);
        Parent = NULL;

        AddToList();
    }

    traceOut;
}

Entity* Entity::FindByClass(Class *cls, Entity *prevEntity, Entity *parent)
{
    BOOL bFoundEntity = (!prevEntity);
    if(parent)
    {
        for(DWORD i=0; i<parent->Children.Num(); i++)
        {
            Entity *ent = parent->Children[i];

            if(!bFoundEntity)
            {
                if(ent == prevEntity)
                    bFoundEntity = TRUE;
            }
            else
            {
                if(ent->IsOf(cls))
                    return ent;
            }

            if(ent->Children.Num())
            {
                Entity *childEnt = FindByClass(cls, prevEntity, ent);
                if(childEnt)
                    return childEnt;
            }
        }
    }
    else
    {
        Entity *ent = pFirstEntity;

        while(ent)
        {
            if(!bFoundEntity)
            {
                if(ent == prevEntity)
                    bFoundEntity = TRUE;
            }
            else
            {
                if(ent->IsOf(cls))
                    return ent;
            }

            if(ent->Children.Num())
            {
                Entity *childEnt = FindByClass(cls, prevEntity, ent);
                if(childEnt)
                    return childEnt;
            }

            ent = ent->pNextEntity;
        }
    }

    return NULL;
}

Entity* Entity::FindByClass(TSTR lpName, Entity *prevEntity, Entity *parent)
{
    Class *cls = NULL;
    if(prevEntity)  //speeds class name lookup if we've already started searching
    {
        cls = prevEntity->GetObjectClass();
        while(scmp(cls->GetName(), lpName) != 0)
            cls = cls->GetParent();
    }
    else
        cls = FindClass(lpName);

    if(!cls)
        return NULL;

    return FindByClass(cls, prevEntity, parent);
}


Entity* Entity::FindByName(TSTR lpName, Entity *parent)
{
    if(parent)
    {
        for(int i=0; i<parent->Children.Num(); i++)
        {
            Entity *ent = parent->Children[i];

            if(ent->name.CompareI(lpName))
                return ent;

            if(ent->Children.Num())
            {
                Entity *childEnt = FindByName(lpName, ent);
                if(childEnt)
                    return childEnt;
            }
        }
    }
    else
    {
        Entity *ent = pFirstEntity;

        while(ent)
        {
            if(ent->name.CompareI(lpName))
                return ent;

            if(ent->Children.Num())
            {
                Entity *childEnt = FindByName(lpName, ent);
                if(childEnt)
                    return childEnt;
            }

            ent = ent->pNextEntity;
        }
    }

    return NULL;
}

Entity* Entity::FindByID(UINT id, Entity *parent)
{
    if(parent)
    {
        for(int i=0; i<parent->Children.Num(); i++)
        {
            Entity *ent = parent->Children[i];

            if(ent->entityID == id)
                return ent;

            if(ent->Children.Num())
            {
                Entity *childEnt = FindByID(id, ent);
                if(childEnt)
                    return childEnt;
            }
        }
    }
    else
    {
        Entity *ent = pFirstEntity;

        while(ent)
        {
            if(ent->entityID == id)
                return ent;

            if(ent->Children.Num())
            {
                Entity *childEnt = FindByID(id, ent);
                if(childEnt)
                    return childEnt;
            }

            ent = ent->pNextEntity;
        }
    }

    return NULL;
}


void Entity::FindAllByClass(Class *cls, List<Entity*> &ents, Entity *parent)
{
    if(parent)
    {
        for(DWORD i=0; i<parent->Children.Num(); i++)
        {
            Entity *child = parent->Children[i];
            if(child->IsOf(cls))
                ents.SafeAdd(child);

            FindAllByClass(cls, ents, child);
        }
    }
    else
    {
        Entity *ent = pFirstEntity;
        while(ent)
        {
            if(ent->IsOf(cls))
                ents.SafeAdd(ent);

            FindAllByClass(cls, ents, ent);

            ent = ent->pNextEntity;
        }
    }
}

void Entity::FindAllByClass(TSTR lpName, List<Entity*> &ents, Entity *parent)
{
    Class *cls = FindClass(lpName);
    if(!cls)
        return;

    return FindAllByClass(cls, ents, parent);
}

void Entity::EditorRender()
{
    traceIn(Entity::EditorRender);

    if(editorSprite && level)
        GS->DrawSprite3D(level->GetCurrentCamera()->worldRot, editorSprite, worldPos, 1.0f, 1.0f);

    if(bSelected)
    {
        DWORD i;

        Shader *shader = GetVertexShader(TEXT("Base:SolidColor.vShader"));
        LoadVertexShader(shader);
        LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

        shader->SetColor(shader->GetParameter(1), 1.0f, 1.0f, 1.0f, 1.0f);

        RenderStart();

        Vertex(0.0f, 0.0f, 0.0f);
        Vertex(0.0f, 0.0f, -3.0f);

        /*Vertex(0.0f, 0.0f, 5.0f);
        Vertex(1.0f, 0.0f, 4.0f);

        Vertex(0.0f, 0.0f, 5.0f);
        Vertex(-1.0f, 0.0f, 4.0f);

        Vertex(0.0f, 0.0f, 5.0f);
        Vertex(0.0f, 1.0f, 4.0f);

        Vertex(0.0f, 0.0f, 5.0f);
        Vertex(0.0f, -1.0f, 4.0f);*/

        float curAngle1 = 0.0f;
        float sinAngle1 = 0.0f;
        float cosAngle1 = 0.25f;

        for(i=0; i<10; i++)
        {
            float curAngle2 = RAD(36.0*float(i+1));
            float sinAngle2 = sinf(curAngle2)*0.25f;
            float cosAngle2 = cosf(curAngle2)*0.25f;

            Vertex(sinAngle1, cosAngle1, -2.0f);
            Vertex(sinAngle2, cosAngle2, -2.0f);

            Vertex(sinAngle1, cosAngle1, -2.0f);
            Vertex(0.0f, 0.0f, -3.0f);

            curAngle1 = curAngle2;
            sinAngle1 = sinAngle2;
            cosAngle1 = cosAngle2;
        }

        MatrixPush();   
        MatrixTranslate(worldPos);
        MatrixRotate(worldRot);
        RenderStop(GS_LINES);
        MatrixPop();

        RenderStart();

        LoadVertexShader(NULL);
        LoadPixelShader(NULL);
    }

    traceOut;
}

BOOL Entity::InsideClip(const ViewClip &clip)
{
    traceIn(Entity::InsideClip);

    return clip.SphereVisible(worldPos, 3.0f);

    traceOut;
}

BOOL Entity::CanSelect(const Vect &rayOrig, const Vect &rayDir)
{
    traceIn(Entity::CanSelect);

    return SphereRayCollision(worldPos, 1.0f, rayOrig, rayDir);

    traceOut;
}

Entity *Entity::DuplicateEntity()
{
    traceIn(Entity::DuplicateEntity);

    Entity *ent = (Entity*)this->GetObjectClass()->Create(FALSE);
    List<BYTE> data;

    BufferOutputSerializer sSave(data);
    this->Serialize(sSave);

    BufferInputSerializer sLoad(data);
    ent->Serialize(sLoad);
    ent->GenerateUniqueName();
    ent->Init();

    return ent;

    traceOut;
}

BOOL Entity::SetState(CTSTR newState)
{
    traceIn(Entity::SetState);

    if(state.CompareI(newState))
        return FALSE;

    if(StateChanging(newState))
    {
        state = newState;
        return TRUE;
    }

    return FALSE;

    traceOut;
}

void Entity::SetAsSavable(BOOL bCanSave)
{
    bSavable = TRUE;
}

void Entity::GenerateUniqueName(CTSTR lpPrefix)
{
    traceIn(Entity::GenerateUniqueName);

    Class *cls = this->GetObjectClass();

    if(!lpPrefix)
        lpPrefix = cls->GetName();

    List<Entity*> Ents;
    FindAllByClass(cls, Ents);

    BOOL bFoundUnique = FALSE;
    int curID=0;
    String uniqueName;

    while(!bFoundUnique && (curID < 500))
    {
        TCHAR lpNum[10];
        tsprintf_s (lpNum, 6, TEXT("%05d"), ++curID);

        uniqueName.Clear() << lpPrefix << lpNum;

        bFoundUnique = TRUE;

        if(FindByName(uniqueName))
            bFoundUnique = FALSE;
    }

    if(bFoundUnique)
        name = uniqueName;

    traceOut;
}

void Entity::Serialize(Serializer &s)
{
    traceIn(Entity::Serialize);

    s << name << entityID << Pos << Rot << UserCreatedObjectType;

    if(s.IsLoading())
    {
        worldPos = Pos;
        worldRot = Rot;
        UpdatePositionalData();
    }

    Super::Serialize(s);

    traceOut;
}

void Entity::SerializeGameData(Serializer &s)
{
    SerializerObject *sObj = CreateObjectParam2(SerializerObject, &s, FALSE);
    scriptSerializeGameData(sObj);
    DestroyObject(sObj);
}


void Entity::SaveEntity(Serializer &s, Entity *ent)
{
    traceIn(Entity::SaveEntity);

    if(!s.IsLoading() && ent)
    {
        Class *classInfo = ent->GetObjectClass();

        String strClassName = classInfo->GetName();
        s << strClassName;

        int offsetPos = 0;
        int savePos = s.GetPos();
        s << offsetPos;

        ent->Serialize(s);

        BOOL bDeactivatePhysics = FALSE;
        if(ent->phyObject)
            bDeactivatePhysics = !ent->phyObject->IsActive();

        s << bDeactivatePhysics;

        offsetPos = s.GetPos();
        s.Seek(savePos, SERIALIZE_SEEK_START);
        s << offsetPos;
        s.Seek(0, SERIALIZE_SEEK_END);
    }

    traceOut;
}

Entity* Entity::LoadEntity(Serializer &s)
{
    traceIn(Entity::LoadEntity);

    if(s.IsLoading())
    {
        String strClassName;
        s << strClassName;

        int skipPos;
        s << skipPos;

        Entity *ent = (Entity*)CreateFactoryObject(strClassName, FALSE);
        if(!ent)
        {
            Log(TEXT("Warning: Could not load object of class '%s' - skipping"), (TSTR)strClassName);
            s.Seek(skipPos);
        }
        else
        {
            ent->Serialize(s);
            ent->InitializeObject();

            BOOL bDeactivatePhysics;
            s << bDeactivatePhysics;

            if(ent->phyObject && bDeactivatePhysics)
                ent->phyObject->Deactivate();
        }

        return ent;
    }

    return NULL;

    traceOut;
}

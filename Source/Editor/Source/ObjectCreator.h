/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ObjectCreator.cpp

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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#pragma once



enum PrefabType
{
    PrefabYAlign,
    PrefabFloor,
    PrefabDetail
};

enum EntityType
{
    EntityYAlign,
    EntityDetail
};


class ObjectCreator : public Object
{
    DeclareClass(ObjectCreator, Object);

protected:
    bool bEnabled;

public:
    ObjectCreator() : bEnabled(true)    {}
    virtual ~ObjectCreator()            {}

    virtual bool  Create(EditorViewport* vp, const Vect &startPosition)=0;
    virtual bool  Created()=0;

    virtual void  MouseMove(EditorViewport* vp, const Vect &inputPosition)=0;
    virtual bool  MouseDown(DWORD button, const Vect &inputPosition)=0;
    virtual bool  MouseUp(DWORD button, const Vect &inputPosition)=0;

    virtual void  SetStatusString()=0;

    virtual bool  ProcessKeyStroke(unsigned int kbc, bool bKeydown) {return false;}

    virtual void  SetEnabled(bool bEnabledIn) {bEnabled = bEnabledIn;}

    virtual void  Render() {}

    virtual bool  CanIgnoreViewport(ViewportType vpType)=0;
};

class BoxPrimitive : public ObjectCreator
{
    DeclareClass(BoxPrimitive, ObjectCreator);

public:
    ViewportType    curAxis;

    Vect            *PointList;
    VertexBuffer    *vertBuffer;
    IndexBuffer     *idxBuffer;

    int             curMode;
    float           curHeightOffset;

    Vect            curMousePos;

    BoxPrimitive();
    ~BoxPrimitive();

    bool  Create(EditorViewport* vp, const Vect &startPosition);
    bool  Created() {return curMode > -1;}

    void  MouseMove(EditorViewport* vp, const Vect &inputPosition);
    bool  MouseDown(DWORD button, const Vect &inputPosition);
    bool  MouseUp(DWORD button, const Vect &inputPosition);

    void  SetStatusString();

    void  Render();

    void  SaveMesh();

    bool  CanIgnoreViewport(ViewportType vpType) {return vpType == ViewportType_Main;}
};

class FloorPrimitive : public BoxPrimitive
{
    DeclareClass(FloorPrimitive, BoxPrimitive);

public:
    bool  Create(EditorViewport* vp, const Vect &startPosition);
    bool  Created() {return curMode > -1;}

    void  MouseMove(EditorViewport* vp, const Vect &inputPosition);
    bool  MouseDown(DWORD button, const Vect &inputPosition) {return true;}
    bool  MouseUp(DWORD button, const Vect &inputPosition);

    void  SetStatusString();

    bool  ProcessKeyStroke(unsigned int kbc, bool bKeydown);

    bool  CanIgnoreViewport(ViewportType vpType) {return (vpType != ViewportType_Main) && (vpType != ViewportType_Top);}
};

class PlanePrimitive : public ObjectCreator
{
    DeclareClass(PlanePrimitive, ObjectCreator);

public:
    PlanePrimitive();
    ~PlanePrimitive();

    ViewportType    curAxis;

    Vect            *PointList;
    VertexBuffer    *vertBuffer;
    IndexBuffer     *idxBuffer;

    int             curMode;

    Vect            curMousePos;

    bool  Create(EditorViewport* vp, const Vect &startPosition);
    bool  Created() {return curMode > -1;}

    void  MouseMove(EditorViewport* vp, const Vect &inputPosition);
    bool  MouseDown(DWORD button, const Vect &inputPosition) {return true;}
    bool  MouseUp(DWORD button, const Vect &inputPosition);

    void  SetStatusString();

    void  Render();

    void  SaveMesh();

    bool  ProcessKeyStroke(unsigned int kbc, bool bKeydown);

    bool  CanIgnoreViewport(ViewportType vpType) {return false;}
};

class EntityPlacer : public ObjectCreator
{
    DeclareClass(EntityPlacer, ObjectCreator);

public:
    EntityPlacer() {}
    ~EntityPlacer();

    Entity *desiredEnt;

    EditMode prevEditMode;
    EntityType entityType;

    EditorViewport *lastViewport;

    Quat curRot;

    EntityPlacer(EditMode lastEditMode, EntityType entityTypeIn);

    bool  Create(EditorViewport* vp, const Vect &startPosition);
    bool  Created() {return false;}

    void  MouseMove(EditorViewport* vp, const Vect &inputPosition);
    bool  MouseDown(DWORD button, const Vect &inputPosition);
    bool  MouseUp(DWORD button, const Vect &inputPosition) {return true;}

    void  SetStatusString();

    void  SetEnabled(bool bEnabledIn);

    bool  ProcessKeyStroke(unsigned int kbc, bool bKeydown);

    bool  CanIgnoreViewport(ViewportType vpType) {return false;}
};

class PrefabPlacer : public ObjectCreator
{
    DeclareClass(PrefabPlacer, ObjectCreator);

public:
    PrefabPlacer() {}

    MeshEntity *desiredEnt;

    EditMode prevEditMode;
    PrefabType prefabType;

    EditorViewport *lastViewport;

    Vect meshOffset;

    Quat curRot;

    PrefabPlacer(EditMode lastEditMode, PrefabType prefabTypeIn);
    ~PrefabPlacer();

    bool  Create(EditorViewport* vp, const Vect &startPosition);
    bool  Created() {return false;}

    void  MouseMove(EditorViewport* vp, const Vect &inputPosition);
    bool  MouseDown(DWORD button, const Vect &inputPosition);
    bool  MouseUp(DWORD button, const Vect &inputPosition) {return true;}

    void  SetStatusString();

    void  SetEnabled(bool bEnabledIn);

    bool  ProcessKeyStroke(unsigned int kbc, bool bKeydown);

    bool  CanIgnoreViewport(ViewportType vpType) {return (vpType != ViewportType_Main);}
};


class YPlaneAdjuster : public ObjectCreator
{
    DeclareClass(YPlaneAdjuster, ObjectCreator);

public:
    YPlaneAdjuster();

    bool bStarted;

    EditMode prevEditMode;

    bool bRealTimeWasOn;

    int startX, startY;

    float lastPos;
    float curPos, realPos;

    bool  Create(EditorViewport* vp, const Vect &startPosition);
    bool  Created() {return false;}

    void  MouseMove(EditorViewport* vp, const Vect &inputPosition);
    bool  MouseDown(DWORD button, const Vect &inputPosition) {return true;}
    bool  MouseUp(DWORD button, const Vect &inputPosition);

    bool  ProcessKeyStroke(unsigned int kbc, bool bKeydown);

    void  SetStatusString();

    bool  CanIgnoreViewport(ViewportType vpType) {return false;}
};


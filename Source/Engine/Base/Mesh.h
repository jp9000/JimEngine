/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Mesh.h

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

#ifndef MESH_HEADER
#define MESH_HEADER



#define ANIMFILE_VER    0x110
#define MODELFILE_VER   0x110



//-----------------------------------------
// Draw Section
struct DrawSection
{
    DWORD   startFace;
    DWORD   numFaces;
};


//-----------------------------------------
// Bone Section
struct AnimSubSection
{
    BYTE    numBones;
    WORD    bones[4];
    DWORD   startFace;
    DWORD   numFaces;
};


//-----------------------------------------
// Animated Section
struct AnimSection
{
    List<AnimSubSection> SubSections;

    inline void FreeData() {SubSections.Clear();}
};


/*=========================================================
    Animation Data
==========================================================*/

//-----------------------------------------
// Sequence
struct MeshSequence
{
    String strName;
    DWORD nFrames;
    float fKeyframeTime;
    BYTE flags;

    friend inline Serializer& operator<<(Serializer &s, MeshSequence &ms)
    {
        char name[128];
        s.Serialize(name, 128);
        ms.strName = name;

        DWORD millisecondKeyframeTime;

        if(!s.IsLoading()) millisecondKeyframeTime = DWORD(ms.fKeyframeTime*1000.0f);

        s << ms.nFrames << millisecondKeyframeTime << ms.flags;

        if(s.IsLoading())  ms.fKeyframeTime = float(millisecondKeyframeTime)*0.001f;

        return s;
    }
};


//-----------------------------------------
// Sequence Info
struct SeqKeys
{
    BYTE                hasRotKeys;
    BYTE                hasPosKeys;

    Vect                *lpPosKeys;
    Vect                *lpPosTans;

    Quat                *lpRotKeys;
    Quat                *lpRotTans;
};


//-----------------------------------------
// Vertex Weight
struct VWeight
{
    DWORD  vert;
    float weight;
};


//-----------------------------------------
// flags
#define BONE_ROOT       1

//-----------------------------------------
// BoneExtension (Guns, etc)
struct BoneExtension
{
    DWORD               idParent;
    Vect                Pos;
    Quat                Rot;

    friend static Serializer& operator<<(Serializer &s, BoneExtension &extension)
    {
        return s << extension.idParent << extension.Pos << extension.Rot;
    }
};


//-----------------------------------------
// Bone
struct Bone
{
public:

    //Variables
    Vect                Pos;
    Quat                Rot;
    BYTE                flags;
    DWORD               idParent;
    Vect                LocalPos;    //its position in relation to the parent
    List<VWeight>       Weights;
    List<SeqKeys>       seqKeys;

    Bone                *Parent;
    List<Bone*>         Children;
};



/*=========================================================
    Mesh
==========================================================*/

struct NameStruct
{
    char name[128];
};

struct BASE_EXPORT Mesh
{
    friend class EditorLevelInfo;

    DWORD               nVerts;
    DWORD               nFaces;
    DWORD               nEdges;
    DWORD               nSections;
    BOOL                isStatic;
    BOOL                bHasAnimation;

    String              strName;

    VertexBuffer        *VertBuffer;
    Edge                *EdgeList;
    IndexBuffer         *IdxBuffer;
    Face                *FaceList;
    DrawSection         *SectionList;
    Bounds              bounds;

    List<NameStruct>    DefaultMaterialList;

    PhyShape            *meshShape;

    //wireframe
    IndexBuffer         *WireframeBuffer;

    Mesh();
    virtual ~Mesh();

    virtual void LoadMesh(CTSTR lpName/*, BOOL bMeshList*/);
    virtual void UnloadMesh();

    virtual void SaveMeshFile();

    virtual PhyShape* GetShape(BOOL bDynamic=FALSE);


    /*static inline void ENGINEAPI TerminateMeshEngine()
    {
        DWORD numMeshes = Mesh::MeshList.Num();
        for(DWORD i=0; i<numMeshes; i++)
            delete Mesh::MeshList[0];
        Mesh::MeshList.Clear();
    }*/

    //static Mesh* ENGINEAPI GetMesh(CTSTR lpName);

protected:

    unsigned int index;
    //static List<Mesh*> MeshList;
};



/*=========================================================
    SkinMesh
==========================================================*/

struct BASE_EXPORT SkinMesh : public Mesh
{
    List<MeshSequence>   SequenceList;
    List<Bone>           BoneList;

    StringList           BoneExtensionNames;
    List<BoneExtension>  BoneExtensions;

    List<AnimSection>    AnimatedSections;

    virtual ~SkinMesh();

    void LoadMesh(CTSTR lpName/*, BOOL bMeshList*/)
    {
        Mesh::LoadMesh(lpName/*, bMeshList*/);
        isStatic = 0;
        LoadAnimations(lpName);
    }

    virtual void LoadAnimations(CTSTR lpName);
    void SaveAnimations();

    virtual void UnloadAnimations();
};




#endif

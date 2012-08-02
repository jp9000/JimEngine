/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditorBrush.h

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



enum BrushType
{
    BrushType_WorkBrush,
    BrushType_Addition,
    BrushType_Subtraction
};

inline Serializer& operator<<(Serializer &s, BrushType &brushType)
{
    s.Serialize(&brushType, sizeof(BrushType));
    return s;
}


//------------------------------------------
// Editable Face

struct EditFace
{
    Face    pointFace;
    Face    uvFace;
    Face    normalFace;

    DWORD   smoothFlags;
    DWORD   polyID;

    bool    edgeVisible[3];

    bool    FacesConnect(const EditFace &face2, DWORD &e1, DWORD &e2) const;
};

//------------------------------------------
// Editable Brush

class EditorBrush : public EditorObject
{
    DeclareClass(EditorBrush, EditorObject);

public:
    EditorBrush();
    ~EditorBrush();

    void Clear();

    /*----------------------
      Mesh Data
    -----------------------*/
    List<EditFace>      FaceList;
    List<Vect>          PointList;
    List<Vect>          NormalList;
    List<UVCoord>       UVList;
    List<SimpleEdge>    VisibleEdgeList;

    List<Face>          LightmapFaces;
    List<UVCoord>       LightmapUVs;

    List<PolyFace>      PolyList;
    List<Material*>     Materials;

    EditorMesh          mesh;
    long                brushID;

    List<Vect>          FaceNormals;

    BrushType           brushType;

    BOOL                bUseLightmapping;
    int                 lightmapResolution;

    BOOL                bCanSubtract;

    Bounds              bounds;


    void PreRender()            {}
    void Render()               {}
    void PostRender()           {}

    void PreFrame()             {}
    void Tick(float fSeconds)   {}

    Bounds GetBounds()          {return bounds;}

    void EditorRender() {}


    void ProcessBasicMeshData();
    void RebuildVisibleEdges();
    void RebuildFaceNormals();
    void RebuildNormals();
    void MergeAllDuplicateVertices();

    void RebuildBounds();

    void BuildEditorMesh(bool bTransform);

    void SubtractBrush(BOOL bRebuilding=FALSE);

    void AddSegmentedGeometry();
    void AddGeometry(BOOL bRebuilding=FALSE);
    void SubtractGeometry();

    void InvertBrush();

    void UpdateLevelBrush();
    void UpdateTangents();
    void UpdateNormals(BOOL bUpdateLevelBrush=TRUE);
    void AddLevelBrush();
    void RemoveLevelBrush(int removeBrushID=-1, int removeBrushType=-1);

    void BuildLightmapUVs();

    BOOL CanSelect(const Vect &rayOrig, const Vect &rayDir);

    void CopyFromMesh();

    Brush* GetLevelBrush();

    Entity* DuplicateEntity();

    void ReorderMaterialPolys();

    void SelectAllFaces();

    static void ENGINEAPI UndoRedoSubtract(Serializer &s, Serializer &sOut, BOOL bUndo);
    static void ENGINEAPI UndoRedoAddGeometry(Serializer &s, Serializer &sOut, BOOL bUndo);
    static void ENGINEAPI UndoRedoAddSegmented(Serializer &s, Serializer &sOut, BOOL bUndo);
    static void ENGINEAPI UndoRedoSubtractGeometry(Serializer &s, Serializer &sOut, BOOL bUndo);

    //-----------------------
    // Selection
    //-----------------------

    void RebuildSelectionIndices();

    List<DWORD>         TempSelectedPolys;

    List<DWORD>         SelectedPolys;
    IndexBuffer         *SelectionIdxBuffer;

    //-----------------------
    // Wireframe Buffers
    //-----------------------
    void RebuildWireframeBuffers();
    void FreeWireframeBuffers();

    VertexBuffer        *WFVertBuffer;
    IndexBuffer         *WFIdxBuffer;

    //-----------------------
    // Serialization
    //-----------------------
    void SerializeUVData(Serializer &s);
    void SerializeBareBrush(Serializer &s);
    void SerializeBrushData(Serializer &s, BOOL bUndoRedoData);
    void Serialize(Serializer &s);
};


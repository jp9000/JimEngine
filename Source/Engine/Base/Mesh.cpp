/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Mesh.cpp:  Mesh Processing Engine

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

void Mesh::LoadMesh(CTSTR lpName/*, BOOL bMeshList*/)
{
    traceIn(Mesh::LoadMesh);

    assert(lpName);
    DWORD i;

    isStatic = 1;

    /*if(bMeshList)
        MeshList << this;*/

    String strFilePath;
    if(!Engine::ConvertResourceName(lpName, TEXT("models"), strFilePath))
        return;

    //-----------------------------------------------

    XFileInputSerializer modelData;

    if(!modelData.Open(strFilePath))
    {
        AppWarning(TEXT("Could not open model file \"%s\""), (TSTR)strFilePath);
        return;
    }

    DWORD ver;
    modelData << ver;

    if(ver != MODELFILE_VER && ver != 0x100)
    {
        AppWarning(TEXT("'%s': Bad model file version"), strFilePath);
        modelData.Close();
        return;
    }

    strName = lpName;

    //-----------------------------------------------

    VBData *vbData = new VBData;

    vbData->TVList.SetSize(1);
    vbData->TVList[0].SetWidth(2);

    List<UVCoord> LMCoords;
    TVertList tv;

    Vect::SerializeList(modelData, vbData->VertList);//modelData << vbData->VertList;
    Vect::SerializeList(modelData, vbData->NormalList);//modelData << vbData->NormalList;
    modelData << *vbData->TVList[0].GetV2();

    if(ver == 0x100) //remove
    {
        modelData << LMCoords;
        if(LMCoords.Num())
        {
            vbData->TVList.SetSize(2);
            vbData->TVList[1].SetWidth(2);
            vbData->TVList[1].GetV2()->TransferFrom(LMCoords);
        }
    }
    else
    {
        modelData << tv;
        if(tv.Num())
        {
            vbData->TVList.SetSize(2);
            vbData->TVList[1].TransferFrom(tv);
        }
    }

    Vect::SerializeList(modelData, vbData->TangentList);//modelData << vbData->TangentList;

    nVerts = vbData->VertList.Num();

    VertBuffer = CreateVertexBuffer(vbData, FALSE);

    //-----------------------------------------------

    modelData << nFaces;
    FaceList = (Face*)Allocate(nFaces*sizeof(Face));
    modelData.Serialize(FaceList, nFaces*sizeof(Face));

    IdxBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, FaceList, nFaces*3);

    //-----------------------------------------------

    modelData << nEdges;
    EdgeList = (Edge*)Allocate(nEdges*sizeof(Edge));
    modelData.Serialize(EdgeList, nEdges*sizeof(Edge));

    //-----------------------------------------------

    modelData << DefaultMaterialList;

    modelData << nSections;
    SectionList = (DrawSection*)Allocate(nSections*sizeof(DrawSection));
    modelData.Serialize(SectionList, nSections*sizeof(DrawSection));

    //-----------------------------------------------

    modelData << bounds;

    modelData.Close();

    //-----------------------------------------------

    if(engine->InEditor())
    {
        DWORD *wireframeIndices = (DWORD*)Allocate(nEdges*2*sizeof(DWORD));

        for(i=0; i<nEdges; i++)
        {
            DWORD curPos = i*2;
            wireframeIndices[curPos]   = EdgeList[i].v1;
            wireframeIndices[curPos+1] = EdgeList[i].v2;
        }

        WireframeBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, wireframeIndices, nEdges*2);
    }

    traceOut;
}

void Mesh::SaveMeshFile()
{
    traceIn(Mesh::SaveMeshFile);

    String path;
    Engine::ConvertResourceName(strName, TEXT("models"), path, FALSE);

    XFileOutputSerializer modelFile;
    if(!modelFile.Open(path, XFILE_CREATEALWAYS))
    {
        AppWarning(TEXT("Could not create model file '%s'"), path);
        return;
    }

    VBData *vbd = VertBuffer->GetData();

    DWORD temp = MODELFILE_VER;
    modelFile << temp;

    Vect::SerializeList(modelFile, vbd->VertList);
    Vect::SerializeList(modelFile, vbd->NormalList);
    modelFile << vbd->TVList[0];
    if(vbd->TVList.Num() < 2)
    {
        temp = 0;
        modelFile << temp;
    }
    else
        modelFile << vbd->TVList[1];
    Vect::SerializeList(modelFile, vbd->TangentList);

    modelFile << nFaces;
    modelFile.Serialize(FaceList, nFaces*sizeof(Face));

    modelFile << nEdges;
    if(nEdges) modelFile.Serialize(EdgeList, nEdges*sizeof(Edge));

    modelFile << DefaultMaterialList;

    modelFile << nSections;
    modelFile.Serialize(SectionList, nSections*sizeof(DrawSection));

    modelFile << bounds;
    modelFile.Close();

    traceOut;
}

void Mesh::UnloadMesh()
{
    traceIn(Mesh::UnloadMesh);

    Free(EdgeList);

    delete meshShape;

    delete VertBuffer;
    delete IdxBuffer;
    delete WireframeBuffer;

    delete SectionList;
    DefaultMaterialList.Clear();

    traceOut;
}

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    traceIn(Mesh::~Mesh);

    UnloadMesh();

    traceOut;
}

/*Mesh* Mesh::GetMesh(CTSTR lpName)
{
    traceIn(Mesh::GetMesh);

    for(DWORD i=0;i<MeshList.Num();i++)
    {
        if(MeshList[i]->strName.CompareI(lpName))
            return MeshList[i];
    }

    String strMeshPath;
    if(!Engine::ConvertResourceName(lpName, TEXT("models"), strMeshPath))
        return NULL;

    String strAnimPath;
    strAnimPath << GetPathWithoutExtension(strMeshPath) << TEXT(".xan");

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(strAnimPath, ofd);
    BOOL bHasAnimation = (hFind != NULL);
    if(hFind) OSFindClose(hFind);

    Mesh *newMesh = (bHasAnimation) ? new SkinMesh : new Mesh;
    newMesh->LoadMesh(lpName, TRUE);
    newMesh->bHasAnimation = bHasAnimation;

    return newMesh;

    traceOut;
}*/


PhyShape *Mesh::GetShape(BOOL bDynamic)
{
    traceIn(Mesh::GetShape);

    if(!meshShape)
    {
        if(bDynamic)
            meshShape = physics->MakeDynamicMeshShape(VertBuffer->GetData()->VertList.Array(), nVerts, (UINT*)IdxBuffer->GetData(), nFaces*3);
        else
            meshShape = physics->MakeStaticMeshShape(VertBuffer->GetData()->VertList.Array(), nVerts, (UINT*)IdxBuffer->GetData(), nFaces*3);
    }

    meshShape->AddReference();
    return meshShape;

    traceOut;
}

struct VertAnimInfo
{
    List<UINT>  bones;
    List<float> weights;

    List<UINT>  sections;
    List<UINT>  sectionVertIDs;

    inline void FreeData() {bones.Clear(); weights.Clear(); sections.Clear(); sectionVertIDs.Clear();}
};

struct TriBoneInfo
{
    List<UINT> bones;

    inline void FreeData() {bones.Clear();}
};

struct SubSectionInfo
{
    UINT section;
    List<UINT> bones;
    List<UINT> tris;

    inline void FreeData() {bones.Clear(); tris.Clear();}
};

void SkinMesh::LoadAnimations(CTSTR lpName)
{
    traceIn(SkinMesh::LoadAnimations);

    assert(lpName);

    DWORD i, j, k;

    String strMeshPath;
    if(!Engine::ConvertResourceName(lpName, TEXT("models"), strMeshPath))
        return;

    String strFilePath;
    strFilePath << GetPathWithoutExtension(strMeshPath) << TEXT(".xan");


    //-----------------------------------------------

    XFileInputSerializer animData;

    if(!animData.Open(strFilePath))
    {
        AppWarning(TEXT("Could not open animation file \"%s\""), (TSTR)strFilePath);
        return;
    }

    DWORD ver;
    animData << ver;

    if(ver != ANIMFILE_VER && ver != 0x100)
    {
        AppWarning(TEXT("'%s': Bad animation file version"), strFilePath);
        animData.Close();
        return;
    }

    //-----------------------------------------------

    unsigned int numSequences;
    animData << numSequences;
    SequenceList.SetSize(numSequences);
    for(i=0; i<numSequences; i++)
        animData << SequenceList[i];

    DWORD nBones;
    animData << nBones;
    BoneList.SetSize(nBones);
    for(i=0; i<nBones; i++)
    {
        Bone &bone = BoneList[i];

        List<DWORD>     RigidVerts;
        List<VWeight>   BlendedVerts;
        DWORD           nRigid, nBlended;

        //----------------------------

        animData << bone.Pos
                 << bone.Rot
                 << nRigid
                 << nBlended
                 << bone.flags
                 << bone.idParent;

        animData << RigidVerts;
        animData << BlendedVerts;

        if(bone.idParent != 0xFFFFFFFF)
        {
            bone.Parent = BoneList+bone.idParent;
            bone.LocalPos = bone.Pos - BoneList[bone.idParent].Pos;
            BoneList[bone.idParent].Children << &bone;
        }
        else
        {
            bone.Parent = NULL;
            bone.LocalPos = bone.Pos;

            bone.flags |= BONE_ROOT;
        }

        //----------------------------

        bone.Weights.SetSize(nRigid);

        for(j=0; j<RigidVerts.Num(); j++)
        {
            VWeight &weight = bone.Weights[j];

            weight.vert = RigidVerts[j];
            weight.weight = 1.0f;
        }

        bone.Weights.AppendList(BlendedVerts);

        //----------------------------

        DWORD nAnim;
        animData << nAnim;
        bone.seqKeys.SetSize(nAnim);
        for(j=0; j<nAnim; j++)
        {
            SeqKeys &keys = bone.seqKeys[j];

            DWORD nPosKeys, nRotKeys;

            //----------------------------
            
            animData << nRotKeys;
            keys.hasRotKeys = nRotKeys > 0;
            if(keys.hasRotKeys)
            {
                keys.lpRotKeys = (Quat*)Allocate(nRotKeys*sizeof(Quat));
                animData.Serialize(keys.lpRotKeys, nRotKeys*sizeof(Quat));

                keys.lpRotTans = (Quat*)Allocate(nRotKeys*sizeof(Quat));
                for(k=0; k<nRotKeys; k++)
                {
                    DWORD kp1 = (k == (nRotKeys-1)) ? k : (k+1);
                    DWORD km1 = (k == 0) ? 0 : k-1;

                    Quat &qk   = keys.lpRotKeys[k];
                    Quat &qkp1 = keys.lpRotKeys[kp1];
                    Quat &qkm1 = keys.lpRotKeys[km1];

                    keys.lpRotTans[k] = qk.GetInterpolationTangent(qkm1, qkp1);
                }
            }

            //----------------------------
            
            animData << nPosKeys;
            keys.hasPosKeys = nPosKeys > 0;
            if(keys.hasPosKeys)
            {
                keys.lpPosKeys = (Vect*)Allocate(nPosKeys*sizeof(Vect));
                Vect::SerializeArray(animData, keys.lpPosKeys, nPosKeys);
                
                keys.lpPosTans = (Vect*)Allocate(nPosKeys*sizeof(Vect));
                for(k=0; k<nPosKeys; k++)
                {
                    DWORD kp1 = (k == (nPosKeys-1)) ? k : (k+1);
                    DWORD km1 = (k == 0) ? 0 : k-1;

                    Vect &pk   = keys.lpPosKeys[k];
                    Vect &pkp1 = keys.lpPosKeys[kp1];
                    Vect &pkm1 = keys.lpPosKeys[km1];

                    keys.lpPosTans[k] = pk.GetInterpolationTangent(pkm1, pkp1);
                }
            }
        }
    }

    //-----------------------------------------------

    int num;
    animData << num;
    BoneExtensions.SetSize(num);
    BoneExtensionNames.SetSize(num);

    for(int i=0; i<num; i++)
    {
        animData << BoneExtensionNames[i];
        animData << BoneExtensions[i];
    }

    //-----------------------------------------------

    AnimatedSections.SetSize(nSections);

    if(ver == 0x100) //remove
    {
        UINT *indices = (UINT*)IdxBuffer->GetData();

        List<UINT> adjustedIndices;
        adjustedIndices.CopyArray(indices, IdxBuffer->NumIndices());

        VBData *vbd = VertBuffer->GetData();
        List<Vect> &verts = vbd->VertList;

        //--------- 
        // get vert data
        List<VertAnimInfo> vertInfo;
        vertInfo.SetSize(nVerts);

        for(int i=0; i<nBones; i++)
        {
            Bone &bone = BoneList[i];
            for(int j=0; j<bone.Weights.Num(); j++)
            {
                VWeight &vertWeight = bone.Weights[j];

                if(!vertInfo[vertWeight.vert].bones.HasValue(i))
                {
                    vertInfo[vertWeight.vert].bones << i;
                    vertInfo[vertWeight.vert].weights << vertWeight.weight;
                }
            }
        }

        //--------- 
        // remove excess bone influence from verts (let just set the max to 3 for this)
        /*for(int i=0; i<vertInfo.Num(); i++)
        {
            VertAnimInfo &vert = vertInfo[i];

            while(vert.bones.Num() > 3)
            {
                float weakestWeight = M_INFINITE;
                UINT weakestID;

                for(int j=0; j<vert.bones.Num(); j++)
                {
                    if(vert.weights[j] < weakestWeight)
                    {
                        weakestID = j;
                        weakestWeight = vert.weights[j];
                    }
                }

                float weightAdjust = 1.0f/(1.0f-weakestWeight);
                vert.weights.Remove(weakestID);
                vert.bones.Remove(weakestID);

                for(int j=0; j<vert.weights.Num(); j++)
                    vert.weights[j] *= weightAdjust;
            }
        }*/
        for(int i=0; i<vertInfo.Num(); i++)
        {
            VertAnimInfo &vert = vertInfo[i];

            for(int j=0; j<vert.bones.Num(); j++)
            {
                if(vert.weights[j] <= 0.15f)
                {
                    float weightAdjust = 1.0f/(1.0f-vert.weights[j]);
                    vert.weights.Remove(j);
                    vert.bones.Remove(j);

                    for(int k=0; k<vert.weights.Num(); k++)
                        vert.weights[k] *= weightAdjust;

                    --j;
                }
            }
        }

        //--------- 
        // remove excess bone influence from tris (can only have 4 bones influencing any triangle)
        for(int i=0; i<nSections; i++)
        {
            DrawSection &section = SectionList[i];
            if(!section.numFaces) continue;

            for(int j=0; j<section.numFaces; j++)
            {
                UINT *triVertIDs = &indices[(section.startFace+j)*3];

                List<UINT> bones;
                List<float> bestVertWeights;

                for(int k=0; k<3; k++)
                {
                    VertAnimInfo &info = vertInfo[triVertIDs[k]];

                    for(int l=0; l<info.bones.Num(); l++)
                    {
                        UINT id = bones.FindValueIndex(info.bones[l]);
                        if(id == INVALID)
                        {
                            bones.Add(info.bones[l]);
                            bestVertWeights.Add(info.weights[l]);
                        }
                        else
                            bestVertWeights[id] = MAX(bestVertWeights[id], info.weights[l]);
                    }
                }

                while(bones.Num() > 4)
                {
                    int removeBone, removeBoneID;
                    float bestWeight = M_INFINITE;

                    for(int k=0; k<bones.Num(); k++)
                    {
                        if(bestVertWeights[k] < bestWeight)
                        {
                            removeBone = bones[k];
                            removeBoneID = k;
                            bestWeight = bestVertWeights[k];
                        }
                    }

                    for(int k=0; k<3; k++)
                    {
                        VertAnimInfo &info = vertInfo[triVertIDs[k]];
                        UINT id = info.bones.FindValueIndex(removeBone);
                        if(id == INVALID) continue;

                        float weightAdjust = 1.0f/(1.0f-info.weights[id]);
                        info.weights.Remove(id);
                        info.bones.Remove(id);

                        for(int l=0; l<info.weights.Num(); l++)
                            info.weights[l] *= weightAdjust;
                    }

                    bones.Remove(removeBoneID);
                    bestVertWeights.Remove(removeBoneID);
                }
            }
        }

        //--------- 
        // sort out sections of triangles that are influenced up to a max of 4 bones
        // also, duplicate shared verts
        VBData *newVBD = new VBData;
        newVBD->CopyList(*vbd);

        newVBD->TVList.SetSize(2);
        newVBD->TVList[1].SetWidth(4);
        newVBD->TVList[1].SetSize(nVerts);

        List<SubSectionInfo> newSubSections;

        for(int i=0; i<nSections; i++)
        {
            List<TriBoneInfo> triInfo;

            DrawSection &section = SectionList[i];
            if(!section.numFaces) continue;

            for(int j=0; j<section.numFaces; j++)
            {
                UINT *triVertIDs = &indices[(section.startFace+j)*3];

                TriBoneInfo &newTri = *triInfo.CreateNew();

                for(int k=0; k<3; k++)
                {
                    VertAnimInfo &info = vertInfo[triVertIDs[k]];

                    for(int l=0; l<info.bones.Num(); l++)
                        newTri.bones.SafeAdd(info.bones[l]);
                }
            }

            BitList UsedTris;
            UsedTris.SetSize(section.numFaces);
            DWORD nUsedTris = 0;

            while(nUsedTris != section.numFaces)
            {
                DWORD triSectionID = newSubSections.Num();
                SubSectionInfo *curSubSecInfo = newSubSections.CreateNew();
                curSubSecInfo->section = i;

                for(int j=0; j<triInfo.Num(); j++)
                {
                    if(UsedTris[j]) continue;

                    TriBoneInfo &tri = triInfo[j];

                    List<UINT> secBones;
                    secBones.CopyList(curSubSecInfo->bones);

                    BOOL bBadTri = FALSE;
                    for(int k=0; k<tri.bones.Num(); k++)
                    {
                        secBones.SafeAdd(tri.bones[k]);
                        if(secBones.Num() > 4)
                            break;
                    }

                    if(secBones.Num() > 4) continue;

                    DWORD triID = section.startFace+j;

                    curSubSecInfo->bones.CopyList(secBones);
                    curSubSecInfo->tris << triID;

                    UINT *triVertIDs = &indices[triID*3];
                    for(int k=0; k<3; k++)
                    {
                        VertAnimInfo *info = &vertInfo[triVertIDs[k]];
                        UINT id = info->sections.FindValueIndex(triSectionID);
                        if(id == INVALID)
                        {
                            UINT vertID;
                            if(info->sections.Num() >= 1) //duplicate vertex
                            {
                                vertID = newVBD->DuplicateVertex(triVertIDs[k]);
                                adjustedIndices[(triID*3)+k] = vertID;

                                VertAnimInfo &newVertInfo = *vertInfo.CreateNew();
                                info = &vertInfo[triVertIDs[k]]; //reset pointer

                                newVertInfo.bones.CopyList(info->bones);
                                newVertInfo.weights.CopyList(info->weights);
                                newVertInfo.sections << triSectionID;
                            }
                            else
                                vertID = triVertIDs[k];

                            info->sections << triSectionID;
                            info->sectionVertIDs << vertID;

                            List<Vect4> &vertWeights = *newVBD->TVList[1].GetV4();
                            for(int l=0; l<4; l++)
                            {
                                if(l >= curSubSecInfo->bones.Num())
                                    vertWeights[vertID].ptr[l] = 0.0f;
                                else
                                {
                                    UINT boneID = curSubSecInfo->bones[l];
                                    UINT weightID = info->bones.FindValueIndex(boneID);
                                    if(weightID == INVALID)
                                        vertWeights[vertID].ptr[l] = 0.0f;
                                    else
                                        vertWeights[vertID].ptr[l] = info->weights[weightID];
                                }
                            }
                        }
                        else
                            adjustedIndices[(triID*3)+k] = info->sectionVertIDs[id];
                    }

                    UsedTris.Set(j);
                    ++nUsedTris;
                }
            }
            for(int j=0; j<triInfo.Num(); j++)
                triInfo[j].FreeData();
        }

        //---------
        // create animated draw sections and create new index buffer
        DWORD curTriID = 0;
        List<UINT> newIndices;

        for(int i=0; i<newSubSections.Num(); i++)
        {
            SubSectionInfo &subSecInfo = newSubSections[i];
            AnimSection &animSection = AnimatedSections[subSecInfo.section];
            AnimSubSection &subSection = *animSection.SubSections.CreateNew();

            subSection.numBones = subSecInfo.bones.Num();
            for(int j=0; j<subSecInfo.bones.Num(); j++)
                subSection.bones[j] = subSecInfo.bones[j];

            subSection.startFace = curTriID;

            for(int j=0; j<subSecInfo.tris.Num(); j++)
            {
                UINT *tri = &adjustedIndices[subSecInfo.tris[j]*3];
                newIndices << tri[0] << tri[1] << tri[2];

                ++curTriID;
            }

            subSection.numFaces = curTriID-subSection.startFace;

            subSecInfo.FreeData();
        }

        //rebuild original bone data
        for(int i=0; i<BoneList.Num(); i++)
            BoneList[i].Weights.Clear();

        for(int i=0; i<vertInfo.Num(); i++)
        {
            for(int j=0; j<vertInfo[i].bones.Num(); j++)
            {
                VWeight weight;
                weight.vert = i;
                weight.weight = vertInfo[i].weights[j];

                BoneList[vertInfo[i].bones[j]].Weights << weight;
            }

            vertInfo[i].FreeData();
        }

        delete VertBuffer;
        delete IdxBuffer;

        UINT numIndices;
        UINT *indexArray;
        newIndices.TransferTo(indexArray, numIndices);

        nVerts = newVBD->VertList.Num();

        IdxBuffer = CreateIndexBuffer(GS_UNSIGNED_LONG, indexArray, numIndices);
        VertBuffer = CreateVertexBuffer(newVBD);
    }
    else
    {
        for(int i=0; i<nSections; i++)
            animData << AnimatedSections[i].SubSections;
    }

    animData.Close();

    traceOut;
}

void SkinMesh::UnloadAnimations()
{
    traceIn(SkinMesh::UnloadAnimations);

    for(int i=0; i<AnimatedSections.Num(); i++)
        AnimatedSections[i].FreeData();
    AnimatedSections.Clear();

    BoneExtensionNames.Clear();
    BoneExtensions.Clear();

    for(int i=0; i<BoneList.Num(); i++)
    {
        for(DWORD j=0;j<BoneList[i].seqKeys.Num();j++)
        {
            if(BoneList[i].seqKeys[j].lpPosKeys)
            {
                Free(BoneList[i].seqKeys[j].lpPosTans);
                Free(BoneList[i].seqKeys[j].lpPosKeys);
            }
            if(BoneList[i].seqKeys[j].lpRotKeys)
            {
                Free(BoneList[i].seqKeys[j].lpRotTans);
                Free(BoneList[i].seqKeys[j].lpRotKeys);
            }
        }
        BoneList[i].seqKeys.Clear();
        BoneList[i].Weights.Clear();
        BoneList[i].Children.Clear();
    }
    for(int i=0; i<SequenceList.Num(); i++)
        SequenceList[i].strName.Clear();
    SequenceList.Clear();
    BoneList.Clear();

    traceOut;
}

SkinMesh::~SkinMesh()
{
    traceIn(SkinMesh::~SkinMesh);

    UnloadAnimations();

    traceOut;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  xmdExp.cpp

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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "XenModelExporter.h"

#define ALMOST_ZERO    1.0e-3f

Modifier*   FindPhysiqueModifier(INode* nodePtr);
Modifier*   FindSkinModifier(INode* nodePtr);
BOOL        EqualPoint3(Point3 p1, Point3 p2);


#define CHARACTER_CLASSID Class_ID(0xCBBF6400, 0x89949900)



static xmdExpClassDesc xmdExpDesc;
ClassDesc2* GetxmdExpDesc() { return &xmdExpDesc; }



INT_PTR CALLBACK xmdExpOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
//    ISpinnerControl *spin;
//    Interval animRange;
	static xmdExp *exp = NULL;
//    DWORD step;

	switch(message)
    {
		case WM_INITDIALOG:
			exp = (xmdExp *)lParam;
			CenterWindow(hWnd,GetParent(hWnd));

            SendMessage(GetDlgItem(hWnd, IDC_UNROTATED), BM_SETCHECK, BST_CHECKED, 0);

			return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    if(SendMessage(GetDlgItem(hWnd, IDC_UNROTATED), BM_GETCHECK, 0, 0) == BST_CHECKED)
                        exp->rotation = 0.0f;
                    else if(SendMessage(GetDlgItem(hWnd, IDC_ROTATE90), BM_GETCHECK, 0, 0) == BST_CHECKED)
                        exp->rotation = 90.0f;
                    else if(SendMessage(GetDlgItem(hWnd, IDC_ROTATE180), BM_GETCHECK, 0, 0) == BST_CHECKED)
                        exp->rotation = 180.0f;
                    else if(SendMessage(GetDlgItem(hWnd, IDC_ROTATE270), BM_GETCHECK, 0, 0) == BST_CHECKED)
                        exp->rotation = 270.0f;

                    exp->SetKeyframeStep(2);
                    EndDialog(hWnd, 0);
                    break;

                case IDCANCEL:
                    EndDialog(hWnd, 1);
            }
            return TRUE;

		case WM_CLOSE:
			return TRUE;
	}
	return FALSE;
}

#define err(msg) MessageBox(NULL, msg, NULL, 0)

//--- xmdExp -------------------------------------------------------
xmdExp::xmdExp() :
    ip(NULL), hOut(NULL), tStart(0), tStop(0), bAsciiOut(0), nVerts(0)
{
    memset(&modelHead, 0, sizeof(MODELHEAD));
    memset(&animHead,  0, sizeof(ANIMHEAD));
}

xmdExp::~xmdExp() 
{
}

int	xmdExp::DoExport(const MCHAR *mbcName,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)
{
    INode *curNode;
    bHasAnimation=1;

    WStr wstrName(mbcName);
    String name = wstrName;

    ip = i;

	if(suppressPrompts)
        return 1;

    if(DialogBoxParam(hInstance, 
				MAKEINTRESOURCE(IDD_PANEL), 
				GetActiveWindow(), 
				xmdExpOptionsDlgProc, (LPARAM)this))
        return 1;


    if(!HasAnimation())
    {
        //MessageBox(NULL, "Sequencer data not found!", NULL, 0);
        bHasAnimation = 0;
        animHead.nSequences = 0;
    }

    int c;


    for(c=0; c<ip->GetRootNode()->NumberOfChildren(); c++)
    {
        curNode = ip->GetRootNode()->GetChildNode(c);
        Modifier *skinmod = NULL;

        skinmod = FindPhysiqueModifier(curNode);
        if(!skinmod)
            skinmod = FindSkinModifier(curNode);

        if(skinmod)
            SkinModifiers << skinmod;
    }

    if(bHasAnimation)
        ExportBones();

    for(c=0; c<ip->GetRootNode()->NumberOfChildren(); c++)
    {
        curNode = ip->GetRootNode()->GetChildNode(c);
        Modifier *skinmod = NULL;

        skinmod = FindPhysiqueModifier(curNode);
        if(!skinmod)
            skinmod = FindSkinModifier(curNode);

        if(skinmod)
            skinmod->DisableMod();

        if(options & SCENE_EXPORT_SELECTED)
        {
            if(curNode->Selected())
                ProcessNode(curNode);
        }
        else
            ProcessNode(curNode);

        if(skinmod)
            skinmod->EnableMod();
    }

    if(!VertList.Num())
    {
        FreeData();
        return 0;
    }

    for(c=0; c<VertList.Num(); c++)
    {
        Vect UNormal(0, 0, 0);

        for(int j=0; j<FaceList.Num(); j++)
        {
            FACE &curFace = FaceList[j];
            for(int k=0; k<3; k++)
            {
                if(curFace.ptr[k] == c)
                {
                    UNormal += TempFaceTangentUList[j];
                    break;
                }
            }
        }

        /*CompressedNormal temp;
        temp = UNormal.GetNorm();*/

        //Readjust to be orthogonal with vertex normal
        Vect vertNorm = NormalList[c];

        float tempf = UNormal|vertNorm;
        UNormal = (UNormal - (vertNorm*tempf)).Norm();

        Vect temp;
        temp = UNormal;
        VertexTangentUList << temp;


        /*Vect vertNorm = NormalList[c].GetVectNorm();
        UNormal = (vertNorm^UNormal)^vertNorm;

        VertexTangentUList << temp;*/
    }

    if(bHasAnimation)
        ExportAnimation();

    if(bAsciiOut)
    {
        String strName = name;
        strName.SetLength(strName.Length()-4) << TEXT(".txt");
        SaveAscii(strName);
    }
    else
    {
        SaveModel(name);
        if(bHasAnimation)
        {
            String strName = name;
            strName.SetLength(strName.Length()-4) << TEXT(".xan");
            SaveAnimation(strName);
        }
    }

    for(c=0;c<BoneList.Num();c++)
    {
        for(int j=0;j<BoneList[c].AnimKeyList.Num();j++)
        {
            if(BoneList[c].AnimKeyList[j])
                delete BoneList[c].AnimKeyList[j];
        }
    }

    FreeData();

	return 1;
}

void xmdExp::FreeData()
{
    VertList.Clear();
    UVList.Clear();
    VertexTangentUList.Clear();
    NormalList.Clear();
    FaceList.Clear();
    FacePlaneList.Clear();
    PlaneList.Clear();
    EdgeList.Clear();
    FaceInfo.Clear();
    MaterialList.Clear();
    namelist.Clear();
    SequenceList.Clear();
    ftlist.Clear();
    TempMaterialDataList.Clear();
    TempFaceTangentUList.Clear();
    SkinModifiers.Clear();

    for(int c=0; c<BoneList.Num(); c++)
    {
        BoneList[c].AnimKeyList.Clear();
        BoneList[c].blendedVerts.Clear();
        BoneList[c].rigidVerts.Clear();
    }
    BoneList.Clear();

    for(int c=0; c<VertSave.Num(); c++)
    {
        VertSave[c].Clear();
    }
    VertSave.Clear();

    FaceSave.Clear();
}


void xmdExp::ExportAnimation()
{
    NAMELIST names;
    FTLIST ftlist;
    BOOL pos,rot;
    AppDataChunk *chunk;
    int num,i,j;
    List<MaxQuat> rotKeys;
    List<Point3> posKeys;

    GetNames(&names);
    //GetFTData(&ftlist);

    animHead.nSequences = names.Num();

    for(i=0;i<names.Num();i++)
    {
        SEQ seq;

        seq.nFrames = 0;
        tstr_to_utf8(names[i].name, seq.name, 127);
        seq.keyframeTime = (DWORD)(((float)names[i].keyframeLength)/4.8f);

        seq.flags = (names[i].bStillAnimation) ? SEQUENCE_STILL : 0;


        for(int id=0;id<BoneList.Num();id++)
        {
            ANIM *boneAnim = new ANIM;


            CheckForAnimation(BoneList[id].node, i, pos, rot);

            if(names[i].bNoRotation && (BoneList[id].bone.flags & BONE_ROOT))
                rot = 0;

            if(rot)
            {
                chunk = GetData(BoneList[id].node, ROTNUM_ID+i);
                num = *(DWORD*)chunk->data;

                chunk = GetData(BoneList[id].node, ROTDATA_ID+i);
                rotKeys.CopyArray((MaxQuat*)chunk->data, num);

                if(!seq.nFrames)
                    seq.nFrames = rotKeys.Num();

                for(j=0; j<rotKeys.Num(); j++)
                {
                    XT::Quat q;

                    q = GetXenQuat(rotKeys[j]);

                    /*if(CloseFloat(q.x, 0.0f)) q.x = 0.0f;
                    else if(CloseFloat(q.x, 1.0f)) q.x = 1.0f;
                    if(CloseFloat(q.y, 0.0f)) q.y = 0.0f;
                    else if(CloseFloat(q.y, 1.0f)) q.y = 1.0f;
                    if(CloseFloat(q.z, 0.0f)) q.z = 0.0f;
                    else if(CloseFloat(q.z, 1.0f)) q.z = 1.0f;
                    if(CloseFloat(q.w, 0.0f)) q.w = 0.0f;
                    else if(CloseFloat(q.w, 1.0f)) q.w = 1.0f;*/
                    boneAnim->rotKey << q;
                }
            }

            if(/*(BoneList[id].bone.flags & BONE_ROOT) &&*/ pos && (!names[i].bNoXYTransform || !names[i].bNoZTransform))
            {
                chunk = GetData(BoneList[id].node, POSNUM_ID+i);
                num = *(DWORD*)chunk->data;

                chunk = GetData(BoneList[id].node, POSDATA_ID+i);
                posKeys.CopyArray((Point3*)chunk->data, num);

                if(!seq.nFrames)
                    seq.nFrames = posKeys.Num();

                for(j=0; j<posKeys.Num(); j++)
                {
                    Vect v;

                    v = GetVect(posKeys[j]);

                    //v -= BoneList[id].bone.Pos;

                    if(names[i].bNoXYTransform)
                        v.x = v.z = 0.0f;
                    if(names[i].bNoZTransform)
                        v.y = 0.0f;
                    boneAnim->posKey << v;
                }
            }

            /*if(!rot && !pos)
            {
                delete boneAnim;
                continue;
            }*/
            BoneList[id].AnimKeyList << boneAnim;
        }

        SequenceList << seq;
    }

    posKeys.Clear();
    rotKeys.Clear();
}


void xmdExp::SaveModel(CTSTR lpFile)
{
    int i;

    bounds.Max = VertList[0];

    for(i=0;i<VertList.Num();i++)
    {
        bounds.Max.x = MAX(VertList[i].x, bounds.Max.x);
        bounds.Max.y = MAX(VertList[i].y, bounds.Max.y);
        bounds.Max.z = MAX(VertList[i].z, bounds.Max.z);
    }
    memcpy(&bounds.Min, &bounds.Max, sizeof(Vect));
    for(i=0;i<VertList.Num();i++)
    {
        bounds.Min.x = MIN(VertList[i].x, bounds.Min.x);
        bounds.Min.y = MIN(VertList[i].y, bounds.Min.y);
        bounds.Min.z = MIN(VertList[i].z, bounds.Min.z);
    }

    XFileOutputSerializer modelData;

    if(!modelData.Open(lpFile, XFILE_CREATEALWAYS))
        return;

    if(rotation != 0.0f)
    {
        Matrix m = XT::AxisAngle(0.0f, 1.0f, 0.0f, rotation).GetQuat();

        for(i=0; i<VertList.Num(); i++)
        {
            VertList[i].TransformVector(m);
            NormalList[i].TransformVector(m);
            VertexTangentUList[i].TransformVector(m);
        }
    }

    //DWORD bspNodes = 0;

    DWORD versionNum = 0x00000100;
    modelData << versionNum;
    Vect::SerializeList(modelData, VertList);
    Vect::SerializeList(modelData, NormalList);
    modelData << UVList;
    modelData << LMCoords;
    Vect::SerializeList(modelData, VertexTangentUList);
    modelData << FaceList;
    //modelData << FacePlaneList;
    //modelData << PlaneList;
    //modelData << bspNodes;
    modelData << EdgeList;
    modelData << MaterialNameList;
    modelData << MaterialList;
    modelData << bounds;

    modelData.Close();


    /*modelHead.dwSigniture = '\0dmx';
    modelHead.nEdges = EdgeList.Num();
    ModelFile.Write(&modelHead, sizeof(MODELHEAD));

    //convert to 32bit

    for(i=0; i<VertList.Num(); i++)
    {
        Vect &vert = VertList[i];
        ModelFile << (float)vert.x << (float)vert.y << (float)vert.z;
    }

    ModelFile.Write(NormalList.array,         sizeof(Vect)*NormalList.Num());

    for(i=0; i<UVList.Num(); i++)
    {
        UVCoord &uv = UVList[i];
        ModelFile << (float)uv.x << (float)uv.y;
    }

    ModelFile.Write(VertexTangentUList.array, sizeof(Vect)*NormalList.Num());
    ModelFile.Write(FaceList.array,           6*FaceList.Num());
    ModelFile.Write(FaceNormalList.array,     sizeof(Vect)*FaceNormalList.Num());
    ModelFile.Write(EdgeList.array,           sizeof(EDGE)*EdgeList.Num());
    ModelFile.Write(MaterialList.array,       sizeof(MATERIAL)*MaterialList.Num());

    ModelFile << (float)bounds.Min.x << (float)bounds.Min.y << (float)bounds.Min.z;
    ModelFile << (float)bounds.Max.x << (float)bounds.Max.y << (float)bounds.Max.z;

    VertList.Clear();
    NormalList.Clear();
    UVList.Clear();
    VertexTangentUList.Clear();
    FaceList.Clear();
    FaceNormalList.Clear();
    EdgeList.Clear();
    MaterialList.Clear();*/
}

void xmdExp::SaveAnimation(CTSTR lpFile)
{
    int i;

    XFileOutputSerializer animData;

    if(!animData.Open(lpFile, XFILE_CREATEALWAYS))
        return;

    DWORD versionNum = 0x00000100;
    animData << versionNum;

    DWORD temp = SequenceList.Num();
    animData << temp;
    for(i=0; i<SequenceList.Num(); i++)
    {
        SEQ &seq = SequenceList[i];
        animData.Serialize(seq.name, 128);
        animData << seq.nFrames << seq.keyframeTime << seq.flags;
    }

    XT::Quat baseRot = XT::AxisAngle(0.0f, 1.0f, 0.0f, rotation).GetQuat();
    Matrix mRot = baseRot;

    temp = BoneList.Num();
    animData << temp;
    for(i=0;i<BoneList.Num();i++)
    {
        BONEDATA &boneData = BoneList[i];

        if(rotation != 0.0f)
        {
            if(boneData.Parent)
                boneData.bone.Pos.TransformVector(mRot);
        }

        animData << boneData.bone.Pos
                 << boneData.bone.Rot
                 << boneData.bone.nRigidVerts
                 << boneData.bone.nBlendedVerts
                 << boneData.bone.flags
                 << boneData.bone.idParent;

        animData << boneData.rigidVerts;
        animData << boneData.blendedVerts;

        temp = boneData.AnimKeyList.Num();
        animData << temp;
        for(int j=0;j<boneData.AnimKeyList.Num();j++)
        {
            ANIM *&AnimKey = boneData.AnimKeyList[j];

            for(int k=0; k<AnimKey->rotKey.Num(); k++)
            {
                Vect test(AnimKey->rotKey[k].x, AnimKey->rotKey[k].y, AnimKey->rotKey[k].z);
                test.TransformVector(mRot);
                AnimKey->rotKey[k].Set(test.x, test.y, test.z, AnimKey->rotKey[k].w);
            }

            for(int k=0; k<AnimKey->posKey.Num(); k++)
                AnimKey->posKey[k].TransformVector(mRot);

            animData << AnimKey->rotKey;
            animData << AnimKey->posKey;
        }
    }

    temp = ExtensionList.Num();
    animData << temp;
    for(int i=0; i<ExtensionList.Num(); i++)
        animData << ExtensionList[i];

    animData.Close();

    /*animHead.dwSigniture = '\0nax';
    animHead.nGuns = GunList.Num();
    AnimFile.Write(&animHead, sizeof(ANIMHEAD));

    for(i=0;i<SequenceList.Num();i++)
        AnimFile.Write(&SequenceList[i], sizeof(SEQ));

    for(i=0;i<BoneList.Num();i++)
    {
        BONEDATA &boneData = BoneList[i];

        AnimFile << (float)boneData.bone.Pos.x << (float)boneData.bone.Pos.y
                 << (float)boneData.bone.Pos.z;

        AnimFile << (float)boneData.bone.Rot.x << (float)boneData.bone.Rot.y
                 << (float)boneData.bone.Rot.z << (float)boneData.bone.Rot.w;

        AnimFile.Write(&boneData.bone.nRigidVerts, 7);

        if(boneData.rigidVerts.Num())
            AnimFile.Write(boneData.rigidVerts.array,   sizeof(DWORD)*boneData.rigidVerts.Num());
        if(boneData.blendedVerts.Num())
            AnimFile.Write(boneData.blendedVerts.array, sizeof(WEIGHT)*boneData.blendedVerts.Num());
        for(int j=0;j<boneData.AnimKeyList.Num();j++)
        {
            ANIM *&AnimKey = boneData.AnimKeyList[j];
            DWORD bHasKeys = ((AnimKey->rotKey.Num()) ? 1 : 0) | ((AnimKey->posKey.Num()) ? 256 : 0);
            AnimFile.Write(&bHasKeys, 2);
            if(AnimKey->rotKey.Num())
                AnimFile.Write(AnimKey->rotKey.array, AnimKey->rotKey.Num()*sizeof(Vect));

            for(int k=0; k<AnimKey->posKey.Num(); k++)
            {
                Vect &key = AnimKey->posKey[k];
                AnimFile << (float)key.x << (float)key.y << (float)key.z;
            }
        }
    }

    for(i=0; i<GunList.Num(); i++)
    {
        Gun &gun = GunList[i];

        AnimFile.Write(&gun.parentBone, 4);
        AnimFile << (float)gun.Pos.x << (float)gun.Pos.y << (float)gun.Pos.z;
        AnimFile << (float)gun.Rot.x << (float)gun.Rot.y << (float)gun.Rot.z << (float)gun.Rot.w;
    }*/

    for(int i=0; i<ExtensionList.Num(); i++)
        ExtensionList[i].FreeData();
    ExtensionList.Clear();

    for(i=0;i<BoneList.Num();i++)
    {
        BONEDATA &boneData = BoneList[i];

        boneData.rigidVerts.Clear();
        boneData.blendedVerts.Clear();

        for(int j=0;j<boneData.AnimKeyList.Num();j++)
        {
            ANIM *&AnimKey = boneData.AnimKeyList[j];
            AnimKey->posKey.Clear();
            AnimKey->rotKey.Clear();
        }
        boneData.AnimKeyList.Clear();
    }

    BoneList.Clear();

    SequenceList.Clear();
}

unsigned short xmdExp::FindVertex(unsigned short vert)
{
	for(unsigned short i=0;i<VertSave.Num();i++)
    {
        for(unsigned short j=0;j<VertSave[i].Num();j++)
        {
            if(VertSave[i][j].vert == vert)
                return i;
        }
    }
    return -1;
}

//testing output -- mostly used for debugging perposes
void xmdExp::SaveAscii(CTSTR lpFile)
{
    /*ModelFile << "VERTEX DATA\r\n";

    char *bla = (char*)malloc(1024);

    int i, j, k;

    for(i=0; i<VertList.Num(); i++)
    {
        sprintf(bla, "\tVERT %d  x:%.4f  y:%.4f  z:%.4f\r\n\t\tNORM x:%.4f  y:%.4f  z:%.4f\r\n\t\tCOORDS U:%.4f  V:%.4f\r\n\t\tTANGENT U VECTOR x:%.4 y:%.4 z:%.4\r\n",
            i,
            VertList[i].x,
            VertList[i].y,
            VertList[i].z,
            CharToNFloat(NormalList[i].x),
            CharToNFloat(NormalList[i].y),
            CharToNFloat(NormalList[i].z),
            UVList[i].x,
            UVList[i].y,
            CharToNFloat(VertexTangentUList[i].x),
            CharToNFloat(VertexTangentUList[i].y),
            CharToNFloat(VertexTangentUList[i].z));
        ModelFile.Write(bla, strlen(bla));
    }

    ModelFile << "\r\nFACE DATA\r\n";

    for(i=0; i<FaceList.Num(); i++)
    {
        sprintf(bla, "\tFACE %d  a:%d  b:%d  c:%d\r\n",
            i,
            FaceList[i].A,
            FaceList[i].B,
            FaceList[i].C);
        ModelFile.Write(bla, strlen(bla));
    }


    ModelFile << "\r\nMATERIAL DATA\r\n";

    for(i=0; i<MaterialList.Num(); i++)
    {
        sprintf(bla, "\tMATERIAL %d (id: %d), File: \"%s\"\r\n\t\tstartfaceID: %d\r\n\t\tnumfaces: %d\r\n",
            i,
            TempMaterialDataList[i].id,
            MaterialList[i].name,
            MaterialList[i].startFace,
            TempMaterialDataList[i].startFaceID,
            MaterialList[i].nFaces);
        ModelFile.Write(bla, strlen(bla));
    }

    ModelFile << "\r\nGUN DATA\r\n\r\n";

    for(i=0;i<GunList.Num();i++)
    {
        sprintf(bla, "\tGUN %d, Parent Bone: %d\r\n\t\tPos: x:%.4f  y:%.4f  z:%.4f\r\n\t\tRot: x:%.4f  y:%.4f  z:%.4f  z:%.4f\r\n\r\n",
            i,
            GunList[i].parentBone,
            GunList[i].Pos.x, GunList[i].Pos.y, GunList[i].Pos.z,
            GunList[i].Rot.x, GunList[i].Rot.y, GunList[i].Rot.z, GunList[i].Rot.w);
        ModelFile.Write(bla, strlen(bla));
    }

    ModelFile << "\r\nSEQUENCE DATA\r\n";

    for(i=0;i<SequenceList.Num();i++)
    {
        sprintf(bla, "\tSEQUENCE %d '%s'\tframes: %d\tkeyframe time: %d\r\n\r\n",
            i,
            SequenceList[i].name,
            SequenceList[i].nFrames,
            SequenceList[i].keyframeTime);
        ModelFile.Write(bla, strlen(bla));
    }

    ModelFile << "BONE DATA\r\n\r\n";

    for(i=0; i<BoneList.Num(); i++)
    {
        int verts = BoneList[i].bone.nBlendedVerts+BoneList[i].bone.nRigidVerts;
        sprintf(bla, "\tBONE %d %s\tidParent: %d\tverts: %d\r\n\t\tPos: x:%.4f  y:%.4f  z:%.4f\r\n\t\tRot: x:%.4f  y:%.4f  z:%.4f  z:%.4f\r\n\r\n",
            i,
            BoneList[i].node->GetName(),
            BoneList[i].bone.idParent,
            verts,
            BoneList[i].bone.Pos.x, BoneList[i].bone.Pos.y, BoneList[i].bone.Pos.z,
            BoneList[i].bone.Rot.x, BoneList[i].bone.Rot.y, BoneList[i].bone.Rot.z, BoneList[i].bone.Rot.w);
        ModelFile.Write(bla, strlen(bla));

        for(j=0;j<BoneList[i].bone.nBlendedVerts;j++)
        {
            sprintf(bla, "\t\tVERT %d %d\tweight: %.4f\r\n",
                FindVertex(BoneList[i].blendedVerts[j].vert),
                BoneList[i].blendedVerts[j].vert,
                ByteToNFloat(BoneList[i].blendedVerts[j].weight));
            ModelFile.Write(bla, strlen(bla));
        }
        for(j=0;j<BoneList[i].bone.nRigidVerts;j++)
        {
            sprintf(bla, "\t\tVERT %d %d\tweight: %.4f\r\n",
                FindVertex(BoneList[i].rigidVerts[j]),
                BoneList[i].rigidVerts[j],
                1.0f);
            ModelFile.Write(bla, strlen(bla));
        }
        for(j=0;j<BoneList[i].AnimKeyList.Num();j++)
        {
            sprintf(bla, "\t\tSEQUENCE %d\trotkeys: %d\tposkeys: %d\r\n",
                j,
                BoneList[i].AnimKeyList[j]->rotKey.Num(),
                BoneList[i].AnimKeyList[j]->posKey.Num());
            ModelFile.Write(bla, strlen(bla));
            for(k=0;k<BoneList[i].AnimKeyList[j]->rotKey.Num();k++)
            {
                sprintf(bla, "\t\t\tROTKEY x: %f\ty: %f\tz: %f\tw: %f\r\n",
                    CharToNFloat(BoneList[i].AnimKeyList[j]->rotKey[k].x),
                    CharToNFloat(BoneList[i].AnimKeyList[j]->rotKey[k].y),
                    CharToNFloat(BoneList[i].AnimKeyList[j]->rotKey[k].z),
                    ByteToNFloat(BoneList[i].AnimKeyList[j]->rotKey[k].w));
                ModelFile.Write(bla, strlen(bla));
            }
            for(k=0;k<BoneList[i].AnimKeyList[j]->posKey.Num();k++)
            {
                sprintf(bla, "\t\t\tPOSKEY x: %f\ty: %f\tz: %f\r\n",
                    BoneList[i].AnimKeyList[j]->posKey[k].x,
                    BoneList[i].AnimKeyList[j]->posKey[k].y,
                    BoneList[i].AnimKeyList[j]->posKey[k].z);
                ModelFile.Write(bla, strlen(bla));
            }
        }
    }

    free(bla);*/
}

/*****************************************************************************/

TriObject* GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
    {
		TriObject *tri = (TriObject *) obj->ConvertToType(t, Class_ID(TRIOBJ_CLASS_ID, 0));
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else
		return NULL;
}

Modifier* FindPhysiqueModifier (INode* nodePtr)
{
    if(!nodePtr) return NULL;

	Object* ObjectPtr = nodePtr->GetObjectRef();
	if (!ObjectPtr) return NULL;

	while (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID && ObjectPtr)
	{
		IDerivedObject *DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);
						
		int ModStackIndex = 0;
		while (ModStackIndex < DerivedObjectPtr->NumModifiers())
		{
			Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

			if (ModifierPtr->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
				return ModifierPtr;

			ModStackIndex++;
		}
		ObjectPtr = DerivedObjectPtr->GetObjRef();
	}

	return NULL;
}

Modifier* FindSkinModifier (INode* nodePtr)
{
    if(!nodePtr) return NULL;

	Object* ObjectPtr = nodePtr->GetObjectRef();
	if (!ObjectPtr) return NULL;

	while (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID && ObjectPtr)
	{
		IDerivedObject *DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);
						
		int ModStackIndex = 0;
		while (ModStackIndex < DerivedObjectPtr->NumModifiers())
		{
			Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

			if (ModifierPtr->ClassID() == SKIN_CLASSID)
				return ModifierPtr;

			ModStackIndex++;
		}
		ObjectPtr = DerivedObjectPtr->GetObjRef();
	}

	return NULL;
}

void xmdExp::ProcessNode(INode *node)
{
    int i;

    ObjectState os = node->EvalWorldState(0);

    if(os.obj)
    {
        Class_ID id = os.obj->ClassID(),cid=node->GetTMController()->ClassID();
        
        switch(os.obj->SuperClassID())
        {
            case GEOMOBJECT_CLASS_ID:
                if((cid != BIPBODY_CONTROL_CLASS_ID) && (cid != BIPSLAVE_CONTROL_CLASS_ID) && (cid != FOOTPRINT_CLASS_ID))
                {
                    if(!IsGun(node))
                        GetGeometry(node);
                }
                break;
        }
    }

    for(i=0; i<node->NumberOfChildren(); i++)
        ProcessNode(node->GetChildNode(i));
}

void xmdExp::ExportBones()//INode *parentNode)
{
    /*if(!parentNode)
        parentNode = ip->GetRootNode();

    for(int i=0; i<parentNode->NumberOfChildren(); i++)
    {
        INode *node = parentNode->GetChildNode(i);*/

        /*ObjectState os = node->EvalWorldState(0);
        Class_ID id= os.obj->ClassID(),cid=node->GetTMController()->ClassID();

        switch(os.obj->SuperClassID())
        {
            case GEOMOBJECT_CLASS_ID:
                if((cid == BIPBODY_CONTROL_CLASS_ID) || (cid == BIPSLAVE_CONTROL_CLASS_ID) || (id == BONE_OBJ_CLASSID))
                    ExportBoneData(node, -1);
                break;
            case HELPER_CLASS_ID:
                if(id == Class_ID(BONE_CLASS_ID, 0) || (id == CHARACTER_CLASSID))
                    ExportBoneData(node, -1);
                break;
            default:
                ExportBones(node);
                break;
        }*/

        //if(IsBone(node))
            ExportBoneData(ip->GetRootNode(), -1);
        /*else
            ExportBones(node);
    }*/
}

void xmdExp::ExportBoneData(INode *bone, DWORD idParent, Matrix3 *lpT)
{
    BONE ebone;
    DWORD tStart=0;
    Matrix3 WorldMtx,LocalMtx,T;
    AffineParts ap;
    //Class_ID id = bone->EvalWorldState(0).obj->ClassID(),cid=bone->GetTMController()->ClassID();
    int i;

    if(IsGun(bone) && (idParent != 0xFFFFFFFF))
    {
        WStr extensionName(bone->GetName()+4);

        WorldMtx = bone->GetNodeTM(0);
        //WorldMtx.RotateZ(RAD(rotation));

        Matrix3 ParentMtx = bone->GetParentTM(0);
        //ParentMtx.RotateZ(RAD(rotation));
        LocalMtx = WorldMtx * Inverse(ParentMtx);

        BoneExtension &extension = *ExtensionList.CreateNew(); 

        extension.name = extensionName;
        extension.parentBone = idParent;

        //------------------------------------------------------------
        BOOL needDel;
	    TriObject* tri = GetTriObjectFromNode(bone, GetStartTime(), needDel);
	    if (!tri) return;

	    Mesh* mesh = &tri->GetMesh();

        if(mesh->numFaces == 1)
        {
            int longEdge, shortEdge, medEdge, numEdges = 3;
            float length[3];
            Edge *edgeList = mesh->MakeEdgeList(&numEdges);

            for(i=0; i<3; i++)
                length[i] = Length(mesh->verts[edgeList[i].v[0]]-mesh->verts[edgeList[i].v[1]]);

            for(i=0; i<3; i++)
            {
                DWORD im1 = (i == 0) ? 2 : i-1;
                DWORD ip1 = (i == 2) ? 0 : i+1;

                if((length[i] >= length[im1]) && (length[i] > length[ip1]))
                    longEdge = i;
                else if((length[i] <= length[im1]) && (length[i] < length[ip1]))
                    shortEdge = i;
                else
                    medEdge = i;
            }

            Point3 center,y,z;

            if((edgeList[medEdge].v[0] == edgeList[shortEdge].v[0]) || (edgeList[medEdge].v[0] == edgeList[shortEdge].v[1]))
            {
                center = mesh->verts[edgeList[medEdge].v[0]];
                y = mesh->verts[edgeList[medEdge].v[1]] - center;
            }
            else
            {
                center = mesh->verts[edgeList[medEdge].v[1]];
                y = mesh->verts[edgeList[medEdge].v[0]] - center;
            }

            if((edgeList[shortEdge].v[0] == edgeList[longEdge].v[0]) || (edgeList[shortEdge].v[0] == edgeList[longEdge].v[1]))
                z = (mesh->verts[edgeList[shortEdge].v[0]] - center);
            else
                z = (mesh->verts[edgeList[shortEdge].v[1]] - center);

            Point3 x = CrossProd(z, y);

            Matrix3 mRot(x.Normalize(), y.Normalize(), z.Normalize(), Point3(0.0f, 0.0f, 0.0f));

            mRot.Orthogonalize();

            mRot = LocalMtx * mRot;

            MaxQuat rot(mRot);

            rot.Normalize();

            extension.Pos = GetVect(LocalMtx.GetTrans());
            extension.Rot.x = rot.x;
            extension.Rot.y = rot.z;
            extension.Rot.z = -rot.y;
            extension.Rot.w = rot.w;
        }
        else
        {
            MaxQuat rot(LocalMtx);

            extension.Pos = GetVect(LocalMtx.GetTrans());
            extension.Rot.x = rot.x;
            extension.Rot.y = rot.z;
            extension.Rot.z = -rot.y;
            extension.Rot.w = rot.w;
        }

        if(needDel) delete tri;
    }

    /*if( (id  != Class_ID(BONE_CLASS_ID, 0)) &&
        (cid != BIPBODY_CONTROL_CLASS_ID)   &&
        (cid != BIPSLAVE_CONTROL_CLASS_ID)  &&
        (id  != BONE_OBJ_CLASSID) &&
        (id  != CHARACTER_CLASSID) ||   
        (id  == Class_ID(DUMMY_CLASS_ID, 0)))
    {
        for(int c
        ExportBones(bone);
        return;
    }


    if(id == CHARACTER_CLASSID)
    {
        for(int c=0; c<bone->NumberOfChildren(); c++)
            ExportBoneData(bone->GetChildNode(c), idParent, NULL);
    }
    else*/
    if(!IsBone(bone))
    {
        for(int c=0; c<bone->NumberOfChildren(); c++)
            ExportBoneData(bone->GetChildNode(c), idParent, NULL);
    }
    else
    {
        INode *curParent = bone->GetParentNode();

        while(!IsBone(curParent) && (curParent != ip->GetRootNode()))
            curParent = curParent->GetParentNode();

        Matrix3 parentMat = GetBoneInitTM(curParent);

        WorldMtx = GetBoneInitTM(bone);
        LocalMtx = WorldMtx * Inverse(parentMat);

        Point3 test = WorldMtx.GetTrans();

        decomp_affine(WorldMtx, &ap);

        memset(&ebone, 0, sizeof(BONE));

        animHead.nBones++;

        //MCHAR *chi = bone->GetName();
        //WStr newName = chi;

        ebone.Rot = XT::Quat(0.0, 0.0, 0.0, 1.0);
        ebone.Pos = GetVect(ap.t);
        ebone.idParent = idParent;

        int i = BoneList.Num();
        BoneList.SetSize(BoneList.Num()+1);
	    if(idParent != 0xFFFFFFFF) BoneList[i].Parent = BoneList+idParent;
        BoneList[i].node = bone;
	    BoneList[i].Rot = ap.q;
        BoneList[i].starttm = parentMat;

        decomp_affine(LocalMtx, &ap);
        BoneList[i].firstPos = ap.t;


        if(idParent == 0xFFFFFFFF)
            ebone.flags |= BONE_ROOT;

        memcpy(&BoneList[i].bone, &ebone, sizeof(BONE));

        for(int c=0; c<bone->NumberOfChildren(); c++)
            ExportBoneData(bone->GetChildNode(c), i, NULL);
    }
}

BOOL xmdExp::TMNegParity(Matrix3 &m)
{
	return (DotProd(CrossProd(m.GetRow(0),m.GetRow(1)),m.GetRow(2))<0.0)?1:0;
}

Point3 xmdExp::GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv)
{
	Face* f = &mesh->faces[faceNo];
	DWORD smGroup = f->smGroup;
	int numNormals;
	Point3 vertexNormal;
	
    if (rv->rFlags & SPECIFIED_NORMAL)
		vertexNormal = rv->rn.getNormal();

    else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup)
    {
		if (numNormals == 1)
			vertexNormal = rv->rn.getNormal();
		else
        {
            for (int i = 0; i < numNormals; i++)
            {
				if (rv->ern[i].getSmGroup() & smGroup)
					vertexNormal = rv->ern[i].getNormal();
			}
		}
	}
	else
		vertexNormal = mesh->getFaceNormal(faceNo);
	
	return vertexNormal;
}

void xmdExp::GetGeometry(INode *node)
{
    int i;
	Mtl* nodeMtl = node->GetMtl();
	Matrix3 tm;
    tm = (bHasAnimation) ? GetMeshInitTM(node) : node->GetNodeTM(ip->GetTime());
	BOOL negScale = TMNegParity(tm),isMultiMaterial=0;
	int vx1, vx2, vx3, maps=7;

    if(!nodeMtl) return;

    if(nodeMtl->ClassID() == Class_ID(MULTI_CLASS_ID, 0))
    {
        isMultiMaterial = 1;

        maps = nodeMtl->NumSubMtls();
    }

	ObjectState os = node->EvalWorldState(GetStartTime());
	if (!os.obj || os.obj->SuperClassID()!=GEOMOBJECT_CLASS_ID)
		return;

    tm.NoTrans();
    //tm.NoScale();
    //tm.NoRot();

    

	if (negScale)
    {
		vx1 = 2;
		vx2 = 1;
		vx3 = 0;
	}
	else
    {
		vx1 = 0;
		vx2 = 1;
		vx3 = 2;
	}

    
    BOOL needDel,firstRun;
	TriObject* tri = GetTriObjectFromNode(node, GetStartTime(), needDel);
	if (!tri) return;

    
	
	Mesh* mesh = &tri->GetMesh();
    List<BOOL> FaceExported;

    if(!mesh->mapFaces(1))
    {
        String strText;
        strText << TEXT("Could not find UV Coordinate information for object '") << String(node->GetName()) << TEXT("'.");
        MessageBox(ip->GetMAXHWnd(), strText, TEXT("Unable to export model"), MB_ICONEXCLAMATION);
        if(needDel) delete tri;
        return;
    }

    mesh->buildNormals();

    VertSave.SetSize(mesh->getNumVerts());
    modelHead.nFaces += mesh->getNumFaces();
    FaceSave.SetSize(modelHead.nFaces);
    FaceExported.SetSize(modelHead.nFaces);

    nVerts = mesh->getNumVerts();

    List<StdMat*>   TempMaterialList;
    List<DWORD>     TempMaterialIDList;

    //merge duplicate materials
    for(DWORD matid=0;matid<maps;matid++)
    {
        BOOL bDupeMaterial = 0;
        DWORD dupeID;
        StdMat *mtl;


        if(!isMultiMaterial)
            mtl = (StdMat*)nodeMtl;
        else
        {
            MultiMtl *multi = (MultiMtl*)nodeMtl;
            mtl = (StdMat*)multi->GetSubMtl(matid);
        }

        if(mtl)
        {
            for(i=0; i<TempMaterialList.Num(); i++)
            {
                if(TempMaterialList[i] == mtl)
                {
                    bDupeMaterial = 1;
                    dupeID = TempMaterialIDList[i];
                }
            }

            if(bDupeMaterial)
            {
                for(i=0; i<mesh->getNumFaces(); i++)
                {
                    Face *face = &mesh->faces[i];
                    if(face->getMatID() == matid)
                        face->setMatID(dupeID);
                }
            }
            else
            {
                TempMaterialList << mtl;
                TempMaterialIDList << matid;
            }
        }
    }

    TempMaterialList.Clear();
    TempMaterialIDList.Clear();

    //get mesh/material data
    for(DWORD matid=0;matid<maps;matid++)
    {
        MATERIAL section;
        int nFaces,startFace,startFaceID;
        StdMat *mtl;

        memset(&section, 0, sizeof(section));

        nFaces = 0;
        firstRun = 1;
        startFace = FaceList.Num();
        startFaceID = -1;


        //get mesh data
        for(long sg=-1;sg<32;sg++)
        {   
            for(int j=0;j<mesh->getNumFaces();j++)
            {
                Face   *face    = &mesh->faces[j];
                TVFace *tvface  = mesh->mapFaces(1);
                UVVert *tverts  = mesh->mapVerts(1);

                TVFace *tvface2 = mesh->mapFaces(2);
                UVVert *tverts2 = mesh->mapVerts(2);

                if(tvface) tvface = &tvface[j];

                DWORD dwSG = (sg<0) ? 0 : (1<<sg);

                if( ((face->getMatID() == matid) || !isMultiMaterial)  &&
                    ((face->smGroup & dwSG) || (face->smGroup == 0)) &&
                    (!FaceExported[j]))
                {
                    FACE     f;    
                    VERTDATA v;
                    Vect     vert;
                    Vect     norm;
                    UVCoord  uv;
                    UVCoord  lmCoord;
                    BOOL     bTVMirrored;

                    if(startFaceID == -1)
                        startFaceID = j;

                    memset(&f,    -1, sizeof(f));
                    memset(&v,     0, sizeof(VERTDATA));
                    memset(&vert,  0, sizeof(Vect));
                    memset(&norm,  0, sizeof(Vect));
                    memset(&uv,    0, sizeof(UVCoord));
                    memset(&lmCoord, 0, sizeof(UVCoord));

                    Vect uv1 = GetUVCoord3(tverts[tvface->t[vx1]]);
                    Vect uv2 = GetUVCoord3(tverts[tvface->t[vx2]]);
                    Vect uv3 = GetUVCoord3(tverts[tvface->t[vx3]]);

                    Vect temp1(uv1-uv3), temp2(uv2-uv3);
                    Vect chi = (temp1^temp2).Norm();

                    bTVMirrored = !(chi.z > 0.0f);

                    for(i=0;i<VertSave[face->v[vx1]].Num();i++)
                    {
                        if( (VertSave[face->v[vx1]][i].matID == matid) &&
                            (VertSave[face->v[vx1]][i].sg & dwSG)      &&
                            (VertSave[face->v[vx1]][i].tvert  == (tvface  ? tvface->t[vx1]  : 0xFFFFFFFF)) &&
                            (VertSave[face->v[vx1]][i].tvert2 == (tvface2 ? tvface2->t[vx1] : 0xFFFFFFFF)) &&
                            (VertSave[face->v[vx1]][i].bTVMirrored == bTVMirrored))
                        {
                            f.ptr[0] = VertSave[face->v[vx1]][i].vert;
                        }
                    }
                    for(i=0;i<VertSave[face->v[vx2]].Num();i++)
                    {
                        if( (VertSave[face->v[vx2]][i].matID == matid) &&
                            (VertSave[face->v[vx2]][i].sg & dwSG)      &&
                            (VertSave[face->v[vx2]][i].tvert  == (tvface  ? tvface->t[vx2]  : 0xFFFFFFFF)) &&
                            (VertSave[face->v[vx2]][i].tvert2 == (tvface2 ? tvface2->t[vx2] : 0xFFFFFFFF)) &&
                            (VertSave[face->v[vx2]][i].bTVMirrored == bTVMirrored))
                        {
                            f.ptr[1] = VertSave[face->v[vx2]][i].vert;
                        }
                    }
                    for(i=0;i<VertSave[face->v[vx3]].Num();i++)
                    {
                        if( (VertSave[face->v[vx3]][i].matID == matid) &&
                            (VertSave[face->v[vx3]][i].sg & dwSG)      &&
                            (VertSave[face->v[vx3]][i].tvert  == (tvface  ? tvface->t[vx3]  : 0xFFFFFFFF)) &&
                            (VertSave[face->v[vx3]][i].tvert2 == (tvface2 ? tvface2->t[vx3] : 0xFFFFFFFF)) &&
                            (VertSave[face->v[vx3]][i].bTVMirrored == bTVMirrored))
                        {
                            f.ptr[2] = VertSave[face->v[vx3]][i].vert;
                        }
                    }
                    if(f.ptr[0] == 0xFFFFFFFF)
                    {
                        f.ptr[0] = v.vert = VertList.Num();
                        v.sg    |= dwSG;
                        v.matID  = matid;
                        v.bTVMirrored = bTVMirrored;

                        if(tvface2)
                        {
                            v.tvert2 = tvface2->t[vx1];
                            lmCoord = GetUVCoord2(tverts2[tvface2->t[vx1]]);
                        }
                        else v.tvert2 = 0xFFFFFFFF;

                        if(tvface)
                        {
                            v.tvert  = tvface->t[vx1];
                            uv = GetUVCoord2(tverts[tvface->t[vx1]]);
                        }
                        else v.tvert = 0xFFFFFFFF;
                        VertSave[face->v[vx1]] << v;

                        vert = GetVect(VectorTransform(tm, mesh->verts[face->v[vx1]]));
                        Vect vNorm = GetNorm(VectorTransform(tm, GetVertexNormal(mesh, j, mesh->getRVertPtr(face->v[vx1])))).Norm();
                        norm = vNorm;

                        uv.y = 1.0f-uv.y;

                        VertList    << vert;
                        NormalList  << norm;
                        UVList      << uv;

                        if(v.tvert2 != 0xFFFFFFFF)
                            LMCoords    << lmCoord;
                    }
                    if(f.ptr[1] == 0xFFFFFFFF)
                    {
                        f.ptr[1] = v.vert = VertList.Num();
                        v.sg    |= dwSG;
                        v.matID  = matid;
                        v.bTVMirrored = bTVMirrored;

                        if(tvface2)
                        {
                            v.tvert2 = tvface2->t[vx2];
                            lmCoord = GetUVCoord2(tverts2[tvface2->t[vx2]]);
                        }
                        else v.tvert2 = 0xFFFFFFFF;

                        if(tvface)
                        {
                            v.tvert  = tvface->t[vx2];
                            uv = GetUVCoord2(tverts[tvface->t[vx2]]);
                        }
                        else v.tvert = 0xFFFFFFFF;
                        VertSave[face->v[vx2]] << v;

                        vert = GetVect(VectorTransform(tm, mesh->verts[face->v[vx2]]));
                        Vect vNorm = GetNorm(VectorTransform(tm, GetVertexNormal(mesh, j, mesh->getRVertPtr(face->v[vx2])))).Norm();
                        norm = vNorm;

                        uv.y = 1.0f-uv.y;

                        VertList    << vert;
                        NormalList  << norm;
                        UVList      << uv;

                        if(v.tvert2 != 0xFFFFFFFF)
                            LMCoords    << lmCoord;
                    }
                    if(f.ptr[2] == 0xFFFFFFFF)
                    {
                        f.ptr[2] = v.vert = VertList.Num();
                        v.sg    |= dwSG;
                        v.matID  = matid;
                        v.bTVMirrored = bTVMirrored;

                        if(tvface2)
                        {
                            v.tvert2 = tvface2->t[vx3];
                            lmCoord = GetUVCoord2(tverts2[tvface2->t[vx3]]);
                        }
                        else v.tvert2 = 0xFFFFFFFF;

                        if(tvface)
                        {
                            v.tvert  = tvface->t[vx3];
                            uv = GetUVCoord2(tverts[tvface->t[vx3]]);
                        }
                        else v.tvert = 0xFFFFFFFF;
                        VertSave[face->v[vx3]] << v;

                        vert = GetVect(VectorTransform(tm, mesh->verts[face->v[vx3]]));
                        Vect vNorm = GetNorm(VectorTransform(tm, GetVertexNormal(mesh, j, mesh->getRVertPtr(face->v[vx3])))).Norm();
                        norm = vNorm;

                        uv.y = 1.0f-uv.y;

                        VertList    << vert;
                        NormalList  << norm;
                        UVList      << uv;

                        if(v.tvert2 != 0xFFFFFFFF)
                            LMCoords    << lmCoord;
                    }

                    //we got our triangle info, finish up
                    if(firstRun)
                    {
                        section.startFace = FaceList.Num();
                        firstRun = 0;
                    }


                    FaceExported[j] = 1;
                    Plane faceplane;
                    faceplane.Dir = GetNorm(VectorTransform(tm, mesh->getFaceNormal(j)));
                    faceplane.Dist = faceplane.Dir.Dot(VertList[f.A]);

                    FaceSave[j] = FaceList.Num();
                    FaceList << f;
                    FacePlaneList << PlaneList.SafeAdd(faceplane);

                    nFaces++;


                    Vect *newvert;
                    newvert = TempFaceTangentUList.CreateNew();

                    //newvert->Set(directionU.x, directionU.z, -directionU.y);
                    //newvert->Set(directionU.x, directionU.y, directionU.z);

                    if( (UVList[f.A] == UVList[f.B]) ||
                        (UVList[f.B] == UVList[f.C]) ||
                        (UVList[f.C] == UVList[f.A]))
                    {
                        *newvert = 0;
                    }
                    else
                    {
                        //dbg("een here..?1\r\n");
                        Vect vec1 = VertList[f.B] - VertList[f.A];
                        Vect vec2 = VertList[f.C] - VertList[f.A];

                        double deltaU1 = UVList[f.B].x - UVList[f.A].x, deltaU2 = UVList[f.C].x - UVList[f.A].x;
                        double deltaV1 = UVList[f.B].y - UVList[f.A].y, deltaV2 = UVList[f.C].y - UVList[f.A].y;

                        ///////////////////////////////////////////////////////

                        double a = (deltaU1 - deltaV1*deltaU2/deltaV2);
                        if(a != 0.0)
                            a = 1.0/a;
                        double b = (deltaU2 - deltaV2*deltaU1/deltaV1);
                        if(b != 0.0)
                            b = 1.0/b;

                        Vect duTemp = ((vec1*a) + (vec2*b));
                        double tempf = 1.0 / sqrt(duTemp|duTemp);
                        duTemp *= tempf;

                        ///////////////////////////////////////////////////////

                        a = (deltaV1 - deltaU1*deltaV2/deltaU2);
                        if(a != 0.0)
                            a = 1.0/a;
                        b = (deltaV2 - deltaU2*deltaV1/deltaU1);
                        if(b != 0.0)
                            b = 1.0/b;

                        Vect dvTemp = ((vec1*a) + (vec2*b));
                        tempf = 1.0 / sqrt(dvTemp|dvTemp);
                        dvTemp *= tempf;

                        ///////////////////////////////////////////////////////

                        Vect norm1 = faceplane.Dir;
                        tempf = duTemp|norm1;
                        *newvert = -(duTemp - (norm1*tempf)).Norm();
                        if(bTVMirrored)
                            *newvert *= -1.0f;
                    }
                }
            }
        }

        //we get our material info here
        section.startFace = startFace;
        section.nFaces = nFaces;

        if(!isMultiMaterial)
            mtl = (StdMat*)nodeMtl;
        else
        {
            MultiMtl *multi = (MultiMtl*)nodeMtl;
            mtl = (StdMat*)multi->GetSubMtl(matid);
        }

        if(mtl && nFaces)
        {
            MATERIALNAME matName;

            WStr newString(mtl->GetName());
            tstr_to_utf8(newString.data(), matName.name, 127);
            MaterialList << section;
            MaterialNameList << matName;

            TempMaterialData *chi = TempMaterialDataList.CreateNew();

            chi->id = matid;
            chi->startFaceID = startFaceID;
        }

        if(!isMultiMaterial) break;
    }

    int numEdges;
    Edge *TempEdgeList = mesh->MakeEdgeList(&numEdges);
    EdgeList.SetSize(numEdges);
    for(i=0; i<numEdges; i++)
    {
        EdgeList[i].f1 = (TempEdgeList[i].f[0] != 0xFFFFFFFF) ? FaceSave[TempEdgeList[i].f[0]] : 0xFFFFFFFF;
        EdgeList[i].f2 = (TempEdgeList[i].f[1] != 0xFFFFFFFF) ? FaceSave[TempEdgeList[i].f[1]] : 0xFFFFFFFF;

        if(EdgeList[i].f1 != 0xFFFFFFFF)
        {
            Face &f = mesh->faces[TempEdgeList[i].f[0]];

            DWORD v1 = TempEdgeList[i].v[0], v2 = TempEdgeList[i].v[1];

            f.OrderVerts(v1, v2);
            EdgeList[i].v1 = VertSave[v1][0].vert;
            EdgeList[i].v2 = VertSave[v2][0].vert;
        }
        else
        {
            Face &f = mesh->faces[TempEdgeList[i].f[1]];

            DWORD v1 = TempEdgeList[i].v[1], v2 = TempEdgeList[i].v[0];

            f.OrderVerts(v1, v2);
            EdgeList[i].v1 = VertSave[v1][0].vert;
            EdgeList[i].v2 = VertSave[v2][0].vert;
        }
    }

    MAX_free(TempEdgeList);

    ////////////////////////////

    modelHead.nMaterials = MaterialList.Num();

    modelHead.nVerts = VertList.Num();

    if(needDel) delete tri;

    GetSkinData(node);
}

BONEDATA *xmdExp::GetBoneByNode(INode *node)
{
    for(int i=0;i<BoneList.Num();i++)
        if(BoneList[i].node == node) return &BoneList[i];

    return NULL;
}

void xmdExp::GetSkinData(INode *node)
{
    Modifier *mod = FindSkinModifier(node);

    if(!mod) {GetPhysiqueData(node); return;}

    ISkin *skin = (ISkin *)mod->GetInterface(I_SKIN);
    if(!skin) return;

    ISkinContextData *data = skin->GetContextInterface(node);

    if(data)
    {
        for(int c=0;c<data->GetNumPoints();c++)
        {
            List<BONEDATA*> Bones;  //mccoy
            List<float>     Weights;
            float           totalWeight = 0.0f;

            for(int i=0;i<data->GetNumAssignedBones(c);i++)
            {
                BONEDATA *boneOut = GetBoneByNode(skin->GetBone(data->GetAssignedBone(c, i)));

                if(boneOut)
                {
                    Bones << boneOut;

                    float weight = data->GetBoneWeight(c, i);
                    Weights << weight;
                    totalWeight += weight;
                }
            }

            for(int i=0; i<Weights.Num(); i++)
            {
                BONEDATA *boneOut = Bones[i];
                float weight = Weights[i]/totalWeight;

                if(CloseFloat(weight, 1.0f, 0.0001f))
                    SaveRigidVert(boneOut, c);
                else if(weight > 0.0f)
                    SaveBlendedVert(boneOut, c, weight);
            }
        }
    }
}

void xmdExp::GetPhysiqueData(INode *node)
{
    Modifier *mod = FindPhysiqueModifier(node);

    if(!mod) return;

    IPhysiqueExport *phy = (IPhysiqueExport*)mod->GetInterface(I_PHYINTERFACE);
    if(!phy) return;

    IPhyContextExport *skin = phy->GetContextInterface(node);
    if(skin)
    {
        skin->ConvertToRigid();

        for(int i=0;i<skin->GetNumberVertices();i++)
        {
            IPhyVertexExport *vertExp = skin->GetVertexInterface(i);
            BONEDATA *boneOut;


            if(vertExp->GetVertexType() & BLENDED_TYPE)
            {
                IPhyBlendedRigidVertex *vert = (IPhyBlendedRigidVertex*)vertExp;
                for(int c=0;c<vert->GetNumberNodes();c++)
                {
                    float weight;

                    boneOut = GetBoneByNode(vert->GetNode(c));
                    if(!boneOut) continue;  //we should never get here!!!

                    weight = vert->GetWeight(c);

                    SaveBlendedVert(boneOut, i, weight);
                }
            }
            else
            {
                IPhyRigidVertex *vert = (IPhyRigidVertex*)vertExp;

                boneOut = GetBoneByNode(vert->GetNode());
                if(!boneOut) continue;  //we should never get here!!!

                SaveRigidVert(boneOut, i);
            }
        }
        phy->ReleaseContextInterface(skin);
    }
    
}

void xmdExp::SaveBlendedVert(BONEDATA* bone, int vertex, float weight)
{
    WEIGHT w;
    for(int i=0;i<VertSave[vertex].Num();i++)
    {
        w.vert = VertSave[vertex][i].vert;
        w.weight = weight;
        if(w.weight == 255)
        {
            bone->rigidVerts << w.vert;

            bone->bone.nRigidVerts++;
        }
        else if(w.weight)
        {
            bone->blendedVerts << w;

            bone->bone.nBlendedVerts++;
        }
    }
}

void xmdExp::SaveRigidVert(BONEDATA* bone, int vertex)
{
    for(int i=0;i<VertSave[vertex].Num();i++)
    {
        bone->rigidVerts << VertSave[vertex][i].vert;

        bone->bone.nRigidVerts++;
    }
}

BOOL xmdExp::CheckForAnimation(INode* node, int id, BOOL& bPos, BOOL& bRot)
{
	int i, num, delta = GetTicksPerFrame();
	Matrix3 tm;
	AffineParts ap;
	float rotAngle, firstRotAngle;
	Point3 firstPos, rotAxis, firstRotAxis;
    List<MaxQuat> rotKeys;
    List<Point3> posKeys;

    MSTR chi = node->GetName();

    if(!IsBone(node))
        return (bPos = bRot = 0);

    tm.IdentityMatrix();

    tm = GetBoneInitTM(node);

    decomp_affine(tm, &ap);
    firstPos = ap.t;
    AngAxisFromQ(ap.q, &firstRotAngle, firstRotAxis);


    AppDataChunk *chunk = GetData(node, ROTNUM_ID+id);
    num = *(DWORD*)chunk->data;
    chunk = GetData(node, ROTDATA_ID+id);
    rotKeys.CopyArray((MaxQuat*)chunk->data, num);


    chunk = GetData(node, POSNUM_ID+id);
    num = *(DWORD*)chunk->data;
    chunk = GetData(node, POSDATA_ID+id);
    posKeys.CopyArray((Point3*)chunk->data, num);


    bPos = bRot = FALSE;

	for(i=0; i<rotKeys.Num(); i++)
    {
		AngAxisFromQ(rotKeys[i], &rotAngle, rotAxis);

		if (!bPos)
        {
			if(!posKeys[i].Equals(Point3(0.0, 0.0, 0.0), 0.01f))
				bPos = TRUE;
		}
        if (!bRot)
        {
			if (fabs(rotAngle) > 0.0001f)
				bRot = TRUE;
		}

		if (bPos && bRot)
			break;
    }

    posKeys.Clear();
    rotKeys.Clear();

	return bPos || bRot;
}

/*void xmdExp::GetPosSamples(BONEDATA *bone, ANIM* anim) 
{
    INode *node = bone->node;
	TimeValue start = GetStartTime(), end = GetEndTime(), t;
	int delta = GetTicksPerFrame() * GetKeyframeStep();
	Matrix3 tm;
    Vect posKey;// = {0.0, 0.0, 0.0};
    Point3 pos;


	for (t=start; t<end; t+=delta) {
		tm = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));

		pos = tm.GetTrans();

        posKey = (pos-bone->firstPos);

        anim->posKey << posKey;
	}
}

void xmdExp::GetRotSamples(BONEDATA *bone, ANIM* anim)
{	
    INode *node = bone->node;
	TimeValue start = GetStartTime(), end = GetEndTime(), t;
	int delta = GetTicksPerFrame() * GetKeyframeStep();
	Matrix3 tm;
    CompressedQuat rotKey;
	//MaxQuat firstQ(node->GetNodeTM(start));

	for (t=start; t<end; t+=delta) {
		tm = (node->GetNodeTM(t) * Inverse(node->GetParentTM(t))) * bone->starttm;

		MaxQuat q(tm);

        q = q / bone->Rot;

		if(q.w < 0)
		{
			q.x = -q.x;
			q.y = -q.y;
			q.z = -q.z;
			q.w = -q.w;
		}

        rotKey = q;

        anim->rotKey << rotKey;
	}
}*/

BOOL EqualPoint3(Point3 p1, Point3 p2)
{
	if (fabs(p1.x - p2.x) > ALMOST_ZERO)
		return FALSE;
	if (fabs(p1.y - p2.y) > ALMOST_ZERO)
		return FALSE;
	if (fabs(p1.z - p2.z) > ALMOST_ZERO)
		return FALSE;

	return TRUE;
}

BOOL xmdExp::IsBone(INode *bone)
{
    Control *controller = bone->GetTMController();
    Object *obj = bone->EvalWorldState(0).obj;
    Class_ID id,cid;

    /*if(SkinModifiers.Num())
    {
        BOOL bValid = FALSE;
        Matrix3 output;
        for(int i=0; i<SkinModifiers.Num(); i++)
        {
            Modifier *mod = SkinModifiers[i];
            if(mod->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
            {
                IPhysiqueExport *phy = (IPhysiqueExport*)mod->GetInterface(I_PHYINTERFACE);

                if(phy && (phy->GetInitNodeTM(bone, output) == MATRIX_RETURNED))
                {
                    bValid = TRUE;
                    break;
                }
            }
            else if(mod->ClassID() == SKIN_CLASSID)
            {
                ISkin *skin = (ISkin*)mod->GetInterface(I_SKIN);

                if(skin && (skin->GetBoneInitTM(bone, output) == SKIN_OK))
                {
                    bValid = TRUE;
                    break;
                }
            }
        }
        if(!bValid)
            return FALSE;
    }*/

    id = (obj) ? obj->ClassID() : Class_ID(0, 0);
    cid = (controller) ? controller->ClassID() : Class_ID(0, 0);

    return ((id  == Class_ID(BONE_CLASS_ID, 0)) ||
            (cid == BIPBODY_CONTROL_CLASS_ID)   ||
            (cid == BIPSLAVE_CONTROL_CLASS_ID)  ||
            (id  == BONE_OBJ_CLASSID));
}


Point3 GetPoint3(const Vect &v)
{
    return Point3(v.x*10.0, -v.z*10.0, v.y*10.0);
}

Vect GetVect(const Point3 &v)
{
    return Vect(v.x*0.1, v.z*0.1, -v.y*0.1);
}

Vect GetNorm(const Point3 &v)
{
    return Vect(v.x, v.z, -v.y);
}

Vect GetUVCoord3(const Point3 &v)
{
    return Vect(v.x, v.y, v.z);
}

UVCoord GetUVCoord2(const Point3 &v)
{
    return UVCoord(v.x, v.y);
}

UVVert GetUVVert(const UVCoord &v)
{
    return UVVert(v.x, v.y, 0.0f);
}

XT::Quat GetXenQuat(MaxQuat &q)
{
    return XT::Quat(q.x, q.z, -q.y, q.w);
}


bool __stdcall IsGun(INode *node)
{
    WStr wstrName(node->GetName());
    wstrName.toLower();

    if(!_tcsncmp(wstrName, _T("ext_"), 4))
        return wstrName[4] != 0;
    else
        return false;
}

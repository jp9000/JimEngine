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



static xbrExpClassDesc xbrExpDesc;
ClassDesc2* GetxbrExpDesc() { return &xbrExpDesc; }


xbrExp::xbrExp()
{
}

xbrExp::~xbrExp()
{
}


struct EditFace
{
    FACE    pointFace;
    FACE    uvFace;
    FACE    normalFace;

    DWORD   smoothFlags;
    DWORD   polyID;

    bool    edgeVisible[3];
};


int	xbrExp::DoExport(const MCHAR *mbcName,ExpInterface *ei,Interface *interfaceIn, BOOL suppressPrompts, DWORD options)
{
    INode *curNode = NULL;
    BOOL needDel;

    ip = interfaceIn;

    WStr wstrName(mbcName);
    String name = wstrName;

    //------------------------------------------------------

    int i, j;

    for(i=0; i<ip->GetRootNode()->NumberOfChildren(); i++)
    {
        INode *childNode = ip->GetRootNode()->GetChildNode(i);

        ObjectState os = childNode->EvalWorldState(0);

        if(os.obj)
        {
            Class_ID cid=childNode->GetTMController()->ClassID();
            if( (os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID) &&
                (cid != BIPBODY_CONTROL_CLASS_ID) &&
                (cid != BIPSLAVE_CONTROL_CLASS_ID) &&
                (cid != FOOTPRINT_CLASS_ID) )
            {
                if(!(options & SCENE_EXPORT_SELECTED) || childNode->Selected())
                {
                    curNode = childNode;
                    break;
                }
            }
        }
    }

    //------------------------------------------------------

    if(curNode)
    {
        TriObject* tri = GetTriObjectFromNode(curNode, 0, needDel);
        if (!tri) return TRUE;
        Mesh &mesh = tri->GetMesh();
        MNMesh newMesh(mesh);
        if(needDel) delete tri;

        //-------------------------------------

        newMesh.FenceNonPlanarEdges(0.9999f, TRUE);
        newMesh.MakePolyMesh();

        MNMap &curMap = *newMesh.M(1);
        MNMap *curMap2 = newMesh.M(2);

        //-------------------------------------

        List<Vect>      Vertices;
        List<Vect2>     UVCoords;
        List<Vect2>     LMCoords;
        List<EditFace>  BrushFaces;
        List<BYTE>      PolyMats;
        List<String>    MatNames;

        //-------------------------------------

	    Mtl* nodeMtl = curNode->GetMtl();
        BOOL isMultiMaterial=FALSE;

        if(nodeMtl && (nodeMtl->ClassID() == Class_ID(MULTI_CLASS_ID, 0)))
        {
            MultiMtl *multi = (MultiMtl*)nodeMtl;

            DWORD maps = multi->NumSubMtls();
            MatNames.SetSize(maps);

            for(i=0; i<maps; i++)
            {
                Mtl *subMat = multi->GetSubMtl(i);
                if(subMat)
                    MatNames[i] = WStr(subMat->GetName());
            }

            isMultiMaterial = TRUE;
        }
        else
        {
            MatNames.SetSize(1);
            if(nodeMtl)
                MatNames[0] = WStr(nodeMtl->GetName());
        }

        //-------------------------------------

        Vertices.SetSize(newMesh.numv);
        for(i=0; i<newMesh.numv; i++)
            Vertices[i] = GetVect(newMesh.P(i));

        //-------------------------------------

        UVCoords.SetSize(curMap.numv);
        for(i=0; i<curMap.numv; i++)
        {
            UVCoords[i] = GetUVCoord2(curMap.V(i));
            UVCoords[i].x = -UVCoords[i].x;
        }

        //-------------------------------------

        if(curMap2)
        {
            LMCoords.SetSize(curMap2->numv);
            for(i=0; i<curMap2->numv; i++)
            {
                LMCoords[i] = GetUVCoord2(curMap2->V(i));
                LMCoords[i].x = -LMCoords[i].x;
            }
        }

        //-------------------------------------

        List<FACE> LMUVFaces;

        for(i=0; i<newMesh.numf; i++)
        {
            MNFace &face = *newMesh.F(i);
            MNMapFace &mapFace = *curMap.F(i);
            MNMapFace *map2Face = curMap2 ? curMap2->F(i) : NULL;

            Tab<int> triData;
            face.GetTriangles(triData);

            for(j=0; j<triData.Count(); j+=3)
            {
                int v[3] = {triData[j], triData[j+1], triData[j+2]};

                EditFace newFace;
                FACE lmFace;

                for(int k=0; k<3; k++)
                {
                    int kp1 = (k == 2) ? 0 : (k+1);
                    int nextVert = (v[k] == (face.deg-1)) ? 0 : (v[k]+1);

                    newFace.pointFace.ptr[k] = face.vtx[v[k]];
                    newFace.uvFace.ptr[k] = mapFace.tv[v[k]];

                    newFace.edgeVisible[k] = (nextVert == v[kp1]);

                    if(map2Face)
                        lmFace.ptr[k] = map2Face->tv[v[k]];

                    nop();
                }

                newFace.smoothFlags = face.smGroup;
                newFace.polyID = i;

                if(map2Face)
                    LMUVFaces << lmFace;

                BrushFaces << newFace;
            }

            if(isMultiMaterial)
                PolyMats << face.material;
            else
                PolyMats << 0;
        }

        //-------------------------------------

        XFileOutputSerializer fileDataOut;

        if(!fileDataOut.Open(name, XFILE_CREATEALWAYS))
        {
            MessageBox(NULL, TEXT("Unable to create export file"), NULL, MB_ICONEXCLAMATION|MB_OK);
            return TRUE;
        }

        DWORD versionNum = 0x00000100;
        fileDataOut << versionNum;
        fileDataOut << BrushFaces;
        Vect::SerializeList(fileDataOut, Vertices);
        fileDataOut << UVCoords;
        fileDataOut << LMUVFaces;
        fileDataOut << LMCoords;
        fileDataOut << PolyMats;

        DWORD temp = MatNames.Num();
        fileDataOut << temp;
        for(i=0; i<MatNames.Num(); i++)
        {
            fileDataOut << MatNames[i];
            MatNames[i].Clear();
        }
        MatNames.Clear();

        fileDataOut.Close();
    }

    return TRUE;
}


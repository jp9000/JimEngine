/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  sequencing.cpp

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


AppDataChunk* xmdExp::GetData(INode *node, DWORD dwID)
{
    return node->GetAppDataChunk(ANIMSEQ_CLASS_ID, UTILITY_CLASS_ID, dwID);
}

void xmdExp::GetNames(NAMELIST *names)
{
    AppDataChunk *chunk;
    int num;
    INode *root = ip->GetRootNode();

    if(!names) return;

    chunk = GetData(root, NAMENUM_ID);
    if(!chunk) return;
    num = *(DWORD*)chunk->data;


    chunk = GetData(root, NAMEDATA_ID);
    if(!chunk) return;
    names->CopyArray((NAME*)chunk->data, num);
}

BOOL xmdExp::HasAnimation()
{
    return GetData(ip->GetRootNode(), ANIMATED_ID) != NULL;
}

void xmdExp::RefreshMesh()
{
    INode *root = ip->GetRootNode();
    AppDataChunk *adc;

    adc = GetData(root, TIME_ID);
    if(adc)
        memcpy(&time, adc->data, sizeof(TimeValue));

    ip->SetTime(0);

    ip->RedrawViews(0);
}

Matrix3 xmdExp::GetMeshInitTM(INode* mesh)
{
    Matrix3 output;

    for(int i=0; i<SkinModifiers.Num(); i++)
    {
        Modifier *mod = SkinModifiers[i];
        if(mod->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
        {
            IPhysiqueExport *phy = (IPhysiqueExport*)mod->GetInterface(I_PHYINTERFACE);

            if(phy && (phy->GetInitNodeTM(mesh, output) == MATRIX_RETURNED))
                return output;
        }
        else if(mod->ClassID() == SKIN_CLASSID)
        {
            ISkin *skin = (ISkin*)mod->GetInterface(I_SKIN);

            if(skin && (skin->GetSkinInitTM(mesh, output) == SKIN_OK))
                return output;
        }
    }

    output.IdentityMatrix();
    return output;
}

Matrix3 xmdExp::GetBoneInitTM(INode* bone)
{
    Matrix3 output;

    for(int i=0; i<SkinModifiers.Num(); i++)
    {
        Modifier *mod = SkinModifiers[i];
        if(mod->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
        {
            IPhysiqueExport *phy = (IPhysiqueExport*)mod->GetInterface(I_PHYINTERFACE);

            if(phy && (phy->GetInitNodeTM(bone, output) == MATRIX_RETURNED))
                return output;
        }
        else if(mod->ClassID() == SKIN_CLASSID)
        {
            ISkin *skin = (ISkin*)mod->GetInterface(I_SKIN);

            if(skin && (skin->GetBoneInitTM(bone, output) == SKIN_OK))
                return output;
        }
    }

    output.IdentityMatrix();
    return output;
}
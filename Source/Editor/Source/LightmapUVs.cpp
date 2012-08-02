/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Lightmapping

  Copyright (c) 2009, Hugh Bailey
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


#include "Xed.h"


//holy dear fricken lord this UV flattener was a serious pain to write.
//but I have to admit, I did a pretty decent job of it.


struct UVSection
{
    List<UINT>  Faces;
    List<UINT>  Verts;  //stored as tris so there -are- dupes
    List<Vect2> ProjectedVerts;
    BitList     SplitVerts;
    float width, height;
    Vect mapDir;

    inline void FreeData() {ProjectedVerts.Clear(); Verts.Clear(); SplitVerts.Clear(); Faces.Clear();}
};

struct StorageData
{
    UINT item;
    Vect2 pos;
    BOOL bRot;
};

bool LineCheckForBoxOnIsle2(const Vect2 &minVal, const Vect2 &maxVal, const Vect2 &v1, const Vect2 &v2);

bool LineCheckForBoxOnIsle2(const Vect2 &minVal, const Vect2 &maxVal, const Vect2 &v1, const Vect2 &v2)
{
    float tMax = M_INFINITE;
    float tMin = -M_INFINITE;

    Vect2 rayVect = v2-v1;
    float rayLength = rayVect.Len();
    Vect2 rayDir = rayVect*(1.0f/rayLength);

    Vect2 center = ((maxVal-minVal)*0.5f)+minVal;
    Vect2 E = maxVal - center;
    Vect2 T = center-v1;

    for(int i=0; i<2; i++)
    {
        float e = T.ptr[i];
        float f = rayDir.ptr[i];
        float fI = 1.0f/f;

        if(fabs(f) > 0.0f)
        {
            float t1 = (e+E.ptr[i])*fI;
            float t2 = (e-E.ptr[i])*fI;
            if(t1 > t2)
            {
                if(t2 > tMin) tMin = t2;
                if(t1 < tMax) tMax = t1;
            }
            else
            {
                if(t1 > tMin) tMin = t1;
                if(t2 < tMax) tMax = t2;
            }
            if(tMin > tMax) return false;
            if(tMax < 0.0f) return false;
        }
        else if( ((-e - E.ptr[i]) > 0.0f) ||
                 ((-e + E.ptr[i]) < 0.0f) )
        {
            return FALSE;
        }

        if(tMin > rayLength) return false;
    }

    return true; 
};

//int storageBlockRebuildCount = 0;

struct BestStorageInfo
{
    inline BestStorageInfo(List<UVSection> &SectionsIn, int seperationUnitsIn) : Sections(SectionsIn) {availableOffsetBlocks = 0; seperationUnits = seperationUnitsIn; curSize = 0.0f; numAvailable = Sections.Num(); UnavailableItems.SetSize(Sections.Num()); StorageBlocks.SetSize(seperationUnits*seperationUnits); storageBlockSize = 0.0f;}

    Vect2 curSize;
    int numAvailable;
    BitList UnavailableItems;
    List<StorageData> CurStorage;
    List<UVSection> &Sections;
    int seperationUnits, availableOffsetBlocks;
    BitList StorageBlocks;
    Vect2 storageBlockSize, storageBlockSizeI;

    void PosToBlock(const Vect2 &pos, int &x, int &y, bool bRoundUp=false) const
    {
        Vect2 adjPos = pos*storageBlockSizeI;

        if(bRoundUp)
        {
            x = int(ceilf(adjPos.x)+0.1f);
            y = int(ceilf(adjPos.y)+0.1f);
        }
        else
        {
            x = int(adjPos.x);
            y = int(adjPos.y);
        }
    }

    void BlockToPos(int x, int y, Vect2 &pos) const
    {
        pos.Set(float(x)*storageBlockSize.x, float(y)*storageBlockSize.y);
    }

    void RebuildStorageBlocks(bool bSpecificArea=false, StorageData *rebuildArea=NULL)
    {
        int startX = 0, startY = 0, endX = seperationUnits, endY = seperationUnits;
        if(!bSpecificArea)
        {
            StorageBlocks.ClearAll();
            storageBlockSize = curSize/float(seperationUnits);
            storageBlockSizeI = 1.0f/storageBlockSize;

            availableOffsetBlocks = (curSize.x > curSize.y) ? int(curSize.x*storageBlockSizeI.y) : int(curSize.y*storageBlockSizeI.x);
            availableOffsetBlocks -= seperationUnits;
        }
        else
        {
            UVSection &section = Sections[rebuildArea->item];

            Vect2 max;
            if(rebuildArea->bRot)   max.Set(rebuildArea->pos.x+section.height, rebuildArea->pos.y+section.width);
            else                    max.Set(rebuildArea->pos.x+section.width,  rebuildArea->pos.y+section.height);

            PosToBlock(rebuildArea->pos, startX, startY);
            PosToBlock(max, endX, endY, true);
        }

        for(int y=startY; y<endY; y++)
        {
            DWORD yPos = y*seperationUnits;
            for(int x=startX; x<endX; x++)
            {
                DWORD blockPos = yPos+x;

                //doing a 5-point check to speed things up rather than a full box/tri intersection
                Vect2 checkPoints[2];
                checkPoints[0].Set(float(x)*storageBlockSize.x, float(y)*storageBlockSize.y);
                checkPoints[1] = checkPoints[0]+storageBlockSize;

                for(int i=0; i<CurStorage.Num(); i++)
                {
                    StorageData &data = CurStorage[i];
                    UVSection &section = Sections[data.item];

                    Vect2 max;
                    if(data.bRot)   max.Set(data.pos.x+section.height, data.pos.y+section.width);
                    else            max.Set(data.pos.x+section.width,  data.pos.y+section.height);

                    if( checkPoints[0].x > max.x || data.pos.x > checkPoints[1].x ||
                        checkPoints[0].y > max.y || data.pos.y > checkPoints[1].y )
                    {
                        continue;
                    }

                    int numTris = section.Verts.Num()/3;

                    bool bFoundBlocker = false;

                    for(int j=0; j<numTris; j++)
                    {
                        int startVert = j*3;
                        Vect2 vPos1 = section.ProjectedVerts[startVert];
                        Vect2 vPos2 = section.ProjectedVerts[startVert+1];
                        Vect2 vPos3 = section.ProjectedVerts[startVert+2];

                        if(data.bRot)
                        {
                            vPos1.SwapVals();
                            vPos2.SwapVals();
                            vPos3.SwapVals();

                            vPos1.x = section.height-vPos1.x;
                            vPos2.x = section.height-vPos2.x;
                            vPos3.x = section.height-vPos3.x;
                        }

                        vPos1 += data.pos;
                        vPos2 += data.pos;
                        vPos3 += data.pos;

                        if( LineCheckForBoxOnIsle2(checkPoints[0], checkPoints[1], vPos1, vPos2) ||
                            LineCheckForBoxOnIsle2(checkPoints[0], checkPoints[1], vPos2, vPos3) ||
                            LineCheckForBoxOnIsle2(checkPoints[0], checkPoints[1], vPos3, vPos1) )
                        {
                            StorageBlocks.Set(blockPos);
                            bFoundBlocker = true;
                            break;
                        }

                        Vect2 norm1 = (vPos1-checkPoints[0]).Norm();
                        Vect2 norm2 = (vPos2-checkPoints[0]).Norm();
                        Vect2 norm3 = (vPos3-checkPoints[0]).Norm();
                        float checkVal = acosf(norm1.Dot(norm2)) +
                                         acosf(norm2.Dot(norm3)) +
                                         acosf(norm3.Dot(norm1));

                        if(fabsf(checkVal-(M_PI*2.0f)) < 0.01f)
                        {
                            StorageBlocks.Set(blockPos);
                            bFoundBlocker = true;
                            break;
                        }
                    }

                    if(bFoundBlocker)
                        break;
                }
            }
        }

        /*String fileName;
        fileName << TEXT("floong") << IntString(storageBlockRebuildCount) << TEXT(".txt");
        XFile wong(fileName, XFILE_WRITE, XFILE_CREATEALWAYS);
        for(int y=0; y<seperationUnits; y++)
        {
            DWORD yPos = y*seperationUnits;
            for(int x=0; x<seperationUnits; x++)
            {
                wong.WriteAsUTF8(IntString(StorageBlocks[yPos+x] != 0));
            }

            wong.WriteAsUTF8(TEXT("\r\n"));
        }
        ++storageBlockRebuildCount;*/
    }
};

void FindBestUVStorage(BestStorageInfo &info);

void FindBestUVStorage(BestStorageInfo &info)
{
    int i, j;

    nop();

    //reorder sections from biggest to smallest
    for(i=0; i<info.Sections.Num(); i++)
    {
        UVSection &section1 = info.Sections[i];
        float totalSize1 = section1.width*section1.height;

        for(j=i+1; j<info.Sections.Num(); j++)
        {
            UVSection &section2 = info.Sections[j];
            float totalSize2 = section2.width*section2.height;

            if(totalSize2 > totalSize1)
            {
                info.Sections.SwapValues(i, j);
                totalSize1 = totalSize2;
            }
        }
    }

    //go through each section and find a place to put it
    for(i=0; i<info.Sections.Num(); i++)
    {
        UVSection &curSection = info.Sections[i];
        if(i == 0)
        {
            info.curSize.Set(curSection.width, curSection.height);
            info.StorageBlocks.ClearAll();
            info.storageBlockSize = info.curSize/float(info.seperationUnits);
            info.storageBlockSizeI = 1.0f/info.storageBlockSize;

            StorageData data;
            data.item = i;
            data.pos.Set(0.0f, 0.0f);
            data.bRot = FALSE;
            info.CurStorage << data;

            info.RebuildStorageBlocks();

            continue;
        }

        int x, y, sizeX, sizeY, bestExtendX = 0, bestExtendY = info.seperationUnits;
        int bestExtend = 0x7FFFFFFF;
        BOOL bRot, bBestExtendRot = FALSE;
        bool bFoundSpot = false;

        //---------------------------------------------------------------------
        // search for empty spots
        for(bRot=0; bRot<2; bRot++)
        {
            float curHeight, curWidth;
            if(bRot)
            {
                curWidth  = curSection.height;
                curHeight = curSection.width;
            }
            else
            {
                curWidth  = curSection.width;
                curHeight = curSection.height;
            }

            info.PosToBlock(Vect2(curWidth, curHeight), sizeX, sizeY, true);

            int endX = (info.seperationUnits-sizeX), endY = (info.seperationUnits-sizeY);

            for(y=0; y<info.seperationUnits; y++)
            {
                DWORD yPos = y*info.seperationUnits;
                for(x=0; x<info.seperationUnits; x++)
                {
                    if(!info.StorageBlocks[yPos+x])
                    {
                        int checkEndX = x+sizeX, checkEndY = y+sizeY, newExtend;
                        bool bUsable = true, bExtendCheck = false;

                        if(checkEndX > info.seperationUnits || checkEndY > info.seperationUnits)
                        {
                            int extendX=0, extendY=0;
                            if(checkEndX > info.seperationUnits)
                                extendX = (checkEndX-info.seperationUnits);
                            if(checkEndY > info.seperationUnits)
                                extendY = (checkEndY-info.seperationUnits);

                            if(info.curSize.x > info.curSize.y)
                            {
                                extendY -= info.availableOffsetBlocks;
                                if(extendY < 0) extendY = 0;
                            }
                            else if(info.curSize.y > info.curSize.x)
                            {
                                extendX -= info.availableOffsetBlocks;
                                if(extendX < 0) extendX = 0;
                            }

                            newExtend = extendX+extendY;

                            if(newExtend >= bestExtend)
                                continue;

                            checkEndX = MIN(checkEndX, info.seperationUnits);
                            checkEndY = MIN(checkEndY, info.seperationUnits);
                            bExtendCheck = true;
                        }

                        for(int testY=y; testY<checkEndY; testY++)
                        {
                            DWORD yTestPos = testY*info.seperationUnits;
                            for(int testX=x; testX<checkEndX; testX++)
                            {
                                if(info.StorageBlocks[yTestPos+testX])
                                {
                                    bUsable = false;
                                    break;
                                }
                            }
                            if(!bUsable) break;
                        }

                        //spot found
                        if(bUsable)
                        {
                            if(bExtendCheck)
                            {
                                bestExtend = newExtend;
                                bestExtendX = x;
                                bestExtendY = y;
                                bBestExtendRot = bRot;
                            }
                            else
                            {
                                bFoundSpot = true;
                                break;
                            }
                        }
                    }
                }

                if(bFoundSpot)
                    break;
            }

            if(bFoundSpot)
                break;
        }

        if(bFoundSpot)
        {
            StorageData data;
            data.item = i;
            data.bRot = bRot;
            info.BlockToPos(x, y, data.pos);
            info.CurStorage << data;
            info.RebuildStorageBlocks(true, &data); //rebuild only the new section
            continue;
        }

        //---------------------------------------------------------------------
        // no empty spots found, use whatever best spot was found to extend the current stuff
        if(bestExtend == 0x7FFFFFFF)
        {
            if(info.curSize.x > info.curSize.y)
                bBestExtendRot = (curSection.width < curSection.height);
            else
                bBestExtendRot = (curSection.width > curSection.height);
        }

        StorageData data;
        data.item = i;
        data.bRot = bBestExtendRot;
        info.BlockToPos(bestExtendX, bestExtendY, data.pos);

        if(bBestExtendRot)
        {
            Vect2 maxVal(data.pos.x+curSection.height, data.pos.y+curSection.width);
            info.curSize.x = MAX(maxVal.x, info.curSize.x);
            info.curSize.y = MAX(maxVal.y, info.curSize.y);
        }
        else
        {
            Vect2 maxVal(data.pos.x+curSection.width, data.pos.y+curSection.height);
            info.curSize.x = MAX(maxVal.x, info.curSize.x);
            info.curSize.y = MAX(maxVal.y, info.curSize.y);
        }

        info.CurStorage << data;
        info.RebuildStorageBlocks();

        nop();
    }
}


void EditorMesh::BuildLightmapUVs(float maxAngle, float adjVal, int seperationUnits)
{
    Vect maxSize = bounds.Max-bounds.Min;

    //adjVal *= MAX(maxSize.x, MAX(maxSize.y, maxSize.z));
    adjVal *= bounds.GetDiamater();

    int i, j;

    MakePolyEdges();

    maxAngle = cosf(RAD(maxAngle));

    BitList ProcessedPolys, ProcessedEdges, ProcessedVerts;
    List<UINT> UnprocessedPolys;

    ProcessedPolys.SetSize(PolyList.Num());
    ProcessedEdges.SetSize(PolyEdgeList.Num());
    ProcessedVerts.SetSize(VertList.Num());

    UnprocessedPolys.SetSize(PolyList.Num());
    for(i=0; i<UnprocessedPolys.Num(); i++)
        UnprocessedPolys[i] = i;

    List<UVSection> Sections;
    BitList         SplitVerts;

    SplitVerts.SetSize(VertList.Num());

    //go through each polygon and find neighboring polygons
    while(UnprocessedPolys.Num())
    {
        UINT curPoly = UnprocessedPolys[0];

        List<UINT> SectionPolys;
        UVSection &section = *Sections.CreateNew();
        section.mapDir = GetPolyDir(curPoly);

        List<Vect> PolyLines;

        // calling this recursively will build us up a section based upon this poly
        AddLMPolyInfo info(curPoly, maxAngle, section.mapDir, SectionPolys, PolyLines, UnprocessedPolys, ProcessedPolys, ProcessedEdges);
        AddLightmapUVPoly(info);

        section.mapDir.Norm();

        //find best mapping direction
        Vect XDir, YDir;

        BitList addedLines;
        addedLines.SetSize(PolyLines.Num());

        float bestLength = 0.0f;
        Vect bestDir;

        for(i=0; i<PolyLines.Num(); i++)
        {
            if(addedLines[i]) continue;

            addedLines.Set(i);

            Vect &line1 = PolyLines[i];
            Vect line1Norm = (line1 + (section.mapDir * -section.mapDir.Dot(line1))).Norm();

            Vect lineTotal = line1;

            for(j=i+1; j<PolyLines.Num(); j++)
            {
                if(addedLines[j]) continue;

                Vect &line2 = PolyLines[j];
                Vect line2Norm = (line2 + (section.mapDir * -section.mapDir.Dot(line2))).Norm();

                if(line1Norm.Dot(line2Norm) > 0.9f) //about 10 degrees
                {
                    lineTotal += line2;
                    addedLines.Set(j);
                }
            }

            float length = lineTotal.Len();
            if(length > bestLength)
            {
                bestDir = lineTotal.Norm();
                bestLength = length;
            }
        }

        if(section.mapDir.CloseTo(bestDir) || (-section.mapDir).CloseTo(bestDir))
        {
            if(bestDir.x > bestDir.y)
            {
                if(bestDir.x > bestDir.z)
                    *(DWORD*)&bestDir.x |= 0x80000000;
                else
                    *(DWORD*)&bestDir.z |= 0x80000000;
            }
            else
            {
                if(bestDir.y > bestDir.z)
                    *(DWORD*)&bestDir.y |= 0x80000000;
                else
                    *(DWORD*)&bestDir.z |= 0x80000000;
            }
        }

        YDir = bestDir.Cross(section.mapDir).Norm();
        XDir = YDir.Cross(section.mapDir).Norm();

        /*if(section.mapDir.GetAbs().CloseTo(Vect(0.0f, 1.0f, 0.0f)))
        {
            float fOne = (*(DWORD*)&section.mapDir.y & 0x80000000) ? -1.0f : 1.0f;

            XDir.Set(fOne, 0.0f, 0.0f);
            YDir.Set(0.0f, 0.0f, fOne);
        }
        else
        {
            XDir = Vect(0.0f, 1.0f, 0.0f).Cross(section.mapDir).Norm();
            YDir = XDir.Cross(section.mapDir).Norm();
        }*/

        float top, bottom, left, right;

        //project UVs onto this section
        for(i=0; i<SectionPolys.Num(); i++)
        {
            PolyFace &poly = PolyList[SectionPolys[i]];
            for(j=0; j<poly.Faces.Num(); j++)
            {
                UINT faceID = poly.Faces[j];
                Face &f = FaceList[faceID];

                section.Faces << faceID;

                for(int k=0; k<3; k++)
                {
                    UINT vert = f.ptr[k];

                    Vect &v = VertList[vert];

                    Vect2 uv;
                    uv.x = XDir.Dot(v);
                    uv.y = YDir.Dot(v);

                    if(!section.ProjectedVerts.Num())
                    {
                        left = right = uv.x;
                        top = bottom = uv.y;
                    }
                    else
                    {
                        if(uv.x < left) left = uv.x;
                        else if(uv.x > right) right = uv.x;

                        if(uv.y < top) top = uv.y;
                        else if(uv.y > bottom) bottom = uv.y;
                    }

                    section.Verts << vert;
                    section.ProjectedVerts << uv;
                }
            }
        }

        section.width  = (right-left)+(adjVal*2.0f);
        section.height = (bottom-top)+(adjVal*2.0f);

        for(i=0; i<section.Verts.Num(); i++)
        {
            section.ProjectedVerts[i].x -= left-adjVal;
            section.ProjectedVerts[i].y -= top-adjVal;
        }

        //find any verts that might need splitting
        section.SplitVerts.SetSize(section.Verts.Num());

        for(i=0; i<section.Verts.Num(); i++)
        {
            BOOL bFoundVert = FALSE;
            UINT vert = section.Verts[i];

            if(SplitVerts[vert])
                bFoundVert = TRUE;
            else 
            {
                for(j=0; j<Sections.Num()-1; j++)
                {
                    UVSection &prevSection = Sections[j];

                    if(prevSection.Verts.HasValue(vert))
                    {
                        SplitVerts.Set(vert);
                        bFoundVert = TRUE;
                        break;
                    }
                }
            }

            if(bFoundVert)
                section.SplitVerts.Set(i);
        }
    }

    //split verts where the lightmap coordinates are
    for(i=1; i<Sections.Num(); i++)
    {
        UVSection &section = Sections[i];
        for(j=0; j<section.SplitVerts.Num(); j++)
        {
            if(section.SplitVerts[j])
            {
                UINT faceID  = j/3;
                UINT faceVert = j%3;
                UINT vertID = section.Verts[j];

                UINT newVertID = VertList.Num();
                Face &f = FaceList[faceID];

                f.ptr[faceVert] = newVertID;

                VertList.Add(VertList[vertID]);
                UVList.Add(UVList[vertID]);
                NormalList.Add(NormalList[vertID]);
                if(TangentList.Num())
                    TangentList.Add(TangentList[vertID]);
            }
        }
    }

    LMUVList.SetSize(VertList.Num());

    //find the most optimal way to put sections together
    BestStorageInfo info(Sections, seperationUnits);
    FindBestUVStorage(info);

    float curSizeI = 1.0f/MAX(info.curSize.x, info.curSize.y);

    for(i=0; i<info.CurStorage.Num(); i++)
    {
        StorageData &data = info.CurStorage[i];
        UVSection &section = Sections[data.item];
        for(j=0; j<section.Verts.Num(); j++)
        {
            UINT vertID = section.Verts[j];
            if(ProcessedVerts[vertID]) continue;

            Vect2 &v = section.ProjectedVerts[j];
            Vect2 newPos;

            if(data.bRot)
            {
                newPos.x = section.height-v.y;
                newPos.y = v.x;
            }
            else
                newPos = v;

            LMUVList[vertID] = (data.pos+newPos)*curSizeI;

            ProcessedVerts.Set(vertID);
        }
    }

    //free data
    for(i=0; i<Sections.Num(); i++)
        Sections[i].FreeData();
    Sections.Clear();

    FreePolyEdges();
}

void EditorMesh::AddLightmapUVPoly(AddLMPolyInfo &info)
{
    int i;

    info.UnprocessedPolys.RemoveItem(info.curPoly);
    info.ProcessedPolys.Set(info.curPoly);
    info.SectionPolys.Add(info.curPoly);

    //try to find new poly
    PolyEdgeData &polyEdgeData = PolyEdgeDataList[info.curPoly];
    for(i=0; i<polyEdgeData.PolyEdges.Num(); i++)
    {
        UINT edgeID = polyEdgeData.PolyEdges[i];
        if(info.ProcessedEdges[edgeID]) continue;

        Edge &edge = PolyEdgeList[edgeID];

        UINT poly2 = (edge.f1 == info.curPoly) ? edge.f2 : edge.f1;
        if(poly2 == INVALID)
        {
            info.PolyLines << (VertList[edge.v2]-VertList[edge.v1]).Abs();
            continue;
        }

        if(info.ProcessedPolys[poly2])
        {
            if(!info.SectionPolys.HasValue(poly2))
                info.PolyLines << (VertList[edge.v2]-VertList[edge.v1]).Abs();
            continue;
        }

        Vect polyDir = GetPolyDir(poly2);
        if(info.mapDir.GetNorm().Dot(polyDir) >= info.maxAngle)
        {
            info.mapDir += polyDir;

            info.curPoly = poly2;
            AddLightmapUVPoly(info);

            info.ProcessedEdges.Set(edgeID);
        }
        else
            info.PolyLines << (VertList[edge.v2]-VertList[edge.v1]).Abs();
    }
}

Vect EditorMesh::GetPolyDir(UINT polyID)
{
    PolyFace &poly = PolyList[polyID];

    Vect polyDir(0.0f, 0.0f, 0.0f);
    for(int i=0; i<poly.Faces.Num(); i++)
        polyDir += PlaneList[FacePlaneList[poly.Faces[i]]].Dir;
    return polyDir.Norm();
}


/*void EditorMesh::BuildLightmapUVs()
{
    int i, j, k;

    BitList ProcessedVerts;
    LMUVList.SetSize(VertList.Num());

    for(i=0; i<PolyList.Num(); i++)
    {
        List<DWORD> &faces = PolyList[i].Faces;
        if(!faces.Num()) continue;

        Plane &plane = PlaneList[FacePlaneList[faces[0]]];

        ProcessedVerts.SetSize(VertList.Num());

        Vect tangent, binormal;
        float left, right, top, bottom;

        tangent = -GetFaceTangent(faces[0]);
        binormal = (-tangent).Cross(plane.Dir).Norm();

        for(j=0; j<faces.Num(); j++)
        {
            Face &face = FaceList[faces[j]];
            for(k=0; k<3; k++)
            {
                DWORD vertID = face.ptr[k];

                if(!ProcessedVerts[vertID])
                {
                    Vect &vert = VertList[vertID];

                    float fU = tangent.Dot(vert);
                    float fV = binormal.Dot(vert);

                    if(!k && !j)
                    {
                        left = right = fU;
                        top = bottom = fV;
                    }
                    else
                    {
                        if(fU < left) left = fU;
                        else if(fU > right) right = fU;

                        if(fV < top) top = fV;
                        else if(fV > bottom) bottom = fV;
                    }

                    LMUVList[vertID].Set(fU, fV);
                    ProcessedVerts.Set(vertID);
                }
            }
        }

        float fMapWidth  = right-left;
        float fMapHeight = bottom-top;

        float widthAdj   = fMapWidth*0.05f;
        float heightAdj  = fMapHeight*0.05f;

        left -= widthAdj;  right  += widthAdj;
        top  -= heightAdj; bottom += heightAdj;

        fMapWidth  += widthAdj*2.0f;
        fMapHeight += heightAdj*2.0f;

        ProcessedVerts.ClearAll();

        if(!CloseFloat(fMapWidth*fMapHeight, 0.0f, EPSILON))
        {
            float fSizeI = 1.0f/MAX(fMapWidth, fMapHeight);

            for(j=0; j<faces.Num(); j++)
            {
                Face &face = FaceList[faces[j]];
                for(k=0; k<3; k++)
                {
                    DWORD vertID = face.ptr[k];

                    if(!ProcessedVerts[vertID])
                    {
                        UVCoord &uv = LMUVList[vertID];

                        uv.x -= left;
                        uv.x *= fSizeI;
                        uv.y -= top;
                        uv.y *= fSizeI;

                        ProcessedVerts.Set(vertID);
                    }
                }
            }
        }
    }
}*/

void EditorBrush::BuildLightmapUVs()
{
    Brush *brush = GetLevelBrush();
    VBData *vbd = brush->VertBuffer->GetData();

    mesh.BuildLightmapUVs();

    if(vbd->TVList.Num() == 1)
    {
        vbd->TVList.SetSize(2);
        vbd->TVList[1].SetWidth(2);
        vbd->TVList[1].GetV2()->TransferFrom(mesh.LMUVList);
        brush->VertBuffer->FlushBuffers(TRUE);
    }
    else
    {
        vbd->TVList[1].SetWidth(2);
        vbd->TVList[1].GetV2()->TransferFrom(mesh.LMUVList);
        brush->VertBuffer->FlushBuffers(FALSE);
    }
}

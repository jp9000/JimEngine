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

//need..  polys, faces, verts, normals, tangents, binormals

//LightmapSections
//PlaneList
//FaceList
//FacePlaneList
//VertList
//NormalList
//TangentList

//then..
//generate binormals
//generate tridata

//for each brush
//  build info above
//  generate texelinfo
//  for each light affecting brush:
//    do lightmapping


const Matrix NormalShadeVectors =  Matrix(Vect(-0.707106781f, -0.40824829046f, 0.57735026918f),
                                          Vect( 0.707106781f, -0.40824829046f, 0.57735026918f),
                                          Vect(         0.0f,  0.81649658092f, 0.57735026918f),
                                          Vect(         0.0f,            0.0f,           0.0f));


struct IBLThing
{
    Texture *Z, *Xneg, *Xpos, *Yneg, *Ypos;

    inline void Setup(int size)
    {
        Z = GS->CreateFrameBuffer(size, size, GS_RGBA32F, FALSE);
        Xneg = GS->CreateFrameBuffer(size/2, size, GS_RGBA32F, FALSE);
        Xpos = GS->CreateFrameBuffer(size/2, size, GS_RGBA32F, FALSE);
        Yneg = GS->CreateFrameBuffer(size, size/2, GS_RGBA32F, FALSE);
        Ypos = GS->CreateFrameBuffer(size, size/2, GS_RGBA32F, FALSE);
    }

    inline ~IBLThing() {FreeData();}

    inline void FreeData()
    {
        DestroyObject(Z);
        DestroyObject(Xneg);
        DestroyObject(Xpos);
        DestroyObject(Yneg);
        DestroyObject(Ypos);
    }
};

struct IBLRender
{
    OffscreenSurface *surfaceZ, *surfaceXneg, *surfaceXpos, *surfaceYneg, *surfaceYpos;
    Vect *Z, *Xneg, *Xpos, *Yneg, *Ypos;

    inline IBLRender() {zero(this, sizeof(IBLRender));}
    inline IBLRender(int size)
    {
        Setup(size);
    }

    inline void Setup(int size)
    {
        int totalSize = size*size;
        int halfSize = size*(size/2);

        surfaceZ    = GS->CreateOffscreenSurface(size, size, GS_RGBA32F);
        surfaceXneg = GS->CreateOffscreenSurface(size/2, size, GS_RGBA32F);
        surfaceXpos = GS->CreateOffscreenSurface(size/2, size, GS_RGBA32F);
        surfaceYneg = GS->CreateOffscreenSurface(size, size/2, GS_RGBA32F);
        surfaceYpos = GS->CreateOffscreenSurface(size, size/2, GS_RGBA32F);
    }

    inline ~IBLRender() {FreeData();}

    inline void Lock(BOOL bReadOnly)
    {
        Z    = (Vect*)surfaceZ->Lock(bReadOnly);
        Xneg = (Vect*)surfaceXneg->Lock(bReadOnly);
        Xpos = (Vect*)surfaceXpos->Lock(bReadOnly);
        Yneg = (Vect*)surfaceYneg->Lock(bReadOnly);
        Ypos = (Vect*)surfaceYpos->Lock(bReadOnly);
    }

    inline void Unlock()
    {
        surfaceZ->Unlock();
        surfaceXneg->Unlock();
        surfaceXpos->Unlock();
        surfaceYneg->Unlock();
        surfaceYpos->Unlock();

        Z    = NULL;
        Xneg = NULL;
        Xpos = NULL;
        Yneg = NULL;
        Ypos = NULL;
    }

    inline void FreeData()
    {
        DestroyObject(surfaceZ);
        DestroyObject(surfaceXneg);
        DestroyObject(surfaceXpos);
        DestroyObject(surfaceYneg);
        DestroyObject(surfaceYpos);

        zero(this, sizeof(IBLRender));
    }
};

int thingSize = 64;
float maxPhotonDist = 50.0f, gammaVal, brightnessVal;
LightmapSettings *curSettings = NULL;
BOOL bUseIBLStuff = FALSE, bFinalPass, bCancelLightmapRendering = FALSE;
IBLRender *iblAxiiMask[3], *iblFullMask;



BOOL CALLBACK LightmapStatusDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK LightmapStatusDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_COMMAND && LOWORD(wParam) == IDCANCEL)
    {
        bCancelLightmapRendering = TRUE;
        EnableWindow((HWND)lParam, FALSE);
    }

    return FALSE;
};

struct BGR
{
    BYTE B, G, R;

    inline Color3 GetColor3() const {return Color3(float(R)/255.0f, float(G)/255.0f, float(B)/255.0f);}
};

struct BGRA
{
    BYTE B, G, R, A;

    inline Color3 GetColor3() const {return Color3(float(R)/255.0f, float(G)/255.0f, float(B)/255.0f);}
};

struct RGBA
{
    BYTE R, G, B, A;
};

struct RGBVal
{
    BYTE R, G, B;
};


BGRA *ConvertToBGRA(Vect *&tex, DWORD sizeX, DWORD sizeY);
BGRA *ConvertToBGRA(Vect *&tex, DWORD sizeX, DWORD sizeY)
{
    DWORD totalSize = sizeX*sizeY;

    BGRA *newTex = (BGRA*)Allocate(totalSize*sizeof(BGRA));

    for(int i=0; i<totalSize; i++)
    {
        if(tex[i].x > 1.0f) tex[i].x = 1.0f;
        if(tex[i].y > 1.0f) tex[i].y = 1.0f;
        if(tex[i].z > 1.0f) tex[i].z = 1.0f;

        newTex[i].R = BYTE(tex[i].x*255.0f);
        newTex[i].G = BYTE(tex[i].y*255.0f);
        newTex[i].B = BYTE(tex[i].z*255.0f);
        newTex[i].A = 0xFF;
    }
        
    Free(tex);
    tex = NULL;

    return newTex;
}

#ifdef USE_SSE
inline void switchXZ(Vect &v) {v.m = _mm_shuffle_ps(v.m, v.m, _MM_SHUFFLE(3, 0, 1, 2));}
#else
inline void switchXZ(Vect &v) {float x = v.x; v.x = v.x; v.z = v.x;}
#endif


RGBA *ConvertToRGBA(Vect *&tex, DWORD sizeX, DWORD sizeY);
RGBA *ConvertToRGBA(Vect *&tex, DWORD sizeX, DWORD sizeY)
{
    DWORD totalSize = sizeX*sizeY;

    RGBA *newTex = (RGBA*)Allocate(totalSize*sizeof(RGBA));

    for(int i=0; i<totalSize; i++)
    {
        if(tex[i].x > 1.0f) tex[i].x = 1.0f;
        if(tex[i].y > 1.0f) tex[i].y = 1.0f;
        if(tex[i].z > 1.0f) tex[i].z = 1.0f;

        newTex[i].R = BYTE(tex[i].x*255.0f);
        newTex[i].G = BYTE(tex[i].y*255.0f);
        newTex[i].B = BYTE(tex[i].z*255.0f);
        newTex[i].A = 0xFF;
    }
        
    Free(tex);
    tex = NULL;

    return newTex;
}


RGBVal *ConvertToRGB(Vect *&tex, DWORD sizeX, DWORD sizeY);
RGBVal *ConvertToRGB(Vect *&tex, DWORD sizeX, DWORD sizeY)
{
    DWORD totalSize = sizeX*sizeY;

    RGBVal *newTex = (RGBVal*)Allocate(totalSize*sizeof(RGBVal));

    for(int i=0; i<totalSize; i++)
    {
        if(tex[i].x > 1.0f) tex[i].x = 1.0f;
        if(tex[i].y > 1.0f) tex[i].y = 1.0f;
        if(tex[i].z > 1.0f) tex[i].z = 1.0f;

        newTex[i].R = BYTE(tex[i].x*255.0f);
        newTex[i].G = BYTE(tex[i].y*255.0f);
        newTex[i].B = BYTE(tex[i].z*255.0f);
    }
        
    Free(tex);
    tex = NULL;

    return newTex;
}

inline void ProcessWindowMessages()
{
    MSG msg;

    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if(!IsDialogMessage(levelInfo->hwndProgressBox, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void BuildIBLAxii()
{
    int y, x;

    float sizeI = 1.0f/float(thingSize);
    const float posAdj = sizeI*0.5f;

    iblAxiiMask[0] = new IBLRender(thingSize);
    iblAxiiMask[1] = new IBLRender(thingSize);
    iblAxiiMask[2] = new IBLRender(thingSize);
    iblFullMask = new IBLRender(thingSize);

    iblFullMask->Lock(FALSE);
    iblAxiiMask[0]->Lock(FALSE);
    iblAxiiMask[1]->Lock(FALSE);
    iblAxiiMask[2]->Lock(FALSE);

    Matrix invShadeVectors = NormalShadeVectors;
    //invShadeVectors.Transpose();
    invShadeVectors.X.x = -NormalShadeVectors.X.x;
    invShadeVectors.X.y = -NormalShadeVectors.X.y;

    invShadeVectors.Y.x = -NormalShadeVectors.Y.x;
    invShadeVectors.Y.y = -NormalShadeVectors.Y.y;

    invShadeVectors.Z.x = -NormalShadeVectors.Z.x;
    invShadeVectors.Z.y = -NormalShadeVectors.Z.y;

    float pixelSumI = 0.0f;
    float pixelSumIX = 0.0f;
    float pixelSumIY = 0.0f;
    float pixelSumIZ = 0.0f;

    //Z
    for(y=0; y<thingSize; y++)
    {
        DWORD yPos = y*thingSize;
        for(x=0; x<thingSize; x++)
        {
            DWORD texPos = yPos+x;
            Vect pos = Vect((float(x)*sizeI)+posAdj, -((float(y)*sizeI)+posAdj), 1.0f);
            pos.x = (pos.x*2.0f)-1.0f;
            pos.y = (pos.y*2.0f)+1.0f;

            Vect norm = pos.GetNorm();

            float shade = norm.z;
            float hemiAdjust = norm.z;
            float hemiVal = shade*hemiAdjust*hemiAdjust*hemiAdjust; //I know what this looks like.  just trust me.  power of three.  it works.
            float hemiValX = norm.Dot(invShadeVectors.X)*hemiVal;
            float hemiValY = norm.Dot(invShadeVectors.Y)*hemiVal;
            float hemiValZ = norm.Dot(invShadeVectors.Z)*hemiVal;

            pixelSumI += hemiVal;
            pixelSumIX += hemiValX;
            pixelSumIY += hemiValY;
            pixelSumIZ += hemiValZ;

            iblFullMask->Z[texPos] = hemiVal;
            iblAxiiMask[0]->Z[texPos] = hemiValX;
            iblAxiiMask[1]->Z[texPos] = hemiValY;
            iblAxiiMask[2]->Z[texPos] = hemiValZ;
        }
    }

    //XNeg
    for(y=0; y<thingSize; y++)
    {
        DWORD yPos = y*(thingSize/2);
        for(x=0; x<(thingSize/2); x++)
        {
            DWORD texPos = yPos+x;
            Vect pos = Vect(-1.0f, -((float(y)*sizeI)+posAdj), (float(x)*sizeI)+posAdj);
            pos.y = (pos.y*2.0f)+1.0f;
            pos.z = (pos.z*2.0f);

            Vect norm = pos.GetNorm();

            float shade = norm.z;
            float hemiAdjust = -norm.x;
            float hemiVal = shade*hemiAdjust*hemiAdjust*hemiAdjust;
            float hemiValX = norm.Dot(invShadeVectors.X)*hemiVal;
            float hemiValY = norm.Dot(invShadeVectors.Y)*hemiVal;
            float hemiValZ = norm.Dot(invShadeVectors.Z)*hemiVal;

            pixelSumI += hemiVal;
            pixelSumIX += hemiValX;
            pixelSumIY += hemiValY;
            pixelSumIZ += hemiValZ;

            iblFullMask->Xneg[texPos] = hemiVal;
            iblAxiiMask[0]->Xneg[texPos] = hemiValX;
            iblAxiiMask[1]->Xneg[texPos] = hemiValY;
            iblAxiiMask[2]->Xneg[texPos] = hemiValZ;
        }
    }

    //XPos
    for(y=0; y<thingSize; y++)
    {
        DWORD yPos = y*(thingSize/2);
        for(x=0; x<(thingSize/2); x++)
        {
            DWORD texPos = yPos+x;
            Vect pos = Vect(1.0f, -((float(y)*sizeI)+posAdj), 1.0f-((float(x)*sizeI)+posAdj));
            pos.y = (pos.y*2.0f)+1.0f;
            pos.z = (pos.z*2.0f)-1.0f;

            Vect norm = pos.GetNorm();

            float shade = norm.z;
            float hemiAdjust = norm.x;
            float hemiVal = shade*hemiAdjust*hemiAdjust*hemiAdjust;
            float hemiValX = norm.Dot(invShadeVectors.X)*hemiVal;
            float hemiValY = norm.Dot(invShadeVectors.Y)*hemiVal;
            float hemiValZ = norm.Dot(invShadeVectors.Z)*hemiVal;

            pixelSumI += hemiVal;
            pixelSumIX += hemiValX;
            pixelSumIY += hemiValY;
            pixelSumIZ += hemiValZ;

            iblFullMask->Xpos[texPos] = hemiVal;
            iblAxiiMask[0]->Xpos[texPos] = hemiValX;
            iblAxiiMask[1]->Xpos[texPos] = hemiValY;
            iblAxiiMask[2]->Xpos[texPos] = hemiValZ;
        }
    }

    //YNeg
    for(y=0; y<(thingSize/2); y++)
    {
        DWORD yPos = y*thingSize;
        for(x=0; x<thingSize; x++)
        {
            DWORD texPos = yPos+x;
            Vect pos = Vect((float(x)*sizeI)+posAdj, -1.0f, 1.0f-((float(y)*sizeI)+posAdj));
            pos.x = (pos.x*2.0f)-1.0f;
            pos.z = (pos.z*2.0f)-1.0f;

            Vect norm = pos.GetNorm();

            float shade = norm.z;
            float hemiAdjust = -norm.y;
            float hemiVal = shade*hemiAdjust*hemiAdjust*hemiAdjust;
            float hemiValX = norm.Dot(invShadeVectors.X)*hemiVal;
            float hemiValY = norm.Dot(invShadeVectors.Y)*hemiVal;
            float hemiValZ = norm.Dot(invShadeVectors.Z)*hemiVal;

            pixelSumI += hemiVal;
            pixelSumIX += hemiValX;
            pixelSumIY += hemiValY;
            pixelSumIZ += hemiValZ;

            iblFullMask->Yneg[texPos] = hemiVal;
            iblAxiiMask[0]->Yneg[texPos] = hemiValX;
            iblAxiiMask[1]->Yneg[texPos] = hemiValY;
            iblAxiiMask[2]->Yneg[texPos] = hemiValZ;
        }
    }

    //YPos
    for(y=0; y<(thingSize/2); y++)
    {
        DWORD yPos = y*thingSize;
        for(x=0; x<thingSize; x++)
        {
            DWORD texPos = yPos+x;
            Vect pos = Vect((float(x)*sizeI)+posAdj, 1.0f, (float(y)*sizeI)+posAdj);
            pos.x = (pos.x*2.0f)-1.0f;
            pos.z = (pos.z*2.0f);

            Vect norm = pos.GetNorm();

            float shade = norm.z;
            float hemiAdjust = norm.y;
            float hemiVal = shade*hemiAdjust*hemiAdjust*hemiAdjust;
            float hemiValX = norm.Dot(invShadeVectors.X)*hemiVal;
            float hemiValY = norm.Dot(invShadeVectors.Y)*hemiVal;
            float hemiValZ = norm.Dot(invShadeVectors.Z)*hemiVal;

            pixelSumI += hemiVal;
            pixelSumIX += hemiValX;
            pixelSumIY += hemiValY;
            pixelSumIZ += hemiValZ;

            iblFullMask->Ypos[texPos] = hemiVal;
            iblAxiiMask[0]->Ypos[texPos] = hemiValX;
            iblAxiiMask[1]->Ypos[texPos] = hemiValY;
            iblAxiiMask[2]->Ypos[texPos] = hemiValZ;
        }
    }

    //--------------------------------------------------------------------------------

    pixelSumI = 1.0f/pixelSumI;
    pixelSumIX = 1.0f/pixelSumIX;
    pixelSumIY = 1.0f/pixelSumIY;
    pixelSumIZ = 1.0f/pixelSumIZ;

    for(y=0; y<thingSize; y++)
    {
        DWORD yPos = y*thingSize;
        for(x=0; x<thingSize; x++)
        {
            DWORD texPos = yPos+x;

            iblFullMask->Z[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[0]->Z[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[1]->Z[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[2]->Z[texPos].ClampMin(0.0f).ClampMax(1.0f);

            iblFullMask->Z[texPos] *= pixelSumI;
            iblAxiiMask[0]->Z[texPos] *= pixelSumI;
            iblAxiiMask[1]->Z[texPos] *= pixelSumI;
            iblAxiiMask[2]->Z[texPos] *= pixelSumI;
        }
    }

    for(y=0; y<thingSize; y++)
    {
        DWORD yPos = y*(thingSize/2);
        for(x=0; x<(thingSize/2); x++)
        {
            DWORD texPos = yPos+x;

            iblFullMask->Xneg[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[0]->Xneg[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[1]->Xneg[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[2]->Xneg[texPos].ClampMin(0.0f).ClampMax(1.0f);

            iblFullMask->Xneg[texPos] *= pixelSumI;
            iblAxiiMask[0]->Xneg[texPos] *= pixelSumI;
            iblAxiiMask[1]->Xneg[texPos] *= pixelSumI;
            iblAxiiMask[2]->Xneg[texPos] *= pixelSumI;

            iblAxiiMask[0]->Xpos[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[1]->Xpos[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[2]->Xpos[texPos].ClampMin(0.0f).ClampMax(1.0f);

            iblFullMask->Xpos[texPos] *= pixelSumI;
            iblAxiiMask[0]->Xpos[texPos] *= pixelSumI;
            iblAxiiMask[1]->Xpos[texPos] *= pixelSumI;
            iblAxiiMask[2]->Xpos[texPos] *= pixelSumI;
        }
    }

    for(y=0; y<(thingSize/2); y++)
    {
        DWORD yPos = y*thingSize;
        for(x=0; x<thingSize; x++)
        {
            DWORD texPos = yPos+x;

            iblAxiiMask[0]->Yneg[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[1]->Yneg[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[2]->Yneg[texPos].ClampMin(0.0f).ClampMax(1.0f);

            iblFullMask->Yneg[texPos] *= pixelSumI;
            iblAxiiMask[0]->Yneg[texPos] *= pixelSumI;
            iblAxiiMask[1]->Yneg[texPos] *= pixelSumI;
            iblAxiiMask[2]->Yneg[texPos] *= pixelSumI;

            iblAxiiMask[0]->Ypos[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[1]->Ypos[texPos].ClampMin(0.0f).ClampMax(1.0f);
            iblAxiiMask[2]->Ypos[texPos].ClampMin(0.0f).ClampMax(1.0f);

            iblFullMask->Ypos[texPos] *= pixelSumI;
            iblAxiiMask[0]->Ypos[texPos] *= pixelSumI;
            iblAxiiMask[1]->Ypos[texPos] *= pixelSumI;
            iblAxiiMask[2]->Ypos[texPos] *= pixelSumI;
        }
    }
}


void EditorLevelInfo::UpdateLightmapProgress(float percentage, CTSTR lpText, CTSTR lpText2)
{
    HWND hwndText     = GetDlgItem(hwndProgressBox, IDC_ACTION);
    HWND hwndText2    = GetDlgItem(hwndProgressBox, IDC_ACTION2);
    HWND hwndProgress = GetDlgItem(hwndProgressBox, IDC_PROGRESS);

    int val = int(percentage);
    SendMessage(hwndProgress, PBM_SETPOS, val, 0);

    SetWindowText(hwndText,  lpText);
    SetWindowText(hwndText2, lpText2);
}

void EditorLevelInfo::BuildLightmaps(LightmapSettings &settings)
{
    traceIn(EditorLevelInfo::BuildLightmaps);

    bCancelLightmapRendering = FALSE;

    level->bHasLightmaps = FALSE;

    OSColorAdjust();
    EnableWindow(hwndEditor, FALSE);
    hwndProgressBox = CreateDialog(hinstMain, MAKEINTRESOURCE(IDD_STATUSBOX), hwndEditor, (DLGPROC)LightmapStatusDialogProc);
    NotifyWinEvent(EVENT_SYSTEM_DIALOGSTART, hwndProgressBox, 0, 0);
    SendMessage(hwndEditor, WM_ENTERIDLE, 0, 0);

    engine->bBlockViewUpdates = TRUE;

    curSettings = &settings;
    thingSize = settings.hemicubeResolution;
    maxPhotonDist = settings.maxPhotonDist;

    gammaVal = 1.0f/curSettings->gamma;
    brightnessVal = curSettings->brightness-1.0f;

    if(settings.bUseGI || settings.bUseIBL || settings.bUseAO)
        BuildIBLAxii();

    OSCreateDirectory(TEXT("EditorTemp"));

    LightmapScene scene;

    bFinalPass = FALSE;

    int i, j;

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        Brush *baseBrush = brush->GetLevelBrush();

        if(!brush->bUseLightmapping)
            continue;

        VBData *data = baseBrush->VertBuffer->GetData();
        if(data->TVList.Num() < 2)
        {
            UpdateLightmapProgress(0.0f, FormattedString(TEXT("Building Brush Lightmaps...\r\nBrush %d of %d"), i+1, BrushList.Num()), TEXT("Building Lightmap Coordinates"));
            brush->BuildLightmapUVs();
            UpdateLightmapProgress(100.0f, FormattedString(TEXT("Building Brush Lightmaps...\r\nBrush %d of %d"), i+1, BrushList.Num()), TEXT("Building Lightmap Coordinates"));
            Sleep(10);
        }
    }

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        Brush *baseBrush = brush->GetLevelBrush();

        baseBrush->lightmap.FreeData();
        baseBrush->bLightmapped = FALSE;

        if(!brush->bUseLightmapping)
            continue;

        scene.Brushes << brush;
    }

    Entity *ent = Entity::FirstEntity();
    while(ent)
    {
        if(!ent->IsOf(GetClass(MeshEntity)) || ent->IsOf(GetClass(AnimatedEntity)))
        {
            ent = ent->NextEntity();
            continue;
        }

        MeshEntity *meshEnt = static_cast<MeshEntity*>(ent);
        meshEnt->lightmap.FreeData();
        meshEnt->bLightmapped = FALSE;

        if(!meshEnt->bUseLightmapping)
        {
            ent = ent->NextEntity();
            continue;
        }

        if(meshEnt->mesh && !meshEnt->mesh->bHasAnimation)
        {
            BOOL bFoundMesh = FALSE;
            for(j=0; j<scene.LMMeshList.Num(); j++)
            {
                if(scene.LMMeshList[j].mesh == meshEnt->mesh)
                {
                    bFoundMesh = TRUE;
                    break;
                }
            }

            if(bFoundMesh)
                scene.LMMeshList[j].Entities << meshEnt;
            else
            {
                LMMesh &meshInfo = *scene.LMMeshList.CreateNew();
                meshInfo.Entities << meshEnt;
                meshInfo.mesh = meshEnt->mesh;
            }
        }

        ent = ent->NextEntity();
    }

    if(!settings.bUseAO || !settings.bVisualizeAO)
    {
        if(!settings.bUseGI)
            bFinalPass = TRUE;
        BuildSubLightmaps(scene, 0);

        if(settings.bUseGI && !bCancelLightmapRendering)
        {
            DWORD final = MIN(settings.nGIPasses, 2);
            for(i=0; i<final; i++)
            {
                UploadLightmaps(scene, FALSE);

                if(i == (final-1))
                    bFinalPass = TRUE;
                BuildSubLightmaps(scene, i+1);

                if(bCancelLightmapRendering)
                    break;
            }
        }
    }

    if(!bCancelLightmapRendering)
    {
        if( (settings.bUseAO && !settings.bUseGI && !settings.bUseIBL) ||
            (settings.bUseAO && settings.bVisualizeAO) )
        {
            bFinalPass = TRUE;

            if(!settings.bVisualizeAO)
                UploadLightmaps(scene, FALSE);
            BuildSubLightmaps(scene, settings.bVisualizeAO ? 0 : 1);
        }
        for(i=0; i<scene.LMMeshList.Num(); i++)
        {
            LMMesh &lmmeshInfo = scene.LMMeshList[i];
        }
    }

    if(!bCancelLightmapRendering)
    {
        UploadLightmaps(scene, TRUE);
    }

    for(i=0; i<scene.LMMeshList.Num(); i++)
        scene.LMMeshList[i].FreeData();

    for(i=0; i<scene.counter; i++)
    {
        String tempFileName;
        tempFileName << TEXT("EditorTemp/") << IntString(i) << TEXT(".tmp");
        DeleteFile(tempFileName);
    }
    RemoveDirectory(TEXT("EditorTemp"));

    if(!bCancelLightmapRendering)
    {
        for(i=0; i<Light::NumLights(); i++)
        {
            Light *light = Light::GetLight(i);
            if(light->bStaticLight)
            {
                light->bLightmapped = TRUE;
                light->Reinitialize();
            }
        }

        level->bHasLightmaps = TRUE;
    }
    else
    {
        for(i=0; i<Light::NumLights(); i++)
        {
            Light *light = Light::GetLight(i);
            if(light->bStaticLight)
            {
                light->bLightmapped = FALSE;
                light->Reinitialize();
            }
        }
        for(i=0; i<scene.Brushes.Num(); i++)
        {
            EditorBrush *brush = scene.Brushes[i];
            Brush *baseBrush = brush->GetLevelBrush();
            baseBrush->lightmap.FreeData();
            baseBrush->bLightmapped = FALSE;
        }
        for(i=0; i<scene.LMMeshList.Num(); i++)
        {
            LMMesh &lmmeshInfo = scene.LMMeshList[i];
            for(j=0; j<lmmeshInfo.Entities.Num(); j++)
            {
                MeshEntity *meshEnt = lmmeshInfo.Entities[j];
                meshEnt->lightmap.FreeData();
                meshEnt->bLightmapped = FALSE;
            }
        }
    }

    bModified = true;

    EnableWindow(hwndEditor, TRUE);
    NotifyWinEvent(EVENT_SYSTEM_DIALOGEND, hwndProgressBox, 0, 0);
    DestroyWindow(hwndProgressBox);

    float fGamma        = AppConfig->GetFloat(TEXT("Display"), TEXT("Gamma"), 1.0f);
    float fBrightness   = AppConfig->GetFloat(TEXT("Display"), TEXT("Brightness"), 1.0f);
    float fContrast     = AppConfig->GetFloat(TEXT("Display"), TEXT("Contrast"), 1.0f);
    OSColorAdjust(fGamma, fBrightness, fContrast);

    if(settings.bUseGI || settings.bUseIBL || settings.bUseAO)
    {
        iblFullMask->FreeData();
        iblAxiiMask[0]->FreeData();
        iblAxiiMask[1]->FreeData();
        iblAxiiMask[2]->FreeData();
        delete iblFullMask;
        delete iblAxiiMask[0];
        delete iblAxiiMask[1];
        delete iblAxiiMask[2];
    }

    engine->bBlockViewUpdates = FALSE;

    UpdateViewports();

    traceOut;
}



void LightmapInfo::BuildData()
{
    int i, j;

    triData.SetSize(mi.nFaces);
    for(j=0; j<mi.nFaces; j++)
    {
        Face &f = mi.Faces[j];
        TriData &data = triData[j];

        UVCoord UVs[3] = {mi.LightmapCoords[f.A], mi.LightmapCoords[f.B], mi.LightmapCoords[f.C]};
        UVCoord edgeVector;

        //effectively rotating the vectors by 90 degrees.
        edgeVector = (UVs[0]-UVs[1]).Norm();
        data.edgeVectors[0].Set(-edgeVector.y, edgeVector.x);

        edgeVector = (UVs[1]-UVs[2]).Norm();
        data.edgeVectors[1].Set(-edgeVector.y, edgeVector.x);

        edgeVector = (UVs[2]-UVs[0]).Norm();
        data.edgeVectors[2].Set(-edgeVector.y, edgeVector.x);

        data.edgeDist[0] = data.edgeVectors[0] | UVs[0];
        data.edgeDist[1] = data.edgeVectors[1] | UVs[1];
        data.edgeDist[2] = data.edgeVectors[2] | UVs[2];

        data.edgeAdj[0] = 1.0/((data.edgeVectors[0] | UVs[2]) - data.edgeDist[0]);
        data.edgeAdj[1] = 1.0/((data.edgeVectors[1] | UVs[0]) - data.edgeDist[1]);
        data.edgeAdj[2] = 1.0/((data.edgeVectors[2] | UVs[1]) - data.edgeDist[2]);
    }

    Binormals.SetSize(mi.nVerts);
    for(i=0; i<mi.nVerts; i++)
    {
        Binormals[i] = mi.TangentList[i].Cross(mi.NormalList[i]);
    }
}

void LightmapInfo::GenerateTexelInfo(DWORD size, LightmapRender &TexelData)
{
    const float padding = 4.0f;

    DWORD endFace = mi.nFaces;

    DWORD totalSize = size*size;

    TexelData.texelInfo.SetSize(totalSize);
    TexelData.baseLightmap = (Vect*)Allocate(totalSize*sizeof(Vect));
    TexelData.lightmap[0] = (Vect*)Allocate(totalSize*sizeof(Vect));
    TexelData.lightmap[1] = (Vect*)Allocate(totalSize*sizeof(Vect));
    TexelData.lightmap[2] = (Vect*)Allocate(totalSize*sizeof(Vect));

    zero(TexelData.baseLightmap, totalSize*sizeof(Vect));
    zero(TexelData.lightmap[0],  totalSize*sizeof(Vect));
    zero(TexelData.lightmap[1],  totalSize*sizeof(Vect));
    zero(TexelData.lightmap[2],  totalSize*sizeof(Vect));

    float fSize = float(size);
    float texOffset = 1.0f/fSize;
    float centerAdj = texOffset*0.5f;

    DWORD x, y;
    UVCoord UVPos;

    for(y=0; y<size; y++)
    {
        DWORD yPos = y*size;
        UVPos.y = (texOffset*float(y))+centerAdj;

        for(x=0; x<size; x++)
        {
            DWORD pos = yPos+x;
            TexelInfo &texel = TexelData.texelInfo[pos];

            UVPos.x = (texOffset*float(x))+centerAdj;

            DWORD bestTri = 0;
            float smallestDist = padding;
            float bestCoords[3];

            for(DWORD tri=0; tri<mi.nFaces; tri++)
            {
                TriData &data = triData[tri];
                float baryCoords[3];

                data.GetBaryCoords(UVPos, baryCoords);

                float tempCoords[3];
                mcpy(tempCoords, baryCoords, sizeof(float)*3);

                if(tempCoords[0] > 1.0)      tempCoords[0] -= 1.0;
                else if(tempCoords[0] > 0.0) tempCoords[0] = 0.0;

                if(tempCoords[1] > 1.0)      tempCoords[1] -= 1.0;
                else if(tempCoords[1] > 0.0) tempCoords[1] = 0.0;

                if(tempCoords[2] > 1.0)      tempCoords[2] -= 1.0;
                else if(tempCoords[2] > 0.0) tempCoords[2] = 0.0;

                float val = fabs((tempCoords[0]/data.edgeAdj[1])*fSize) +
                            fabs((tempCoords[1]/data.edgeAdj[2])*fSize) +
                            fabs((tempCoords[2]/data.edgeAdj[0])*fSize);

                if(CloseFloat(val, 0.0f, EPSILON))
                {
                    bestTri = tri+1;
                    mcpy(bestCoords, baryCoords, sizeof(float)*3);
                    break;
                }
                else if(val < smallestDist)
                {
                    smallestDist = val;
                    mcpy(bestCoords, baryCoords, sizeof(float)*3);
                    bestTri = tri+1;
                }
            }

            texel.tri = bestTri;
            if(texel.tri)
            {
                Face &face = mi.Faces[bestTri-1];
                mcpy(texel.bary, bestCoords, sizeof(float)*3);
                texel.pos = (mi.VertList[face.A]*bestCoords[0]) +
                            (mi.VertList[face.B]*bestCoords[1]) +
                            (mi.VertList[face.C]*bestCoords[2]);

                texel.pos += mi.Planes[mi.FacePlanes[bestTri-1]].Dir*0.02f;

                if(bHasTransform)
                    texel.pos.TransformPoint(transform);
            }
        }
    }
}

void LightmapInfo::GenerateLightData(Light *light)
{
    if(!LightVectors.Num())
        LightVectors.SetSize(mi.nVerts);

    Vect lightPos = light->GetWorldPos();
    if(bHasTransform)
        lightPos.TransformPoint(transform.GetTranspose());

    for(int i=0; i<mi.nVerts; i++)
    {
        Matrix m(-mi.TangentList[i], -Binormals[i], mi.NormalList[i], Vect(0.0f, 0.0f, 0.0f));

        Vect &lightVec = LightVectors[i];
        if(light->IsOf(GetClass(DirectionalLight)))
            lightVec = light->GetWorldRot().GetDirectionVector();
        else
            lightVec = lightPos-mi.VertList[i];
        lightVec.TransformVector(m);
    }
}

void *ConvertToDXT1(Vect *&tex, DWORD sizeX, DWORD sizeY);
void *ConvertToDXT1(Vect *&tex, DWORD sizeX, DWORD sizeY)
{
    BGRA *rgbaTex = ConvertToBGRA(tex, sizeX, sizeY);

    int dxtSize = squish::GetStorageRequirements(sizeX, sizeY, squish::kDxt1);
    void *dxt = Allocate(dxtSize);

    squish::CompressImage((squish::u8*)rgbaTex, sizeX, sizeY, dxt, squish::kDxt1|squish::kColourIterativeClusterFit);

    Free(rgbaTex);

    return dxt;
}

void *ConvertToDXT3(Vect *&tex, DWORD sizeX, DWORD sizeY);
void *ConvertToDXT3(Vect *&tex, DWORD sizeX, DWORD sizeY)
{
    RGBA *rgbaTex = ConvertToRGBA(tex, sizeX, sizeY);

    for(int y=0; y<sizeY; y++)
    {
        DWORD yPos = y*sizeX;
        for(int x=0; x<sizeX; x++)
        {
            RGBA &val = rgbaTex[yPos+x];
            val.A = val.R;
            val.R = 0;
        }
    }

    int dxtSize = squish::GetStorageRequirements(sizeX, sizeY, squish::kDxt3);
    void *dxt = Allocate(dxtSize);

    squish::CompressImage((squish::u8*)rgbaTex, sizeX, sizeY, dxt, squish::kDxt3|squish::kColourIterativeClusterFit);

    Free(rgbaTex);

    return dxt;
}

void DXT1ToBGRA(void *&tex, DWORD sizeX, DWORD sizeY);
void DXT1ToBGRA(void *&tex, DWORD sizeX, DWORD sizeY)
{
    BGRA *outTex = (BGRA*)Allocate(sizeX*sizeY*4);

    squish::DecompressImage((squish::u8*)outTex, sizeX, sizeY, tex, squish::kDxt1);
    for(int y=0; y<sizeY; y++)
    {
        int yPos = y*sizeX;
        for(int x=0; x<sizeX; x++)
        {
            BGRA &bgra = outTex[yPos+x];
            BYTE b = bgra.B;
            bgra.B = bgra.R;
            bgra.R = b;
        }
    }
    Free(tex);
    tex = (void*)outTex;
}

void DXT3ToBGRA(void *&tex, DWORD sizeX, DWORD sizeY);
void DXT3ToBGRA(void *&tex, DWORD sizeX, DWORD sizeY)
{
    BGRA *outTex = (BGRA*)Allocate(sizeX*sizeY*4);

    squish::DecompressImage((squish::u8*)outTex, sizeX, sizeY, tex, squish::kDxt3);
    for(int y=0; y<sizeY; y++)
    {
        int yPos = y*sizeX;
        for(int x=0; x<sizeX; x++)
        {
            BGRA &bgra = outTex[yPos+x];
            BYTE b = bgra.B;
            bgra.B = bgra.R;
            bgra.R = b;
        }
    }
    Free(tex);
    tex = (void*)outTex;
}

void DXT5ToBGRA(void *&tex, DWORD sizeX, DWORD sizeY);
void DXT5ToBGRA(void *&tex, DWORD sizeX, DWORD sizeY)
{
    BGRA *outTex = (BGRA*)Allocate(sizeX*sizeY*4);

    squish::DecompressImage((squish::u8*)outTex, sizeX, sizeY, tex, squish::kDxt5);
    for(int y=0; y<sizeY; y++)
    {
        int yPos = y*sizeX;
        for(int x=0; x<sizeX; x++)
        {
            BGRA &bgra = outTex[yPos+x];
            BYTE b = bgra.B;
            bgra.B = bgra.R;
            bgra.R = b;
        }
    }
    Free(tex);
    tex = (void*)outTex;
}

inline void AdjustColorVal(Vect &color)
{
    color.x = pow(color.x, gammaVal);
    color.y = pow(color.y, gammaVal);
    color.z = pow(color.z, gammaVal);

    color *= curSettings->contrast;
    color += brightnessVal;

    color.ClampMin(0.0f);
    color.w = 1.0f;
}

inline int GetBrushRes(EditorBrush *brush)
{
    switch(curSettings->resMultiplier)
    {
        case 0:
            return brush->lightmapResolution/2;
        case 2:
            return brush->lightmapResolution*2;
    }

    return brush->lightmapResolution;
}

inline int GetMeshRes(MeshEntity *meshEnt)
{
    switch(curSettings->resMultiplier)
    {
        case 0:
            return meshEnt->lightmapResolution/2;
        case 2:
            return meshEnt->lightmapResolution*2;
    }

    return meshEnt->lightmapResolution;
}

void EditorLevelInfo::BuildSubLightmaps(LightmapScene &scene, DWORD pass)
{
    int i, j, k;

    scene.counter = 0;

    BOOL bVisualizingAO = (curSettings->bUseAO && curSettings->bVisualizeAO);

    for(i=0; i<scene.Brushes.Num(); i++)
    {
        UpdateLightmapProgress(0.0f, FormattedString(TEXT("Building Brush Lightmaps...\r\nBrush %d of %d"), i+1, BrushList.Num()));

        EditorBrush *brush = scene.Brushes[i];
        Brush *baseBrush = brush->GetLevelBrush();

        List<LMLightInfo> Lights;

        if(!brush->bUseLightmapping)
            continue;

        if(pass == 0 && !curSettings->bUseIBL && !bVisualizingAO)
        {
            for(j=0; j<Light::NumLights(); j++)
            {
                Light *light = Light::GetLight(j);

                if(!light->bStaticLight) continue;

                if(light->IsOf(GetClass(PointLight)))
                {
                    PointLight *pointLight = (PointLight*)light;

                    if(baseBrush->bounds.SphereIntersects(pointLight->GetWorldPos(), pointLight->lightRange))
                    {
                        LMLightInfo lightInfo;
                        lightInfo.light = light;
                        lightInfo.ghost = physics->CreateGhost();
                        lightInfo.ghost->SetShape(physics->MakeSphere(pointLight->lightRange));
                        lightInfo.ghost->SetPos(pointLight->GetWorldPos());
                        lightInfo.ghost->UpdatePositionalData();
                        Lights << lightInfo;
                    }
                }
                else if(light->IsOf(GetClass(DirectionalLight)))
                {
                    LMLightInfo lightInfo;
                    lightInfo.light = light;
                    lightInfo.ghost = NULL;
                    Lights << lightInfo;
                }
                /*else if(light->IsOf(GetClass(SpotLight)))
                {
                    SpotLight *spotLight = (SpotLight*)light;

                    ViewClip clip;
                    Matrix rotMatrix;
                    rotMatrix.SetIdentity();
                    rotMatrix *= -spot->worldRot;
                    rotMatrix *= -spot->worldPos;

                    clip.planes.Clear();
                    clip.SetPerspective(spot->cutoff+1.0f, 1.0f, 1.0, 4096.0f);
                    clip.Transform(rotMatrix.GetTranspose());

                    if(clip.BoundsVisible(baseBrush->bounds))
                    {
                        LMLightInfo lightInfo;
                        lightInfo.light = light;
                        Lights << lightInfo;
                    }
                }*/
            }
        }

        baseBrush->bLightmapped = TRUE;

        VBData *vbd = baseBrush->VertBuffer->GetData();

        LightmapMeshInfo meshInfo;
        meshInfo.nPlanes            = brush->mesh.PlaneList.Num();
        meshInfo.nFaces             = brush->mesh.FaceList.Num();
        meshInfo.nVerts             = brush->mesh.VertList.Num();

        meshInfo.Planes             = brush->mesh.PlaneList.Array();
        meshInfo.Faces              = brush->mesh.FaceList.Array();
        meshInfo.FacePlanes         = brush->mesh.FacePlaneList.Array();
        meshInfo.VertList           = brush->mesh.VertList.Array();
        meshInfo.LightmapCoords     = (UVCoord*)vbd->TVList[1].Array();
        meshInfo.NormalList         = brush->mesh.NormalList.Array();
        meshInfo.TangentList        = baseBrush->TangentList;

        LightmapInfo lightmapInfo(meshInfo);

        UpdateLightmapProgress(33.0f, FormattedString(TEXT("Building Brush Lightmaps...\r\nBrush %d of %d"), i+1, BrushList.Num()), TEXT("Generating Texel Info"));

        String tempFileName;
        tempFileName << TEXT("EditorTemp/") << IntString(scene.counter) << TEXT(".tmp");

        LightmapRender render;
        if(pass == 0)
            lightmapInfo.GenerateTexelInfo(GetBrushRes(brush), render);
        else
        {
            XFileInputSerializer tempFile;
            if(!tempFile.Open(tempFileName))
            {
                ++scene.counter;
                continue;
            }

            render.Serialize(tempFile, GetBrushRes(brush), GetBrushRes(brush), TRUE);
        }

        UpdateLightmapProgress(66.0f, FormattedString(TEXT("Building Brush Lightmaps...\r\nBrush %d of %d"), i+1, BrushList.Num()), TEXT("Rendering Lightmap"));

        if(pass == 0 && !curSettings->bUseIBL && !bVisualizingAO)
        {
            for(j=0; j<Lights.Num(); j++)
            {
                RenderLightmap(lightmapInfo, GetBrushRes(brush), Lights[j], render);
                Lights[j].FreeData();

                ProcessWindowMessages();
                if(bCancelLightmapRendering) return;
            }
        }
        else
            RenderIBLLightmap(lightmapInfo, baseBrush->bounds, GetBrushRes(brush), pass, render);

        ProcessWindowMessages();
        if(bCancelLightmapRendering) return;

        XFileOutputSerializer fileOut;
        fileOut.Open(tempFileName, XFILE_CREATEALWAYS);
        render.Serialize(fileOut, GetBrushRes(brush), GetBrushRes(brush), TRUE);

        UpdateLightmapProgress(100.0f, FormattedString(TEXT("Building Brush Lightmaps...\r\nBrush %d of %d"), i+1, BrushList.Num()));
        Sleep(10);

        ++scene.counter;
    }

    //build lightmap UV coords if any
    for(i=0; i<scene.LMMeshList.Num(); i++)
    {
        LMMesh &lmmeshInfo = scene.LMMeshList[i];

        EditorMesh editMesh;
        editMesh.MakeFromMesh(lmmeshInfo.mesh);

        if(lmmeshInfo.mesh->VertBuffer->GetData()->TVList.Num() < 2)
        {
            UpdateLightmapProgress(0.0f, FormattedString(TEXT("Building Mesh UV Coordinates...\r\nMesh %d of %d"), i+1, scene.LMMeshList.Num()));

            PhyShape *meshShape = lmmeshInfo.mesh->meshShape;

            editMesh.BuildLightmapUVs();
            editMesh.SaveToMesh(lmmeshInfo.mesh);

            if(meshShape)
            {
                for(j=0; j<lmmeshInfo.Entities.Num(); j++)
                {
                    MeshEntity *ent = lmmeshInfo.Entities[j];
                    if(ent->phyShape == meshShape)
                        DestroyObject(ent->phyObject);
                }

                DestroyObject(meshShape);
                lmmeshInfo.mesh->meshShape = NULL;

                for(j=0; j<lmmeshInfo.Entities.Num(); j++)
                {
                    MeshEntity *ent = lmmeshInfo.Entities[j];
                    if(ent->phyShape == meshShape)
                    {
                        ent->phyShape = lmmeshInfo.mesh->GetShape();
                        ent->phyObject = physics->CreateStaticObject(ent->phyShape, ent->GetWorldPos(), ent->GetWorldRot());
                        ent->phyObject->SetEntityOwner(ent);
                    }
                }
            }

            lmmeshInfo.mesh->SaveMeshFile();

            UpdateLightmapProgress(100.0f, FormattedString(TEXT("Building Mesh UV Coordinates...\r\nMesh %d of %d"), i+1, scene.LMMeshList.Num()));
            Sleep(10.0f);
        }

        LightmapMeshInfo meshInfo;
        meshInfo.nPlanes            = editMesh.PlaneList.Num();
        meshInfo.nFaces             = editMesh.FaceList.Num();
        meshInfo.nVerts             = editMesh.VertList.Num();

        meshInfo.Planes             = editMesh.PlaneList.Array();
        meshInfo.Faces              = editMesh.FaceList.Array();
        meshInfo.FacePlanes         = editMesh.FacePlaneList.Array();
        meshInfo.VertList           = editMesh.VertList.Array();
        meshInfo.LightmapCoords     = editMesh.LMUVList.Array();
        meshInfo.NormalList         = editMesh.NormalList.Array();
        meshInfo.TangentList        = editMesh.TangentList.Array();

        LightmapInfo lightmapInfo(meshInfo);
        lightmapInfo.bHasTransform = TRUE;

        for(j=0; j<lmmeshInfo.Entities.Num(); j++)
        {
            MeshEntity *meshEnt = lmmeshInfo.Entities[j];

            List<LMLightInfo> Lights;

            if(pass == 0 && !curSettings->bUseIBL && !bVisualizingAO)
            {
                for(k=0; k<Light::NumLights(); k++)
                {
                    Light *light = Light::GetLight(k);

                    if(!light->bStaticLight) continue;

                    if(light->IsOf(GetClass(PointLight)))
                    {
                        PointLight *pointLight = static_cast<PointLight*>(light);
                        Vect lightPos = pointLight->GetWorldPos().GetTransformedPoint(meshEnt->GetInvTransform());
                        if(lmmeshInfo.mesh->bounds.SphereIntersects(lightPos, pointLight->lightRange))
                        {
                            LMLightInfo lightInfo;
                            lightInfo.light = light;
                            lightInfo.ghost = physics->CreateGhost();
                            lightInfo.ghost->SetShape(physics->MakeSphere(pointLight->lightRange));
                            lightInfo.ghost->SetPos(pointLight->GetWorldPos());
                            lightInfo.ghost->UpdatePositionalData();
                            Lights << lightInfo;
                        }
                    }
                }
            }

            meshEnt->bLightmapped = TRUE;

            UpdateLightmapProgress(33.0f, FormattedString(TEXT("Lightmapping Mesh %d of %d\r\nEntity '%s'..."), i+1, scene.LMMeshList.Num(), meshEnt->GetName().Array()), TEXT("Generating Texel Info"));

            lightmapInfo.transform = meshEnt->GetTransform();

            String tempFileName;
            tempFileName << TEXT("EditorTemp/") << IntString(scene.counter) << TEXT(".tmp");

            LightmapRender render;
            if(pass == 0)
                lightmapInfo.GenerateTexelInfo(GetMeshRes(meshEnt), render);
            else
            {
                XFileInputSerializer tempFile;
                if(!tempFile.Open(tempFileName))
                {
                    ++scene.counter;
                    continue;
                }

                render.Serialize(tempFile, GetMeshRes(meshEnt), GetMeshRes(meshEnt), TRUE);
            }

            UpdateLightmapProgress(66.0f, FormattedString(TEXT("Lightmapping Mesh %d of %d\r\nEntity '%s'..."), i+1, scene.LMMeshList.Num(), meshEnt->GetName().Array()), TEXT("Rendering Lightmap"));

            if(pass == 0 && !curSettings->bUseIBL && !bVisualizingAO)
            {
                for(k=0; k<Lights.Num(); k++)
                {
                    RenderLightmap(lightmapInfo, GetMeshRes(meshEnt), Lights[k], render);
                    Lights[k].FreeData();

                    ProcessWindowMessages();
                    if(bCancelLightmapRendering) return;
                }
            }
            else
                RenderIBLLightmap(lightmapInfo, meshEnt->GetMeshBounds().GetTransformedBounds(meshEnt->GetTransform()), GetMeshRes(meshEnt), pass, render);

            ProcessWindowMessages();
            if(bCancelLightmapRendering) return;

            XFileOutputSerializer fileOut;
            fileOut.Open(tempFileName, XFILE_CREATEALWAYS);
            render.Serialize(fileOut, GetMeshRes(meshEnt), GetMeshRes(meshEnt), TRUE);

            UpdateLightmapProgress(100.0f, FormattedString(TEXT("Lightmapping Mesh %d of %d\r\nEntity '%s'..."), i+1, scene.LMMeshList.Num(), meshEnt->GetName().Array()));
            Sleep(10.0f);

            ++scene.counter;
        }
    }
}

void EditorLevelInfo::RenderLightmap(LightmapInfo &li, DWORD size, LMLightInfo &lightInfo, LightmapRender &lm)
{
    Light *light = lightInfo.light;
    PhyGhost *ghost = lightInfo.ghost;

    DWORD totalSize = size*size;
    int x, y, i;

    li.GenerateLightData(light);

    float lightRange = 1.0f;
    float lightIntensity = float(light->intensity)*0.01f;
    float lightBur = light->lightVolume*1.6f;
    Color3 lightColor;
    lightColor.x = RGB_Bf(light->color);
    lightColor.y = RGB_Gf(light->color);
    lightColor.z = RGB_Rf(light->color);

    bool bDirectional = false;

    if(light->IsOf(GetClass(PointLight)))
        lightRange = static_cast<PointLight*>(light)->lightRange;
    else if(light->IsOf(GetClass(SpotLight)))
        lightRange = static_cast<SpotLight*>(light)->lightRange;
    else if(light->IsOf(GetClass(DirectionalLight)))
        bDirectional = true;

    float lightRangeI = 1.0f/lightRange;

    //-------------------------------------------------------------------------------------
    // Get Shadow

    float *shadowMap  = (float*)Allocate(totalSize*sizeof(float));

    if(light->bCastShadows)
    {
        float *blurMap;
        BYTE  *preShadow;

        if(curSettings->bBlurShadows)
        {
            blurMap    = (float*)Allocate(totalSize*sizeof(float));
            preShadow  = (BYTE*) Allocate(totalSize);
            zero(preShadow, totalSize);
        }
        else
        {
            float fOne = 1.0f;
            msetd(shadowMap, *(DWORD*)&fOne, totalSize*sizeof(float));
        }

        for(y=0; y<size; y++)
        {
            DWORD yPos = y*size;

            for(x=0; x<size; x++)
            {
                DWORD texPos = yPos+x;
                TexelInfo &ti = lm.texelInfo[texPos];
                if(!ti.tri) continue;

                if(bDirectional)
                {
                    Vect lightVec = light->GetWorldRot().GetDirectionVector();

                    PhyCollisionInfo ci;
                    if(physics->GetLineCollision(ti.pos, ti.pos+(lightVec*1000.0f), &ci, PHY_STATIC))
                    {
                        if(!ci.hitPos.CloseTo(ti.pos, 0.0201f))
                        {
                            if(curSettings->bBlurShadows)
                            {
                                preShadow[texPos] = 1;
                                blurMap[texPos] = 0.2f;
                            }
                            else
                                shadowMap[texPos] = 0.0f;
                        }
                    }
                    else if(curSettings->bBlurShadows)
                        blurMap[texPos] = 0.9f;
                }
                else
                {
                    Vect lightVec = light->GetWorldPos()-ti.pos;
                    float dist = lightVec.Len();

                    if(dist < lightRange)
                    {
                        float rangeMul = 1.0f-(dist*lightRangeI);
                        Saturate(rangeMul);

                        PhyCollisionInfo ci;
                        if(ghost->GetAreaLineCollision(ti.pos, light->GetWorldPos(), &ci, PHY_STATIC))
                        {
                            if(!ci.hitPos.CloseTo(ti.pos, 0.0201f))
                            {
                                if(curSettings->bBlurShadows)
                                    preShadow[texPos] = 1;
                                else
                                    shadowMap[texPos] = 0.0f;
                            }
                        }

                        if(curSettings->bBlurShadows)
                            blurMap[texPos] = rangeMul;
                    }
                    else if(curSettings->bBlurShadows)
                        blurMap[texPos] = -1.0f;
                }
            }
        }

        //-------------------------------------------------------------------------------------
        // Blur Shadow

        if(curSettings->bBlurShadows)
        {
            int sizem1 = size-1;

            for(y=0; y<size; y++)
            {
                DWORD yPos = y*size;

                for(x=0; x<size; x++)
                {
                    DWORD texPos = yPos+x;
                    TexelInfo &ti = lm.texelInfo[texPos];
                    if(!ti.tri) continue;

                    float blurVal = blurMap[texPos];
                    if(*(DWORD*)&blurVal & 0x80000000)
                        continue;

                    if(blurVal > 1.0f) blurVal = 1.0f;

                    float val = (1.0f-blurVal)*lightBur;
                    int next = int(ceil(val));

                    if(next == 0)
                        continue;

                    float valI = 1.0f/val;

                    int blurX, blurY;

                    blurVal = 0.0f;
                    float curSum = 0.0f;

                    int startX=x-next, endX=x+next,
                        startY=y-next, endY=y+next;

                    if(startX < 0) startX = 0;
                    else if(endX >= size) endX = sizem1;

                    if(startY < 0) startY = 0;
                    else if(endY >= size) endY = sizem1;

                    for(blurY=startY; blurY<=endY; blurY++)
                    {
                        DWORD blurYPos = blurY*size;
                        for(blurX=startX; blurX<=endX; blurX++)
                        {
                            DWORD blurPos = blurYPos+blurX;

                            Vect2 blurLoc(blurX-x, blurY-y);
                            float adj = 1.0f-(blurLoc.Len()*valI);

                            if(!(*(DWORD*)&adj & 0x80000000))
                            {
                                curSum += adj;
                                blurVal += float(1-preShadow[blurPos])*adj;
                            }
                        }
                    }

                    shadowMap[texPos] = blurVal/curSum;
                }
            }

            Free(blurMap);
            Free(preShadow);
        }
    }
    else
    {
        float fOne = 1.0f;
        msetd(shadowMap, *(DWORD*)&fOne, totalSize*sizeof(float));
    }

    //-------------------------------------------------------------------------------------
    // Render Lightmap

    Vect texelShadeVals;

    for(y=0; y<size; y++)
    {
        DWORD yPos = y*size;

        for(x=0; x<size; x++)
        {
            DWORD texPos = yPos+x;
            TexelInfo &ti = lm.texelInfo[texPos];
            if(!ti.tri) continue;

            /*lm.lightmap[0][texPos] = 1.0f;
            continue;*/

            Face &face = li.mi.Faces[ti.tri-1];
            Vect lightVec = li.GetLightVec(face, ti.bary);
            if((*(DWORD*)&lightVec.z) & 0x80000000)
                continue;

            float attenuation;
            Vect lightDir;

            if(bDirectional)
            {
                lightDir = lightVec;
                attenuation = 1.0f;
            }
            else
            {
                float lightDist = lightVec.Len();
                if(lightDist > lightRange)
                    continue;

                lightDir = lightVec*(1.0f/lightDist);

                attenuation = (lightRange-lightDist)*lightRangeI;
            }
            //if((*(DWORD*)&attenuation) & 0x80000000) attenuation = 0.0f;

            attenuation = pow(attenuation, 0.75f);

            register BOOL bAdjust = FALSE;

            texelShadeVals = lightDir.GetTransformedVector(NormalShadeVectors);

            if((*(DWORD*)&texelShadeVals.x) & 0x80000000) {bAdjust = TRUE; texelShadeVals.x = 0.0f;}
            if((*(DWORD*)&texelShadeVals.y) & 0x80000000) {bAdjust = TRUE; texelShadeVals.y = 0.0f;}
            if((*(DWORD*)&texelShadeVals.z) & 0x80000000) {bAdjust = TRUE; texelShadeVals.z = 0.0f;}

            if(bAdjust)
            {
                float adjust = (NormalShadeVectors.X.z*texelShadeVals.x) + 
                               (NormalShadeVectors.X.z*texelShadeVals.y) +
                               (NormalShadeVectors.X.z*texelShadeVals.z);
                texelShadeVals *= (lightDir.z/adjust);
            }

            Vect mulVal = lightColor*lightIntensity*attenuation*shadowMap[texPos];
            if(bFinalPass && curSettings->bFilter)
                AdjustColorVal(mulVal);

            for(i=0; i<3; i++)
                lm.lightmap[i][texPos] += mulVal*texelShadeVals[i];
            lm.baseLightmap[texPos] += mulVal*lightDir.z;
        }
    }

    Free(shadowMap);
}

struct ProcessHemiCubeInfo
{
    inline ProcessHemiCubeInfo(HANDLE hThreadTrigger,
                               Vect *hemi, Vect *fullMask, Vect *axiiMask1, Vect *axiiMask2, Vect *axiiMask3,
                               int hemi_x_size, int hemi_y_size)
    {
        this->hThreadTrigger = hThreadTrigger;
        this->bTriggered = false;
        this->hemi = hemi;
        this->fullMask = fullMask;
        this->axiiMasks[0] = axiiMask1;
        this->axiiMasks[1] = axiiMask2;
        this->axiiMasks[2] = axiiMask3;
        this->hemi_x_size = hemi_x_size;
        this->hemi_y_size = hemi_y_size;
    }

    HANDLE hThreadTrigger;
    bool bTriggered;

    Vect *hemi;
    Vect *fullMask;
    Vect *axiiMasks[3];
    int hemi_x_size, hemi_y_size;
    Vect out, outX, outY, outZ;
};

DWORD ENGINEAPI ProcessHemiCubeThread(ProcessHemiCubeInfo *info)
{
    for(int hemi_y=0; hemi_y<info->hemi_y_size; hemi_y++)
    {
        DWORD hemiYPos = hemi_y*info->hemi_x_size;

        for(int hemi_x=0; hemi_x<info->hemi_x_size; hemi_x++)
        {
            DWORD pos = hemiYPos+hemi_x;

            Vect &val = info->hemi[pos];
            switchXZ(val);
            info->out  += info->fullMask[pos]*val;
            info->outX += info->axiiMasks[0][pos]*val;
            info->outY += info->axiiMasks[1][pos]*val;
            info->outZ += info->axiiMasks[2][pos]*val;
        }
    }

    info->bTriggered = true;
    OSIncrementSemaphore(info->hThreadTrigger);

    return 0;
}

#define NUM_BUFFERS 25

void EditorLevelInfo::RenderIBLLightmap(LightmapInfo &li, const Bounds &bounds, DWORD size, DWORD pass, LightmapRender &lm)
{
    DWORD totalSize = size*size;
    int x, y;

    profileSegment("renderIBL");

    int numProcessors = OSGetProcessorCount();

    Bounds objectArea = bounds;
    objectArea.Min -= maxPhotonDist;
    objectArea.Max += maxPhotonDist;

    List<Brush*> Brushes;
    List<MeshEntity*> StaticMeshes;
    
    level->GetStaticGeometry(objectArea, Brushes, StaticMeshes);

    BOOL bVisualizeAO = curSettings->bUseAO && curSettings->bVisualizeAO;
    BOOL bWriteColor = !bVisualizeAO && (curSettings->bUseGI || curSettings->bUseIBL);
    BOOL bNoCull = (pass == 0) || (bFinalPass && curSettings->bUseAO);
    BOOL bUsingAO = bFinalPass && curSettings->bUseAO;

    if(curSettings->bUseAO && curSettings->bVisualizeAO)
        pass = 1;

    DWORD extraPasses = MAX(curSettings->nGIPasses, 2)-2;
    float extraPassVal = 1.0f+float(extraPasses);

    IBLRender renders[NUM_BUFFERS];
    IBLThing texStuff[NUM_BUFFERS];

    ZStencilBuffer *depthBuffer = CreateZStencilBuffer(thingSize, thingSize);

    for(int i=0; i<NUM_BUFFERS; i++)
    {
        renders[i].Setup(thingSize);
        texStuff[i].Setup(thingSize);
    }

    Matrix texelTransform, adjTransform;

    float pixelOffset = (0.01f/(float)thingSize);

    float left = -0.01f+pixelOffset;
    float right = 0.01f+pixelOffset;
    float top = -0.01f-pixelOffset;
    float bottom = 0.01f-pixelOffset;
    float nearVal = 0.01f;
    float farVal = 10.0f*maxPhotonDist;

    float fAOMulVal = 1.0f/(1.0f-curSettings->aoDarknessCutoff);

    //ViewClip mainClip;
    //mainClip.planes << Plane(0.0f, 0.0f, 1.0f, 0.0f);

    Effect *effect = GS->CreateEffectFromFile(TEXT("data/Editor/effects/RadiosityRender.effect"));
    sphereMesh = new Mesh;
    sphereMesh->LoadMesh(TEXT("Editor:LPSphere.xmd"));

    lightmapIllumColor      = effect->GetParameterByName(TEXT("illuminationColor"));
    lightmapIllumTexture    = effect->GetParameterByName(TEXT("illuminationMap"));
    lightmapDiffuse         = effect->GetParameterByName(TEXT("diffuseTexture"));
    lightmapTexture         = effect->GetParameterByName(TEXT("lightmap"));
    lightmapMeshScale       = effect->GetScale();
    HANDLE lightmapNormVec  = effect->GetParameterByName(TEXT("normVector"));
    lightmapCamDir          = effect->GetParameterByName(TEXT("camDir"));

    lightmapWorldMatrix     = effect->GetWorld();

    HANDLE lmCamPos         = effect->GetParameterByName(TEXT("camPos"));

    blackTex = GetTexture(TEXT("Base:Default/black.tga"));
    whiteTex = GetTexture(TEXT("Base:Default/white.tga"));

    UINT curDrawBuffer = 0, curDelayBuffer=0, readDelay=0;

    BeginScene();

    DWORD timeTest = TrackTimeBegin(FALSE);
    DWORD numFrames = 0;

    DWORD texelPositions[NUM_BUFFERS];

    HANDLE hTech = effect->GetTechnique(bUsingAO ? TEXT("RadiosityRenderOcclusion") : TEXT("RadiosityRender"));
    effect->BeginTechnique(hTech);

    //debug----------------
    static int curThing = 0;
    ++curThing;
    //debug----------------

    effect->BeginPass(pass > 0 ? 1 : 0);

    if(pass == 1)
        nop();

    EnableBlending(FALSE);
    GS->DepthWriteEnable(TRUE);
    EnableDepthTest(TRUE);
    SetCullMode(bNoCull ? GS_NEITHER : GS_BACK);
    DepthFunction(GS_LEQUAL);
    BlendFunction(GS_BLEND_ONE, GS_BLEND_ONE);
    GS->ColorWriteEnable(1, 1, 1, 1);

    SetZStencilBufferTarget(depthBuffer);

    for(y=0; y<size; y++)
    {
        DWORD yPos = y*size;

        ProcessWindowMessages();
        if(bCancelLightmapRendering) break;

        for(x=0; x<size; x++)
        {
            DWORD texPos = yPos+x;
            TexelInfo &ti = lm.texelInfo[texPos];
            if(!ti.tri) continue;

            texelPositions[curDrawBuffer] = texPos;

            Face &face = li.mi.Faces[ti.tri-1];

            texelTransform.X = li.GetTanU(face, ti.bary).Norm();
            texelTransform.Z = -li.GetNorm(face, ti.bary).Norm();
            texelTransform.Y = texelTransform.Z ^ texelTransform.X;

            if(li.bHasTransform)
            {
                texelTransform.X.TransformVector(li.transform);
                texelTransform.Y.TransformVector(li.transform);
                texelTransform.Z.TransformVector(li.transform);
            }

            texelTransform.T = ti.pos;

            Euler test123(0.0f, 0.0f, RandomFloat(TRUE)*90.0f);
            Matrix baseTransform;
            baseTransform.SetIdentity();
            baseTransform *= test123;

            //texelTransform = baseTransform * texelTransform.GetTranspose();
            

            /*ViewClip curClip;
            curClip = mainClip;
            curClip.Transform(texelTransform.GetTranspose());*/

            profileIn("rendering");

            //-------------------

            SetFrameBufferTarget(texStuff[curDrawBuffer].Z);
            SetViewport(0, 0, thingSize, thingSize);
            ClearColorBuffer(TRUE, 0); ClearDepthBuffer();

            Frustum(left, right, top, bottom, nearVal, farVal);

            effect->SetVector(lmCamPos, ti.pos);
            effect->SetVector(lightmapNormVec, -texelTransform.Z);
            effect->SetVector(lightmapCamDir, -texelTransform.Z);

            MatrixSet(texelTransform);
            DrawHemicube(Brushes, StaticMeshes, ti.pos, pass, effect);

            //-------------------

            SetFrameBufferTarget(texStuff[curDrawBuffer].Xneg);
            SetViewport(0, 0, thingSize/2, thingSize);
            ClearColorBuffer(TRUE, 0); ClearDepthBuffer();

            Frustum(0.0f, right, top, bottom, nearVal, farVal);

            effect->SetVector(lightmapCamDir, -texelTransform.X);

            adjTransform.X = -texelTransform.Z;
            adjTransform.Y = texelTransform.Y;
            adjTransform.Z = texelTransform.X;
            adjTransform.T = texelTransform.T;
            MatrixSet(adjTransform);

            DrawHemicube(Brushes, StaticMeshes, ti.pos, pass, effect);

            //-------------------

            SetFrameBufferTarget(texStuff[curDrawBuffer].Xpos);
            ClearColorBuffer(TRUE, 0); ClearDepthBuffer();

            Frustum(left, 0.0f, top, bottom, nearVal, farVal);

            effect->SetVector(lightmapCamDir, texelTransform.X);

            adjTransform.X = texelTransform.Z;
            adjTransform.Y = texelTransform.Y;
            adjTransform.Z = -texelTransform.X;
            adjTransform.T = texelTransform.T;
            MatrixSet(adjTransform);

            DrawHemicube(Brushes, StaticMeshes, ti.pos, pass, effect);

            //-------------------

            SetFrameBufferTarget(texStuff[curDrawBuffer].Yneg);
            SetViewport(0, 0, thingSize, thingSize/2);
            ClearColorBuffer(TRUE, 0); ClearDepthBuffer();

            Frustum(left, right, 0.0f, bottom, nearVal, farVal);

            effect->SetVector(lightmapCamDir, -texelTransform.Y);

            adjTransform.X = texelTransform.X;
            adjTransform.Y = -texelTransform.Z;
            adjTransform.Z = texelTransform.Y;
            adjTransform.T = texelTransform.T;
            MatrixSet(adjTransform);

            DrawHemicube(Brushes, StaticMeshes, ti.pos, pass, effect);

            //-------------------

            SetFrameBufferTarget(texStuff[curDrawBuffer].Ypos);
            ClearColorBuffer(TRUE, 0); ClearDepthBuffer();

            Frustum(left, right, top, 0.0f, nearVal, farVal);

            effect->SetVector(lightmapCamDir, texelTransform.Y);

            adjTransform.X = texelTransform.X;
            adjTransform.Y = texelTransform.Z;
            adjTransform.Z = -texelTransform.Y;
            adjTransform.T = texelTransform.T;
            MatrixSet(adjTransform);

            DrawHemicube(Brushes, StaticMeshes, ti.pos, pass, effect);

            profileOut;

            //-------------------

            BOOL bLast = (x == (size-1) && y == (size-1));

            //this read delay is done so that we don't halt the GPU
            if(readDelay == NUM_BUFFERS-1 || bLast)
            {
                SetFrameBufferTarget(NULL);
                int processBuffers;

                processBuffers = readDelay+1;

                //doing all the "gets" first ensures the data will be ready in time for the locks with no delays
                for(int i=0; i<processBuffers; i++)
                {
                    int newDelayBuffer = curDelayBuffer+i;
                    if(newDelayBuffer >= NUM_BUFFERS)
                        newDelayBuffer -= NUM_BUFFERS;

                    IBLRender &render = renders[newDelayBuffer];

                    profileIn("getrendertargetimage");
                    texStuff[newDelayBuffer].Z->GetRenderTargetImage(render.surfaceZ);
                    texStuff[newDelayBuffer].Xneg->GetRenderTargetImage(render.surfaceXneg);
                    texStuff[newDelayBuffer].Xpos->GetRenderTargetImage(render.surfaceXpos);
                    texStuff[newDelayBuffer].Yneg->GetRenderTargetImage(render.surfaceYneg);
                    texStuff[newDelayBuffer].Ypos->GetRenderTargetImage(render.surfaceYpos);
                    profileOut;
                }

                for(int i=0; i<processBuffers; i++)
                {
                    IBLRender &render = renders[curDelayBuffer];

                    profileIn("reading buffer");
                    render.Lock(TRUE);
                    profileOut;

                    //debug-----------------------------------------
                    /*if(curThing == 0 && pass == 1 && x == 96 && y == 29)
                    {
                        void *bla;

                        bla = ConvertToBGRA(render.Z, thingSize, thingSize);
                        WriteTargaFile(TEXT("chiZ.tga"), thingSize, thingSize, 4, (BYTE*)bla);
                        Free(bla);

                        bla = ConvertToBGRA(render.Xneg, thingSize/2, thingSize);
                        WriteTargaFile(TEXT("chiXneg.tga"), thingSize/2, thingSize, 4, (BYTE*)bla);
                        Free(bla);

                        bla = ConvertToBGRA(render.Xpos, thingSize/2, thingSize);
                        WriteTargaFile(TEXT("chiXpos.tga"), thingSize/2, thingSize, 4, (BYTE*)bla);
                        Free(bla);

                        bla = ConvertToBGRA(render.Yneg, thingSize, thingSize/2);
                        WriteTargaFile(TEXT("chiYneg.tga"), thingSize, thingSize/2, 4, (BYTE*)bla);
                        Free(bla);

                        bla = ConvertToBGRA(render.Ypos, thingSize, thingSize/2);
                        WriteTargaFile(TEXT("chiYpos.tga"), thingSize, thingSize/2, 4, (BYTE*)bla);
                        Free(bla);
                    }*/
                    //debug-----------------------------------------

                    if(bWriteColor)
                    {
                        Vect out, outX, outY, outZ;
                        out.SetZero();
                        outX.SetZero();
                        outY.SetZero();
                        outZ.SetZero();

                        profileIn("processing output");

                        List<ProcessHemiCubeInfo*> pending, processing;
                        List<HANDLE> threads;
                        pending.SetSize(5);

                        HANDLE hEndThreadTrigger = OSCreateSemaphore(0);

                        pending[0] = new ProcessHemiCubeInfo(hEndThreadTrigger, render.Z, iblFullMask->Z, iblAxiiMask[0]->Z, iblAxiiMask[1]->Z, iblAxiiMask[2]->Z, thingSize, thingSize);
                        pending[1] = new ProcessHemiCubeInfo(hEndThreadTrigger, render.Xneg, iblFullMask->Xneg, iblAxiiMask[0]->Xneg, iblAxiiMask[1]->Xneg, iblAxiiMask[2]->Xneg, thingSize/2, thingSize);
                        pending[2] = new ProcessHemiCubeInfo(hEndThreadTrigger, render.Xpos, iblFullMask->Xpos, iblAxiiMask[0]->Xpos, iblAxiiMask[1]->Xpos, iblAxiiMask[2]->Xpos, thingSize/2, thingSize);
                        pending[3] = new ProcessHemiCubeInfo(hEndThreadTrigger, render.Yneg, iblFullMask->Yneg, iblAxiiMask[0]->Yneg, iblAxiiMask[1]->Yneg, iblAxiiMask[2]->Yneg, thingSize, thingSize/2);
                        pending[4] = new ProcessHemiCubeInfo(hEndThreadTrigger, render.Ypos, iblFullMask->Ypos, iblAxiiMask[0]->Ypos, iblAxiiMask[1]->Ypos, iblAxiiMask[2]->Ypos, thingSize, thingSize/2);

                        int curThreadCount = MIN(pending.Num(), numProcessors);
                        for(int i=0; i<curThreadCount; i++)
                        {
                            ProcessHemiCubeInfo *threadInfo = pending[0];
                            pending.Remove(0);

                            processing << threadInfo;
                            threads << OSCreateThread((ENGINETHREAD)ProcessHemiCubeThread, (LPVOID)threadInfo);
                        }

                        while(processing.Num() != 0)
                        {
                            OSWaitForSemaphore(hEndThreadTrigger);

                            int numRemoved = 0;

                            for(int i=0; i<processing.Num(); i++)
                            {
                                if(processing[i]->bTriggered)
                                {
                                    OSWaitForThread(threads[i], WAIT_INFINITE);
                                    OSCloseThread(threads[i], NULL);
                                    threads.Remove(i);

                                    out  += processing[i]->out;
                                    outX += processing[i]->outX;
                                    outY += processing[i]->outY;
                                    outZ += processing[i]->outZ;

                                    delete processing[i];
                                    processing.Remove(i);

                                    ++numRemoved;
                                }
                            }

                            int numToAdd = MIN(pending.Num(), numRemoved);
                            for(int i=0; i<numToAdd; i++)
                            {
                                ProcessHemiCubeInfo *threadInfo = pending[0];
                                pending.Remove(0);

                                processing << threadInfo;
                                threads << OSCreateThread((ENGINETHREAD)ProcessHemiCubeThread, (LPVOID)threadInfo);
                            }
                        }

                        OSCloseSemaphore(hEndThreadTrigger);

                        profileOut;

                        /*for(int hemi_y=0; hemi_y<thingSize; hemi_y++)
                        {
                            DWORD hemiYPos = hemi_y*thingSize;
                            for(int hemi_x=0; hemi_x<thingSize; hemi_x++)
                            {
                                DWORD pos = hemiYPos+hemi_x;
                                Vect &val = render.Z[pos];
                                switchXZ(val);
                                out  += iblFullMask->Z[pos]*val;
                                outX += iblAxiiMask[0]->Z[pos]*val;
                                outY += iblAxiiMask[1]->Z[pos]*val;
                                outZ += iblAxiiMask[2]->Z[pos]*val;
                            }
                        }

                        for(int hemi_y=0; hemi_y<thingSize; hemi_y++)
                        {
                            DWORD hemiYPos = hemi_y*(thingSize/2);
                            for(int hemi_x=0; hemi_x<(thingSize/2); hemi_x++)
                            {
                                DWORD pos = hemiYPos+hemi_x;
                                Vect &val = render.Xneg[pos];
                                switchXZ(val);
                                out  += iblFullMask->Xneg[pos]*val;
                                outX += iblAxiiMask[0]->Xneg[pos]*val;
                                outY += iblAxiiMask[1]->Xneg[pos]*val;
                                outZ += iblAxiiMask[2]->Xneg[pos]*val;
                            }
                        }

                        for(int hemi_y=0; hemi_y<thingSize; hemi_y++)
                        {
                            DWORD hemiYPos = hemi_y*(thingSize/2);
                            for(int hemi_x=0; hemi_x<(thingSize/2); hemi_x++)
                            {
                                DWORD pos = hemiYPos+hemi_x;
                                Vect &val = render.Xpos[pos];
                                switchXZ(val);
                                out  += iblFullMask->Xpos[pos]*val;
                                outX += iblAxiiMask[0]->Xpos[pos]*val;
                                outY += iblAxiiMask[1]->Xpos[pos]*val;
                                outZ += iblAxiiMask[2]->Xpos[pos]*val;
                            }
                        }

                        for(int hemi_y=0; hemi_y<(thingSize/2); hemi_y++)
                        {
                            DWORD hemiYPos = hemi_y*thingSize;
                            for(int hemi_x=0; hemi_x<thingSize; hemi_x++)
                            {
                                DWORD pos = hemiYPos+hemi_x;
                                Vect &val = render.Yneg[pos];
                                switchXZ(val);
                                out  += iblFullMask->Yneg[pos]*val;
                                outX += iblAxiiMask[0]->Yneg[pos]*val;
                                outY += iblAxiiMask[1]->Yneg[pos]*val;
                                outZ += iblAxiiMask[2]->Yneg[pos]*val;
                            }
                        }

                        for(int hemi_y=0; hemi_y<(thingSize/2); hemi_y++)
                        {
                            DWORD hemiYPos = hemi_y*thingSize;
                            for(int hemi_x=0; hemi_x<thingSize; hemi_x++)
                            {
                                DWORD pos = hemiYPos+hemi_x;
                                Vect &val = render.Ypos[pos];
                                switchXZ(val);
                                out  += iblFullMask->Ypos[pos]*val;
                                outX += iblAxiiMask[0]->Ypos[pos]*val;
                                outY += iblAxiiMask[1]->Ypos[pos]*val;
                                outZ += iblAxiiMask[2]->Ypos[pos]*val;
                            }
                        }*/

                        if(bFinalPass)
                        {
                            if(extraPasses)
                            {
                                out *= extraPassVal;
                                outX *= extraPassVal;
                                outY *= extraPassVal;
                                outZ *= extraPassVal;
                            }

                            if(curSettings->bFilter)
                            {
                                AdjustColorVal(out);
                                AdjustColorVal(outX);
                                AdjustColorVal(outY);
                                AdjustColorVal(outZ);
                            }
                        }

                        lm.baseLightmap[texelPositions[curDelayBuffer]] += out;
                        lm.lightmap[0][texelPositions[curDelayBuffer]]  += outX;
                        lm.lightmap[1][texelPositions[curDelayBuffer]]  += outY;
                        lm.lightmap[2][texelPositions[curDelayBuffer]]  += outZ;
                    }

                    if(bFinalPass && curSettings->bUseAO)
                    {
                        float fAOVal  = 0.0f;

                        for(int hemi_y=0; hemi_y<thingSize; hemi_y++)
                        {
                            DWORD hemiYPos = hemi_y*thingSize;
                            for(int hemi_x=0; hemi_x<thingSize; hemi_x++)
                            {
                                DWORD pos = hemiYPos+hemi_x;
                                Vect &val = render.Z[pos];
                                if(val.w == 0.0f || (val.w > curSettings->aoDist))
                                    fAOVal += iblFullMask->Z[pos].x;
                            }
                        }

                        for(int hemi_y=0; hemi_y<thingSize; hemi_y++)
                        {
                            DWORD hemiYPos = hemi_y*(thingSize/2);
                            for(int hemi_x=0; hemi_x<(thingSize/2); hemi_x++)
                            {
                                DWORD pos = hemiYPos+hemi_x;
                                Vect &val = render.Xneg[pos];
                                if(val.w == 0.0f || (val.w > curSettings->aoDist))
                                    fAOVal += iblFullMask->Xneg[pos].x;
                            }
                        }

                        for(int hemi_y=0; hemi_y<thingSize; hemi_y++)
                        {
                            DWORD hemiYPos = hemi_y*(thingSize/2);
                            for(int hemi_x=0; hemi_x<(thingSize/2); hemi_x++)
                            {
                                DWORD pos = hemiYPos+hemi_x;
                                Vect &val = render.Xpos[pos];
                                if(val.w == 0.0f || (val.w > curSettings->aoDist))
                                    fAOVal += iblFullMask->Xpos[pos].x;
                            }
                        }

                        for(int hemi_y=0; hemi_y<(thingSize/2); hemi_y++)
                        {
                            DWORD hemiYPos = hemi_y*thingSize;
                            for(int hemi_x=0; hemi_x<thingSize; hemi_x++)
                            {
                                DWORD pos = hemiYPos+hemi_x;
                                Vect &val = render.Yneg[pos];
                                if(val.w == 0.0f || (val.w > curSettings->aoDist))
                                    fAOVal += iblFullMask->Yneg[pos].x;
                            }
                        }

                        for(int hemi_y=0; hemi_y<(thingSize/2); hemi_y++)
                        {
                            DWORD hemiYPos = hemi_y*thingSize;
                            for(int hemi_x=0; hemi_x<thingSize; hemi_x++)
                            {
                                DWORD pos = hemiYPos+hemi_x;
                                Vect &val = render.Ypos[pos];
                                if(val.w == 0.0f || (val.w > curSettings->aoDist))
                                    fAOVal += iblFullMask->Ypos[pos].x;
                            }
                        }

                        if(curSettings->aoExponent != 1.0f)
                            fAOVal = powf(fAOVal, curSettings->aoExponent);
                        if(curSettings->aoDarknessCutoff != 0.0f)
                        {
                            fAOVal = (fAOVal-curSettings->aoDarknessCutoff)*fAOMulVal;
                            if(*(DWORD*)&fAOVal & 0x80000000) fAOVal = 0.0f;
                        }

                        if(curSettings->bVisualizeAO)
                        {
                            lm.baseLightmap[texelPositions[curDelayBuffer]] = fAOVal;
                            lm.lightmap[0][texelPositions[curDelayBuffer]]  = fAOVal/2.45f;
                            lm.lightmap[1][texelPositions[curDelayBuffer]]  = fAOVal/2.45f;
                            lm.lightmap[2][texelPositions[curDelayBuffer]]  = fAOVal/2.45f;
                        }
                        else
                        {
                            lm.baseLightmap[texelPositions[curDelayBuffer]] *= fAOVal;
                            lm.lightmap[0][texelPositions[curDelayBuffer]]  *= fAOVal;
                            lm.lightmap[1][texelPositions[curDelayBuffer]]  *= fAOVal;
                            lm.lightmap[2][texelPositions[curDelayBuffer]]  *= fAOVal;
                        }
                    }

                    render.Unlock();

                    ++curDelayBuffer;
                    if(curDelayBuffer == NUM_BUFFERS) curDelayBuffer = 0;
                }

                readDelay = 0;
            }
            else
                ++readDelay;

            /*//-------------------------------------------
            
            if(x == 96 && y == 29)
            {
                void *bla;

                if(readDelay != NUM_BUFFERS-1)
                    SetFrameBufferTarget(NULL);

                texStuff[curDrawBuffer].Z->GetImage(TRUE, render.Z);
                bla = ConvertToBGRA(render.Z, thingSize, thingSize);
                WriteTargaFile(TEXT("chiZ.tga"), thingSize, thingSize, 4, (BYTE*)bla);
                Free(bla);

                texStuff[curDrawBuffer].Xneg->GetImage(TRUE, render.Xneg);
                bla = ConvertToBGRA(render.Xneg, thingSize/2, thingSize);
                WriteTargaFile(TEXT("chiXneg.tga"), thingSize/2, thingSize, 4, (BYTE*)bla);
                Free(bla);

                texStuff[curDrawBuffer].Xpos->GetImage(TRUE, render.Xpos);
                bla = ConvertToBGRA(render.Xpos, thingSize/2, thingSize);
                WriteTargaFile(TEXT("chiXpos.tga"), thingSize/2, thingSize, 4, (BYTE*)bla);
                Free(bla);

                texStuff[curDrawBuffer].Yneg->GetImage(TRUE, render.Yneg);
                bla = ConvertToBGRA(render.Yneg, thingSize, thingSize/2);
                WriteTargaFile(TEXT("chiYneg.tga"), thingSize, thingSize/2, 4, (BYTE*)bla);
                Free(bla);

                texStuff[curDrawBuffer].Ypos->GetImage(TRUE, render.Ypos);
                bla = ConvertToBGRA(render.Ypos, thingSize, thingSize/2);
                WriteTargaFile(TEXT("chiYpos.tga"), thingSize, thingSize/2, 4, (BYTE*)bla);
                Free(bla);

                effect->EndPass();
                effect->EndTechnique();

                EndScene();

                DestroyObject(effect);
                delete sphereMesh;
                return;
            }
            
            //-------------------------------------------*/

            ++curDrawBuffer;
            if(curDrawBuffer == NUM_BUFFERS) curDrawBuffer = 0;

            texelPositions[curDrawBuffer] = texPos;

            ++numFrames;
        }
    }

    DWORD elapsedTime = TrackTimeEnd(timeTest);
    float timeDiv = float(elapsedTime)*0.001f;
    float fpsVal = float(numFrames)/timeDiv;

    Log(TEXT("IBL pass %d - Time Spent: %f seconds.  FPS = %f"), pass, timeDiv, fpsVal);

    effect->EndPass();
    effect->EndTechnique();

    SetZStencilBufferTarget(NULL);
    DestroyObject(depthBuffer);

    delete sphereMesh;
    DestroyObject(effect);
    EndScene();
}

struct SamplerTexture
{
    int texWidth, texHeight;
    BOOL bHasAlpha;
    union
    {
        BGR  *rgb;
        BGRA *rgba;
    };
};

struct SamplerMeshInfo
{
    int nVerts, nSections;
    Vect *NormalList;
    Face *FaceList;
    UVCoord *UVList;
    LightmapRender *render;
    DrawSection *SectionList;
    List<SamplerTexture> textures;

    const Matrix *transform;

    SamplerMeshInfo() {zero(this, sizeof(SamplerMeshInfo));}
    ~SamplerMeshInfo()
    {
        for(int i=0; i<textures.Num(); i++)
            Free(textures[i].rgba);
        textures.Clear();
    }
};

inline void MapObjSample(const PhyCollisionInfo &ci, SamplerMeshInfo &info, Lightmap &lightmap, int size)
{
    float closestDist = M_INFINITE;
    int winningTexel = -1;

    for(int y=0; y<size; y++)
    {
        int yPos = y*size;

        for(int x=0; x<size; x++)
        {
            int texPos = yPos+x;
            if(!info.render->texelInfo[texPos].tri) continue;

            Vect offset = ci.hitPos-info.render->texelInfo[texPos].pos;

            float dist = fabs(offset.x) + fabs(offset.y) + fabs(offset.z);

            if(dist < closestDist)
            {
                winningTexel = texPos;
                closestDist = dist;
            }
        }
    }

    if(winningTexel != -1)
    {
        IndirectLightSample sample;
        Color3 shade = info.render->baseLightmap[winningTexel];
        SamplerTexture *tex;
        DWORD tri = info.render->texelInfo[winningTexel].tri-1;
        for(int i=0; i<info.textures.Num(); i++)
        {
            if(info.SectionList[i].startFace <= tri)
            {
                tex = &info.textures[i];
                break;
            }
        }

        Face &f = info.FaceList[tri];
        float *bary = (float*)info.render->texelInfo[winningTexel].bary;
        UVCoord uv = (info.UVList[f.A]*bary[0]) + (info.UVList[f.B]*bary[1]) + (info.UVList[f.C]*bary[2]);

        if(uv.x > 1.0f)         uv.x -= floorf(uv.x);
        else if(uv.x < -1.0f)   uv.x -= ceilf(uv.x);
        if(uv.x < 0.0f)         uv.x += 1.0f;

        if(uv.y > 1.0f)         uv.y -= floorf(uv.y);
        else if(uv.y < -1.0f)   uv.y -= ceilf(uv.y);
        if(uv.y < 0.0f)         uv.y += 1.0f;

        int texX = uv.x*float(tex->texWidth);
        int texY = uv.y*float(tex->texHeight);

        Color3 color;
        if(tex->bHasAlpha)
            color = tex->rgba[(texY*tex->texWidth)+texX].GetColor3();
        else
            color = tex->rgb[(texY*tex->texWidth)+texX].GetColor3();

        sample.color = (color*shade).ClampMax(1.0f).GetRGB();
        sample.pos  = ci.hitPos;
        Vect norm = ((info.NormalList[f.A]*bary[0]) + (info.NormalList[f.B]*bary[1]) + (info.NormalList[f.C]*bary[2])).Norm();
        if(info.transform != NULL)
            norm.TransformVector(*info.transform);

        sample.norm = norm;
        lightmap.ILSamples << sample;
    }
}

inline void BuildObjSamples(Bounds &bounds, SamplerMeshInfo &info, Lightmap &lightmap, int size, PhyObject *phyObj)
{
    Vect projPos, endPos, origProj;
    PhyCollisionInfo ci;
    BOOL bPass;

    //----------------------------------------------------------------------------
    // x axis
    projPos = bounds.Min;
    projPos.y = ceilf(projPos.y/3.0f)*3.0f;
    projPos.z = ceilf(projPos.z/3.0f)*3.0f;

    bPass = FALSE;
    if((projPos.y >= bounds.Max.y) || (projPos.z >= bounds.Max.z))
    {
        if(CloseFloat(bounds.Min.y, bounds.Max.y) || CloseFloat(bounds.Min.z, bounds.Max.z))
            bPass = TRUE;
        else
        {
            if(projPos.y >= bounds.Max.y) projPos.y = Lerp<float>(bounds.Min.y, bounds.Max.y, 0.5f);
            if(projPos.z >= bounds.Max.z) projPos.z = Lerp<float>(bounds.Min.z, bounds.Max.z, 0.5f);
        }
    }

    if(!bPass)
    {
        projPos.x -= 0.1f;
        endPos = projPos;
        endPos.x = bounds.Max.x+0.1f;
        origProj = projPos;

        while(projPos.y < bounds.Max.y)
        {
            while(projPos.z < bounds.Max.z)
            {
                while(phyObj->GetLineCollision(projPos, endPos, &ci))
                {
                    if(ci.hitNorm.x < -0.70711f)
                        MapObjSample(ci, info, lightmap, size);

                    projPos = ci.hitPos;
                    projPos.x += 0.09f;
                }

                projPos.x = origProj.x;
                projPos.z += 3.0f;
                endPos.z  += 3.0f;
            }

            projPos.z = origProj.z;
            endPos.z  = origProj.z;
            projPos.y += 3.0f;
            endPos.y  += 3.0f;
        }
    }

    //----------------------------------------------------------------------------
    // y axis
    projPos = bounds.Min;
    projPos.x = ceilf(projPos.x/3.0f)*3.0f;
    projPos.z = ceilf(projPos.z/3.0f)*3.0f;

    bPass = FALSE;
    if((projPos.x >= bounds.Max.x) || (projPos.z >= bounds.Max.z))
    {
        if(CloseFloat(bounds.Min.x, bounds.Max.x) || CloseFloat(bounds.Min.z, bounds.Max.z))
            bPass = TRUE;
        else
        {
            if(projPos.x >= bounds.Max.x) projPos.x = Lerp<float>(bounds.Min.x, bounds.Max.x, 0.5f);
            if(projPos.z >= bounds.Max.z) projPos.z = Lerp<float>(bounds.Min.z, bounds.Max.z, 0.5f);
        }
    }

    if(!bPass)
    {
        projPos.y -= 0.1f;
        endPos = projPos;
        endPos.y = bounds.Max.y+0.1f;
        origProj = projPos;

        while(projPos.x < bounds.Max.x)
        {
            while(projPos.z < bounds.Max.z)
            {
                while(phyObj->GetLineCollision(projPos, endPos, &ci))
                {
                    if(ci.hitNorm.y < -0.70711f)
                        MapObjSample(ci, info, lightmap, size);

                    projPos = ci.hitPos;
                    projPos.y += 0.09f;
                }

                projPos.y = origProj.y;
                projPos.z += 3.0f;
                endPos.z  += 3.0f;
            }

            projPos.z = origProj.z;
            endPos.z  = origProj.z;
            projPos.x += 3.0f;
            endPos.x  += 3.0f;
        }
    }

    //----------------------------------------------------------------------------
    // z axis
    projPos = bounds.Min;
    projPos.x = ceilf(projPos.x/3.0f)*3.0f;
    projPos.y = ceilf(projPos.y/3.0f)*3.0f;

    bPass = FALSE;
    if((projPos.x >= bounds.Max.x) || (projPos.y >= bounds.Max.y))
    {
        if(CloseFloat(bounds.Min.x, bounds.Max.x) || CloseFloat(bounds.Min.y, bounds.Max.y))
            bPass = TRUE;
        else
        {
            if(projPos.x >= bounds.Max.x) projPos.x = Lerp<float>(bounds.Min.x, bounds.Max.x, 0.5f);
            if(projPos.y >= bounds.Max.y) projPos.y = Lerp<float>(bounds.Min.y, bounds.Max.y, 0.5f);
        }
    }

    if(!bPass)
    {
        projPos.z -= 0.1f;
        endPos = projPos;
        endPos.z = bounds.Max.z+0.1f;
        origProj = projPos;

        while(projPos.x < bounds.Max.x)
        {
            while(projPos.y < bounds.Max.y)
            {
                while(phyObj->GetLineCollision(projPos, endPos, &ci))
                {
                    if(ci.hitNorm.z < -0.70711f)
                        MapObjSample(ci, info, lightmap, size);

                    projPos = ci.hitPos;
                    projPos.z += 0.09f;
                }

                projPos.z = origProj.z;
                projPos.y += 3.0f;
                endPos.y  += 3.0f;
            }

            projPos.y = origProj.y;
            endPos.y  = origProj.y;
            projPos.x += 3.0f;
            endPos.x  += 3.0f;
        }
    }
}

void EditorLevelInfo::UploadLightmaps(LightmapScene &scene, BOOL bFinal)
{
    void *texVal;

    scene.counter = 0;

    BOOL bDXT1 = curSettings->bUseDXT1;

    for(int i=0; i<scene.Brushes.Num(); i++)
    {
        EditorBrush *brush = scene.Brushes[i];
        Brush *baseBrush = brush->GetLevelBrush();

        String tempFileName;
        tempFileName << TEXT("EditorTemp/") << IntString(scene.counter) << TEXT(".tmp");

        XFileInputSerializer tempFile;
        if(!tempFile.Open(tempFileName))
        {
            ++scene.counter;
            continue;
        }

        baseBrush->lightmap.FreeData();

        LightmapRender render;
        render.Serialize(tempFile, GetBrushRes(brush), GetBrushRes(brush), bFinal);

        if(bFinal)
        {
            UpdateLightmapProgress(25.0f, FormattedString(TEXT("Calculating Indirect Light Samples...\r\nBrush %d of %d"), i+1, BrushList.Num()));

            SamplerMeshInfo info;
            info.nVerts = baseBrush->nVerts;
            info.nSections = baseBrush->nSections;
            info.UVList = baseBrush->UVList;
            info.FaceList = baseBrush->FaceList;
            info.SectionList = baseBrush->SectionList;
            info.NormalList = baseBrush->VertBuffer->GetData()->NormalList.Array();
            info.render = &render;
            for(int j=0; j<baseBrush->nSections; j++)
            {
                Material *mat = baseBrush->Materials[j];
                if(!mat) mat = level->defaultMaterial;

                SamplerTexture tex;
                Texture *texture = (Texture*)mat->GetCurrentTexture(TEXT("diffuseTexture"));
                tex.rgba = (BGRA*)texture->GetImage(TRUE);
                if(texture->GetFormat() == GS_DXT1)
                {
                    DXT1ToBGRA((void*&)tex.rgba, texture->Width(), texture->Height());
                    tex.bHasAlpha = TRUE;
                }
                else if(texture->GetFormat() == GS_DXT3)
                {
                    DXT3ToBGRA((void*&)tex.rgba, texture->Width(), texture->Height());
                    tex.bHasAlpha = TRUE;
                }
                else if(texture->GetFormat() == GS_DXT5)
                {
                    DXT5ToBGRA((void*&)tex.rgba, texture->Width(), texture->Height());
                    tex.bHasAlpha = TRUE;
                }
                else if(texture->GetFormat() == GS_RGB)
                    tex.bHasAlpha = FALSE;
                else if(texture->GetFormat() == GS_RGBA)
                    tex.bHasAlpha = TRUE;

                tex.texWidth = texture->Width();
                tex.texHeight = texture->Height();

                info.textures << tex;
            }

            BuildObjSamples(baseBrush->bounds, info, baseBrush->lightmap, GetBrushRes(brush), baseBrush->phyObj);

            UpdateLightmapProgress(100.0f, FormattedString(TEXT("Calculating Indirect Light Samples...\r\nBrush %d of %d"), i+1, BrushList.Num()));

            if(bDXT1)   texVal = ConvertToDXT1(render.lightmap[0], GetBrushRes(brush), GetBrushRes(brush));
            else        texVal = ConvertToRGB(render.lightmap[0], GetBrushRes(brush), GetBrushRes(brush));
            baseBrush->lightmap.X = CreateTexture(GetBrushRes(brush), GetBrushRes(brush), (bDXT1) ? GS_DXT1 : 3, texVal, FALSE);
            Free(texVal);

            if(bDXT1)   texVal = ConvertToDXT1(render.lightmap[1], GetBrushRes(brush), GetBrushRes(brush));
            else        texVal = ConvertToRGB(render.lightmap[1], GetBrushRes(brush), GetBrushRes(brush));
            baseBrush->lightmap.Y = CreateTexture(GetBrushRes(brush), GetBrushRes(brush), (bDXT1) ? GS_DXT1 : 3, texVal, FALSE);
            Free(texVal);

            if(bDXT1)   texVal = ConvertToDXT1(render.lightmap[2], GetBrushRes(brush), GetBrushRes(brush));
            else        texVal = ConvertToRGB(render.lightmap[2], GetBrushRes(brush), GetBrushRes(brush));
            baseBrush->lightmap.Z = CreateTexture(GetBrushRes(brush), GetBrushRes(brush), (bDXT1) ? GS_DXT1 : 3, texVal, FALSE);
            Free(texVal);

            Sleep(10);
        }
        else
        {
            texVal = ConvertToRGB(render.baseLightmap, GetBrushRes(brush), GetBrushRes(brush));

            baseBrush->lightmap.plain = CreateTexture(GetBrushRes(brush), GetBrushRes(brush), (bDXT1) ? GS_DXT1 : 3, texVal, FALSE);
            Free(texVal);
        }

        ++scene.counter;
    }

    for(int i=0; i<scene.LMMeshList.Num(); i++)
    {
        LMMesh &mesh = scene.LMMeshList[i];

        for(int j=0; j<mesh.Entities.Num(); j++)
        {
            MeshEntity *meshEnt = mesh.Entities[j];

            String tempFileName;
            tempFileName << TEXT("EditorTemp/") << IntString(scene.counter) << TEXT(".tmp");

            XFileInputSerializer tempFile;
            if(!tempFile.Open(tempFileName))
            {
                ++scene.counter;
                continue;
            }

            meshEnt->lightmap.FreeData();

            LightmapRender render;
            render.Serialize(tempFile, GetMeshRes(meshEnt), GetMeshRes(meshEnt), bFinal);

            if(bFinal)
            {
                UpdateLightmapProgress(25.0f, FormattedString(TEXT("Calculating Indirect Light Samples...\r\nEntity '%s'..."), meshEnt->GetName().Array()));

                SamplerMeshInfo info;
                info.nVerts = meshEnt->mesh->nVerts;
                info.nSections = meshEnt->mesh->nSections;
                info.UVList = (UVCoord*)meshEnt->mesh->VertBuffer->GetData()->TVList[0].Array();
                info.FaceList = meshEnt->mesh->FaceList;
                info.SectionList =  meshEnt->mesh->SectionList;
                info.NormalList =  meshEnt->mesh->VertBuffer->GetData()->NormalList.Array();
                info.render = &render;
                info.transform = &meshEnt->GetTransform();
                for(int j=0; j<meshEnt->mesh->nSections; j++)
                {
                    Material *mat = meshEnt->MaterialList[j];
                    if(!mat) mat = level->defaultMaterial;

                    SamplerTexture tex;
                    Texture *texture = (Texture*)mat->GetCurrentTexture(TEXT("diffuseTexture"));
                    tex.rgba = (BGRA*)texture->GetImage(TRUE);
                    if(texture->GetFormat() == GS_DXT1)
                    {
                        DXT1ToBGRA((void*&)tex.rgba, texture->Width(), texture->Height());
                        tex.bHasAlpha = TRUE;
                    }
                    else if(texture->GetFormat() == GS_DXT3)
                    {
                        DXT3ToBGRA((void*&)tex.rgba, texture->Width(), texture->Height());
                        tex.bHasAlpha = TRUE;
                    }
                    else if(texture->GetFormat() == GS_DXT5)
                    {
                        DXT5ToBGRA((void*&)tex.rgba, texture->Width(), texture->Height());
                        tex.bHasAlpha = TRUE;
                    }
                    else if(texture->GetFormat() == GS_RGB)
                        tex.bHasAlpha = FALSE;
                    else if(texture->GetFormat() == GS_RGBA)
                        tex.bHasAlpha = TRUE;

                    tex.texWidth = texture->Width();
                    tex.texHeight = texture->Height();

                    info.textures << tex;
                }

                BuildObjSamples(meshEnt->GetMeshBounds().GetTransformedBounds(meshEnt->GetTransform()), info, meshEnt->lightmap, GetMeshRes(meshEnt), meshEnt->phyObject);

                UpdateLightmapProgress(100.0f, FormattedString(TEXT("Calculating Indirect Light Samples...\r\nEntity '%s'..."), meshEnt->GetName().Array()));
                Sleep(10);

                if(bDXT1)   texVal = ConvertToDXT1(render.lightmap[0], GetMeshRes(meshEnt), GetMeshRes(meshEnt));
                else        texVal = ConvertToRGB(render.lightmap[0], GetMeshRes(meshEnt), GetMeshRes(meshEnt));
                meshEnt->lightmap.X = CreateTexture(GetMeshRes(meshEnt), GetMeshRes(meshEnt), (bDXT1) ? GS_DXT1 : 3, texVal, FALSE);
                Free(texVal);
                if(bDXT1)   texVal = ConvertToDXT1(render.lightmap[1], GetMeshRes(meshEnt), GetMeshRes(meshEnt));
                else        texVal = ConvertToRGB(render.lightmap[1], GetMeshRes(meshEnt), GetMeshRes(meshEnt));
                meshEnt->lightmap.Y = CreateTexture(GetMeshRes(meshEnt), GetMeshRes(meshEnt), (bDXT1) ? GS_DXT1 : 3, texVal, FALSE);
                Free(texVal);

                if(bDXT1)   texVal = ConvertToDXT1(render.lightmap[2], GetMeshRes(meshEnt), GetMeshRes(meshEnt));
                else        texVal = ConvertToRGB(render.lightmap[2], GetMeshRes(meshEnt), GetMeshRes(meshEnt));
                meshEnt->lightmap.Z = CreateTexture(GetMeshRes(meshEnt), GetMeshRes(meshEnt), (bDXT1) ? GS_DXT1 : 3, texVal, FALSE);
                Free(texVal);
            }
            else
            {
                texVal = ConvertToRGB(render.baseLightmap, GetMeshRes(meshEnt), GetMeshRes(meshEnt));
                meshEnt->lightmap.plain = CreateTexture(GetMeshRes(meshEnt), GetMeshRes(meshEnt), (bDXT1) ? GS_DXT1 : 3, texVal, FALSE);
                Free(texVal);
            }

            ++scene.counter;
        }
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  MaterialWindow.h

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


#include "Xed.h"


DefineClass(MaterialWindow);
DefineClass(MaterialItem);


void MaterialWindow::Init()
{
    traceIn(MaterialWindow::Init);

    Super::Init();

    SetSystem(materialEditor->materialView);
    Size.x = 450.0f;

    OSFindData ofdModule;
    HANDLE hFindModule = OSFindFirstFile(TEXT("data/*."), ofdModule);
    if(hFindModule)
    {
        do
        {
            String strPath;
            strPath << TEXT("data/") << ofdModule.fileName;

            OSFindData fd;
            HANDLE hFind = OSFindFirstFile(strPath + TEXT("/effects/*.effect"), fd);

            if(hFind)
            {
                do
                {
                    String effectName, path;

                    effectName << ofdModule.fileName << TEXT(":") << fd.fileName;

                    path << strPath << TEXT("/effects/") << fd.fileName;

                    Effects << materialEditor->materialView->CreateEffectFromFile(path);
                    EffectNames << new String(effectName);
                }while(OSFindNextFile(hFind, fd));

                OSFindClose(hFind);
            }
        }while(OSFindNextFile(hFindModule, ofdModule));

        OSFindClose(hFindModule);
    }

    //-------------------------------------------------

    Mesh *sphereMesh = new Mesh;
    Mesh *boxMesh = new Mesh;

    sphereMesh->LoadMesh(TEXT("Editor:Sphere.xmd"));
    boxMesh->LoadMesh(TEXT("Editor:Box.xmd"));

    sphereVertBuffer = materialEditor->materialView->CloneVertexBuffer(sphereMesh->VertBuffer);
    sphereIdxBuffer = materialEditor->materialView->CloneIndexBuffer(sphereMesh->IdxBuffer);

    boxVertBuffer = materialEditor->materialView->CloneVertexBuffer(boxMesh->VertBuffer);
    boxIdxBuffer = materialEditor->materialView->CloneIndexBuffer(boxMesh->IdxBuffer);

    delete sphereMesh;
    delete boxMesh;

    BYTE chi[24];

    memset(chi, INVALID, 12);

    BlankAttenuation = materialEditor->materialView->CreateTexture(2, 2, 3, chi, FALSE, TRUE);

    traceOut;
}

void MaterialWindow::Destroy()
{
    traceIn(MaterialWindow::Destroy);

    DWORD i;

    delete BlankAttenuation;

    CleanupItems();

    for(i=0; i<Effects.Num(); i++)
    {
        delete EffectNames[i];
        delete Effects[i];
    }
    Effects.Clear();
    EffectNames.Clear();

    delete sphereIdxBuffer;
    delete sphereVertBuffer;

    delete boxIdxBuffer;
    delete boxVertBuffer;

    Super::Destroy();

    traceOut;
}

void MaterialWindow::CleanupItems()
{
    traceIn(MaterialWindow::CleanupItems);

    for(DWORD i=0; i<Items.Num(); i++)
        DestroyObject(Items[i]);
    Items.Clear();

    bCanDraw = FALSE;

    selectedMaterial = NULL;

    traceOut;
}


void MaterialWindow::SetDirectory(CTSTR lpDir)
{
    traceIn(MaterialWindow::SetDirectory);

    DWORD i;

    CleanupItems();

    Pos.Set(0.0f, 0.0f);

    OSFindData fd;

    String path;
    if(lpDir)
        path << TEXT("data/") << materialEditor->curModule << TEXT("/materials/") << lpDir << TEXT("/*.mtl");
    else
        path << TEXT("data/") << materialEditor->curModule << TEXT("/materials/*.mtl");
    HANDLE hFind = OSFindFirstFile(path, fd);

    float curX = 10.0f, curY = 10.0f;

    Font *font = materialEditor->materialView->GetFont(TEXT("Base:Arial Medium.xft"));

    if(hFind)
    {
        do
        {
            path.Clear();
            if(lpDir)
                path << materialEditor->curModule << TEXT(":") << lpDir << TEXT("/") << fd.fileName;
            else
                path << materialEditor->curModule << TEXT(":") << fd.fileName;

            Engine::ConvertResourceName(path, TEXT("materials"), path, editor->bAddonModule);

            ConfigFile materialFile;
            materialFile.Open(path);

            String strEffect = materialFile.GetString(TEXT("Material"), TEXT("Effect"));
            Effect *effect;

            for(i=0; i<EffectNames.Num(); i++)
            {
                if(EffectNames[i]->CompareI(strEffect))
                {
                    effect = Effects[i];

                    String strName(fd.fileName);

                    MaterialItem *item = new MaterialItem;
                    item->strName = GetPathWithoutExtension(strName);
                    item->LoadFile(effect, materialFile);
                    item->SetSize(120.0f, 120.0f);
                    item->InitializeObject();

                    item->Attach(this);

                    if(Items.Num())
                    {
                        curX += 130.0f;
                        if(curX >= 320.0f)
                        {
                            curX = 10.0f;
                            curY += 130.0f;
                            curY += float(font->GetFontHeight());
                        }
                    }

                    item->SetPos(curX, curY);

                    Items << item;
                    break;
                }
            }
        }while(OSFindNextFile(hFind, fd));
        OSFindClose(hFind);

        curY += 130.0f;
        curY += float(font->GetFontHeight());

        if(curY > (500.0f-130.0f))
        {
            int adjust = int(curY - (500.0f-130.0f));

            SCROLLINFO si;
            zero(&si, sizeof(si));

            si.cbSize = sizeof(si);
            si.fMask = SIF_PAGE|SIF_POS|SIF_RANGE;
            si.nPos = 0;
            si.nMin = 0;
            si.nMax = adjust;
            si.nPage = 130;
            SetScrollInfo(materialEditor->hwndScrollBar, SB_CTL, &si, TRUE);
            EnableWindow(materialEditor->hwndScrollBar, TRUE);
        }
        else
            EnableWindow(materialEditor->hwndScrollBar, FALSE);
    }

    Size.y = MAX(500.0f, curY+130.0f);

    traceOut;
}

void MaterialWindow::MouseDown(DWORD button)
{
    traceIn(MaterialWindow::MouseDown);

    if(button != MOUSE_MIDDLEBUTTON) 
    {
        selectedMaterial = NULL;
        UpdateViewports(materialEditor->hwndMaterialView);

        if(button == MOUSE_RIGHTBUTTON)
        {
            HMENU hmenu = LoadMenu(hinstMain, MAKEINTRESOURCE(IDR_POPUPS));
            HMENU hmenuPopup = GetSubMenu(hmenu, 1);

            POINT p;
            GetCursorPos(&p);
            TrackPopupMenuEx(hmenuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, materialEditor->hwndMaterialEditor, NULL);
            SendMessage(materialEditor->hwndMaterialView, WM_RBUTTONUP, 0, 0);

            DestroyMenu(hmenu);
        }
    }

    traceOut;
}


void MaterialItem::LoadFile(Effect *effect, ConfigFile &materialFile)
{
    traceIn(MaterialWindow::LoadFile);

    if(!curMaterial)
    {
        curMaterial = CreateObject(Material);
        curMaterial->flags |= MATERIAL_EDITING;
    }

    curMaterial->effect = effect;

    DWORD curParamID = 0;

    dwDrawType = materialFile.GetInt(TEXT("Material"), TEXT("DrawType"));
    curMaterial->SetFriction(materialFile.GetFloat(TEXT("Material"), TEXT("Friction"), 0.5f));
    curMaterial->SetRestitution(materialFile.GetFloat(TEXT("Material"), TEXT("Restitution"), 0.0f));
    String softSound = materialFile.GetString(TEXT("Material"), TEXT("SoftSound"));
    curMaterial->SetSoftHitSound(softSound);
    curMaterial->SetHardHitSound(materialFile.GetString(TEXT("Material"), TEXT("HardSound"), softSound));

    HANDLE hCurParam;
    while(hCurParam = effect->GetParameter(curParamID++))
    {
        EffectParameterInfo paramInfo;
        effect->GetEffectParameterInfo(hCurParam, paramInfo);

        if(paramInfo.propertyType != EffectProperty_None)
        {
            MaterialParameter newParam;

            if(paramInfo.propertyType == EffectProperty_Texture)
            {
                String strTexture;
                if(Engine::ConvertResourceName(materialFile.GetString(TEXT("Parameters"), paramInfo.name), TEXT("textures"), strTexture))
                {
                    newParam.type = Parameter_Texture;
                    newParam.handle = hCurParam;

                    Texture *texture = materialEditor->materialView->CreateTextureFromFile(strTexture, FALSE);
                    *(Texture**)newParam.data = texture;

                    if(scmpi(paramInfo.name, TEXT("diffuseTexture")) == 0)
                        mainTextureParam = curMaterial->Params.Num();

                    curMaterial->Params << newParam;
                }
            }
            else if(paramInfo.propertyType == EffectProperty_Float)
            {
                newParam.type = Parameter_Float;
                newParam.handle = hCurParam;

                *(float*)newParam.data = materialFile.GetFloat(TEXT("Parameters"), paramInfo.name)*paramInfo.fMul;

                curMaterial->Params << newParam;
            }
            else if(paramInfo.propertyType == EffectProperty_Color)
            {
                newParam.type = Parameter_Vector3;
                newParam.handle = hCurParam;
                Color3 val = materialFile.GetColor3(TEXT("Parameters"), paramInfo.name);
                mcpy(newParam.data, &val, sizeof(Color3));

                curMaterial->Params << newParam;
            }
        }
    }

    traceOut;
}

Material* MaterialWindow::SetEditMode(DWORD dwDrawType)
{
    traceIn(MaterialWindow::SetEditMode);

    CleanupItems();

    selectedMaterial = NULL;
    bCanDraw = FALSE;

    Pos.Set(0.0f, 0.0f);

    MaterialItem *item = CreateBare(MaterialItem);
    item->SetSize(200.0f, 200.0f);
    item->InitializeObject();

    item->dwDrawType = dwDrawType;

    item->Attach(this);

    Items << item;
    
    item->curMaterial = CreateObject(Material);
    item->curMaterial->flags |= MATERIAL_EDITING;

    return item->curMaterial;

    traceOut;
}

void MaterialWindow::DeleteMaterial()
{
    traceIn(MaterialWindow::DeleteMaterial);

    if(!selectedMaterial)
        return;

    //--------------------------------------

    String message;
    message << TEXT("Are you sure you wish to delete '") << selectedMaterial->strName << TEXT("'?");
    int query = MessageBox(materialEditor->hwndMaterialEditor, message, TEXT("Just out of curiosity."), MB_YESNO);

    if(query == IDNO)
        return;

    //---------------------------------------

    String strResourceName;
    materialEditor->GetCurItemResourceName(strResourceName);

    Material *levelMaterial = RM->UsingMaterial(strResourceName);
    if(levelMaterial)
    {
        levelInfo->RemoveMaterialFromScene(levelMaterial);
        UpdateViewports();
    }

    //---------------------------------------

    String path;
    materialEditor->GetCurItemPath(path);

    path << TEXT("/") << selectedMaterial->strName << TEXT(".mtl");

    DeleteFile(path);

    int matID = Items.FindValueIndex(selectedMaterial);

    Items.Remove(matID);

    Vect2 curVal = selectedMaterial->GetLocalPos();

    selectedMaterial->CleanupParams();
    DestroyObject(selectedMaterial);

    Font *font = materialEditor->materialView->GetFont(TEXT("Base:Arial Medium.xft"));

    for(DWORD i=matID; i<Items.Num(); i++)
    {
        MaterialItem *item = Items[i];

        if(i > matID)
        {
            curVal.x += 130.0f;
            if(curVal.x >= 320.0f)
            {
                curVal.x = 10.0f;
                curVal.y += 130.0f;
                curVal.y += float(font->GetFontHeight());
            }
        }

        item->SetPos(curVal);
    }

    curVal.y += 130.0f;
    curVal.y += float(font->GetFontHeight());

    if(curVal.y > (500.0f-130.0f))
    {
        int adjust = int(curVal.y - (500.0f-130.0f));

        SCROLLINFO si;
        zero(&si, sizeof(si));

        si.cbSize = sizeof(si);
        si.fMask = SIF_PAGE|SIF_POS|SIF_RANGE;
        si.nPos = 0;
        si.nMin = 0;
        si.nMax = adjust;
        si.nPage = 130;
        SetScrollInfo(materialEditor->hwndScrollBar, SB_CTL, &si, TRUE);
        EnableWindow(materialEditor->hwndScrollBar, TRUE);
    }
    else
        EnableWindow(materialEditor->hwndScrollBar, FALSE);

    Size.y = MAX(500.0f, curVal.y+130.0f);

    UpdateViewports(materialEditor->hwndMaterialView);

    selectedMaterial = NULL;

    //--------------------------------------

    String strOldDir;
    strOldDir << TEXT("data/") << materialEditor->curModule << TEXT("/materials");

    OSFindData ofd;
    HANDLE hFind = OSFindFirstFile(strOldDir + TEXT("/*.*"), ofd);

    if(!hFind)
        RemoveDirectory(strOldDir);
    else
        OSFindClose(hFind);

    traceOut;
}


void MaterialItem::Init()
{
    traceIn(MaterialItem::Init);

    Super::Init();

    SetSystem(materialEditor->materialView);

    traceOut;
}

void MaterialItem::Destroy()
{
    traceIn(MaterialItem::Destroy);

    Super::Destroy();

    CleanupParams();

    traceOut;
}

void MaterialItem::Render()
{
    traceInFast(MaterialItem::Render);

    MaterialWindow *window = (MaterialWindow*)GetParent();
    Effect *effect = curMaterial->effect;

    if(!effect)
        return;

    GraphicsSystem *materialView = materialEditor->materialView;

    //------------------------------

    //note: |  most of this stuff is to adjust the frustum in case it's
    //      |  cropped at the bottom or top of the window.
    //      |
    //      |  ...sometimes these types of notes are just plain useful.
    //      |  I mean, have you ever spent months away from specific
    //      |  code to come back and say, "WTF did I write here?"

    Vect2 offsetPos = GetRealPos();

    Vect2 materialViewSize = materialView->GetSize();

    float top = offsetPos.y;
    float bottom = offsetPos.y+Size.y;

    if(bottom <= 0.0f)
        return;

    if(top >= materialViewSize.y)
        return;

    float topMultiply = 1.0f;
    float bottomMultiply = 1.0f;

    if(bottom > materialViewSize.y)
    {
        bottom = materialViewSize.y - offsetPos.y;
        bottomMultiply = bottom/Size.y;
    }
    else
        bottom = Size.y;

    if(top < 0.0f)
    {
        topMultiply = ((Size.y+top)/Size.y);
        bottom += top;
        top = 0.0f;
    }

    //------------------------------

    materialView->SetViewport(offsetPos.x, top, Size.x, bottom);

    VertexBuffer *flatVertBuffer = NULL;

    if(dwDrawType == DRAWMATERIAL_SPHERE)
    {
        materialView->LoadVertexBuffer(window->sphereVertBuffer);
        materialView->LoadIndexBuffer(window->sphereIdxBuffer);
    }
    else if(dwDrawType == DRAWMATERIAL_BOX)
    {
        materialView->LoadVertexBuffer(window->boxVertBuffer);
        materialView->LoadIndexBuffer(window->boxIdxBuffer);
    }
    else
    {
        assert(mainTextureParam != -1);
        if(mainTextureParam != -1)
        {
            MaterialParameter &param = curMaterial->Params[mainTextureParam];

            Texture* texture = *(Texture**)param.data;

            if(texture)
            {
                float texWidth = texture->Width();
                float texHeight = texture->Height();

                if(texWidth > texHeight)
                {
                    texHeight = (10.0f*(texHeight/texWidth))*0.5f;
                    texWidth = 5.0f;
                }
                else
                {
                    texWidth = (10.0f*(texWidth/texHeight))*0.5f;
                    texHeight = 5.0f;
                }

                VBData *vbd = new VBData;

                    vbd->VertList << Vect(-texWidth, -texHeight, 0.0f);
                    vbd->VertList << Vect( texWidth, -texHeight, 0.0f);
                    vbd->VertList << Vect(-texWidth,  texHeight, 0.0f);
                    vbd->VertList << Vect( texWidth,  texHeight, 0.0f);

                    Vect normalVect(0.0f, 0.0f, 1.0f);
                    vbd->NormalList << normalVect;
                    vbd->NormalList << normalVect;
                    vbd->NormalList << normalVect;
                    vbd->NormalList << normalVect;

                    normalVect.Set(-1.0f, 0.0f, 0.0f);
                    vbd->TangentList << normalVect;
                    vbd->TangentList << normalVect;
                    vbd->TangentList << normalVect;
                    vbd->TangentList << normalVect;

                    vbd->TVList.SetSize(1);
                    vbd->TVList[0].SetWidth(2);
                    *vbd->TVList[0].GetV2() << UVCoord(0.0f, 1.0f);
                    *vbd->TVList[0].GetV2() << UVCoord(1.0f, 1.0f);
                    *vbd->TVList[0].GetV2() << UVCoord(0.0f, 0.0f);
                    *vbd->TVList[0].GetV2() << UVCoord(1.0f, 0.0f);

                flatVertBuffer = materialView->CreateVertexBuffer(vbd);

                materialView->LoadIndexBuffer(NULL);
                materialView->LoadVertexBuffer(flatVertBuffer);
            }
        }
    }

    //------------------------------

    Vect eyePos(0.0f, 0.0f, 21.0f);
    Vect lightPos(-6.0f, 6.0f, 13.0f);

    //------------------------------

    // automatically adjusts for cropping!  ...  :D
    materialView->Frustum(-0.1, 0.1, (-0.2*bottomMultiply)+0.1, (0.2*topMultiply)-0.1, 0.4, 1000.0f);

    //------------------------------

    materialView->MatrixPush();
    materialView->MatrixIdentity();
    materialView->MatrixTranslate(-eyePos);

    if(dwDrawType == DRAWMATERIAL_BOX)
    {
        Quat adjustRot;
        adjustRot = AxisAngle(1.0f, 0.0f, 0.0f, RAD(30.0f));
        adjustRot *= AxisAngle(0.0f, 1.0f, 0.0f, RAD(45.0f));

        Matrix rotMatrix(adjustRot);
        eyePos.TransformVector(rotMatrix);
        lightPos.TransformVector(rotMatrix);

        materialView->MatrixRotate(adjustRot);
    }
    else if(dwDrawType == DRAWMATERIAL_SPHERE)
    {
        Quat adjustRot;
        adjustRot = AxisAngle(1.0f, 0.0f, 0.0f, RAD(20.0f));
        adjustRot *= AxisAngle(0.0f, 1.0f, 0.0f, RAD(40.0f));

        Matrix rotMatrix(adjustRot);
        eyePos.TransformVector(rotMatrix);
        lightPos.TransformVector(rotMatrix);

        materialView->MatrixRotate(adjustRot);
    }

    //------------------------------

    HANDLE hTechnique;

    if(window->selectedMaterial == this)
        materialView->ClearColorBuffer(FALSE, 0xFF505000);

    materialView->ColorWriteEnable(1, 1, 1, 1);
    materialView->EnableBlending(FALSE);
    materialView->BlendFunction(GS_BLEND_ONE, GS_BLEND_ONE);

    materialView->EnableDepthTest(TRUE);
    materialView->DepthWriteEnable(TRUE);
    materialView->DepthFunction(GS_LESS);

    hTechnique = effect->GetTechnique(TEXT("InitialPass"));
    if(hTechnique)
    {
        effect->BeginTechnique(hTechnique);
        if(effect->BeginPassByName(TEXT("Normal")))
        {
            if(curMaterial->LoadParameters())
            {
                materialView->Draw((dwDrawType == DRAWMATERIAL_FLAT) ? GS_TRIANGLESTRIP : GS_TRIANGLES);
                window->bCanDraw = TRUE;
            }
            else
                window->bCanDraw = FALSE;

            effect->EndPass();
        }
        effect->EndTechnique();
    }

    //------------------------------

    materialView->EnableBlending(TRUE);
    materialView->DepthWriteEnable(FALSE);
    materialView->DepthFunction(GS_LEQUAL);

    hTechnique = effect->GetTechnique(TEXT("PointLight"));

    if(hTechnique)
    {
        HANDLE hParam;

        hParam = effect->GetParameterByName(TEXT("lightRange"));
        effect->SetFloat(hParam, 1.0f);

        hParam = effect->GetParameterByName(TEXT("lightColor"));
        effect->SetColor(hParam, 0xFFFFFFFF);

        hParam = effect->GetParameterByName(TEXT("attenuationMap"));
        effect->SetTexture(hParam, window->BlankAttenuation);

        hParam = effect->GetParameterByName(TEXT("eyePos"));
        effect->SetVector(hParam, eyePos);

        hParam = effect->GetParameterByName(TEXT("lightPos"));
        effect->SetVector(hParam, lightPos);

        DWORD passes = effect->BeginTechnique(hTechnique);

        if(effect->BeginPassByName(TEXT("Normal")))
        {
            if(curMaterial->LoadParameters())
                materialView->Draw((dwDrawType == DRAWMATERIAL_FLAT) ? GS_TRIANGLESTRIP : GS_TRIANGLES);

            effect->EndPass();
        }
        effect->EndTechnique();
    }

    //------------------------------

    materialView->MatrixPop();

    materialView->LoadVertexBuffer(NULL);
    materialView->LoadIndexBuffer(NULL);
    materialView->ResetViewport();

    delete flatVertBuffer;

    if(strName.Length())
    {
        materialView->SetCurFont(TEXT("Base:Arial Medium.xft"));
        materialView->SetFontColor(0xFF7F7FFF);

        String adjName;

        Font *curFont = materialView->GetCurFont();

        TSTR lpTemp = strName;
        int curWidth = 0;
        do
        {
            int letterWidth = curFont->LetterWidth(*lpTemp);
            curWidth += letterWidth;
            if(curWidth > (int)Size.x)
            {
                adjName.AppendChar('\n');
                curWidth = letterWidth;
            }
            adjName.AppendChar(*lpTemp);
        }while(*++lpTemp);

        materialView->BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);

        for(int i=0; i<adjName.NumTokens('\n'); i++)
            materialView->DrawTextCenter(Size.x*0.5f, Size.y+((float)curFont->GetFontHeight()*i), adjName.GetToken(i, '\n'));
    }

    traceOutFast;
}


void MaterialItem::MouseDown(DWORD button)
{
    traceIn(MaterialItem::MouseDown);

    MaterialWindow *window = (MaterialWindow*)GetParent();
    if(button == MOUSE_LEFTBUTTON)
    {
        if(!materialEditor->bEditMode)
        {
            if(clickTime)
            {
                clickTime = OSGetTime() - clickTime;

                if(clickTime <= GetDoubleClickTime())
                {
                    EditMaterial();
                    SendMessage(materialEditor->hwndMaterialView, WM_LBUTTONUP, 0, 0);
                    return;
                }

                clickTime = 0;
            }

            clickTime = OSGetTime();

            window->selectedMaterial = this;
            UpdateViewports(materialEditor->hwndMaterialView);

            String resName;
            materialEditor->GetCurItemResourceName(resName);
            levelInfo->SetPolyMaterials(resName);

            UpdateViewports(hwndMain);
        }
    }
    else if(button == MOUSE_RIGHTBUTTON)
    {
        if(!materialEditor->bEditMode)
        {
            clickTime = 0;

            window->selectedMaterial = this;
            UpdateViewports(materialEditor->hwndMaterialView);

            String resName;
            materialEditor->GetCurItemResourceName(resName);
            levelInfo->SetPolyMaterials(resName);

            UpdateViewports(hwndMain);

            HMENU hmenu = LoadMenu(hinstMain, MAKEINTRESOURCE(IDR_POPUPS));
            HMENU hmenuPopup = GetSubMenu(hmenu, 2);

            POINT p;
            GetCursorPos(&p);
            TrackPopupMenuEx(hmenuPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, materialEditor->hwndMaterialEditor, NULL);
            SendMessage(materialEditor->hwndMaterialView, WM_RBUTTONUP, 0, 0);

            DestroyMenu(hmenu);
        }
    }

    traceOut;
}


void MaterialItem::EditMaterial()
{
    traceIn(MaterialItem::EditMaterial);

    materialEditor->EditMaterial(String(strName));

    traceOut;
}


void MaterialItem::CleanupParams()
{
    traceIn(MaterialItem::CleanupParams);

    /*for(DWORD i=0; i<curMaterial->Params.Num(); i++)
    {
        MaterialParameter &param = curMaterial->Params[i];

        if(param.type == Parameter_Texture)
            delete (*(Texture**)param.data);
    }
    curMaterial->strSoundHard.Clear();
    curMaterial->strSoundSoft.Clear();
    curMaterial->Params.Clear();
    zero(&curMaterial, sizeof(curMaterial));*/
    DestroyObject(curMaterial);
    curMaterial = NULL;

    traceOut;
}


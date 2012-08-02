/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditorLevel.cpp

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


DefineClass(EditorLevelInfo);

EditorLevelInfo *levelInfo;




EditorLevelInfo::EditorLevelInfo()
  : curSelectMode(SelectMode_ObjectsAndDetails),
    gridSpacing(1.0f),
    bSnapToGrid(TRUE)
{
    traceIn(EditorLevelInfo::EditorLevelInfo);

    SetCurModifyMode(ModifyMode_Select);
    SetCurSelectMode(SelectMode_ObjectsAndDetails);

    traceOut;
}

EditorLevelInfo::~EditorLevelInfo()
{
    traceIn(EditorLevelInfo::~EditorLevelInfo);

    if(WorkBrush)
        DestroyObject(WorkBrush);

    if(newObject)
        delete newObject;

    DWORD numBrushes = BrushList.Num();
    for(DWORD i=0; i<numBrushes; i++)
        DestroyObject(BrushList[0]);
    BrushList.Clear();

    if(curManipulator)
        DestroyObject(curManipulator);

    editor->undoStack->Clear();
    editor->redoStack->Clear();

    traceOut;
}

void EditorLevelInfo::Deselect(Entity *ent)
{
    traceIn(EditorLevelInfo::Deselect);

    if(ent)
    {
        SelectedObjects.RemoveItem(ent);
        ent->bSelected = false;

        if(SelectedObjects.Num())
        {
            Vect averagedPos;
            zero(&averagedPos, sizeof(averagedPos));

            for(DWORD i=0; i<SelectedObjects.Num(); i++)
                averagedPos += SelectedObjects[i]->GetWorldPos();

            averagedPos /= float(SelectedObjects.Num());

            curManipulator->SetPos(averagedPos);
        }
        else
        {
            DestroyObject(curManipulator);
            curManipulator = NULL;
        }
    }
    else
    {
        for(DWORD i=0; i<SelectedObjects.Num(); i++)
        {
            Entity *curEnt = SelectedObjects[i];
            curEnt->bSelected = false;
        }

        SelectedObjects.Clear();

        DestroyObject(curManipulator);
        curManipulator = NULL;
    }

    traceOut;
}

void EditorLevelInfo::Select(Entity *ent)
{
    traceIn(EditorLevelInfo::Select);

    if(SelectedObjects.FindValueIndex(ent) == INVALID)
    {
        SelectedObjects << ent;
        ent->bSelected = true;

        if(!curManipulator)
        {
            if(levelInfo->curModifyMode == ModifyMode_Select)
                curManipulator = CreateObject(DefaultManipulator);
            else if(levelInfo->curModifyMode == ModifyMode_Move)
                curManipulator = CreateObject(PositionManipulator);
            else if(levelInfo->curModifyMode == ModifyMode_Rotate)
                curManipulator = CreateObject(RotationManipulator);
        }

        UpdateManipulatorPos();
    }

    traceOut;
}

void SelectMultiple(List<Entity*> entities)
{
    traceIn(EditorLevelInfo::SelectMultiple);

    

    traceOut;
}


void EditorLevelInfo::UpdateManipulatorPos()
{
    traceIn(EditorLevelInfo::UpdateManipulatorPos);

    if(curManipulator)
    {
        Vect averagedPos;
        zero(&averagedPos, sizeof(averagedPos));

        for(DWORD i=0; i<SelectedObjects.Num(); i++)
            averagedPos += SelectedObjects[i]->GetWorldPos();

        averagedPos /= float(SelectedObjects.Num());

        curManipulator->SetPos(averagedPos);
    }

    traceOut;
}

void EditorLevelInfo::DrawEditorObjects(Camera *camera, float cameraZoom, EditorViewport *vp)
{
    traceIn(EditorLevelInfo::DrawEditorObjects);

    assert(camera);

    profileSegment("editor objects");

    if(!camera)
        return;

    Matrix m;
    DWORD i;

    if(level)
        level->renderCamera = camera;

    m.SetIdentity();
    m *= camera->GetWorldPos();
    m *= camera->GetWorldRot();

    MatrixSet(m.GetTranspose());

    ViewClip curClip;

    curClip = camera->clip;
    curClip.Transform(m);

    List<LevelObject*> objects;
    level->GetObjects(camera->GetWorldPos(), curClip, objects);

    //---------------------------------------------------

    DepthFunction(GS_LEQUAL);

    Shader *solidColor = GetVertexShader(TEXT("Base:SolidColor.vShader"));
    LoadVertexShader(solidColor);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    for(i=0; i<objects.Num(); i++)
    {
        LevelObject *levelObj = objects[i];

        if(levelObj->type == ObjectType_Entity)
        {
            Entity *ent = levelObj->ent;
            if(ent->IsOf(GetClass(EditorBrush)))
            {
                EditorBrush *brush = (EditorBrush*)ent;

                EnableDepthTest(TRUE);

                if(brush->SelectionIdxBuffer)
                {
                    solidColor->SetColor(solidColor->GetParameter(1), 1.0f, 1.0f, 0.0f, 0.4f);

                    LoadVertexBuffer(brush->GetLevelBrush()->VertBuffer);
                    LoadIndexBuffer(brush->SelectionIdxBuffer);

                    GS->Draw(GS_TRIANGLES);
                }

                if((vp->drawType == ViewportDrawType_Brushes) || brush->bSelected)
                {
                    MatrixPush();
                    MatrixTranslate(brush->GetLocalPos());
                    MatrixRotate(brush->GetLocalRot());

                    if(brush->brushType == BrushType_Addition)
                        solidColor->SetColor(solidColor->GetParameter(1), 1.0f, 0.75f, 0.25f, 1.0f);
                    else if(brush->brushType == BrushType_Subtraction)
                        solidColor->SetColor(solidColor->GetParameter(1), 0.25f, 0.75f, 1.0f, 1.0f);

                    LoadVertexBuffer(brush->WFVertBuffer);
                    LoadIndexBuffer(brush->WFIdxBuffer);

                    Draw(GS_LINES);

                    LoadVertexBuffer(NULL);
                    LoadIndexBuffer(NULL);

                    MatrixPop();
                }
            }
            else if(ent->IsOf(GetClass(MeshEntity)))
            {
                MeshEntity *meshEnt = (MeshEntity*)ent;
                if(meshEnt->bSelected)
                {
                    EnableDepthTest(FALSE);

                    MatrixPush();
                    MatrixMultiply(meshEnt->GetInvTransform());
                        editor->selBoxThing->Render(meshEnt->GetMeshBounds());
                    MatrixPop();
                }
            }
        }
    }

    //---------------------------------------------------

    EnableDepthTest(FALSE);
    LoadVertexShader(NULL);
    LoadPixelShader(NULL);

    if(curManipulator)
    {
        float adjustSize;

        if(camera->bPerspective)
            adjustSize = (camera->GetWorldPos().Dist(curManipulator->GetWorldPos())*cameraZoom)/50.0f;
        else
            adjustSize = cameraZoom/50.0f;

        Vect cameraDir;

        if(camera->bPerspective)
            cameraDir = (curManipulator->GetWorldPos()-camera->GetWorldPos()).Norm();
        else
            cameraDir = -camera->GetLocalRot().GetDirectionVector();

        curManipulator->RenderScaled(cameraDir, adjustSize);
    }

    if(newObject && !vp->bWasDragging)
        newObject->Render();

    LoadVertexShader(solidColor);
    LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

    if(WorkBrush)
    {
        MatrixPush();
        MatrixTranslate(WorkBrush->GetLocalPos());
        MatrixRotate(WorkBrush->GetLocalRot());

        solidColor->SetColor(solidColor->GetParameter(1), 1.0f, 1.0f, 0.0f, 0.8f);
        LoadVertexBuffer(WorkBrush->WFVertBuffer);
        LoadIndexBuffer(WorkBrush->WFIdxBuffer);

        Draw(GS_LINES);

        LoadVertexBuffer(NULL);
        LoadIndexBuffer(NULL);

        MatrixPop();
    }

    LoadVertexShader(NULL);
    EnableDepthTest(TRUE);

    LoadVertexShader(NULL);
    LoadPixelShader(NULL);

    //---------------------------------------------------

    for(i=0; i<objects.Num(); i++)
    {
        LevelObject *levelObj = objects[i];

        if(levelObj->type == ObjectType_Entity)
        {
            Entity *ent = levelObj->ent;
            if(ent->UserCreatedObjectType)
                ent->EditorRender();
        }
    }

    traceOut;
}


void EditorLevelInfo::ProcessSelection(const Vect &rayOrig, const Vect &rayDir, BOOL bSelectBrushes)
{
    traceIn(EditorLevelInfo::ProcessSelection);

    BOOL bControlDown = GS->GetInput()->GetButtonState(KBC_CONTROL);

    BOOL bFoundEntity = FALSE;
    DWORD i;

    //---------------------------------------

    float bestDist = M_INFINITE;
    float entityDist = M_INFINITE;
    Entity *bestEnt;

    Entity *ent = Entity::FirstEntity();
    while(ent)
    {
        BOOL bIsEditorObj = ent->IsOf(GetClass(EditorObject));

        if(ent->IsOf(GetClass(EditorBrush)))
        {
            EditorBrush *brush = (EditorBrush*)ent;
            if((brush->brushID != -1) && !bSelectBrushes)
            {
                ent = ent->NextEntity();
                continue;
            }
        }

        if(bIsEditorObj || ent->UserCreatedObjectType)
        {
            float fDist = M_INFINITE;

            if(ent->CanSelect(rayOrig, rayDir))
                fDist = ent->GetWorldPos().Dist(rayOrig);

            if(fDist < bestDist)
            {
                bestEnt = ent;
                bestDist = fDist;
                bFoundEntity = TRUE;
            }
        }

        ent = ent->NextEntity();
    }

    if(bFoundEntity)
    {
        if( ((bestEnt->UserCreatedObjectType == TYPE_PREFAB) && (levelInfo->curSelectMode != SelectMode_WorldPrefabs))       ||
            ((bestEnt->UserCreatedObjectType == TYPE_OBJECT) && (levelInfo->curSelectMode != SelectMode_ObjectsAndDetails))  )
        {
            bFoundEntity = FALSE;
        }

        entityDist = bestDist;
    }

    //---------------------------------------

    DWORD bestBrush;
    DWORD bestPoly = INVALID;

    bestDist = M_INFINITE;

    if(!bSelectBrushes)
    {
        for(i=0; i<BrushList.Num(); i++)
        {
            EditorBrush *brush = BrushList[i];
            EditorMesh  &mesh  = brush->mesh;

            float fT;
            DWORD dwPoly = mesh.RayMeshTest(rayOrig, rayDir, &fT);

            if(dwPoly != INVALID)
            {
                if((fT > 0.0f) && (fT < bestDist))
                {
                    bestPoly = dwPoly;
                    bestBrush = i;
                    bestDist = fT;
                }
            }
        }
    }

    if((bestPoly != INVALID) && (levelInfo->curSelectMode == SelectMode_Brushes))
    {
        entityDist = bestDist-0.01f;
        bFoundEntity = TRUE;
        bestEnt = BrushList[bestBrush];
    }

    if((entityDist >= bestDist) && (levelInfo->curSelectMode == SelectMode_Textures))
    {
        if(bestPoly != INVALID)
        {
            SavePolySelectionUndoData();

            EditorBrush *brush = BrushList[bestBrush];
            DWORD existingID = brush->SelectedPolys.FindValueIndex(bestPoly);

            if(!bControlDown)
            {
                for(i=0; i<BrushList.Num(); i++)
                {
                    EditorBrush *curBrush = BrushList[i];
                    curBrush->SelectedPolys.Clear();

                    if(i != bestBrush)
                        curBrush->RebuildSelectionIndices();
                }
            }
            else if(existingID != INVALID)
                brush->SelectedPolys.Remove(existingID);

            if(existingID == INVALID)
                brush->SelectedPolys << bestPoly;

            brush->RebuildSelectionIndices();
        }
        else if(!bControlDown)
        {
            SavePolySelectionUndoData();

            for(i=0; i<BrushList.Num(); i++)
            {
                EditorBrush *curBrush = BrushList[i];

                if(curBrush->SelectedPolys.Num())
                {
                    curBrush->SelectedPolys.Clear();
                    curBrush->RebuildSelectionIndices();
                }
            }
        }
    }
    else if(bFoundEntity)
    {
        SaveObjectSelectionUndoData();

        if(!bControlDown)
            Deselect();

        if(bestEnt->bSelected)
            Deselect(bControlDown ? bestEnt : NULL);
        else
            Select(bestEnt);

        if(objectProperties)
            objectProperties->UpdateProperties();
    }

    if(!bControlDown && !bFoundEntity)
    {
        if(!bControlDown)
        {
            if(SelectedObjects.Num())
                SaveObjectSelectionUndoData();

            Deselect();
        }

        if(objectProperties)
            objectProperties->UpdateProperties();
    }

    if(surfaceProperties)
        surfaceProperties->UpdateProperties();

    traceOut;
}


void EditorLevelInfo::RightClick(const Vect &rayOrig, const Vect &rayDir, BOOL bSelectBrushes)
{
    traceIn(EditorLevelInfo::RightClick);

    BOOL bControlDown = GS->GetInput()->GetButtonState(KBC_CONTROL);

    BOOL bFoundSomething = FALSE;
    DWORD i;

    //---------------------------------------

    Plane bestPlane;
    float bestDist = M_INFINITE;
    
    Entity *bestEnt;

    Entity *ent = Entity::FirstEntity();
    while(ent)
    {
        BOOL bIsEditorObj = ent->IsOf(GetClass(EditorObject));

        if(ent->IsOf(GetClass(EditorBrush)))
        {
            EditorBrush *brush = (EditorBrush*)ent;
            if((brush->brushID != -1) && !bSelectBrushes)
            {
                ent = ent->NextEntity();
                continue;
            }
        }

        if(bIsEditorObj || ent->UserCreatedObjectType)
        {
            float fDist = M_INFINITE;

            if(ent->IsOf(GetClass(Prefab)))
            {
                ent = ent->NextEntity();
                continue;
            }

            PhyCollisionInfo ci;

            if(ent->GetRayCollision(rayOrig, rayDir, &ci))
                fDist = ci.hitPos.Dist(rayOrig);

            if(fDist < bestDist)
            {
                bestEnt = ent;
                bestDist = fDist;
                bFoundSomething = TRUE;
                bestPlane = ci.HitPlane();

                //todo - um, selection stuff here
            }
        }

        ent = ent->NextEntity();
    }

    //---------------------------------------

    DWORD bestBrush;
    DWORD bestPoly = INVALID;

    if(!bSelectBrushes)
    {
        for(i=0; i<BrushList.Num(); i++)
        {
            EditorBrush *brush = BrushList[i];
            EditorMesh  &mesh  = brush->mesh;

            float fT;
            Plane facePlane;
            DWORD dwPoly = mesh.RayMeshTest(rayOrig, rayDir, &fT, &facePlane);

            if(dwPoly != INVALID)
            {
                if((fT > 0.0f) && (fT < bestDist))
                {
                    bestPoly = dwPoly;
                    bestBrush = i;
                    bestDist = fT;
                    bestPlane = facePlane;

                    bFoundSomething = TRUE;
                }
            }
        }
    }

    POINT p;
    GetCursorPos(&p);

    Vect hitPos;
    EditorBrush *hitBrush = NULL;

    if(bestPoly != INVALID)
        hitBrush = BrushList[bestBrush];

    if(!bFoundSomething)
    {
        bestPlane.Set(0.0f, 1.0f, 0.0f, levelInfo->curYPlanePosition);

        if(CloseFloat(rayDir.Dot(bestPlane.Dir), 1.57f, 0.2) || !bestPlane.GetRayIntersection(rayOrig, rayDir, bestDist))
            return;
    }

    hitPos = rayOrig+(rayDir*bestDist);

    if(levelInfo->curSelectMode == SelectMode_Textures)
    {
        bool bHasFacesSelected = false;

        for(int i=0; i<BrushList.Num(); i++)
        {
            if(BrushList[i]->SelectedPolys.Num())
            {
                bHasFacesSelected = true;
                break;
            }
        }

        if(bHasFacesSelected)
        {
            HMENU hmenu = LoadMenu(hinstMain, MAKEINTRESOURCE(IDR_POPUPS));
            HMENU hmenuPopup = GetSubMenu(hmenu, 4);

            String strName;
            strName << TEXT("Create ") << editor->selectedEntityClass->GetName();

            if(editor->selectedEntityClass->IsAbstract())
                strName << TEXT(" (Abstract, Impossible.)");

            MENUITEMINFO mii;
            zero(&mii, sizeof(MENUITEMINFO));
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STATE|MIIM_STRING;
            mii.dwTypeData = (TSTR)strName;
            mii.fState = editor->selectedEntityClass->IsAbstract() ? MFS_DISABLED : 0;
            SetMenuItemInfo(hmenuPopup, ID_LEVELMESHCLICK_CREATEOBJ, FALSE, &mii);

            //-----------------------------------------------------------

            SetContextMenuItems(hmenuPopup);

            UINT contextItem = TrackPopupMenuEx(hmenuPopup, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, hwndEditor, NULL);
            switch(contextItem)
            {
                case ID_LEVELMESHCLICK_CREATELIGHT:
                    {
                        Light *light = CreateObject(PointLight);
                        hitPos += bestPlane.Dir*2.0f;
                        light->SetPos(hitPos);

                        light->GenerateUniqueName();

                        light->UserCreatedObjectType = TYPE_OBJECT;

                        SaveObjectCreationUndoData(light);
                        break;
                    }

                case ID_LEVELMESHCLICK_CREATEOBJ:
                    {
                        Entity *ent = (Entity*)editor->selectedEntityClass->Create();

                        if(ent->phyShape)
                            hitPos += bestPlane.Dir*ent->phyShape->GetNormalOffset(bestPlane.Dir);
                        else
                            hitPos += bestPlane.Dir*2.04f;

                        ent->SetPos(hitPos);
                        ent->InitializeObject(TRUE);

                        ent->UserCreatedObjectType = TYPE_OBJECT;

                        ent->GenerateUniqueName();

                        SaveObjectCreationUndoData(ent);
                        break;
                    }

                case ID_LEVELMESHCLICK_TEXTUREALIGN:
                    if(!editor->hwndUVEdit)
                        editor->hwndUVEdit = CreateDialog(hinstMain, MAKEINTRESOURCE(IDD_UVEDIT), hwndMain, (DLGPROC)TextureAdjustProc);
                    else
                        ShowWindow(editor->hwndUVEdit, SW_RESTORE);
                    break;

                case ID_LEVELMESHCLICK_SELECTALLBRUSHFACES:
                    if(hitBrush)
                    {
                        if(hitBrush->SelectedPolys.Num() < hitBrush->PolyList.Num())
                        {
                            SavePolySelectionUndoData();
                            hitBrush->SelectAllFaces();
                            hitBrush->RebuildSelectionIndices();
                            UpdateViewports();
                        }
                        break;
                    }

                case ID_LEVELMESHCLICK_SURFACEPROPERTIES:
                    if(!surfaceProperties)
                        new SurfacePropertiesEditor;
                    else
                        ShowWindow(surfaceProperties->hwndSurfaceProperties, SW_RESTORE);
                    break;

                default:
                    SendMessage(hwndEditor, WM_COMMAND, contextItem, 0);
            }

            GetCursorPos(&p);
            ScreenToClient(hwndMain, &p);
            StandardInput *input = static_cast<StandardInput*>(GS->GetInput());
            input->last_x = p.x;
            input->last_y = p.y;

            SendMessage(hwndMain, WM_RBUTTONUP, 0, 0);

            DestroyMenu(hmenu);
            return;
        }
    }
    else
    {
        if(SelectedObjects.Num())
        {
            HMENU hmenu = LoadMenu(hinstMain, MAKEINTRESOURCE(IDR_POPUPS));
            HMENU hmenuPopup = GetSubMenu(hmenu, 5);

            //-----------------------------------------------------------

            SetContextMenuItems(hmenuPopup);

            UINT contextItem = TrackPopupMenuEx(hmenuPopup, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, hwndEditor, NULL);

            switch(contextItem)
            {
                case ID_LEVELOBJECTCLICK_PROPERTIES:
                    if(!objectProperties)
                        new ObjectPropertiesEditor;
                    else
                        ShowWindow(objectProperties->hwndObjectProperties, SW_RESTORE);
                    break;

                default:
                    SendMessage(hwndEditor, WM_COMMAND, contextItem, 0);
            }

            SendMessage(hwndMain, WM_RBUTTONUP, 0, 0);

            GetCursorPos(&p);
            ScreenToClient(hwndMain, &p);
            StandardInput *input = static_cast<StandardInput*>(GS->GetInput());
            input->last_x = p.x;
            input->last_y = p.y;

            DestroyMenu(hmenu);
            return;
        }
    }

    //if nothing selected
    HMENU hmenu = LoadMenu(hinstMain, MAKEINTRESOURCE(IDR_POPUPS));
    HMENU hmenuPopup = GetSubMenu(hmenu, 9);

    String strName;
    strName << TEXT("Create ") << editor->selectedEntityClass->GetName();

    if(editor->selectedEntityClass->IsAbstract())
        strName << TEXT(" (Abstract, Impossible.)");

    MENUITEMINFO mii;
    zero(&mii, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STATE|MIIM_STRING;
    mii.dwTypeData = (TSTR)strName;
    mii.fState = editor->selectedEntityClass->IsAbstract() ? MFS_DISABLED : 0;
    SetMenuItemInfo(hmenuPopup, ID_LEVELMESHCLICK_CREATEOBJ, FALSE, &mii);

    //-----------------------------------------------------------

    SetContextMenuItems(hmenuPopup);

    UINT contextItem = TrackPopupMenuEx(hmenuPopup, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, hwndEditor, NULL);

    switch(contextItem)
    {
        case ID_LEVELMESHCLICK_CREATELIGHT:
            {
                Light *light = CreateObject(PointLight);
                hitPos += bestPlane.Dir*2.04f;
                light->SetPos(hitPos);

                light->GenerateUniqueName();

                light->UserCreatedObjectType = TYPE_OBJECT;

                SaveObjectCreationUndoData(light);
                break;
            }

        case ID_LEVELMESHCLICK_CREATEOBJ:
            {
                Entity *ent = (Entity*)editor->selectedEntityClass->Create();

                if(ent->phyShape)
                    hitPos += bestPlane.Dir*ent->phyShape->GetNormalOffset(bestPlane.Dir);
                else
                    hitPos += bestPlane.Dir*2.04f;

                ent->SetPos(hitPos);

                ent->UserCreatedObjectType = TYPE_OBJECT;

                ent->GenerateUniqueName();

                SaveObjectCreationUndoData(ent);
                break;
            }

        default:
            SendMessage(hwndEditor, WM_COMMAND, contextItem, 0);
    }

    SendMessage(hwndMain, WM_RBUTTONUP, 0, 0);

    GetCursorPos(&p);
    ScreenToClient(hwndMain, &p);
    StandardInput *input = static_cast<StandardInput*>(GS->GetInput());
    input->last_x = p.x;
    input->last_y = p.y;

    DestroyMenu(hmenu);

    traceOut;
}

void EditorLevelInfo::SetContextMenuItems(HMENU hMenu)
{
    traceIn(EditorLevelInfo::SetContextMenuItems);

    MENUITEMINFO mii;
    zero(&mii, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask  = MIIM_STATE;
    mii.fState = MFS_CHECKED;

    UINT contextItem;

    if(curEditMode == EditMode_Modify)
    {
        switch(curModifyMode)
        {
            case ModifyMode_Select:
                contextItem = ID_SCENE_SELECT;
                break;

            case ModifyMode_Move:
                contextItem = ID_SCENE_MOVE;
                break;

            case ModifyMode_Rotate:
                contextItem = ID_SCENE_ROTATE;
                break;

        }

        SetMenuItemInfo(hMenu, contextItem, FALSE, &mii);
    }

    //-----------------------------------------------------------

    switch(curSelectMode)
    {
        case SelectMode_ObjectsAndDetails:
            contextItem = ID_SCENE_SELECTOBJECTS;
            break;

        case SelectMode_WorldPrefabs:
            contextItem = ID_SCENE_SELECTPREFABS;
            break;

        case SelectMode_Brushes:
            contextItem = ID_SCENE_SELECTBRUSHES;
            break;

        case SelectMode_Textures:
            contextItem = ID_SCENE_SELECTTEXTURES;
            break;

    }
    SetMenuItemInfo(hMenu, contextItem, FALSE, &mii);

    traceOut;
}


void EditorLevelInfo::SetPolyMaterials(CTSTR lpMaterialName, BOOL bSaveUndo)
{
    traceIn(EditorLevelInfo::SetPolyMaterials);

    DWORD i,j;

    if(bSaveUndo)
    {
        DWORD numModifiedBrushes = 0;

        for(i=0; i<BrushList.Num(); i++)
        {
            EditorBrush *brush = BrushList[i];
            if(brush->SelectedPolys.Num())
                ++numModifiedBrushes;
        }

        if(numModifiedBrushes)
        {
            Action action;
            action.strName = TEXT("Change Materials");
            action.actionProc = EditorLevelInfo::UndoRedoChangeSurfaceMaterials;
            BufferOutputSerializer s(action.data);

            s << String(lpMaterialName);
            s << numModifiedBrushes;

            for(i=0; i<BrushList.Num(); i++)
            {
                EditorBrush *brush = BrushList[i];

                if(brush->SelectedPolys.Num())
                {
                    s << brush->name;

                    for(j=0; j<brush->Materials.Num(); j++)
                        s << RM->GetMaterialName(brush->Materials[j]);
                }
            }

            editor->undoStack->Push(action);
        }
    }

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];

        if(brush->SelectedPolys.Num())
        {
            for(j=0; j<brush->SelectedPolys.Num(); j++)
            {
                DWORD polyID = brush->SelectedPolys[j];
                if(brush->Materials[polyID])
                    RM->ReleaseMaterial(brush->Materials[polyID]);

                brush->Materials[polyID] = RM->GetMaterial(lpMaterialName);
            }

            brush->UpdateLevelBrush();
        }
    }

    traceOut;
}

void EditorLevelInfo::RemoveMaterialFromScene(Material *material)
{
    traceIn(EditorLevelInfo::RemoveMaterialFromScene);

    DWORD i,j;

    RM->ForceFreeMaterial(material);

    IndoorLevel *indoorLevel = (IndoorLevel*)level;

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        Brush *levelBrush = brush->GetLevelBrush();

        for(j=0; j<brush->Materials.Num(); j++)
        {
            if(levelBrush->Materials[j] == material)
            {
                brush->Materials[j] = NULL;
                levelBrush->Materials[j] = NULL;
            }
        }
    }

    traceOut;
}

void EditorLevelInfo::DeleteSelectedObjects(BOOL bRedo)
{
    traceIn(EditorLevelInfo::DeleteSelectedObjects);

    if(!SelectedObjects.Num())
        return;

    int i;

    DWORD num = SelectedObjects.Num();

    //--------------------------------------
    // undo stuff

    if(!bRedo)
    {
        Action action;
        BufferOutputSerializer s(action.data);

        action.actionProc = EditorLevelInfo::UndoRedoDelete;
        action.strName = TEXT("Delete Object(s)");

        s << num;

        for(i=num-1; i>=0; i--)
        {
            Entity *ent = SelectedObjects[i];

            String className = ent->GetObjectClass()->GetName();
            s << className;
            ent->Serialize(s);
        }

        editor->undoStack->Push(action);
    }

    //--------------------------------------

    for(i=num-1; i>=0; i--)
        DestroyObject(SelectedObjects[i]);
    SelectedObjects.Clear();

    if(curManipulator)
    {
        DestroyObject(curManipulator);
        curManipulator = NULL;
    }

    UpdateViewports();

    if(objectProperties)
        objectProperties->UpdateProperties();

    traceOut;
}

void EditorLevelInfo::SavePolySelectionUndoData()
{
    traceIn(EditorLevelInfo::SavePolySelectionUndoData);

    int i;

    Action action;
    action.strName = TEXT("Polygon Selection");
    action.actionProc = EditorLevelInfo::UndoRedoSelectPolys;

    BufferOutputSerializer s(action.data);

    DWORD numBrushes = 0;

    for(i=0; i<BrushList.Num(); i++)
    {
        if(BrushList[i]->SelectedPolys.Num())
            ++numBrushes;
    }

    s << numBrushes;

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *brush = BrushList[i];
        if(brush->SelectedPolys.Num())
        {
            s << brush->name;
            s << brush->SelectedPolys;
        }
    }

    editor->undoStack->Push(action);

    traceOut;
}

void EditorLevelInfo::SaveObjectSelectionUndoData()
{
    traceIn(EditorLevelInfo::SaveObjectSelectionUndoData);

    Action action;
    action.strName = TEXT("Object Selection");
    action.actionProc = EditorLevelInfo::UndoRedoSelectObjects;

    BufferOutputSerializer s(action.data);

    DWORD numSelected = SelectedObjects.Num();
    s << numSelected;

    for(int i=0; i<numSelected; i++)
        s << SelectedObjects[i]->name;

    editor->undoStack->Push(action);

    traceOut;
}

void EditorLevelInfo::SaveObjectCreationUndoData(Entity *ent)
{
    traceIn(EditorLevelInfo::SaveObjectCreationUndoData);

    Action action;
    action.strName << TEXT("Create ") << ent->GetObjectClass()->GetName();
    action.actionProc = EditorLevelInfo::UndoRedoCreateObject;

    BufferOutputSerializer s(action.data);
    s << String(ent->name);

    editor->undoStack->Push(action);

    traceOut;
}

void EditorLevelInfo::ResetEntityLevelData()
{
    traceIn(EditorLevelInfo::ResetEntityLevelData);

    level->bUpdateAllEntities = TRUE;

    if(level->IsOf(GetClass(IndoorLevel)))
    {
        IndoorLevel *indoorLevel = (IndoorLevel*)level;

        for(int j=0; j<indoorLevel->PVSList.Num(); j++)
        {
            PVS &pvs = indoorLevel->PVSList[j];

            pvs.entities.Clear();
            pvs.lights.Clear();
            pvs.visEntities.Clear();
            pvs.visMeshEntities.Clear();
            pvs.visLights.Clear();
        }
    }
    else if(level->IsOf(GetClass(OctLevel)))
    {
        OctLevel *octLevel = (OctLevel*)level;

        delete octLevel->objectTree;
        octLevel->objectTree = new OctBVH;
    }

    traceOut;
}


void EditorLevelInfo::RebuildScene()
{
    traceIn(EditorLevelInfo::RebuildScene);

    level->ClearRenderItems();

    DWORD i;

    if(level->IsOf(GetClass(IndoorLevel)))
    {
        IndoorLevel *indoorLevel = (IndoorLevel*)level;

        for(i=0; i<indoorLevel->PVSList.Num(); i++)
            indoorLevel->PVSList[i].Clear();
        indoorLevel->PVSList.Clear();

        for(i=0; i<indoorLevel->BrushList.Num(); i++)
            indoorLevel->BrushList[i].Clear();
        indoorLevel->BrushList.Clear();
    }
    else if(level->IsOf(GetClass(OutdoorLevel)))
    {
        OutdoorLevel *outdoorLevel = (OutdoorLevel*)level;

        for(i=0; i<outdoorLevel->BrushList.Num(); i++)
            outdoorLevel->BrushList[i].Clear();
        outdoorLevel->BrushList.Clear();
    }
    else if(level->IsOf(GetClass(OctLevel)))
    {
        OctLevel *octLevel = (OctLevel*)level;

        for(i=0; i<octLevel->BrushList.Num(); i++)
            delete octLevel->BrushList[i];
        octLevel->BrushList.Clear();
    }

    for(i=0; i<Light::NumLights(); i++)
    {
        Light *light = Light::GetLight(i);

        //if(!light->bLightmapped)
            light->UpdatePositionalData();
    }

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *editBrush = BrushList[i];
        editBrush->brushID = -1;
    }

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *editBrush = BrushList[i];
        if(editBrush->brushType == BrushType_Subtraction)
            editBrush->SubtractBrush(TRUE);
    }

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *editBrush = BrushList[i];
        if(editBrush->brushType == BrushType_Addition)
            editBrush->AddGeometry(TRUE);
    }

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *editBrush = BrushList[i];
        editBrush->UpdateLevelBrush();
    }

    UpdateViewports();

    editor->undoStack->Clear();
    editor->redoStack->Clear();

    traceOut;
}

void EditorLevelInfo::RebuildAdditions()
{
    traceIn(EditorLevelInfo::RebuildAdditions);

    level->ClearRenderItems();

    DWORD i;

    if(level->IsOf(GetClass(IndoorLevel)))
    {
        IndoorLevel *indoorLevel = (IndoorLevel*)level;

        for(i=0; i<indoorLevel->BrushList.Num(); i++)
            indoorLevel->BrushList[i].Clear();
        indoorLevel->BrushList.Clear();
    }
    else if(level->IsOf(GetClass(OutdoorLevel)))
    {
        OutdoorLevel *outdoorLevel = (OutdoorLevel*)level;

        for(i=0; i<outdoorLevel->BrushList.Num(); i++)
            outdoorLevel->BrushList[i].Clear();
        outdoorLevel->BrushList.Clear();
    }
    else if(level->IsOf(GetClass(OctLevel)))
    {
        OctLevel *octLevel = (OctLevel*)level;

        for(i=0; i<octLevel->BrushList.Num(); i++)
            delete octLevel->BrushList[i];
        octLevel->BrushList.Clear();
    }

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *editBrush = BrushList[i];

        if(editBrush->brushType == BrushType_Addition)
        {
            editBrush->brushID = -1;
            editBrush->AddGeometry(TRUE);
            editBrush->UpdateLevelBrush();
        }
    }

    UpdateViewports();

    editor->undoStack->Clear();
    editor->redoStack->Clear();

    traceOut;
}

void EditorLevelInfo::RebuildSubtractions()
{
    traceIn(EditorLevelInfo::RebuildSubtractions);

    level->ClearRenderItems();

    DWORD i;

    if(level->IsOf(GetClass(IndoorLevel)))
    {
        IndoorLevel *indoorLevel = (IndoorLevel*)level;

        for(i=0; i<indoorLevel->PVSList.Num(); i++)
            indoorLevel->PVSList[i].Clear();
        indoorLevel->PVSList.Clear();
    }
    else
        return;

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *editBrush = BrushList[i];
        if(editBrush->brushType == BrushType_Subtraction)
            editBrush->brushID = -1;
    }

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *editBrush = BrushList[i];
        if(editBrush->brushType == BrushType_Subtraction)
            editBrush->SubtractBrush(TRUE);
    }

    for(i=0; i<BrushList.Num(); i++)
    {
        EditorBrush *editBrush = BrushList[i];
        if(editBrush->brushType == BrushType_Subtraction)
            editBrush->UpdateLevelBrush();
    }

    UpdateViewports();

    editor->undoStack->Clear();
    editor->redoStack->Clear();

    traceOut;
}

#define BRUSHFILE_VER 0x100


void EditorLevelInfo::ImportBrush()
{
    traceIn(EditorLevelInfo::ImportBrush);

    TCHAR lpFile[256];
    OPENFILENAME ofn;
    zero(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndEditor;
    ofn.lpstrFile = lpFile;
    ofn.nMaxFile = 255;

    ofn.lpstrFile[0] = 0;
    ofn.lpstrFilter = TEXT("Brush Files\0*.xbr\0");
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = TEXT(".");
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    TCHAR curDirectory[256];
    GetCurrentDirectory(255, curDirectory);

    if(GetOpenFileName(&ofn))
    {
        SetCurrentDirectory(curDirectory);

        XFileInputSerializer fileData;

        if(fileData.Open(lpFile))
        {
            int i;

            DWORD ver;
            fileData << ver;

            if(ver == BRUSHFILE_VER)
            {
                if(newObject)
                    delete newObject;
                newObject = NULL;

                if(WorkBrush)
                    EditorLevelInfo::DestroyWorkbrush();

                List<BYTE> matIDs;

                WorkBrush = CreateObject(EditorBrush);
                fileData << WorkBrush->FaceList;
                Vect::SerializeList(fileData, WorkBrush->PointList);
                fileData << WorkBrush->UVList;
                fileData << WorkBrush->LightmapFaces;
                fileData << WorkBrush->LightmapUVs;
                fileData << matIDs;

                WorkBrush->RebuildFaceNormals();
                WorkBrush->RebuildNormals();

                DWORD numMaterials;
                fileData << numMaterials;

                List<String> MatNames;
                MatNames.SetSize(numMaterials);

                for(i=0; i<numMaterials; i++)
                {
                    String &str = MatNames[i];
                    fileData << str;
                }

                WorkBrush->ProcessBasicMeshData();

                WorkBrush->GenerateUniqueName(TEXT("Brush"));

                WorkBrush->RebuildBounds();

                for(i=0; i<matIDs.Num(); i++)
                {
                    String &matName = MatNames[matIDs[i]];
                    if(!matName.IsEmpty())
                        WorkBrush->Materials[i] = RM->GetMaterial(matName);
                }

                for(i=0; i<numMaterials; i++)
                    MatNames[i].Clear();
                MatNames.Clear();

                fileData.Close();
            }
            else
                AppWarning(TEXT("'%s': Invalid brush file version"), lpFile);
        }
    }

    traceOut;
}

void EditorLevelInfo::SetCurEditMode(EditMode newEditMode)
{
    traceIn(EditorLevelInfo::SetCurEditMode);

    if(levelInfo->curEditMode != EditMode_Modify)
    {
        MENUITEMINFO mii;
        zero(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);

        mii.fMask  = MIIM_STATE;
        mii.fState = MFS_UNCHECKED;

        HMENU hMainMenu = GetMenu(hwndEditor);
        HMENU hSceneMenu = GetSubMenu(hMainMenu, 2);

        SetMenuItemInfo(hSceneMenu, ID_SCENE_SELECT, FALSE, &mii);
        SetMenuItemInfo(hSceneMenu, ID_SCENE_MOVE, FALSE, &mii);
        SetMenuItemInfo(hSceneMenu, ID_SCENE_ROTATE, FALSE, &mii);
    }

    curEditMode = newEditMode;

    traceOut;
}

void EditorLevelInfo::SetCurModifyMode(ModifyMode newModifyMode)
{
    traceIn(EditorLevelInfo::SetCurModifyMode);

    if(newObject)
    {
        delete newObject;
        newObject = NULL;
    }
    curEditMode   = EditMode_Modify;
    curModifyMode = newModifyMode;

    if(SelectedObjects.Num())
    {
        if(curManipulator)
            DestroyObject(curManipulator);

        switch(newModifyMode)
        {
            case ModifyMode_Select:
                curManipulator = CreateObject(DefaultManipulator);
                break;

            case ModifyMode_Move:
                curManipulator = CreateObject(PositionManipulator);
                break;

            case ModifyMode_Rotate:
                curManipulator = CreateObject(RotationManipulator);
                break;
        }
        UpdateManipulatorPos();

        UpdateViewports();
    }

    //---------------------------------------------------

    HMENU hMainMenu = GetMenu(hwndEditor);
    HMENU hSceneMenu = GetSubMenu(hMainMenu, 2);

    DWORD buttonID;

    switch(newModifyMode)
    {
        case ModifyMode_Select:
            buttonID = ID_MODIFY_SELECT;
            break;

        case ModifyMode_Move:
            buttonID = ID_MODIFY_MOVE;
            break;

        case ModifyMode_Rotate:
            buttonID = ID_MODIFY_ROTATE;
            break;
    }

    MENUITEMINFO mii;
    zero(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);

    mii.fMask  = MIIM_STATE;

    mii.fState = (newModifyMode == ModifyMode_Select) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hSceneMenu, ID_SCENE_SELECT, FALSE, &mii);
    mii.fState = (newModifyMode == ModifyMode_Move) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hSceneMenu, ID_SCENE_MOVE, FALSE, &mii);
    mii.fState = (newModifyMode == ModifyMode_Rotate) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hSceneMenu, ID_SCENE_ROTATE, FALSE, &mii);

    //---------------------------------------------------

    SetRadioButtonCheck(hwndSidebar, buttonID);

    traceOut;
}

void EditorLevelInfo::DrawHemicube(const List<Brush*> &Brushes, const List<MeshEntity*> &StaticMeshes, const Vect &texelPos, DWORD pass, Effect *curEffect)
{
    traceInFast(EditorLevelInfo::DrawHemicube);

    if(!level->bLoaded)
        return;

    level->curEffect = curEffect;

    for(int i=0; i<Brushes.Num(); i++)
    {
        Brush *brush = Brushes[i];

        if(brush->lightmap.plain)
            curEffect->SetTexture(lightmapTexture, brush->lightmap.plain);

        LoadVertexBuffer(brush->VertBuffer);
        LoadIndexBuffer(brush->IdxBuffer);

        for(DWORD i=0;i<brush->nSections;i++)
        {
            DrawSection &section  = brush->SectionList[i];
            Material    *material = brush->Materials[i];

            if(!material) material = level->defaultMaterial;

            curEffect->SetTexture(lightmapDiffuse, material->GetCurrentTexture(TEXT("diffuseTexture")));
            curEffect->SetTexture(lightmapIllumTexture, material->GetCurrentTexture(TEXT("illuminationMap")));

            Color3 illum;
            illum.MakeFromRGB(material->GetColor(TEXT("illuminationColor")));
            illum *= 5.0f;
            curEffect->SetColor3(lightmapIllumColor, illum);

            GS->Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
        }
    }

    for(int i=0; i<StaticMeshes.Num(); i++)
    {
        MeshEntity *meshEnt = StaticMeshes[i];

        if(meshEnt->bUseLightmapping)
        {
            if(meshEnt->lightmap.plain)
                curEffect->SetTexture(lightmapTexture, meshEnt->lightmap.plain);

            curEffect->SetMatrix(lightmapWorldMatrix, meshEnt->GetInvTransform());

            MatrixPush();
            MatrixMultiply(meshEnt->GetInvTransform());

            LoadVertexBuffer(meshEnt->VertBuffer);
            LoadIndexBuffer(meshEnt->mesh->IdxBuffer);

            for(DWORD i=0;i<meshEnt->mesh->nSections;i++)
            {
                DrawSection &section  = meshEnt->mesh->SectionList[i];
                Material    *material = meshEnt->MaterialList[i];

                if(!material) continue;

                curEffect->SetTexture(lightmapDiffuse, material->GetCurrentTexture(TEXT("diffuseTexture")));
                curEffect->SetTexture(lightmapIllumTexture, material->GetCurrentTexture(TEXT("illuminationMap")));
                Color3 illum;
                illum.MakeFromRGB(material->GetColor(TEXT("illuminationColor")));
                illum *= 5.0f;
                curEffect->SetColor3(lightmapIllumColor, illum);

                GS->Draw(GS_TRIANGLES, 0, section.startFace*3, section.numFaces*3);
            }

            MatrixPop();

            curEffect->SetMatrixIdentity(lightmapWorldMatrix);
        }
    }

    //EnableBlending(TRUE);
    //DepthWriteEnable(FALSE);

    if(!pass)
    {
        for(int j=0; j<Light::NumLights(); j++)
        {
            Light *curLight = Light::GetLight(j);
            if(curLight->IsOf(GetClass(PointLight)))
            {
                PointLight *light = (PointLight*)curLight;

                if(light->bStaticLight && light->bEnableEmission)
                {
                    curEffect->SetTexture(lightmapTexture, whiteTex);
                    curEffect->SetTexture(lightmapDiffuse, whiteTex);
                    curEffect->SetTexture(lightmapIllumTexture, whiteTex);

                    float lightSize = light->lightVolume;
                    float dist = texelPos.Dist(light->GetWorldPos());

                    /*float attenuation;

                    attenuation = 1.0f-(dist/light->lightRange);
                    if(attenuation < 0.0f) //attenuation = 0.0f;
                        continue;

                    attenuation = pow(attenuation, 0.75f);

                    //dist -= lightSize*0.5f;
                    if(dist < 0.0f) dist = 0.0f;

                    float chong = (1.0f/(lightSize*lightSize));

                    float illumVal = dist*dist*1.62f*chong*attenuation;
                    //float illumVal = 40.0f*attenuation;*/

                    //-----------------
                    float illumVal = 150.0f*(1.0f/lightSize/lightSize);
                    //-----------------

                    Color3 lightColor;
                    lightColor.MakeFromRGB(light->color);

                    float fIntensity = float(light->intensity)*0.01f;

                    curEffect->SetFloat(lightmapMeshScale, lightSize);
                    curEffect->SetColor3(lightmapIllumColor, lightColor*illumVal*fIntensity);

                    Matrix m;
                    m.SetIdentity();
                    m.T = light->GetWorldPos();

                    curEffect->SetMatrix(lightmapWorldMatrix, m);

                    MatrixPush();
                    MatrixTranslate(light->GetWorldPos());

                    LoadIndexBuffer(sphereMesh->IdxBuffer);
                    LoadVertexBuffer(sphereMesh->VertBuffer);

                    Draw(GS_TRIANGLES);

                    LoadIndexBuffer(NULL);
                    LoadVertexBuffer(NULL);

                    MatrixPop();

                    curEffect->SetFloat(lightmapMeshScale, 1.0f);
                    curEffect->SetMatrixIdentity(lightmapWorldMatrix);
                    curEffect->SetColor3(lightmapIllumColor, 0.0f, 0.0f, 0.0f);
                }
            }
            else if(curLight->IsOf(GetClass(DirectionalLight)))
            {
                DirectionalLight *light = (DirectionalLight*)curLight;

                if(light->bStaticLight && light->bEnableEmission)
                {
                    curEffect->SetTexture(lightmapTexture, whiteTex);
                    curEffect->SetTexture(lightmapDiffuse, whiteTex);
                    curEffect->SetTexture(lightmapIllumTexture, whiteTex);

                    float lightSize = light->lightVolume;

                    float chong = (1.0f/(lightSize*lightSize));

                    float illumVal = 6.0f*chong;
                    //float illumVal = 40.0f*attenuation;

                    Color3 lightColor;
                    lightColor.MakeFromRGB(light->color);

                    float fIntensity = float(light->intensity);

                    curEffect->SetFloat(lightmapMeshScale, lightSize*5.0f);
                    curEffect->SetColor3(lightmapIllumColor, lightColor*fIntensity*illumVal);

                    Matrix m;
                    m.SetIdentity();
                    m.T = texelPos + (light->GetWorldRot().GetDirectionVector()*100.0f);

                    curEffect->SetMatrix(lightmapWorldMatrix, m);

                    MatrixPush();
                    MatrixTranslate(m.T);

                    LoadIndexBuffer(sphereMesh->IdxBuffer);
                    LoadVertexBuffer(sphereMesh->VertBuffer);

                    Draw(GS_TRIANGLES);

                    LoadIndexBuffer(NULL);
                    LoadVertexBuffer(NULL);

                    MatrixPop();

                    curEffect->SetFloat(lightmapMeshScale, 1.0f);
                    curEffect->SetMatrixIdentity(lightmapWorldMatrix);
                    curEffect->SetColor3(lightmapIllumColor, 0.0f, 0.0f, 0.0f);
                }
            }
        }
    }

    traceOutFast;
}

void EditorLevelInfo::SetCurSelectMode(SelectMode newSelectMode)
{
    traceIn(EditorLevelInfo::SetCurSelectMode);

    if(newSelectMode == curSelectMode)
        return;

    curSelectMode = newSelectMode;

    //---------------------------------------------------

    Deselect();
    for(int i=0; i<BrushList.Num(); i++)
    {
        if(BrushList[i]->SelectedPolys.Num())
        {
            BrushList[i]->SelectedPolys.Clear();
            BrushList[i]->RebuildSelectionIndices();
        }
    }

    //---------------------------------------------------

    HMENU hMainMenu = GetMenu(hwndEditor);
    HMENU hSceneMenu = GetSubMenu(hMainMenu, 2);

    MENUITEMINFO mii;
    zero(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);

    mii.fMask  = MIIM_STATE;

    mii.fState = (newSelectMode == SelectMode_ObjectsAndDetails) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hSceneMenu, ID_SCENE_SELECTOBJECTS, FALSE, &mii);
    mii.fState = (newSelectMode == SelectMode_WorldPrefabs) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hSceneMenu, ID_SCENE_SELECTPREFABS, FALSE, &mii);
    mii.fState = (newSelectMode == SelectMode_Brushes) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hSceneMenu, ID_SCENE_SELECTBRUSHES, FALSE, &mii);
    mii.fState = (newSelectMode == SelectMode_Textures) ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfo(hSceneMenu, ID_SCENE_SELECTTEXTURES, FALSE, &mii);

    //---------------------------------------------------

    DWORD buttonID;

    switch(newSelectMode)
    {
        case SelectMode_ObjectsAndDetails:
            buttonID = ID_SELECTMODE_OBJECTS;
            break;

        case SelectMode_WorldPrefabs:
            buttonID = ID_SELECTMODE_WORLD;
            break;

        case SelectMode_Brushes:
            buttonID = ID_SELECTMODE_BRUSHES;
            break;

        case SelectMode_Textures:
            buttonID = ID_SELECTMODE_TEXTURES;
            break;
    }

    SetRadioButtonCheck(hwndSidebar, buttonID);

    UpdateViewports();

    traceOut;
}

void EditorLevelInfo::SetWorldPos(Entity *ent, const Vect &newPos)
{
    ent->Pos = ent->worldPos = newPos;
}

void EditorLevelInfo::SetWorldRot(Entity *ent, const Quat &newRot)
{
    ent->Rot = ent->worldRot = newRot;
}



void ENGINEAPI EditorLevelInfo::UndoRedoDelete(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoDelete);

    if(bUndo)
    {
        DWORD numObjects;

        s    << numObjects;
        sOut << numObjects;

        for(int i=0; i<numObjects; i++)
        {
            String className;
            s << className;

            Entity *ent = (Entity*)CreateFactoryObject(className, FALSE);

            ent->Serialize(s);
            ent->InitializeObject();

            levelInfo->Select(ent);

            sOut << ent->name;
        }
    }
    else
    {
        levelInfo->Deselect();

        DWORD numObjects;

        s    << numObjects;
        sOut << numObjects;

        for(int i=0; i<numObjects; i++)
        {
            String entName;
            s << entName;

            Entity *ent = Entity::FindByName(entName);

            levelInfo->Select(ent);

            String className = ent->GetObjectClass()->GetName();
            sOut << className;
            ent->Serialize(sOut);
        }

        levelInfo->DeleteSelectedObjects(TRUE);
    }

    if(objectProperties)
        objectProperties->UpdateProperties();

    traceOut;
}


void ENGINEAPI EditorLevelInfo::UndoRedoSelectPolys(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoSelectPolys);

    DWORD numBrushes = 0;
    int i;

    for(i=0; i<levelInfo->BrushList.Num(); i++)
    {
        if(levelInfo->BrushList[i]->SelectedPolys.Num())
            ++numBrushes;
    }

    sOut << numBrushes;

    for(i=0; i<levelInfo->BrushList.Num(); i++)
    {
        EditorBrush *brush = levelInfo->BrushList[i];
        if(brush->SelectedPolys.Num())
        {
            String brushName = brush->name;
            sOut << brushName;
            sOut << brush->SelectedPolys;
        }

        brush->SelectedPolys.Clear();
    }

    //---------------------------------------------

    s << numBrushes;

    for(i=0; i<numBrushes; i++)
    {
        String strBrush;
        s << strBrush;

        EditorBrush *brush = (EditorBrush*)Entity::FindByName(strBrush);

        s << brush->SelectedPolys;
    }

    //---------------------------------------------

    for(i=0; i<levelInfo->BrushList.Num(); i++)
    {
        EditorBrush *brush = levelInfo->BrushList[i];
        brush->RebuildSelectionIndices();
    }

    traceOut;
}


void ENGINEAPI EditorLevelInfo::UndoRedoSelectObjects(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoSelectObjects);

    int i;

    //-------------------------

    DWORD numSelected = levelInfo->SelectedObjects.Num();
    sOut << numSelected;

    for(i=0; i<numSelected; i++)
    {
        String strName = levelInfo->SelectedObjects[i]->name;
        sOut << strName;
    }

    //-------------------------

    levelInfo->Deselect();

    s << numSelected;

    for(i=0; i<numSelected; i++)
    {
        String strName;
        s << strName;

        Entity *ent = Entity::FindByName(strName);

        levelInfo->Select(ent);
    }

    if(objectProperties)
        objectProperties->UpdateProperties();

    traceOut;
}

void ENGINEAPI EditorLevelInfo::UndoRedoCreateObject(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoCreateObject);

    if(bUndo)
    {
        String strName;
        s << strName;

        Entity *ent = Entity::FindByName(strName);

        strName = ent->GetObjectClass()->GetName();
        sOut << strName;

        ent->Serialize(sOut);

        DestroyObject(ent);
    }
    else
    {
        String strName;
        s << strName;

        Entity *ent = (Entity*)CreateFactoryObject(strName, FALSE);
        ent->Serialize(s);
        ent->InitializeObject();

        strName = ent->name;
        sOut << strName;
    }

    traceOut;
}

void ENGINEAPI EditorLevelInfo::UndoRedoChangeSurfaceMaterials(Serializer &s, Serializer &sOut, BOOL bUndo)
{
    traceIn(EditorLevelInfo::UndoRedoChangeSurfaceMaterials);

    int i, j;

    String strReplacedMaterial;

    s    << strReplacedMaterial;
    sOut << strReplacedMaterial;

    if(bUndo)
    {
        DWORD numBrushes;
        s << numBrushes;

        for(i=0; i<numBrushes; i++)
        {
            String strBrushName;
            s << strBrushName;

            EditorBrush *brush = (EditorBrush*)Entity::FindByName(strBrushName);
            Brush *levelBrush = brush->GetLevelBrush();

            for(j=0; j<brush->Materials.Num(); j++)
            {
                String strMaterialName;
                s << strMaterialName;

                if(brush->Materials[j])
                {
                    RM->ReleaseMaterial(brush->Materials[j]);
                    RM->ReleaseMaterial(levelBrush->Materials[j]);
                }

                if(strMaterialName.IsEmpty())
                    brush->Materials[j] = levelBrush->Materials[j] = NULL;
                else
                {
                    brush->Materials[j] = RM->GetMaterial(strMaterialName);
                    levelBrush->Materials[j] = RM->GetMaterial(strMaterialName);
                }
            }
        }
    }
    else
    {
        DWORD numModifiedBrushes = 0;

        for(i=0; i<levelInfo->BrushList.Num(); i++)
        {
            EditorBrush *brush = levelInfo->BrushList[i];
            if(brush->SelectedPolys.Num())
                ++numModifiedBrushes;
        }

        if(numModifiedBrushes)
        {
            sOut << numModifiedBrushes;

            for(i=0; i<levelInfo->BrushList.Num(); i++)
            {
                EditorBrush *brush = levelInfo->BrushList[i];

                if(brush->SelectedPolys.Num())
                {
                    sOut << brush->name;

                    for(j=0; j<brush->Materials.Num(); j++)
                        sOut << RM->GetMaterialName(brush->Materials[j]);
                }
            }
        }

        levelInfo->SetPolyMaterials(strReplacedMaterial, FALSE);
    }

    traceOut;
}

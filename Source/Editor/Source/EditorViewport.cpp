/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  EditorViewport.cpp

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

DefineClass(EditorViewport);



void EditorViewport::Init()
{
    traceIn(EditorViewport::Init);

    SetSystem(GS);

    traceOut;
}

void EditorViewport::Destroy()
{
    traceIn(EditorViewport::Destroy);

    DestroyObject(camera);
    Super::Destroy();

    traceOut;
}

void EditorViewport::SetViewportType(ViewportType newType)
{
    traceIn(EditorViewport::SetViewportType);

    if(newType == type)
        return;

    bool bResetSound = false;

    if(type == ViewportType_Main)
        bResetSound = true;

    if(camera)
        DestroyObject(camera);

    type = newType;

    camera = CreateObject(Camera);
    camera->UpdatePositionalData();

    if(type == ViewportType_Main)
    {
        zoom = 1.0f;//tan(RAD(90.0f)/2); //view radius

        if(level && level->IsOf(GetClass(OutdoorLevel)))
        {
            rotX = -65.0f;
            rotY = 0.0f;

            camera->SetPos(Vect(-5.0f, 200.0f, 60.0f));
            SetWorldRot(camera, Quat().MakeFromDirection(-camera->GetLocalPos().GetNorm()));
            //todo: had GetWorldRot() manually set here
            //camera->GetWorldRot() = camera->Rot.MakeFromDirection(-camera->Pos.GetNorm());
            camera->SetSoundCamera(FALSE);
        }
        else if(level && level->IsOf(GetClass(OctLevel)))
        {
            camera->SetPos(Vect(-5.0f, 30.0f, 20.0f));
            Vect camNorm = -camera->GetLocalPos().GetNorm();

            //todo: had it manually set here
            //camera->GetWorldRot() = camera->Rot.SetLookDirection(camNorm);
            SetWorldRot(camera, Quat().SetLookDirection(camNorm));

            if(CloseFloat(fabsf(camNorm.y), 1.0f))
            {
                rotX = 90*camNorm.y;
                rotY = 0.0f;
            }
            else
            {
                Vect flattenedVect = Vect(camNorm.x, 0.0f, camNorm.z).Norm();
                rotY = DEG(acosf(Vect(0.0f, 0.0f, -1.0f).Dot(flattenedVect)));
                if(camNorm.x > 0.0f)
                    rotY = -rotY;
                rotX = DEG(acosf(flattenedVect.Dot(camNorm)));
                if(camNorm.y < 0.0f)
                    rotX = -rotX;
            }

            camera->SetSoundCamera(FALSE);
        }
        else
        {
            rotX = rotY = 0.0f;

            SetWorldPos(camera, Vect(-5.0f, 3.0f, 60.0f));
            SetWorldRot(camera, Quat::Identity());
            //todo - manually assigned here
            //camera->GetWorldRot() = camera->Rot.SetIdentity();
            camera->SetSoundCamera(FALSE);
        }

        typeName = TEXT("Main");

        drawType = ViewportDrawType_TexturedOnly;
        bShowGrid = true;
    }
    else
    {
        drawType = ViewportDrawType_Brushes;
        bShowGrid = true;

        zoom = 50.0f;
        //todo: manually set
        //camera->GetWorldRot().SetIdentity();
        AxisAngle newRot;
        newRot.Clear();
        camera->SetSoundCamera(FALSE);
        switch(type)
        {
            case ViewportType_Bottom:
                newRot.Set(1.0f, 0.0f, 0.0f, RAD(90.0f));
                typeName = TEXT("Bottom");
                break;
            case ViewportType_Top:
                newRot.Set(1.0f, 0.0f, 0.0f, RAD(-90.0f));
                typeName = TEXT("Top");
                break;
            case ViewportType_Right:
                newRot.Set(0.0f, 1.0f, 0.0f, RAD(90.0f));
                typeName = TEXT("Right");
                break;
            case ViewportType_Left:
                newRot.Set(0.0f, 1.0f, 0.0f, RAD(-90.0f));
                typeName = TEXT("Left");
                break;
            case ViewportType_Back:
                newRot.Set(0.0f, 1.0f, 0.0f, RAD(-180.0f));
                typeName = TEXT("Back");
                break;
            case ViewportType_Front:
                typeName = TEXT("Front");
                break;
        }

        SetWorldRot(camera, newRot.GetQuat());
    }

    SetCamera(camera);

    if(bResetSound)
        editor->ResetSound();

    traceOut;
}


void EditorViewport::MouseMove(int x, int y, short x_offset, short y_offset)
{
    traceIn(EditorViewport::MouseMove);

    mouseOrig.x = 2.0f*(((float(x)-Pos.x)/Size.x)-0.5f)*camera->Right();
    mouseOrig.y = 2.0f*(((float(y)-Pos.y)/Size.y)-0.5f)*camera->Top();
    mouseOrig.z = -camera->Near();

    Matrix camMatrix;
    camMatrix.SetIdentity();
    camMatrix *= camera->GetWorldPos();
    camMatrix *= camera->GetWorldRot();

    float fT;

    mouseOrig.TransformPoint(camMatrix);

    if(type == ViewportType_Main)
        mouseDir = (mouseOrig - camera->GetWorldPos()).Norm();
    else
        mouseDir = -camera->GetWorldRot().GetDirectionVector();

    if(type != ViewportType_Main)
    {
        Plane axisPlane;
        axisPlane.Dist = 0.0f;

        switch(type)
        {
            case ViewportType_Left:
            case ViewportType_Right:
                axisPlane.Dir.Set(1.0f, 0.0f, 0.0f);
                break;

            case ViewportType_Top:
            case ViewportType_Bottom:
                axisPlane.Dir.Set(0.0f, 1.0f, 0.0f);
                break;

            case ViewportType_Front:
            case ViewportType_Back:
                axisPlane.Dir.Set(0.0f, 0.0f, 1.0f);
                break;
        }

        axisPlane.GetRayIntersection(mouseOrig, mouseDir, fT);
    }
    else
    {
        Plane bestPlane(0.0f, 1.0f, 0.0f, levelInfo->curYPlanePosition);

        if(!CloseFloat(mouseDir.Dot(bestPlane.Dir), 1.57f, 0.2) && bestPlane.GetRayIntersection(mouseOrig, mouseDir, fT))
            mouseWorldPos = mouseOrig+(mouseDir*fT);
    }

    mouseWorldPos = mouseOrig+(mouseDir*fT);

    if(bSelecting)
    {
        selEndX = x;
        selEndY = y;
    }
    else
    {
        editor->SnapPoint(mouseWorldPos);

        if(!bKeepFocus || levelInfo->curEditMode == EditMode_Create)
        {
            TCHAR position[256];
            tsprintf_s(position, 255, TEXT("(%f, %f, %f)    "), mouseWorldPos.x, mouseWorldPos.y, mouseWorldPos.z);
            editor->SetStatusText(1, position);

            if(levelInfo->newObject)
                levelInfo->newObject->SetStatusString();
            else
                editor->SetStatusText(0, NULL);
        }
        else
            editor->SetStatusText(1, NULL);

        if(levelInfo->curEditMode == EditMode_Create)
            levelInfo->newObject->MouseMove(this, mouseWorldPos);

        if(bKeepFocus)
        {
            if( levelInfo->curEditMode == EditMode_Modify   ||
                bWasDragging                                ||
                ((levelInfo->curEditMode == EditMode_Create) && levelInfo->newObject->CanIgnoreViewport(type)) )
            {
                if(bLeftMouseDown && levelInfo && levelInfo->curManipulator && levelInfo->curManipulator->Manipulating())
                    levelInfo->curManipulator->Manipulate(mouseOrig, mouseDir);
                else
                    ProcessMovement(x_offset, y_offset);
            }
        }
        else
        {
            Font *font = GS->GetFont(TEXT("Base:Arial Medium.xft"));

            int width = font->TextWidth(typeName);
            int height = font->GetFontHeight();

            bInsideName = ((x > Pos.x) && (x < (Pos.x+width ))) &&
                          ((y > Pos.y) && (y < (Pos.y+height)));

            if(levelInfo->curEditMode == EditMode_Modify)
            {
                if(levelInfo && levelInfo->curManipulator)
                {
                    float adjustSize;

                    if(camera->IsPerspective())
                        adjustSize = (camera->GetWorldPos().Dist(levelInfo->curManipulator->GetWorldPos())*zoom)/50.0f;
                    else
                        adjustSize = zoom/50.0f;

                    Vect cameraDir;

                    if(camera->IsPerspective())
                        cameraDir = (levelInfo->curManipulator->GetWorldPos()-camera->GetWorldPos()).Norm();
                    else
                        cameraDir = -camera->GetLocalRot().GetDirectionVector();

                    levelInfo->curManipulator->ProcessMouseRay(cameraDir, adjustSize, mouseOrig, mouseDir);
                }
            }
        }
    }

    UpdateViewports();

    traceOut;
}

void EditorViewport::MouseDown(DWORD button)
{
    traceIn(EditorViewport::MouseDown);

    if(GetFocus() != hwndMain)
        SetFocus(hwndMain);

    if(!bInsideName)
    {
        switch(button)
        {
            case MOUSE_LEFTBUTTON:
                bLeftMouseDown = true;
                break;

            case MOUSE_RIGHTBUTTON:
                bRightMouseDown = true;
                break;

            case MOUSE_MIDDLEBUTTON:
                bMiddleMouseDown = true;
                break;
        };

        BOOL bManpilating = (levelInfo && levelInfo->curManipulator && levelInfo->curManipulator->Manipulating());

        if(button == MOUSE_LEFTBUTTON && levelInfo->curEditMode == EditMode_Modify && GS->GetInput()->GetButtonState(KBC_SHIFT) && !bManpilating)
        {
            bSelecting = true;
            bKeepFocus = TRUE;

            GS->GetLocalMousePos(selStartX, selStartY);
            selStartX -= int(Pos.x);
            selStartY -= int(Pos.y);

            selEndX = selStartX;
            selEndY = selStartY;
        }
        else if( levelInfo->curEditMode == EditMode_Modify   ||
                 bWasDragging                                ||
                 (
                   levelInfo->curEditMode == EditMode_Create &&
                   (levelInfo->newObject->CanIgnoreViewport(type) || GS->GetInput()->GetButtonState(KBC_CONTROL))
                 )
               )
        {
            if(!bKeepFocus && (bLeftMouseDown || bRightMouseDown))
            {
                bKeepFocus = TRUE;
                bMovedMouse = false;
                bWasDragging = true;

                if(levelInfo->newObject)
                    levelInfo->newObject->SetEnabled(FALSE);
            }

            if(levelInfo && levelInfo->curManipulator)
                levelInfo->curManipulator->bBeginManipulation = bLeftMouseDown;
        }
        else if(levelInfo->curEditMode == EditMode_Create)
        {
            if(!levelInfo->newObject->MouseDown(button, mouseWorldPos))
                bKeepFocus = FALSE;

            else if(bLeftMouseDown)
            {
                if(levelInfo->newObject->Create(this, mouseWorldPos))
                    bKeepFocus = TRUE;
            }
        }
    }

    UpdateViewports();

    traceOut;
}

void EditorViewport::MouseUp(DWORD button)
{
    traceIn(EditorViewport::MouseUp);

    if(!bInsideName)
    {
        switch(button)
        {
            case MOUSE_LEFTBUTTON:
                bLeftMouseDown = false;
                break;

            case MOUSE_RIGHTBUTTON:
                bRightMouseDown = false;
                break;

            case MOUSE_MIDDLEBUTTON:
                bMiddleMouseDown = false;
                break;
        };
    }

    if(bKeepFocus)
    {
        if(button == MOUSE_LEFTBUTTON && bSelecting)
        {
            bSelecting = false;
            bKeepFocus = FALSE;

            ProcessMultiSelection();
        }
        else if( levelInfo->curEditMode == EditMode_Modify  ||
                 bWasDragging                               ||
                 (
                   type == ViewportType_Main &&
                   levelInfo->curEditMode == EditMode_Create &&
                   levelInfo->newObject->CanIgnoreViewport(type)
                 )
               )
        {
            if(!bLeftMouseDown && !bRightMouseDown)
            {
                bWasDragging = false;
                bKeepFocus = FALSE;

                if(levelInfo->newObject)
                    levelInfo->newObject->SetEnabled(TRUE);

                if(bMovedMouse)
                {
                    DWORD displayCount = ShowCursor(TRUE);
                    while(displayCount > 0)
                        displayCount = ShowCursor(FALSE);

                    OSClipCursor(FALSE);

                    GetSystem()->SetLocalMousePos(lastMouseX, lastMouseY);
                }
                else if(levelInfo)
                {
                    if((button == MOUSE_LEFTBUTTON) && levelInfo->curManipulator && levelInfo->curManipulator->Manipulating())
                        return;

                    //check to see if we've clicked on something.
                    BOOL bSelectBrushes = (drawType == ViewportDrawType_Brushes);

                    if(button == MOUSE_LEFTBUTTON)
                        levelInfo->ProcessSelection(mouseOrig, mouseDir, bSelectBrushes);
                    else if(button == MOUSE_RIGHTBUTTON)
                        levelInfo->RightClick(mouseOrig, mouseDir, bSelectBrushes);
                }
            }
        }
        else if(levelInfo->curEditMode == EditMode_Create)
        {
            if(!levelInfo->newObject->MouseUp(button, mouseWorldPos))
                bKeepFocus = FALSE;
        }
    }
    else
    {
        if(bInsideName && (button == MOUSE_RIGHTBUTTON))
        {
            HMENU hmenu = LoadMenu(hinstMain, MAKEINTRESOURCE(IDR_POPUPS));
            HMENU hmenuPopup = GetSubMenu(hmenu, 0);

            MENUITEMINFO mii;
            zero(&mii, sizeof(MENUITEMINFO));
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STATE;

            //---------------------------------------------

            mii.fState = (drawType == ViewportDrawType_Wireframe) ? MFS_CHECKED : 0;
            SetMenuItemInfo(hmenu, ID_VIEWPORT_WIREFRAME, FALSE, &mii);

            //---------------------------------------------

            mii.fState = (drawType == ViewportDrawType_Brushes) ? MFS_CHECKED : 0;
            SetMenuItemInfo(hmenu, ID_VIEWPORT_BRUSHES, FALSE, &mii);

            //---------------------------------------------

            if(type == ViewportType_Main)
                mii.fState = (drawType == ViewportDrawType_FullRender) ? MFS_CHECKED : 0;
            else
                mii.fState = MFS_DISABLED;

            SetMenuItemInfo(hmenu, ID_VIEWPORT_FULLRENDERING, FALSE, &mii);

            //---------------------------------------------

            if(type == ViewportType_Main)
                mii.fState = (drawType == ViewportDrawType_TexturedOnly) ? MFS_CHECKED : 0;
            else
                mii.fState = MFS_DISABLED;

            SetMenuItemInfo(hmenu, ID_VIEWPORT_TEXTUREDONLY, FALSE, &mii);

            //---------------------------------------------

            if(type == ViewportType_Main)
                mii.fState = camera->IsSoundCamera() ? MFS_CHECKED : 0;
            else
                mii.fState = MFS_DISABLED;

            SetMenuItemInfo(hmenu, ID_VIEWPORT_SOUND, FALSE, &mii);

            //---------------------------------------------

            mii.fState = bShowGrid ? MFS_CHECKED : 0;
            SetMenuItemInfo(hmenu, ID_VIEWPORT_SHOWGRID, FALSE, &mii);

            //---------------------------------------------

            POINT p;
            GetCursorPos(&p);
            switch(TrackPopupMenuEx(hmenuPopup, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, hwndEditor, NULL))
            {
                case ID_VIEWPORT_FRONT:
                    SetViewportType(ViewportType_Front);
                    break;

                case ID_VIEWPORT_BACK:
                    SetViewportType(ViewportType_Back);
                    break;

                case ID_VIEWPORT_LEFT:
                    SetViewportType(ViewportType_Left);
                    break;

                case ID_VIEWPORT_RIGHT:
                    SetViewportType(ViewportType_Right);
                    break;

                case ID_VIEWPORT_TOP:
                    SetViewportType(ViewportType_Top);
                    break;

                case ID_VIEWPORT_BOTTOM:
                    SetViewportType(ViewportType_Bottom);
                    break;

                case ID_VIEWPORT_MAIN:
                    SetViewportType(ViewportType_Main);
                    break;

                case ID_VIEWPORT_WIREFRAME:
                    drawType = ViewportDrawType_Wireframe;
                    break;

                case ID_VIEWPORT_BRUSHES:
                    drawType = ViewportDrawType_Brushes;
                    break;

                case ID_VIEWPORT_TEXTUREDONLY:
                    drawType = ViewportDrawType_TexturedOnly;
                    break;

                case ID_VIEWPORT_FULLRENDERING:
                    drawType = ViewportDrawType_FullRender;
                    break;

                case ID_VIEWPORT_CONFIGUREVIEWPORTS:
                    editor->ConfigureViewports();
                    break;

                case ID_VIEWPORT_SHOWGRID:
                    bShowGrid = !bShowGrid;
                    break;

                case ID_VIEWPORT_SOUND:
                    {
                        BOOL bLastValue = camera->IsSoundCamera();
                        for(DWORD i=0; i<editor->Viewports.Num(); i++)
                            editor->Viewports[i]->camera->SetSoundCamera(FALSE);

                        camera->SetSoundCamera(!bLastValue);

                        editor->ResetSound();
                        break;
                    }
            }

            DestroyMenu(hmenu);
        }
    }

    UpdateViewports();

    traceOut;
}


void EditorViewport::MouseWheel(short scroll)
{
    traceIn(EditorViewport::MouseWheel);

    if(type != ViewportType_Main)
    {
        zoom += float(-scroll)/20.0f;

        if(zoom < 10.0f)
            zoom = 10.0f;
        else if(zoom > 1000.0f)
            zoom = 1000.0f;

        UpdateViewports();
    }

    traceOut;
}

inline bool TrueViewclipBoundsIntersection(const ViewClip &clip, const Bounds &bounds)
{
    DWORD test;
    if((test = clip.BoundsTest(bounds)) & BOUNDS_OUTSIDE)
        return false;
    else if(test == BOUNDS_INSIDE)
        return true;
    else
    {
        Vect p1[12], p2[12];
        float fT;

        //ultra slow
        p1[ 0] = bounds.GetPoint(0);            p2[ 0] = bounds.GetPoint(MAX_X);
        p1[ 1] = bounds.GetPoint(MAX_Y);        p2[ 1] = bounds.GetPoint(MAX_X|MAX_Y);
        p1[ 2] = bounds.GetPoint(MAX_Z);        p2[ 2] = bounds.GetPoint(MAX_X|MAX_Z);
        p1[ 3] = bounds.GetPoint(MAX_Y|MAX_Z);  p2[ 3] = bounds.GetPoint(MAX_X|MAX_Y|MAX_Z);

        p1[ 4] = bounds.GetPoint(0);            p2[ 4] = bounds.GetPoint(MAX_Y);
        p1[ 5] = bounds.GetPoint(MAX_X);        p2[ 5] = bounds.GetPoint(MAX_Y|MAX_X);
        p1[ 6] = bounds.GetPoint(MAX_Z);        p2[ 6] = bounds.GetPoint(MAX_Y|MAX_Z);
        p1[ 7] = bounds.GetPoint(MAX_X|MAX_Z);  p2[ 7] = bounds.GetPoint(MAX_Y|MAX_X|MAX_Z);

        p1[ 8] = bounds.GetPoint(0);            p2[ 8] = bounds.GetPoint(MAX_Z);
        p1[ 9] = bounds.GetPoint(MAX_X);        p2[ 9] = bounds.GetPoint(MAX_Z|MAX_X);
        p1[10] = bounds.GetPoint(MAX_Y);        p2[10] = bounds.GetPoint(MAX_Z|MAX_Y);
        p1[11] = bounds.GetPoint(MAX_X|MAX_Y);  p2[11] = bounds.GetPoint(MAX_Z|MAX_X|MAX_Y);

        for(int i=0; i<clip.planes.Num(); i++)
        {
            const Plane &plane = clip.planes[i];

            for(int j=0; j<12; j++)
            {
                if(plane.GetIntersection(p1[j], p2[j], fT))
                {
                    Vect p = Lerp<Vect>(p1[j], p2[j], fT);

                    if(clip.PointWithin(p, EPSILON))
                        return true;
                }
            }
        }

        return false;
    }
}

void EditorViewport::ProcessMultiSelection()
{
    traceIn(EditorViewport::ProcessMultiSelection);

    int startX = MIN(selStartX, selEndX),
        startY = MIN(selStartY, selEndY),
        endX = MAX(selStartX, selEndX),
        endY = MAX(selStartY, selEndY);

    if(startX == endX || startY == endY)
        return;

    Vect ul, ur, ll, lr;
    Vect ulDir, urDir, llDir, lrDir;

    GetMouseRay(startX,  startY, ul, ulDir);
    GetMouseRay(endX,    startY, ur, urDir);
    GetMouseRay(startX,  endY,   ll, llDir);
    GetMouseRay(endX,    endY,   lr, lrDir);

    Vect camPos = GetCamera()->GetWorldPos();
    Vect camDir = GetCamera()->GetWorldRot().GetDirectionVector();

    ViewClip clip;
    clip.planes << Plane(camDir, camDir.Dot(camPos));
    clip.planes << Plane(camPos, ul, ll);
    clip.planes << Plane(camPos, ur, ul);
    clip.planes << Plane(camPos, lr, ur);
    clip.planes << Plane(camPos, ll, lr);

    BOOL bAdd = GS->GetInput()->GetButtonState(KBC_CONTROL);
    BOOL bSubtract = GS->GetInput()->GetButtonState(KBC_ALT);

    List<Entity*> TempSelectionList;

    if(levelInfo->curSelectMode == SelectMode_Brushes || levelInfo->curSelectMode == SelectMode_Textures)
    {
        for(int i=0; i<levelInfo->BrushList.Num(); i++)
        {
            EditorBrush *brush = levelInfo->BrushList[i];

            if(TrueViewclipBoundsIntersection(clip, brush->mesh.bounds))
            {
                if(levelInfo->curSelectMode == SelectMode_Brushes)
                    TempSelectionList << brush;
                else
                {
                    for(int j=0; j<brush->mesh.FaceList.Num(); j++)
                    {
                        Face &face = brush->mesh.FaceList[j];

                        Vect &faceNorm = brush->mesh.PlaneList[brush->mesh.FacePlaneList[j]].Dir;

                        if(faceNorm.Dot(camDir) < EPSILON)
                            continue;

                        BOOL bWithin = FALSE;

                        if( clip.PointWithin(brush->mesh.VertList[face.A])  ||
                            clip.PointWithin(brush->mesh.VertList[face.B])  ||
                            clip.PointWithin(brush->mesh.VertList[face.C])  ||
                            brush->mesh.RayTriangleTest(j, ul, ulDir)       ||
                            brush->mesh.RayTriangleTest(j, ur, urDir)       ||
                            brush->mesh.RayTriangleTest(j, ll, llDir)       ||
                            brush->mesh.RayTriangleTest(j, lr, lrDir)       )
                        {
                            bWithin = TRUE;
                        }
                        else
                        {
                            Vect points[3] = {brush->mesh.VertList[face.A],
                                              brush->mesh.VertList[face.B],
                                              brush->mesh.VertList[face.C]};

                            for(int k=0; k<clip.planes.Num(); k++)
                            {
                                Plane &plane = clip.planes[k];

                                for(int l=0; l<3; l++)
                                {
                                    int a = l;
                                    int b = (l == 2) ? 0 : a+1;

                                    float fT;
                                    if(plane.GetIntersection(points[a], points[b], fT))
                                    {
                                        Vect p = Lerp<Vect>(points[a], points[b], fT);
                                        if(clip.PointWithin(p, EPSILON))
                                        {
                                            bWithin = TRUE;
                                            break;
                                        }
                                    }
                                }

                                if(bWithin)
                                    break;
                            }
                        }

                        if(bWithin)
                            brush->TempSelectedPolys.SafeAdd(brush->mesh.FacePolyList[j]);
                    }
                }
            }
        }
    }
    else
    {
        if(level->IsOf(GetClass(OctLevel)))
        {
            OctLevel *octLevel = (OctLevel*)level;

            List<LevelObject*> leaves;
            octLevel->GetObjects(camPos, clip, leaves);

            for(int i=0; i<leaves.Num(); i++)
            {
                LevelObject *leaf = leaves[i];

                if(leaf->type == ObjectType_Entity)
                {
                    Entity *ent = leaf->ent;

                    if(!ent->GetEditType())
                        continue;

                    if( ((ent->GetEditType() == TYPE_PREFAB) && (levelInfo->curSelectMode != SelectMode_WorldPrefabs))       ||
                        ((ent->GetEditType() == TYPE_OBJECT) && (levelInfo->curSelectMode != SelectMode_ObjectsAndDetails))  )
                    {
                        continue;
                    }

                    if(TrueViewclipBoundsIntersection(clip.GetTransform(ent->GetEntityInvTransform()), ent->GetBounds()))
                        TempSelectionList.SafeAdd(ent);
                }
            }
        }
    }

    BOOL bChanged = FALSE;

    if(levelInfo->curSelectMode == SelectMode_Textures)
    {
        for(int i=0; i<levelInfo->BrushList.Num(); i++)
        {
            EditorBrush *brush = levelInfo->BrushList[i];

            if(brush->TempSelectedPolys.Num() != brush->SelectedPolys.Num())
            {
                bChanged = TRUE;
                break;
            }
            else
            {
                for(int j=0; j<brush->SelectedPolys.Num(); j++)
                {
                    if(!brush->TempSelectedPolys.HasValue(brush->SelectedPolys[j]))
                    {
                        bChanged = TRUE;
                        break;
                    }
                }
            }
        }

        if(bChanged)
        {
            levelInfo->SavePolySelectionUndoData();

            for(int i=0; i<levelInfo->BrushList.Num(); i++)
            {
                EditorBrush *brush = levelInfo->BrushList[i];

                if(bAdd)
                {
                    brush->SelectedPolys.AppendList(brush->TempSelectedPolys);
                    brush->SelectedPolys.RemoveDupes();
                }
                else if(bSubtract)
                {
                    for(int j=0; j<brush->TempSelectedPolys.Num(); j++)
                        brush->SelectedPolys.RemoveItem(brush->TempSelectedPolys[j]);
                }
                else
                    brush->SelectedPolys.CopyList(brush->TempSelectedPolys);

                brush->RebuildSelectionIndices();
                brush->TempSelectedPolys.Clear();
            }
        }
        else
        {
            for(int i=0; i<levelInfo->BrushList.Num(); i++)
            {
                EditorBrush *brush = levelInfo->BrushList[i];
                brush->TempSelectedPolys.Clear();
            }
        }
    }
    else
    {
        if(levelInfo->SelectedObjects.Num() != TempSelectionList.Num())
            bChanged = TRUE;
        else
        {
            for(int i=0; i<levelInfo->SelectedObjects.Num(); i++)
            {
                Entity *ent = levelInfo->SelectedObjects[i];
                if(!TempSelectionList.HasValue(ent))
                {
                    bChanged = TRUE;
                    break;
                }
            }
        }

        if(bChanged)
        {
            levelInfo->SaveObjectSelectionUndoData();

            if(bAdd)
            {
                levelInfo->SelectedObjects.AppendList(TempSelectionList);
                levelInfo->SelectedObjects.RemoveDupes();
            }
            else if(bSubtract)
            {
                for(int j=0; j<TempSelectionList.Num(); j++)
                {
                    Entity *selection = TempSelectionList[j];
                    selection->SetSelected(FALSE);
                    levelInfo->SelectedObjects.RemoveItem(selection);
                }
            }
            else
            {
                levelInfo->Deselect();
                levelInfo->SelectedObjects.CopyList(TempSelectionList);
            }

            for(int j=0; j<levelInfo->SelectedObjects.Num(); j++)
                levelInfo->SelectedObjects[j]->SetSelected(TRUE);

            if(levelInfo->SelectedObjects.Num())
            {
                if(!levelInfo->curManipulator)
                {
                    if(levelInfo->curModifyMode == ModifyMode_Select)
                        levelInfo->curManipulator = CreateObject(DefaultManipulator);
                    else if(levelInfo->curModifyMode == ModifyMode_Move)
                        levelInfo->curManipulator = CreateObject(PositionManipulator);
                    else if(levelInfo->curModifyMode == ModifyMode_Rotate)
                        levelInfo->curManipulator = CreateObject(RotationManipulator);
                }

                levelInfo->UpdateManipulatorPos();
            }
            else
            {
                DestroyObject(levelInfo->curManipulator);
                levelInfo->curManipulator = NULL;
            }
        }
    }

    traceOut;
}


void EditorViewport::GotFocus()
{
}

void EditorViewport::LostFocus()
{
}


void EditorViewport::PreFrame()
{
    traceIn(EditorViewport::PreFrame);

    float aspectx = Size.x/Size.y;
    float aspecty = Size.y/Size.x;
    float zoomAdjustX = zoom*(Size.x/GS->GetSizeXF());
    float zoomAdjustY = zoom*(Size.y/GS->GetSizeXF());

    if(type == ViewportType_Main)
    {
        zoomAdjustX *= 0.4f;
        zoomAdjustY *= 0.4f;

        camera->SetFrustum(-zoomAdjustX, zoomAdjustX, -zoomAdjustY, zoomAdjustY, 0.4f, 4096.0f);
    }
    else
        camera->SetOrtho(-zoomAdjustX, zoomAdjustX, -zoomAdjustY, zoomAdjustY, -2048.0f, 2048.0f);

    traceOut;
}


void EditorViewport::Render()
{
    traceIn(EditorViewport::Render);

    if(camera && (Size.x != 0.0f) && (Size.y != 0.0f))
    {
        //------------------------------------------------
        //  Set Audio Listener Position
        /*if(camera->bSoundCamera)
        {
            SS->SetListenerOriantation(camera->GetWorldRot());
            SS->SetListenerPosition(camera->GetWorldPos());
        }*/

        //------------------------------------------------
        //  Draw World
        SetViewport((int)Pos.x, (int)Pos.y, (int)Size.x, (int)Size.y);

        camera->LoadProjectionTransform();

        
        MatrixPush();
            MatrixIdentity();

            //Matrix m;
            //m.SetIdentity();
            //m *= camera->GetWorldRot().GetInv();
            //m *= -camera->GetWorldPos();

            if( (drawType == ViewportDrawType_Wireframe) ||
                (drawType == ViewportDrawType_Brushes)   )
            {
                if(level && (drawType == ViewportDrawType_Wireframe))
                    level->DrawWireframe(camera);

                if(type != ViewportType_Main)
                {
                    if(bShowGrid)
                    {
                        float spacing = levelInfo->gridSpacing;

                        float camX, camY;

                        switch(type)
                        {
                            case ViewportType_Front:
                                camX = camera->GetWorldPos().x;
                                camY = camera->GetWorldPos().y;
                                break;

                            case ViewportType_Back:
                                camX = -camera->GetWorldPos().x;
                                camY = camera->GetWorldPos().y;
                                break;

                            case ViewportType_Bottom:
                                camX = camera->GetWorldPos().x;
                                camY = camera->GetWorldPos().z;
                                break;

                            case ViewportType_Top:
                                camX = camera->GetWorldPos().x;
                                camY = -camera->GetWorldPos().z;
                                break;

                            case ViewportType_Right:
                                camX = -camera->GetWorldPos().z;
                                camY = camera->GetWorldPos().y;
                                break;

                            case ViewportType_Left:
                                camX = camera->GetWorldPos().z;
                                camY = camera->GetWorldPos().y;
                                break;
                        }

                        LoadIndexBuffer(NULL);

                        float xStart    = camX+camera->Left();
                        float yStart    = camY+camera->Top();

                        while(((camera->Right()/spacing) > 50.0f) || ((camera->Bottom()/spacing) > 50.0f))
                            spacing *= 2.0f;

                        float xPos = ceilf(xStart/spacing)*spacing;
                        float yPos = ceilf(yStart/spacing)*spacing;

                        xPos -= camX;
                        yPos -= camY;

                        float stopThingy = level->IsOf(GetClass(OctLevel)) ? 20.0f : 10.0f;

                        Shader *solidShader = GetVertexShader(TEXT("Base:SolidColor.vShader"));
                        LoadVertexShader(solidShader);
                        LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

                        MatrixPush();
                        MatrixIdentity();

                        LoadVertexBuffer(editor->vbGridLineH);

                        while(xPos < camera->Right())
                        {
                            float realX = fabsf(xPos+camX);

                            Vect4 gridColor(0.75f, 0.75f, 1.0f, 0.5f);

                            if(realX < EPSILON)
                                solidShader->SetColor(solidShader->GetParameter(1), gridColor);
                            else
                            {
                                float val = fmodf(realX, stopThingy);

                                if((val < EPSILON) || ((stopThingy-val) < EPSILON))
                                    gridColor *= 0.5f;
                                else
                                    gridColor *= 0.25f;

                                gridColor.w = 0.5f;
                                solidShader->SetColor(solidShader->GetParameter(1), gridColor);
                            }

                            MatrixPush();
                            MatrixTranslate(xPos, 0.0f, 0.0f);
                            MatrixScale(1.0f, camera->Bottom(), 1.0f);

                            Draw(GS_LINES);

                            MatrixPop();

                            xPos += spacing;
                        }

                        LoadVertexBuffer(editor->vbGridLineV);

                        while(yPos < camera->Bottom())
                        {
                            float realY = fabs(yPos+camY);

                            Vect4 gridColor(0.75f, 0.75f, 1.0f, 0.5f);

                            if(realY < EPSILON)
                                solidShader->SetColor(solidShader->GetParameter(1), gridColor);
                            else
                            {
                                float val = fmodf(realY, stopThingy);

                                if((val < EPSILON) || ((stopThingy-val) < EPSILON))
                                    gridColor *= 0.5f;
                                else
                                    gridColor *= 0.25f;

                                gridColor.w = 0.5f;
                                solidShader->SetColor(solidShader->GetParameter(1), gridColor);
                            }

                            MatrixPush();
                            MatrixTranslate(0.0f, yPos, 0.0f);
                            MatrixScale(camera->Right(), 1.0f, 1.0f);

                            Draw(GS_LINES);

                            MatrixPop();

                            yPos += spacing;
                        }

                        MatrixPop();

                        LoadVertexBuffer(NULL);
                        LoadVertexShader(NULL);
                        LoadPixelShader(NULL);
                    }
                }
            }

            if(level)
            {
                if( (drawType == ViewportDrawType_FullRender)   ||
                    (drawType == ViewportDrawType_TexturedOnly) )
                {
                    if(drawType == ViewportDrawType_FullRender)
                    {
                        SetFrameBufferTarget(level->MainRenderTexture);
                        ClearColorBuffer(TRUE);
                    }

                    level->Draw(camera, NULL, (drawType == ViewportDrawType_TexturedOnly));
                }
            }

            if(bShowGrid && (type == ViewportType_Main) && level && level->IsOf(GetClass(OctLevel)) && editor->editorEffects)
            {
                profileSegment("editor grid");

                float yOffset = levelInfo->curYPlanePosition;

                Matrix m;
                m.SetIdentity();
                m *= camera->GetWorldRot().GetInv();
                m *= -camera->GetWorldPos();

                DepthWriteEnable(FALSE);
                EnableDepthTest(TRUE);
                DepthFunction(GS_LESS);
                EnableBlending(TRUE);
                BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
                ColorWriteEnable(1, 1, 1, 0);

                MatrixPush();
                MatrixSet(m);

                //...don't ask.
                LoadVertexBuffer(NULL);
                LoadIndexBuffer(NULL);

                //-----------------------------------------------------------------------------------------------------------

                register Effect *gridShaders = editor->editorEffects;

                HANDLE hTechnique = gridShaders->GetTechnique(TEXT("QuickGrid"));
                gridShaders->BeginTechnique(hTechnique);

                HANDLE hColor = gridShaders->GetParameterByName(TEXT("lineColor"));
                HANDLE hLineOffset = gridShaders->GetParameterByName(TEXT("lineOffset"));
                Matrix curLineMatrix;

                //-----------------------------------------------------------------------------------------------------------

                int numAlignRows = 256;
                float spacing = 10.0f;
                int half = numAlignRows/2;

                gridShaders->SetFloat(gridShaders->GetParameterByName(TEXT("yOffset")), yOffset-0.02f);

                gridShaders->SetFloat(gridShaders->GetParameterByName(TEXT("lineLength")), spacing*float(half));

                LoadVertexBuffer(editor->vbGridLineH);

                for(int pass=0; pass<2; pass++)
                {
                    gridShaders->BeginPass(pass);
                    for(int i=0; i<=numAlignRows; i++)
                    {
                        float pos = float(i-half)*spacing;

                        Vect4 gridColor(0.75f, 0.75f, 1.0f, 0.5f);

                        if((i != 0) && (i != half) && (i != numAlignRows))
                        {
                            gridColor *= 0.5f;
                            gridColor.w = 0.5f;
                        }

                        gridShaders->SetColor(hColor, gridColor);
                        gridShaders->SetFloat(hLineOffset, pos);

                        Draw(GS_LINES);
                    }
                    gridShaders->EndPass();
                }

                gridShaders->EndTechnique();

                //-----------------------------------------------------------------------------------------------------------

                hTechnique = gridShaders->GetTechnique(TEXT("SphereFadeGrid"));
                gridShaders->BeginTechnique(hTechnique);

                float fDetailArea = 80.0f/levelInfo->gridSpacing;
                int iDetailArea = int(fDetailArea);
                int iTotalSize = iDetailArea*2;

                Vect spherePos = camera->GetWorldPos();
                spherePos.y = 0.0f;

                gridShaders->SetFloat(gridShaders->GetParameterByName(TEXT("yOffset")), yOffset-0.02f);

                gridShaders->SetFloat(gridShaders->GetParameterByName(TEXT("sphereSize")), 80.0f);
                gridShaders->SetVector(gridShaders->GetParameterByName(TEXT("spherePos")), spherePos);
                gridShaders->SetFloat(gridShaders->GetParameterByName(TEXT("lineLength")), 80.0f);

                Vect spherePosSnapped = spherePos;
                SnapToSpacing(spherePosSnapped, levelInfo->gridSpacing);
                Vect snapOffset = spherePosSnapped-spherePos;

                for(int pass=0; pass<2; pass++)
                {
                    float posOffset = (pass == 0) ? snapOffset.z : snapOffset.x;

                    gridShaders->BeginPass(pass);
                    for(int i=0; i<=iTotalSize; i++)
                    {
                        float pos = (float(i-iDetailArea)*levelInfo->gridSpacing)+posOffset;

                        Vect4 gridColor(0.75f, 0.75f, 1.0f, 0.2f);

                        gridShaders->SetColor(hColor, gridColor);
                        gridShaders->SetFloat(hLineOffset, pos);

                        Draw(GS_LINES);
                    }
                    gridShaders->EndPass();
                }

                gridShaders->EndTechnique();

                //-----------------------------------------------------------------------------------------------------------

                ColorWriteEnable(1, 1, 1, 1);

                LoadVertexBuffer(NULL);

                MatrixPop();
            }

            if(level && !editor->bHideEditorObjects)
                levelInfo->DrawEditorObjects(camera, zoom, this);
        MatrixPop();

        Set2DMode();

        ResetViewport();
    }

    if(bSelecting)
    {
        Shader *solidShader = GetVertexShader(TEXT("Base:SolidColor.vShader"));
        LoadVertexShader(solidShader);
        LoadPixelShader(GetPixelShader(TEXT("Base:SolidColor.pShader")));

        solidShader->SetColor(solidShader->GetParameter(1), 1.0f, 1.0f, 1.0f, 1.0f);

        EnableBlending(TRUE);
        BlendFunction(GS_BLEND_INVDSTCOLOR, GS_BLEND_ZERO);

        RenderStart();
            Vertex(float(selStartX), float(selStartY));
            Vertex(float(selEndX), float(selStartY));
            Vertex(float(selEndX), float(selEndY));
            Vertex(float(selStartX), float(selEndY));
            Vertex(float(selStartX), float(selStartY));
        RenderStop(GS_LINESTRIP);

        LoadVertexShader(NULL);
        LoadPixelShader(NULL);

        BlendFunction(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
    }

    GS->SetFontColor(INVALID);
    GS->SetCurFont(TEXT("Base:Arial Medium.xft"));

    MatrixPush();
    MatrixTranslate(ceilf(Pos.x)-Pos.x, ceilf(Pos.y)-Pos.y, 0);
        GS->DrawText(0.0f, 0.0f, Size.x, Size.y, FALSE, typeName);
    MatrixPop();

    traceOut;
}


void EditorViewport::ProcessMovement(short x_offset, short y_offset)
{
    traceIn(EditorViewport::ProcessMovement);

    if(!bMovedMouse)
    {
        GetSystem()->GetLocalMousePos(lastMouseX, lastMouseY);
        DWORD displayCount = ShowCursor(FALSE);
        while(displayCount > -1)
            displayCount = ShowCursor(FALSE);
        OSClipCursor(TRUE);
        OSCenterCursor();
    
        bMovedMouse = true;
        bIgnoreMove = false;
    }

    if(!bIgnoreMove)
    {
        if(type == ViewportType_Main)
        {
            if(bLeftMouseDown && !bRightMouseDown)
            {
                rotY -= float(x_offset)*0.075f;

                if(rotY >= 360.0f)   rotY -= 360.0f;
                else if(rotY < 0.0f) rotY += 360.0f;

                Vect add(0.0f, 0.0f, float(y_offset)*editor->moveSpeed);

                Matrix rotMatrix(AxisAngle(0.0f, 1.0f, 0.0f, RAD(rotY)));
                rotMatrix.Transpose();

                add.TransformVector(rotMatrix);

                camera->SetPos(camera->GetLocalPos()+add);

                Quat newRot;
                newRot = AxisAngle(0.0f, 1.0f, 0.0f, RAD(rotY));
                newRot *= AxisAngle(1.0f, 0.0f, 0.0f, RAD(rotX));

                camera->SetRot(newRot);
            }

            else if(!bLeftMouseDown && bRightMouseDown)
            {
                rotX -= float(y_offset)*0.05f;
                rotY -= float(x_offset)*0.05f;

                if(rotX >= 360.0f)   rotX -= 360.0f;
                else if(rotX < 0.0f) rotX += 360.0f;

                if(rotY >= 360.0f)   rotY -= 360.0f;
                else if(rotY < 0.0f) rotY += 360.0f;

                Quat newRot;
                newRot = AxisAngle(0.0f, 1.0f, 0.0f, RAD(rotY));
                newRot *= AxisAngle(1.0f, 0.0f, 0.0f, RAD(rotX));

                camera->SetRot(newRot);
            }

            else if(bLeftMouseDown && bRightMouseDown)
            {
                Vect addX(float(x_offset)*editor->moveSpeed, 0.0f, 0.0f);
                Vect addY(0.0f, -float(y_offset)*editor->moveSpeed, 0.0f);

                Matrix rotMatrix(AxisAngle(0.0f, 1.0f, 0.0f, RAD(rotY)));
                rotMatrix.Transpose();

                addX.TransformVector(rotMatrix);

                camera->SetPos(camera->GetLocalPos()+addX+addY);
            }
        }
        else
        {
            if(bLeftMouseDown && bRightMouseDown)
            {
                zoom += float(y_offset);

                if(zoom < 10.0f)
                    zoom = 10.0f;
                else if(zoom > 1000.0f)
                    zoom = 1000.0f;
            }
            else
            {
                Vect add(float(x_offset)*editor->moveSpeed, -float(y_offset)*editor->moveSpeed, 0.0f);

                if(!bLeftMouseDown && bRightMouseDown)
                    add *= zoom/8.0f;
                else
                    add *= zoom/16.0f;

                Matrix rotMatrix(camera->GetLocalRot());
                rotMatrix.Transpose();

                add.TransformVector(rotMatrix);

                camera->SetPos(camera->GetLocalPos()+add);
            }
        }

        OSCenterCursor();
    }

    SetWorldPos(camera, camera->GetLocalPos());
    SetWorldRot(camera, camera->GetLocalRot());
    //todo - had it manually set world posotion/rot here
    //camera->SetPos(camera->Pos;
    //camera->GetWorldRot() = camera->Rot;

    bIgnoreMove = !bIgnoreMove;

    traceOut;
}

void EditorViewport::Tick(float fTime)
{
    Super::Tick(fTime);

    if(bMovingLeft || bMovingRight || bMovingForward || bMovingBackward)
    {
        Matrix rotMatrix(camera->GetWorldRot());
        rotMatrix.Transpose();

        Vect addVector(0.0, 0.0f, 0.0f);

        if(bMovingLeft)
            addVector.x = -editor->moveSpeed;
        else if(bMovingRight)
            addVector.x = editor->moveSpeed;

        if(bMovingForward)
            addVector.z = -editor->moveSpeed;
        else if(bMovingBackward)
            addVector.z = editor->moveSpeed;

        addVector *= 6000.0f;
        addVector *= fTime;
        addVector.TransformVector(rotMatrix);

        camera->SetPos(camera->GetLocalPos()+addVector);
    }
}


void EditorViewport::SerializeSettings(Serializer &s)
{
    traceIn(EditorViewport::SerializeSettings);

    if(s.IsLoading())
    {
        ViewportType newType;
        s << (DWORD&)newType;
        SetViewportType(newType);

        s << bShowGrid;
        s << (DWORD&)drawType;
        camera->Serialize(s);

        s << rotX << rotY;

        UpdateViewports();
    }
    else
    {
        s << (DWORD&)type;

        s << bShowGrid;
        s << (DWORD&)drawType;
        camera->Serialize(s);

        s << rotX << rotY;
    }

    traceOut;
}

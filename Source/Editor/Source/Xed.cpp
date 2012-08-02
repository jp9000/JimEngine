/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Xed.cpp:  Main source file

  Copyright (c) by H.B.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#define ENGINE_MAIN_PROGRAM
#include "Xed.h"


int XedMessageLoop();
BOOL ProcessAccelerators(HACCEL hAccel, MSG *msg);
void KeyPressThingy(HWND targetWindow, UINT message, WPARAM keyinfo, LPARAM lParam);


int ENGINEAPI EngineMain(TSTR *lpParams, int numParams)
{
    INITCOMMONCONTROLSEX iccex;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    InitSqrtTable();

    iccex.dwSize = sizeof(iccex);
    iccex.dwICC = ICC_TREEVIEW_CLASSES|ICC_BAR_CLASSES;
    InitCommonControlsEx(&iccex);

    if(InitBase(TEXT("Xed")))
    {
        float fGamma        = AppConfig->GetFloat(TEXT("Display"), TEXT("Gamma"), 1.0f);
        float fBrightness   = AppConfig->GetFloat(TEXT("Display"), TEXT("Brightness"), 1.0f);
        float fContrast     = AppConfig->GetFloat(TEXT("Display"), TEXT("Contrast"), 1.0f);
        OSColorAdjust(fGamma, fBrightness, fContrast);

        while(XedMessageLoop());

        TerminateBase();
    }

    return 0;
}


BOOL XedMessageLoop()
{
    traceIn(XedMessageLoop);

    MSG msg;

    HACCEL hAccel = LoadAccelerators(hinstMain, MAKEINTRESOURCE(IDR_MAINACCELERATORS));

    BOOL bPlayingMeshBrowserAnimation = meshBrowser && meshBrowser->bPlayingAnimation;

    if(editor->bRealTimeEnabled || bPlayingMeshBrowserAnimation)
    {
        BOOL bProcessNextMessage = FALSE;

        if(!bBaseLoaded)
            return 0;

        while (!bExiting)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    if(msg.wParam == 1)
                        return TRUE;

                    break;
                }
                else
                {
                    if((msg.message == WM_KEYDOWN) || (msg.message == WM_KEYUP))
                        KeyPressThingy(msg.hwnd, msg.message, msg.wParam, msg.lParam);

                    if(!ProcessAccelerators(hAccel, &msg))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    } 
                    bProcessNextMessage = TRUE;
                }
            }
            else if(bProcessNextMessage)
                bProcessNextMessage = FALSE;

            if(!bProcessNextMessage)
            {
                if(editor->bRealTimeEnabled)
                    engine->Update();
                if(bPlayingMeshBrowserAnimation)
                    meshBrowser->UpdateMeshView();
            }
        }
        return FALSE;
    }
    else
    {
        long iRet;
        while(iRet = (long)GetMessage(&msg, NULL, 0, 0))
        {
            if(!ProcessAccelerators(hAccel, &msg))
            {
                if((msg.message == WM_KEYDOWN) || (msg.message == WM_KEYUP))
                    KeyPressThingy(msg.hwnd, msg.message, msg.wParam, msg.lParam);

                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            } 
        }

        if((msg.message == WM_QUIT) && (msg.wParam == 1))
            return TRUE;

        return FALSE;
    }

    traceOutStop;

    return FALSE;
}

void KeyPressThingy(HWND targetWindow, UINT message, WPARAM keyinfo, LPARAM lParam)
{
    traceIn(KeyPressThingy);

    if(!editor)
        return;

    TCHAR targetName[10];
    GetClassName(targetWindow, targetName, 9);

    if(scmpi(targetName, TEXT("edit")) == 0)
        return;

    do
    {
        if(targetWindow == hwndEditor)
            SendMessage(hwndMain, message, keyinfo, lParam);
        else if(prefabBrowser && targetWindow == prefabBrowser->hwndPrefabBrowser)
            SendMessage(prefabBrowser->hwndPrefabView, message, keyinfo, lParam);
        else if(materialEditor && targetWindow == materialEditor->hwndMaterialEditor)
            SendMessage(materialEditor->hwndMaterialView, message, keyinfo, lParam);
        else if(meshBrowser && targetWindow == meshBrowser->hwndMeshBrowser)
            SendMessage(meshBrowser->hwndMeshView, message, keyinfo, lParam);
        else
            continue;

        return;
    }while(targetWindow = GetParent(targetWindow));

    traceOut;
}

BOOL ProcessAccelerators(HACCEL hAccel, MSG *msg)
{
    traceIn(ProcessAccelerators);

    HWND hwndCurrent = msg->hwnd;

    while(hwndCurrent && !SendMessage(hwndCurrent, WM_PROCESSACCELERATORS, 0, 0))
        hwndCurrent = GetParent(hwndCurrent);

    if(hwndCurrent)
        return TranslateAccelerator(hwndCurrent, hAccel, msg);

    return FALSE;

    traceOut;
}

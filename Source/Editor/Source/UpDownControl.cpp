/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  UpDownControl.cpp:  Up Down Control

  (c) 2001-by H.B.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#include "Xed.h"


struct UDCStruct
{
    BOOL    bIsFloat;

    union
    {
        struct
        {
            float floatVal;
            float floatMin;
            float floatMax;
            float floatInc;

            float floatStart;
        };
        struct
        {
            int intVal;
            int intMin;
            int intMax;
            int intInc;

            int intStart;
        };
    };

    //0 = nonpushed;  1 = up pushed;  2 = down pushed;  3 = scrolling
    DWORD   firstState;
    DWORD   state;

    DWORD   linkedID;

    BOOL    bInitialized;
    BOOL    bDisabled;
    int     cx,cy;
    int     scrollYStart;
};


struct LinkedUpDownInfo
{
    HWND hwndSubclass;
    HWND hwndLinkedUpDown;
    FARPROC oldEditProc;
};

SafeList<LinkedUpDownInfo> LinkedEdits;


LRESULT WINAPI PropertyEditControlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
WNDPROC oldEdit = NULL;


LRESULT WINAPI UpDownControlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI LinkedEditSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void WINAPI ScaleUpDownValue(HWND hwndUpDown);
void WINAPI DrawUpDownControl(HDC hDC, UDCStruct *pUDCData);

void WINAPI UpdateFromText(HWND hwndUpDown);


LRESULT WINAPI UpDownControlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UDCStruct *pUDCData;

    switch(message)
    {
        case WM_NCCREATE:
            {
                CREATESTRUCT *pCreateData = (CREATESTRUCT*)lParam;

                pUDCData = (UDCStruct*)malloc(sizeof(UDCStruct));
                zero(pUDCData, sizeof(UDCStruct));
                SetWindowLongPtr(hwnd, 0, (LONG_PTR)pUDCData);

                pUDCData->bDisabled = ((pCreateData->style & WS_DISABLED) != 0);

                pUDCData->cx = pCreateData->cx;
                pUDCData->cy = pCreateData->cy;

                pUDCData->linkedID = INVALID;

                return TRUE;
            }

        case WM_DESTROY:
            {
                pUDCData = (UDCStruct*)GetWindowLongPtr(hwnd, 0);

                if(pUDCData->linkedID != INVALID)
                    LinkUpDown(hwnd, NULL);

                if(pUDCData)
                    free(pUDCData);

                break;
            }

        case WM_PAINT:
            {
                pUDCData = (UDCStruct*)GetWindowLongPtr(hwnd, 0);

                PAINTSTRUCT ps;

                HDC hDC = BeginPaint(hwnd, &ps);
                DrawUpDownControl(hDC, pUDCData);
                EndPaint(hwnd, &ps);

                break;
            }

        case WM_ENABLE:
            {
                pUDCData = (UDCStruct*)GetWindowLongPtr(hwnd, 0);

                pUDCData->bDisabled = !wParam;

                //redraw control
                HDC hDC = GetDC(hwnd);
                DrawUpDownControl(hDC, pUDCData);
                ReleaseDC(hwnd, hDC);

                break;
            }

        case WM_LBUTTONDOWN:
            {
                pUDCData = (UDCStruct*)GetWindowLongPtr(hwnd, 0);

                int xPos = (int)(short)LOWORD(lParam); 
                int yPos = (int)(short)HIWORD(lParam);

                int middlePoint = pUDCData->cy/2;

                UpdateFromText(hwnd);

                pUDCData->firstState = pUDCData->state = (yPos < middlePoint) ? 1 : 2;

                //redraw control
                HDC hDC = GetDC(hwnd);
                DrawUpDownControl(hDC, pUDCData);
                ReleaseDC(hwnd, hDC);

                SetCapture(hwnd);

                SetTimer(hwnd, 0, 500, NULL);

                if(pUDCData->bIsFloat)
                    pUDCData->floatStart = pUDCData->floatVal;
                else
                    pUDCData->intStart = pUDCData->intVal;

                ScaleUpDownValue(hwnd);

                break;
            }

        case WM_LBUTTONUP:
            {
                pUDCData = (UDCStruct*)GetWindowLongPtr(hwnd, 0);

                ReleaseCapture();

                if(pUDCData->state)
                {
                    pUDCData->state = 0;

                    //redraw control
                    HDC hDC = GetDC(hwnd);
                    DrawUpDownControl(hDC, pUDCData);
                    ReleaseDC(hwnd, hDC);

                    SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS)));
                }

                pUDCData->firstState = 0;

                KillTimer(hwnd, 0);

                BOOL bSendUpdateMessage = FALSE;

                if(pUDCData->bIsFloat)
                {
                    if(fabsf(pUDCData->floatStart-pUDCData->floatVal) > 1e-4f)
                        bSendUpdateMessage = TRUE;
                }
                else
                {
                    if(pUDCData->intStart != pUDCData->intVal)
                        bSendUpdateMessage = TRUE;
                }

                if(bSendUpdateMessage)
                {
                    DWORD controlID = GetWindowLongPtr(hwnd, GWLP_ID);
                    SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(controlID, JUDN_UPDATE), (LPARAM)hwnd);
                }

                break;
            }

        case WM_MOUSEMOVE:
            {
                pUDCData = (UDCStruct*)GetWindowLongPtr(hwnd, 0);

                if(pUDCData->firstState)
                {
                    int xPos = (int)(short)LOWORD(lParam); 
                    int yPos = (int)(short)HIWORD(lParam);

                    int middlePoint = pUDCData->cy/2;

                    DWORD lastState = pUDCData->state;

                    if(pUDCData->state == 3)
                    {
                        int adjustAmount = pUDCData->scrollYStart - yPos;
                        float fAdjust = floorf((((float)adjustAmount) * 0.2f) + 0.5f);
                        adjustAmount = (int)fAdjust;

                        if(pUDCData->bIsFloat)
                            SetUpDownFloat(hwnd, pUDCData->floatStart + (fAdjust*pUDCData->floatInc), true);
                        else
                            SetUpDownInt(hwnd, pUDCData->intStart + (adjustAmount*pUDCData->intInc), true);
                    }
                    else
                    {
                        if( (yPos < 0) || (yPos >= pUDCData->cy) ||
                            (xPos < 0) || (xPos >= pUDCData->cx) )
                        {
                            int buttonY, buttonCY;
                            if(pUDCData->firstState == 1)
                            {
                                buttonY = 0;
                                buttonCY = middlePoint;
                            }
                            else
                            {
                                buttonY = middlePoint;
                                buttonCY = pUDCData->cy;
                            }

                            if(yPos < (buttonY-15) || yPos >= (buttonCY+15))
                            {
                                pUDCData->state = 3;
                                pUDCData->scrollYStart = yPos;
                                pUDCData->intStart = pUDCData->intVal;

                                SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS)));
                            }
                            else
                                pUDCData->state = 0;
                        }
                        else
                        {
                            pUDCData->state = (yPos < middlePoint) ? 1 : 2;

                            if(pUDCData->state != pUDCData->firstState)
                                pUDCData->state = 0;
                        }
                    }

                    if(lastState != pUDCData->state)
                    {
                        //redraw control
                        HDC hDC = GetDC(hwnd);
                        DrawUpDownControl(hDC, pUDCData);
                        ReleaseDC(hwnd, hDC);
                    }
                }

                break;
            }

        case WM_TIMER:
            {
                pUDCData = (UDCStruct*)GetWindowLongPtr(hwnd, 0);

                if(wParam == 0)
                {
                    KillTimer(hwnd, 0);
                    SetTimer(hwnd, 0, 100, NULL);

                    if(pUDCData->state && pUDCData->state != 3)
                        ScaleUpDownValue(hwnd);
                }

                break;
            }

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void WINAPI UpdateFromText(HWND hwndUpDown)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);
    HWND hwndText = LinkedEdits[pUDCData->linkedID].hwndSubclass;

    int controlID = GetWindowLongPtr(hwndUpDown, GWLP_ID);

    if(!hwndText)
        return;

    TCHAR lpNum[64];

    if(pUDCData->bIsFloat)
    {
        pUDCData->floatStart = pUDCData->floatVal;

        GetWindowText(hwndText, lpNum, 63);
        float val = tstof(lpNum);

        if(ValidFloatString(lpNum))
        {
            if(!_finite(val) || (val < pUDCData->floatMin) || (val > pUDCData->floatMax))
            {
                tsprintf_s(lpNum, 63, TEXT("%0.3g"), pUDCData->floatVal);
                SetWindowText(hwndText, lpNum);
            }
            else
                pUDCData->floatVal = val;

            if(fabsf(pUDCData->floatVal - pUDCData->floatStart) > 1e-4)
                SendMessage(GetParent(hwndText), WM_COMMAND, MAKEWPARAM(controlID, JUDN_UPDATE), (LPARAM)hwndUpDown);
        }
        else
        {
            tsprintf_s(lpNum, 63, TEXT("%0.3g"), pUDCData->floatVal);
            SetWindowText(hwndText, lpNum);
        }
    }
    else
    {
        pUDCData->intStart = pUDCData->intVal;

        GetWindowText(hwndText, lpNum, 63);
        int val = tstoi(lpNum);

        if(ValidIntString(lpNum))
        {
            if((val < pUDCData->intMin) || (val > pUDCData->intMax))
            {
                tsprintf_s(lpNum, 63, TEXT("%d"), pUDCData->intVal);
                SetWindowText(hwndText, lpNum);
            }
            else
                pUDCData->intVal = val;

            if(pUDCData->intStart != pUDCData->intVal)
                SendMessage(GetParent(hwndText), WM_COMMAND, MAKEWPARAM(controlID, JUDN_UPDATE), (LPARAM)hwndUpDown);
        }
        else
        {
            tsprintf_s(lpNum, 63, TEXT("%d"), pUDCData->intVal);
            SetWindowText(hwndText, lpNum);
        }
    }
}

LRESULT WINAPI LinkedEditSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LinkedUpDownInfo *pLudi = NULL;

    for(int i=0; i<LinkedEdits.Num(); i++)
    {
        if(LinkedEdits[i].hwndSubclass == hwnd)
            pLudi = &LinkedEdits[i];
    }

    HWND hwndUpDown = (HWND)pLudi->hwndLinkedUpDown;

    //---------------------------------------------

    if( ((message == WM_KEYDOWN) && (wParam == VK_RETURN)) ||
        (message == WM_KILLFOCUS) )
    {
        UpdateFromText(hwndUpDown);
    }
    else if(message == WM_CHAR)
    {
        if((wParam == '\r') || (wParam == '\t'))
            return 0;
    }
    else if(message == WM_DESTROY)
    {
        FARPROC oldEditProc = pLudi->oldEditProc;
        LinkUpDown(pLudi->hwndLinkedUpDown, NULL);

        return CallWindowProc(oldEditProc, hwnd, message, wParam, lParam);
    }

    return CallWindowProc(pLudi->oldEditProc, hwnd, message, wParam, lParam);
}





void WINAPI DrawUpDownControl(HDC hDC, UDCStruct *pUDCData)
{
    int middlePoint = pUDCData->cy/2;

    RECT rc1 = {0, 0, pUDCData->cx, middlePoint};
    RECT rc2 = {0, middlePoint, pUDCData->cx, pUDCData->cy};

    DWORD upButtonFlags=0, downButtonFlags=0;

    if(pUDCData->bDisabled)
        upButtonFlags = downButtonFlags = DFCS_INACTIVE;
    else
    {
        if(pUDCData->state & 1)
            upButtonFlags   = DFCS_PUSHED;
        if(pUDCData->state & 2)
            downButtonFlags = DFCS_PUSHED;
    }

    DrawFrameControl(hDC, &rc1, DFC_SCROLL, DFCS_SCROLLUP|upButtonFlags);
    DrawFrameControl(hDC, &rc2, DFC_SCROLL, DFCS_SCROLLDOWN|downButtonFlags);
}



void WINAPI LinkUpDown(HWND hwndUpDown, HWND hwndEdit)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);

    if(!hwndEdit)
    {
        if(pUDCData->linkedID != INVALID)
        {
            LinkedUpDownInfo &linkData = LinkedEdits[pUDCData->linkedID];
            if(linkData.hwndLinkedUpDown == hwndUpDown)
            {
                SetWindowLongPtr(linkData.hwndSubclass, GWLP_WNDPROC, (LONG_PTR)linkData.oldEditProc);
                LinkedEdits.Remove(pUDCData->linkedID);
            }

            pUDCData->linkedID = INVALID;
        }
    }
    else
    {
        if(pUDCData->linkedID != INVALID)
            LinkUpDown(hwndUpDown, NULL);

        LinkedUpDownInfo ludi;

        ludi.hwndLinkedUpDown = hwndUpDown;
        ludi.oldEditProc = (FARPROC)GetWindowLongPtr(hwndEdit, GWLP_WNDPROC);
        ludi.hwndSubclass = hwndEdit;
        pUDCData->linkedID = LinkedEdits.Add(ludi);

        SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)LinkedEditSubclassProc);

        SendMessage(hwndEdit, EM_SETLIMITTEXT, 10, 0);

        ResetUpDownText(hwndUpDown);
    }
}

void WINAPI InitUpDownFloatData(HWND hwndUpDown, float initialValue, float minValue, float maxValue, float incValue)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);

    pUDCData->bInitialized = TRUE;
    pUDCData->bIsFloat = TRUE;

    pUDCData->floatVal = initialValue;
    pUDCData->floatMin = minValue;
    pUDCData->floatMax = maxValue;
    pUDCData->floatInc = incValue;

    ResetUpDownText(hwndUpDown);
}

void WINAPI InitUpDownIntData(HWND hwndUpDown, int initialValue, int minValue, int maxValue, int incValue)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);

    pUDCData->bInitialized = TRUE;
    pUDCData->bIsFloat = FALSE;

    pUDCData->intVal = initialValue;
    pUDCData->intMin = minValue;
    pUDCData->intMax = maxValue;
    pUDCData->intInc = incValue;

    ResetUpDownText(hwndUpDown);
}

void WINAPI SetUpDownFloat(HWND hwndUpDown, float value, bool bNotify)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);

    float lastVal = pUDCData->floatVal;
    pUDCData->floatVal = MIN(MAX(value, pUDCData->floatMin), pUDCData->floatMax);

    if(lastVal != pUDCData->floatVal)
    {
        ResetUpDownText(hwndUpDown);

        if(bNotify)
        {
            DWORD controlID = (DWORD)GetWindowLongPtr(hwndUpDown, GWLP_ID);
            SendMessage(GetParent(hwndUpDown), WM_COMMAND, MAKEWPARAM(controlID, JUDN_SCROLLING), (LPARAM)hwndUpDown);
        }
    }
}

void WINAPI SetUpDownInt(HWND hwndUpDown, int value, bool bNotify)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);

    int lastVal = pUDCData->intVal;
    pUDCData->intVal = MIN(MAX(value, pUDCData->intMin), pUDCData->intMax);

    if(lastVal != pUDCData->intVal)
    {
        ResetUpDownText(hwndUpDown);

        if(bNotify)
        {
            DWORD controlID = (DWORD)GetWindowLongPtr(hwndUpDown, GWLP_ID);
            SendMessage(GetParent(hwndUpDown), WM_COMMAND, MAKEWPARAM(controlID, JUDN_SCROLLING), (LPARAM)hwndUpDown);
        }
    }
}

float WINAPI GetUpDownFloat(HWND hwndUpDown)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);
    return pUDCData->floatVal;
}

int   WINAPI GetUpDownInt(HWND hwndUpDown)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);
    return pUDCData->intVal;
}

float WINAPI GetPrevUpDownFloat(HWND hwndUpDown)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);
    return pUDCData->floatStart;
}

int   WINAPI GetPrevUpDownInt(HWND hwndUpDown)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);
    return pUDCData->intStart;
}



void WINAPI ScaleUpDownValue(HWND hwndUpDown)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);

    if(pUDCData->bInitialized && !pUDCData->bDisabled)
    {
        BOOL bUp = (pUDCData->state == 1);
        BOOL bChanged = FALSE;

        if(pUDCData->bIsFloat)
        {
            float prevVal = pUDCData->floatVal;

            pUDCData->floatVal += bUp ? pUDCData->floatInc : -pUDCData->floatInc;
            pUDCData->floatVal = MIN(MAX(pUDCData->floatVal, pUDCData->floatMin), pUDCData->floatMax);

            if(prevVal != pUDCData->floatVal)
                bChanged = TRUE;
        }
        else
        {
            float prevVal = pUDCData->intVal;

            pUDCData->intVal += bUp ? pUDCData->intInc : -pUDCData->intInc;
            pUDCData->intVal = MIN(MAX(pUDCData->intVal, pUDCData->intMin), pUDCData->intMax);

            if(prevVal != pUDCData->intVal)
                bChanged = TRUE;
        }

        if(bChanged)
        {
            ResetUpDownText(hwndUpDown);

            DWORD controlID = (DWORD)GetWindowLongPtr(hwndUpDown, GWLP_ID);
            SendMessage(GetParent(hwndUpDown), WM_COMMAND, MAKEWPARAM(controlID, JUDN_SCROLLING), (LPARAM)hwndUpDown);
        }
    }
}

void WINAPI ResetUpDownText(HWND hwndUpDown)
{
    UDCStruct *pUDCData = (UDCStruct*)GetWindowLongPtr(hwndUpDown, 0);

    if(pUDCData->bInitialized && (pUDCData->linkedID != INVALID))
    {
        if(pUDCData->bIsFloat)
        {
            TCHAR floatText[64];
            tsprintf_s(floatText, 64, TEXT("%0.3g"), double(pUDCData->floatVal));

            SetWindowText(LinkedEdits[pUDCData->linkedID].hwndSubclass, floatText);
        }
        else
        {
            TCHAR intText[64];
            tsprintf_s(intText, 64, TEXT("%d"), pUDCData->intVal);

            SetWindowText(LinkedEdits[pUDCData->linkedID].hwndSubclass, intText);
        }
    }
}

void InitUpDownControl()
{
    WNDCLASS wnd;
    zero(&wnd, sizeof(WNDCLASS));

    wnd.cbClsExtra = 0;
    wnd.cbWndExtra = sizeof(LPVOID);
    wnd.hbrBackground = NULL;
    wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
    wnd.hIcon = NULL;
    wnd.hInstance = hinstMain;
    wnd.lpfnWndProc = UpDownControlProc;
    wnd.lpszClassName = TEXT("UpDownControl");
    wnd.lpszMenuName = NULL;
    wnd.style = CS_PARENTDC | CS_VREDRAW | CS_HREDRAW;

    RegisterClass(&wnd);

    //-----------------------------------

    zero(&wnd, sizeof(WNDCLASS));
    GetClassInfo(NULL, TEXT("EDIT"), &wnd);

    oldEdit = wnd.lpfnWndProc;

    wnd.lpszClassName = TEXT("PropertyEdit");
    wnd.lpfnWndProc = PropertyEditControlProc;
    wnd.hInstance = hinstMain;

    if(!RegisterClass(&wnd))
        CrashError(TEXT("Could not register the property edit control class"));
}



LRESULT WINAPI PropertyEditControlProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if((message == WM_KEYDOWN) && (wParam == VK_RETURN))
    {
        SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetWindowLongPtr(hwnd, GWLP_ID), EN_ENTERHIT), (LPARAM)hwnd);
        return 0;
    }
    else if(message == WM_CHAR)
    {
        if((wParam == '\r') || (wParam == '\t'))
            return 0;
    }

    return CallWindowProc((FARPROC)oldEdit, hwnd, message, wParam, lParam);
}


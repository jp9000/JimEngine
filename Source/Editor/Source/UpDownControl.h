/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  UpDownControl.h:  Up Down Control

  (c) 2001-by H.B.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#pragma once

void InitUpDownControl();

void WINAPI LinkUpDown(HWND hwndUpDown, HWND hwndEdit);
void WINAPI InitUpDownFloatData(HWND hwndUpDown, float initialValue, float minValue, float maxValue, float incValue=1.0f);
void WINAPI InitUpDownIntData(HWND hwndUpDown, int initialValue, int minValue, int maxValue, int incValue=1);
void WINAPI SetUpDownFloat(HWND hwndUpDown, float value, bool bNotify=false);
void WINAPI SetUpDownInt(HWND hwndUpDown, int value, bool bNotify=false);
void WINAPI ResetUpDownText(HWND hwndUpDown);
float WINAPI GetUpDownFloat(HWND hwndUpDown);
int   WINAPI GetUpDownInt(HWND hwndUpDown);
float WINAPI GetPrevUpDownFloat(HWND hwndUpDown);
int   WINAPI GetPrevUpDownInt(HWND hwndUpDown);


#define JUDN_UPDATE     0x301
#define JUDN_SCROLLING  0x300

#define EN_ENTERHIT     0x301

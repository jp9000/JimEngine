/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ColorControl.h:  Color Control

  (c) 2001-by H.B.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#pragma once

void InitColorControl();

DWORD CCGetColor(HWND hwnd);
void  CCSetColor(HWND hwnd, DWORD color);
void  CCSetColor(HWND hwnd, const Color3 &color);


#define CCN_CHANGED     0
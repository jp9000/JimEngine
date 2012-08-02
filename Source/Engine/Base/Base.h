/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Base.h:  Main Base.dll header
  
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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#ifndef BASE_HEADER
#define BASE_HEADER

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define USE_TRACE 1

#if defined(_WIN64) && !defined(C64)
    #define C64
#endif


#if defined(WIN32) && defined(C64)
    #define USE_SSE
#endif

#include "EngineDefs.h"

#define DLL_LOADING     1
#define DLL_UNLOADING   2

#if defined WIN32
    #include "Base_Windows.h"
#elif defined __UNIX__
    #include "Base_Unix.h"
#else
    #error Unknown Operating System
#endif



#pragma inline_recursion(on)
#pragma inline_depth(50)

//-----------------------------------------
//color defines
//-----------------------------------------
#define Color_White              0xFFFFFFFF  //255,255,255
#define Color_VeryLightGray      0xFFE0E0E0  //224,224,224
#define Color_LightGray          0xFFC0C0C0  //192,192,192
#define Color_Gray               0xFF808080  //128,128,128
#define Color_DarkGray           0xFF404040  //64,64,64
#define Color_Black              0xFF000000  //0,0,0      
                                               
#define Color_Cyan               0xFF00FFFF  //0,255,255  
#define Color_DarkCyan           0xFF008080  //0,128,128  
#define Color_Purple             0xFFFF00FF  //255,0,255  
#define Color_DarkPurple         0xFF800080  //128,0,128  
#define Color_Yellow             0xFFFFFF00  //255,255,0  
#define Color_DarkYellow         0xFF808000  //128,128,0  
                                               
#define Color_Red                0xFFFF0000  //255,0,0    
#define Color_DarkRed            0xFF800000  //128,0,0    
#define Color_Green              0xFF00FF00  //0,255,0    
#define Color_DarkGreen          0xFF008000  //0,128,0    
#define Color_Blue               0xFF0000FF  //0,0,255    
#define Color_DarkBlue           0xFF000080  //0,0,128    

#define RGB_A(rgba)         (((DWORD)(rgba) & 0xFF000000) >> 24)
#define RGB_R(rgb)          (((DWORD)(rgb) & 0xFF0000) >> 16)
#define RGB_G(rgb)          (((DWORD)(rgb) & 0x00FF00) >> 8)
#define RGB_B(rgb)          ((DWORD)(rgb) & 0x0000FF)

#define RGB_Af(rgba)        (((float)RGB_A(rgba)) / 255.0)
#define RGB_Rf(rgb)         (((float)RGB_R(rgb)) / 255.0)
#define RGB_Gf(rgb)         (((float)RGB_G(rgb)) / 255.0)
#define RGB_Bf(rgb)         (((float)RGB_B(rgb)) / 255.0)

#define MAKERGBA(r,g,b,a)   ((((DWORD)a) << 24)|(((DWORD)r) << 16)|(((DWORD)g) << 8)|((DWORD)b))
#define MAKERGB(r,g,b)      ((((DWORD)r << 16)|((DWORD)g << 8)|(DWORD)b))

#define REVERSE_COLOR(col)  MAKERGB(RGB_B(col), RGB_G(col), RGB_R(col))

#define Vect4_to_RGBA(v)    (MAKERGBA(((v).x*255.0f), ((v).y*255.0f), ((v).z*255.0f), ((v).w*255.0f)))
#define Vect_to_RGB(v)      (MAKERGB(((v).x*255.0f), ((v).y*255.0f), ((v).z*255.0f)))
#define RGBA_to_Vect4(dw)   Vect4(RGB_Rf(dw), RGB_Gf(dw), RGB_Bf(dw), RGB_Af(dw))
#define RGB_to_Vect(dw)     Vect(RGB_Rf(dw), RGB_Gf(dw), RGB_Bf(dw))
#define RGB_to_VectExp(dw)  Vect((RGB_Rf(dw)-0.5f)*2.0f, (RGB_Gf(dw)-0.5f)*2.0f, (RGB_Bf(dw)-0.5f)*2.0f)

#define Color4_to_RGBA(c)   (MAKERGBA(((c).x*255.0f), ((c).y*255.0f), ((c).z*255.0f), ((c).w*255.0f)))
#define Color_to_RGB(c)     (MAKERGB(((c).x*255.0f), ((c).y*255.0f), ((c).z*255.0f)))
#define RGBA_to_Color4(dw)  Color4(RGB_Rf(dw), RGB_Gf(dw), RGB_Bf(dw), RGB_Af(dw))
#define RGB_to_Color(dw)    Color(RGB_Rf(dw), RGB_Gf(dw), RGB_Bf(dw))
#define RGB_to_ColorExp(dw) Color((RGB_Rf(dw)-0.5f)*2.0f, (RGB_Gf(dw)-0.5f)*2.0f, (RGB_Bf(dw)-0.5f)*2.0f)


//-----------------------------------------
//Externs
//-----------------------------------------
BASE_EXPORT extern BOOL bBaseLoaded;
BASE_EXPORT extern BOOL bWindowActive;
BASE_EXPORT extern DWORD dwIconID;


//-----------------------------------------
//forwards
//-----------------------------------------
class   Object;
class   Light;
class   GraphicsSystem;
class   Window;
class   ControlWindow;
class   Input;
class   Camera;
struct  Vect2;
struct  Vect;
struct  Vect4;
struct  Quat;
struct  AxisAngle;
struct  Bounds;
struct  Plane;
struct  Matrix;
struct  ViewClip;
struct  DisplayMode;
typedef Vect Color3;
typedef Vect4 Color4;
template<typename T> class List;


#include "BasePlatform.h"

//-----------------------------------------
//Base functions
//-----------------------------------------
BASE_EXPORT BOOL ENGINEAPI InitBase(TSTR lpConfig);
BASE_EXPORT int  ENGINEAPI BaseLoop(BOOL bRealTime=TRUE);
BASE_EXPORT int  ENGINEAPI EnableRealTime(BOOL bEnable);
BASE_EXPORT void ENGINEAPI TerminateBase();
BASE_EXPORT void ENGINEAPI TerminateEngines();
BASE_EXPORT void ENGINEAPI EndProgram();
void ENGINEAPI CriticalExit();

BASE_EXPORT double ENGINEAPI SetTimeSpeed(double Speed);
BASE_EXPORT void   ENGINEAPI PauseTime(BOOL bPause);
BASE_EXPORT DWORD  ENGINEAPI TrackTimeBegin(BOOL bGameTimer=TRUE);
BASE_EXPORT DWORD  ENGINEAPI TrackTimeEnd(DWORD timeID);
BASE_EXPORT DWORD  ENGINEAPI TrackTimeRestart(DWORD timeID);

BASE_EXPORT CTSTR  ENGINEAPI GetBinDir();

BASE_EXPORT void __cdecl Logva(const TCHAR *format, va_list argptr);
BASE_EXPORT void __cdecl Log(const TCHAR *format, ...);
BASE_EXPORT void __cdecl CrashError(const TCHAR *format, ...);
BASE_EXPORT void __cdecl AppWarning(const TCHAR *format, ...);
#define ErrOut CrashError

BASE_EXPORT void ENGINEAPI TraceCrash(const TCHAR *trackName);
BASE_EXPORT void ENGINEAPI TraceCrashEnd();


//-----------------------------------------
//defines
//-----------------------------------------
#define MIN(a, b)               (((a) < (b)) ? (a) : (b))
#define MAX(a, b)               (((a) > (b)) ? (a) : (b))
#define MAKEDWORD(low, high)    ((DWORD)(low) | ((DWORD)(high) << 16))
#define MAKEQUAD(low, high)     ((QWORD)(low) | ((QWORD)(high) << 32))
#define LODW(quad)              (DWORD)(quad)
#define HIDW(quad)              (DWORD)((quad) >> 32)
#define HIWORD(l)               ((WORD)((l) >> 16))
#define LOWORD(l)               ((WORD)(l))
#ifdef _DEBUG
    #define assert(check)           if(!(check)) CrashError(TEXT("Assertion Failiure: (") TEXT(#check) TEXT(") failed\r\nFile: %s, line %d"), TEXT(__FILE__), __LINE__);
    #define assertmsg(check, msg)   if(!(check)) CrashError(TEXT("Assertion Failiure: %s\r\nFile: %s, line %d"), (TSTR)msg, TEXT(__FILE__), __LINE__);
#else
    #define assert(check)
    #define assertmsg(check, msg)
#endif


//-----------------------------------------
//includes
//-----------------------------------------
#include "Inline.h"
#include "TargaHeader.h"
#include "Alloc.h"
#include "FastAlloc.h"
#include "DebugAlloc.h"
#include "Serializer.h"
#include "Template.h"
#include "utf8.h"
#include "EngineString.h"
#include "Profiler.h"
#include "ClassID.h"
#include "EngineLocalization.h"
#include "Script.h"
#include "EngineMath.h"
#include "Geometry.h"
#include "Obj.h"
#include "ScriptLists.h"
#include "ConfigFile.h"
#include "SoundSystem.h"
#include "Window.h"
#include "GraphicsSystem.h"
#include "Input.h"
#include "XFile.h"
#include "XConfig.h"
#if defined WIN32
#include "InputWindows.h"
#include "DXInput.h"
#elif defined __UNIX__
#include "InputUnix.h"
#endif
#include "Music.h"
#include "Sprite2D.h"
#include "Material.h"
#include "Physics.h"
#include "Mesh.h"
#include "Entity.h"
#include "MeshEntity.h"
#include "AnimatedEntity.h"
#include "Prefab.h"
#include "TexturedLine.h"
#include "Sprite3D.h"
#include "ParticleEmitter.h"
#include "Particle.h"
#include "LenseFlare.h"
#include "Camera.h"
#include "Viewport.h"
#include "Controller.h"
#include "Character.h"
#include "PathFinding.h"
#include "Projector.h"
#include "Engine.h"
#include "Level.h"
#include "IndoorLevel.h"
#include "OutdoorLevel.h"
#include "SpaceLevel.h"
#include "OctLevel.h"
#include "Light.h"
#include "Game.h"
#include "GameModule.h"
#include "AINode.h"
#include "Trigger.h"
#include "GroundTrigger.h"

#include "ResourceManager.h"

#include "Button.h"
#include "CheckBox.h"
#include "EditBox.h"
#include "Listbox.h"
#include "Slider.h"
#include "StaticText.h"


//-----------------------------------------
//externs
//-----------------------------------------
BASE_EXPORT extern ConfigFile   *AppConfig;
BASE_EXPORT extern BOOL         bExiting;
BASE_EXPORT extern BOOL         bErrorMessage;



#endif

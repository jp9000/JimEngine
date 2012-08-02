/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Xed.h:  Main header

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


#pragma once

/*==============================================================
  Main Headers
===============================================================*/

//#define _WIN32_WINDOWS  0x0410
//#define _WIN32_WINNT    0x0400
//#define WIN32_LEAN_AND_MEAN

#pragma warning(disable : 4800)

#define TokenType TokenTypeWindowsUsesWhichIsPrettyCrappyIfYouAskMe
#define FUNC_STATIC FUNC_STATIC_WHY_MUST_I_HAVE_TO_DO_THIS_WHY_DOES_WINDOWS_HAVE_TO_FRICKEN_HAUNT_ME_TO_THIS_EXTENT

#include <windows.h>
#include <commctrl.h>
#include <Commdlg.h>
#include <Richedit.h>
#include "..\resource.h"

#undef TokenType
#undef FUNC_STATIC

#include <Base.h>
#include "ColorControl.h"
#include "UpDownControl.h"
#include <squish.h>


typedef List<DWORD> IDList;


/*==============================================================
  Editor Includes
===============================================================*/

#include "Triangulator.h"
#include "TextureAdjust.h"
#include "ObjectBrowser.h"
#include "Splitter.h"
#include "EditorViewport.h"
#include "EditorObject.h"
#include "Manipulator.h"
#include "DefaultManipulator.h"
#include "PositionManipulator.h"
#include "RotationManipulator.h"
#include "SelectionBox.h"
#include "UndoRedoStack.h"
#include "EditorMesh.h"
#include "EditorBrush.h"
#include "EditorEngine.h"
#include "EditorLevelInfo.h"
#include "ObjectCreator.h"
#include "MaterialWindow.h"
#include "MaterialEditor.h"
#include "ObjectPropertiesEditor.h"
#include "SurfacePropertiesEditor.h"
#include "ShapeEditor.h"
#include "MeshBrowser.h"
#include "PrefabBrowser.h"
#include "ModuleManagement.h"
#include "ScriptEditor.h"




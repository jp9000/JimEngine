/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  AnimSeq.h

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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#pragma once

/*==================================================================
  Headers
===================================================================*/
#pragma warning( disable : 4819 )

#include <Max.h>
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <CS\bipexp.h>
#include <CS\phyexp.h>
#include <iskin.h>
#include <utilapi.h>
#include <decomp.h>
#include <modstack.h>
#include "resource.h"


/*==================================================================
  Functions/Externs/Structs
===================================================================*/

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

BOOL __stdcall IsGun(INode *node);

//---------------------------------------------

struct ANIM
{
    Tab<Quat>      rotKey;
    Tab<Point3>    posKey;
};

//---------------------------------------------

struct OLDNAME
{
    char  name[128];
    BOOL  bNoXYTransform;
    BOOL  bNoZTransform;
    BOOL  bNoRotation;
    BOOL  bStillAnimation;
    DWORD keyframeLength;
};
typedef Tab<OLDNAME> OLDNAMELIST;

struct NAME
{
    wchar_t name[128];
    BOOL  bNoXYTransform;
    BOOL  bNoZTransform;
    BOOL  bNoRotation;
    BOOL  bStillAnimation;
    DWORD keyframeLength;
};
typedef Tab<NAME> NAMELIST;

//---------------------------------------------

struct FROMTO
{
    int begin;
    int end;
};
typedef Tab<FROMTO> FTLIST;

//---------------------------------------------

struct NEWSEQUENCEINFO
{
    NAME name;
};



/*==================================================================
  AnimSeq class
===================================================================*/

class AnimSeq : public UtilityObj {
	public:



		HWND			hPanel;
		IUtil			*iu;
		Interface		*ip;
		
		void BeginEditParams(Interface *ip,IUtil *iu);
		void EndEditParams(Interface *ip,IUtil *iu);
        void SelectionSetChanged(Interface *ip,IUtil *iu);

		void Init(HWND hWnd);
		void Destroy(HWND hWnd);
		

		void DeleteThis() { }		
		//Constructor/Destructor

		AnimSeq();
		~AnimSeq();		

        //keyframe functions
        void            SetStartingFrame(DWORD Frame)       {tStart = Frame;}
        void            SetEndFrame(DWORD Frame)            {tStop = Frame*GetTicksPerFrame();}
        void            SetKeyframeStep(DWORD num)          {step = num;}
        DWORD           GetStartingFrame()                  {return tStart;}
        DWORD           GetStartTime()                      {return tStart*GetTicksPerFrame();}
        DWORD           GetEndTime()                        {return tStop;}
        DWORD           GetKeyframeStep()                   {return step;}

        void UpdateSequence(INode *node, int id);
        void AddSequence(INode *node);
        void RemoveSequence(INode *node, int id);
        void GetOldNames(OLDNAMELIST *names);
        void GetNames(NAMELIST *names);
        void SetNames(NAMELIST *names);
        void Refresh();
        void LoadNames();
        void SaveStartData(BOOL bStarting=0);

        void ClearAllDamnDataDamnit(INode *curNode);

//    private:

        void SetData(INode *node, DWORD dwID, DWORD dwSize, void *d);
        void RemoveData(INode *node, DWORD dwID);
        AppDataChunk* GetData(INode *node, DWORD dwID);
        BOOL IsBone(INode *bone);
        void SetAnimated(BOOL bAnimated);

        Matrix3 GetBoneInitialTM(INode *bone);

        void GetSkins();

        void GetRotSamples(INode *node, ANIM &anim);
        void GetPosSamples(INode *node, ANIM &anim);

        Tab<Modifier*> curSkins;


        DWORD               tStart;
        DWORD               tStop;
        DWORD               step;
};

extern AnimSeq theAnimSeq;

#define ANIMSEQ_CLASS_ID	Class_ID(0x5ac1bba9, 0x732224f5)

class AnimSeqClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &theAnimSeq; }
	const MCHAR *	ClassName() { return _M("AnimSeq"); }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return ANIMSEQ_CLASS_ID; }
	const MCHAR* 	Category() { return _M(""); }

	const MCHAR*	InternalName() { return _M("AnimSeq"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

};

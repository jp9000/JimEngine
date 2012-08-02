/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  xmdExp.h

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
#pragma warning(disable : 4819)

#define _UNICODE_MODULE_FOR_MBCS_MAX

#include <Max.h>
#include <maxapi.h>
#include "resource.h"
#include <istdplug.h>
#include <iparamb2.h>
#include <iparamm2.h>
#include <iskin.h>
#include <stdmat.h>
#include <CS\bipexp.h>
#include <CS\phyexp.h>
#include <modstack.h>
//#include <IGame\IGame.h>
//#include <IGame\IGameModifier.h>
#include <decomp.h>
#include <MNMATH.H>
#include <MESHDLIB.H>
//#include <Max_Mem.h>

typedef Quat MaxQuat;

#define LEAVE_NEW
#include <XT.h>
using namespace XT;



#pragma warning(disable : 4244)
#pragma warning(disable : 4018)




bool __stdcall IsGun(INode *node);


#define PointWithinBounds(bounds, point) \
        ((point.x >= bounds.Min.x) && \
        (point.x <= bounds.Max.x) && \
        (point.y >= bounds.Min.y) && \
        (point.y <= bounds.Max.y) && \
        (point.z >= bounds.Min.z) && \
        (point.z <= bounds.Max.z))


#define TriWithinBounds(bounds, tri) \
        PointWithinBounds(bounds, VertList[tri.A]) && \
        PointWithinBounds(bounds, VertList[tri.B]) && \
        PointWithinBounds(bounds, VertList[tri.C])




/*=====================================================================
  3dsmax Conversion functions
======================================================================*/

Point3 GetPoint3(const Vect &v);
Vect GetVect(const Point3 &v);
Vect GetNorm(const Point3 &v);
Vect GetUVCoord3(const Point3 &v);
UVCoord GetUVCoord2(const Point3 &v);
UVVert GetUVVert(const UVCoord &uv);
XT::Quat GetXenQuat(MaxQuat &q);



/*=====================================================================
  temp stuff
======================================================================*/
struct FACEINFO
{
    int smoothgroup;
    int matID;
};

struct VERTDATA
{
    DWORD  vert;
    DWORD  tvert;
    DWORD  tvert2;
    DWORD  matID;
    DWORD sg;
    BOOL  bTVMirrored;
};


/*=====================================================================
  main file header
======================================================================*/

struct MODELHEAD
{
    DWORD dwSigniture;  //must be '\0dmx'  ('xmd')
    DWORD nVerts;
    DWORD nFaces;
    DWORD nEdges;
    DWORD nMaterials;
};

struct ANIMHEAD
{
    DWORD dwSigniture;  //must be '\0nax'  ('xan')
    DWORD nSequences;
    DWORD nBones;
    DWORD nExtensions;
};

//////////////
//face
struct FACE
{
    union
    {
        struct
        {
            DWORD A;
            DWORD B;
            DWORD C;
        };
        DWORD ptr[3];
    };
};

struct EDGE
{
    union
    {
        struct
        {
            DWORD v1;
            DWORD v2;
        };
        DWORD v[2];
    };
    union
    {
        struct
        {
            DWORD f1;
            DWORD f2;
        };
        DWORD f[2];
    };
};




//////////////
//bone header
#define BONE_ROOT     1

struct BONE
{
    Vect   Pos;
    //union
    //{
        XT::Quat  Rot;
/*        struct
        {
            Vect  axis;
            float angle;
        };
    };*/
    //vert info
    DWORD   nRigidVerts;    //rigid   (weight=1)
    DWORD   nBlendedVerts;  //blended (weight<1)

    //flags
    BYTE   flags;

    //other
    DWORD   idParent;
};
typedef BONE *LPBONE;

struct WEIGHT
{
    DWORD vert;
    float weight;
};

//////////////
//Material
#define MATERIAL_TWOSIDED    1
#define MATERIAL_TRANSPARANT 2
#define MATERIAL_ENVIRONMENT 4

struct MATERIALNAME
{
    char  name[128];
};

struct MATERIAL
{
    DWORD startFace; //starting face which material is applied
    DWORD nFaces;    //number of mapped faces
};


/////////////
//animation sequencing
struct KEYDATA
{
    int sequence;
    int size;
    Class_ID type;
    BYTE lpData[128];
};
typedef List<KEYDATA> KEYLIST;

//---------------------------------------

struct NAME
{
    wchar_t name[128];
    BOOL  bNoXYTransform;
    BOOL  bNoZTransform;
    BOOL  bNoRotation;
    BOOL  bStillAnimation;
    DWORD keyframeLength;
};
typedef List<NAME> NAMELIST;

//---------------------------------------

struct FROMTO
{
    int begin;
    int end;
};
typedef List<FROMTO> FTLIST;

//---------------------------------------

struct ANIM
{
    List<XT::Quat>   rotKey;
    List<Vect>   posKey;
};

#define SEQUENCE_STILL 1

//---------------------------------------

struct SEQ
{
    char name[128];
    DWORD nFrames;
    DWORD keyframeTime;
    BYTE flags;
};
typedef List<SEQ> SEQLIST;

///////////////
//temp bone structure
struct BONEDATA
{
    BONE            bone;
    INode           *node;
	MaxQuat         Rot;
    Matrix3         starttm;
    Point3          firstPos;
	BONEDATA        *Parent;
    List<DWORD>      rigidVerts;
    List<WEIGHT>    blendedVerts;
    List<ANIM*>     AnimKeyList;
};

typedef BONEDATA *LPBONEDATA;

typedef VERTDATA VECTBAK;
typedef List<VECTBAK>  VLIST;

struct TempMaterialData
{
    DWORD id;
    DWORD startFaceID;
};

struct BoneExtension
{
    String      name;
    DWORD       parentBone;
    Vect        Pos;
    XT::Quat   Rot;

    inline void FreeData() {name.Clear();}

    friend static Serializer& operator<<(Serializer &s, BoneExtension &extension)
    {
        return s << extension.name << extension.parentBone << extension.Pos << extension.Rot;
    }
};


///////////////
//exporter classes

class xmdExp : public SceneExport {
	public:


		static HWND hParams;


        int				ExtCount()              {return 1;}
        const MCHAR *	Ext(int n) 				{return (MCHAR*)_M("xmd");}                     //......better not to ask.  3dsmax retardation
        const MCHAR *	LongDesc() 				{return (MCHAR*)_M("Xen Model File");}
        const MCHAR *	ShortDesc() 			{return (MCHAR*)_M("Xen Model");}
        const MCHAR *	AuthorName() 			{return (MCHAR*)_M("Hugh Bailey");}
        const MCHAR *	CopyrightMessage() 		{return (MCHAR*)_M("Copyright (c) 2001");}
        const MCHAR *	OtherMessage1() 		{return (MCHAR*)_M("");}
		const MCHAR *	OtherMessage2() 		{return (MCHAR*)_M("");}
        unsigned int	Version() 				{return 99;}
        void			ShowAbout(HWND hWnd) 	{}
        BOOL            SupportsOptions(int ext, DWORD options) {return TRUE;}

		int				DoExport(const MCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);
		void            ProcessNode(INode *node);
        Interface *     GetInterface() {return ip;}
        void            GetGeometry(INode *node);
        BOOL            TMNegParity(Matrix3 &m);
        Point3          GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
        void            SaveModel(CTSTR lpFile);
        void            SaveAnimation(CTSTR lpFile);
		unsigned short	FindVertex(unsigned short vert);
        void            SaveAscii(CTSTR lpFile);

        //keyframe functions
        void            SetStartingFrame(DWORD Frame)       {tStart = Frame;}
        void            SetEndFrame(DWORD Frame)            {tStop = Frame*GetTicksPerFrame();}
        void            SetKeyframeStep(DWORD num)          {step = num;}
        DWORD           GetStartingFrame()                  {return tStart;}
        DWORD           GetStartTime()                      {return tStart*GetTicksPerFrame();}
        DWORD           GetEndTime()                        {return tStop;}
        DWORD           GetKeyframeStep()                   {return step;}

        //basic bone functions
        void            GetSkinData(INode *node);
        void            GetPhysiqueData(INode *node);
        void            ExportBones();//INode *parentNode=NULL);
        void            ExportBoneData(INode *bone, DWORD idParent, Matrix3 *lpT=NULL);
        BONEDATA *      GetBoneByNode(INode *node);

        //vertex weight functions
        void            SaveBlendedVert(BONEDATA* bone, int vertex, float weight);
        void            SaveRigidVert(BONEDATA* bone, int vertex);

        //bone keyframe functions
        BOOL            CheckForAnimation(INode* node, int id, BOOL& bPos, BOOL& bRot);
        void            GetRotSamples(BONEDATA *bone, ANIM* anim);
        void            GetPosSamples(BONEDATA *bone, ANIM* anim);

        //animation sequencer data retreival functions
        void ExportAnimation();
        AppDataChunk* GetData(INode *node, DWORD dwID);
        void GetNames(NAMELIST *names);
        void GetFTData(FTLIST *lpFTList);  //from/to data
        void LoadSequencesFromNode(INode *node, KEYLIST *lpSeqList);
        void LoadSequence(INode *node, int id);
        void LoadScaleKeys(Control* cont, KEYLIST &SeqList, int id);
        void LoadRotKeys(Control* cont, KEYLIST &SeqList, int id);
        void LoadPosKeys(Control* cont, KEYLIST &SeqList, int id);
        BOOL HasAnimation();
        Matrix3 GetMeshInitTM(INode* mesh);
        Matrix3 GetBoneInitTM(INode* bone);
        void RefreshMesh();
        void DeleteAllKeys(Control *cont);
        void GetWorldUVPosFromFace(DWORD face, const Vect2 &UV, Vect &Pos);
        BOOL IsBone(INode *bone);

        void FreeData();


        //Constructor/Destructor

		xmdExp();
		~xmdExp();		

    //file headers
    MODELHEAD           modelHead;
    ANIMHEAD            animHead;
    BOOL                bAsciiOut;
    BOOL                bHasAnimation;

    float               rotation;

    List<Modifier*>     SkinModifiers;

private:
	HANDLE              hOut;
	Interface          *ip;
    DWORD               tStart;
    DWORD               tStop;
    DWORD               step;
    DWORD               nVerts;

    //main info
    List<Vect>          VertList;
    List<UVCoord>       UVList;
    List<UVCoord>       LMCoords;
    List<Vect>          TempFaceTangentUList;
    List<Vect>          VertexTangentUList;
    List<Vect>          NormalList;
    List<FACE>          FaceList;
    List<DWORD>         FacePlaneList;
    List<Plane>         PlaneList;
    List<EDGE>          EdgeList;
    List<FACEINFO>      FaceInfo;
    List<MATERIALNAME>  MaterialNameList;
    List<MATERIAL>      MaterialList;
    List<TempMaterialData> TempMaterialDataList;
    Bounds              bounds;
    NAMELIST            namelist;
    FTLIST              ftlist;
    SEQLIST             SequenceList;

    BOOL                bHasLMCoords;

    List<BoneExtension> ExtensionList;

    //bone stuff
    List<BONEDATA>      BoneList;

    //temp data
    List<VLIST>         VertSave;
    List<DWORD>         FaceSave;
};


class xbrExp : public SceneExport {
	public:


		static HWND hParams;


        int				ExtCount()              {return 1;}
        const MCHAR *	Ext(int n) 				{return (MCHAR*)_M("xbr");}
        const MCHAR *	LongDesc() 				{return (MCHAR*)_M("Xen Brush File");}
        const MCHAR *	ShortDesc() 			{return (MCHAR*)_M("Xen Brush");}
        const MCHAR *	AuthorName() 			{return (MCHAR*)_M("Hugh Bailey");}
        const MCHAR *	CopyrightMessage() 		{return (MCHAR*)_M("Copyright (c) 2001");}
        const MCHAR *	OtherMessage1() 		{return (MCHAR*)_M("");}
		const MCHAR *	OtherMessage2() 		{return (MCHAR*)_M("");}
        unsigned int	Version() 				{return 99;}
        void			ShowAbout(HWND hWnd) 	{}
        BOOL            SupportsOptions(int ext, DWORD options) {return TRUE;}

		int				DoExport(const MCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);
        Interface *     GetInterface() {return ip;}

		xbrExp();
		~xbrExp();		

private:
	    Interface       *ip;
};

#define XMDEXP_CLASS_ID	Class_ID(0x4500c11, 0x2b22d354)
#define XBREXP_CLASS_ID	Class_ID(0x790c7ccd, 0x5d083313)

TriObject* GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

class xmdExpClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new xmdExp(); }
	const MCHAR *	ClassName() { return _M("xmdExp"); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return XMDEXP_CLASS_ID; }
	const MCHAR* 	Category() { return _M(""); }

	const MCHAR*	InternalName() { return _M("xmdExp"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

class xbrExpClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new xbrExp(); }
	const MCHAR *	ClassName() { return _M("xbrExp"); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return XBREXP_CLASS_ID; }
	const MCHAR* 	Category() { return _M(""); }

	const MCHAR*	InternalName() { return _M("xbrExp"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

//////////////
//ID defines
#define NAMENUM_ID  4099
#define NAMEDATA_ID 4100

#define ROTNUM_ID   5000
#define ROTDATA_ID  6000

#define POSNUM_ID   7000
#define POSDATA_ID  8000

#define ANIMATED_ID 4102
#define TIME_ID     4104

#define ANIMSEQ_CLASS_ID	Class_ID(0x5ac1bba9, 0x732224f5)


#undef BitTest
#define BitTest(Bla, bit)  (Bla & bit)


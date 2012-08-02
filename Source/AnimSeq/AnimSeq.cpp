/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  AnimSeq.cpp

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


#include "AnimSeq.h"


/*==================================================================
  Defines
===================================================================*/

#define NAMENUM_ID  4099
#define NAMEDATA_ID 4100

#define ANIMATED_ID 4102
#define TIME_ID     4104

#define ROTNUM_ID   5000
#define ROTDATA_ID  6000

#define POSNUM_ID   7000
#define POSDATA_ID  8000



/*==================================================================
  Globals
===================================================================*/




int beginValue,endValue;
BOOL bHasStartTransform=0;



/*==================================================================
  Max Stuff
===================================================================*/

static AnimSeqClassDesc AnimSeqDesc;

AnimSeq theAnimSeq;

ClassDesc2* GetAnimSeqDesc() { return &AnimSeqDesc; }


static BOOL CALLBACK NameDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
Modifier* GetAnimationModifier(INode* nodePtr);




static BOOL CALLBACK NameDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ISpinnerControl *spin;

    switch(msg)
    {
        case WM_INITDIALOG:
            SetWindowLong(hWnd, GWL_USERDATA, lParam);
            SendMessage(GetDlgItem(hWnd, ID_NAME), EM_LIMITTEXT, 15, 0);

            spin = GetISpinner(GetDlgItem(hWnd, ID_KEYFRAMELENGTH_SPIN));
            spin->LinkToEdit(GetDlgItem(hWnd, ID_KEYFRAMELENGTH), EDITTYPE_FLOAT);
            spin->SetLimits(0.1f, 100.0f, TRUE);
            spin->SetScale(0.1f);
            spin->SetValue(1.0f, FALSE);
            ReleaseISpinner(spin);

            return 1;

        case WM_COMMAND:
            if(LOWORD(wParam) == IDOK)
            {
                NEWSEQUENCEINFO *nsi = (NEWSEQUENCEINFO*)GetWindowLong(hWnd, GWL_USERDATA);

                SendMessage(GetDlgItem(hWnd, ID_NAME), WM_GETTEXT, 127, (LPARAM)nsi->name.name);
                nsi->name.bNoXYTransform  = IsDlgButtonChecked(hWnd, IDC_NOXYTRANSFORM);
                nsi->name.bNoZTransform   = IsDlgButtonChecked(hWnd, IDC_NOZTRANSFORM);
                nsi->name.bNoRotation     = IsDlgButtonChecked(hWnd, IDC_NOROTATION);
                nsi->name.bStillAnimation = IsDlgButtonChecked(hWnd, IDC_STILLANIMATION);

                spin = GetISpinner(GetDlgItem(hWnd, ID_KEYFRAMELENGTH_SPIN));

                nsi->name.keyframeLength = (DWORD)(spin->GetFVal()*(float)GetTicksPerFrame());

                ReleaseISpinner(spin);

                EndDialog(hWnd, IDOK);
            }
            else if(LOWORD(wParam) == IDCANCEL)
                EndDialog(hWnd, IDCANCEL);
    }
    return 0;
}

static BOOL CALLBACK AnimSeqDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ISpinnerControl *spin;
    Interval animRange;
    BOOL bYes = 1;

	switch (msg) {
		case WM_INITDIALOG:
			theAnimSeq.Init(hWnd);

            theAnimSeq.hPanel = hWnd;

            theAnimSeq.SaveStartData(1);

            spin = GetISpinner(GetDlgItem(hWnd, ID_BEGIN_SPIN));
            spin->LinkToEdit(GetDlgItem(hWnd, ID_BEGIN), EDITTYPE_INT);
            animRange = theAnimSeq.ip->GetAnimRange();
            spin->SetLimits(animRange.Start() / GetTicksPerFrame(), animRange.End() / GetTicksPerFrame(), TRUE);
            spin->SetScale(1.0f);
            spin->SetValue(0, FALSE);
            ReleaseISpinner(spin);

            spin = GetISpinner(GetDlgItem(hWnd, ID_END_SPIN));
            spin->LinkToEdit(GetDlgItem(hWnd, ID_END), EDITTYPE_INT);
            animRange = theAnimSeq.ip->GetAnimRange();
            spin->SetLimits((animRange.Start() / GetTicksPerFrame())+1, animRange.End() / GetTicksPerFrame(), TRUE);
            spin->SetScale(1.0f);
            spin->SetValue(animRange.End() / GetTicksPerFrame(), FALSE);
            ReleaseISpinner(spin);

            break;

        case CC_SPINNER_CHANGE:
            spin = GetISpinner(GetDlgItem(hWnd, ID_BEGIN_SPIN));
            beginValue = spin->GetIVal();
            ReleaseISpinner(spin);

            spin = GetISpinner(GetDlgItem(hWnd, ID_END_SPIN));
            endValue = spin->GetIVal();
            ReleaseISpinner(spin);

            switch(LOWORD(wParam))
            {
                case ID_BEGIN_SPIN:
                    if((beginValue+1) > endValue)
                    {
                        spin = GetISpinner(GetDlgItem(hWnd, ID_BEGIN_SPIN));
                        spin->SetValue((float)endValue-1, TRUE);
                        beginValue = endValue-1;
                        ReleaseISpinner(spin);
                    }
                    break;
                case ID_END_SPIN:
                    if(beginValue > (endValue-1))
                    {
                        spin = GetISpinner(GetDlgItem(hWnd, ID_END_SPIN));
                        spin->SetValue((float)beginValue+1, TRUE);
                        endValue = beginValue+1;
                        ReleaseISpinner(spin);
                    }
                    break;
            }
            break;

		case WM_DESTROY:
			theAnimSeq.Destroy(hWnd);
			break;


		case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_ADD:
                {
                    spin = GetISpinner(GetDlgItem(hWnd, ID_BEGIN_SPIN));
                    beginValue = spin->GetIVal();
                    ReleaseISpinner(spin);

                    spin = GetISpinner(GetDlgItem(hWnd, ID_END_SPIN));
                    endValue = spin->GetIVal();
                    ReleaseISpinner(spin);

                    NAMELIST nm;
                    NEWSEQUENCEINFO bla;
                    int count = SendMessage(GetDlgItem(hWnd, ID_LIST), LB_GETCOUNT,  0, 0);

                    memset(&nm, 0, sizeof(NAMELIST));

                    if(DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_GETNAME), theAnimSeq.ip->GetMAXHWnd(), NameDlgProc, (LPARAM)&bla) == IDOK)
                    {
                        theAnimSeq.GetNames(&nm);

                        nm.Append(1, &bla.name);

                        /*for(int i=0;i<count;i++)
                        {
                            memset(bla.name, 0, 50);
                            nm << bla;
                        }*/
                        theAnimSeq.SetNames(&nm);

                        theAnimSeq.SetKeyframeStep(bla.name.keyframeLength);

                        theAnimSeq.GetSkins();
                        theAnimSeq.AddSequence(theAnimSeq.ip->GetRootNode());
                        theAnimSeq.curSkins.ZeroCount();

                        //SendMessage(GetDlgItem(hWnd, ID_LIST), WM_GETTEXT,  50, (LPARAM)bla.name);
                        SendMessage(GetDlgItem(hWnd, ID_LIST), LB_ADDSTRING, 0, (LPARAM)bla.name.name);

                        theAnimSeq.SetAnimated(SendMessage(GetDlgItem(hWnd, ID_LIST), LB_GETCOUNT, 0, 0));

                        nm.ZeroCount();
                    }

                    break;
                }
                case ID_DELETE:
                {
                    NAMELIST nm;
                    TCHAR msgText[128],name[128];
                    int count = SendMessage(GetDlgItem(hWnd, ID_LIST), LB_GETCOUNT,  0, 0);
                    int id    = SendMessage(GetDlgItem(hWnd, ID_LIST), LB_GETCURSEL, 0, 0);

                    if(id == LB_ERR) break;

                    SendMessage(GetDlgItem(hWnd, ID_LIST), LB_GETTEXT, id, (LPARAM)name);
                    wsprintf(msgText, TEXT("Are you sure you want to delete '%s'?"), name);
                    if(MessageBox(theAnimSeq.ip->GetMAXHWnd(), msgText, TEXT("Confirm Deletion"), MB_ICONEXCLAMATION|MB_YESNO) == IDNO)
                        break;

                    memset(&nm, 0, sizeof(NAMELIST));

                    theAnimSeq.GetNames(&nm);

                    nm.Delete(id, 1);

                    /*for(int i=0;i<count;i++)
                    {
                        if(i == id) continue;
                        memset(bla.name, 0, 50);
                        SendMessage(GetDlgItem(hWnd, ID_LIST), LB_GETLBTEXT, i, (LPARAM)bla.name);
                        nm << bla;
                    }*/
                    theAnimSeq.SetNames(&nm);

                    theAnimSeq.RemoveSequence(theAnimSeq.ip->GetRootNode(), id);

                    SendMessage(GetDlgItem(hWnd, ID_LIST), LB_DELETESTRING, id, 0);

                    theAnimSeq.SetAnimated(SendMessage(GetDlgItem(hWnd, ID_LIST), LB_GETCOUNT, 0, 0));

                    nm.ZeroCount();

                    break;
                }
                case ID_RESET:
                    theAnimSeq.Refresh();
                    break;

                case ID_CLEARALLDAMNDATADAMNIT:
                    theAnimSeq.ClearAllDamnDataDamnit(theAnimSeq.ip->GetRootNode());
                    theAnimSeq.SetAnimated(FALSE);
                    theAnimSeq.SetAnimated(SendMessage(GetDlgItem(hWnd, ID_LIST), LB_RESETCONTENT, 0, 0));
                    break;

                case ID_SETRESET:
                    theAnimSeq.SaveStartData();
                    break;

                case ID_UPDATE:
                {
                    TCHAR msgText[256],name[256];
                    int id    = SendMessage(GetDlgItem(hWnd, ID_LIST), LB_GETCURSEL, 0, 0);
                    if(id == LB_ERR) break;

                    SendMessage(GetDlgItem(hWnd, ID_LIST), LB_GETTEXT, id, (LPARAM)name);
                    wsprintf(msgText, TEXT("Are you sure you want to update '%s'?"), name);
                    if(MessageBox(theAnimSeq.ip->GetMAXHWnd(), msgText, TEXT("Confirm Update"), MB_ICONEXCLAMATION|MB_YESNO) == IDNO)
                        break;

                    theAnimSeq.UpdateSequence(theAnimSeq.ip->GetRootNode(), id);
                    break;
                }
            }
			break;


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theAnimSeq.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


Modifier* GetAnimationModifier(INode* nodePtr)
{
    if(!nodePtr) return NULL;

	Object* ObjectPtr = nodePtr->GetObjectRef();
	if (!ObjectPtr) return NULL;

	while (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID && ObjectPtr)
	{
		IDerivedObject *DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);
						
		int ModStackIndex = 0;
		while (ModStackIndex < DerivedObjectPtr->NumModifiers())
		{
			Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

			if((ModifierPtr->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B)) ||
			   (ModifierPtr->ClassID() == SKIN_CLASSID) )
				return ModifierPtr;

			ModStackIndex++;
		}
		ObjectPtr = DerivedObjectPtr->GetObjRef();
	}

	return NULL;
}


/*==================================================================
  AnimSeq Class
===================================================================*/

AnimSeq::AnimSeq()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

AnimSeq::~AnimSeq()
{

}

void AnimSeq::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	ip->AddRollupPage(
		hInstance,
		(MCHAR*)IDD_PANEL,
		AnimSeqDlgProc,
        _M("Parameters"),
        (LPARAM)this);
}

void AnimSeq::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}


void AnimSeq::SelectionSetChanged(Interface *ip,IUtil *iu)
{
    ISpinnerControl *spin;
    Interval animRange;

    UtilityObj::SelectionSetChanged(ip, iu);

    LoadNames();

    spin = GetISpinner(GetDlgItem(hPanel, ID_BEGIN_SPIN));
    spin->LinkToEdit(GetDlgItem(hPanel, ID_BEGIN), EDITTYPE_INT);
    animRange = ip->GetAnimRange();
    spin->SetLimits(animRange.Start() / GetTicksPerFrame(), animRange.End() / GetTicksPerFrame(), TRUE);
    ReleaseISpinner(spin);

    spin = GetISpinner(GetDlgItem(hPanel, ID_END_SPIN));
    spin->LinkToEdit(GetDlgItem(hPanel, ID_END), EDITTYPE_INT);
    animRange = ip->GetAnimRange();
    spin->SetLimits((animRange.Start() / GetTicksPerFrame())+1, animRange.End() / GetTicksPerFrame(), TRUE);
    ReleaseISpinner(spin);

    AppDataChunk *data = GetData(ip->GetRootNode(), ANIMATED_ID);
    if(!data)
    {
        if(SendMessage(GetDlgItem(hPanel, ID_LIST), LB_GETCOUNT, 0, 0))
            SetAnimated(TRUE);
    }
    else
    {
        if(!SendMessage(GetDlgItem(hPanel, ID_LIST), LB_GETCOUNT, 0, 0))
            SetAnimated(FALSE);
    }
}

void AnimSeq::Init(HWND hWnd)
{
}

void AnimSeq::Destroy(HWND hWnd)
{

}

void AnimSeq::GetSkins()
{
    for(int c=0; c<ip->GetRootNode()->NumberOfChildren(); c++)
    {
        INode *curNode = ip->GetRootNode()->GetChildNode(c);
        Modifier* skin = GetAnimationModifier(curNode);

        if(skin)
            curSkins.Append(1, &skin);
    }
}

Matrix3 AnimSeq::GetBoneInitialTM(INode *bone)
{
    Matrix3 output;

    for(int i=0; i<curSkins.Count(); i++)
    {
        Modifier *mod = curSkins[i];
        if(mod->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
        {
            IPhysiqueExport *phy = (IPhysiqueExport*)mod->GetInterface(I_PHYINTERFACE);

            if(phy && (phy->GetInitNodeTM(bone, output) == MATRIX_RETURNED))
                return output;
        }
        else if(mod->ClassID() == SKIN_CLASSID)
        {
            ISkin *skin = (ISkin*)mod->GetInterface(I_SKIN);

            if(skin && (skin->GetBoneInitTM(bone, output) == SKIN_OK))
                return output;
        }
    }

    output.IdentityMatrix();
    return output;
}

void AnimSeq::UpdateSequence(INode *node, int id)
{
    int i;

    MCHAR *bla = node->GetName();

    if(IsBone(node))
    {
        ANIM anim;
        memset(&anim, 0, sizeof(anim));

        //AppDataChunk *chunk;
        //if(!(chunk = GetData(node, ROTNUM_ID+id))) {MessageBox(NULL, TEXT("Sequence does not exist..?  どういう意味だ？"), TEXT("I am made of cheese"), 0);return;}

        GetRotSamples(node, anim);
        GetPosSamples(node, anim);

        DWORD dwTemp;

        dwTemp = anim.rotKey.Count();
        SetData(node, ROTNUM_ID+id,  4, &dwTemp);
        SetData(node, ROTDATA_ID+id, sizeof(Quat)*dwTemp, anim.rotKey.Addr(0));

        dwTemp = anim.posKey.Count();
        SetData(node, POSNUM_ID+id,  4, &dwTemp);
        SetData(node, POSDATA_ID+id, sizeof(Point3)*dwTemp, anim.posKey.Addr(0));

        anim.posKey.ZeroCount();
        anim.rotKey.ZeroCount();
    }

    for(i=0;i<node->NumberOfChildren();i++)
        UpdateSequence(node->GetChildNode(i), id);
}

void AnimSeq::AddSequence(INode *node)
{
    int i;

    MCHAR *bla = node->GetName();

    if(IsBone(node))
    {
        ANIM anim;
        memset(&anim, 0, sizeof(anim));

        GetRotSamples(node, anim);
        GetPosSamples(node, anim);

        AppDataChunk *chunk;

        for(i=0; chunk = GetData(node, ROTNUM_ID+i); i++);

        DWORD dwTemp;

        dwTemp = anim.rotKey.Count();
        SetData(node, ROTNUM_ID+i,  4, &dwTemp);
        SetData(node, ROTDATA_ID+i, sizeof(Quat)*dwTemp, anim.rotKey.Addr(0));

        dwTemp = anim.posKey.Count();
        SetData(node, POSNUM_ID+i,  4, &dwTemp);
        SetData(node, POSDATA_ID+i, sizeof(Point3)*dwTemp, anim.posKey.Addr(0));

        anim.posKey.ZeroCount();
        anim.rotKey.ZeroCount();
    }

    for(i=0;i<node->NumberOfChildren();i++)
        AddSequence(node->GetChildNode(i));
}

void AnimSeq::RemoveSequence(INode *node, int id)
{
    int i;

    if(IsBone(node))
    {
        AppDataChunk *chunk;

        //if(!(chunk = GetData(node, ROTNUM_ID+id))) {MessageBox(NULL, TEXT("Sequence does not exist..?  どういう意味だ？"), TEXT("I am made of cheese"), 0);return;}

        for(i=id ;; i++)
        {
            chunk = GetData(node, ROTNUM_ID+i+1);
            if(!chunk)
            {
                RemoveData(node, ROTNUM_ID+i);
                RemoveData(node, ROTDATA_ID+i);
                RemoveData(node, POSNUM_ID+i);
                RemoveData(node, POSDATA_ID+i);
                break;
            }
            SetData(node, ROTNUM_ID+i, chunk->length, chunk->data);
            chunk = GetData(node, ROTDATA_ID+i+1);
            if(chunk)
                SetData(node, ROTDATA_ID+i, chunk->length, chunk->data);

            chunk = GetData(node, POSNUM_ID+i+1);
            SetData(node, POSNUM_ID+i, chunk->length, chunk->data);
            chunk = GetData(node, POSDATA_ID+i+1);
            if(chunk)
                SetData(node, POSDATA_ID+i, chunk->length, chunk->data);
        }
    }

    for(i=0;i<node->NumberOfChildren();i++)
        RemoveSequence(node->GetChildNode(i), id);
}

void AnimSeq::GetOldNames(OLDNAMELIST *names)
{
    AppDataChunk *chunk;
    DWORD num;
    INode *root = ip->GetRootNode();


    if(!names) return;

    chunk = GetData(root, NAMENUM_ID);
    if(!chunk) return;
    num = *(DWORD*)chunk->data;

    chunk = GetData(root, NAMEDATA_ID);
    if(!chunk) return;
    names->ZeroCount();
    names->Append(num, (OLDNAME*)chunk->data);
}

void AnimSeq::GetNames(NAMELIST *names)
{
    AppDataChunk *chunk;
    DWORD num;
    INode *root = ip->GetRootNode();


    if(!names) return;

    chunk = GetData(root, NAMENUM_ID);
    if(!chunk) return;
    num = *(DWORD*)chunk->data;

    chunk = GetData(root, NAMEDATA_ID);
    if(!chunk) return;
    names->ZeroCount();
    names->Append(num, (NAME*)chunk->data);
}

void AnimSeq::SetNames(NAMELIST *names)
{
    INode *root = ip->GetRootNode();

    if(!names) return;

    if(names->Count())
    {
        DWORD dwNum = names->Count();
        SetData(root, NAMENUM_ID, 4, (LPVOID)&dwNum);
        SetData(root, NAMEDATA_ID, dwNum*sizeof(NAME), names->Addr(0));
    }
    else
    {
        RemoveData(root, NAMENUM_ID);
        RemoveData(root, NAMEDATA_ID);
    }
}

BOOL AnimSeq::IsBone(INode *bone)
{
    Control *controller = bone->GetTMController();
    Object *obj = bone->EvalWorldState(0).obj;
    Class_ID id,cid;

    /*if(curSkins.Count())
    {
        BOOL bValid = FALSE;
        Matrix3 output;
        for(int i=0; i<curSkins.Count(); i++)
        {
            Modifier *mod = curSkins[i];
            if(mod->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
            {
                IPhysiqueExport *phy = (IPhysiqueExport*)mod->GetInterface(I_PHYINTERFACE);

                if(phy && (phy->GetInitNodeTM(bone, output) == MATRIX_RETURNED))
                {
                    bValid = TRUE;
                    break;
                }
            }
            else if(mod->ClassID() == SKIN_CLASSID)
            {
                ISkin *skin = (ISkin*)mod->GetInterface(I_SKIN);

                if(skin && (skin->GetBoneInitTM(bone, output) == SKIN_OK))
                {
                    bValid = TRUE;
                    break;
                }
            }
        }
        if(!bValid)
            return FALSE;
    }*/

    id = (obj) ? obj->ClassID() : Class_ID(0, 0);
    cid = (controller) ? controller->ClassID() : Class_ID(0, 0);

    return ((id  == Class_ID(BONE_CLASS_ID, 0)) ||
            (cid == BIPBODY_CONTROL_CLASS_ID)   ||
            (cid == BIPSLAVE_CONTROL_CLASS_ID)  ||
            (id  == BONE_OBJ_CLASSID));
}

void AnimSeq::SetData(INode *node, DWORD dwID, DWORD dwSize, void *d)
{
    if(d && dwSize)
    {
        if(GetData(node, dwID))
            RemoveData(node, dwID);

        LPSTR lpData = (LPSTR)MAX_malloc(dwSize);

        memcpy(lpData, d, dwSize);
        node->AddAppDataChunk(ANIMSEQ_CLASS_ID, UTILITY_CLASS_ID, dwID, dwSize, (LPVOID)lpData);
    }
}

void AnimSeq::RemoveData(INode *node, DWORD dwID)
{
    node->RemoveAppDataChunk(ANIMSEQ_CLASS_ID, UTILITY_CLASS_ID, dwID);
}

AppDataChunk* AnimSeq::GetData(INode *node, DWORD dwID)
{
    return node->GetAppDataChunk(ANIMSEQ_CLASS_ID, UTILITY_CLASS_ID, dwID);
}

void AnimSeq::SetAnimated(BOOL bAnimated)
{
    if(bAnimated)
        SetData(ip->GetRootNode(), ANIMATED_ID, sizeof(tm), &bAnimated);
    else
        RemoveData(ip->GetRootNode(), ANIMATED_ID);
}

void AnimSeq::Refresh()
{
    INode *root = ip->GetRootNode();
    TimeValue time=0;
    AppDataChunk *adc;

    adc = GetData(root, TIME_ID);
    if(adc)
        memcpy(&time, adc->data, sizeof(TimeValue));

    ip->SetTime(time);

    ip->RedrawViews(time);
}

void AnimSeq::LoadNames()
{
    INode *root = ip->GetRootNode();
    /*OLDNAMELIST oldnames;

    SendMessage(GetDlgItem(hPanel, ID_LIST), LB_RESETCONTENT, 0, 0);

    GetOldNames(&oldnames);

    NAMELIST names;
    names.SetCount(oldnames.Count());

    for(int i=0; i<names.Count(); i++)
    {
        WStr wstrNewName(oldnames[i].name);
        wcscpy(names[i].name, wstrNewName);
        names[i].bNoRotation = oldnames[i].bNoRotation;
        names[i].bNoXYTransform = oldnames[i].bNoXYTransform;
        names[i].bNoZTransform = oldnames[i].bNoZTransform;
        names[i].bStillAnimation = oldnames[i].bStillAnimation;
        names[i].keyframeLength = oldnames[i].keyframeLength;
    }
    SetNames(&names);
    oldnames.ZeroCount();*/

    NAMELIST names;

    SendMessage(GetDlgItem(hPanel, ID_LIST), LB_RESETCONTENT, 0, 0);

    GetNames(&names);
    

    for(int i=0;i<names.Count();i++)
        SendMessage(GetDlgItem(hPanel, ID_LIST), LB_ADDSTRING, 0, (LPARAM)(TCHAR*)names[i].name);

    names.ZeroCount();
}

void AnimSeq::SaveStartData(BOOL bStarting)
{
    INode *root = ip->GetRootNode();
    TimeValue time = 0;

    if(bStarting && GetData(root, TIME_ID))
    {
        LoadNames();
        return;
    }

    SetData(root, TIME_ID, sizeof(time), &time);
}


void AnimSeq::GetPosSamples(INode *node, ANIM& anim) 
{
	TimeValue t;
	int delta = GetKeyframeStep();
    int start = GetTicksPerFrame() * beginValue;
    int end = (GetTicksPerFrame() * endValue);
	Matrix3 tm,startParentTM,localTM;
    Point3 pos;
    INode *curParent = node->GetParentNode();

    while(!IsBone(curParent) && (curParent != ip->GetRootNode()))
        curParent = curParent->GetParentNode();

    startParentTM = GetBoneInitialTM(curParent);
    localTM = GetBoneInitialTM(node);

    localTM = localTM * Inverse(startParentTM);


	for (t=start; t<end; t+=delta) {
        tm = node->GetNodeTM(t) * Inverse(curParent->GetNodeTM(t));

		pos = tm.GetTrans();

        Point3 val = (pos-localTM.GetTrans());
        anim.posKey.Append(1, &val);
	}
}

void AnimSeq::GetRotSamples(INode *node, ANIM &anim)
{	

    if(!IsBone(node)) return;

	TimeValue t;
	int delta = GetKeyframeStep();
    int start = GetTicksPerFrame() * beginValue;
    int end = (GetTicksPerFrame() * endValue);
	Matrix3 mainTM,mainInvParentTM,startParentTM,startTM;
    AffineParts ap;
    Quat startRot;
    INode *curParent = node->GetParentNode();

    while(!IsBone(curParent) && (curParent != ip->GetRootNode()))
        curParent = curParent->GetParentNode();

    mainTM = node->GetNodeTM(0);
    mainInvParentTM = Inverse(curParent->GetNodeTM(0));

    startParentTM = GetBoneInitialTM(curParent);
    startTM = GetBoneInitialTM(node);

    decomp_affine(startTM, &ap);
    startRot = ap.q;

    //4800=one second

    for (t=start; t<end; t+=delta) {
		Matrix3 tm = (node->GetNodeTM(t) * Inverse(curParent->GetNodeTM(t))) * startParentTM;

		Quat q(tm);

        q = q / startRot;

		if(q.w < 0)
		{
			q.x = -q.x;
			q.y = -q.y;
			q.z = -q.z;
			q.w = -q.w;
		}

        anim.rotKey.Append(1, &q);
	}
}

void AnimSeq::ClearAllDamnDataDamnit(INode *curNode)
{
    RemoveData(curNode, NAMENUM_ID);
    RemoveData(curNode, NAMEDATA_ID);

    RemoveData(curNode, ANIMATED_ID);
    RemoveData(curNode, TIME_ID);

    int i;

    for(i=0; i<1000; i++)
    {
        RemoveData(curNode, ROTNUM_ID+i);
        RemoveData(curNode, ROTDATA_ID+i);
        RemoveData(curNode, POSNUM_ID+i);
        RemoveData(curNode, POSDATA_ID+i);
    }

    for(i=0; i<curNode->NumberOfChildren(); i++)
        ClearAllDamnDataDamnit(curNode->GetChildNode(i));
}


BOOL __stdcall IsGun(INode *node)
{
    WStr wstrName(node->GetName());
    wstrName.toLower();

    if(!_tcsncmp(wstrName, _T("ext_"), 4))
        return wstrName[4] != 0;
    else
        return false;
}

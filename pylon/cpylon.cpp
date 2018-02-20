// ==============================================================
//
//  CPylon.cpp
//
//	ORBITER Vessel Interface
//
//	Author: Juan Jose Luna
//
//	Distributed under GPL. See License.txt
//
// ==============================================================

#define STRICT

#include <stdio.h>

#include "CPylon.h"
#include "..\PYLONRT\CPylonRT.h"

// Internal functions prototypes


// Constructor
CPylon::CPylon (OBJHANDLE hObj, int fmodel)
: VESSEL2 (hObj, fmodel)
{
	canNavigate = true;
	userParametersEnabled = true;
	userReleaseEnabled = true;
	userCreateEnabled = true;
	userDestroyEnabled = true;
	actualizeNotPylonChilds = false;//true;
	releaseVel = 0.0;
//todo: si owner=NULL al necesitarlo, devolver false. para ejecutar seqs desde otro sitio
	parent = NULL;
	root = NULL;
	attachmentToParent = NULL;
	sequences = NULL;
	numSeq = 0;
	icurSeq = -2;
	curSeq = NULL;

	firstFrame = true;
	firstFrameAttached = true;
	inited = false;

	mfdSelectedParameter = 0;

	orbiterSoundId = -1;

	this->meshes = 0;
}

// Destructor
CPylon::~CPylon ()
{
	if (sequences) delete sequences;
}

/*
		Vessel Callback Functions
*/

// Read status from scenario file
void CPylon::clbkLoadStateEx (FILEHANDLE scn, void *status)
{
    char *line;
	int seq;
	TPylParamValue theValue, theValue2;
	bool insideSequence = false;

	int numCmds = 0;
	while (oapiReadScenario_nextline (scn, line)) {
		char *lastPos;
		if (!insideSequence) {
			lastPos = PylonSequence::GetToken(line, theValue);
			if (lastPos == NULL) {
				ParseScenarioLineEx (line, status);
				continue;
			}
			if (theValue.type == PYL_PARAM_TOKEN_BEGINSEQ) {
				lastPos = PylonSequence::GetToken(lastPos, theValue);
				if (theValue.type != PYL_PARAM_STRING || *(theValue.strValue)==0) {
					ParseScenarioLineEx (line, status);
					continue;
				}
				int seqtype = PYL_PARAM_NOT_DEFINED;
				lastPos = PylonSequence::GetToken(lastPos, theValue2);
				if (theValue2.type == PYL_PARAM_TOKEN_OPENBR) {
					lastPos = PylonSequence::GetToken(lastPos, theValue2);
					if (theValue2.type == PYL_PARAM_STRING) {
						int i;
						for (i=0;i<PYL_PARAM_NOT_DEFINED;i++) {
							if (strcmp(PylonParamTypeName[i],theValue2.strValue)==0) break;
						}
						if (i < PYL_PARAM_NOT_DEFINED)
							seqtype = i;
						else continue;
						lastPos = PylonSequence::GetToken(lastPos, theValue2);
						if (theValue2.type != PYL_PARAM_TOKEN_CLOSEBR) continue;
						lastPos = PylonSequence::GetToken(lastPos, theValue2);
					}
				}
				int mode = SEQ_MODE_SEQUENTIAL, user = false;
				while (lastPos!=NULL) {
					switch (theValue2.type) {
					case PYL_PARAM_TOKEN_PARALLEL:
						mode = SEQ_MODE_PARALLEL;
						break;
					case PYL_PARAM_TOKEN_WAITONE:
						mode = SEQ_MODE_WAIT_ONE;
						break;
					case PYL_PARAM_TOKEN_USER:
						user = true;
						break;
					}
					lastPos = PylonSequence::GetToken(lastPos, theValue2);
				}
				if (strcmp(theValue.strValue,"STATE")==0) {
					mode = SEQ_MODE_PARALLEL;
					user = false;
				}
				seq = CreateSequence(theValue.strValue, seqtype, mode);
				if (user) SetUserSequence(seq, true);
				insideSequence = true;
				numCmds = 0;

			} else {
				ParseScenarioLineEx (line, status);
			}
		} else {

			lastPos = AddSequenceCmd(seq, line);

			if (lastPos == line) {
				lastPos = PylonSequence::GetToken(line, theValue);
				if (lastPos==NULL) continue;
				switch (theValue.type) {
				case PYL_PARAM_TOKEN_ENDSEQ:
					insideSequence = false;
					break;
				}
			} else numCmds++;
		}
    }
}

// Write status to scenario file
void CPylon::clbkSaveState (FILEHANDLE scn)
{

	#define bufSize 4096
	char cbuf[bufSize];

	// default vessel parameters
	SaveDefaultState (scn);

	// print STATE sequence
	int i = GetSequenceByName("STATE");
	if ( i != -1 ) {
		DeleteSequence(i);
	}

	i = CreateSequence("STATE",PYL_PARAM_NOT_DEFINED,false);

	int nc = GetParameterCount();
	for (int p=0;p<nc;p++) {
		int type = GetParameterType(p);
		char *name = GetParameterName(p);
		if ( type < PYL_PARAM_INTEGER ) {
			_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "SET_PARAM \"%s\" %d",name,GetParamBol(p)?1:0);
		} else if ( type < PYL_PARAM_SCALAR ) {
			_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "SET_PARAM \"%s\" %d",name,GetParamInt(p));
		} else if ( type < PYL_PARAM_STRING ) {
			_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "SET_PARAM \"%s\" %f",name,GetParamDbl(p));
		} else if ( type < PYL_PARAM_NOT_DEFINED ) {
			_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "SET_PARAM \"%s\" \"%s\"",name,GetParamStr(p));
		}

		AddSequenceCmd(i, cbuf);
	}

	if ( !canNavigate ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "CAN_NAVIGATE \"%s\" 0",this->GetName());
		AddSequenceCmd(i, cbuf);
	}
	if ( !userParametersEnabled ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "USER_PARAMETERS_ENABLED \"%s\" 0",this->GetName());
		AddSequenceCmd(i, cbuf);
	}
	if ( !userReleaseEnabled ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "USER_RELEASE_ENABLED \"%s\" 0",this->GetName());
		AddSequenceCmd(i, cbuf);
	}
	if ( !userCreateEnabled ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "USER_CREATE_ENABLED \"%s\" 0",this->GetName());
		AddSequenceCmd(i, cbuf);
	}
	if ( !userDestroyEnabled ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "USER_DESTROY_ENABLED \"%s\" 0",this->GetName());
		AddSequenceCmd(i, cbuf);
	}
	if ( actualizeNotPylonChilds ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "ACTUALIZE_NP_CHILDS_ENABLED \"%s\" 1",this->GetName());
		AddSequenceCmd(i, cbuf);
	}
	_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "CURRENT_MFD_PARAM\"%s\" %d",this->GetName(), this->GetMFDSelectedParameter());
	AddSequenceCmd(i, cbuf);

	PylonSequence* seq = 0;
	SelectSequence(i);
	seq = curSeq;
	if ( seq != 0 ) {
		seq->Print(cbuf, bufSize);
		oapiWriteScenario_string (scn, cbuf, "");
		seq = seq->nextCmd;
		while ( seq ) {
			seq->Print(cbuf, bufSize);
			oapiWriteScenario_string (scn, cbuf, "");
			seq = seq->nextSeq;
		}
		oapiWriteScenario_string (scn, "ENDSEQ", "");
	}

	// print the rest of sequences
	int n = GetSequenceCount();
	for (int j=0;j<n;j++) {
		if ( j == i ) continue;
		SelectSequence(j);
		seq = curSeq;
		if ( seq != 0 ) {
			seq->Print(cbuf, bufSize);
			oapiWriteScenario_string (scn, cbuf, "");
			seq = seq->nextCmd;
			while ( seq ) {
				seq->Print(cbuf, bufSize);
				oapiWriteScenario_string (scn, cbuf, "");
				seq = seq->nextSeq;
			}
			oapiWriteScenario_string (scn, "ENDSEQ", "");
		}
	}
}

// Frame update
// This function should be called from the child vessel class callback function as the
// LAST instruction. It updates the parent attachment from the local variables attachmenToParent,
// pos, dir and rot (if it is attached to something) and executes the active sequences.
void CPylon::clbkPreStep (double simt, double simdt, double mjd)
{

    // TODO2018 remove!
	ActualizeParent();

	if (attachmentToParent != NULL) {
		SetAttachmentParams(attachmentToParent,pos,dir,rot);
	}

/*
old code
	bool f = firstFrame;
	if (firstFrame) {
		firstFrame = false;
	}
	else
		if (attachmentToParent != NULL) {
		SetAttachmentParams(attachmentToParent,pos,dir,rot);
	}
*/
/*	else
	{
		_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE,"TIMESTEP 1: %s",GetName());
	}
bug: no deberian pasar por aqui, y parece que los childs en attachment 2,3,4 etc no se actualizan
_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE,"CHUNGONOKO: %s",GetName());
*/


	if ( firstFrame && actualizeNotPylonChilds) {
_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE,"CHUNGONOKO: %s",GetName());

        ATTACHMENTHANDLE top;
        OBJHANDLE op;

		int n = AttachmentCount(false);
		VESSEL *v;
		for (int i=0;i<n;i++) {
			ATTACHMENTHANDLE a = GetAttachmentHandle(false, i);
			OBJHANDLE o = GetAttachmentStatus(a);
			if (o!=NULL) {
				v = oapiGetVesselInterface(o);
				if (v==NULL) continue;
				if (IsPylonVessel(v)!=NULL) continue;

                VESSEL *r=this;
                double cm = v->GetEmptyMass();
                op = NULL;
                while (op==NULL) {
                    double rm = r->GetEmptyMass();
                    r->SetEmptyMass(rm + cm);
                    int j=0, m= r->AttachmentCount(true);
                    while (j<m && op==NULL) {
                        top = r->GetAttachmentHandle(true, j);
                        op = r->GetAttachmentStatus(top);
                        j=j+1;
                    }
                    if (op==NULL) break;
                    r = oapiGetVesselInterface(op);
                    if (r==NULL) break;
                    op=NULL;
                }
			}
		}
	}

	if (sequences) sequences->Execute();

	firstFrame = false;
	firstFrameAttached = false;
}

int CPylon::clbkConsumeBufferedKey (DWORD key, bool down, char *kstate)
{
	if (!down) return 0; // only process keydown events

	if (KEYMOD_SHIFT (kstate)) {
	} else if (KEYMOD_CONTROL (kstate)) {
/*		switch (key) {
		case OAPI_KEY_:
			return 1;
		}
*/
	} else {
		if (this->ActivateSequenceByKey(key)) {
			return 1;
		}
	}
	return 0;
}

void CPylon::clbkPostCreation (void)
{

	orbiterSoundId = ConnectToOrbiterSoundDLL(GetHandle());

	this->initializePylon();

}

// ==============================================================
// Pylon API
// ==============================================================

// Returns the number of parameters that the pylon publishes.
// The number, name and type of parameters must be constant throughout the Orbiter session.
int CPylon::GetParameterCount(void)
{
	return 0;
}

// Returns the name of a parameter. Different parameters should not have the same name.
// The parameter names should not contain spaces or the following symbols : " , . ( )
// and should not start with _underscore
// Parameters:
//   index: The index of the parameter. if index is < 0 or >= GetParameterCount, the
// returned value should be NULL.
char *CPylon::GetParameterName(int index)
{
	return NULL;
}

// Returns the type of a parameter:
//   PYLON_PARAM_BOOL
//   PYLON_PARAM_INT
//   PYLON_PARAM_DOUBLE
//   PYLON_PARAM_STRING
//   PYLON_PARAM_NOTDEFINED
// Parameters:
//   index: The index of the parameter. if index is < 0 or >= GetParameterCount(),
// the returned value should be PYLON_PARAM_NOTDEFINED
int CPylon::GetParameterType(int index)
{
	return PYL_PARAM_NOT_DEFINED;
}

// Returns if the parameter is a user parameter. A user parameter could be set
// manually by the pilot of the vessel. All other parameters are set only by API calls or
// sequences.
// Parameters:
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the returned value should be false.
bool CPylon::IsUserParameter(int index) {
	return false;
}

// Returns the value of a boolean parameter. If the parameter is not of this type,
// the return value is undefined.
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the returned value is undefined.
bool CPylon::GetParamBol(int index)
{
	return false;
}

// Returns the value of an integer parameter. If the parameter is not of this type,
// the return value is undefined.
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the returned value is undefined.
int CPylon::GetParamInt(int index)
{
	return 0;
}

// Returns the value of a double parameter. If the parameter is not of this type,
// the return value is undefined.
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the returned value is undefined.
double CPylon::GetParamDbl(int index)
{
	return 0;
}

// Returns the value of a string parameter (a NULL terminated string). If the parameter
// is not of type string, the returned value is undefined. The returned string should only
// be read, not written to.
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the returned value is undefined.
char *CPylon::GetParamStr(int index)
{
	return NULL;
}

bool CPylon::SetParam(int index, TPylParamValue *value) {
	int paramt = GetParameterType(index);
	if (paramt < PYL_PARAM_INTEGER) {
		return SetParamBol(index, value->bolValue);
	}
	else if (paramt < PYL_PARAM_SCALAR)
		return SetParamInt(index, value->intValue);
	else if (paramt < PYL_PARAM_STRING)
		return SetParamDbl(index, value->dblValue);
	else if (paramt < PYL_PARAM_NOT_DEFINED)
		return SetParamStr(index, value->strValue);
	else return true;
}

// Sets the value of a boolean parameter. The action depends on the meaning of the para-
//meter.
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the call should be ignored.
//   value: The value
bool CPylon::SetParamBol(int index, bool value)
{
	return true;
}

// Sets the value of an integer parameter. The action depends on the meaning of the para-
//meter.
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the call should be ignored.
//   value: The value
bool CPylon::SetParamInt(int index, int value)
{
	return true;
}

// Sets the value of a double parameter. The action depends on the meaning of the para-
//meter.
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the call should be ignored.
//   value: The value
bool CPylon::SetParamDbl(int index, double value)
{
	return true;
}

// Sets the value of a string parameter. The action depends on the meaning of the para-
//meter.
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the call should be ignored.
//   value: The value (a pointer to a NULL terminated string). The string should not be
// modified by the call.
bool CPylon::SetParamStr(int index, char *string)
{
	return true;
}

// Increments a parameter. The behavior of this function depends on the meaning of the parameter.
// Parameters:
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the call should be ignored.
bool CPylon::IncParam(int index)// Default behavior: does nothing
{
	return true;
}

// Decrements a parameter. The behavior of this function depends on the meaning of the parameter.
// Parameters:
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the call should be ignored.
bool CPylon::DecParam(int index)// Default behavior: does nothing
{
	return true;
}

// Returns if the parameter has reached the value or state required by the last setParamx,
// IncParam() or DecParam() call. This depends on the meaning of the parameter.
//   index: The index of the parameter. If index is < 0 or >= GetParameterCount(),
// the returned value is undefined.
bool CPylon::IsParameterSet(int index)
{
	return true;
}

// ==============================================================
// Sequence management functions
// ==============================================================
void CPylon::SelectSequence(int i) {
//_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE,"selseq1: i=%d,icurseq=%d",i,icurSeq);
/*
todo quitar icurseq

	if (i==icurSeq+1) { icurSeq++; curSeq = curSeq->nextSeq; }
	else / *if (i!=icurSeq)probar sin el segundo else* / {*/
		icurSeq = 0; curSeq = sequences;
		while (i > 0 && icurSeq < numSeq) { curSeq = curSeq->nextSeq; i--; }
/*	}
if (curSeq==NULL) {
_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE,"***********************************************************selseqERROR: i=%d,icurseq=%d",i,icurSeq);
curSeq=sequences;return;
}
*/
}

int CPylon::GetSequenceCount(void) {
	return numSeq;
}
char *CPylon::GetSequenceName(int index) {
	if (index<0 || index >= numSeq) return NULL;
	SelectSequence(index);
	return curSeq->name;
}
int CPylon::GetSequenceType(int index) {
	if (index<0 || index >= numSeq) return PYL_PARAM_NOT_DEFINED;
	SelectSequence(index);
	return curSeq->value.type;
}
int CPylon::GetSequenceCancelType(int index) {
	if (index<0 || index >= numSeq) return PYL_PARAM_NOT_DEFINED;
	SelectSequence(index);
	if (curSeq->nextCmd==NULL) return PYL_PARAM_NOT_DEFINED;
	if (curSeq->nextCmd->cmdcode!=PYL_CMD_SET_CANCEL_SEQUENCE) return PYL_PARAM_NOT_DEFINED;
	if (!curSeq->nextCmd->valid) return PYL_PARAM_NOT_DEFINED;
	int cs = curSeq->nextCmd->value.tempIndex;
	return GetSequenceType(cs);
}

bool CPylon::IsUserSequence(int index) {
	if (index<0 || index >= numSeq) return false;
	SelectSequence(index);
	return curSeq->userSequence;
}
void CPylon::SetUserSequence(int index, bool user) {
	if (index<0 || index >= numSeq) return;
	SelectSequence(index);
	curSeq->userSequence = user;
}
int CPylon::CreateSequence(char *name, int type, int execmode) {

	int i = GetSequenceByName(name);
	if ( i != -1 ) {
//kk		DeleteSequence(i);
	}

	PylonSequence *ns = new PylonSequence(true, this);

	if(strcmp("STATE",name)==0) ns->activateByKey = false;

	strncpy_s(ns->name, NAME_SIZE, name, NAME_SIZE);
	if (execmode<0) execmode = 0; if (execmode >= SEQ_MODE_LAST) execmode = SEQ_MODE_LAST-1;
	ns->mode = execmode;
	ns->value.type = type;
	if (numSeq>0) {
		icurSeq = numSeq; curSeq = sequences;
		while (curSeq->nextSeq) { curSeq = curSeq->nextSeq;}
		curSeq->nextSeq = ns;
	} else {
		sequences = ns;
		icurSeq = 0;
	}
	curSeq = ns;
	icurSeq = numSeq++;
	return icurSeq;
}
char *CPylon::AddSequenceCmd(int index, char *cmd) {
	if (index<0 || index >= numSeq) return false;
	SelectSequence(index);
	TPylParamValue val;
	return curSeq->AddCmd(cmd, val);
}
bool CPylon::IsSequenceValid(int index) {
	if (index<0 || index >= numSeq) return false;
	SelectSequence(index);
	return curSeq->valid;
}
bool CPylon::ActivateSequence(int index) {
	if (index<0 || index >= numSeq) return false;
	SelectSequence(index);
	TPylParamValue val; val.type = PYL_PARAM_NOT_DEFINED;
	return curSeq->Call(&val, NULL);
}
bool CPylon::ActivateSequenceValue(int index, TPylParamValue *value) {
	if (index<0 || index >= numSeq) return false;
	SelectSequence(index);
	return curSeq->Call(value, NULL);
}
bool CPylon::CancelSequence(int index) {
	if (index<0 || index >= numSeq) return false;
	SelectSequence(index);
	TPylParamValue val; val.type = PYL_PARAM_NOT_DEFINED;
	return curSeq->Cancel(&val);
}
bool CPylon::CancelSequenceValue(int index, TPylParamValue *value) {
	if (index<0 || index >= numSeq) return false;
	SelectSequence(index);
	return curSeq->Cancel(value);
}
bool CPylon::IsSequenceActive(int index) {
	if (index<0 || index >= numSeq) return false;
	SelectSequence(index);
	return curSeq->activating;
}
bool CPylon::IsSequenceCancelling(int index) {
	if (index<0 || index >= numSeq) return false;
	SelectSequence(index);
	return curSeq->cancelling;
}
int CPylon::GetSequenceByName(char *name)
{
	int n = GetSequenceCount();
	for (int i=0;i<n;i++)
	{
		SelectSequence(i);
		if (strcmp(curSeq->name,name)==0) {
			return i;
		}
	}
	return -1;
}

// TODO: This method needs to be rewritten because it frees memory already freed by PylonSequence's destructor; this is
// what causes a crash when you save the scenario more than once.  A quick & dirty workaround would be to simply comment out
// all the "delete" commands throughout the source code.  However, it really needs to be fixed properly sooner or later.
void CPylon::DeleteSequence(int index)
{
	if (index<0 || index >= numSeq) return;
	PylonSequence *prevSeq;
	if (index==0) {
		prevSeq = NULL;
	} else {
		SelectSequence(index-1);
		prevSeq = curSeq;
	}
	SelectSequence(index);

// todo this is the loop where sequence commands are deleted. move this to the sequence destructor
	PylonSequence *cmd = curSeq->nextCmd;
	while (cmd != NULL) {
// todo uncomment this, if PYL_PARAM_GET_PARAMETER commands effectively have a string value to be deleted
/*		PylonSequence *val = cmd->nextSeq;
		while ( val != NULL) {
			if ( val->value.type==PYL_PARAM_GET_PARAMETER) {
				delete val->name;
			}
			PylonSequence *oldVal= val;
			val = val->nextCmd;
			delete oldVal;
		}*/
//_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE,"%d,%d,%d",index,cmd,cmd->value.type);
//oapiWriteScenario_string(scnr,"*****2",f);
		PylonSequence *old= cmd;
		cmd = cmd->nextCmd;
		delete old;
	}

	delete curSeq->name;

//todo this is ok
	if ( prevSeq == NULL ) {
		sequences = curSeq->nextSeq;
	} else {
		prevSeq->nextSeq = curSeq->nextSeq;
	}
// todo uncomment this
//	delete curSeq;

//todo this is ok
	icurSeq = 0;
	curSeq = sequences;
	numSeq--;
}

bool CPylon::ActivateSequenceByKey(DWORD key) {
	bool b = false;
	PylonSequence *seq = sequences;
	int i = 0;
	while (seq) {
//		_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE,"-(B2)tecla %d, i=%d, act=%d,seqkey=%d",key,i,seq->activateByKey?1:0, seq->activationKey);
		if (seq->activateByKey && seq->activationKey == key) {
			ActivateSequence( i );
			b = true;
		}
		seq = seq->nextSeq;
		i++;
	}
	return b;
}

// ==============================================================
// Generic management functions
// ==============================================================

// Returns the parent of the vessel if it is connected to a parent, or NULL otherwise.
VESSEL *CPylon::GetParent(void) const {
	return parent;
}

// Returns the root of the vessel if it is connected to a parent, or 'this' otherwise.
VESSEL *CPylon::GetRoot(void) const {
	return root;
}

ATTACHMENTHANDLE CPylon::GetAttToParent(void) const {
	return attachmentToParent;
}


bool CPylon::PylonAttach(OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, ATTACHMENTHANDLE child_attachment) {
	VESSEL *p = oapiGetVesselInterface(parent),
		*c = oapiGetVesselInterface(child);

	if (p==NULL || c==NULL) return false;
	CPylon *pp = IsPylonVessel(p), *pc = IsPylonVessel(c);

	ATTACHMENTHANDLE top;
	OBJHANDLE o = NULL;
	int i=0, n= c->AttachmentCount(true);
	while (i<n && o==NULL) {
		top = c->GetAttachmentHandle(true, i);
		o = c->GetAttachmentStatus(top);
		i=i+1;
	}
	if (o!=NULL)
	{
		PylonDetach(o, c->GetHandle(), parent_attachment, 0.0);
	}

	if (!p->AttachChild(child, parent_attachment, child_attachment)) return false;

	if (pc!=NULL)
		pc->PropagateChange(false, p);
	else if (pp!=NULL)
		pp->PropagateChange(false, pp->GetParent());

	o = NULL;
	VESSEL* r=p;
	double cm = c->GetEmptyMass();
	while (o==NULL) {
		double rm = r->GetEmptyMass();
		r->SetEmptyMass(rm + cm);
		int i=0, n= r->AttachmentCount(true);
		while (i<n && o==NULL) {
			top = r->GetAttachmentHandle(true, i);
			o = r->GetAttachmentStatus(top);
			i=i+1;
		}
		if (o==NULL) break;
		r = oapiGetVesselInterface(o);
		if (r==NULL) break;
		o=NULL;
	}
	return true;
}

bool CPylon::PylonDetach(OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, double vel) {

	if (parent==NULL || child==NULL) return false;
	VESSEL *p = oapiGetVesselInterface(parent),
		*c = oapiGetVesselInterface(child);
	if (p==NULL || c==NULL) return false;
	CPylon *pp = IsPylonVessel(p), *pc = IsPylonVessel(c);
	/*if (pc!=NULL) {
		if (pp==NULL) {
			pc->PropagateChange(true, NULL);
		}
	}*/
	if (pp!=NULL)
		pp->PropagateChange(false, pp->GetParent());
	if (pc!=NULL) {
		pc->PropagateChange(true, NULL);
	}

	ATTACHMENTHANDLE top;
	OBJHANDLE o = NULL;
	VESSEL *r=p;
	double cm = c->GetEmptyMass() + c->GetFuelMass();
	while (o==NULL) {
		double rm = r->GetEmptyMass();
		if (rm > cm) r->SetEmptyMass(rm - cm);
		int i=0, n= r->AttachmentCount(true);
		while (i<n && o==NULL) {
			top = r->GetAttachmentHandle(true, i);
			o = r->GetAttachmentStatus(top);
			i=i+1;
		}
		if (o==NULL) break;
		r = oapiGetVesselInterface(o);
		if (r==NULL) break;
		o=NULL;
	}

	bool done = p->DetachChild(parent_attachment, vel);

	double invdt = oapiGetSimStep(),
		cmass = c->GetMass() + c->GetFuelMass();

	if (done && invdt > 0) {
		VECTOR3 force, pos, tempv;
		invdt = 1 / invdt;

		CPylonRT *prt = (CPylonRT *)pc;

		if (prt==NULL) prt = (CPylonRT *)pp;

		if (prt!=NULL) {
//commented 2018
//_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE, "pylon detach 8,  prt=%s",prt->GetName());

			if (prt->rotAxis != PYL_RT_NO_ROTATION || prt->releaseAngVel != 0.0) {

				VESSELSTATUS2 status;
				status.version = 2;
				status.flag = 0;
				status.fuel = NULL;
				status.thruster = NULL;
				status.dockinfo = NULL;

				c->GetStatusEx(&status);
				VECTOR3 localAngVel;
				switch (prt->rotAxis) {
				case PYL_RT_ROTATION_X: localAngVel = _V(prt->angVel,0,0); break;
				case PYL_RT_ROTATION_Y: localAngVel = _V(0,prt->angVel,0); break;
				case PYL_RT_NO_ROTATION:
				case PYL_RT_ROTATION_Z: localAngVel = _V(0,0,-prt->angVel); break;
				}
				if (prt != c) {
					VECTOR3 globalAngVel;
					prt->GlobalRot( localAngVel, globalAngVel );

					VECTOR3 oX, oY, oZ;
					c->GlobalRot(_V(1,0,0), oX);
					c->GlobalRot(_V(0,1,0), oY);
					c->GlobalRot(_V(0,0,1), oZ);

					localAngVel = _V( dotp( globalAngVel, oX ),
									  dotp( globalAngVel, oY ),
									  dotp( globalAngVel, oZ ) );
				}

				status.vrot = localAngVel;
				c->DefSetStateEx(&status);

			}

			if (prt->hasTraslation) {
				c->AddForce(prt->unitTrasl * (prt->linVel * invdt), _V(0,0,0));
			}

		}

		if (vel!=0) {
			if (pp!=NULL) {
				r = pp->GetRoot();
			} else {
				o = NULL;
				r = p;
				while (o==NULL) {
					int i=0, n= r->AttachmentCount(true);
					while (i<n && o==NULL) {
						top = r->GetAttachmentHandle(true, i);
						o = r->GetAttachmentStatus(top);
						i=i+1;
					}
					if (o==NULL) break;
					r = oapiGetVesselInterface(o);
					if (r==NULL) break;
					o=NULL;
				}
			}
			if (r != NULL) {
				VECTOR3 dir;
				p->GetAttachmentParams(parent_attachment, pos, dir, tempv);
				if (p != r) {
					dir+=pos; p->/*GlobalRot-1*/Local2Global(dir, tempv);
					r->Global2Local(tempv, dir);
					p->Local2Global(pos, tempv);
					r->Global2Local(tempv, pos);
					dir = dir - pos;
				}
				force = dir * ( -vel * cmass * invdt );
				r->AddForce(force, pos);
			}
		}
	}

	return done;
}


CPylon * CPylon::IsPylonVessel(VESSEL *v) {
	static char *s1 = "PYLON\0";
	static char *s2 = "pylon\0";
	if (v==NULL) return NULL;
	char *n = v->GetClassName();
	if (strlen(n)<5) return NULL;
	for (int i=0;i<5;i++) {
		if (n[i]!=s1[i] && n[i]!=s2[i]) return NULL;
	}
	return (CPylon *) v;
}



// This function is called when a call to PropagateChange() is done to notify that a change
// has ocurred in the pylon tree structure. In this function the childs should actualize
// its references to other components in the structure, or other work could be performed here.
//
// Attachment management should be done here to improve efficiency. In the Timestep,
// related vessels (CPylons or other) in the structure should be accesed from direct
// references that are actualized here, rather than searching them in every frame.
//
// Each class that overrides this function should make a call in the first line to her mother
// class Changed() function.
//
// The CPylon::Changed() function actualizes the sequences that have object references.
//
// When actualizing references to brother, cousin, uncles objects etc, and going  up in the
// structure, the parent field of the CPylons should be used here instead of the api calls,
// wich could not show the final tree state. This also aplies to root and attachmentToParent.
//
// When 'navigating' down, use the api calls, but make sure that any child CPylon has his
// parent field equal to its actual parent. If not, that child is being detached.
//
// The references should be unlinked if that object/s are being detached. The references
// should not go through non-CPylon vessels in the down direction, but can in the up one, only
// one time: that is, the CPylons can communicate through the ship that owns those CPylon.
//
// The function IsConnected() can be used here to determine if a vessel is connected to this
// under those conditions.
//
void CPylon::Changed(void)
{
//_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE, "Changed[%s]: parent = %s, root = %s, atttop = %d",GetName(),parent->GetName(),root->GetName(), attachmentToParent);
//	ActualizeSequences();
}

// The PropagateChange() function should be called before or after a change occurs in a CPylon
// tree structure (that is, any object, CPylon or not, is detached or attached to or from
// the structure containing CPylon objects).
//
// The Changed() functions of all the Pylon vessels so affected are called in a descent order
// down the tree, so they can actualize references to other pylons in the structure.
//
// When Detaching object A from B, a call should be made prior to B->DetachChild(). If A is
// a CPylon, call A->PropagateChange(true, B, NULL).
//
// You must ensure first that A is a CPylon using the IsPylonVessel() static function. If it is not,
// then check B and call B->PropagateChanges(false, B->GetParent(), B->GetAttToParent())
// If neither is a CPylon, just call DetachChild().
//
// When attaching A to B, the call A->PropagateChanges(false, B, attP) must be made AFTER the
// call to B->AttachChild() (attP is the 'to parent' attachment in A) If A is not CPylon, make
// instead the call to B:
// B->PropagateChanges(false, B->GetParent(), B->GetAttToParent()) and if neither is, no call is needed.
//
// For convenience, the functions PylonAttachChild() and PylonDetachChild() could be used
// when attaching or detaching. They do all the checking and the call to this function.
//
// This method should not be overriden
void CPylon::PropagateChange(bool detaching, VESSEL *parent)
{
	if (detaching) {
		if (this->parent==NULL) return;
		RecursivePropagateChange(NULL, NULL);
	}
	if (parent==NULL) return;
	VESSEL *r=parent;
	CPylon *pr=IsPylonVessel(r);
	while (pr!=NULL) {
		if (pr->parent==NULL) break;
		r = pr->parent;
		pr = IsPylonVessel(r);
	}

	if (pr!=NULL) {
		pr->RecursivePropagateChange(pr, NULL);
	} else {

		for (int i=0,n=r->AttachmentCount(false); i<n; i++) {
			ATTACHMENTHANDLE a = r->GetAttachmentHandle(false, i);
			OBJHANDLE o = GetAttachmentStatus(a);
			if (o==NULL) continue;
			CPylon *p = IsPylonVessel(oapiGetVesselInterface(o));
			if (p!=NULL) {
				p->RecursivePropagateChange(r, r);
				break;
			}
		}
	}
}

// This function tells if two vessels are connected. It's a costly function, use it only in
// the body of the virtual function Changed()
//
bool CPylon::IsConnected(VESSEL *v, VESSEL *root, VESSEL *parent) {

	return true;
}

// ==============================================================
// End of Pylon API
// ==============================================================


// ==============================================================
// Internal functions
// ==============================================================

void CPylon::RecursivePropagateChange(VESSEL *root, VESSEL *parent) {

	if (this->root == NULL && root != NULL) firstFrameAttached = true;

	this->root = root;
	this->parent = parent;

	if (parent) {
		ATTACHMENTHANDLE att;
		int i, n = AttachmentCount(true);
		for	(i=0;i<n;i++) {
			att = GetAttachmentHandle(true, i);
			if (GetAttachmentStatus(att)!=NULL) {attachmentToParent = att; i = n;}
		}
	}

	Changed();

	for (int i=0,n=AttachmentCount(false); i<n; i++) {
		ATTACHMENTHANDLE a = GetAttachmentHandle(false, i);
		OBJHANDLE o = GetAttachmentStatus(a);
		if (o==NULL) continue;
		CPylon *p = IsPylonVessel(oapiGetVesselInterface(o));
		if (p!=NULL) {
			p->RecursivePropagateChange(root, this);
		} else {
		}
	}
}

//Called only in simulation init
void CPylon::ActualizeParent(void) {

	ATTACHMENTHANDLE top;
	OBJHANDLE o = NULL;


	int i=0, n= AttachmentCount(true);
	while (i<n && o==NULL) {
		top = GetAttachmentHandle(true, i);
		o = GetAttachmentStatus(top);
		if ( !o )
		{
			i=i+1;
		}
	}

	if (i>=n) return;

	VESSEL *v = oapiGetVesselInterface(o);
	CPylon *p = CPylon::IsPylonVessel(v);

//	_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE, "actualize parent: %s, parent=%s, i=%d, n=%d,o=%d,v=%s %f",GetName(), this->parent?this->parent->GetName():"NOPARENT", i,n,o, v?v->GetName():"NOVESSELPARENT", oapiGetSimTime());

//	this->parent = v;

	if (p==NULL) {
		PropagateChange(false, v);
	}

//	_snprintf_s(oapiDebugString(),NAME_SIZE, NAME_SIZE, "actualize parent2: %s, parent=%s, i=%d, n=%d,o=%d, %f",GetName(), this->parent?this->parent->GetName():"NOPARENT", i,n,o, oapiGetSimTime());
}

bool CPylon::IsFirstFrame() {
	return firstFrame;
}

bool CPylon::IsFirstFrameAttached() {
	return firstFrameAttached;
}

int CPylon::GetMFDSelectedParameter()
{
	return mfdSelectedParameter;
}
void CPylon::SetMFDSelectedParameter(int selectedParameter)
{
	mfdSelectedParameter = selectedParameter;
}

void CPylon::initializePylon() {

    ActualizeParent();

    if (parent) {
        ATTACHMENTHANDLE top;
        OBJHANDLE op;
        VESSEL *r=parent;

        double cm = GetEmptyMass();
        op = NULL;
        while (op==NULL) {
            double rm = r->GetEmptyMass();
            r->SetEmptyMass(rm + cm);
            int	j=0, m= r->AttachmentCount(true);
            while (j<m && op==NULL) {
                top = r->GetAttachmentHandle(true, j);
                op = r->GetAttachmentStatus(top);
                j=j+1;
            }
            if (op==NULL) break;
            r = oapiGetVesselInterface(op);
            if (r==NULL) break;
            op=NULL;
        }
    }

    int n = GetSequenceCount();
    TPylParamValue v; v.type = PYL_PARAM_NOT_DEFINED;
    for (int i=0;i<n;i++) {
        if (strcmp(GetSequenceName(i),"STATE\0")==0) {
            SelectSequence(i);
            curSeq->Call(&v, NULL);
            curSeq->Execute();
            v.type = PYL_PARAM_NOT_DEFINED;
            curSeq->Cancel(&v);
            break;
        }
    }

    this->inited = true;

}

PylonMesh::PylonMesh()
{
	*file = 0;
 	*name = 0;
	visMode = 0;
	meshIndex = -1;
	prev = 0;
	next = 0;
	owner = 0;
}

PylonMesh* PylonMesh::FindMesh(const char *n)
{

	if ( next )
	{
		PylonMesh *pm = next->FindMesh(n);
		if (pm) return pm;
	}

	if ( *this->file!=0 && strcmp(n, (const char *)this->file) == 0 ) return this;
	//if ( next ) return next->FindMesh(n);
	return 0;
}

PylonMesh* PylonMesh::FindMeshName(const char *n)
{

	if ( *this->file!=0 && strcmp(n, (const char *)this->name) == 0 ) return this;
	if ( next ) return next->FindMeshName(n);
	return 0;
}

void PylonMesh::Add(CPylon * p, unsigned char *filename, unsigned char *name, int visMode, VECTOR3 pos)
{
	if ( !p ) return;

	if ( this->FindMeshName((const char *)name) ) return;

	int index = -1;
	PylonMesh *m = this->FindMesh((const char *)filename);
	if ( m )
	{
		index = m->meshIndex;
	}

	bool k=*file!=0;
	m = this;
	int i=0;
	if ( *file )
	{
		while (m->next) { m = m->next; i++;}
		m->next = new PylonMesh();
		if (!m->next) { m->next = 0; return; }
		m->next->prev = m;
		m = m->next;
	}

	_snprintf_s((char *)m->file, NAME_SIZE, NAME_SIZE, "%s", filename);
	_snprintf_s((char *)m->name, NAME_SIZE, NAME_SIZE, "%s", name);

	if ( index != -1 )
	{
	    p->InsertMesh( (const char *)filename, m->meshIndex, &pos );
	}
	else
    {
        m->meshIndex = p->AddMesh((const char *)filename, &pos);
    }

	p->SetMeshVisibilityMode( m->meshIndex, visMode );
	m->visMode = visMode;
}

void PylonMesh::Clear()
{
	PylonMesh *next1 = next;
	if ( next1 ) next->Clear();

	*file = 0;
 	*name = 0;
	visMode = 0;
	meshIndex = -1;
	prev = 0;
	next = 0;
	owner = 0;

	delete next1;
}

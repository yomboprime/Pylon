// ==============================================================
//
//  CPylon.cpp
//
//	ORBITER Vessel Interface
//
//	Author: Juan Jose Luna
//
//	Distributed MIT licensed. See License.txt
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

	sequences = NULL;
	numSeq = 0;
	icurSeq = -2;
	curSeq = NULL;

	firstFrame = true;

	attToParent = NULL;
	inited = false;

	mfdSelectedParameter = 0;

	orbiterSoundId = -1;

	meshes = NULL;

	redirectKeysVesselHandle = NULL;
	redirectKeysVesselName[ 0 ] = 0;

	for ( int i = 0; i < 60; i++ ) {
        soundNames[ i ].soundName[ 0 ] = 0;
	}

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
	int stateSequenceIndex = GetSequenceByName("STATE");
	if ( stateSequenceIndex != -1 ) {
		DeleteSequence(stateSequenceIndex);
	}

	stateSequenceIndex = CreateSequence("STATE",PYL_PARAM_NOT_DEFINED,false);

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

		AddSequenceCmd(stateSequenceIndex, cbuf);
	}

	if ( !canNavigate ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "CAN_NAVIGATE \"%s\" 0",this->GetName());
		AddSequenceCmd(stateSequenceIndex, cbuf);
	}
	if ( !userParametersEnabled ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "USER_PARAMETERS_ENABLED \"%s\" 0",this->GetName());
		AddSequenceCmd(stateSequenceIndex, cbuf);
	}
	if ( !userReleaseEnabled ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "USER_RELEASE_ENABLED \"%s\" 0",this->GetName());
		AddSequenceCmd(stateSequenceIndex, cbuf);
	}
	if ( !userCreateEnabled ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "USER_CREATE_ENABLED \"%s\" 0",this->GetName());
		AddSequenceCmd(stateSequenceIndex, cbuf);
	}
	if ( !userDestroyEnabled ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "USER_DESTROY_ENABLED \"%s\" 0",this->GetName());
		AddSequenceCmd(stateSequenceIndex, cbuf);
	}
	if ( actualizeNotPylonChilds ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "ACTUALIZE_NP_CHILDS_ENABLED \"%s\" 1",this->GetName());
		AddSequenceCmd(stateSequenceIndex, cbuf);
	}
	_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "CURRENT_MFD_PARAM \"%s\" %d",this->GetName(), this->GetMFDSelectedParameter());
	AddSequenceCmd(stateSequenceIndex, cbuf);

	PylonMesh *pm = this->meshes;
	while ( pm ) {
        _snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "ADD_MESH \"%s\" \"%s\" %d %f %f %f",
                    pm->file,
                    pm->name,
                    pm->visMode,
                    pm->pos.x,
                    pm->pos.y,
                    pm->pos.z );
        AddSequenceCmd( stateSequenceIndex, cbuf );
        pm = pm->next;
	}

	PylonSequence* seq;
	int n = GetSequenceCount();
	for ( int j = 0; j < n;j++ ) {
		if ( j == stateSequenceIndex ) continue;
		SelectSequence( j );
		seq = curSeq;
		if ( seq != 0 && seq->activationKey ) {
            _snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "SET_SEQUENCE_KEY \"%s:%s\" %d", this->GetName(), seq->name, seq->activationKey );
            AddSequenceCmd( stateSequenceIndex, cbuf );
		}
	}

	for ( int i = 0; i < 60; i++ ) {
        if ( *this->soundNames[ i ].soundName != 0 ) {
            _snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "SOUND_LOAD \"%s\" \"%s\" %d %d", this->soundNames[ i ].soundPath, this->soundNames[ i ].soundName, i + 1, this->soundNames[ i ].mode );
            AddSequenceCmd( stateSequenceIndex, cbuf );
        }
	}


	if ( this->redirectKeysVesselHandle != NULL ) {
		_snprintf_s(cbuf, NAME_SIZE, NAME_SIZE, "REDIRECT_KEYS \"%s\" 1", redirectKeysVesselName );
		AddSequenceCmd(stateSequenceIndex, cbuf);
	}

	seq = NULL;
	SelectSequence(stateSequenceIndex);
	seq = curSeq;
	if ( seq != NULL ) {
		seq->Print(cbuf, bufSize);
		oapiWriteScenario_string (scn, cbuf, "");
		//*( cbuf + 0 ) = ' ';
		//*( cbuf + 1 ) = ' ';
		seq = seq->nextCmd;
		while ( seq ) {
			seq->Print(cbuf /*+ 2*/, bufSize /*- 2*/);
			oapiWriteScenario_string (scn, cbuf, "");
			seq = seq->nextSeq;
		}
		oapiWriteScenario_string (scn, "ENDSEQ", "");
	}

	// print the rest of sequences
	n = GetSequenceCount();
	for (int j=0;j<n;j++) {
		if ( j == stateSequenceIndex ) continue;
		SelectSequence(j);
		seq = curSeq;
		if ( seq != 0 ) {
			seq->Print(cbuf, bufSize);
			oapiWriteScenario_string (scn, cbuf, "");
			//*( cbuf + 0 ) = ' ';
            //*( cbuf + 1 ) = ' ';
			seq = seq->nextCmd;
			while ( seq ) {
				seq->Print(cbuf /*+ 2*/, bufSize /*- 2*/);
				oapiWriteScenario_string (scn, cbuf, "");
				seq = seq->nextSeq;
			}
			oapiWriteScenario_string (scn, "ENDSEQ", "");
		}
	}
}

// Frame update
// This function should be called from the child vessel class callback function as the
// LAST instruction. It updates the parent attachment from the local variables
// pos, dir and rot (if it is attached to something) and executes the active sequences.
void CPylon::clbkPreStep(double simt, double simdt, double mjd)
{

    if ( firstFrame ) {
        this->initializePylon();
    }

	if ( this->attToParent != NULL ) {
		SetAttachmentParams( this->attToParent, pos, dir, rot );
	}

	if (sequences) sequences->Execute();

	firstFrame = false;
}

int CPylon::clbkConsumeBufferedKey ( DWORD key, bool down, char *kstate ) {

	if ( down && this->ActivateSequenceByKey( key ) ) {
        return 1;
	}

	if ( this->redirectKeysVesselHandle != NULL &&
            key != OAPI_KEY_H &&
            ( key < OAPI_KEY_1 || key > OAPI_KEY_0 ) &&
            key != OAPI_KEY_DIVIDE &&
            key != OAPI_KEY_A &&
            key != OAPI_KEY_S &&
            key != OAPI_KEY_A &&
            key != OAPI_KEY_E &&
            key != OAPI_KEY_P &&
            key != OAPI_KEY_O &&
            ( key < OAPI_KEY_B || key > OAPI_KEY_PERIOD ) ) {

        VESSEL *v = oapiGetVesselInterface( this->redirectKeysVesselHandle );
        if ( v != NULL ) {

            return v->SendBufferedKey( key, down, kstate );

        }
	}

	return 0;

}

int CPylon::clbkConsumeDirectKey(char *kstate) {

    if ( this->redirectKeysVesselHandle == NULL ) {
        return 0;
    }

    VESSEL *v = oapiGetVesselInterface( this->redirectKeysVesselHandle );
    if ( v == NULL ) {
        return 0;
    }

    // TODO Main has complex behaviour with ctrl key modifier
    //v->SetThrusterGroupLevel( THGROUP_MAIN, KEYDOWN( kstate, OAPI_KEY_ADD ) ? 1.0 : 0.0 );

    // TODO Is there a key for retro thrusters?
	//v->SetThrusterGroupLevel( THGROUP_RETRO, KEYDOWN( kstate,  ) ? 1.0 : 0.0 );

	// TODO hover is progressive.
	//v->SetThrusterGroupLevel( THGROUP_HOVER, KEYDOWN( kstate,  ) ? 1.0 : 0.0 );

	int rcsMode = v->GetAttitudeMode();
	switch  ( rcsMode ) {
	case RCS_ROT:
        v->SetThrusterGroupLevel( THGROUP_ATT_PITCHUP, KEYDOWN( kstate, OAPI_KEY_NUMPAD2 ) ? 1.0 : 0.0 );
        v->SetThrusterGroupLevel( THGROUP_ATT_PITCHDOWN, KEYDOWN( kstate, OAPI_KEY_NUMPAD8 ) ? 1.0 : 0.0 );
        v->SetThrusterGroupLevel( THGROUP_ATT_YAWLEFT, KEYDOWN( kstate, OAPI_KEY_NUMPAD1 ) ? 1.0 : 0.0 );
        v->SetThrusterGroupLevel( THGROUP_ATT_YAWRIGHT, KEYDOWN( kstate, OAPI_KEY_NUMPAD3 ) ? 1.0 : 0.0 );
        v->SetThrusterGroupLevel( THGROUP_ATT_BANKLEFT, KEYDOWN( kstate, OAPI_KEY_NUMPAD4 ) ? 1.0 : 0.0 );
        v->SetThrusterGroupLevel( THGROUP_ATT_BANKRIGHT, KEYDOWN( kstate, OAPI_KEY_NUMPAD6 ) ? 1.0 : 0.0 );
        break;

	case RCS_LIN:
        v->SetThrusterGroupLevel( THGROUP_ATT_RIGHT, KEYDOWN( kstate, OAPI_KEY_NUMPAD3 ) ? 1.0 : 0.0 );
        v->SetThrusterGroupLevel( THGROUP_ATT_LEFT, KEYDOWN( kstate, OAPI_KEY_NUMPAD1 ) ? 1.0 : 0.0 );
        v->SetThrusterGroupLevel( THGROUP_ATT_UP, KEYDOWN( kstate, OAPI_KEY_NUMPAD2 ) ? 1.0 : 0.0 );
        v->SetThrusterGroupLevel( THGROUP_ATT_DOWN, KEYDOWN( kstate, OAPI_KEY_NUMPAD8 ) ? 1.0 : 0.0 );
        v->SetThrusterGroupLevel( THGROUP_ATT_FORWARD, KEYDOWN( kstate, OAPI_KEY_NUMPAD6 ) ? 1.0 : 0.0 );
        v->SetThrusterGroupLevel( THGROUP_ATT_BACK, KEYDOWN( kstate, OAPI_KEY_NUMPAD9 ) ? 1.0 : 0.0 );
        break;

    default:
        break;
	}

	return 0;

}

void CPylon::clbkRCSMode (int mode) {

    if ( this->redirectKeysVesselHandle == NULL ) {
        return;
    }

    VESSEL *v = oapiGetVesselInterface( this->redirectKeysVesselHandle );
    if ( v == NULL ) {
        return;
    }

    v->SetAttitudeMode( mode );

}

void CPylon::clbkNavMode (int mode, bool active) {

    if ( this->redirectKeysVesselHandle == NULL ) {
        return;
    }

    VESSEL *v = oapiGetVesselInterface( this->redirectKeysVesselHandle );
    if ( v == NULL ) {
        return;
    }

    if ( active ) {
        v->ActivateNavmode( mode );
    }
    else {
        v->DeactivateNavmode( mode );
    }

}

void CPylon::clbkPostCreation (void)
{

	orbiterSoundId = ConnectToOrbiterSoundDLL(GetHandle());

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
//		DeleteSequence(i);
	}

	PylonSequence *ns = new PylonSequence(true, this);

	if(strcmp("STATE",name)==0) ns->activationKey = 0;

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

bool CPylon::ActivateSequenceByKey( DWORD key ) {

	bool b = false;
	PylonSequence *seq = sequences;
	int i = 0;
	while (seq) {
		if ( seq->activationKey == key ) {
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

// Returns the P ATTACHEMTNHANDLE which is connected to a parent, or NULL otherwise.
ATTACHMENTHANDLE CPylon::GetParentAttachment( VESSEL *v ) {

	for ( int i = 0, n = v->AttachmentCount( true ); i < n; i++ ) {
		ATTACHMENTHANDLE pAtt = v->GetAttachmentHandle( true, i );
		if ( v->GetAttachmentStatus( pAtt ) != NULL ) {
            return pAtt;
		}
	}

	return NULL;
}

VESSEL *CPylon::GetParent( VESSEL *v ) {

    ATTACHMENTHANDLE parentAttachment = CPylon::GetParentAttachment( v );

    if ( parentAttachment == NULL ) {
        return NULL;
    }

    OBJHANDLE parentHandle = v->GetAttachmentStatus( parentAttachment );

    if ( parentHandle == NULL ) {
        return NULL;
    }

    return oapiGetVesselInterface( parentHandle );

}

// Get root of vessel v in attachment tree
VESSEL *CPylon::GetRoot( VESSEL *v ) {

    OBJHANDLE o = NULL;
    while ( o == NULL ) {
        int i = 0;
        int n = v->AttachmentCount( true );
        while ( i < n && o == NULL ) {
            ATTACHMENTHANDLE top = v->GetAttachmentHandle( true, i );
            o = v->GetAttachmentStatus( top );
            i = i + 1;
        }
        if ( o == NULL ) break;
        v = oapiGetVesselInterface( o );
        if ( v == NULL) break;
        o = NULL;
    }

    return v;

}

double CPylon::GetChildMasses( VESSEL *v ) {

    double mass = 0;

    for ( int i = 0, n = v->AttachmentCount( false ); i < n; i++ ) {
        ATTACHMENTHANDLE toc = v->GetAttachmentHandle( false, i );
        OBJHANDLE o = v->GetAttachmentStatus( toc );
        if ( o != NULL ) {
            VESSEL *v = oapiGetVesselInterface( o );
            mass += v->GetMass() + CPylon::GetChildMasses( v );
        }
    }

    return mass;

}


void CPylon::addAttachedMasses( VESSEL *v, bool initialization ) {

    // Vessel v was just attached, so add its subtree mass to the root empty mass.

    VESSEL *root = CPylon::GetRoot( v );

    if ( root != v ) {
        if ( initialization ) {
            if ( CPylon::getFirstPylonDescendant( root ) == v ) {
                root->SetEmptyMass( root->GetEmptyMass() + CPylon::GetChildMasses( root ) );
            }
        }
        else {
            double cm = cm = CPylon::GetChildMasses( v );

            bool addChildMasses = CPylon::getFirstPylonDescendant( v ) == NULL;

            root->SetEmptyMass( root->GetEmptyMass() + v->GetMass() + ( addChildMasses ? cm : 0 ) );

            if ( v->GetEmptyMass() > cm ) {
                v->SetEmptyMass( v->GetEmptyMass() - cm );
            }
        }
    }
    else {
        if ( initialization ) {
            v->SetEmptyMass( v->GetEmptyMass() + CPylon::GetChildMasses( v ) );
        }
    }

}

void CPylon::subtractAttachedMasses( VESSEL *v ) {

    // The vessel v is about to be detached, so subtract its subtree mass from the root empty mass.

    VESSEL *root = CPylon::GetRoot( v );
    if ( root != v ) {

        double cm = CPylon::GetChildMasses( v );
        double m = v->GetMass() + cm;

        if ( root->GetEmptyMass() > m ) {
            root->SetEmptyMass( root->GetEmptyMass() - m );
        }

        v->SetEmptyMass( v->GetEmptyMass() + cm );

    }

}

CPylon *CPylon::getFirstPylonDescendant( VESSEL *v ) {

    CPylon *pv = CPylon::IsPylonVessel( v );

    if ( pv != NULL ) {
        return pv;
    }

    for ( int i = 0, n = v->AttachmentCount( false ); i < n; i++ ) {
        ATTACHMENTHANDLE toc = v->GetAttachmentHandle( false, i );
        OBJHANDLE o = v->GetAttachmentStatus( toc );
        if ( o != NULL ) {
            VESSEL *cv = oapiGetVesselInterface( o );
            CPylon *pcv = getFirstPylonDescendant( cv );
            if ( pcv != NULL ) {
                return pcv;
            }
        }
    }

    return NULL;

}

bool CPylon::PylonAttach( OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, ATTACHMENTHANDLE child_attachment ) {

    VESSEL *c = oapiGetVesselInterface( child );
	if ( c == NULL ) {
        return false;
	}

	CPylon *pc = IsPylonVessel( c );
	if ( pc != NULL ) {
        // Virtual version
        return pc->PylonAttachInternal( parent, child, parent_attachment, child_attachment );
	}
	else {
	    // Generic version
        return CPylon::AttachInternal( parent, child, parent_attachment, child_attachment );
	}

}

bool CPylon::PylonDetach( OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, double vel, bool leanParent ) {

    VESSEL *c = oapiGetVesselInterface( child );

	if ( c == NULL ) {
        return false;
	}
	CPylon *pc = IsPylonVessel( c );

	if ( pc != NULL ) {
        // Virtual version
        return pc->PylonDetachInternal( parent, child, parent_attachment, vel, leanParent );
	}
	else {

        VESSEL *p = oapiGetVesselInterface( parent );

        if ( p == NULL ) {
            return false;
        }
        CPylon *pp = IsPylonVessel( p );

        if ( pp != NULL ) {
            // Virtual version
            return pp->PylonDetachInternal( parent, child, parent_attachment, vel, leanParent );
        }
        else {
            // Generic version
            return CPylon::DetachInternal( parent, child, parent_attachment, vel, leanParent );
        }
	}

}

bool CPylon::PylonAttachInternal( OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, ATTACHMENTHANDLE child_attachment ) {

    return CPylon::AttachInternal( parent, child, parent_attachment, child_attachment );

}

bool CPylon::PylonDetachInternal( OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, double vel, bool leanParent ) {

	return CPylon::DetachInternal( parent, child, parent_attachment, vel, leanParent );

}

bool CPylon::AttachInternal( OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, ATTACHMENTHANDLE child_attachment ) {

	VESSEL *p = oapiGetVesselInterface( parent );
    VESSEL *c = oapiGetVesselInterface( child );

	if ( p == NULL || c == NULL ) {
        return false;
	}

	ATTACHMENTHANDLE attParent = CPylon::GetParentAttachment( c );
	if ( attParent != NULL ) {
        OBJHANDLE o = c->GetAttachmentStatus( attParent );
        if ( o != NULL ) {
            PylonDetach( o, c->GetHandle(), attParent, 0.0, false );
        }
	}

	if ( ! p->AttachChild( child, parent_attachment, child_attachment ) ) {
        return false;
	}

    CPylon::addAttachedMasses( c, false );

	return true;

}

bool CPylon::DetachInternal( OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, double vel, bool leanParent ) {

	if ( parent == NULL || child == NULL ) {
        return false;
	}

	VESSEL *p = oapiGetVesselInterface(parent);
    VESSEL *c = oapiGetVesselInterface(child);

	if ( p == NULL || c == NULL) {
	    return false;
    }

    CPylon::subtractAttachedMasses( c );

	return p->DetachChild( parent_attachment, vel );

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

// ==============================================================
// End of Pylon API
// ==============================================================


// ==============================================================
// Internal functions
// ==============================================================

bool CPylon::IsFirstFrame() {
	return firstFrame;
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

    if ( this->inited ) {
        return;
    }

    // Add child masses to root vessel
    CPylon::addAttachedMasses( this, true );


    // Execute STATE sequence
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

void CPylon::setKeyMap( bool lShift, bool rShift, bool lCtrl, bool rCtrl, bool lAlt, bool rAlt ) {

    for ( int i = 0; i < 256; i++ ) {
        this->keyMap[ i ] = 0;
    }

    char pressed = (char)0x80;

    if ( lShift ) {
        this->keyMap[ OAPI_KEY_LSHIFT ] = pressed;
    }

    if ( rShift ) {
        this->keyMap[ OAPI_KEY_RSHIFT ] = pressed;
    }

    if ( lCtrl ) {
        this->keyMap[ OAPI_KEY_LCONTROL ] = pressed;
    }

    if ( rCtrl ) {
        this->keyMap[ OAPI_KEY_RCONTROL ] = pressed;
    }

    if ( lAlt ) {
        this->keyMap[ OAPI_KEY_LALT ] = pressed;
    }

    if ( rAlt ) {
        this->keyMap[ OAPI_KEY_RALT ] = pressed;
    }

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

    veccpy( m->pos, pos );

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

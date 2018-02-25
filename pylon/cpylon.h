// ==============================================================
//
//  CPylon.h
//
//	ORBITER Vessel Interface
//
//	Author: Juan Jose Luna
//
//	Distributed MIT licensed. See License.txt
//
// ==============================================================

#ifndef __CPYLON_H
#define __CPYLON_H

#include <stdio.h>

#include "orbitersdk.h"
#include "OrbiterSoundSDK40.h"
#include "PylonSequence.h"

class PylonMesh
{
public:
	PylonMesh();
	PylonMesh* FindMesh(const char *n);
	PylonMesh* FindMeshName(const char *n);
	void Add(CPylon * p, unsigned char *filename, unsigned char *name, int visMode, VECTOR3 pos);
	void Clear();

	unsigned char file[NAME_SIZE];
	unsigned char name[NAME_SIZE];
	int visMode;
	int meshIndex;

	PylonMesh *prev, *next;
	CPylon *owner;
};

// ==========================================================
// Interface for derived vessel class: CPylon
// ==========================================================
// This class should not be subclassed! Subclass CPylonRT instead.
class CPylon: public VESSEL2 {
public:
	CPylon (OBJHANDLE hObj, int fmodel);
	~CPylon ();

    // ==============================================================
	// VESSEL2 callbacks
	// ==============================================================

	virtual void clbkPostCreation(void);
    virtual void clbkLoadStateEx (FILEHANDLE scn, void *status);
    virtual void clbkSaveState (FILEHANDLE scn);
    virtual void clbkPreStep(double simt, double simdt, double mjd);
    virtual int clbkConsumeBufferedKey (DWORD key, bool down, char *kstate);


	// ==============================================================
	// Pylon API
	// ==============================================================

	// Pylon paramaters management. Every CPylon subclass must override these functions

	virtual int GetParameterCount(void);
	virtual char *GetParameterName(int index);
	virtual int GetParameterType(int index);
	virtual bool IsUserParameter(int index);

	virtual bool GetParamBol(int index);
	virtual int GetParamInt(int index);
	virtual double GetParamDbl(int index);
	virtual char *GetParamStr(int index);

	virtual bool SetParam(int index, TPylParamValue *value);
	virtual bool SetParamBol(int index, bool value);//devolver bool que indica si ya ha terminado na mas. , PylSequence *caller = NULL);
	virtual bool SetParamInt(int index, int value);//, PylSequence *caller = NULL);
	virtual bool SetParamDbl(int index, double value);//, PylSequence *caller = NULL);
	virtual bool SetParamStr(int index, char *string);//, PylSequence *caller = NULL);

	virtual bool IncParam(int index);//, PylSequence *caller = NULL);
	virtual bool DecParam(int index);//, PylSequence *caller = NULL);

	virtual bool IsParameterSet(int index);

	// Common properties of pylons
	bool canNavigate,
		 userParametersEnabled,
		 userReleaseEnabled,
		 userCreateEnabled,
		 userDestroyEnabled,
		 actualizeNotPylonChilds;
	double releaseVel;

	VECTOR3 pos,dir,rot;

	// Sequence management functions

	int GetSequenceCount(void);
	char *GetSequenceName(int index);
	int GetSequenceType(int index);
	int GetSequenceCancelType(int index);
	bool IsUserSequence(int index);
	void SetUserSequence(int index, bool user);
	int CreateSequence(char *name, int type, int execmode);
	char *AddSequenceCmd(int index, char *cmd);
	bool IsSequenceValid(int index);
	bool ActivateSequence(int index);
	bool ActivateSequenceValue(int index, TPylParamValue *value);
	bool CancelSequence(int index);
	bool CancelSequenceValue(int index, TPylParamValue *value);
	bool IsSequenceActive(int index);
	bool IsSequenceCancelling(int index);
	int GetSequenceByName(char *name);
	void DeleteSequence(int index);
	bool ActivateSequenceByKey(DWORD key);

	int GetOrbiterSoundId()
	{
		return orbiterSoundId;
	}

	// Generic management

    static ATTACHMENTHANDLE GetParentAttachment( VESSEL *v );
    static VESSEL *GetParent( VESSEL *v );
    static VESSEL *GetRoot( VESSEL *v );
    static double GetChildMasses( VESSEL *v );

    static CPylon * IsPylonVessel(VESSEL *v);

	static bool PylonAttach(OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, ATTACHMENTHANDLE child_attachment);
	static bool PylonDetach(OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, double vel = 0.0);

	virtual bool PylonAttachInternal(OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, ATTACHMENTHANDLE child_attachment);
	virtual bool PylonDetachInternal(OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, double vel = 0.0);

	void addAttachedMasses( bool initialization );
	void subtractAttachedMasses();

	bool IsFirstFrame();

	// --------- End of API ---------

	int GetMFDSelectedParameter();
	void SetMFDSelectedParameter(int selectedParameter);

	ATTACHMENTHANDLE attToParent;

private:
	bool firstFrame;

	PylonSequence *sequences;
	int numSeq, icurSeq; PylonSequence *curSeq;

	bool inited;
	int mfdSelectedParameter;

	// Orbiter Sound Id
	int orbiterSoundId;

	// Sound names
	struct {
		char soundName[100];
	} soundNames[60];

	// Visual handle
	VISHANDLE visHandle;

	// Meshes
	PylonMesh *meshes;

	void initializePylon(void);
	void SelectSequence(int i);

	static bool AttachInternal(OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, ATTACHMENTHANDLE child_attachment);
	static bool DetachInternal(OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, double vel = 0.0);

	friend class PylonSequence;
	friend class PylonMesh;
};

#endif // !__CPYLON_H

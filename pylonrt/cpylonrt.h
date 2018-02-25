// ==============================================================
//
//  CPylonRT.h
//
//
//	Generic pylon for performing rotation and traslation.
//
//	Author: Juan Jose Luna
//
//	Distributed MIT licensed. See License.txt
//
// ==============================================================

#ifndef __CPYLONRT_H
#define __CPYLONRT_H

#include "orbitersdk.h"
#include "OrbiterSoundSDK40.h"
#include "..\Pylon\CPylon.h"

// Parameters constants
#define PYL_RT_PARAM_COUNT 22
static const char *PylonRTPNames[] = {"Rot_Angle","Traslation","Rot_Axis","Has_Traslation",
									"Lin_Accel", "Lin_Vel_Max", "Ang_Accel", "Ang_Vel_Max",
									"Ang_Min", "Ang_Max", "X0", "Y0", "Z0", "X1", "Y1", "Z1",
									"Clamp", "Curr_Trasl", "Lin_Vel", "Curr_Angle", "Ang_Vel",
									"Release_Ang_Vel"};
static const int PylonRTPTypes[] = { PYL_PARAM_ANGLE,
									 PYL_PARAM_DISTANCE,
									 PYL_PARAM_STRING,
									 PYL_PARAM_BOOLEAN,
									 PYL_PARAM_ACCELERATION,
									 PYL_PARAM_VELOCITY,
									 PYL_PARAM_ANGULARACC,
									 PYL_PARAM_ANGULARVEL,
									 PYL_PARAM_ANGLE,
									 PYL_PARAM_ANGLE,
									 PYL_PARAM_DISTANCE,
									 PYL_PARAM_DISTANCE,
									 PYL_PARAM_DISTANCE,
									 PYL_PARAM_DISTANCE,
									 PYL_PARAM_DISTANCE,
									 PYL_PARAM_DISTANCE,
									 PYL_PARAM_BOOLEAN,
									 PYL_PARAM_DISTANCE,
									 PYL_PARAM_VELOCITY,
									 PYL_PARAM_ANGLE,
									 PYL_PARAM_ANGULARVEL,
									 PYL_PARAM_ANGULARVEL };
static const bool PylonRTPUser[] = { true, true, true, true, true, true, true, true,
									true, true, true, true, true, true, true,
									true, true, true, true, true, true, true };
/*static const bool PylonRTPUser[] = { true, true, false, false, false, false, false, false,
									false, false, false, false, false, false, false,
									false, false, false, true, false, true, true };*/

/*static const bool PylonRTPUser[] = { true, true, true, true, true, true, true, true,
									true, true, true, true, true, true, true,
									true, true, true, true, true, true, false };
*/

// PylonRT rotation modes
enum PYL_RT_AXIS {
	PYL_RT_NO_ROTATION=0,
	PYL_RT_ROTATION_X,
	PYL_RT_ROTATION_Y,
	PYL_RT_ROTATION_Z,
	PYL_RT_ROTATION_LAST
};
static const char *PYL_RT_AXIS_NAMES[PYL_RT_ROTATION_LAST] = { "", "X", "Y", "Z" };

static VECTOR3 PYL_RT_AXIS_VECTORS[PYL_RT_ROTATION_LAST] = { _V(1, 0, 0), _V(1, 0, 0), _V(0, 1, 0), _V(0, 0, 1) };

// ==========================================================
// Interface for derived vessel class: CPylonRT
// ==========================================================

class CPylonRT: public CPylon {
public:
	CPylonRT (OBJHANDLE hObj, int fmodel);
	~CPylonRT ();

    // ==============================================================
	// VESSEL2 callbacks
	// ==============================================================
	virtual void clbkPostCreation( void );
    virtual void clbkPreStep(double simt, double simdt, double mjd);

	// ==============================================================
	// Pylon API
	// ==============================================================
	virtual int GetParameterCount(void);
	virtual char *GetParameterName(int index);
	virtual int GetParameterType(int index);
	virtual bool IsUserParameter(int index);

	virtual bool GetParamBol(int index);
	virtual int GetParamInt(int index);
	virtual double GetParamDbl(int index);
	virtual char *GetParamStr(int index);

	virtual bool SetParamBol(int index, bool value);
	virtual bool SetParamInt(int index, int value);
	virtual bool SetParamDbl(int index, double value);
	virtual bool SetParamStr(int index, char *string);

	virtual bool IncParam(int index);
	virtual bool DecParam(int index);

	virtual bool IsParameterSet(int index);
	//virtual void Changed(void);

	// --------- End of API ---------

	virtual bool PylonDetachInternal(OBJHANDLE parent, OBJHANDLE child, ATTACHMENTHANDLE parent_attachment, double vel = 0.0);

private:

	void actualizeTraslVectors(void);

	// User parameters
	double angRotSet, traslSet;

	// Sequence set parameters
	int rotAxis;
	bool hasTraslation;
	double linAccel, linVelMax, angAccel, angVelMax, angMin, angMax, releaseAngVel;
	VECTOR3 traslation0, traslation1;
	bool clampEnabled;

	// Internal state parameters
	double traslation, linVel, angRot, angVel;

	// Internal variables
	double traslationMod;
	VECTOR3 unitTrasl;
	bool doInit;

	VECTOR3 pos0,dir0,rot0;

	bool firstFrameAttached;

	friend class CPylon;
};

#endif // !__CPYLONRT_H

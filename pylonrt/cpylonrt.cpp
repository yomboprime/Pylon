// ==============================================================
//
//  CPylonRT.cpp
//
//
//	Generic pylon for performing rotation and traslation.
//
//	Author: Juan Jose Luna
//
//	Distributed MIT licensed. See License.txt
//
// ==============================================================

#define STRICT

#include "CPylonRT.h"
#include <stdio.h>
//#include <math.h>

// Constructor
CPylonRT::CPylonRT (OBJHANDLE hObj, int fmodel)
: CPylon (hObj, fmodel)
{
	angRotSet = 0, traslSet = 0;

	rotAxis = PYL_RT_ROTATION_X;
	hasTraslation = false;

	linAccel = 1, linVelMax = 5,
	traslation0 = _V(-0.075,0,-2), traslation1 = _V(0.075,0,10);
	angAccel = 2*PI/180.0, angVelMax = 20*PI/180.0, angMin = -PI, angMax = PI;

	releaseAngVel = 0;

	clampEnabled = true;

	traslation = 0, linVel = 0, angRot = 0, angVel = 0;

	actualizeTraslVectors();

	doInit = true;
}

// Destructor
CPylonRT::~CPylonRT ()
{
	//probar de destruirlo y ver si se ejecuta este destructor y el de CPylon
}

// Frame update
void CPylonRT::clbkPreStep(double simt, double simdt, double mjd)
{

	this->attToParent = CPylon::GetParentAttachment( this );
    if ( this->attToParent == NULL) {
		CPylon::clbkPreStep( simt, simdt, mjd );
		return;
	}

	GetAttachmentParams( attToParent, pos, dir, rot );

	if ( rotAxis != PYL_RT_NO_ROTATION ) {
		if ( this->IsFirstFrame() || angRot != angRotSet ) {
			doInit = false;
			if (angVelMax==0) {
				angRot = angRotSet;
			} else {

				double signAngVel = angVel<0?-1:1;
				double absAngVel = angVel<0?-angVel:angVel;
				double dist = angRotSet - angRot;
				double signTgtAng;
				if (dist != 0) signTgtAng = (dist < 0)?-1:1;
				else signTgtAng = 0;
				if (dist < 0) dist = -dist;

				// Select direction
				double forward = 0;
				if (clampEnabled) {
					if (signTgtAng!=0) {
						forward = signTgtAng;
						if (angAccel > 0) {
							if (signTgtAng*signAngVel==1) {
								double distBrake = 0.5*angVel*angVel/angAccel;
								if (distBrake > dist) {
									if (clampEnabled) angVel = sqrt(2*angAccel*dist) * signAngVel;
									forward = -signTgtAng;
								}
//sprintf(oapiDebugString(),"angVel=%f, DistBrake = %f, %s",angVel,distBrake, forward==1?"ACCEL\0":"BRAKE\0");
							}
						}
					}
				} else {
					if (angRotSet!=0) forward = (angRotSet>0)?1:-1;
				}

				// Cinematics
				if (forward!=0) {
					double antSignAngVel = signAngVel;
					if ( angVel == 0 ) antSignAngVel = 0;
					angVel += forward * angAccel * simdt;
					signAngVel = angVel<0?-1:1;
					absAngVel = angVel<0?-angVel:angVel;
					if ( antSignAngVel != 0 && antSignAngVel != signAngVel ) {
						angRotSet = angRot;
						angVel = 0;
					} else {
						if (absAngVel > angVelMax) angVel = signAngVel * angVelMax;
					}
				}

				angRot += angVel * simdt;

				double newsignTgtAng;
				dist = angRotSet - angRot;
				if (dist != 0) newsignTgtAng = (dist < 0)?-1:1;
				else newsignTgtAng = 0;
				if (dist < 0) dist = -dist;

				if (clampEnabled) {
					if (angRot < angMin) { angRot = angMin; }
					if (angRot > angMax) { angRot = angMax; }
					if (newsignTgtAng != signTgtAng) {
						angRot = angRotSet;
						angVel = 0;
					}
				} else {
					while (angRot >= PI) { angRot -= 2*PI; }
					while (angRot < -PI) { angRot += 2*PI; }
				}
			}

			// Transformation
			double s = sin(angRot), c = cos(angRot);
			switch (rotAxis) {
				case PYL_RT_ROTATION_X:
					dir = _V(0, s,-c);
					rot = _V(0, c, s);
					break;
				case PYL_RT_ROTATION_Y:
					dir = _V(-s, 0, -c);
					rot = _V(0, 1, 0);
					break;
				case PYL_RT_ROTATION_Z:
					dir = _V(0, 0, -1);
					rot	= _V(s, c, 0);
					break;
			}
		}
	}


	if (hasTraslation) {
        doInit = false;
        if (linVelMax==0) {
            traslation = traslSet;
        } else {

            double signLinVel = linVel<0?-1:1;
            double absLinVel = linVel<0?-linVel:linVel;
            double dist = traslSet - traslation;
            double signTgtTrasl;
            if (dist != 0) signTgtTrasl = (dist < 0)?-1:1;
            else signTgtTrasl = 0;
            if (dist < 0) dist = -dist;

            // Select direction
            double forward = 0;
            if (clampEnabled) {
                if (signTgtTrasl!=0) {
                    forward = signTgtTrasl;
                    if (linAccel > 0) {
                        if (signTgtTrasl*signLinVel==1) {
                            double distBrake = 0.5*linVel*linVel/linAccel;
                            if (distBrake > dist) {
                                if (clampEnabled) linVel = sqrt(2*linAccel*dist) * signLinVel;
                                forward = -signTgtTrasl;
                            }
                        }
                    }
                }
            } else {
                if (traslSet!=0) forward = (traslSet>0)?1:-1;
            }

            // Cinematics
            if (forward!=0) {
                linVel += forward * linAccel * simdt;
                signLinVel = linVel<0?-1:1;
                absLinVel = linVel<0?-linVel:linVel;
                if (absLinVel > linVelMax) linVel = signLinVel * linVelMax;
            }

            traslation += linVel * simdt;

            double newsignTgtTrasl;
            dist = traslSet - traslation;
            if (dist != 0) newsignTgtTrasl = (dist < 0)?-1:1;
            else newsignTgtTrasl = 0;
            if (dist < 0) dist = -dist;

            if (clampEnabled) {
                if (traslation < 0) { traslation = 0; }
                if (traslation > traslationMod) { traslation = traslationMod; }
                if (newsignTgtTrasl != signTgtTrasl) {
                    traslation = traslSet;
                    linVel = 0;
                }
            } else {
            }
        }

        // Traslation
        pos = traslation0 + unitTrasl*traslation;
        pos.x = - pos.x;
        pos.y = - pos.y;
        pos.z = - pos.z;

	}

	CPylon::clbkPreStep(simt, simdt, mjd);
}

// ==============================================================
// OVC functions
// ==============================================================

// Initialisation
DLLCLBK VESSEL *ovcInit (OBJHANDLE hvessel, int flightmodel)
{
    // D. Beachy: GROW THE STACK HERE SO WE CAN USE BOUNDSCHECKER FOR DEBUGGING
    // I believe the reason we need this is because BoundsChecker (for this object) grows the stack more than 1 full page
    // at once, skipping over the guard page that Winvoid CPylonRT::clbkPreStep(double simt, double simdt, double mjd)dows places below the stack to grow it automatically.
    // Therefore we will grow the stack here.
    // TODO: COMMENT THIS OUT once BoundsChecker debugging is complete
    int pageCount = 250;  // 250 4K pages = reserve 1 MB of stack
    DWORD dwStackDelta = 0; // total # of stack bytes used
    for (int i=0; i < pageCount; i++)
    {
        dwStackDelta += 4096;
        __asm
        {
            sub     esp, 4092;  // 1 page - 4 bytes
            push    0xFEEDFEED  // touch the page
        }
    }

    // now pop the stack we touched
    __asm
    {
        mov     eax, [dwStackDelta] // size in bytes
        add     esp, eax
    }


	return new CPylonRT (hvessel, flightmodel);
}

/*DLLCLBK void ovcPostCreation (VESSEL *vessel) {
}*/

void CPylonRT::clbkPostCreation( void ) {

    CPylon::clbkPostCreation();

    actualizeTraslVectors();

}

// Cleanup
DLLCLBK void ovcExit (VESSEL *vessel)
{
	if (vessel) delete (CPylonRT*)vessel;
}

// Set the capabilities of the vessel class
/*DLLCLBK void ovcSetClassCaps (VESSEL *vessel, FILEHANDLE cfg)
{
	((CPylon...*)vessel)->SetClassCaps (cfg);
}*/

/*
// Read status from scenario file
DLLCLBK void ovcLoadStateEx (VESSEL *vessel, FILEHANDLE scn, void *vs)
{
	((CPylon*)vessel)->LoadState (scn, vs);
}

// Save status to scenario file
DLLCLBK void ovcSaveState (VESSEL *vessel, FILEHANDLE scn)
{
	((CPylon*)vessel)->SaveState (scn);
}

DLLCLBK void ovcTimestep (VESSEL *vessel, double simt)
{
	((CPylonRT*)vessel)->Timestep (simt);
}

DLLCLBK int ovcConsumeBufferedKey (VESSEL *vessel, DWORD key, bool down, char *kstate)
{
	if (!down) return 0; // only process keydown events

	CPylonRT *p = (CPylonRT*)vessel;

	if (KEYMOD_SHIFT (kstate)) {
	} else if (KEYMOD_CONTROL (kstate)) {
//		switch (key) {
//		case OAPI_KEY_:
//			return 1;
//		}

	} else {
		if (((CPylon*)p)->ActivateSequenceByKey(key)) {
			return 1;
		}
	}
	return 0;
}
*/

// ==============================================================
// Pylon API
// ==============================================================

int CPylonRT::GetParameterCount(void)
{
	return PYL_RT_PARAM_COUNT + CPylon::GetParameterCount();
}

char *CPylonRT::GetParameterName(int index)
{
	if (index<0) return NULL;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::GetParameterName(index - PYL_RT_PARAM_COUNT);
	return (char*)PylonRTPNames[index];
}

int CPylonRT::GetParameterType(int index)
{
	if (index<0) return PYL_PARAM_NOT_DEFINED;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::GetParameterType(index - PYL_RT_PARAM_COUNT);
	return PylonRTPTypes[index];
}

bool CPylonRT::IsUserParameter(int index) {
	if (index<0) return false;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::IsUserParameter(index - PYL_RT_PARAM_COUNT);
	return PylonRTPUser[index];
}

bool CPylonRT::GetParamBol(int index)
{
	if (index<0) return false;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::GetParamBol(index - PYL_RT_PARAM_COUNT);
	switch (index) {
		case  3: return hasTraslation;
		case 16: return clampEnabled;
		default: return false;
	}
}

int CPylonRT::GetParamInt(int index)
{
	if (index<0) return 0;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::GetParamInt(index - PYL_RT_PARAM_COUNT);
	return 0;
}

double CPylonRT::GetParamDbl(int index)
{
	if (index<0) return 0;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::GetParamDbl(index - PYL_RT_PARAM_COUNT);
	switch (index) {
		case  0:	return angRotSet*180.0/PI;
		case  1:	return traslation;
		case  4:	return linAccel;
		case  5:	return linVelMax;
		case  6:	return angAccel*180.0/PI;
		case  7:	return angVelMax*180.0/PI;
		case  8:	return angMin*180.0/PI;
		case  9:	return angMax*180.0/PI;
		case 10:	return traslation0.x;
		case 11:	return traslation0.y;
		case 12:	return traslation0.z;
		case 13:	return traslation1.x;
		case 14:	return traslation1.y;
		case 15:	return traslation1.z;
		case 17:	return traslation;
		case 18:	return linVel;
		case 19:	return angRot*180.0/PI;
		case 20:	return angVel*180.0/PI;
		case 21:	return releaseAngVel*180.0/PI;
		default: return 0;
	}
}

char *CPylonRT::GetParamStr(int index)
{
	if (index<0) return NULL;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::GetParamStr(index - PYL_RT_PARAM_COUNT);
	if (index==2) return (char *)PYL_RT_AXIS_NAMES[rotAxis];
	return NULL;
}

bool CPylonRT::SetParamBol(int index, bool value)
{
	if (index<0) return true;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::SetParamBol(index - PYL_RT_PARAM_COUNT, value);
	switch (index) {
		case  3: {
			hasTraslation = value; doInit = true; return true; }
		case 16: { clampEnabled = value; doInit = true; return true; }
		default: return true;
	}
}

bool CPylonRT::SetParamInt(int index, int value)
{
	if (index<0) return true;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::SetParamInt(index - PYL_RT_PARAM_COUNT, value);
	return true;
}

bool CPylonRT::SetParamDbl(int index, double value)
{
	if (index<0) return true;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::SetParamDbl(index - PYL_RT_PARAM_COUNT, value);

	double signVel, absVel;

	switch (index) {
		case  0:
			angRotSet = value*PI/180.0;
			if (clampEnabled) {
				if (angRotSet < angMin) { angRotSet = angMin; }
				if (angRotSet > angMax) { angRotSet = angMax; }
			}
			break;
		case  1:
			traslSet = value;
			if (clampEnabled) {
				if (traslSet < 0) { traslSet = 0; }
				if (traslSet > traslationMod) { traslSet = traslationMod; }
			}
			break;
		case  4:
			linAccel = value;
			doInit = true;
			return true;
		case  5:
			linVelMax = value;
			signVel = linVel<0?-1:1;
			absVel = linVel<0?-linVel:linVel;
			if (absVel > linVelMax) linVel = signVel * linVelMax;
			doInit = true;
			return true;
		case  6:
			angAccel = value*PI/180.0;
			doInit = true;
			return true;
		case  7:
			angVelMax = value*PI/180.0;
			signVel = angVel<0?-1:1;
			absVel = angVel<0?-angVel:angVel;
			if (absVel > angVelMax) angVel = signVel * angVelMax;
			doInit = true;
			return true;
		case  8:
			angMin = value*PI/180.0;
			if (clampEnabled) {
				if (angRotSet < angMin) { angRotSet = angMin; }
				if (angRotSet > angMax) { angRotSet = angMax; }
				if (angRot < angMin) { angRot = angMin; }
				if (angRot > angMax) { angRot = angMax; }
			}
			doInit = true;
			return true;
		case  9:
			angMax = value*PI/180.0;
			if (clampEnabled) {
				if (angRotSet < angMin) { angRotSet = angMin; }
				if (angRotSet > angMax) { angRotSet = angMax; }
				if (angRot < angMin) { angRot = angMin; }
				if (angRot > angMax) { angRot = angMax; }
			}
			doInit = true;
			return true;
		case 10: traslation0.x = value; actualizeTraslVectors(); return true;
		case 11: traslation0.y = value; actualizeTraslVectors(); return true;
		case 12: traslation0.z = value; actualizeTraslVectors(); return true;
		case 13: traslation1.x = value; actualizeTraslVectors(); return true;
		case 14: traslation1.y = value; actualizeTraslVectors(); return true;
		case 15: traslation1.z = value; actualizeTraslVectors(); return true;
		case 17:
			traslation = value;
			if (clampEnabled) {
				if (traslation < 0) { traslation = 0; }
				if (traslation > traslationMod) { traslation = traslationMod; }
			}
			doInit = true;
			return true;
		case 18:
			linVel = value;
			signVel = linVel<0?-1:1;
			absVel = linVel<0?-linVel:linVel;
			if (absVel > linVelMax) linVel = signVel * linVelMax;
			doInit = true;
			return true;
		case 19:
			angRot = value*PI/180.0;
			if (clampEnabled) {
				if (angRot < angMin) { angRot = angMin; }
				if (angRot > angMax) { angRot = angMax; }
			}
			doInit = true;
			return true;
		case 20:
			angVel = value*PI/180.0;
			signVel = angVel<0?-1:1;
			absVel = angVel<0?-angVel:angVel;
			if (absVel > angVelMax) angVel = signVel * angVelMax;
			doInit = true;
			return true;
		case 21:
			releaseAngVel = value*PI/180.0;
			return true;
		default: return true;
	}
	return IsParameterSet(index);
}

bool CPylonRT::SetParamStr(int index, char *string)
{
	if (index<0) return true;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::SetParamStr(index - PYL_RT_PARAM_COUNT, string);
	if (index==2) {
		for (int i=0;i<PYL_RT_ROTATION_LAST;i++) {
			if (strcmp(string, PYL_RT_AXIS_NAMES[i])==0) {
				rotAxis = i; return true;
			}
		}
	}
	return true;
}

bool CPylonRT::IncParam(int index)
{
	// Behavior: increment by 1 degree
	if (index<0) return true;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::IncParam(index - PYL_RT_PARAM_COUNT);
	switch (index) {
		case 0:
			angRotSet += 10*PI/180.0;
			if (clampEnabled) {
				if (angRotSet < angMin) { angRotSet = angMin; }
				if (angRotSet > angMax) { angRotSet = angMax; }
			}
			break;
		case 1:
			traslSet += 1;
			if (clampEnabled) {
				if (traslSet < 0) { traslSet = 0; }
				if (traslSet > traslationMod) { traslSet = traslationMod; }
			}
			break;
		default: return true;
	}
	return true;
}

bool CPylonRT::DecParam(int index)
{
	// Behavior: decrement by 1 degree
	if (index<0) return true;
	if (index>PYL_RT_PARAM_COUNT)
		CPylon::DecParam(index - PYL_RT_PARAM_COUNT);
	switch (index) {
		case 0:	angRotSet -= 10*PI/180.0; break;
		case 1:	traslSet -= 1; break;
		default: return true;
	}
	return true;
}

bool CPylonRT::IsParameterSet(int index)
{
	if (index<0) return true;
	if (index>PYL_RT_PARAM_COUNT)
		return CPylon::IsParameterSet(index - PYL_RT_PARAM_COUNT);
	switch (index) {
		case 0:
			if (clampEnabled) {
				return 	angRot == angRotSet;
			} else {
				if (angRotSet==0) return true;
				if (angVelMax==fabs(angVel)) return true;
				return false;
			}

		case 1:
			if (clampEnabled) {
				return traslation == traslSet;
			} else {
				if (traslSet==0) return true;
				if (linVelMax==fabs(linVel)) return true;
				return false;
			}

		default:return true;
	}
}

/*void CPylonRT::Changed(void)
{
	CPylon::Changed();

	// Do other things here
}*/

void CPylonRT::actualizeTraslVectors(void) {
	unitTrasl = traslation1 - traslation0;
	traslationMod = length(unitTrasl);
	if (traslationMod < 0.000001) {
		traslation1 = traslation0 + _V(0,0,1);
		unitTrasl = traslation1 - traslation0;
		traslationMod = length(unitTrasl);
	}
	unitTrasl *= 1/traslationMod;
	pos = traslation0 + unitTrasl*traslation;
	pos.x = - pos.x;
	pos.y = - pos.y;
	pos.z = - pos.z;
}
// ==============================================================
// End of Pylon API
// ==============================================================


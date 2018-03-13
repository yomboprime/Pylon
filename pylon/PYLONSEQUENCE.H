//	Distributed MIT licensed. See License.txt

#ifndef __PYLONSEQUENCE_H
#define __PYLONSEQUENCE_H

#include "orbitersdk.h"

// ==========================================================
// Constants
// ==========================================================
#define NAME_SIZE 256

// Type of pylon parameters
enum TPylParameter {

	// Boolean
	PYL_PARAM_BOOLEAN = 0,
	PYL_PARAM_BOOLEAN_SWITCH,
	PYL_PARAM_BOOLEAN_ACTIVATION,

	// Integer
	PYL_PARAM_INTEGER,

	// Scalar (double) types:
	PYL_PARAM_SCALAR,
	PYL_PARAM_UNITARY,	// Between 0 and 1
	PYL_PARAM_DISTANCE,
	PYL_PARAM_VELOCITY,
	PYL_PARAM_ACCELERATION,
	PYL_PARAM_ANGLE,
	PYL_PARAM_ANGULARVEL,
	PYL_PARAM_ANGULARACC,

	// String types:
	PYL_PARAM_STRING,
	PYL_PARAM_OBJECT_NAME,
	PYL_PARAM_VESSEL_NAME,
	PYL_PARAM_VESSEL_CLASS,
	PYL_PARAM_GBODY_NAME,
	PYL_PARAM_BASE_NAME,
	PYL_PARAM_PARAMETER_NAME,
	PYL_PARAM_SEQUENCE_NAME,
	PYL_PARAM_PYLON_NAME,

	// Parameter not defined (in sequence values it means the '#')
	PYL_PARAM_NOT_DEFINED
};


// Additional types of sequence values
enum TPylSequenceValue {
	PYL_PARAM_GET_PARAMETER = -1, // Symbol '?'. Has one parameter, of type PYL_PARAM_PARAMETER_NAME (in strValue)
	PYL_PARAM_GENERATE_UNIFORM = -2, // symbol '_U'. Has two parameters, min and max (double)
	PYL_PARAM_GENERATE_NORMAL = -3, // symbol '_N'. Has two parameters, mean and stddesv (double)
	PYL_PARAM_THIS = -4
};

// Additional types of sequence tokens
enum TPylTokenType {
	PYL_PARAM_TOKEN_COMMAND = -1000, // intValue has the command code
	PYL_PARAM_TOKEN_BEGINSEQ = -1001,
	PYL_PARAM_TOKEN_ENDSEQ = -1002,
	PYL_PARAM_TOKEN_PARALLEL = -1003,
	PYL_PARAM_TOKEN_OPENBR = -1004,
	PYL_PARAM_TOKEN_CLOSEBR = -1005,
	PYL_PARAM_TOKEN_COMMA = -1006,
	PYL_PARAM_TOKEN_USER = -1007,
	PYL_PARAM_TOKEN_WAITONE = -1008,
	PYL_PARAM_LAST_TOKEN = -1009
};

//  Description in the files:
//   Bool: {0, 1}
//   Integer and Scalar: {-+.(0..9)eE}*
//   String: "{}*"
//   Sequence parameter: # (type: pylon_param_notdefined)
//   Input parameter value: ?paramname or ?objname:paramname (type: pylon_param_get_parameter)
// keywords: _THIS, _U(f,f), _N(f,f)
// All types are mutually castable.

// Struct for parameter value
#define PYL_NUM_BOL_ARRAY_SIZE (NAME_SIZE/sizeof(bool))
#define PYL_NUM_INT_ARRAY_SIZE (NAME_SIZE/sizeof(int))
#define PYL_NUM_DBL_ARRAY_SIZE (NAME_SIZE/sizeof(double))
#define PYL_NUM_VEC_ARRAY_SIZE (NAME_SIZE/sizeof(VECTOR3))

typedef struct {
	int type;
	union {
		struct {
			bool bolValue;
			bool booleanArray[PYL_NUM_BOL_ARRAY_SIZE];
		};
		struct {
			int intValue;
			int integerArray[PYL_NUM_INT_ARRAY_SIZE];
		};
		struct {
			double dblValue;
			double doubleArray[PYL_NUM_DBL_ARRAY_SIZE];
		};
		struct {
			double vecValue;
			VECTOR3 vectorArray[PYL_NUM_VEC_ARRAY_SIZE];
		};
		struct {
			char strValue[NAME_SIZE];
			union {void *tempRef; OBJHANDLE tempH;};
			int tempIndex;
			double tempDbl;
		};
	};
} TPylParamValue;

// Type of sequence commands
enum TPylSeqCommand {

	// Set parameter
	PYL_CMD_SET_PARAM=0,

	// Call sequence and  cancel sequence
	PYL_CMD_CALL_SEQ,
	PYL_CMD_CANCEL_SEQ,
	PYL_CMD_TERM_SEQ,

	// Sets the cancel sequence. Called when the sequence is cancelled
	PYL_CMD_SET_CANCEL_SEQUENCE,

	// Sets the key activation for the sequence
	PYL_CMD_SET_SEQUENCE_KEY,

	// Logic control
	PYL_CMD_RESET,
	PYL_CMD_SKIP_IF,
	PYL_CMD_SKIP_IF_NOT,
	PYL_CMD_RESET_IF,
	PYL_CMD_RESET_IF_NOT,

	// Time events
	PYL_CMD_DELAY,
	PYL_CMD_DELAY_UNTIL_MJD,
	PYL_CMD_DELAY_UNTIL_SIMT,

	// Other events
	PYL_CMD_WAIT_SEQUENCE,
	PYL_CMD_WAIT_FUEL_LEVEL,
	PYL_CMD_WAIT_THRUSTER_LEVEL,

	//WAIT_COLLISION
	//WAIT_COLLISION_CENTER

	// Vessel creation and destruction
	PYL_CMD_ATTACH,
	PYL_CMD_ATTACH_DISTANCE,
	PYL_CMD_ATTACH_ALIGNED,
	PYL_CMD_RELEASE,
	PYL_CMD_RELEASE_ATT,
	PYL_CMD_CREATE_ATTACHED,
	PYL_CMD_DESTROY,

	// Setting vessel status
	PYL_CMD_SET_FUEL,
	PYL_CMD_TRANSFER_FUEL,
	PYL_CMD_SET_MAIN_THRUST,
	PYL_CMD_SET_RETRO_THRUST,
	PYL_CMD_SET_HOVER_THRUST,
	PYL_CMD_SET_THRUST,
	PYL_CMD_CHANGE_THRUST_DIRECTION,

	PYL_CMD_SET_CAMERA_OFFSET,
	PYL_CMD_SET_CAMERA_DIRECTION,
	PYL_CMD_CREATE_ATTACHMENT,
	PYL_CMD_SET_TOCHILD_ATT,
	PYL_CMD_SET_ENABLE_FOCUS,

	// Rotating default thrusters
//	PYL_CMD_TRIM_MAIN, // vesel angle vel
//	PYL_CMD_TRIM_RETRO, // vesel angle vel
//	PYL_CMD_TRIM_HOVER, // vessel angle vel
//	PYL_CMD_SET_MAIN_TO_COG, // vessel axis(string)[x,y,z,all] vel
//	PYL_CMD_SET_RETRO_TO_COG, // vessel axis vel
//	PYL_CMD_SET_HOVER_TO_COG, // vessel axis vel

	// OrbiterSound interface
	PYL_CMD_SOUND_LOAD,
	PYL_CMD_SOUND_OPTION,
	PYL_CMD_SOUND_PLAY,
	PYL_CMD_SOUND_STOP,
	PYL_CMD_SOUND_REPLACE_STOCK,

	// Mesh handling
	PYL_CMD_ADD_MESH,
	PYL_CMD_SET_MESH_VISIBILITY,
	PYL_CMD_CLEAR_MESHES,

	// CPylon common properties management
	PYL_CMD_USER_SEQ_ENABLED,
	PYL_CMD_USER_RELEASE_ENABLED,
	PYL_CMD_USER_CREATE_ENABLED,
	PYL_CMD_USER_DESTROY_ENABLED,
	PYL_CMD_ACTUALIZE_NP_CHILDS_ENABLED,
	PYL_CMD_CAN_NAVIGATE,
	PYL_CMD_USER_PARAMETERS_ENABLED,
	PYL_CMD_CURRENT_MFD_PARAM,

	// Other generic commands
	PYL_CMD_SET_CAMERA_FOV,
	PYL_CMD_SET_FOCUS,
	PYL_CMD_SEND_KEY,

	PYL_CMD_REDIRECT_KEYS,

	// PYL_CMD_CREATE_ATTACH_AT_DOCK
	// PYL_CMD_CREATE_DOCK_AT_ATTACH

	PYL_CMD_NOT_DEFINED // Not usable (command parameter)
};

// Sequences execution modes
enum TSeqMode {
	SEQ_MODE_SEQUENTIAL,
	SEQ_MODE_PARALLEL,
	SEQ_MODE_WAIT_ONE,
	SEQ_MODE_LAST
};


// ==========================================================
// Pylon Sequence Class
// ==========================================================
class CPylon;

class PylonSequence {
public:
	bool userSequence;

private:
	PylonSequence(bool mainSeq, void *owner);
	~PylonSequence();
	char *AddCmd(char *cmd, TPylParamValue &val);
	void SetCancelSequence(PylonSequence *ps);
	bool Call(TPylParamValue *val, PylonSequence *caller);
	bool Cancel(TPylParamValue *val);
	bool Execute(void);

	bool CastValue(int cmdType, TPylParamValue *val);
	bool ConvertValue(int destType, int sourceType, TPylParamValue *source, bool doValidate);
	bool Validate(int destType, int srcType);
	static char *GetToken(char *line, TPylParamValue &value);
	void Print(char *str, int strSize);
	void Terminate(void);

	bool mainSequence, valid,
		 activate, cancel,
		 activating, cancelling, finished, finishedcommand;

	int mode;

	char *name;
	void *owner;
	PylonSequence *caller;
	union {
		PylonSequence *cmdcurrent;
		int cmdcode;
	};
	TPylParamValue value;

	DWORD activationKey;

	PylonSequence *nextCmd, *nextSeq;

	friend class CPylon;
};



// ==========================================================
// Constant strings
// ==========================================================


// Name of parameter types
static char PylonParamTypeName[PYL_PARAM_NOT_DEFINED][32] = {
	// Boolean
	"Boolean",
	"Switch",
	"Activation",

	// Integer
	"Integer",

	// Scalar (double) types:
	"Scalar",
	"Unitary",
	"Distance",
	"Velocity",
	"Acceleration",
	"Angle",
	"Angular velocity",
	"Angular acceleration",

	// String types:
	"String",
	"Object name",
	"Vessel name",
	"Vessel class",
	"GBody name",
	"Base name",
	"Parameter name",
	"Sequence name",
	"Pylon name"
};

// Names of boolean parameter types
// Access: [value][boolean type][] (0 < boolean type < PYL_PARAM_INTEGER)
static char PylonBoolParamTypeName[PYL_PARAM_INTEGER][2][16] = {
	// Boolean
	{"False","True"},
	{"Off","On"},
	{"Inactive","Active"}
};

// ==========================================================
// Table of command mnemonics and mandatory parameters
// ==========================================================
static struct {
	char *name;
	int numParams;
	int paramType[12];
}
PylonCommands[PYL_CMD_NOT_DEFINED] = {

	{"SET_PARAM\0",2,{PYL_PARAM_PARAMETER_NAME, PYL_PARAM_NOT_DEFINED,0,0,0,0,0,0,0,0,0,0}},

	{"CALL_SEQ\0",1,{PYL_PARAM_SEQUENCE_NAME, 0,0,0,0,0,0,0,0,0,0,0}},
	{"CANCEL_SEQ\0",2,{PYL_PARAM_SEQUENCE_NAME, PYL_PARAM_NOT_DEFINED,0,0,0,0,0,0,0,0,0,0}},
	{"TERM_SEQ\0",1,{PYL_PARAM_SEQUENCE_NAME, 0,0,0,0,0,0,0,0,0,0,0}},
	{"SET_CANCEL_SEQUENCE\0",1,{PYL_PARAM_SEQUENCE_NAME, 0,0,0,0,0,0,0,0,0,0,0}},
	{"SET_SEQUENCE_KEY\0",2,{PYL_PARAM_SEQUENCE_NAME,PYL_PARAM_INTEGER, 0,0,0,0,0,0,0,0,0,0}},

	{"RESET\0",0,{0, 0,0,0,0,0,0,0,0,0,0,0}},
	{"**SKIP_IF\0",1,{PYL_PARAM_BOOLEAN, 0,0,0,0,0,0,0,0,0,0,0}},
	{"**SKIP_IF_NOT\0",1,{PYL_PARAM_BOOLEAN, 0,0,0,0,0,0,0,0,0,0,0}},
	{"**RESET_IF\0",1,{PYL_PARAM_BOOLEAN, 0,0,0,0,0,0,0,0,0,0,0}},
	{"**RESET_IF_NOT\0",1,{PYL_PARAM_BOOLEAN, 0,0,0,0,0,0,0,0,0,0,0}},

	{"DELAY\0",1,{PYL_PARAM_SCALAR, 0,0,0,0,0,0,0,0,0,0,0}},
	{"DELAY_UNTIL_MJD\0",1,{PYL_PARAM_SCALAR, 0,0,0,0,0,0,0,0,0,0,0}},
	{"DELAY_UNTIL_SIMT\0",1,{PYL_PARAM_SCALAR, 0,0,0,0,0,0,0,0,0,0,0}},

	{"WAIT_SEQUENCE\0",1,{PYL_PARAM_SEQUENCE_NAME, 0,0,0,0,0,0,0,0,0,0}},
	{"WAIT_FUEL_LEVEL\0",3,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_SCALAR, 0,0,0,0,0,0,0,0,0}},
	{"WAIT_THRUSTER_LEVEL\0",3,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_SCALAR, 0,0,0,0,0,0,0,0,0}},

	{"ATTACH\0",4,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_INTEGER, PYL_PARAM_VESSEL_NAME,0,0,0,0,0,0,0,0}},
	{"ATTACH_DISTANCE\0",5,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_INTEGER, PYL_PARAM_VESSEL_NAME,PYL_PARAM_SCALAR,0,0,0,0,0,0,0}},
	{"ATTACH_ALIGNED\0",6,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_INTEGER, PYL_PARAM_VESSEL_NAME,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,0,0,0,0,0,0}},
	{"RELEASE\0",2,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_SCALAR, 0,0,0,0,0,0,0,0,0,0}},
	{"RELEASE_ATT\0",4,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_SCALAR, PYL_PARAM_INTEGER,0,0,0,0,0,0,0,0}},
	{"CREATE_ATTACHED\0",5,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_INTEGER, PYL_PARAM_STRING,PYL_PARAM_STRING,0,0,0,0,0,0,0}},
	{"DESTROY\0",1,{PYL_PARAM_VESSEL_NAME, 0, 0,0,0,0,0,0,0,0,0,0}},

	{"SET_FUEL_LEVEL\0",3,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_SCALAR, 0,0,0,0,0,0,0,0,0}},
	{"TRANSFER_FUEL\0",5,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_SCALAR, 0,0,0,0,0,0,0}},
	{"SET_MAIN_THRUST\0",3,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_SCALAR, PYL_PARAM_SCALAR,0,0,0,0,0,0,0,0,0}},
	{"SET_RETRO_THRUST\0",3,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_SCALAR, PYL_PARAM_SCALAR,0,0,0,0,0,0,0,0,0}},
	{"SET_HOVER_THRUST\0",3,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_SCALAR, PYL_PARAM_SCALAR,0,0,0,0,0,0,0,0,0}},
	{"SET_THRUSTER_LEVEL\0",3,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_SCALAR, 0,0,0,0,0,0,0,0,0}},
	{"CHANGE_THRUST_DIRECTION\0",3,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_VESSEL_NAME, 0,0,0,0,0,0,0,0,0}},

	{"SET_CAMERA_OFFSET\0",4,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_SCALAR, PYL_PARAM_SCALAR, PYL_PARAM_SCALAR,0,0,0,0,0,0,0,0}},
	{"SET_CAMERA_DIRECTION\0",5,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_SCALAR, PYL_PARAM_SCALAR, PYL_PARAM_SCALAR, PYL_PARAM_ANGLE,0,0,0,0,0,0,0}},
// vessel id x y x alfa beta gamma
	{"**CREATE_ATTACHMENT\0",12,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_STRING, PYL_PARAM_SCALAR, PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_BOOLEAN}},
	{"**SET_TOCHILD_ATT\0",11,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_SCALAR, PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,PYL_PARAM_SCALAR,0}},
	{"SET_ENABLE_FOCUS\0",2,{PYL_PARAM_VESSEL_NAME, PYL_PARAM_BOOLEAN, 0,0,0,0,0,0,0,0,0,0}},

	{"SOUND_LOAD\0",4,{PYL_PARAM_STRING, PYL_PARAM_STRING, PYL_PARAM_INTEGER, PYL_PARAM_INTEGER,0,0,0,0,0,0,0,0}},
	{"SOUND_OPTION\0",3,{PYL_PARAM_INTEGER, PYL_PARAM_INTEGER,PYL_PARAM_BOOLEAN,0,0,0,0,0,0,0,0,0}},
	{"SOUND_PLAY\0",3,{PYL_PARAM_STRING, PYL_PARAM_INTEGER,PYL_PARAM_INTEGER,0,0,0,0,0,0,0,0,0}},
	{"SOUND_STOP\0",1,{PYL_PARAM_STRING, 0,0,0,0,0,0,0,0,0,0,0}},
	{"SOUND_REPLACE_STOCK\0",2,{PYL_PARAM_STRING, PYL_PARAM_INTEGER,0,0,0,0,0,0,0,0,0,0}},

	{"ADD_MESH\0",6,{PYL_PARAM_STRING, PYL_PARAM_STRING, PYL_PARAM_INTEGER, PYL_PARAM_SCALAR, PYL_PARAM_SCALAR, PYL_PARAM_SCALAR,0,0,0,0,0,0}},
	{"SET_MESH_VISIBILITY\0",2,{PYL_PARAM_STRING, PYL_PARAM_INTEGER,0,0,0,0,0,0,0,0,0,0}},
	{"CLEAR_MESHES\0",0,{0,0,0,0,0,0,0,0,0,0,0,0}},

	{"USER_SEQ_ENABLED\0",2,{PYL_PARAM_SEQUENCE_NAME, PYL_PARAM_BOOLEAN,0,0,0,0,0,0,0,0,0,0}},
	{"USER_RELEASE_ENABLED\0",2,{PYL_PARAM_PYLON_NAME, PYL_PARAM_BOOLEAN, 0,0,0,0,0,0,0,0,0,0}},
	{"USER_CREATE_ENABLED\0",2,{PYL_PARAM_PYLON_NAME, PYL_PARAM_BOOLEAN, 0,0,0,0,0,0,0,0,0,0}},
	{"USER_DESTROY_ENABLED\0",2,{PYL_PARAM_PYLON_NAME, PYL_PARAM_BOOLEAN, 0,0,0,0,0,0,0,0,0,0}},
	{"ACTUALIZE_NP_CHILDS_ENABLED\0",2,{PYL_PARAM_PYLON_NAME, PYL_PARAM_BOOLEAN, 0,0,0,0,0,0,0,0,0,0}},
	{"CAN_NAVIGATE\0",2,{PYL_PARAM_PYLON_NAME, PYL_PARAM_BOOLEAN, 0,0,0,0,0,0,0,0,0,0}},
	{"USER_PARAMETERS_ENABLED\0",2,{PYL_PARAM_PYLON_NAME, PYL_PARAM_BOOLEAN, 0,0,0,0,0,0,0,0,0,0}},
	{"CURRENT_MFD_PARAM\0",2,{PYL_PARAM_PYLON_NAME, PYL_PARAM_INTEGER, 0,0,0,0,0,0,0,0,0,0}},

	{"SET_CAMERA_FOV\0",2,{PYL_PARAM_SCALAR, PYL_PARAM_SCALAR,0,0,0,0,0,0,0,0,0,0}},
	{"SET_FOCUS\0",1,{PYL_PARAM_VESSEL_NAME, 0,0,0,0,0,0,0,0,0,0,0}},

	{"SEND_KEY\0", 9, {PYL_PARAM_VESSEL_NAME, PYL_PARAM_INTEGER, PYL_PARAM_BOOLEAN, PYL_PARAM_BOOLEAN, PYL_PARAM_BOOLEAN, PYL_PARAM_BOOLEAN, PYL_PARAM_BOOLEAN, PYL_PARAM_BOOLEAN, PYL_PARAM_BOOLEAN,0,0,0}},

	{"REDIRECT_KEYS\0", 1, {PYL_PARAM_VESSEL_NAME, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0}}

};



#endif //__PYLONSEQUENCE_H






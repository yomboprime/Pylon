// ==============================================================
// ==============================================================
// Pylon MFD - Pylon management
// ==============================================================

#define STRICT
#define ORBITER_MODULE
#include "stdio.h"
#include "stdlib.h"
#include "random.h"
#include "windows.h"
#include "orbitersdk.h"
#include "PylonMFD.h"

// ==============================================================
// Global variables

int g_MFDmode; // identifier for new MFD mode

//Some strings
const char* strings[1]= {
	"Pylon MFD"
};

// ==============================================================

// Prototypes of Callbacks for Input Boxes
bool cbCreateObject(void *id, char *str, void *data);
bool cbReleaseObject(void *id, char *str, void *data);
bool cbAttachObject(void *id, char *str, void *data);
bool cbDestroyObject(void *id, char *str, void *data);
bool cbSetParam(void *id, char *str, void *data);
bool cbActivateSeq(void *id, char *str, void *data);
bool cbCancelSeq(void *id, char *str, void *data);

// ==============================================================
// API interface
DLLCLBK void InitModule( HINSTANCE hDLL ) {

	static char *name = "Pylon";

	MFDMODESPECEX spec;
	spec.name = name;
	spec.key = OAPI_KEY_P;
	spec.context = NULL;

	// Check Orbiter version.
	// The PylonMFD2010 class has the old graphics, the PylonMFD class has the new Sketchpad graphics
	if ( oapiGetOrbiterVersion() >= 160828 ) {
        spec.msgproc = PylonMFD::MsgProc;
	}
	else {
        spec.msgproc = PylonMFD2010::MsgProc;
	}

	// Register the new MFD mode with Orbiter
	g_MFDmode = oapiRegisterMFDMode( spec );

}

DLLCLBK void ExitModule( HINSTANCE hDLL ) {

	// Unregister the custom MFD mode when the module is unloaded
	oapiUnregisterMFDMode( g_MFDmode );

}

// ==============================================================
// MFD classes

// 2010

// Constructor
PylonMFD2010::PylonMFD2010( DWORD w, DWORD h, VESSEL *vessel)
: MFD( w, h, vessel ), PylonMFDInnerClass() {
    // Nothing to do here
}

// Destructor
PylonMFD2010::~PylonMFD2010() {
    // Nothing to do here
}

// MFD message parser
int PylonMFD2010::MsgProc( UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam ) {

	switch (msg) {
/*
	case OAPI_MSG_MFD_OPENED:
		// Our new MFD mode has been selected, so we create the MFD and
		// return a pointer to it.
		return (int)(new PylonMFD (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam));
*/
        case OAPI_MSG_MFD_OPENEDEX:
            MFDMODEOPENSPEC *ospec = (MFDMODEOPENSPEC*)wparam;
            return (int)( new PylonMFD2010( ospec->w, ospec->h, (VESSEL*)lparam ) );
	}
	return 0;
}


char *PylonMFD2010::ButtonLabel (int bt) {
    return this->serviceButtonLabel( bt );
}

int PylonMFD2010::ButtonMenu (const MFDBUTTONMENU **menu) const {
    return this->serviceButtonMenu( menu );
}

bool PylonMFD2010::ConsumeButton (int bt, int event) {
    return this->serviceConsumeButton( bt, event );
}

bool PylonMFD2010::ConsumeKeyBuffered(DWORD key) {
    return this->serviceConsumeKeyBuffered( key );
}

void PylonMFD2010::WriteStatus(FILEHANDLE scn) const {
    this->serviceWriteStatus( scn );
}

void PylonMFD2010::ReadStatus(FILEHANDLE scn) {
    this->serviceReadStatus( scn );
}

// ==============================================================

// > 2010

// Constructor
PylonMFD::PylonMFD( DWORD w, DWORD h, VESSEL *vessel)
: MFD2( w, h, vessel ), PylonMFDInnerClass() {

    //brush1 = oapiCreateBrush( this->GetDefaultColour( 1, 1 ) );
}

// Destructor
PylonMFD::~PylonMFD() {
	//oapiReleaseBrush( brush1 );
}

// MFD message parser
int PylonMFD::MsgProc( UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam ) {

	switch (msg) {
/*
	case OAPI_MSG_MFD_OPENED:
		// Our new MFD mode has been selected, so we create the MFD and
		// return a pointer to it.
		return (int)(new PylonMFD (LOWORD(wparam), HIWORD(wparam), (VESSEL*)lparam));
*/
        case OAPI_MSG_MFD_OPENEDEX:
            MFDMODEOPENSPEC *ospec = (MFDMODEOPENSPEC*)wparam;
            return (int)( new PylonMFD( ospec->w, ospec->h, (VESSEL*)lparam ) );
	}
	return 0;
}

char *PylonMFD::ButtonLabel (int bt) {
    return this->serviceButtonLabel( bt );
}

int PylonMFD::ButtonMenu (const MFDBUTTONMENU **menu) const {
    return this->serviceButtonMenu( menu );
}

bool PylonMFD::ConsumeButton (int bt, int event) {
    return this->serviceConsumeButton( bt, event );
}

bool PylonMFD::ConsumeKeyBuffered(DWORD key) {
    return this->serviceConsumeKeyBuffered( key );
}

void PylonMFD::WriteStatus(FILEHANDLE scn) const {
    this->serviceWriteStatus( scn );
}

void PylonMFD::ReadStatus(FILEHANDLE scn) {
    this->serviceReadStatus( scn );
}


// ==============================================================
// Repaint the MFD
//
// New version using sketchpad
bool PylonMFD::Update( oapi::Sketchpad *skp ) {


    this->Title( skp, strings[0] );

    DWORD width = GetWidth();
    DWORD height = GetHeight();

    //skp->SetPen( this->GetDefaultPen( 0, 0, 1 ) );
    //skp->Rectangle( 100, 100, 150, 150 );

    //oapi::IVECTOR2 points[3];
    //points[0].x = 200;
    //points[0].y = 200;
    //points[1].x = 300;
    //points[1].y = 256;
    //points[2].x = 220;
    //points[2].y = 340;
    //skp->SetPen( NULL );
    //skp->SetBrush( this->brush1 );
    //skp->Polygon( points, 3 );

    DWORD cSize = skp->GetCharSize();
    int lineHeight = cSize & 0x0FFFF;
    //lineHeight = (int)( lineHeight * 1.5 );
    int charWidth = ( cSize & 0xFFFF0000 ) >> 16;
    int tabSize = 2 * charWidth;

    // Set colors, etc
    // Default color is white
    skp->SetTextColor( this->GetDefaultColour( 2, 0 ) );
    skp->SetFont( this->GetDefaultFont( 0 ) );
    skp->SetPen( this->GetDefaultPen( 2, 0, 1 ) );
    skp->SetBrush( NULL );

    DWORD x = 2 * charWidth;
    DWORD y = lineHeight * 2;

    #define print2( msg ) { skp->Text( x, y, msg, strlen( msg ) ); y += lineHeight; }

    // Debug is red
    #define  PRINTDEBUG2	{ skp->SetTextColor( this->GetDefaultColour( 3, 0 ) ); x = 2 * charWidth; y += lineHeight; print2( debugString ) }


    // todo: aqui se asume que se tiene focus y focusH validos.
    // usar oapiGetFocusObject para ver si ha cambiado el focus, y si
    // es asi actualizar todas las variables llamando a selectCurrentParameter
    // (ademas se puede poner a 0 antes)

	char s[NAME_SIZE], s2[NAME_SIZE];

	if (focus == NULL || focusH == NULL) {
		print2( "No object in focus." );
		focus = NULL;
		focusH = NULL;
		selectedAttachment = NULL;
		selectedParameter = 0;
		PRINTDEBUG2;
		return true;
	}

	// Print the parent and actual vessel.
	int i=0,n = focus->AttachmentCount(true);
	ATTACHMENTHANDLE attachH;
	OBJHANDLE pH = NULL;
	VESSEL *p = NULL;
	while (i<n) {
		attachH = focus->GetAttachmentHandle(true, i);
		pH = focus->GetAttachmentStatus(attachH);
		if (pH!=NULL) i=n;
		i++;
	}
	if (pH!=NULL) {
		p = oapiGetVesselInterface(pH);
	}

	oapiGetObjectName(focusH, s2, NAME_SIZE);
	sprintf(s,"Vessel: %s", s2);
	print2( s );
	if (p!=NULL) {
		oapiGetObjectName(pH, s2, NAME_SIZE);
		sprintf(s,"Parent: %s", s2);
		print2( s );
	}

	double mass=focus->GetEmptyMass();
	sprintf(s,"Mass: %.0f", mass);
	print2( s );


	// Sub-object management

	// Print attachment info
	n = focus->AttachmentCount(false);
	if (n==0) {
		selectedAttachment = NULL;
		print2( "No attachments in vessel." );
		PRINTDEBUG2;
		return true;
	}

	//2018 BUG 1
	selectCurrentParameter();
	//selectCurrentAttachment();

	if (selectedAttachment!=NULL) {
		y += lineHeight;
		sprintf(s,"Attachment %d of %d", selectedAttachmentIndex+1, n);
		print2( s );
		if (childH == NULL) {
			sprintf(s,"Id: %s (Status: Empty)", focus->GetAttachmentId(selectedAttachment));
			print2( s );
			PRINTDEBUG2;
			return true;
		}

		sprintf(s,"Id: %s", focus->GetAttachmentId(selectedAttachment));
		print2( s );
		sprintf(s,"Sub-vessel: %s",child->GetName());
		print2( s );

	} else {
		sprintf(s,"Class: %s",child->GetClassName());
		print2( s );
	}
	x += tabSize;
	y += lineHeight;


    // Print child info

    if (pchild == NULL) {
        PRINTDEBUG2;
        return true;
    }

    // print parameter info
    x += tabSize;
    int np = pchild->GetParameterCount();
    int ns = pchild->GetSequenceCount();

    if (np+ns==0) {
        print2( "No user parameters or sequences." );
        PRINTDEBUG2;
        return true;
    }
//**********************************************************************

    selectedParameter = pchild->GetMFDSelectedParameter();
    //if ( selectedParameter < 0 ) selectedParameter = 0;
    //if ( selectedParameter >= np + ns ) selectedParameter = np + ns - 1;

    //2018 BUG 1
    //selectCurrentParameter();

    if (selectedParameter<np && pchild!=NULL && ( !pchild->userParametersEnabled || !showCommands ) ) {
        selSequence=0;
        selectedParameter=np;
        if ( !pchild->IsUserSequence(selSequence) ) selectNextParam();
    }

    if (selectedParameter<np) {

        // Green
        skp->SetTextColor( this->GetDefaultColour( 0, 0 ) );

        int nup = np, upi = 0, i = 0;
        while (i < selectedParameter) {
            if (pchild->userParametersEnabled && pchild->IsUserParameter(i)) upi++;
            else nup--;
            i++;
        }
        while (i < np) {
            if (!(pchild->userParametersEnabled && pchild->IsUserParameter(i))) nup--;
            i++;
        }

        sprintf(s,"Parameter %d of %d", upi+1, nup);
        print2( s );

        sprintf(s,"Name: %s", pchild->GetParameterName(selectedParameter));
        print2( s );
        sprintf(s,"Type: %s", PylonParamTypeName[selParamtype]);
        print2( s );


        // Parameter
        if (selParamtype < PYL_PARAM_INTEGER) {
            bool value = pchild->GetParamBol(selectedParameter);
            sprintf(s,"Value: %s", PylonBoolParamTypeName[selParamtype][value]);
            print2( s );
        } else if (selParamtype < PYL_PARAM_SCALAR) {
            int value = pchild->GetParamInt(selectedParameter);
            sprintf(s,"Value: %d", value);
            print2( s );
        } else if (selParamtype < PYL_PARAM_STRING) {
            double value = pchild->GetParamDbl(selectedParameter);
            sprintf(s,"Value: %f", value);
            print2( s );
        } else if (selParamtype < PYL_PARAM_NOT_DEFINED) {
            char *value = pchild->GetParamStr(selectedParameter);
            sprintf(s,"Value: %s", value);
            print2( s );
        } else {
            PRINTDEBUG2;
            return true;
        }

    } else if (ns != 0){

        // Blue
        skp->SetTextColor( this->GetDefaultColour( 4, 0 ) );

        if ( !pchild->IsUserSequence(selSequence) ) selectNextParam();

//sprintf(oapiDebugString(), "sequence = %d, showcmds = %d", selSequence, showCommands);

        int nus = ns, usi = 0, i = 0;
        while (i < selSequence) {
            if (pchild->IsUserSequence(i)) usi++;
            else nus--;
            i++;
        }
        while (i < ns) {
            if (!pchild->IsUserSequence(i)) nus--;
            i++;
        }

        sprintf(s,"Sequence %d of %d", usi+1, nus);
        print2( s );

        if (selParamtype==PYL_PARAM_NOT_DEFINED)
            sprintf(s,"Name: %s", pchild->GetSequenceName(selSequence));
        else
            sprintf(s,"Name: %s (%s)", pchild->GetSequenceName(selSequence), PylonParamTypeName[selParamtype]);
        print2( s );

        if (!pchild->IsSequenceValid(selSequence))
            sprintf(s,"Not valid");
        else if (pchild->IsSequenceCancelling(selSequence))
            sprintf(s,"Cancelling");
        else
            sprintf(s,"%s", &PylonBoolParamTypeName[PYL_PARAM_BOOLEAN_ACTIVATION][pchild->IsSequenceActive(selSequence)]);
        print2( s );
    }

	// Print debug
	PRINTDEBUG2;

    return true;

}


// ==============================================================
// Repaint the MFD
//
// Old version using hDC
#define LINE 12
#define TAB 10
void print(HDC hDC, int i, int *j, LPCTSTR cad) {
	TextOut(hDC, i, *j+=LINE, cad, sizeof(char)*strlen(cad));
}
#define  PRINTDEBUG	line+=LINE;print(hDC, 10, &line, debugString)

void PylonMFD2010::Update (HDC hDC)
{
	Title (hDC, strings[0]);

// todo: aqui se asume que se tiene focus y focusH validos.
// usar oapiGetFocusObject para ver si ha cambiado el focus, y si
// es asi actualizar todas las variables llamando a selectCurrentParameter
// (ademas se puede poner a 0 antes)

	int line = 20, tab = 2;
	char s[NAME_SIZE], s2[NAME_SIZE];
	#define RESET_LINE { line = 32; }

	if (focus == NULL || focusH == NULL) {
		print(hDC, tab, &line, "No object in focus.");
		focus = NULL;
		focusH = NULL;
		selectedAttachment = NULL;
		selectedParameter = 0;
		PRINTDEBUG;
		return;
	}

	// Print the parent and actual vessel.

	int i=0,n = focus->AttachmentCount(true);
	ATTACHMENTHANDLE attachH;
	OBJHANDLE pH = NULL;
	VESSEL *p = NULL;
	while (i<n) {
		attachH = focus->GetAttachmentHandle(true, i);
		pH = focus->GetAttachmentStatus(attachH);
		if (pH!=NULL) i=n;
		i++;
	}
	if (pH!=NULL) {
		p = oapiGetVesselInterface(pH);
	}

	oapiGetObjectName(focusH, s2, NAME_SIZE);
	sprintf(s,"Vessel: %s", s2);
	print(hDC, 5, &line, s);
	if (p!=NULL) {
		oapiGetObjectName(pH, s2, NAME_SIZE);
		sprintf(s,"Parent: %s", s2);
		print(hDC, 5, &line, s);
	}

	double mass=focus->GetEmptyMass();
	sprintf(s,"Mass: %.0f", mass);
	print(hDC, 5, &line, s);


	// Sub-object management


	// Print attachment info
	n = focus->AttachmentCount(false);
	if (n==0) {
		selectedAttachment = NULL;
		print(hDC, tab, &line, "No attachments in vessel.");
		PRINTDEBUG;
		return;
	}

	//2018 BUG 1
	selectCurrentParameter();
	//selectCurrentAttachment();

	if (selectedAttachment!=NULL) {
		line+=LINE;
		sprintf(s,"Attachment %d of %d", selectedAttachmentIndex+1, n);
		print(hDC, tab, &line, s);
		if (childH == NULL) {
			sprintf(s,"Id: %s (Status: Empty)", focus->GetAttachmentId(selectedAttachment));
			print(hDC, tab, &line, s);
			PRINTDEBUG;
			return;
		}

		sprintf(s,"Id: %s", focus->GetAttachmentId(selectedAttachment));
		print(hDC, tab, &line, s);
		sprintf(s,"Sub-vessel: %s",child->GetName());
		print(hDC, tab, &line, s);

	} else {
		sprintf(s,"Class: %s",child->GetClassName());
		print(hDC, tab, &line, s);
	}
	tab+=TAB;
	line+=LINE;

		// print child info

		if (pchild == NULL) {
			PRINTDEBUG;
			return;
		}

		// print parameter info
		tab+=TAB;
		int np = pchild->GetParameterCount();
		int ns = pchild->GetSequenceCount();

		if (np+ns==0) {
			print(hDC, tab, &line, "No user parameters or sequences.");
			PRINTDEBUG;
			return;
		}
//**********************************************************************

		selectedParameter = pchild->GetMFDSelectedParameter();

		//2018 BUG 1
		//selectCurrentParameter();

		if (selectedParameter<np && pchild!=NULL && ( !pchild->userParametersEnabled || !showCommands ) ) {
			selSequence=0;
			selectedParameter=np;
			if ( !pchild->IsUserSequence(selSequence) ) selectNextParam();
		}

		if (selectedParameter<np) {

			int nup = np, upi = 0, i = 0;
			while (i < selectedParameter) {
				if (pchild->userParametersEnabled && pchild->IsUserParameter(i)) upi++;
				else nup--;
				i++;
			}
			while (i < np) {
				if (!(pchild->userParametersEnabled && pchild->IsUserParameter(i))) nup--;
				i++;
			}

			sprintf(s,"Parameter %d of %d", upi+1, nup);
			print(hDC, tab, &line, s);

			sprintf(s,"Name: %s", pchild->GetParameterName(selectedParameter));
			print(hDC, tab, &line, s);
			sprintf(s,"Type: %s", PylonParamTypeName[selParamtype]);
			print(hDC, tab, &line, s);


			// Parameter
			if (selParamtype < PYL_PARAM_INTEGER) {
				bool value = pchild->GetParamBol(selectedParameter);
				sprintf(s,"Value: %s", PylonBoolParamTypeName[selParamtype][value]);
				print(hDC, tab, &line, s);
			} else if (selParamtype < PYL_PARAM_SCALAR) {
				int value = pchild->GetParamInt(selectedParameter);
				sprintf(s,"Value: %d", value),
				print(hDC, tab, &line, s);
			} else if (selParamtype < PYL_PARAM_STRING) {
				double value = pchild->GetParamDbl(selectedParameter);
				sprintf(s,"Value: %f", value),
				print(hDC, tab, &line, s);
			} else if (selParamtype < PYL_PARAM_NOT_DEFINED) {
				char *value = pchild->GetParamStr(selectedParameter);
				sprintf(s,"Value: %s", value),
				print(hDC, tab, &line, s);
			} else {PRINTDEBUG; return;}

		} else if (ns != 0){

		    if ( !pchild->IsUserSequence(selSequence) ) selectNextParam();

//sprintf(oapiDebugString(), "sequence = %d, showcmds = %d", selSequence, showCommands);

			int nus = ns, usi = 0, i = 0;
			while (i < selSequence) {
				if (pchild->IsUserSequence(i)) usi++;
				else nus--;
				i++;
			}
			while (i < ns) {
				if (!pchild->IsUserSequence(i)) nus--;
				i++;
			}

			sprintf(s,"Sequence %d of %d", usi+1, nus);
			print(hDC, tab, &line, s);

			if (selParamtype==PYL_PARAM_NOT_DEFINED)
				sprintf(s,"Name: %s", pchild->GetSequenceName(selSequence));
			else
				sprintf(s,"Name: %s (%s)", pchild->GetSequenceName(selSequence), PylonParamTypeName[selParamtype]);
			print(hDC, tab, &line, s);

			if (!pchild->IsSequenceValid(selSequence))
				sprintf(s,"Not valid");
			else if (pchild->IsSequenceCancelling(selSequence))
				sprintf(s,"Cancelling");
			else
				sprintf(s,"%s", &PylonBoolParamTypeName[PYL_PARAM_BOOLEAN_ACTIVATION][pchild->IsSequenceActive(selSequence)]);
			print(hDC, tab, &line, s);
		}


	// Print debug
	PRINTDEBUG;
}


// ==============================================================
// Internal functionality

PylonMFDInnerClass::PylonMFDInnerClass() {

    selectedAttachmentIndex = selectedParameter = selParamtype = selSequence = 0;

	showCommands = 0;

	strvel[0]=0;
	objnameIndex = 0;
	*objectName=0;
	className[0]=0;
	debugString[0]=0;

	initfocusH = oapiGetFocusObject();
	if (initfocusH==NULL)
		initfocus = NULL;
	else
		initfocus = oapiGetVesselInterface(initfocusH);
	focusH = initfocusH;
	focus = initfocus;

	if ( focus == NULL ) {
        selectedAttachment = NULL;
        selectedParameter = -1; selSequence = -1;
        return;
	}

	if ( focus->AttachmentCount( false ) > 0) {
		selectedAttachment = focus->GetAttachmentHandle( false, 0 );
	}
	else {
        selectedAttachment = NULL;
	}
/*
    2018 BUG 1
	pchild = CPylon::IsPylonVessel(child);
	if ( pchild != NULL ) {
        this->selectedParameter = pchild->GetMFDSelectedParameter();
	}
*/
	child = NULL;
	childH = NULL;
	pchild = NULL;
	bool b = selectCurrentParameter();
/*
	if (b) {
		int np = pchild->GetParameterCount(), ns = pchild->GetSequenceCount();
		if (!pchild->userParametersEnabled || showCommands ) {
			selectedParameter = np;
			selSequence = 0;

			if ( !pchild->IsUserSequence(selSequence) ) selectNextParam();
		}
		while (b) {
			if (selectedParameter >= np) {
				b = !pchild->IsUserSequence(selectedParameter - np);
			} else {
				b = !(pchild->userParametersEnabled && pchild->IsUserParameter(selectedParameter));
			}
			if (b) selectNextParam();
			if (selectedParameter >= np + ns) {
				selectedParameter = -1; selSequence = -1;
			}
		}
	}
*/

}

int PylonMFDInnerClass::selectCurrentAttachment(void) {

    if ( focus == NULL ) {
        return -1;
    }

	int n = focus->AttachmentCount(false), i=n-1;
	if (n==0) {
		selectedAttachment = NULL;
		return -1;
	} else {
		if (selectedAttachment == NULL) return -1;
		ATTACHMENTHANDLE attachH = focus->GetAttachmentHandle(false, i);
		while (i>=0 && attachH != selectedAttachment) {
			i--;
			if (i>=0) attachH = focus->GetAttachmentHandle(false, i);
		}
		if (i<0) {
			selectedAttachment = attachH;
			selectedParameter = 0;
			selSequence = -1;
			return -1;
		}
		selectedAttachmentIndex = i;
		return i;
	}
}

void PylonMFDInnerClass::selectPrevAttachment(void) {
	int n = focus->AttachmentCount(false), i=n-1;
	if (n==0) {
		selectedAttachment = NULL;
	} else {
		if (selectedAttachment != NULL) {
			ATTACHMENTHANDLE attachH = focus->GetAttachmentHandle(false, i);
			while (i>=0 && attachH != selectedAttachment) {
				i--;
				if (i>=0) attachH = focus->GetAttachmentHandle(false, i);
			}
			if (i<0) { selectedAttachment = attachH; i = 0; selectedParameter = 0;}
			i--;
			if (i<0) {
				selectedAttachment = NULL;
				return;
			}
		}

		selectedAttachment = focus->GetAttachmentHandle(false, i);
	}
}
void PylonMFDInnerClass::selectNextAttachment(void)  {
	int n = focus->AttachmentCount(false), i=n-1;
	if (n==0) {
		selectedAttachment = NULL;
	} else {
		if (selectedAttachment == NULL) i = 0;
		else {
			ATTACHMENTHANDLE attachH = focus->GetAttachmentHandle(false, i);
			while (i>=0 && attachH != selectedAttachment) {
				i--;
				if (i>=0) attachH = focus->GetAttachmentHandle(false, i);
			}
			if (i<0) { selectedAttachment = attachH; i = 0; selectedParameter = 0;}
			i++;
			if (i>=n) {selectedAttachment = NULL; return;}
		}
		selectedAttachment = focus->GetAttachmentHandle(false, i);
	}
}
void PylonMFDInnerClass::selectPrevParam(void) {
	selectCurrentAttachment();
	if ( focus == NULL ) {
        return;
	}
	if (selectedAttachment==NULL) {
		childH = focusH;
		child = focus;
	}
	else {
		childH = focus->GetAttachmentStatus(selectedAttachment);
		if (childH == NULL) return;
		child = oapiGetVesselInterface(childH);
	}
	pchild = CPylon::IsPylonVessel(child);
	if (pchild==NULL) {return;}

	int np = pchild->GetParameterCount();
	int ns = pchild->GetSequenceCount();

	if (np==0 && ns==0) return;
	if (selectedParameter>=np+ns) selectedParameter = np+ns-1;
	int sp = selectedParameter;
	bool flag = false;
	while (!flag) {
		sp--;
		if (sp<0) break;
		if (sp>=np)
			flag = pchild->IsUserSequence(sp-np);
		else flag = pchild->userParametersEnabled && pchild->IsUserParameter(sp);
	}
	if (!flag) return;

	selectedParameter = sp;
	if (selectedParameter<0) selectedParameter = 0;
	if (selectedParameter>=np+ns) selectedParameter = np+ns-1;
	if (selectedParameter<np) {
		selSequence = -1;
		selParamtype = pchild->GetParameterType(selectedParameter);
	} else {
		selSequence = selectedParameter - np;
		selParamtype = pchild->GetSequenceType(selSequence);
	}
	pchild->SetMFDSelectedParameter(selectedParameter);
//sprintf(oapiDebugString(),"selPrev: np=%d,ns=%d,sp=%d,ss=%d",np,ns,selectedParameter+1, selSequence+1);
}
void PylonMFDInnerClass::selectNextParam(void) {
	selectCurrentAttachment();
	if ( focus == NULL ) {
        return;
	}
	if (selectedAttachment==NULL) {
		childH = focusH;
		child = focus;
	}
	else {
		childH = focus->GetAttachmentStatus(selectedAttachment);
		if (childH == NULL) return;
		child = oapiGetVesselInterface(childH);
	}
	pchild = CPylon::IsPylonVessel(child);
	if (pchild==NULL) {return;}

	int np = pchild->GetParameterCount();
	int ns = pchild->GetSequenceCount();

	if (np==0 && ns==0) return;
	if (selectedParameter<0) selectedParameter = 0;
	if (selectedParameter>=np+ns) selectedParameter = np+ns-1;
	int sp = selectedParameter;
	bool flag = false;
	while (!flag) {
		sp++;
		if (sp>=np+ns) break;
		if (sp>=np)
			flag = pchild->IsUserSequence(sp-np);
		else flag = pchild->userParametersEnabled && pchild->IsUserParameter(sp) && showCommands;
	}
	if (!flag) return;

	selectedParameter = sp;
	if (selectedParameter<0) selectedParameter = 0;
	if (selectedParameter>=np+ns) selectedParameter = np+ns-1;
	if (selectedParameter<np) {
		selSequence = -1;
		selParamtype = pchild->GetParameterType(selectedParameter);
	} else {
		selSequence = selectedParameter - np;
		selParamtype = pchild->GetSequenceType(selSequence);
	}
	pchild->SetMFDSelectedParameter(selectedParameter);
//	sprintf(oapiDebugString(),"selPrev: np=%d,ns=%d,sp=%d,ss=%d",np,ns,selectedParameter+1, selSequence+1);
}

bool PylonMFDInnerClass::selectCurrentParameter(void) {
	selectCurrentAttachment();
	if ( focus == NULL ) {
        return false;
	}
	if (selectedAttachment==NULL) {
		childH = focusH;
		child = focus;
	}
	else {
		childH = focus->GetAttachmentStatus(selectedAttachment);
		if (childH == NULL) return false;
		child = oapiGetVesselInterface(childH);
	}

	pchild = CPylon::IsPylonVessel(child);
	if (pchild == NULL) return false;

	int np = pchild->GetParameterCount();
	int ns = pchild->GetSequenceCount();
	if (np==0 && ns==0) return false;
	if (selectedParameter<0) selectedParameter = 0;
	if (selectedParameter>=np+ns) selectedParameter = np+ns-1;

	if (selectedParameter<np) {
		selSequence = -1;
		selParamtype = pchild->GetParameterType(selectedParameter);
	} else {
		selSequence = selectedParameter - np;
		selParamtype = pchild->GetSequenceType(selSequence);
	}

	return true;
}

void PylonMFDInnerClass::upLevel(void)
{
	int i=0,n = focus->AttachmentCount(true);
	ATTACHMENTHANDLE a;
	OBJHANDLE pH = NULL;
	while (i<n) {
		a = focus->GetAttachmentHandle(true, i);
		pH = focus->GetAttachmentStatus(a);
		if (pH!=NULL) i=n;
		i++;
	}
	if (pH==NULL) return;
	VESSEL *p = oapiGetVesselInterface(pH);
	if (p==NULL) return;

	CPylon *pP = CPylon::IsPylonVessel(p);
	/*if (p!= NULL && !p->CanNavigate()) return;*/
	selectedAttachment = (void *)1;
	i=0,n = p->AttachmentCount(false);
	while (i<n) {
		a = p->GetAttachmentHandle(false, i);
		if (p->GetAttachmentStatus(a)==focusH) {
			selectedAttachment = a;
			i=n;
		}
		i++;
	}
	focusH = pH;
	focus = p;
	selectCurrentAttachment();
}

void PylonMFDInnerClass::downLevel(void)
{
	selectCurrentParameter();
	if (selectedAttachment==NULL) return;
	if (childH == NULL) return;
	/*if (pchild != NULL && !pchild->CanNavigate()) return;*/
	focusH = childH;
	focus = child;

	selectedAttachment = (void *)1;
	selectCurrentAttachment();
}

void PylonMFDInnerClass::decParameter(void)
{
	if (focusH == NULL) return;

	if (!selectCurrentParameter()) { sprintf(debugString,"[Dec]: No parameter or sequence.");return;}

	if (selSequence == -1) {
		// Parameter
		pchild->DecParam(selectedParameter);

	} else {
		// Sequence
		int st = pchild->GetSequenceCancelType(selSequence);
		if (st==PYL_PARAM_NOT_DEFINED) {
			pchild->CancelSequence(selSequence);
		} else {
			char s[NAME_SIZE];
			sprintf(s,"[PylonMFD]: Enter value for cancel sequence of %s (type %s)",pchild->GetSequenceName(selSequence), PylonParamTypeName[st]);
			oapiOpenInputBox(s, cbCancelSeq, 0, 40, (void *)this);
		}
	}

	return;
}

void PylonMFDInnerClass::incParameter(void)
{
	if (focusH == NULL) return;

	if (!selectCurrentParameter()) { sprintf(debugString,"[Inc]: No parameter or sequence.");return;}

	if (selSequence == -1) {
		// Parameter
		pchild->IncParam(selectedParameter);

	} else {
		// Sequence
		if (selParamtype==PYL_PARAM_NOT_DEFINED)
			pchild->ActivateSequence(selSequence);
		else {
			char s[NAME_SIZE];
			sprintf(s,"[PylonMFD]: Enter value for sequence %s (type %s)",pchild->GetSequenceName(selSequence), PylonParamTypeName[pchild->GetSequenceType(selSequence)]);
			oapiOpenInputBox(s, cbActivateSeq, 0, 40, (void *)this);
		}
	}

	return;
}


// Return button labels
char *PylonMFDInnerClass::serviceButtonLabel (int bt)
{
	// The labels for the buttons used by the MFD
	static char *label[12] = {"AT-", "AT+", "CRT", "RLS", "DES", "P-", "UP", "DWN", "SET", "INC", "DEC", "P+"};
	return (bt < 12 ? label[bt] : 0);
}

// Return button menus
int PylonMFDInnerClass::serviceButtonMenu (const MFDBUTTONMENU **menu) const
{
	// The menu descriptions for the buttons
	static const MFDBUTTONMENU mnu[12] = {
	   //"1234567890123456"
		{"<- Select prev.",
		 "   attachment.", 'Q'},
		{"<- Select next",
		 "   attachment.", 'A'},
		{"<- Create object",
		 "   in attachment", 'C'},
		{"<- Release",
		 "   object.", 'R'},
		{"<- Destroy",
		 "   object.", 'D'},
 		{"<- Select prev.",
		 "   pylon param.", 'Z'},

		{" Go up in the ->",
		 "    structure.", 'W'},
		{"   Go down in ->",
		 "the structure.", 'S'},
		{"Set parameter ->",
		 "       value. ", 'V'},
		{"    Increment ->",
		 " param. value.", 'N'},
		{"    Decrement ->",
		 " param. value.", 'B'},
 		{"  Select next ->",
		 "  pylon param.", 'X'},

	};
	if (menu) *menu = mnu;
	return 12; // return the number of buttons used
}

bool PylonMFDInnerClass::serviceConsumeButton (int bt, int event) {

	if (!(event & PANEL_MOUSE_LBDOWN)) return false;

	switch (bt) {
		case 0 : this->serviceConsumeKeyBuffered(OAPI_KEY_Q); break;
		case 1 : this->serviceConsumeKeyBuffered(OAPI_KEY_A); break;
		case 2 : this->serviceConsumeKeyBuffered(OAPI_KEY_C); break;
		case 3 : this->serviceConsumeKeyBuffered(OAPI_KEY_R); break;
		case 4 : this->serviceConsumeKeyBuffered(OAPI_KEY_D); break;
		case 5 : this->serviceConsumeKeyBuffered(OAPI_KEY_Z); break;

		case 6 : this->serviceConsumeKeyBuffered(OAPI_KEY_W); break;
		case 7 : this->serviceConsumeKeyBuffered(OAPI_KEY_S); break;
		case 8 : this->serviceConsumeKeyBuffered(OAPI_KEY_V); break;
		case 9 : this->serviceConsumeKeyBuffered(OAPI_KEY_N); break;
		case 10: this->serviceConsumeKeyBuffered(OAPI_KEY_B); break;
		case 11: this->serviceConsumeKeyBuffered(OAPI_KEY_X); break;

		default: return false;
	}
	return true;
}

bool PylonMFDInnerClass::serviceConsumeKeyBuffered(DWORD key)
{
	switch(key) {
	case OAPI_KEY_Q:
		debugString[0]=0;
		selectPrevAttachment();
		return true;
	case OAPI_KEY_A:
		debugString[0]=0;
		selectNextAttachment();
		return true;
	case OAPI_KEY_W:
		debugString[0]=0;
		upLevel();
		return true;
	case OAPI_KEY_S:
		debugString[0]=0;
		downLevel();
		return true;
	case OAPI_KEY_Z:
		selectPrevParam();
		debugString[0]=0;
		return true;
	case OAPI_KEY_X:
		selectNextParam();
		debugString[0]=0;
		return true;
	case OAPI_KEY_C:
		debugString[0]=0;
		selectCurrentAttachment();
		if (selectedAttachment==NULL) { sprintf(debugString,"[Create]: No attachment.");return true;}
		if (focus->GetAttachmentStatus(selectedAttachment)!=NULL) { sprintf(debugString,"[Create]: Attachment in use.");return true;}

		char s[NAME_SIZE];

		sprintf(s, "%s", objectName, objnameIndex);
		if (oapiGetVesselByName(s)!=NULL) {
			int i = 1;
			sprintf(s, "%s%d", objectName, i);
			while (oapiGetVesselByName(s)!=NULL) {
				i++;
				sprintf(s, "%s%d", objectName, i);
			}
			sprintf(s, "%s%d:%s", objectName, i, className);
			objnameIndex = strlen(objectName);
		} else {
			sprintf(s, "%s:%s", objectName, className);
			objnameIndex = 0;
		}

		oapiOpenInputBox("[PylonMFD]: Enter ""name:class"" of new object:", cbCreateObject, s, 40, (void *)this);
		return true;
	case OAPI_KEY_R:
		debugString[0]=0;
		selectCurrentAttachment();
		if (selectedAttachment==NULL) { sprintf(debugString,"[Release]: No attachment.");return true;}
		if ((focus->GetAttachmentStatus(selectedAttachment)==NULL)) {sprintf(debugString,"[Release]: Attachment not in use.");return true;}
		oapiOpenInputBox("[PylonMFD]: Enter release velocity:", cbReleaseObject, strvel, 40, (void *)this);
		return true;
	case OAPI_KEY_D:
		debugString[0]=0;
		selectCurrentAttachment();
		if (selectedAttachment==NULL) { sprintf(debugString,"[Destroy]: No attachment.");return true;}
		if (focus->GetAttachmentStatus(selectedAttachment)==NULL) { sprintf(debugString,"[Destroy]: Attachment not in use.");return true;}
		oapiOpenInputBox("[PylonMFD]: Destroy object? (y/n)", cbDestroyObject, 0, 40, (void *)this);
		return true;
	case OAPI_KEY_T:
		debugString[0]=0;
		selectCurrentAttachment();
		if (selectedAttachment==NULL) { sprintf(debugString,"[Attach]: No attachment.");return true;}
		if (focus->GetAttachmentStatus(selectedAttachment)!=NULL) { sprintf(debugString,"[Attach]: Attachment in use.");return true;}
		oapiOpenInputBox("[PylonMFD]: Enter name of object to attach:", cbAttachObject, 0, 40, (void *)this);
		return true;
	case OAPI_KEY_V:
		debugString[0]=0;
		if (!selectCurrentParameter()) { sprintf(debugString,"[Set value]: No parameter");return true;}

		if (selSequence != -1) { sprintf(debugString,"[Value]: Select a parameter");return true;}

		sprintf(s, "[PylonMFD]: Enter value for parameter %s (type %s)",pchild->GetParameterName(selectedParameter), PylonParamTypeName[pchild->GetParameterType(selectedParameter)]);

		if (selSequence==-1)
			oapiOpenInputBox(s, cbSetParam, 0, 40, (void *)this);
		else
			sprintf(debugString, "[Set value]: Select a parameter.");
		return true;

	case OAPI_KEY_B:
		debugString[0]=0;
		decParameter();
		return true;
	case OAPI_KEY_N:
		debugString[0]=0;
		incParameter();
		return true;
	case OAPI_KEY_F:
		if (focusH==NULL) { sprintf(debugString,"[Change focus]: No object in focus.");return true;}
		if (focusH!=oapiGetFocusObject()) oapiSetFocusObject(focusH);
		return true;

	default:
		return false;
	}

}

// ==============================================================
// Save status to file
void PylonMFDInnerClass::serviceWriteStatus(FILEHANDLE scn) const {
	oapiWriteScenario_int (scn, "SHOWCOMMANDS", showCommands);
}

// ==============================================================
// Save status to file
void PylonMFDInnerClass::serviceReadStatus(FILEHANDLE scn) {
    char *line;
	while (oapiReadScenario_nextline (scn, line)) {
		if (!strnicmp (line, "SHOWCOMMANDS", 12)) {
			sscanf (line+12, "%d", &showCommands);
			showCommands = showCommands? 1:0;
		}
	}
}



// ==============================================================
// Input Boxes Callback functions

bool cbCreateObject(void *id, char *str, void *data)
{
	PylonMFDInnerClass *w = (PylonMFDInnerClass *)data;
	if (w == NULL) { return true;}
	if (w->focusH == NULL) return true;

	w->selectCurrentAttachment();
	if (w->selectedAttachment==NULL) { sprintf(w->debugString,"[Create]: No attachment.");return true;}
	if (w->focus->GetAttachmentStatus(w->selectedAttachment)!=NULL) { sprintf(w->debugString,"[Create]: Attachment in use.");return true;}

	char *oName = strtok(str,":");
	char *cName = strtok(NULL,":");

	if (CPylon::IsPylonVessel(w->focus)) {
		if (!((CPylon*)w->focus)->userCreateEnabled) { sprintf(w->debugString,"[Create]: Can't create on this object.");return true;}
	} else {
		// Solo por el attachment bug:
		// si el padre no es pylon y focus tampoco -> falla
		// si focus no es pylon y el objeto a crear tampoco (comprobar cName) -> falla
	}

	if (cName==NULL) { sprintf(w->debugString,"[Create]: Invalid class name.");return false;}
	if (oName==NULL) { sprintf(w->debugString,"[Create]: Invalid object name.");return false;}
	if (oapiGetVesselByName(oName)!=NULL)  { sprintf(w->debugString,"[Create]: Object name already exists.");return false;}

	sprintf(w->debugString,"[Create]: Object created.");

	// Create new vessel (with a 'toparent' attachment)
	VESSELSTATUS2 vs;
	vs.version = 2;
	vs.flag = 0;
	vs.rbody = oapiGetGbodyByIndex(0);
	vs.base = NULL;
	vs.port = 0;
	vs.status = 0;
	w->focus->GetGlobalPos( vs.rpos );
	w->focus->GetGlobalVel( vs.rvel );
	VECTOR3 angVel1;
	w->focus->GetAngularVel(angVel1);
	w->focus->GlobalRot( angVel1, vs.vrot );
	w->focus->GetGlobalOrientation( vs.arot );
	vs.nfuel = 0;
	vs.fuel = NULL;
	vs.nthruster = 0;
	vs.thruster = NULL;
	vs.ndockinfo = 0;
	vs.dockinfo = NULL;
	vs.xpdr = 0;

	OBJHANDLE obj = oapiCreateVesselEx(oName, cName, &vs);

	VESSEL *objInt = oapiGetVesselInterface(obj);

	if (obj==NULL || objInt==NULL) {
		sprintf(w->debugString,"[Create]: Error while creating object.");
		return false;
	}

	// If there is no toparent attachmet in the new object, delete it
	int n = objInt->AttachmentCount(true);
	if (n==0) {
		oapiDeleteVessel(obj);
		sprintf(w->debugString,"[Create]: Object doesn't have toparent attachments.");
		return true;
	}
	ATTACHMENTHANDLE objAtt = objInt->GetAttachmentHandle(true, 0);

	// Link the object to the focus vessel
	CPylon::PylonAttach(w->focusH, obj, w->selectedAttachment, objAtt);

	if (w->objnameIndex>0) {
		w->objectName[w->objnameIndex] = 0;
	}
	else sprintf(w->objectName, "%s", oName);
	sprintf(w->className, "%s", cName);
	return true;
}

bool cbReleaseObject(void *id, char *str, void *data)
{
	PylonMFDInnerClass *w = (PylonMFDInnerClass *)data;
	if (w == NULL) { return true;}
	if (w->focusH == NULL) return true;

	w->selectCurrentParameter();
	if (w->selectedAttachment==NULL) { sprintf(w->debugString,"[Release]: No attachment.");return true;}
	if (w->childH == NULL) { sprintf(w->debugString,"[Release]: Attachment not in use.");return true;}
	if (w->pchild != NULL && !w->pchild->userReleaseEnabled) { sprintf(w->debugString,"[Release]: Can't release.");return true;}

/*	w->selectCurrentAttachment();
	if (w->selectedAttachment==NULL) { sprintf(w->debugString,"[Release]: No attachment.");return true;}
	OBJHANDLE objH = w->focus->GetAttachmentStatus(w->selectedAttachment);
	if (objH==NULL) { sprintf(w->debugString,"[Release]: Attachment not in use.");return true;}
*/
	double vel = atof(str);

	CPylon::PylonDetach(w->focusH, w->childH, w->selectedAttachment, vel);

	if (w->childH == oapiGetFocusObject()) {
		w->initfocusH = w->childH;
		w->focusH = w->childH;
		w->focus = w->child;
		w->initfocus = w->child;
	}

	return true;
}

bool cbAttachObject(void *id, char *str, void *data)
{
	PylonMFDInnerClass *w = (PylonMFDInnerClass *)data;
	if (w == NULL) { return true;}
	if (w->focusH == NULL) return true;

	w->selectCurrentAttachment();

	if (w->selectedAttachment==NULL) { sprintf(w->debugString,"[Attach]: No attachment.");return true;}

	if (w->focus->GetAttachmentStatus(w->selectedAttachment)!=NULL) { sprintf(w->debugString,"[Attach]: Attachment in use.");return true;}

	OBJHANDLE obj = oapiGetVesselByName(str);
	if (obj==NULL) { sprintf(w->debugString,"[Attach]: Object doesn't exist.");return true;}

	VESSEL *objInt = oapiGetVesselInterface(obj);

	// If there is no toparent attachment in the object, cancel operation
	int n = objInt->AttachmentCount(true);
	if (n==0) {
		sprintf(w->debugString,"[Attach]: Object doesn't have toparent attachments.");
		return true;
	}
	ATTACHMENTHANDLE objAtt = objInt->GetAttachmentHandle(true, 0);

	// Link the object to the focus vessel
	CPylon::PylonAttach(w->focusH, obj, w->selectedAttachment, objAtt);
	return true;

}

bool cbDestroyObject(void *id, char *str, void *data)
{
	PylonMFDInnerClass *w = (PylonMFDInnerClass *)data;
	if (w == NULL) { return true;}
	if (w->focusH == NULL) return true;

	w->selectCurrentParameter();
	if (w->selectedAttachment==NULL) { sprintf(w->debugString,"[Destroy]: No attachment.");return true;}
	if (w->childH == NULL)  { sprintf(w->debugString,"[Destroy]: Attachment not in use.");return true;}

	if (w->pchild != NULL && !w->pchild->userDestroyEnabled) { sprintf(w->debugString,"[Destroy]: Can't destroy.");return true;}

	CPylon::PylonDetach(w->focusH, w->focus->GetAttachmentStatus(w->selectedAttachment), w->selectedAttachment);
	oapiDeleteVessel(w->childH);

	return true;
}

bool cbSetParam(void *id, char *str, void *data)
{
	PylonMFDInnerClass *w = (PylonMFDInnerClass *)data;
	if (w == NULL) { return true;}
	if (w->focusH == NULL) return true;
	if (!w->selectCurrentParameter()) { sprintf(w->debugString,"[Value]: No parameter");return true;}
	if (w->selSequence == -1) {
		if (w->selParamtype < PYL_PARAM_INTEGER) {
			bool value = true;
			if (str[0]==0 || str[0]=='0' || str[0]=='f' || str[0]=='F') value = false;
			w->pchild->SetParamBol(w->selectedParameter, value);
		} else if (w->selParamtype < PYL_PARAM_SCALAR) {
			int value = atoi(str);
			w->pchild->SetParamInt(w->selectedParameter, value);
		} else if (w->selParamtype < PYL_PARAM_STRING) {
			double value = atof(str);
			w->pchild->SetParamDbl(w->selectedParameter, value);
		} else if (w->selParamtype < PYL_PARAM_NOT_DEFINED) {
			w->pchild->SetParamStr(w->selectedParameter, str);
		}
		return true;
	} else { sprintf(w->debugString,"[Value]: Select a parameter");return true;}
	return true;
}
bool cbActivateSeq(void *id, char *str, void *data)
{
	PylonMFDInnerClass *w = (PylonMFDInnerClass *)data;
	if (w == NULL) { return true;}
	if (w->focusH == NULL) return true;
	if (!w->selectCurrentParameter()) { sprintf(w->debugString,"[Activate]: No sequence");return true;}
	if (w->selSequence != -1) {
		w->tempValue.type = PYL_PARAM_STRING;
		strcpy(w->tempValue.strValue, str);
		w->pchild->ActivateSequenceValue(w->selSequence, &(w->tempValue));
	}
	return true;
}
bool cbCancelSeq(void *id, char *str, void *data)
{
	PylonMFDInnerClass *w = (PylonMFDInnerClass *)data;
	if (w == NULL) { return true;}
	if (w->focusH == NULL) return true;
	if (!w->selectCurrentParameter()) { sprintf(w->debugString,"[Cancel]: No sequence");return true;}
	if (w->selSequence != -1) {
		w->tempValue.type = PYL_PARAM_STRING;
		strcpy(w->tempValue.strValue, str);
		w->pchild->CancelSequenceValue(w->selSequence, &(w->tempValue));
	}
	return true;
}

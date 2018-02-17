// ==============================================================
//                 ORBITER MODULE: PylonMFD
//
//              Copyright (C) 2004 J.J. Luna
//                   All rights reserved
//
// PylonMFD.h 
//
// See PylonMFDreadme.txt
//
// ==============================================================

#ifndef __PYLONMFD_H
#define __PYLONMFD_H

#include "CPylon.h"

class PylonMFD: public MFD {
public:

	PylonMFD (DWORD w, DWORD h, VESSEL *vessel);
	~PylonMFD ();
	char *ButtonLabel (int bt);
	int ButtonMenu (const MFDBUTTONMENU **menu) const;
	bool ConsumeButton (int bt, int event);
	bool ConsumeKeyBuffered(DWORD key); 
	void WriteStatus(FILEHANDLE scn) const;
	void ReadStatus(FILEHANDLE scn);
	void Update (HDC hDC);
	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);

	char debugString[NAME_SIZE];
	
	OBJHANDLE focusH, initfocusH;
	VESSEL *focus, *initfocus;
	ATTACHMENTHANDLE selectedAttachment;
	int selectedAttachmentIndex, selectedParameter, selParamtype, selSequence;//, displaySelectedIndex;
	OBJHANDLE childH;
	VESSEL *child;
	CPylon *pchild;
	TPylParamValue tempValue;
	int showCommands;

	char objectName[NAME_SIZE];
	char className[NAME_SIZE];
	char strvel[NAME_SIZE];
	int objnameIndex;

	int selectCurrentAttachment(void);
	void selectPrevAttachment(void);
	void selectNextAttachment(void);
	void upLevel(void);
	void downLevel(void);

	void selectPrevParam(void);
	void selectNextParam(void);	
	bool selectCurrentParameter(void);
	void decParameter(void);
	void incParameter(void);
};

#endif // !__PYLONMFD_H
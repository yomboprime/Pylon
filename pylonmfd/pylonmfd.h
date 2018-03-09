// ==============================================================
//                 ORBITER MODULE: PylonMFD
//
//              Copyright (C) 2004-2018 J.J. Luna
//                      MIT Licensed
//
// PylonMFD.h
//
// See PylonMFDreadme.txt
//
// ==============================================================

#ifndef __PYLONMFD_H
#define __PYLONMFD_H

#include "CPylon.h"

class PylonMFDInnerClass {
public:

    PylonMFDInnerClass();

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

    char debugString[NAME_SIZE];

    char *serviceButtonLabel (int bt);
	int serviceButtonMenu (const MFDBUTTONMENU **menu) const;
	bool serviceConsumeButton (int bt, int event);
	bool serviceConsumeKeyBuffered(DWORD key);
	void serviceWriteStatus(FILEHANDLE scn) const;
	void serviceReadStatus(FILEHANDLE scn);

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

class PylonMFD2010: public MFD, public PylonMFDInnerClass {
public:

	PylonMFD2010 (DWORD w, DWORD h, VESSEL *vessel);
	~PylonMFD2010 ();
	char *ButtonLabel (int bt);
	int ButtonMenu (const MFDBUTTONMENU **menu) const;
	bool ConsumeButton (int bt, int event);
	bool ConsumeKeyBuffered(DWORD key);
	void WriteStatus(FILEHANDLE scn) const;
	void ReadStatus(FILEHANDLE scn);
	void Update (HDC hDC);

	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);

};

class PylonMFD: public MFD2, public PylonMFDInnerClass {
public:

	PylonMFD (DWORD w, DWORD h, VESSEL *vessel);
	~PylonMFD ();
	char *ButtonLabel (int bt);
	int ButtonMenu (const MFDBUTTONMENU **menu) const;
	bool ConsumeButton (int bt, int event);
	bool ConsumeKeyBuffered(DWORD key);
	void WriteStatus(FILEHANDLE scn) const;
	void ReadStatus(FILEHANDLE scn);
	virtual bool Update (oapi::Sketchpad *skp);

	static int MsgProc (UINT msg, UINT mfd, WPARAM wparam, LPARAM lparam);

	//oapi::Brush *brush1;
};

#endif // !__PYLONMFD_H


// ZeissController_X.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CZeissControllerXApp:
// See ZeissController_X.cpp for the implementation of this class
//

class CZeissControllerXApp : public CWinApp
{
public:
	CZeissControllerXApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CZeissControllerXApp theApp;

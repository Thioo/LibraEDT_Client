
// ZeissController_XDlg.cpp : implementation file
//

#include "pch.h"
#include "ZeissControllerX_Dialog.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CZeissControllerXDlg dialog


void CZeissControllerX_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CZeissControllerX_Dialog, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(BTN_GET, &CZeissControllerX_Dialog::OnBnCZGet)
	ON_BN_CLICKED(BTN_INIT, &CZeissControllerX_Dialog::OnBnCZInitialise)
	ON_BN_CLICKED(BTN_CLOSE_CONTROL, &CZeissControllerX_Dialog::OnBnCZCloseControl)
	ON_BN_CLICKED(BTN_SET, &CZeissControllerX_Dialog::OnBnCZSet)
	ON_BN_CLICKED(BTN_EXEC, &CZeissControllerX_Dialog::OnBnCZExec)
	ON_BN_CLICKED(BTN_CAPTURE, &CZeissControllerX_Dialog::OnBnCZCapture)
	ON_BN_CLICKED(IDC_MoveStageMouse, &CZeissControllerX_Dialog::OnBnClickedMovestagemouse)
	ON_BN_CLICKED(IDD_DATA_COLLECTION, &CZeissControllerX_Dialog::OnMenuDataCollection)
	ON_BN_CLICKED(ID_BTN_CLOSE, &CZeissControllerX_Dialog::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_OPN_DATA_COLLECTION, &CZeissControllerX_Dialog::OnBnClickedOpnDataCollection)
	ON_BN_CLICKED(IDC_LENS_CTRL_BTN, &CZeissControllerX_Dialog::OnBnClickedLensCtrlBtn)
END_MESSAGE_MAP()

// CZeissControllerXDlg message handlers

BOOL CZeissControllerX_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	
	
	// TODO: Add extra initialization here
	SetDlgItemText(IDC_EDIT_CMD_INS, "AP_FOCUS");
	SetDlgItemText(IDC_EDIT_CMD_INS_TYPE, "4");
	SetDlgItemText(IDC_SET_VALUE, "0.1");
	SetDlgItemText(IDC_MOVESTAGE_X, "300");
	SetDlgItemText(IDC_MOVESTAGE_Y, "300");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CZeissControllerX_Dialog::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CZeissControllerX_Dialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



/* 
 ======================================================================================================================================================================================
 ======================================================================================================================================================================================
 ======================================================================================================================================================================================
 ======================================================================================================================================================================================
 ======================================================================================================================================================================================
*/



// Constructor & Destructor
CZeissControllerX_Dialog::CZeissControllerX_Dialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ZEISSCONTROLLER_X_MAINDIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_oZeissDataCollection.Create(IDD_DATA_COLLECTION, this);

	m_bInitialised = false;


	m_pZeissControlManager = CTEMControlManager::GetInstance();
}

CZeissControllerX_Dialog::~CZeissControllerX_Dialog()
{
	SAFE_RELEASE(m_pZeissControlManager);
}



// Useful functions

void CZeissControllerX_Dialog::CreateConsole()
{
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

}




void CZeissControllerX_Dialog::OnBnCZInitialise()
{
	 
	
	m_pZeissControlManager = CTEMControlManager::GetInstance(); // The constructor will internally run the initializeapi function
}

struct _data_test_speed_
{
	float fAngle;
	float fRotSpeed;
};
void CZeissControllerX_Dialog::OnBnCZCloseControl()
{
	auto zeiss = CTEMControlManager::GetInstance();
	CString _usrString;
	GetDlgItemText(IDC_MOVESTAGE_X, _usrString);
	float fGoTo = atof(_usrString);

	if (zeiss)
	{
		std::ofstream rotation_speed("Rotation_Speed_Test.txt", std::ios::binary);
		std::vector<float> rotation_speed_val;

		zeiss->set_stage_tilt_angle(fGoTo);
		while (zeiss->is_stage_rotating() == false)
			Sleep(10);

		std::vector<_data_test_speed_> data;
		while (zeiss->is_stage_rotating())
		{
			rotation_speed_val.push_back(zeiss->get_stage_tilt_angle());
			std::this_thread::sleep_for(500ms);
		}

		for (int i = 1; i < rotation_speed_val.size(); i++)
		{
			//rotation_speed << std::format("{:.4f}\t\t{:.4f}\n", rotation_speed_val[i], fabs(rotation_speed_val[i]- rotation_speed_val[i-1]));
		}
		rotation_speed.close();
	}


	return;
}


void CZeissControllerX_Dialog::OnBnCZGet()
{
	VARIANT _var;
	//ZM(_var);

	CString _usrMsg;
	GetDlgItemText(IDC_EDIT_CMD_INS, _usrMsg);

	CString _usrMsgType;
	GetDlgItemText(IDC_EDIT_CMD_INS_TYPE, _usrMsgType);
	if (_usrMsgType.IsEmpty() == false)
		_var.vt = atoi(_usrMsgType);

	system("CLS");
	CTimer oTimer;
	oTimer.doStart();
	m_pZeissControlManager->zeiss_read(_usrMsg, _var, 0, true);
	oTimer.doEnd();
	printf("The call method to zeiss_read took: %d ms\n", oTimer.returnTotalElapsed());
	oTimer.doReset();
	

	
}


void CZeissControllerX_Dialog::OnBnCZSet()
{

	CString _usrMsgCmd, _usrMsgVal, _usrMsgType;
	VARIANT _var;
	//ZM(_var);

	GetDlgItemText(IDC_EDIT_CMD_INS, _usrMsgCmd);
	GetDlgItemText(IDC_SET_VALUE, _usrMsgVal);
	GetDlgItemText(IDC_EDIT_CMD_INS_TYPE, _usrMsgType);
	if (_usrMsgType.IsEmpty() == false)
		_var.vt = atoi(_usrMsgType);
	_var.fltVal = (float)atof(_usrMsgVal);


	CTimer oTimer;
	oTimer.doStart(); 
	
	m_pZeissControlManager->zeiss_write(_usrMsgCmd, _var, true, true);

	oTimer.doEnd();
	printf("The call method to zeiss_write took: %d ms\n", oTimer.returnTotalElapsed());
	oTimer.doReset();
}


void CZeissControllerX_Dialog::OnBnCZExec()
{
	CString _usrMsgCmd;
	GetDlgItemText(IDC_EDIT_CMD_INS, _usrMsgCmd);

	CTimer oTimer;
	oTimer.doStart(); 
	
	m_pZeissControlManager->zeiss_execute(_usrMsgCmd);

	oTimer.doEnd();
	printf("The call method to zeiss_execute took: %d ms\n", oTimer.returnTotalElapsed());
	oTimer.doReset();

}


void CZeissControllerX_Dialog::OnBnCZCapture()
{
	static unsigned int iNameIndex = 0;
	std::string oFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/STEMImageFromApp";
	m_pZeissControlManager->acquire_stem_image(oFileName, iNameIndex, true);
	m_pZeissControlManager->freeze_stem_mode(false);


}




void CZeissControllerX_Dialog::OnBnClickedMovestagemouse()
{
	CString _usrXcoords, _usrYcoords;
	GetDlgItemText(IDC_MOVESTAGE_X, _usrXcoords);
	GetDlgItemText(IDC_MOVESTAGE_Y, _usrYcoords); ;

	m_pZeissControlManager->m_pStage->move_stage_to_pixel_coordinates(static_cast<unsigned int>(atoi(_usrXcoords)), static_cast<unsigned int>(atoi(_usrYcoords)));
}


void CZeissControllerX_Dialog::OnMenuDataCollection()
{
	m_oZeissDataCollection.ShowWindow(SW_SHOW);
}


void CZeissControllerX_Dialog::OnBnClickedBtnClose()
{
	CZeissDataCollection_Dialog::m_bQuitThreads = true;
	std::this_thread::sleep_for(500ms);
	m_pZeissControlManager->do_large_screen_down();
	m_pZeissControlManager->close_control();


	//SAFE_RELEASE(m_pZeissControlManager);
	CDialogEx::OnCancel();
}

void CZeissControllerX_Dialog::OnBnClickedOpnDataCollection()
{
	
	//if (pDC && pDC->m_bKeepThreadRunning == false)
	//	pDC->infinite_loop_for_monitoring_ex();

	
	m_oZeissDataCollection.ShowWindow(SW_SHOW);
}

void CZeissControllerX_Dialog::OnBnClickedLensCtrlBtn()
{
	m_oZeissDataCollection.ShowWindow(SW_SHOW);

}

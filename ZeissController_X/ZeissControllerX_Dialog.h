#include "CZeissDataCollection_Dialog.h"
// ZeissController_XDlg.h : header file
//

#pragma once
using namespace APILib;

// CZeissControllerXDlg dialog
class CZeissControllerX_Dialog : public CDialogEx
{
private:
	CTEMControlManager*			m_pZeissControlManager;
	CZeissDataCollection_Dialog	m_oZeissDataCollection;
	CMenu						m_oMenu;
	bool						m_bInitialised;
	VARIANT						m_varError;

// Construction & Destruction
public:
	CZeissControllerX_Dialog(CWnd* pParent = nullptr);	// standard constructor
	~CZeissControllerX_Dialog(); // destructor

private:
	void CreateConsole();








// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ZEISSCONTROLLER_X_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnCZGet();
	afx_msg void OnBnCZInitialise();
	afx_msg void OnBnCZCloseControl();
	afx_msg void OnBnCZSet();
	afx_msg void OnBnCZExec();
	
	afx_msg void OnBnCZCapture();
	afx_msg void OnBnClickedMovestagemouse();
	afx_msg void OnMenuDataCollection();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedOpnDataCollection();
	afx_msg void OnBnClickedLensCtrlBtn();
};




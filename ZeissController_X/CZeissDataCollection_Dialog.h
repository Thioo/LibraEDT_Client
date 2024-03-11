#pragma once
#include "afxdialogex.h"


// CDataCollectionDialog dialog

class CZeissDataCollection_Dialog : public CDialogEx
{
	DECLARE_DYNAMIC(CZeissDataCollection_Dialog)
public:
	CZeissDataCollection_Dialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CZeissDataCollection_Dialog();
	static bool				 m_bQuitThreads;
	void EditItem(int nID, LPCTSTR val);

private:
	CDataCollection*	 m_pZeissDataCollection;
	CTimepix*			 m_pTimepix;
	CZeissQuickControl*  m_pZeissQuickControl;



private:
	void UpdateDataCollectionParameters();
	void InitItems();
	void UpdateDataGUI();
	void UpdateTEMParameters();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DATA_COLLECTION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual	BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDcBtnClose();
	afx_msg void OnBnClickedDcRecord();
	afx_msg void OnBnClickedDcTrack();
	
	afx_msg void OnBnClickedDcFindZHeightBtn();
	afx_msg void OnBnClickedDcReviewBtn();
	afx_msg void OnBnClickedDcQcToglargescreen();
	afx_msg void OnDeltaposDcQcWriteEmissionstep(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedDcLivestreaming();
	afx_msg void OnEnChangeDcGain();
	afx_msg void OnBnClickedDcQcTogblanking();
	afx_msg void OnBnClickedDcQcGoAlpha();
	afx_msg void OnBnClickedDcQcGoZ();
	afx_msg void OnBnClickedDcQcTogparallel();
	afx_msg void OnBnClickedDcCollectFrames();
	afx_msg void OnBnClickedDcQcSpeed70();
	afx_msg void OnBnClickedDcQcSpeed50();
	afx_msg void OnBnClickedDcQcSpeed40();
	afx_msg void OnBnClickedDcQcSpeed20();
	afx_msg void OnBnClickedDcCalibBeam();
	afx_msg void OnBnClickedDcSaveSearchBtn();
	afx_msg void OnBnClickedDcSaveImagingBtn();
	afx_msg void OnBnClickedDcSaveDiffBtn();
	afx_msg void OnBnClickedDcLoadSearchBtn();
	afx_msg void OnBnClickedDcLoadImagingBtn();
	afx_msg void OnBnClickedDcLoadDiffBtn();
	void create_required_directories();
	void disable_items();
	void disable_interval_items(int* item_list);
	void enable_items();
	void enable_interval_items(int* item_list);

	afx_msg void OnBnClickedDcTpxReset();
	afx_msg void OnBnClickedDcTpxResetchips();
	afx_msg void OnDeltaposDcQcChangeCamlen(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposDcQcChangeBrightness(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedDcUpdateRecordInfo();
	afx_msg void OnBnClickedDcSaveRawImg();
};

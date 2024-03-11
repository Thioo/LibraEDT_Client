// CDataCollectionDialog.cpp : implementation file
//

#include "pch.h"
#include "afxdialogex.h"
#include "CZeissDataCollection_Dialog.h"


// CDataCollectionDialog dialog
bool CZeissDataCollection_Dialog::m_bQuitThreads = false;

void CZeissDataCollection_Dialog::EditItem(int nID, LPCTSTR val)
{
	SetDlgItemText(nID, val);
}

IMPLEMENT_DYNAMIC(CZeissDataCollection_Dialog, CDialogEx)

CZeissDataCollection_Dialog::CZeissDataCollection_Dialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DATA_COLLECTION, pParent)
{

	
	m_pZeissDataCollection = CDataCollection::GetInstance();
	m_pZeissDataCollection->m_pDlg = this;
	m_pTEMControlManager   = CTEMControlManager::GetInstance();

	m_pTimepix			   = CTimepix::GetInstance();
	m_pZeissQuickControl   = new CZeissQuickControl();
}

CZeissDataCollection_Dialog::~CZeissDataCollection_Dialog()
{
	PRINT("CZeissDataCollection_Dlg::~CZeissDataCollection_Dlg()");
	m_bQuitThreads = true;
	std::this_thread::sleep_for(500ms);
	SAFE_RELEASE(m_pZeissQuickControl);
}

void CZeissDataCollection_Dialog::UpdateDataCollectionParameters()
{
	CString _usrMsg;
	float fValue = 0.0f;
	GetDlgItemText(IDC_ANG_FROM, _usrMsg);
	fValue = m_pZeissDataCollection->m_fStartingAng = static_cast<float>(atof(_usrMsg));
	m_pZeissDataCollection->m_fStartingAng = std::clamp(m_pZeissDataCollection->m_fStartingAng, -60.0f, 60.0f);
	_usrMsg.Format("%.2f", m_pZeissDataCollection->m_fStartingAng);
	if(fValue < -60.0f || fValue > 60.0f)
		SetDlgItemText(IDC_ANG_FROM, _usrMsg);

	GetDlgItemText(IDC_ANG_TO, _usrMsg);
	fValue = m_pZeissDataCollection->m_fEndingAng = static_cast<float>(atof(_usrMsg));
	m_pZeissDataCollection->m_fEndingAng = std::clamp(m_pZeissDataCollection->m_fEndingAng, -60.0f, 60.0f);
	_usrMsg.Format("%.2f", m_pZeissDataCollection->m_fEndingAng);
	if (fValue < -60.0f || fValue > 60.0f)
		SetDlgItemText(IDC_ANG_TO, _usrMsg);

	GetDlgItemText(IDC_DC_DELTA_Z, _usrMsg);
	fValue = m_pZeissDataCollection->m_fEucentricHeightDeltaZ = static_cast<float>(atof(_usrMsg));
	m_pZeissDataCollection->m_fEucentricHeightDeltaZ = std::clamp(m_pZeissDataCollection->m_fEucentricHeightDeltaZ, -100.0f, 100.0f);
	_usrMsg.Format("%.2f", m_pZeissDataCollection->m_fEucentricHeightDeltaZ);
	if (fValue < -100.0f || fValue > 100.0f)
		SetDlgItemText(IDC_DC_DELTA_Z, _usrMsg);

	GetDlgItemText(IDC_DC_RECORD_IMG_STEPS, _usrMsg);
	fValue = m_pZeissDataCollection->m_fRecordImgSteps = static_cast<float>(atof(_usrMsg));
	m_pZeissDataCollection->m_fRecordImgSteps = std::clamp(m_pZeissDataCollection->m_fRecordImgSteps, 1.0f, 25.0f);
	_usrMsg.Format("%.2f", m_pZeissDataCollection->m_fRecordImgSteps);
	if (fValue < 1.0f || fValue > 25.0f)
		SetDlgItemText(IDC_DC_RECORD_IMG_STEPS, _usrMsg);

	GetDlgItemText(IDC_DC_INTERVAL_FROM, _usrMsg);
	fValue = m_pZeissDataCollection->m_fRecordImgStepStartingAngVariable = static_cast<float>(atof(_usrMsg));
	m_pZeissDataCollection->m_fRecordImgStepStartingAngVariable = std::clamp(m_pZeissDataCollection->m_fRecordImgStepStartingAngVariable, -90.0f, 90.0f);
	_usrMsg.Format("%.2f", m_pZeissDataCollection->m_fRecordImgStepStartingAngVariable);
	if (fValue < -90.0f || fValue > 90.0f)
		SetDlgItemText(IDC_DC_INTERVAL_FROM, _usrMsg);

	GetDlgItemText(IDC_DC_INTERVAL_TO, _usrMsg);
	fValue = m_pZeissDataCollection->m_fRecordImgStepEndingAngVariable = static_cast<float>(atof(_usrMsg));
	m_pZeissDataCollection->m_fRecordImgStepEndingAngVariable = std::clamp(m_pZeissDataCollection->m_fRecordImgStepEndingAngVariable, -90.0f, 90.0f);
	_usrMsg.Format("%.2f", m_pZeissDataCollection->m_fRecordImgStepEndingAngVariable);
	if (fValue < -90.0f || fValue > 90.0f)
		SetDlgItemText(IDC_DC_INTERVAL_TO, _usrMsg);

	GetDlgItemText(IDC_DC_RECORD_IMG_STEPS_VARIABLE, _usrMsg);
	fValue = m_pZeissDataCollection->m_fRecordImgStepsVariable = static_cast<float>(atof(_usrMsg));
	m_pZeissDataCollection->m_fRecordImgStepsVariable = std::clamp(m_pZeissDataCollection->m_fRecordImgStepsVariable, 0.1f, 25.0f);
	_usrMsg.Format("%.2f", m_pZeissDataCollection->m_fRecordImgStepsVariable);
	if (fValue < 0.10f || fValue > 25.0f)
		SetDlgItemText(IDC_DC_RECORD_IMG_STEPS_VARIABLE, _usrMsg);

	GetDlgItemText(IDC_DC_DELTA_T, _usrMsg);
	fValue = m_pZeissDataCollection->m_fEucentricHeightTiltSteps = static_cast<float>(atof(_usrMsg));
	m_pZeissDataCollection->m_fEucentricHeightTiltSteps = std::clamp(m_pZeissDataCollection->m_fEucentricHeightTiltSteps, -60.0f, 60.0f);
	_usrMsg.Format("%.2f", m_pZeissDataCollection->m_fEucentricHeightTiltSteps);
	if (fValue < -60.0f || fValue > 60.0f)
		SetDlgItemText(IDC_DC_DELTA_T, _usrMsg);

	GetDlgItemText(IDC_DC_CALIBRATION_DELTA, _usrMsg);
	fValue = m_pZeissDataCollection->m_iCalibrationDeltaGUI = static_cast<float>(atoi(_usrMsg));
	m_pZeissDataCollection->m_iCalibrationDeltaGUI = std::clamp(m_pZeissDataCollection->m_iCalibrationDeltaGUI, 5, 500);
	_usrMsg.Format("%d", m_pZeissDataCollection->m_iCalibrationDeltaGUI);
	if (fValue < 5.0f || fValue > 500.0f)
		SetDlgItemText(IDC_DC_CALIBRATION_DELTA, _usrMsg);

	//m_pZeissDataCollection->m_bContinuousRecord = IsDlgButtonChecked(IDC_DC_CONTINUOUS_RECRD_RDBTN);
	m_pZeissDataCollection->m_bImageBasedRecord = IsDlgButtonChecked(IDC_DC_IMGBASED_RECRD_RDBTN);

	//m_pZeissDataCollection->m_bTimeBasedTrack = IsDlgButtonChecked(IDC_DC_TIMER_BASED_TRCK_RDBTN);
	//m_pZeissDataCollection->m_bAngleBasedTrack = IsDlgButtonChecked(IDC_DC_ANGLE_BASED_TRCK_RDBTN);
	m_pZeissDataCollection->m_bCheckForZHeight = IsDlgButtonChecked(IDC_DC_FIND_Z_HEIGHT_CHCKBTN);
	m_pZeissDataCollection->m_bSingleRun = IsDlgButtonChecked(IDC_DC_SINGLERUN_CHCKBTN);
	m_pZeissDataCollection->m_bWaitAfterRotation = IsDlgButtonChecked(IDC_DC_WAIT_CHCKBTN);
	m_pZeissDataCollection->m_bDoBlankRotation = IsDlgButtonChecked(IDC_DC_BLANK_ROT);
	m_pZeissDataCollection->m_bImageBasedTrack = IsDlgButtonChecked(IDC_DC_IMG_BASED_TRCK_RDBTN);
	m_pZeissDataCollection->m_bTrackCrystalPath = IsDlgButtonChecked(IDC_DC_DO_TRACKING);
	m_pZeissDataCollection->m_bPrecession = IsDlgButtonChecked(IDC_DC_PRECESSION);
	m_pZeissDataCollection->m_bVariableRecordSteps = IsDlgButtonChecked(IDC_DC_RECORD_VARIABLE_STEP);
	m_pZeissDataCollection->m_bStepwiseRecord = IsDlgButtonChecked(IDC_DC_RECORD_STEPWISE);
	//m_pZeissDataCollection->m_bLinearMovementTrack = IsDlgButtonChecked(IDC_DC_LINEAR_MOV_CHECKBTN);
	m_pZeissDataCollection->m_iImageBasedTrackingMode = IsDlgButtonChecked(IDC_DC_IMGBASED_ANGLEBASED_CHCKBTN) ? MODE_ANGLE_BASED : MODE_TIME_BASED;
	m_pZeissDataCollection->m_bMoveMouseTest = IsDlgButtonChecked(IDC_DC_MOVEMOUSE_CHCKBTN);
	m_pZeissDataCollection->m_bReadjustZValue = IsDlgButtonChecked(IDC_DC_READJUST_Z_CHCKBTN);
	m_pZeissDataCollection->m_bCorrectCalibration = IsDlgButtonChecked(IDC_DC_CORRECT_CALIBRATION);
	m_pTEMControlManager->m_bSaveLensParams = IsDlgButtonChecked(IDC_DC_SAVE_LENS_PARAMS);
	if (m_pZeissDataCollection->m_bSingleRun == false)
	{
		CheckDlgButton(IDC_DC_WAIT_CHCKBTN, false);
		m_pZeissDataCollection->m_bWaitAfterRotation = false;
	}
	if (m_pZeissDataCollection->m_bWaitAfterRotation)
	{
		CheckDlgButton(IDC_DC_SINGLERUN_CHCKBTN, true);
		m_pZeissDataCollection->m_bSingleRun = true;
	}
	


	/* Detector Parameters Here */
	
	GetDlgItemText(IDC_DC_EXPOSURE, _usrMsg);
	fValue = m_pTimepix->m_iExposureTimeDiff = static_cast<int>(atoi(_usrMsg));
	m_pTimepix->m_iExposureTimeDiff = std::clamp(m_pTimepix->m_iExposureTimeDiff, 1, 30000);
	_usrMsg.Format("%d", m_pTimepix->m_iExposureTimeDiff);
	if (fValue < 1.0f || fValue > 30000.0f)
		SetDlgItemText(IDC_DC_EXPOSURE, _usrMsg);

	GetDlgItemText(IDC_DC_IMG_EXPTIME, _usrMsg);
	fValue = m_pTimepix->m_iExposureTimeImg = static_cast<int>(atoi(_usrMsg));
	m_pTimepix->m_iExposureTimeImg = std::clamp(m_pTimepix->m_iExposureTimeImg, 1, 30000);
	_usrMsg.Format("%d", m_pTimepix->m_iExposureTimeImg);
	if (fValue < 1.0f || fValue > 30000.0f)
		SetDlgItemText(IDC_DC_IMG_EXPTIME, _usrMsg);

	GetDlgItemText(IDC_DC_GAIN, _usrMsg);
	m_pTimepix->m_fContrastDiff = static_cast<float>(atof(_usrMsg));
	GetDlgItemText(IDC_DC_IMG_GAIN, _usrMsg);
	m_pTimepix->m_fContrastImg = static_cast<float>(atof(_usrMsg));

	GetDlgItemText(IDC_DC_BIAS, _usrMsg);
	m_pTimepix->m_fBrightnessDiff = static_cast<float>(atof(_usrMsg));
	GetDlgItemText(IDC_DC_IMG_BIAS, _usrMsg);
	m_pTimepix->m_fBrightnessImg = static_cast<float>(atof(_usrMsg));

	m_pTimepix->m_bInvertColours = IsDlgButtonChecked(IDC_DC_CB_INVERTED) ? true : false;

}

void CZeissDataCollection_Dialog::InitItems()
{

	CDataCollection* pDC = CDataCollection::GetInstance();
	CTEMControlManager* pTem = CTEMControlManager::GetInstance();
	CWriteFile* pWriter = CWriteFile::GetInstance();

	pWriter->read_params_file();
	auto mapConfig = pWriter->get_configMap();
	if (mapConfig.empty() == false)
	{
		if (mapConfig.contains("Date"))
		{
			auto val = mapConfig.at("Date");
			auto today = std::format("{:%Y_%m_%d}", std::chrono::system_clock::now());
			if (val == today)
			{
				// Only load the config if its been saved on the same day.
				pDC->restore_parameters();
				pDC->restore_tracking_data();


				pWriter->is_config_loaded = true;
			}
		}
	}

	SetDlgItemText(IDC_ANG_FROM, "-60.00");
	SetDlgItemText(IDC_DC_INTERVAL_FROM, "-40.00");
	SetDlgItemText(IDC_ANG_TO, "60.00");
	SetDlgItemText(IDC_DC_INTERVAL_TO, "-05.00");
	SetDlgItemText(IDC_DC_DELTA_T, "30.00");
	SetDlgItemText(IDC_DC_DELTA_Z, "10.00");
	SetDlgItemText(IDC_DC_CALIBRATION_DELTA, "50");
	SetDlgItemText(IDC_DC_EXPOSURE, "500");
	SetDlgItemText(IDC_DC_IMG_EXPTIME, "100");
	SetDlgItemText(IDC_DC_GAIN, "50.0");
	SetDlgItemText(IDC_DC_IMG_GAIN, "1.0");
	SetDlgItemText(IDC_DC_BIAS, "0.0");
	SetDlgItemText(IDC_DC_IMG_BIAS, "0.0");
	SetDlgItemText(IDC_DC_CRYSTAL_NAME, "experiment");
	SetDlgItemText(IDC_DC_CRYSTAL_ID, "01");
	SetDlgItemText(IDC_DC_NUM_OF_FRAMES_TO_TAKE, "0");
	SetDlgItemText(IDC_DC_RECORD_IMG_STEPS, "2.0");
	SetDlgItemText(IDC_DC_RECORD_IMG_STEPS_VARIABLE, "2.0");

	CheckDlgButton(IDC_DC_IMGBASED_RECRD_RDBTN, true);
	CheckDlgButton(IDC_DC_IMG_BASED_TRCK_RDBTN, true);
	CheckDlgButton(IDC_DC_MOVEMOUSE_CHCKBTN, false);
	CheckDlgButton(IDC_DC_SINGLERUN_CHCKBTN, true);
	CheckDlgButton(IDC_DC_BLANK_ROT, true);
	CheckDlgButton(IDC_DC_RECORD_VARIABLE_STEP, false);
	CheckDlgButton(IDC_DC_READJUST_Z_CHCKBTN, false);
	CheckDlgButton(IDC_DC_DO_TRACKING, true);
	CheckDlgButton(IDC_DC_SAVE_LENS_PARAMS, false);
	CheckDlgButton(IDC_DC_CORRECT_CALIBRATION, false);

	if (pWriter->is_config_loaded)
	{
		SetDlgItemText(IDC_DC_OPERATOR_NAME, pDC->m_sOperatorName.c_str());
		SetDlgItemText(IDC_DC_CRYSTAL_NAME, pDC->m_sCrystalName.c_str());
		SetDlgItemText(IDC_DC_CRYSTAL_ID, pDC->m_sCrystalNumberID.c_str());
		SetDlgItemText(IDC_DC_CRYSTAL_ID_EXTRA, pDC->m_sCrystalNameExtra.c_str());
		SetDlgItemText(IDC_DC_NUM_OF_FRAMES_TO_TAKE, std::to_string(pDC->m_iNumOfFramesToTake).c_str());
		SetDlgItemText(IDC_DC_CALIBRATION_DELTA, std::to_string(pDC->m_iCalibrationDeltaGUI).c_str());

		SetDlgItemText(IDC_ANG_FROM, std::format("{:.2f}", pDC->m_fStartingAng).c_str());
		SetDlgItemText(IDC_ANG_TO, std::format("{:.2f}", pDC->m_fEndingAng).c_str());
		SetDlgItemText(IDC_DC_INTERVAL_FROM, std::format("{:.2f}", pDC->m_fRecordImgStepStartingAngVariable).c_str());
		SetDlgItemText(IDC_DC_INTERVAL_TO, std::format("{:.2f}", pDC->m_fRecordImgStepEndingAngVariable).c_str());
		SetDlgItemText(IDC_DC_DELTA_T, std::format("{:.2f}", pDC->m_fEucentricHeightTiltSteps).c_str());
		SetDlgItemText(IDC_DC_DELTA_Z, std::format("{:.2f}", pDC->m_fEucentricHeightDeltaZ).c_str());
		SetDlgItemText(IDC_DC_RECORD_IMG_STEPS, std::format("{:.2f}", pDC->m_fRecordImgSteps).c_str());
		SetDlgItemText(IDC_DC_RECORD_IMG_STEPS_VARIABLE, std::format("{:.2f}", pDC->m_fRecordImgStepsVariable).c_str());

		CheckDlgButton(IDC_DC_IMGBASED_RECRD_RDBTN, pDC->m_bImageBasedRecord);
		CheckDlgButton(IDC_DC_BLANK_ROT, pDC->m_bDoBlankRotation);
		CheckDlgButton(IDC_DC_RECORD_STEPWISE, pDC->m_bStepwiseRecord);
		CheckDlgButton(IDC_DC_RECORD_VARIABLE_STEP, pDC->m_bVariableRecordSteps);
		//CheckDlgButton(IDC_DC_CONTINUOUS_RECRD_RDBTN, pDC->m_bContinuousRecord);
		CheckDlgButton(IDC_DC_IMG_BASED_TRCK_RDBTN, pDC->m_bImageBasedTrack);
		//CheckDlgButton(IDC_DC_ANGLE_BASED_TRCK_RDBTN, pDC->m_bAngleBasedTrack);
		//CheckDlgButton(IDC_DC_TIMER_BASED_TRCK_RDBTN, pDC->m_bTimeBasedTrack);
		CheckDlgButton(IDC_DC_IMGBASED_ANGLEBASED_CHCKBTN, static_cast<bool>(pDC->m_iImageBasedTrackingMode));
		//CheckDlgButton(IDC_DC_LINEAR_MOV_CHECKBTN, pDC->m_bLinearMovementTrack);
		CheckDlgButton(IDC_DC_MOVEMOUSE_CHCKBTN, pDC->m_bMoveMouseTest);
		CheckDlgButton(IDC_DC_READJUST_Z_CHCKBTN, pDC->m_bReadjustZValue);
		CheckDlgButton(IDC_DC_SAVE_LENS_PARAMS, pTem->m_bSaveLensParams);
		CheckDlgButton(IDC_DC_CORRECT_CALIBRATION, pDC->m_bCorrectCalibration);
	}


}


void CZeissDataCollection_Dialog::UpdateDataGUI()
{
	std::this_thread::sleep_for(2s);
	//if (CTEMControlManager::GetInstance()->Initialised() == false)
	//	return;
	auto dc = CDataCollection::GetInstance();

	bool is_variable_interval_disabled = false;
	while(m_bQuitThreads == false)
	{
		if (m_pZeissQuickControl == nullptr || m_pZeissQuickControl->m_pZeissStage == nullptr || m_pZeissQuickControl->m_pZeissController == nullptr)
			break;
		if (m_pZeissDataCollection == nullptr)
			break;
		
		m_pZeissQuickControl->m_bUpdateData = false;
		CString _textVal;

		_textVal.Format("%.2f", m_pZeissQuickControl->get_actual_emission_current());
		SetDlgItemText(IDC_DC_QC_READ_CURRENT, _textVal);

		_textVal.Format("%.2f", m_pZeissQuickControl->get_stage_tilt_angle());
		SetDlgItemText(IDC_DC_QC_READ_ALPHA, _textVal);

		_textVal.Format("%.2f", m_pZeissQuickControl->get_stage_z_position());
		SetDlgItemText(IDC_DC_QC_READ_ZVAL, _textVal);

		_textVal.Format("%.2f", m_pZeissQuickControl->get_spot_size());
		SetDlgItemText(IDC_DC_QC_READ_SPOTSIZE, _textVal);

		_textVal.Format("%.2f", m_pZeissQuickControl->get_camera_length());
		SetDlgItemText(IDC_DC_QC_READ_CAMLENGTH, _textVal);

		this->UpdateDataCollectionParameters();

		if (dc->m_bEnable_items)
			this->enable_items();

		if (dc->m_bVariableRecordSteps == false && is_variable_interval_disabled == false )
		{
			this->disable_interval_items(nullptr);
			is_variable_interval_disabled = true;

		}
		else if (dc->m_bVariableRecordSteps && is_variable_interval_disabled)
		{
			this->enable_interval_items(nullptr);
			is_variable_interval_disabled = false;
		}

		m_pZeissQuickControl->reload_tem_settings();


		if (m_bQuitThreads)
			break;
		//std::this_thread::sleep_for(2s);
		std::this_thread::sleep_for(500ms);
	}
}

void CZeissDataCollection_Dialog::UpdateTEMParameters()
{
	auto zeiss = CTEMControlManager::GetInstance();
	while (m_bQuitThreads == false)
	{
		if (zeiss == nullptr)
			zeiss = CTEMControlManager::GetInstance();
		else
		{
			zeiss->get_stage_tilt_angle(true); // unchecked
			zeiss->is_stage_rotating(true); // unchecked
			zeiss->is_stage_busy(true); // unchecked
			zeiss->get_actual_emission_current(true);
			zeiss->get_spot_size(true);
			zeiss->get_stage_z(true);
			zeiss->get_camera_length(true); 
		}
		std::this_thread::sleep_for(500ms);
	}
}

void CZeissDataCollection_Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BOOL CZeissDataCollection_Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitItems();
	return TRUE;
}

BEGIN_MESSAGE_MAP(CZeissDataCollection_Dialog, CDialogEx)
	ON_BN_CLICKED(ID_DC_BTN_CLOSE, &CZeissDataCollection_Dialog::OnBnClickedDcBtnClose)
	ON_BN_CLICKED(IDC_DC_RECORD, &CZeissDataCollection_Dialog::OnBnClickedDcRecord)
	ON_BN_CLICKED(IDC_DC_TRACK, &CZeissDataCollection_Dialog::OnBnClickedDcTrack)

	ON_BN_CLICKED(IDC_DC_FIND_Z_HEIGHT_BTN, &CZeissDataCollection_Dialog::OnBnClickedDcFindZHeightBtn)
	ON_BN_CLICKED(IDC_DC_REVIEW_BTN, &CZeissDataCollection_Dialog::OnBnClickedDcReviewBtn)
	ON_BN_CLICKED(IDC_DC_QC_TOGLARGESCREEN, &CZeissDataCollection_Dialog::OnBnClickedDcQcToglargescreen)
	ON_NOTIFY(UDN_DELTAPOS, IDC_DC_QC_WRITE_EMISSIONSTEP, &CZeissDataCollection_Dialog::OnDeltaposDcQcWriteEmissionstep)
	ON_BN_CLICKED(IDC_DC_LIVESTREAMING, &CZeissDataCollection_Dialog::OnBnClickedDcLivestreaming)
	ON_EN_CHANGE(IDC_DC_GAIN, &CZeissDataCollection_Dialog::OnEnChangeDcGain)
	ON_BN_CLICKED(IDC_DC_QC_TOGBLANKING, &CZeissDataCollection_Dialog::OnBnClickedDcQcTogblanking)
	ON_BN_CLICKED(IDC_DC_QC_GO_ALPHA, &CZeissDataCollection_Dialog::OnBnClickedDcQcGoAlpha)
	ON_BN_CLICKED(IDC_DC_QC_GO_Z, &CZeissDataCollection_Dialog::OnBnClickedDcQcGoZ)
	ON_BN_CLICKED(IDC_DC_QC_TOGPARALLEL, &CZeissDataCollection_Dialog::OnBnClickedDcQcTogparallel)
	ON_BN_CLICKED(IDC_DC_COLLECT_FRAMES, &CZeissDataCollection_Dialog::OnBnClickedDcCollectFrames)
	ON_BN_CLICKED(IDC_DC_QC_SPEED_70, &CZeissDataCollection_Dialog::OnBnClickedDcQcSpeed70)
	ON_BN_CLICKED(IDC_DC_QC_SPEED_50, &CZeissDataCollection_Dialog::OnBnClickedDcQcSpeed50)
	ON_BN_CLICKED(IDC_DC_QC_SPEED_40, &CZeissDataCollection_Dialog::OnBnClickedDcQcSpeed40)
	ON_BN_CLICKED(IDC_DC_QC_SPEED_20, &CZeissDataCollection_Dialog::OnBnClickedDcQcSpeed20)
	ON_BN_CLICKED(IDC_DC_CALIB_BEAM, &CZeissDataCollection_Dialog::OnBnClickedDcCalibBeam)
	ON_BN_CLICKED(IDC_DC_SAVE_SEARCH_BTN, &CZeissDataCollection_Dialog::OnBnClickedDcSaveSearchBtn)
	ON_BN_CLICKED(IDC_DC_SAVE_IMGING_BTN, &CZeissDataCollection_Dialog::OnBnClickedDcSaveImagingBtn)
	ON_BN_CLICKED(IDC_DC_SAVE_DIFF_BTN, &CZeissDataCollection_Dialog::OnBnClickedDcSaveDiffBtn)
	ON_BN_CLICKED(IDC_DC_LOAD_SEARCH_BTN, &CZeissDataCollection_Dialog::OnBnClickedDcLoadSearchBtn)
	ON_BN_CLICKED(IDC_DC_LOAD_IMGING_BTN, &CZeissDataCollection_Dialog::OnBnClickedDcLoadImagingBtn)
	ON_BN_CLICKED(IDC_DC_LOAD_DIFF_BTN, &CZeissDataCollection_Dialog::OnBnClickedDcLoadDiffBtn)
	ON_BN_CLICKED(IDC_DC_TPX_RESETCHIPS, &CZeissDataCollection_Dialog::OnBnClickedDcTpxResetchips)
	ON_NOTIFY(UDN_DELTAPOS, IDC_DC_QC_CHANGE_CAMLEN, &CZeissDataCollection_Dialog::OnDeltaposDcQcChangeCamlen)
	ON_NOTIFY(UDN_DELTAPOS, IDC_DC_QC_CHANGE_BRIGHTNESS, &CZeissDataCollection_Dialog::OnDeltaposDcQcChangeBrightness)
	ON_BN_CLICKED(IDC_DC_UPDATE_RECORD_INFO, &CZeissDataCollection_Dialog::OnBnClickedDcUpdateRecordInfo)
	ON_BN_CLICKED(IDC_DC_SAVE_RAW_IMG, &CZeissDataCollection_Dialog::OnBnClickedDcSaveRawImg)
END_MESSAGE_MAP()


BOOL CZeissDataCollection_Dialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// we update the gui anyways
	static bool bDoOnce = false;
	if (bDoOnce == false)
	{
		bDoOnce = true;
		std::thread t(&CZeissDataCollection_Dialog::UpdateDataGUI, this);
		t.detach();

		std::thread t2(&CZeissDataCollection_Dialog::UpdateTEMParameters, this);
		t2.detach();
	}


	return CDialogEx::OnCommand(wParam, lParam);
}



// CDataCollectionDialog message handlers


void CZeissDataCollection_Dialog::OnBnClickedDcBtnClose()
{
	CDialogEx::OnCancel();
	m_pZeissQuickControl->do_toggle_large_screen(1);
	m_pZeissDataCollection->m_bKeepThreadRunning = false;
	//m_pTimepix->do_reset_matrix();
	std::this_thread::sleep_for(200ms);
	m_pTimepix->m_bLiveStreaming = false;
	m_pTimepix->tcp_is_live_streaming = false;
	GetDlgItem(IDC_DC_LIVESTREAMING)->SetWindowText("Start Streaming");

	CWriteFile::GetInstance()->write_params_file();
}


void CZeissDataCollection_Dialog::OnBnClickedDcRecord()
{

	this->disable_items();
	this->UpdateDataCollectionParameters();

	this->create_required_directories();

	if (m_pZeissDataCollection->m_bCheckForZHeight)
		m_pZeissDataCollection->do_find_eucentric_height_regions_ex();


	//m_pZeissDataCollection->do_record_ex();
	std::thread t(&CDataCollection::do_record_ex, m_pZeissDataCollection);
	t.detach();


	CString _usrMsg;
	GetDlgItemText(IDC_ANG_TO, _usrMsg);
	float _fAlpha = atof(_usrMsg);
	if (m_pZeissDataCollection->is_data_collection_direction_positive() == false) // Right to Left
		_fAlpha -= 0.5f;
	else // Left to Right
		_fAlpha += 0.5f;
	_fAlpha = std::clamp(_fAlpha, -65.0f, 65.0f);
	_usrMsg.Format("%.2f", _fAlpha);
	SetDlgItemText(IDC_DC_QC_WRITE_ALPHA, _usrMsg);

	CWriteFile::GetInstance()->write_params_file();
}


void CZeissDataCollection_Dialog::OnBnClickedDcTrack()
{
	this->disable_items();
	this->UpdateDataCollectionParameters();


	//m_pZeissDataCollection->do_track_ex();
	std::thread t(&CDataCollection::do_track_ex, m_pZeissDataCollection);
	t.detach();
}	



void CZeissDataCollection_Dialog::OnBnClickedDcFindZHeightBtn()
{
	//NO LONGER USED
	return;

	bool bStartStreamingAfterwards = m_pTimepix->m_bLiveStreaming;
	if (bStartStreamingAfterwards)
		this->OnBnClickedDcLivestreaming();

	this->UpdateDataCollectionParameters();
	
	m_pZeissDataCollection->do_find_eucentric_height_regions_ex();
	if (bStartStreamingAfterwards)
		this->OnBnClickedDcLivestreaming();
}


void CZeissDataCollection_Dialog::OnBnClickedDcReviewBtn()
{
	//this->disable_items();
/*
	bool bStartStreamingAfterwards = m_pTimepix->m_bLiveStreaming;
	if (bStartStreamingAfterwards)
		this->OnBnClickedDcLivestreaming();*/

	std::thread t(&CDataCollection::display_images_and_calculate_z_value, m_pZeissDataCollection);
	t.join(); // t.detach() instead, if window not responding is annoying}
/*

	if (bStartStreamingAfterwards)
		this->OnBnClickedDcLivestreaming();*/
}




void CZeissDataCollection_Dialog::OnBnClickedDcQcToglargescreen()
{

	m_pZeissQuickControl->do_toggle_large_screen();
}


void CZeissDataCollection_Dialog::OnDeltaposDcQcWriteEmissionstep(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here

	bool bValueUp = pNMUpDown->iDelta == -1 ? true : false;
	m_pZeissQuickControl->change_actual_current_step(bValueUp);

	*pResult = 0;
}


void CZeissDataCollection_Dialog::OnBnClickedDcLivestreaming()
{
	//if (m_pTimepix->m_bLiveStreaming == false)
	if(m_pTimepix->tcp_is_live_streaming == false)
	{
		if(CTimepix::GetInstance()->is_relaxd_module_initialized())
		{
			m_pZeissQuickControl->do_toggle_large_screen(0); // check if its up, if not, lift it up.
			//CTimepix::GetInstance()->prepare_for_live_stream();
			CTimepix::GetInstance()->tcp_prepare_for_live_stream();
			GetDlgItem(IDC_DC_LIVESTREAMING)->SetWindowText("Stop Streaming");
		}
		else
			PRINT("Patience! Communication with Timepix has not been established yet!");

	}
	else
	{
		m_pTimepix->m_bLiveStreaming = false;
		m_pTimepix->tcp_is_live_streaming = false;
		GetDlgItem(IDC_DC_LIVESTREAMING)->SetWindowText("Start Streaming");
	}
}


void CZeissDataCollection_Dialog::OnEnChangeDcGain()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	
}


void CZeissDataCollection_Dialog::OnBnClickedDcQcTogblanking()
{
	m_pZeissQuickControl->do_toggle_beam_blank();
}


void CZeissDataCollection_Dialog::OnBnClickedDcQcGoAlpha()
{

	// Clamp the alpha value to the limits
	CString _usrMsg;
	GetDlgItemText(IDC_DC_QC_WRITE_ALPHA, _usrMsg);
	float _fAlpha = atof(_usrMsg);
	_fAlpha = std::clamp(_fAlpha, -65.0f, 65.0f);
	_usrMsg.Format("%.2f", _fAlpha);
	SetDlgItemText(IDC_DC_QC_WRITE_ALPHA, _usrMsg); // Just to reassure the user. But in reality, the mscope itself checks also for the limits




	if(m_pZeissDataCollection->m_bOnRecording)
	{
		// Take the first still image to use as a reference for offsetting later on during tracking.
		
		ImagesInfo oImgInfo;
		CTimer oTimer;
		std::string oFileName = m_pZeissDataCollection->m_sTrackingImgPath + "tracking";
		unsigned int imgCount = 0;
		if(m_pZeissDataCollection->is_on_stem_mode() == true)
			m_pZeissDataCollection->do_fill_image_based_vector_stem(oImgInfo, oTimer, oFileName, imgCount);
		else
			m_pZeissDataCollection->do_fill_image_based_vector_tem(oImgInfo, oTimer, oFileName, imgCount);


		//m_pZeissDataCollection->m_oRecordTimer.doStart();
	}
/*
	else if (m_pZeissDataCollection->m_bOnTracking)
	{
		m_pZeissDataCollection->m_oTrackingTimer.doStart();
	}
*/


	m_pZeissDataCollection->m_bOnRotateRequest = true;
	if (m_pZeissDataCollection->m_bOnTracking)
	{
		m_pZeissQuickControl->stage_go_to_alpha(_fAlpha);
		m_pZeissDataCollection->m_oTrackingTimer.doStart();

		// Update the crystal id (crystal number) textbox
		GetDlgItemText(IDC_DC_CRYSTAL_ID, _usrMsg);
		SetDlgItemText(IDC_DC_CRYSTAL_ID, std::format("{:03d}", 1 + atoi(_usrMsg)).c_str());
		GetDlgItemText(IDC_DC_CRYSTAL_ID, _usrMsg);
		m_pZeissDataCollection->m_sCrystalNumberID = std::format("{:03d}", atoi(_usrMsg));
	}
	else if (m_pZeissDataCollection->m_bOnRecording)
	{
		if(m_pZeissDataCollection->m_bStepwiseRecord == false)
			m_pZeissQuickControl->stage_go_to_alpha(_fAlpha);
		m_pZeissDataCollection->m_oRecordTimer.doStart();
	}



	std::this_thread::sleep_for(500ms);
	m_pZeissDataCollection->m_bOnRotateRequest = false;


	CWriteFile::GetInstance()->write_params_file();
}


void CZeissDataCollection_Dialog::OnBnClickedDcQcGoZ()
{
	// Clamp the z value to the limits
	CString _usrMsg;
	GetDlgItemText(IDC_DC_QC_WRITE_ZVAL, _usrMsg);
	float _fAlpha = atof(_usrMsg);
	_fAlpha = std::clamp(_fAlpha, -360.0f, 870.0f);
	_usrMsg.Format("%.2f", _fAlpha);
	SetDlgItemText(IDC_DC_QC_WRITE_ZVAL, _usrMsg); // Just to reassure the user. But in reality, the mscope itself checks also for the limits

	m_pZeissQuickControl->stage_go_to_z(_fAlpha);
}


void CZeissDataCollection_Dialog::OnBnClickedDcQcTogparallel()
{
	bool bCurrentState = m_pZeissQuickControl->do_toggle_parallel_beam();
	if (bCurrentState == true)
		GetDlgItem(IDC_DC_QC_TOGPARALLEL)->SetWindowText("Go Convergent");
	else
		GetDlgItem(IDC_DC_QC_TOGPARALLEL)->SetWindowText("Go Parallel");
}



//#define _TEST_
#ifdef _TEST_
void CZeissDataCollection_Dialog::OnBnClickedDcCollectFrames()
{
	PRINT("DEBUG_BUTTON_ ORIGINAL COPIED BELLOW");
	bool bStartStreamingAfterwards = m_pTimepix->m_bLiveStreaming;
	if (bStartStreamingAfterwards)
		this->OnBnClickedDcLivestreaming();


	CString _usrMsg;
	const auto now = std::chrono::system_clock::now();

	GetDlgItemText(IDC_DC_NUM_OF_FRAMES_TO_TAKE, _usrMsg);
	m_pZeissDataCollection->m_iNumOfFramesToTake = atoi(_usrMsg);

	GetDlgItemText(IDC_DC_CRYSTAL_NAME, _usrMsg);
	m_pZeissDataCollection->m_sCrystalName = _usrMsg;

	m_pZeissDataCollection->m_sCollectionDate = std::format("{:%Y_%m_%d}", now);
	/*std::string sDateandName = std::format("{}/{:%Y_%m_%d}/", _usrMsg, now);*/

	GetDlgItemText(IDC_DC_CRYSTAL_ID, _usrMsg);
	m_pZeissDataCollection->m_sCrystalNumberID = std::format("{:03d}", atoi(_usrMsg));

	std::string sCrystalNum = m_pZeissDataCollection->m_sCrystalNumberID;
	sCrystalNum.append("/");

	GetDlgItemText(IDC_DC_OPERATOR_NAME, _usrMsg);
	m_pZeissDataCollection->m_sOperatorName = _usrMsg;
	std::string sOperatorName = m_pZeissDataCollection->m_sOperatorName;
	sOperatorName.append("/");
/*
	m_pZeissDataCollection->m_sDatasetPath = "C:/Moussa_Software_Data/" + sOperatorName + sDateandName + sCrystalNum;
*/



	//CWriteFile::create_directory(m_pZeissDataCollection->m_sDatasetPath);
	//this->UpdateDataCollectionParameters();


	std::vector<SFrame> _pFrame;
	SFrame frame;
	//frame.sDirectory = "C:/Users/Administrator/Documents/Temporary_Working_Files/Iph_Ext_Mn/2023_01_23/003/"; //C:/Moussa_Software_Data/2022_12_23_with beam_shift/003/
	frame.sDirectory = "F:/3D_Electron_Diffraction/3D_ED_Samples/ULL_MOF/002_RT/";
	frame.bLiveStream = true;
	_pFrame.push_back(frame);
	frame.bLiveStream = false;
	for (int i = 0; i < 105; i++)
	{
		frame.fAngle = -60.0f + 0.65 * i;
		frame.sImgName = std::format("RT{:05d}.tiff", i+1);
		frame.sFullpath = frame.sDirectory + frame.sImgName;
		_pFrame.push_back(frame);
	}


	CPostDataCollection oDataCol(&_pFrame, 230);
	oDataCol.m_fIntegrationSteps = 0.65;
	oDataCol.m_fOscRange = 0.65;
	m_pZeissDataCollection->m_fStartingAng = -60.0f;
	//oDataCol.do_make_pets_file();
	oDataCol.do_flatfield_and_cross_correction();
	oDataCol.do_make_xds_file();

	return;
	//m_pZeissDataCollection->do_collect_frames_ex();

	//m_pZeissDataCollection->DoTrackEx();


	SetDlgItemText(IDC_DC_CRYSTAL_ID, std::format("{:03d}", 1 + atoi(_usrMsg)).c_str());

}

#else
void CZeissDataCollection_Dialog::OnBnClickedDcCollectFrames()
{
	if (CTimepix::GetInstance()->is_relaxd_module_initialized() == false)
	{
		PRINT("Communication with Timepix has not been established yet! Try again later");
		return;
	}

	this->disable_items();
	//bool bStartStreamingAfterwards = m_pTimepix->m_bLiveStreaming;
	//if (bStartStreamingAfterwards)
	//	this->OnBnClickedDcLivestreaming();
	
	this->UpdateDataCollectionParameters();
	this->create_required_directories();


	/*CString _usrMsg;
	GetDlgItemText(IDC_DC_CRYSTAL_ID, _usrMsg);
	SetDlgItemText(IDC_DC_CRYSTAL_ID, std::format("{:03d}", 1 + atoi(_usrMsg)).c_str());*/
	
	m_pZeissDataCollection->m_sCrystalNumberID_cpy = m_pZeissDataCollection->m_sCrystalNumberID;
	m_pZeissDataCollection->tcp_do_collect_frames_ex();

	
	/*GetDlgItemText(IDC_DC_CRYSTAL_ID, _usrMsg);
	m_pZeissDataCollection->m_sCrystalNumberID = std::format("{:03d}", atoi(_usrMsg));*/
	CWriteFile::GetInstance()->write_params_file();

}

#endif

void CZeissDataCollection_Dialog::OnBnClickedDcCalibBeam()
{
	if (m_pTimepix->is_relaxd_module_initialized() == false)
	{
		PRINT("Communication with Timepix has not been established yet! Try again later");
		return;
	}

	this->UpdateDataCollectionParameters();
	this->create_required_directories();
	




	// If the streaming window is not opened, let's open it.
	//if (m_pTimepix->m_bLiveStreaming == false)
	if (m_pTimepix->tcp_is_live_streaming == false)
	{
		OnBnClickedDcLivestreaming();
	}

	// Step 1. On a new thread.
	m_pZeissDataCollection->do_calibrate_beam_shift_tem_ex();
	this->OnBnClickedDcSaveDiffBtn();

	CWriteFile::GetInstance()->write_params_file();

}

void CZeissDataCollection_Dialog::OnBnClickedDcQcSpeed70()
{
	auto zeiss = CTEMControlManager::GetInstance();
	if(zeiss)
		zeiss->set_stage_tilt_speed(70);
}


void CZeissDataCollection_Dialog::OnBnClickedDcQcSpeed50()
{
	auto zeiss = CTEMControlManager::GetInstance();
	if (zeiss)
		zeiss->set_stage_tilt_speed(50);
}


void CZeissDataCollection_Dialog::OnBnClickedDcQcSpeed40()
{
	auto zeiss = CTEMControlManager::GetInstance();
	if (zeiss)
		zeiss->set_stage_tilt_speed(40);
}


afx_msg void CZeissDataCollection_Dialog::OnBnClickedDcQcSpeed20()
{
	auto zeiss = CTEMControlManager::GetInstance();
	if (zeiss)
		zeiss->set_stage_tilt_speed(20);
}


void CZeissDataCollection_Dialog::OnBnClickedDcSaveSearchBtn()
{
	TEMModeParameters::current_parameters = CTEMControlManager::GetInstance()->get_mag_mode() == MAG_MODE::MAG_MODE_TEM ? PARAM_SEARCHING : PARAM_SEARCHING_LOWMAG;
	m_pZeissDataCollection->m_oSearchingParams.SaveCurrentParameters();

	CWriteFile::GetInstance()->write_params_file();
}

void CZeissDataCollection_Dialog::OnBnClickedDcLoadSearchBtn()
{
	TEM_SETTING tem_setting = CTEMControlManager::GetInstance()->get_mag_mode() == MAG_MODE::MAG_MODE_TEM ? PARAM_SEARCHING : PARAM_SEARCHING_LOWMAG;
	m_pZeissDataCollection->m_oSearchingParams.RestoreCurrentParameters(tem_setting);
	static auto pTpx = CTimepix::GetInstance();
	pTpx->m_vTargetBeamCoords.clear();
	pTpx->m_iTargetBeamCoordsIndex = 0;

	CWriteFile::GetInstance()->write_params_file();
}

afx_msg void CZeissDataCollection_Dialog::OnBnClickedDcSaveImagingBtn()
{
	TEMModeParameters::current_parameters = CTEMControlManager::GetInstance()->get_mag_mode() == MAG_MODE::MAG_MODE_TEM ? PARAM_IMAGING : PARAM_IMAGING_LOWMAG;
	m_pZeissDataCollection->m_oImagingParams.SaveCurrentParameters();
	
	CWriteFile::GetInstance()->write_params_file();
}

afx_msg void CZeissDataCollection_Dialog::OnBnClickedDcLoadImagingBtn()
{
	TEM_SETTING tem_setting = CTEMControlManager::GetInstance()->get_mag_mode() == MAG_MODE::MAG_MODE_TEM ? PARAM_IMAGING : PARAM_IMAGING_LOWMAG;
	m_pZeissDataCollection->m_oImagingParams.RestoreCurrentParameters(tem_setting);
	static auto pTpx = CTimepix::GetInstance();
	pTpx->m_vTargetBeamCoords.clear();
	pTpx->m_iTargetBeamCoordsIndex = 0;

	CWriteFile::GetInstance()->write_params_file();
}

void CZeissDataCollection_Dialog::OnBnClickedDcSaveDiffBtn()
{
	TEMModeParameters::current_parameters = PARAM_DIFFRACTION;
	m_pZeissDataCollection->m_oDiffractionParams.SaveCurrentParameters();

	CWriteFile::GetInstance()->write_params_file();
}

void CZeissDataCollection_Dialog::OnBnClickedDcLoadDiffBtn()
{
	m_pZeissDataCollection->m_oDiffractionParams.RestoreCurrentParameters(PARAM_DIFFRACTION);
	static auto pTpx = CTimepix::GetInstance();
	static auto pDC = CDataCollection::GetInstance();
	static auto pMscope = CTEMControlManager::GetInstance();

	/*if(pDC->m_bCorrectCalibration == false)
	{
		pMscope->do_calibrate_focus();
		//pMscope->do_calibrate_magnification();
	}*/
	if (pTpx->m_vTargetBeamCoords.empty() == false)
	{
		pMscope->do_blank_beam(true);
		pMscope->set_image_mode(DIFFRACTION_MODE);
		auto pos = pTpx->m_vTargetBeamCoords.at(pTpx->m_iTargetBeamCoordsIndex);
		cv::Point2f dummy;
		pDC->do_beam_shift_at_coordinates_alternative(pos, true);
		printf("Pos (%d) -- ", pTpx->m_iTargetBeamCoordsIndex + 1);
		pDC->do_set_current_beam_screen_coordinates(pos.x, pos.y);
		pMscope->do_blank_beam(false);
	}
	CWriteFile::GetInstance()->write_params_file();
}

void CZeissDataCollection_Dialog::create_required_directories()
{
	CString _usrMsg;
	const auto now = std::chrono::system_clock::now();

	GetDlgItemText(IDC_DC_NUM_OF_FRAMES_TO_TAKE, _usrMsg);
	m_pZeissDataCollection->m_iNumOfFramesToTake = atoi(_usrMsg);

	GetDlgItemText(IDC_DC_CRYSTAL_NAME, _usrMsg);
	m_pZeissDataCollection->m_sCrystalName = _usrMsg;
	m_pZeissDataCollection->m_sCollectionDate = std::format("{:%Y_%m_%d}", now);
	std::string sDateandName = std::format("{}/{}/", m_pZeissDataCollection->m_sCrystalName, m_pZeissDataCollection->m_sCollectionDate);

	GetDlgItemText(IDC_DC_CRYSTAL_ID, _usrMsg);
	m_pZeissDataCollection->m_sCrystalNumberID = std::format("{:03d}", atoi(_usrMsg));
	std::string sCrystalName = m_pZeissDataCollection->m_sCrystalNumberID;

	GetDlgItemText(IDC_DC_CRYSTAL_ID_EXTRA, _usrMsg);
	m_pZeissDataCollection->m_sCrystalNameExtra = std::string(_usrMsg);
	if (m_pZeissDataCollection->m_sCrystalNameExtra.empty() == false)
		sCrystalName += "_" + m_pZeissDataCollection->m_sCrystalNameExtra;
	sCrystalName.append("/");


	GetDlgItemText(IDC_DC_OPERATOR_NAME, _usrMsg);
	m_pZeissDataCollection->m_sOperatorName = _usrMsg;
	std::string sOperatorName = m_pZeissDataCollection->m_sOperatorName;
	sOperatorName.append("/");

	m_pZeissDataCollection->m_sDatasetPath = "C:/Moussa_Software_Data/" + sOperatorName + sDateandName + sCrystalName;
	m_pZeissDataCollection->m_sRawImagePath = "C:/Moussa_Software_Data/" + sOperatorName + sDateandName + "RawImages/";
	m_pZeissDataCollection->m_sBeamCalibrationPath = "C:/Moussa_Software_Data/" + sOperatorName + sDateandName + "BeamCalibration/";
	m_pZeissDataCollection->m_sEucentricHeightPath = m_pZeissDataCollection->m_sDatasetPath + "EucentricHeight/";
	m_pZeissDataCollection->m_sTrackingImgPath = m_pZeissDataCollection->m_sDatasetPath + "CrystalTracking/";
	

	/*CString _usrMsg2;
	_usrMsg2.Format("%.2f", m_pZeissQuickControl->get_stage_tilt_angle());
	SetDlgItemText(IDC_ANG_FROM, _usrMsg2);*/



	// Create the important directories:
	CWriteFile::create_directory(m_pZeissDataCollection->m_sDatasetPath);
	CWriteFile::create_directory(m_pZeissDataCollection->m_sRawImagePath);
	CWriteFile::create_directory(m_pZeissDataCollection->m_sBeamCalibrationPath);
	CWriteFile::create_directory(m_pZeissDataCollection->m_sEucentricHeightPath);
	CWriteFile::create_directory(m_pZeissDataCollection->m_sTrackingImgPath);
}

int items_list[] = {
IDC_DC_RECORD, IDC_DC_TRACK, IDC_DC_COLLECT_FRAMES, IDC_DC_REVIEW_BTN, IDC_ANG_FROM, IDC_ANG_TO,
IDC_DC_QC_SPEED_20, IDC_DC_QC_SPEED_40,IDC_DC_QC_SPEED_50 ,IDC_DC_QC_SPEED_70,
IDC_DC_SAVE_DIFF_BTN, IDC_DC_SAVE_IMGING_BTN, IDC_DC_SAVE_SEARCH_BTN, IDC_DC_CALIB_BEAM,
/*IDC_DC_LOAD_DIFF_BTN, IDC_DC_LOAD_IMGING_BTN, IDC_DC_LOAD_SEARCH_BTN,*/ IDC_DC_QC_TOGLARGESCREEN,
IDC_DC_QC_TOGBLANKING, IDC_DC_QC_TOGPARALLEL, IDC_DC_QC_WRITE_EMISSIONSTEP,
IDC_DC_IMGBASED_RECRD_RDBTN, /*IDC_DC_CONTINUOUS_RECRD_RDBTN,*/ IDC_DC_IMG_BASED_TRCK_RDBTN,
/*IDC_DC_ANGLE_BASED_TRCK_RDBTN, IDC_DC_TIMER_BASED_TRCK_RDBTN, IDC_DC_LINEAR_MOV_CHECKBTN,*/
IDC_DC_IMGBASED_ANGLEBASED_CHCKBTN, IDC_DC_READJUST_Z_CHCKBTN, IDC_DC_MOVEMOUSE_CHCKBTN
};

void CZeissDataCollection_Dialog::disable_items()
{
	for (auto item : items_list)
		GetDlgItem(item)->EnableWindow(FALSE);

}

void CZeissDataCollection_Dialog::enable_items()
{
	for (auto item : items_list)
		GetDlgItem(item)->EnableWindow(TRUE);
	CDataCollection::GetInstance()->m_bEnable_items = false;
}

void CZeissDataCollection_Dialog::enable_interval_items(int* items_list)
{
	static int my_items[] = { IDC_DC_RECORD_IMG_STEPS_VARIABLE, IDC_DC_INTERVAL_TO, IDC_DC_INTERVAL_FROM };
	for (auto item : my_items)
		GetDlgItem(item)->EnableWindow(TRUE);
}

void CZeissDataCollection_Dialog::disable_interval_items(int* items_list)
{
	static int my_items[] = { IDC_DC_RECORD_IMG_STEPS_VARIABLE, IDC_DC_INTERVAL_TO, IDC_DC_INTERVAL_FROM };
	for (auto item : my_items)
		GetDlgItem(item)->EnableWindow(FALSE);
}


void CZeissDataCollection_Dialog::OnBnClickedDcTpxResetchips()
{
	CTimepix::GetInstance()->do_reset_chips();
}


void CZeissDataCollection_Dialog::OnDeltaposDcQcChangeCamlen(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	bool bValueUp = pNMUpDown->iDelta == -1 ? true : false;
	m_pZeissQuickControl->change_actual_camera_length(bValueUp);
	
	*pResult = 0;
}


void CZeissDataCollection_Dialog::OnDeltaposDcQcChangeBrightness(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	bool bValueUp = pNMUpDown->iDelta == -1 ? true : false;
	m_pZeissQuickControl->change_actual_brightness(bValueUp);
	*pResult = 0;
}







void CZeissDataCollection_Dialog::OnBnClickedDcUpdateRecordInfo()
{

	this->UpdateDataCollectionParameters();
	this->create_required_directories();

	std::thread t;
	if(m_pZeissDataCollection->is_on_stem_mode())
		t = std::thread(&CDataCollection::display_images_and_create_tracking_data_stem, m_pZeissDataCollection);
	else
		t = std::thread(&CDataCollection::display_images_and_create_tracking_data_tem, m_pZeissDataCollection);

	t.join(); // t.detach() instead, if window not responding is annoying}
}


void CZeissDataCollection_Dialog::OnBnClickedDcSaveRawImg()
{
	if (m_pTimepix != nullptr)
	{
		create_required_directories();

		// Count number of images (.tiffs) in the folder
		std::string sPath = m_pZeissDataCollection->m_sRawImagePath;
		int iCount = 0;

		for (const auto& entry : std::filesystem::directory_iterator(sPath))
		{
			if (entry.path().extension() == ".tiff")
				iCount++;
		}

		sPath.append(std::format("raw_{}_{:03d}.tiff",m_pZeissDataCollection->m_sCrystalName,iCount));
		m_pTimepix->tcp_grab_single_image_from_detector(sPath);
	}
}

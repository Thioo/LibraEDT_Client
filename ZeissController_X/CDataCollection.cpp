#include "pch.h"
#define M_PI       3.14159265358979323846   // pi
#define _CANVAS_SIZE_ 400
#define _GRID_MAX_LENGTH 2000.0f
#define COLOR_GRIDREGION_UNSCANNED cv::Scalar(0, 255, 0)
#define COLOR_GRIDREGION_SCANNED cv::Scalar(0, 0, 255)



CDataCollection* CDataCollection::m_pZeissDataCollection = nullptr;
std::vector<GridRegions> CDataCollection::gridRegions;


CDataCollection::CDataCollection() : m_pStage(nullptr), m_pZeissControlManager(nullptr), m_pFile(nullptr),
m_fStartingAng(0.f), m_fEndingAng(0.f), m_bAngleBasedTrack(false), m_bImageBasedTrack(false), m_bImageBasedRecord(false),
m_bContinuousRecord(false), m_fRecordSTEMMagnification(4000.0f), m_fWorkingStageTSpeed(70.0f), m_fWorkingStageXYSpeed(50.0f),
m_bKeepThreadRunning(false), m_bLinearMovementTrack(false), m_bMoveMouseTest(false), m_bReadjustZValue(false), m_bSingleRun(false), m_bOnDataCollection(false),
m_bTimeBasedTrack(false), m_bCheckForZHeight(false), m_bWaitAfterRotation(false), m_fEucentricHeightDeltaZ(5.0f), m_fEucentricHeightTiltSteps(10.0f),
m_iImageBasedTrackingMode(MODE_TIME_BASED), m_pTimepix(nullptr), m_fBeamShiftCalibrationX(-999.9f), m_fBeamShiftCalibrationY(-999.9f), m_bBeamShiftCalibrated(false), m_bCanStartCollecting(false), m_bCorrectCalibration(true), m_bIs2DMapVisible(false), m_bSerialEDScanRegions(false), m_bTrackStageMovement(false)


{
	PRINTD("\t\t\t\tCDataCollection::CDataCollection() - Constructor\n");
	static bool bDoOnce = false;
	if (bDoOnce == false)
	{
		bDoOnce = true;
		
		m_pZeissControlManager = CTEMControlManager::GetInstance();
		m_pFile = CWriteFile::GetInstance();
		m_pTimepix = CTimepix::GetInstance();
		m_pStage = m_pZeissControlManager->m_pStage;
		ZM(m_oTrackingTimer);
		ZM(m_oRecordTimer);

		m_fRecordSTEMMagnification = 4000.0f;
		
		m_oImageBasedVec.reserve(50);
		m_oContinuousRecordVec.reserve(300); // TODO: Have to be checked!
		m_oEucentricHeightsFirstImgsVec.reserve(20);
		m_oEucentricHeightsSecondImgsVec.reserve(20);
		m_oFirstRegionVec.reserve(20);
		m_oSecondRegionVec.reserve(20);
		m_oDiffracctionFrames.reserve(1000); 
		m_oCrystalPathCoordinates.reserve(100);
		m_oCrystalPathCoordinatesSwapped.reserve(100);
		m_oCrystalPathCoordinatesSpline.reserve(500);
		m_oCrystalPathCoordinatesSwappedSpline.reserve(500);
		_raw_img_vec.reserve(1000);
		//_raw_collected_frames.reserve(1000);
		m_CorrectSpotPosition.x = STEMRESX / 2;
		m_CorrectSpotPosition.y = STEMRESY / 2;

		m_pZeissControlManager->get_illumination_shift_limits(m_ill_shift_x_lower_limit, m_ill_shift_x_upper_limit, m_ill_shift_y_lower_limit, m_ill_shift_y_upper_limit);
		m_vCurrentBeamPosition.x = m_vCurrentBeamPosition.y = 298.f;

		m_fine_calib_X_coefficients = cv::Mat(6, 1, CV_64F);
		m_fine_calib_Y_coefficients = cv::Mat(6, 1, CV_64F);

		m_StagePosition2D = cv::Point2f(_CANVAS_SIZE_ / 2, _CANVAS_SIZE_ / 2);

		gridRegions.reserve(100);
	}

}

void CDataCollection::acquire_image_for_z_height(std::string& oFileName, unsigned int& iNameIndex, std::vector<ImagesInfo>& _imgVec, ImagesInfo& _imgInfo)
{
	
	if(is_on_stem_mode())
	{
		this->m_pZeissControlManager->set_scanning_speed(4);
		std::this_thread::sleep_for(2s); 
		m_pZeissControlManager->acquire_stem_image(oFileName, iNameIndex, true);
		this->m_pZeissControlManager->freeze_stem_mode(false);
		this->m_pZeissControlManager->set_scanning_speed(1);
	}
	else
	{
		m_pZeissControlManager->acquire_tem_image(oFileName, iNameIndex);
	}
	std::this_thread::sleep_for(2s);

	_imgInfo._sFileName = oFileName;
	_imgInfo._fImageAngle = m_pStage->get_current_tilt_angle();
	_imgInfo._fZHeightVal = m_pStage->get_stage_z();
	_imgVec.push_back(_imgInfo);
	oFileName = oFileName.substr(0, oFileName.length() - 9); // File name correction

}

void CDataCollection::do_fill_eucentric_height_regions(std::vector<ImagesInfo>& _imgVec, std::vector<EucentricHeightRegion>& _regionVec)
{
	EucentricHeightRegion region;
	for (int i = 1; i < _imgVec.size(); i++)
	{
		int j = i - 1;

		region.fGivenZHeight  = _imgVec.at(i)._fZHeightVal;
		region.fRegionStart	  = _imgVec.at(j)._fImageAngle;
		region.fRegionEnd     = _imgVec.at(i)._fImageAngle;
		region.fRegionMidAng  =  region.fRegionStart + (-region.fRegionStart + region.fRegionEnd) / 2; //std::midpoint(region.fRegionStart, region.fRegionEnd);
		region.oFirstImgName  = _imgVec.at(j)._sFileName;
		region.oSecondImgName = _imgVec.at(i)._sFileName;
		region.bRegionValid	  =  false;
		
		_regionVec.push_back(region);
	}
}

CDataCollection::~CDataCollection()
{
	PRINTD("\t\t\t\tCDataCollection::~CDataCollection() - Destructor\n");
	
	SAFE_RELEASE(m_pFile);
	SAFE_RELEASE(m_pTimepix);
}


void CDataCollection::do_find_eucentric_height_regions_ex()
{
	PRINTD("\t\t\t\tCDataCollection::DoFindEucentricHeightsEx()\n");

	std::thread t(&CDataCollection::do_find_eucentric_height, this);
	t.join(); // t.detach() instead, if window not responding is annoying
}

void CDataCollection::do_record_ex()
{
	PRINTD("\t\t\t\tCDataCollection::DoRecordEx()\n");

	if (m_pZeissControlManager->is_stage_rotating() == false)
	{	
		m_bOnRecording = true;

		do_save_data_collection_parameters();
		
		// Rotate to m_fStartingAng, Do Backlash correction (?)
		bool bPositive = is_data_collection_direction_positive();
		this->do_tilt_backlash_correction(m_fStartingAng, bPositive);
		std::this_thread::sleep_for(100ms);

		if(m_bReadjustZValue)
		{
			std::thread t1(&CDataCollection::do_readjust_z_value, this);
			t1.detach();
		} 
		this->do_record_crystal_coordinates();
		//std::thread t2(&CDataCollection::do_record_crystal_coordinates, this);
		//t2.detach(); //t2.join(); // t.detach() instead, if window not responding is annoying
		//printf("do_record_crystal_coordinates thread joined! YOU CAN NOW TRY DETACH\nOptionally, call record_ex on a new thread, and call do_Record_crystal_coords in this new thread.\n");

	}
	else
		PRINT("Stage is currently busy! Try again later when it's idle.");

	m_bEnable_items = true;
	m_bOnRecording = false;
}


void CDataCollection::do_track_ex()
{
	PRINTD("\t\t\t\tCDataCollection::DoTrackEx()\n");

	if (m_bTrackCrystalPath)
	{
		// No longer remember why we're making this call here and not after the recording. Maybe its for the cases where user restarted the program and reloaded the tracking data??
		do_prepare_data_for_tracking();

		m_bOnTracking = true;

		if (m_pZeissControlManager->is_stage_rotating() == false)
		{
			//do_fast_stage_movement_parameters();

			// Rotate to m_fStartingAng, Do Backlash correction (?)
			bool bPositive = is_data_collection_direction_positive();
			this->do_tilt_backlash_correction(m_fStartingAng, bPositive);
			std::this_thread::sleep_for(100ms);

			/*if (this->is_on_stem_mode())
			{
				//m_pStage->stage_go_to_xy(m_vStartingStagePos.x, m_vStartingStagePos.y);

				//DoRestoreDataCollectionParameters();
				if (m_bReadjustZValue)
				{
					std::thread t1(&CDataCollection::do_readjust_z_value, this);
					t1.detach();
				}
			}*/
			std::thread t(&CDataCollection::do_crystal_tracking_correction, this); // This was for the knoevenagel crystal structure "tracking" by defocusing the beam when space is pressed
			this->do_track_crystal_coordinates();
			//std::thread t(&CDataCollection::do_track_crystal_coordinates, this);
			m_bOnTracking = false;
			t.join();

		}
		else
			PRINT("Stage is currently busy! Try again later when it's idle.");
	}
	else
	{
		PRINT("Crystal tracking not checked!");
		m_bCanStartCollecting = true;
	}

	m_bOnTracking = false;
	m_bOnDataCollection = false;
	m_CorrectSpotPosition.x = STEMRESX / 2;
	m_CorrectSpotPosition.y = STEMRESY / 2;
	m_bEnable_items = true;
}


void CDataCollection::do_collect_frames_ex()
{
	PRINTD("\t\t\t\tCDataCollection::DoCollectFramesEx()\n");
	if(m_bOnDataCollection == false)
	{
		m_bOnDataCollection = true;

		if(m_bTrackCrystalPath)
		{
			m_bCanStartCollecting = false;
			std::thread t(&CDataCollection::do_track_ex, this);
			t.detach();
		}
		else
			m_bCanStartCollecting = true;
		
		std::thread t1(&CDataCollection::do_collect_frames, this);
		t1.detach();

		std::thread t2(&CDataCollection::do_live_stream_collected_frames, this);
		t2.detach();

	}
	else
		PRINT("Already in CollectFrames(), maybe start rotating...");
}

void CDataCollection::tcp_do_collect_frames_ex()
{
	PRINTD("\t\t\t\tCDataCollection::DoCollectFramesEx()\n");
	if (m_bOnDataCollection == false)
	{
		m_bOnDataCollection = true;

		if (m_bTrackCrystalPath)
		{
			m_bCanStartCollecting = false;
			std::thread t(&CDataCollection::do_track_ex, this);
			t.detach();
		}
		else
			m_bCanStartCollecting = true;

		std::thread t1(&CDataCollection::tcp_do_collect_frames, this);
		t1.detach();

		//std::thread t2(&CDataCollection::do_live_stream_collected_frames, this);
		//t2.detach();

	}
	else
		PRINT("Already in CollectFrames(), maybe start rotating...");
}

void CDataCollection::infinite_loop_for_monitoring_ex()
{
	PRINTD("\t\t\t\tCDataCollection::DataCollectionMainLoopEx()\n");
	// NO LONGER USED
	return;

}

void CDataCollection::set_control_manager(CTEMControlManager* _pCtrlMgr)
{
	m_pZeissControlManager = _pCtrlMgr;
}

void CDataCollection::Test()
{
	PRINTD("\t\t\t\tCDataCollection::Test()\n");
	
	m_oEucentricHeightsSecondImgsVec.clear();
	m_oEucentricHeightsFirstImgsVec.clear();
	m_oFirstRegionVec.clear();
	m_oSecondRegionVec.clear();
	ImagesInfo imgInf;
	imgInf._fZHeightVal = 0;
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_1stImg_000.tiff";
	m_oEucentricHeightsFirstImgsVec.push_back(imgInf);
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_1stImg_001.tiff";
	m_oEucentricHeightsFirstImgsVec.push_back(imgInf);
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_1stImg_002.tiff";
	m_oEucentricHeightsFirstImgsVec.push_back(imgInf);
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_1stImg_003.tiff";
	m_oEucentricHeightsFirstImgsVec.push_back(imgInf);
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_1stImg_004.tiff";
	m_oEucentricHeightsFirstImgsVec.push_back(imgInf);
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_1stImg_005.tiff";
	m_oEucentricHeightsFirstImgsVec.push_back(imgInf);

	imgInf._fZHeightVal = 5;
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_2ndImg_000.tiff";
	m_oEucentricHeightsSecondImgsVec.push_back(imgInf);
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_2ndImg_001.tiff";
	m_oEucentricHeightsSecondImgsVec.push_back(imgInf);
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_2ndImg_002.tiff";
	m_oEucentricHeightsSecondImgsVec.push_back(imgInf);
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_2ndImg_003.tiff";
	m_oEucentricHeightsSecondImgsVec.push_back(imgInf);
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_2ndImg_004.tiff";
	m_oEucentricHeightsSecondImgsVec.push_back(imgInf);
	imgInf._sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_2ndImg_005.tiff";
	m_oEucentricHeightsSecondImgsVec.push_back(imgInf);



	display_images_and_calculate_z_value();

	//CImageManager* pImgMgr = CImageManager::GetInstance();
	//pImgMgr->DisplayImageEx(&m_oEucentricHeightsFirstImgsVec);
	//pImgMgr->DisplayImageEx(&m_oEucentricHeightsSecondImgsVec);

	//DoFillEucentricHeightRegions(m_oEucentricHeightsFirstImgsVec, m_oFirstRegionVec);
	//DoFillEucentricHeightRegions(m_oEucentricHeightsSecondImgsVec, m_oSecondRegionVec); 
	//DoCalculateZValue(m_oEucentricHeightsFirstImgsVec, m_oEucentricHeightsSecondImgsVec, m_oFirstRegionVec, m_oSecondRegionVec);

	///* For Testing Purposes */
	//int _invalidRegion = 0;
	//for (auto& v : m_oFirstRegionVec)
	//{
	//	if (v.bRegionValid == false)
	//	{
	//		_invalidRegion++;
	//		printf("Invalid Region: Start(%.2f) | End(%.2f)\n", v.fRegionStart, v.fRegionEnd);
	//		continue;
	//	}
	//	printf("Region: (%s) -> (%s) -- CalculatedZValue (%.3f)\n", v.oFirstImgName.c_str(), v.oSecondImgName.c_str(), v.fCalculatedZHeight);
	//}
	//WAIT();


	return;
	std::vector<ImagesInfo> imgFirst, imgSecond;
	std::vector<EucentricHeightRegion> regionFirst, regionSecond;
	
	ImagesInfo img;
	EucentricHeightRegion region;

	img._bIsImgValid = true;
	img._iPosY = 384;
	imgFirst.emplace_back(img);
	img._iPosY = 599;
	imgFirst.emplace_back(img);
	region.fGivenZHeight = 30;
	regionFirst.emplace_back(region);

	img._iPosY = 384;
	imgSecond.emplace_back(img);
	img._iPosY = 533;
	imgSecond.emplace_back(img);
	region.fGivenZHeight = 40;
	regionSecond.emplace_back(region);

	img._bIsImgValid = false;
	imgFirst.emplace_back(img);
	regionFirst.emplace_back(region);
	//imgFirst.push_back(img);
	imgSecond.emplace_back(img);
	regionSecond.emplace_back(region);

//	imgSecond.push_back(img);


	img._bIsImgValid = true;
	img._iPosY = 384;
	imgFirst.emplace_back(img);
	regionFirst.emplace_back(region);
	img._iPosY = 555;
	imgFirst.emplace_back(img);
	region.fGivenZHeight = 60;
	regionFirst.emplace_back(region);

	img._iPosY = 384;
	imgSecond.emplace_back(img);
	regionSecond.emplace_back(region);
	img._iPosY = 359;
	imgSecond.emplace_back(img);
	region.fGivenZHeight = 55;
	regionSecond.emplace_back(region);

	do_calculate_z_value(imgFirst, imgSecond, regionFirst, regionSecond);

}

bool CDataCollection::is_data_collection_direction_positive()
{
	bool bIsPositive = true;
	if (m_fStartingAng > m_fEndingAng)
		bIsPositive = false;
	
	return bIsPositive;
}


void CDataCollection::do_find_eucentric_height()
{
	PRINTD("\t\t\t\tCDataCollection::DoFindEucentricHeights()\n");
	// Z height changes as we go to high tilt angles
	// So I'm trying to find different Z values for different tilt ranges
	// and later on smoothly transition from one to another as we're tilting
	// not sure how it'll work :)


	//PRINT("Now stage will do backlash correction and go to the desired starting angle");
	//WAIT();
	// Back up parameters
	float fOrgTiltSpeed = m_pZeissControlManager->get_stage_tilt_speed();
	float fOrgXYSpeed = m_pZeissControlManager->get_stage_xy_speed();
	m_pZeissControlManager->make_beam_convergent(); // STEM only (internal check)

	// Increase rotation speed
	//m_pZeissControlManager->SetStageTSpeed(50.0f);
	//m_pZeissControlManager->SetStageXYSpeed(95.0f);


	// Go to starting angle
	bool bPositive = is_data_collection_direction_positive();
	do_tilt_backlash_correction(m_fStartingAng, bPositive);
	if (bPositive == false)
		m_fEucentricHeightTiltSteps *= -1;

	m_oEucentricHeightsFirstImgsVec.clear();
	m_oEucentricHeightsSecondImgsVec.clear();
	m_oFirstRegionVec.clear();
	m_oSecondRegionVec.clear();


	if (m_bSingleRun)
		do_find_eucentric_height_single_run();
	else
		do_find_eucentrigh_height_double_run();	
	
	
	
	//m_pZeissControlManager->SetStageTSpeed(fOrgTiltSpeed);
	//m_pZeissControlManager->SetStageXYSpeed(fOrgXYSpeed);
	m_pZeissControlManager->make_beam_parallel();
	display_images_and_calculate_z_value();

}

void CDataCollection::do_find_eucentric_height_single_run()
{
	PRINTD("\t\t\t\tCDataCollection::DoFindEucentricHeightsSingleRun()\n");
	
	//std::string oFileNameFirst = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_1stImg";
	//std::string oFileNameSecond = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_2ndImg";
	std::string oFileNameFirst = m_pZeissDataCollection->m_sEucentricHeightPath + "Z_Img_1st";
	std::string oFileNameSecond = m_pZeissDataCollection->m_sEucentricHeightPath + "Z_Img_2nd";


	unsigned int iNameIndex1st = 0;
	unsigned int iNameIndex2nd = 0;
	ImagesInfo	 oFirstImgRun;
	ImagesInfo	 oSecondImgRun;
	float		 fZValueFirst = m_pStage->get_stage_z();
	float		 fZValueSecond = fZValueFirst + m_fEucentricHeightDeltaZ;

	float fNumOfRotations = fabs(m_fStartingAng - m_fEndingAng) / m_fEucentricHeightTiltSteps;
	
	float fStepCopy = m_fEucentricHeightTiltSteps;
	for (int i = 0; i <= fNumOfRotations; i++)
	{
		if ((fNumOfRotations - i) < 1.0f)
			m_fEucentricHeightTiltSteps = m_fEndingAng - m_pStage->get_current_tilt_angle();


		//	Go to Z Value and Acquire STEM images
		m_pStage->stage_go_to_z(fZValueFirst);
		std::this_thread::sleep_for(1s);
		while (m_pStage->is_stage_busy())
			std::this_thread::sleep_for(500ms);
		// Acquire Image for 1st Z Value
		acquire_image_for_z_height(oFileNameFirst, iNameIndex1st, m_oEucentricHeightsFirstImgsVec, oFirstImgRun);


		m_pStage->stage_go_to_z(fZValueSecond);
		std::this_thread::sleep_for(1s);
		while (m_pStage->is_stage_busy())
			std::this_thread::sleep_for(500ms);
		// Acquire Image for 2nd Z Value
		acquire_image_for_z_height(oFileNameSecond, iNameIndex2nd, m_oEucentricHeightsSecondImgsVec, oSecondImgRun);

		//Then we rotate to the next angle
		if (i < fNumOfRotations)
		{
			m_pStage->stage_rotate_to_delta_angle(m_fEucentricHeightTiltSteps);
			while (m_pStage->is_stage_busy())
				std::this_thread::sleep_for(500ms);
			std::this_thread::sleep_for(1s);
		}

		if (m_bWaitAfterRotation)
		{
			WAIT();
		}
	}
	m_fEucentricHeightTiltSteps = fStepCopy;

}

void CDataCollection::do_find_eucentrigh_height_double_run()
{
	PRINTD("\t\t\t\tCDataCollection::DoFindEucentricHeightsDoubleRun()\n");

	//std::string oFileNameFirst = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_1stImg";
	//std::string oFileNameSecond = "C:/Users/TEM/Documents/Moussa_SoftwareImages/EucentricHeight/EucentricHeight_2ndImg";
	std::string oFileNameFirst = m_pZeissDataCollection->m_sEucentricHeightPath + "Z_Img_1st";
	std::string oFileNameSecond = m_pZeissDataCollection->m_sEucentricHeightPath + "Z_Img_2nd";

	std::string* pFileName = &oFileNameFirst;

	unsigned int iNameIndex1st = 0;
	unsigned int iNameIndex2nd = 0;
	unsigned int* iNameIndexptr = &iNameIndex1st;

	ImagesInfo	 oFirstImgRun;
	ImagesInfo	 oSecondImgRun;
	ImagesInfo* pImgInfo = &oFirstImgRun;
	auto pVec = &m_oEucentricHeightsFirstImgsVec;

	float		 fZValueFirst = m_pStage->get_stage_z();
	float		 fZValueSecond = fZValueFirst + m_fEucentricHeightDeltaZ;
	float*		 fZValueptr = &fZValueFirst;

	float fNumOfRotations = fabs(m_fStartingAng - m_fEndingAng) / m_fEucentricHeightTiltSteps;

	printf("Before Rotating\n");
	WAIT();
	float fStepCopy = m_fEucentricHeightTiltSteps;
	for(int i = 0; i < 2; i++)
	{
		for (int i = 0; i <= fNumOfRotations; i++)
		{
			if ((fNumOfRotations - i) < 1.0f)
				m_fEucentricHeightTiltSteps = m_fEndingAng - m_pStage->get_current_tilt_angle();


			//	Go to Z Value and Acquire STEM images
			m_pStage->stage_go_to_z(*fZValueptr);
			std::this_thread::sleep_for(1s);
			while (m_pStage->is_stage_busy() || m_pStage->is_stage_rotating())
				std::this_thread::sleep_for(500ms);
			acquire_image_for_z_height(*pFileName, *iNameIndexptr, *pVec, *pImgInfo);

			//Then we rotate to the next angle
			if(i < fNumOfRotations)
			{
				m_pStage->stage_rotate_to_delta_angle(m_fEucentricHeightTiltSteps);
				while (m_pStage->is_stage_busy() || m_pStage->is_stage_rotating())
					std::this_thread::sleep_for(500ms);
				std::this_thread::sleep_for(1s);
			}

		}
	
		if(iNameIndexptr == &iNameIndex1st)
		{
			pFileName = &oFileNameSecond;
			iNameIndexptr = &iNameIndex2nd;
			pImgInfo = &oSecondImgRun;
			pVec = &m_oEucentricHeightsSecondImgsVec;
			fZValueptr = &fZValueSecond;
			m_pStage->stage_go_to_z(*fZValueptr);
			do_tilt_backlash_correction(m_fStartingAng, is_data_collection_direction_positive());
			WAIT();
		}
		
	}
	m_fEucentricHeightTiltSteps = fStepCopy;
	WAIT();
}

void CDataCollection::do_calculate_z_value(std::vector<ImagesInfo>& _imgInfoFirst, std::vector<ImagesInfo>& _imgInfoSecond, std::vector<EucentricHeightRegion>& _imgRegionFirst, std::vector<EucentricHeightRegion>& _imgRegionSecond)
{
	PRINTD("\t\t\t\tCDataCollection::DoCalculateZValue()\n");
	if ((_imgRegionFirst.size() != _imgRegionSecond.size()) || (_imgInfoFirst.size() != _imgInfoSecond.size()))
		return;

	for (int i = 1; i < _imgInfoFirst.size(); i++)
	{
		int j = i - 1;
		if (_imgInfoFirst.at(i)._bIsImgValid == false || _imgInfoFirst.at(j)._bIsImgValid == false)
			continue;		
		
		
		int iDeltaYFirst = _imgInfoFirst.at(i)._iPosY - _imgInfoFirst.at(j)._iPosY;
		int iDeltaYSecond = _imgInfoSecond.at(i)._iPosY - _imgInfoSecond.at(j)._iPosY;
		int iDeltaY = iDeltaYSecond - iDeltaYFirst;

		float _fDeltaZ = _imgRegionSecond.at(j).fGivenZHeight - _imgRegionFirst.at(j).fGivenZHeight;
		float fSlope = iDeltaY / _fDeltaZ;
		float fIntercept = iDeltaYFirst - (fSlope * _imgRegionFirst.at(j).fGivenZHeight);

		float fCalculatedZVal = (-fIntercept / fSlope );
		_imgRegionFirst.at(j).fCalculatedZHeight = _imgRegionSecond.at(j).fCalculatedZHeight = fCalculatedZVal;
		_imgRegionFirst.at(j).bRegionValid = _imgRegionSecond.at(j).bRegionValid = true;
		
	}
}


void CDataCollection::display_images_and_calculate_z_value()
{
	PRINTD("\t\t\t\tCDataCollection::ShowImagesAndCalculateZValue()\n");

	if (m_oEucentricHeightsFirstImgsVec.empty() || m_oEucentricHeightsSecondImgsVec.empty())
	{
		PRINT("There is no images to be reviewed!");
		return;
	}

	m_oFirstRegionVec.clear();
	m_oSecondRegionVec.clear();
	CImageManager* pImgMgr = CImageManager::GetInstance();

	ImgInfoPurpose purpose = STEM_TRACKING;
	if (m_pZeissControlManager->is_on_stem_mode() == false)
		purpose = TEM_TRACKING;


	pImgMgr->display_image_ex(&m_oEucentricHeightsFirstImgsVec, purpose);
	pImgMgr->display_image_ex(&m_oEucentricHeightsSecondImgsVec, purpose);

	do_fill_eucentric_height_regions(m_oEucentricHeightsFirstImgsVec, m_oFirstRegionVec);
	do_fill_eucentric_height_regions(m_oEucentricHeightsSecondImgsVec, m_oSecondRegionVec);

	do_calculate_z_value(m_oEucentricHeightsFirstImgsVec, m_oEucentricHeightsSecondImgsVec, m_oFirstRegionVec, m_oSecondRegionVec);

	/* For Testing Purposes */
	int _invalidRegion = 0;
	for (auto& v : m_oFirstRegionVec)
	{
		if (v.bRegionValid == false)
		{
			_invalidRegion++;
			printf("Invalid Region: Start(%.2f) | End(%.2f)\n", v.fRegionStart, v.fRegionEnd);
			continue;
		}
		printf("Region: Start (%.2f) | End (%.2f) -- CalculatedZValue (%.3f)\n", v.fRegionStart, v.fRegionEnd, v.fCalculatedZHeight);
		//auto point = pImgMgr->CalculateImageShiftEx(v.oFirstImgName, v.oSecondImgName);
		//printf("Calculated shift for this region: (%.2f, %.2f)\n", point.x, point.y);
	}
	//WAIT();
}

void CDataCollection::display_images_and_create_tracking_data_tem()
{
	if (m_oImageBasedVec.empty())
	{
		PRINT("There are no images to be updated!");
		return;
	}



	for (auto& imgVec : m_pZeissDataCollection->m_oImageBasedVec)
		imgVec._iPosX = imgVec._iPosY = -1;

	CImageManager* pImgMgr = CImageManager::GetInstance();
	m_oImageBasedVec.at(0)._bSaveVisuals = true; // Save the visuals of the first image
	pImgMgr->display_image_ex(&m_oImageBasedVec, TEM_TRACKING);

	std::string tracking_data_file = m_sTrackingImgPath + "tracking_file.trck";
	CWriteFile::GetInstance()->write_tracking_data(tracking_data_file);
	do_fill_crystal_coordinates_vec();
}

void CDataCollection::display_images_and_create_tracking_data_stem()
{
	if (m_oImageBasedVec.empty())
	{
		PRINT("There are no images to be updated!");
		return;
	}

	for (auto& imgVec : m_pZeissDataCollection->m_oImageBasedVec)
		imgVec._iPosX = imgVec._iPosY = -1;

	CImageManager* pImgMgr = CImageManager::GetInstance();
	m_oImageBasedVec.at(0)._bSaveVisuals = true; // Save the visuals of the first image
	pImgMgr->display_image_ex(&m_oImageBasedVec, STEM_TRACKING);

	std::string tracking_data_file = m_sTrackingImgPath + "tracking_file.trck";
	CWriteFile::GetInstance()->write_tracking_data(tracking_data_file);
	do_fill_crystal_coordinates_vec();
}

double CDataCollection::angleBetweenVectors(cv::Point2d v1, cv::Point2d v2) {
	double dot = v1.dot(v2);   // dot product of vectors
	double mag1 = sqrt(v1.dot(v1));  // magnitude of v1
	double mag2 = sqrt(v2.dot(v2));  // magnitude of v2
	double cosine = dot / (mag1 * mag2);  // cosine of the angle between the vectors
	double angle = acos(cosine); // angle in degrees
	return angle;
}


void CDataCollection::do_calibrate_beam_shift_tem_ex()
{
	PRINTD("\t\t\t\tCDataCollection::do_calibrate_beam_shift_tem_ex()\n");

	std::thread t(&CDataCollection::do_calibrate_beam_shift_tem, this);
	t.join();
}

cv::Point2f CDataCollection::rotate_x_degrees(cv::Point2f toRotate, float x, bool bClockwise)
{
	cv::Point2f vReturn;

	double a11, a12, a21, a22;
	if (bClockwise == false)
	{
		a11 = a22 = cos(x * (M_PI / 180.0));
		a12 = -sin(x * (M_PI / 180.0));
		a21 = -a12;
	}
	else
	{
		a11 = a22 = cos(x * (M_PI / 180.0));
		a12 = sin(x * (M_PI / 180.0));
		a21 = -a12;
	}

	float newX = a11 * toRotate.x + a12 * toRotate.y;
	float newY = a21 * toRotate.x + a22 * toRotate.y;
	
	vReturn.x = newX;
	vReturn.y = newY;
	return vReturn;

}



cv::Point2f CDataCollection::estimate_current_beam_coordinates()
{
	cv::Point2f beamCoords(0,0);

	if (m_bBeamShiftCalibrated == false)
		return beamCoords;

	float x_basis_angle = angleBetweenVectors(m_vBaseX, cv::Point2f(1.0f, 0.0f));
	float y_basis_angle = angleBetweenVectors(m_vBaseY, cv::Point2f(0.0f, 1.0f));
	float fAverage = (x_basis_angle + y_basis_angle) / 2;

	float curr_ill_x = m_pZeissControlManager->get_illumination_shift_x();
	float curr_ill_y = m_pZeissControlManager->get_illumination_shift_y();
	cv::Point2d currPos(curr_ill_x * m_fBeamShiftCalibrationX, curr_ill_y * m_fBeamShiftCalibrationY);
	beamCoords = rotate_x_degrees(currPos);



	printf("Calculated beam coordinates: (%.f, %.f)\n", beamCoords.x, beamCoords.y);
	return beamCoords;


	/*cv::Point2f beamCoords(0,0);

	if (m_bBeamShiftCalibrated == false)
		return beamCoords;


	float curr_ill_x = m_pZeissControlManager->get_illumination_shift_x();
	float curr_ill_y = m_pZeissControlManager->get_illumination_shift_y();
	cv::Point2f targ = cv::Point2f(-curr_ill_x, -curr_ill_y);

	float startingPos_angle_X = angleBetweenVectors(targ, cv::Point2f(1.0f, 0.0f));
	float startingPos_angle_Y = angleBetweenVectors(targ, cv::Point2f(0.0f, 1.0f));

	float vec_len = sqrt(curr_ill_x * curr_ill_x * m_fBeamShiftCalibrationX * m_fBeamShiftCalibrationX + curr_ill_y * curr_ill_y * m_fBeamShiftCalibrationY * m_fBeamShiftCalibrationY);
	beamCoords.x = vec_len * cos(startingPos_angle_X) + 512.0f;
	beamCoords.y = vec_len * cos(startingPos_angle_Y) * -1.0f;

	beamCoords.x += ImagesInfo::_radius;
	beamCoords.y += ImagesInfo::_radius;

	printf("Calculated beam coordinates: (%.f, %.f)\n", beamCoords.x, beamCoords.y);
	return beamCoords;*/
}


void CDataCollection::do_beam_shift_at_coordinates(cv::Point2f& _targetPos, cv::Point2f* pStartingPos, cv::Point2f& _ill_shift_vec_init, cv::Point2f& _ill_shift_vec_final, bool bShiftBeam /*= false*/)
{
	if (m_bBeamShiftCalibrated == false)
		return;

	/*do_beam_shift_at_coordinates_optimized(_targetPos, _ill_shift_vec_final, bShiftBeam);
	return;*/

	// IMPORTANT: The ORIGIN of the cartesian is the initial position of the beam. Keep this in mind when tracking as well
	// In order words, I can't just say that I want to put the beam at coordinate 250, 250. I need to get the vector from the initial position to 250, 250
	// https://www.geogebra.org/m/VWN3g9rE
	// a. Get the position vector.

	float curr_ill_x = m_pZeissControlManager->get_illumination_shift_x();
	float curr_ill_y = m_pZeissControlManager->get_illumination_shift_y();	
	_ill_shift_vec_init.x = curr_ill_x;
	_ill_shift_vec_init.y = curr_ill_y;

	if (pStartingPos == nullptr)
		pStartingPos = &m_vCurrentBeamPosition;

	cv::Point2f newVec = _targetPos - *pStartingPos;
	float newVec_mag = sqrt(newVec.dot(newVec));

	//float cartesian_angles = angleBetweenVectors(m_vBaseX, m_vBaseY);
	float newVec_angles_X = angleBetweenVectors(newVec, m_vBaseX);
	float newVec_angles_Y = angleBetweenVectors(newVec, m_vBaseY); // This approach is safer in case the angles are not exactly 90º even tho they should be.

	float newVecX = newVec_mag * cos(newVec_angles_X);
	//float newVecYz = newVec_mag * sin(newVec_angles_X); // Only works if x_hat and y_hat are orthonormal
	float newVecY = newVec_mag * cos(newVec_angles_Y); // this is safer then

	float toShiftX = curr_ill_x;
	float toShiftY = curr_ill_y;

	toShiftX += newVecX / m_fBeamShiftCalibrationX;
	toShiftY += newVecY / m_fBeamShiftCalibrationY;
	
	_ill_shift_vec_final.x = toShiftX;
	_ill_shift_vec_final.y = toShiftY;

	if (bShiftBeam)
	{
		if (std::_Is_nan(toShiftX) || std::_Is_nan(toShiftY) ||
			std::_Is_nan(_ill_shift_vec_init.x) || std::_Is_nan(_ill_shift_vec_final.y) ||
			std::_Is_nan(_ill_shift_vec_final.x) || std::_Is_nan(_ill_shift_vec_final.y))
			printf("ill_shift returned is_nan\n");
		else
		{
			m_pZeissControlManager->set_illumination_shift(toShiftX, toShiftY);
			m_vCurrentBeamPosition = _targetPos;
		}
	}


	// this problem below is now solved
	
	//if (std::_Is_nan(toShiftX) || std::_Is_nan(toShiftY) ||
	//	std::_Is_nan(_ill_shift_vec_init.x) || std::_Is_nan(_ill_shift_vec_final.y) ||
	//	std::_Is_nan(_ill_shift_vec_final.x) || std::_Is_nan(_ill_shift_vec_final.y))
	//	printf("One of the important parameters returned is_nan");
}


void CDataCollection::do_beam_shift_at_coordinates_optimized(cv::Point2f& _targetPos, cv::Point2f& _ill_shift_vec_final, bool bShiftBeam /*= false*/)
{
	if (m_bBeamShiftCalibrated == false)
		return;


	// IMPORTANT: The ORIGIN of the cartesian is the initial position of the beam. Keep this in mind when tracking as well
	// In other words, I can't just say that I want to put the beam at coordinate 250, 250. I need to get the vector from the initial position to 250, 250
	// https://www.geogebra.org/m/VWN3g9rE
	// a. Get the position vector.


	cv::Point2f newVec = _targetPos - m_oDiffractionParams.Illumination_shift_screen_coords();
	float newVec_mag = sqrt(newVec.dot(newVec));

	//float cartesian_angles = angleBetweenVectors(m_vBaseX, m_vBaseY);
	float newVec_angles_X = angleBetweenVectors(newVec, m_vBaseX);
	float newVec_angles_Y = angleBetweenVectors(newVec, m_vBaseY); // This approach is safer in case the angles are not exactly 90º even tho they should be.

	float newVecX = newVec_mag * cos(newVec_angles_X);
	//float newVecYz = newVec_mag * sin(newVec_angles_X); // Only works if x_hat and y_hat are orthonormal
	float newVecY = newVec_mag * cos(newVec_angles_Y); // this is safer then

	float toShiftX = m_oDiffractionParams.Illumination_shift_vec().x;
	float toShiftY = m_oDiffractionParams.Illumination_shift_vec().y;
	
	toShiftX += newVecX / m_fBeamShiftCalibrationX;
	toShiftY += newVecY / m_fBeamShiftCalibrationY;
	
	_ill_shift_vec_final.x = toShiftX;
	_ill_shift_vec_final.y = toShiftY;

	if (bShiftBeam)
	{
		m_pZeissControlManager->set_illumination_shift(toShiftX, toShiftY);
	}
}

void CDataCollection::do_beam_shift_at_coordinates_alternative(cv::Point2f& _targetPos, bool bShiftBeam /*= false*/, bool bOnUpdate /*= false*/)
{
	//PRINT("TESTING (do_beam_shift_at_coordinates_alternative) - USING NEW METHOD FOR BEAM SHIFT - REPORT IF IT FAILS!");
	cv::Point2f deltaVec = _targetPos - m_oDiffractionParams.Illumination_shift_screen_coords();
	if(deltaVec == cv::Point2f(0,0))
		return;
	float qL = deltaVec.y * m_vBeamShiftAfterX.x - deltaVec.x * m_vBeamShiftAfterX.y;
	float qM = -1 * m_vBeamShiftAfterY.x * m_vBeamShiftAfterX.y + m_vBeamShiftAfterY.y * m_vBeamShiftAfterX.x;
	float q = qL / qM;

	float pL1 = q * m_vBeamShiftAfterY.x;
	float pL = deltaVec.x - pL1;
	float pM = m_vBeamShiftAfterX.x;
	float p = pL / pM;

	// Calculate the shift X and shift Y.
	float shiftX = p * m_fBeamShiftDelta;
	float shiftY = q * m_fBeamShiftDelta;

	shiftX += m_oDiffractionParams.Illumination_shift_vec().x;
	shiftY += m_oDiffractionParams.Illumination_shift_vec().y;

	if (bShiftBeam)
	{
		m_pZeissControlManager->set_illumination_shift(shiftX, shiftY);

		// Update beam screen coordinates
		if (bOnUpdate)
			this->do_set_current_beam_screen_coordinates(_targetPos.x, _targetPos.y, false);
	}

}

void CDataCollection::do_calibrate_beam_shift_tem()
{
	// 1. Take an image of the beam
	// 2. Show it to the user so that he can select the position and the size of the beam
	// 3. shift the beam along the x and repeat step 1 and 2
	// 4. shift the beam along the y and repeat step 1 and 2
	// 5. remember the save the radius/size of the beam.
	// 6. TODO: store the information somewhere in disk, in case the app crashes, so that you don't have to do it all the time.


	std::vector<ImagesInfo>	_beam_calibration_images;
	m_fBeamShiftDelta = m_pZeissDataCollection->m_iCalibrationDeltaGUI / 1000.0f;

	bool bWasOnDiff = false;
	if (m_pZeissControlManager->get_image_mode() == DIFFRACTION_MODE)
	{
		bWasOnDiff = true;
		m_pZeissControlManager->set_image_mode(IMAGE_MODE);
		std::this_thread::sleep_for(1s);
	}

	// Let's get the starting values
	float ill_shift_x = m_pZeissControlManager->get_illumination_shift_x();
	float ill_shift_y = m_pZeissControlManager->get_illumination_shift_y();
	do_set_current_illumination_shift_coordinates(ill_shift_x, ill_shift_y);

	ImagesInfo imgsInfo;
	imgsInfo._sFileName = m_sBeamCalibrationPath + "beam_img";
	unsigned int iNameIndex = 1;

	// We first take an image of the inital beam position:
	m_pZeissControlManager->acquire_tem_image(imgsInfo._sFileName, iNameIndex);
	std::this_thread::sleep_for(500ms);
	_beam_calibration_images.push_back(imgsInfo);
	imgsInfo._sFileName = imgsInfo._sFileName.substr(0, imgsInfo._sFileName.length() - 9);
	
	// We shift along X axis and take an image:
	m_pZeissControlManager->set_illumination_shift_x(ill_shift_x + m_fBeamShiftDelta);
	std::this_thread::sleep_for(500ms);
	m_pZeissControlManager->acquire_tem_image(imgsInfo._sFileName, iNameIndex);
	_beam_calibration_images.push_back(imgsInfo);
	imgsInfo._sFileName = imgsInfo._sFileName.substr(0, imgsInfo._sFileName.length() - 9);



	// we restore the initial X value and shift along the Y and take an image:
	m_pZeissControlManager->set_illumination_shift(ill_shift_x, ill_shift_y + m_fBeamShiftDelta);
	std::this_thread::sleep_for(500ms);
	m_pZeissControlManager->acquire_tem_image(imgsInfo._sFileName, iNameIndex);
	m_pZeissControlManager->set_illumination_shift_y(ill_shift_y);
	_beam_calibration_images.push_back(imgsInfo);
	imgsInfo._sFileName = imgsInfo._sFileName.substr(0, imgsInfo._sFileName.length() - 9);

	/*
	// For testing purposes, remove this later
	imgsInfo._sFileName = "C:/beam_img_001.tiff";
	_beam_calibration_images.push_back(imgsInfo);
	imgsInfo._sFileName = "C:/beam_img_002.tiff";
	_beam_calibration_images.push_back(imgsInfo);
	imgsInfo._sFileName = "C:/beam_img_003.tiff";
	_beam_calibration_images.push_back(imgsInfo);*/



	if (bWasOnDiff == true)
		m_pZeissControlManager->set_image_mode(DIFFRACTION_MODE);
	
	// Now we should display the images taken to the user, so that he can click on the position of the beam
	//ImagesInfo::_radius = 0.0f;
	CImageManager::GetInstance()->display_image_ex(&_beam_calibration_images, TEM_BEAM_CALIB);

	//_beam_calibration_images at 0, 1 and 2 contain the coordinates of the beam at the initial position, X-shifted and Y-shifted respectively.
	// Let's get the calibration values:
	cv::Point2f initialPos = m_vCurrentBeamPosition = _beam_calibration_images.at(0)._center;
	cv::Point2f beamX = _beam_calibration_images.at(1)._center;
	cv::Point2f beamY = _beam_calibration_images.at(2)._center;
	printf("Beam shift coordinates:\n\t1. Initial Pos: (%.f, %.f)\n\t2. X-Shifted: (%.f, %.f)\n\t3. Y-Shifted: (%.f, %.f)\n",
		initialPos.x, initialPos.y, beamX.x, beamX.y, beamY.x, beamY.y);

	m_oDiffractionParams.set_Illumination_shift_screen_coords(m_vCurrentBeamPosition);
	//do_set_current_beam_screen_coordinates(m_vCurrentBeamPosition);
	// Calculate the shift X and shift Y. 
	m_vBaseX = m_vBeamShiftAfterX = beamX - initialPos;
	m_vBaseY = m_vBeamShiftAfterY = beamY - initialPos;

	// Calculate the magnitude/modulus of the vectors.
	float deltaX_mag = sqrt(m_vBaseX.dot(m_vBaseX));
	float deltaY_mag = sqrt(m_vBaseY.dot(m_vBaseY));

	m_fBeamShiftCalibrationX =  deltaX_mag / m_fBeamShiftDelta;
	m_fBeamShiftCalibrationY = deltaY_mag / m_fBeamShiftDelta;

	// Normalize the base vectors
	m_vBaseX = m_vBaseX / deltaX_mag;
	m_vBaseY = m_vBaseY / deltaY_mag;

	m_bBeamShiftCalibrated = true;

	//if (m_bDo_fine_beam_shift_calibration)
	//	this->fine_beam_shift_calibration();
}

void CDataCollection::fine_beam_shift_calibration(int order /*= 4*/)
{
	m_bIs_beam_shift_calibrated_fine = false;
	cv::Point2f startingPos, targetPos, dummy;
	std::vector<ImagesInfo>	fine_beam_calibration_images;
	ImagesInfo imgsInfo;
	imgsInfo._sFileName = m_sBeamCalibrationPath + "beam_img_fine";
	unsigned int iNameIndex = 1;
	int iX = 280;
	int iY = 70;
	float current_ill_shift_x;
	float current_ill_shift_y;
	
	/*m_bIs_beam_shift_calibrated_fine = false;
	cv::Point2f startingPos, targetPos, dummy;
	std::vector<ImagesInfo>	fine_beam_calibration_images;
	ImagesInfo imgsInfo;
	imgsInfo._sFileName = m_sBeamCalibrationPath + "beam_img_fine";
	unsigned int iNameIndex = 1;
	int iCoordMin = 50;
	int iCoordMax = 550;
	float current_ill_shift_x;
	float current_ill_shift_y;

	if (order > 5)
		order = 5;
	else if (order < 2)
		order = 2;

	// Go from top right (516 0) to bottom left (0 516)
	cv::Point2f top_right_, bottom_left;
	float ill_curr_top_right_x, ill_curr_bottom_left_x;
	targetPos.x = iCoordMax;
	targetPos.y = iCoordMin;
	cv::Point2f vInitialBeamScreenCoords = m_vCurrentBeamPosition;

	this->do_beam_shift_at_coordinates(targetPos, nullptr, dummy, top_right_, true);
	ill_curr_top_right_x = top_right_.x;

	// Used later for storing the beam back to this position since we have the screen coords.
	current_ill_shift_x = m_pZeissControlManager->get_illumination_shift_x();
	current_ill_shift_y = m_pZeissControlManager->get_illumination_shift_y();

	targetPos.x = iCoordMin;
	targetPos.y = iCoordMax;
	this->do_beam_shift_at_coordinates(targetPos, nullptr, dummy, bottom_left, false);
	ill_curr_bottom_left_x = bottom_left.x;

	float fDifferenceX = ill_curr_bottom_left_x - ill_curr_top_right_x;
	float fStepsX = fDifferenceX / (order + 1);

	for (int i = 0; i < order + 1; i++)
	{
		m_pZeissControlManager->set_illumination_shift_x(ill_curr_top_right_x + fStepsX * i);
		std::this_thread::sleep_for(500ms);
		m_pZeissControlManager->acquire_tem_image(imgsInfo._sFileName, iNameIndex);
		fine_beam_calibration_images.push_back(imgsInfo);
		imgsInfo._sFileName = imgsInfo._sFileName.substr(0, imgsInfo._sFileName.length() - 9);
	}
	m_pZeissControlManager->set_illumination_shift(current_ill_shift_x, current_ill_shift_y);
	std::this_thread::sleep_for(500ms);

	// go from top left (0 0) to bottom_right (516 516)
	cv::Point2f top_left_, bottom_right_;
	float ill_curr_top_left_y, ill_curr_bottom_right_y;
	targetPos.x = iCoordMin;
	targetPos.y = iCoordMin;

	this->do_beam_shift_at_coordinates(targetPos, nullptr, dummy, top_left_, true);
	ill_curr_top_left_y = top_left_.y;
	// Used later for storing the beam back to this position since we have the screen coords.
	current_ill_shift_x = m_pZeissControlManager->get_illumination_shift_x();
	current_ill_shift_y = m_pZeissControlManager->get_illumination_shift_y();


	targetPos.x = iCoordMax;
	targetPos.y = iCoordMax;
	this->do_beam_shift_at_coordinates(targetPos, nullptr, dummy, bottom_right_, false);
	ill_curr_bottom_right_y = bottom_right_.y;

	float fDifferenceY = ill_curr_bottom_right_y - ill_curr_top_left_y;
	float fStepsY = fDifferenceY / (order + 1);

	for (int i = 0; i < order + 1; i++)
	{
		m_pZeissControlManager->set_illumination_shift_y(ill_curr_top_left_y + fStepsY * i);
		std::this_thread::sleep_for(500ms);
		m_pZeissControlManager->acquire_tem_image(imgsInfo._sFileName, iNameIndex);
		fine_beam_calibration_images.push_back(imgsInfo);
		imgsInfo._sFileName = imgsInfo._sFileName.substr(0, imgsInfo._sFileName.length() - 9);
	}

	m_pZeissControlManager->set_illumination_shift(current_ill_shift_x, current_ill_shift_y);

	CImageManager::GetInstance()->display_image_ex(&fine_beam_calibration_images, TEM_BEAM_CALIB);

	int num_data = fine_beam_calibration_images.size();
	
	int half_data = num_data / 2;
	std::vector<cv::Point2f> v_XData;
	for (int i = 0; i < half_data; i++)
	{
		// Calculate the shift X and shift Y. 
		m_vBaseX = fine_beam_calibration_images.at(i)._center - fine_beam_calibration_images.at(0)._center;

		// Calculate the magnitude/modulus of the vectors.
		float deltaX_mag = sqrt(m_vBaseX.dot(m_vBaseX));

		//Store/link the corresponding X and Y values
		v_XData.emplace_back(cv::Point2f(deltaX_mag, fStepsX * i));

		// Normalize the base vectors
		m_vBaseX = m_vBaseX / deltaX_mag;
	}

	std::vector<cv::Point2f> v_YData;
	for (int i = half_data; i < num_data; i++)
	{
		// Calculate the shift X and shift Y. 
		m_vBaseY = fine_beam_calibration_images.at(i)._center - fine_beam_calibration_images.at(half_data)._center;

		// Calculate the magnitude/modulus of the vectors.
		float deltaY_mag = sqrt(m_vBaseY.dot(m_vBaseY));

		//Store/link the corresponding X and Y values
		v_YData.emplace_back(cv::Point2f(deltaY_mag, fStepsY * (i - half_data)));

		// Normalize the base vectors
		m_vBaseY = m_vBaseY / deltaY_mag;
	}

	m_fine_calib_X_coefficients = this->poly_fit(v_XData, order);
	m_fine_calib_Y_coefficients = this->poly_fit(v_YData, order);
	m_bIs_beam_shift_calibrated_fine = true;*/
}

cv::Mat CDataCollection::poly_fit(std::vector<cv::Point2f>& points, unsigned int order)
{
	// Fit a polynomial to the points
	cv::Mat A(points.size(), order + 1, CV_64F);
	cv::Mat b(points.size(), 1, CV_64F);

	for (int i = 0; i < points.size(); i++)
	{
		double* a_ptr = A.ptr<double>(i);
		a_ptr[0] = 1.0;

		for (int j = 1; j <= order; j++)
			a_ptr[j] = a_ptr[j - 1] * points[i].x;

		b.at<double>(i, 0) = points[i].y;
	}

	cv::Mat coeffs;
	cv::solve(A, b, coeffs, cv::DECOMP_QR);
	return coeffs;
}




float CDataCollection::get_beam_calibration_y()
{
	return m_fBeamShiftCalibrationY;
}

float CDataCollection::get_beam_calibration_x()
{
	return m_fBeamShiftCalibrationX;
}


void CDataCollection::do_record_crystal_coordinates()
{
	PRINTD("\t\t\t\tCDataCollection::DoRecord()\n");
	
	
	m_oImageBasedVec.clear();
	
	// 1. Start rotating (check that the goniometer started rotating)
	while (m_pZeissControlManager->is_stage_rotating() == false && m_bOnRotateRequest == false)
	{
		std::this_thread::sleep_for(5ms);
		if (do_check_for_emergency_exit())
			return;
	}

	if(m_oRecordTimer.isReset())
		m_oRecordTimer.doStart();
	printf("Recording Loop entered after: %d ms\n", m_oRecordTimer.returnElapsed());

	if (this->is_on_stem_mode())
	{
		m_pStage->get_stage_coordinates();
		m_vStartingStagePos.x = m_pStage->fX_um;
		m_vStartingStagePos.y = m_pStage->fY_um; 
		
		m_fRecordSTEMMagnification = m_pZeissControlManager->get_stem_magnification();
		//if (m_bContinuousRecord)
		//	do_continuous_record_stem(oTimer);
		//else if (m_bImageBasedRecord)
		if (m_bImageBasedRecord)
			do_image_based_record_stem();
	}
	else
	{
		if (m_bStepwiseRecord)
			do_image_based_record_tem_steps();
		else
			do_image_based_record_tem();
	}

	static int i = 0;
	std::string sNewFileName = m_pFile->GenerateNewFileName(m_fStartingAng, m_fEndingAng, i);
	m_pFile->WriteToFile(sNewFileName, m_oTimerBasedMap, true);
	i++;

	
	m_bEnable_items = true;
}

bool inRange(float low, float high, float x, bool includelimits = true)
{
	if(includelimits)
		return ((x - high) * (x - low) <= 0);
	else
		return ((x - high) * (x - low) < 0);

}

bool inRange(unsigned int low, unsigned int high, unsigned int x, bool includelimits = true)
{
	/*if (includelimits)
		return ((x - high) * (x - low) <= 0);
	else
		return ((x - high) * (x - low) < 0);
	*/

	if (includelimits)
		return (x >= low && x <= high);
	else
		return (x > low && x < high);

}

void CDataCollection::do_readjust_z_value()
{

	// We Assume that all the regions are valid
	PRINTD("\t\t\t\tCDataCollection::DoReAdjustZValue() -- Starting Thread");

	if (m_oFirstRegionVec.empty() || m_oSecondRegionVec.empty())
	{
		PRINT("To enable this option, you need to try to find the different eucenctric height regions first!");
		return;
	}
	if (m_oFirstRegionVec.size() != m_oSecondRegionVec.size())
	{
		PRINT("Region's do not match!");
		return;
	}

	// TODO: 1st. We trust that region 0 is always valid, but this should be checked and correctly implemented.
	// TODO: 2nd. Implement backlash correction for Z axis	
	std::this_thread::sleep_for(1s);
	m_pStage->stage_go_to_z(m_oFirstRegionVec[0].fCalculatedZHeight); // GoToZ GetLimits Error, don't know why, maybe we Sleep(for a bit)
	
	for(int i = 1; i < m_oFirstRegionVec.size(); i++)
	{
		printf("Entered Region (%d)\n", i - 1);
		auto region = &m_oFirstRegionVec[i - 1];
		auto nextRegion = &m_oFirstRegionVec[i]; 
		if (region->bRegionValid == false || nextRegion->bRegionValid == false)
			continue; // Invalid Region
		
		
		float fCurrAngle = m_pStage->get_current_tilt_angle();
		float fCurrentZVal = m_pStage->get_stage_z();
		while(inRange(region->fRegionStart, region->fRegionMidAng, fCurrAngle))	//From Starting ang to Mid Point - we leave  Z
		{
			std::this_thread::sleep_for(100ms); //
			fCurrAngle = m_pStage->get_current_tilt_angle();
			continue;
		}
			
		if (inRange(region->fRegionMidAng, region->fRegionEnd, fCurrAngle))		// Mid Point to End - we modify Z
		{
			//CLinearMover oMover;
			printf("Entered Region (%d) -- ZVal From (%.2f) To (%.2f)\n", i - 1, region->fCalculatedZHeight, nextRegion->fCalculatedZHeight);

			cv::Point2f currPos(0, fCurrentZVal);
			cv::Point2f targPos(0, nextRegion->fCalculatedZHeight);
			float t2 = region->fRegionEnd;
			float t1 = region->fRegionMidAng;

			double x[2] = { region->fRegionMidAng, region->fRegionEnd };
			double y[2] = { region->fCalculatedZHeight, nextRegion->fCalculatedZHeight };
			Regresion oRegresion(x, y);

			while (true)
			{
				fCurrAngle = m_pStage->get_current_tilt_angle();
				float fNewZ = oRegresion.extrapolate(fCurrAngle);
				m_pStage->stage_go_to_z(fNewZ);
				if (m_pStage->is_stage_rotating() == false || inRange(region->fRegionMidAng, region->fRegionEnd, fCurrAngle, false) == false)
					break;
				std::this_thread::sleep_for(75ms);//105ms
			}

			

			/*oMover.UpdateMoverData(currPos, targPos, fabs(t2 - t1));

			while (oMover.isComplete() == false)
			{
				t2 = m_pStage->GetCurrentTAngle() - region->fRegionMidAng; // like returnElapsed() for angles
				auto point = oMover.update(fabs(t2 - t1));
				float fNewZVal = point.y;
				if (fabs(fNewZVal - m_pStage->GetStageZ()) > 0.01f)
					m_pStage->GoToZ(fNewZVal);
				t1 = t2;

				if (DoCheckForEmergencyExit())
					break;

				std::this_thread::sleep_for(105ms);
			}*/
		}
		
		if (m_pStage->is_stage_rotating() == false)
			break;

				
	} 
	
	PRINTD("\t\t\t\tCDataCollection::DoReAdjustZValue() -- Exiting thread");
}

void CDataCollection::do_continuous_record_stem(CTimer& oTimer)
{
	//PRINT("do_continuous_record_stem no longer used!");
	return;
	PRINTD("\t\t\t\tCDataCollection::DoContinuousRecord()");
	m_oTimerBasedMap.clear();
	m_oAngleBasedMap.clear();
	m_oContinuousRecordVec.clear();

	do
	{
		// 2. Start timer asap and start collecting information
		//_oTimer.doStart();

		m_pStage->get_stage_coordinates();
		int iElapsedTime = oTimer.returnElapsed();

		float fDeltaAngle = fabs(m_fStartingAng - m_pStage->fT);
		float fRotSpeed = fDeltaAngle * 1000.0f / iElapsedTime;
		CVec vStageInfo(m_pStage->fX_um, m_pStage->fY_um, m_pStage->fZ_um, m_pStage->fT, fRotSpeed, iElapsedTime);

		//TimerBased Map:
		unsigned int iTimeToKey = make_time_key(iElapsedTime);
		m_oTimerBasedMap.insert(std::make_pair(iTimeToKey, vStageInfo));

		//AngleBased Map:
		unsigned int iAngleToKey = make_angle_key(m_pStage->fT);
		m_oAngleBasedMap.insert(std::make_pair(iAngleToKey, vStageInfo));

		//Angle to Time map, so we can know how long it takes to go from a given angle to another
		//m_oAngleTimeMap.insert(std::make_pair(iAngleToKey, iElapsedTime));


	} while (fabs(m_pStage->get_current_tilt_angle() - m_fEndingAng) > 0.001f && m_pStage->is_stage_rotating()); //TODO: EDIT

	oTimer.doEnd();
	printf("The whole record session took: %d seconds\n", oTimer.returnTotalElapsed() / 1000);
	oTimer.doReset();

	// Lets copy the acquired information in a vector
	// Keep in mind that we are using an std::map (ORDERED), therefor, the keys are ordered in an ascending order
	// This is not a problem for timer based (because the time increments anyways), but might be for angle based if we record/track from positive to negative (+60 -> -60)
	// I believe this is more efficient for doing linear movement later on, instead of trying to iterate through the maps.
	// Since TimerBasedMap and AngleBasedMap contain the same stageinfo (different keys), it's safer to simply use timerbasedmap
	// and adjust later on if we want to use this for linearmovement on anglebasedmap keys
	for (auto& v : m_oTimerBasedMap)
		m_oContinuousRecordVec.push_back(v.second);
	
}


void CDataCollection::do_image_based_record_stem(/*CTimer& _oTimer*/)
{
	PRINTD("\t\t\t\tCDataCollection::DoImageBasedRecord()");
	// TODO: OPTIMIZE

	ImagesInfo oImgInfo;
	std::string oFileName = this->m_sTrackingImgPath + "tracking";

	unsigned int imgCount = 1;
	float fCurrentAngle = 0.0f;
	float fDefaultRecordStep = m_fRecordImgSteps;
	float fPreviousAngle = m_fStartingAng;
	int iPreviousTime, iCurrentTime;
	iPreviousTime = m_oRecordTimer.returnElapsed();

	float rotation_speed_percentage = m_pZeissControlManager->get_stage_tilt_speed();
	// exponential fit, for image acquisition during backlash, based on rotation speed
	std::chrono::milliseconds sleep_ms_var = 500ms + static_cast<std::chrono::milliseconds>(static_cast<unsigned int>(std::round((1711 - 2) * std::exp(-0.07 * rotation_speed_percentage))));


	// Take Initial Images to correct for the initial jump during rotation start
	//for (int i = 0; i < 2; i++)
	{
		std::this_thread::sleep_for(sleep_ms_var);
		this->do_fill_image_based_vector_stem(oImgInfo, m_oRecordTimer, oFileName, imgCount);
		//this->m_pZeissControlManager->freeze_stem_mode(false);

	}
	//if (m_bDoBlankRotation)
	//{
		//m_pZeissControlManager->simulate_mdf(true, &m_oSearchingParams);
		//std::this_thread::sleep_for(500ms);
	//}

	do
	{
		fCurrentAngle = m_pStage->get_current_tilt_angle();
		iCurrentTime = m_oRecordTimer.returnElapsed();

		if (m_bVariableRecordSteps)
		{
			if (inRange(m_fRecordImgStepStartingAngVariable, m_fRecordImgStepEndingAngVariable, fCurrentAngle))
			{
				m_fRecordImgSteps = m_fRecordImgStepsVariable;
			}
			else
			{
				m_fRecordImgSteps = fDefaultRecordStep;
			}
		}

		//if (fabs(fCurrentAngle - fPreviousAngle) >= m_fRecordImgSteps)
		if (fabs(iCurrentTime - iPreviousTime) >= m_fRecordImgSteps * 1000.0f)
		{
			//while (m_pZeissControlManager->is_stem_frozen() == false)
			//{
			//	this->m_pZeissControlManager->freeze_stem_mode(true);
			//	std::this_thread::sleep_for(100ms);
			//}
			std::this_thread::sleep_for(200ms);
			this->do_fill_image_based_vector_stem(oImgInfo, m_oRecordTimer, oFileName, imgCount);
			

			fPreviousAngle = fCurrentAngle;
			iPreviousTime = iCurrentTime;
		}
		if (do_check_for_emergency_exit() || (iCurrentTime > 1000 && m_pStage->is_stage_busy() == false && m_pStage->is_stage_rotating() == false))
			break;

	} while (fabs(fCurrentAngle - m_fEndingAng) > 0.001f);
	fCurrentAngle = m_pStage->get_current_tilt_angle();

	// Get the last image
	//while (m_pZeissControlManager->is_stem_frozen() == false)
	//{
	//	this->m_pZeissControlManager->freeze_stem_mode(true);
	//	std::this_thread::sleep_for(100ms);
	//}
	//std::this_thread::sleep_for(200ms);
	this->do_fill_image_based_vector_stem(oImgInfo, m_oRecordTimer, oFileName, imgCount);

	m_oRecordTimer.doEnd();
	printf("The whole record session took: %d seconds\n", m_oRecordTimer.returnTotalElapsed() / 1000);
	m_oRecordTimer.doReset();

	m_pZeissControlManager->do_blank_beam(true);
	//if (m_bDoBlankRotation)
		//m_pZeissControlManager->simulate_mdf(false, &m_oSearchingParams);

	bool bDirection = !is_data_collection_direction_positive(); // Negative because we are going back to the starting angle
	do_tilt_backlash_correction(fCurrentAngle, bDirection);
	while (m_pStage->is_stage_busy())
		std::this_thread::sleep_for(500ms);
	float fGoTo = fabs(m_fStartingAng) + 5.0f;
	if (m_fStartingAng < 0.0f)
		fGoTo *= -1;
	m_pStage->stage_rotate_to_angle(fGoTo);

	// Show all the images and allow the user to select the position of interest.
	display_images_and_create_tracking_data_stem();
}

void CDataCollection::do_prepare_data_for_tracking()
{
	
	bool bTrackingCorrections = false;
	m_oTrackingDataVec.clear();
	
	// Make sure that we have images to prepare the data
	if (m_oImageBasedVec.empty())
	{
		PRINT("No images to prepare the data for tracking!");
		return;
	}else if(m_oImageBasedVec[0]._bIsImgValid == false) // No idea why...
	{
		// Lets make sure that the user knows that the first image SHOULD be valid. Again, I don't remember why I have this requirement here.
		PRINT("First image is invalid, cannot prepare the data for tracking!");
		return;
	}


	// The most important part of this loop (and this function) is to prepare the data so that there are no invalid images in the vector.
	// Invalid images are those with posx and posy equal to -1.0f (because the user didn't click on the image, maybe because there was no crystal)
	// So what we do is to ignore those images and connect the previous valid image with the next valid image.
	// A(valid) ---- B(invalid) ---- C(valid) ====> A -------------- C
	for (int i = 0; i < m_oImageBasedVec.size(); i++)
	{
		// Skip if the image is invalid
		if(m_oImageBasedVec[i]._bIsImgValid == false)
			continue;

		TrackingData data;
		data.crystalCoordinates = cv::Point2f(m_oImageBasedVec[i]._iPosX, m_oImageBasedVec[i]._iPosY);
		data.fAngle = m_oImageBasedVec[i]._fImageAngle;
		data.uiTime = m_oImageBasedVec[i]._uiImgTime;
		if (i > 0)
		{
			data.direction_vec = data.crystalCoordinates - cv::Point2f(m_oImageBasedVec[i - 1]._iPosX, m_oImageBasedVec[i - 1]._iPosY);;
			double distance = cv::norm(data.direction_vec);
			data.movementSpeed = 1000.0f * distance / m_oImageBasedVec[i]._uiImgTime; // pixels/sec
	
		}
		else
		{
			data.direction_vec = cv::Point2f(0, 0);
			data.movementSpeed = 0.0f;
		}

		m_oTrackingDataVec.push_back(data);
	}
	
	// I'm curious to see if a spline interpolation would model better the movement of the crystal

	xt::xarray<double> x = xt::zeros<double>({m_oTrackingDataVec.size()});
	xt::xarray<double> y = xt::zeros_like(x);
	xt::xarray<double> t = xt::zeros_like(x);
	
	auto it_x = x.begin();
	auto it_y = y.begin();
	auto it_t = t.begin();

	for (auto& data : m_oTrackingDataVec)
	{
		*it_x++ = static_cast<double>(data.crystalCoordinates.x);
		*it_y++ = static_cast<double>(data.crystalCoordinates.y);
		*it_t++ = static_cast<double>(data.uiTime);
	}

	// We want the spline to be a function of time
	m_oSplineX = this->generate_spline(t, x); // f(t) = x
	m_oSplineY = this->generate_spline(t, y); // f(t) = y

	xt::xarray<double> new_t = xt::linspace<double>(xt::amin(t)[0], xt::amax(t)[0], 500);
	m_oSplineInterpX = this->interpolate_spline(m_oSplineX, new_t);
	m_oSplineInterpY = this->interpolate_spline(m_oSplineY, new_t);

	// Lets construct the coordinates vector as well as the swapped ones
	xt::xarray<float> XYs = xt::transpose(xt::concatenate(xt::xtuple(m_oSplineInterpX, m_oSplineInterpY)).reshape({ 2, -1 }));
	m_oCrystalPathCoordinatesSpline.clear();
	m_oCrystalPathCoordinatesSwappedSpline.clear();
	auto it = XYs.begin();
	while (it != XYs.end())
	{
		m_oCrystalPathCoordinatesSpline.push_back(cv::Point2f(it[0], it[1]));
		m_oCrystalPathCoordinatesSwappedSpline.push_back(cv::Point2f(it[1], it[0]));
		it += 2;
	}



	// This identifies outliers and splits the region into multiple parts to simulate an acceleration and deceleration rather than a constant speed from A to B
	if(bTrackingCorrections) // unusued. 
	{
		// Calculate the average speed
		float totalSpeed = 0.0f;
		for (const auto& data : m_oTrackingDataVec)
		{
			totalSpeed += data.movementSpeed;
		}
		float averageSpeed = totalSpeed / static_cast<float>(m_oTrackingDataVec.size());

		// Identify outliers and split the region
		float threshold = averageSpeed; // You can use averageSpeed or medianSpeed here

		for (int i = 1; i < m_oTrackingDataVec.size(); i++)
		{
			float speed = m_oTrackingDataVec[i].movementSpeed;

			if (speed > threshold)
			{
				// Determine the number of parts to split the region
				int numParts = 2; 

				// Calculate the duration for each part
				unsigned int totalDuration = m_oTrackingDataVec[i].uiTime - m_oTrackingDataVec[i - 1].uiTime;
				unsigned int partDuration = totalDuration / numParts;

				// Calculate the intermediate speeds
				float intermediateSpeed = speed / static_cast<float>(numParts);

				// Split the region into multiple parts
				for (int j = 0; j < numParts; j++)
				{
					TrackingData newData = m_oTrackingDataVec[i - 1];
					newData.movementSpeed = intermediateSpeed;
					newData.uiTime += j * partDuration;

					// Calculate intermediate coordinates based on speed and direction
					float t = static_cast<float>(j) / static_cast<float>(numParts);
					newData.crystalCoordinates = m_oTrackingDataVec[i - 1].crystalCoordinates + t * m_oTrackingDataVec[i - 1].direction_vec;

					m_oTrackingDataVec.insert(m_oTrackingDataVec.begin() + i + j, newData);
				}

				// Adjust the next data point's time to match the last part's end time
				m_oTrackingDataVec[i + numParts].uiTime = m_oTrackingDataVec[i].uiTime;
			}
		}
	}
	
}

// void CDataCollection::do_track_crystal_coordinates_original()
// {
// 	PRINTD("\t\t\t\tCDataCollection::DoTrack()\n");
// 
// 	if (this->is_on_stem_mode())
// 		m_pZeissControlManager->set_stem_magnification(m_fRecordSTEMMagnification);
// 	else
// 	{
// 		std::vector<ImagesInfo>	_initialImgVec;
// 		ImagesInfo imgsInfo;
// 
// 		m_oImagingParams.RestoreCurrentParameters(PARAM_IMAGING);
// 
// 		m_pZeissControlManager->do_blank_beam(false);
// 		std::this_thread::sleep_for(300ms);
// 		std::string imgName = this->m_sTrackingImgPath + "_initial_Img";
// 		unsigned int idx = 0;
// 		m_pZeissControlManager->acquire_tem_image(imgName, idx);
// 		imgsInfo._sFileName = imgName;
// 		imgsInfo._bSaveVisuals = true;
// 		imgsInfo._bIsShowCrystalPath = true;
// 		_initialImgVec.push_back(imgsInfo);
// 
// 		m_oDiffractionParams.RestoreCurrentParameters(PARAM_DIFFRACTION);
// 
// 		if (m_pZeissControlManager->get_image_mode() == DIFFRACTION_MODE)
// 		{
// 			m_pZeissControlManager->set_image_mode(IMAGE_MODE);
// 			std::this_thread::sleep_for(250ms);
// 		}
// 
// 		imgName = this->m_sTrackingImgPath + "_probe_Img";
// 		idx = 0;
// 		m_pZeissControlManager->acquire_tem_image(imgName, idx);
// 		//imgsInfo._sFileName = imgName;
// 		//_initialImgVec.push_back(imgsInfo);
// 
// 		CImageManager::GetInstance()->display_image_ex(&_initialImgVec, TEM_TRACKING);
// 		
// 		m_oDiffractionParams.RestoreCurrentParameters(PARAM_DIFFRACTION);
// 
// 		//m_targetPosNew = cv::Point2f(_initialImgVec.at(1)._iPosX, _initialImgVec.at(1)._iPosY);
// 		m_targetPosNew = cv::Point2f(_initialImgVec.at(0)._iPosX, _initialImgVec.at(0)._iPosY);
// 		cv::Point2f dummy1, dummy2;
// 		do_beam_shift_at_coordinates(m_targetPosNew, nullptr, dummy1, dummy2, true);
// 		//this->do_set_current_beam_screen_coordinates(_initialImgVec.at(0)._iPosX, _initialImgVec.at(0)._iPosY);
// 
// 		cv::Point2f targetPosOld = cv::Point2f(m_oImageBasedVec[0]._iPosX, m_oImageBasedVec[0]._iPosY);
// 		cv::Point shift_offset = m_targetPosNew - targetPosOld;
// 
// 		
// 		do_fill_crystal_coordinates_vec();
// 		for (int i = 0; i < m_oCrystalPathCoordinates.size(); i++)
// 		{
// 			m_oCrystalPathCoordinates[i].x += shift_offset.x;
// 			m_oCrystalPathCoordinates[i].y += shift_offset.y; 
// 			
// 			m_oCrystalPathCoordinatesSwapped[i].y += shift_offset.x;
// 			m_oCrystalPathCoordinatesSwapped[i].x += shift_offset.y;
// 
// 		}
// 	}
// 
// 	m_bCanStartCollecting = true;
// 	// 2. Start rotating (check that the goniometer started rotating)
// 	while (m_pZeissControlManager->is_stage_rotating() == false && m_bOnRotateRequest == false)
// 	{
// 		std::this_thread::sleep_for(5ms);
// 		if(do_check_for_emergency_exit())
// 		{
// 			m_bOnDataCollection = false;
// 			return;
// 		}
// 	}
// 	
// 	if(m_oTrackingTimer.isReset()) // In case the user has started rotating on his own
// 		m_oTrackingTimer.doStart();
// 	printf("Tracking Loop entered after: %d ms\n", m_oTrackingTimer.returnElapsed());
// 
// 	auto t1_time = m_oTrackingTimer.returnElapsed();
// 	auto t1_angle = m_pStage->get_current_tilt_angle();
// 
// 	if(this->is_on_stem_mode())
// 	{
// 		//if (m_bTimeBasedTrack)
// 		//	do_timer_based_continuous_track_stem();
// 		//else if (m_bAngleBasedTrack)
// 		//	do_angle_based_continuous_track_stem();
// 		//else if (m_bImageBasedTrack)
// 		if (m_bImageBasedTrack)
// 			do_image_based_track_stem2(t1_angle, m_iImageBasedTrackingMode);
// 	}
// 	else
// 	{
// 		if (m_bImageBasedTrack)
// 			do_image_based_track_tem(t1_angle, m_iImageBasedTrackingMode);
// 	}
// 
// 	m_oTrackingTimer.doEnd();
// 	printf("The whole tracking session took: %d seconds\n", m_oTrackingTimer.returnTotalElapsed() / 1000);
// 	m_oTrackingTimer.doReset();
// 
// 	m_bEnable_items = true;
// }

void CDataCollection::do_track_crystal_coordinates()
{
	PRINTD("\t\t\t\tCDataCollection::DoTrack()\n");

	{
		std::vector<ImagesInfo>	_initialImgVec;
		ImagesInfo imgsInfo;
		m_pZeissControlManager->do_blank_beam(false);
		std::this_thread::sleep_for(300ms);


		if (m_pZeissControlManager->is_on_stem_mode())
		{
			m_pZeissControlManager->set_stem_magnification(m_fRecordSTEMMagnification);

			std::string imgName = this->m_sTrackingImgPath + "_initial_STEM_Img";
			unsigned int idx = 0;
			m_pZeissControlManager->acquire_stem_image(imgName, idx);
			m_pZeissControlManager->freeze_stem_mode(false);
			imgsInfo._sFileName = imgName;
			imgsInfo._bSaveVisuals = true;
			imgsInfo._bIsShowCrystalPath = true;
			_initialImgVec.push_back(imgsInfo);


			m_pZeissControlManager->set_stem_spot(true);
			CImageManager::GetInstance()->display_image_ex(&_initialImgVec, STEM_TRACKING);
			m_pZeissControlManager->make_beam_parallel();

		}
		else
		{
			m_oImagingParams.RestoreCurrentParameters(PARAM_IMAGING);
			
			std::string imgName = this->m_sTrackingImgPath + "_initial_TEM_Img";
			unsigned int idx = 0;
			m_pZeissControlManager->acquire_tem_image(imgName, idx);
			imgsInfo._sFileName = imgName;
			imgsInfo._bSaveVisuals = true;
			imgsInfo._bIsShowCrystalPath = true;
			_initialImgVec.push_back(imgsInfo);

			m_oDiffractionParams.RestoreCurrentParameters(PARAM_DIFFRACTION);

			if (m_pZeissControlManager->get_image_mode() == DIFFRACTION_MODE)
			{
				m_pZeissControlManager->set_image_mode(IMAGE_MODE);
				std::this_thread::sleep_for(250ms);
			}

			imgName = this->m_sTrackingImgPath + "_probe_Img";
			idx = 0;
			m_pZeissControlManager->acquire_tem_image(imgName, idx);

			//imgsInfo._sFileName = imgName;
			//_initialImgVec.push_back(imgsInfo);

			CImageManager::GetInstance()->display_image_ex(&_initialImgVec, TEM_TRACKING);

			m_oDiffractionParams.RestoreCurrentParameters(PARAM_DIFFRACTION);

			if(m_bCorrectCalibration == false) // In case its true, this will be done in the RestoreCurrentParameters function.
			{
				//m_pZeissControlManager->do_calibrate_focus();
				//m_pZeissControlManager->do_calibrate_magnification();
			}

		}
		

		//m_targetPosNew = cv::Point2f(_initialImgVec.at(1)._iPosX, _initialImgVec.at(1)._iPosY);
		m_targetPosNew = cv::Point2f(_initialImgVec.at(0)._iPosX, _initialImgVec.at(0)._iPosY);
		cv::Point2f dummy1, dummy2;
		if (m_pZeissControlManager->is_on_stem_mode() == false)
			do_beam_shift_at_coordinates_alternative(m_targetPosNew, true);
			//do_beam_shift_at_coordinates(m_targetPosNew, nullptr, dummy1, dummy2, true);
		//this->do_set_current_beam_screen_coordinates(_initialImgVec.at(0)._iPosX, _initialImgVec.at(0)._iPosY);
		else
			m_pZeissControlManager->set_spot_pos(m_targetPosNew.x, m_targetPosNew.y);

		cv::Point2f targetPosOld = cv::Point2f(m_oImageBasedVec[0]._iPosX, m_oImageBasedVec[0]._iPosY);
		cv::Point shift_offset = m_targetPosNew - targetPosOld;


		do_fill_crystal_coordinates_vec();
		for (int i = 0; i < m_oCrystalPathCoordinates.size(); i++)
		{
			m_oCrystalPathCoordinates[i].x += shift_offset.x;
			m_oCrystalPathCoordinates[i].y += shift_offset.y;

			m_oCrystalPathCoordinatesSwapped[i].y += shift_offset.x;
			m_oCrystalPathCoordinatesSwapped[i].x += shift_offset.y;

		}

	}

	m_bCanStartCollecting = true;
	// 2. Start rotating (check that the goniometer started rotating)
	while (m_pZeissControlManager->is_stage_rotating() == false && m_bOnRotateRequest == false)
	{
		std::this_thread::sleep_for(5ms);
		if (do_check_for_emergency_exit())
		{
			m_bOnDataCollection = false;
			return;
		}
	}

	if (m_oTrackingTimer.isReset()) // In case the user has started rotating on his own
		m_oTrackingTimer.doStart();
	printf("Tracking Loop entered after: %d ms\n", m_oTrackingTimer.returnElapsed());

	auto t1_time = m_oTrackingTimer.returnElapsed();
	auto t1_angle = m_pStage->get_current_tilt_angle();

	if (this->is_on_stem_mode())
	{
		//if (m_bTimeBasedTrack)
		//	do_timer_based_continuous_track_stem();
		//else if (m_bAngleBasedTrack)
		//	do_angle_based_continuous_track_stem();
		//else if (m_bImageBasedTrack)
		if (m_bImageBasedTrack)
			do_image_based_track_stem2(t1_angle, m_iImageBasedTrackingMode);
	}
	else
	{
		if (m_bImageBasedTrack)
			do_image_based_track_tem_spline(t1_angle, m_iImageBasedTrackingMode);
			//do_image_based_track_tem(t1_angle, m_iImageBasedTrackingMode);
	}

	m_oTrackingTimer.doEnd();
	printf("The whole tracking session took: %d seconds\n", m_oTrackingTimer.returnTotalElapsed() / 1000);
	m_oTrackingTimer.doReset();

	m_bEnable_items = true;
}


void CDataCollection::do_crystal_tracking_correction()
{
	auto pTpx = CTimepix::GetInstance();

	bool bToggleFocus = false;
	float orig_Focus = 0.0f;
	float orig_Contrast = pTpx->m_fContrastDiff;
	int orig_mis = 0;
	

	while (m_bOnTracking)
	{
		if (GetAsyncKeyState(VK_SPACE) & 0x1)
		{
			bToggleFocus = !bToggleFocus;

			if (bToggleFocus)
			{
				orig_Focus = m_pZeissControlManager->get_focus();
				orig_mis = m_pZeissControlManager->get_mis_num();

				// Change exposure time(?) and contrast
				m_pDlg->SetDlgItemTextA(IDC_DC_GAIN, "1.0");
				
				// Defocus
				m_pZeissControlManager->set_focus(0.1f);

				// Use a slightly bigger condenser aperture hole (?)
				m_pZeissControlManager->set_mis_num(4);
			}
			else
			{
				// Restore the original diffraction contrast
				m_pDlg->SetDlgItemTextA(IDC_DC_GAIN, std::to_string(orig_Contrast).c_str());
				
				// Restore the original focus value
				m_pZeissControlManager->set_focus(orig_Focus);

				// Restore the original mis aperture
				m_pZeissControlManager->set_mis_num(orig_mis);
			}
		}
		std::this_thread::sleep_for(5ms);
	}
}

void CDataCollection::do_image_based_record_tem(/*CTimer& _oTimer*/)
{
	PRINTD("\t\t\t\tCDataCollection::DoImageBasedRecord()");
	// TODO: OPTIMIZE

	ImagesInfo oImgInfo;
	std::string oFileName = this->m_sTrackingImgPath + "tracking";

	unsigned int imgCount = 1;	
	float fCurrentAngle = 0.0f;
	float fDefaultRecordStep = m_fRecordImgSteps;
	float fPreviousAngle = m_fStartingAng;
	int iPreviousTime, iCurrentTime;
	iPreviousTime = m_oRecordTimer.returnElapsed();

	float rotation_speed_percentage = m_pZeissControlManager->get_stage_tilt_speed();
	// exponential fit, for image acquisition during backlash, based on rotation speed
	std::chrono::milliseconds sleep_ms_var = 500ms + static_cast<std::chrono::milliseconds>(static_cast<unsigned int>(std::round((1711 - 2 ) * std::exp(-0.07 * rotation_speed_percentage)))); 


	// Take Initial Images to correct for the initial jump during rotation start
	for(int i = 0; i < 2; i++)
	{
		std::this_thread::sleep_for(sleep_ms_var);
		this->do_fill_image_based_vector_tem(oImgInfo, m_oRecordTimer, oFileName, imgCount);
	}
	if(m_bDoBlankRotation)
	{
		m_pZeissControlManager->simulate_mdf(true, &m_oSearchingParams);
		std::this_thread::sleep_for(500ms);
	}

	do
	{
		fCurrentAngle = m_pStage->get_current_tilt_angle();
		iCurrentTime = m_oRecordTimer.returnElapsed();

		if (m_bVariableRecordSteps)
		{
			if (inRange(m_fRecordImgStepStartingAngVariable, m_fRecordImgStepEndingAngVariable, fCurrentAngle))
			{
				m_fRecordImgSteps = m_fRecordImgStepsVariable;
			}
			else
			{
				m_fRecordImgSteps = fDefaultRecordStep;
			}
		}

		//if (fabs(fCurrentAngle - fPreviousAngle) >= m_fRecordImgSteps)
		if (fabs(iCurrentTime - iPreviousTime) >= m_fRecordImgSteps * 1000.0f)
		{
			this->do_fill_image_based_vector_tem(oImgInfo, m_oRecordTimer, oFileName, imgCount, m_bDoBlankRotation);

			fPreviousAngle = fCurrentAngle;
			iPreviousTime = iCurrentTime;
		}
		if (do_check_for_emergency_exit() || (iCurrentTime > 1000 && m_pStage->is_stage_busy() == false && m_pStage->is_stage_rotating() == false))
			break;

	} while (fabs(fCurrentAngle - m_fEndingAng) > 0.001f);
	fCurrentAngle = m_pStage->get_current_tilt_angle();
	
// Get the last image
	this->do_fill_image_based_vector_tem(oImgInfo, m_oRecordTimer, oFileName, imgCount, m_bDoBlankRotation);

	m_oRecordTimer.doEnd();
	printf("The whole record session took: %d seconds\n", m_oRecordTimer.returnTotalElapsed() / 1000);
	m_oRecordTimer.doReset();

	m_pZeissControlManager->do_blank_beam(true);
	if(m_bDoBlankRotation)
		m_pZeissControlManager->simulate_mdf(false, &m_oSearchingParams);


	bool bDirection = !is_data_collection_direction_positive(); // Negative because we are going back to the starting angle
	do_tilt_backlash_correction(fCurrentAngle, bDirection);
	while(m_pStage->is_stage_busy())
		std::this_thread::sleep_for(500ms);
	float fGoTo = fabs(m_fStartingAng) + 5.0f;
	if (m_fStartingAng < 0.0f)
		fGoTo *= -1;
	m_pStage->stage_rotate_to_angle(fGoTo);

	// Show all the images and allow the user to select the position of interest.
	display_images_and_create_tracking_data_tem();

}



void CDataCollection::do_image_based_record_tem_steps()
{
	PRINTD("\t\t\t\tCDataCollection::DoImageBasedRecord()");
	// TODO: OPTIMIZE

	ImagesInfo oImgInfo;
	std::string oFileName = this->m_sTrackingImgPath + "tracking";

	unsigned int imgCount = 1;
	float fCurrentAngle = 0.0f;
	float fDefaultRecordStep = m_fRecordImgSteps;
	int iPreviousTime, iCurrentTime;
	iPreviousTime = m_oRecordTimer.returnElapsed();

	
	if (m_bDoBlankRotation)
	{
		m_pZeissControlManager->simulate_mdf(true, &m_oSearchingParams);
		std::this_thread::sleep_for(100ms);
	}

	do
	{
		if (m_bVariableRecordSteps)
		{
			if (inRange(m_fRecordImgStepStartingAngVariable, m_fRecordImgStepEndingAngVariable, fCurrentAngle))
			{
				m_fRecordImgSteps = m_fRecordImgStepsVariable;

			}
			else
			{
				m_fRecordImgSteps = fDefaultRecordStep;
			}
		}
		
		if (m_fStartingAng > m_fEndingAng)
			m_fRecordImgSteps *= -1.0f;
		m_oRecordTimer.doResume();
		m_pStage->stage_rotate_to_delta_angle(m_fRecordImgSteps);
		std::this_thread::sleep_for(500ms);
		while (m_pStage->is_stage_rotating())
			std::this_thread::sleep_for(10ms);
		m_oRecordTimer.doPause();
		std::this_thread::sleep_for(2000ms);
		fCurrentAngle = m_pStage->get_current_tilt_angle();


		this->do_fill_image_based_vector_tem(oImgInfo, m_oRecordTimer, oFileName, imgCount, m_bDoBlankRotation);
		

		if (do_check_for_emergency_exit())
			break;
	} while (fabs(fCurrentAngle - m_fEndingAng) > 0.01f);
	//;

	//fCurrentAngle = m_pStage->get_current_tilt_angle();
	//if (fabs(fCurrentAngle - m_fEndingAng) > 0.5f) // Get the last image
	//{
	//	this->do_fill_image_based_vector_tem(oImgInfo, m_oRecordTimer, oFileName, imgCount, m_bDoBlankRotation);
	//}

	m_oRecordTimer.doEnd();
	printf("The whole record session took: %d seconds\n", m_oRecordTimer.returnTotalElapsed() / 1000);
	m_oRecordTimer.doReset();


	std::this_thread::sleep_for(500ms);
	float fGoTo = fabs(m_fStartingAng) + 5.0f;
	if (m_fStartingAng < 0.0f)
		fGoTo *= -1;
	m_pStage->stage_rotate_to_angle(fGoTo);

	// Show all the images and allow the user to select the position of interest.
	CImageManager* pImgMgr = CImageManager::GetInstance();
	pImgMgr->display_image_ex(&m_oImageBasedVec, TEM_TRACKING);
}

void CDataCollection::do_fill_image_based_vector_tem(ImagesInfo& oImgInfo, CTimer& oTimer, std::string& sFileName, unsigned int& imgCount, bool bDoBlank /*= false*/)
{
	if(bDoBlank)
	{
		m_pZeissControlManager->simulate_mdf(false, &m_oSearchingParams);
		//m_pZeissControlManager->do_blank_beam(false);
		//while (m_pZeissControlManager->get_beam_state() == 1)
		//	std::this_thread::sleep_for(5ms);
		std::this_thread::sleep_for(500ms);
	}
	// It is up to the caller to make sure that the m_oImageBaseVec is cleared.
	this->m_pZeissControlManager->acquire_tem_image(sFileName, imgCount);
	oImgInfo._uiImgTime = oTimer.returnElapsed();
	
	if(bDoBlank)
		m_pZeissControlManager->simulate_mdf(true, &m_oSearchingParams);

	oImgInfo._bIsLowMagImg = static_cast<bool>(m_pZeissControlManager->get_mag_mode());
	oImgInfo._fTEMMagnification = static_cast<float>(std::round(m_pZeissControlManager->get_tem_magnification()));
	oImgInfo._fZHeightVal = static_cast<float>(std::round(m_pZeissControlManager->get_stage_z()));
	oImgInfo._sFileName = sFileName;
	oImgInfo._fImageAngle = m_pStage->get_current_tilt_angle();
	m_oImageBasedVec.push_back(oImgInfo);
	sFileName = sFileName.substr(0, sFileName.length() - 9);
	
	//if(bDoBlank)
	//	m_pZeissControlManager->do_blank_beam(true);

}

void CDataCollection::do_fill_image_based_vector_stem(ImagesInfo& oImgInfo, CTimer& oTimer, std::string& sFileName, unsigned int& imgCount)
{
	while (m_pZeissControlManager->is_stem_frozen() == false)
	{
		this->m_pZeissControlManager->freeze_stem_mode(true);
		std::this_thread::sleep_for(50ms);
	}
	this->m_pZeissControlManager->acquire_stem_image(sFileName, imgCount);
	oImgInfo._uiImgTime = oTimer.returnElapsed(); // before the image acquisition or after ?

	// This block seems to be a better alternative to the m_pZeissControlManager->freeze_stem_mode(false);
	// I used to get tones of black images for some reason...
	m_pZeissControlManager->set_stem_spot(true);
	std::this_thread::sleep_for(50ms);
	m_pZeissControlManager->set_stem_spot(false);
	// Turning spot on and off seems to correct the issue.

	oImgInfo._sFileName = sFileName;
	oImgInfo._fImageAngle = m_pStage->get_current_tilt_angle();
	m_oImageBasedVec.push_back(oImgInfo);
	sFileName = sFileName.substr(0, sFileName.length() - 9);
}

void CDataCollection::do_fill_crystal_coordinates_vec()
{
	m_oCrystalPathCoordinates.clear();
	m_oCrystalPathCoordinatesSwapped.clear();
	for (int i = 0; i < m_oImageBasedVec.size(); i++)
	{
		if (m_oImageBasedVec[i]._bIsImgValid == false)
			continue;

		m_oCrystalPathCoordinates.emplace_back(cv::Point(m_oImageBasedVec[i]._iPosX, m_oImageBasedVec[i]._iPosY));
		m_oCrystalPathCoordinatesSwapped.emplace_back(cv::Point(m_oImageBasedVec[i]._iPosY, m_oImageBasedVec[i]._iPosX)); // X and Y are swapped

	}
}

void CDataCollection::do_angle_based_continuous_track_stem()
{
	//PRINT("do_angle_based_continuous_track_stem no longer used!");
	return;
	PRINTD("\t\t\t\tCDataCollection::DoAngleBasedContinuousTrack()");
	if (m_oAngleBasedMap.empty())
	{
		PRINT("Angle Based Tracking undoable. No data recorded!");
		return;
	}

	if (m_bLinearMovementTrack)
		do_linear_movement_for_continuous_rec_stem(MODE_ANGLE_BASED);
	else
	{
		m_pStage->get_stage_coordinates();
		float fOffsetX = m_pStage->fX_um - m_vStartingStagePos.x;
		float fOffsetY = m_pStage->fY_um - m_vStartingStagePos.y;
		//float fOffsetZ = m_pStage->fZ_um - m_oAngleBasedMap.at(0).fZ;

		do
		{
			unsigned int iAngleKey = make_angle_key(m_pStage->fT);
			if (m_oAngleBasedMap.contains(iAngleKey))
			{
				CVec vStage = m_oAngleBasedMap.at(iAngleKey);
				m_pStage->fX_um = vStage.fX; m_pStage->fY_um = vStage.fY; m_pStage->fZ_um = vStage.fZ;

				m_pStage->stage_go_to_x(vStage.fX + fOffsetX); m_pStage->stage_go_to_y(vStage.fY - fOffsetY);
				std::this_thread::sleep_for(5ms);

			}

		} while (fabs(m_pStage->get_current_tilt_angle() - m_fEndingAng) > 0.001f && m_pStage->is_stage_rotating());
	}
}


void CDataCollection::do_timer_based_continuous_track_stem()
{
	//PRINT("do_timer_based_continuous_track_stem no longer used!");
	return;
	PRINTD("\t\t\t\tCDataCollection::DoTimeBasedContinuousTrack()");
	if (m_oTimerBasedMap.empty())
	{
		PRINT("Timer Based Tracking undoable. No data recorded!");
		return;
	}
	
	if (m_bLinearMovementTrack)
		do_linear_movement_for_continuous_rec_stem(MODE_TIME_BASED); 
	else
	{
		m_pStage->get_stage_coordinates();
		float fOffsetX = m_pStage->fX_um - m_oTimerBasedMap.at(0).fX;
		float fOffsetY = m_pStage->fY_um - m_oTimerBasedMap.at(0).fY;
		float fOffsetZ = m_pStage->fZ_um - m_oTimerBasedMap.at(0).fZ;

		do
		{
			int iTimeToKey = make_time_key(m_oTrackingTimer.returnElapsed());
			if (m_oTimerBasedMap.contains(iTimeToKey))
			{
				CVec vStage = m_oTimerBasedMap.at(iTimeToKey);
				//CVec vStage(m_oTimerBasedMap.at(iTimeToKey));
				m_pStage->fX_um = vStage.fX; m_pStage->fY_um = vStage.fY; m_pStage->fZ_um = vStage.fZ;

				m_pStage->stage_go_to_x(vStage.fX + fOffsetX); m_pStage->stage_go_to_y(vStage.fY - fOffsetY);
				std::this_thread::sleep_for(5ms);

			}

		} while (fabs(m_pStage->get_current_tilt_angle() - m_fEndingAng) > 0.001f && m_pStage->is_stage_rotating());
	}
}


void CDataCollection::do_linear_movement_for_continuous_rec_stem(TrackingMode _Mode)
{
	PRINTD("\t\t\t\tCDataCollection::DoLinearMovementForContinuousRec()");

	// TODO: Clean this part of the code, the two possible modes can be made into a single function.

	CLinearMover oMover;

	for (int i = 1; i < m_oContinuousRecordVec.size(); i++)
	{
		if (_Mode == MODE_TIME_BASED) //Timer based
		{
			unsigned int t1 = m_oContinuousRecordVec.at(i - 1).iTime;
			unsigned int t2 = m_oContinuousRecordVec.at(i).iTime; // m_oTimer.returnElapsed();
			unsigned int iKey = make_time_key(t2);


			cv::Point2f currPos(m_oContinuousRecordVec.at(i - 1).fX, m_oContinuousRecordVec.at(i - 1).fY);
			cv::Point2f targPos(m_oContinuousRecordVec.at(i).fX, m_oContinuousRecordVec.at(i).fY);
			auto originalpoints = oMover.UpdateMoverData(currPos, targPos, t2 - t1);
			bool bCorrectOnce = false;
			float fOffsetX = 0.0f;
			float fOffsetY = 0.0f;
			while (oMover.isComplete() == false)
			{
				if(m_CorrectSpotPosition.x != 512 || m_CorrectSpotPosition.y != 318)
				{
					if (bCorrectOnce == false)
					{
						fOffsetX = originalpoints.at(0).x - m_CorrectSpotPosition.x;
						fOffsetY = originalpoints.at(0).y - m_CorrectSpotPosition.y;
					}
				}
				/*
					static float correctionX = data.p1.x - correction->x;
					static float correctionY = data.p1.y - correction->y;

				*/
				// check if stage is moving first?
				t2 = m_oTrackingTimer.returnElapsed();
				auto point = oMover.update(t2 - t1, fOffsetX, fOffsetY);
				cv::Point2f currPoint(m_pStage->get_stage_x(), m_pStage->get_stage_y());
				if(cv::norm(currPoint - point) > 0.5f)
					m_pStage->stage_go_to_xy(point.x, point.y);
				t1 = t2;

				if (do_check_for_emergency_exit())
					break;

				std::this_thread::sleep_for(105ms);
			}

		}
		else if (_Mode == MODE_ANGLE_BASED)// Angle based
		{
			float t1 = m_oContinuousRecordVec.at(i - 1).fT;
			float t2 = m_oContinuousRecordVec.at(i).fT; //m_pStage->GetCurrentTAngle();
			unsigned int iKey = make_angle_key(t2);

			cv::Point2f currPos(m_oContinuousRecordVec.at(i - 1).fX, m_oContinuousRecordVec.at(i - 1).fY);
			cv::Point2f targPos(m_oContinuousRecordVec.at(i).fX, m_oContinuousRecordVec.at(i).fY);
			oMover.UpdateMoverData(currPos, targPos, fabs(t2 - t1));
			while (oMover.isComplete() == false)
			{
				// check if stage is moving first?
				//t2 = m_pStage->GetCurrentTAngle();
				t2 = m_pStage->get_current_tilt_angle() - m_oContinuousRecordVec.at(i - 1).fT;
				auto point = oMover.update(fabs(t2 - t1));
				cv::Point2f currPoint(m_pStage->get_stage_x(), m_pStage->get_stage_y());

				if (cv::norm(currPoint - point) > 0.5f) //Length
					m_pStage->stage_go_to_xy(point.x, point.y);
				t1 = t2;

				if (do_check_for_emergency_exit())
					break;
				std::this_thread::sleep_for(105ms);
			}

		}
	
	}
}

void CDataCollection::do_image_based_track_tem(float& _t1_angle, TrackingMode _Mode)
{
	PRINTD("\t\t\t\tCDataCollection::DoImageBasedTrack()");
	PRINT("REMOVE LATER: TRY THE SPINE INTERPOLATION METHOD INSTEAD OF THIS LINEAR ONE");

	if (m_oTrackingDataVec.empty())
		return;

	m_vShiftOffset = m_targetPosNew - m_oTrackingDataVec[0].crystalCoordinates;

	// We loop through the tracking data and move the beam to the target position
	for (int i = 1; i < m_oTrackingDataVec.size(); i++)
	{
		auto now = m_oTrackingTimer.returnElapsed();
		auto vCurrPos = m_oTrackingDataVec[i - 1].crystalCoordinates;
		auto vTargPos = m_oTrackingDataVec[i].crystalCoordinates;

		// When the target position is the same as the current position, we skip the iteration and sleep for the appropriate time
		if(vTargPos == vCurrPos) // fixes the nan error
		{
			std::chrono::milliseconds duration(m_oTrackingDataVec[i].uiTime - m_oTrackingDataVec[i - 1].uiTime);
			std::this_thread::sleep_for(duration);
			continue;
		}

		// We correct for the shift offset
		cv::Point2f _startPos, _targPos;
		vCurrPos += m_vShiftOffset;
		vTargPos += m_vShiftOffset;

		// We convert from screen coordinates to beam current.
		this->do_beam_shift_at_coordinates(vTargPos, nullptr, _startPos, _targPos);
		
		//CTimer timerDebug;
		if (_Mode == MODE_ANGLE_BASED)
		{
			float fStartAng = _t1_angle;
			float fMovementDuration = fabs(m_oTrackingDataVec[i].fAngle - m_oTrackingDataVec[i-1].fAngle);
			
			move_to_point_angle_based(_startPos, _targPos, fMovementDuration);
			/*while (oMover.isComplete() == false)
			{
				//auto t2 = m_pStage->GetCurrentTAngle(); TODO: COMPARE OLD
				auto t2 = m_pStage->get_current_tilt_angle() - fStartAng; // NEW
				auto point = oMover.update(t2 - _t1_angle);

				m_pZeissControlManager->set_illumination_shift(point.x, point.y);
				_t1_angle = t2;
				if (do_check_for_emergency_exit() || m_pStage->is_stage_busy() == false)
					break;

				std::this_thread::sleep_for(25ms);
			}*/
		}
		else
		{

			// We calculate the duration of the movement
			float fMovementDuration = static_cast<float>(m_oTrackingDataVec[i].uiTime) - m_oTrackingDataVec[i - 1].uiTime;
			
			// We check if the current time is within the range of the target time
			if (inRange(m_oTrackingDataVec[i - 1].uiTime, m_oTrackingDataVec[i].uiTime, now, true) == false)
			{
				// If the current time is greater than the target time, we skip the iteration
				if (now > m_oTrackingDataVec[i].uiTime)
					continue;
				// If however the current time is less than the target time, it means that we are ahead of the schedule, so we wait before moving on
				else if (now < m_oTrackingDataVec[i-1].uiTime) 
				{
					PRINT("Error@inRange - timer_now < vTargetPos.timer");
					std::chrono::milliseconds duration(m_oTrackingDataVec[i - 1].uiTime - now);
					std::this_thread::sleep_for(duration);
				}
			}

			//timerDebug.doStart();
			// For each iteration, we know the starting pos, ending pos, and duration, we can smoothly move the beam to the target position
			bool bRet = move_to_point_time_based_tem(vCurrPos, vTargPos, fMovementDuration, m_oTrackingDataVec[i - 1].uiTime, now);
			if (bRet == false)
			{
				PRINT("bRet == FALSE");
				break;
			}
			//timerDebug.doEnd();
			
		}
		//printf("Tracking Region(%d): record(%d)\ttracking(%d)\n", i, m_oTrackingDataVec[i].uiTime - m_oTrackingDataVec[i - 1].uiTime, timerDebug.returnElapsed());
	}
}

void CDataCollection::do_image_based_track_tem_spline(float& _t1_angle, TrackingMode _Mode)
{
	PRINTD("\t\t\t\tCDataCollection::DoImageBasedTrack()");
	if (m_oTrackingDataVec.empty()) // We no longer need this check here for the spline interpolation method. But we keep it here for now, just in case...
		return;

	while(true)
	{
		m_vShiftOffset = m_targetPosNew - m_oTrackingDataVec[0].crystalCoordinates;
		int iTimerOffset = 0; // Check how we can account for the offset during recording and tracking.

		xt::xarray<double> xnow = xt::xarray<double>(m_oTrackingTimer.returnElapsed() + iTimerOffset);

		// Lets get the interpolated X and Y values. Note that these are in screen coordinates.
		double interpX = this->interpolate_spline(m_oSplineX, xnow)[0];
		double interpY = this->interpolate_spline(m_oSplineY, xnow)[0];

		// Lets convert them to a cv::Point2f vector and add the shift offset.
		cv::Point2f vInterpPos = cv::Point2f(interpX, interpY) + m_vShiftOffset;

		// We calculate and shift the beam to the target position
		this->do_beam_shift_at_coordinates_alternative(vInterpPos, true);

		std::this_thread::sleep_for(5ms);
		if(static_cast<unsigned int>(xnow(0)) > m_oTrackingDataVec.back().uiTime)
			break;
		if (do_check_for_emergency_exit() || (static_cast<unsigned int>(xnow(0)) > 5000 && m_pStage->is_stage_busy() == false))
		{
			PRINT("BREAKING HERE");
			if (m_pStage->is_stage_busy() == false)
				PRINT("STAGE IS NOT BUSY...");
			break;
		}

	}






	/*// We loop through the tracking data and move the beam to the target position
	for (int i = 1; i < m_oTrackingDataVec.size(); i++)
	{
		auto now = m_oTrackingTimer.returnElapsed();
		auto vCurrPos = m_oTrackingDataVec[i - 1].crystalCoordinates;
		auto vTargPos = m_oTrackingDataVec[i].crystalCoordinates;

		// When the target position is the same as the current position, we skip the iteration and sleep for the appropriate time
		if (vTargPos == vCurrPos) // fixes the nan error
		{
			std::chrono::milliseconds duration(m_oTrackingDataVec[i].uiTime - m_oTrackingDataVec[i - 1].uiTime);
			std::this_thread::sleep_for(duration);
			continue;
		}

		// We correct for the shift offset
		cv::Point2f _startPos, _targPos;
		vCurrPos += m_vShiftOffset;
		vTargPos += m_vShiftOffset;

		// We convert from screen coordinates to beam current.
		this->do_beam_shift_at_coordinates(vTargPos, nullptr, _startPos, _targPos);

		//CTimer timerDebug;
		if (_Mode == MODE_ANGLE_BASED)
		{
			float fStartAng = _t1_angle;
			float fMovementDuration = fabs(m_oTrackingDataVec[i].fAngle - m_oTrackingDataVec[i - 1].fAngle);

			move_to_point_angle_based(_startPos, _targPos, fMovementDuration);
			/ *while (oMover.isComplete() == false)
			{
				//auto t2 = m_pStage->GetCurrentTAngle(); TODO: COMPARE OLD
				auto t2 = m_pStage->get_current_tilt_angle() - fStartAng; // NEW
				auto point = oMover.update(t2 - _t1_angle);

				m_pZeissControlManager->set_illumination_shift(point.x, point.y);
				_t1_angle = t2;
				if (do_check_for_emergency_exit() || m_pStage->is_stage_busy() == false)
					break;

				std::this_thread::sleep_for(25ms);
			}* /
		}
		else
		{

			// We calculate the duration of the movement
			float fMovementDuration = static_cast<float>(m_oTrackingDataVec[i].uiTime) - m_oTrackingDataVec[i - 1].uiTime;

			// We check if the current time is within the range of the target time
			if (inRange(m_oTrackingDataVec[i - 1].uiTime, m_oTrackingDataVec[i].uiTime, now, true) == false)
			{
				// If the current time is greater than the target time, we skip the iteration
				if (now > m_oTrackingDataVec[i].uiTime)
					continue;
				// If however the current time is less than the target time, it means that we are ahead of the schedule, so we wait before moving on
				else if (now < m_oTrackingDataVec[i - 1].uiTime)
				{
					PRINT("Error@inRange - timer_now < vTargetPos.timer");
					std::chrono::milliseconds duration(m_oTrackingDataVec[i - 1].uiTime - now);
					std::this_thread::sleep_for(duration);
				}
			}

			//timerDebug.doStart();
			// For each iteration, we know the starting pos, ending pos, and duration, we can smoothly move the beam to the target position
			bool bRet = move_to_point_time_based_tem(vCurrPos, vTargPos, fMovementDuration, m_oTrackingDataVec[i - 1].uiTime, now);
			if (bRet == false)
			{
				PRINT("bRet == FALSE");
				break;
			}
			//timerDebug.doEnd();

		}
		//printf("Tracking Region(%d): record(%d)\ttracking(%d)\n", i, m_oTrackingDataVec[i].uiTime - m_oTrackingDataVec[i - 1].uiTime, timerDebug.returnElapsed());
	}*/
}


void CDataCollection::do_image_based_track_stem2(float& _t1_angle, TrackingMode _Mode)
{
	PRINTD("\t\t\t\tCDataCollection::DoImageBasedTrack()");
	if (m_oTrackingDataVec.empty())
		return;

	m_vShiftOffset = m_targetPosNew - m_oTrackingDataVec[0].crystalCoordinates;

	for (int i = 1; i < m_oTrackingDataVec.size(); i++)
	{
		auto now = m_oTrackingTimer.returnElapsed();
		auto vCurrPos = m_oTrackingDataVec[i - 1].crystalCoordinates;
		auto vTargPos = m_oTrackingDataVec[i].crystalCoordinates;

		if (vTargPos == vCurrPos) // fixes the nan error
		{
			std::chrono::milliseconds duration(m_oTrackingDataVec[i].uiTime - m_oTrackingDataVec[i - 1].uiTime);
			std::this_thread::sleep_for(duration);
			continue;
		}


		cv::Point2f _startPos, _targPos;
		vCurrPos += m_vShiftOffset;
		vTargPos += m_vShiftOffset;
		m_pZeissControlManager->set_spot_pos(vTargPos.x, vTargPos.y);
		//this->do_beam_shift_at_coordinates(vTargPos, nullptr, _startPos, _targPos);

		CTimer timerDebug;
		if (_Mode == MODE_ANGLE_BASED)
		{
			float fStartAng = _t1_angle;
			float fMovementDuration = fabs(m_oTrackingDataVec[i].fAngle - m_oTrackingDataVec[i - 1].fAngle);

			move_to_point_angle_based(_startPos, _targPos, fMovementDuration);
			/*while (oMover.isComplete() == false)
			{
				//auto t2 = m_pStage->GetCurrentTAngle(); TODO: COMPARE OLD
				auto t2 = m_pStage->get_current_tilt_angle() - fStartAng; // NEW
				auto point = oMover.update(t2 - _t1_angle);

				m_pZeissControlManager->set_illumination_shift(point.x, point.y);
				_t1_angle = t2;
				if (do_check_for_emergency_exit() || m_pStage->is_stage_busy() == false)
					break;

				std::this_thread::sleep_for(25ms);
			}*/
		}
		else
		{
			float fMovementDuration = static_cast<float>(m_oTrackingDataVec[i].uiTime) - m_oTrackingDataVec[i - 1].uiTime;

			if (inRange(m_oTrackingDataVec[i - 1].uiTime, m_oTrackingDataVec[i].uiTime, now, true) == false)
			{
				//PRINT("Error@inRange - returned FALSE");
				if (now > m_oTrackingDataVec[i].uiTime)
					continue;
				else if (now < m_oTrackingDataVec[i - 1].uiTime)
				{
					PRINT("Error@inRange - timer_now < vTargetPos.timer");
					std::chrono::milliseconds duration(m_oTrackingDataVec[i - 1].uiTime - now);
					std::this_thread::sleep_for(duration);
				}
			}

			timerDebug.doStart();
			bool bRet = move_to_point_time_based_stem(vCurrPos, vTargPos, fMovementDuration, m_oTrackingDataVec[i - 1].uiTime, now);
			if (bRet == false)
			{
				PRINT("bRet == FALSE");
				break;
			}
			timerDebug.doEnd();

		}
		//printf("Tracking Region(%d): record(%d)\ttracking(%d)\n", i, m_oTrackingDataVec[i].uiTime - m_oTrackingDataVec[i - 1].uiTime, timerDebug.returnElapsed());
	}
}

void CDataCollection::do_image_based_track_stem(int& _t1_time, float& _t1_angle, TrackingMode _Mode)
{
	PRINTD("\t\t\t\tCDataCollection::DoImageBasedTrack()");
	if (m_oTrackingDataVec.empty())
		return; 
	
	CLinearMover oMover;

	ImagesInfo* pLastValidFromImg = nullptr;

	for (int i = 1; i < m_oImageBasedVec.size(); i++)
	{
		ImagesInfo vCurrPos = m_oImageBasedVec.at(i - 1);
		if (vCurrPos._bIsImgValid == false && pLastValidFromImg == nullptr)
			continue;
		pLastValidFromImg = &vCurrPos;

		ImagesInfo vTargPos = m_oImageBasedVec.at(i);
		if (vTargPos._bIsImgValid == false)
			continue;

		cv::Point2f currPos(pLastValidFromImg->_iPosX, pLastValidFromImg->_iPosY);
		cv::Point2f targPos(vTargPos._iPosX, vTargPos._iPosY);


		float fMovementDuration = 0.0f;
		if (_Mode == MODE_ANGLE_BASED)
			fMovementDuration = fabs(vTargPos._fImageAngle - pLastValidFromImg->_fImageAngle);
		else
			fMovementDuration = vTargPos._uiImgTime - pLastValidFromImg->_uiImgTime;

		auto originalpoints = oMover.UpdateMoverData(currPos, targPos, fMovementDuration);
		bool bCorrectOnce = false;
		float fOffsetX = 0.0f;
		float fOffsetY = 0.0f;
		float fStartAng = _t1_angle;
		if (_Mode == MODE_ANGLE_BASED)
		{
			while (oMover.isComplete() == false)
			{
				//auto t2 = m_pStage->GetCurrentTAngle(); TODO: COMPARE OLD
				auto t2 = m_pStage->get_current_tilt_angle() - fStartAng; // NEW
				auto point = oMover.update(t2 - _t1_angle);

				if (m_bMoveMouseTest)
					SetCursorPos(point.x + DESKTOSTEMX, point.y + DESKTOSTEMY);
				else
					m_pZeissControlManager->set_spot_pos(point.x, point.y);
				_t1_angle = t2;
				if (do_check_for_emergency_exit() || m_pStage->is_stage_busy() == false)
					break;

				std::this_thread::sleep_for(105ms);
			}
		}
		else
		{
			while (oMover.isComplete() == false)
			{
				if (m_CorrectSpotPosition.x != 512 || m_CorrectSpotPosition.y != 318)
				{
					if (bCorrectOnce == false)
					{
						fOffsetX = originalpoints.at(0).x - m_CorrectSpotPosition.x;
						fOffsetY = originalpoints.at(0).y - m_CorrectSpotPosition.y;
					}
				}
				auto t2 = m_oTrackingTimer.returnElapsed();
				auto point = oMover.update(t2 - _t1_time, fOffsetY, fOffsetY);

				
				if (m_bMoveMouseTest)
					SetCursorPos(/*static_cast<int>*/(point.x) + DESKTOSTEMX, /*static_cast<int>*/(point.y) + DESKTOSTEMY);
				else if(m_pZeissControlManager->is_stem_spot_on())
					m_pZeissControlManager->set_spot_pos(point.x, point.y);
				_t1_time = t2;
				if (do_check_for_emergency_exit() || m_pStage->is_stage_busy() == false)
					break;

				std::this_thread::sleep_for(105ms);
			}
		}


	}
}

tinyspline::BSpline CDataCollection::generate_spline(xt::xarray<double>& params, xt::xarray<double>& ys)
{
	// Tinyspline lib expects the data this way -> [X0, Y0, X1, Y1, X2, Y2, ..., Xn, Yn]ç

	// We begin by concatenating the x and y arrays into a single array -> [X0, X1, X2, ..., Xn, Y0, Y1, Y2, ..., Yn]
	// We then reshape it in a 2D array -> [[X0, X1, X2, ..., Xn], [Y0, Y1, Y2, ..., Yn]]
	xt::xarray<double> xy = xt::concatenate(xt::xtuple(params, ys)).reshape({ -1, static_cast<int>(params.shape()[0]) });

	// Finally, we transpose the array to get the desired format -> [[X0, Y0], [X1, Y1], [X2, Y2], ..., [Xn, Yn]]
	xy = xt::transpose(xy);

	// We convert the xtensor array to a std::vector -> [X0, Y0, X1, Y1, X2, Y2, ..., Xn, Yn]
	std::vector<tinyspline::real> points(xy.begin(), xy.end());

	// 2nd Param is dimension of the data, which is 2 in this case
	tinyspline::BSpline spline = tinyspline::BSpline::
		interpolateCubicNatural(points, static_cast<int>(xy.shape()[1]));
	return spline;
}

xt::xarray<double> CDataCollection::interpolate_spline(tinyspline::BSpline& spline, xt::xarray<double>& targetVals)
{
	// We create an empty array with the same shape/size as the targetVal
	xt::xarray<double> xReturn = xt::zeros_like(targetVals);

	// We iterate over the targetVal and interpolate the x values
	auto it = xReturn.begin();
	for (auto v : targetVals)
	{
		std::vector<tinyspline::real> result = spline.bisect(v).result(); // [0] is the given x, [1] is the inteprolated y
		*it++ = result[1];
	}
	return xReturn;
}

bool CDataCollection::move_to_point_time_based(cv::Point2f& _start, cv::Point2f& _end, float _duration_ms, std::chrono::steady_clock::time_point& now)
{
	bool bReturn = true;
	
	// Get the current time
	auto startTime = std::chrono::high_resolution_clock::now();
	

	while (true)
	{
		// Calculate the elapsed time
		auto currentTime = std::chrono::high_resolution_clock::now();
		float elapsedTime = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - startTime).count();

		// Check if the movement duration is complete
		if (elapsedTime >= _duration_ms)
		{
			// Move to the final position
			m_pZeissControlManager->set_illumination_shift(_end.x, _end.y);
			break;
		}

		// Calculate the interpolated position
		float percent = elapsedTime / _duration_ms;
		float interpolatedX = _start.x + percent * (_end.x - _start.x);
		float interpolatedY = _start.y + percent * (_end.y - _start.y);

		// Move to the interpolated position
		m_pZeissControlManager->set_illumination_shift(interpolatedX, interpolatedY);

		// Sleep for a short duration
		std::this_thread::sleep_for(std::chrono::milliseconds(15));
		if (do_check_for_emergency_exit() || m_pStage->is_stage_busy() == false)
		{
			bReturn = false;
			break;
		}
	}
	return bReturn;
}




bool CDataCollection::move_to_point_time_based_tem(cv::Point2f& _startingPos, cv::Point2f& _targetPos, float _duration_ms, unsigned int _startingTime, unsigned int _currentTime)
{
	bool bReturn = true;
	static CTimepix* pTpx = CTimepix::GetInstance();
	cv::Point2f _start, _end, dummy, new_start, interpolatedPos;


	this->do_beam_shift_at_coordinates(_targetPos, &_startingPos, _start, _end);

	while (true)
	{
		if (pTpx->update_beam_pos)
		{
			std::unique_lock<std::mutex> lock(pTpx->beam_pos_mtx);
			
			_startingPos += m_vShiftOffset_rotation;
			_targetPos += m_vShiftOffset_rotation;
			
			// update the new shift coordinates
			this->do_beam_shift_at_coordinates(_startingPos, nullptr, new_start, dummy);
			
			_end += (new_start - _start);
			_start = new_start;
				
			pTpx->update_beam_pos = false;
			lock.unlock();
		}

		// Calculate the elapsed time
		float currentTime = static_cast<float>(_currentTime);
		float elapsedTime = currentTime - static_cast<float>(_startingTime); // ???

		// Check if the movement duration is complete
		if (elapsedTime >= static_cast<float>(_duration_ms))
		{
			//printf("elapsedTime >= duration_ms (%.4f >= %.4f)\n", elapsedTime, static_cast<float>(_duration_ms));
			this->do_beam_shift_at_coordinates(_targetPos, nullptr, dummy, dummy, true);
			break;
		}

		// Calculate the interpolated position
		float percent = elapsedTime / _duration_ms;
		interpolatedPos.x = _startingPos.x + percent * (_targetPos.x - _startingPos.x);
		interpolatedPos.y = _startingPos.y + percent * (_targetPos.y - _startingPos.y);
		this->do_beam_shift_at_coordinates(interpolatedPos, nullptr, dummy, dummy, true);


		// Sleep for a short duration
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		if (do_check_for_emergency_exit() || (currentTime > 1000 && m_pStage->is_stage_busy() == false))
		{
			bReturn = false;
			break;
		}
		_currentTime = m_oTrackingTimer.returnElapsed();
	}
	m_vShiftOffset += m_vShiftOffset_rotation;
	m_vShiftOffset_rotation = cv::Point2f(0.0f, 0.0f);
	return bReturn;
}

bool CDataCollection::move_to_point_time_based_stem(cv::Point2f& _startingPos, cv::Point2f& _targetPos, float _duration_ms, unsigned int _startingTime, unsigned int _currentTime)
{
	bool bReturn = true;
	static CTimepix* pTpx = CTimepix::GetInstance();
	cv::Point2f _start, _end, dummy, new_start, interpolatedPos;


	this->do_beam_shift_at_coordinates(_targetPos, &_startingPos, _start, _end);

	while (true)
	{
	
		// Calculate the elapsed time
		float currentTime = static_cast<float>(_currentTime);
		float elapsedTime = currentTime - static_cast<float>(_startingTime); // ???

		// Check if the movement duration is complete
		if (elapsedTime >= static_cast<float>(_duration_ms))
		{
			printf("elapsedTime >= duration_ms (%.4f >= %.4f)\n", elapsedTime, static_cast<float>(_duration_ms));
			//this->do_beam_shift_at_coordinates(_targetPos, nullptr, dummy, dummy, true);
			m_pZeissControlManager->set_spot_pos(_targetPos.x, _targetPos.y);
			break;
		}

		// Calculate the interpolated position
		float percent = elapsedTime / _duration_ms;
		interpolatedPos.x = _startingPos.x + percent * (_targetPos.x - _startingPos.x);
		interpolatedPos.y = _startingPos.y + percent * (_targetPos.y - _startingPos.y);
		m_pZeissControlManager->set_spot_pos(interpolatedPos.x, interpolatedPos.y);
		//this->do_beam_shift_at_coordinates(interpolatedPos, nullptr, dummy, dummy, true);


		// Sleep for a short duration
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		if (do_check_for_emergency_exit() || (currentTime > 1000 && m_pStage->is_stage_busy() == false))
		{
			bReturn = false;
			break;
		}
		_currentTime = m_oTrackingTimer.returnElapsed();
	}
	m_vShiftOffset += m_vShiftOffset_rotation;
	m_vShiftOffset_rotation = cv::Point2f(0.0f, 0.0f);
	return bReturn;
}

bool CDataCollection::move_to_point_angle_based(cv::Point2f& _start, cv::Point2f& _end, float _duration_angle)
{
	bool bReturn = true;

	// Get the current angle
	auto startAngle = m_pZeissControlManager->get_stage_tilt_angle();

	while (true)
	{
		// Calculate the delta angle
		auto currentAngle = m_pZeissControlManager->get_stage_tilt_angle();
		float deltaAngle = currentAngle - startAngle;

		// Check if the movement duration is complete
		if (deltaAngle >= _duration_angle)
		{
			// Move to the final position
			m_pZeissControlManager->set_illumination_shift(_end.x, _end.y);
			break;
		}

		// Calculate the interpolated position
		float percent = deltaAngle / _duration_angle;
		float interpolatedX = _start.x + percent * (_end.x - _start.x);
		float interpolatedY = _start.y + percent * (_end.y - _start.y);

		// Move to the interpolated position
		m_pZeissControlManager->set_illumination_shift(interpolatedX, interpolatedY);

		// Sleep for a short duration
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		if (do_check_for_emergency_exit() || m_pStage->is_stage_busy() == false)
		{
			bReturn = false;
			break;
		}
	}
	return bReturn;
}

/* for testing */
#include <future>

bool bKeepLooping = true;

void save_image_data_to_disk(std::string& _fileName, void* _data, int imgSize=512)
{
	TinyTIFFWriterFile* tiffWriter = TinyTIFFWriter_open(_fileName.c_str(), 16, TinyTIFFWriter_Int, 1, imgSize, imgSize, TinyTIFFWriter_Greyscale);
	if (tiffWriter)
	{
		TinyTIFFWriter_writeImage(tiffWriter, _data);
		TinyTIFFWriter_close(tiffWriter);
	}
	else
		printf("Error@CTimePix::SaveImage: Error creating image file (%s).\n", _fileName.c_str());
}

void arrange_frame_and_save(std::string& _fileName, i16* m_pData)
{	
	static auto tpx = CTimepix::GetInstance();
	static int chipCount = tpx->m_pRelaxdModule->chipCount();
	//i16* m_pData = new i16[512 * 512];
	i16* m_pConvertedData = new i16[512 * 512];
	int width, height;
	if (chipCount > 1) {
		width = 2 * MPIX_COLS;
		height = 2 * MPIX_ROWS;
	}
	else {
		width = MPIX_COLS;
		height = MPIX_ROWS;
	}

	//m_pConvertedData = new i16[width * height]();

	// The chips are placed (for V2 chipboard) like
	// +-----+-----+
	// | -3  | -2  |
	// +-----+-----+
	// | +0  | +1  |
	// +-----+-----+
	// where - means rotated by 180 degrees
	// but for V3 chipboard chips 0 and 1 are swapped
	int places[4][4] = { {511,   0, -1,  1},
						{511, 256, -1,  1},
						{  0, 511,  1, -1},
						{  0, 255,  1, -1} };
	i16* src = m_pData;
	for (int chip = 0; chip < chipCount; chip++) {
		// When a chip is missing, it is skipped in the output
		// and the pos will give back the actual position.

		int pos = tpx->m_pRelaxdModule->chipPosition(chip);
		int row = places[pos][0], col = places[pos][1],
			rowstr = places[pos][2], colstr = places[pos][3];
		for (int j = 0; j < MPIX_ROWS; j++) {
			i16* dst = m_pConvertedData + row * width + col;
			for (int i = 0; i < MPIX_COLS; i++) {
				*dst = *(src++);
				dst += colstr;
			}
			row += rowstr;
		}
	}


	save_image_data_to_disk(_fileName, m_pConvertedData);
	delete[] m_pConvertedData;
}

void run_grab_img_asyn(std::future<bool>& _collect)
{
	//std::map<unsigned int, bool> avoid_duplicates_angle;
	auto zeiss = CTEMControlManager::GetInstance();
	auto tpx = CTimepix::GetInstance();
	auto dc = CDataCollection::GetInstance();
	dc->_raw_img_vec.clear();
	_collect.get(); // wait for the signal to start collecting
	CTimer oCaptureImgTimer;
	int iCounter = 0;
	bKeepLooping = true;
	static int iChipCount = tpx->m_pRelaxdModule->chipCount();

	while (zeiss->is_stage_rotating())
	{
		dc->raw_img.fAngle = zeiss->get_stage_tilt_angle(true);
		iCounter++;

		oCaptureImgTimer.doStart();
		
		
		tpx->m_pRelaxdModule->enableTimer(true, tpx->m_iExposureTimeDiff * 1000);
		tpx->m_pRelaxdModule->openShutter();
		while (!tpx->m_pRelaxdModule->timerExpired()) Sleep(1);
		tpx->m_pRelaxdModule->closeShutter();

	
		if (tpx->m_pRelaxdModule->readMatrix(dc->raw_img.data, MPIX_PIXELS * iChipCount))
			PRINT("Error @ReadingMatrix");


	

		dc->raw_img.iCounter = iCounter;
		dc->raw_img.iTotalTime = oCaptureImgTimer.returnElapsed();
		dc->_raw_img_vec.push_back(dc->raw_img);

		
		if (dc->m_iNumOfFramesToTake > 0 && dc->raw_img.iCounter >= dc->m_iNumOfFramesToTake)
			break;
		oCaptureImgTimer.doEnd();
		//printf("Async Timer: %d ms\n", oCaptureImgTimer.returnTotalElapsed());
		oCaptureImgTimer.doReset();
	}
	bKeepLooping = false;
}


void CDataCollection::do_collect_frames()
{
	PRINT("\t\t\t\tCDataCollection::DoCollectFrames() - Start - TEST FUNCTION, ORIGINAL BELOW\n");
	std::map<int, bool> avoid_duplicates;

	if (m_pZeissControlManager->Initialised() == false || m_pTimepix->is_relaxd_module_initialized() == false)
		return;
	//PRINT("ResetMatrix Being Called, REMOVE if it causes problems");
	//m_pTimepix->m_pRelaxdModule->resetMatrix();
	//PRINT("ResetMatrix Being Called, REMOVE if it causes problems");

	if (is_on_stem_mode() == false)
		while (m_bCanStartCollecting == false)
			std::this_thread::sleep_for(10ms);



	int iCounter = 0;
	SFrame imgFrame;
	//_raw_collected_frames.clear();
	m_oDiffracctionFrames.clear();
	imgFrame.bValid = false;
	imgFrame.fAngle = -999.f;
	imgFrame.sFullpath = "C:/Users/TEM/Documents/Moussa_SoftwareImages/LiveStreaming/LiveStream.tiff";
	imgFrame.sDirectory = "C:/Users/TEM/Documents/Moussa_SoftwareImages/LiveStreaming/";
	imgFrame.sImgName = "Livestream.tiff";
	m_oDiffracctionFrames.push_back(imgFrame);
	CTimer oTimer;
	m_fStartingAng = m_pStage->get_current_tilt_angle();
	std::promise<bool> _collect; // new
	std::future<bool> f = _collect.get_future(); //new
	std::future<void> img_async = std::async(std::launch::async, run_grab_img_asyn, std::ref(f)); //new


	while (m_pZeissControlManager->is_stage_rotating() == false)
	{
		imgFrame.bLiveStream = true;
		imgFrame.fAngle = m_fStartingAng;
		m_pTimepix->grab_image_from_detector(imgFrame.sFullpath);
		//m_pTimepix->arrange_image_and_save(imgFrame.sFullpath, _raw_collected_frames.at(0));
		m_oDiffracctionFrames.at(0) = imgFrame;
		//	_raw_collected_frames.clear(); // if you dont want to include the "livestream image"
	}

	bKeepLooping = true;
	_collect.set_value(bKeepLooping); // new
	oTimer.doStart();
	imgFrame.sDirectory = m_sDatasetPath;
	CTimer oCollectStreamTimer;
	oCollectStreamTimer.doStart();

	//maybe we need to change the loop bellow, with a new one that reads the last data in the vector
	//arrange it and save it so that it can be streamed.
	int iCounterTracker = 0;
	while (bKeepLooping)
	{
		while (_raw_img_vec.empty())
			std::this_thread::sleep_for(2ms);
		if(iCounterTracker >= _raw_img_vec.size())
			continue;
		auto _data_struct = _raw_img_vec.at(iCounterTracker);
		iCounterTracker++;

		if (avoid_duplicates.contains(_data_struct.iCounter) == false)
		{
			avoid_duplicates.insert(std::make_pair(_data_struct.iCounter, 1));

			imgFrame.bValid = true;//m_pZeissControlManager->is_stem_spot_on();
			imgFrame.bLiveStream = false;
			imgFrame.fAngle = _data_struct.fAngle;
			imgFrame.iTotalTime = _data_struct.iTotalTime;
			imgFrame.iTimeOfAcquisition = oTimer.returnElapsed();
			int iSize = m_oDiffracctionFrames.size();
			int iMovingAverage = 20;
			if (iSize > iMovingAverage) // moving average rotation speed
			{
				auto previous_imgFrame = m_oDiffracctionFrames.at(iSize - iMovingAverage);
				imgFrame.fRotSpeed = fabs(previous_imgFrame.fAngle - imgFrame.fAngle) / ((imgFrame.iTimeOfAcquisition - previous_imgFrame.iTimeOfAcquisition) * 0.001);
			}
			else
				imgFrame.fRotSpeed = fabs(m_oDiffracctionFrames.at(0).fAngle - imgFrame.fAngle) / (oTimer.returnElapsed() * 0.001);
			imgFrame.sImgName = std::format("{:05d}.tiff", _data_struct.iCounter);
			imgFrame.sFullpath = m_sDatasetPath + imgFrame.sImgName;
			arrange_frame_and_save(imgFrame.sFullpath, _data_struct.data);

			m_oDiffracctionFrames.push_back(imgFrame);
		}
	}
	img_async.get();
	avoid_duplicates.clear();


	/*
	do
	{
		oCollectStreamTimer.doEnd();
		printf("The loop to grab an image took: %d ms\n", oCollectStreamTimer.returnTotalElapsed());
		oCollectStreamTimer.doReset();
		oCollectStreamTimer.doStart();

		imgFrame.bValid = true;//m_pZeissControlManager->is_stem_spot_on();
		imgFrame.bLiveStream = false;
		imgFrame.fAngle = m_pStage->get_current_tilt_angle();
		imgFrame.sImgName = std::format("{:05d}.tiff", iCounter);
		imgFrame.sFullpath = m_sDatasetPath + imgFrame.sImgName;
		imgFrame.fRotSpeed = fabs(m_oDiffracctionFrames.at(0).fAngle - imgFrame.fAngle) / (oTimer.returnElapsed() * 0.001);
		m_pTimepix->grab_image_from_detector(imgFrame.sFullpath);

		m_oDiffracctionFrames.push_back(imgFrame);
		iCounter++;

		if (m_iNumOfFramesToTake > 0 && iCounter >= m_iNumOfFramesToTake)
			break;

		//if (do_check_for_emergency_exit())
		//	break;
	} while (inRange(m_fStartingAng, m_fEndingAng, m_pStage->get_current_tilt_angle(), false) &&
		m_pStage->is_stage_rotating() == true);

		*/

	oTimer.doEnd();
	m_bCanStartCollecting = false;
	std::this_thread::sleep_for(1s);
	m_pZeissControlManager->stage_abort_exec();
	std::this_thread::sleep_for(500ms);
	float fDeltaAng = fabs(m_fStartingAng - m_pStage->get_current_tilt_angle());


	CPostDataCollection oPostDataCollection(&m_oDiffracctionFrames, static_cast<int>(m_pZeissControlManager->get_camera_length()));
	oPostDataCollection.m_fRotationSpeed = fDeltaAng * 1000.0f / oTimer.returnTotalElapsed();
	oPostDataCollection.m_fIntegrationSteps = oPostDataCollection.m_fRotationSpeed * (m_pTimepix->m_iExposureTimeDiff * 0.001f);
	m_bOnDataCollection = false;



	std::this_thread::sleep_for(1s);
	//oPostDataCollection.do_make_pets_file();
	oPostDataCollection.do_flatfield_and_cross_correction(); // creates tiff_corrected files
	m_oDiffracctionFrames.clear();

	//float fRotationSpeed = m_pZeissControlManager->get_stage_tilt_speed();
	//m_pZeissControlManager->set_stage_tilt_speed(70);
	m_pStage->stage_rotate_to_angle(0.0f);
	std::this_thread::sleep_for(1s);
	while (m_pStage->is_stage_rotating())
		std::this_thread::sleep_for(1s);
	//m_pZeissControlManager->set_stage_tilt_speed(fRotationSpeed);

	m_bEnable_items = true;
	PRINT("\t\t\t\tCDataCollection::DoCollectFrames() - Exit - TEST FUNCTION, ORIGINAL BELOW\n");
}


void CDataCollection::tcp_do_collect_frames()
{
	PRINT("\t\t\t\tCDataCollection::DoCollectFrames() - Start - TEST FUNCTION, ORIGINAL BELOW\n");

	static auto pTimepix = CTimepix::GetInstance();
	CTimer oTimer;
	
	if (m_pZeissControlManager->Initialised() == false || m_pTimepix->is_relaxd_module_initialized() == false)
		return;

	if (m_bTrackCrystalPath == true)
		while (m_bCanStartCollecting == false)
			std::this_thread::sleep_for(10ms);

	m_oDiffracctionFrames.clear();
	_raw_img_vec.clear();
	
	pTimepix->tcp_store_frames_counter = 0;
	pTimepix->tcp_startCaptureImgTimer = true;
	m_fStartingAng = m_pStage->get_current_tilt_angle();



	if (m_bPrecession == false)
	{
		while (m_pZeissControlManager->is_stage_rotating() == false)
		{
			std::this_thread::sleep_for(10ms);
			if (m_bOnDataCollection == false)
			{
				m_bEnable_items = true;
				return;
			}
		}

		// STAGE is rotating, we can switch the required variables in order to store the received images.
		pTimepix->tcp_store_diffraction_images = true;
		oTimer.doStart();


		while (m_pZeissControlManager->is_stage_rotating() == true)
			std::this_thread::sleep_for(500ms);
		m_fEndingAng = m_pStage->get_current_tilt_angle();
		pTimepix->tcp_store_diffraction_images = false;
		pTimepix->tcp_store_frames_counter = 0;
		std::this_thread::sleep_for(100ms);
	}
	else
	{
		float offset = 0.5f; 
		if (m_fStartingAng < m_fEndingAng)
			offset = -0.5f;
		float fCurrAng = m_pStage->get_current_tilt_angle();

		bool bToggleFocus = false;
		float orig_Focus = 0.0f;
		float orig_Contrast = 100.0f;
		float orig_Exposure = 1000.0f;
		int orig_mis = 6;
		int orig_ill_idx = m_pZeissControlManager->get_illumination_index();
		do
		{
			if(GetAsyncKeyState(VK_MBUTTON) & 0x1)
			{
				printf("Acquiring frame number (%d) at angle (%.2f)\n", static_cast<int>(pTimepix->tcp_store_frames_counter), fCurrAng);
				//m_pDlg->SetDlgItemTextA(IDC_DC_EXPOSURE, "1000");
				//m_pDlg->SetDlgItemTextA(IDC_DC_EXPOSURE, std::to_string(orig_Exp).c_str());
				
				if(m_pZeissControlManager->is_on_stem_mode())
				{
					pTimepix->tcp_store_diffraction_images = true; // Take an image, the tcp will reset this variable to false
					std::this_thread::sleep_for(100ms);

					m_pStage->move_stage_to_pixel_coordinates(m_pZeissControlManager->get_spot_pos_x(), m_pZeissControlManager->get_spot_pos_y());
					m_pStage->stage_rotate_to_delta_angle(1.0f);
					m_pZeissControlManager->set_stem_spot(false);

					std::this_thread::sleep_for(200ms);
					m_pZeissControlManager->freeze_stem_mode(true);
				}
				else
				{
					if (bToggleFocus == false) // Make sure we're not defocusing...
					{
						pTimepix->tcp_store_diffraction_images = true; // Take an image, the tcp will reset this variable to false
						std::this_thread::sleep_for(100ms);

						m_pStage->stage_rotate_to_delta_angle(1.0f);
					}
				}

				fCurrAng = m_pStage->get_current_tilt_angle();
			}

			if (m_pZeissControlManager->is_on_stem_mode() == false && GetAsyncKeyState(VK_RBUTTON) & 0x1)
			{

				bToggleFocus = !bToggleFocus;

				if (bToggleFocus)
				{
					orig_Focus = m_pZeissControlManager->get_focus();
					orig_mis = m_pZeissControlManager->get_mis_num();
					orig_Contrast = m_pTimepix->get_contrast();
					orig_Exposure = m_pTimepix->m_iExposureTimeDiff;

					// Change exposure time, contrast and illumination angle ot the minimum
					m_pZeissControlManager->set_illumination_index(0);
					m_pDlg->SetDlgItemTextA(IDC_DC_GAIN, "1.0");
					m_pDlg->SetDlgItemTextA(IDC_DC_EXPOSURE, "100.0");
					

					// Defocus
					m_pZeissControlManager->set_focus(0.1f);

					// Use a slightly bigger condenser aperture hole (?)
					//m_pZeissControlManager->set_mis_num(0);
				}
				else
				{
					// Restore the original diffraction contrast
					m_pDlg->SetDlgItemTextA(IDC_DC_GAIN, std::to_string(orig_Contrast).c_str());
					m_pDlg->SetDlgItemTextA(IDC_DC_EXPOSURE, std::to_string(orig_Exposure).c_str());

					// Restore the original focus value
					m_pZeissControlManager->set_illumination_index(orig_ill_idx);
					m_pZeissControlManager->set_focus(orig_Focus);

					// Restore the original mis aperture
					//m_pZeissControlManager->set_mis_num(orig_mis);
				}
			}

		} while (m_bPrecession == true && inRange(m_fStartingAng + offset, m_fEndingAng - offset, fCurrAng, true));
	}


	for (auto& data : _raw_img_vec)
	{
		SFrame imgFrame;
		imgFrame.bValid = true;
		imgFrame.bLiveStream = false;
		imgFrame.sImgName = std::format("{:05d}.tiff", data.iCounter);
		imgFrame.sFullpath = m_sDatasetPath + imgFrame.sImgName;
		imgFrame.sDirectory = m_sDatasetPath;
		imgFrame.fAngle = data.fAngle;
		save_image_data_to_disk(imgFrame.sFullpath, &data.data);
		m_oDiffracctionFrames.push_back(imgFrame);
	}


	oTimer.doEnd();
	m_bCanStartCollecting = false;
	m_pZeissControlManager->do_blank_beam(true);
	std::this_thread::sleep_for(1s);
	m_pZeissControlManager->stage_abort_exec();
	std::this_thread::sleep_for(500ms);
	float fDeltaAng = fabs(m_fStartingAng - m_pStage->get_current_tilt_angle());


	if (m_pZeissControlManager->is_on_stem_mode() == false)
		m_oSearchingParams.RestoreCurrentParameters(PARAM_SEARCHING);
	

	if(m_oDiffracctionFrames.empty() == false)
	{
		CPostDataCollection oPostDataCollection(&m_oDiffracctionFrames, static_cast<int>(m_pZeissControlManager->get_camera_length()));
		oPostDataCollection.m_fRotationSpeed = fDeltaAng * 1000.0f / oTimer.returnTotalElapsed();
		oPostDataCollection.m_fIntegrationSteps = oPostDataCollection.m_fRotationSpeed * (m_pTimepix->m_iExposureTimeDiff * 0.001f);
		m_bOnDataCollection = false;

		std::this_thread::sleep_for(1s);
		oPostDataCollection.do_make_pets_file();
		oPostDataCollection.do_flatfield_and_cross_correction(); // creates tiff_corrected files
		m_oDiffracctionFrames.clear();
		_raw_img_vec.clear();
	}else
		printf("No frames were collected\n");


	

	

	//float fRotationSpeed = m_pZeissControlManager->get_stage_tilt_speed();
	//m_pZeissControlManager->set_stage_tilt_speed(70);
	m_pStage->stage_rotate_to_angle(0.0f);
	std::this_thread::sleep_for(1s);
	while (m_pStage->is_stage_rotating())
		std::this_thread::sleep_for(1s);
	//m_pZeissControlManager->set_stage_tilt_speed(fRotationSpeed);

	m_bEnable_items = true;
	PRINT("\t\t\t\tCDataCollection::DoCollectFrames() - Exit - TEST FUNCTION, ORIGINAL BELOW\n");
}

/*for testing*/




/*
void CDataCollection::do_collect_frames()
{
	PRINT("\t\t\t\tCDataCollection::DoCollectFrames() - Start\n");

	if (m_pZeissControlManager->Initialised() == false || m_pTimepix->is_relaxd_module_initialized() == false)
		return;
	//PRINT("ResetMatrix Being Called, REMOVE if it causes problems");
	//m_pTimepix->m_pRelaxdModule->resetMatrix();
	//PRINT("ResetMatrix Being Called, REMOVE if it causes problems");

	int iCounter = 0;
	SFrame imgFrame;
	//_raw_collected_frames.clear();
	m_oDiffracctionFrames.clear();
	imgFrame.bValid = false;
	imgFrame.fAngle = -999.f;
	imgFrame.sFullpath = "C:/Users/TEM/Documents/Moussa_SoftwareImages/LiveStreaming/LiveStream.tiff";
	imgFrame.sDirectory = "C:/Users/TEM/Documents/Moussa_SoftwareImages/LiveStreaming/";
	imgFrame.sImgName = "Livestream.tiff";
	m_oDiffracctionFrames.push_back(imgFrame);
	CTimer oTimer;
	float fStartingAng = m_pStage->get_current_tilt_angle();
	while (m_pZeissControlManager->is_stage_rotating() == false)
	{
		imgFrame.bLiveStream = true;
		imgFrame.fAngle = fStartingAng;
		m_pTimepix->grab_image_from_detector(imgFrame.sFullpath);
		//m_pTimepix->arrange_image_and_save(imgFrame.sFullpath, _raw_collected_frames.at(0));
		m_oDiffracctionFrames.at(0) = imgFrame;
	//	_raw_collected_frames.clear(); // if you dont want to include the "livestream image"
	}
	oTimer.doStart();
	imgFrame.sDirectory = m_sDatasetPath;
	CTimer oCollectStreamTimer;
	oCollectStreamTimer.doStart();
	do
	{
		oCollectStreamTimer.doEnd();
		printf("The loop to grab an image took: %d ms\n", oCollectStreamTimer.returnTotalElapsed());
		oCollectStreamTimer.doReset();
		oCollectStreamTimer.doStart();

		imgFrame.bValid = true;//m_pZeissControlManager->is_stem_spot_on();
		imgFrame.bLiveStream = false;
		imgFrame.fAngle = m_pStage->get_current_tilt_angle();
		imgFrame.sImgName = std::format("{:05d}.tiff", iCounter);
		imgFrame.sFullpath = m_sDatasetPath + imgFrame.sImgName;
		imgFrame.fRotSpeed = fabs(m_oDiffracctionFrames.at(0).fAngle - imgFrame.fAngle) / (oTimer.returnElapsed() * 0.001);
		m_pTimepix->grab_image_from_detector(imgFrame.sFullpath);
		
		m_oDiffracctionFrames.push_back(imgFrame);
		iCounter++;

		if (m_iNumOfFramesToTake > 0 &&  iCounter >= m_iNumOfFramesToTake)
				break;

		//if (do_check_for_emergency_exit())
		//	break;
	} 
	while (inRange(m_fStartingAng, m_fEndingAng, m_pStage->get_current_tilt_angle(), false) &&
		   m_pStage->is_stage_rotating() == true);

	oTimer.doEnd();
	float fDeltaAng = fabs(fStartingAng - m_pStage->get_current_tilt_angle());


	CPostDataCollection oPostDataCollection(&m_oDiffracctionFrames, static_cast<int>(m_pZeissControlManager->get_camera_length()));
	oPostDataCollection.m_fRotationSpeed = fDeltaAng * 1000.0f / oTimer.returnTotalElapsed();
	oPostDataCollection.m_fIntegrationSteps = oPostDataCollection.m_fRotationSpeed * (m_pTimepix->m_iExposureTime * 0.001f);
	m_bOnDataCollection = false;



	oPostDataCollection.do_make_pets_file();
	oPostDataCollection.do_flatfield_and_cross_correction(); // creates tiff_corrected files
	m_oDiffracctionFrames.clear();

	std::this_thread::sleep_for(2s);
	float fRotationSpeed = m_pZeissControlManager->get_stage_tilt_speed();
	m_pZeissControlManager->set_stage_tilt_speed(70);
	m_pStage->stage_rotate_to_angle(0.0f);
	std::this_thread::sleep_for(1s);
	while (m_pStage->is_stage_rotating())
		std::this_thread::sleep_for(2s);
	m_pZeissControlManager->set_stage_tilt_speed(fRotationSpeed);

	PRINT("\t\t\t\tCDataCollection::DoCollectFrames() - Exit\n");
}
*/

void CDataCollection::do_live_stream_collected_frames()
{
	if (m_pZeissControlManager->Initialised() == false || m_pTimepix->is_relaxd_module_initialized() == false)
		return;


	bool bStopLoop = false;
	std::string LiveStreamWindowName = "Live Streaming - Collected Frames";
	cv::namedWindow(LiveStreamWindowName);
	cv::moveWindow(LiveStreamWindowName, 0, -620);

	cv::Mat oLiveCollectedFrame;
	std::string sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/LiveStreaming/LiveStream.tiff";

	while (bStopLoop == false)
	{
		if (m_oDiffracctionFrames.empty())
		{
			oLiveCollectedFrame = cv::Mat::zeros(512, 512, 2);
		}
		else if (m_oDiffracctionFrames.size() == 1)
		{
			oLiveCollectedFrame = cv::imread(sFileName, cv::IMREAD_ANYDEPTH); // remove later
		}
		else
		{
			oLiveCollectedFrame = cv::imread(m_oDiffracctionFrames.back().sFullpath, cv::IMREAD_ANYDEPTH); // remove later
		}

		if (oLiveCollectedFrame.empty() == false)
		{
			double max_val;
			minMaxLoc(oLiveCollectedFrame, NULL, &max_val);
			oLiveCollectedFrame.convertTo(oLiveCollectedFrame, CV_16UC1, 65535.0f / max_val);
			cv::convertScaleAbs(oLiveCollectedFrame, oLiveCollectedFrame, m_pTimepix->get_contrast() / 100.0f, m_pTimepix->get_brightness());
			cv::resize(oLiveCollectedFrame, oLiveCollectedFrame, cv::Size(), IMG_RESIZE_FACTOR__, IMG_RESIZE_FACTOR__);

			if (m_pTimepix->m_bInvertColours)
				cv::bitwise_not(oLiveCollectedFrame, oLiveCollectedFrame);
			
			cv::imshow(LiveStreamWindowName, oLiveCollectedFrame);
			cv::waitKey(100);

			if (do_check_for_emergency_exit())
				break;
		}
		else
			std::this_thread::sleep_for(1s);


		if (m_bOnDataCollection == false)
			bStopLoop = true; // break;
	}
	cv::destroyWindow(LiveStreamWindowName);
}

void CDataCollection::do_tilt_backlash_correction(float _fTargetAngle, bool _bPositiveDirection, float _offset /*= 5.0f*/, bool inSteps /*= false*/)
{
	PRINTD("\t\t\t\tCDataCollection::DoTiltBacklashCorrection()\n");
	
	float fDirection = 1.0f;
	if (_bPositiveDirection == false)
		fDirection = -1.0f;

	do_fast_stage_movement_parameters();

	float _fAngle = _fTargetAngle - (fDirection * _offset);
	m_pStage->stage_rotate_to_angle(_fAngle);
	std::this_thread::sleep_for(1s);
	while (m_pStage->is_stage_busy() || m_pStage->is_stage_rotating())
		std::this_thread::sleep_for(150ms);

	do_restore_data_collection_parameters();

	if (inSteps)
	{
		int ioffset = static_cast<int>(_offset);
		for (int i = 0; i < ioffset; i++)
		{
			_fAngle += (fDirection);
			m_pStage->stage_rotate_to_angle(_fAngle);

			while (m_pStage->is_stage_busy() || m_pStage->is_stage_rotating())
				std::this_thread::sleep_for(150ms);

			if (do_check_for_emergency_exit())
				break;
		}
	}
	else
	{
		m_pStage->stage_rotate_to_angle(_fTargetAngle);
		while (m_pStage->is_stage_busy() || m_pStage->is_stage_rotating())
			std::this_thread::sleep_for(500ms);
	}


	std::this_thread::sleep_for(500ms);
	if(fabs(m_pStage->get_current_tilt_angle() - _fTargetAngle) > 0.05f)
	{
		m_pStage->stage_rotate_to_angle(_fTargetAngle);

		while (m_pStage->is_stage_busy() || m_pStage->is_stage_rotating())
			std::this_thread::sleep_for(500ms);
	}

}


cv::Mat adaptiveNormalize2(const cv::Mat& image, double minIntensityThreshold = 500.0, double scaleFactor = 10.0) {
	double minVal, maxVal;
	cv::minMaxLoc(image, &minVal, &maxVal);

	cv::Mat normalizedImage;
	double minEffectiveVal = maxVal * 0.00f;

	if (maxVal - minVal < minIntensityThreshold) {
		// Low dynamic range case: apply an adaptive scaling factor
		image.convertTo(normalizedImage, CV_16U, scaleFactor, -minEffectiveVal * scaleFactor);
	}
	else {
		// Normal case: normalize to fit within the 14-bit range
		image.convertTo(normalizedImage, CV_16U, std::pow(2, 16) / maxVal, -minEffectiveVal * (std::pow(2, 16) / maxVal));
	}

	return normalizedImage;
}




void CDataCollection::serial_ed_store_valid_images_to_disk(std::vector<_img_data>& img_data_vec, unsigned int& imgIndex, unsigned int& validImgCount)
{

	for (auto& data : _raw_img_vec)
	{
		cv::Mat oImage = cv::Mat(512, 512, CV_16U, data.data);
		cv::Mat crossCorrectImg = cv::Mat::zeros(cv::Size(516, 516), oImage.type());
		CPostDataCollection::do_flatfield_correction(oImage);
		CPostDataCollection::do_dead_pixels_correction(oImage);
		CPostDataCollection::do_cross_correction(oImage, crossCorrectImg);
		
		cv::Mat preprocessImg = adaptiveNormalize2(crossCorrectImg);
		cv::resize(preprocessImg, preprocessImg, cv::Size(), IMG_RESIZE_FACTOR__, IMG_RESIZE_FACTOR__);
		if (is_diffraction_pattern_valid(preprocessImg, m_iSerialED_min_num_peaks, m_fSerialED_dmax, true))
		{
			std::string sFileName = m_sDatasetPath + std::format("{:010d}.tiff", imgIndex);
			save_image_data_to_disk(sFileName, crossCorrectImg.data, 516);
			validImgCount++;
			imgIndex++;
		}
	}

	img_data_vec.clear();
	printf("Saved Images: %d out of %d collected images\n", validImgCount, static_cast<int>(m_pTimepix->tcp_store_frames_counter));
}

bool CDataCollection::is_diffraction_pattern_valid(const cv::Mat&diffractionImg, int num_of_peaks, float d_max, bool bUseGFTT)
{
	bool bReturn = true;
	static CImageManager* m_pImgMgr = CImageManager::GetInstance();
	static SCameraLengthSettings oCameraLength;
	static int iLastCameraLength = 0;
	
	cv::Point2f centralBeamCoords;
	bReturn = m_pImgMgr->find_central_beam_position(diffractionImg, centralBeamCoords);
	if (bReturn == false)
		return false;

	std::vector<cv::Point2f> detectedPeaks;
	std::vector<cv::Point2f> rejectedPeaks;
	if (iLastCameraLength != int(m_fCameraLength))
	{
		oCameraLength.UpdateCameraLengthSettings(int(m_fCameraLength));
		iLastCameraLength = int(m_fCameraLength);
	}

	//cv::Mat contrastCorrect;
	//cv::convertScaleAbs(diffractionImg, contrastCorrect, 0.8f * 0.5f * 1.0f / 100.0f, 1.0f);
	bReturn = m_pImgMgr->detect_diffraction_peaks_cheetahpf8(diffractionImg, centralBeamCoords, oCameraLength.flPixelSize, d_max, detectedPeaks, m_fSerialED_i_sigma, m_fSerialED_peaksize, m_fSerialED_peakthreshold, 4);
	//bReturn = m_pImgMgr->detect_diffraction_peaks_cheetahpf8(diffractionImg, centralBeamCoords, oCameraLength.flPixelSize, d_max, detectedPeaks, rejectedPeaks, num_of_peaks, m_fSerialED_i_sigma, m_fSerialED_peaksize, m_fSerialED_peakdistance, 4, bUseGFTT);
	if (bReturn == false || detectedPeaks.size() < num_of_peaks)
		return false;
	return bReturn;

}

void CDataCollection::do_save_data_collection_parameters()
{
	PRINTD("\t\t\t\tCDataCollection::DoSaveDataCollectionParameters()\n");

	// Parameters to save and restore later
	m_fWorkingStageTSpeed = m_pZeissControlManager->get_stage_tilt_speed();
	m_fWorkingStageXYSpeed = m_pZeissControlManager->get_stage_xy_speed();
	if(is_on_stem_mode())
		m_fRecordSTEMMagnification = m_pZeissControlManager->get_stem_magnification();

	//DoFastStageMovementsParameters();
}


void CDataCollection::do_fast_stage_movement_parameters()
{
	//printf("do_fast_stage_movement_parameters no longer used.\n");
	return;
	PRINTD("\t\t\t\tCDataCollection::DoFastStageMovementsParameters()");
	// Parameters to edit to make life easier/faster.
	m_pZeissControlManager->set_stage_tilt_speed(90.0f);
	m_pZeissControlManager->set_stage_xy_speed(50.0f);
}


void CDataCollection::do_restore_data_collection_parameters()
{
	PRINTD("\t\t\t\tCDataCollection::DoRestoreDataCollectionParameters()\n");
	
	m_pZeissControlManager->set_stem_magnification(m_fRecordSTEMMagnification);
	m_pZeissControlManager->set_stage_tilt_speed(m_fWorkingStageTSpeed);
	m_pZeissControlManager->set_stage_xy_speed(m_fWorkingStageXYSpeed);
}


unsigned int CDataCollection::make_time_key(int _iTimekey, int _iMultiple /*= 25*/)
{
	if (_iMultiple == 1)
	{
		_iTimekey /= 100; _iTimekey *= 100;
	}

	const int iReturn = ((_iTimekey + _iMultiple / 2) / _iMultiple) * _iMultiple;
	return iReturn;
}


unsigned int CDataCollection::make_angle_key(float _fAngle)
{
	return static_cast<unsigned int>(std::round(_fAngle * _ANG_TO_KEY_MULTIPLIER_));
}


void CDataCollection::infinite_loop_for_monitoring()
{
	PRINTD("\t\t\t\tCDataCollection::DataCollectionMainLoop() -- Starting...");
	
	while (m_pZeissControlManager == nullptr)
		std::this_thread::sleep_for(500ms);

	if (m_pZeissControlManager->Initialised())
	{
		while (m_bKeepThreadRunning)
		{
			// EMERGENCY EXIT
			do_check_for_emergency_exit(VK_DIVIDE); // MULTIPLY KEY TO EXIT
			if (GetAsyncKeyState(VK_MBUTTON) & 0x8000)
					do_move_stage_to_mouse_coordinates();

			if (m_bOnTracking)
			{
				if (GetAsyncKeyState(VK_F5) & 0x1)
				{
					//TODO: REMOVE LATER ON
					bool bMouseTestOn = m_bMoveMouseTest;
					if (m_bMoveMouseTest)
						m_bMoveMouseTest = false;

					m_pZeissControlManager->set_stem_spot(false); // go to scanning mode
					
					while ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == false)
						std::this_thread::sleep_for(15ms);

					GetCursorPos(&m_CorrectSpotPosition);
					m_CorrectSpotPosition.x -= DESKTOSTEMX;
					m_CorrectSpotPosition.y -= DESKTOSTEMY;
					m_bMoveMouseTest = bMouseTestOn; // restore, REMOVE LATER

					if(m_bMoveMouseTest == false)
						m_pZeissControlManager->set_stem_spot(true); // go to scanning mode

				}
			}

			std::this_thread::sleep_for(25ms);
		}
	}
	else
		m_bKeepThreadRunning = false;

	
	PRINTD("\t\t\t\tCDataCollection::DataCollectionMainLoop() -- Exiting...");
}


void CDataCollection::do_move_stage_to_mouse_coordinates()
{
	PRINTD("\t\t\t\tCDataCollection::DoMoveStageToMouseCoords()");
	
	POINT mouseCoords;
	GetCursorPos(&mouseCoords);
	mouseCoords.x -= DESKTOSTEMX;
	mouseCoords.y -= DESKTOSTEMY;
	//printf("Corrected MouseCoords: (%d, %d)\n", mouseCoords.x, mouseCoords.y);


	// Should we change the stage speed???
	if (mouseCoords.x >= 0 && mouseCoords.x < 1024 && mouseCoords.y >= 0 && mouseCoords.y < 768)
		m_pStage->move_stage_to_pixel_coordinates(mouseCoords.x, mouseCoords.y);
}


bool CDataCollection::do_check_for_emergency_exit(DWORD _dwKey /*= VK_MULTIPLY*/)
{
	if ((GetAsyncKeyState(_dwKey) & 0x8000) == false) // EMERGENCY EXIT
		return false;

	// EVERYTHING THAT SHOULD BE STOPPED GOES HERE
	if(_dwKey == VK_DIVIDE)
		m_bKeepThreadRunning = false;
	m_pZeissControlManager->stage_abort_exec();
	PRINT("Aborting current task...!");
	return true;
}


void CDataCollection::onMouse_2DMap(int event, int x, int y, int, void* userdata) {
	auto* params = static_cast<std::tuple<cv::Mat*, GridRegions*, float*, bool*, cv::Point*>*>(userdata);
	cv::Mat* canvas = std::get<0>(*params);
	GridRegions* selection = std::get<1>(*params);
	float scalingFactor = *std::get<2>(*params);
	bool* isDragging = std::get<3>(*params);
	cv::Point2f* stagePosition = &reinterpret_cast<CDataCollection*>(std::get<4>(*params))->m_StagePosition2D;

	if (event == cv::EVENT_LBUTTONDOWN) {
		// Start drawing the selection rectangle
		*isDragging = true;
		selection->stage_z = CTEMControlManager::GetInstance()->get_stage_z();
		selection->gridRegion = cv::Rect(x, y, 0, 0);
	}
	else if (event == cv::EVENT_RBUTTONDOWN) {
		// Check if clicked inside any existing region to delete it
		for (auto it = gridRegions.begin(); it != gridRegions.end(); ++it) {
			if (it->gridRegion.contains(cv::Point(x, y))) {
				gridRegions.erase(it);
				// Update the display immediately after deletion
				cv::Mat tempCanvas = canvas->clone();
				for (const auto& region : gridRegions) {
						cv::rectangle(tempCanvas, region.gridRegion, region.isRegionScanned ? COLOR_GRIDREGION_SCANNED : COLOR_GRIDREGION_UNSCANNED, 2, cv::LINE_AA);
					

				}
				// Draw the current stage position
				cv::circle(tempCanvas, *stagePosition, 5, cv::Scalar(255, 0, 0), -1, cv::LINE_AA); // Blue circle for stage position
				cv::imshow("TEM Stage Map", tempCanvas);
				return;
			}
		}

	}
	else if (event == cv::EVENT_MBUTTONDBLCLK)
	{
		gridRegions.clear();
	}
	else if (event == cv::EVENT_MOUSEMOVE) {
		if (*isDragging) {
			// Update the size of the selection rectangle
			selection->gridRegion.width = x - selection->gridRegion.x;
			selection->gridRegion.height = y - selection->gridRegion.y;

			// Draw the current selection on a temporary canvas
			cv::Mat tempCanvas = canvas->clone();
			for (const auto& region : gridRegions) {
				cv::rectangle(tempCanvas, region.gridRegion, region.isRegionScanned ? COLOR_GRIDREGION_SCANNED : COLOR_GRIDREGION_UNSCANNED, 2, cv::LINE_AA);
			}
			cv::rectangle(tempCanvas, selection->gridRegion, COLOR_GRIDREGION_UNSCANNED, 2, cv::LINE_AA);
			// Draw the current stage position
			cv::circle(tempCanvas, *stagePosition, 5, cv::Scalar(255, 0, 0), -1, cv::LINE_AA); // Blue circle for stage position
			cv::imshow("TEM Stage Map", tempCanvas);
		}
	}
	else if (*isDragging == true && event == cv::EVENT_LBUTTONUP) {
		// Finish drawing the selection rectangle
		*isDragging = false;

		if (selection->gridRegion.width != 0 && selection->gridRegion.height != 0) {
			// Add the selection to the list of regions

			gridRegions.push_back(*selection);

			// Calculate the corresponding stage coordinates (adjusted for the coordinate system)
			float startStageX = (selection->gridRegion.x - canvas->cols / 2) / scalingFactor;
			float endStageX = (selection->gridRegion.x + selection->gridRegion.width - canvas->cols / 2) / scalingFactor;
			float startStageY = (canvas->rows / 2 - selection->gridRegion.y) / scalingFactor;
			float endStageY = (canvas->rows / 2 - (selection->gridRegion.y + selection->gridRegion.height)) / scalingFactor;

			// Print stage coordinates to the console
			//std::cout << "Selected Area Stage Coordinates: Start X = " << startStageX << " um, End X = " << endStageX
				//<< " um, Start Y = " << startStageY << " um, End Y = " << endStageY << " um" << std::endl;
		}
	}
	else if (event == cv::EVENT_LBUTTONDBLCLK) {
		// Move the stage to the clicked coordinates (adjusted for the coordinate system)
		float stageX = (x - canvas->cols / 2) / scalingFactor;
		float stageY = (canvas->rows / 2 - y) / scalingFactor;
		std::cout << "Moving to Stage Coordinates: X = " << stageX << " um, Y = " << stageY << " um" << std::endl;
		m_pZeissDataCollection->m_pStage->stage_go_to_xy(stageX, stageY);


	}
}

void CDataCollection::display_2d_map() {
	std::string windowName = "TEM Stage Map";

	// Static part: Create an empty image to represent the TEM stage
	int canvasSize = _CANVAS_SIZE_; // Size of the canvas in pixels

	cv::Mat canvas = cv::Mat::zeros(canvasSize, canvasSize, CV_8UC3);
	canvas.setTo(cv::Scalar(200, 200, 200)); // gray background

	// Calculate the scaling factor to fit the stage movement limit within the canvas
	float scalingFactor = static_cast<float>(canvasSize) / _GRID_MAX_LENGTH; // 2000um is the stage movement limit

	// Define the center and radius for the circle representing the stage movement limit
	cv::Point center(canvasSize / 2, canvasSize / 2);
	int radius = static_cast<int>(_GRID_MAX_LENGTH * 0.5f * scalingFactor);  // Scale radius to fit within the new canvas size

	// Draw the circle representing the stage movement limit
	cv::circle(canvas, center, radius, cv::Scalar(0, 0, 200), 2, cv::LINE_AA); // Red circle

	// Draw grid lines every 100um, scaled to fit the canvas
	int gridSpacing = static_cast<int>(100 * scalingFactor);
	for (int i = gridSpacing; i < canvasSize; i += gridSpacing) {
		// Vertical lines
		int thickness = (i == canvasSize / 2) ? 3 : 2; // Thicker line for the middle cross
		cv::line(canvas, cv::Point(i, 0), cv::Point(i, canvasSize), cv::Scalar(50, 50, 50), thickness, cv::LINE_AA);
		// Horizontal lines
		cv::line(canvas, cv::Point(0, i), cv::Point(canvasSize, i), cv::Scalar(50, 50, 50), thickness, cv::LINE_AA);
	}

	cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE); // Use WINDOW_AUTOSIZE to maintain size without resizing
	cv::moveWindow(windowName, 0, -1024);
#ifdef _DEBUGGING_
	cv::moveWindow(windowName, 0, 0);
#endif

	// Variables for interaction
	GridRegions selection;
	bool isDragging = false;

	auto params = std::make_tuple(&canvas, &selection, &scalingFactor, &isDragging, this);
	cv::setMouseCallback(windowName, onMouse_2DMap, &params);

	// Dynamic part: Wait for user input to toggle the map display or exit
	m_bIs2DMapVisible = true;
	// Draw the current stage position
	while (m_bIs2DMapVisible) {
		cv::Mat tempCanvas = canvas.clone();
		for (const auto& region : gridRegions) {
			cv::rectangle(tempCanvas, region.gridRegion, region.isRegionScanned ? COLOR_GRIDREGION_SCANNED : COLOR_GRIDREGION_UNSCANNED, 2, cv::LINE_AA);
		}
		if (isDragging && (selection.gridRegion.width != 0 || selection.gridRegion.height != 0)) {
			cv::rectangle(tempCanvas, selection.gridRegion, COLOR_GRIDREGION_UNSCANNED, 2, cv::LINE_AA);
		}


		m_StagePosition2D = get_2d_stage_coordinates(m_pStage->get_stage_x(), m_pStage->get_stage_y());
		cv::circle(tempCanvas, m_StagePosition2D, 5, cv::Scalar(255, 0, 0), -1, cv::LINE_AA); // Blue circle for stage position


		cv::imshow(windowName, tempCanvas);
		auto q = cv::waitKey(100);
		if (q == 'q' || q == 'Q') {
			m_bIs2DMapVisible = false;
			break;
		}
	}

	cv::destroyWindow(windowName);
}


void CDataCollection::do_serial_ed_regions_scan() {
	// Iterate over each selected region and perform zigzag scanning

	float scalingFactor = static_cast<float>(_CANVAS_SIZE_) / _GRID_MAX_LENGTH; // 2000um is the stage movement limit
	unsigned int validImgCount = 0;
	m_pTimepix->tcp_store_frames_counter = 0;


	unsigned int imgIndex = 0;
	for (const auto& entry : std::filesystem::directory_iterator(m_sDatasetPath))
	{
		if (entry.path().extension() == ".tiff")
			imgIndex++;
	}

	int iRegionID = -1;
	
	bool bBreakScan = false;
	for (auto& region : gridRegions) {
		iRegionID++;

		if (region.isRegionScanned)
			continue;

		printf("\tStarting Region [%d] out of [%d]\n", iRegionID, gridRegions.size());
		// Calculate the starting and ending stage coordinates (adjusted for the coordinate system)
		int startX = static_cast<int>((region.gridRegion.x - _CANVAS_SIZE_ * 0.5f) / scalingFactor);
		int endX = static_cast<int>((region.gridRegion.x + region.gridRegion.width - _CANVAS_SIZE_ * 0.5f) / scalingFactor);
		int startY = static_cast<int>((_CANVAS_SIZE_ * 0.5f - region.gridRegion.y) / scalingFactor);
		int endY = static_cast<int>((_CANVAS_SIZE_ * 0.5f - (region.gridRegion.y + region.gridRegion.height)) / scalingFactor);

		printf("StartX: %d um - StartY: %d um\nEndX: %d um - EndY: %d um\n\n", startX, startY, endX, endY);

		// Let's move the stage to the top left of the region and wait a bit until the stage is no longer busy/moving.
		m_pZeissControlManager->do_blank_beam(true);
		m_pZeissDataCollection->m_pStage->stage_go_to_xy(startX, startY);

		while (m_pZeissDataCollection->m_pStage->is_stage_busy())
			std::this_thread::sleep_for(500ms);

		m_pZeissControlManager->do_blank_beam(false);
		std::this_thread::sleep_for(50ms);

		bool moveRight = true;

		for (float y = startY; y >= endY; y -= m_fSerialED_ysteps) {

			if(bBreakScan)
				break;

			m_pTimepix->tcp_store_diffraction_images = true;
			if (moveRight) {

				float fCurrentStageX = startX;
				
				m_StagePosition2D.x = region.gridRegion.x + region.gridRegion.width;

				while (int(std::round(fCurrentStageX)) != int(std::round(endX)))
				{
					m_pZeissDataCollection->m_pStage->stage_go_to_x(endX);
					while (m_pZeissDataCollection->m_pStage->is_stage_busy()) // As long as the stage is busy, we're good. If I'm not mistaken, the stage will be busy until it reaches the desired position
					{
						bBreakScan = do_check_for_emergency_exit();
						std::this_thread::sleep_for(1000ms);

						if(bBreakScan)
							break;
					}


					if (m_pTimepix->tcp_store_diffraction_images == false) // If we're here, it's because we've stored more than the fixed number of images in the vector.
					{

						serial_ed_store_valid_images_to_disk(_raw_img_vec, imgIndex, validImgCount);
						_raw_img_vec.clear();
						m_pTimepix->tcp_store_diffraction_images = true;

					}

					fCurrentStageX = m_pZeissDataCollection->m_pStage->get_stage_x();
					if(bBreakScan)
						break;
				}
				m_pTimepix->tcp_store_diffraction_images = false;
				serial_ed_store_valid_images_to_disk(_raw_img_vec, imgIndex, validImgCount);
				_raw_img_vec.clear();

				if (bBreakScan)
					break;
			}
			else {
				
				float fCurrentStageX = endX;

				while (int(std::round(fCurrentStageX)) != int(std::round(startX)))
				{
					m_pZeissDataCollection->m_pStage->stage_go_to_x(startX);
					while (m_pZeissDataCollection->m_pStage->is_stage_busy()) // As long as the stage is busy, we're good. If I'm not mistaken, the stage will be busy until it reaches the desired position
					{
						bBreakScan = do_check_for_emergency_exit();
						std::this_thread::sleep_for(1000ms);
						if (bBreakScan)
							break;
					}


					if (m_pTimepix->tcp_store_diffraction_images == false) // If we're here, it's because we've stored more than the fixed number of images in the vector.
					{

						serial_ed_store_valid_images_to_disk(_raw_img_vec, imgIndex, validImgCount);
						_raw_img_vec.clear();
						m_pTimepix->tcp_store_diffraction_images = true;

					}

					fCurrentStageX = m_pZeissDataCollection->m_pStage->get_stage_x();
					if (bBreakScan)
						break;
				}
				m_pTimepix->tcp_store_diffraction_images = false;
				serial_ed_store_valid_images_to_disk(_raw_img_vec, imgIndex, validImgCount);
				
				if (bBreakScan)
					break;
			}
			moveRight = !moveRight;
			

			m_pTimepix->tcp_store_diffraction_images = false;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			m_pZeissDataCollection->m_pStage->stage_go_to_y(y); 
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			while (m_pZeissDataCollection->m_pStage->is_stage_busy())
				std::this_thread::sleep_for(500ms);


				if (m_bSerialEDScanRegions == false)
					break;
		}


		if (m_bSerialEDScanRegions == false || bBreakScan)
		{
			m_pTimepix->tcp_store_diffraction_images = false;
			serial_ed_store_valid_images_to_disk(_raw_img_vec, imgIndex, validImgCount);
			break;
		}

		region.isRegionScanned = true;

		CWriteFile::GetInstance()->write_params_file(); // update the params file

	}


	m_pZeissControlManager->stage_abort_exec();
	m_pZeissControlManager->do_blank_beam(true);
	m_bSerialEDScanRegions = false;
	m_bEnable_items = true;
	if(!bBreakScan)
		gridRegions.clear();
	PRINT("SERIAL_ED_SCAN_REGION FINISHED");
}


cv::Point CDataCollection::get_2d_stage_coordinates(float _stage_x, float _stage_y)
{
	// float startStageX = (selection->x - canvas->cols / 2) / scalingFactor;

	float scalingFactor = static_cast<float>(_CANVAS_SIZE_) / _GRID_MAX_LENGTH;

	m_StagePosition2D.x = _stage_x * scalingFactor + _CANVAS_SIZE_ * 0.5f;
	m_StagePosition2D.y = _CANVAS_SIZE_ * 0.5f - _stage_y * scalingFactor;

	return m_StagePosition2D;
}





bool CDataCollection::is_on_stem_mode()
{
	return m_pZeissControlManager->is_on_stem_mode();
}

std::vector<my_params> CDataCollection::save_parameters()
{
	PRINTD("\t\t\t\tCDataCollection::save_parameters\n");

	std::vector<my_params> params_to_save;
	
	// Info on the Detector Section
	params_to_save.emplace_back("Operator", m_sOperatorName);
	params_to_save.emplace_back("Date", std::format("{:%Y_%m_%d}", std::chrono::system_clock::now()));
	params_to_save.emplace_back("CrystalName", m_sCrystalName);
	params_to_save.emplace_back("CrystalNum", m_sCrystalNumberID);
	params_to_save.emplace_back("CrystalExtra", m_sCrystalNameExtra);
	params_to_save.emplace_back("FramesNumLimit", m_iNumOfFramesToTake);

	//Info on the Crystal Tracking Section:
	params_to_save.emplace_back("DatasetPath", m_sDatasetPath);
	params_to_save.emplace_back("TrackingImgPath", m_sTrackingImgPath);
	params_to_save.emplace_back("EucentricHeighPath", m_sEucentricHeightPath);
	params_to_save.emplace_back("AngleStart", m_fStartingAng);
	params_to_save.emplace_back("AngleEnd", m_fEndingAng);
	params_to_save.emplace_back("VariableIntervalStart", m_fRecordImgStepStartingAngVariable);
	params_to_save.emplace_back("VariableIntervalEnd", m_fRecordImgStepEndingAngVariable);
	params_to_save.emplace_back("EucentricHeightTiltSteps", m_fEucentricHeightTiltSteps);
	params_to_save.emplace_back("EucentricHeightDeltaZ", m_fEucentricHeightDeltaZ);
	params_to_save.emplace_back("ImageBasedTrackingSteps", m_fRecordImgSteps);
	params_to_save.emplace_back("ImageBasedTrackingStepsVariable", m_fRecordImgStepsVariable);

	// Info on the SerialED Section:
	params_to_save.emplace_back("SerialED_YSteps", m_fSerialED_ysteps);
	params_to_save.emplace_back("SerialED_DMin", m_fSerialED_dmax);
	params_to_save.emplace_back("SerialED_SepMin", m_fSerialED_peakthreshold);
	params_to_save.emplace_back("SerialED_I_Sigma", m_fSerialED_i_sigma);
	params_to_save.emplace_back("SerialED_PeakSize", m_fSerialED_peaksize);
	params_to_save.emplace_back("SerialED_MinPeaks", m_iSerialED_min_num_peaks);

	//params_to_save.emplace_back("Bool_ContinuousRecord", m_bContinuousRecord);
	params_to_save.emplace_back("Bool_ImageBasedRecord", m_bImageBasedRecord);
	//params_to_save.emplace_back("Bool_TimeBasedTrack", m_bTimeBasedTrack);
	//params_to_save.emplace_back("Bool_AngleBasedTrack", m_bAngleBasedTrack);
	params_to_save.emplace_back("Bool_StepwiseTracking", m_bStepwiseRecord);
	params_to_save.emplace_back("Bool_BlankDuringTracking", m_bDoBlankRotation);
	params_to_save.emplace_back("Bool_ImageBasedTrack", m_bImageBasedTrack);
	params_to_save.emplace_back("Bool_ImageBasedMode", m_iImageBasedTrackingMode);
	params_to_save.emplace_back("Bool_LinearMovementTrack", m_bLinearMovementTrack);
	params_to_save.emplace_back("Bool_MouveMouseTest", m_bMoveMouseTest);
	params_to_save.emplace_back("Bool_ReadjustZ", m_bReadjustZValue);
	params_to_save.emplace_back("Bool_CheckEucentricHeight", m_bCheckForZHeight);
	params_to_save.emplace_back("Bool_EnableVariableInterval", m_bVariableRecordSteps);
	params_to_save.emplace_back("Bool_SaveLensParams", m_pZeissControlManager->m_bSaveLensParams);
	params_to_save.emplace_back("Bool_CorrectCalibratrion", m_pZeissDataCollection->m_bCorrectCalibration);

	// TEM's State:
	params_to_save.emplace_back("CALIB_Ill_shift_X", m_fBeamShiftCalibrationX);
	params_to_save.emplace_back("CALIB_Ill_shift_Y", m_fBeamShiftCalibrationY);
	params_to_save.emplace_back("CALIB_Delta_Shift_GUI", m_iCalibrationDeltaGUI);
	params_to_save.emplace_back("CALIB_Delta_Shift", m_fBeamShiftDelta);

	params_to_save.emplace_back("CALIB_Ill_shift_X0", m_fine_calib_X_coefficients.at<double>(0));
	params_to_save.emplace_back("CALIB_Ill_shift_X1", m_fine_calib_X_coefficients.at<double>(1));
	params_to_save.emplace_back("CALIB_Ill_shift_X2", m_fine_calib_X_coefficients.at<double>(2));
	params_to_save.emplace_back("CALIB_Ill_shift_X3", m_fine_calib_X_coefficients.at<double>(3));
	params_to_save.emplace_back("CALIB_Ill_shift_X4", m_fine_calib_X_coefficients.at<double>(4));
	params_to_save.emplace_back("CALIB_Ill_shift_X5", m_fine_calib_X_coefficients.at<double>(5));

	params_to_save.emplace_back("CALIB_Ill_shift_Y0", m_fine_calib_Y_coefficients.at<double>(0));
	params_to_save.emplace_back("CALIB_Ill_shift_Y1", m_fine_calib_Y_coefficients.at<double>(1));
	params_to_save.emplace_back("CALIB_Ill_shift_Y2", m_fine_calib_Y_coefficients.at<double>(2));
	params_to_save.emplace_back("CALIB_Ill_shift_Y3", m_fine_calib_Y_coefficients.at<double>(3));
	params_to_save.emplace_back("CALIB_Ill_shift_Y4", m_fine_calib_Y_coefficients.at<double>(4));
	params_to_save.emplace_back("CALIB_Ill_shift_Y5", m_fine_calib_Y_coefficients.at<double>(5));

	params_to_save.emplace_back("CALIB_Ill_shift_state", m_bBeamShiftCalibrated);
	params_to_save.emplace_back("CALIB_Ill_shift_state_fine", m_bIs_beam_shift_calibrated_fine);
	params_to_save.emplace_back("CALIB_Ill_radius", ImagesInfo::_radius);
	params_to_save.emplace_back("CALIB_BASE_VEC_X_X", m_vBaseX.x);
	params_to_save.emplace_back("CALIB_BASE_VEC_X_Y", m_vBaseX.y);
	params_to_save.emplace_back("CALIB_BASE_VEC_Y_X", m_vBaseY.x);
	params_to_save.emplace_back("CALIB_BASE_VEC_Y_Y", m_vBaseY.y);
	params_to_save.emplace_back("CALIB_AFTER_X_SHIFT_X", m_vBeamShiftAfterX.x);
	params_to_save.emplace_back("CALIB_AFTER_X_SHIFT_Y", m_vBeamShiftAfterX.y);
	params_to_save.emplace_back("CALIB_AFTER_Y_SHIFT_X", m_vBeamShiftAfterY.x);
	params_to_save.emplace_back("CALIB_AFTER_Y_SHIFT_Y", m_vBeamShiftAfterY.y);

	//Searching mode
	params_to_save.emplace_back("SEARCH_Ill_shift_X", m_oSearchingParams.Illumination_shift_vec().x);
	params_to_save.emplace_back("SEARCH_Ill_shift_Y", m_oSearchingParams.Illumination_shift_vec().y);
	params_to_save.emplace_back("SEARCH_Ill_tilt_X", m_oSearchingParams.Illumination_tilt_vec().x);
	params_to_save.emplace_back("SEARCH_Ill_tilt_Y", m_oSearchingParams.Illumination_tilt_vec().y);
	params_to_save.emplace_back("SEARCH_Ill_shift_X_lowmag", m_oSearchingParams.Illumination_shift_vec(MAG_MODE::LOW_MAG_MODE_TEM).x);
	params_to_save.emplace_back("SEARCH_Ill_shift_Y_lowmag", m_oSearchingParams.Illumination_shift_vec(MAG_MODE::LOW_MAG_MODE_TEM).y);

	params_to_save.emplace_back("SEARCH_Ill_shift_X_to_Screen", m_oSearchingParams.Illumination_shift_screen_coords().x);
	params_to_save.emplace_back("SEARCH_Ill_shift_Y_to_Screen", m_oSearchingParams.Illumination_shift_screen_coords().y);

	params_to_save.emplace_back("SEARCH_Img_shift_X", m_oSearchingParams.Image_shift_vec().x);
	params_to_save.emplace_back("SEARCH_Img_shift_Y", m_oSearchingParams.Image_shift_vec().y);
	params_to_save.emplace_back("SEARCH_Img_shift_X_lowmag", m_oSearchingParams.Image_shift_vec(MAG_MODE::LOW_MAG_MODE_TEM).x);
	params_to_save.emplace_back("SEARCH_Img_shift_Y_lowmag", m_oSearchingParams.Image_shift_vec(MAG_MODE::LOW_MAG_MODE_TEM).y);

	params_to_save.emplace_back("SEARCH_Apert_Num", m_oSearchingParams.Aperture_selection_number());
	params_to_save.emplace_back("SEARCH_Img_mode", m_oSearchingParams.Image_mode());
	params_to_save.emplace_back("SEARCH_Img_mode_lowmag", m_oSearchingParams.Image_mode(MAG_MODE::LOW_MAG_MODE_TEM));
	params_to_save.emplace_back("SEARCH_Ill_index", m_oSearchingParams.Illumination_idx());
	params_to_save.emplace_back("SEARCH_Ill_index_lowmag", m_oSearchingParams.Illumination_idx(MAG_MODE::LOW_MAG_MODE_TEM));
	params_to_save.emplace_back("SEARCH_Mag_index", m_oSearchingParams.Magnification_idx());
	params_to_save.emplace_back("SEARCH_Mag_index_lowmag", m_oSearchingParams.Magnification_idx(MAG_MODE::LOW_MAG_MODE_TEM));
	params_to_save.emplace_back("SEARCH_Spec_Mag_index", m_oSearchingParams.Spec_mag_idx());
	params_to_save.emplace_back("SEARCH_Spec_Mag_index_lowmag", m_oSearchingParams.Spec_mag_idx(MAG_MODE::LOW_MAG_MODE_TEM));
	params_to_save.emplace_back("SEARCH_Focus", m_oSearchingParams.get_focus());
	params_to_save.emplace_back("SEARCH_Focus_lowmag", m_oSearchingParams.get_focus(MAG_MODE::LOW_MAG_MODE_TEM));
	params_to_save.emplace_back("SEARCH_C1_Lens", m_oSearchingParams.get_current_C1_lens());
	params_to_save.emplace_back("SEARCH_C2_Lens", m_oSearchingParams.get_current__C2_lens());
	params_to_save.emplace_back("SEARCH_C3_Lens", m_oSearchingParams.get_current__C3_lens());
	params_to_save.emplace_back("SEARCH_OBJECTIVE_Lens", m_oSearchingParams.get_current_objective_lens());
	params_to_save.emplace_back("SEARCH_LensStored", m_oSearchingParams.is_lens_stored());


	//Imaging-Tracking mode
	params_to_save.emplace_back("IMG_Ill_shift_X", m_oImagingParams.Illumination_shift_vec().x);
	params_to_save.emplace_back("IMG_Ill_shift_Y", m_oImagingParams.Illumination_shift_vec().y); 
	params_to_save.emplace_back("IMG_Ill_tilt_X", m_oImagingParams.Illumination_tilt_vec().x);
	params_to_save.emplace_back("IMG_Ill_tilt_Y", m_oImagingParams.Illumination_tilt_vec().y);
	params_to_save.emplace_back("IMG_Ill_shift_X_lowmag", m_oImagingParams.Illumination_shift_vec(MAG_MODE::LOW_MAG_MODE_TEM).x);
	params_to_save.emplace_back("IMG_Ill_shift_Y_lowmag", m_oImagingParams.Illumination_shift_vec(MAG_MODE::LOW_MAG_MODE_TEM).y);

	params_to_save.emplace_back("IMG_Ill_shift_X_to_Screen", m_oImagingParams.Illumination_shift_screen_coords().x);
	params_to_save.emplace_back("IMG_Ill_shift_Y_to_Screen", m_oImagingParams.Illumination_shift_screen_coords().y);

	params_to_save.emplace_back("IMG_Img_shift_X", m_oImagingParams.Image_shift_vec().x);
	params_to_save.emplace_back("IMG_Img_shift_Y", m_oImagingParams.Image_shift_vec().y);
	params_to_save.emplace_back("IMG_Img_shift_X_lowmag", m_oImagingParams.Image_shift_vec(MAG_MODE::LOW_MAG_MODE_TEM).x);
	params_to_save.emplace_back("IMG_Img_shift_Y_lowmag", m_oImagingParams.Image_shift_vec(MAG_MODE::LOW_MAG_MODE_TEM).y);

	params_to_save.emplace_back("IMG_Apert_Num", m_oImagingParams.Aperture_selection_number());
	params_to_save.emplace_back("IMG_Img_mode", m_oImagingParams.Image_mode());
	params_to_save.emplace_back("IMG_Img_mode_lowmag", m_oImagingParams.Image_mode(MAG_MODE::LOW_MAG_MODE_TEM));
	params_to_save.emplace_back("IMG_Ill_index", m_oImagingParams.Illumination_idx());
	params_to_save.emplace_back("IMG_Ill_index_lowmag", m_oImagingParams.Illumination_idx(MAG_MODE::LOW_MAG_MODE_TEM));
	params_to_save.emplace_back("IMG_Mag_index", m_oImagingParams.Magnification_idx());
	params_to_save.emplace_back("IMG_Mag_index_lowmag", m_oImagingParams.Magnification_idx(MAG_MODE::LOW_MAG_MODE_TEM));
	params_to_save.emplace_back("IMG_Spec_Mag_index", m_oImagingParams.Spec_mag_idx());
	params_to_save.emplace_back("IMG_Spec_Mag_index_lowmag", m_oImagingParams.Spec_mag_idx(MAG_MODE::LOW_MAG_MODE_TEM));
	params_to_save.emplace_back("IMG_Focus", m_oImagingParams.get_focus());
	params_to_save.emplace_back("IMG_Focus_lowmag", m_oImagingParams.get_focus(MAG_MODE::LOW_MAG_MODE_TEM));
	params_to_save.emplace_back("IMG_C1_Lens", m_oImagingParams.get_current_C1_lens());
	params_to_save.emplace_back("IMG_C2_Lens", m_oImagingParams.get_current__C2_lens());
	params_to_save.emplace_back("IMG_C3_Lens", m_oImagingParams.get_current__C3_lens());
	params_to_save.emplace_back("IMG_OBJECTIVE_Lens", m_oImagingParams.get_current_objective_lens());
	params_to_save.emplace_back("IMG_LensStored", m_oImagingParams.is_lens_stored());

	//Diffraction Mode
	params_to_save.emplace_back("DIFF_Ill_shift_X", m_oDiffractionParams.Illumination_shift_vec().x);
	params_to_save.emplace_back("DIFF_Ill_shift_Y", m_oDiffractionParams.Illumination_shift_vec().y);
	params_to_save.emplace_back("DIFF_Ill_tilt_X", m_oDiffractionParams.Illumination_tilt_vec().x);
	params_to_save.emplace_back("DIFF_Ill_tilt_Y", m_oDiffractionParams.Illumination_tilt_vec().y);
	params_to_save.emplace_back("DIFF_Ill_shift_X_to_Screen", m_oDiffractionParams.Illumination_shift_screen_coords().x);
	params_to_save.emplace_back("DIFF_Ill_shift_Y_to_Screen", m_oDiffractionParams.Illumination_shift_screen_coords().y);
	params_to_save.emplace_back("DIFF_Img_shift_X", m_oDiffractionParams.Image_shift_vec().x);
	params_to_save.emplace_back("DIFF_Img_shift_Y", m_oDiffractionParams.Image_shift_vec().y);
	params_to_save.emplace_back("DIFF_Apert_Num", m_oDiffractionParams.Aperture_selection_number());
	params_to_save.emplace_back("DIFF_Img_mode", m_oDiffractionParams.Image_mode());
	params_to_save.emplace_back("DIFF_Ill_index", m_oDiffractionParams.Illumination_idx());
	params_to_save.emplace_back("DIFF_Mag_index", m_oDiffractionParams.Magnification_idx());
	params_to_save.emplace_back("DIFF_Spec_Mag_index", m_oDiffractionParams.Spec_mag_idx());
	params_to_save.emplace_back("DIFF_Focus", m_oDiffractionParams.get_focus());
	params_to_save.emplace_back("DIFF_C1_Lens", m_oDiffractionParams.get_current_C1_lens());
	params_to_save.emplace_back("DIFF_C2_Lens", m_oDiffractionParams.get_current__C2_lens());
	params_to_save.emplace_back("DIFF_C3_Lens", m_oDiffractionParams.get_current__C3_lens());
	params_to_save.emplace_back("DIFF_OBJECTIVE_Lens", m_oDiffractionParams.get_current_objective_lens());
	params_to_save.emplace_back("DIFF_LensStored", m_oDiffractionParams.is_lens_stored());


	// SERIAL_ED GRID REGIONS
	int gridRegionsSize = gridRegions.size();
	params_to_save.emplace_back("SERIALED_GRIDREGIONS_SIZE", gridRegionsSize);
	for (int i = 0; i < gridRegionsSize; i++)
	{
		params_to_save.emplace_back(std::to_string(i) + "_" + "GRID_X", gridRegions[i].gridRegion.x);
		params_to_save.emplace_back(std::to_string(i) + "_" + "GRID_Y", gridRegions[i].gridRegion.y);
		params_to_save.emplace_back(std::to_string(i) + "_" + "GRID_WIDTH", gridRegions[i].gridRegion.width);
		params_to_save.emplace_back(std::to_string(i) + "_" + "GRID_HEIGHT", gridRegions[i].gridRegion.height);
		params_to_save.emplace_back(std::to_string(i) + "_" + "IS_REGION_SCANNED", gridRegions[i].isRegionScanned);
		params_to_save.emplace_back(std::to_string(i) + "_" + "GRID_ANGLE", gridRegions[i].angle);
		params_to_save.emplace_back(std::to_string(i) + "_" + "GRID_STAGE_Z", gridRegions[i].stage_z);
	}





	return params_to_save;
}

std::vector<my_params> CDataCollection::save_tracking_data()
{
	PRINTD("\t\t\t\tCDataCollection::save_tracking_data\n");

	std::vector<my_params> params_to_save;
	int iSize = m_oImageBasedVec.size();
	params_to_save.emplace_back("DATA_SIZE", iSize);

	params_to_save.emplace_back("ROTATION_SPEED", m_pZeissControlManager->get_stage_tilt_speed());
	params_to_save.emplace_back("STAGE_POS_X", m_pZeissControlManager->get_stage_x() * 1000000.0f);
	params_to_save.emplace_back("STAGE_POS_Y", m_pZeissControlManager->get_stage_y() * 1000000.0f);
	params_to_save.emplace_back("STAGE_POS_Z", m_pZeissControlManager->get_stage_z() * 1000000.0f);


	for (int i = 0; i < iSize; i++)
	{
		params_to_save.emplace_back(std::to_string(i) + "_" + "FILENAME", m_oImageBasedVec[i]._sFileName);
		params_to_save.emplace_back(std::to_string(i) + "_" + "WINDOWTITLE", m_oImageBasedVec[i]._sWindowTitle);
		
		params_to_save.emplace_back(std::to_string(i) + "_" + "IMAGE_ANGLE", m_oImageBasedVec[i]._fImageAngle);
		params_to_save.emplace_back(std::to_string(i) + "_" + "TEM_MAGNIFICATION", m_oImageBasedVec[i]._fTEMMagnification);
		params_to_save.emplace_back(std::to_string(i) + "_" + "ZHEIGHT_VAL", m_oImageBasedVec[i]._fZHeightVal);
		params_to_save.emplace_back(std::to_string(i) + "_" + "IMAGE_TIME", m_oImageBasedVec[i]._uiImgTime);
		
		params_to_save.emplace_back(std::to_string(i) + "_" + "POS_X", m_oImageBasedVec[i]._iPosX);
		params_to_save.emplace_back(std::to_string(i) + "_" + "POS_Y", m_oImageBasedVec[i]._iPosY);
			
		params_to_save.emplace_back(std::to_string(i) + "_" + "IS_VALID", m_oImageBasedVec[i]._bIsImgValid);
		params_to_save.emplace_back(std::to_string(i) + "_" + "SAVE_VISUALS", m_oImageBasedVec[i]._bSaveVisuals);
		params_to_save.emplace_back(std::to_string(i) + "_" + "IS_LOWMAG_IMG", m_oImageBasedVec[i]._bIsLowMagImg);

	}
		
	
	return params_to_save;
}

void CDataCollection::restore_parameters()
{
	PRINTD("\t\t\t\tCDataCollection::restore_parameters\n");


	auto pFileWriter = CWriteFile::GetInstance();

	// Info on the Detector Section
	pFileWriter->restore_param("Operator", m_sOperatorName);
	pFileWriter->restore_param("Date", m_sCollectionDate);
	pFileWriter->restore_param("CrystalName", m_sCrystalName);
	pFileWriter->restore_param("CrystalNum", m_sCrystalNumberID);
	pFileWriter->restore_param("CrystalExtra", m_sCrystalNameExtra);
	pFileWriter->restore_param("FramesNumLimit", m_iNumOfFramesToTake);

	//Info on the Crystal Tracking Section:
	pFileWriter->restore_param("DatasetPath", m_sDatasetPath);
	pFileWriter->restore_param("TrackingImgPath", m_sTrackingImgPath);
	pFileWriter->restore_param("EucentricHeighPath", m_sEucentricHeightPath);
	pFileWriter->restore_param("AngleStart", m_fStartingAng);
	pFileWriter->restore_param("AngleEnd", m_fEndingAng);
	pFileWriter->restore_param("VariableIntervalStart", m_fRecordImgStepStartingAngVariable);
	pFileWriter->restore_param("VariableIntervalEnd", m_fRecordImgStepEndingAngVariable);
	pFileWriter->restore_param("EucentricHeightTiltSteps", m_fEucentricHeightTiltSteps);
	pFileWriter->restore_param("EucentricHeightDeltaZ", m_fEucentricHeightDeltaZ);
	pFileWriter->restore_param("ImageBasedTrackingSteps", m_fRecordImgSteps);
	pFileWriter->restore_param("ImageBasedTrackingStepsVariable", m_fRecordImgStepsVariable);

	// Info on the SerialED Section:
	pFileWriter->restore_param("SerialED_YSteps", m_fSerialED_ysteps);
	pFileWriter->restore_param("SerialED_DMin", m_fSerialED_dmax);
	pFileWriter->restore_param("SerialED_SepMin", m_fSerialED_peakthreshold);
	pFileWriter->restore_param("SerialED_I_Sigma", m_fSerialED_i_sigma);
	pFileWriter->restore_param("SerialED_PeakSize", m_fSerialED_peaksize);
	pFileWriter->restore_param("SerialED_MinPeaks", m_iSerialED_min_num_peaks);

	//pFileWriter->restore_param("Bool_ContinuousRecord", m_bContinuousRecord);
	pFileWriter->restore_param("Bool_ImageBasedRecord", m_bImageBasedRecord);
	//pFileWriter->restore_param("Bool_TimeBasedTrack", m_bTimeBasedTrack);
	//pFileWriter->restore_param("Bool_AngleBasedTrack", m_bAngleBasedTrack);
	pFileWriter->restore_param("Bool_StepwiseTracking", m_bStepwiseRecord);
	pFileWriter->restore_param("Bool_BlankDuringTracking", m_bDoBlankRotation);
	pFileWriter->restore_param("Bool_ImageBasedTrack", m_bImageBasedTrack);
	pFileWriter->restore_param("Bool_ImageBasedMode", reinterpret_cast<int&>(m_iImageBasedTrackingMode));
	pFileWriter->restore_param("Bool_LinearMovementTrack", m_bLinearMovementTrack);
	pFileWriter->restore_param("Bool_MouveMouseTest", m_bMoveMouseTest);
	pFileWriter->restore_param("Bool_ReadjustZ", m_bReadjustZValue);
	pFileWriter->restore_param("Bool_CheckEucentricHeight", m_bCheckForZHeight);
	pFileWriter->restore_param("Bool_EnableVariableInterval", m_bVariableRecordSteps);
	pFileWriter->restore_param("Bool_SaveLensParams", m_pZeissControlManager->m_bSaveLensParams);
	pFileWriter->restore_param("Bool_CorrectCalibratrion", m_pZeissDataCollection->m_bCorrectCalibration);

	// TEM's State:
	pFileWriter->restore_param("CALIB_Ill_shift_X", m_fBeamShiftCalibrationX);
	pFileWriter->restore_param("CALIB_Ill_shift_Y", m_fBeamShiftCalibrationY);
	pFileWriter->restore_param("CALIB_Delta_Shift_GUI", m_iCalibrationDeltaGUI);
	pFileWriter->restore_param("CALIB_Delta_Shift", m_fBeamShiftDelta);

	pFileWriter->restore_param("CALIB_Ill_shift_X0", m_fine_calib_X_coefficients.at<double>(0));
	pFileWriter->restore_param("CALIB_Ill_shift_X1", m_fine_calib_X_coefficients.at<double>(1));
	pFileWriter->restore_param("CALIB_Ill_shift_X2", m_fine_calib_X_coefficients.at<double>(2));
	pFileWriter->restore_param("CALIB_Ill_shift_X3", m_fine_calib_X_coefficients.at<double>(3));
	pFileWriter->restore_param("CALIB_Ill_shift_X4", m_fine_calib_X_coefficients.at<double>(4));
	pFileWriter->restore_param("CALIB_Ill_shift_X5", m_fine_calib_X_coefficients.at<double>(5));
	pFileWriter->restore_param("CALIB_Ill_shift_Y0", m_fine_calib_Y_coefficients.at<double>(0));
	pFileWriter->restore_param("CALIB_Ill_shift_Y1", m_fine_calib_Y_coefficients.at<double>(1));
	pFileWriter->restore_param("CALIB_Ill_shift_Y2", m_fine_calib_Y_coefficients.at<double>(2));
	pFileWriter->restore_param("CALIB_Ill_shift_Y3", m_fine_calib_Y_coefficients.at<double>(3));
	pFileWriter->restore_param("CALIB_Ill_shift_Y4", m_fine_calib_Y_coefficients.at<double>(4));
	pFileWriter->restore_param("CALIB_Ill_shift_Y5", m_fine_calib_Y_coefficients.at<double>(5));
	pFileWriter->restore_param("CALIB_Ill_shift_state", m_bBeamShiftCalibrated);
	pFileWriter->restore_param("CALIB_Ill_shift_state_fine", m_bIs_beam_shift_calibrated_fine);
	pFileWriter->restore_param("CALIB_Ill_radius", ImagesInfo::_radius);
	pFileWriter->restore_param("CALIB_BASE_VEC_X_X", m_vBaseX.x);
	pFileWriter->restore_param("CALIB_BASE_VEC_X_Y", m_vBaseX.y);
	pFileWriter->restore_param("CALIB_BASE_VEC_Y_X", m_vBaseY.x);
	pFileWriter->restore_param("CALIB_BASE_VEC_Y_Y", m_vBaseY.y);
	pFileWriter->restore_param("CALIB_AFTER_X_SHIFT_X", m_vBeamShiftAfterX.x);
	pFileWriter->restore_param("CALIB_AFTER_X_SHIFT_Y", m_vBeamShiftAfterX.y);
	pFileWriter->restore_param("CALIB_AFTER_Y_SHIFT_X", m_vBeamShiftAfterY.x);
	pFileWriter->restore_param("CALIB_AFTER_Y_SHIFT_Y", m_vBeamShiftAfterY.y);

	m_oSearchingParams.restore_param("SEARCH_");
	m_oImagingParams.restore_param("IMG_");
	m_oDiffractionParams.restore_param("DIFF_");


	int gridRegionsSize = 0;
	pFileWriter->restore_param("SERIALED_GRIDREGIONS_SIZE", gridRegionsSize);
	gridRegions.clear();
	for (int i = 0; i < gridRegionsSize; i++)
	{
		GridRegions gridRegion;
		pFileWriter->restore_param(std::to_string(i) + "_" + "GRID_X", gridRegion.gridRegion.x);
		pFileWriter->restore_param(std::to_string(i) + "_" + "GRID_Y", gridRegion.gridRegion.y);
		pFileWriter->restore_param(std::to_string(i) + "_" + "GRID_WIDTH", gridRegion.gridRegion.width);
		pFileWriter->restore_param(std::to_string(i) + "_" + "GRID_HEIGHT", gridRegion.gridRegion.height);
		pFileWriter->restore_param(std::to_string(i) + "_" + "IS_REGION_SCANNED", gridRegion.isRegionScanned);
		pFileWriter->restore_param(std::to_string(i) + "_" + "GRID_ANGLE", gridRegion.angle);
		pFileWriter->restore_param(std::to_string(i) + "_" + "GRID_STAGE_Z", gridRegion.stage_z);
		gridRegions.push_back(gridRegion);
	}
}

void CDataCollection::restore_tracking_data()
{
	PRINTD("\t\t\t\tCDataCollection::restore_tracking_data\n");
	
	std::map<std::string, std::string> configMap;
	auto pFileWriter = CWriteFile::GetInstance();

	std::string tracking_data_file = m_sTrackingImgPath + "tracking_file.trck";
	pFileWriter->read_params_file(tracking_data_file, &configMap);
	if (configMap.empty())
	{
		printf("DELETE LATER, NO TRACKING DATA TO RESTORE!\n");
		return;
	}


	int iSize = 0;
	pFileWriter->restore_param("DATA_SIZE", iSize, "", &configMap);
	
	m_oImageBasedVec.clear();
	for (int i = 0; i < iSize; i++)
	{
		ImagesInfo imgInf;
		pFileWriter->restore_param("FILENAME", imgInf._sFileName, std::to_string(i) + "_", &configMap);
		pFileWriter->restore_param("WINDOWTITLE", imgInf._sWindowTitle, std::to_string(i) + "_", &configMap);

		pFileWriter->restore_param("IMAGE_ANGLE", imgInf._fImageAngle, std::to_string(i) + "_", &configMap);
		pFileWriter->restore_param("TEM_MAGNIFICATION", imgInf._fTEMMagnification, std::to_string(i) + "_", &configMap);
		pFileWriter->restore_param("ZHEIGHT_VAL", imgInf._fZHeightVal, std::to_string(i) + "_", &configMap);
		pFileWriter->restore_param("IMAGE_TIME", imgInf._uiImgTime, std::to_string(i) + "_", &configMap);
		pFileWriter->restore_param("POS_X", imgInf._iPosX, std::to_string(i) + "_", &configMap);
		pFileWriter->restore_param("POS_Y", imgInf._iPosY, std::to_string(i) + "_", &configMap);
		
		pFileWriter->restore_param("IS_VALID", imgInf._bIsImgValid, std::to_string(i) + "_", &configMap);
		pFileWriter->restore_param("SAVE_VISUALS", imgInf._bSaveVisuals, std::to_string(i) + "_", &configMap);
		pFileWriter->restore_param("IS_LOWMAG_IMG", imgInf._bIsLowMagImg, std::to_string(i) + "_", &configMap);

		m_oImageBasedVec.push_back(imgInf);
	}
	do_fill_crystal_coordinates_vec();
	do_prepare_data_for_tracking();
}

CDataCollection* CDataCollection::GetInstance()
{
	PRINTD("\t\t\t\tCDataCollection::GetInstance\n");

	if (m_pZeissDataCollection == nullptr)
		m_pZeissDataCollection = new CDataCollection();

	return m_pZeissDataCollection;
	//return m_pZeissDataCollection ? m_pZeissDataCollection : new CDataCollection();
}



void CDataCollection::prepare_for_2d_map()
{
	std::thread tCreate2DMap(&CDataCollection::display_2d_map, this);
	tCreate2DMap.detach();

}

void CDataCollection::prepare_for_serialed_regions_scan()
{
	std::thread tScanMapRegions(&CDataCollection::do_serial_ed_regions_scan, this);
	tScanMapRegions.detach();
}


bool ImagesInfo::_bSuccess = false;
ImgInfoPurpose ImagesInfo::_purpose = STEM_TRACKING;
float ImagesInfo::_radius = 0.0f;
float ImagesInfo::_circle_detection_param = 1.0f;
TEM_SETTING TEMModeParameters::current_parameters = PARAM_SEARCHING;


void TEMModeParameters::SaveCurrentParameters()
{
	auto zeissControl = CTEMControlManager::GetInstance();
	MAG_MODE mag_mode = zeissControl->get_mag_mode();

	if (current_parameters == TEM_SETTING::PARAM_SEARCHING || current_parameters == PARAM_IMAGING || current_parameters == PARAM_DIFFRACTION) {
		aperture_selection_number = zeissControl->get_mis_num();

		magnification_idx = zeissControl->get_magnification_index();
		illumination_idx = zeissControl->get_illumination_index();

		illumination_shift_vec.x = zeissControl->get_illumination_shift_x();
		illumination_shift_vec.y = zeissControl->get_illumination_shift_y();
		set_Illumination_shift_screen_coords(CDataCollection::GetInstance()->get_current_beam_screen_coordinates());
		image_shift_vec.x = zeissControl->get_image_shift_x();
		image_shift_vec.y = zeissControl->get_image_shift_y();

		illumination_tilt_vec.x = zeissControl->get_illumination_tilt_x();
		illumination_tilt_vec.y = zeissControl->get_illumination_tilt_y();
		//objective_stig_vec.x = zeissControl->get_objective_stig_x();
		//objective_stig_vec.y = zeissControl->get_objective_stig_y();

		image_mode = zeissControl->get_image_mode();
		spec_mag_idx = zeissControl->get_spec_mag_index();
		focus = zeissControl->get_focus();


		if (zeissControl->m_bSaveLensParams == true)
		{
			fC1lens = zeissControl->get_C1_lens();
			fC2lens = zeissControl->get_C2_lens();
			fC3lens = zeissControl->get_C3_lens();
			fObjlens = zeissControl->get_objective_lens();

			bLensesStored = true;
		}
		else
			bLensesStored = false; // If we don't store them, or we uncheck the checkbox, we make sure that we don't restore them later on.

	}
	else {

		magnification_idx_lowmag = zeissControl->get_magnification_index();
		illumination_idx_lowmag = zeissControl->get_illumination_index();

		illumination_shift_vec_lowmag.x = zeissControl->get_illumination_shift_x();
		illumination_shift_vec_lowmag.y = zeissControl->get_illumination_shift_y();
		image_shift_vec_lowmag.x = zeissControl->get_image_shift_x();
		image_shift_vec_lowmag.y = zeissControl->get_image_shift_y();
		
		image_mode_lowmag = IMG_MODE::IMAGE_MODE;
		spec_mag_idx_lowmag = zeissControl->get_spec_mag_index();
		focus_lowmag = zeissControl->get_focus();

	}
	

	// AP_FOCUS_DIFFRACTION
	// IDX 0 FOCUS -0.038
	// IDX 1 FOCUS -0.031
	// IDX 2 FOCUS -0.021
 	// IDX 3 FOCUS -0.021
	// IDX 4 FOCUS 0.00	


}

void TEMModeParameters::RestoreCurrentParameters(TEM_SETTING id)
{
	if (id == current_parameters)
		return;

	static auto zeissControl = CTEMControlManager::GetInstance();
	if (zeissControl->is_on_stem_mode() == false)
	{
		MAG_MODE mag_mode = zeissControl->get_mag_mode();
		static auto pDC = CDataCollection::GetInstance();

		if (id == TEM_SETTING::PARAM_SEARCHING || id == PARAM_IMAGING || id == PARAM_DIFFRACTION) {
			zeissControl->simulate_mdf(true, this);

			float currIdx = zeissControl->get_spec_mag_index();
			if (currIdx != spec_mag_idx)
				zeissControl->set_spec_mag_index(spec_mag_idx);

			// Image mode
			zeissControl->set_image_mode(image_mode);

			std::this_thread::sleep_for(500ms);

			// Let's first restore the ill and mag indices followed by the MIS
			zeissControl->set_illumination_index(illumination_idx);
			//zeissControl->set_magnification_index(magnification_idx);


			// Then image shift
			zeissControl->set_image_shift(image_shift_vec.x, image_shift_vec.y);

			// Important: Illumination shift screen coordinates
			pDC->do_set_current_beam_screen_coordinates(illumination_shift_screen_coords.x, illumination_shift_screen_coords.y);

			// Then illumination shift
			pDC->do_set_current_illumination_shift_coordinates(illumination_shift_vec.x, illumination_shift_vec.y);
			zeissControl->set_illumination_shift(illumination_shift_vec.x, illumination_shift_vec.y);

			zeissControl->set_illumination_tilt(illumination_tilt_vec.x, illumination_tilt_vec.y);

			// MIS number
			zeissControl->set_mis_num(aperture_selection_number);


			if (zeissControl->m_bSaveLensParams == true)
			{
				if (bLensesStored == true) // We wouldn't wanna set the lenses to 0.0 or to anything weird.
				{
					if (fC1lens >= 0.0f)
						zeissControl->set_C1_lens(fC1lens);
					else
						printf("C1 lens value is negative, make sure you've had it initialized first.\n");

					std::this_thread::sleep_for(25ms);
					if (fC2lens >= 0.0f)
						zeissControl->set_C2_lens(fC2lens);
					else
						printf("C2 lens value is negative, make sure you've had it initialized first.\n");
					std::this_thread::sleep_for(25ms);

					if(fC3lens >= 0.0f)
						zeissControl->set_C3_lens(fC3lens);
					else
						printf("C3 lens value is negative, make sure you've had it initialized first.\n");
					std::this_thread::sleep_for(25ms);

					if(fObjlens >= 0.0f)
						zeissControl->set_objective_lens(fObjlens);
					else
						printf("OBJ lens value is negative, make sure you've had it initialized first.\n");
					std::this_thread::sleep_for(125ms);

				}
			}

			/*if(pDC->m_bCorrectCalibration)
			{
				std::this_thread::sleep_for(75ms);
				zeissControl->do_calibrate_focus();
				std::this_thread::sleep_for(75ms);
				zeissControl->do_calibrate_magnification();
			}*/

			// Focus
			zeissControl->set_focus(focus);

		}
		else // We're in low mag
		{
			float currIdx = zeissControl->get_spec_mag_index();
			if (currIdx != spec_mag_idx_lowmag)
				zeissControl->set_spec_mag_index(spec_mag_idx_lowmag);


			// Let's first restore the ill and mag indices followed by the MIS
			zeissControl->set_illumination_index(illumination_idx_lowmag);
			//zeissControl->set_magnification_index(magnification_idx);

			// Then image shift
			zeissControl->set_image_shift(image_shift_vec_lowmag.x, image_shift_vec_lowmag.y);

			// Then illumination shift
			pDC->do_set_current_illumination_shift_coordinates(illumination_shift_vec_lowmag.x, illumination_shift_vec_lowmag.y);
			zeissControl->set_illumination_shift(illumination_shift_vec_lowmag.x, illumination_shift_vec_lowmag.y);


			std::this_thread::sleep_for(1000ms);

			// Image mode
			zeissControl->set_image_mode(image_mode_lowmag);

			// Focus
			zeissControl->set_focus(focus_lowmag);
		}
	}
	else
	{
		if (id == PARAM_DIFFRACTION)
			zeissControl->make_beam_parallel();
		else
			zeissControl->make_beam_convergent();
	}




	TEMModeParameters::current_parameters = id;
	std::this_thread::sleep_for(250ms);
}

void TEMModeParameters::restore_param(std::string _customPrefix)
{
	auto pWriter = CWriteFile::GetInstance();

	pWriter->restore_param("Ill_shift_X", this->illumination_shift_vec.x, _customPrefix);
	pWriter->restore_param("Ill_shift_Y", this->illumination_shift_vec.y, _customPrefix);
	pWriter->restore_param("Ill_tilt_x", this->illumination_tilt_vec.y, _customPrefix);
	pWriter->restore_param("Ill_tilt_Y", this->illumination_tilt_vec.y, _customPrefix);
	pWriter->restore_param("Ill_shift_X_lowmag", this->illumination_shift_vec_lowmag.x, _customPrefix);
	pWriter->restore_param("Ill_shift_Y_lowmag", this->illumination_shift_vec_lowmag.y, _customPrefix);
	pWriter->restore_param("Ill_shift_X_to_Screen", this->illumination_shift_screen_coords.x, _customPrefix);
	pWriter->restore_param("Ill_shift_Y_to_Screen", this->illumination_shift_screen_coords.y, _customPrefix);
	pWriter->restore_param("Img_shift_X", this->image_shift_vec.x, _customPrefix);
	pWriter->restore_param("Img_shift_Y", this->image_shift_vec.y, _customPrefix);
	pWriter->restore_param("Img_shift_X_lowmag", this->image_shift_vec_lowmag.x, _customPrefix);
	pWriter->restore_param("Img_shift_Y_lowmag", this->image_shift_vec_lowmag.y, _customPrefix);
	pWriter->restore_param("Apert_Num", this->aperture_selection_number, _customPrefix);
	pWriter->restore_param("Img_mode", reinterpret_cast<int&>(this->image_mode), _customPrefix);
	pWriter->restore_param("Img_mode_lowmag", reinterpret_cast<int&>(this->image_mode_lowmag), _customPrefix);
	pWriter->restore_param("Ill_index", this->illumination_idx, _customPrefix);
	pWriter->restore_param("Ill_index_lowmag", this->illumination_idx_lowmag, _customPrefix);
	pWriter->restore_param("Mag_index", this->magnification_idx, _customPrefix);
	pWriter->restore_param("Mag_index_lowmag", this->magnification_idx_lowmag, _customPrefix);
	pWriter->restore_param("Spec_Mag_index", this->spec_mag_idx, _customPrefix);
	pWriter->restore_param("Spec_Mag_index_lowmag", this->spec_mag_idx_lowmag, _customPrefix);
	pWriter->restore_param("Focus", this->focus, _customPrefix);
	pWriter->restore_param("Focus_lowmag", this->focus_lowmag, _customPrefix);
	pWriter->restore_param("C1_Lens", this->fC1lens, _customPrefix);
	pWriter->restore_param("C2_Lens", this->fC2lens, _customPrefix);
	pWriter->restore_param("C3_Lens", this->fC3lens, _customPrefix);
	pWriter->restore_param("OBJECTIVE_Lens", this->fObjlens, _customPrefix);
	pWriter->restore_param("LensStored", this->bLensesStored, _customPrefix);

	//CDataCollection::GetInstance()->do_set_current_beam_screen_coordinates(illumination_shift_screen_coords.x, illumination_shift_screen_coords.y);
}

cv::Point2f TEMModeParameters::Illumination_shift_vec(MAG_MODE _mag_mode /*= MAG_MODE::MAG_MODE_TEM*/) const
{
	return _mag_mode == MAG_MODE::MAG_MODE_TEM ? illumination_shift_vec : illumination_shift_vec_lowmag;
}

cv::Point2f TEMModeParameters::Image_shift_vec(MAG_MODE _mag_mode /*= MAG_MODE::MAG_MODE_TEM*/) const
{
	return _mag_mode == MAG_MODE::MAG_MODE_TEM ? image_shift_vec : image_shift_vec_lowmag;
}

cv::Point2f TEMModeParameters::Illumination_tilt_vec(MAG_MODE _mag_mode /*= MAG_MODE::MAG_MODE_TEM*/) const
{
	return _mag_mode == MAG_MODE::MAG_MODE_TEM ? illumination_tilt_vec : cv::Point2f(0,0);

}

int TEMModeParameters::Aperture_selection_number() const
{
	return aperture_selection_number;
}

IMG_MODE TEMModeParameters::Image_mode(MAG_MODE _mag_mode /*= MAG_MODE::MAG_MODE_TEM*/) const
{
	return _mag_mode == MAG_MODE::MAG_MODE_TEM ? image_mode : image_mode_lowmag;
}

float TEMModeParameters::Illumination_idx(MAG_MODE _mag_mode /*= MAG_MODE::MAG_MODE_TEM*/) const
{
	return _mag_mode == MAG_MODE::MAG_MODE_TEM ? illumination_idx : illumination_idx_lowmag;
}

float TEMModeParameters::Magnification_idx(MAG_MODE _mag_mode /*= MAG_MODE::MAG_MODE_TEM*/) const
{
	return _mag_mode == MAG_MODE::MAG_MODE_TEM ? magnification_idx : magnification_idx_lowmag;
}

float TEMModeParameters::Spec_mag_idx(MAG_MODE _mag_mode /*= MAG_MODE::MAG_MODE_TEM*/)
{
	return _mag_mode == MAG_MODE::MAG_MODE_TEM ? spec_mag_idx : spec_mag_idx_lowmag;
}

float TEMModeParameters::get_focus(MAG_MODE _mag_mode /*= MAG_MODE::MAG_MODE_TEM*/)
{
	return _mag_mode == MAG_MODE::MAG_MODE_TEM ? focus : focus_lowmag;
}

cv::Point2f TEMModeParameters::Illumination_shift_screen_coords() const
{
	return illumination_shift_screen_coords;
}

bool TEMModeParameters::is_lens_stored()
{
	return bLensesStored;
}

float TEMModeParameters::get_current_C1_lens()
{
	return fC1lens;
}

float TEMModeParameters::get_current__C2_lens()
{
	return fC2lens;
}

float TEMModeParameters::get_current__C3_lens()
{
	return fC3lens;
}

float TEMModeParameters::get_current_objective_lens()
{
	return fObjlens;
}
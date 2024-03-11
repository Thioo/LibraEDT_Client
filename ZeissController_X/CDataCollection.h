#pragma once

struct TEMModeParameters;
struct TrackingData;
struct ImagesInfo;
struct SFrame;
struct EucentricHeightRegion;
struct Regresion;
enum   TrackingMode;
enum   ImgInfoPurpose;

struct _img_data
{
	i16 data[512 * 512];
	int iCounter;
	float fAngle;
	unsigned int iTotalTime;
};

struct TEMModeParameters
{
private:
	// Parameters that should be saved:
	cv::Point2f illumination_shift_vec;
	cv::Point2f illumination_shift_screen_coords;
	cv::Point2f image_shift_vec;
	int			aperture_selection_number;
	IMG_MODE    image_mode;
	float		illumination_idx;
	float		magnification_idx;
	float       spec_mag_idx;
	float		focus;
	float		focus_lowmag;

	cv::Point2f illumination_shift_vec_lowmag;
	cv::Point2f image_shift_vec_lowmag;
	IMG_MODE    image_mode_lowmag;
	float		illumination_idx_lowmag;
	float		magnification_idx_lowmag;
	float		spec_mag_idx_lowmag;

public:
	static TEM_SETTING current_parameters;

public:

	void SaveCurrentParameters();

	void RestoreCurrentParameters(TEM_SETTING id);

	void restore_param(std::string _customPrefix);

	// getters
	cv::Point2f Illumination_shift_vec(MAG_MODE _mag_mode = MAG_MODE::MAG_MODE_TEM) const;
	cv::Point2f Image_shift_vec(MAG_MODE _mag_mode = MAG_MODE::MAG_MODE_TEM) const;
	int Aperture_selection_number() const;
	IMG_MODE Image_mode(MAG_MODE _mag_mode = MAG_MODE::MAG_MODE_TEM) const;
	float Illumination_idx(MAG_MODE _mag_mode = MAG_MODE::MAG_MODE_TEM) const;
	float Magnification_idx(MAG_MODE _mag_mode = MAG_MODE::MAG_MODE_TEM) const;
	float Spec_mag_idx(MAG_MODE _mag_mode = MAG_MODE::MAG_MODE_TEM);
	float get_focus(MAG_MODE _mag_mode = MAG_MODE::MAG_MODE_TEM);

	cv::Point2f Illumination_shift_screen_coords() const;
	void set_Illumination_shift_screen_coords(cv::Point2f val) { illumination_shift_screen_coords = val; }

	// Constructors


	TEMModeParameters() {
		PRINTD("TEMModeParameters::TEMModeParameters(): To be continued...\n")
		illumination_idx = illumination_idx_lowmag = 4.0f; // In our microscope, 4 is the minimum value for the illumination that is calibrated.
		/*illumination_shift_vec.x = illumination_shift_vec.y = 0.0f;
		image_shift_vec.x = image_shift_vec.y = 0.0f;
		aperture_selection_number = 0;
		image_mode = IMG_MODE::IMAGE_MODE;
		illumination_idx = 0.0f;
		magnification_idx = 0.0f;
		spec_mag_idx = 0.0f;
		focus = 0.0f;
		focus_lowmag = 0.0f;

		illumination_shift_vec_lowmag.x = illumination_shift_vec_lowmag.y = 0.0f;
		image_shift_vec_lowmag.x = image_shift_vec_lowmag.y = 0.0f;
		image_mode_lowmag = IMG_MODE::IMAGE_MODE;
		illumination_idx_lowmag = 0.0f;
		magnification_idx_lowmag = 0.0f;
		spec_mag_idx_lowmag = 0.0f;*/
	}
};



class CDataCollection
{
public:

	CTimer					m_oTrackingTimer;
	CTimer					m_oRecordTimer;
	CDialogEx*				m_pDlg;

	void fine_beam_shift_calibration(int order = 4);
	cv::Mat poly_fit(std::vector<cv::Point2f>& points, unsigned int order);
	
private:
	static CDataCollection*	m_pZeissDataCollection;
	CTEMControlManager*		m_pZeissControlManager;
	CTimepix*				m_pTimepix;
	CZeissStage*			m_pStage;	
	CWriteFile*				m_pFile;

	std::map<unsigned int, CVec> m_oTimerBasedMap;
	std::map<unsigned int, CVec> m_oAngleBasedMap;

	std::vector<ImagesInfo>	m_oImageBasedVec;
	std::vector<CVec>		m_oContinuousRecordVec;
	
	std::vector<ImagesInfo>	m_oEucentricHeightsFirstImgsVec;
	std::vector<ImagesInfo>	m_oEucentricHeightsSecondImgsVec;

	//Contains information about diffraction frames. vector.at(0) usually is the livestream img
	std::vector<SFrame>		m_oDiffracctionFrames; 

	//Contains information about starting and ending angle of a "region", together with its estimated Z value
	std::vector<EucentricHeightRegion>	m_oFirstRegionVec;
	std::vector<EucentricHeightRegion>	m_oSecondRegionVec;
	


	cv::Point2f m_vStartingStagePos; // initial stage pos before recording, so that we can put it there again when about to track
	float m_fRecordSTEMMagnification;// idem for magnification, record and track have to be done on the same conditions (magnification, etc.)
	float m_fWorkingStageTSpeed;
	float m_fWorkingStageXYSpeed;

	float m_ill_shift_x_lower_limit;
	float m_ill_shift_x_upper_limit;
	float m_ill_shift_y_lower_limit;
	float m_ill_shift_y_upper_limit;
	CDataCollection();
	
	cv::Point2f m_vBaseX;
	cv::Point2f m_vBaseY;
	cv::Point2f m_vCurrentBeamPosition;
	cv::Point2f m_vBeamShiftAfterX;
	cv::Point2f m_vBeamShiftAfterY;
	cv::Point2f m_vCurrentIlluminationShift;
	cv::Point2f m_targetPosNew;
	float		m_fBeamShiftDelta;
	float		m_fBeamShiftCalibrationX;
	float		m_fBeamShiftCalibrationY;
	bool		m_bBeamShiftCalibrated;



	double      m_fBeamShiftCalibrationY0;
	double      m_fBeamShiftCalibrationY1;
	double      m_fBeamShiftCalibrationY2;
	double      m_fBeamShiftCalibrationY3;
	double      m_fBeamShiftCalibrationY4;
	double      m_fBeamShiftCalibrationY5;
	bool		m_bIs_beam_shift_calibrated_fine;

	bool		m_bCanStartCollecting;

public:
	cv::Mat						m_fine_calib_X_coefficients;
	cv::Mat						m_fine_calib_Y_coefficients;
	cv::Point2f					m_vShiftOffset;
	cv::Point2f					m_vShiftOffset_rotation;
	std::vector<cv::Point>		m_oCrystalPathCoordinates;
	std::vector<TrackingData>	m_oTrackingDataVec;
	std::vector<cv::Point>		m_oCrystalPathCoordinatesSwapped;
	std::vector<_img_data>		_raw_img_vec;
	_img_data					raw_img;
	ILL_MODE					m_illumination_mode;

	float m_fStartingAng;
	float m_fEndingAng;
	float m_fEucentricHeightTiltSteps;
	float m_fEucentricHeightDeltaZ;
	float m_fRecordImgSteps;
	float m_fRecordImgStepsVariable;
	float m_fRecordImgStepStartingAngVariable;
	float m_fRecordImgStepEndingAngVariable;
	bool m_bContinuousRecord;
	bool m_bImageBasedRecord;
	
	bool m_bTimeBasedTrack;
	bool m_bAngleBasedTrack;
	bool m_bImageBasedTrack;
	bool m_bLinearMovementTrack;

	bool m_bMoveMouseTest;
	bool m_bReadjustZValue;
	bool m_bDo_fine_beam_shift_calibration;
	bool m_bCheckForZHeight;
	bool m_bWaitAfterRotation;
	bool m_bDoBlankRotation;
	bool m_bSingleRun;


	bool m_bKeepThreadRunning;
	bool m_bOnDataCollection; // for threading
	bool m_bOnTracking;
	bool m_bOnRecording;
	bool m_bOnRotateRequest;

	int  m_iCalibrationDeltaGUI;
	int  m_iNumOfFramesToTake;
	TrackingMode  m_iImageBasedTrackingMode;
	POINT		  m_CorrectSpotPosition;
	std::string   m_sDatasetPath;
	std::string	  m_sRawImagePath;
	std::string	  m_sBeamCalibrationPath;
	std::string	  m_sTrackingImgPath;
	std::string   m_sEucentricHeightPath;
	std::string	  m_sOperatorName;
	std::string   m_sCrystalName;
	std::string   m_sCrystalNumberID;
	std::string   m_sCrystalNumberID_cpy;
	std::string	  m_sCrystalNameExtra;
	std::string   m_sCollectionDate;
	
	TEMModeParameters m_oSearchingParams;
	TEMModeParameters m_oImagingParams;
	TEMModeParameters m_oDiffractionParams;

	void set_control_manager(CTEMControlManager* _pCtrlMgr);
	void Test();
	void display_images_and_calculate_z_value();
	void display_images_and_create_tracking_data_tem();
	void display_images_and_create_tracking_data_stem();
	double angleBetweenVectors(cv::Point2d v1, cv::Point2d v2);
	void do_calibrate_beam_shift_tem_ex();
	cv::Point2f rotate_x_degrees(cv::Point2f toRotate, float x = 45.0f, bool bClockwise = false);
	cv::Point2f estimate_current_beam_coordinates();
	void do_beam_shift_at_coordinates(cv::Point2f& _targetPos, cv::Point2f* pStartingPos, cv::Point2f& _ill_shift_vec_init, cv::Point2f& _ill_shift_vec_final, bool bShiftBeam = false);
	void do_beam_shift_at_coordinates_optimized(cv::Point2f& _targetPos, cv::Point2f& _ill_shift_vec_final, bool bShiftBeam = false);
	void do_beam_shift_at_coordinates_alternative(cv::Point2f& _targetPos, bool bShiftBeam = false);
	void do_calibrate_beam_shift_tem();
	float get_beam_calibration_y();
	float get_beam_calibration_x();
	bool is_data_collection_direction_positive(); // True = Positive ( - to +), False = Negative ( + to -)

private:
	
	void acquire_image_for_z_height(std::string& oFileName, unsigned int& iNameIndex, std::vector<ImagesInfo>& _imgVec, ImagesInfo& _imgInfo);
	void do_fill_eucentric_height_regions(std::vector<ImagesInfo>& _imgVec, std::vector<EucentricHeightRegion>& _regionVec);
	void do_find_eucentric_height();
	void do_find_eucentric_height_single_run();
	void do_find_eucentrigh_height_double_run();
	void do_calculate_z_value(std::vector<ImagesInfo>& _imgInfoFirst, std::vector<ImagesInfo>& _imgInfoSecond, std::vector<EucentricHeightRegion>& _imgRegionFirst, std::vector<EucentricHeightRegion>& _imgRegionSecond);

	void do_record_crystal_coordinates();
	void do_readjust_z_value();
	void do_continuous_record_stem(CTimer& oTimer);
	void do_image_based_record_stem();
	
	void do_prepare_data_for_tracking();
	void do_track_crystal_coordinates();
	void do_crystal_tracking_correction();
	void do_image_based_record_tem();
	void do_image_based_record_tem_steps();
	void do_angle_based_continuous_track_stem();
	void do_timer_based_continuous_track_stem();
	void do_linear_movement_for_continuous_rec_stem(TrackingMode _Mode);
	void do_image_based_track_tem(float& _t1_angle, TrackingMode _Mode);
	void do_image_based_track_stem2(float& _t1_angle, TrackingMode _Mode);
	void do_image_based_track_stem(int& _t1_time, float& _t1_angle, TrackingMode _Mode);
	bool move_to_point_time_based(cv::Point2f& _start, cv::Point2f& _end, float _duration_ms, std::chrono::steady_clock::time_point& now);
	bool move_to_point_time_based_tem(cv::Point2f& _startingPos, cv::Point2f& _targetPos, float _duration_ms, unsigned int _startingTime, unsigned int _currentTime);
	bool move_to_point_time_based_stem(cv::Point2f& _startingPos, cv::Point2f& _targetPos, float _duration_ms, unsigned int _startingTime, unsigned int _currentTime);
	bool move_to_point_angle_based(cv::Point2f& _start, cv::Point2f& _end, float _duration_ms);

	void do_collect_frames();
	void tcp_do_collect_frames();
	void do_live_stream_collected_frames();
	void do_tilt_backlash_correction(float _fTargetAngle, bool _bPositiveDirection, float _offset = 5.0f, bool steps = false);
	
	void do_save_data_collection_parameters();
	void do_fast_stage_movement_parameters();
	void do_restore_data_collection_parameters();
	
	unsigned int make_time_key(int _iTimekey, int _iMultiple = 25);
	unsigned int make_angle_key(float _fAngle);

	void infinite_loop_for_monitoring();
	void do_move_stage_to_mouse_coordinates();
	bool do_check_for_emergency_exit(DWORD _dwKey = VK_MULTIPLY);
	

public:
	~CDataCollection();

	void do_fill_image_based_vector_tem(ImagesInfo& oImgInfo, CTimer& oTimer, std::string& sFileName, unsigned int& imgCount, bool bDoBlank = false);
	void do_fill_image_based_vector_stem(ImagesInfo& oImgInfo, CTimer& oTimer, std::string& sFileName, unsigned int& imgCount);
	void do_fill_crystal_coordinates_vec();
	void do_find_eucentric_height_regions_ex();
	void do_record_ex();
	void do_track_ex();
	void do_collect_frames_ex();
	void tcp_do_collect_frames_ex();
	void infinite_loop_for_monitoring_ex();
	void do_set_current_beam_screen_coordinates(float x, float y) { 
		printf("Beam coordinates updated to (%.f, %.f)\n", x, y);
		m_vCurrentBeamPosition.x = x; m_vCurrentBeamPosition.y = y; }
	void do_set_current_illumination_shift_coordinates(float x, float y) {
		printf("Illumination shift coordinates updated to (%.f, %.f)\n", x, y);
		m_vCurrentIlluminationShift.x = x; m_vCurrentIlluminationShift.y = y;
	}
	cv::Point2f get_current_beam_screen_coordinates() { return m_vCurrentBeamPosition; }
	bool is_on_stem_mode();
	
	// Saving / Loading parameters
	std::vector<my_params> save_parameters();
	std::vector<my_params> save_tracking_data();
	void restore_parameters();
	void restore_tracking_data();


	static CDataCollection* GetInstance();
	bool m_bEnable_items;
	bool m_bTrackCrystalPath;
	bool m_bPrecession;
	bool m_bVariableRecordSteps;
	bool m_bStepwiseRecord;
};

struct TrackingData
{
	cv::Point2f crystalCoordinates;
	cv::Point2f direction_vec;
	float fAngle;
	unsigned int uiTime;
	float movementSpeed;
};

struct ImagesInfo
{
	// Common Vars:
	std::string		_sFileName;
	std::string		_sWindowTitle;
	float			_fImageAngle;
	float			_fZHeightVal;
	float			_fTEMMagnification;
	unsigned int	_uiImgTime;
	int				_iPosX;
	int				_iPosY;
	cv::Point2f		_ImgShift; //Point<double> _Imgshift;
	bool			_bIsImgValid;
	bool			_bSaveVisuals;
	bool			_bIsShowCrystalPath;
	bool			_bIsLowMagImg;

	// for tem beam calibration
	cv::Point2f		_center;
	static float	_radius;
	static float	_circle_detection_param;

	static ImgInfoPurpose  _purpose; // 0 = default (stem things, eucentric height, record/tracking), 1 = tem beam calibration, 2 = tem record/tracking
	static bool _bSuccess; // if all the images in the data structure have been successfully managed.

	ImagesInfo() {
		Reset();
	}
	void Reset() {
		_sWindowTitle = ""; _sFileName = ""; _fImageAngle = _fZHeightVal = 0.0f; _iPosX = _iPosY = -1; _bSuccess = false; _bIsImgValid = false; 
		_bIsShowCrystalPath = false; _bSaveVisuals = false, _ImgShift.x = _ImgShift.y = 0.0f; _uiImgTime = 0; _bIsLowMagImg = false;
		_center.x = _center.y = -999;
	}
};


struct SFrame
{
	static int iFrameNum;
	std::vector<i16*> imgData;
	std::string sFullpath; // Directory + ImgName
	std::string sDirectory;
	std::string sImgName;

	bool		 bValid; // if image was taken while beam was scanning, we can exclude it
	bool		 bLiveStream;
	float		 fRotSpeed;
	float		 fAngle; // Angle at which the img was taken
	unsigned int iTotalTime;
	unsigned int iTimeOfAcquisition;

	SFrame()
	{
		imgData.reserve(1000);
	}
};
int SFrame::iFrameNum = 0;

struct EucentricHeightRegion
{
	float fRegionStart;			// Starting angle of the region
	float fRegionEnd;			// Ending angle of the region
	float fRegionMidAng;		// Angle between the starting and ending angles. Point from which, we start modifying the z value
	float fGivenZHeight;		// the Z value at which the measurements were taken
	float fCalculatedZHeight;		// Calculated Z value for the region
	unsigned int uiRegionTime;	// Measured time for a the region (used later on for smooth transition)
	bool  bRegionValid;			// Usefull in case there is a useless img

	std::string oFirstImgName;
	std::string oSecondImgName;
	
	EucentricHeightRegion() {
		Reset();
	}
		
	void Reset() {
		fRegionStart = fRegionEnd = fRegionMidAng =  -99.0f;
		fGivenZHeight = fCalculatedZHeight = 0.0f;
		uiRegionTime = 0;
		bRegionValid = false;
	}
};

struct Regresion {

private:

	double x[2];
	double y[2];
	int n;          //número de datos
	double a, b;    //pendiente y ordenada en el origen
	std::vector<double> vec;

public:
	Regresion(double* _x, double* _y, int _n = 2) {
		vec.reserve(2);
		n = _n;
		for (int i = 0; i < n; i++)
		{
			this->x[i] = _x[i];
			this->y[i] = _y[i];

		}
		lineal();
	}

	void lineal() {
		vec.empty();
		double pxy, sx, sy, sx2, sy2;
		pxy = sx = sy = sx2 = sy2 = 0.0;
		for (int i = 0; i < n; i++) {
			sx += x[i];
			sy += y[i];
			sx2 += x[i] * x[i];
			sy2 += y[i] * y[i];
			pxy += x[i] * y[i];
		}
		a = (n * pxy - sx * sy) / (n * sx2 - sx * sx);
		b = (sy - (a * sx)) / n;
	}

	double extrapolate(const float& fX)
	{
		return a * fX + b;
	}


	double correlacion() {
		//valores medios
		double suma = 0.0;
		for (int i = 0; i < n; i++) {
			suma += x[i];
		}
		double mediaX = suma / n;

		suma = 0.0;
		for (int i = 0; i < n; i++) {
			suma += y[i];
		}
		double mediaY = suma / n;
		//coeficiente de correlación
		double pxy, sx2, sy2;
		pxy = sx2 = sy2 = 0.0;
		for (int i = 0; i < n; i++) {
			pxy += (x[i] - mediaX) * (y[i] - mediaY);
			sx2 += (x[i] - mediaX) * (x[i] - mediaX);
			sy2 += (y[i] - mediaY) * (y[i] - mediaY);
		}
		return pxy / sqrt(sx2 * sy2);
	}

};

enum TrackingMode
{
	MODE_TIME_BASED,
	MODE_ANGLE_BASED
};

enum ImgInfoPurpose
{
	STEM_TRACKING = 0,
	TEM_BEAM_CALIB = 1,
	TEM_TRACKING = 2,
};
#pragma once
using namespace APILib;
enum ZeissErrorCode;
enum ILL_MODE;
enum IMG_MODE;
enum MAG_MODE;

struct TEMModeParameters;
class CZeissStage;
class CDataCollection;
/*
	This class is supposed to have functions and methods that we can call to control
	the microscope, for example:
	Reading: Getting the stage coordinates, Beam position, Magnification value, Camera Length, etc...
	Writting: Modifying stage coordinates, magnification value, spot size, etc...
	Executing: Beam blanking, small/large screen lifting, etc...
	
	
*/

class CTEMControlManager
{
private:
	// Singleton class, one and only one instance needed
	static CTEMControlManager* m_pZeissControlManager;
	CTEMControlManager();

	float m_fCurrentTiltAngle;
	bool m_bIsStageRotating;
	bool m_bIsStageBusy;
	float m_fActualEmissionCurrent;
	float m_fStageZ;
	float m_fCameraLength;
	float m_fSpotSize;
	float m_fIllAngle;
private:
	_EMApi*				m_pZeissApi;
	bool				m_bInitialised;

public:
	//float fCurrentTitleAngle;

public:
	CZeissStage*		m_pStage;
	CDataCollection*	m_pZeissDataCollection;
	CImageManager*		m_pImageManager;
	bool				m_bSaveLensParams;
	bool Initialised() const {
		if (m_bInitialised == false)
		{
#ifndef _DEBUGGING_
			PRINT("API Not Initialied!");
#endif
		}
		return m_bInitialised;
	}

private:
	// Api Setup methods
	bool InitializeApi();
	bool PatchLicense();
	void CreateConsole();

	// Private Api control methods
	void PrintReturnType(const VARIANT& _var);
	void PrintErrorMsg(const ZeissErrorCode& _zErrCode);
	void GetLastErrorPrint();


public:
	void ShowAboutBox();

	// General
	ILL_MODE   get_illumination_mode();
	IMG_MODE   get_image_mode();
	MAG_MODE   get_mag_mode();
	void       set_image_mode(IMG_MODE mode);
	void       set_mag_mode(MAG_MODE mode);
	float get_stem_magnification();
	void  set_stem_magnification(float _fMag);
	void set_magnification_index(float _fIndex);
	void set_spec_mag_index(float _fIndex);
	void set_illumination_index(float _fIndex);
	float get_tem_magnification();
	float get_magnification_index();
	float get_spec_mag_index();
	float get_illumination_index();
	float get_illumination_angle();
	float get_spot_size(bool _bUpdate = false); // in meters
	float get_camera_length(bool _bUpdate = false); // in meters

	int   get_ais_state();
	int   get_ais_num();
	int   get_mis_num();
	void  set_mis_num(int num);

	// Gun
	float get_actual_emission_current(bool _bUpdate = false);
	int   get_actual_emission_step();
	float get_illumination_shift_x();
	float get_illumination_shift_y();
	void  get_illumination_shift_limits(float& _lowerLimitX, float& _upperLimitX, float& _lowerLimitY, float& _upperLimitY);
	float get_image_shift_x();
	float get_image_shift_y();
	float get_illumination_tilt_x();
	float get_illumination_tilt_y();
	float get_illumination_stig_x();
	float get_illumination_stig_y();

	void set_illumination_shift_x(float fShift);
	void set_illumination_shift_y(float fShift);
	void set_illumination_shift(float fShiftX, float fShiftY);
	void set_image_shift_x(float fShift);
	void set_image_shift_y(float fShift);
	void set_image_shift(float fShiftX, float fShiftY);
	void set_illumination_tilt_x(float fTilt);
	void set_illumination_tilt_y(float fTilt);
	void set_illumination_tilt(float fTiltX, float fTiltY);
	void set_target_emission__step(float _fStep);
	

	// Column
	float get_defocus();
	float get_focus();
	void set_focus(float focus_val);

	// Stage | Goniometer
	float get_stage_x(); // in meters
	float get_stage_y(); // in meters
	float get_stage_z(bool _bUpdate = false); // in meters
	float get_stage_tilt_angle(bool _bUpdate = false); // in degrees
	float get_stage_m(); // in degrees
	float get_stage_tilt_speed();
	float get_stage_xy_speed();

	void set_stage_x(float _fX); // in meters
	void set_stage_y(float _fY); // in meters
	void set_stage_z(float _fZ); // in meters
	void set_stage_tilt_angle(float _fAng); // in degrees
	void set_stage_tilt_delta(float _fDelta);
	void set_stage_m(float _fAng); // in degrees
	ZeissErrorCode set_stage_tilt_speed(float _fSpeed);
	ZeissErrorCode set_stage_xy_speed(float _fSpeed);


	bool is_stage_busy(bool _bUpdate = false); // true = busy, false = idle 
	bool is_stage_rotating(bool _bUpdate = false);

	// Beam
	float get_sten_pixel_size(); // in meters, we convert it internally to uM
	int get_beam_state();
	void do_blank_beam(bool _blank);
	void simulate_mdf(bool _bOn, TEMModeParameters* pTEMMode);

	//Scanning
	float get_spot_pos_x();
	float get_spot_pos_y();
	float get_width(); // not working
	float get_height(); // not working
	void set_stem_spot(bool bOn);
	bool is_stem_spot_on();
	bool is_on_stem_mode();
	void freeze_stem_mode(bool _bFreeze);
	void make_beam_parallel();
	void make_beam_convergent();
	bool is_beam_parallel();
	void set_scanning_speed(short _Speed);
	void stage_abort_exec();
	bool is_stem_frozen();

	void set_spot_pos_x(float _x);
	void set_spot_pos_y(float _y);
	void set_spot_pos(float _x, float _y);


	// Camera
	int get_large_screen_status();
	void do_large_screen_up();
	void do_large_screen_down();
	

	// Lenses
	float get_C1_lens();
	float get_C2_lens();
	float get_C3_lens();
	float get_objective_lens();
	void  set_C1_lens(float _fVal);
	void  set_C2_lens(float _fVal);
	void  set_C3_lens(float _fVal);
	void  set_objective_lens(float _fVal);

	void do_calibrate_magnification();
	void do_calibrate_focus();
	void do_calibrate_all();



	ZeissErrorCode close_control(bool _bPrintErrMsg = false);
	ZeissErrorCode update_stage_position(bool _bPrintErrMsg = false);
	ZeissErrorCode move_stage(float _x, float _y, float _z, float _t, float _r, float _m, bool _bPrintErrMsg = false, bool _bWaitIdle = true);
	void acquire_tem_image(std::string& _fileName, unsigned int& _iNameIndex, int _exposureTime = -1);
	ZeissErrorCode acquire_stem_image(std::string& _fileName, unsigned int& _iNameIndex, bool _bTiffFile = true, bool _bPrintErrMsg = false);

	// These functions should be PRIVATE, for now, we'll use it externally for debugging
	ZeissErrorCode zeiss_read(CString _usrMsg, VARIANT& _var, bool _bPrintErrMsg = false, bool _bPrintType = false);
	ZeissErrorCode zeiss_write(CString _usrMsg, VARIANT& _var, bool _bPrintErrMsg = false, bool _bPrintType = false);
	ZeissErrorCode zeiss_execute(CString _usrMsg);
	ZeissErrorCode get_limits(CString _usrMsg, VARIANT& _lowerLimit, VARIANT& _upperLimit);

	static CTEMControlManager* GetInstance();
	~CTEMControlManager();
	friend class CZeissStage;
};

struct CVec
{
	float fX, fY, fZ, fT, fRotSpeed;
	unsigned int iTime;
	
	CVec(float _fX, float _fY, float _fZ, float _fT, float _fSpeed = 0.0f, unsigned int _iTime = 0) : fX(_fX), fY(_fY), fZ(_fZ), fT(_fT), fRotSpeed(_fSpeed), iTime(_iTime) {};
};


enum ZeissErrorCode
{
	API_E_NO_ERROR = 0, // Success

	API_E_GET_TRANSLATE_FAIL = 1000, // Failed to translate parameter into an id

	API_E_GET_AP_FAIL = 1001, // Failed to get analogue value

	API_E_GET_DP_FAIL = 1002, // Failed to get digital value

	API_E_GET_BAD_PARAMETER = 1003, // Parameter supplied is not analogue nor digital

	API_E_SET_TRANSLATE_FAIL = 1004, // Failed to translate parameter into an id

	API_E_SET_STATE_FAIL = 1005, // Failed to set a digital state 

	API_E_SET_FLOAT_FAIL = 1006, // Failed to set a float value

	API_E_SET_FLOAT_LIMIT_LOW = 1007, // Value supplied is too low

	API_E_SET_FLOAT_LIMIT_HIGH = 1008, // Value supplied is too high

	API_E_SET_BAD_VALUE = 1009, // Value supplied is is of wrong type

	API_E_SET_BAD_PARAMETER = 1010, // Parameter supplied is not analogue nor digital

	API_E_EXEC_TRANSLATE_FAIL = 1011, // Failed to translate command into an id

	API_E_EXEC_CMD_FAIL = 1012, // Failed to execute command

	API_E_EXEC_MCF_FAIL = 1013, // Failed to execute file macro

	API_E_EXEC_MCL_FAIL = 1014, // Failed to execute library macro

	API_E_EXEC_BAD_COMMAND = 1015, // Command supplied is not implemented

	API_E_GRAB_FAIL = 1016, // Grab command failed	 

	API_E_GET_STAGE_FAIL = 1017, // Get Stage position failed

	API_E_MOVE_STAGE_FAIL = 1018, // Move Stage position failed

	API_E_NOT_INITIALISED = 1019, 	// API not initialised

	API_E_NOTIFY_TRANSLATE_FAIL = 1020, // Failed to translate parameter to an id

	API_E_NOTIFY_SET_FAIL = 1021, // Set notification failed

	API_E_GET_LIMITS_FAIL = 1022, //  Get limits failed

	API_E_GET_MULTI_FAIL = 1023,	// Get multiple parameters failed

	API_E_SET_MULTI_FAIL = 1024, //	Set multiple parameters failed

	API_E_NOT_LICENSED = 1025, // 	Missing API license

	API_E_NOT_IMPLEMENTED = 1026, // 	Reserved or not implemented

	API_E_GET_USER_NAME_FAIL = 1027, // 	Failed to get user name(Remoting Interface only)

	API_E_GET_USER_IDLE_FAIL = 1028, // Failed to get user idle state(Remoting Interface only)

	API_E_GET_LAST_REMOTING_CONNECT_ERROR_FAIL = 1029, //	Failed to get the last remoting connection error string(Remoting Interface Only)

	API_E_EMSERVER_LOGON_FAILED = 1030, //	Failed to remotely logon to the EM Server(username and password may be incorrect or EM Server is not running or User is already logged on, Remoting only)

	API_E_EMSERVER_START_FAILED = 1031, //	Failed to start the EM Server.This may be because the Server is already running or has an internal error(Remoting Interface only)

	API_E_PARAMETER_IS_DISABLED = 1032, //	The command or parameter is currently disabled(you cannot execute or set it. Remoting Interface only)

};


enum ILL_MODE
{
	TEM_MODE = 0,
	SPOT_MODE = 1,
};

enum IMG_MODE
{
	IMAGE_MODE = 0,
	DIFFRACTION_MODE = 1,
};

enum MAG_MODE
{
	MAG_MODE_TEM = 0,
	LOW_MAG_MODE_TEM = 1

};

enum TEM_SETTING
{
	PARAM_SEARCHING = 1,
	PARAM_IMAGING = 2,
	PARAM_DIFFRACTION = 3,
	
	PARAM_SEARCHING_LOWMAG = 4,
	PARAM_IMAGING_LOWMAG = 5,
};

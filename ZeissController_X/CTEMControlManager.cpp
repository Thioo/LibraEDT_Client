#include "pch.h"

CTEMControlManager* CTEMControlManager::m_pZeissControlManager = nullptr;

CTEMControlManager::CTEMControlManager() : m_pZeissApi(nullptr), m_bInitialised(false), m_pStage(nullptr), m_bSaveLensParams(false)
{
    this->CreateConsole(); Sleep(10);

    PRINTD("\t\tCTEMControlManager::CTEMControlManager\n");
    static bool bDoOnce = false;
    if (bDoOnce == false) // Things that should only be initialized ONCE should go here.
    {
		bDoOnce = true;
        m_pStage = new CZeissStage(this);
       // m_pZeissDataCollection = CDataCollection::GetInstance();
       // m_pZeissDataCollection->SetConteirdrolManager(this);
        m_pImageManager = CImageManager::GetInstance();
    }
    

    if (m_pStage)
    {
        if (this->InitializeApi())
            m_bInitialised = true; // Initialized and licensed
    }
    else
        PRINT("CTEMControlManager::m_pStage = nullptr");

    if (Initialised())
        PRINT("API INITIALIZED & LICENSED");
    
}

bool CTEMControlManager::InitializeApi()
{
    PRINTD("\t\tCTEMControlManager::InitializeApi\n");
    bool bReturn = false;
    HRESULT hResult = CoInitialize(nullptr);
    if (SUCCEEDED(hResult))
    {
        hResult = CoCreateInstance(__uuidof(Api), 0, CLSCTX_ALL, __uuidof(_EMApi), (LPVOID*)&m_pZeissApi);
        if (SUCCEEDED(hResult) && m_pZeissApi)
        {
            if (m_pZeissApi)
            {
                long lReturn = m_pZeissApi->Initialise(L"");
                VARIANT variant;
                ZM(variant);
                m_pZeissApi->GetLastError(&variant);
                int iRet = StrCmpW(variant.bstrVal, L"Failed to create PM OCX control");
                int iRet2 = StrCmpW(variant.bstrVal, L"Control Initialize failed");
				if (lReturn == 0 && iRet != 0 && iRet2 != 0)
				{
					PRINT("Initialisation was succesful!");
					bReturn = true;
				}
				else
                {
                    PRINT("Initialisation was unsuccesful! Please make sure you have the API installed/registered");

                }
                // SysFreeString(variant.bstrVal);
            }
        }
        else
            PRINT("Error@CoCreateInstance");

    }
	return bReturn;
}



void CTEMControlManager::CreateConsole()
{
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    PRINTD("\t\tCTEMControlManager::CreateConsole\n");
}

void CTEMControlManager::PrintReturnType(const VARIANT& _var)
{
    PRINTD("\t\tCTEMControlManager::PrintReturnType\n");
    char	cReturnMsg[MAX_PATH];
    ZM(cReturnMsg);
    wchar_t wReturnMsg[MAX_PATH];
    ZM(wReturnMsg);

   // PRINTD("\t\t***REMINDER***\nFunction for debugging in order to know the variable type.\nDelete the call to this function once the return type is known/understood and treat it approprietely\n\t\t***REMINDER***\n");
    switch (_var.vt)
    {
    case VT_BOOL: // boolean
        sprintf_s(cReturnMsg, "Boolean -> Current Value: %s\n", _var.boolVal ? "true" : "false");
        PRINT(cReturnMsg);
        break;

    case VT_INT: // integer
        sprintf_s(cReturnMsg, "Integer -> Current Value: %d\n", _var.intVal);
        PRINT(cReturnMsg);
        break;

    case VT_R4: // float
        sprintf_s(cReturnMsg, "Float -> Current Value: %0.3f\n", _var.fltVal);
        PRINT(cReturnMsg);
        break;

    case VT_R8: // double
        sprintf_s(cReturnMsg, "Double -> Current Value: %0.5f\n", _var.dblVal);
        PRINT(cReturnMsg);
        break;


    case VT_BSTR: // wstring
        wsprintfW(wReturnMsg, L"WString -> Current Value %s\n", _var.bstrVal);
      //  SysFreeString(_var.bstrVal);
        WPRINT(wReturnMsg);
        break;
       
	case VT_ERROR: // Error
		//PRINT("Error -> Maybe the command doesn't exist? I dunno yet :)");
		break;

    default:
        wsprintfW(wReturnMsg, L"Unmanaged return/variable type number: (%d)\nPlease check from the table/definition", _var.vt);
        WPRINT(wReturnMsg);
        break;
    }
}

void CTEMControlManager::PrintErrorMsg(const ZeissErrorCode& _zErrCode)
{
    PRINTD("\t\tCTEMControlManager::PrintErrorMsg\n");
    switch (_zErrCode)
    {
    case API_E_NO_ERROR:
        PRINT("ZeissErrorCode: Command executed successfully! ");
        break;

    case API_E_GET_TRANSLATE_FAIL:
        PRINT("ZeissErrorCode: Failed to translate parameter into id. (Parameter doesn't exist!)");
        break;

    case API_E_GET_AP_FAIL:
       // PRINT("ZeissErrorCode: Failed to get analogue value");
        break;

    case API_E_GET_DP_FAIL:
        PRINT("ZeissErrorCode: Failed to get digital value");
        break;

    case API_E_GET_BAD_PARAMETER:
        PRINT("ZeissErrorCode: Parameter supplied is not analogue nor digital");
        break;

    case API_E_SET_TRANSLATE_FAIL:
        PRINT("ZeissErrorCode: Failed to translate parameter into an id.");
        break;

    case API_E_SET_STATE_FAIL:
        PRINT("ZeissErrorCode: Failed to set a digital state.");
        break;

    case API_E_SET_FLOAT_FAIL:
        PRINT("ZeissErrorCode: Failed to set a float value.");
        break;

    case API_E_SET_FLOAT_LIMIT_LOW:
        PRINT("ZeissErrorCode: Value supplied is too low.");
        break;

    case API_E_SET_FLOAT_LIMIT_HIGH:
        PRINT("ZeissErrorCode: Value supplied is too high.");
        break;

    case API_E_SET_BAD_VALUE:
        PRINT("ZeissErrorCode: Value supplied is is of wrong type.");
        break;

    case API_E_EXEC_TRANSLATE_FAIL:
        PRINT("ZeissErrorCode: Failed to translate command into an id.");
        break;

    case API_E_EXEC_CMD_FAIL:
        PRINT("ZeissErrorCode: Failed to execute command.");
        break;

    case API_E_EXEC_MCF_FAIL:
        PRINT("ZeissErrorCode: Failed to execute file macro.");
        break;

    case API_E_EXEC_MCL_FAIL:
        PRINT("ZeissErrorCode: Failed to execute library macro.");
        break;

    case API_E_EXEC_BAD_COMMAND:
        PRINT("ZeissErrorCode: Command supplied is not implemented.");
        break;

    case API_E_NOT_INITIALISED:
#ifndef _DEBUGGING_
        PRINT("ZeissErrorCode: API not initialised.");
#endif
        break;

    case API_E_GRAB_FAIL:
        PRINT("ZeissErrorCode: Grab command failed.");
        break;



    default:
        printf("ZeissErrorCode: Unmanaged with error number = %d || %0x\n", _zErrCode, _zErrCode);
        GetLastErrorPrint();
        break;

    }
}

void CTEMControlManager::GetLastErrorPrint()
{
    PRINTD("\t\tCTEMControlManager::GetLastErrorPrint\n");
    VARIANT v;
	wchar_t wReturnMsg[MAX_PATH];
	ZM(wReturnMsg);
    ZM(v);
    m_pZeissApi->GetLastError(&v);
 
    wsprintfW(wReturnMsg, L"GetLastError(&v) returned -> %s\n", v.bstrVal);
    WPRINT(wReturnMsg);
}

void CTEMControlManager::ShowAboutBox()
{
    if (Initialised() == false)
        return;
    PRINTD("\t\tCTEMControlManager::ShowAboutBox\n");
    m_pZeissApi->AboutBox();
}

ILL_MODE CTEMControlManager::get_illumination_mode()
{
	// 0 == TEM
	// 1 == SPOT

	PRINTD("\t\tCTEMControlManager::get_illumination_mode\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_ILL_MODE", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return static_cast<ILL_MODE>(static_cast<int>(_var.fltVal));
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return TEM_MODE;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return TEM_MODE;
}



IMG_MODE CTEMControlManager::get_image_mode()
{
#ifdef _DEBUGGING_
	return DIFFRACTION_MODE;
#endif

	// 0 == IMAGE
	// 1 == DIFFRACTION

	PRINTD("\t\tCTEMControlManager::get_image_mode\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_IMAGE_MODE", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return static_cast<IMG_MODE>(static_cast<int>(_var.fltVal));
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return IMAGE_MODE;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return IMAGE_MODE;
}

MAG_MODE CTEMControlManager::get_mag_mode()
{
	// 0 == NORMAL MAG
	// 1 == LOW MAG

	PRINTD("\t\tCTEMControlManager::get_mag_mode\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_MAG_MODE", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return static_cast<MAG_MODE>(static_cast<int>(_var.fltVal));
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return MAG_MODE_TEM;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return MAG_MODE_TEM;
}


void CTEMControlManager::set_image_mode(IMG_MODE mode)
{
	PRINTD("\t\tCTEMControlManager::set_image_mode\n");
	if (this->get_illumination_mode() != TEM_MODE) // MANUAL
		return;

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = static_cast<float>(mode);
	ZeissErrorCode zeissRetCode = this->zeiss_write("DP_IMAGE_MODE", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}


void CTEMControlManager::set_mag_mode(MAG_MODE mode)
{
	PRINTD("\t\tCTEMControlManager::set_mag_mode\n");
	if (this->get_illumination_mode() != TEM_MODE) // MANUAL
		return;

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = static_cast<float>(mode);
	ZeissErrorCode zeissRetCode = this->zeiss_write("DP_MAG_MODE", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}



float CTEMControlManager::get_stem_magnification()
{
    PRINTD("\t\tCTEMControlManager::GetSTEMMagnification\n");

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_MAG", _var);
    if (zeissRetCode == API_E_NO_ERROR)
        return _var.fltVal;
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

void CTEMControlManager::set_stem_magnification(float _fMag)
{
	PRINTD("\t\tCTEMControlManager::SetSTEMMagnification\n");
    if (_fMag > 20000000.0f)
        _fMag = 20000000.0f;
    if (_fMag < 4000.0f)
        _fMag = 4000.0f;

	VARIANT _var;
	//ZM(_var);
    _var.vt = VT_R4;
    _var.fltVal = _fMag;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_MAG", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_magnification_index(float _fIndex)
{
	PRINTD("\t\tCTEMControlManager::set_magnification_index\n");
	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fIndex;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_MAG_INDEX", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}


void CTEMControlManager::set_spec_mag_index(float _fIndex)
{
	PRINTD("\t\tCTEMControlManager::set_spec_mag_index\n");
	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fIndex;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_SPEC_MAG_INDEX", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}



void CTEMControlManager::set_illumination_index(float _fIndex)
{
	PRINTD("\t\tCTEMControlManager::set_illumination_index\n");
	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fIndex;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_ILL_INDEX", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

float CTEMControlManager::get_tem_magnification()
{
	PRINTD("\t\tCTEMControlManager::GetTEMMagnification\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_TEM_MAG", _var); // 
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

float CTEMControlManager::get_magnification_index()
{
	PRINTD("\t\tCTEMControlManager::get_magnification_index\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_MAG_INDEX", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}


float CTEMControlManager::get_spec_mag_index()
{
	PRINTD("\t\tCTEMControlManager::get_spec_mag_index\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_SPEC_MAG_INDEX", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}


float CTEMControlManager::get_illumination_index()
{
	PRINTD("\t\tCTEMControlManager::get_illumination_index(nm)\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_ILL_INDEX", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

float CTEMControlManager::get_illumination_angle()
{
	PRINTD("\t\tCTEMControlManager::get_illumination_angle(nm)\n");
	
	VARIANT _var;
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_ILL_ANGLE", _var);
	if (zeissRetCode == API_E_NO_ERROR)
	{
		m_fIllAngle = _var.fltVal * 1000000.0f; // convert to urad
		return  m_fIllAngle; // convert to nm
	}
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

float CTEMControlManager::get_spot_size(bool _bUpdate /*= false*/)
{
    PRINTD("\t\tCTEMControlManager::GetSpotSize(nm)\n");
	if (_bUpdate == false)
		return m_fSpotSize;

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_SPOT_SIZE", _var);
    if (zeissRetCode == API_E_NO_ERROR)
	{
		m_fSpotSize = _var.fltVal * 1000000000.0f;
		return  m_fSpotSize; // convert to nm
	}
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

float CTEMControlManager::get_camera_length(bool _bUpdate /*= false*/)
{
    PRINTD("\t\tCTEMControlManager::GetCameraLength(mm)\n");
	if (_bUpdate == false)
		return m_fCameraLength;
    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_CAMERA_LENGTH", _var);
    if (zeissRetCode == API_E_NO_ERROR)
	{
		m_fCameraLength = _var.fltVal * 1000.0f;
		return m_fCameraLength;
	}
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

int CTEMControlManager::get_ais_state()
{
	// 0 = OFF, 1 = AUTO, 2 = MANUAL
	PRINTD("\t\tCTEMControlManager::get_ais_state\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_AIS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return static_cast<int>(_var.fltVal);
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0;
}

int CTEMControlManager::get_ais_num()
{
	PRINTD("\t\tCTEMControlManager::get_ais_num\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_AIS_N", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return static_cast<int>(_var.fltVal);
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0;
}

int CTEMControlManager::get_mis_num()
{
	PRINTD("\t\tCTEMControlManager::get_mis_num\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_MIS_N", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return static_cast<int>(_var.fltVal);
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0;
}

void CTEMControlManager::set_mis_num(int num)
{
	PRINTD("\t\tCTEMControlManager::set_mis_num(...)\n");
	if (this->get_ais_state() != 2) // MANUAL
		return;

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = static_cast<float>(num);
	ZeissErrorCode zeissRetCode = this->zeiss_write("DP_MIS_N", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

float CTEMControlManager::get_actual_emission_current(bool _bUpdate /*= false*/)
{
    PRINTD("\t\tCTEMControlManager::GetEmissionCurrent\n");
	if (_bUpdate == false)
		return m_fActualEmissionCurrent;

    VARIANT _var;
   // //ZM(_var);
	//_var.vt = VT_BSTR; TODO
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_ACTUAL_EMISSION_I", _var);
	if (zeissRetCode == API_E_NO_ERROR)
	{
		m_fActualEmissionCurrent = _var.fltVal * 1000000;
		return m_fActualEmissionCurrent;
	}
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

int CTEMControlManager::get_actual_emission_step()
{
	PRINTD("\t\tCTEMControlManager::GetActualEmissionStep\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_ACTUAL_EMISSION_STEP", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return static_cast<int>(_var.fltVal);
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return -1;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return -1;
}

float CTEMControlManager::get_illumination_shift_x()
{
	PRINTD("\t\tCTEMControlManager::get_illumination_shift_x\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_ILL_SHIFT_X", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}



float CTEMControlManager::get_illumination_shift_y()
{
	PRINTD("\t\tCTEMControlManager::get_illumination_shift_y\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_ILL_SHIFT_Y", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

void CTEMControlManager::get_illumination_shift_limits(float& _lowerLimitX, float& _upperLimitX, float& _lowerLimitY, float& _upperLimitY)
{
	PRINTD("\t\tCTEMControlManager::get_illumination_shift_limits\n");
	VARIANT _varMin, _varMax;

	get_limits("AP_ILL_SHIFT_X", _varMin, _varMax);
	_lowerLimitX = _varMin.fltVal;
	_upperLimitX = _varMax.fltVal;

	get_limits("AP_ILL_SHIFT_Y", _varMin, _varMax);
	_lowerLimitY = _varMin.fltVal;
	_upperLimitY = _varMax.fltVal;
}

void CTEMControlManager::set_illumination_shift_x(float fShift)
{
	PRINTD("\t\tCTEMControlManager::set_illumination_shift_x\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = fShift;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_ILL_SHIFT_X", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_illumination_shift_y(float fShift)
{
	PRINTD("\t\tCTEMControlManager::set_illumination_shift_y\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = fShift;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_ILL_SHIFT_Y", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_illumination_shift(float fShiftX, float fShiftY)
{
	PRINTD("\t\tCTEMControlManager::set_illumination_shift\n");

	this->set_illumination_shift_x(fShiftX);
	this->set_illumination_shift_y(fShiftY);

}


float CTEMControlManager::get_image_shift_x()
{
	PRINTD("\t\tCTEMControlManager::get_image_shift_x\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_IMAGE_SHIFT_X", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}



float CTEMControlManager::get_image_shift_y()
{
	PRINTD("\t\tCTEMControlManager::get_image_shift_y\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_IMAGE_SHIFT_Y", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

float CTEMControlManager::get_illumination_tilt_x()
{
	PRINTD("\t\tCTEMControlManager::get_illumination_tilt_x\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_ILL_TILT_X", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

float CTEMControlManager::get_illumination_tilt_y()
{
	PRINTD("\t\tCTEMControlManager::get_illumination_tilt_y\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_ILL_TILT_Y", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}


float CTEMControlManager::get_illumination_stig_x()
{
	PRINTD("\t\tCTEMControlManager::get_illumination_stig_x\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_ILL_STIG_X", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}



float CTEMControlManager::get_illumination_stig_y()
{
	PRINTD("\t\tCTEMControlManager::get_illumination_stig_y\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_ILL_STIG_Y", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;

}




void CTEMControlManager::set_image_shift_x(float fShift)
{
	PRINTD("\t\tCTEMControlManager::set_image_shift_x\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = fShift;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_IMAGE_SHIFT_X", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_image_shift_y(float fShift)
{
	PRINTD("\t\tCTEMControlManager::set_image_shift_y\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = fShift;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_IMAGE_SHIFT_Y", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_image_shift(float fShiftX, float fShiftY)
{
	PRINTD("\t\tCTEMControlManager::set_image_shift\n");

	this->set_image_shift_x(fShiftX);
	this->set_image_shift_y(fShiftY);

}


void CTEMControlManager::set_illumination_tilt_x(float fTilt)
{
	PRINTD("\t\tCTEMControlManager::set_illumination_tilt_x\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = fTilt;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_ILL_TILT_X", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_illumination_tilt_y(float fTilt)
{
	PRINTD("\t\tCTEMControlManager::set_illumination_tilt_y\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = fTilt;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_ILL_TILT_Y", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;

}

void CTEMControlManager::set_illumination_tilt(float fTiltX, float fTiltY)
{
	PRINTD("\t\tCTEMControlManager::set_illumination_tilt\n");

	this->set_illumination_tilt_x(fTiltX);
	this->set_illumination_tilt_y(fTiltY);
}

void CTEMControlManager::set_target_emission__step(float _fStep)
{
	PRINTD("\t\tCTEMControlManager::SetTargetEmissionStep\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fStep;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_TARGET_EMISSION_STEP", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

float CTEMControlManager::get_defocus()
{
    PRINTD("\t\tCTEMControlManager::GetDefocus\n");

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_DEFOCUS", _var);
    if (zeissRetCode == API_E_NO_ERROR)
        return _var.fltVal;
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

float CTEMControlManager::get_focus()
{
    PRINTD("\t\tCTEMControlManager::GetFocus\n");

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_FOCUS", _var);
    if (zeissRetCode == API_E_NO_ERROR)
        return _var.fltVal;
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}


void CTEMControlManager::set_focus(float focus_val)
{
	PRINTD("\t\tCTEMControlManager::set_focus(nm)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = std::clamp(focus_val, -0.1f, 0.1f);
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_FOCUS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

float CTEMControlManager::get_stage_x()
{
    PRINTD("\t\tCTEMControlManager::GetStageX(meters)\n");

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_STAGE_AT_X", _var);
    if (zeissRetCode == API_E_NO_ERROR)
        return _var.fltVal;
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

float CTEMControlManager::get_stage_y()
{
    PRINTD("\t\tCTEMControlManager::GetStageY(meters)\n");

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_STAGE_AT_Y", _var);
    if (zeissRetCode == API_E_NO_ERROR)
        return _var.fltVal;
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

float CTEMControlManager::get_stage_z(bool _bUpdate /*= false*/)
{
    PRINTD("\t\tCTEMControlManager::GetStageZ(meters)\n");
	if (_bUpdate == false)
		return m_fStageZ;

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_STAGE_AT_Z", _var);
    if (zeissRetCode == API_E_NO_ERROR)
	{
		m_fStageZ = _var.fltVal;
		return m_fStageZ;
	}
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

float CTEMControlManager::get_stage_tilt_angle(bool _bUpdate /*= false*/)
{
    PRINTD("\t\tCTEMControlManager::GetStageT(Deg)\n");

	//if (_bUpdate == false)
	//	return m_fCurrentTiltAngle;


    VARIANT _var;
   // //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_STAGE_AT_T", _var);
    if (zeissRetCode == API_E_NO_ERROR)
	{

		m_fCurrentTiltAngle = _var.fltVal;
		return _var.fltVal;
	}
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}


float CTEMControlManager::get_stage_m()
{
    PRINTD("\t\tCTEMControlManager::GetStageM(Deg)\n");

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_STAGE_AT_M", _var);
    if (zeissRetCode == API_E_NO_ERROR)
        return _var.fltVal;
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

float CTEMControlManager::get_stage_tilt_speed()
{
	PRINTD("\t\tCTEMControlManager::GetStageTSpeed(Deg)\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_STAGE_T_SPEED", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

float CTEMControlManager::get_stage_xy_speed()
{
	PRINTD("\t\tCTEMControlManager::GetStageXYSpeed(Deg)\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_STAGE_XY_SPEED", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

void CTEMControlManager::set_stage_x(float _fX)
{
	PRINTD("\t\tCTEMControlManager::SetStageX(meters)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = std::clamp(_fX, (-1120.75f / 1000000), (1120.75f / 1000000));
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_STAGE_GOTO_X", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
    return;
}

void CTEMControlManager::set_stage_y(float _fY)
{
	PRINTD("\t\tCTEMControlManager::SetStageY(meters)\n");

	VARIANT _var;
	//ZM(_var);
    _var.vt = VT_R4;
	_var.fltVal = std::clamp(_fY, (-1102.75f / 1000000), (1102.75f / 1000000));
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_STAGE_GOTO_Y", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_stage_z(float _fZ)
{
	PRINTD("\t\tCTEMControlManager::SetStageZ(meters)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = std::clamp(_fZ, (-360.00f/1000000), (870.00f/1000000));
	
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_STAGE_GOTO_Z", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
    return;
}

void CTEMControlManager::set_stage_tilt_angle(float _fAng)
{
	PRINTD("\t\tCTEMControlManager::SetStageT(Deg)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = std::clamp(_fAng, -65.0f, 65.0f);
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_STAGE_GOTO_T", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_stage_tilt_delta(float _fDelta)
{
	PRINTD("\t\tCTEMControlManager::SetSTageTDelta(Deg)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fDelta;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_STAGE_DELTA_T", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_stage_m(float _fAng)
{
	PRINTD("\t\tCTEMControlManager::SetStageM(Deg)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fAng;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_STAGE_AT_M", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
        if (zeissRetCode == API_E_NOT_INITIALISED)
            return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

ZeissErrorCode CTEMControlManager::set_stage_tilt_speed(float _fSpeed)
{
	PRINTD("\t\tCTEMControlManager::SetStageTSpeed(%)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fSpeed;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_STAGE_T_SPEED", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return API_E_NO_ERROR;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return zeissRetCode;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return zeissRetCode;
}

ZeissErrorCode CTEMControlManager::set_stage_xy_speed(float _fSpeed)
{
	PRINTD("\t\tCTEMControlManager::SetStageXYSpeed(%)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fSpeed;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_STAGE_XY_SPEED", _var);
	if (zeissRetCode != API_E_NO_ERROR)
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return zeissRetCode;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return zeissRetCode;
}

bool CTEMControlManager::is_stage_busy(bool _bUpdate /*= false*/)
{
	PRINTD("\t\tCTEMControlManager::isStageBusy(bool)\n");
	//if (_bUpdate == false)
	//	return m_bIsStageBusy;

    VARIANT _var;
	//ZM(_var);
    _var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_STAGE_IS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
	{
		m_bIsStageBusy = _var.fltVal < 0.95f ? false : true;
		return  m_bIsStageBusy;
	}
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return true;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
    return true;
}

bool CTEMControlManager::is_stage_rotating(bool _bUpdate /*= false*/)
{
	PRINTD("\t\tCTEMControlManager::isStageRotating(bool)\n");
	//if (_bUpdate == false)
	//	return m_bIsStageRotating;


	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_T_AXIS_IS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
	{
		m_bIsStageRotating = _var.fltVal < 0.95f ? false : true;
		return  m_bIsStageRotating;
	}
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return true;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return true;
}

float CTEMControlManager::get_sten_pixel_size()
{
    PRINTD("\t\tCTEMControlManager::GetPixelSize(uM)\n");

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_PIXEL_SIZE", _var);
    if (zeissRetCode == API_E_NO_ERROR)
        return _var.fltVal * 1000000.0f;
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

int CTEMControlManager::get_beam_state()
{
    // 0 = No | 1 = User Blank | 2 = Camera | 3 = Ext Beam Blank  | 4 = Ext Shutter  | 
    // 5 = Blanker5 | 6 = STEM Frozen  | 7 = Remote CCD  | 8 = Blanker8 
    // 9 = Blanker9 | 10 =Blanker10  | 11 = Blanker11 | 12 = Blanker12
    // 13 = Blanker13 | 14 = Sample Holder | 15 = Gun Valve | 16 = X-Ray
    
    PRINTD("\t\tCTEMControlManager::GetBeamState\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_BLANKED", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return static_cast<int>(_var.fltVal);
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

void CTEMControlManager::do_blank_beam(bool _blank)
{
	PRINTD("\t\tCTEMControlManager::SetBeamBlankState(on/off)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	if (_blank)
		_var.fltVal = 1.0f;
	else
		_var.fltVal = 0.0f;
	ZeissErrorCode zeissRetCode = this->zeiss_write("DP_BLANK_BEAM", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::simulate_mdf(bool _bOn, TEMModeParameters* pTEMMode)
{
	static auto pDC = CDataCollection::GetInstance();
	if(_bOn)
	{
		m_pZeissControlManager->set_mis_num(6);
		m_pZeissControlManager->set_illumination_shift(-1.000f, -1.000f);
	}
	else
	{
		m_pZeissControlManager->set_mis_num(pTEMMode->Aperture_selection_number());
		m_pZeissControlManager->set_illumination_shift(pTEMMode->Illumination_shift_vec().x, pTEMMode->Illumination_shift_vec().y);
	}
}

float CTEMControlManager::get_spot_pos_x()
{
    PRINTD("\t\tCTEMControlManager::GetSpotPosX\n");

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_SPOT_POSN_X", _var);
    if (zeissRetCode == API_E_NO_ERROR)
        return _var.fltVal;
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

float CTEMControlManager::get_spot_pos_y()
{
    PRINTD("\t\tCTEMControlManager::GetSpotPosY\n");

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_SPOT_POSN_Y", _var);
    if (zeissRetCode == API_E_NO_ERROR)
        return _var.fltVal;
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

float CTEMControlManager::get_width()
{
    PRINTD("\t\tCTEMControlManager::GetWidth\n");

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_WIDTH", _var);
    if (zeissRetCode == API_E_NO_ERROR)
        return _var.fltVal;
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}

float CTEMControlManager::get_height()
{
    PRINTD("\t\tCTEMControlManager::GetHeight\n");

    VARIANT _var;
    //ZM(_var);
    ZeissErrorCode zeissRetCode = this->zeiss_read("AP_HEIGHT", _var);
    if (zeissRetCode == API_E_NO_ERROR)
        return _var.fltVal;
    else
    {
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
        PrintReturnType(_var);
    }
    return 0.0f;
}



void CTEMControlManager::set_stem_spot(bool bOn)
{
	PRINTD("\t\tCTEMControlManager::SetSTEMSpot(on/off)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
    if (bOn)
        _var.fltVal = 1.0f;
	ZeissErrorCode zeissRetCode = this->zeiss_write("DP_SPOT", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

bool CTEMControlManager::is_stem_spot_on()
{
	PRINTD("\t\tCTEMFControlManager::isSTEMSpotOn(bool)\n");
	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_SPOT", _var);
	if (zeissRetCode == API_E_NO_ERROR)
	{
		//printf("IsStageBusy: %d - %f\n", static_cast<bool>(_var.fltVal), _var.fltVal);
		return  _var.fltVal < 0.95f ? false : true;;
	}
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return false;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return false;
}

bool CTEMControlManager::is_on_stem_mode()
{
	PRINTD("\t\tCTEMFControlManager::is_on_stem_mode(bool)\n");
	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_STEM_MODE", _var);
	if (zeissRetCode == API_E_NO_ERROR)
	{
		//printf("IsStageBusy: %d - %f\n", static_cast<bool>(_var.fltVal), _var.fltVal);
		return  _var.fltVal < 0.95f ? false : true;;
	}
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return false;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return false;
}

bool CTEMControlManager::is_stem_frozen()
{
	PRINTD("\t\tCTEMFControlManager::isSTEMFrozen(bool)\n");
	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_FROZEN", _var);
	if (zeissRetCode == API_E_NO_ERROR)
	{
		//printf("IsStageBusy: %d - %f\n", static_cast<bool>(_var.fltVal), _var.fltVal);
		return  _var.fltVal < 0.95f ? false : true;;
	}
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return false;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return false;
}


void CTEMControlManager::freeze_stem_mode(bool _bFreeze)
{
	PRINTD("\t\tCTEMControlManager::DoFreezeSTEMMode(on/off)\n");
	if (Initialised() == false)
		return;

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	if (_bFreeze)
		_var.fltVal = 1.0f;
	ZeissErrorCode zeissRetCode = this->zeiss_write("DP_FROZEN", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
    return;
}

void CTEMControlManager::make_beam_parallel()
{
	PRINTD("\t\tCTEMControlManager::DoParalellBeam\n");
	if (Initialised() == false || is_on_stem_mode() == false)
		return;

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
		_var.fltVal = -100.0f;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_COND_FOCUS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::make_beam_convergent()
{
	PRINTD("\t\tCTEMControlManager::DoParalellBeam\n");
	if (Initialised() == false || is_on_stem_mode() == false)
		return;

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = 0.0f;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_COND_FOCUS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

bool CTEMControlManager::is_beam_parallel()
{
	PRINTD("\t\tCTEMControlManager::IsBeamParalell\n");
	
	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_COND_FOCUS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal == -100.0f ? true : false;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return false;
}

void CTEMControlManager::set_scanning_speed(short _Speed)
{
	PRINTD("\t\tCTEMControlManager::SetScanningSpeed\n");

	_Speed = std::clamp<short>(_Speed, 0, 15);
	std::string sMsg = "CMD_SCANRATE";
	sMsg.append(std::to_string(_Speed));

	this->zeiss_execute(sMsg.c_str());
}

void CTEMControlManager::stage_abort_exec()
{
	PRINTD("\t\tCTEMControlManager::stage_abort_exec\n");



	this->zeiss_execute("CMD_STAGE_ABORT");
}

void CTEMControlManager::set_spot_pos_x(float _x)
{
	PRINTD("\t\tCTEMControlManager::SetSpotPosX(pixels)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _x;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_SPOT_POSN_X", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_spot_pos_y(float _y)
{
	PRINTD("\t\tCTEMControlManager::SetSpotPosY(pixels)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _y;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_SPOT_POSN_Y", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_spot_pos(float _x, float _y)
{
	PRINTD("\t\tCTEMControlManager::SetSpotPosY(pixels)\n");
    
    set_spot_pos_x(_x);
    set_spot_pos_y(_y);
}

int CTEMControlManager::get_large_screen_status()
{
	PRINTD("\t\tCTEMControlManager::GetLargeScreenState\n");
    // 0 = Up | 1 = Down | 2 = Moving | 3 = Error

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	ZeissErrorCode zeissRetCode = this->zeiss_read("DP_LARGE_SCREEN", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return static_cast<int>(_var.fltVal);
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0;
}

void CTEMControlManager::do_large_screen_up()
{
	PRINTD("\t\tCTEMControlManager::DoLargeScreenUp\n");

	std::string sMsg = "CMD_LARGE_SCREEN_UP";

	this->zeiss_execute(sMsg.c_str());
}

void CTEMControlManager::do_large_screen_down()
{
	PRINTD("\t\tCTEMControlManager::DoLargeScreenUp\n");

	std::string sMsg = "CMD_LARGE_SCREEN_DOWN";

	this->zeiss_execute(sMsg.c_str());
}

float CTEMControlManager::get_C1_lens()
{
	PRINTD("\t\tCTEMControlManager::get_C1_lens\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_C1_LENS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

float CTEMControlManager::get_C2_lens()
{
	PRINTD("\t\tCTEMControlManager::get_C2_lens\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_C2_LENS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

float CTEMControlManager::get_C3_lens()
{
	PRINTD("\t\tCTEMControlManager::get_C3_lens\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_C3_LENS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

float CTEMControlManager::get_objective_lens()
{
	PRINTD("\t\tCTEMControlManager::get_objective_lens\n");

	VARIANT _var;
	//ZM(_var);
	ZeissErrorCode zeissRetCode = this->zeiss_read("AP_OBJECTIVE_LENS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return _var.fltVal;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return 0.0f;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return 0.0f;
}

void CTEMControlManager::set_C1_lens(float _fVal)
{
	PRINTD("\t\tCTEMControlManager::set_C1_lens(current)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fVal;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_C1_LENS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_C2_lens(float _fVal)
{
	PRINTD("\t\tCTEMControlManager::set_C2_lens(current)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fVal;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_C2_LENS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_C3_lens(float _fVal)
{
	PRINTD("\t\tCTEMControlManager::set_C3_lens(current)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fVal;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_C3_LENS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::set_objective_lens(float _fVal)
{
	PRINTD("\t\tCTEMControlManager::set_objective_lens(current)\n");

	VARIANT _var;
	//ZM(_var);
	_var.vt = VT_R4;
	_var.fltVal = _fVal;
	ZeissErrorCode zeissRetCode = this->zeiss_write("AP_OBJECTIVE_LENS", _var);
	if (zeissRetCode == API_E_NO_ERROR)
		return;
	else
	{
		if (zeissRetCode == API_E_NOT_INITIALISED)
			return;
		PrintErrorMsg(zeissRetCode);
		PrintReturnType(_var);
	}
	return;
}

void CTEMControlManager::do_calibrate_magnification()
{
	PRINTD("\t\tCTEMControlManager::do_calibrate_magnification\n");

	std::string sMsg = "CMD_CAL_MAG";

	this->zeiss_execute(sMsg.c_str());
}

void CTEMControlManager::do_calibrate_focus()
{
	PRINTD("\t\tCTEMControlManager::do_calibrate_focus\n");

	std::string sMsg = "CMD_CAL_FOCUS";

	this->zeiss_execute(sMsg.c_str());
}

void CTEMControlManager::do_calibrate_all()
{
	PRINTD("\t\tCTEMControlManager::do_calibrate_all\n");

	std::string sMsg = "CMD_CAL_ALL";

	this->zeiss_execute(sMsg.c_str());
}

ZeissErrorCode CTEMControlManager::close_control(bool _bPrintErrMsg /*= false*/)
{
    PRINTD("\t\tCTEMControlManager::CloseControl\n");
    if (Initialised() == false)
        return API_E_NOT_INITIALISED;
    ZeissErrorCode zeissRetCode = static_cast<ZeissErrorCode>(m_pZeissApi->ClosingControl());

    if (_bPrintErrMsg)
        PrintErrorMsg(zeissRetCode);
    return zeissRetCode;
}

ZeissErrorCode CTEMControlManager::update_stage_position(bool _bPrintErrMsg /*= false*/)
{
    PRINTD("\t\tCTEMControlManager::GetStagePosition\n");
    ZeissErrorCode zeissRetCode = API_E_NOT_INITIALISED;
    if (Initialised() == false)
        return zeissRetCode;
    
    /*zeissRetCode = static_cast<ZeissErrorCode>(this->m_pZeissApi->GetStagePosition(&m_pStage->vX_m, &m_pStage->vY_m, &m_pStage->vZ_m,
                                                                                    &m_pStage->vT, &m_pStage->vR, &m_pStage->vM));*/
    zeissRetCode = m_pStage->get_stage_coordinates();
  //  m_pStage->ToMicrons(); // Converts the Coordinates from meters to microns (in x,y,z,t,r,m variables)
    if (_bPrintErrMsg)
        PrintErrorMsg(zeissRetCode);

    return zeissRetCode;
}

ZeissErrorCode CTEMControlManager::move_stage(float _x, float _y, float _z, float _t, float _r, float _m, bool _bPrintErrMsg /*= false*/, bool _bWaitIdle /*= true*/)
{
    // For now, we expect the caller to give the coordinates in MICRONS.

    PRINTD("\t\tCTEMControlManager::MoveStage\n");
    ZeissErrorCode zeissRetCode = API_E_NOT_INITIALISED;
    if (Initialised() == false)
        return zeissRetCode;

    if (_bWaitIdle)
    {
        while (this->is_stage_busy())
            Sleep(50);
    }
    
   
    zeissRetCode = m_pStage->move_stage_to_coordinates(_x, _y, _z, _t, _r, _m); // This function will internally convert microns to meters and call API->MoveStage(...)
    if (_bPrintErrMsg)
        PrintErrorMsg(zeissRetCode);

    return zeissRetCode;
}

void CTEMControlManager::acquire_tem_image(std::string& _fileName, unsigned int& _iNameIndex, int _exposureTime)
{
	PRINTD("\t\tCTEMControlManager::acquire_tem_image\n");

	char cBuff[0x10];
	sprintf_s(cBuff, 0x10, "_%03d.tiff", _iNameIndex);


	_fileName.append(cBuff);
	_iNameIndex++;

	CTimepix::GetInstance()->tcp_grab_single_image_from_detector(_fileName, _exposureTime);
}

ZeissErrorCode CTEMControlManager::acquire_stem_image(std::string& _fileName, unsigned int& _iNameIndex, bool _bTiffFile /*= true*/, bool _bPrintErrMsg /*= false*/)
{
    PRINTD("\t\tCTEMControlManager::AcquireSTEMImage\n");
    ZeissErrorCode zeissRetCode = API_E_NOT_INITIALISED;
    if (this->Initialised() == false)
        return zeissRetCode;

    char cBuff[0x10];
    if(_bTiffFile)
		sprintf_s(cBuff, 0x10, "_%03d.tiff", _iNameIndex);
    else
		sprintf_s(cBuff, 0x10, "_%04d.bmp", _iNameIndex);

	_fileName.append(cBuff);
    _iNameIndex++;

    zeissRetCode = static_cast<ZeissErrorCode>(m_pZeissApi->Grab(0, 0, STEMRESX, STEMRESY, -1, _bstr_t(_fileName.c_str())));
    if (_bPrintErrMsg)
        PrintErrorMsg(zeissRetCode);
    return zeissRetCode;
}


ZeissErrorCode CTEMControlManager::zeiss_read(CString _usrMsg, VARIANT& _var, bool _bPrintErrMsg /*= false*/, bool _bPrintType /*= false*/)
{
    PRINTD("\t\tCTEMControlManager::Read\n");
    ZeissErrorCode zeissRetCode = API_E_NOT_INITIALISED;
    if (Initialised() == false)
        return zeissRetCode;

    zeissRetCode = static_cast<ZeissErrorCode>(m_pZeissApi->Get(_bstr_t(_usrMsg), &_var));
    
    if(_bPrintErrMsg)
        this->PrintErrorMsg(zeissRetCode);
   
    if (_bPrintType)
        this->PrintReturnType(_var);

    if (_var.vt == VT_BSTR)
	{
		PRINT("DO NOT FORGET TO FREE THE MEMORY FreeSysString(_var.bstrVal)");
		SysFreeString(_var.bstrVal);
	}
    return zeissRetCode;
}

ZeissErrorCode CTEMControlManager::zeiss_write(CString _usrMsg, VARIANT& _var, bool _bPrintErrMsg /*= false*/, bool _bPrintType /*= false*/)
{
    PRINTD("\t\tCTEMControlManager::Write\n");
    ZeissErrorCode zeissRetCode = API_E_NOT_INITIALISED;
    if (Initialised() == false)
        return zeissRetCode;
   
    zeissRetCode = static_cast<ZeissErrorCode>(m_pZeissApi->Set(_bstr_t(_usrMsg), &_var));

    if (_bPrintErrMsg)
        this->PrintErrorMsg(zeissRetCode);

    if (_bPrintType)
        this->PrintReturnType(_var);

    return zeissRetCode;
}

ZeissErrorCode CTEMControlManager::zeiss_execute(CString _usrMsg)
{
    PRINTD("\t\tCTEMControlManager::Execute\n");
    ZeissErrorCode zeissRetCode = API_E_NOT_INITIALISED;
    if (Initialised() == false)
        return zeissRetCode;

    zeissRetCode = static_cast<ZeissErrorCode>(m_pZeissApi->Execute(_bstr_t(_usrMsg)));
	if (zeissRetCode != API_E_NO_ERROR)
		this->PrintErrorMsg(zeissRetCode);

    return zeissRetCode;
}

ZeissErrorCode CTEMControlManager::get_limits(CString _usrMsg, VARIANT& _lowerLimit, VARIANT& _upperLimit)
{
	PRINTD("\t\tCTEMControlManager::get_limits\n");
	ZeissErrorCode zeissRetCode = API_E_NOT_INITIALISED;
	if (Initialised() == false)
		return zeissRetCode;

	zeissRetCode = static_cast<ZeissErrorCode>(m_pZeissApi->GetLimits(_bstr_t(_usrMsg), &_lowerLimit, &_upperLimit));
	if (zeissRetCode != API_E_NO_ERROR)
		this->PrintErrorMsg(zeissRetCode);
	return zeissRetCode;
}

CTEMControlManager* CTEMControlManager::GetInstance()
{
    PRINTD("\t\tCTEMControlManager::GetInstance\n");
    if (m_pZeissControlManager == nullptr)
        m_pZeissControlManager = new CTEMControlManager();

    return m_pZeissControlManager;
}

CTEMControlManager::~CTEMControlManager()
{
    PRINTD("\t\tCTEMControlManager::~CTEMControlManager - Destructor\n");
    
	if (m_pZeissApi) // Equivalent to SAFE_RELEASE
    {
        m_pZeissApi->ClosingControl();
        //delete m_pZeissApi;
        //m_pZeissApi = nullptr;
    }
    SAFE_RELEASE(m_pStage);
    SAFE_RELEASE(m_pZeissDataCollection);
    SAFE_RELEASE(m_pImageManager);

	Sleep(1000);

}



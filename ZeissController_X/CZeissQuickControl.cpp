#include "pch.h"
#include "CZeissQuickControl.h"

CZeissQuickControl::CZeissQuickControl() : m_pZeissController(nullptr), m_pZeissStage(nullptr), m_bUpdateData(false), m_magMode(MAG_MODE::MAG_MODE_TEM)
{
	m_pZeissController	= CTEMControlManager::GetInstance();
	m_pZeissStage		= new CZeissStage(m_pZeissController);



	if (m_pZeissController == nullptr || m_pZeissStage == nullptr)
		MB("Error@CZeissQuickControl - Constructor");
	else
		m_oTimer.doStart();
}

CZeissQuickControl::~CZeissQuickControl()
{

}

void CZeissQuickControl::do_toggle_large_screen(int state /*= -1*/)
{
	// 0 = Up | 1 = Down | 2 = Moving | 3 = Error
	int iLargeScreenState = m_pZeissController->get_large_screen_status();
	if (state == -1)
	{
		if (iLargeScreenState == 0 || iLargeScreenState == 3)
			m_pZeissController->do_large_screen_down();
		else if (iLargeScreenState == 1)
			m_pZeissController->do_large_screen_up();
		else if (iLargeScreenState == 2)
			PRINT("Large Screen is currently moving, try again once it is idle");
	}
	else
	{
		if (state ==  0)
			if (iLargeScreenState == 1) // Down
				m_pZeissController->do_large_screen_up();
		if (state == 1)
			if (iLargeScreenState == 0) // Up
				m_pZeissController->do_large_screen_down();
		}
}

void CZeissQuickControl::do_toggle_beam_blank()
{
	// 0 = No | 1 = User Blank | 2 = Camera | 3 = Ext Beam Blank  | 4 = Ext Shutter  | 
	// 5 = Blanker5 | 6 = STEM Frozen  | 7 = Remote CCD  | 8 = Blanker8 
	// 9 = Blanker9 | 10 =Blanker10  | 11 = Blanker11 | 12 = Blanker12
	// 13 = Blanker13 | 14 = Sample Holder | 15 = Gun Valve | 16 = X-Ray

	int iBeamState = m_pZeissController->get_beam_state();
	if (iBeamState == 0)
		m_pZeissController->do_blank_beam(true);
	else //else if(iBeamState == 1) // Should we make a special case for STEM FROZEN state?
		m_pZeissController->do_blank_beam(false); 
}

bool CZeissQuickControl::do_toggle_parallel_beam()
{
	bool bIsParallel = m_pZeissController->is_beam_parallel();
	if (bIsParallel)
		m_pZeissController->make_beam_convergent();
	else
		m_pZeissController->make_beam_parallel();

	return !bIsParallel;
}

void CZeissQuickControl::reload_tem_settings()
{
	static CDataCollection* pDC = CDataCollection::GetInstance();
	if(pDC == nullptr)
		return;

	if (m_pZeissController->get_illumination_mode() != ILL_MODE::TEM_MODE)
		return;

	MAG_MODE currMagMode = m_pZeissController->get_mag_mode();
	if (currMagMode == m_magMode)
		return;

	m_magMode = currMagMode;
	TEM_SETTING setting = m_magMode == MAG_MODE::MAG_MODE_TEM ? TEM_SETTING::PARAM_SEARCHING : TEM_SETTING::PARAM_SEARCHING_LOWMAG;
	pDC->m_oSearchingParams.RestoreCurrentParameters(setting);

}



void CZeissQuickControl::check_update_data()
{
	static int iTimer = 0;
	if(m_bUpdateData == false)
	{
		if (m_oTimer.returnElapsed() > iTimer * 1000) // every second
		{
			m_bUpdateData = true; // we allow the data/gui to be updated every second, so we don't overcall the tem's functions
			iTimer++;
		}
	}

	if (m_oTimer.returnElapsed() > 120000) // 2 minutes
	{
		m_oTimer.doReset();
		iTimer = 0;
	}
}

void CZeissQuickControl::stage_go_to_z(float _Z)
{
	m_pZeissStage->stage_go_to_z(_Z);
}

void CZeissQuickControl::stage_go_to_alpha(float _aph)
{
	m_pZeissStage->stage_rotate_to_angle(_aph);
}


void CZeissQuickControl::stage_go_to_delta_alpha(float _delta_aph)
{
	m_pZeissStage->stage_rotate_to_delta_angle(_delta_aph);

}

void CZeissQuickControl::change_actual_current_step(bool _bUp)
{
	if (m_pZeissController == nullptr)
		return;
	int iCurrentStep = m_pZeissController->get_actual_emission_step();
	
	if (_bUp) iCurrentStep++;
	else iCurrentStep--;

	m_pZeissController->set_target_emission__step(iCurrentStep);
}


void CZeissQuickControl::change_actual_camera_length(bool _bUp)
{
	if (m_pZeissController == nullptr)
		return;
	if (m_pZeissController->get_illumination_mode() != TEM_MODE || m_pZeissController->get_image_mode() != DIFFRACTION_MODE)
		return;

	int iCurrIndex = m_pZeissController->get_magnification_index();

	if (_bUp) iCurrIndex++;
	else iCurrIndex--;

	m_pZeissController->set_magnification_index(iCurrIndex);
}

void CZeissQuickControl::change_actual_brightness(bool _bUp)
{
	if (m_pZeissController == nullptr)
		return;

	int iCurrIndex = m_pZeissController->get_illumination_index();

	if (_bUp) iCurrIndex++;
	else iCurrIndex--;

	m_pZeissController->set_illumination_index(iCurrIndex);
}

float CZeissQuickControl::get_stage_z_position()
{
	if (m_pZeissController == nullptr)
		return 0.0f;
	return m_pZeissStage->get_stage_z();
}

float CZeissQuickControl::get_stage_tilt_angle()
{
	if (m_pZeissController == nullptr)
		return 0.0f;
	return m_pZeissStage->get_current_tilt_angle();
}

float CZeissQuickControl::get_actual_emission_current()
{
	if (m_pZeissController == nullptr)
		return 0.0f;
	return m_pZeissController->get_actual_emission_current();
}

float CZeissQuickControl::get_spot_size()
{
	if (m_pZeissController == nullptr)
		return 0.0f;
	return m_pZeissController->get_spot_size();
}

float CZeissQuickControl::get_camera_length()
{
	if (m_pZeissController == nullptr)
		return 0.0f; 
	return m_pZeissController->get_camera_length();
}

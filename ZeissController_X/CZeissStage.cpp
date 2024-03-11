#include "pch.h"

bool CZeissStage::is_stage_rotating()
{
	return m_pZeissControlManager->is_stage_rotating();
}

float CZeissStage::get_stage_x()
{
	return m_pZeissControlManager->get_stage_x() * 1000000.0f;
}

float CZeissStage::get_stage_y()
{
	return m_pZeissControlManager->get_stage_y() * 1000000.0f;
}

float CZeissStage::get_stage_z()
{
	return m_pZeissControlManager->get_stage_z() * 1000000.0f;

}

CZeissStage::~CZeissStage()
{
	PRINTD("\t\t\t\tCZeissStage::~CZeissStage() - Destructor\n");
}

CZeissStage::CZeissStage(CTEMControlManager* _pControlManager)
{
	fX_um = fY_um = fZ_um = fT = fR = fM = 0.0f;
	ZM(vX_m); ZM(vY_m); ZM(vZ_m); ZM(vT); ZM(vR); ZM(vM);

	m_pZeissControlManager = _pControlManager;
}

void CZeissStage::convert_to_microns()
{
	fX_um = vX_m.fltVal * 1000000;
	fY_um = vY_m.fltVal * 1000000;
	fZ_um = vZ_m.fltVal * 1000000;
	fT = vT.fltVal; // Same units
	fR = vR.fltVal; // Same units
	fM = vM.fltVal; // Same units
}

void CZeissStage::convert_to_meters()
{
	vX_m.fltVal = fX_um / 1000000;
	vY_m.fltVal = fY_um / 1000000;
	vZ_m.fltVal = fZ_um / 1000000;
	vT.fltVal = fT;  // Same units
	vR.fltVal = fR;  // Same units
	vM.fltVal = fM;  // Same units
}

ZeissErrorCode CZeissStage::move_stage_to_coordinates(float _x, float _y, float _z, float _t, float _r, float _m)
{
	fX_um = _x; fY_um = _y; fZ_um = _z; fT = _t; fR = _r; fM = _m;
	this->convert_to_meters();
	return static_cast<ZeissErrorCode>(m_pZeissControlManager->m_pZeissApi->MoveStage(vX_m.fltVal, vY_m.fltVal, vZ_m.fltVal,
		vT.fltVal, vR.fltVal, vM.fltVal));
}

ZeissErrorCode CZeissStage::move_stage()
{
	if (m_pZeissControlManager->Initialised() == false)
		return API_E_NOT_INITIALISED;
	this->convert_to_meters();
	return static_cast<ZeissErrorCode>(m_pZeissControlManager->m_pZeissApi->MoveStage(vX_m.fltVal, vY_m.fltVal, vZ_m.fltVal,
																					  vT.fltVal, vR.fltVal, vM.fltVal));
}

void CZeissStage::move_stage_to_pixel_coordinates(float _x, float _y)
{
	// This function doesn't belong to this class. Will be moved out in the future

	PRINTD("\t\t\t\CZeissStage::MoveStageToPixelCoordinates\n");
	float fPixelSize = m_pZeissControlManager->get_pixel_size(); // already converted to uM;
	
	//printf("TESTING: Pixelsize: %.2f\n", fPixelSize);
	float fCurrentX = STEMRESX / 2.0f;
	float fCurrentY = STEMRESY / 2.0f;

	float fDeltaX = static_cast<float>(_x - fCurrentX);
	float fDeltaY = static_cast<float>(_y - fCurrentY);
		
	fDeltaX *= fPixelSize;
	fDeltaY *= fPixelSize;



	//this->GetStagePosition();
	this->get_stage_coordinates();
	this->fX_um -= fDeltaX;
	this->fY_um += fDeltaY;
	
	this->move_stage();
}

void CZeissStage::stage_rotate_to_angle(float _fAng)
{
	m_pZeissControlManager->set_stage_tilt_angle(_fAng);
	this->fT = _fAng;
}

void CZeissStage::stage_rotate_to_delta_angle(float _fDelta)
{
	this->fT = get_current_tilt_angle();
	m_pZeissControlManager->set_stage_tilt_delta(_fDelta);
	this->fT += _fDelta;
}

float CZeissStage::get_current_tilt_angle()
{
	return m_pZeissControlManager->get_stage_tilt_angle();
}

bool CZeissStage::is_stage_busy()
{
	return m_pZeissControlManager->is_stage_busy();
}

ZeissErrorCode CZeissStage::get_stage_coordinates()
{
	if (m_pZeissControlManager->Initialised() == false)
		return API_E_NOT_INITIALISED;
	ZeissErrorCode zeissRetCode = static_cast<ZeissErrorCode>(m_pZeissControlManager->m_pZeissApi->GetStagePosition(&vX_m, &vY_m, &vZ_m,
		&vT, &vR, &vM));

	this->convert_to_microns();
	return zeissRetCode;
}

void CZeissStage::stage_go_to_x(float _x)
{
	m_pZeissControlManager->set_stage_x(_x / 1000000);
}


void CZeissStage::stage_go_to_y(float _y)
{
	m_pZeissControlManager->set_stage_y(_y / 1000000);

}

void CZeissStage::stage_go_to_z(float _z)
{
	m_pZeissControlManager->set_stage_z(_z / 1000000);
}

void CZeissStage::stage_go_to_xy(float _x, float _y)
{
	stage_go_to_x(_x);
	stage_go_to_y(_y);
}


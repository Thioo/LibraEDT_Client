#pragma once
class CZeissStage
{
private:
	CTEMControlManager* m_pZeissControlManager;
public:
	// x, y, z stage coordinates
	// t = alpha tilt, r=stage rotation, m=beta tilt
	float fX_um, fY_um, fZ_um, fT, fR, fM;
	VARIANT vX_m, vY_m, vZ_m, vT, vR, vM;

	float fTargetX, fTargetY, fTargetZ, fTargetT; // This variables will store the last target coordinates. So they should be updated afger
	
public:
	~CZeissStage();
	CZeissStage(CTEMControlManager* _pControlManager);

	void convert_to_microns();
	void convert_to_meters();

	void move_stage_to_pixel_coordinates(float _x, float _y);
	ZeissErrorCode move_stage_to_coordinates(float _x, float _y, float _z, float _t, float _r, float _m);
	ZeissErrorCode move_stage();
	ZeissErrorCode get_stage_coordinates();

	float get_stage_x(); // in microns
	float get_stage_y(); // in microns
	float get_stage_z(); // in microns
	float get_current_tilt_angle(); // in degrees

	void stage_go_to_x(float _x);
	void stage_go_to_y(float _y);
	void stage_go_to_xy(float _x, float _y);
	void stage_go_to_z(float _z);
	void stage_rotate_to_angle(float _fAng);
	void stage_rotate_to_delta_angle(float _fDelta);
	bool is_stage_busy();
	bool is_stage_rotating();

	// DoAlphaWobbler (Use AP_STAGE_DELTA_T)

};


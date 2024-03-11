#pragma once

class CZeissQuickControl
{
public:
	CTEMControlManager* m_pZeissController;
	CZeissStage*		m_pZeissStage;
	CTimer				m_oTimer;
	bool				m_bUpdateData;
	MAG_MODE			m_magMode;


	
	CZeissQuickControl();
	~CZeissQuickControl();

	
public:
	void check_update_data();
	
	void stage_go_to_z(float _Z);
	void stage_go_to_alpha(float _aph);
	void stage_go_to_delta_alpha(float _delta_aph);
	void change_actual_current_step(bool _bUp);
	void change_actual_camera_length(bool _bUp);
	void change_actual_brightness(bool _bUp);

	float get_stage_z_position();
	float get_stage_tilt_angle();
	float get_actual_emission_current();
	float get_spot_size();
	float get_camera_length();

	void do_toggle_large_screen(int state = -1);
	void do_toggle_beam_blank();
	bool do_toggle_parallel_beam();

	void reload_tem_settings();
};


#pragma once
#include "Timepix/include/mpxmodule.h"
#include "TinyTiff/tinytiffwriter.h"
#include <atomic>
#include <mutex>

#define _DATA_SIZE_ 512 * 512
#define _GRAB_SINGLE_IMG_REQ_ 1337
#define _GRAB_MULTIPLE_IMG_REQ_ 1338
#define _GRAB_MULTIPLE_IMG_STOP_REQ_ 1339
#define _UPDATE_EXPOSURE_TIME_REQ_ 1340

/*
	IP Addresses:
	192.168.34.1 -> Zeiss TEM - Local Area 1
	192.168.34.5 -> Zeiss TEM - Local Area 2, for visual studio debugging
	192.168.34.175 -> Linux PC

*/



struct outPacket
{
	int iRequestCode;
	int exposureTime;
	int num_frames_limit;
};

struct inPacket
{
	char buffer[_DATA_SIZE_ * 2];
	i16 is_last_packet; // easier if its an i16/short datatype rather than a boolean :)

	inPacket() { ZeroMemory(buffer, sizeof(buffer)); is_last_packet = 0; }
};

struct SFrame;

class CTimepix
{

	static CTimepix*			m_pTimepix;
	TinyTIFFWriterFile*			m_pTiffWriter;
	i16*						m_pData;
	i16*						m_pConvertedData;
	bool						m_bRelaxdInitialized;
	cv::Mat						m_oLiveImageMat;
	bool						m_bCancopyLiveImg;
	bool						m_bRotateImg;
	int							m_iWaitkey;
	int							m_iExposureTime;
	float						m_fBrightness;
	float						m_fContrast;

	std::mutex					m_fft_mtx;
	std::condition_variable		m_fft_cv;

	cv::Mat						m_fft_image;

public:
	MpxModule* m_pRelaxdModule; //TODO: Make Private later
	std::vector<cv::Point2f>	m_vTargetBeamCoords;
	cv::Point2f					m_vReferencePosition;
	int					m_iTargetBeamCoordsIndex;
	cv::Mat				m_InverseMat;
	bool				m_bLiveStreaming;
	int					m_iExposureTimeDiff;
	float				m_fContrastDiff;
	float				m_fBrightnessDiff;
	int					m_iExposureTimeImg;
	float				m_fBrightnessImg;
	float				m_fContrastImg;
	bool				m_bInvertColours;
	bool				m_bShowResolutionRings;
	bool				m_bShowPeaks;
	bool				m_bLiveFFT;
	bool				m_fft_update_needed;
	bool				m_stop_fft_thread;


	bool is_relaxd_module_initialized() const { return m_bRelaxdInitialized; }
	void RelaxdInitialized(bool val) { m_bRelaxdInitialized = val; }

	void do_reset_matrix();
	void do_reset();
	void do_reset_chips();
	void init_module();

	float get_contrast() const { return m_fContrast; }
	float get_brightness() const { return m_fBrightness; }

public:
	std::mutex beam_pos_mtx;
	std::atomic<bool> update_beam_pos{ false };
		
	// TCP - Server variables and functions
	SOCKET    m_TpxSocket;
	inPacket  tcp_inPacket;
	outPacket tcp_outPacket;
	i16*	  tcp_imgData;
	bool	  tcp_is_live_streaming;
	bool	  tcp_stop_live_stream_display;
	CTimer	  tcp_oCaptureTimer;

	std::mutex tcp_img_mtx;

	std::atomic<bool> tcp_lock_img_data{ false };
	std::atomic<bool> tcp_store_diffraction_images{ false };
	std::atomic<bool> tcp_startCaptureImgTimer{ false };

	std::atomic<int> tcp_store_frames_counter{ 0 };

	bool tcp_connect_to_server();
	void tcp_grab_single_image_from_detector(std::string& fileName, int exposureTime = -1);
	void tcp_grab_images_for_live_stream();
	void tcp_live_stream_images();
	void tcp_rotate_and_flip_image(cv::Mat& img);
	void tcp_prepare_for_live_stream();

	void tcp_fft_thread_function();
	void tcp_update_fft_image(cv::Mat image);


public:
	~CTimepix();
	static CTimepix* GetInstance();

	void prepare_for_live_stream();
	void grab_image_from_detector(std::string& _fileName, int _exposureTime = -1);
	bool is_image_rotated_and_flipped() { return m_bRotateImg; }
private:
	CTimepix();
	void prepare_for_NAT();
	void initialize_Relaxd_module();
	void save_image(std::string&_fileName, const void*_data, uint32_t iSize = 512, uint16_t type = 16);
	void grab_image_live_stream();
	void stream_image_live_stream();

	cv::Mat correct_raw_image(cv::Mat& img_to_correct);
};

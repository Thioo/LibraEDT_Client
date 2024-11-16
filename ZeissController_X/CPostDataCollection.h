#pragma once
#include <set>

struct SCameraLengthSettings
{
	float flPixelSize;
	float fRotationAxis;
	float fDetectorDistance;
	float fdstarmax;

	SCameraLengthSettings(int _iCameraLength = 180)
	{
		UpdateCameraLengthSettings(_iCameraLength);
	}

	void UpdateCameraLengthSettings(int _iCameraLength)
	{
		// These values have to be changed in the future once we have a proper way of correcting for distortions
		// I believe the pixel size calibration is done considering the elliptical distortions

		switch (_iCameraLength)
		{
		case 115:
			flPixelSize = 0.007437;
			fRotationAxis = 12.00;
			fdstarmax = 1.85;
			fDetectorDistance = (1 / 0.0335) * (0.055 / flPixelSize);
			break;

		case 144:
			flPixelSize = 0.006027;
			fRotationAxis = 16.00;
			fdstarmax = 1.52;
			fDetectorDistance = (1 / 0.0335) * (0.055 / flPixelSize);
			break;

		case 180:
			flPixelSize = 0.004815;
			fRotationAxis = 22.50;
			fdstarmax = 1.24;
			fDetectorDistance = (1 / 0.0335) * (0.055 / flPixelSize);
			break;

		case 230:
			flPixelSize = 0.003493;
			fRotationAxis = 29.70;
			fdstarmax = 0.90;
			fDetectorDistance = (1 / 0.0335) * (0.055 / flPixelSize);
			break;

		default: // Make a polyfit...
			PRINT("Undefined camera length used, using a polynomial fit settings by default.");

			std::vector<cv::Point2f> vPixelSizeData;
			std::vector<cv::Point2f> vRotAxisData;

			vPixelSizeData.emplace_back(115, 0.007437);
			vPixelSizeData.emplace_back(144, 0.006027);
			vPixelSizeData.emplace_back(180, 0.004815);
			vPixelSizeData.emplace_back(230, 0.003493);
			vRotAxisData.emplace_back(115, 12.0);
			vRotAxisData.emplace_back(144, 16.0);
			vRotAxisData.emplace_back(180, 22.50);
			vRotAxisData.emplace_back(230, 29.70);

			flPixelSize = 0.0f;
			fRotationAxis = 0.0f;

			cv::Mat coeffs_pixelsize = CDataCollection::GetInstance()->poly_fit(vPixelSizeData, 2);
			cv::Mat coeffs_rotaxis = CDataCollection::GetInstance()->poly_fit(vRotAxisData, 2);

			for (int i = 0; i < coeffs_pixelsize.rows; i++) {
				flPixelSize += coeffs_pixelsize.at<double>(i) * std::pow(_iCameraLength, i);
			}
			for (int i = 0; i < coeffs_rotaxis.rows; i++) {
				fRotationAxis += coeffs_rotaxis.at<double>(i) * std::pow(_iCameraLength, i);
			}

			fdstarmax = 1.24;
			fDetectorDistance = (1 / 0.0335) * (0.055 / flPixelSize);
			break;
		}
	}
};

class CPostDataCollection
{
public:
	CPostDataCollection(std::vector<SFrame>* _pFrame, int _cameraLength);
	CDataCollection* m_pZeissDataCollection;
	SCameraLengthSettings m_oCameraSettings;

	std::vector<SFrame>* m_collected_frames;
	std::vector<std::string> header_file_SMV;

	std::string m_tiffcorrected_folder;
	std::string m_smvimg_folder;
	std::string m_xds_proc_folder;
	cv::Point   m_shift_all_centers_to;


	static cv::Mat m_flatfield_image;
	static std::vector<cv::Point> m_DeadPixels_vec;

	int			m_iCameraLength;
	float		m_fRotationSpeed;
	float		m_fOscRange;
	float		m_fIntegrationSteps;
	float		m_fSpotSize;

	int			m_iBlurKernel;

private:

	void do_load_images_from_directory(std::string& _directory, std::set<std::string>& _images_set, std::string _extension);
	void delete_files_in_directory(std::string& _directory, std::string _extension);
	void replace_string_in_text(std::string& _string, std::string _toreplace, std::string _replacewith);
	static bool do_load_flatfield_image();
	void do_find_central_beam_coordinates(cv::Mat& _target_img, cv::Point& maxL, bool _blur=true);
	void do_make_cross_corrected_smv_files();
	void do_make_cross_corrected_tiff_files(std::set<std::string>& _raw_images_set);
	void do_translate_image(cv::Mat& _img, int _offsetX, int _offsetY);

public:
	static void do_flatfield_correction(cv::Mat& _img_to_correct);
	static void do_dead_pixels_correction(cv::Mat& _img_to_correct);
	static void do_cross_correction(cv::Mat& _image_to_correct, cv::Mat& _corrected_image, float _corrFactor = 1.0f, float _midcross_corr = 1.0f);
	static void do_get_dead_pixels_coordinates();

	void do_flatfield_and_cross_correction();
	void do_make_smv_header(float _phi, float _osc_start, float _osc_range, float _beam_center_X, float _beam_center_Y );
	void do_make_pets_header(std::vector<std::string>& _pets_header, bool _bOnTiffCorrected);
	void do_make_pets_file(bool _bOnTiffCorrected = false);
	void do_make_xds_file();
};




#include "pch.h"
#include <format>

cv::Mat CPostDataCollection::m_flatfield_image;
std::vector<cv::Point> CPostDataCollection::m_DeadPixels_vec;
CPostDataCollection::CPostDataCollection(std::vector<SFrame>* _pFrame, int _cameraLength)
{
	m_collected_frames = _pFrame;
	m_pZeissDataCollection = CDataCollection::GetInstance();

	if (m_collected_frames->back().bLiveStream == false)
	{
		// create all the required folders
		m_tiffcorrected_folder = m_collected_frames->back().sDirectory + "tiff_corrected/";
		m_smvimg_folder = m_collected_frames->back().sDirectory + "SMV_img/";
		m_xds_proc_folder = m_smvimg_folder + "xds_proc/";
		
		m_iCameraLength = _cameraLength;
		m_oCameraSettings.UpdateCameraLengthSettings(m_iCameraLength);

		CWriteFile::create_directory(m_tiffcorrected_folder);
		CWriteFile::create_directory(m_smvimg_folder);
		CWriteFile::create_directory(m_xds_proc_folder);

		this->do_get_dead_pixels_coordinates();

		m_fOscRange = fabs(CDataCollection::GetInstance()->m_fStartingAng - CTEMControlManager::GetInstance()->get_stage_tilt_angle()) / (m_collected_frames->size() - 1);
		m_iBlurKernel = 15;
	}
}

void CPostDataCollection::do_load_images_from_directory(std::string& _directory, std::set<std::string>& _images_set, std::string _extension)
{
	for (const auto& entry : std::filesystem::directory_iterator(_directory))
	{
		std::string sCurrentItemPath = entry.path().string();
		if (sCurrentItemPath.substr(sCurrentItemPath.find_last_of(".") + 1) == _extension.c_str() || sCurrentItemPath.substr(sCurrentItemPath.find_last_of(".") + 1) == "tif")
			_images_set.insert(sCurrentItemPath);
	}
}

void CPostDataCollection::delete_files_in_directory(std::string& _directory, std::string _extension)
{
	// Delete all the files in the directory
	for (const auto& entry : std::filesystem::directory_iterator(_directory))
	{
		std::string sCurrentItemPath = entry.path().string();
		if (sCurrentItemPath.substr(sCurrentItemPath.find_last_of(".") + 1) == _extension.c_str())
			std::filesystem::remove(sCurrentItemPath);
	}
}

bool CPostDataCollection::do_load_flatfield_image()
{
	if (m_flatfield_image.empty())
	{

	//	m_flatfield_image = cv::imread("Flatfield/Flatfield.tiff", cv::IMREAD_UNCHANGED);
		m_flatfield_image = cv::imread("Flatfield/Flatfield.tiff", cv::IMREAD_UNCHANGED);

		if (m_flatfield_image.empty())
		{
			PRINT("Flatfield correction will not be applied. Make sure you have flatfield.tiff placed in the flatfield folder");
			return false;
		}
	}
	return true;
}

void CPostDataCollection::do_get_dead_pixels_coordinates()
{
	if (do_load_flatfield_image())
	{
		cv::Mat flatfield_img;
		m_flatfield_image.copyTo(flatfield_img);
		m_DeadPixels_vec.clear();

		double minVal = 0.0f;
		double maxVal = 0.0f;
		cv::Point maxLoc;
		cv::Point minLoc;

		// Get the coordinate of all the deadpixels from the reference img.
		while (minVal <= 0.01f)
		{
			cv::minMaxLoc(flatfield_img, &minVal, &maxVal, &minLoc, &maxLoc);
			if (minVal <= 0.01f)
				m_DeadPixels_vec.push_back(minLoc);
			// give it a non zero value so that we can look for other deadpixels
			flatfield_img.at<uint16_t>(minLoc) = 1;

		}

		printf("there are %d dead pixels to correct\n", m_DeadPixels_vec.size());
	}
	else
		PRINT("No deadpixel correction will be applied, flatfield image is unexistent!");
}

void CPostDataCollection::do_find_central_beam_coordinates(cv::Mat& _target_img, cv::Point& maxL, bool _blur/*=true*/)
{
	double min, max;
	cv::Point minl;

	cv::Mat gray;
	cv::Mat blur;
	cv::cvtColor(_target_img, gray, cv::COLOR_BGR2GRAY);
	if (_blur)
		cv::GaussianBlur(gray, blur, cv::Size(m_iBlurKernel, m_iBlurKernel), 0, 0);
	cv::minMaxLoc(blur, &min, &max, &minl, &maxL);

	// If we couldn't find the central beam, let's decrease the blur effect
	if(maxL.x == 0 && maxL.y == 0)
	{
		m_iBlurKernel = 5;
		cv::GaussianBlur(gray, blur, cv::Size(m_iBlurKernel, m_iBlurKernel), 0, 0);
		cv::minMaxLoc(blur, &min, &max, &minl, &maxL);
		
		// if we still can't find it, apply no blur at all
		if (maxL.x == 0 && maxL.y == 0)
		{
			cv::minMaxLoc(gray, &min, &max, &minl, &maxL);
		}
	}

}

void CPostDataCollection::do_flatfield_and_cross_correction()
{
	if (do_load_flatfield_image())
	{
		std::set<std::string> _raw_images_set;
		do_load_images_from_directory(m_collected_frames->back().sDirectory, _raw_images_set, "tiff");
		do_make_cross_corrected_tiff_files(_raw_images_set);
		do_make_pets_file(true);
		do_make_xds_file();

		//if (m_bDeleteRAWData)
			delete_files_in_directory(m_collected_frames->back().sDirectory, "tiff");
	}
}

void CPostDataCollection::do_make_smv_header(float _phi, float _osc_start, float _osc_range, float _beam_center_X, float _beam_center_Y)
{
	header_file_SMV.clear();

	header_file_SMV.push_back("{\n"); // beginning
	header_file_SMV.push_back("HEADER_BYTES=  512;\n");
	header_file_SMV.push_back("DIM=2;\n");
	header_file_SMV.push_back("BYTE_ORDER=little_endian;\n");
	header_file_SMV.push_back("TYPE=unsigned_short;\n");
	header_file_SMV.push_back("SIZE1=516;\n");
	header_file_SMV.push_back("SIZE2=516;\n");
	header_file_SMV.push_back("PIXEL_SIZE=0.055000;\n");
	header_file_SMV.push_back("BIN=1x1;\n");
	header_file_SMV.push_back("BIN_TYPE=HW;\n");
	header_file_SMV.push_back("ADC=fast;\n");
	header_file_SMV.push_back("CREV=1;\n");
	header_file_SMV.push_back("BEAMLINE=TIMEPIX_SU;\n");
	header_file_SMV.push_back("DETECTOR_SN=901;\n");
	header_file_SMV.push_back("DATE=Tue Jun 26 09:43:09 2007;\n");
	header_file_SMV.push_back("TIME=0.096288;\n");
	header_file_SMV.push_back(std::format("DISTANCE={:.2f};\n", m_oCameraSettings.fDetectorDistance));
	header_file_SMV.push_back("TWOTHETA=0.00;\n");
	header_file_SMV.push_back(std::format("PHI={:.2f};\n", _phi));
	header_file_SMV.push_back(std::format("OSC_START={:.2f};\n", _osc_start));
	header_file_SMV.push_back(std::format("OSC_RANGE={:.2f};\n", _osc_range));
	header_file_SMV.push_back(std::format("WAVELENGTH={:.6f};\n", 0.0335f));
	header_file_SMV.push_back(std::format("BEAM_CENTER_X={:.2f};\n", _beam_center_X));
	header_file_SMV.push_back(std::format("BEAM_CENTER_Y={:.2f};\n", _beam_center_Y));
	header_file_SMV.push_back(std::format("DENZO_X_BEAM={:.2f};\n", _beam_center_X * 0.055f));
	header_file_SMV.push_back(std::format("DENZO_Y_BEAM={:.2f};\n", _beam_center_Y * 0.055f));
	header_file_SMV.push_back("}"); //end
}

void CPostDataCollection::do_dead_pixels_correction(cv::Mat& _img_to_correct)
{
	for (auto& minPos : m_DeadPixels_vec)
	{
		// take the location and create a rect(area) around it
		cv::Mat neighbourhood = cv::Mat(_img_to_correct, cv::Rect(minPos.x - 3, minPos.y - 3, 6, 6));
		auto val = _img_to_correct.at<uint16_t>(minPos);
		_img_to_correct.at<uint16_t>(minPos) = std::round(cv::mean(neighbourhood)[0]);
		val = _img_to_correct.at<uint16_t>(minPos);
	}
}

void CPostDataCollection::do_cross_correction(cv::Mat& _image_to_correct, cv::Mat& _corrected_image, float _corrFactor /*= 1.0f*/, float _midcross_corr /*= 1.0f*/)
{
	_corrected_image = cv::Mat::zeros(516, 516, _image_to_correct.type());
	float corrFacDef = 1.0f;
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
			_corrected_image.at<uint16_t>(j, i) = _image_to_correct.at<uint16_t>(j, i);

	for (int i = 256; i < 512; i++)
		for (int j = 0; j < 256; j++)
			_corrected_image.at<uint16_t>(j, i + 4) = _image_to_correct.at<uint16_t>(j, i);

	for (int i = 0; i < 256; i++)
		for (int j = 256; j < 512; j++)
			_corrected_image.at<uint16_t>(j + 4, i) = _image_to_correct.at<uint16_t>(j, i);

	for (int i = 256; i < 512; i++)
		for (int j = 256; j < 512; j++)
			_corrected_image.at<uint16_t>(j + 4, i + 4) = _image_to_correct.at<uint16_t>(j, i);


	// left side of the vertical cross
	for (int i = 255; i < 258; i++)
	{
		for (int j = 0; j < 516; j++)
		{
			int k = j;
			if (j == 255 || j == 256 || j == 257)
				k = 255;
			else if (j == 258 || j == 259 || j == 260)
				k = 256;
			else if (j > 260)
				k -= 4;

			//auto iTestVal = _original[255 + 512 * j];
			_corrected_image.at<uint16_t>(j, i) = _image_to_correct.at<uint16_t>(k, 255) * _corrFactor;
			//_cross_fixed[i + 516 * j] = 77;

		}
	}

	// right side of the vertical cross
	for (int i = 258; i < 261; i++)
	{
		for (int j = 0; j < 516; j++)
		{
			int k = j;
			if (j == 255 || j == 256 || j == 257)
				k = 255;
			else if (j == 258 || j == 259 || j == 260)
				k = 256;
			else if (j > 260)
				k -= 4;

			//auto iTestVal = _original[256 + 512 * j];
			_corrected_image.at<uint16_t>(j, i) = _image_to_correct.at<uint16_t>(k, 256) * _corrFactor;
			//_cross_fixed[i + 516 * j] = 33;

		}
	}

	// Upper part of the horizontal line (cross)
	for (int i = 0; i < 516; i++)
	{
		for (int j = 255; j < 258; j++)
		{
			int k = i;
			if (i == 255 || i == 256 || i == 257)
				k = 255;
			else if (i == 258 || i == 259 || i == 260)
				k = 256;
			else if (i > 260)
				k -= 4;

			_corrected_image.at<uint16_t>(j, i) = _image_to_correct.at<uint16_t>(255, k) * _corrFactor;

		}
	}

	// bottom part of the horizontal line (cross)
	for (int i = 0; i < 516; i++)
	{
		for (int j = 258; j < 261; j++)
		{
			int k = i;
			if (i == 255 || i == 256 || i == 257)
				k = 255;
			else if (i == 258 || i == 259 || i == 260)
				k = 256;
			else if (i > 260)
				k -= 4;

			_corrected_image.at<uint16_t>(j, i) = _image_to_correct.at<uint16_t>(256, k) * _corrFactor;
		}
	}
	
	// center of the cross
	for (int i = 255; i < 261; i++)
		for (int j = 255; j < 261; j++)
			_corrected_image.at<uint16_t>(j, i) *= _corrFactor * _midcross_corr;
}

void CPostDataCollection::do_make_cross_corrected_smv_files()
{
	// find beam center of all the frames


	// shift them all so they fit the first frame

	// use opencv to load and shift the images and access data generate the SMV file
}

void CPostDataCollection::do_make_cross_corrected_tiff_files(std::set<std::string>& _raw_images_set)
{
	if (_raw_images_set.empty())
	{
		PRINT("No images to correct.");
		return;
	}

	cv::Mat flatfield_img;
	m_flatfield_image.copyTo(flatfield_img);
	float mean_intensity = cv::mean(flatfield_img)[0];

	auto fileloc = *_raw_images_set.begin();
	auto firstimg = cv::imread(fileloc);
	do_find_central_beam_coordinates(firstimg, m_shift_all_centers_to);
	printf("All SMV images will be translated so that the central beam coordinates will be: (%d, %d)", m_shift_all_centers_to.x, m_shift_all_centers_to.y);
	int iCounter = -1;
	for (auto& _image_to_correct : _raw_images_set)
	{
		iCounter++;
		cv::Mat currImg = cv::imread(_image_to_correct, cv::IMREAD_UNCHANGED);
		if(currImg.empty())
			continue;

		currImg.convertTo(currImg, CV_32F);
		flatfield_img.convertTo(flatfield_img, CV_32F);
		currImg = currImg * static_cast<float>(mean_intensity) / flatfield_img;
		//cv::divide(currImg, flatfield_img, currImg);
		currImg.convertTo(currImg, CV_16U);

		do_dead_pixels_correction(currImg);

		cv::Mat flatfield_and_crosscorrected_image;
		do_cross_correction(currImg, flatfield_and_crosscorrected_image);

		if(true)
		{
			std::string sNewNameTIFF = m_tiffcorrected_folder + m_collected_frames->at(iCounter).sImgName;
			TinyTIFFWriterFile* tiffwriter = TinyTIFFWriter_open(sNewNameTIFF.c_str(), 16, TinyTIFFWriter_Int, 1, 516, 516, TinyTIFFWriter_Greyscale);
			if (tiffwriter)
			{
				TinyTIFFWriter_writeImage(tiffwriter, flatfield_and_crosscorrected_image.data);
				TinyTIFFWriter_close(tiffwriter);
			}
		}
		bool bVal = false;
		if(bVal)
		{
			continue;
			std::string sNewNameSMV = m_smvimg_folder + m_collected_frames->at(iCounter).sImgName;
			sNewNameSMV = sNewNameSMV.substr(0, sNewNameSMV.find_last_of("."));
			sNewNameSMV.append(".img");
			std::ofstream fout(sNewNameSMV, std::ios::binary);
			
			auto frame = m_collected_frames->at(iCounter);
			
			cv::Point maxl;
			//auto img = cv::imread(frame.sFullpath);
			auto img = cv::imread(_image_to_correct);
			do_find_central_beam_coordinates(img, maxl);
				
			// Only make the translation if the central beam coordinate was actually found.
			if(maxl.x != 0 && maxl.y != 0)
			{
				if (m_shift_all_centers_to.x == 0 && m_shift_all_centers_to.y == 0)
				{
					m_shift_all_centers_to.x = maxl.x;
					m_shift_all_centers_to.y = maxl.y;
				}

				cv::Point central_beam_offset = maxl - m_shift_all_centers_to;
				do_translate_image(flatfield_and_crosscorrected_image, -central_beam_offset.x, -central_beam_offset.y);
			}
			do_make_smv_header(frame.fAngle, frame.fAngle,m_fIntegrationSteps, m_shift_all_centers_to.x, m_shift_all_centers_to.y);
			int written_bytes = 0;
			for (auto& v : header_file_SMV)
			{
				fout << v;
				written_bytes += v.length();
			}
			for (int i = written_bytes; i < 512; i++) 
				fout << '\0';
			fout.write(reinterpret_cast<char*>(flatfield_and_crosscorrected_image.data), 516 * 516 * 2);
			fout.close();

		}
	}
	

}

void CPostDataCollection::do_translate_image(cv::Mat& _img, int _offsetX, int _offsetY)
{
	cv::Mat trans_mat = (cv::Mat_<double>(2, 3) << 1, 0, _offsetX, 0, 1, _offsetY);
	warpAffine(_img, _img, trans_mat, _img.size());
}

void CPostDataCollection::do_make_pets_header(std::vector<std::string>& _pets_header, bool _bOnTiffCorrected)
{

	float lambda = 0.0335f;
	float aperpixel = m_oCameraSettings.flPixelSize;

	std::string geometry = "continuous";
	std::string detector = "asitmp";
	if (_bOnTiffCorrected)
		detector = "default";
	float dstarmax = m_oCameraSettings.fdstarmax;
	float dstarmaxps = m_oCameraSettings.fdstarmax;
	float dstarmin = 0.05f;
	float phi = 1.0f; // for precession
	if (geometry == "continuous")
	{
		phi = 0.5f * m_fOscRange;

		//phi = m_fIntegrationSteps * 0.5f;
	}
	float omega = m_oCameraSettings.fRotationAxis;
	auto pDC = CDataCollection::GetInstance();
	unsigned int _average_time = 0;
	for (auto& v : *m_collected_frames)
	{
		_average_time += v.iTotalTime;
	}
	_average_time /= m_collected_frames->size();

	_pets_header.push_back(std::format("#Operator:\t\t{}\n", pDC->m_sOperatorName));
	_pets_header.push_back(std::format("#Date:\t\t{}\n", pDC->m_sCollectionDate));
	_pets_header.push_back(std::format("#Emission Current (uA):\t\t{:.2f}\n", CTEMControlManager::GetInstance()->get_actual_emission_current()));
	
	if(CTEMControlManager::GetInstance()->is_on_stem_mode())
		_pets_header.push_back(std::format("#STEM Mode: Spot Size (nm):\t\t{:.2f}\n", CTEMControlManager::GetInstance()->get_spot_size()));
	else
		_pets_header.push_back(std::format("#TEM Mode: Illumination Angle (urad):\t\t{:.2f}\n", CTEMControlManager::GetInstance()->get_illumination_angle()));
	
	_pets_header.push_back(std::format("#C1 Lens (A):\t\t{:.4f}\n", CTEMControlManager::GetInstance()->get_C1_lens()));
	_pets_header.push_back(std::format("#C2 Lens (A):\t\t{:.4f}\n", CTEMControlManager::GetInstance()->get_C1_lens()));
	_pets_header.push_back(std::format("#C3 Lens (A):\t\t{:.4f}\n", CTEMControlManager::GetInstance()->get_C1_lens()));
	_pets_header.push_back(std::format("#Objective Lens (A):\t\t{:.4f}\n", CTEMControlManager::GetInstance()->get_C1_lens()));

	_pets_header.push_back(std::format("#Camera Length (mm):\t\t{:.2f}\n", CTEMControlManager::GetInstance()->get_camera_length()));
	_pets_header.push_back(std::format("#Exposure Time(ms):\t\t{:03d}\n", CTimepix::GetInstance()->m_iExposureTimeDiff));
	_pets_header.push_back(std::format("#Rotation Speed (%):\t\t{:02d}%\n", static_cast<int>(CTEMControlManager::GetInstance()->get_stage_tilt_speed())));
	_pets_header.push_back(std::format("#Data collection duration (s):\t\t{:.02f}\n",  fabs(pDC->m_fStartingAng - CTEMControlManager::GetInstance()->get_stage_tilt_angle()) / m_fRotationSpeed));
	_pets_header.push_back(std::format("#Rotation Speed (Deg/s):\t\t{:.02f}\n", m_fRotationSpeed));
	_pets_header.push_back(std::format("#Oscillation Range (Deg):\t\t{:.02f}\n", m_fOscRange));

	_pets_header.push_back(std::format("#Starting Angle: (Deg):\t\t{:.02f}\n", pDC->m_fStartingAng));
	_pets_header.push_back(std::format("#Ending Angle (Deg):\t\t{:.02f}\n", CTEMControlManager::GetInstance()->get_stage_tilt_angle()));
	_pets_header.push_back(std::format("#StageX (um):\t\t{:.02f}\n", CTEMControlManager::GetInstance()->get_stage_x() * 1000000.0f));
	_pets_header.push_back(std::format("#StageY (um):\t\t{:.02f}\n", CTEMControlManager::GetInstance()->get_stage_y() * 1000000.0f));
	_pets_header.push_back(std::format("#StageZ (um):\t\t{:.02f}\n", CTEMControlManager::GetInstance()->get_stage_z() * 1000000.0f));

	std::string beamstop = "beamstop no\n";
	if (_bOnTiffCorrected)
		beamstop = "beamstop\n#255\t#0\n#261\t#0\n#261\t#255\n#516\t#255\n#516\t#261\n#261\t#261\n#261\t#516\n#255\t#516\n#255\t#261\n#0\t#261\n#0\t#255\n#255\t#255\nendbeamstop\n";
	_pets_header.push_back(std::format("\n\n\n\nlambda\t{:.6f}\n", lambda));
	_pets_header.push_back(std::format("aperpixel\t{:.6f}\n", aperpixel));
	_pets_header.push_back(std::format("geometry\t{}\n", geometry));
	_pets_header.push_back(std::format("detector\t{}\n", detector));
	_pets_header.push_back("center AUTO\n");
	_pets_header.push_back("centermode centralbeam 1\n");
	_pets_header.push_back(beamstop);
	_pets_header.push_back(std::format("dstarmax\t{:.4f}\n", dstarmax));
	_pets_header.push_back(std::format("dstarmaxps\t{:.4f}\n", dstarmaxps));
	_pets_header.push_back(std::format("dstarmin\t{:.4f}\n", dstarmin));
	_pets_header.push_back(std::format("phi\t{:.6f}\n", phi));
	_pets_header.push_back(std::format("omega\t{:.5f} 0 1\n", omega));
	_pets_header.push_back(std::format("pixelsize\t0.0055\n"));
	_pets_header.push_back(std::format("bin\t1\n"));
	_pets_header.push_back(std::format("i/sigma\t10.00\t10.00\n"));
	_pets_header.push_back(std::format("noiseparameters\t2.0000\t0.0000\n"));
	_pets_header.push_back(std::format("errormodel\t1.0000\t0.0000\t0.0000\t1\n"));
	_pets_header.push_back(std::format("skipsaturated yes\n"));
	_pets_header.push_back(std::format("reflectionsize\t8.0\n"));
	_pets_header.push_back(std::format("peaksearchmode\tauto\n"));
	_pets_header.push_back(std::format("saturationlimit\t11000\n"));

	_pets_header.push_back("imagelist\n");
}


void CPostDataCollection::do_make_pets_file(bool _bOnTiffCorrected /*= false*/)
{

	std::vector<std::string> pets_header;
	do_make_pets_header(pets_header, _bOnTiffCorrected);



	std::string sPetsFolder = m_collected_frames->back().sDirectory;
	if (_bOnTiffCorrected)
		sPetsFolder.append("tiff_corrected/");
	sPetsFolder.append("Pets_Folder/");
	CWriteFile::create_directory(sPetsFolder);

	auto pDc = m_pZeissDataCollection;
	//std::string sPetsFile = sPetsFolder + pDc->m_sCollectionDate + "_" + pDc->m_sCrystalName + "_" + pDc->m_sCrystalNumberID_cpy + ".pts";
	std::string sPetsFile_cte_step = sPetsFolder + pDc->m_sCollectionDate + "_" + pDc->m_sCrystalName + "_" + pDc->m_sCrystalNumberID_cpy + ".pts";
	//std::ofstream pets_file(sPetsFile, std::ios::binary);
	std::ofstream pets_file2(sPetsFile_cte_step, std::ios::binary);
	float fStartingAngle = CDataCollection::GetInstance()->m_fStartingAng;
	float _average_time = 0;
	float _average_delta_alpha = 0;
	int iCounterAVG = 0;
	int iAdded = 0;
	float fPrevious = m_collected_frames->at(0).fAngle;
	for (auto& v : *m_collected_frames)
	{
		_average_time += v.iTotalTime;
		
		if (v.bLiveStream)
			continue;
		_average_delta_alpha += fabs(v.fAngle - fPrevious);
		fPrevious = v.fAngle;
		iAdded++;

	}
	_average_time /= m_collected_frames->size();
	_average_delta_alpha /= iAdded;


	for (auto& v : pets_header)
	{
	//	pets_file << v;
		pets_file2 << v;
	}
	fPrevious = m_collected_frames->at(0).fAngle;
	float fStep = m_fRotationSpeed * _average_time * 0.001;
	if (_average_delta_alpha)
		fStep = _average_delta_alpha;
	int iCounter = -1;

	float fMeanStep = m_fOscRange;
	if (fStartingAngle <= 0)
		fMeanStep *= -1;

	for (auto& frame : *m_collected_frames)
	{
		iCounter++;
		if (frame.bLiveStream)
			continue;

		float fDifference = fabs(frame.fAngle - fPrevious);
		fPrevious = frame.fAngle;
		//pets_file << std::format("\"../{}\"\t{:.2f}\t#DeltaAlpha({:.5f})\tRotationSpeed({:.2f})\n", frame.sImgName, frame.fAngle, fDifference, frame.fRotSpeed);
		pets_file2 << std::format("\"../{}\"\t{:.2f}\t#{:.2f}\n", frame.sImgName, (fStartingAngle - (fMeanStep * 0.5f)) - (iCounter * fMeanStep), (fStartingAngle - (fStep * 0.5f)) + (iCounter * fStep));
	}
	//pets_file << "endimagelist\n";
	pets_file2 << "endimagelist\n";
	//pets_file.close();
	pets_file2.close();

}


void CPostDataCollection::replace_string_in_text(std::string& _string, std::string _toreplace, std::string _replacewith)
{
	int oldLen = _toreplace.length();
	int newLen = _replacewith.length();

	_string.replace(_string.find(_toreplace), oldLen, _replacewith);
}


void CPostDataCollection::do_make_xds_file()
{

	std::ifstream xds_inp_template("XDS_Template\\XDS_INP_template.msa", std::ios::binary);
	std::stringstream xds_inp_buff;
	xds_inp_buff << xds_inp_template.rdbuf();
	std::string xds_inp = xds_inp_buff.str();



	replace_string_in_text(xds_inp, "__ORGX__", std::format("{:.2f}", static_cast<float>(m_shift_all_centers_to.x) + 5));
	replace_string_in_text(xds_inp, "__ORGY__", std::format("{:.2f}", static_cast<float>(m_shift_all_centers_to.y) + 1));

	replace_string_in_text(xds_inp, "__LORES__", std::format("{:.2f}", 50.00f));
	replace_string_in_text(xds_inp, "__HIRES__", std::format("{:.2f}", 0.00f));

	replace_string_in_text(xds_inp, "__DATARANGESTART__", std::format("{:04d}", 1));
	replace_string_in_text(xds_inp, "__DATARANGEEND__", std::format("{:04d}", m_collected_frames->size()-1));

	replace_string_in_text(xds_inp, "__SPOTRANGESTART__", std::format("{:04d}", 1));
	replace_string_in_text(xds_inp, "__SPOTRANGEEND__", std::format("{:04d}", m_collected_frames->size() - 1));

	replace_string_in_text(xds_inp, "__BACKGROUNDRANGESTART__", std::format("{:04d}", 1));
	replace_string_in_text(xds_inp, "__BACKGROUNDRANGEEND__", std::format("{:04d}", m_collected_frames->size() - 1));

	replace_string_in_text(xds_inp, "__STARTINGANGLE__", std::format("{:.2f}", CDataCollection::GetInstance()->m_fStartingAng));
	replace_string_in_text(xds_inp, "__STARTINGFRAME__", std::format("{:04d}", 1));

	replace_string_in_text(xds_inp, "__DETECTORDISTANCE__", std::format("{:.4f}", m_oCameraSettings.fDetectorDistance));

	replace_string_in_text(xds_inp, "__OSCILLATIONRANGE__", std::format("{:.4f}\t!{:.4f} you can also try this value", m_fOscRange, m_fIntegrationSteps));


	replace_string_in_text(xds_inp, "__ROTATIONAXIS1__", std::format("{:.5f}", std::cos(m_oCameraSettings.fRotationAxis * (22 / 7) / 180)));  //pi/180
	replace_string_in_text(xds_inp, "__ROTATIONAXIS2__", std::format("{:.5f}", std::sin(-m_oCameraSettings.fRotationAxis * (22 / 7) / 180))); //pi/180
	replace_string_in_text(xds_inp, "__ROTATIONAXIS3__", std::format("{:.5f}", 0.00f));

	replace_string_in_text(xds_inp, "__WAVELENGTH__", std::format("{:.5f}", 0.0335f));


	std::string xds_inp_path = m_xds_proc_folder + "XDS.inp";
	std::ofstream xds_inp_out(xds_inp_path, std::ios::binary);
	xds_inp_out << xds_inp;
	xds_inp_out.close();

	std::string run_xdsgui_path = m_xds_proc_folder + "run_xdsgui.bat";
	std::ofstream run_xdsgui_out(run_xdsgui_path, std::ios::binary);
	run_xdsgui_out << "bash -ilc xdsgui";
	run_xdsgui_out.close();

	std::string run_xds_path = m_xds_proc_folder + "run_xds.bat";
	std::ofstream run_xds_out(run_xds_path, std::ios::binary);
	run_xds_out << "bash -ilc xds";
	run_xds_out.close();
}

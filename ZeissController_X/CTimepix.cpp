#include "pch.h"
#include "Timepix/include/shared.h"
#include "Timepix/include/mpxerrors.h"
 
#define IMG_RESIZE_FACTOR_ 1

CTimepix* CTimepix::m_pTimepix = nullptr;


CTimepix::CTimepix() : m_pRelaxdModule(nullptr), m_bRelaxdInitialized(false), m_bLiveStreaming(false), m_iExposureTime(500), m_pData(nullptr),
m_fContrast(2.0f), m_fBrightness(0.0f), m_bInvertColours(false), m_bLiveFFT(false), m_bShowResolutionRings(false), m_bCancopyLiveImg(false), m_iWaitkey(100), m_bRotateImg(false)
{
	PRINTD("\t\t\t\tCTimepix::CTimepix() - Constructor");

	// Patch before calling MpxModule's constructor
	prepare_for_NAT();

	
	if (is_relaxd_module_initialized() == false)
	{
		// Running here because it takes time...
		//std::thread tRelaxdInitialization(&CTimepix::initialize_Relaxd_module, this);
		//tRelaxdInitialization.detach();
		bool bConnection = this->tcp_connect_to_server();
		RelaxdInitialized(bConnection);

	}
	m_pConvertedData = new i16[512 * 512];
	tcp_imgData = new i16[512 * 512];

	//CPostDataCollection::m_flatfield_image = cv::imread("Flatfield/Flatfield.tiff", cv::IMREAD_UNCHANGED);
	//CPostDataCollection::do_get_dead_pixels_coordinates();
}

void CTimepix::prepare_for_NAT()
{
	PRINT("PREPARE FOR NAT - IGNORED");
	return;

	DWORD dwOld, dwBase;
	dwBase = reinterpret_cast<DWORD>(GetModuleHandle("mpxhwrelaxd.dll"));
	if (dwBase)
	{
		DWORD dwIPAddr = dwBase + 0x1F86C;
		DWORD dwPort = dwBase + 0x1F8BF;
		VirtualProtect((LPVOID)dwIPAddr, 0x64, PAGE_EXECUTE_READWRITE, &dwOld);
		dwIPAddr++;

		*(DWORD*)(dwIPAddr) = 0x3F57DD51; // Should be 192.168.34.175 (0 - 0x3f57dd51)
		//*(WORD*)(dwTargetTwo + 0x6) = 4444; // Port
		VirtualProtect((LPVOID)dwIPAddr, 0x64, dwOld, &dwOld);
	}
}

void CTimepix::initialize_Relaxd_module()
{
	PRINTD("\t\t\t\tCTimepix::InitializeRelaxdModule()");
	int size = sizeof(MpxModule);
	if(m_pRelaxdModule == nullptr)
		m_pRelaxdModule = new MpxModule(0); // 0 because we are using Timepix1

	if (m_pRelaxdModule == nullptr)
	{
		PRINT("Error@CTimepix::CTimepix() - new MpxModule(0) returned nullptr");
		return;
	}

	this->init_module();




}
std::mutex mtx;

void CTimepix::grab_image_from_detector(std::string& _fileName, int _exposureTime /*= -1*/)
{
	//printf("grab_image_from_detector - no longer used for now...\n using TCP version instead\n");
	tcp_grab_single_image_from_detector(_fileName, _exposureTime);
	return;

	PRINTD("\t\t\t\tCTimepix::GrabImageFromDetector()");
	static int iLastExposureTime = -100;
	AcqParams acq;
	MpxModule* relaxd = m_pRelaxdModule;
	if (_exposureTime == -1)
		_exposureTime = m_iExposureTime;


	std::lock_guard<std::mutex> lock(mtx);
	if (is_relaxd_module_initialized())
	{
		if (iLastExposureTime != _exposureTime)
		{
			acq.enableCst = 0;                     // enable Charge Sharing Test
			acq.polarityPositive = 1;              // positive polarity
			acq.acqCount = 1;                      // acq count (>1 for burst mode)


			if (_exposureTime == 0) {
				acq.useHwTimer = 0;                            // hw timer is used
				acq.mode = ACQMODE_HWTRIGSTART_HWTRIGSTOP;     // acq mode (see ACQMODE_xxx in common.h)
				acq.time = 0;                                  // acq time (if controlled by HW timer)
			}
			else {
				acq.useHwTimer = 1;                            // hw timer is used
				acq.mode = ACQMODE_ACQSTART_TIMERSTOP;         // acq mode (see ACQMODE_xxx in common.h)
				acq.time = _exposureTime * 0.001;            // acq time (if controlled by HW timer)
			}

			relaxd->setAcqPars(&acq);
		}


		relaxd->startAcquisition();
		Sleep(_exposureTime);

		bool busy = true;
		while (relaxd->getBusy(&busy) == MPXERR_NOERROR && busy) {
			Sleep(1);
		};

		relaxd->stopAcquisition();


		int chipCount = relaxd->chipCount();
		int iErrCode = relaxd->readMatrix(m_pData, chipCount * MPIX_PIXELS);

		if (iErrCode || chipCount != 4) { 
		};


		// This illustrates how to put the data in the usual
		// order for an image, rows 0 is the top row

		int width, height;
		if (chipCount > 1) {
			width = 2 * MPIX_COLS;
			height = 2 * MPIX_ROWS;
		}
		else {
			width = MPIX_COLS;
			height = MPIX_ROWS;
		}

		//m_pConvertedData = new i16[width * height]();

		// The chips are placed (for V2 chipboard) like
		// +-----+-----+
		// | -3  | -2  |
		// +-----+-----+
		// | +0  | +1  |
		// +-----+-----+
		// where - means rotated by 180 degrees
		// but for V3 chipboard chips 0 and 1 are swapped
		int places[4][4] = { {511,   0, -1,  1},
							{511, 256, -1,  1},
							{  0, 511,  1, -1},
							{  0, 255,  1, -1} };
		i16* src = m_pData;
		for (int chip = 0; chip < chipCount; chip++) {
			// When a chip is missing, it is skipped in the output
			// and the pos will give back the actual position.

			int pos = relaxd->chipPosition(chip);
			int row = places[pos][0], col = places[pos][1],
				rowstr = places[pos][2], colstr = places[pos][3];
			for (int j = 0; j < MPIX_ROWS; j++) {
				i16* dst = m_pConvertedData + row * width + col;
				for (int i = 0; i < MPIX_COLS; i++) {
					*dst = *(src++);
					dst += colstr;
				}
				row += rowstr;
			}
		}


		save_image(_fileName, m_pConvertedData);

	}
	else
		PRINT("Relaxed Module is not connected or properly initialized!");
}

void CTimepix::save_image(std::string&_fileName, const void*_data, uint32_t iSize /*= 512*/, uint16_t type /*= 16*/ )
{
	TinyTIFFWriterFile* tiffWriter = TinyTIFFWriter_open(_fileName.c_str(), type, TinyTIFFWriter_Int, 1, iSize, iSize, TinyTIFFWriter_Greyscale);
	if (tiffWriter)
	{
		TinyTIFFWriter_writeImage(tiffWriter, _data);
		TinyTIFFWriter_close(tiffWriter);
	}
	else
		PRINT("Error@CTimePix::SaveImage: Error creating image file.");
}

void CTimepix::grab_image_live_stream()
{
	if (is_relaxd_module_initialized() == false || m_bLiveStreaming)
		return;

	std::string sFileName = "C:/Users/TEM/Documents/Moussa_SoftwareImages/LiveStreaming/LiveStream.tiff";
	bool bStopLoop = false;
	while (bStopLoop == false)
	{
		grab_image_from_detector(sFileName);

		m_bCancopyLiveImg = false; // Don't wanna use a mutex, maybe this can fix a rare crash that happens when changing the contrast ??
		m_oLiveImageMat = cv::imread(sFileName, cv::IMREAD_ANYDEPTH);
		
		// Our timepix detector is rotated with respect to images we see in the fluorescent screen
		// This makes it complicated to move the stage. A fix for OUR setup is to rotate the image 180º and flip it horizontally
		// I believe this is only for IMAGING. For diffraction, this shouldn't be applied.
		ILL_MODE illmode = CTEMControlManager::GetInstance()->get_illumination_mode();
		IMG_MODE imgmode = CTEMControlManager::GetInstance()->get_image_mode();


		if (illmode == TEM_MODE && imgmode == IMAGE_MODE)
		{
			cv::Point2f pt(m_oLiveImageMat.cols / 2., m_oLiveImageMat.rows / 2.);
			cv::Mat rotated = cv::getRotationMatrix2D(pt, 270.0f, 1.0);
			cv::warpAffine(m_oLiveImageMat, m_oLiveImageMat, rotated, cv::Size(m_oLiveImageMat.cols, m_oLiveImageMat.rows));
			cv::flip(m_oLiveImageMat, m_oLiveImageMat, 1);

			m_iWaitkey		= 5;
			m_iExposureTime = m_iExposureTimeImg;
			m_fContrast		= m_fContrastImg;
			m_fBrightness	= m_fBrightnessImg;
			m_bRotateImg = true;
		}
		else
		{
			m_iWaitkey		= 100;
			m_iExposureTime = m_iExposureTimeDiff;
			m_fContrast		= m_fContrastDiff;
			m_fBrightness	= m_fBrightnessDiff;
			m_bRotateImg	= false;

		}

		m_bCancopyLiveImg = true;

		if (m_bLiveStreaming == false)
			bStopLoop = true; // break;
	}
	m_oLiveImageMat.data = 0; // make sure it will return empty
}

cv::Point zoomStart;
cv::Rect zoomRect;
int zoomWidth, zoomHeight;
void onMouse_live_streaming(int event, int x, int y, int flag, void* _vImageInfo)
{
	if (CTimepix::GetInstance()->is_image_rotated_and_flipped())
	{
		// image is rotated (270º) and flipped for convenience, we will need to transform the x and y
		int temp = x;
		x = y;
		y = temp;

	}


	if (event == cv::EVENT_LBUTTONDBLCLK)
	{
		static auto pDC = CDataCollection::GetInstance();
		static auto pTEM = CTEMControlManager::GetInstance(); 
		if (TEMModeParameters::current_parameters == PARAM_DIFFRACTION)
		{
			cv::Point2f newBeamPos(x / IMG_RESIZE_FACTOR_, y / IMG_RESIZE_FACTOR_);

			std::unique_lock<std::mutex> lock(CTimepix::GetInstance()->beam_pos_mtx);

			cv::Point2f dummy;
			auto oldBeamPos = pDC->get_current_beam_screen_coordinates();
			pDC->do_beam_shift_at_coordinates_alternative(newBeamPos, true);

			if (pDC->m_bOnTracking)
			{
				CTimepix::GetInstance()->update_beam_pos = true;
				pDC->m_vShiftOffset_rotation = newBeamPos - oldBeamPos;
			}
			lock.unlock();
		}
		else
		{
			auto pTpx = CTimepix::GetInstance();
			if(pTEM->get_mag_mode() == MAG_MODE_TEM)
			{
				cv::Point2f targetPos = cv::Point2f(x / IMG_RESIZE_FACTOR_, y / IMG_RESIZE_FACTOR_);
				pTpx->m_vTargetBeamCoords.push_back(targetPos);
				printf("New coordinates: %.1f, %.1f have been added to the vector(%d).\n", targetPos.x, targetPos.y, pTpx->m_vTargetBeamCoords.size());
			}
			else
			{
				pTpx->m_vTargetBeamCoords.clear();
				cv::Point2f targetPos = cv::Point2f(x / IMG_RESIZE_FACTOR_, y / IMG_RESIZE_FACTOR_);
				pTpx->m_vTargetBeamCoords.push_back(targetPos);
			}
		}

	}
	else if (event == cv::EVENT_MOUSEWHEEL)
	{
		static auto pDC = CDataCollection::GetInstance();
		static auto pTpx = CTimepix::GetInstance();
		if (pDC->m_bOnTracking == false && TEMModeParameters::current_parameters == 3) // not tracking and diffraction settings loaded
		{
			pTpx->m_iTargetBeamCoordsIndex += cv::getMouseWheelDelta(flag) > 0 ? 1 : -1;

			if (pTpx->m_vTargetBeamCoords.empty() == false)
			{
				pTpx->m_iTargetBeamCoordsIndex = std::clamp(pTpx->m_iTargetBeamCoordsIndex, 0, static_cast<int>(pTpx->m_vTargetBeamCoords.size() - 1));
				auto pos = pTpx->m_vTargetBeamCoords.at(pTpx->m_iTargetBeamCoordsIndex);

				cv::Point2f dummy;
				pDC->do_beam_shift_at_coordinates_alternative(pos, true);
				printf("Pos (%d) --\n", pTpx->m_iTargetBeamCoordsIndex + 1);

			}
		}
				
	}
	else if (event == cv::EVENT_RBUTTONDBLCLK)
	{
		cv::Point2f oldBeamPos = CDataCollection::GetInstance()->get_current_beam_screen_coordinates();
		CDataCollection::GetInstance()->do_set_current_beam_screen_coordinates(x / IMG_RESIZE_FACTOR_, y / IMG_RESIZE_FACTOR_);
		if(TEMModeParameters::current_parameters == PARAM_DIFFRACTION)
			CDataCollection::GetInstance()->m_oDiffractionParams.SaveCurrentParameters();

		CDataCollection::GetInstance()->do_beam_shift_at_coordinates_alternative(oldBeamPos, true);

	}
	else if (event == cv::EVENT_MBUTTONDOWN)
	{
		// Right mouse button is pressed, start the zooming process
		zoomStart = cv::Point(x, y);
	}
	else if (event == cv::EVENT_MBUTTONUP)
	{
		// Right mouse button is released, perform zooming
		cv::Mat zoomedInSquare;

		// Calculate the width and height of the zoom rectangle
		zoomWidth = abs(zoomStart.x - x);
		zoomHeight = abs(zoomStart.y - y);

		if (zoomWidth > 0 && zoomHeight > 0)
		{
			// Create the zoom rectangle
			zoomRect = cv::Rect(std::min(zoomStart.x, x), std::min(zoomStart.y, y), zoomWidth, zoomHeight);
			zoomWidth = zoomHeight = std::max(zoomWidth, zoomHeight);

		}
	}
	else if (event == cv::EVENT_MBUTTONDBLCLK)
	{
		static auto pTpx = CTimepix::GetInstance();

		pTpx->m_vReferencePosition.x = y / IMG_RESIZE_FACTOR_;
		pTpx->m_vReferencePosition.y = x / IMG_RESIZE_FACTOR_;

	}
}


struct DistortionCostFunctor
{
	DistortionCostFunctor(const cv::Point2d& center,
		const std::pair<cv::Point2d, cv::Point2d>& pair)
		: center(center), pair(pair) {
	}

	template <typename T>
	bool operator()(const T* const k, T* residual) const
	{
		T k1 = k[0];
		T k2 = k[1];

		// Apply distortion model to both points
		cv::Point2d p1_distorted = apply_radial_distortion(pair.first, center, k1, k2);
		cv::Point2d p2_distorted = apply_radial_distortion(pair.second, center, k1, k2);

		// Compute radial distances
		T r1 = T(cv::norm(p1_distorted));
		T r2 = T(cv::norm(p2_distorted));

		// Compute angular positions
		T theta1 = T(atan2(p1_distorted.y, p1_distorted.x));
		T theta2 = T(atan2(p2_distorted.y, p2_distorted.x));

		// Residuals
		residual[0] = r1 - r2;
		residual[1] = theta1 - theta2;

		return true;
	}

	const cv::Point2d center;
	const std::pair<cv::Point2d, cv::Point2d> pair;
};


cv::Point2d apply_radial_distortion(const cv::Point2d& point, const cv::Point2d& center, double k1, double k2)
{
	double x = point.x - center.x;
	double y = point.y - center.y;
	double r2 = x * x + y * y;
	double radial_factor = 1 + k1 * r2 + k2 * r2 * r2;
	double x_distorted = x * radial_factor;
	double y_distorted = y * radial_factor;
	return cv::Point2d(x_distorted, y_distorted);
}


// Cost function to minimize: sum of squared differences between distorted distances
double distortion_cost(const cv::Point2d& center, const std::vector<std::pair<cv::Point2d, cv::Point2d>>& friedel_pairs)
{
	double cost = 0.0;
	for (const auto& pair : friedel_pairs)
	{
		// Apply distortion model to both points
		cv::Point2d p1_distorted = apply_radial_distortion(pair.first, center, 0, 0);
		cv::Point2d p2_distorted = apply_radial_distortion(pair.second, center, 0, 0);

		// Compute radial distances
		double r1 = cv::norm(p1_distorted);
		double r2 = cv::norm(p2_distorted);

		// Compute angular positions
		double theta1 = atan2(p1_distorted.y, p1_distorted.x);
		double theta2 = atan2(p2_distorted.y, p2_distorted.x);

		// Add squared differences to cost
		cost += pow(r1 - r2, 2) + pow(theta1 - theta2, 2);
	}
	return cost;
}



// variable to store pairs of reflections coordinates (x1, y1) and (x2, y2)
std::vector<std::pair<cv::Point2f, cv::Point2f>> g_reflectionPairs;
std::vector<cv::Point2f> g_reflection;
cv::Point2f g_mouseCoords;
cv::Point2f g_centralBeamRefined;

void onMouse_live_streaming_debugging(int event, int x, int y, int flag, void* params)
{
	static CCheetah_PeakFinder* pPeakFinder = nullptr;
	static tPeakList* pPeakList = nullptr;
	static bool bCentralBeamRefined = false;

	if (pPeakFinder == nullptr)
	{
		pPeakFinder = CCheetah_PeakFinder::GetInstance();
		if (pPeakList == nullptr)
			pPeakList = pPeakFinder->GetPeakList();
	}
	g_mouseCoords = cv::Point2f(x, y);

	// Let's see if we can somehow measure the distortions from our diffraction patterns...
	if (event == cv::EVENT_LBUTTONDOWN)
	{

		cv::Point2f pos = cv::Point2f(x, y);

		// Let's loop through all the peaks and see if we can find the closest one to the clicked position
		float minDist = FLT_MAX;
		int minIndex = -1;
		for (int i = 0; i < pPeakList->nPeaks; i++)
		{
			cv::Point2f peakPos = cv::Point2f(pPeakList->peak_com_x[i], pPeakList->peak_com_y[i]);
			float dist = cv::norm(pos - peakPos);
			if (dist < minDist && dist < 20.0f) // 
			{
				minDist = dist;
				minIndex = i;
			}
		}
		g_reflection.emplace_back(cv::Point2f(pPeakList->peak_com_x[minIndex], pPeakList->peak_com_y[minIndex]));

		if(g_reflection.size() >= 2) // Then we have a pair of reflections
		{
			g_reflectionPairs.emplace_back(g_reflection[0], g_reflection[1]);
			g_reflection.clear();
		}
	}
	else if (event == cv::EVENT_RBUTTONDOWN)
	{
		// LSQ fitting of all the pairs of reflections to find out the point of intersection
		int num_lines = g_reflectionPairs.size();

		if (num_lines >= 2)
		{
			// Initialize matrices for the least-squares system
			cv::Mat A = cv::Mat::zeros(num_lines, 2, CV_64F); // Coefficient matrix (A_i and B_i)
			cv::Mat b = cv::Mat::zeros(num_lines, 1, CV_64F); // Constant terms (-C_i)

			for (int i = 0; i < num_lines; ++i) {
				// Get points from the pair
				cv::Point2d p1 = g_reflectionPairs[i].first;
				cv::Point2d p2 = g_reflectionPairs[i].second;

				// Calculate the line parameters (Ax + By + C = 0)
				double A_i = p2.y - p1.y;   // A = y2 - y1
				double B_i = p1.x - p2.x;   // B = x1 - x2
				double C_i = p1.y * (p2.x - p1.x) - p1.x * (p2.y - p1.y); // C = y1 * (x2 - x1) - x1 * (y2 - y1)

				// Normalize the coefficients (optional for numerical stability)
				double norm = std::sqrt(A_i * A_i + B_i * B_i);
				A_i /= norm;
				B_i /= norm;
				C_i /= norm;

				// Populate the matrices
				A.at<double>(i, 0) = A_i;
				A.at<double>(i, 1) = B_i;
				b.at<double>(i, 0) = -C_i;
			}

			// Solve the least-squares problem: minimize ||A * [x, y] - b||
			cv::Mat solution;
			cv::solve(A, b, solution, cv::DECOMP_SVD); // Use Singular Value Decomposition for LSQ

			// The intersection point
			g_centralBeamRefined.x = solution.at<double>(0, 0);
			g_centralBeamRefined.y = solution.at<double>(1, 0);
			bCentralBeamRefined = true;
		}else
			printf("Need at least 2 pairs of reflections to refine the central beam.\n");
	}
	else if (GetAsyncKeyState(VK_XBUTTON1) & 0x1)
	{
		if (g_reflection.empty())
			g_reflectionPairs.clear();
		else
			g_reflection.clear();
	}
	else if (event == cv::EVENT_LBUTTONDBLCLK)
	{
		// Ensure we have enough reflection pairs
		int num_pairs = g_reflectionPairs.size();

		if (num_pairs >= 2)
		{
			// Initial guesses for distortion parameters
			double rx = 1.0;  // Elliptical ratio along x (scaling factor)
			double ry = 1.0;  // Elliptical ratio along y (scaling factor)
			double theta = 0.0; // Elliptical angle (in radians)

			// Optimization parameters
			double learning_rate = 0.001;
			int max_iterations = 1000;

			// Gradient descent optimization
			for (int iter = 0; iter < max_iterations; ++iter)
			{
				// Compute gradients (finite difference approximation)
				double drx = 1e-5, dry = 1e-5, dtheta = 1e-5;

				
			}

			// Output the refined distortion parameters
			printf("Refined elliptical ratio (rx): %f\n", rx);
			printf("Refined elliptical ratio (ry): %f\n", ry);
			printf("Refined elliptical angle (theta in degrees): %f\n", theta * 180.0 / CV_PI);

			// Optionally, you can now apply the distortion correction to your image or further processing
			//bDistortionCorrected = true;
		}
		else
		{
			printf("Need at least 2 pairs of reflections to perform distortion correction.\n");
		}
	}
}

void CTimepix::stream_image_live_stream()
{
	if (is_relaxd_module_initialized() == false || m_bLiveStreaming)
		return;
	m_bLiveStreaming = true;

	bool bStopLoop = false;
	std::string LiveStreamWindowName = "Live Streaming";
	cv::namedWindow(LiveStreamWindowName);
	cv::moveWindow(LiveStreamWindowName, 0, -620);
	cv::setMouseCallback(LiveStreamWindowName, onMouse_live_streaming, nullptr);

	std::string ZoomedLivedStreamWindowName = "Zoomed Live Streaming";
	cv::namedWindow(ZoomedLivedStreamWindowName);
	cv::moveWindow(ZoomedLivedStreamWindowName, 0, -620);
	cv::setMouseCallback(ZoomedLivedStreamWindowName, onMouse_live_streaming, nullptr);
	cv::Mat contrastCorrected;

	while (bStopLoop == false)
	{
		if (m_oLiveImageMat.empty() == false)
		{
			if (m_bCancopyLiveImg)
			{
				contrastCorrected = m_oLiveImageMat;
				double max_val;
				minMaxLoc(contrastCorrected, NULL, &max_val);
				contrastCorrected.convertTo(contrastCorrected, CV_16UC1, 65535.0f / max_val); // Adjust contrast automatically

				cv::convertScaleAbs(contrastCorrected, contrastCorrected, m_fContrast / 100.0f, m_fBrightness);
				cv::resize(contrastCorrected, contrastCorrected, cv::Size(), IMG_RESIZE_FACTOR__, IMG_RESIZE_FACTOR__);
				printf("DEBUGGING IMG_RESIZE_FACTOR__ = %d\n", IMG_RESIZE_FACTOR__);

				if (m_bInvertColours)
					cv::bitwise_not(contrastCorrected, contrastCorrected);
			}

			if (zoomRect.x == 0 && zoomRect.y == 0)
			{
				cv::imshow(LiveStreamWindowName, contrastCorrected);
				cv::waitKey(m_iWaitkey);
			}
			else
			{
				cv::Mat zoomedInSquare = contrastCorrected(zoomRect);
				// Resize the cropped region to fit the window
				cv::resize(zoomedInSquare, zoomedInSquare, cv::Size(), 10, 10); // You can adjust the zoom factor here

				// Display the zoomed-in image
				cv::imshow(ZoomedLivedStreamWindowName, zoomedInSquare);
				auto q = cv::waitKey(m_iWaitkey);
				if (q == 'q' || q == 'Q' )
					zoomRect.x = zoomRect.y = 0;
				
			}

		}
		else
			std::this_thread::sleep_for(2s);


		if (m_bLiveStreaming == false)
			bStopLoop = true; // break;
	}
	cv::destroyWindow(LiveStreamWindowName);
}




cv::Mat CTimepix::correct_raw_image(cv::Mat& img_to_correct)
{
	static cv::Mat flatfield_img = cv::imread("Flatfield/Flatfield.tiff", cv::IMREAD_UNCHANGED);
	static float corrFac = 0.885f;
	static float corrFacMid = 1.45f; 
	cv::Mat correctImg;

	//if (GetAsyncKeyState(VK_ADD) & 0x1)
	//	corrFac += 0.001f;
	//else if (GetAsyncKeyState(VK_SUBTRACT) & 0x1)
	//	corrFac -= 0.001f;

	// Check if flatfield image is loaded successfully
	if (flatfield_img.empty())
	{
		std::cerr << "Error: Flatfield image not found or failed to load." << std::endl;
		return img_to_correct;  // Return the uncorrected image if there's an error
	}
	// Flatfield Correction
	img_to_correct.convertTo(img_to_correct, CV_32F);
	flatfield_img.convertTo(flatfield_img, CV_32F);
	img_to_correct = img_to_correct * static_cast<float>(cv::mean(flatfield_img)[0]) / flatfield_img;
	img_to_correct.convertTo(img_to_correct, CV_16U);

	// Dead pixels correction
	//CPostDataCollection::do_dead_pixels_correction(img_to_correct);

	// Cross correction
	CPostDataCollection::do_cross_correction(img_to_correct, correctImg, 1.0f / corrFac, corrFacMid);
	return correctImg;
}

CTimepix* CTimepix::GetInstance()
{
	PRINTD("\t\t\t\tCTimepix::GetInstance()");
	if (m_pTimepix == nullptr)
		m_pTimepix = new CTimepix();
	return m_pTimepix;
}

void CTimepix::prepare_for_live_stream()
{
	// FOR THIS WE NEED TWO FUNCTIONS, RUNNING ON SEPARATE THREADS
	// ONE WILL BE CONSTANTLY READING AND SAVING THE DETECTOR'S MATRIX 
	// WHILE THE OTHER THREAD WILL BE CONSTANTLY SHOWING THE LAST STORED IMAGE
	// TO AVOID DISK USAGE, WE CAN JUST REWRITE THE SAME IMAGE OVER AND OVER AGAIN

	std::thread tGrabImgLoop(&CTimepix::grab_image_live_stream, this);
	tGrabImgLoop.detach();

	std::thread tStreamImgLoop(&CTimepix::stream_image_live_stream, this);
	tStreamImgLoop.detach();

}

void CTimepix::do_reset_matrix()
{
	if (is_relaxd_module_initialized() == false)
		return;
	printf("Resetting matrix...\n");
	m_pRelaxdModule->resetMatrix();
}

void CTimepix::do_reset()
{
	if (is_relaxd_module_initialized() == false)
		return;

	printf("Resetting Tpx...\n");
	m_pRelaxdModule->reset();
	this->init_module();

}

void CTimepix::do_reset_chips()
{
	if (is_relaxd_module_initialized() == false)
		return; 
	printf("Resetting Tpx chips...\n");
	m_pRelaxdModule->resetChips();
	this->init_module();

}

void CTimepix::init_module()
{
	system("ping 192.168.33.175");
	// See if the communication with the detector is all set
	if (m_pRelaxdModule->ping() == false)
	{
		PRINT("Ping Failed: Detector is not connected or there is a problem with the NAT");
		return;
	}

	//Initialize the detector - returns 0 on success
	if (m_pRelaxdModule->init())
	{
		PRINT("Init Failed");
		return;
	}



	MpxModule* relaxd = m_pRelaxdModule;
	std::string hwInfo = "Timepix_data/HW.dat";
	std::string dacs = "Timepix_data/SophyPontedera_Settings.bpc.dacs";
	std::string pixels = "Timepix_data/SophyPontedera_Settings.bpc";
	int hwId = NULL; // 0 because we have Timepix1

	int chipCount = relaxd->chipCount();


	int sz = chipCount * MPIX_PIXELS;
	m_pData = new i16[sz];
	if (m_pData)
	{

		//this->do_reset_matrix();
		setHwInfo(*relaxd, hwInfo);
		int iRet = relaxd->setPixelsCfg(pixels);
		//printf("relaxd.setPixelsCfg(pixels): %d\n", iRet);
		if (iRet != MPXERR_NOERROR)
		{
			std::cerr << "Error: setPixelsCfg() failed " << pixels << std::endl;
		}
		else
		{
			//std::cout << "setPixelsCfg() set" << std::endl;
			// NB: after setPixelsCfg, flush the matrix
			//printf("relaxd.readMatrix(data, sz): %d\n", relaxd->readMatrix(m_pData, sz));
			relaxd->resetMatrix();

		}

		setDacs(*relaxd, dacs);

		//Do we even need this???

		int delta_thl = -20;

		// obtain array with current dac values
		int current_dacs[4][20], ndacs;
		for (int i = 0; i < chipCount; i++)
			if (relaxd->readFsr(i, current_dacs[i], &ndacs))
				std::cout << "### readFsr() chip " << i << " failed" << std::endl;

		// adjust the thlfine values
		std::cout << " current thl values: " << std::endl;
		for (int i = 0; i < chipCount; i++) {
			int tmpthl = current_dacs[i][TPX_THLFINE];
			std::cout << tmpthl << ' ';
			int newthl = tmpthl + delta_thl;

			// check if thlfine stays within range
			if (newthl < 0) {
				newthl = 0;
			}
			if (newthl > 1023) {
				newthl = 1023;
			}
			current_dacs[i][TPX_THLFINE] = newthl;
		}
		std::cout << std::endl;

		// update all DAC values
		for (int i = 0; i < chipCount; ++i) {
			if (relaxd->setFsr(i, current_dacs[i]))
				std::cout << "### setFsr() chip " << i << " failed, " << std::endl;
		}
		// check new values
		std::cout << " new thl values: " << std::endl;
		for (int i = 0; i < chipCount; i++) {
			if (relaxd->readFsr(i, current_dacs[i], &ndacs))
				std::cout << "###  readFsr() chip " << i << " failed" << std::endl;
		}
		for (int i = 0; i < chipCount; i++) {
			int tmpthl = current_dacs[i][TPX_THLFINE];
			std::cout << tmpthl << ' ';
		}
		std::cout << std::endl;

		system("CLS");
		PRINT("Communication with Timepix was succesful!");
		if (chipCount == 0)
			PRINT("chipCount == 0 -- Something is probably wrong with the connection!");

		RelaxdInitialized(true);
	}
	else
		PRINT("Error@InitializeRelaxdModule: m_pData == nullptr ");
}



bool CTimepix::tcp_connect_to_server()
{
#if defined _DEBUGGING_
	PRINT("SETTING CONNECTION TO SUCCESSFUL");
	return true;
#endif

	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cerr << "WSAStartup failed with error: " << iResult << std::endl;
		return 1;
	}

	// Create a socket
	m_TpxSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int buf_size = 1024 * 1024 * 20; // 20MB buffer size
	setsockopt(m_TpxSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&buf_size), sizeof(buf_size));

	if (m_TpxSocket == INVALID_SOCKET) {
		printf("socket failed with error: %0x\n", WSAGetLastError());
		WSACleanup();
		return false;
	}


	// Connect to the server
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	inet_pton(AF_INET, "192.168.35.2", &clientService.sin_addr); // IP address of the server
	clientService.sin_port = htons(12195); // port number

	iResult = connect(m_TpxSocket, (SOCKADDR*)&clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
		std::cerr << "connect failed with error: " << WSAGetLastError() << std::endl;
		closesocket(m_TpxSocket);
		WSACleanup();
		return false;
	}
	return true;
}






void CTimepix::tcp_grab_single_image_from_detector(std::string& fileName, int exposureTime /*= -1*/)
{
	if (tcp_is_live_streaming == false) // should we use atomic bool instead? TODO!
	{
		tcp_outPacket.iRequestCode = _GRAB_SINGLE_IMG_REQ_;
		tcp_outPacket.exposureTime = m_iExposureTime;
		tcp_outPacket.num_frames_limit = 0;
		if (exposureTime != -1)
			tcp_outPacket.exposureTime = exposureTime;

		int dataSize = sizeof(outPacket);
		int iResult = send(m_TpxSocket, reinterpret_cast<const char*>(&tcp_outPacket), dataSize, 0);
		if (iResult == SOCKET_ERROR) {
			std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
			//MB("This error shouldn't have happened!");
			closesocket(m_TpxSocket);
			WSACleanup();
			return;
		}
		else
		{
			int received_size = 0;
			while (received_size < dataSize) {
				int bytes_received = recv(m_TpxSocket, reinterpret_cast<char*>(&tcp_inPacket) + received_size, dataSize - received_size, 0);
				if (bytes_received < 0) {
					std::cerr << "Failed to receive data\n";
					//MB("This error shouldn't have happened!");
					closesocket(m_TpxSocket);
					WSACleanup();
					return;
				}
				received_size += bytes_received;
			}
			tcp_imgData = reinterpret_cast<i16*>(tcp_inPacket.buffer);
			
			cv::Mat raw_img = cv::Mat(512, 512, CV_16U, tcp_imgData);
			cv::Mat correctedImg = this->correct_raw_image(raw_img);
			//save_image(fileName, tcp_imgData);
			save_image(fileName, correctedImg.data, 516);

		}
	}
	else
	{
		this->tcp_lock_img_data.store(true);
		std::unique_lock<std::mutex> lock(this->tcp_img_mtx);
		
		tcp_imgData = reinterpret_cast<i16*>(tcp_inPacket.buffer);
		
		cv::Mat raw_img = cv::Mat(512, 512, CV_16U, tcp_imgData);
		cv::Mat correctedImg = this->correct_raw_image(raw_img);
		//save_image(fileName, tcp_imgData);
		save_image(fileName, correctedImg.data, 516);

		lock.unlock();
		this->tcp_lock_img_data.store(false);
	}
}

void CTimepix::tcp_grab_images_for_live_stream()
{
	PRINT("TEST - tcp_grab_images_for_live_stream");
	{
		tcp_outPacket.iRequestCode = _GRAB_MULTIPLE_IMG_REQ_;
		tcp_outPacket.exposureTime = m_iExposureTime;
		tcp_outPacket.num_frames_limit = 0;


		int iResult = send(m_TpxSocket, reinterpret_cast<const char*>(&tcp_outPacket), sizeof(outPacket), 0);
		if (iResult == SOCKET_ERROR) {
			std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
			//MB("This error shouldn't have happened!");
			closesocket(m_TpxSocket);
			WSACleanup();
			return;
		}
		else
		{
			tcp_is_live_streaming = true;
			int dataSize = sizeof(inPacket);
			bool bContinueRecvLoop = true;
			auto pDC = CDataCollection::GetInstance();
			tcp_store_frames_counter = 0;
			while(bContinueRecvLoop)
			{
				int received_size = 0;
				while (received_size < dataSize) {
					int bytes_received = recv(m_TpxSocket, reinterpret_cast<char*>(&tcp_inPacket) + received_size, dataSize - received_size, 0);
					if (bytes_received < 0) {
						std::cerr << "Failed to receive data\n";
						closesocket(m_TpxSocket);
						WSACleanup();
						return;
					}
					received_size += bytes_received;
				}
				if(tcp_lock_img_data)
				{
					std::unique_lock<std::mutex> lock(tcp_img_mtx);
					std::memcpy(tcp_imgData, &tcp_inPacket.buffer, sizeof(tcp_inPacket.buffer));
					lock.unlock();
				}
				else
					std::memcpy(tcp_imgData, &tcp_inPacket.buffer, sizeof(tcp_inPacket.buffer));
				
				
				if (tcp_is_live_streaming == false)
				{
					tcp_outPacket.iRequestCode = _GRAB_MULTIPLE_IMG_STOP_REQ_;
					send(m_TpxSocket, reinterpret_cast<const char*>(&tcp_outPacket), sizeof(outPacket), 0);
					tcp_is_live_streaming = true; // We're still live streaming ...
				}

				if (m_iExposureTime != tcp_outPacket.exposureTime)
				{
					tcp_outPacket.iRequestCode = _UPDATE_EXPOSURE_TIME_REQ_;
					tcp_outPacket.exposureTime = m_iExposureTime;
					if (tcp_outPacket.exposureTime <= 0)
						tcp_outPacket.exposureTime = 1;
					else if(tcp_outPacket.exposureTime > 30000)
						tcp_outPacket.exposureTime = 30000;

					send(m_TpxSocket, reinterpret_cast<const char*>(&tcp_outPacket), sizeof(outPacket), 0);
				}

				// For continuous rotation, we store all the images as we receive them
				if (tcp_store_diffraction_images)
				{
					tcp_store_frames_counter++;
					if (tcp_startCaptureImgTimer)
					{
						tcp_oCaptureTimer.doStart();
						tcp_startCaptureImgTimer = false;
					}
					pDC->raw_img.iCounter = tcp_store_frames_counter;
					std::memcpy(&pDC->raw_img.data, tcp_imgData, sizeof(tcp_inPacket.buffer));
					pDC->raw_img.fAngle = CTEMControlManager::GetInstance()->get_stage_tilt_angle();
					pDC->raw_img.iTotalTime = tcp_oCaptureTimer.returnElapsed();
					pDC->_raw_img_vec.push_back(pDC->raw_img);

					// If we're in precession mode, we don't want to store all the images, but just first one until next tilt
					if (pDC->m_bPrecession)
						tcp_store_diffraction_images = false;

					// In SerialED, as it is implemented, the function could be running for hours, or even overnight. 
					// So we don't and can't be storing all the images. So, we store around 1000 images in the raw_img vector
					// Pause/Abort the stage movement, so that we can nicely check which images to store and which not to store
					if (pDC->m_bSerialEDScanRegions)
					{
						if (pDC->_raw_img_vec.size() >= 500)
						{
							tcp_store_diffraction_images = false;
							CTEMControlManager::GetInstance()->stage_abort_exec();
						}
					}

				}

				if(tcp_inPacket.is_last_packet)
					bContinueRecvLoop = false;
			}
		}
		
		tcp_is_live_streaming = false;
		std::this_thread::sleep_for(500ms);
		tcp_stop_live_stream_display = true;
		ZM(tcp_inPacket);
		ZM(tcp_outPacket);
	}

	return;
}

void applyWindowFunction(cv::Mat& src) {
	// Apply a Hanning window
	cv::Mat hann;
	cv::createHanningWindow(hann, src.size(), CV_32F);
	src = src.mul(hann);
}

cv::Mat computeFFT(const cv::Mat& src) {
	// Convert the image to float type
	cv::Mat floatImg;
	src.convertTo(floatImg, CV_32F);

	// Apply a window function to reduce edge effects
	applyWindowFunction(floatImg);

	// Perform FFT
	cv::Mat planes[] = { floatImg, cv::Mat::zeros(floatImg.size(), CV_32F) };
	cv::Mat complexI;
	cv::merge(planes, 2, complexI);
	cv::dft(complexI, complexI);

	// Compute the magnitude
	cv::split(complexI, planes);
	cv::magnitude(planes[0], planes[1], planes[0]);
	return planes[0];
}

cv::Mat shiftFFT(const cv::Mat& magI) {
	// Shift the quadrants of the FFT image to center the low frequencies
	cv::Mat magIShifted = magI.clone();
	magIShifted = magIShifted(cv::Rect(0, 0, magIShifted.cols & -2, magIShifted.rows & -2));
	int cx = magIShifted.cols / 2;
	int cy = magIShifted.rows / 2;

	cv::Mat q0(magIShifted, cv::Rect(0, 0, cx, cy));
	cv::Mat q1(magIShifted, cv::Rect(cx, 0, cx, cy));
	cv::Mat q2(magIShifted, cv::Rect(0, cy, cx, cy));
	cv::Mat q3(magIShifted, cv::Rect(cx, cy, cx, cy));

	cv::Mat tmp;
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);

	return magIShifted;
}

void showFFT(const cv::Mat& src, const std::string& windowName) {
	// Compute FFT and shift it
	cv::Mat magI = computeFFT(src);
	magI = shiftFFT(magI);

	// Switch to a logarithmic scale
	magI += cv::Scalar::all(1);
	cv::log(magI, magI);

	// Normalize the magnitude image for better visualization
	//cv::normalize(magI, magI, 0, 1, cv::NORM_MINMAX);

	double max_val;
	minMaxLoc(magI, NULL, &max_val);
	magI.convertTo(magI, CV_16UC1, 65535.0f / max_val); // Adjust contrast automatically
	cv::convertScaleAbs(magI, magI, 0.8f * 0.5f * CTimepix::GetInstance()->m_fContrastDiff / 100.0f, CTimepix::GetInstance()->m_fBrightnessDiff);
	

	// Convert to 8-bit for display
	// magI.convertTo(magI, CV_8UC1, 255);

	

	cv::imshow(windowName, magI);
}


void xt_shiftFFT(xt::xarray<std::complex<float>>& arr) {
	int centerX = arr.shape()[0] / 2;
	int centerY = arr.shape()[1] / 2;
	xt::xarray<std::complex<double>> temp = arr;
	xt::view(arr, xt::range(0, centerX), xt::range(0, centerY)) = xt::view(temp, xt::range(centerX, temp.shape()[0]), xt::range(centerY, temp.shape()[1]));
	xt::view(arr, xt::range(centerX, temp.shape()[0]), xt::range(centerY, temp.shape()[1])) = xt::view(temp, xt::range(0, centerX), xt::range(0, centerY));
	xt::view(arr, xt::range(0, centerX), xt::range(centerY, temp.shape()[1])) = xt::view(temp, xt::range(centerX, temp.shape()[0]), xt::range(0, centerY));
	xt::view(arr, xt::range(centerX, temp.shape()[0]), xt::range(0, centerY)) = xt::view(temp, xt::range(0, centerX), xt::range(centerY, temp.shape()[1]));
}


void combinedLoGandGFTT(const cv::Mat& inputImage, std::vector<cv::Point2f>& detectedPeaks, int maxCorners = 20, double qualityLevel = 0.01, double minDistance = 10.0) {
	// Step 1: Convert the image to grayscale if necessary
	cv::Mat gray;
	if (inputImage.channels() == 3) {
		cv::cvtColor(inputImage, gray, cv::COLOR_BGR2GRAY);
	}
	else {
		gray = inputImage.clone();
	}

	// Step 2: Apply Gaussian blur to reduce noise
	cv::Mat blurred;
	cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.0);

	// Step 3: Apply Laplacian to detect edges and enhance features
	cv::Mat laplacian;
	cv::Laplacian(blurred, laplacian, CV_64F, 3);
	cv::Mat absLaplacian;
	cv::convertScaleAbs(laplacian, absLaplacian);

	// Step 4: Normalize the Laplacian image for better contrast
	cv::Mat normalized;
	cv::normalize(absLaplacian, normalized, 0, 255, cv::NORM_MINMAX);
	normalized.convertTo(normalized, CV_8U);

	// Step 5: Apply Good Features to Track on the normalized Laplacian image
	cv::goodFeaturesToTrack(normalized, detectedPeaks, maxCorners, qualityLevel, minDistance);
}	

cv::Mat adaptiveNormalize(const cv::Mat& image, double minIntensityThreshold = 500.0, double scaleFactor = 10.0) {
	double minVal, maxVal;
	cv::minMaxLoc(image, &minVal, &maxVal);

	cv::Mat normalizedImage;
	double minEffectiveVal = maxVal * 0.00f;

	if (maxVal - minVal < minIntensityThreshold) {
		// Low dynamic range case: apply an adaptive scaling factor
		image.convertTo(normalizedImage, CV_16U, scaleFactor, -minEffectiveVal * scaleFactor);
	}
	else {
		// Normal case: normalize to fit within the 14-bit range
		image.convertTo(normalizedImage, CV_16U, std::pow(2, 16) / maxVal, -minEffectiveVal * (std::pow(2, 16) / maxVal));
	}

	return normalizedImage;
}


void CTimepix::tcp_live_stream_images()
{
	bool is_live_fft_window_running = false;
	tcp_stop_live_stream_display = false;
	std::string LiveStreamWindowName = "Live Streaming";
	std::string ZoomLiveStreamWindowName = " Zoomed Live Streaming";
	std::string FFTWindowName = "Live FFT (log scale)";

	CImageManager* pImgMgr = CImageManager::GetInstance();


	cv::namedWindow(LiveStreamWindowName);
#ifndef _DEBUGGING_
	cv::moveWindow(LiveStreamWindowName, 0, -620);
#endif


#ifdef _DEBUGGING_
	cv::setMouseCallback(LiveStreamWindowName, onMouse_live_streaming_debugging, nullptr);
#else
	cv::setMouseCallback(LiveStreamWindowName, onMouse_live_streaming, nullptr);
#endif
	auto pDC = CDataCollection::GetInstance();

#if defined _DEBUGGING_
	PRINT("REMOVE LATER: CALLING DO_TRACK_EX");
	pDC->do_track_ex();
	PRINT("REMOVE LATER: CALLING DO_TRACK_EX");

	std::vector<std::string> files;
#endif // DEBUG

	SCameraLengthSettings m_oCameraSettings(pDC->m_fCameraLength);
	float fLastCameraLength = pDC->m_fCameraLength;
	while (tcp_stop_live_stream_display == false)
	{
		this->tcp_lock_img_data = true;
		std::unique_lock<std::mutex> lock(tcp_img_mtx);
#ifndef _DEBUGGING_
		m_oLiveImageMat = cv::Mat(512, 512, CV_16U, tcp_imgData);
#else
		static bool bOnce = false;

		if (bOnce == false)
		{
			// Read a list of .tiff files in a directory and load them one by one
			//std::string path = "C:/Users/Administrator/OneDrive - Fondazione Istituto Italiano Tecnologia/Documents/Temporary_Working_Files/LuAg/2023_07_01/002/tiff_corrected";
			//std::string path = "C:/Users/Administrator/OneDrive - Fondazione Istituto Italiano Tecnologia/Documents/3D ED Data/ULL_MOF/2023_07_15/022/tiff_corrected";
			//std::string path = "C:/Users/Administrator/OneDrive - Fondazione Istituto Italiano Tecnologia/Documents/3D ED Data/ICMOL_6/790/2024_06_10/001/tiff_corrected";
			//std::string path = "C:/Users/Administrator/OneDrive - Fondazione Istituto Italiano Tecnologia/Documents/3D ED Data/test/";
			//std::string path = "C:/SerialED/453e/2024_10_12/002_SerialED";
			std::string path = "C:/SerialED/001_SerialED/distortions/4";
			//std::string path = "C:/SerialED/006";
			pDC->m_bSerialEDScanRegions = true;

			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				if (entry.path().extension() == ".tiff")
					files.push_back(entry.path().string());
			}
			bOnce = true;
		}
		
		// Now we load the images one by one
		//m_oLiveImageMat = cv::imread(files[rand() % files.size()], cv::IMREAD_UNCHANGED);
		static int i = 0;
		m_oLiveImageMat = cv::imread(files[i], cv::IMREAD_ANYDEPTH);
		//m_oLiveImageMat = cv::imread("C:/SerialED/453e/2024_10_12/003_SerialED/0000000000.tiff", cv::IMREAD_UNCHANGED);
		//if (GetAsyncKeyState(VK_MBUTTON) & 0x1)
			i++;
		i = 4;
		if (i >= files.size())
			i = 0;

#endif
		cv::Mat correctImg = m_oLiveImageMat.clone();

		

		lock.unlock();
		this->tcp_lock_img_data = false;

		if (m_oLiveImageMat.empty() == false)
		{

#ifndef _DEBUGGING_
			correctImg = correct_raw_image(correctImg);
			//cv::Mat crossCorr;
			//CPostDataCollection::do_flatfield_correction(correctImg);
			//CPostDataCollection::do_dead_pixels_correction(correctImg);
			//CPostDataCollection::do_cross_correction(correctImg, crossCorr);
			//correctImg = crossCorr;

#else
			if (correctImg.cols == 512 && correctImg.rows == 512)
			{
				cv::Mat crossCorr;
				CPostDataCollection::do_flatfield_correction(correctImg);
				CPostDataCollection::do_dead_pixels_correction(correctImg);
				CPostDataCollection::do_cross_correction(correctImg, crossCorr);
				correctImg = crossCorr;

			}
#endif // !_DEBUGGING_
			


			tcp_rotate_and_flip_image(correctImg);
			

			
			double min_val, max_val;
			minMaxLoc(correctImg, &min_val, &max_val);
			if (max_val < 3)
			{
				correctImg -= max_val;
				max_val = 0;
			}

			//correctImg.convertTo(correctImg, CV_16UC1, std::pow(2,16) / max_val); 
			correctImg = adaptiveNormalize(correctImg);
			cv::resize(correctImg, correctImg, cv::Size(), IMG_RESIZE_FACTOR__, IMG_RESIZE_FACTOR__);
			printf("DEBUGGING IMG_RESIZE_FACTOR__: %d\n", IMG_RESIZE_FACTOR__);
			
			cv::Mat correctImgCopy = correctImg.clone();
			cv::convertScaleAbs(correctImg, correctImg, 0.8f * 0.5f * m_fContrast / 100.0f, m_fBrightness);
			//correctImg.convertTo(correctImg, CV_16U, 1.0f * m_fContrast, m_fBrightness);

			//cv::GaussianBlur(correctImgCopy, correctImgCopy, cv::Size(55, 55), 1);

			//cv::medianBlur(correctImg, correctImg, 5);
			if (m_bInvertColours)
				cv::bitwise_not(correctImg, correctImg);

			//correctImg = pImgMgr->preprocessImageForPeakDetection(correctImg, pDC->m_fSerialED_peaksize, 2);

			if (m_bLiveFFT) {
				//showFFT(m_oLiveImageMat, FFTWindowName);
				
				auto image = m_oLiveImageMat.clone();
				if(image.cols % 2 == 0 && image.rows % 2 == 0) // Make sure that rows & cols are even numbers
				{
					// Convert image to float and normalize
					image.convertTo(image, CV_32F);
					image = image / cv::norm(image, cv::NORM_INF);

					// Adapt cv::Mat to xt::xarray
					xt::xarray<float> arr1 = xt::adapt(reinterpret_cast<float*>(image.data), { static_cast<size_t>(image.rows), static_cast<size_t>(image.cols) });

					// Perform FFT
					auto fft_arr1 = xt::fftw::fft2(xt::xarray<std::complex<float>>(arr1));
					xt_shiftFFT(fft_arr1);

					// Calculate magnitude spectrum
					xt::xarray<double> magnitude_spectrum = xt::abs(fft_arr1);

					// Convert magnitude spectrum to log scale for visualization
					xt::xarray<float> log_magnitude_spectrum = xt::log(magnitude_spectrum + 1e-5);

					// Convert the log magnitude spectrum to cv::Mat for visualization
					cv::Mat log_magnitude_mat(log_magnitude_spectrum.shape()[0], log_magnitude_spectrum.shape()[1], CV_32F, log_magnitude_spectrum.data());

					double max_val;
					cv::minMaxLoc(log_magnitude_mat, nullptr, &max_val);
					log_magnitude_mat.convertTo(log_magnitude_mat, CV_8UC1, 255.0 / max_val);
					if (m_bInvertColours)
						cv::bitwise_not(log_magnitude_mat, log_magnitude_mat);

					// Display the magnitude spectrum
					cv::imshow(FFTWindowName, log_magnitude_mat);
					is_live_fft_window_running = true;
				}

			}
			else
			{
				if (is_live_fft_window_running)
					cv::destroyWindow(FFTWindowName);
				is_live_fft_window_running = false;
			}


			auto crystalPath = pDC->m_oCrystalPathCoordinates; // Default, in diffraction mode
			auto crystalPathSpline = pDC->m_oCrystalPathCoordinatesSpline;

			if(m_bRotateImg)
			{
				crystalPath = pDC->m_oCrystalPathCoordinatesSwapped; // Swapped, in image mode
				crystalPathSpline = pDC->m_oCrystalPathCoordinatesSwappedSpline;
			}
			
			cv::Point2f centralBeamPos;
			if(m_bShowPeaks || m_bShowResolutionRings)
			{
				bool bCentralBeamValid = pImgMgr->find_central_beam_position(correctImgCopy, centralBeamPos);



				if(bCentralBeamValid)
				{
					if (fLastCameraLength != pDC->m_fCameraLength)
					{
						fLastCameraLength = pDC->m_fCameraLength;
						m_oCameraSettings.UpdateCameraLengthSettings(pDC->m_fCameraLength);
					}

					if (m_bShowResolutionRings)
						pImgMgr->draw_resolution_rings(correctImg, centralBeamPos, m_oCameraSettings.flPixelSize / IMG_RESIZE_FACTOR__);

					if (m_bShowPeaks)
					{
						std::vector <cv::Point2f> detectedPeaks, detectedPeaks2;
						std::vector <cv::Point2f> rejectedPeaks, rejectedPeaks2;
						
						pImgMgr->detect_diffraction_peaks_cheetahpf8(correctImgCopy, centralBeamPos, m_oCameraSettings.flPixelSize / IMG_RESIZE_FACTOR__, pDC->m_bSerialEDScanRegions ? pDC->m_fSerialED_dmax : 0.20f, detectedPeaks, pDC->m_fSerialED_i_sigma, pDC->m_fSerialED_peaksize, pDC->m_fSerialED_peakthreshold, 1);

						if (correctImg.channels() == 1)
							cv::cvtColor(correctImg, correctImg, cv::COLOR_GRAY2BGR);
						for (const auto& peak : detectedPeaks) {
							cv::circle(correctImg, peak, pDC->m_fSerialED_peaksize, cv::Scalar(0, 255, 0), 1);
						}

#ifdef _DEBUGGING_

						if (g_reflection.size() == 1)
						{
							cv::circle(correctImg, g_reflection.at(0), pDC->m_fSerialED_peaksize, cv::Scalar(0, 255, 0), -1);
							cv::line(correctImg, g_reflection.at(0), g_mouseCoords, cv::Scalar(0, 255, 0), 2);
						}

						// loop through the g_reflectionPairs and draw a circle around the peaks
						for (auto& reflectionPair : g_reflectionPairs)
						{
							// random color
							auto color = cv::Scalar(rand() % 255, rand() % 255, rand() % 255);
							cv::circle(correctImg, reflectionPair.first, pDC->m_fSerialED_peaksize - 2, cv::Scalar(0, 255, 255), -1);
							cv::circle(correctImg, reflectionPair.second, pDC->m_fSerialED_peaksize - 2, cv::Scalar(0, 255, 255), -1);
							cv::line(correctImg, reflectionPair.first, reflectionPair.second, cv::Scalar(0, 255, 255), 1);
						}
#endif

					}
				}


			}


			if(pDC->m_bOnTracking || GetAsyncKeyState(0x56) & 0x8000) // Key==V -> Show pos and crystal path
			{
				auto a = pDC->get_current_beam_screen_coordinates();
				cv::Point2f pos;
				if(m_bRotateImg)
					pos = cv::Point2f(a.y, a.x);
				else
					pos = cv::Point2f(a.x, a.y);

				if(CTEMControlManager::GetInstance()->is_on_stem_mode() == false)
				{
					//cv::Mat display = correctImg.clone();
					if(correctImg.channels() == 1)
						cv::cvtColor(correctImg, correctImg, cv::COLOR_GRAY2BGR);
					cv::ellipse(correctImg, pos, cv::Size(ImagesInfo::_radius, ImagesInfo::_radius), 0, 0, 360, cv::Scalar(0, 255, 0), 2); // good alternative :)
					cv::polylines(correctImg, crystalPath, false, cv::Scalar(255, 0, 255), 1); // This is the linear path
					cv::polylines(correctImg, crystalPathSpline, false, cv::Scalar(255, 255, 125), 1); // this should be the spline path
					for (const cv::Point& point : crystalPath) {
						cv::circle(correctImg, point, 2, cv::Scalar(255, 0, 0), -1); // Draws a small filled circle at each point
					}

#ifdef _DEBUGGING_
					cv::resize(correctImg, correctImg, cv::Size(), 2, 2);
#endif // DEBUG


				}
				cv::imshow(LiveStreamWindowName, correctImg);
			}
			else
			{
				// Draw text where user has previously double clicked
				for (int i = 0; i < m_pTimepix->m_vTargetBeamCoords.size(); i++)
				{
					auto str = std::to_string(CTEMControlManager::GetInstance()->get_mag_mode() == LOW_MAG_MODE_TEM ? i : i+1);
					cv::putText(correctImg, str, cv::Point(m_pTimepix->m_vTargetBeamCoords.at(i).y, m_pTimepix->m_vTargetBeamCoords.at(i).x),
						cv::FONT_HERSHEY_SIMPLEX, 0.5f, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

				}
				

				cv::imshow(LiveStreamWindowName, correctImg);
				if (zoomRect.x != 0 && zoomRect.y != 0)
				{
					if(zoomRect.width > 0 || zoomRect.height > 0)
					{
						zoomRect.width = zoomRect.height = std::max(zoomWidth, zoomHeight);

						cv::Mat zoomedInSquare = correctImg(zoomRect);
						cv::resize(zoomedInSquare, zoomedInSquare, cv::Size(), correctImg.cols / zoomRect.width, correctImg.rows / zoomRect.height);

						// Display the zoomed-in image
						cv::imshow(ZoomLiveStreamWindowName, zoomedInSquare);
					}

				}
			}

#ifndef _DEBUGGING_
			auto k = cv::waitKey(1);
#else
			auto k = cv::waitKey(100);
#endif
			if(k == 'q' || k == 'Q')
			{
				if (zoomRect.x || zoomRect.y)
				{
					zoomRect.x = zoomRect.y = 0;
					cv::destroyWindow(ZoomLiveStreamWindowName);
				}
			}

			

		}
		else
			std::this_thread::sleep_for(2s);

	}
	//cv::destroyWindow(LiveStreamWindowName);
	cv::destroyAllWindows();

}

void CTimepix::tcp_rotate_and_flip_image(cv::Mat& img)
{
	// Our timepix detector is rotated with respect to images we see in the fluorescent screen
		// This makes it complicated to move the stage. A fix for OUR setup is to rotate the image 180º and flip it horizontally
		// I believe this is only for IMAGING. For diffraction, this shouldn't be applied.
	ILL_MODE illmode;
	IMG_MODE imgmode;
	MAG_MODE magmode;
	if(tcp_store_diffraction_images == false)
	{
		illmode = CTEMControlManager::GetInstance()->get_illumination_mode();
		imgmode = CTEMControlManager::GetInstance()->get_image_mode();
		magmode = CTEMControlManager::GetInstance()->get_mag_mode();
	}
	else
	{
		illmode = TEM_MODE;
		imgmode = DIFFRACTION_MODE;
	}

	if (illmode == TEM_MODE && imgmode == IMAGE_MODE)
	{
		if(magmode != LOW_MAG_MODE_TEM)
		{
			cv::Point2f pt(img.cols / 2., img.rows / 2.);
			cv::Mat rotated = cv::getRotationMatrix2D(pt, 270.0f, 1.0);
			cv::warpAffine(img, img, rotated, cv::Size(img.cols, img.rows));
			cv::flip(img, img, 1);

		}else
		{
			cv::flip(img, img, 0); // Makes it easier to move the stage			
		}

		m_iWaitkey = 5;
		m_iExposureTime = m_iExposureTimeImg;
		m_fContrast = m_fContrastImg;
		m_fBrightness = m_fBrightnessImg;
		m_bRotateImg = true;
		

	}
	else
	{
		m_iWaitkey = 100;
		m_iExposureTime = m_iExposureTimeDiff;
		m_fContrast = m_fContrastDiff;
		m_fBrightness = m_fBrightnessDiff;
		m_bRotateImg = false;

	}
}

void CTimepix::tcp_prepare_for_live_stream()
{
	std::thread tGrabImgLoop(&CTimepix::tcp_grab_images_for_live_stream, this);
	tGrabImgLoop.detach();

	std::thread tStreamImgLoop(&CTimepix::tcp_live_stream_images, this);
	tStreamImgLoop.detach();
}

CTimepix::~CTimepix()
{
	PRINTD("\t\t\t\tCTimepix::~CTimepix() - Destructor");
	SAFE_RELEASE(m_pRelaxdModule);

	if (m_pData)
	{
		delete[] m_pData;
		m_pData = nullptr;
	}

	if (m_pConvertedData)
	{
		delete[] m_pConvertedData;
		m_pConvertedData = nullptr;
	}

	if (tcp_imgData)
	{
		delete[] tcp_imgData;
		tcp_imgData = nullptr;
	}
	closesocket(m_TpxSocket);
	WSACleanup();
}


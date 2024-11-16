#include "pch.h"
#define IMG_RESIZE_FACTOR 1
CImageManager* CImageManager::m_pImgMgr = nullptr;
std::vector<cv::RotatedRect> CImageManager::m_ellipses;

CImageManager::CImageManager()
{
    static bool bDoOnce = false;
    if (bDoOnce == false)
    {
        bDoOnce = true;
		

		m_ellipses.reserve(100);
    }
}

void MouseEventHandler_STEM(int event, int x, int y, int flag, void* _vImageInfo)
{
	if(ImagesInfo::_purpose == STEM_TRACKING)
	{
		ImagesInfo* pImgInf = reinterpret_cast<ImagesInfo*>(_vImageInfo);
		pImgInf->_center = cv::Point(x / IMG_RESIZE_FACTOR, y / IMG_RESIZE_FACTOR);
		if (event == cv::EVENT_LBUTTONDOWN)
		{
			pImgInf->_iPosX = static_cast<int>(x / IMG_RESIZE_FACTOR);
			pImgInf->_iPosY = static_cast<int>(y / IMG_RESIZE_FACTOR);
			pImgInf->_bIsImgValid = true;
			cv::destroyWindow(pImgInf->_sWindowTitle);
		}
		else if (event == cv::EVENT_RBUTTONDBLCLK)
		{
			pImgInf->_iPosX = pImgInf->_iPosY = -2;
			pImgInf->_bIsImgValid = false;
			cv::destroyWindow(pImgInf->_sWindowTitle);
		}
	}

}

void MouseEventHandler_TEM(int event, int x, int y, int flag, void* _vImageInfo)
{
	ImagesInfo* pImgInf = reinterpret_cast<ImagesInfo*>(_vImageInfo);
    if (ImagesInfo::_purpose == TEM_BEAM_CALIB)
    {
		if (event == cv::EVENT_LBUTTONDOWN) {
			// Set center of circle on left mouse click
			pImgInf->_center = cv::Point(x / IMG_RESIZE_FACTOR, y / IMG_RESIZE_FACTOR);
		}
		else if (event == cv::EVENT_LBUTTONDBLCLK) {
			// Set center of circle on left mouse click
			pImgInf->_center = cv::Point(x / IMG_RESIZE_FACTOR, y / IMG_RESIZE_FACTOR);
            pImgInf->_iPosX = static_cast<int>(x / IMG_RESIZE_FACTOR);
            pImgInf->_iPosY = static_cast<int>(y / IMG_RESIZE_FACTOR);
			pImgInf->_bIsImgValid = true;
			cv::destroyWindow(pImgInf->_sWindowTitle);

		}
		else if (event == cv::EVENT_MBUTTONDBLCLK)
		{
			// If we're OK with the automatic finder
			pImgInf->_iPosX = pImgInf->_center.x;
			pImgInf->_iPosY = pImgInf->_center.y;
			pImgInf->_bIsImgValid = true;
			cv::destroyWindow(pImgInf->_sWindowTitle);
		}

    }
    else
    {
		pImgInf->_center = cv::Point(x / IMG_RESIZE_FACTOR, y / IMG_RESIZE_FACTOR);
		if (event == cv::EVENT_LBUTTONDOWN) {
			// Set center of circle on left mouse click
			pImgInf->_iPosX = static_cast<int>(x / IMG_RESIZE_FACTOR);
			pImgInf->_iPosY = static_cast<int>(y / IMG_RESIZE_FACTOR);
			pImgInf->_bIsImgValid = true;
			cv::destroyWindow(pImgInf->_sWindowTitle);
		}
		else if (event == cv::EVENT_RBUTTONDBLCLK)
		{
			pImgInf->_iPosX = pImgInf->_iPosY = -2;
			pImgInf->_bIsImgValid = false;
			cv::destroyWindow(pImgInf->_sWindowTitle);
		}
    }
	
	if (event == cv::EVENT_MOUSEWHEEL) {
		// Adjust radius of circle on mouse scroll up/down
        if (ImagesInfo::_purpose == TEM_BEAM_CALIB)
            pImgInf->_radius += cv::getMouseWheelDelta(flag) > 0 ? 1 : -1;
	}

}

void CImageManager::display_image(std::vector<ImagesInfo>* _vImages)
{
    ImagesInfo::_bSuccess = true; // Assume that everything will go just fine.
    for (auto& v : *_vImages)
    {
		if (ImagesInfo::_purpose == TEM_BEAM_CALIB || ImagesInfo::_purpose == TEM_TRACKING)
		{
			myImg = cv::imread(v._sFileName, cv::IMREAD_UNCHANGED);

			if(v._bIsLowMagImg == false)
			{
				cv::Point2f pt(myImg.cols / 2., myImg.rows / 2.);
				cv::Mat rotated = cv::getRotationMatrix2D(pt, 270.0f, 1.0);
				cv::warpAffine(myImg, myImg, rotated, cv::Size(myImg.cols, myImg.rows));
				cv::flip(myImg, myImg, 1);
			}
			else
			{
				/*cv::flip(myImg, myImg, 0); // horizontally
				cv::Point2f pt(myImg.cols / 2., myImg.rows / 2.);
				cv::Mat rotated;
				if(v._fTEMMagnification == 1600.0f)
					rotated = cv::getRotationMatrix2D(pt, +107, 1.0); // For Mag=1600
				else if(v._fTEMMagnification == 800.0f)
					rotated = cv::getRotationMatrix2D(pt, +91, 1.0); // For Mag=800
				else
				{
					printf("Rotation angle for this magnification is not calibrated. Using +90º instead.");
					rotated = cv::getRotationMatrix2D(pt, +90, 1.0); // For Mag=1600
				}
				cv::warpAffine(myImg, myImg, rotated, cv::Size(myImg.cols, myImg.rows));*/
			}

			double max_val;
			minMaxLoc(myImg, NULL, &max_val);
			myImg.convertTo(myImg, CV_16UC1, 65535.0f / max_val); // Adjust contrast automatically
			cv::convertScaleAbs(myImg, myImg, 0.8f * 0.5f * 1.0f / 100.0, CTimepix::GetInstance()->m_fBrightnessImg);

		}
		else
			myImg = cv::imread(v._sFileName);

		
		auto str = v._sFileName.substr(v._sFileName.find_last_of("/", v._sFileName.length()) + 1,
		                               v._sFileName.find_last_of(".") - v._sFileName.find_last_of("/") - 1) + " -> Angle: " + std::to_string(v._fImageAngle) + " Deg";
		printf("Current Image: %s\n", str.c_str());
		//v._sWindowTitle= v._sFileName.substr(v._sFileName.find_last_of("/", v._sFileName.length()) + 1,
        //                                    v._sFileName.find_last_of(".") - v._sFileName.find_last_of("/") - 1) + " @ " + std::to_string(v._fImageAngle);
		v._sWindowTitle = "Click on desired position!";
		cv::namedWindow(v._sWindowTitle);

        if(ImagesInfo::_purpose == STEM_TRACKING)
		{
			cv::moveWindow(v._sWindowTitle, DESKTOSTEMX, DESKTOSTEMY);
			cv::setMouseCallback(v._sWindowTitle, MouseEventHandler_STEM, &v);

			int iCounter = 0;
			while (true)
			{
				cv::Mat display = myImg.clone();
				//cv::circle(display, v._center, ImagesInfo::_radius, cv::Scalar(125, 0, 125), 2); // for some reason crashes....
				//cv::cvtColor(display, display, cv::COLOR_GRAY2BGR); // Convert grayscale image to color image
				cv::ellipse(display, v._center, cv::Size(15, 15), 0, 0, 360, cv::Scalar(0, 255, 0), 2); // 

				auto display_clone = display.clone();
				if (v._bIsShowCrystalPath)
				{
					cv::polylines(display, CDataCollection::GetInstance()->m_oCrystalPathCoordinates, false, cv::Scalar(255, 0, 255), 1); // Linear path
					cv::polylines(display, CDataCollection::GetInstance()->m_oCrystalPathCoordinatesSpline, false, cv::Scalar(255, 255, 125), 1); // Spline path

					for (const cv::Point& point : CDataCollection::GetInstance()->m_oCrystalPathCoordinates) {
						cv::circle(display, point, 2, cv::Scalar(255, 0, 0), -1); // Draws a small filled circle at each point
					}
				}
				cv::imshow(v._sWindowTitle, display);
				auto q = cv::waitKey(10); // User has two minutes per image to decide where to click!
				if (iCounter >= 12000 || q == 'q' || q == 'Q' || v._iPosX == -2 || v._iPosY == -2)
				{
					PRINT("You have not chosen the point of interest in the previous image during the last 2 minutes. Aborting...");
					cv::destroyWindow(v._sWindowTitle);
					ImagesInfo::_bSuccess = false;
					break;
				}
				else if (v._iPosX >= 0 || v._iPosY >= 0)
				{
					if(v._bSaveVisuals)
					{
						std::string newName = v._sFileName.substr(0, v._sFileName.length() - 9);
						newName = newName + "_" + "_pos.png";
						cv::imwrite(newName, myImg);
					}
					break;
				}
				iCounter++;
			}
        }
        else // if (ImagesInfo::_purpose == TEM_BEAM_CALIB || ImagesInfo::_purpose == TEM_TRACKING)
        {
			cv::resize(myImg, myImg, cv::Size(), IMG_RESIZE_FACTOR_, IMG_RESIZE_FACTOR_);
			
			// Try to find the beam automatically
			if(ImagesInfo::_purpose == TEM_BEAM_CALIB)
			{
				cv::Mat processedImg; // I think there is no need to blur the img
				myImg.convertTo(processedImg, CV_8U, 1.0 / 256.0);
			
				if(v._radius < 0.015f)
					find_beam_position_hough_circles(processedImg, v); // Use this only if the radius is not set (the first time)
				find_beam_position_center_gravity(processedImg, v); // This one seems to be more robust/accurate
			}


			// cv::moveWindow(v._sWindowTitle, 0, -620);
			cv::moveWindow(v._sWindowTitle, 0, -620);
			cv::setMouseCallback(v._sWindowTitle, MouseEventHandler_TEM, &v);
			

			int iCounter = 0;
			float fLowMagScaleFactor = 0.98f;
            while (true)
            {
				cv::Mat display = myImg.clone();
				//cv::circle(display, v._center, ImagesInfo::_radius, cv::Scalar(125, 0, 125), 2); // for some reason crashes....
				cv::cvtColor(display, display, cv::COLOR_GRAY2BGR); // Convert grayscale image to color image
				int radius = v._bIsLowMagImg ? ImagesInfo::_radius / fLowMagScaleFactor : ImagesInfo::_radius;
				cv::ellipse(display, v._center, cv::Size(radius, radius), 0, 0, 360, cv::Scalar(0, 255, 0), 2); // good alternative :)


				auto display_clone = display.clone();
				if (v._bIsShowCrystalPath)
				{
					cv::polylines(display, CDataCollection::GetInstance()->m_oCrystalPathCoordinatesSwapped, false, cv::Scalar(255, 0, 255), 1); // Linear Path
					cv::polylines(display, CDataCollection::GetInstance()->m_oCrystalPathCoordinatesSwappedSpline, false, cv::Scalar(255, 255, 125), 1); // Spine Path
					for (const cv::Point& point : CDataCollection::GetInstance()->m_oCrystalPathCoordinatesSwapped) {
						cv::circle(display, point, 2, cv::Scalar(255, 0, 0), -1); // Draws a small filled circle at each point
					}
				}
				cv::imshow(v._sWindowTitle, display);

				auto q = cv::waitKey(10); // User has two minutes per image to decide where to click!
				iCounter++;
				if (v._iPosX >= 0 || v._iPosY >= 0)
				{
					int temp = v._iPosX;
					v._iPosX = v._center.x = v._iPosY;
					v._iPosY = v._center.y = temp;
					
					if (v._bIsLowMagImg)
					{
						cv::Point2d correctedCoords;
						if(v._fTEMMagnification == 1600.0f)
							correctedCoords = correct_lowmag_normalmag(v._center, 107.0f) * fLowMagScaleFactor; // 0.98f is a correction factor for the low mag images
						else if(v._fTEMMagnification == 800.0f)
							correctedCoords = correct_lowmag_normalmag(v._center, 91.0f);
						else
						{
							printf("Rotation angle for this magnification is not calibrated. Using +90 degrees instead.\n");
							correctedCoords = correct_lowmag_normalmag(v._center, 90.0f);
						}
						v._iPosX = v._center.x = correctedCoords.x;
						v._iPosY = v._center.y = correctedCoords.y;

					}
					if(v._bSaveVisuals)
					{
						std::string newName = v._sFileName.substr(0, v._sFileName.length() - 9);
						newName = newName + "_" + "_pos.png";
						cv::imwrite(newName, display_clone);
					}

					break;
				}
				else if (v._iPosX == -2 && v._iPosY == -2)
					break;

				if (q == 'q' || q == 'Q' || iCounter > 12000)
				{
					PRINT("You have chosen to quit or have not clicked on the point of interest in the previous image during the last 2 minutes. Aborting...");
					cv::destroyWindow(v._sWindowTitle);
					ImagesInfo::_bSuccess = false;
					break;
				}

				
            }
        }
    }



}


cv::Mat CImageManager::poly_fit(std::vector<cv::Point>& points, unsigned int poly_order)
{
	// Fit a polynomial to the points
	cv::Mat A(points.size(), poly_order + 1, CV_64F);
	cv::Mat b(points.size(), 1, CV_64F);

	for (int i = 0; i < points.size(); i++)
	{
		double* a_ptr = A.ptr<double>(i);
		a_ptr[0] = 1.0;

		for (int j = 1; j <= poly_order; j++)
			a_ptr[j] = a_ptr[j - 1] * points[i].x;

		b.at<double>(i, 0) = points[i].y;
	}

	cv::Mat coeffs;
	cv::solve(A, b, coeffs, cv::DECOMP_QR);
	return coeffs;
}


void CImageManager::draw_poly_fit_on_img(cv::Mat& img, cv::Mat& coeffs, cv::Vec3b color, unsigned int poly_order)
{
	for (int x = 0; x < img.cols; x++)
	{
		double y = 0.0;
		for (int i = 0; i <= poly_order; i++)
			y += coeffs.at<double>(i, 0) * pow(x, i);

		cv::Point p(x, cv::saturate_cast<int>(y));
		if (p.y >= 0 && p.y < img.rows)
		{
			img.at<cv::Vec3b>(p.y, p.x) = color;
			img.at<cv::Vec3b>(p.y + 1, p.x + 1) = color;
			img.at<cv::Vec3b>(p.y + 2, p.x + 2) = color;

		}
	}
}


void CImageManager::display_tracking_plots(std::vector<ImagesInfo>* _vImages)
{
	std::string windowName = "Crystal Path Plots";
	cv::namedWindow(windowName);
	cv::moveWindow(windowName, 516, -620);

	cv::Mat img(512, 512, CV_8UC3, cv::Scalar(255, 255, 255));
	std::vector<cv::Point> points;
	for (auto& imgInfo : *_vImages)
	{
		if (imgInfo._bIsImgValid)
		{
			cv::Point2f p(imgInfo._iPosX, imgInfo._iPosY);
			cv::ellipse(img, p, cv::Size(5, 5), 0, 0, 360, cv::Scalar(125, 0, 125), -1); // good alternative :)
			points.push_back(p);
		}
	}

	// Polynomial Fit - Order 5
	auto coeffs = poly_fit(points, 5);
	
	// Plot Curve
	draw_poly_fit_on_img(img, coeffs, cv::Vec3b(0, 0, 255));
	// Plot line
	for (int i = 1; i < points.size(); i++)
		cv::line(img, points[i - 1], points[i], cv::Scalar(125, 125, 0), 2);
	
	cv::resize(img, img, cv::Size(), 0.5f, 0.5f);
	cv::imshow("Crystal Path Plots", img);
	auto key = cv::waitKey(7000);
	cv::destroyWindow("Crystal Path Plots");
	
	printf("returning...\n");
	return;
}

cv::Point2d CImageManager::calculate_image_shift(cv::Mat& _ImgRef, cv::Mat& _Img)
{
    // Convert to gray color space
	cv::cvtColor(_ImgRef, _ImgRef, CV_BGR2GRAY);
	cv::cvtColor(_Img, _Img, CV_BGR2GRAY);

    // convert to 32F
	_ImgRef.convertTo(_ImgRef, CV_32F);
	_Img.convertTo(_Img, CV_32F);

    // Calculates the shift between the two images
    return cv::phaseCorrelate(_ImgRef, _Img);
}

void CImageManager::display_image_ex(std::vector<ImagesInfo>* _vImages, ImgInfoPurpose purpose)
{
	CTEMControlManager::GetInstance()->do_blank_beam(true);
	ImagesInfo::_purpose = purpose;


	std::thread t(&CImageManager::display_image, this, std::ref(_vImages));
	t.join();

	std::this_thread::sleep_for(500ms);

}

cv::Point2d CImageManager::calculate_image_shift_ex(std::string& _sReferenceImg, std::string& _sTargetImg)
{
    cv::Mat ImgRef = cv::imread(_sReferenceImg);
    cv::Mat ImgTarget = cv::imread(_sTargetImg);

    if (ImgRef.empty() || ImgTarget.empty())
        return {0, 0};
    return calculate_image_shift(ImgRef, ImgTarget);
}

cv::Point2d CImageManager::correct_lowmag_normalmag(cv::Point2d _originalCoords, float _correctionAngle)
{
	cv::Point2d flippedHorizontally = { -_originalCoords.x + 516, _originalCoords.y };

	cv::Point2d corrected;
	corrected.x = (flippedHorizontally.x - 256) * cos(_correctionAngle * CV_PI / 180.0f) - (flippedHorizontally.y - 256) * sin(_correctionAngle * CV_PI / 180.0f) + 256;
	corrected.y = (flippedHorizontally.x - 256) * sin(_correctionAngle * CV_PI / 180.0f) + (flippedHorizontally.y - 256) * cos(_correctionAngle * CV_PI / 180.0f) + 256;

	return corrected;
}

bool CImageManager::find_central_beam_position(const cv::Mat& imgInput, cv::Point2f& beamLocation)
{
	try {
		if (imgInput.empty()) {
			std::cerr << "Error: Input image is empty." << std::endl;
			return false;
		}

		cv::Mat grayImage;
		if (imgInput.channels() != 1) {
			if (imgInput.channels() == 3) {
				cv::cvtColor(imgInput, grayImage, cv::COLOR_BGR2GRAY);
			}
			else if (imgInput.channels() == 4) {
				cv::cvtColor(imgInput, grayImage, cv::COLOR_BGRA2GRAY);
			}
			else {
				return false;
			}
		}
		else {
			grayImage = imgInput.clone();
		}

		cv::Mat grayFloat;
		grayImage.convertTo(grayFloat, CV_32F);
		cv::GaussianBlur(grayFloat, grayFloat, cv::Size(5, 5), 0);

		double minVal, maxVal;
		cv::minMaxLoc(grayFloat, &minVal, &maxVal);
		if (maxVal - minVal < 1e-5) {
			//std::cerr << "Error: Image has no variation in intensity." << std::endl;
			return false;
		}
		cv::Mat normalizedImage = (grayFloat - static_cast<float>(minVal)) / static_cast<float>(maxVal - minVal);

		cv::Point2f imageCenter(imgInput.cols / 2.0f, imgInput.rows / 2.0f);

		float searchRadius = 500.0f; // Adjust based on expected beam size
		cv::Mat mask = cv::Mat::zeros(normalizedImage.size(), CV_8UC1);
		cv::circle(mask, imageCenter, static_cast<int>(searchRadius), 255, -1, cv::LINE_AA);

		double localMin, localMax;
		cv::Point maxLoc;
		cv::minMaxLoc(normalizedImage, &localMin, &localMax, nullptr, &maxLoc, mask);

		int regionSize = 20; 
		int xStart = std::max(maxLoc.x - regionSize, 0);
		int yStart = std::max(maxLoc.y - regionSize, 0);
		int xEnd = std::min(maxLoc.x + regionSize, normalizedImage.cols - 1);
		int yEnd = std::min(maxLoc.y + regionSize, normalizedImage.rows - 1);

		cv::Rect roi(xStart, yStart, xEnd - xStart + 1, yEnd - yStart + 1);
		cv::Mat region = normalizedImage(roi);

		cv::Moments moments = cv::moments(region, true);
		if (moments.m00 == 0) {
			//std::cerr << "Error: Zero moments, cannot compute centroid." << std::endl;
			return false;
		}

		float cx = static_cast<float>(moments.m10 / moments.m00) + xStart;
		float cy = static_cast<float>(moments.m01 / moments.m00) + yStart;

		beamLocation = cv::Point2f(cx, cy);

		CDataCollection::GetInstance()->m_fCentralBeamTolerance = 40.0f;
		if(CDataCollection::GetInstance()->m_bSerialEDScanRegions)
			CDataCollection::GetInstance()->m_fCentralBeamTolerance = 20.0f;

		float actualDistance = cv::norm(beamLocation - imageCenter);
		if (actualDistance > CDataCollection::GetInstance()->m_fCentralBeamTolerance ) {
			//std::cerr << "Error: Beam location is too far from image center (" << actualDistance << " pixels)." << std::endl;
			
			return true;
		}

		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "CImageManager::find_central_beam_position " << e.what() << std::endl;
		return false;
	}
}

bool CImageManager::detect_diffraction_peaks_old_champ(const cv::Mat& imgInput, const cv::Point2f& centralBeam, double pixelSize, double resolutionLimit, std::vector<cv::Point2f>& detectedPeaks, int maxCorners, double qualityLevel /*= 0.2*/ , double minDistance /*= 10.0*/)
{
	try {
		cv::Mat grayImg;
		if (imgInput.channels() == 3) {
			cv::cvtColor(imgInput, grayImg, cv::COLOR_BGR2GRAY);
		}
		else {
			grayImg = imgInput.clone();
		}

		cv::Mat normalizedImg;
		cv::normalize(grayImg, normalizedImg, 0, 255, cv::NORM_MINMAX);
		normalizedImg.convertTo(normalizedImg, CV_8U);

		// Create a Mask to Exclude the Central Beam Region up to the Resolution Limit
		double resolutionRadiusPixels = resolutionLimit / pixelSize;
		cv::Mat mask = cv::Mat::ones(normalizedImg.size(), CV_8U) * 255;
		cv::circle(mask, centralBeam, static_cast<int>(resolutionRadiusPixels), cv::Scalar(0), -1);

		cv::Mat maskedImg;
		normalizedImg.copyTo(maskedImg, mask); // Apply mask


		// Detect Corners/Peaks Using Good Features To Track
		cv::goodFeaturesToTrack(normalizedImg, detectedPeaks, maxCorners, qualityLevel, minDistance, mask);

		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception in CImageManager::detectDiffractionPeaks: " << e.what() << std::endl;
		return false;
	}
}

bool CImageManager::detect_diffraction_peaks_old(const cv::Mat& imgInput, std::vector<cv::Point2f>& detectedPeaks)
{
	try {
		// Convert the image to grayscale if it's not already
		cv::Mat gray;
		if (imgInput.channels() == 3) {
			cv::cvtColor(imgInput, gray, cv::COLOR_BGR2GRAY);
		}
		else {
			gray = imgInput.clone();
		}

		// Apply a Gaussian blur to reduce noise and avoid false detections
		cv::Mat blurred;
		cv::GaussianBlur(gray, blurred, cv::Size(7, 7), 1.0);

		if (blurred.type() != CV_8U) {
			blurred.convertTo(blurred, CV_8U);
		}

		cv::Mat centralBinary;
		double centralThresholdValue = cv::threshold(blurred, centralBinary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

		// Find contours in the binary image for the central beam
		std::vector<std::vector<cv::Point>> centralContours;
		cv::findContours(centralBinary, centralContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// Identify the central beam as the largest contour
		cv::Rect centralROI;
		if (!centralContours.empty()) {
			centralROI = cv::boundingRect(*std::max_element(centralContours.begin(), centralContours.end(), [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
				return cv::contourArea(a) < cv::contourArea(b);
				}));
		}

		// Apply a lower threshold to find additional peaks
		cv::Mat additionalBinary;
		double additionalThresholdValue = centralThresholdValue * 0.15; // Using additionalPeakThresholdScale
		cv::threshold(blurred, additionalBinary, additionalThresholdValue, 255, cv::THRESH_BINARY);

		// Find contours in the binary image for additional peaks
		std::vector<std::vector<cv::Point>> additionalContours;
		cv::findContours(additionalBinary, additionalContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// Check if any contours are outside the central region and add detected peaks
		detectedPeaks.clear();
		for (const auto& contour : additionalContours) {
			cv::Rect boundingBox = cv::boundingRect(contour);
			cv::Point2f center(boundingBox.x + boundingBox.width / 2.0f, boundingBox.y + boundingBox.height / 2.0f);
			if (!centralROI.contains(center)) {
				detectedPeaks.push_back(center);
			}
		}

		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception in CImageManager::detectDiffractionPeaks: " << e.what() << std::endl;
		return false;
	}
}

// Function to initialize histogram
std::vector<int> initializeHistogram(const cv::Mat& window, int numBins, int histRangeEnd) {
	std::vector<int> histogram(numBins, 0);
	for (int y = 0; y < window.rows; ++y) {
		for (int x = 0; x < window.cols; ++x) {
			int intensity = window.at<uchar>(y, x);
			int bin = (intensity * numBins) / histRangeEnd;
			histogram[bin]++;
		}
	}
	return histogram;
}

// Function to update histogram
void updateHistogram(std::vector<int>& histogram, const std::vector<uchar>& incomingPixels, const std::vector<uchar>& outgoingPixels, int numBins, int histRangeEnd) {
	for (size_t i = 0; i < incomingPixels.size(); ++i) {
		int incomingBin = (incomingPixels[i] * numBins) / histRangeEnd;
		int outgoingBin = (outgoingPixels[i] * numBins) / histRangeEnd;
		histogram[incomingBin]++;
		histogram[outgoingBin]--;
	}
}

// Function to apply Huang's thresholding based on histogram
uchar huangThreshold(const std::vector<int>& histogram, int numBins, int histRangeEnd) {
	// Implementation of Huang's fuzzy thresholding
	// Placeholder: Replace with actual Huang's algorithm
	// For demonstration, we'll use Otsu's threshold
	int total = 0;
	for (auto count : histogram) total += count;
	float sum = 0;
	for (int i = 0; i < numBins; ++i) sum += i * histogram[i];
	float sumB = 0;
	int wB = 0;
	int threshold = 0;
	float varMax = 0;

	for (int t = 0; t < numBins; ++t) {
		wB += histogram[t];
		if (wB == 0) continue;
		int wF = total - wB;
		if (wF == 0) break;
		sumB += t * histogram[t];
		float mB = sumB / wB;
		float mF = (sum - sumB) / wF;
		float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);
		if (varBetween > varMax) {
			varMax = varBetween;
			threshold = t;
		}
	}
	return static_cast<uchar>((threshold * histRangeEnd) / numBins);
}


cv::Mat CImageManager::preprocessImageForPeakDetection(const cv::Mat& grayImg, double reflectionSize, unsigned int binFactor)
{
	static bool bMethodOne = true;

	if (GetAsyncKeyState(VK_NUMPAD9) & 0x1)
	{
		if(bMethodOne)
		{
			bMethodOne = false;
			PRINT("Using Binarization Method");
		}
		else
		{
			bMethodOne = true;
			PRINT("Using Default Method");
		}

	}

	if (!bMethodOne)
	{
		// Check for valid input
		if (grayImg.empty()) {
			std::cerr << "Error: Input image is empty!" << std::endl;
			return cv::Mat();
		}

		cv::Mat preprocessedImg;
		if (grayImg.type() != CV_8U) {
			grayImg.convertTo(preprocessedImg, CV_8U);
		}
		else {
			preprocessedImg = grayImg.clone();
		}

		cv::normalize(preprocessedImg, preprocessedImg, 0, 255, cv::NORM_MINMAX);
		preprocessedImg.convertTo(preprocessedImg, CV_8U);

		if (binFactor > 1) {
			cv::Mat binnedImg;
			cv::Size newSize(preprocessedImg.cols / binFactor, preprocessedImg.rows / binFactor);

			newSize.width = std::max(newSize.width, 1);
			newSize.height = std::max(newSize.height, 1);

			cv::resize(preprocessedImg, binnedImg, newSize, 0, 0, cv::INTER_AREA);
			cv::resize(binnedImg, preprocessedImg, preprocessedImg.size(), 0, 0, cv::INTER_NEAREST);
		}

		// Apply Median Blur
		// Determine the kernel size based on reflection size
		int kernelSize = static_cast<int>(std::round(reflectionSize));
		if (kernelSize % 2 == 0) { // kernel has to be odd
			kernelSize += 1;
		}
		kernelSize = std::max(kernelSize, 3); 

		cv::Mat blurredImg;
		cv::medianBlur(preprocessedImg, blurredImg, kernelSize);
		preprocessedImg = preprocessedImg - blurredImg;

		return preprocessedImg;
	}
	else
	{
		// binarize the image
		cv::Mat binaryImg;
		cv::Mat preprocessedImg;
		//preprocessedImg = grayImg.clone();
		grayImg.convertTo(preprocessedImg, CV_8U);

		//cv::threshold(preprocessedImg, preprocessedImg, 0, 255, cv::THRESH_BINARY);
		cv::normalize(preprocessedImg, preprocessedImg, 0, 255, cv::NORM_MINMAX);
		preprocessedImg.convertTo(preprocessedImg, CV_8U);
		GaussianBlur(preprocessedImg, preprocessedImg, cv::Size(5, 5), 3, 3);
		return preprocessedImg;
	}

}

cv::Vec4d fitLeastSquaresPlane(const std::vector<cv::Point>& bgPoints, const cv::Mat& img) {
	int n = static_cast<int>(bgPoints.size());
	cv::Mat A(n, 3, CV_64F);  
	cv::Mat B(n, 1, CV_64F);  

	for (int i = 0; i < n; ++i) {
		A.at<double>(i, 0) = bgPoints[i].x;  
		A.at<double>(i, 1) = bgPoints[i].y;  
		A.at<double>(i, 2) = 1.0;           
		B.at<double>(i, 0) = img.at<uchar>(bgPoints[i]);  
	}

	cv::Mat coeffs;
	cv::solve(A, B, coeffs, cv::DECOMP_SVD);  // LSQ fitting

	return cv::Vec4d(coeffs.at<double>(0), coeffs.at<double>(1), coeffs.at<double>(2), 0);
}

// Peak position refinement by maximum intensity method
cv::Point refinePeakPositionMaxIntensity(const cv::Mat& roi) {
	double minVal, maxVal;
	cv::Point minLoc, maxLoc;
	cv::minMaxLoc(roi, &minVal, &maxVal, &minLoc, &maxLoc);

	return maxLoc;  // Return the position of the maximum intensity
}

// Peak position refinement by centroid method (using moments)
cv::Point2f refinePeakPositionCentroid(const cv::Mat& roi) {
	cv::Moments m = cv::moments(roi, true);  // Calculate moments

	if (m.m00 != 0) {
		return cv::Point2f(static_cast<float>(m.m10 / m.m00), static_cast<float>(m.m01 / m.m00));
	}
	else {
		return cv::Point2f(roi.cols / 2.0, roi.rows / 2.0);
	}
}

bool CImageManager::detect_diffraction_peaks(const cv::Mat& imgInput, const cv::Point2f& centralBeam, double pixelSize, double resolutionLimit, std::vector<cv::Point2f>& detectedPeaks, std::vector<cv::Point2f>& rejectedPeaks, int maxCorners, float i_sigma, double qualityLevel_contourSize, double minDistance /*= 10.0*/, unsigned int binFactor /*= 2*/, bool bGFTT /*= true*/)
{
    try {
        // Convert image to grayscale if necessary
        cv::Mat grayImg;
        if (imgInput.channels() == 3) {
            cv::cvtColor(imgInput, grayImg, cv::COLOR_BGR2GRAY);
        }
        else {
            grayImg = imgInput.clone();
        }
        // Preprocess the image for peak detection
        cv::Mat preprocessedImg = preprocessImageForPeakDetection(grayImg, qualityLevel_contourSize, binFactor);

        // Create a mask to exclude the central beam and its region up to the resolution limit
        double resolutionRadiusPixels = resolutionLimit / pixelSize;
        cv::Mat mask = cv::Mat::ones(preprocessedImg.size(), CV_8U) * 255;
        cv::circle(mask, centralBeam, static_cast<int>(resolutionRadiusPixels), cv::Scalar(0), -1);

        // Apply the mask to remove the central beam
        cv::Mat maskedImg;
        preprocessedImg.copyTo(maskedImg, mask);

        // Use your bhFindLocalMaximum method to find the initial peak positions
		std::vector<cv::Point> initialPeaks;
		
		if(bGFTT)
			initialPeaks = bhFindLocalMaximumGFTT(preprocessedImg, mask, 2, maxCorners, 0.01, minDistance);
		else
			initialPeaks = bhFindLocalMaximum(maskedImg, mask, 2, maxCorners, qualityLevel_contourSize, minDistance);

		for (auto initialPeak : initialPeaks)
		{
			double distanceFromCenter = cv::norm(initialPeak - cv::Point(centralBeam));

			// Check if the peak is outside the resolution limit
			if (distanceFromCenter <= resolutionRadiusPixels)
				continue;

			// Define a small ROI around the initial peak position in the raw image
			int refinementWindowSize = static_cast<int>(qualityLevel_contourSize * 5);
			int halfWindowSize = refinementWindowSize / 2;

			cv::Rect refinementROI(initialPeak.x - halfWindowSize, initialPeak.y - halfWindowSize, refinementWindowSize, refinementWindowSize);
			refinementROI &= cv::Rect(0, 0, grayImg.cols, grayImg.rows); 

			cv::Mat roi = preprocessedImg(refinementROI);

			// Refine the peak position using either maximum intensity or centroid method
			cv::Point2f refinedPeak;
			if (true) {
				refinedPeak = refinePeakPositionCentroid(roi);  // Centroid method
				refinedPeak += cv::Point2f(refinementROI.x, refinementROI.y); 
			}
			else {
				cv::Point maxLoc = refinePeakPositionMaxIntensity(roi);  // Max intensity method
				refinedPeak = cv::Point2f(maxLoc.x + refinementROI.x, maxLoc.y + refinementROI.y);  
			}

			// Define a circular region around the refined peak for intensity calculation
			int reflectionRadius = static_cast<int>(qualityLevel_contourSize / 2.0);
			cv::Mat peakMask = cv::Mat::zeros(grayImg.size(), CV_8U);
			cv::circle(peakMask, refinedPeak, reflectionRadius, cv::Scalar(255), -1);

			// Sum the intensity within the circular peak area
			cv::Mat peakArea;
			preprocessedImg.copyTo(peakArea, peakMask);
			double totalPeakIntensity = cv::sum(peakArea)[0];

			// Define background points surrounding the peak
			std::vector<cv::Point> bgPoints;
			int backgroundOuterRadius = reflectionRadius + static_cast<int>(minDistance / 2.0);
			cv::circle(peakMask, refinedPeak, backgroundOuterRadius, cv::Scalar(255), -1);
			cv::circle(peakMask, refinedPeak, reflectionRadius, cv::Scalar(0), -1); // Annulus region

			// Collect valid background points
			for (int y = 0; y < grayImg.rows; ++y) {
				for (int x = 0; x < grayImg.cols; ++x) {
					if (peakMask.at<uchar>(y, x) == 255) {
						bgPoints.emplace_back(cv::Point(x, y));
					}
				}
			}

			// Fit a least-squares plane to the background points
			cv::Vec4d planeCoeffs = fitLeastSquaresPlane(bgPoints, preprocessedImg);

			// Create a mask for the peak area
			cv::Mat peakAreaMask = cv::Mat::zeros(grayImg.size(), CV_8U);
			cv::circle(peakAreaMask, refinedPeak, reflectionRadius, cv::Scalar(255), -1);

			// Estimate the background intensity under the peak area
			double backgroundSum = 0.0;
			for (int y = 0; y < grayImg.rows; ++y) {
				for (int x = 0; x < grayImg.cols; ++x) {
					if (peakAreaMask.at<uchar>(y, x) == 255) {
						double estimatedBg = planeCoeffs[0] * x + planeCoeffs[1] * y + planeCoeffs[2];
						backgroundSum += estimatedBg;
					}
				}
			}

			// Calculate net intensity (peak intensity - background)
			double netIntensity = totalPeakIntensity - backgroundSum;

			// Calculate sigma(I) using error propagation
			double sigmaSq_I = 0.0;
			for (int i = 0; i < bgPoints.size(); ++i) {
				double pi = grayImg.at<uchar>(bgPoints[i]);  // Intensity at background points
				sigmaSq_I += (2 * pi + 1);  // Use detector parameters for sigma calculation
			}

			double sigma_I = std::sqrt(sigmaSq_I);

			// Calculate I/sigma
			double I_sigma = sigma_I > 1e-9 ? netIntensity / sigma_I : -1.0f;

			// Decide whether to accept or reject the peak based on I/sigma
			if (I_sigma > i_sigma) {
				detectedPeaks.push_back(refinedPeak);
			}
			else {
				rejectedPeaks.push_back(refinedPeak);
			}
		}

		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception in CImageManager::detect_diffraction_peaks: " << e.what() << std::endl;
		return false;
	}
}
bool CImageManager::detect_diffraction_peaks_cheetahpf8(CCheetah_PeakFinder* _cheetah_pf, const cv::Mat&imgInput, const cv::Point2f&centralBeam, double pixelSize, double d_max, std::vector<cv::Point2f>&detectedPeaks, float i_sigma, double peaksize, double threshold, unsigned int binFactor)
{
	// convert the image to a float image
	cv::Mat imgFloat;
	imgInput.convertTo(imgFloat, CV_32F);

	float resizeFac = 1.0f / binFactor;
	cv::resize(imgFloat, imgFloat, cv::Size(), resizeFac, resizeFac);
	//if (m_pCheetahPeakFinder == nullptr)
	//	m_pCheetahPeakFinder = CCheetah_PeakFinder::GetInstance(imgFloat.rows, imgFloat.cols);

	if (_cheetah_pf == nullptr)
	{
		std::cerr << "Error: Peak Finder is not initialized." << std::endl;
		return false;
	}

	float* data = reinterpret_cast<float*>(imgFloat.data);

	// Mask around the central beam up to the resolution limit
	cv::Mat maskMatInner = cv::Mat::ones(imgFloat.size(), CV_8U);
	cv::Mat maskMatOuter = cv::Mat::zeros(imgFloat.size(), CV_8U);
	cv::circle(maskMatOuter, cv::Point(imgFloat.rows/2, imgFloat.cols/2), (imgFloat.rows - 10) / 2, 255, -1);
	pixelSize *= binFactor;
	cv::circle(maskMatInner, cv::Point2f(centralBeam.x / binFactor, centralBeam.y / binFactor), static_cast<int>(d_max / pixelSize), 0, -1);
	cv::Mat maskMat = maskMatInner & maskMatOuter;
	char* mask = reinterpret_cast<char*>(maskMat.data);

	long pix_nx = imgFloat.cols; // Width
	long pix_ny = imgFloat.rows; // Height
	long pix_nn = pix_nx * pix_ny; 
	//float* pix_r = new float[pix_nn]; 


	// Define the center of the image (central beam's coords)
	float cx = centralBeam.x / binFactor;
	float cy = centralBeam.y / binFactor;

	for (long y = 0; y < pix_ny; y++) {
		for (long x = 0; x < pix_nx; x++) {
			long idx = y * pix_nx + x;
			float dx = x - cx;
			float dy = y - cy;
			_cheetah_pf->m_pix_r[idx] = sqrt(dx * dx + dy * dy);
			//pix_r[idx] = sqrt(dx * dx + dy * dy);
		}
	}
	
	
	long asic_nx = pix_nx; 
	long asic_ny = pix_ny; 
	long nasics_x = 1;    
	long nasics_y = 1;     

	float ADCthresh = threshold;           // ADC threshold to ignore low-intensity pixels
	float hitfinderMinSNR = i_sigma;     // Minimum signal-to-noise ratio for a peak
	long hitfinderMinPixCount = peaksize / binFactor;    // Minimum number of pixels to be considered a peak
	long hitfinderMaxPixCount = 500 ;   // Maximum number of pixels in a peak
	long hitfinderLocalBGRadius = (hitfinderMinPixCount + 2);  // Radius for local background calculation

	

	
	tPeakList* peaklist = _cheetah_pf->GetPeakList();
	_cheetah_pf->reset_variables();
	_cheetah_pf->peakfinder8_moussa(data, mask, _cheetah_pf->m_pix_r, asic_nx, asic_ny, nasics_x, nasics_y, ADCthresh, hitfinderMinSNR, hitfinderMinPixCount, hitfinderMaxPixCount, hitfinderLocalBGRadius);
	//m_pCheetahPeakFinder->peakfinder8_original(data, mask, pix_r, asic_nx, asic_ny, nasics_x, nasics_y, ADCthresh, hitfinderMinSNR, hitfinderMinPixCount, hitfinderMaxPixCount, hitfinderLocalBGRadius);
	//m_pCheetahPeakFinder->killNearbyPeaks(hitfinderMaxPixCount * 3);
	for (int i = 0; i < peaklist->nPeaks; i++)
		detectedPeaks.emplace_back(cv::Point2f(binFactor * peaklist->peak_com_x[i], binFactor * peaklist->peak_com_y[i]));
	
	
	return bool(peaklist->nPeaks);
}
bool CImageManager::detect_diffraction_peaks_imageJ(const cv::Mat& imgInput, const cv::Point2f& centralBeam, double pixelSize, double d_max, std::vector<cv::Point2f>& detectedPeaks, float tol)
{
	cv::Mat imgInputFloat = imgInput.clone();
	imgInputFloat.convertTo(imgInputFloat, CV_32F);

	const int x_offsets[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	const int y_offsets[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };

	const int height = imgInputFloat.rows;
	const int width = imgInputFloat.cols;
	const int consize = width * height;

	cv::Mat peak_img = cv::Mat::zeros(height, width, CV_32SC1);
	std::vector<cv::Point2f> peak_list;

	cv::Mat expixels = cv::Mat::zeros(height, width, CV_32SC1); 
	cv::Mat clist = cv::Mat::zeros(height, width, CV_32SC1);    

	std::vector<int> p_indices;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (imgInputFloat.at<float>(y, x) > tol) {  
				p_indices.push_back(y * width + x);
			}
		}
	}

	int index = 0;  

	for (int p : p_indices) {
		index++;
		const int x = p % width;
		const int y = p / width;
		const float pval = imgInputFloat.at<float>(y, x);

		// Skip already processed candidates
		if (peak_img.at<int>(y, x) != 0) continue;

		std::deque<int> exlist; 
		bool is_peak = true;
		exlist.push_back(p);
		expixels.at<int>(y, x) = index;
		peak_img.at<int>(y, x) = 32;  

		while (!exlist.empty() && is_peak) {
			const int current_p = exlist.front();
			exlist.pop_front();

			const int cx = current_p % width;
			const int cy = current_p / width;

			if (cx <= 0 || cy <= 0 || cx >= width - 1 || cy >= height - 1) continue;

			for (int k = 0; k < 8; k++) {
				const int nx = cx + x_offsets[k];
				const int ny = cy + y_offsets[k];
				const int np = ny * width + nx;

				if (np == p) continue;

				if (nx < 0 || ny < 0 || nx >= width || ny >= height) continue;

				if (clist.at<int>(ny, nx) == index) continue;

				const float neighbor_val = imgInputFloat.at<float>(ny, nx);
				clist.at<int>(ny, nx) = index;

				if (neighbor_val > pval) {
					peak_img.at<int>(y, x) = 11;  // Mark reason
					peak_img.at<int>(ny, nx) = 13;
					is_peak = false;
					exlist.clear();
					break;
				}

				if (neighbor_val >= (pval - tol)) {
					if (expixels.at<int>(ny, nx) != index) {
						expixels.at<int>(ny, nx) = index;
						exlist.push_back(np);
					}
					peak_img.at<int>(ny, nx) = 16;
				}
				else {
					peak_img.at<int>(ny, nx) = 8;
				}
			}
		}

		if (is_peak) {
			peak_list.push_back(cv::Point2f(x, y));  
		}
		else {
			if (peak_img.at<int>(y, x) >= 16) {
				peak_img.at<int>(y, x) = 1;
			}
		}
	}

	// Filter peaks based on distance from central beam (optional)
	detectedPeaks.clear();
	for (const auto& peak : peak_list) {
		double distance = cv::norm(peak - centralBeam) * pixelSize;
		if (distance <= d_max) {
			detectedPeaks.push_back(peak);
		}
	}

	return !detectedPeaks.empty();
	//return { peak_img, detectedPeaks };
}



bool CImageManager::detect_diffraction_peaks_imageJ2(const cv::Mat& imgInput, const cv::Point2f& centralBeam, double pixelSize, double d_max, std::vector<cv::Point2f>& detectedPeaks, float ntol)
{
	

	double min, max;
	cv::minMaxLoc(imgInput, &min, &max);
	cv::Mat img;
	imgInput.convertTo(img, CV_64F);
	
	cv::normalize(img, img, 0, 255, cv::NORM_MINMAX);


	cv::Mat dilated;
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::dilate(img, dilated, kernel);
	cv::Mat localMax;
	cv::compare(img, dilated, localMax, cv::CMP_EQ);

	double minVal;
	cv::minMaxLoc(img, &minVal, nullptr);
	cv::Mat minMask;
	cv::compare(img, minVal, minMask, cv::CMP_EQ);
	localMax.setTo(0, minMask);

	struct Pixel {
		cv::Point pt;
		double intensity;
	};
	std::vector<Pixel> candidates;
	for (int y = 0; y < localMax.rows; ++y)
	{
		for (int x = 0; x < localMax.cols; ++x)
		{
			if (localMax.at<uchar>(y, x))
			{
				Pixel p;
				p.pt = cv::Point(x, y);
				p.intensity = static_cast<double>(imgInput.at<uchar>(y, x));
				candidates.push_back(p);
			}
		}
	}

	std::sort(candidates.begin(), candidates.end(), [](const Pixel& a, const Pixel& b) {
		return a.intensity > b.intensity;
		});

	// Create a mask to mark processed pixels.
	cv::Mat processed = cv::Mat::zeros(imgInput.size(), CV_8U);

	const int directions[8][2] = { {0,-1}, {1,-1}, {1,0}, {1,1},
								   {0,1}, {-1,1}, {-1,0}, {-1,-1} };

	for (const auto& cand : candidates)
	{
		int x0 = cand.pt.x;
		int y0 = cand.pt.y;
		if (processed.at<uchar>(y0, x0))
			continue; 

		double seedIntensity = cand.intensity;
		bool isMaximum = true;
		std::vector<cv::Point> equalPoints;
		std::vector<cv::Point> stack;

		stack.push_back(cand.pt);
		processed.at<uchar>(y0, x0) = 1;
		equalPoints.push_back(cand.pt);
		double sumX = x0, sumY = y0;
		int countEqual = 1;

		while (!stack.empty())
		{
			cv::Point pt = stack.back();
			stack.pop_back();

			for (int d = 0; d < 8; d++)
			{
				int nx = pt.x + directions[d][0];
				int ny = pt.y + directions[d][1];

				if (nx < 0 || ny < 0 || nx >= imgInput.cols || ny >= imgInput.rows)
					continue;
				if (processed.at<uchar>(ny, nx))
					continue;

				double neighborIntensity = static_cast<double>(imgInput.at<uchar>(ny, nx));

				if (neighborIntensity > seedIntensity)
					isMaximum = false;

				if (neighborIntensity >= seedIntensity - ntol)
				{
					stack.push_back(cv::Point(nx, ny));
					processed.at<uchar>(ny, nx) = 1;
					if (neighborIntensity == seedIntensity)
					{
						equalPoints.push_back(cv::Point(nx, ny));
						sumX += nx;
						sumY += ny;
						countEqual++;
					}
				}
			}
		}

		if (isMaximum)
		{
			double cx = sumX / countEqual;
			double cy = sumY / countEqual;
			cv::Point bestPoint = equalPoints[0];
			double bestDist = std::hypot(bestPoint.x - cx, bestPoint.y - cy);
			for (const auto& pt : equalPoints)
			{
				double d = std::hypot(pt.x - cx, pt.y - cy);
				if (d < bestDist)
				{
					bestDist = d;
					bestPoint = pt;
				}
			}
			detectedPeaks.push_back(cv::Point2f(bestPoint.x, bestPoint.y));
		}
	}
	return !detectedPeaks.empty();

}



std::vector<cv::Point> CImageManager::bhFindLocalMaximum(const cv::InputArray& _src, const cv::InputArray& _mask, int neighbor, int maxPeaks, float minContourArea, float minDistance)
{
	cv::Mat src = _src.getMat();
	cv::Mat peak_img = src.clone();

	cv::dilate(peak_img, peak_img, cv::Mat(), cv::Point(-1, -1), neighbor);
	peak_img = peak_img - src;

	cv::Mat flat_img;
	cv::erode(src, flat_img, cv::Mat(), cv::Point(-1, -1), neighbor);
	flat_img = src - flat_img;

	cv::threshold(peak_img, peak_img, 0, 255, cv::THRESH_BINARY);
	cv::threshold(flat_img, flat_img, 0, 255, cv::THRESH_BINARY);
	cv::bitwise_not(flat_img, flat_img);

	peak_img.setTo(cv::Scalar::all(255), flat_img);
	cv::bitwise_not(peak_img, peak_img);

	cv::Mat labels, stats, centroids;
	int numLabels = cv::connectedComponentsWithStats(peak_img, labels, stats, centroids);

	std::vector<cv::Point> filteredCenters;
	for (int i = 1; i < numLabels; i++) {  // Start from 1 to skip the background
		int area = stats.at<int>(i, cv::CC_STAT_AREA);
		if (area >= minContourArea) {
			double centerX = centroids.at<double>(i, 0);
			double centerY = centroids.at<double>(i, 1);
			filteredCenters.push_back(cv::Point(static_cast<int>(centerX), static_cast<int>(centerY)));
		}
	}

	if (filteredCenters.size() > static_cast<size_t>(maxPeaks)) {
		std::sort(filteredCenters.begin(), filteredCenters.end(), [&](const cv::Point& a, const cv::Point& b) {
			return src.at<uchar>(a) > src.at<uchar>(b);  // Sort by intensity at the peak
			});
	}

	std::vector<cv::Point> finalPeaks;
	for (const auto& peak : filteredCenters) {
		bool tooClose = false;

		for (const auto& selectedPeak : finalPeaks) {
			if (cv::norm(peak - selectedPeak) < minDistance) {
				tooClose = true;
				break;
			}
		}

		// Only add the peak if it's not too close to any already selected peaks
		if (!tooClose) {
			finalPeaks.push_back(peak);
		}

		// Stop if we reach the max number of peaks
		if (finalPeaks.size() >= static_cast<size_t>(maxPeaks)) {
			break;
		}
	}

	return finalPeaks;
}



std::vector<cv::Point> CImageManager::bhFindLocalMaximumGFTT(const cv::InputArray& _src, const cv::InputArray& _mask, int neighbor, int maxCorners, float qualityLevel, float minDistance)
{
	
	std::vector<cv::Point2f> corners;
	cv::goodFeaturesToTrack(_src, corners, maxCorners, qualityLevel, minDistance, _mask);

	std::vector<cv::Point> finalPeaks;

	for (const auto& peak : corners)
		finalPeaks.push_back(peak);

	return finalPeaks;

}


std::vector<cv::Point> CImageManager::bhContoursCenter(const std::vector<std::vector<cv::Point>>& contours, bool centerOfMass, int contourIdx)
{
	std::vector<cv::Point> result;
	if (contourIdx > -1) {
		if (centerOfMass) {
			cv::Moments m = cv::moments(contours[contourIdx], true);
			result.push_back(cv::Point(static_cast<int>(m.m10 / m.m00), static_cast<int>(m.m01 / m.m00)));
		}
		else {
			cv::Rect rct = cv::boundingRect(contours[contourIdx]);
			result.push_back(cv::Point(rct.x + rct.width / 2, rct.y + rct.height / 2));
		}
	}
	else {
		for (size_t i = 0; i < contours.size(); ++i) {
			if (centerOfMass) {
				cv::Moments m = cv::moments(contours[i], true);
				result.push_back(cv::Point(static_cast<int>(m.m10 / m.m00), static_cast<int>(m.m01 / m.m00)));
			}
			else {
				cv::Rect rct = cv::boundingRect(contours[i]);
				result.push_back(cv::Point(rct.x + rct.width / 2, rct.y + rct.height / 2));
			}
		}
	}

	return result;
}



void CImageManager::draw_resolution_rings(cv::Mat& imgOutput, const cv::Point2f& beamLocation, double pixelSize)
{
	int id = 0;
	std::vector<double> resolutions = { 0.5, 1.0, 1.5, 2.0, 2.5, 3.0 }; // in Å
	for (const auto& res : resolutions) {
		id++;
		// Convert resolution from angstroms to pixels
		double radiusPixels = res / pixelSize;

		int thickness = 2 - (id % 2);
		if (radiusPixels > 0 && radiusPixels < std::sqrt(std::pow(imgOutput.cols, 2) + std::pow(imgOutput.rows, 2))) {
			// Draw the resolution ring
			cv::circle(imgOutput, beamLocation, static_cast<int>(radiusPixels), cv::Scalar(35000, 35000, 35000), thickness, cv::LINE_AA);
		}
	}

	// Draw a circle in the middle of the image
	cv::circle(imgOutput, cv::Point(imgOutput.cols / 2, imgOutput.rows / 2), CDataCollection::GetInstance()->m_fCentralBeamTolerance, cv::Scalar(35000, 0, 0), 1, cv::LINE_AA);
	cv::circle(imgOutput, cv::Point(imgOutput.cols / 2, imgOutput.rows / 2), 10, cv::Scalar(35000, 0, 0), 1, cv::LINE_AA);
}

void CImageManager::adjustImageJStyle(cv::Mat& image, double min_val, double max_val) {
	cv::Mat image_float;
	image.convertTo(image_float, CV_32F);

	image_float = (image_float - min_val) * (255.0 / (max_val - min_val));

	cv::threshold(image_float, image_float, 255, 255, cv::THRESH_TRUNC);
	cv::threshold(image_float, image_float, 0, 0, cv::THRESH_TOZERO); 

	cv::Mat adjusted;
	image_float.convertTo(image, CV_8U); 
}


void CImageManager::adjustBrightnessContrast(const cv::Mat& image, int min_val, int max_val, int target_min = 0, int target_max = 4095) {
	cv::Mat image_float, adjusted;

	image.convertTo(image_float, CV_32F);

	image_float = image_float - min_val;

	float scale = static_cast<float>(target_max - target_min) / (max_val - min_val);
	image_float = image_float * scale;

	cv::min(image_float, target_max, image_float);
	cv::max(image_float, target_min, image_float);

	image_float.convertTo(image, CV_16U);
}

void CImageManager::computeHistogramBasedMinMax(const cv::Mat& image, double& min_val, double& max_val, double lower_percentile = 2.0, double upper_percentile = 98.0) {
	int histSize = int(std::pow(2, 16)); 
	float range[] = { 0, histSize };
	const float* histRange = { range };

	cv::Mat hist;
	cv::calcHist(&image, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

	int total_pixels = image.total();
	int lower_thresh = static_cast<int>(total_pixels * (lower_percentile / 100.0));
	int upper_thresh = static_cast<int>(total_pixels * (upper_percentile / 100.0));

	int cumulative = 0;
	min_val = 0;
	max_val = histSize;

	// Find min_val (2% percentile)
	for (int i = 0; i < histSize; i++) {
		cumulative += hist.at<float>(i);
		if (cumulative >= lower_thresh) {
			min_val = i;
			break;
		}
	}

	cumulative = 0;
	// Find max_val (98% percentile)
	for (int i = 0; i < histSize; i++) {
		cumulative += hist.at<float>(i);
		if (cumulative >= upper_thresh) {
			max_val = i;
			break;
		}
	}
}

struct MouseData {
	cv::Mat& image;
	cv::Mat& orgImg;
	std::vector<cv::Point>& points;
	std::vector<cv::Point2f>& detectedPeaks;
	std::vector<cv::RotatedRect>& ellipses;
	std::vector < cv::Point3f>& circle;
	std::string windowName;
};

// Function to find the closest detected peak within 5 pixels
cv::Point CImageManager::findClosestPeak(int x, int y, const std::vector<cv::Point2f>& peaks, int maxDistance = 10) {
	cv::Point closestPoint(x, y);
	double minDist = maxDistance + 1; 

	for (const auto& peak : peaks) {
		double dist = cv::norm(cv::Point2f(x, y) - peak);
		if (dist <= maxDistance && dist < minDist) {
			closestPoint = peak;
			minDist = dist;
		}
	}
	return closestPoint;
}

// For debugging, trying to measure elliptical distortion values
void  CImageManager::onMousePowderRings(int event, int x, int y, int flags, void* userdata) {

	auto* data = static_cast<MouseData*>(userdata); 
	auto pImgMgr = CImageManager::GetInstance();

	if (event == cv::EVENT_LBUTTONDOWN) {
		cv::Point2f selectedPoint = pImgMgr->findClosestPeak(x, y, data->detectedPeaks); 
		data->points.emplace_back(selectedPoint); 
				
	}
	else if (event == cv::EVENT_RBUTTONDBLCLK)
	{
		if (data->points.size() >= 5) { 
			data->ellipses.emplace_back(cv::fitEllipse(data->points));
			pImgMgr->m_ellipses.push_back(cv::fitEllipse(data->points));

			printf("Ellipse Size: %d\n", pImgMgr->m_ellipses.size());
			cv::Point2f center;
			float radius;
			cv::minEnclosingCircle(data->points, center, radius);
			data->circle.emplace_back(cv::Point3f(center.x, center.y, radius));

			// Compute EPS (Ellipticity)
			double major_axis = std::max(data->ellipses.back().size.width, data->ellipses.back().size.height);
			double minor_axis = std::min(data->ellipses.back().size.width, data->ellipses.back().size.height);
			double EPS = minor_axis / major_axis;

			// Get PHI (orientation angle)
			double PHI = data->ellipses.back().angle; 

			

			// Print EPS and PHI
			std::cout << "Ellipse: Center = " << data->ellipses.back().center << ", EPS =" << EPS << ", PHI = " << PHI << " degrees" << std::endl;
			data->points.clear();
		}
	}
	else if (event == cv::EVENT_MBUTTONDOWN) {


		std::cout << "Performing distortion correction..." << std::endl;
		double EPS = 0.0;        // elliptical distortion parameter
		double PHI = 0.0;    // distortion angle (degrees)
		double X_center = 0.0;  // distortion center X (pixels)
		double Y_center = 0.0;  // distortion center Y (pixels)
		for (auto& ellipse : data->ellipses)
		{
			double major_axis = std::max(ellipse.size.width, ellipse.size.height);
			double minor_axis = std::min(ellipse.size.width, ellipse.size.height);
			EPS += (minor_axis / major_axis);

			PHI += ellipse.angle;

			X_center += ellipse.center.x;
			Y_center += ellipse.center.y;
		}

		EPS /= data->ellipses.size();
		PHI /= data->ellipses.size();
		X_center /= data->ellipses.size();
		Y_center /= data->ellipses.size();
		
		X_center /= IMG_RESIZE_FACTOR_;
		Y_center /= IMG_RESIZE_FACTOR_;

		std::cout << "EPS = " << EPS << ", PHI = " << PHI << " degrees" << std::endl;
		std::cout << "X_center = " << X_center << ", Y_center = " << Y_center << std::endl;

		// Convert PHI to radians
		double phi_rad = PHI * CV_PI / 180.0;
		double cos_phi = std::cos(phi_rad);
		double sin_phi = std::sin(phi_rad);

		// Compute the distortion matrix U
		cv::Mat U = (cv::Mat_<double>(2, 2) <<
			std::pow(cos_phi, 2) + EPS * std::pow(sin_phi, 2),
			sin_phi * cos_phi - EPS * sin_phi * cos_phi,
			cos_phi * sin_phi - EPS * sin_phi * cos_phi,
			std::pow(sin_phi, 2) + EPS * std::pow(cos_phi, 2));

		cv::Mat M = (cv::Mat_<double>(2, 3) <<
			U.at<double>(0, 0), U.at<double>(0, 1),
			-U.at<double>(0, 0) * X_center - U.at<double>(0, 1) * Y_center + X_center,
			U.at<double>(1, 0), U.at<double>(1, 1),
			-U.at<double>(1, 0) * X_center - U.at<double>(1, 1) * Y_center + Y_center);

		// Load the distorted image
		cv::Mat distorted_image = data->orgImg.clone();

		// Apply the inverse transformation
		cv::Mat corrected_image;
		cv::warpAffine(
			distorted_image, corrected_image, M, distorted_image.size(),
			cv::INTER_NEAREST,     
			cv::BORDER_CONSTANT,   
			cv::Scalar(0)
		);

		std::string fileName = "F:/3D_Electron_Diffraction/3D_ED_Samples/LuAg/005__DIST_115/test/Corrected_Image-a.tiff";
		TinyTIFFWriterFile* tiffwriter = TinyTIFFWriter_open(fileName.c_str(), 16, TinyTIFFWriter_Int, 1, 516, 516, TinyTIFFWriter_Greyscale);
		if (tiffwriter)
		{
			TinyTIFFWriter_writeImage(tiffwriter, corrected_image.data);
			TinyTIFFWriter_close(tiffwriter);
		}
	}
}
void CImageManager::apply_distortion_corrections(const cv::Mat& dist_img, std::string&fileName, float EPS, float PHI, float X_center, float Y_center)
{	
	// Adapted from --> https://xds.mr.mpg.de/html_doc/geocorr.html

	double phi_rad = PHI * CV_PI / 180.0;
	double cos_phi = std::cos(phi_rad);
	double sin_phi = std::sin(phi_rad);

	cv::Mat U = (cv::Mat_<double>(2, 2) <<
		std::pow(cos_phi, 2) + EPS * std::pow(sin_phi, 2),
		sin_phi * cos_phi - EPS * sin_phi * cos_phi,
		cos_phi * sin_phi - EPS * sin_phi * cos_phi,
		std::pow(sin_phi, 2) + EPS * std::pow(cos_phi, 2));

	cv::Mat M = (cv::Mat_<double>(2, 3) <<
		U.at<double>(0, 0), U.at<double>(0, 1),
		-U.at<double>(0, 0) * X_center - U.at<double>(0, 1) * Y_center + X_center,
		U.at<double>(1, 0), U.at<double>(1, 1),
		-U.at<double>(1, 0) * X_center - U.at<double>(1, 1) * Y_center + Y_center);

	cv::Mat distorted_image = dist_img.clone();

	cv::Mat corrected_image;
	cv::warpAffine(
		dist_img, corrected_image, M, dist_img.size(),
		cv::INTER_NEAREST,      
		cv::BORDER_CONSTANT,   
		cv::Scalar(0)
	);


	TinyTIFFWriterFile* tiffwriter = TinyTIFFWriter_open(fileName.c_str(), 16, TinyTIFFWriter_Int, 1, 516, 516, TinyTIFFWriter_Greyscale);
	if (tiffwriter)
	{
		TinyTIFFWriter_writeImage(tiffwriter, corrected_image.data);
		TinyTIFFWriter_close(tiffwriter);
	}
	
}

std::string BrowseFolder(const std::wstring& title = L"Select a folder")
{
	std::string result;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "Failed to initialize COM.", "Error", MB_OK | MB_ICONERROR);
		return result;
	}

	IFileDialog* pFileDialog = nullptr;
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));

	if (SUCCEEDED(hr))
	{
		DWORD dwOptions;
		pFileDialog->GetOptions(&dwOptions);
		pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
		pFileDialog->SetTitle(title.c_str());

		hr = pFileDialog->Show(NULL);
		if (SUCCEEDED(hr))
		{
			IShellItem* pItem = nullptr;
			hr = pFileDialog->GetResult(&pItem);
			if (SUCCEEDED(hr))
			{
				PWSTR pszFilePath = nullptr;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
				if (SUCCEEDED(hr))
				{
					char path[MAX_PATH];
					WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, path, MAX_PATH, NULL, NULL);
					result = std::string(path) + "/";
					CoTaskMemFree(pszFilePath);
				}
				pItem->Release();
			}
		}
		pFileDialog->Release();
	}

	CoUninitialize();
	return result;
}

// ALso for debugging, this function is terribly written.
void CImageManager::calibrate_distortions()
{
	CDataCollection* pDC = CDataCollection::GetInstance();
	CTimepix* pTpx = CTimepix::GetInstance();

	std::string sPath = BrowseFolder(L"Select Folder containing Powder ring patterns.");// "F:/3D_Electron_Diffraction/3D_ED_Samples/LuAg/005__DIST_115";
	std::string sDataPath = BrowseFolder(L"Select Folder containing data to correct");

	if (sPath == "" || sDataPath == "")
	{
		printf("Please select the directories.");
		return;
	}

	std::string sOutputPath = sDataPath + "Corrected_Images/";
	

	std::vector<cv::String> filenames;
	cv::glob(sPath, filenames);

	CCheetah_PeakFinder* cheetah_pf_tpx = nullptr;
	cv::Point2f centralBeamPos = cv::Point2f(0, 0);
	SCameraLengthSettings m_oCameraSettings(pDC->m_fCameraLength);
	std::vector<cv::Point> points;
	std::vector<cv::RotatedRect> ellipses;
	std::vector<cv::Point3f> circles;
	std::string windowName = "Powder Ring Images";
	cv::namedWindow(windowName);

	int fileIdx = 0;
	cv::Mat ringImg = cv::imread(filenames[fileIdx], cv::IMREAD_UNCHANGED);
	cv::Mat ringImgCopy = ringImg.clone();
	cv::Mat ringImgRAW = ringImg.clone();
	cv::resize(ringImgCopy, ringImgCopy, cv::Size(), IMG_RESIZE_FACTOR_, IMG_RESIZE_FACTOR_);
	while(fileIdx < filenames.size())
	{

		if (cheetah_pf_tpx == nullptr)
			cheetah_pf_tpx = new CCheetah_PeakFinder(ringImgCopy.rows, ringImgCopy.cols);

		double min_val, max_val;
		cv::minMaxLoc(ringImgCopy, &min_val, &max_val);
		ringImgCopy.convertTo(ringImg, CV_16UC1, std::pow(2, 16) / max_val);

		this->computeHistogramBasedMinMax(ringImg, min_val, max_val);
		this->adjustImageJStyle(ringImg, pTpx->m_fBrightnessDiff, min_val + 10 + max_val / (0.01f * pTpx->m_fContrastDiff + 0.001f));

		bool bCentralBeamValid = this->find_central_beam_position(ringImgCopy, centralBeamPos);
		if (bCentralBeamValid == false)
			continue;
		
		m_oCameraSettings.UpdateCameraLengthSettings(pDC->m_fCameraLength);
		std::vector <cv::Point2f> detectedPeaks;
		this->detect_diffraction_peaks_cheetahpf8(cheetah_pf_tpx, ringImgCopy, centralBeamPos, m_oCameraSettings.flPixelSize / IMG_RESIZE_FACTOR_, pDC->m_fSerialED_dmax, detectedPeaks, pDC->m_fSerialED_i_sigma, pDC->m_fSerialED_peaksize, pDC->m_fSerialED_peakthreshold, 1);

		cv::cvtColor(ringImg, ringImg, cv::COLOR_GRAY2BGR);
		for (const auto& peak : detectedPeaks) {
			cv::circle(ringImg, peak, pDC->m_fSerialED_peaksize, cv::Scalar(0, 255, 0), 1);
		}

		for (const auto& point : points)
		{
			cv::circle(ringImg, point, pDC->m_fSerialED_peaksize, cv::Scalar(0, 0, 255), -1);
		}

		for (const auto& Ellipse : ellipses) {
			cv::ellipse(ringImg, Ellipse, cv::Scalar(0, 0, 255), 1);
			cv::circle(ringImg, Ellipse.center, 15, cv::Scalar(255, 0, 255), 1);
		}

		for (const auto& circle : circles)
		{
			cv::circle(ringImg, cv::Point(circle.x, circle.y), circle.z, cv::Scalar(255, 0, 0), 2);
		}

		cv::imshow(windowName, ringImg);
		MouseData data = { ringImgCopy, ringImgRAW, points, detectedPeaks, ellipses, circles, windowName };
		cv::setMouseCallback(windowName, onMousePowderRings, &data);
		auto q = cv::waitKey(100);
		if (q == 'n' || q == 'N')
		{
			fileIdx++;
			points.clear();
			ellipses.clear();
			circles.clear();

			if (fileIdx < filenames.size())
			{
				printf("Loading image: %s\n", filenames[fileIdx].c_str());
				ringImg = cv::imread(filenames[fileIdx], cv::IMREAD_UNCHANGED);
				ringImgCopy = ringImg.clone();
				ringImgRAW = ringImg.clone();
				cv::resize(ringImgCopy, ringImgCopy, cv::Size(), IMG_RESIZE_FACTOR_, IMG_RESIZE_FACTOR_);
			}
		}
		else if (q == 'p' || q == 'P')
		{
			fileIdx--;
			points.clear();
			ellipses.clear();
			circles.clear();
			if (fileIdx >= 0)
			{
				printf("Loading image: %s\n", filenames[fileIdx].c_str());
				ringImg = cv::imread(filenames[fileIdx], cv::IMREAD_UNCHANGED);
				ringImgCopy = ringImg.clone();
				ringImgRAW = ringImg.clone();
				cv::resize(ringImgCopy, ringImgCopy, cv::Size(), IMG_RESIZE_FACTOR_, IMG_RESIZE_FACTOR_);
			}
		}
		else if (q == 'r' || q == 'R')
		{
			printf("Reloading image: %s\n", filenames[fileIdx].c_str());
			ringImg = cv::imread(filenames[fileIdx], cv::IMREAD_UNCHANGED);
			ringImgCopy = ringImg.clone();
			ringImgRAW = ringImg.clone();
			cv::resize(ringImgCopy, ringImgCopy, cv::Size(), IMG_RESIZE_FACTOR_, IMG_RESIZE_FACTOR_);
		}
		else if (q == 'q' || q == 'Q')
		{
			break;
		}

	}
	cv::destroyWindow(windowName);



	CWriteFile::create_directory(sOutputPath);
	// Get Average values for EPS, PHI, X_center, Y_center
	int numEllipses = m_ellipses.size();
	float EPS = 0.0f;
	float PHI = 0.0f;;
	float X_center = 0.0f;
	float Y_center = 0.0f;

	if (numEllipses > 0)
	{
		for (auto& ellipse : m_ellipses)
		{
			double major_axis = std::max(ellipse.size.width, ellipse.size.height);
			double minor_axis = std::min(ellipse.size.width, ellipse.size.height);
			EPS += (minor_axis / major_axis);

			PHI += ellipse.angle;

			X_center += ellipse.center.x;
			Y_center += ellipse.center.y;
		}

		EPS /= numEllipses;
		PHI /= numEllipses;
		X_center /= numEllipses;
		Y_center /= numEllipses;

		//X_center /= IMG_RESIZE_FACTOR_;
		//Y_center /= IMG_RESIZE_FACTOR_;


		X_center = Y_center = ringImgRAW.rows * 0.5f;

		printf("EPS = %f\nPHI = %f\nX_center = %f\nY_center = %f\n", EPS, PHI, X_center, Y_center);

		int imgIndex = 0;
		for (const auto& entry : std::filesystem::directory_iterator(sDataPath))
		{
			if (entry.path().extension() == ".tiff")
			{
				imgIndex++;

				// Load the image
				cv::Mat img = cv::imread(entry.path().string(), cv::IMREAD_UNCHANGED);

				std::string fullImgName = sOutputPath + std::format("{:05d}.tiff", imgIndex);
				this->apply_distortion_corrections(img, fullImgName, EPS, PHI, X_center, Y_center);

			}
		}
	}
	else
	{
		std::cout << "No Ellipses found for distortion correction" << std::endl;
		return;
	}
	

	m_ellipses.clear();
	delete cheetah_pf_tpx;
}

void CImageManager::find_beam_position_hough_circles(cv::Mat& _Img, ImagesInfo& imgInfo)
{
	std::vector<cv::Vec3f> circles;  // Detected circles
	while (true)
	{
		cv::HoughCircles(_Img, circles, cv::HOUGH_GRADIENT, 1, 50, imgInfo._circle_detection_param, imgInfo._circle_detection_param, 0, 0);
		if (circles.size() == 0)
		{
			printf("Couldn't find automatically the circle's position. Try manually...\n");
			imgInfo._circle_detection_param = 1.0f;
			break;;
		}

		if (circles.size() == 1)
		{
			imgInfo._center.x = circles[0][0];
			imgInfo._center.y = circles[0][1];
			if (imgInfo._radius < 0.015f)
				imgInfo._radius = circles[0][2];
			imgInfo._circle_detection_param = 1.0f;
			break;
		}
		imgInfo._circle_detection_param += 0.5;
	}
}


void CImageManager::find_beam_position_center_gravity(cv::Mat& _img, ImagesInfo& imgInfo)
{
	//PRINT("TESTING - Finding beam position using center of gravity method...\nReport if problems occur.");
	// convert the image to CV_64F
	cv::Mat _img_64F;
	_img.clone().convertTo(_img_64F, CV_64F);

	// Create a 2D meshgrid
	cv::Mat x = cv::Mat::zeros(_img_64F.rows, _img_64F.cols, CV_64F);
	cv::Mat y = cv::Mat::zeros(_img_64F.rows, _img_64F.cols, CV_64F);
	for (int i = 0; i < _img_64F.rows; ++i) {
		for (int j = 0; j < _img_64F.cols; ++j) {
			x.at<double>(i, j) = j;
			y.at<double>(i, j) = i;
		}
	}


	// Calculate the weighted sum of coordinates
	double weighted_sum_x = cv::sum(x.mul(_img_64F))[0];
	double weighted_sum_y = cv::sum(y.mul(_img_64F))[0];

	// Calculate the sum of intensities
	double total_intensity = cv::sum(_img_64F)[0];

	// Calculate the center of gravity
	double center_of_gravity_x = weighted_sum_x / total_intensity;
	double center_of_gravity_y = weighted_sum_y / total_intensity;

	// Print the beam coordinates
	//std::cout << "Beam coordinates at: " << std::round(center_of_gravity_x) << ", " << std::round(center_of_gravity_y) << std::endl;
	imgInfo._center.x = std::round(center_of_gravity_x);
	imgInfo._center.y = std::round(center_of_gravity_y);

}

CImageManager* CImageManager::GetInstance()
{
    return m_pImgMgr ? m_pImgMgr : new CImageManager();
}

CImageManager::~CImageManager()
{
	PRINTD("\t\tCImageManager::~CImageManager() - Destructor\n");
	SAFE_RELEASE(m_pImgMgr);
	SAFE_RELEASE(m_pCheetahPeakFinder);
	
}



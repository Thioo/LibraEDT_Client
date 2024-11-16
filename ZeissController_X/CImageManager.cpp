#include "pch.h"
#define IMG_RESIZE_FACTOR 1
CImageManager* CImageManager::m_pImgMgr = nullptr;

CImageManager::CImageManager()
{
    static bool bDoOnce = false;
    if (bDoOnce == false)
    {
        bDoOnce = true;
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
			cv::resize(myImg, myImg, cv::Size(), IMG_RESIZE_FACTOR__, IMG_RESIZE_FACTOR__);
			
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

		float searchRadius = 50.0f; // Adjust based on expected beam size
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

		float maxAllowedDistance = 75.0f; // Maximum distance from image center
		if(CDataCollection::GetInstance()->m_bSerialEDScanRegions)
			maxAllowedDistance = 20.0f;

		float actualDistance = cv::norm(beamLocation - imageCenter);
		if (actualDistance > maxAllowedDistance) {
			//std::cerr << "Error: Beam location is too far from image center (" << actualDistance << " pixels)." << std::endl;
			return false; // alternatively we can use the center of the image instead...
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

		// Ensure the blurred image is in CV_8U format
		if (blurred.type() != CV_8U) {
			blurred.convertTo(blurred, CV_8U);
		}

		// Threshold to find the central beam using Otsu's method (full strength)
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

		// Step 1: Convert to 8-bit unsigned integer if not already
		cv::Mat preprocessedImg;
		if (grayImg.type() != CV_8U) {
			grayImg.convertTo(preprocessedImg, CV_8U);
		}
		else {
			preprocessedImg = grayImg.clone();
		}

		// Step 2: Normalize the image to range [0, 255]
		cv::normalize(preprocessedImg, preprocessedImg, 0, 255, cv::NORM_MINMAX);
		preprocessedImg.convertTo(preprocessedImg, CV_8U);

		// Step 3: Apply Binning (Downsampling and Upsampling)
		if (binFactor > 1) {
			cv::Mat binnedImg;
			// Calculate new size for downsampling
			cv::Size newSize(preprocessedImg.cols / binFactor, preprocessedImg.rows / binFactor);

			// Ensure new size is at least 1x1
			newSize.width = std::max(newSize.width, 1);
			newSize.height = std::max(newSize.height, 1);

			// Downsample using INTER_AREA for binning (averaging)
			cv::resize(preprocessedImg, binnedImg, newSize, 0, 0, cv::INTER_AREA);

			// Upsample back to original size using INTER_NEAREST to preserve averaged values
			cv::resize(binnedImg, preprocessedImg, preprocessedImg.size(), 0, 0, cv::INTER_NEAREST);
		}

		// Step 4: Apply Median Blur
		// Determine the kernel size based on reflection size
		// Ensure kernel size is odd and at least 3
		int kernelSize = static_cast<int>(std::round(reflectionSize));
		if (kernelSize % 2 == 0) {
			kernelSize += 1;
		}
		kernelSize = std::max(kernelSize, 3); // Minimum kernel size of 3

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
	cv::Mat A(n, 3, CV_64F);  // Coefficient matrix
	cv::Mat B(n, 1, CV_64F);  // Intensities

	for (int i = 0; i < n; ++i) {
		A.at<double>(i, 0) = bgPoints[i].x;  // x-coordinates
		A.at<double>(i, 1) = bgPoints[i].y;  // y-coordinates
		A.at<double>(i, 2) = 1.0;            // Constant term
		B.at<double>(i, 0) = img.at<uchar>(bgPoints[i]);  // Intensity values
	}

	cv::Mat coeffs;
	cv::solve(A, B, coeffs, cv::DECOMP_SVD);  // Least squares fitting

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
		// Return centroid position (x = m10/m00, y = m01/m00)
		return cv::Point2f(static_cast<float>(m.m10 / m.m00), static_cast<float>(m.m01 / m.m00));
	}
	else {
		// In case of zero division, fall back to center of ROI
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

		/*for (auto initialPeak : initialPeaks)
		{
			double distanceFromCenter = cv::norm(initialPeak - cv::Point(centralBeam));

			// Check if the peak is outside the resolution limit
			if (distanceFromCenter <= resolutionRadiusPixels)
				continue;


			// Now refine the peak position using the raw image
			// Define a small ROI around the initial peak position in the raw image
			int refinementWindowSize = static_cast<int>(qualityLevel_contourSize * 5);
			int halfWindowSize = refinementWindowSize / 2;

			cv::Rect refinementROI(initialPeak.x - halfWindowSize, initialPeak.y - halfWindowSize, refinementWindowSize, refinementWindowSize);
			refinementROI &= cv::Rect(0, 0, grayImg.cols, grayImg.rows); // Ensure ROI is within image bounds

			cv::Mat roi = grayImg(refinementROI);

			// Find the local maximum within the ROI in the raw image
			double minVal, maxVal;
			cv::Point minLoc, maxLoc;
			cv::minMaxLoc(roi, &minVal, &maxVal, &minLoc, &maxLoc);

			// Adjust maxLoc to image coordinates
			cv::Point refinedPeak = maxLoc + cv::Point(refinementROI.x, refinementROI.y);

			// Optionally, calculate the centroid around the refined peak
			cv::Point2f centroid = refinePeakPositionCentroid(roi);  // Centroid method
			centroid += cv::Point2f(refinementROI.x, refinementROI.y);  // Adjust to full image coordinates

			// Now perform I/sigma calculation as before, using the centroid position
			// Define ROI size based on contour_size
			int roiRadius = static_cast<int>(qualityLevel_contourSize / 2.0);
			roiRadius = std::max(roiRadius, 1);  // Ensure at least a 1-pixel radius

			// Define the ROI for the peak
			cv::Rect roiRect(static_cast<int>(centroid.x - roiRadius), static_cast<int>(centroid.y - roiRadius),
				roiRadius * 2 + 1, roiRadius * 2 + 1);
			roiRect &= cv::Rect(0, 0, grayImg.cols, grayImg.rows);  // Ensure ROI is within image bounds

			// Extract the ROI from the raw image
			cv::Mat peakRoi = grayImg(roiRect);

			// Sum the intensity within the ROI for peak intensity
			double peakIntensity = cv::mean(peakRoi)[0];

			// Define a background annulus around the ROI
			int backgroundInnerRadius = roiRadius + 1;
			int backgroundOuterRadius = backgroundInnerRadius + static_cast<int>(minDistance / 2.0);

			// Create a mask for the background annulus
			cv::Mat backgroundMask = cv::Mat::zeros(grayImg.size(), CV_8U);
			cv::circle(backgroundMask, refinedPeak, backgroundOuterRadius, cv::Scalar(255), -1);
			cv::circle(backgroundMask, refinedPeak, backgroundInnerRadius, cv::Scalar(0), -1);

			// Exclude areas outside the image
			backgroundMask &= mask;

			// Avoid overlapping with other peaks by excluding regions around them
			for (const auto& otherMax : initialPeaks)
			{
				if (otherMax == initialPeak)
					continue;
				cv::circle(backgroundMask, otherMax, roiRadius, cv::Scalar(0), -1);
			}

			// Calculate the background statistics
			cv::Scalar backgroundMean, backgroundStdDev;
			cv::meanStdDev(grayImg, backgroundMean, backgroundStdDev, backgroundMask);

			double sigma = backgroundStdDev[0];

			// Check if sigma is valid
			if (sigma <= 0)
			{
				detectedPeaks.push_back(centroid);
				continue;
			}

			// Calculate I/sigma
			double I_sigma = peakIntensity / sigma;

			// Decide whether to accept or reject the peak
			if (I_sigma > i_sigma)
				detectedPeaks.push_back(centroid);
			else
				rejectedPeaks.push_back(centroid);
		}

		return true;*/

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
			refinementROI &= cv::Rect(0, 0, grayImg.cols, grayImg.rows); // Ensure ROI is within image bounds

			cv::Mat roi = preprocessedImg(refinementROI);

			// Refine the peak position using either maximum intensity or centroid method
			cv::Point2f refinedPeak;
			if (true) {
				refinedPeak = refinePeakPositionCentroid(roi);  // Centroid method
				refinedPeak += cv::Point2f(refinementROI.x, refinementROI.y);  // Adjust to full image coordinates
			}
			else {
				cv::Point maxLoc = refinePeakPositionMaxIntensity(roi);  // Max intensity method
				refinedPeak = cv::Point2f(maxLoc.x + refinementROI.x, maxLoc.y + refinementROI.y);  // Adjust to full image coordinates
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
bool CImageManager::detect_diffraction_peaks_cheetahpf8(const cv::Mat&imgInput, const cv::Point2f&centralBeam, double pixelSize, double d_max, std::vector<cv::Point2f>&detectedPeaks, float i_sigma, double peaksize, double threshold, unsigned int binFactor)
{
	// convert the image to a float image
	cv::Mat imgFloat;
	imgInput.convertTo(imgFloat, CV_32F);

	float resizeFac = 1.0f / binFactor;
	cv::resize(imgFloat, imgFloat, cv::Size(), resizeFac, resizeFac);
	if (m_pCheetahPeakFinder == nullptr)
		m_pCheetahPeakFinder = CCheetah_PeakFinder::GetInstance(imgFloat.rows, imgFloat.cols);
	float* data = reinterpret_cast<float*>(imgFloat.data);

	// Mask around the central beam up to the resolution limit
	cv::Mat maskMatInner = cv::Mat::ones(imgFloat.size(), CV_8U);
	cv::Mat maskMatOuter = cv::Mat::zeros(imgFloat.size(), CV_8U);
	cv::circle(maskMatOuter, cv::Point(imgFloat.rows/2, imgFloat.cols/2), (imgFloat.rows - 10) / 2, 255, -1);
	pixelSize *= binFactor;
	cv::circle(maskMatInner, cv::Point2f(centralBeam.x / binFactor, centralBeam.y / binFactor), static_cast<int>(d_max / pixelSize), 0, -1);
	cv::Mat maskMat = maskMatInner & maskMatOuter;
	char* mask = reinterpret_cast<char*>(maskMat.data);

	// pix_r
	long pix_nx = imgFloat.cols; // Width
	long pix_ny = imgFloat.rows; // Height
	long pix_nn = pix_nx * pix_ny;  // Total number of pixels


	// Define the center of the image (central beam's coords)400
	float cx = centralBeam.x / binFactor;
	float cy = centralBeam.y / binFactor;

	// Compute the radius for each pixel
	for (long y = 0; y < pix_ny; y++) {
		for (long x = 0; x < pix_nx; x++) {
			long idx = y * pix_nx + x;
			float dx = x - cx;
			float dy = y - cy;
			m_pCheetahPeakFinder->m_pix_r[idx] = sqrt(dx * dx + dy * dy);
		}
	}
	
	
	long asic_nx = pix_nx; // ASIC width
	long asic_ny = pix_ny; // ASIC height
	long nasics_x = 1;     // Number of ASICs in x-direction
	long nasics_y = 1;     // Number of ASICs in y-direction

	float ADCthresh = threshold;           // ADC threshold to ignore low-intensity pixels
	float hitfinderMinSNR = i_sigma;     // Minimum signal-to-noise ratio for a peak
	long hitfinderMinPixCount = peaksize / binFactor;    // Minimum number of pixels to be considered a peak
	long hitfinderMaxPixCount = 500 ;   // Maximum number of pixels in a peak
	long hitfinderLocalBGRadius = (hitfinderMinPixCount + 2);  // Radius for local background calculation

	

	
	tPeakList* peaklist = m_pCheetahPeakFinder->GetPeakList();
	m_pCheetahPeakFinder->reset_variables();
	m_pCheetahPeakFinder->peakfinder8_moussa(data, mask, m_pCheetahPeakFinder->m_pix_r, asic_nx, asic_ny, nasics_x, nasics_y, ADCthresh, hitfinderMinSNR, hitfinderMinPixCount, hitfinderMaxPixCount, hitfinderLocalBGRadius);
	//m_pCheetahPeakFinder->killNearbyPeaks(hitfinderMaxPixCount * 3);
	for (int i = 0; i < peaklist->nPeaks; i++)
		detectedPeaks.emplace_back(cv::Point2f(binFactor * peaklist->peak_com_x[i], binFactor * peaklist->peak_com_y[i]));
	
	return bool(peaklist->nPeaks);
}
// Function to find local maxima using dilation and erosion
/*
std::vector<cv::Point> CImageManager::bhFindLocalMaximum(cv::InputArray _src, int neighbor)
{

	cv::Mat src = _src.getMat();
	cv::Mat peak_img = src.clone();

	// Dilate image to highlight peaks
	cv::dilate(peak_img, peak_img, cv::Mat(), cv::Point(-1, -1), neighbor);
	peak_img = peak_img - src;

	// Erode image to flatten the non-peak regions
	cv::Mat flat_img;
	cv::erode(src, flat_img, cv::Mat(), cv::Point(-1, -1), neighbor);
	flat_img = src - flat_img;

	// Threshold both peak and flat images
	cv::threshold(peak_img, peak_img, 0, 255, cv::THRESH_BINARY);
	cv::threshold(flat_img, flat_img, 0, 255, cv::THRESH_BINARY);
	cv::bitwise_not(flat_img, flat_img);

	// Combine peak and flat images to isolate local maxima
	peak_img.setTo(cv::Scalar::all(255), flat_img);
	cv::bitwise_not(peak_img, peak_img);

	// Find contours of the local maxima
	try {
		cv::findContours(flat_img, m_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	double minContourArea = 10.0;  // Minimum contour area to consider as a peak

	// Filter contours by area
	for (size_t i = 0; i < m_contours.size(); i++) {
		double contourArea = cv::contourArea(m_contours[i]);
		if (contourArea >= minContourArea) {
			// Calculate center of mass
			cv::Moments m = cv::moments(m_contours[i], true);
			if (m.m00 != 0) {
				cv::Point center(m.m10 / m.m00, m.m01 / m.m00);
				m_filteredCenters.push_back(center);
			}
		}
	}
	m_contours.clear();

	// Sort peaks by contour area or intensity (optional)
	std::sort(m_filteredCenters.begin(), m_filteredCenters.end(), [&](const cv::Point& a, const cv::Point& b) {
		return src.at<uchar>(a) > src.at<uchar>(b);  // Sort by intensity at the peak
		});

	int maxPeaks = 100;
	if (m_filteredCenters.size() > static_cast<size_t>(maxPeaks)) {
		m_filteredCenters.resize(maxPeaks);  // Limit to maxPeaks
	}

	//filteredCenters.clear();
	// Return the filtered centers directly (no need to call bhContoursCenter)
	return m_filteredCenters;
}
*/


std::vector<cv::Point> CImageManager::bhFindLocalMaximum(const cv::InputArray& _src, const cv::InputArray& _mask, int neighbor, int maxPeaks, float minContourArea, float minDistance)
{
	cv::Mat src = _src.getMat();
	cv::Mat peak_img = src.clone();

	// Step 1: Dilate image to highlight peaks
	cv::dilate(peak_img, peak_img, cv::Mat(), cv::Point(-1, -1), neighbor);
	peak_img = peak_img - src;

	// Step 2: Erode image to flatten the non-peak regions
	cv::Mat flat_img;
	cv::erode(src, flat_img, cv::Mat(), cv::Point(-1, -1), neighbor);
	flat_img = src - flat_img;

	// Step 3: Threshold both peak and flat images
	cv::threshold(peak_img, peak_img, 0, 255, cv::THRESH_BINARY);
	cv::threshold(flat_img, flat_img, 0, 255, cv::THRESH_BINARY);
	cv::bitwise_not(flat_img, flat_img);

	// Step 4: Combine peak and flat images to isolate local maxima
	peak_img.setTo(cv::Scalar::all(255), flat_img);
	cv::bitwise_not(peak_img, peak_img);

	// Step 5: Use connectedComponentsWithStats instead of findContours
	cv::Mat labels, stats, centroids;
	int numLabels = cv::connectedComponentsWithStats(peak_img, labels, stats, centroids);

	std::vector<cv::Point> filteredCenters;
	// Step 6: Filter components by area and store their centroids
	for (int i = 1; i < numLabels; i++) {  // Start from 1 to skip the background
		int area = stats.at<int>(i, cv::CC_STAT_AREA);
		if (area >= minContourArea) {
			double centerX = centroids.at<double>(i, 0);
			double centerY = centroids.at<double>(i, 1);
			filteredCenters.push_back(cv::Point(static_cast<int>(centerX), static_cast<int>(centerY)));
		}
	}

	// Step 7: Sort by intensity if we have more than maxPeaks
	if (filteredCenters.size() > static_cast<size_t>(maxPeaks)) {
		std::sort(filteredCenters.begin(), filteredCenters.end(), [&](const cv::Point& a, const cv::Point& b) {
			return src.at<uchar>(a) > src.at<uchar>(b);  // Sort by intensity at the peak
			});
	}

	// Step 8: Remove peaks that are too close (less than minDistance apart)
	std::vector<cv::Point> finalPeaks;
	for (const auto& peak : filteredCenters) {
		bool tooClose = false;

		// Check against already selected peaks in finalPeaks
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

	// Return the final filtered peaks
	return finalPeaks;
}



std::vector<cv::Point> CImageManager::bhFindLocalMaximumGFTT(const cv::InputArray& _src, const cv::InputArray& _mask, int neighbor, int maxCorners, float qualityLevel, float minDistance)
{
	/*cv::Mat src = _src.getMat();
	cv::Mat peak_img = src.clone();

	// Dilate image to highlight peaks
	cv::dilate(peak_img, peak_img, cv::Mat(), cv::Point(-1, -1), neighbor);
	peak_img = peak_img - src;

	// Erode image to flatten the non-peak regions
	cv::Mat flat_img;
	cv::erode(src, flat_img, cv::Mat(), cv::Point(-1, -1), neighbor);
	flat_img = src - flat_img;

	// Threshold both peak and flat images
	cv::threshold(peak_img, peak_img, 0, 255, cv::THRESH_BINARY);
	cv::threshold(flat_img, flat_img, 0, 255, cv::THRESH_BINARY);
	cv::bitwise_not(flat_img, flat_img);

	// Combine peak and flat images to isolate local maxima
	peak_img.setTo(cv::Scalar::all(255), flat_img);
	cv::bitwise_not(peak_img, peak_img);*/

	// Now apply GFTT (Good Features to Track)
	std::vector<cv::Point2f> corners;
	cv::goodFeaturesToTrack(_src, corners, maxCorners, qualityLevel, minDistance, _mask);

	std::vector<cv::Point> finalPeaks;
	/*for (const auto& peak : corners) {
		bool tooClose = false;

		// Check against already selected peaks in finalPeaks
		for (const auto& selectedPeak : finalPeaks) {
			if (cv::norm(peak - cv::Point2f(selectedPeak)) < minDistance) {
				tooClose = true;
				break;
			}
		}

		// Only add the peak if it's not too close to any already selected peaks
		if (!tooClose) {
			finalPeaks.push_back(peak);
		}

		// Stop if we reach the max number of peaks
		if (finalPeaks.size() >= static_cast<size_t>(maxCorners)) {
			break;
		}
	}*/

	for (const auto& peak : corners)
		finalPeaks.push_back(peak);

	return finalPeaks;

}


// Helper function to find the center of mass of contours
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
	PRINTD("\t\t\t\tCImageManager::~CImageManager() - Destructor\n");
	SAFE_RELEASE(m_pImgMgr);
	SAFE_RELEASE(m_pCheetahPeakFinder);
	
}



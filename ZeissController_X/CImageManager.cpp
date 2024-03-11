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
			cv::convertScaleAbs(myImg, myImg, 0.8f * 0.5f * CTimepix::GetInstance()->m_fContrastImg / 100.0, CTimepix::GetInstance()->m_fBrightnessImg);

		}
		else
			myImg = cv::imread(v._sFileName);

		
		v._sWindowTitle= v._sFileName.substr(v._sFileName.find_last_of("/", v._sFileName.length()) + 1,
                                             v._sFileName.find_last_of(".") - v._sFileName.find_last_of("/") - 1) + " @ " + std::to_string(v._fImageAngle);
		
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
				cv::ellipse(display, v._center, cv::Size(5, 5), 0, 0, 360, cv::Scalar(65000, 0, 65000), 2); // 

				auto display_clone = display.clone();
				if (v._bIsShowCrystalPath)
				{
					cv::polylines(display, CDataCollection::GetInstance()->m_oCrystalPathCoordinates, false, cv::Scalar(65000, 0, 65000), 1);
					for (const cv::Point& point : CDataCollection::GetInstance()->m_oCrystalPathCoordinates) {
						cv::circle(display, point, 2, cv::Scalar(255, 0, 0), -1); // Draws a small filled circle at each point
					}
				}
				cv::imshow(v._sWindowTitle, display);
				auto q = cv::waitKey(10); // User has two minutes per image to decide where to click!
				if (iCounter >= 12000 || q == 'q' || v._iPosX == -2 || v._iPosY == -2)
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
			cv::resize(myImg, myImg, cv::Size(), 1.15, 1.15);
			
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
				cv::ellipse(display, v._center, cv::Size(radius, radius), 0, 0, 360, cv::Scalar(65000, 0, 65000), 2); // good alternative :)


				auto display_clone = display.clone();
				if (v._bIsShowCrystalPath)
				{
					cv::polylines(display, CDataCollection::GetInstance()->m_oCrystalPathCoordinatesSwapped, false, cv::Scalar(65000, 0, 65000), 1);
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

				if (q == 'q' || iCounter > 12000)
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
}



#pragma once


struct ImagesInfo;
enum ImgInfoPurpose;

class CImageManager
{
	cv::Mat myImg;

	static CImageManager* m_pImgMgr;
	CImageManager();

private:
	void display_image(std::vector<ImagesInfo>* _vImages); 
	cv::Mat poly_fit(std::vector<cv::Point>& points, unsigned int poly_order = 5);
	void draw_poly_fit_on_img(cv::Mat& img, cv::Mat& coeffs, cv::Vec3b color, unsigned int poly_order = 5);
	void display_tracking_plots(std::vector<ImagesInfo>* _vImages);
	cv::Point2d calculate_image_shift(cv::Mat& _ImgRef, cv::Mat& _Img);
//	void MouseEventHandler(int event, int x, int y, int flag, void*);

public:
	void display_image_ex(std::vector<ImagesInfo>* _vImages, ImgInfoPurpose purpose);
	cv::Point2d calculate_image_shift_ex(std::string& _sReferenceImg, std::string& _sTargetImg);
	cv::Point2d correct_lowmag_normalmag(cv::Point2d _originalCoords, float _correctionAngle);


	static CImageManager* GetInstance();
	~CImageManager();
private:
	void find_beam_position_hough_circles(cv::Mat& procesedImg, ImagesInfo& imgInfo);
	void find_beam_position_center_gravity(cv::Mat& img2, ImagesInfo& imgInfo);
};


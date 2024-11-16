#pragma once
#define IMG_RESIZE_FACTOR_ 1.15


struct ImagesInfo;
enum ImgInfoPurpose;

class CImageManager
{
	cv::Mat myImg;
	static std::vector<cv::RotatedRect> m_ellipses;
	CCheetah_PeakFinder* m_pCheetahPeakFinder;

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

	bool find_central_beam_position(const cv::Mat& imgInput, cv::Point2f& beamLocation);
	bool detect_diffraction_peaks_old_champ(const cv::Mat& imgInput, const cv::Point2f& centralBeam, double pixelSize, double resolutionLimit, std::vector<cv::Point2f>& detectedPeaks, int maxCorners, double qualityLevel = 0.2, double minDistance = 10.0);
	bool detect_diffraction_peaks_old(const cv::Mat& imgInput, std::vector<cv::Point2f>& detectedPeaks);
	cv::Mat preprocessImageForPeakDetection(const cv::Mat& grayImg, double reflectionSize, unsigned int binFactor);
	cv::Point2f calculateCentroid(const cv::Mat& img, const cv::Point& peakPosition, int windowSize);
	bool detect_diffraction_peaks(const cv::Mat& imgInput, const cv::Point2f& centralBeam, double pixelSize, double resolutionLimit, std::vector<cv::Point2f>& detectedPeaks, std::vector<cv::Point2f>& rejectedPeaks, int maxCorners, float i_sigma, double qualityLevel_contourSize, double minDistance = 10.0, unsigned int binFactor = 2, bool bGFTT = true);
	bool detect_diffraction_peaks_cheetahpf8(CCheetah_PeakFinder* _cheetah_pf, const cv::Mat& imgInput, const cv::Point2f& centralBeam, double pixelSize, double d_max, std::vector<cv::Point2f>& detectedPeaks, float i_sigma, double peaksize, double threshold, unsigned int binFactor = 2);
	bool detect_diffraction_peaks_imageJ(const cv::Mat& imgInput, const cv::Point2f& centralBeam, double pixelSize, double d_max, std::vector<cv::Point2f>& detectedPeaks, float tol);
	bool detect_diffraction_peaks_imageJ2(const cv::Mat& imgInput, const cv::Point2f& centralBeam, double pixelSize, double d_max, std::vector<cv::Point2f>& detectedPeaks, float tol);
	std::vector<cv::Point> bhFindLocalMaximum(const cv::InputArray& _src, const cv::InputArray & _mask, int neighbor, int maxPeaks, float minContourArea, float minDistance);
	std::vector<cv::Point> bhFindLocalMaximumGFTT(const cv::InputArray& _src, const cv::InputArray& _mask, int neighbor, int maxCorners, float qualityLevel, float minDistance);
	std::vector<cv::Point> bhContoursCenter(const std::vector<std::vector<cv::Point>>& contours, bool centerOfMass, int contourIdx);
	void draw_resolution_rings(cv::Mat& imgOutput, const cv::Point2f& beamLocation, double pixelSize);
	void adjustImageJStyle(cv::Mat& image, double min_val, double max_val);
	void adjustBrightnessContrast(const cv::Mat& image, int min_val, int max_val, int target_min, int target_max);
	void computeHistogramBasedMinMax(const cv::Mat& image, double& min_val, double& max_val, double lower_percentile, double upper_percentile);
	cv::Point findClosestPeak(int x, int y, const std::vector<cv::Point2f>& peaks, int maxDistance);
	static void onMousePowderRings(int event, int x, int y, int flags, void* userdata);
	void apply_distortion_corrections(const cv::Mat&dist_img, std::string&fileName, float EPS, float PHI, float X_center, float Y_center);
	void calibrate_distortions();
	static CImageManager* GetInstance();
	~CImageManager();
private:
	void find_beam_position_hough_circles(cv::Mat& procesedImg, ImagesInfo& imgInfo);
	void find_beam_position_center_gravity(cv::Mat& img2, ImagesInfo& imgInfo);
};


#pragma once

// Stolen/Adapted from --> https://github.com/antonbarty/cheetah


typedef struct {
public:
	long	    nPeaks;
	long	    nHot;
	float		peakResolution;			// Radius of 80% of peaks
	float		peakResolutionA;		// Radius of 80% of peaks
	float		peakDensity;			// Density of peaks within this 80% figure
	float		peakNpix;				// Number of pixels in peaks
	float		peakTotal;				// Total integrated intensity in peaks
	int			memoryAllocated;
	long		nPeaks_max;

	float* peak_maxintensity;		// Maximum intensity in peak
	float* peak_totalintensity;	// Integrated intensity in peak
	float* peak_sigma;			// Signal-to-noise ratio of peak
	float* peak_snr;				// Signal-to-noise ratio of peak
	float* peak_npix;				// Number of pixels in peak
	float* peak_com_x;			// peak center of mass x (in raw layout)
	float* peak_com_y;			// peak center of mass y (in raw layout)
	long* peak_com_index;		// closest pixel corresponding to peak
	float* peak_com_x_assembled;	// peak center of mass x (in assembled layout)
	float* peak_com_y_assembled;	// peak center of mass y (in assembled layout)
	float* peak_com_r_assembled;	// peak center of mass r (in assembled layout)
	float* peak_com_q;			// Scattering vector of this peak
	float* peak_com_res;			// REsolution of this peak
} tPeakList;

void allocatePeakList(tPeakList*, long);
void freePeakList(tPeakList);

class CCheetah_PeakFinder
{

	//static CCheetah_PeakFinder* m_pCheetahPeakFinder;
	tPeakList peaklist;
	
	long* m_inx;
	long* m_iny;
	long* m_peakpixels;
	char* m_peakpixel;
	float* m_rsigma;
	float* m_roffset;
	long* m_rcount;
	float* m_rthreshold;

	int lmaxr;
	unsigned int pix_nn;

public:
	float* m_pix_r;
	CCheetah_PeakFinder(unsigned int rows, unsigned int cols);


public:
	int killNearbyPeaks(float hitfinderMinPeakSeparation);
	int peakfinder3(float* data, char* mask, long asic_nx, long asic_ny, long nasics_x, long nasics_y, float ADCthresh, float hitfinderMinSNR, long hitfinderMinPixCount, long hitfinderMaxPixCount, long hitfinderLocalBGRadius);
	int peakfinder8_original(float* data, char* mask, float* pix_r, long asic_nx, long asic_ny, long nasics_x, long nasics_y, float ADCthresh, float hitfinderMinSNR, long hitfinderMinPixCount, long hitfinderMaxPixCount, long hitfinderLocalBGRadius);
	int peakfinder8_moussa(float* data, char* mask, float* pix_r, long asic_nx, long asic_ny, long nasics_x, long nasics_y, float ADCthresh, float hitfinderMinSNR, long hitfinderMinPixCount, long hitfinderMaxPixCount, long hitfinderLocalBGRadius);
	int box_snr(float* im, char* mask, int center, int radius, int thickness, int stride, float* SNR, float* background, float* backgroundSigma);
	void reset_variables();

	static CCheetah_PeakFinder* GetInstance(unsigned int rows, unsigned int cols);
	//static CCheetah_PeakFinder* GetInstance();
	tPeakList* GetPeakList();
	~CCheetah_PeakFinder();
};


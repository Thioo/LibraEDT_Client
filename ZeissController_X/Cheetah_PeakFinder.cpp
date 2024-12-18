#include "pch.h"
#include "Cheetah_PeakFinder.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

CCheetah_PeakFinder* CCheetah_PeakFinder::m_pCheetahPeakFinder = nullptr;



/*
 *	Create arrays for remembering Bragg peak data
 */
void allocatePeakList(tPeakList* peak, long NpeaksMax) {
	peak->nPeaks = 0;
	peak->nPeaks_max = NpeaksMax;
	peak->nHot = 0;
	peak->peakResolution = 0;
	peak->peakResolutionA = 0;
	peak->peakDensity = 0;
	peak->peakNpix = 0;
	peak->peakTotal = 0;

	peak->peak_maxintensity = (float*)calloc(NpeaksMax, sizeof(float));
	peak->peak_totalintensity = (float*)calloc(NpeaksMax, sizeof(float));
	peak->peak_sigma = (float*)calloc(NpeaksMax, sizeof(float));
	peak->peak_snr = (float*)calloc(NpeaksMax, sizeof(float));
	peak->peak_npix = (float*)calloc(NpeaksMax, sizeof(float));
	peak->peak_com_x = (float*)calloc(NpeaksMax, sizeof(float));
	peak->peak_com_y = (float*)calloc(NpeaksMax, sizeof(float));
	peak->peak_com_index = (long*)calloc(NpeaksMax, sizeof(long));
	peak->peak_com_x_assembled = (float*)calloc(NpeaksMax, sizeof(float));
	peak->peak_com_y_assembled = (float*)calloc(NpeaksMax, sizeof(float));
	peak->peak_com_r_assembled = (float*)calloc(NpeaksMax, sizeof(float));
	peak->peak_com_q = (float*)calloc(NpeaksMax, sizeof(float));
	peak->peak_com_res = (float*)calloc(NpeaksMax, sizeof(float));
	peak->memoryAllocated = 1;
}

/*
 *	Clean up Bragg peak arrays
 */
void freePeakList(tPeakList peak) {
	free(peak.peak_maxintensity);
	free(peak.peak_totalintensity);
	free(peak.peak_sigma);
	free(peak.peak_snr);
	free(peak.peak_npix);
	free(peak.peak_com_x);
	free(peak.peak_com_y);
	free(peak.peak_com_index);
	free(peak.peak_com_x_assembled);
	free(peak.peak_com_y_assembled);
	free(peak.peak_com_r_assembled);
	free(peak.peak_com_q);
	free(peak.peak_com_res);
	peak.memoryAllocated = 0;
}



CCheetah_PeakFinder::CCheetah_PeakFinder(unsigned int rows, unsigned int cols)
{
	
	allocatePeakList(&peaklist, 500);

	

	pix_nn = rows * cols;
	m_inx = (long*)malloc(pix_nn * sizeof(long));
	m_iny = (long*)malloc(pix_nn * sizeof(long));
	m_peakpixel = (char*)malloc(pix_nn * sizeof(char));
	m_peakpixels = (long*)malloc(1000 * sizeof(long)); // We're using 100 to be on the safe side again, but this should be = to the maximum number of pixels in a peak
	

	// Assuming the central beam is ALWAYS close to the center of the image... we can estimate the lmaxr and avoid dynamically allocating memory per call
	// Get the distance between vector (0,0) to (rows/2,cols/2)
	float maxr = sqrt((rows / 2) * (rows / 2) + (cols / 2) * (cols / 2));
	lmaxr = (int)ceil(maxr) + 100; // extra 100 pixels to be extra safe!


	// Allocate and zero arrays
	m_rsigma = (float*)malloc(lmaxr * sizeof(float));
	m_roffset = (float*)malloc(lmaxr * sizeof(float));
	m_rcount = (long*)malloc(lmaxr * sizeof(long));
	m_rthreshold = (float*)malloc(lmaxr * sizeof(float));
	// m_pix_r = new float[pix_nn] using malloc
	m_pix_r = (float*)malloc(pix_nn * sizeof(float));




}

CCheetah_PeakFinder::~CCheetah_PeakFinder()
{
	freePeakList(peaklist);

	free(m_inx);
	free(m_iny);
	free(m_peakpixel);
	free(m_peakpixels);

	free(m_rsigma);
	free(m_pix_r);
	free(m_roffset);
	free(m_rcount);
	free(m_rthreshold);




}



/*
 *	Find peaks that are too close together and remove them
 */
int CCheetah_PeakFinder::killNearbyPeaks(float hitfinderMinPeakSeparation) {

	tPeakList* peaklist = &this->peaklist;
	int p, p1, p2;
	int k = 0;
	int n = peaklist->nPeaks;
	float	x1, x2;
	float	y1, y2;
	float	d2;
	float	min_dsq = hitfinderMinPeakSeparation * hitfinderMinPeakSeparation;


	if (hitfinderMinPeakSeparation <= 0) {
		return n;
	}

	char* killpeak = (char*)calloc(n, sizeof(char));
	for (long i = 0; i < n; i++)
		killpeak[i] = 0;

	if (n > peaklist->nPeaks_max)
		n = peaklist->nPeaks_max;

	for (p1 = 0; p1 < n; p1++) {
		x1 = peaklist->peak_com_x_assembled[p1];
		y1 = peaklist->peak_com_y_assembled[p1];
		for (p2 = p1 + 1; p2 < n; p2++) {
			x2 = peaklist->peak_com_x_assembled[p2];
			y2 = peaklist->peak_com_y_assembled[p2];

			d2 = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);

			if (d2 <= min_dsq) {
				if (peaklist->peak_maxintensity[p1] > peaklist->peak_maxintensity[p2]) {
					killpeak[p1] = 0;
					killpeak[p2] = 1;
					k++;
				}
				else {
					killpeak[p1] = 1;
					killpeak[p2] = 0;
					k++;
				}
			}
		}
	}

	long c = 0;
	for (p = 0; p < n; p++) {
		if (killpeak[p] == 0) {
			peaklist->peak_maxintensity[c] = peaklist->peak_maxintensity[p];
			peaklist->peak_totalintensity[c] = peaklist->peak_totalintensity[p];
			peaklist->peak_snr[c] = peaklist->peak_snr[p];
			peaklist->peak_sigma[c] = peaklist->peak_sigma[p];
			peaklist->peak_npix[c] = peaklist->peak_npix[p];
			peaklist->peak_com_x[c] = peaklist->peak_com_x[p];
			peaklist->peak_com_y[c] = peaklist->peak_com_y[p];
			peaklist->peak_com_index[c] = peaklist->peak_com_index[p];
			peaklist->peak_com_x_assembled[c] = peaklist->peak_com_x_assembled[p];
			peaklist->peak_com_y_assembled[c] = peaklist->peak_com_y_assembled[p];
			peaklist->peak_com_r_assembled[c] = peaklist->peak_com_r_assembled[p];
			peaklist->peak_com_q[c] = peaklist->peak_com_q[p];
			peaklist->peak_com_res[c] = peaklist->peak_com_res[p];
			c++;
		}
	}
	free(killpeak);

	peaklist->nPeaks = c;
	return c;
}




/*
 *	Peakfinder 3
 *	Count peaks by searching for connected pixels above threshold
 *	Anton Barty
 */
int CCheetah_PeakFinder::peakfinder3(float* data, char* mask, long asic_nx, long asic_ny, long nasics_x, long nasics_y, float ADCthresh, float hitfinderMinSNR, long hitfinderMinPixCount, long hitfinderMaxPixCount, long hitfinderLocalBGRadius) {

	tPeakList* peaklist = &this->peaklist;

	// Derived values
	long	pix_nx = asic_nx * nasics_x;
	long	pix_ny = asic_ny * nasics_y;
	long	pix_nn = pix_nx * pix_ny;
	//long	asic_nn = asic_nx*asic_ny;
	long	hitfinderNpeaksMax = peaklist->nPeaks_max;


	peaklist->nPeaks = 0;
	peaklist->peakNpix = 0;
	peaklist->peakTotal = 0;


	// Variables for this hitfinder
	long	nat = 0;
	long	lastnat = 0;
	long	counter = 0;
	float	total;
	int		search_x[] = { 0,-1,0,1,-1,1,-1,0,1 };
	int		search_y[] = { 0,-1,-1,-1,0,0,1,1,1 };
	int		search_n = 9;
	long	e;
	long* inx = (long*)calloc(pix_nn, sizeof(long));
	long* iny = (long*)calloc(pix_nn, sizeof(long));
	char* peakpixel = (char*)calloc(pix_nn, sizeof(long));
	float	totI;
	float	maxI;
	float	snr;
	float	peak_com_x;
	float	peak_com_y;
	long	thisx;
	long	thisy;
	long	fs, ss;
	float	com_x, com_y;


	nat = 0;
	counter = 0;
	total = 0.0;
	snr = 0;
	maxI = 0;

	/*
	 *	Create a buffer for image data so we don't nuke the main image by mistake
	 */
	float* temp = (float*)calloc(pix_nn, sizeof(float));
	memcpy(temp, data, pix_nn * sizeof(float));


	/*
	 *	Apply mask (multiply data by 0 to ignore regions - this makes data below threshold for peak finding)
	 */
	for (long i = 0; i < pix_nn; i++) {
		temp[i] *= mask[i];
	}
	for (long i = 0; i < pix_nn; i++) {
		peakpixel[i] = 0;
	}



	// Loop over modules (8x8 array)
	for (long mj = 0; mj < nasics_y; mj++) {
		for (long mi = 0; mi < nasics_x; mi++) {

			// Loop over pixels within a module
			for (long j = 1; j < asic_ny - 1; j++) {
				for (long i = 1; i < asic_nx - 1; i++) {


					ss = (j + mj * asic_ny) * pix_nx;
					fs = i + mi * asic_nx;
					e = ss + fs;

					if (e >= pix_nn) {
						printf("Array bounds error: e=%li\n", e);
						exit(1);
					}

					if (temp[e] > ADCthresh && peakpixel[e] == 0) {
						// This might be the start of a new peak - start searching
						inx[0] = i;
						iny[0] = j;
						nat = 1;
						totI = 0;
						maxI = 0;
						peak_com_x = 0;
						peak_com_y = 0;

						// Keep looping until the pixel count within this peak does not change
						do {

							lastnat = nat;
							// Loop through points known to be within this peak
							for (long p = 0; p < nat; p++) {
								// Loop through search pattern
								for (long k = 0; k < search_n; k++) {
									// Array bounds check
									if ((inx[p] + search_x[k]) < 0)
										continue;
									if ((inx[p] + search_x[k]) >= asic_nx)
										continue;
									if ((iny[p] + search_y[k]) < 0)
										continue;
									if ((iny[p] + search_y[k]) >= asic_ny)
										continue;

									// Neighbour point in big array
									thisx = inx[p] + search_x[k] + mi * asic_nx;
									thisy = iny[p] + search_y[k] + mj * asic_ny;
									e = thisx + thisy * pix_nx;

									//if(e < 0 || e >= pix_nn){
									//	printf("Array bounds error: e=%i\n",e);
									//	continue;
									//}

									// Above threshold?
									if (temp[e] > ADCthresh && peakpixel[e] == 0) {
										//if(nat < 0 || nat >= global->pix_nn) {
										//	printf("Array bounds error: nat=%i\n",nat);
										//	break
										//}
										if (temp[e] > maxI)
											maxI = temp[e];
										totI += temp[e]; // add to integrated intensity
										peak_com_x += temp[e] * ((float)thisx); // for center of mass x
										peak_com_y += temp[e] * ((float)thisy); // for center of mass y
										temp[e] = 0; // zero out this intensity so that we don't count it again
										inx[nat] = inx[p] + search_x[k];
										iny[nat] = iny[p] + search_y[k];
										nat++;
										peakpixel[e] = 1;
									}
								}
							}
						} while (lastnat != nat);


						// Too many or too few pixels means ignore this 'peak'; move on now
						if (nat<hitfinderMinPixCount || nat>hitfinderMaxPixCount) {
							continue;
						}


						/*
						 *	Calculate center of mass
						 */
						com_x = peak_com_x / fabs(totI);
						com_y = peak_com_y / fabs(totI);

						long   com_xi = lrint(com_x) - mi * asic_nx;
						long   com_yi = lrint(com_y) - mj * asic_ny;


						/*
						 *	Calculate signal-to-noise ratio in an annulus around this peak
						 */
						float   localSigma = 0;
						long    ringWidth = 2 * hitfinderLocalBGRadius;
						float   thisr;

						float   sum = 0;
						float   sumsquared = 0;
						long    np_sigma = 0;
						long	np_counted = 0;
						for (long bj = -ringWidth; bj < ringWidth; bj++) {
							for (long bi = -ringWidth; bi < ringWidth; bi++) {

								// Within annulus, or square?
								thisr = sqrt(bi * bi + bj * bj);
								//if(thisr < hitfinderLocalBGRadius || thisr > 2*hitfinderLocalBGRadius )
								//if(thisr < hitfinderLocalBGRadius)
								//	continue;

								// Within-ASIC check
								if ((com_xi + bi) < 0)
									continue;
								if ((com_xi + bi) >= asic_nx)
									continue;
								if ((com_yi + bj) < 0)
									continue;
								if ((com_yi + bj) >= asic_ny)
									continue;

								// Position of this point in data stream
								thisx = com_xi + bi + mi * asic_nx;
								thisy = com_yi + bj + mj * asic_ny;
								e = thisx + thisy * pix_nx;


								// If pixel is less than ADC threshold, this pixel is a part of the background and not part of a peak
								if (temp[e] < ADCthresh && peakpixel[e] == 0) {
									np_sigma++;
									sum += temp[e];
									sumsquared += (temp[e] * temp[e]);
								}
								np_counted += 1;
							}
						}

						// Calculate =standard deviation
						if (np_sigma == 0)
							continue;
						if (np_sigma < 0.5 * np_counted)
							continue;

						if (np_sigma != 0)
							localSigma = sqrt(sumsquared / np_sigma - ((sum / np_sigma) * (sum / np_sigma)));
						else
							localSigma = 0;

						// This part of the detector is too noisy if we have too few pixels < ADCthresh in background region

						// sigma
						snr = (float)maxI / localSigma;


						// Signal to noise criterion (turn off check by setting hitfinderMinSNR = 0)
						//printf("HitfinderMinSNR, Imax, sigma: %f, %f, %f\n", hitfinderMinSNR, maxI, localSigma);
						if (hitfinderMinSNR > 0) {
							if (maxI < (localSigma * hitfinderMinSNR)) {
								continue;
							}
						}


						// This is a peak? If so, add info to peak list
						if (nat >= hitfinderMinPixCount && nat <= hitfinderMaxPixCount) {

							// This CAN happen!
							if (totI == 0)
								continue;

							com_x = peak_com_x / fabs(totI);
							com_y = peak_com_y / fabs(totI);

							e = lrint(com_x) + lrint(com_y) * pix_nx;
							if (e < 0 || e >= pix_nn) {
								printf("Array bounds error: e=%ld\n", e);
								continue;
							}

							// Remember peak information
							if (counter < hitfinderNpeaksMax) {
								peaklist->peakNpix += nat;
								peaklist->peakTotal += totI;
								peaklist->peak_com_index[counter] = e;
								peaklist->peak_npix[counter] = nat;
								peaklist->peak_com_x[counter] = com_x;
								peaklist->peak_com_y[counter] = com_y;
								peaklist->peak_totalintensity[counter] = totI;
								peaklist->peak_maxintensity[counter] = maxI;
								peaklist->peak_sigma[counter] = localSigma;
								peaklist->peak_snr[counter] = snr;
								counter++;
								peaklist->nPeaks = counter;
							}
							else {
								counter++;
							}
						}
					}
				}
			}
		}
	}


	free(temp);
	free(inx);
	free(iny);
	free(peakpixel);

	return(peaklist->nPeaks);

}


/*
 *	Peakfinder 8
 *	Version before modifications during Cherezov December 2014 LE80
 *	Count peaks by searching for connected pixels above threshold
 *	Anton Barty
 */
int CCheetah_PeakFinder::peakfinder8_original(float* data, char* mask, float* pix_r, long asic_nx, long asic_ny, long nasics_x, long nasics_y, float ADCthresh, float hitfinderMinSNR, long hitfinderMinPixCount, long hitfinderMaxPixCount, long hitfinderLocalBGRadius) {

	tPeakList* peaklist = &this->peaklist;
	// Derived values
	long	pix_nx = asic_nx * nasics_x;
	long	pix_ny = asic_ny * nasics_y;
	long	pix_nn = pix_nx * pix_ny;
	//long	asic_nn = asic_nx*asic_ny;
	long	hitfinderNpeaksMax = peaklist->nPeaks_max;


	peaklist->nPeaks = 0;
	peaklist->peakNpix = 0;
	peaklist->peakTotal = 0;


	// Variables for this hitfinder
	long	nat = 0;
	long	lastnat = 0;
	//long	counter=0;
	float	total;
	int		search_x[] = { 0,-1,0,1,-1,1,-1,0,1 };
	int		search_y[] = { 0,-1,-1,-1,0,0,1,1,1 };
	int		search_n = 9;
	long	e;
	long* inx = (long*)calloc(pix_nn, sizeof(long));
	long* iny = (long*)calloc(pix_nn, sizeof(long));
	float   thisI, thisIraw;
	float	totI, totIraw;
	float	maxI, maxIraw;
	float	snr;
	float	peak_com_x;
	float	peak_com_y;
	long	thisx;
	long	thisy;
	long	fs, ss;
	float	com_x, com_y, com_e;
	float	thisADCthresh;


	nat = 0;
	//counter = 0;
	total = 0.0;
	snr = 0;
	maxI = 0;

	/*
	 *	Create a buffer for image data so we don't nuke the main image by mistake
	 */
	float* temp = (float*)calloc(pix_nn, sizeof(float)); 
	if (!temp) {
		printf("Memory allocation error: temp\n");
		return 0;
	}
	memcpy(temp, data, pix_nn * sizeof(float));


	/*
	 *	Apply mask (multiply data by 0 to ignore regions - this makes data below threshold for peak finding)
	 */
	for (long i = 0; i < pix_nn; i++) {
		temp[i] *= mask[i];
	}

	/*
	 *	Determine noise and offset as a funciton of radius
	 */
	float	fminr, fmaxr;
	long	lminr, lmaxr;
	fminr = 1e9;
	fmaxr = -1e9;

	// Figure out radius bounds
	for (long i = 0; i < pix_nn; i++) {
		if (pix_r[i] > fmaxr)
			fmaxr = pix_r[i];
		if (pix_r[i] < fminr)
			fminr = pix_r[i];
	}
	lmaxr = (int)ceil(fmaxr) + 1;
	lminr = 0;

	// Allocate and zero arrays
	float* rsigma = (float*)calloc(lmaxr, sizeof(float));
	float* roffset = (float*)calloc(lmaxr, sizeof(float));
	long* rcount = (long*)calloc(lmaxr, sizeof(long));
	float* rthreshold = (float*)calloc(lmaxr, sizeof(float));

	long* peakpixels = (long*)calloc(hitfinderMaxPixCount, sizeof(long));
	char* peakpixel = (char*)calloc(pix_nn, sizeof(char));

	for (long i = 0; i < lmaxr; i++) {
		rthreshold[i] = 1e9;
	}

	// Compute sigma and average of data values at each radius
	// From this, compute the ADC threshold to be applied at each radius
	// Iterate a few times to reduce the effect of positive outliers (ie: peaks)
	long	thisr;
	float	thisoffset, thissigma;
	for (long counter = 0; counter < 5; counter++) {
		for (long i = 0; i < lmaxr; i++) {
			roffset[i] = 0;
			rsigma[i] = 0;
			rcount[i] = 0;
		}
		for (long i = 0; i < pix_nn; i++) {
			if (mask[i] != 0) {
				thisr = lrint(pix_r[i]);
				if (temp[i] < rthreshold[thisr]) {
					roffset[thisr] += temp[i];
					rsigma[thisr] += (temp[i] * temp[i]);
					rcount[thisr] += 1;
				}
			}
		}
		for (long i = 0; i < lmaxr; i++) {
			if (rcount[i] == 0) {
				roffset[i] = 0;
				rsigma[i] = 0;
				rthreshold[i] = 1e9;
				//rthreshold[i] = ADCthresh;		// For testing
			}
			else {
				thisoffset = roffset[i] / rcount[i];
				thissigma = sqrt(rsigma[i] / rcount[i] - ((roffset[i] / rcount[i]) * (roffset[i] / rcount[i])));
				roffset[i] = thisoffset;
				rsigma[i] = thissigma;
				rthreshold[i] = roffset[i] + hitfinderMinSNR * rsigma[i];
				if (rthreshold[i] < ADCthresh)
					rthreshold[i] = ADCthresh;
				//rthreshold[i] = ADCthresh;		// For testing
			}
		}
	}


	com_x = 0;
	com_y = 0;

	// Loop over modules (8x8 array)
	//counter = 0;
	long peakCounter = 0;
	for (long mj = 0; mj < nasics_y; mj++) {
		for (long mi = 0; mi < nasics_x; mi++) {

			// Loop over pixels within a module
			for (long j = 1; j < asic_ny - 1; j++) {
				for (long i = 1; i < asic_nx - 1; i++) {


					ss = (j + mj * asic_ny) * pix_nx;
					fs = i + mi * asic_nx;
					e = ss + fs;

					if (e > pix_nn) {
						printf("Array bounds error: e=%li\n", e);
						exit(1);
					}

					thisr = lrint(pix_r[e]);
					thisADCthresh = rthreshold[thisr];

					if (temp[e] > thisADCthresh && peakpixel[e] == 0) {
						// This might be the start of a new peak - start searching
						inx[0] = i;
						iny[0] = j;
						peakpixels[0] = e;
						nat = 1;
						totI = 0;
						totIraw = 0;
						maxI = 0;
						maxIraw = 0;
						peak_com_x = 0;
						peak_com_y = 0;

						// Keep looping until the pixel count within this peak does not change
						do {

							lastnat = nat;
							// Loop through points known to be within this peak
							for (long p = 0; p < nat; p++) {
								// Loop through search pattern
								for (long k = 0; k < search_n; k++) {
									// Array bounds check
									if ((inx[p] + search_x[k]) < 0)
										continue;
									if ((inx[p] + search_x[k]) >= asic_nx)
										continue;
									if ((iny[p] + search_y[k]) < 0)
										continue;
									if ((iny[p] + search_y[k]) >= asic_ny)
										continue;

									// Neighbour point in big array
									thisx = inx[p] + search_x[k] + mi * asic_nx;
									thisy = iny[p] + search_y[k] + mj * asic_ny;
									e = thisx + thisy * pix_nx;

									//if(e < 0 || e >= pix_nn){
									//	printf("Array bounds error: e=%i\n",e);
									//	continue;
									//}

									thisr = lrint(pix_r[e]);
									thisADCthresh = rthreshold[thisr];

									// Above threshold?
									if (temp[e] > thisADCthresh && peakpixel[e] == 0 && mask[e] != 0) {
										//if(nat < 0 || nat >= global->pix_nn) {
										//	printf("Array bounds error: nat=%i\n",nat);
										//	break
										//}
										thisI = temp[e] - roffset[thisr];
										totI += thisI; // add to integrated intensity
										totIraw += temp[e];
										peak_com_x += thisI * ((float)thisx); // for center of mass x
										peak_com_y += thisI * ((float)thisy); // for center of mass y
										//temp[e] = 0; // zero out this intensity so that we don't count it again
										inx[nat] = inx[p] + search_x[k];
										iny[nat] = iny[p] + search_y[k];
										peakpixel[e] = 1;
										if (nat < hitfinderMaxPixCount)
											peakpixels[nat] = e;
										if (thisI > maxI)
											maxI = thisI;
										if (thisI > maxIraw)
											maxIraw = temp[e];

										nat++;
									}
								}
							}
						} while (lastnat != nat);


						// Too many or too few pixels means ignore this 'peak'; move on now
						if (nat<hitfinderMinPixCount || nat>hitfinderMaxPixCount) {
							continue;
						}


						/*
						 *	Calculate center of mass for this peak from initial peak search
						 */
						com_x = peak_com_x / fabs(totI);
						com_y = peak_com_y / fabs(totI);
						com_e = lrint(com_x) + lrint(com_y) * pix_nx;

						long   com_xi = lrint(com_x) - mi * asic_nx;
						long   com_yi = lrint(com_y) - mj * asic_ny;


						/*
						 *	Calculate the local signal-to-noise ratio and local background in an annulus around this peak
						 *	(excluding pixels which look like they might be part of another peak)
						 */
						float   localSigma = 0;
						float   localOffset = 0;
						long    ringWidth = 2 * hitfinderLocalBGRadius;

						float   sumI = 0;
						float   sumIsquared = 0;
						long    np_sigma = 0;
						long	np_counted = 0;
						float	fbgr;
						float	backgroundMaxI = 0;
						float	fBackgroundThresh = 0;

						for (long bj = -ringWidth; bj < ringWidth; bj++) {
							for (long bi = -ringWidth; bi < ringWidth; bi++) {

								// Within-ASIC check
								if ((com_xi + bi) < 0)
									continue;
								if ((com_xi + bi) >= asic_nx)
									continue;
								if ((com_yi + bj) < 0)
									continue;
								if ((com_yi + bj) >= asic_ny)
									continue;

								// Within outer ring check
								fbgr = sqrt(bi * bi + bj * bj);
								if (fbgr > ringWidth)// || fbgr <= hitfinderLocalBGRadius )				// || fbgr > hitfinderLocalBGRadius)
									continue;

								// Position of this point in data stream
								thisx = com_xi + bi + mi * asic_nx;
								thisy = com_yi + bj + mj * asic_ny;
								e = thisx + thisy * pix_nx;

								thisr = lrint(pix_r[e]);
								thisADCthresh = rthreshold[thisr];

								// Intensity above background
								thisI = temp[e];


								// If above ADC threshold, this could be part of another peak
								//if (temp[e] > thisADCthresh)
								//	continue;

								// Keep track of value and value-squared for offset and sigma calculation
								// if(peakpixel[e] == 0 && mask[e] != 0) {
								if (temp[e] < thisADCthresh && peakpixel[e] == 0 && mask[e] != 0) {
									np_sigma++;
									sumI += thisI;
									sumIsquared += (thisI * thisI);
									if (thisI > backgroundMaxI) {
										backgroundMaxI = thisI;
									}
								}
								np_counted += 1;
							}
						}

						// Calculate local background and standard deviation
						if (np_sigma != 0) {
							localOffset = sumI / np_sigma;
							localSigma = sqrt(sumIsquared / np_sigma - ((sumI / np_sigma) * (sumI / np_sigma)));
						}
						else {
							localOffset = roffset[lrint(pix_r[lrint(com_e)])];
							localSigma = 0.01;
						}


						/*
						 *	Re-integrate (and re-centroid) peak using local background estimates
						 */
						totI = 0;
						totIraw = 0;
						maxI = 0;
						maxIraw = 0;
						peak_com_x = 0;
						peak_com_y = 0;
						for (long counter = 1; counter < nat && counter <= hitfinderMaxPixCount; counter++) {
							e = peakpixels[counter];
							thisIraw = temp[e];
							thisI = thisIraw - localOffset;

							totI += thisI;
							totIraw += thisIraw;

							// Remember that e = thisx + thisy*pix_nx;
							ldiv_t xy = ldiv(e, pix_nx);
							thisx = xy.rem;
							thisy = xy.quot;
							peak_com_x += thisI * ((float)thisx); // for center of mass x
							peak_com_y += thisI * ((float)thisy); // for center of mass y

							if (thisIraw > maxIraw)
								maxIraw = thisIraw;
							if (thisI > maxI)
								maxI = thisI;
						}
						com_x = peak_com_x / fabs(totI);
						com_y = peak_com_y / fabs(totI);
						com_e = lrint(com_x) + lrint(com_y) * pix_nx;



						/*
						 *	Calculate signal-to-noise and apply SNR criteria
						 */
						snr = (float)(totI) / localSigma;
						//snr = (float) (maxI)/localSigma;
						//snr = (float) (totIraw-nat*localOffset)/localSigma;
						//snr = (float) (maxIraw-localOffset)/localSigma;

						// The more pixels there are in the peak, the more relaxed we are about this criterion
						if (snr < hitfinderMinSNR)        //   - nat +hitfinderMinPixCount
							continue;

						// Is the maximum intensity in the peak enough above intensity in background region to be a peak and not noise?
						// The more pixels there are in the peak, the more relaxed we are about this criterion
						//fBackgroundThresh = hitfinderMinSNR - nat;
						//if(fBackgroundThresh > 4) fBackgroundThresh = 4;
						fBackgroundThresh = 1;
						fBackgroundThresh *= (backgroundMaxI - localOffset);
						if (maxI < fBackgroundThresh)
							continue;


						// This is a peak? If so, add info to peak list
						if (nat >= hitfinderMinPixCount && nat <= hitfinderMaxPixCount) {

							// This CAN happen!
							if (totI == 0)
								continue;

							//com_x = peak_com_x/fabs(totI);
							//com_y = peak_com_y/fabs(totI);

							e = lrint(com_x) + lrint(com_y) * pix_nx;
							if (e < 0 || e >= pix_nn) {
								printf("Array bounds error: e=%ld\n", e);
								continue;
							}

							// Remember peak information
							if (peakCounter < hitfinderNpeaksMax) {
								peaklist->peakNpix += nat;
								peaklist->peakTotal += totI;
								peaklist->peak_com_index[peakCounter] = e;
								peaklist->peak_npix[peakCounter] = nat;
								peaklist->peak_com_x[peakCounter] = com_x;
								peaklist->peak_com_y[peakCounter] = com_y;
								peaklist->peak_totalintensity[peakCounter] = totI;
								peaklist->peak_maxintensity[peakCounter] = maxI;
								peaklist->peak_sigma[peakCounter] = localSigma;
								peaklist->peak_snr[peakCounter] = snr;
								peakCounter++;
								peaklist->nPeaks = peakCounter;
							}
							else {
								peakCounter++;
							}
						}
					}
				}
			}
		}
	}

	free(temp);
	free(inx);
	free(iny);
	free(peakpixel);
	free(peakpixels);


	free(roffset);
	free(rsigma);
	free(rcount);
	free(rthreshold);


	peaklist->nPeaks = peakCounter;
	return(peaklist->nPeaks);
	/*************************************************/

}

int CCheetah_PeakFinder::peakfinder8_moussa(float* data, char* mask, float* pix_r, long asic_nx, long asic_ny, long nasics_x, long nasics_y, float ADCthresh, float hitfinderMinSNR, long hitfinderMinPixCount, long hitfinderMaxPixCount, long hitfinderLocalBGRadius) {



	tPeakList* peaklist = &this->peaklist;
	// Derived values
	long	pix_nx = asic_nx * nasics_x;
	long	pix_ny = asic_ny * nasics_y;
	long	pix_nn = pix_nx * pix_ny;
	//long	asic_nn = asic_nx*asic_ny;
	long	hitfinderNpeaksMax = peaklist->nPeaks_max;


	peaklist->nPeaks = 0;
	peaklist->peakNpix = 0;
	peaklist->peakTotal = 0;


	// Variables for this hitfinder
	long	nat = 0;
	long	lastnat = 0;
	//long	counter=0;
	float	total;
	int		search_x[] = { 0,-1,0,1,-1,1,-1,0,1 };
	int		search_y[] = { 0,-1,-1,-1,0,0,1,1,1 };
	int		search_n = 9;
	long	e;
	//long* inx = (long*)calloc(pix_nn, sizeof(long));
	//long* iny = (long*)calloc(pix_nn, sizeof(long));

	long* inx = this->m_inx;
	long* iny = this->m_iny;

	float   thisI, thisIraw;
	float	totI, totIraw;
	float	maxI, maxIraw;
	float	snr;
	float	peak_com_x;
	float	peak_com_y;
	long	thisx;
	long	thisy;
	long	fs, ss;
	float	com_x, com_y, com_e;
	float	thisADCthresh;


	nat = 0;
	//counter = 0;
	total = 0.0;
	snr = 0;
	maxI = 0;

	/*
	 *	Create a buffer for image data so we don't nuke the main image by mistake
	 */
	/*float* temp = (float*)calloc(pix_nn, sizeof(float));
	if (!temp) {
		printf("Memory allocation error: temp\n");
		return 0;
	}
	memcpy(temp, data, pix_nn * sizeof(float));*/
	float* temp = data;


	/*
	 *	Apply mask (multiply data by 0 to ignore regions - this makes data below threshold for peak finding)
	 */
	for (long i = 0; i < pix_nn; i++) {
		temp[i] *= mask[i];
	}

	/*
	 *	Determine noise and offset as a funciton of radius
	 */
	float	fminr, fmaxr;
	long	lminr, lmaxr;
	fminr = 1e9;
	fmaxr = -1e9;

	// Figure out radius bounds
	for (long i = 0; i < pix_nn; i++) {
		if (pix_r[i] > fmaxr)
			fmaxr = pix_r[i];
		if (pix_r[i] < fminr)
			fminr = pix_r[i];
	}
	lmaxr = (int)ceil(fmaxr) + 1;
	lminr = 0;

	// Allocate and zero arrays
	//float* rsigma = (float*)calloc(lmaxr, sizeof(float));
	//float* roffset = (float*)calloc(lmaxr, sizeof(float));
	//long* rcount = (long*)calloc(lmaxr, sizeof(long));
	//float* rthreshold = (float*)calloc(lmaxr, sizeof(float));

	float* rsigma = this->m_rsigma;
	float* roffset = this->m_roffset;
	long* rcount = this->m_rcount;
	float* rthreshold = this->m_rthreshold;

	

	//long* peakpixels = (long*)calloc(hitfinderMaxPixCount, sizeof(long));
	long* peakpixels = this->m_peakpixels;
	char* peakpixel = this->m_peakpixel;

	for (long i = 0; i < lmaxr; i++) {
		rthreshold[i] = 1e9;
	}

	// Compute sigma and average of data values at each radius
	// From this, compute the ADC threshold to be applied at each radius
	// Iterate a few times to reduce the effect of positive outliers (ie: peaks)
	long	thisr;
	float	thisoffset, thissigma;
	for (long counter = 0; counter < 5; counter++) {
		for (long i = 0; i < lmaxr; i++) {
			roffset[i] = 0;
			rsigma[i] = 0;
			rcount[i] = 0;
		}
		for (long i = 0; i < pix_nn; i++) {
			if (mask[i] != 0) {
				thisr = lrint(pix_r[i]);
				if (temp[i] < rthreshold[thisr]) {
					roffset[thisr] += temp[i];
					rsigma[thisr] += (temp[i] * temp[i]);
					rcount[thisr] += 1;
				}
			}
		}
		for (long i = 0; i < lmaxr; i++) {
			if (rcount[i] == 0) {
				roffset[i] = 0;
				rsigma[i] = 0;
				rthreshold[i] = 1e9;
				//rthreshold[i] = ADCthresh;		// For testing
			}
			else {
				thisoffset = roffset[i] / rcount[i];
				thissigma = sqrt(rsigma[i] / rcount[i] - ((roffset[i] / rcount[i]) * (roffset[i] / rcount[i])));
				roffset[i] = thisoffset;
				rsigma[i] = thissigma;
				rthreshold[i] = roffset[i] + hitfinderMinSNR * rsigma[i];
				if (rthreshold[i] < ADCthresh)
					rthreshold[i] = ADCthresh;
				//rthreshold[i] = ADCthresh;		// For testing
			}
		}
	}


	com_x = 0;
	com_y = 0;

	// Loop over modules (8x8 array)
	//counter = 0;
	long peakCounter = 0;
	for (long mj = 0; mj < nasics_y; mj++) {
		for (long mi = 0; mi < nasics_x; mi++) {

			// Loop over pixels within a module
			for (long j = 1; j < asic_ny - 1; j++) {
				for (long i = 1; i < asic_nx - 1; i++) {


					ss = (j + mj * asic_ny) * pix_nx;
					fs = i + mi * asic_nx;
					e = ss + fs;

					if (e > pix_nn) {
						printf("Array bounds error: e=%li\n", e);
						exit(1);
					}

					thisr = lrint(pix_r[e]);
					thisADCthresh = rthreshold[thisr];

					if (temp[e] > thisADCthresh && peakpixel[e] == 0) {
						// This might be the start of a new peak - start searching
						inx[0] = i;
						iny[0] = j;
						peakpixels[0] = e;
						nat = 1;
						totI = 0;
						totIraw = 0;
						maxI = 0;
						maxIraw = 0;
						peak_com_x = 0;
						peak_com_y = 0;

						// Keep looping until the pixel count within this peak does not change
						do {

							lastnat = nat;
							// Loop through points known to be within this peak
							for (long p = 0; p < nat; p++) {
								// Loop through search pattern
								for (long k = 0; k < search_n; k++) {
									// Array bounds check
									if ((inx[p] + search_x[k]) < 0)
										continue;
									if ((inx[p] + search_x[k]) >= asic_nx)
										continue;
									if ((iny[p] + search_y[k]) < 0)
										continue;
									if ((iny[p] + search_y[k]) >= asic_ny)
										continue;

									// Neighbour point in big array
									thisx = inx[p] + search_x[k] + mi * asic_nx;
									thisy = iny[p] + search_y[k] + mj * asic_ny;
									e = thisx + thisy * pix_nx;

									//if(e < 0 || e >= pix_nn){
									//	printf("Array bounds error: e=%i\n",e);
									//	continue;
									//}

									thisr = lrint(pix_r[e]);
									thisADCthresh = rthreshold[thisr];

									// Above threshold?
									if (temp[e] > thisADCthresh && peakpixel[e] == 0 && mask[e] != 0) {
										//if(nat < 0 || nat >= global->pix_nn) {
										//	printf("Array bounds error: nat=%i\n",nat);
										//	break
										//}
										thisI = temp[e] - roffset[thisr];
										totI += thisI; // add to integrated intensity
										totIraw += temp[e];
										peak_com_x += thisI * ((float)thisx); // for center of mass x
										peak_com_y += thisI * ((float)thisy); // for center of mass y
										//temp[e] = 0; // zero out this intensity so that we don't count it again
										inx[nat] = inx[p] + search_x[k];
										iny[nat] = iny[p] + search_y[k];
										peakpixel[e] = 1;
										if (nat < hitfinderMaxPixCount)
											peakpixels[nat] = e;
										if (thisI > maxI)
											maxI = thisI;
										if (thisI > maxIraw)
											maxIraw = temp[e];

										nat++;
									}
								}
							}
						} while (lastnat != nat);


						// Too many or too few pixels means ignore this 'peak'; move on now
						if (nat<hitfinderMinPixCount || nat>hitfinderMaxPixCount) {
							continue;
						}


						/*
						 *	Calculate center of mass for this peak from initial peak search
						 */
						com_x = peak_com_x / fabs(totI);
						com_y = peak_com_y / fabs(totI);
						com_e = lrint(com_x) + lrint(com_y) * pix_nx;

						long   com_xi = lrint(com_x) - mi * asic_nx;
						long   com_yi = lrint(com_y) - mj * asic_ny;


						/*
						 *	Calculate the local signal-to-noise ratio and local background in an annulus around this peak
						 *	(excluding pixels which look like they might be part of another peak)
						 */
						float   localSigma = 0;
						float   localOffset = 0;
						long    ringWidth = 2 * hitfinderLocalBGRadius;

						float   sumI = 0;
						float   sumIsquared = 0;
						long    np_sigma = 0;
						long	np_counted = 0;
						float	fbgr;
						float	backgroundMaxI = 0;
						float	fBackgroundThresh = 0;

						for (long bj = -ringWidth; bj < ringWidth; bj++) {
							for (long bi = -ringWidth; bi < ringWidth; bi++) {

								// Within-ASIC check
								if ((com_xi + bi) < 0)
									continue;
								if ((com_xi + bi) >= asic_nx)
									continue;
								if ((com_yi + bj) < 0)
									continue;
								if ((com_yi + bj) >= asic_ny)
									continue;

								// Within outer ring check
								fbgr = sqrt(bi * bi + bj * bj);
								if (fbgr > ringWidth)// || fbgr <= hitfinderLocalBGRadius )				// || fbgr > hitfinderLocalBGRadius)
									continue;

								// Position of this point in data stream
								thisx = com_xi + bi + mi * asic_nx;
								thisy = com_yi + bj + mj * asic_ny;
								e = thisx + thisy * pix_nx;

								thisr = lrint(pix_r[e]);
								thisADCthresh = rthreshold[thisr];

								// Intensity above background
								thisI = temp[e];


								// If above ADC threshold, this could be part of another peak
								//if (temp[e] > thisADCthresh)
								//	continue;

								// Keep track of value and value-squared for offset and sigma calculation
								// if(peakpixel[e] == 0 && mask[e] != 0) {
								if (temp[e] < thisADCthresh && peakpixel[e] == 0 && mask[e] != 0) {
									np_sigma++;
									sumI += thisI;
									sumIsquared += (thisI * thisI);
									if (thisI > backgroundMaxI) {
										backgroundMaxI = thisI;
									}
								}
								np_counted += 1;
							}
						}

						// Calculate local background and standard deviation
						if (np_sigma != 0) {
							localOffset = sumI / np_sigma;
							localSigma = sqrt(sumIsquared / np_sigma - ((sumI / np_sigma) * (sumI / np_sigma)));
						}
						else {
							localOffset = roffset[lrint(pix_r[lrint(com_e)])];
							localSigma = 0.01;
						}


						/*
						 *	Re-integrate (and re-centroid) peak using local background estimates
						 */
						totI = 0;
						totIraw = 0;
						maxI = 0;
						maxIraw = 0;
						peak_com_x = 0;
						peak_com_y = 0;
						for (long counter = 1; counter < nat && counter <= hitfinderMaxPixCount; counter++) {
							e = peakpixels[counter];
							thisIraw = temp[e];
							thisI = thisIraw - localOffset;

							totI += thisI;
							totIraw += thisIraw;

							// Remember that e = thisx + thisy*pix_nx;
							ldiv_t xy = ldiv(e, pix_nx);
							thisx = xy.rem;
							thisy = xy.quot;
							peak_com_x += thisI * ((float)thisx); // for center of mass x
							peak_com_y += thisI * ((float)thisy); // for center of mass y

							if (thisIraw > maxIraw)
								maxIraw = thisIraw;
							if (thisI > maxI)
								maxI = thisI;
						}
						com_x = peak_com_x / fabs(totI);
						com_y = peak_com_y / fabs(totI);
						com_e = lrint(com_x) + lrint(com_y) * pix_nx;



						/*
						 *	Calculate signal-to-noise and apply SNR criteria
						 */
						snr = (float)(totI) / localSigma;
						//snr = (float) (maxI)/localSigma;
						//snr = (float) (totIraw-nat*localOffset)/localSigma;
						//snr = (float) (maxIraw-localOffset)/localSigma;

						// The more pixels there are in the peak, the more relaxed we are about this criterion
						if (snr < hitfinderMinSNR)        //   - nat +hitfinderMinPixCount
							continue;

						// Is the maximum intensity in the peak enough above intensity in background region to be a peak and not noise?
						// The more pixels there are in the peak, the more relaxed we are about this criterion
						//fBackgroundThresh = hitfinderMinSNR - nat;
						//if(fBackgroundThresh > 4) fBackgroundThresh = 4;
						fBackgroundThresh = 1;
						fBackgroundThresh *= (backgroundMaxI - localOffset);
						if (maxI < fBackgroundThresh)
							continue;


						// This is a peak? If so, add info to peak list
						if (nat >= hitfinderMinPixCount && nat <= hitfinderMaxPixCount) {

							// This CAN happen!
							if (totI == 0)
								continue;

							//com_x = peak_com_x/fabs(totI);
							//com_y = peak_com_y/fabs(totI);

							e = lrint(com_x) + lrint(com_y) * pix_nx;
							if (e < 0 || e >= pix_nn) {
								printf("Array bounds error: e=%ld\n", e);
								continue;
							}

							// Remember peak information
							if (peakCounter < hitfinderNpeaksMax) {
								peaklist->peakNpix += nat;
								peaklist->peakTotal += totI;
								peaklist->peak_com_index[peakCounter] = e;
								peaklist->peak_npix[peakCounter] = nat;
								peaklist->peak_com_x[peakCounter] = com_x;
								peaklist->peak_com_y[peakCounter] = com_y;
								peaklist->peak_totalintensity[peakCounter] = totI;
								peaklist->peak_maxintensity[peakCounter] = maxI;
								peaklist->peak_sigma[peakCounter] = localSigma;
								peaklist->peak_snr[peakCounter] = snr;
								peakCounter++;
								peaklist->nPeaks = peakCounter;
							}
							else {
								peakCounter++;
							}
						}
					}
				}
			}
		}
	}

	//free(temp);
	//free(inx);
	//free(iny);
	//free(peakpixel);
	//free(peakpixels);


	//free(roffset);
	//free(rsigma);
	//free(rcount);
	//free(rthreshold);


	peaklist->nPeaks = peakCounter;
	return(peaklist->nPeaks);
	/*************************************************/

}





/*
 *	Peak finder 6
 *	Rick Kirian
 --

int peakfinder6(tPeakList *peaklist, float *data, char *mask, long asic_nx, long asic_ny, long nasics_x, long nasics_y, float ADCthresh, float hitfinderMinSNR, long hitfinderMinPixCount, long hitfinderMaxPixCount, long hitfinderLocalBGRadius, float hitfinderMinPeakSeparation) {

	// Derived values
	long	pix_nx = asic_nx*nasics_x;
	long	pix_ny = asic_ny*nasics_y;
	long	pix_nn = pix_nx*pix_ny;
	long	hitfinderNpeaksMax = peaklist->nPeaks_max;

	peaklist->nPeaks = 0;
	peaklist->peakNpix = 0;
	peaklist->peakTotal = 0;

	// Local variables
	int counter = 0;
	int hit = 0;
	int fail;
	int stride = pix_nx;
	int i,fs,ss,e,thise,p,ce,ne,nat,lastnat,cs,cf;
	int peakindex,newpeak;
	float dist, itot, ftot, stot, maxI;
	float thisI,snr,bg,bgsig;
	float minPeakSepSq =  hitfinderMinPeakSeparation*hitfinderMinPeakSeparation;
	uint16_t	combined_pixel_options = PIXEL_IS_IN_PEAKMASK|PIXEL_IS_BAD|PIXEL_IS_HOT|PIXEL_IS_BAD|PIXEL_IS_SATURATED|PIXEL_IS_OUT_OF_RESOLUTION_LIMITS;
	nat = 0;
	lastnat = 0;
	maxI = 0;

	// For counting neighbor pixels
	int *nexte = (int *) calloc(pix_nn,sizeof(int));
	int *killpeak = (int *) calloc(pix_nn,sizeof(int));
	for (i=0; i<pix_nn; i++){
		killpeak[i] = 0;
	}

	int * natmask = (int *) calloc(pix_nn, sizeof(int) );
	for(long i=0; i<pix_nn; i++){
		natmask[i] = mask[i];
	}

	// Shift in linear indices to eight nearest neighbors
	int shift[8] = { +1, -1, +stride, -stride,
					 +stride - 1, +stride + 1,
					 -stride - 1, -stride + 1};



	//	Create a buffer for image data so we don't nuke the main image by mistake

	float *temp = (float*) calloc(pix_nn, sizeof(float));
	memcpy(temp, data, pix_nn*sizeof(float));


	//	Apply mask (multiply data by 0 to ignore regions - this makes data below threshold for peak finding)
	for(long i=0;i<pix_nn;i++){
		temp[i] *= mask[i];
	}

	// Loop over modules (8x8 array)
	for(long mj=0; mj<nasics_y; mj++){
		for(long mi=0; mi<nasics_x; mi++){

			// Some day, the local background radius may be different
			// for each panel.  Could even be specified for each pixel
			// when detector geometry is determined
			int bgrad = hitfinderLocalBGRadius;

			int asic_min_fs = mi*asic_nx;
			int asic_min_ss = mj*asic_ny;

			int padding = bgrad + hitfinderLocalBGRadius - 1;

			// Loop over pixels within a module
			for(long j=padding; j<asic_ny-1-padding; j++){
				for(long i=padding; i<asic_nx-1-padding; i++){

					ss = asic_min_ss + j;
					fs = asic_min_fs + i;
					e = ss*stride + fs;

					// Check simple intensity threshold first
					if ( temp[e] < ADCthresh ) continue;

					// Check if this pixel value is larger than all of its neighbors
					for ( int k=0; k<8; k++ ) if ( temp[e] <= temp[e+shift[k]] ) continue;

					// get SNR for this pixel
					fail = box_snr(temp, mask, e, bgrad, hitfinderLocalBGRadius, stride, &snr, &bg, &bgsig);
					if ( fail ) continue;
					// Check SNR threshold
					if ( snr < hitfinderMinSNR ) continue;

					// Count the number of connected pixels and centroid
					nat = 1;
					nexte[0] = e;
					ce = 0;
					maxI = 0;
					itot = temp[e] - bg;
					cf = e % stride;
					cs = e / stride;
					ftot = itot*(float)cf;
					stot = itot*(float)cs;
					do {
						lastnat = nat;
						thise = nexte[ce];
						// if ( natmask[thise] == 0 ) goto skipme;
						for ( int k=0; k<8; k++ ) {
							// this is the index of a neighboring pixel
							ne = thise + shift[k];
							// Array bounds check
							if ( ne < 0 || ne >= pix_nn ) continue;
							// Check that we aren't recounting the same pixel
							if ( natmask[ne] == 0 ) continue;
							// Check SNR condition
							if ( (temp[ne]-bg)/bgsig > hitfinderMinSNR ) {
								natmask[ne] = 0; // Mask this pixel (don't count it again)
								nexte[nat] = ne; // Queue this location to search it's neighbors later
								nat++; // Increment the number of connected pixels
								// Track some info needed for rough center of mass:
								thisI = temp[ne] - bg;
								itot += thisI;
								cf = ne % stride;
								cs = ne / stride;
								if (thisI > maxI)
									maxI = thisI;

								ftot += thisI*(float)cf;
								stot += thisI*(float)cs;
							}
						}
						ce++;
					} while ( nat != lastnat );

					// Final check that we satisfied the connected pixel requirement
					if ( nat < hitfinderMinPixCount || nat > hitfinderMaxPixCount ) continue;

					// Approximate center of mass
					fs = lrint(ftot/itot);
					ss = lrint(stot/itot);
					// Have we already found better peak nearby?
					newpeak = 1;
					peakindex = counter;
					for ( p=counter-1; p >= 0; p-- ) {
						// Distance to neighbor peak
						dist = pow(fs - peaklist->peak_com_x[p],2) +
							pow(ss - peaklist->peak_com_y[p], 2);
						if ( dist <= minPeakSepSq ) {
							if ( snr > peaklist->peak_snr[p]) {
								// This peak will overtake its neighbor
								newpeak = 0;
								peakindex = p;
								continue;
							} else {
								// There is a better peak nearby
								goto skipme;
							}
						}
					}

					// Now find proper centroid
					itot = 0; ftot = 0; stot = 0;
					for ( cs=ss-bgrad; cs<=ss+bgrad; cs++) {
						for ( cf=fs-bgrad; cf<=fs+bgrad; cf++) {
							ce = cs*stride + cf;
							if ( ce < 0 || ce > pix_nn ) continue;
							if ( isAnyOfBitOptionsSet(mask[ce],combined_pixel_options) ) continue;
							thisI = temp[ce] - bg;
							itot += thisI;
							ftot += thisI*(float)cf;
							stot += thisI*(float)cs;
						}
					}

					// Remember peak information
					if (counter < hitfinderNpeaksMax) {

						peaklist->peakNpix += nat;
						peaklist->peakTotal += itot;

						peaklist->peak_totalintensity[peakindex] = itot;
						peaklist->peak_npix[peakindex] = nat;
						peaklist->peak_com_x[peakindex] = ftot/itot;
						peaklist->peak_com_y[peakindex] = stot/itot;
						peaklist->peak_maxintensity[peakindex] = maxI;
						peaklist->peak_snr[peakindex] =snr;
						peaklist->peak_com_index[peakindex] = e;
						peaklist->nPeaks = counter+1;
						//peaklist->peak_com_x_assembled[counter] = global->detector[detIndex].pix_x[e];
						//peaklist->peak_com_y_assembled[counter] = global->detector[detIndex].pix_y[e];
						//peaklist->peak_com_r_assembled[counter] = global->detector[detIndex].pix_r[e];
					}
					if ( newpeak )
						counter++;

					// Have we found too many peaks?
					if ( counter >= hitfinderNpeaksMax ) {
						peaklist->nPeaks = hitfinderNpeaksMax;
						printf("MESSAGE: Found too many peaks - aborting peaksearch early.\n");
						hit = 0;
						goto nohit;
					}

				skipme:;

				}
			}
		}
	}


nohit:

	free(nexte);
	free(natmask);
	free(temp);
	free(killpeak);

	return(peaklist->nPeaks);
}


*/

/* Calculate signal-to-noise ratio for the central pixel, using a square
 * concentric annulus */

int CCheetah_PeakFinder::box_snr(float* im, char* mask, int center, int radius, int thickness,
	int stride, float* SNR, float* background, float* backgroundSigma)
{

	int i, q, a, b, c, d, thisradius;
	int bgcount;
	float bg, bgsq, bgsig, snr;

	bg = 0;
	bgsq = 0;
	bgcount = 0;

	/* Number of pixels in the square annulus */
	int inpix = 2 * (radius - 1) + 1;
	inpix = inpix * inpix;
	int outpix = 2 * (radius + thickness - 1) + 1;
	outpix = outpix * outpix;
	int maxpix = outpix - inpix;

	/* Loop over pixels in the annulus */
	for (i = 0; i < thickness; i++) {

		thisradius = radius + i;

		/* The starting indices at each corner of a square ring */
		int topstart = center - thisradius * (1 + stride);
		int rightstart = center + thisradius * (stride - 1);
		int bottomstart = center + thisradius * (1 + stride);
		int leftstart = center + thisradius * (1 - stride);

		/* Loop over pixels in a thin square ring */
		for (q = 0; q < thisradius * 2; q++) {
			a = topstart + q * stride;
			b = rightstart + q;
			c = bottomstart - q * stride;
			d = leftstart - q;
			bgcount += mask[a];
			bgcount += mask[b];
			bgcount += mask[c];
			bgcount += mask[d];
			bg += im[a] + im[b] + im[c] + im[d];
			bgsq += im[a] * im[a] + im[b] * im[b] +
				im[c] * im[c] + im[d] * im[d];
		}
	}

	/* Assert that 50 % of pixels in the annulus are good */
	if (bgcount < 0.5 * maxpix) {
		return 1;
	};

	/* Final statistics */
	bg = bg / bgcount;
	bgsq = bgsq / bgcount;
	bgsig = sqrt(bgsq - bg * bg);
	snr = (im[center] - bg) / bgsig;

	*SNR = snr;
	*background = bg;
	*backgroundSigma = bgsig;

	return 0;

}

void CCheetah_PeakFinder::reset_variables()
{
	//ZeroMemory(m_inx, pix_nn * sizeof(long));
	//ZeroMemory(m_iny, pix_nn * sizeof(long));
	ZeroMemory(m_peakpixel, pix_nn * sizeof(char));
	//ZeroMemory(m_peakpixels, 1000 * sizeof(long));
	//ZeroMemory(m_rsigma, lmaxr * sizeof(float));
	//ZeroMemory(m_roffset, lmaxr * sizeof(float));
	//ZeroMemory(m_rcount, lmaxr * sizeof(long));
	//ZeroMemory(m_rthreshold, lmaxr * sizeof(float));
}


CCheetah_PeakFinder* CCheetah_PeakFinder::GetInstance(unsigned int rows, unsigned int cols)
{
	if (m_pCheetahPeakFinder == NULL)
	{
		m_pCheetahPeakFinder = new CCheetah_PeakFinder(rows, cols);
	}
	return m_pCheetahPeakFinder;
}

CCheetah_PeakFinder* CCheetah_PeakFinder::GetInstance()
{
	if (m_pCheetahPeakFinder)
		return m_pCheetahPeakFinder;
	else
		printf("Error: CCheetah_PeakFinder instance not created\n");
	return nullptr;
}

tPeakList* CCheetah_PeakFinder::GetPeakList()
{
	return &peaklist;
}

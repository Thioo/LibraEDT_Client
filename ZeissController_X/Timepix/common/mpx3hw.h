/*
   Copyright 2004-2009 IEAP CTU
   Author: Tomas Holy (tomas.holy@utef.cvut.cz)
*/

#pragma once

#include "common.h"

// MPX3 acq. parameters
typedef struct _Mpx3AcqParams
{
   BOOL useHwTimer;                         // hw timer is used
   BOOL polarityPositive;                   // positive polarity
   int mode;                                // acq mode (see ACQMODE_xxx in common.h)
   int acqCount;                            // acq count (for burst mode)
   double time;                             // acq time (if controlled by HW timer)
   Mpx3OpMode opMode;                       // operation mode
   Mpx3CounterDepth counterDepth;           // depth of counter 1,4,12,24 bits
   Mpx3ColumnBlock columnBlock;             // column block selection
   Mpx3RowBlock rowBlock;                   // row block selection
   Mpx3CounterSelect counterSelect;         // counter selection
   double secondCounterStartDelay;          // delay of second counter before starting counting (for SemiSeq)
   double secondCounterTime;                // acq time of second counter (for SemiSeq & CRW)
} Mpx3AcqParams;


// event types for callback
#define HWCB_ACQSTARTED             0x01    // acq started (should be sent for ACQMODE_TRIG_STARTS, ACQMODE_TRIG_EXTSHUTTER)
#define HWCB_ACQFINISHED            0x02    // acq finished (should be sent for ACQMODE_TRIG_STOPS or if hw timer is used)
#define HWCB_DEVDISCONNECTED        0x03    // device was disconnected
#define HWCB_FRAMERECEIVED          0x04    // frame received, but acq is not finished. (for Mpx3 - semi-sequan. & Continuous RW)

//typedef void (*HwCallback)(INTPTR userData, int eventType, void *data);    // callback type, when one of HWCB_XXX event occurs hw lib has to send callback (tag is provided by MpxCtrl, eventType is HWCB_XXX code, data should be 0 (no data are required for currently supported callbacks))

typedef struct _Mpx3Interface
{
    // Finds all connected Mpx3 devices, the devices may not be initialized after this call
    // before any operation hwInit() will be called 
    // [out] ids - array of IDs associated with devices found
    // [in/out] count - size of array, if array is too small, non-zero value should be returned
    // and *count should contain required size
    int (*findDevices)(int ids[], int *count);

    // initialize device associated with id
    // [in] id - device indentification
    int (*init)(int id);

    // close device (for cleanup)
    // [in] hwID - hw identification
    int (*closeDevice)(int hwID);

    // set callback for events like start/stop acq,...
    // [in] cb - callback
    int (*setCallback)(HwCallback cb);

    // sets "user data" for callback
    // [in] id - hw identification
    // [in] data - parameter for callback function, with this parameter callback has to be called
    int (*setCallbackData)(int id, INTPTR data);

    // returns number of HwInfo items for hwlibrary
    int (*getHwInfoCount)();

    // getting HwInfo item flag
    // [in] id - hw identification
    // [in] index - index of HwItem (0..getHwInfoCount()-1)
    // [out] flags - flag of item
    int (*getHwInfoFlags)(int id, int index, u32 *flags);

    // getting HwInfo item
    // [in] id - hw identification
    // [in] index - index of HwItem (0..getHwInfoCount()-1)
    // [out] hwInfo - output structure of hw item
    // [in/out] dataSize - size of allocated hwInfo->data buffer, if buffer is not big enough
    // non-zero value should be returned and dataSize should contain required size
    int (*getHwInfo)(int id, int index, HwInfoItem *hwInfo, int *dataSize);

    // setting data of HwInfo item
    // [in] id - hw identification
    // [in] index - index of HwItem (0..getHwInfoCount()-1)
    // [in] data - data for specified HwItem
    // [in] dataSize - size of data
    int (*setHwInfo)(int id, int index, void *data, int dataSize);

    // fills DevInfo structure (informations about mpx and interface capabilities)
    // [in] id - hw identification
    // [out] devInfo - "device info" structure that should be filled
    int (*getDevInfo)(int id, DevInfo *devInfo);

    // resets interface and medipix to default state
    // [in] id - hw identification
    int (*reset)(int id);

    // sets Mpx3 DACs
    // [in] id - hw identification
    // [in] dacVals - array of DAC values (number of chips)*(number of DACs), order of DACs is given by Mpx3DacsOrder enum
    // [in] size - size of dacVals array (number of items for size check)
    int (*setDACs)(int id, DACTYPE dacVals[], int size);

    // sensing DAC or other signal
    // [in] id - hw identification
    // [in] chipNumber - chip that should be sensed
    // [in] code - DAC or other signal code
    // [out] value - sensed analog value
    int (*senseSignal)(int id, int chipNumber, int code, double *value);

    // [in] id - hw identification
    // [in] code - code of DAC that should be replaced, negative code if ext. DAC should be disabled
    // [in] value - value in Volts (0-1V)
    int (*setExtDAC)(int id, int code, double value);

    // set pixels mask
    // [in] id - hw identification
    // [in] cfgs - array [number of chips * MATRIX_SIZE] of pix. configurations bits
    // [in] size - size of cfgs array (number of items for size check)
    int (*setPixelsCfg)(int id, Mpx3PixCfg cfgs[], u32 size);

    // sets acq. parameters 
    // [in] id - hw identification
    // [in] pars - acquisition parameters/settings
    int (*setAcqPars)(int id, Mpx3AcqParams *pars);

    // starts aquisition
    // [in] id - hw identification
    int (*startAcquisition)(int id);

    // stops aquisition
    // [in] id - hw identification
    int (*stopAcquisition)(int id);

    // gets time of last acquisition
    // [in] id - hw identification
    // [out] time - acq. time (time has to be measured even for "manual" mode)
    int (*getAcqTime)(int id, double *time);

    // resets matrix
    // [in] id - hw identification
    int (*resetMatrix)(int id);

    // matrix readout (deserialized)
    // if both counters are used (separately) first counter should be in low word, second in high word
    // [in] id - hw identification
    // [out] buff - output buffer, size of buffer has to be at least (MATRIX_SIZE*numberOfChips)
    // [in] buffSize - size of buff (number of items for size check)
    int (*readMatrix)(int id, u32 *buff, u32 buffSize);

    // write matrix to chip counters (24 bits per pixel)
    // [in] id - hw identification
    // [in] buff - matrix that should be written, size of buffer has to be at least (MATRIX_SIZE*numberOfChips)
    // [in] buffSize - size of buff (number of items for size check)
    int (*writeMatrix)(int id, u32 *buff, u32 buffSize);

    // [in] id - hw identification
    // [in] charge - charge injected by [TPA, TPB]
    // [in] period - period of pulses [s] (distance between pulses is same as length i.e. period/2)
    // [in] pulseCount - number of pulses to send
    // [in] ctpr - column test pulse register
    int (*sendTestPulses)(int id, double charge[2], double period, u32 pulseCount, u32 ctpr[8]);

    // check if device is busy
    // [in] id - hw identification
    // [out] busy - set TRUE if busy
    int (*isBusy)(int id, BOOL *busy);

    // return string describing last error
    const char* (*getLastError)();

    // return string describing last error for specified HW
    // [in] id - hw identification
    const char* (*getLastDevError)(int id);

} Mpx3Interface;

// for runtime linking of HW library call function getMpx3Interface() of type GetMpx3Interface
// to obtain table of functions which every HW library has to provide
typedef Mpx3Interface* (*GetMpx3Interface)();


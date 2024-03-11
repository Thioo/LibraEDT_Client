#pragma once

// general header file for HW libraries (for run-time linking)
// How to implement HW library:
// Each HW library has to export function with name getMandatoryFuncs() of type GetMandatoryFuncsType.
// This function has to return structure containing pointers to functions which form interface between
// MpxCtrl layer and hw library. Each function in this structure has to be implemented.
// MpxCtrl uses hwGetFirst/hwGetNext functions to obtain all devices which are connected via hwlib-type interfaces.
// Each device is identified via unique hwID (which is generated/selected by hwlib).
// Interface-specific settings are handled by hwGetHwInfoCount, hwGetHwInfoFlags, hwGetHwInfo, hwSetHwInfo functions.
// HwInfoItems are used to make accessible interface-specific settings. In HwInfoItem structure
// is "complete" description of custom data attribute (data type, name, size,...) With this information
// MpxCtrl layer and layers above can make accessible these information to user. The usage of these data can
// be controlled via MPX_HWINFO_XXX flags.
// All operations (except acquisition hwStartAcquisition) has to be blocking (function should return after operation is finished).
// hwStartAcquisition is non-blocking, it should return even if acq. is not started (e.g. if acq. is started by trigger)
// (acq. thread waits in MpxCtrl layer - it measures time (if in manual PC timer mode), checks busy state (manual HW timer)
// or waits for callbacks from hwlib (trigger starts, stops acq,...)
// All functions should return 0 (MPXERR_NOERROR) on success and negative value on error


#include "common.h"

// some useful constants
#define BITS_PER_CHIP           917512      // number of bits for one chip readout (256*256*14 + 8)
#define BITS_PER_CHIP_MXR       917768      // number of bits for one chip readout (256*256*14 + 8 + 256)
#define BYTES_PER_CHIP          114689      // number of bytes for one chip readout
#define BYTES_PER_CHIP_MXR     (114689+32)  // number of bytes for one chip readout
#define CLOCK_RESET             28680       // number of cycle clock to perform reset
#define CLOCK_RESET_MXR         28688       // number of cycle clock to perform reset (8preload+256x8x14+8postload)
#define DAC_COUNT               13          // number of DACs Mpx 2.1
#define DAC_COUNT_MXR           15          // number of DACs Mpx MXR
#define DAC_COUNT_TPX           14          // number of DACs Timepix

// stream buffer size (in i16) for n chips
#define I16STREAM_SIZE(n)       (((n * BYTES_PER_CHIP + 1) / 2) + 1)    // +1 for quad
#define I16STREAM_SIZE_MXR(n)   ((n * BYTES_PER_CHIP_MXR + 1) / 2)

// event types for callback
#define HWCB_ACQSTARTED             0x01    // acq started (should be sent for ACQMODE_TRIG_STARTS, ACQMODE_TRIG_EXTSHUTTER)
#define HWCB_ACQFINISHED            0x02    // acq finished (should be sent for ACQMODE_TRIG_STOPS or if hw timer is used)
#define HWCB_DEVDISCONNECTED        0x03    // device was disconnected

typedef struct _AcqParams
{
    BOOL useHwTimer;                    // hw timer is used
    BOOL enableCst;                     // enable Charge Sharing Test
    BOOL polarityPositive;              // positive polarity
    int mode;                           // acq mode (see ACQMODE_xxx in common.h)
    int acqCount;                       // acq count (>1 for burst mode)
    double time;                        // acq time (if controlled by HW timer)
} AcqParams;

typedef void (*HwCallback)(INTPTR userData, int eventType, void *data);    // callback type, when one of HWCB_XXX event occurs hw lib has to send callback (tag is provided by MpxCtrl, eventType is HWCB_XXX code, data should be 0 (no data are required for currently supported callbacks))

typedef struct _Mpx2Interface
{
  // Finds all connected Mpx2 devices, the devices may not be initialized after this call
  // before any operation hwInit() will be called 
  // [out] ids - array of IDs associated with devices found
  // [in/out] count - size of array, after return *count should contain number of found devices
  // if array is too small, non-zero value should be returned and *count should contain required size
  int (*findDevices)(int ids[], int *count);

  // initialize device associated with hwID
  // [in] hwID - device indentification
  int (*init)(int hwID);

  // close device (for cleanup)
  // [in] hwID - hw identification
  int (*closeDevice)(int hwID);

  // registers callback
  // [in] cb - callback
  int (*setCallback)(HwCallback cb);

  // sets "callback data" for device
  // [in] hwID - hw identification
  // [in] data - parameter for callback function, with this parameter callback has to be called
  int (*setCallbackData)(int hwID, INTPTR data);

  // returns number of HwInfo items for hwlibrary
  int (*getHwInfoCount)();

  // getting HwInfo item flag
  // [in] hwID - hw identification
  // [in] index - index of HwItem (0..HwGetHwInfoCountType()-1)
  // [out] flags - flag of item
  int (*getHwInfoFlags)(int hwID, int index, u32 *flags);

  // getting HwInfo item
  // [in] hwID - hw identification
  // [in] index - index of HwItem (0..HwGetHwInfoCountType()-1)
  // [out] hwInfo - output structure of hw item
  // [in/out] dataSize - size of allocated hwInfo->data buffer, if buffer is not big enough dataSize will contain required size
  int (*getHwInfo)(int hwID, int index, HwInfoItem *hwInfo, int *dataSize);

  // setting data of HwInfo item
  // [in] hwID - hw identification
  // [in] index - index of HwItem (0..HwGetHwInfoCountType()-1)
  // [in] data - data for specified HwItem
  // [in] dataSize - size of data
  int (*setHwInfo)(int hwID, int index, void *data, int dataSize);

  // fills DevInfo structure (informations about mpx and interface capabilities)
  // [in] hwID - hw identification
  // [out] devInfo - "device info" structure that should be filled
  int (*getDevInfo)(int hwID, DevInfo *devInfo);

  // resets interface and medipix to default state
  // [in] hwID - hw identification
  int (*reset)(int hwID);

  // setting MPX DACs
  // [in] hwID - hw identification
  // [in] dacVals - array of DAC values (number of chips)*(number of DACs), order of DACs is given by enum type DACS_ORDER
  // [in] size - size of dacVals array (number of items for size check)
  // [in] senseChip - 8bit mask for enabling/disabling preparation of ADC conversion of DAC selected in codes array
  // [in] extDacChip - 8bit mask for enabling/disabling usage of external DAC for DAC selected in codes array
  // [in] codes - is [number of chip] array which contains DAC codes for individual chips for ADC conversion/external DAC selection
  // [in] tpReg - setting test pulse register in MXR, ignored for original chip
  int (*setDACs)(int hwID, DACTYPE dacVals[], int size, byte senseChip, byte extDacChip, int codes[], u32 tpReg);

  // reading (sensing) analog value of DAC, DAC which is read is set by DAC code in codes array in HwSetMpxDACsType
  // [in] hwID - hw identification
  // [in] chipNumber - chip that should be sensed
  // [out] value - sensed analog value
  int (*getMpxDacVal)(int hwID, int chipNumber, double *value);

  // [in] hwID - hw identification
  // [in] value - value in Volts
  int (*setExtDacVal)(int hwID, double value);

  // set pixels mask
  // [in] hwID - hw identification
  // [in] cfgs - array [number of chips * MATRIX_SIZE] of pix. configurations bits
  // [in] size - size of cfgs array (number of items for size check)
  int (*setPixelsCfg)(int hwID, byte cfgs[], u32 size);

  // sets acq. parameters
  // [in] hwID - hw identification
  // [in] pars - acquisition parameters/settings
  int (*setAcqPars)(int hwID, AcqParams *pars);

  // starts aquisition
  // [in] hwID - hw identification
  int (*startAcquisition)(int hwID);

  // stops aquisition
  // [in] hwID - hw identification
  int (*stopAcquisition)(int hwID);

  // gets time of last acquisition
  // [in] hwID - hw identification
  // [out] time - acq. time (time has to be measured even for "manual" mode)
  int (*getAcqTime)(int hwID, double *time);

  // reset matrix
  // [in] hwID - hw identification
  int (*resetMatrix)(int hwID);

  // matrix readout (deserialized and converted from pseudo)
  // [in] hwID - hw identification
  // [out] buff - output buffer, size of buffer has to be at least (MATRIX_SIZE*numberOfChips)
  // [in] buffSize - size of buff (number of items for size check)
  int (*readMatrix)(int hwID, i16 *buff, u32 buffSize);

  // write matrix to medipix (for each chip MATRIX_SIZE matrix)
  // [in] hwID - hw identification
  // [in] buff - matrix that should be written, size of buffer has to be at least (MATRIX_SIZE*numberOfChips)
  // [in] buffSize - size of buff (number of items for size check)
  int (*writeMatrix)(int hwID, i16 *buff, u32 buffSize);

  // [in] hwID - hw identification
  // [in] pulseHeight - pulse height [V]
  // [in] period - period of pulses [s] (distance between pulses is same as length i.e. period/2)
  // [in] pulseCount - number of pulses to send
  int (*sendTestPulses)(int hwID, double pulseHeight, double period, u32 pulseCount);

  // check if device is busy
  // [in] hwID - hw identification
  // [out] busy - set TRUE if busy
  int (*isBusy)(int hwID, BOOL *busy);

  // return string describing last error
  const char* (*getLastError)();

  // return string describing last error for specified HW
  // [in] hwID - hw identification
  const char* (*getLastDevError)(int hwID);
    
  // raw matrix readout
  // [in] hwID - hw identification
  // [out] buff - output buffer, size of buffer has to be at least (BITS_PER_CHIP/8*numberOfChips)
  // [in] buffSize - size of buff (number of items for size check)
  int (*readMatrixRaw)(int hwId, u8 *data, u32 buffSize);
} Mpx2Interface;

// for runtime linking of HW library call function getMpx2Interface() of type GetMpx2Interface
// to obtain table of functions which every HW library has to provide
typedef Mpx2Interface* (*GetMpx2Interface)();

// DAC codes for medipix2.x
typedef enum _DACS_CODES
{
    DELAYN_CODE = 1,
    DISC_CODE = 2,
    PREAMP_CODE = 3,
    SETDISC_CODE = 4,
    THS_CODE = 5,
    IKRUMHALF_CODE = 8,
    IKRUM_CODE = 7,
    ABUFFER_CODE = 14,
    VTHH_CODE = 12,
    VTHL_CODE = 11,
    VFBK_CODE = 10,
    VGND_CODE = 13,
    BIASLVDSTX_CODE = 6,
    REFLVDSTX_CODE = 9,
} DACS_CODES;

// DAC codes for MXR
typedef enum _DACS_CODES_MXR
{
    MXR_THS_CODE = 1,
    MXR_BIASLVDS_CODE = 2,
    MXR_BUFFA_CODE = 3,
    MXR_BUFFB_CODE = 4,
    MXR_DELAYN_CODE = 9,
    MXR_THL_CODE = 6,
    MXR_PREAMP_CODE = 7,
    MXR_FBK_CODE = 10,
    MXR_DISC_CODE = 11,
    MXR_THH_CODE = 12,
    MXR_GND_CODE = 13,
    MXR_REFLVDS_CODE = 14,
    MXR_IKRUM_CODE = 15,
} DACS_CODES_MXR;

// DAC codes for TPX
typedef enum _DACS_CODES_TPX
{
    TPX_THS_CODE = 1,
    TPX_BIASLVDS_CODE = 2,
    TPX_BUFFA_CODE = 3,
    TPX_BUFFB_CODE = 4,
    TPX_HIST_CODE = 9,
    TPX_THL_CODE = 6,
    TPX_PREAMP_CODE = 7,
    TPX_FBK_CODE = 10,
    TPX_DISC_CODE = 11,
    TPX_VCAS_CODE = 12,
    TPX_GND_CODE = 13,
    TPX_REFLVDS_CODE = 14,
    TPX_IKRUM_CODE = 15,
} DACS_CODES_TPX;


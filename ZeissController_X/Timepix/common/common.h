/*
   Copyright 2004-2009 IEAP CTU
   Author: Tomas Holy (tomas.holy@utef.cvut.cz)
*/

#pragma once

#ifndef COMMON_H
#define COMMON_H

// common data types
//

#include "asitypes.h"

//#ifdef WIN32
#if defined(_WINDLL) || defined(WIN32)

#include <windows.h>

typedef INT_PTR INTPTR; // integral type, safe for storing pointer (32/64 bit)
#define PATH_SEPAR       '\\'
#define PATH_SEPAR_STR   "\\"
typedef int socklen_t;


#else // Linux

typedef int*   INTPTR;
typedef void* HMODULE;
typedef int   SOCKET;

#define INVALID_SOCKET   -1
#define MAXDWORD         0xffffffff
#define PATH_SEPAR       '/'
#define PATH_SEPAR_STR   "/"

#endif

// variables types identifiers
typedef enum _Data_Types
{
    TYPE_BOOL   = 0,        // C bool value (int)
    TYPE_CHAR   = 1,        // signed char
    TYPE_UCHAR  = 2,        // unsigned char
    TYPE_BYTE   = 3,        // byte (unsigned char)
    TYPE_I16    = 4,        // signed short
    TYPE_U16    = 5,        // unsigned short
    TYPE_I32    = 6,        // int
    TYPE_U32    = 7,        // unsigned int
    TYPE_FLOAT  = 8,        // float
    TYPE_DOUBLE = 9,        // double
    TYPE_STRING = 10,       // zero terminated string
    TYPE_LAST   = 11,       // border
} Data_Types;

// structure describing pixel configuration for Mpx 2.1 and MXR
typedef struct _PixelCfg
{
    byte maskBit: 1;        // mask bit (1 bit, low (0) is ACTIVE)
    byte testBit: 1;        // test bit (1 bit, low (0) is ACTIVE)
    byte lowTh:   3;        // low threshold (3 bits, low (0) is ACTIVE)
    byte highTh:  3;        // high threshold (3 bits, low (0) is ACTIVE)
} PixelCfg;

// structure describing pixel configuration for TimePix
typedef struct _TpxPixCfg
{
    byte maskBit: 1;        // mask bit (1 bit, low (0) is ACTIVE)
    byte testBit: 1;        // test bit (1 bit, low (0) is ACTIVE)
    byte thlAdj:  4;        // threshold adjustment
    byte mode:    2;        // pixel mode = {p0, p1}; mode: 0 - medipix,
                            // 1 - TOT, 2 - Timepix 1-hit, 3 - Timepix
} TpxPixCfg;


typedef struct
{
    byte maskBit:  1;       // 0 masked
    byte testBit:  1;       // 0 active for TP
    byte gainMode: 1;
    byte thlAdj:   5;
    byte thhAdj:   5;
    byte dummy:    3;
} Mpx3PixCfg;

// structure describing "custom" item
typedef struct
{
    Data_Types type;        // variable type
    u32 count;              // array size
    u32 flags;              // combination of flags
                            // (e.g. for HwInfoItem MPX_HWINFO_* flags)
    const char *name;       // name of item
    const char *descr;      // item description
    void *data;             // pointer to variable of described type
} ItemInfo;

// structure describing one variable in HW
typedef ItemInfo HwInfoItem;

#define ITEMINFO_CANCHANGE      0x0002 // value can be changed

#define MPX_HWINFO_CFGSAVE      0x0001 // value will be stored in config file
#define MPX_HWINFO_CANCHANGE    0x0002 // value can be changed
#define MPX_HWINFO_MPXFRAME     0x0004 // value should be stored for each frame

// macros for type conversions
#define LOWU16(l)           ((u16)((u32)(l) & 0xffff))
#define HIU16(l)            ((u16)((u32)(l) >> 16))
#define MAKEU32(a, b)       ((u32)(((u16)(a)) | (((u32)((u16)(b))) << 16)))

#define MPXCTRL_MAX_MSGLENGTH   4096  // max length of error message
                                      // for some mpxCtrl functions
#define MPX_MAX_MSGLENGTH       4096  // max length of error message
#define MPX_MAX_PATH            512   // max length of path for file handling
#define MPX_MAX_CHBID           64    // max length of chipboard ID
#define MPX_MAX_IFACENAME       64    // max length of ifaceName name
//#define MATRIX_SIZE            65536// chip matrix size

#ifndef FALSE
#define FALSE                   0    // C bool values
#endif
#ifndef TRUE
#define TRUE                    1
#endif

// order of the Medipix DACs in array DACTYPE[] for mpxCtrlSetDACs
typedef enum _DACS_ORDER
{
    MPX_DELAYN     = 0,
    MPX_DISC       = 1,
    MPX_PREAMP     = 2,
    MPX_SETDISC    = 3,
    MPX_THS        = 4,
    MPX_IKRUM      = 5,
    MPX_ABUFFER    = 6,
    MPX_VTHH       = 7,
    MPX_VTHL       = 8,
    MPX_VFBK       = 9,
    MPX_VGND       = 10,
    MPX_BIASLVDSTX = 11,
    MPX_REFLVDSTX  = 12,
    MPX_IKRUMHALF  = 13,
} DACS_ORDER;

// order of the Medipix MXR DACs in array DACTYPE[] for mpxCtrlSetDACs
typedef enum _DACS_ORDER_MXR
{
    MXR_IKRUM     = 0,
    MXR_DISC      = 1,
    MXR_PREAMP    = 2,
    MXR_BUFFA     = 3,
    MXR_BUFFB     = 4,
    MXR_DELAYN    = 5,
    MXR_THLFINE   = 6,
    MXR_THLCOARSE = 7,
    MXR_THHFINE   = 8,
    MXR_THHCOARSE = 9,   
    MXR_FBK       = 10,
    MXR_GND       = 11,
    MXR_THS       = 12,
    MXR_BIASLVDS  = 13,
    MXR_REFLVDS   = 14,
} DACS_ORDER_MXR;

// order of the Medipix TPX DACs in array DACTYPE[] for mpxCtrlSetDACs
typedef enum _DACS_ORDER_TPX
{
    TPX_IKRUM     = 0,
    TPX_DISC      = 1,
    TPX_PREAMP    = 2,
    TPX_BUFFA     = 3,
    TPX_BUFFB     = 4,
    TPX_HIST      = 5,
    TPX_THLFINE   = 6,
    TPX_THLCOARSE = 7,
    TPX_VCAS      = 8,
    TPX_FBK       = 9,
    TPX_GND       = 10,
    TPX_THS       = 11,
    TPX_BIASLVDS  = 12,
    TPX_REFLVDS   = 13,
} DACS_ORDER_TPX;


typedef enum
{
    MPX3_TH0         = 0,
    MPX3_TH1         = 1,
    MPX3_TH2         = 2,
    MPX3_TH3         = 3,
    MPX3_TH4         = 4,
    MPX3_TH5         = 5,
    MPX3_TH6         = 6,
    MPX3_TH7         = 7,
    MPX3_PREAMP      = 8,
    MPX3_IKRUM       = 9,
    MPX3_SHAPER      = 10,
    MPX3_DISC        = 11,
    MPX3_DISC_LS     = 12,
    MPX3_TH_N        = 13,
    MPX3_DAC_PIXEL   = 14,
    MPX3_DELAY       = 15,
    MPX3_TP_BUFF_IN  = 16,
    MPX3_TP_BUFF_OUT = 17,
    MPX3_RPZ         = 18,
    MPX3_GND         = 19,
    MPX3_TP_REF      = 20,
    MPX3_FBK         = 21,
    MPX3_CAS         = 22,
    MPX3_TP_REFA     = 23,
    MPX3_TP_REFB     = 24,
} Mpx3DacsOrder;

typedef enum
{
   MPX3_COUNTER_1B  = 0,   // 1 bit counter depth
   MPX3_COUNTER_4B  = 1,   // 4 bits counter depth
   MPX3_COUNTER_12B = 2,   // 12 bits counter depth
   MPX3_COUNTER_24B = 3,   // 24 bits counter depth
} Mpx3CounterDepth;

typedef enum
{
   SPM_FSRW     = 0,  // single pixel mode + full sequential read write
   SPM_SSRW     = 1,  // single pixel mode + semi-sequential read write
   SPM_CRW      = 2,  // single pixel mode + continuous read write
   CSM_FSRW     = 3,  // charge summing mode + full sequential read write
   CSM_SSRW     = 4,  // charge summing mode + semi-sequential read write
   CSM_CRW      = 5,  // charge summing mode + continuous read write
   CLR_SPM_FSRW = 6,  // color in single pixel mode + full sequential rd/wr
   CLR_SPM_SSRW = 7,  // color in single pixel mode + semi-sequential rd/wr
   CLR_SPM_CRW  = 8,  // color in single pixel mode + continuous rd/wr
   CLR_CSM_FSRW = 9,  // color in charge summing mode + full sequential rd/wr
   CLR_CSM_SSRW = 10, // color in charge summing mode + semi-sequential rd/wr
   CLR_CSM_CRW  = 11, // color in charge summing mode + continuous rd/wr
} Mpx3OpMode;

typedef enum {
    // all columns
    MPX3_COLBLOCK_ALL     = 0,

    // 32 columns blocks
    MPX3_COLBLOCK_000_031 = 1,
    MPX3_COLBLOCK_032_063 = 2,
    MPX3_COLBLOCK_064_095 = 3,
    MPX3_COLBLOCK_096_127 = 4,
    MPX3_COLBLOCK_128_159 = 5,
    MPX3_COLBLOCK_160_191 = 6,
    MPX3_COLBLOCK_192_223 = 7,
    MPX3_COLBLOCK_224_255 = 8,

    // 64 columns blocks
    MPX3_COLBLOCK_000_063 = 9,
    MPX3_COLBLOCK_064_127 = 10,
    MPX3_COLBLOCK_128_191 = 11,
    MPX3_COLBLOCK_192_255 = 12,

    // 128 columns blocks
    MPX3_COLBLOCK_000_127 = 13,
    MPX3_COLBLOCK_128_256 = 14,
} Mpx3ColumnBlock;

typedef enum {
    MPX3_ROWBLOCK_ALL   = 0,
    MPX3_ROWBLOCK_0_000 = 1,
    MPX3_ROWBLOCK_0_001 = 2,
    MPX3_ROWBLOCK_0_003 = 3,
    MPX3_ROWBLOCK_0_007 = 4,
    MPX3_ROWBLOCK_0_015 = 5,
    MPX3_ROWBLOCK_0_031 = 6,
    MPX3_ROWBLOCK_0_063 = 7,
    MPX3_ROWBLOCK_0_127 = 8,
} Mpx3RowBlock;

typedef enum
{
    MPX3_COUNTERSEL_1    = 0, // First counter - data will be read from first
    MPX3_COUNTERSEL_2    = 1, // Second counter - data will be read from second
    MPX3_COUNTERSEL_BOTH = 2  // Both counters - data will be read from both
} Mpx3CounterSelect;

#define MPX_ORIG                    1   // original medipix2.x
#define MPX_MXR                     2   // medipix mxr 2.0
#define MPX_TPX                     3   // timepix
#define MPX_3                       4   // medipix 3

#define TRIGGER_ACQSTART            1   // start trigger (mpxCtrlTrigger(TRIGGER_ACQSTART))
#define TRIGGER_ACQSTOP             2   // stop trigger (mpxCtrlTrigger(TRIGGER_ACQSTOP))

#define ACQMODE_ACQSTART_TIMERSTOP          0x0001    // acq. is started immediately (after startAcq call), acq. is stopped by timer (HW or PC) - "common acq"
#define ACQMODE_ACQSTART_HWTRIGSTOP         0x0002    // acq. is started immediately (after startAcq call), acq. is stopped by trigger from HW library (e.g. ext shutter monitor)
#define ACQMODE_ACQSTART_SWTRIGSTOP         0x0004    // acq. is started immediately (after startAcq call), acq. is stopped by mpxCtrlTrigger(TRIGGER_ACQSTOP) trigger to HW library (e.g. ext shutter monitor)
#define ACQMODE_HWTRIGSTART_TIMERSTOP       0x0010    // acq. is started by trigger from HW library, stopped by timer (HW or PC)
#define ACQMODE_HWTRIGSTART_HWTRIGSTOP      0x0020    // acq. is started by trigger from HW library, stopped by trigger from HW library
#define ACQMODE_SWTRIGSTART_TIMERSTOP       0x0040    // acq. is started by mpxCtrlTrigger(TRIGGER_ACQSTART), stopped by timer  (HW or PC)
#define ACQMODE_SWTRIGSTART_SWTRIGSTOP      0x0080    // acq. is started by mpxCtrlTrigger(TRIGGER_ACQSTART), stopped by mpxCtrlTrigger(TRIGGER_ACQSTOP)
#define ACQMODE_EXTSHUTTER                  0x0100    // shutter is controlled externally during acq. (e.g. independently on SW)
#define ACQMODE_SWTRIGMASTER_HWTRIGSLAVE    0x0200    // the mode of operation for triggering multiple detectors based on the trigger out of one master
#define ACQMODE_BURST                       0x1000    // burst mode enabled

typedef struct _DevInfo
{
    int pixCount;                       // total number of pixels
    int rowLen;                         // length of row in pixels (e.g 256 for single chip, 512 for quad);
    int numberOfChips;                  // number of chips
    int numberOfRows;                   // number of rows in which chips are aligned (e.g. quad has 4 chips, which are in 2 rows)
    int mpxType;                        // medipix type - MPX_ORIG, MPX_MXR, MPX_TPX
    char chipboardID[MPX_MAX_CHBID];    // id of chip/chipboard
    const char *ifaceName;              // name of interface

    u32 suppAcqModes;                   // supported acq. mode bitwise combinations of ACQMODE_xxx flags
    BOOL suppCallback;                  // callback is supported (acq. is finished, triggers)

    double clockReadout;                // clock frequency [MHz] for readout
    double clockTimepix;                // clock in frequency [MHz] for Timepix information
    // hw timer capabilities
    double timerMinVal;                 // minimum value of hw timer [s]
    double timerMaxVal;                 // maximum value of hw timer [s]
    double timerStep;                   // step of hw timer [s]

    // test pulse capabilities
    u32 maxPulseCount;                  // maximum number of pulses that can be sent
    double maxPulseHeight;              // max pulse height [V]
    double maxPulsePeriod;              // max period of pulses [s], length of pulse should be period/2

    // ext DAC capabilities
    double extDacMinV;                  // minimum external DAC voltage
    double extDacMaxV;                  // maximum external DAC voltage
    double extDacStep;                  // ext. DAC step size
} DevInfo;


typedef struct _Mpx3SpecificAcqParams
{
   Mpx3OpMode opMode;                       // operation mode
   Mpx3CounterDepth counterDepth;           // depth of counter 1,4,12,24 bits
   Mpx3ColumnBlock columnBlock;             // column block selection
   Mpx3RowBlock rowBlock;                   // row block selection
   Mpx3CounterSelect counterSelect;         // counter selection
   double secondCounterStartDelay;          // delay of second counter before starting counting (for SemiSeq)
   double secondCounterTime;                // acq time of second counter (for SemiSeq & CRW)
} Mpx3SpecificAcqParams;


// format flags for saving to file (MgrSaveFrameType or flags for perform acq. functions)
// flags FSAVE_BINARY/FSAVE_ASCII and flags FSAVE_I16/FSAVE_U32/FSAVE_DOUBLE are mutually exclusive
// if FSAVE_SPARSEX and FSAVE_SPARSEXY is not specified full matrix is saved
#define FSAVE_BINARY        0x0001      // save in binary format
#define FSAVE_ASCII         0x0002      // save in ASCII format
#define FSAVE_APPEND        0x0004      // append frame to existing file if exists (multiframe)
#define FSAVE_I16           0x0010      // save as 16bit integer
#define FSAVE_U32           0x0020      // save as unsigned 32bit integer
#define FSAVE_DOUBLE        0x0040      // save as double
#define FSAVE_NODESCFILE    0x0100      // do not save description file
#define FSAVE_SPARSEXY      0x1000      // save only nonzero position in [x y count] format
#define FSAVE_SPARSEX       0x2000      // save only nonzero position in [x count] format
#define FSAVE_NOFILE        0x8000      // frame will not be saved :)

#define sstrncpy(strDest, strSource, count)\
    (strncpy(strDest, strSource, count), (strDest)[count-1] = '\0', (strDest))

static const u32 sizeofType[TYPE_LAST] =
{
    sizeof(BOOL),
    sizeof(char),
    sizeof(unsigned char),
    sizeof(byte),
    sizeof(i16),
    sizeof(u16),
    sizeof(i32),
    sizeof(u32),
    sizeof(float),
    sizeof(double),
    sizeof(char*)
};

static const char * const nameofType[] = 
{
    "bool",
    "char",
    "uchar",
    "byte",
    "i16",
    "u16",
    "i32",
    "u32",
    "float",
    "double",
    "string",
    "uknown"
};

#endif
//COMMON_H
// extra line at the end of file because of gc++ compiler on linux


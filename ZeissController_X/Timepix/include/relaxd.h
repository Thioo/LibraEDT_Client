#ifndef RELAXD_H
#define RELAXD_H

//  -----------------------------------------------------------------
//  --  ADC parameters
//  -----------------------------------------------------------------
#define ADC_ANALOG_MAX_VALUE    3.3f // ADC max analog input voltage
#define ADC_DIGITAL_MAX_VALUE   255  // ADC max digital input voltage
#define ADC_RESOLUTION          8    // ADC resolution in number of bits

#define ADC_MPIX0_DAC_I         0
#define ADC_MPIX1_DAC_I         1
#define ADC_MPIX2_DAC_I         2
#define ADC_MPIX3_DAC_I         3
#define ADC_MPIXA_CURRENT_I     4
#define ADC_MPIXD_CURRENT_I     5
#define ADC_MPIXA_VOLTAGE_I     6
#define ADC_MPIXD_VOLTAGE_I     7

//  -----------------------------------------------------------------
//  --  DAC parameters
//  -----------------------------------------------------------------
#define DAC_ANALOG_MAX_VALUE    2.0f // DAC max analog input voltage
// (MPIXA voltage)
#define DAC_DIGITAL_MAX_VALUE   4095 // DAC max digital input voltage
#define DAC_RESOLUTION          12   // DAC resolution in number of bits

#define DAC_MPIX0_I             0
#define DAC_MPIX1_I             1
#define DAC_MPIX2_I             2
#define DAC_MPIX3_I             3
#define DAC_BIAS_ADJUST_I       4
#define DAC_VDD_ADJUST_I        5
#define DAC_TESTPULSE_A_I       6
#define DAC_TESTPULSE_B_I       7

// ----------------------------------------------------------------------------

// Medipix device parameters
#include "../common/Relaxdhw.h"
// Device (base) IP address      : 192.168.33.175
// Host/Gateway (base) IP address: 192.168.33.1
#define MPIXD_IP_ADDR           ((192<<24) | (168<<16) | (33<<8) | 175)
#define HOST_IP_ADDR            ((192<<24) | (168<<16) | (33<<8) | 1)

// MPIX2 register definitions (note: offsets represent bytes)
#define MPIX2_DATAXMT_REG_OFFSET           0x00
#define MPIX2_DATARCV_REG_OFFSET           0x04
#define MPIX2_STATUS_REG_OFFSET            0x08
#define MPIX2_CONF_REG_OFFSET              0x0C
#define MPIX2_TIMER_REG_OFFSET             0x10
#define MPIX2_ANIN_REG_OFFSET              0x14

/* Status register bits */
#define MPIX2_STATUS_MPIX_IDLE             0x0001
#define MPIX2_STATUS_MPIX_BUSY             0x0002
#define MPIX2_STATUS_XMT_READY             0x0004
#define MPIX2_STATUS_RCV_READY             0x0008
#define MPIX2_STATUS_DATARCV_READY         0x0010
#define MPIX2_STATUS_SHUTTER_READY         0x0020
#define MPIX2_STATUS_BURST_READY_INT       0x0040
#define MPIX2_STATUS_BUS_ERR_INT           0x0080
#define MPIX2_STATUS_CONNECT_ERR_INT       0x0100
#define MPIX2_STATUS_WATCHDOG_ERR_INT      0x0200
#define MPIX2_STATUS_FATAL_ERR_INT         0x0400
#define MPIX2_STATUS_EXT_TRIG_INT          0x0800

/* Configuration register bits */
#define MPIX2_CONF_START                   0x00001
#define MPIX2_CONF_MODE_MASK               0x00006
#define MPIX2_CONF_MODE_READOUT            0x00000
#define MPIX2_CONF_MODE_SET_MATRIX         0x00002
#define MPIX2_CONF_MODE_SET_DAC            0x00004
#define MPIX2_CONF_POLARITY                0x00008
#define MPIX2_CONF_SPARE_FSR               0x00010
#define MPIX2_CONF_ENABLE_TPULSE           0x00020
#define MPIX2_CONF_ENABLE_CST              0x00040
#define MPIX2_CONF_P_S                     0x00080
#define MPIX2_CONF_SHUTTER_CLOSED          0x00100
#define MPIX2_CONF_TIMER_USED              0x00200
#define MPIX2_CONF_RESET_MPIX              0x00400
#define MPIX2_CONF_BURST_READOUT           0x00800
#define MPIX2_CONF_CHIP_ALL                0x01000
#define MPIX2_CONF_CHIP_SEL_MASK           0x06000
#define MPIX2_CONF_CHIP_SEL_SHIFT          13
#define MPIX2_CONF_EXT_TRIG_ENABLE         0x08000
#define MPIX2_CONF_EXT_TRIG_FALLING_EDGE   0x10000
#define MPIX2_CONF_EXT_TRIG_INHIBIT        0x20000
#define MPIX2_CONF_TPX_CLOCK_MASK          0x001C0000
#define MPIX2_CONF_TPX_CLOCK_SHIFT         18
#define MPIX2_CONF_TPX_CLOCK_100MHZ        (0<<MPIX2_CONF_TPX_CLOCK_SHIFT)
#define MPIX2_CONF_TPX_CLOCK_50MHZ         (1<<MPIX2_CONF_TPX_CLOCK_SHIFT)
#define MPIX2_CONF_TPX_CLOCK_10MHZ         (2<<MPIX2_CONF_TPX_CLOCK_SHIFT)
#define MPIX2_CONF_TPX_CLOCK_EXT           (3<<MPIX2_CONF_TPX_CLOCK_SHIFT)
#define MPIX2_CONF_RO_CLOCK_125MHZ         0x00200000
#define MPIX2_CONF_TPX_PRECLOCKS           0x00400000
#define MPIX2_CONF_TPX_TRIG_TYPE_SHIFT     23
#define MPIX2_CONF_TPX_TRIG_TYPE_NEG       (0 << MPIX2_CONF_TPX_TRIG_TYPE_SHIFT)
#define MPIX2_CONF_TPX_TRIG_TYPE_POS       (1 << MPIX2_CONF_TPX_TRIG_TYPE_SHIFT)
#define MPIX2_CONF_TPX_TRIG_TYPE_PULSE     (2 << MPIX2_CONF_TPX_TRIG_TYPE_SHIFT)
#define MPIX2_CONF_TPX_TRIG_TYPE_MASK      (3 << MPIX2_CONF_TPX_TRIG_TYPE_SHIFT)

// Error identifiers from RelaxD module (MPIXD = 'Medipix2 TCP/IP daemon')
#define MPIXD_ERR_NONE          0x0000  // No error, all okay
#define MPIXD_ERR_ILLEGAL       0x0001  // Illegal message type
#define MPIXD_ERR_LENGTH        0x0002  // Incorrect message length
#define MPIXD_ERR_OFFSET        0x0004  // Incorrect address offset
#define MPIXD_ERR_ROW           0x0008  // Incorrect row number sequence
#define MPIXD_ERR_BUS           0x0010  // Wishbone bus error
#define MPIXD_ERR_CONNECT       0x0020  // Medipix2 chip unconnected error
#define MPIXD_ERR_WATCHDOG      0x0040  // Medipix2 controller watchdog error
#define MPIXD_ERR_FATAL         0x0080  // Medipix2 controller fatal error
#define MPIXD_ERR_TRIGGER       0x0100  // Medipix2 external trigger
#define MPIXD_ERR_TIMEOUT       0x2000  // Timed out
#define MPIXD_ERR_INTERRUPT     0x4000  // Interrupted
#define MPIXD_ERR_UNKNOWN       0x8000  // Unknown error

// Additional module class errors
#define MPXERR_COMM             (-65534)  // Communication error

// ----------------------------------------------------------------------------

enum mpixd_type {
    /* Error message */
    MPIXD_ERROR           = 0xDECEA5ED,
    /* Ack message */
    MPIXD_ACK             = 0xACCE55ED,
    /* Trigger message */
    MPIXD_TRIGGER         = 0xAABBCCDD,
    /* Command messages */
    MPIXD_GET_REG         = 0xCA110000,
    MPIXD_SET_REG         = 0xCA110001,
    MPIXD_SET_DACS        = 0xCA110002,
    MPIXD_SET_MATRIX      = 0xCA110003,
    MPIXD_READOUT         = 0xCA110004,
    MPIXD_SET_CONFIG      = 0xCA110005,
    MPIXD_GET_ADC         = 0xCA110006,
    MPIXD_PING            = 0xCA110007,
    MPIXD_SET_TESTPULSE   = 0xCA110008,
    MPIXD_GET_TEMPERATURE = 0xCA110009,
    MPIXD_SET_DACONVERTER = 0xCA11000A,
    MPIXD_GET_CHIPID      = 0xCA11000B,
    MPIXD_GET_DACS        = 0xCA11000C,
    MPIXD_STORE_ID        = 0xCA11000D,
    MPIXD_STORE_DACS      = 0xCA11000E,
    MPIXD_ERASE_DACS      = 0xCA11000F,
    MPIXD_STORE_CONFIG    = 0xCA110010,
    MPIXD_ERASE_CONFIG    = 0xCA110011,
    MPIXD_GET_SWVERSION   = 0xCA110012,
    MPIXD_LOAD_CONFIG     = 0xCA110013,
	MPIXD_WRITE_FIRMWARE  = 0xCA110014,
    MPIXD_READ_FIRMWARE   = 0xCA110015
};

#define AUTO_BURST_EXT
#ifdef AUTO_BURST_EXT
#define MPIXD_AB_ENABLE             0x1000	/* enable bit, won't work without */
#define MPIXD_AB_CHIPNR_MASK	    0xC000	/* chip select mask */
#define MPIXD_AB_CHIPNR_SHIFT       14
#define MPIXD_AB_CHIP_ALL           0x2000	/* parallel mode */
#endif

#if defined(WIN32) || defined(_WINDLL)
#define PACK
#define PACK_ARRAY
#pragma pack(1)
#else
#define PACK __attribute__ ((packed))
#ifndef __APPLE__
/* linux warning cleanup... */
#define PACK_ARRAY
#else
/* on Mac, this may need to be changed, leave it as is until I have a Mac to test on */
#define PACK_ARRAY PACK
#endif
#endif

struct PACK mpixd_get_reg  {
    u32 offset PACK;
} PACK;

struct PACK mpixd_set_reg {
    u32 value PACK;
    u32 offset PACK;
} PACK;

struct PACK mpixd_fsr {
    u8 bytes[32];
} PACK;

struct PACK mpixd_row_header {
    u16 readout PACK;
    u16 number PACK;
    u32 timestamp PACK;
} PACK;

#define MPIX_NUM_ROWS_PER_MSG_ORIG  1
#define MPIX_NUM_ROWS_PER_MSG_V2    2
#define MPIX_NUM_ROWS_PER_MSG_MAX   MPIX_NUM_ROWS_PER_MSG_V2

#define MPIX_NUM_ROWS_PER_MSG_WRITE 2

struct PACK mpixd_row {
    u16 framecnt PACK;
    u16 number PACK;
    u32 timestamp PACK;
    u8  bytes[MPIX_ROW_BYTES * MPIX_NUM_ROWS_PER_MSG_MAX] PACK_ARRAY; /* (256 pixels * 14 bits * 2 (for dual row read)) = 896 bytes  */
} PACK;

struct PACK mpixd_hdr {
    u32 type PACK;
    u32 length PACK;
} PACK;

struct mpixd_flash_pg {	
	u16 pagenr PACK;
	u8	bytes[256] PACK_ARRAY; // (256 bytes = 1 page )
} PACK;

struct PACK mpixd_testpulse {
    u32 low PACK;
    u32 high PACK;
    u32 khzfreq PACK;
    u32 count PACK;
} PACK;

struct PACK mpixd_msg {
    struct mpixd_hdr header;
    union {
        u32                    number   PACK;
        struct mpixd_get_reg   get_reg;  /* Register get arguments */
        struct mpixd_set_reg   set_reg;  /* Register set arguments */
        struct mpixd_fsr       fsr;      /* Transmit data (256 bit) */
        struct mpixd_row       row;      /* Transmit row (3584 bits) */
        struct mpixd_testpulse tpulse;   /* Testpulse paramters */
		struct mpixd_flash_pg flash;     /* Flash page paramters */
    } data;
} PACK;

struct PACK mpixd_reply_msg {
    struct mpixd_hdr header;
    union {
        u32                     error   PACK;  /* Error code */
        u32                     value   PACK;  /* Return value */
        struct mpixd_fsr        fsr;           /* Received data (256 bit) */
        struct mpixd_row        row;           /* Received row (3584 bits) */
        struct mpixd_row_header row_ok;       /* row OK indication */
		struct mpixd_flash_pg	flash;		  /* Flash page paramters */
    } data;
} PACK;

#if defined(WIN32) || defined(_WINDLL)
#pragma pack()
#undef PACK
#else
#undef PACK
#endif

// Definitions concerning DACs for Mpx 2.1 and MXR
#define MXR_FSR_IKRUM_I              3
#define MXR_FSR_IKRUM_BITS           8
#define MXR_FSR_DISC_I               11
#define MXR_FSR_DISC_BITS            8
#define MXR_FSR_PREAMP_I             19
#define MXR_FSR_PREAMP_BITS          8
#define MXR_FSR_DACCODEB0B1_I        37
#define MXR_FSR_DACCODEB0B1_BITS     2
#define MXR_FSR_DACCODEB2B3_I        40
#define MXR_FSR_DACCODEB2B3_BITS     2
#define MXR_FSR_SENSEDAC_I           42
#define MXR_FSR_SENSEDAC_BITS        1
#define MXR_FSR_EXTDACSELECT_I       43
#define MXR_FSR_EXTDACSELECT_BITS    1
#define MXR_FSR_BUFANALOGA30_I       45
#define MXR_FSR_BUFANALOGA30_BITS    4
#define MXR_FSR_BUFANALOGA74_I       55
#define MXR_FSR_BUFANALOGA74_BITS    4
#define MXR_FSR_BUFANALOGB40_I       59
#define MXR_FSR_BUFANALOGB40_BITS    5
#define MXR_FSR_BUFANALOGB75_I       64
#define MXR_FSR_BUFANALOGB75_BITS    3
#define MXR_FSR_DELAYN_I             85
#define MXR_FSR_DELAYN_BITS          8
#define MXR_FSR_THL_I                99
#define MXR_FSR_THL_BITS             14
#define MXR_FSR_THH_I                113
#define MXR_FSR_THH_BITS             14
#define MXR_FSR_FBK_I                127
#define MXR_FSR_FBK_BITS             8
#define MXR_FSR_GND_I                135
#define MXR_FSR_GND_BITS             8
#define MXR_FSR_CTPR_I               143
#define MXR_FSR_CTPR_BITS            32
#define MXR_FSR_THS_I                180
#define MXR_FSR_THS_BITS             8
#define MXR_FSR_FUSES_I              195
#define MXR_FSR_FUSES_BITS           24
#define MXR_FSR_BIASLVDS_I           226
#define MXR_FSR_BIASLVDS_BITS        8
#define MXR_FSR_REFLVDS_I            234
#define MXR_FSR_REFLVDS_BITS         8

#define MXR_FSR_BUFANALOGA_BITS      (MXR_FSR_BUFANALOGA30_BITS+ \
                                      MXR_FSR_BUFANALOGA74_BITS)
#define MXR_FSR_BUFANALOGB_BITS      (MXR_FSR_BUFANALOGB40_BITS+ \
                                      MXR_FSR_BUFANALOGB75_BITS)
#define MXR_FSR_THLFINE_BITS         10
#define MXR_FSR_THLCOARSE_BITS       4
#define MXR_FSR_THHFINE_BITS         10
#define MXR_FSR_THHCOARSE_BITS       4

#define MXR_DACS                     15

// Definitions concerning DACs for TPX
#define TPX_FSR_IKRUM_I              3+1
#define TPX_FSR_IKRUM_BITS           8
#define TPX_FSR_DISC_I               11+1
#define TPX_FSR_DISC_BITS            8
#define TPX_FSR_PREAMP_I             19+1
#define TPX_FSR_PREAMP_BITS          8
#define TPX_FSR_DACCODEB0B1_I        37+1
#define TPX_FSR_DACCODEB0B1_BITS     2
#define TPX_FSR_DACCODEB2B3_I        40+1
#define TPX_FSR_DACCODEB2B3_BITS     2
#define TPX_FSR_SENSEDAC_I           42+1
#define TPX_FSR_SENSEDAC_BITS        1
#define TPX_FSR_EXTDACSELECT_I       43+1
#define TPX_FSR_EXTDACSELECT_BITS    1
#define TPX_FSR_BUFANALOGA30_I       45+1
#define TPX_FSR_BUFANALOGA30_BITS    4
#define TPX_FSR_BUFANALOGA74_I       55+1
#define TPX_FSR_BUFANALOGA74_BITS    4
#define TPX_FSR_BUFANALOGB40_I       59+1
#define TPX_FSR_BUFANALOGB40_BITS    5
#define TPX_FSR_BUFANALOGB75_I       64+1
#define TPX_FSR_BUFANALOGB75_BITS    3
#define TPX_FSR_HIST_I               85+1
#define TPX_FSR_HIST_BITS            8
#define TPX_FSR_THL_I                99+1
#define TPX_FSR_THL_BITS             14
#define TPX_FSR_VCAS_I               119+1
#define TPX_FSR_VCAS_BITS            8
#define TPX_FSR_FBK_I                127+1
#define TPX_FSR_FBK_BITS             8
#define TPX_FSR_GND_I                135+1
#define TPX_FSR_GND_BITS             8
#define TPX_FSR_CTPR_I               143+1
#define TPX_FSR_CTPR_BITS            32
#define TPX_FSR_THS_I                180+1
#define TPX_FSR_THS_BITS             8
#define TPX_FSR_FUSES_I              195+1
#define TPX_FSR_FUSES_BITS           24
#define TPX_FSR_BIASLVDS_I           226+1
#define TPX_FSR_BIASLVDS_BITS        8
#define TPX_FSR_REFLVDS_I            234+1
#define TPX_FSR_REFLVDS_BITS         8

#define TPX_FSR_BUFANALOGA_BITS      (TPX_FSR_BUFANALOGA30_BITS+ \
                                      TPX_FSR_BUFANALOGA74_BITS)
#define TPX_FSR_BUFANALOGB_BITS      (TPX_FSR_BUFANALOGB40_BITS+ \
                                      TPX_FSR_BUFANALOGB75_BITS)
#define TPX_FSR_THLFINE_BITS         10
#define TPX_FSR_THLCOARSE_BITS       4

#define TPX_DACS                     14

// Defines to replace 8-bit 'struct _PixelCfg' in common.h (PixelMan)
#define MXR_CFG8_MASK                ((u8) 0x01)
#define MXR_CFG8_TEST                ((u8) 0x02)
#define MXR_CFG8_LO_THR_MASK         ((u8) 0x1C)
#define MXR_CFG8_HI_THR_MASK         ((u8) 0xE0)
#define MXR_CFG8_MASK_SHIFT          0
#define MXR_CFG8_TEST_SHIFT          1
#define MXR_CFG8_LO_THR_MASK_SHIFT   2
#define MXR_CFG8_HI_THR_MASK_SHIFT   5

// Defines to replace 8-bit 'struct _TpxPixCfg' in common.h (PixelMan)
#define TPX_CFG8_MASK                ((u8) 0x01)
#define TPX_CFG8_TEST                ((u8) 0x02)
#define TPX_CFG8_THRADJ_MASK         ((u8) 0x3C)
#define TPX_CFG8_MODE_MASK           ((u8) 0xC0)
#define TPX_CFG8_MASK_SHIFT          0
#define TPX_CFG8_TEST_SHIFT          1
#define TPX_CFG8_THRADJ_MASK_SHIFT   2
#define TPX_CFG8_MODE_MASK_SHIFT     6

// Defines to replace 16-bit 'struct Mpx3PixCfg' in common.h (PixelMan)
#define MX3_CFG_MASK                 ((u16) 0x0001)
#define MX3_CFG_TEST                 ((u16) 0x0002)
#define MX3_CFG_GAIN_MODE            ((u16) 0x0004)
#define MX3_CFG_LO_THL_ADJ_MASK      ((u16) 0x00F8)
#define MX3_CFG_LO_THH_ADJ_MASK      ((u16) 0x1F00)
#define MX3_CFG_MASK_SHIFT           0
#define MX3_CFG_TEST_SHIFT           1
#define MX3_CFG_GAIN_MODE_SHIFT      2
#define MX3_CFG_THL_ADJ_MASK_SHIFT   3
#define MX3_CFG_THH_ADJ_MASK_SHIFT   8

// Defines to replace 16-bit 'struct _i16PixelCfg'
#define MXR_CFG16_MASK               0x0001
#define MXR_CFG16_LO_THR1_MASK       0x0040
#define MXR_CFG16_LO_THR0_MASK       0x0080
#define MXR_CFG16_LO_THR2_MASK       0x0100
#define MXR_CFG16_TEST               0x0200
#define MXR_CFG16_HI_THR1_MASK       0x0400
#define MXR_CFG16_HI_THR2_MASK       0x0800
#define MXR_CFG16_HI_THR0_MASK       0x1000
#define MXR_CFG16_MASK_SHIFT         0
#define MXR_CFG16_LO_THR1_MASK_SHIFT 6
#define MXR_CFG16_LO_THR0_MASK_SHIFT 7
#define MXR_CFG16_LO_THR2_MASK_SHIFT 8
#define MXR_CFG16_TEST_SHIFT         9
#define MXR_CFG16_HI_THR1_MASK_SHIFT 10
#define MXR_CFG16_HI_THR2_MASK_SHIFT 11
#define MXR_CFG16_HI_THR0_MASK_SHIFT 12

// Defines to replace 16-bit 'struct _i16TpxPixCfg'
#define TPX_CFG16_P1                 0x0040
#define TPX_CFG16_MASK               0x0080
#define TPX_CFG16_THR0               0x0100
#define TPX_CFG16_P0                 0x0200
#define TPX_CFG16_THR2               0x0400
#define TPX_CFG16_THR3               0x0800
#define TPX_CFG16_THR1               0x1000
#define TPX_CFG16_TEST               0x2000
#define TPX_CFG16_P1_SHIFT           6
#define TPX_CFG16_MASK_SHIFT         7
#define TPX_CFG16_THR0_SHIFT         8
#define TPX_CFG16_P0_SHIFT           9
#define TPX_CFG16_THR2_SHIFT         10
#define TPX_CFG16_THR3_SHIFT         11
#define TPX_CFG16_THR1_SHIFT         12
#define TPX_CFG16_TEST_SHIFT         13

// Definitions of TIMEPIX modes (i.e. P0+P1 bits in pixel configuration)
#define TPX_MODE_MEDIPIX             0
#define TPX_MODE_TOT                 1
#define TPX_MODE_TIMEPIX_1HIT        2
#define TPX_MODE_TIMEPIX             3
#define TPX_MODE_DONT_SET            -1

// ----------------------------------------------------------------------------
#endif // RELAXD_H

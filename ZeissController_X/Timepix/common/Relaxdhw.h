#ifndef ASI_RELAXD_HW_Hxx
#define ASI_RELAXD_HW_Hxx

#define MPIX_MAX_DEVS           4
#define MPIX_ROWS               256
#define MPIX_COLS               256
#define MPIX_PIXELS             (MPIX_ROWS*MPIX_COLS) // chip matrix size
#define MPIX_PIXEL_BITS         14
#define MPIX_ROW_BYTES          ((MPIX_COLS*MPIX_PIXEL_BITS)/8)
#define MPIX_FRAME_BYTES        (MPIX_ROWS*MPIX_ROW_BYTES)
#define MPIX_FSR_BITS           256
#define MPIX_FSR_BYTES          ((MPIX_FSR_BITS+7)/8)
#define MPIXD_PORT              0xCE11  // Port 0xCE11, 52753

#endif

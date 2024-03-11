/*
       Copyright 2011 NIKHEF

*/
#ifndef MPXMODULEAPI_H
#define MPXMODULEAPI_H

#include "mpxhw.h"
#include "mpx3hw.h"
#include "mpxplatform.h"

// ----------------------------------------------------------------------------

extern "C"
{
    MPXMODULE_API Mpx2Interface *getMpx2Interface();
}

// ----------------------------------------------------------------------------

extern "C"
{
    MPXMODULE_API Mpx2Interface *getMpx3Interface();
}

// ----------------------------------------------------------------------------
// MPX2 Interface

int         mpx2FindDevices     ( int ids[], int *count );
int         mpx2Init            ( int hw_id );
int         mpx2CloseDevice     ( int hw_id );
int         mpx2SetCallback     ( HwCallback cb );
int         mpx2SetCallbackData ( int hw_id, INTPTR data );
int         mpx2GetHwInfoCount  ();
int         mpx2GetHwInfoFlags  ( int hw_id, int index, u32 *flags );
int         mpx2GetHwInfo       ( int hw_id, int index,
                                  HwInfoItem *hwInfo, int *sz );
int         mpx2SetHwInfo       ( int hw_id, int index,
                                  void *data, int sz );
int         mpx2GetDevInfo      ( int hw_id, DevInfo *dev_info );
int         mpx2Reset           ( int hw_id );
int         mpx2SetDACs         ( int hw_id, DACTYPE dac_vals[],
                                  int sz, byte sense_chip, byte ext_dac_chip,
                                  int codes[], u32 tp_reg );
int         mpx2GetMpxDacVal    ( int hw_id, int chip_nr, double *val );
int         mpx2SetExtDacVal    ( int hw_id, double val );
int         mpx2SetPixelsCfg    ( int hw_id, byte cfgs[], u32 sz );
int         mpx2SetAcqPars      ( int hw_id, AcqParams *pars );
int         mpx2StartAcquisition( int hw_id );
int         mpx2StopAcquisition ( int hw_id );
int         mpx2GetAcqTime      ( int hw_id, double *time );
int         mpx2ResetMatrix     ( int hw_id );
int         mpx2ReadMatrix      ( int hw_id, i16 *data, u32 sz );
int         mpx2ReadMatrixRaw   ( int hw_id, u8* data, u32 sz);
int         mpx2WriteMatrix     ( int hw_id, i16 *data, u32 sz );
int         mpx2SendTestPulses  ( int hw_id, double pulse_height,
                                  double period, u32 pulse_count );
int         mpx2IsBusy          ( int hw_id, BOOL *busy );
const char *mpx2GetLastError    ();
const char *mpx2GetLastDevError ( int hw_id );

// ----------------------------------------------------------------------------
// MPX3 Interface

int         mpx3FindDevices     ( int ids[], int *count );
int         mpx3Init            ( int id );
int         mpx3CloseDevice     ( int id );
int         mpx3SetCallback     ( HwCallback cb );
int         mpx3SetCallbackData ( int id, INTPTR data );
int         mpx3GetHwInfoCount  ();
int         mpx3GetHwInfoFlags  ( int id, int index, u32 *flags );
int         mpx3GetHwInfo       ( int id, int index,
                                  HwInfoItem *hwInfo, int *sz );
int         mpx3SetHwInfo       ( int id, int index,
                                  void *data, int sz );
int         mpx3GetDevInfo      ( int id, DevInfo *dev_info );
int         mpx3Reset           ( int id );
int         mpx3SetDACs         ( int id, DACTYPE dac_vals[], int size );
int         mpx3SenseSignal     ( int id, int chip_nr,
                                  int code, double *val );
int         mpx3SetExtDAC       ( int id, int code, double value );
int         mpx3SetPixelsCfg    ( int id, Mpx3PixCfg cfgs[], u32 sz );
int         mpx3SetAcqPars      ( int id, Mpx3AcqParams *pars );
int         mpx3StartAcquisition( int id );
int         mpx3StopAcquisition ( int id );
int         mpx3GetAcqTime      ( int id, double *time );
int         mpx3ResetMatrix     ( int id );
int         mpx3ReadMatrix      ( int id, u32 *data, u32 sz );
int         mpx3WriteMatrix     ( int id, u32 *data, u32 sz );
int         mpx3SendTestPulses  ( int id, double charge[2], double period,
                                  u32 pulse_count, u32 ctpr[8] );
int         mpx3IsBusy          ( int id, BOOL *busy );
const char *mpx3GetLastError    ();
const char *mpx3GetLastDevError ( int id );

// ----------------------------------------------------------------------------
#endif // MPXMODULEAPI_H

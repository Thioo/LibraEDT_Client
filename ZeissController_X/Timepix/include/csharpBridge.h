#ifndef CSHARPBRIDGE_H
#define CSHARPBRIDGE_H

#define UNMANAGEDDLL_API __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

	extern UNMANAGEDDLL_API MpxModule* MpxModule_newMpxModule(int hwId);

	extern UNMANAGEDDLL_API void MpxModule_dispouseMpxModule(MpxModule* module);

	extern UNMANAGEDDLL_API void MpxModule_getDevInfo(MpxModule* module, DevInfo* devInfo);

	extern UNMANAGEDDLL_API int MpxModule_hwInfoCount(MpxModule* module);

	extern UNMANAGEDDLL_API int MpxModule_init(MpxModule* module);

	extern UNMANAGEDDLL_API void MpxModule_getHwInfo(MpxModule* module, int index, HwInfoItem* hwInfo);

	extern UNMANAGEDDLL_API int MpxModule_getHwInfoFlags(MpxModule* module, int index);

	extern UNMANAGEDDLL_API int MpxModule_setHwInfo(MpxModule* module, int index, void* data, int sz);

	extern UNMANAGEDDLL_API double MpxModule_getAcqTime(MpxModule* module);

	extern UNMANAGEDDLL_API int MpxModule_setAcqPars(MpxModule* module, AcqParams *acqParams);

	extern UNMANAGEDDLL_API int MpxModule_startAcquisition(MpxModule* module);

	extern UNMANAGEDDLL_API int MpxModule_stopAcquisition(MpxModule* module);

	extern UNMANAGEDDLL_API int MpxModule_configAcqMode(MpxModule* module, boolean ext_trig, boolean use_timer, double time_s);

	extern UNMANAGEDDLL_API int MpxModule_setHwInfo(MpxModule* module, int index, void* data, int sz);

	extern UNMANAGEDDLL_API int MpxModule_setFsrs(MpxModule* module, short dacs[], byte senseChip, byte extDacChip, int codes[], int tpReg, int len);

	extern UNMANAGEDDLL_API int MpxModule_setFsr(MpxModule* module, int chipnr, int dac[], int col_test_pulse_reg);

	extern UNMANAGEDDLL_API int MpxModule_refreshFsr(MpxModule* module, int chipnr);

	extern UNMANAGEDDLL_API double MpxModule_getMpxDac(MpxModule* module, int chipnr);

	extern UNMANAGEDDLL_API int MpxModule_setPixelsCfg(MpxModule* module, byte cfg[], int chipnr);

	extern UNMANAGEDDLL_API int MpxModule_writeMatrix(MpxModule* module, short jBuffer[], int len);

	extern UNMANAGEDDLL_API int MpxModule_readMatrix__J_3S(MpxModule* module, short jBuffer[], int len);

	extern UNMANAGEDDLL_API int MpxModule_readMatrixRaw__J_3B(MpxModule* module, byte jByteArray[], int len);

	extern UNMANAGEDDLL_API int MpxModule_resetMatrix(MpxModule* module);

	extern UNMANAGEDDLL_API int MpxModule_reset(MpxModule* module);

	extern UNMANAGEDDLL_API boolean MpxModule_isBusy(MpxModule* module);

	extern UNMANAGEDDLL_API int MpxModule_setPixelsCfgFromFile(MpxModule* module, const char * filename, int mode = TPX_MODE_DONT_SET);

	extern UNMANAGEDDLL_API int MpxModule_enableTimer(MpxModule* module, bool enable, int us = 10);

#ifdef __cplusplus
}
#endif
#endif // CSHARPBRIDGE_H


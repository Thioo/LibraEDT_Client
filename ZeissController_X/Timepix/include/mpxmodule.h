/*
		Copyright 2011 NIKHEF

*/

/*
    Name       : mpxmodule.h

    Description: Contains the definition of the MpxModule class,
    implementing access to a Relaxd module.

    History    :
        05.11.2010: Version 1.2.0:
        - Replaced HW_ITEM_VERSION and HW_ITEM_ADDNOISE (not needed)
          by HW_ITEM_LIBVERSION and HW_ITEM_RELAXDSWVERSION
          for MpxModule class version and Relaxd embedded software
          version respectively.
          Added new command for Relaxd module in relaxd.h:
          MPIXD_GET_SWVERSION = 0xCA110012
          New var : u32 _relaxdSwVersion
          New func: int readSwVersion()
        - Added additional socket for monitoring messages,
          leaving the messages with data and data-acquisition commands
          to the other socket (this change was triggered by
          a requirement from a Medipix setup at AMOLF where they needed
          to monitor the temperature-sensor while taking data).
          New var : SOCKET _sockMon, struct sockaddr_in _mpxMonSockAddr
          New func: int openSockMon(), int txRxMsgMon(), int txMsgMon(),
          int rxMsgMon()

    21.01.2011: Version 1.3.0:
        - Added this version changes comment.
        - Added TIMEPIX pixel configuration modes to relaxd.h.
        - Added new versions of setPixelsCfg() enabling more options
          for pixel configuration, for non-Pixelman (standalone) usage.
        - Added ofstream _fData, openDataFile() and closeDataFile()
          for standalone data logging.
        - Added int enableTimer( bool enable, int us10 = 1 ) and
          int enableExtTrigger( bool enable ) for standalone use of
          this library.

    25.03.2011: Version 1.4.0:
        - In burstReadoutSeq/Par(): added support for messages
          containing 2 rows of pixels per message.
        - Added function: void flushSockInput() and use it in
          writeMatrix(), resetMatrix() and all setPixelCfg().

    28.03.2011: Version 1.4.1:
        - Set socket time-out from 100 to 200 ms (for AMOLF test).
        - Use flushSockInput() in above-mentioned functions before
          accessing a chip, rather than once.

    28.03.2011: Version 1.4.2:
        - Modify flushSockInput(): first call msgAvailable(),
          otherwise time-outs take too long.

    29.03.2011: Version 1.4.3:
        - Set socket time-out back to 100 ms.
        - Add flushSockInput() to setDacs().
        - Added a call to resetChips() in mpxError() in case of
          an MPIXD_ERR_TIMEOUT error, to facilitate possible recovery
          of a 'crashed' system (to solve AMOLF issue).

    30.03.2011: Version 1.4.4:
        - Bugfix: when calling stopDaqThread() in selectChip() forgot
        to set _waitingForTrigger to false.

    30.03.2011: Version 1.4.5:
        - Retry writeFsr() in setDacs() if necessary
          (to solve AMOLF issue).

    31.03.2011: Version 1.4.6:
        - Output 'close file' message to logfile in d'tor.
        - Retry selectChip() and writeRow() in setPixelsCfg().
        - Add flushSockInput() to retry operations.
        - Display header type of last message in flushSockInput().

    01.04.2011: Version 1.4.7:
        - Change flushSockInput() to flushSockInput( bool long_timeout
          = false ) and use it in retries.

    01.04.2011: Version 1.4.8:
        - In several more locations do a retry of selectChip() when
          necessary.
        - Ignore error return in retry in setPixelsCfg()
          (for AMOLF problem only).

    04.04.2011: Version 1.4.9:
        - Remove retries of selectChip(); now solve it by retrying
          in the more basic readReg() function; because of this
          a number of flushSockInput() calls have also been removed.

    04.04.2011: Version 1.4.10:
        - Add retry after communication error/timeout in readReg(),
          rather than only when Relaxd returns an error message.

    04.04.2011: Version 1.4.11:
        - Add retry of burstReadoutSeq() in resetMatrix() and
          readMatrixSeq().

    13.04.2011: Version 1.4.12:
        - Where 'errno' is used, save it asap in a local variable 'err'.
        - Add flushSockInput(true) at end of openSock();
        - Uncomment "#define REPORT_BURST_ERROR" in burstReadoutPar().
        - Add retry of selectChip()/writeFsr() to setDacs(chipnr,..).
        - Add assignment operator to class Fsr in fsr.h/cpp,
          to be used here to store a copy of DAC settings to
          enable a DAC refresh operation.
        - Add _fsr[] pointers to Fsr objects to keep copies of
          uploaded DAC settings, to enable the DAC refresh operation.
        - Add refreshDacs(): if DAC settings not received yet by a host
           application, read them from the Relaxd module itself.

    18.04.2011: Version 1.4.13:
        - Retry receiving row data in burstReadoutPar() and
          burstReadoutSeq().

    26.05.2011: Version 1.5.0:
        - Rename genTestPulses() to generateTestPulses() and display
          its parameters in debug mode.
        - Use proper pulse high and low values when setting test pulse
          frequency (HW_ITEM_TESTPULSEFREQ).
        - In generateTestPulses():
        - When parameter 'pulse_height' is 0, use _testPulseHigh and
          _testPulseLow settings.
        - Use the hardware timer for exact shutter timing.
        - Disable testpulse generator after pulse generation finished.
        - Use a single _floatForPixelman for various 'HW items',
          rather than dedicated member variables not used anywhere
          else (_vdd, _vdda, _mpixAnalogCurrent, _mpixDigitalCurrent).
        - Fix enableTimer(): 2nd parameter in [us], not in [10us].

    08.06.2011: Version 1.5.1:
        - Exposed control for clock settings and disabling bias supply
          to Pixelman.
        - Exposed storing of dac settings to Pixelman.

    20.06.2011: Version 1.5.2:
        - Changed some 'hwInfoItem' names; changed hwInfoItem 'bias'
          from float to percentage.
        - Change _bias to _biasAdjust and _digitalAdjust to _vddAdjust.
        - ADC input and DAC output name changes, in relaxd.h.

    23.06.2011: Version 1.5.3:
        - Bugfix setting _biasAdjust and _vddAdjust.
        - Use proper power-up value for _vddAdjust dependent on Medipix
          device type (to do: values should be read from Relaxd module),
          implemented in Mico32 code version 1.5.0, 22 Jun 2011.
        - Added HW_ITEM_ERASE_STORED_CFG, to erase stored FSR/DACs and
          pixel configurations.

    08.07.2011: Version 1.5.4:
        - Rename reset() to resetChips(),
          rename generalReset() to reset(),
          rename reinit() to init(),
          rename getDac() to getMpxDac(),
          rename readDacs() to readFsr(),
          rename refreshDacs() to refreshFsr(),
          rename setDacs() (multiple devices) to setFsrs(),
          rename setDacs() (single device) to setFsr().
        - Add new command for Relaxd module in relaxd.h:
          MPIXD_LOAD_CONFIG = 0xCA110013
        - Add loadConfig(), instructing Relaxd to (re)load its stored
          configuration to the Medipix devices.
        - In init() (was empty till now) make a call to loadConfig().
        - Set _parReadout default to true (was false).
        - Add 'get' functions parReadout() and logVerbose().
        - Add an additional readFsr() version that translates
          the obtained FSR bytes to DAC values.
        - Allow 'ndevices' in c'tor to be equal to zero:
          it will determine the number and type of devices itself
        - Add chipCount() and chipPosition().
        - Add optional parameter 'chipnr' to setPixelsCfg(u8 *cfgs),
          to allow setting of pixel configuration on a single device.
        - In mpxmodulemgr.h/cpp:
        - Allow ChipCount=0, default value is now -1.
        - Rename _devices to _modules and _devCnt to _modCnt.

    29.07.2011: Version 1.5.5:
        - Added communication error id MPXERR_COMM to relaxd.h
          and use it in case of message send/receive problems.
        - In MpxModule c'tor stop reading chip-ids when encountering
          a communication problem.
        - Added new ping() method.
        - Added a new simple (and in fact incomplete) c'tor,
          meant to call only the new ping() on (for quick scanning
          of Relaxd modules connected to a system).
        - In generateTestPulses(): moved configuration of hardware timer
          into configTestPulse().
        - In testPulseEnable(): in case of disable, disable timer too.
        - Linux bug fix: always use 'struct timeval' when setting socket
          receive time-out.
        - getHwInfo(): when *sz==0 provide required size to caller.
        - mpxmodulemgr.cpp/h: removed 'int _globalLastErr' and
          'string _globalLastErrStr;'

    09.09.2011: Version 1.5.6:
        - Added retrieval of module type, even if you specified the no.
        of chips.
        - Defined correct DLL Import / Export signatures.

    03.10.2011: Version 1.6
        - Added 'real' non-blocking IO
        - Added 'connect' to connecting to a UDP socket, saves some overhead.

    03.11.2011: Version 1.7
        - Changed IO to blocking again, since it didn't improve things
        - Removed nibble reverse, since version 1.7 of the firmware doesn't need it anymore.
        - Added timing information and framecount retrieval
          unsigned long lastClockTick()            // returns clock ticks (10 us per tick)
          double         lastTimeStamp()            // returns seconds
          unsigned long lastFrameCount()        // returns the frame count
          void          resetFrameCount()       // sets the frame count to 0

    13.12.2011: Version 1.7.1
        - Removed no-longer needed code.
        - Improved inconsistent code style (thanks to AStyle)
        - Added code for somewhat 'asynchronous' trigger reception.
        - Added Doxygen comments to methods.
        - Changed readSwVersion in the less ambiguous name getFirmwareVersion,
          and created software a counter-part getDriverVersion.
    13.01.2012: Version 1.8.0
	    - Added functionality to do firmware upgrade over UDP.
		- Separated logging from this class so other logging methods can be
		  implemented (e.g. log4c*, pantheios, your own).
    08.02.2012: Version 1.8.1
	    - Added board revision to getFirmwareVersion()
        - Fixed issue with slow acquisition in Pixelman
        - Fixed bug in PING handling, which could crash firmware.
	29.11.2012: Version 1.9.2
		- Data de-randomizatin improved
		- Fixed issue with Pixelman not storing some of the HW info items
		- Couple of error codes are not propagating out from the driver
		- Fixed issue with firmware uploading via ethernet and exposing the ethernet delay as public method
		- Number of bug-fixies
	05.12.2012: Version 1.9.3
		- Moved sockets code to MpxSocketMgr class
	06.12.2012: Version 1.9.4
		- Consolidated burstReadoutSeq/Par into burstReadout
	07.12.2012: Version 1.9.5
		- Constructor cleanup
		- Fixed all compiler warnings with warning level 4
	07.12.2012: Version 1.9.6
        - Fixed mess in getHwInfo/setHwInfo functions.
	07.13.2012: Version 1.9.7
        - Cleaned up setFrs functions.

*/

#ifndef MPXMODULE_H
#define MPXMODULE_H
// ----------------------------------------------------------------------------

#include <cstring>
#include <vector>
#include <time.h>
#include "mpxcodecmgr.h"


#include "fsr.h"
#include "../common/mpxhw.h"
#include "relaxd.h"
#include "mpxfilelogger.h"
#include "mpxsocketmgr.h"

// Hardware info items
#define  HW_ITEM_LIBVERSION             0
#define  HW_ITEM_RELAXDSWVERSION        1
#define  HW_ITEM_IPADDR                 2
#define  HW_ITEM_PORTNR                 3
#define  HW_ITEM_IMGFILENAME            4
#define  HW_ITEM_BOARDTEMP              5
#define  HW_ITEM_DETECTORTEMP           6
#define  HW_ITEM_VDD                    7
#define  HW_ITEM_VDDA                   8
#define  HW_ITEM_TESTPULSELO            9
#define  HW_ITEM_TESTPULSEHI            10
#define  HW_ITEM_TESTPULSEFREQ          11
#define  HW_ITEM_BIAS_VOLTAGE_ADJUST    12
#define  HW_ITEM_VDD_ADJUST             13
#define  HW_ITEM_ANALOGCURR             14
#define  HW_ITEM_DIGITALCURR            15
#define  HW_ITEM_CHIPIDS                16
#define  HW_ITEM_CHIPNAMES              17
#define  HW_ITEM_FIRSTCHIPNR            18
#define  HW_ITEM_LOGVERBOSE             19
#define  HW_ITEM_PARREADOUT             20
#define  HW_ITEM_STOREPIXELSCFG         21
#define  HW_ITEM_STOREDACS              22
#define  HW_ITEM_ERASE_STORED_CFG       23
#define  HW_ITEM_CONF_TPX_CLOCK         24
#define  HW_ITEM_CONF_RO_CLOCK_125MHZ   25
#define  HW_ITEM_CONF_TPX_PRECLOCKS     26
#define  HW_ITEM_RESERVED               27
#define  HW_ITEM_SET_TRIG_TYPE          28
#define  HW_ITEM_LOST_ROW_RETRY_CNT     29
#define  HW_ITEM_INVALID_LUT_ZERO       30
#define  HW_ITEM_DUMMY                  31


#define MULTIPLE_TRIGS_PER_ACQ

// ----------------------------------------------------------------------------

class MpxDaq;
struct MpxPimpl;

#define MPIX_LO_ONE_COLUMN  -1

/*! \brief  MpxModule encapsulates the Relaxd-modules functionality.

    This class also implements some functions required by Pixelman.
    These functions are not documented here.
*/
class MPXMODULE_API MpxModule
{
public:
    /*! \brief  Creates a new instance of the MpxModule.

        By default only the ID may be entered. All other options will
        automatically be detected. You can however overwrite this
        (not recommended).

        \param id           determines IP-addr of the module:
                            192.168.33+id.175
        \param mpx_type     Medipix2 (MPX_MXR) or Timepix (MPX_TPX) (optional)
        \param ndevices     number of devices (optional)
        \param nrows        number of device rows (optional)
        \param first_chip   the first chip index (optional)
		\param log			the logger to use (optional), by default it will create
		                    a file logger.
     */
    MpxModule( int id,
               int mpx_type = MPX_TPX,
               int ndevices = 0,
               int nrows = 1,
               int first_chipnr = 0,
			   MpxLogger *log = NULL );

    /* Special constructor, used to detect the presents of the module */
    MpxModule( int id, bool ping );

    virtual ~MpxModule();

    int     hwInfoCount     () { return ( int ) _hwInfoItems->size() - 1; }
    
    /*! \brief Pixelman compatibility function: currently calls loadConfig(). 

        \return 0 on success.
     */
    int     init            ();

    int     getHwInfo       ( int index, HwInfoItem *hw_item, int *sz );
    int     getHwInfoFlags  ( int index, u32 *flags );
    int     setHwInfo       ( const std::string &key, const std::string &value);
    int     setHwInfo       ( int index, void *data, int sz );
    int     getDevInfo      ( DevInfo *dev_info );
    int     getAcqTime      ( double *time ) {
        *time = elapsedTime();
        return 0;
    }
    int     setAcqPars      ( AcqParams *pars );
    int     setCallbackData ( INTPTR data );
    int     setCallback     ( HwCallback callback );
    void    callbackToPixMan( int evt_type );

    int     startAcquisition();
    int     stopAcquisition ();
    int     configAcqMode   ( bool   ext_trig,
                              bool   use_timer = false,
                              double time_s = 0.00001 );

    int     setFsrs         ( u16 dac_values[],
                              int size,
                              u8  sense_chip,
                              u8  ext_dac_chip,
                              int codes[],
                              u32 col_testpulse_reg );

    /*! \brief  Sets the DACs in Medipix device chipnr according to the values in dac.
    
        While assembling the FSR bits also sets the Column Test Pulse Register to col_testpulse_reg.
        The DAC settings indices are defined the common.h, and are dependent on the chip-type (Timepix / Medipix).
        Note that all settings for a chip should be set, before writing.
        Note that this function takes care of the correct ordering and position of the bits and bytes.

        Example:
        \code
        int dacs[TPX_DACS];
        dacs[TPX_IKRUM] = 20;
        mpxMod->setFsr(0, dacs, 0);
        \endcode

        Note that the other settings will also need to be set to sensible values.

        \param  chipnr              Medipix device number [0-3].
        \param  dac                 Array with DAC settings.
        \param  col_testpulse_reg	Array with DAC settings.

        \returns    0 on success.
    */
    int     setFsr          ( int chipnr, int *dac, u32 col_testpulse_reg = 0 );

    /*! \brief  Uploads (again) the current DAC settings.
    
        If the class instance has no copy of current DAC settings
        it reads them from the Relaxd module which keeps
        its own copy (may have been obtained from onboard storage).
        
        \param  chipnr	Medipix device number [0-3].

        \return 0 on success.
    */
    int     refreshFsr      ( int chipnr );

    /*! \brief   Reads the Relaxd ADC input.

        Reads the Relaxd ADC input corresponding to a DAC output value
        from the requested Medipix device. The integer ADC reading val_i
        is converted to a value in Volts:
        \code   *value = (val_i * 3.3) / 255    \endcode
        The particular Medipix DAC setting being output to the Relaxd
        ADC can be selected using setFsrs().

        \param  chipnr	Medipix device number [0-3].
        \param  value	Returned DAC voltage.

        \return 0 on success.
    */
    int     getMpxDac       ( int chipnr, double *value );

    /*! \brief  Configures pixels with the provided byte array.
         
         Pixels are configured individually using the bytes in array
         cfgs or from file named filename. In the former call an optional
         chipnr parameter indicates the cfgs array only contains configurations
         for that device. 
         
         Use the _PixelCfg (Medipix 2) and _TpxPixCfg (Timepix) structures from
         common.h to set the per pixel configuration, or use the constants defined
         in relaxd.h, starting with MXR_CFG8 and TPX_CFG8. 

         \param     cfgs	Array with pixel configuration settings (1 byte/pixel).
         \param     chipNr  If set, sets config for a single chip (otherwise
                            a configuration for all chips is expected, e.g 256x256xNrOfChips)

         \return     0 on success.
     */

    int     setPixelsCfg    ( u8 *cfgs, int chipnr = -1 );

    /*! \brief  Loads a complete pixel configuration from a file.
        
        Set the mode parameter optionally to set the Timepix mode to mode for all pixels.

        \param      filename	Name of file containing pixel configuration settings (1 byte/pixel).
        \param      mode	Applies to Timepix devices only: one of TPX_MODE_MEDIPIX, TPX_MODE_TOT, TPX_MODE_TIMEPIX_1HIT, TPX_MODE_TIMEPIX, TPX_MODE_DONT_SET.

        \return     0 on success.
     */
    int     setPixelsCfg    ( std::string filename, int mode = TPX_MODE_DONT_SET );

    /*! \brief  Sets all pixles according to the provided settings.

        Sets all pixels to the same configuration according to the provided parameters.

        \param  mode	Applies to Timepix devices only: one of 
            TPX_MODE_MEDIPIX, TPX_MODE_TOT, TPX_MODE_TIMEPIX_1HIT,
            TPX_MODE_TIMEPIX, TPX_MODE_DONT_SET.

        \param  thresh1	4-bits threshold for Timepix, 3-bits (low) threshold for Medipix2.
        \param  thresh2	3-bits (high) threshold for Medipix2.
        \param  test	Whether to set the pixels in test mode.
        \param  mask	Whether to mask the pixels.

        \return     0 on success.
     */
    int     setPixelsCfg    ( int  mode, int  thresh1, int thresh2,
                              bool test = false, bool mask = false );

    int     writeMatrix     ( i16 *data, u32 sz );
    
    /*! \brief  Reads a frame from all connected devices.

        Reads a frame from all connected devices, decodes the data
        and stores the pixel counts in array data. 

        \param  data	Array for returned pixel counts.
        \param  sz	    Size of array data.

        \return     0 on success.
     */
    int     readMatrix      ( i16 *data, u32 sz );
    
    /*! \brief Reads a frame from all connected devices.
    
        Stores the undecoded frame data in array data. A complete frame arrives
        in a defined number of messages (depending on the number of Medipix devices)
        containing a sequence number and concluded by an end-of-frame-readout message;
        if the total number of messages received is less than expected, the missing
        number of messages is returned in lost_rows. The data bytes in array data
        where the missing messages would be stored, according to their
        sequence number, are set to 0.
    
        \param  data	    Array for returned raw frame data.
        \param  sz	        Number of bytes returned in array data.
        \param  lost_rows	Number of apparently lost messages during readout.

        \returns        0 on success.
    */
    int     readMatrixRaw   ( u8 *bytes, u32 *sz, int *lost_rows );
private:
    int     readMatrixSeq   ( i16 *data, u32 sz );
    int     readMatrixPar   ( i16 *data, u32 sz );
    int     readMatrixRawSeq( u8 *bytes, u32 *sz, int *lost_rows );
    int     readMatrixRawPar( u8 *bytes, u32 *sz, int *lost_rows );
    int     writeDualRowPixelCfg(u8* pbytes);
    int     enableExternalTrigger(u32 reg);
public:

    /*! \brief  Reads a frame from all connected Medipix devices.
        
        Apparently needs to be called after every call to setPixelCfg().

        \return 0 on success.
    */
    int     resetMatrix     ();

    int     reset           ();
    int     getBusy         ( bool *busy );

    /*! \brief  Polls the Relaxd shutter signal to check whether the shutter has closed.

        Polls the Relaxd shutter signal once to check whether
        the (timer-controlled) shutter has closed.

        \returns    shutter closed or not.
    */
    bool    timerExpired    ();

    /*! \brief  Enables or disables the Relaxd shutter timer.
    
        Disables (enable is false) or enables the timer and sets the timer time-out
        to us microseconds. Note that the timer resolution is 10 us. After the Relaxd
        shutter opens (either explicitly by software or by an external trigger),
        it closes again after the set time.
        
        \param  enable  Whether to enable or disable.
        \param  us      Timer setting in microseconds.
        
        \returns     0 on success, or else an error
    */
    int     enableTimer     ( bool enable, int us = 10 );
    
    /*! \brief  Enables or disables the Relaxd external trigger input.
    
        If enabled the Relaxd shutter opens and closes controlled by
        the trigger input signal.
        
        \param  enable  Whether to enable or disable.
        
        \returns 0 on success.
    */
    int     enableExtTrigger( bool enable );

    /* \brief Generates a number of test pulses.

        \param  pulse_height    pulseHeight in V
        \param  period          period time in seconds per pulse.
        \param  count           number of testpulses to generate

        Note that this will open the shutter for you, but you will
        need to enableTimer before calling this function. It will
        open the shutter only once.

        \returns 0 on success.
    */

    int     generateTestPulses( double pulse_height,
                                double period,
                                u32    count );

    /*! \brief  Returns a description of the last error.
    
        Returns in a string a description of the last error that
        occurred during any of the other called operations. It can
        be called for example when a method returns unsuccessful.  
        
        \returns The last error message.
    */
    std::string lastErrStr      () {
        return _lastErrStr;
    }

    //juhe - 05/04/2012    
    /*! \brief  Returns a MPX error code of the last error.
     
     Returns the error code of the last error that
     occurred during any of the other called operations. It can
     be called for example when a method returns unsuccessful.  
     
     \returns The last error code.
     */
    int lastMpxErr() {
        return _lastMpxErr;
    }

    bool    _waitingForTrigger;

private:
    u32     _confReg;
    bool    _confRegValid;
    friend class MpxDaq;
    MpxDaq *_daqThreadHandler;

    int     burstReadout ( u8 *data, int  chipnr, int *lost_rows, bool parallel,
                              bool send_cmd = true );
    int     getBurstReadoutData(u8  *data, int *lost_rows, bool parallel);
    int     checkLostRows(int lost_rows);
    void    reportBurstError(u32 row_cnt, int *lost_rows, bool parallel);
    int     processBurstReadoutMsg(mpixd_reply_msg *mpxr, u8 *data, u32 *row_cnt, bool *ack_recvd, bool *first_row_reported);
    int     processBurstReadoutRowBytesMsg(mpixd_reply_msg *mpxr, u8 *data, u32 *row_cnt, bool *first_row_reported);
    void    processBurstReadoutRowBytesData(mpixd_reply_msg *mpxr, u8 *data, u32 *row_cnt, bool *first_row_reported, int multiplier);
    int     sendReadoutCmd  (int chipnr);
    int     readRow         ( u32 row, u8 *data );
    int     writeRow        ( int row, u8 *data, u32 sz,
                              int cmd = MPIXD_SET_MATRIX  );

    int     setBurstReadout ( bool burst );
    int     selectChip      ( int chipnr );
    int     selectChipAll   ();
    int     writeFsr        ( Fsr &fsr );
    void    getHwInfoVCParam(int nParam);
    void    getHwInfoBool   (bool bParam);
    int     getHwInfoTpxClock();
    int     getHwInfoRoClock125();
    int     getHwInfoTpxPreClocks();
    int     getHwInfoTrigType();
    int     getLostRowRetryCnt();

    int     updateHwInfo    (int index, HwInfoItem *hw_item, int *sz);

    int     setHwInfoTestPulseHiLo(HwInfoItem *info, int testPulse);
    int     setHwInfoTestPulseFreq(HwInfoItem *info);
    int     setHwInfoBiasVoltageAdjust(HwInfoItem *info);
    int     setHwInfoVddAdjust(HwInfoItem *info);
    void    setHwInfoFirstChipNr(HwInfoItem *info);
    void    setHwInfoBool(HwInfoItem *info, bool *bParam, const char* dbg);
    int     setHwInfoStoreDacs(HwInfoItem *info);
    int     setHwInfoEraseStoredCfg(HwInfoItem *info);
    int     setHwInfoConfTpxClock(HwInfoItem *info);
    int     setHwInfoTrigType(HwInfoItem *info);
    int     setLostRowRetryCnt(HwInfoItem *info);
    void    setHwInfoConfigRegBits(HwInfoItem *info, u32 bits);

    void    setFrsMxr(Fsr *fsr, int *dac_i, u8  sense_chip, u8  ext_dac_chip, int codes[], u32 col_testpulse_reg);
    void    setFrsTpx(Fsr *fsr, int *dac_i, u8  sense_chip, u8  ext_dac_chip, int codes[], u32 col_testpulse_reg, int chipnr);
    int     saveFrsResults(Fsr *fsr, int chipnr);
    void    logFsrChange(Fsr *fsr, int chipnr);
    void    logChipIdWafer();
    int     toggleTimerUsedBit(const u32 configReg);

public:
    /*! \brief  Opens or closes the shutter.
    
        Opens or closes the Relaxd shutter under software control. Note
        that opening and closing the shutter under software control does
        not give a good control over the timing and should only be used
        for debugging or very long exposures where timing is less important.

        \param  open Whether to open or close.

        \returns    0 on success.
     */

    int     openShutter     ( bool open );

    /*! \brief  Opens the shutter.
    
        Opens shutter under software control. Invokes openShutter(true);.

        \returns    0 on success.
     */
    int     openShutter     () { return openShutter( true ); }

    /*! \brief  Closes the shutter.
    
        Closes shutter under software control. Invokes openShutter(false);.

        \returns    0 on success.
     */
    int     closeShutter    () { return openShutter( false ); }
    
    /*! \brief  Resets all medipix chips
    
        Applies a reset signal to connected Medipix devices. All device
        and pixel configurations revert to power-up/reset values.
        
        \returns    0 on success.
     */
    int     resetChips      ();
    
    /*! \brief  Starts or stops the Relaxd test pulser circuit.

        A sequence of test pulse generation would consist of calls to
        configTestPulse, testPulseEnable(true), openShutter, then polling using
        timerExpired, followed by testPulseEnable(false) and finally by a frame
        read-out, respectively.

        \param  enable  Whether to enable or disable.

        \returns    0 on success.    
    */
    int     testPulseEnable ( bool enable );
    
    /*! \brief  Gets the values of the last DAC settings.
    
        Gets the values of the last DAC settings of a particular Medipix device,
        copies of which are kept by the Relaxd module (so not read back from the
        device itself). The values are either returned in their ‘raw’ value in fsr
        (as a string of bits called FSR).
        
        \param  chipnr      Medipix device number [0-3].
        \param  fsr_bytes   Array containing ‘FSR’ bytes containing the various
                            Medipix DAC settings (among a few other settings).
        
        \returns    0 on success.
    */
    int     readFsr         ( int chipnr, u8 *fsr_bytes );

    /*! \brief  Gets the values of the last DAC settings.
    
        Gets the values of the last DAC settings of a particular Medipix device,
        copies of which are kept by the Relaxd module (so not read back from the
        device itself). The values are returned in actual DAC values in
        dacs (the number of values returned in sz). The order of DACs in dacs can
        be found in include file common.h. 
        
        \param  chipnr      Medipix device number [0-3].
        \param  dacs        Array containing the various Medipix DAC settings.
        \param  sz          The number of values returned in dacs (depends on
                            Medipix device type).
        
        \returns    0 on success.
    */
    int     readFsr         ( int chipnr, int *dacs, int *sz );
    
    /*! \brief  Reads the Medipix device (24-bits) identifier from the Relaxd module.
    
        The identifier is obtained each time new DAC settings are uploaded to the
        Medipix device. An identifier can be read immediately after power-up as the
        Relaxd module uploads default settings (or stored settings if available).
    
        \param      chipnr  Medipix device number [0-3].
        \param      id      Returned chip identifier.
    
        \returns    0 on success.
    */
    int     readChipId      ( int chipnr, u32 *id );

    /*! \brief  Reads a temperature.
    
        Reads the Medipix quad-board temperature sensor (t_nr = 0) or the Relaxd
        module FPGA die temperature (t_nr = 1).
        
        \param  t_nr        Temperature sensor number [0-1].
        \param  degrees     Returned temperature in degrees centigrade.
        
        \returns    0 on success.
    */
    int     readTsensor     ( int t_nr, u32 *degrees );
    
    /*! \brief  Reads an Relaxd ADC.
    
        Reads an input of the Relaxd module’s ADC and returns the reading.
        
        Relaxd has the following ADCs:
        
        -0.      Read back DAC chip 1 - DAC depends on register
        -1.      Read back DAC chip 2 - DAC depends on register
        -2.      Read back DAC chip 3 - DAC depends on register
        -3.      Read back DAC chip 4 - DAC depends on register
        -4.      Current A opamp
        -5.      Current D opamp
        -6.      Voltage VDA
        -7.      Voltage VDD
        
        \param  adc_nr  The ADC number [0-7].
        \param  value   The read-back value.
        
        \return     0 on success.
        
     */
    int     readAdc         ( int adc_nr, u32 *value );
    
    /*! \brief  Sets an value of one of the Relaxd DACs.
    
        Sets an output of the Relaxd module’s Note that these are not the same DACs
        as those on the Medipix or Timepix chip, which are set by setFsr ().

        Relaxd has the following DACs:        
        
        -0-3.    Unused
        -4.      Bias trim (25-150V)
        -5.      Bias digital supply (0.8 – 2.2V)
        -6.      Test pulse low (0 – 1.2V)
        -7.      Test pulse high (0 – 1.2V)

        \param  dac _nr     DAC output number [0-7].
        \param  value       The DAC value to set.

        \returns    0 on success.
    */
    int     writeDac        ( int dac_nr, u32 value );

    /*! \brief Returns the current firmware version and board revision.
     
        Reads the firmware software version from the Relaxd module.
        This version should be interpreted as decimal, where the least
        significant number is the patch number, the second to the least
        is sub-version number, and all numbers above are version numbers.
        E.g. the decimal number 142 should be interpreted as 1.4.2

        \param   version	The version.
		\param   boardrev	The Relaxd modules board version, optional.

        \returns 0 on success. 

     */
    int     getFirmwareVersion   ( u32 *version, u8 *boardrev = NULL );

    /*! \brief Returns the current driver version.
     
        Gets the 'driver' version. 

        This version should be interpreted as decimal, where the least
        significant number is the patch number, the second to the least
        is sub-version number, and all numbers above are version numbers.
        E.g. the decimal number 142 should be interpreted as 1.4.2

        \returns    The version

     */
    int     getDriverVersion ( );
    /*! \brief This function configures the test pulse.
    
        Used for testing the Medipix or Timepix chips.

        \param  low         The low voltage
        \param  high        The high voltage
        \param  freq_05khz  The frequency
        \param  count       The approximate nr of pulses to give

        \returns    0 on success.
    */
    int     configTestPulse ( u32 low, u32 high, u32 freq_05khz, u32 count );

    /*! \brief  Write a Relaxd module register.

        Writes a register. The register contents depends on the offset.

        The most important registers are the Configuration Register, and
        the Status Register.

        <table>
        <tr> <th> Offset </th> <th> Description            </th>     <th> R/W </th> </tr>
        <tr> <td> 0x008  </td> <td> Status Register        </td>     <td> R/W </td> </tr>
        <tr> <td> 0x00C  </td> <td> Configuration Register </td>     <td> R/W </td> </tr>
        <tr> <td> 0x010  </td> <td> Timer Register         </td>     <td> R/W </td> </tr>
        <tr> <td> 0x014  </td> <td> Test Register          </td>     <td> R/W </td> </tr>
        </table>


        For the Status Register see the MPIX2_STATUS_* constants
        For the Configuration Register see the MPIX2_CONF_* constants.

        For more info see the Relaxd programmers guide.

        \param  offset  The register offset
        \param  value   The value to set the register to

        \returns 0 on success
    */
    int     writeReg        ( u32 offset, u32 value );

    /*! \brief  Read a Relaxd module register.

        For the Status Register see the MPIX2_STATUS_* constants
        For the Configuration Register see the MPIX2_CONF_* constants.

        For more info see the Relaxd programmers guide.

        \see readReg for a brief overview of some of the registers.

        \param  offset  The register offset
        \param  value   The register contents will end up in this variable.

        \returns 0 on success
    */
    int     readReg         ( u32 offset, u32 *value );

    /*! \brief   Reads, modifies and writes a register.

        This convenience function reads a register,
        modifies the masked bits, and writes the result.

        \param      offset      The register offset
        \param      value       The value to set

        \returns    0 on success.
     */
    int     rmwReg          ( u32 offset, u32 value, u32 mask );

    /*! \brief      Sets the ID of this module.

        Sets the ID, or index of the module. When connecting several Relaxd modules,
        each module must have a unique ID. The ID defines the IP address and
        MAC address of the module, in the following manner:

        - RelaxD IP Address : 192.168.(33 + ID).175
        - RelaxD gateway : 192.168.(33 + ID).1
        - RelaxD MAC address : 02:4D:50:49:58:( 30 + ID )

        Note that this is the default public MAC address, and may be changed for
        commercial applications.

        \param      id  The new module ID, 0..31

        \returns    0 on success
        
    */
    int     storeId         ( u32 id );

    /*! \brief  Stores the last-set DAC setting in EEPROM.

        Stores the DACs in the EEPROM. Must be done after setting the DACs.
        After restart of the Relaxd module these DAC settings will be
        loaded again.

        \returns    0 on success.
    */
    int     storeDacs       ();

    /*! \brief  Erases DAC settings from EEPROM.

        Erases the DACs in the EEPROM. After invoking this function the DAC settings
        will no longer be loaded from EEPROM, and the defaults of the chip will be set.

        Note that there is no way to read the chip-defaults. Trying to will return all
        DAC settings as max.
        
        \retruns    0 on success.        
    */
    int     eraseDacs       ();

    /*! \brief  Store pixel config in EEPROM.
    
        Indicates to the driver to store the pixel configuration
        on next invocation of setPixelCfg.

        The pixel configuration will be loaded on device start up.
    */
    void    storePixelsCfg  () { _storePixelsCfg = true; };
    
    /*! \brief  Erase last stored pixel configuration.

        Erases the current pixel configuration from EEPROM. The pixel configuration will
        no longer be loaded on device start up.

        \returns    0 on success.
    */
    int     erasePixelsCfg  ();

    /*! \brief      (Re)load configuration from EEPROM.

        Loads the stored pixel configuration, and chip DAC settings from EEPROM

        \returns 0 on success.
    */
    int     loadConfig      ();

    int     rxMsg           (struct mpixd_reply_msg **rx_msg, const std::vector<mpixd_type> &msgs);

private:

    int     txRxMsg         ( struct mpixd_msg       *tx_msg,
                              int                     tx_len,
                              struct mpixd_reply_msg **rx_msg );
    int     txRxMsg         ( MpxSocketMgr           *sockMgr,
                              struct mpixd_msg       *tx_msg,
                              int                     tx_len,
                              struct mpixd_reply_msg **rx_msg );
    int     txMsg           ( MpxSocketMgr           *sockMgr,
                              struct mpixd_msg       *tx_msg,
                              int                     tx_len );
    int     rxMsgMon        ( struct mpixd_reply_msg **rx_msg, const std::vector<mpixd_type> &msgs);
    int     txRxMsgMon      ( struct mpixd_msg       *tx_msg,
                              int                     tx_len,
                              struct mpixd_reply_msg **rx_msg );

	void    initSocket();
    int     readRegHelper( u32 offset, u32 *value);

public:

    /*! \brief  Returns the number of medipix chips

        \returns    the number of chips connected to this Relaxd module
    */
    int     chipCount       ();

    /*! \brief      Returns the chip index based on the chipNr

        Returns the real chip index on the chipnr. When there is no chip
        connected to the first Relaxd port, the first chip is actually nr 1.
        However, when you specify an offset when creating the MpxModule class,
        this offset becomes 0. The chipmap translates this offset into the real
        chip position. Note that most function use this mapping underneath,
        so you do not need to call it explicitly.

        \param  chipnr  The chip to get the index for

        \return The real chip index on the relaxd module

    */
    int     chipPosition    ( int chipnr );

    /*! \brief      Returns the chip type.

        The type may either MPX_MXR or MPX_TPX. Actually multi-chip configurations
        are not supported for a single relaxd-board. Still this function should return
        each chip-type correctly, even if there are multiple chips.
        
        \param  chipnr      The chip to get the type off.
        
        \returns    The chip type, see MPX_MXR and MPX_TPX macros.
     */
    int     chipType        ( int chipnr );

    /*! \brief  Returns the human-friendly ID.
    
        Return a string with syntax "Yxx-Wwwww-TTT", based on wafer number and X/Y position,
        with Y=[A..M] (mapped from Y[1:13]), xx=[11..01] (mapped from X[1::11]) and wwwww
        the wafer number preceeded by zeroes, the TTT indicates the chip type (TPX or MXR).

        \param  chipnr The chip to get the name off.

        \retruns    Returns the human-friendly ID of the chip.

    */
    std::string  chipName        ( int chipnr );

    /*! \brief  Open a data-file.

        Opens a data file for writing event data. Must be used together with storeFrame()
        and closeDataFile().

        \param  filename The filename to store the text data in.

        \returns    0 on success.
    */
    int     openDataFile    ( std::string filename );

    /*! \brief  Closes the data file

        Closes the data file.

        \see    openDataFile
    */
    void    closeDataFile   ();

    /*! \brief  Stores a frame in the opened data file.

        \see    openDataFile

        \param  info    Additinal information to be stored.

        \returns    0 on success.
    */
    int     storeFrame      ( std::string info = "" );

    /*! \brief  Sets verboseness of logging.

        Indicates how verbose the logging should be. Setting verbose to false
        will only display errors and some additional initialisation info. When
        setting to verbose detailed logging information will be written to the
        logging file. The log file is created in the current working directory.

        \param      b     Should logging be verbose.
    */
    void    setLogVerbose   ( bool b ) { _logVerbose = b; };

    /*! \brief  Returns wether or not verbose logging is on.

        \see setLogVerbose

        \returns    whether or not the logging is verbose.
    */
    bool    logVerbose      () { return _logVerbose; };

    /*! \brief  Sets parallel read-out.

        Parallel read out is a fast read out method for Relaxd modules with a quad-chip
        configuration. It allows the relaxd-module to read a quad chip configuration as
        fast as a single chip. Using this together with setHighSpeedReadout should allow
        for +120 fps.
        
        Note that this setting only has influence on quad chip configurations.

        \param      b     Should logging be verbose.
    */
    void    setParReadout   ( bool b ) { _parReadout = b; };

    /*! \brief  Returns wether or not parallel readout is used.

        \see setParReadout

        \returns    whether or parallel readout is enabled.
    */
    bool    parReadout      () { return _parReadout; };
    
    /*! \brief  Sets high speed readout.

        Sets high-speed read out mode. Should roughly double read out speed.
        Thoug this allows for faster read-out, may introduce some instabilities when using
        longer cables between the relaxd-module and the medipix chip(s).

        \param      b   true for highspeed, false for normal speed.

        \returns    0 on sucess.
    */
    int     setHighSpeedReadout ( bool b );

    /*! \brief  Returns whether or not it is in high-speed read out mode.

        \see setHighSpeedReadout

        \returns    true if in highspeed read-out mode, false otherwise.
    */
    bool    highSpeedReadout();

    
    /*! \brief   Decodes the raw pixel data received from the Relaxd module.
    
        Decodes raw image data and and saves it into the array pixels. Note that
        pixels must atleast contain as many elements as the sensor size, e.g.
        for a QUAD this is 512 * 512 pixels, for a single 256 * 256. 
        
        \see readMatrixRaw
        \see chipCount

        \param  bytes   The raw data from the Relaxd module. 
        \param  pixels  The pixel counts as values from -16383 to -1 (error) and
                        0 to 11810 (actual count)

    */
    void    decode2Pixels   ( u8 *bytes, i16 *pixels );

    /*! \brief  Sends a ping to the module.

        Tries a ping-pong. If no pong is received, this returns false. May also indicate an
        incorretly configured network card.
        
        \returns    true is the module has replied, false otherwise.
    */
    bool    ping            ();

    /* Module Framecounter and clock utils */

    /*! \brief  Returns the last frame clock

        Returns the clock tick belong to the last received frame. The clock time is the time
        beloning to the moment the shutter was opened for this frame. The clock ticks are 
        10us per increment, i.e. to get the value in microseconds multiply by 10. The clock
        is not related to any wall-clock time, and counts from the beginning of device boot.
        The clock will overflow each 11 hours or so and start back at 0.

        \return     The last clock tick. 
    */
    u32     lastClockTick()          { return _lastClockTick; };

    /*! \brief  Returns the last time stamp

        This function is identical to the lastClockTick, but returns the time in seconds.

        \see    lastClockTick

        \return     The last clock tick in seconds.
    */
    double  lastTimeStamp()          { return static_cast<double>( _lastClockTick ) / 100000.0; };

    /*! \brief  Returns the last frame count
        
        Returns the relaxd modules frame count belonging to the last received frame. Frame count 
        will overflow after 65536 frames and start back at 0.
    */
    u16     lastFrameCount()         { return _lastFrameCount; };

    /*! \brief  Resets the modules frame counter to 0.

        Resets the modules frame counter.

        \returns    0 on success.
    */
    int     resetFrameCounter();

    /*!	\brief Checks whether a new frame is available for read out.
	 * New frame only works with
     *
     * \param   check       If true, it will check the socket for new
     *                      messages, and if so, check if it is a new
     *                      frame message.
     *
     * \return              Whether or not a new frame is available.
     */
    bool    newFrame        (bool check = true);


	/*! \brief Read a page from the  firmware flash. 
	 *
	 * This method is used for the flashprog application and should
	 * not be used.
	 * 
	 * \param	page	The page to read (0..4095)
	 * \param	bytes	The array to write the page into (256 bytes).
	 *
	 * \returns    0 on success.
	 */
	int		readFirmware	(int page, u8 * bytes);


	/*! \brief Write a page to the firmware flash. 
	 *
	 * This method is used for the flashprog application and should
	 * not be used. Attempting to use it anyway may brick your relaxd
	 * module.
	 *
	 * The written data is immedately read-back and
	 * put into the bytes parameter, for verification.
	 *
	 * Note that when the page is on the start of a
	 * sector, that sector will be earased before being
	 * written. So if page % 256 = 0, the whole sector
	 * (page to page + 255) is erased. It is assumed that
	 * firmware is written sequentially.
	 * 
	 * \param	page	The page to write (0..4095)
	 * \param	bytes	The page of firmware data to write, and
	 *                  on return contains the written data.
	 *
	 * \returns    0 on success.
	 */
	int		writeFirmware	(int page, u8 * bytes);

    void    stream2Data     ( u8 *stream, i16 *data );
    void    seqStream2Data  ( int ndevs, u8 *stream, i16 *data );
    void    parStream2Data  ( u8 *stream, i16 *data );
    void    mask2Stream     ( i16 *mask, u8 *stream );

//juhe - 2012-11-12 moved among public functions due to flashprog
	void    setSockTimeout  ( bool forever, int ms = 1000 );
private:
    void    checkFirmwareRevision();
    void    setupBoardId    (int id, int ndevices);
    void    setupChipIdMap  (bool remap, int first_chipnr, int ndevices);
    void    initHwInfoItems ();
    void    initFsrs        (bool allocate);
    void    initDevInfo     (int nrows, int ndevices, int mpx_type);
    void    initAccPars     ();
    int     getNumDevices   (int *mpx_type, int *first_chipnr);
    int     getMpxType      (int ndevices);
    int     setNumRows      (int nrows, int ndevices);
    void    logDevicesInfo  (int nrows, int ndevices, int first_chipnr, int mpx_type);
    void    populateIpAddrChar();
    void    populateChipNames();
    void    setupLog(MpxLogger *log);

    bool    validHwInfoIndex( int index );

    int     openSock        ();
    int     openSockMon     ();
    void    closeSock       ();
    void    flushSockInput  ( bool long_timeout = false );

    i16     mxrCfgToI16     ( u8 pixcfg );
    i16     tpxCfgToI16     ( u8 pixcfg );

    void    commError       ( std::string name );
    void    mpxError        ( std::string name );
    void    replyError      ( std::string name, unsigned int reply );
    bool    checkForFrame   (mpixd_reply_msg * mpxr);
	void	setLastError	( std::string lasterr );
    void    initVars        (int id);


    std::string  timestamp       ();

    int        _id;
    DevInfo    _devInfo;
    AcqParams  _acqPars;
    u32        _chipMap[MPIX_MAX_DEVS];// Index to chip number map
    u32        _chipId[MPIX_MAX_DEVS]; // Chip IDs (24 bits)
    std::vector<HwInfoItem> *_hwInfoItems;   // Hardware specific items

    Fsr       *_fsr[MPIX_MAX_DEVS];
    bool       _fsrValid[MPIX_MAX_DEVS];

    char       _ipAddressChar[MPX_MAX_PATH];
    char       _chipNames[128];

    u32        _relaxdFwVersion;       // Relaxd firmware version
    bool       _logVerbose;            // Verbose logging (not just errors)

    u32        _frameCnt;              // software frame counter ( != lastFrameCount )

    INTPTR     _callbackData;          // Provided and used by/for PixelMan
    HwCallback _callback;              // Provided and used by/for PixelMan

#if defined(_WINDLL) || defined(WIN32)
    double     _startTime;             // Start time of acquisition
    double     getTime();
#else
    struct timeval _startTime;         // Start time of acquisition
    struct timeval getTime();
#endif
    double     elapsedTime();

    u32        _ipAddress;             // IP address of device
    u32        _portNumber;            // Port number used for communication
    u32        _biasAdjust;            // Sensor bias voltage
    u32        _vddAdjust;             // Medipix VDD supply adjustment
    float      _testPulseLow;
    float      _testPulseHigh;
    u32        _testPulseFreq;         // Testpulse frequency
    bool       _parReadout;            // Parallel read-out enabled or not
    bool       _storePixelsCfg;        // Indicate to RelaxD to save next config

    u32        _u32ForPixelman;        // For transferring unsigned int HW items
    float      _floatForPixelman;      // For transferring float HW items
    u32        _boolForPixelman;       // Pixelman can't handle 'bool' type;
    // this is a placeholder for several
    int        _lastMpxErr;
    std::string     _lastErrStr;            // Description of last error (string)

	MpxSocketMgrRt *_sockMgr;

    // Socket for monitoring data readout
	MpxSocketMgr *_sockMonMgr;

	// Logging classes
	bool	   _logManaged;	// logger created by this class (default logger)

    // Output stream for (standalone) data storage
    std::ofstream   _fData;
    int        _storedFrames;

    // Frame meta data from Relaxd
    u32         _lastClockTick;         // Relaxd module clock
    u16         _lastFrameCount;        // Relaxd module frame counter

    // Class version
    static const std::string _versionStr;
    static       u32    _version;

    // Whether or not a new frame is available for read-out
    bool        _newFrame;

    void * _curCfg;
    bool _rmNotRequired;
    int _lostRowRetryCnt;
protected:
    void    setInvalidLutZero(BOOL invalidLutZero);
    MpxLogger *_log;
    MpxCodecMgr* _codec;
    bool _hasGaps;
    BOOL _invalidLutZero;
    MpxPimpl *_pimpl;
};

// ----------------------------------------------------------------------------
#endif // MPXMODULE_H


// common errors
#define MPXERR_NOERROR              (0)         // success
#define MPXERR_UNEXPECTED           (-1)        // unexpected error
#define MPXERR_INVALID_PARVAL       (-2)        // invalid param supplied
#define MPXERR_MEMORY_ALLOC         (-3)        // memory allocation error
#define MPXERR_BUFFER_SMALL         (-4)        // supplied buffer is too small
#define MPXERR_FILE_OPENREAD        (-5)        // cannot open file for reading
#define MPXERR_FILE_OPENWRITE       (-6)        // cannot open file for writing
#define MPXERR_FILE_READ            (-7)        // error while reading file
#define MPXERR_FILE_WRITE           (-8)        // error while writing to file
#define MPXERR_FILE_BADDATA         (-9)        // invalid value was read from file
#define MPXERR_LOCK_TIMEOUT         (-10)       // timeout while waiting for synchronization object
#define MPXERR_UNLOCK               (-11)       // failed to unlock sync object (caller is not lock owner)
#define MPXERR_NO_MSG               (-12)       // didnt get a message that we expected

// mpxctrl layer generic errors
#define MPXERR_MPXCTRL_NOTINIT      (-100)      // mpxctrl layer is not initialized
#define MPXERR_NOHWLIBFOUND         (-101)      // no hardware library was found
#define MPXERR_IDINVALID            (-102)      // supplied DEVID is not valid (no assigned Medipix device)
#define MPXERR_LOADHWLIB            (-103)      // failed to load one of HW library
#define MPXERR_HWLIBINIT            (-104)      // initialization of hw library error


// mpxctrl layer device-specific errors
#define MPXERR_MPXDEV_NOTINIT       (-110)      // device is not properly initialized
#define MPXERR_OPENCFG              (-111)      // open cfg file failed
#define MPXERR_READCFG              (-112)      // errors occurred while reading/parsing cfg file
#define MPXERR_HWINFO_SETITEM       (-113)      // setting interface-specific info failed
#define MPXERR_HWINFO_GETITEM       (-114)      // getting interface-specific info failed
#define MPXERR_FRAME_NOTREADY       (-115)      // required frame is not ready 

#define MPXERR_ACQSTART             (-150)      // fail to start acquisition
#define MPXERR_ACQSTOP              (-151)      // fail to stop acquisition
#define MPXERR_ACQABORTED           (-152)      // acq aborted
#define MPXERR_CHECK_BUSY           (-153)      // fail to check busy state
#define MPXERR_READMATRIX           (-154)      // read matrix failed
#define MPXERR_WRITEMATRIX          (-155)      // write matrix failed
#define MPXERR_RESETMATRIX          (-156)      // reset matrix failed
#define MPXERR_TESTPULSES           (-157)      // test pulses sending failed
#define MPXERR_SETMASK              (-158)      // setting mask (pix. configuration) failed
#define MPXERR_READANALOGDAC        (-159)      // failed to sense DAC analog value
#define MPXERR_SETDACS              (-160)      // failed to set DACs
#define MPXERR_CONVSTREAM2DATA      (-161)      // stream to data conversion error
#define MPXERR_CONVDATA2STREAM      (-162)      // data to stream conversion error
#define MPXERR_CONVPSEUDO           (-163)      // error converting pseudorandom counter values
#define MPXERR_SETEXTDAC            (-164)      // failed to set external DAC
#define MPXERR_INVALIDOP            (-165)      // invalid operation (operation cannot be performed)


// frame errors
#define MPXERR_FRAME_NOTEXIST       (-200)      // frame with specified ID does not exist
#define MPXERR_FRAME_NOTINIT        (-201)      // frame was not properly initialized
#define MPXERR_ATTRIB_NOTFOUND      (-202)      // attribute was not found
#define MPXERR_ATTRIB_EXISTS        (-203)      // attribute already exists
#define MPXERR_INVALID_SIZE         (-204)      // invalid size of frame
#define MPXERR_DATAFILE_FORMAT      (-205)      // invalid format of data file
#define MPXERR_DESCFILE_FORMAT      (-206)      // invalid format of description file
#define MPXERR_CLOSE_PROTECT        (-207)      // cannot close frame, frame is protected
#define MPXERR_FILTER_FAILED        (-208)      // filter function failed
#define MPXERR_FRAME_LOST_ROWS      (-209)      // lost row error

// mpxmgr errors
#define MPXERR_PLUGIN_NOTEXIST      (-300)      // referenced plugin is not registered
#define MPXERR_FUNC_ALREADYREG      (-301)      // function is already registered by this plugin
#define MPXERR_FUNC_NOTEXIST        (-302)      // function is not registered by this plugin
#define MPXERR_MENUITEM_ALREADYREG  (-303)      // menu item already exists
#define MPXERR_MENUITEM_NOTEXIST    (-304)      // specified menu item is not registered
#define MPXERR_EVENT_ALREADYREG     (-305)      // event is already registered by this plugin
#define MPXERR_EVENT_NOTEXIST       (-306)      // event is not registered by this plugin
#define MPXERR_FRAMEATTR_ALREADYREG (-307)      // frame attribute template is already registered by this plugin
#define MPXERR_FRAMEATTR_NOTEXIST   (-308)      // frame attribute template with this name is not registered
#define MPXERR_FILTCHAIN_ALREADYREG (-309)      // filter chain is already registered by this plugin
#define MPXERR_FILTCHAIN_NOTEXIST   (-310)      // specified filter chain does not exist
#define MPXERR_FILTERINST_EXIST     (-311)      // instance of filter with specified name already exists

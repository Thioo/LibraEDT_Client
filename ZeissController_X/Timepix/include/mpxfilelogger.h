#ifndef MPXFILELOGGER_H
#define MPXFILELOGGER_H

#include "mpxlogger.h"
#include <fstream>

#define	MPX_LOG_DIR_NAME	    "/.relaxd_logs"
#define MPX_LOG_PREFIX		    "mpx"
#define MPX_LOG_SUFFIX		    "log"
#define MPX_LOG_DIR_ENV_NAME	"MPX_LOG_DIR"
#define ENV_HOME              "HOME"

class MPXMODULE_API MpxFileLogger :
	public MpxLogger
{
private:
	std::ofstream _out;
	std::string homeDir();
	bool dirExists(std::string dir);
	void mkDir(std::string dir);

public:
	MpxFileLogger(const std::string &dir = "", std::string prefix = MPX_LOG_PREFIX, std::string suffix = MPX_LOG_SUFFIX);
	virtual void log(MpxLogLevel level, std::string message);
	virtual ~MpxFileLogger();

private:
	double getTime();
};


#endif

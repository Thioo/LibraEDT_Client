/*
		Copyright 2012 NIKHEF
*/
#ifndef MPXLOGGER_H
#define MPXLOGGER_H


#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include "mpxplatform.h"

/*! 
	Describes the serverity of the log message.
*/
enum MpxLogLevel { 
	MPX_TRACE,	// trace information
	MPX_DEBUG,	// debug information
	MPX_INFO,	// generic information
	MPX_WARN,	// warnings
	MPX_ERROR,	// errors
	MPX_FATAL	// fatal errors 
};

/*!\brief	Base class for all loggers
	This class encaspulates logging functonality. Note that
	is is a mere proxy between the MpxModule and whatever logging
	system is used. It has no filtering capabilities. This should
	be handled by the underlying logging system.
*/
class MPXMODULE_API MpxLogger
{
protected:
	// trim from start
	static std::string& ltrim(std::string& s)
	{
		//s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
			return !std::isspace(ch);
			}));
		return s;
	}

	// trim from end
	static std::string& rtrim(std::string& s)
	{
		// s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
			}).base(), s.end());
		return s;
	}
public:
	// trim from both ends
	static std::string &trim(std::string &s)
	{
		return ltrim(rtrim(s));
	}

	virtual void log(MpxLogLevel level, std::string message) = 0;

	void trace(std::string msg) { this->log(MPX_TRACE, trim(msg)); };
	void debug(std::string msg) { this->log(MPX_DEBUG, trim(msg)); };
	void info(std::string msg)  { this->log(MPX_INFO, trim(msg)); };
	void warn(std::string msg)  { this->log(MPX_WARN, trim(msg)); };
	void error(std::string msg) { this->log(MPX_ERROR, trim(msg)); };

	void logScratch(MpxLogLevel level);

	std::ostringstream scratch;

	
};

#endif
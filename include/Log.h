#ifndef _H_EMULATOR_LOG
#define _H_EMULATOR_LOG

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <fstream>

#include "definitions.h"


class Log : public std::ostream
{
public:
	enum type { NORMAL = 0b1, DEBUG = 0b10, WARNING = 0b100, ERROR = 0b1000, ALL = 0b1111 };

	Log(std::string logPath = "log.txt") : std::ostream(&_buffer), _buffer(std::cout, logPath) { _currentType = type::NORMAL; };

	static bool printConsole;
	static int typeMin;
	static int nbChange;
	static type currentType;

	static bool exitOnWarning;
	static bool exitOnError;

	class LogBuffer : public std::stringbuf
	{
	public:
		LogBuffer(std::ostream& str, std::string logPath) : _output(str)
		{
            _type = type::NORMAL;
            _outFile.open(logPath);
        };

		virtual int sync ( )
		{
			std::string strAdd = str(); str("");
			Log::type oldType = _type;

			Log::nbChange = 0;
			changeType(Log::NORMAL);

			if(strAdd == "") return 0;
			if(oldType == Log::type::DEBUG && !DEBUG_MODE) return 0;
			if(Log::isUniqueType(typeMin) && oldType < Log::typeMin) return 0;
			else if(!Log::isUniqueType(typeMin) && (oldType & typeMin) == 0) return 0;

			_output << "[Log]" << strAdd;
			_outFile << strAdd;
			if(strAdd[strAdd.size()-1] != '\n') {
                _output << std::endl;
                _outFile << std::endl;
            }

			_output.flush();

			if(oldType == WARNING)
			{
				if(exitOnWarning)
					exit(1 + WARNING);
			}
			else if(oldType == ERROR)
			{
				if(exitOnWarning || exitOnWarning)
					exit(1 + ERROR);
			}

			return 0;
		};

		void changeType(Log::type typeLog) { _type = typeLog; };

	private:
		std::ostream& _output;
		std::ofstream _outFile;
		Log::type _type;
	};


	static void print(std::string str, type typeLog = type::NORMAL);
	//static void print(std::stringstream str, type typeLog = type::NORMAL);

	static void saveToFile(std::string filename);

	static std::string getTypeStr(type typeLog);

	static bool isUniqueType(int typeFlag);

	bool changeType(type typeLog);
	type getCurrentType() { return _currentType; };

private:
	LogBuffer _buffer;
	type _currentType;

	void manageType();
};

extern Log slog;
/*
inline Log& operator<<(Log& l, const char* str)
{
	Log::print(std::string(str));
	return l;
}

inline Log& operator<<(Log& l, std::ostream &stream)
{
	std::stringstream ss;
	ss << stream;
	Log::print(ss.str());
	return l;
}*/

class LogNormal { };
class LogDebug { };
class LogWarning { };
class LogError { };

extern LogNormal lnormal;
extern LogDebug ldebug;
extern LogWarning lwarning;
extern LogError lerror;

inline std::string strManipulator(Log::type t)
{
	return Log::getTypeStr(t)+":\t";
}

inline Log& operator<<(Log& l, LogNormal type)
{
	if(Log::nbChange != 0)
		l << std::flush;

	if(!l.changeType(Log::type::NORMAL))
		return l;

	return l;
}
inline Log& operator<<(Log& l, LogDebug t)
{
	if(Log::nbChange != 0)
		l << std::flush;

	if(!l.changeType(Log::type::DEBUG))
		return l;

	l << strManipulator(Log::type::DEBUG);

	return l;
}
inline Log& operator<<(Log& l, LogWarning t)
{
	if(Log::nbChange != 0)
		l << std::flush;

	if(!l.changeType(Log::type::WARNING))
		return l;

	l << strManipulator(Log::type::WARNING);

	return l;
}
inline Log& operator<<(Log& l, LogError t)
{
	if(Log::nbChange != 0)
		l << std::flush;

	if(!l.changeType(Log::type::ERROR))
		return l;

	l << strManipulator(Log::type::ERROR);

	return l;
}

#endif

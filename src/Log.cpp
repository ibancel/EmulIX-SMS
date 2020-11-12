#include <iostream>
#include <cmath>

#include "Log.h"

using namespace std;

bool Log::printConsole = false;
bool Log::printFile = false;
int Log::typeMin = Log::type::ALL;
int Log::nbChange = 0;
bool Log::exitOnWarning = false;
bool Log::exitOnError = false;
Log::type Log::currentType = Log::type::NORMAL;
Log slog;
LogNormal lnormal;
LogDebug ldebug;
LogNotif lnotif;
LogWarning lwarning;
LogError lerror;

void Log::print(string str, type typeLog)
{
	if(printConsole && typeLog)
		cout << getTypeStr(typeLog) << " : " << str << endl;
}

string Log::getTypeStr(type typeLog)
{
	if (typeLog == Log::type::NORMAL) {
		return "Normal";
	}  else if (typeLog == Log::type::DEBUG) {
		return "Debug";
	} else if (typeLog == Log::type::NOTIF) {
		return "Notif";
	} else if (typeLog == Log::type::WARNING) {
		return "Warning";
	} else if (typeLog == Log::type::ERROR) {
		return "Error";
	}

	return "Undefined";
}

bool Log::isUniqueType(s16 typeFlag)
{
	int oneOccurence = 0;
	for (int i = 0; i < 16; i++) {
		if (getBit8(typeFlag, i) == 1u) {
			oneOccurence++;
		}
	}

	if (oneOccurence <= 1) {
		return true;
	}

	return false;
}

bool Log::changeType(type typeLog)
{
	if(_currentType != typeLog || nbChange == 0)
	{
		Log::nbChange++;
		_currentType = typeLog;
		_buffer.changeType(typeLog);

		manageType();

		return true;
	}

	return false;
}


void Log::manageType()
{

}

#include <iostream>
#include <cmath>

#include "Log.h"

using namespace std;

bool Log::printConsole = true;
int Log::typeMin = Log::type::ALL;
int Log::nbChange = 0;
bool Log::exitOnWarning = false;
bool Log::exitOnError = false;
Log::type Log::currentType = Log::type::NORMAL;
Log slog;
LogNormal lnormal;
LogDebug ldebug;
LogWarning lwarning;
LogError lerror;

void Log::print(string str, type typeLog)
{
	if(printConsole && typeLog)
		cout << getTypeStr(typeLog) << " : " << str << endl;
}

string Log::getTypeStr(type typeLog)
{
	if(typeLog == Log::type::NORMAL)
		return "Normal";
	if(typeLog == Log::type::DEBUG)
		return "Debug";
	if(typeLog == Log::type::WARNING)
		return "Warning";
	if(typeLog == Log::type::ERROR)
		return "Error";

	return "Undefined";
}

bool Log::isUniqueType(int typeFlag)
{
	float calc = pow(typeFlag, 0.5f);

    if(calc == floor(calc))
		return true;

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

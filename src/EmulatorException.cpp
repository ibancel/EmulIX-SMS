#include "EmulatorException.h"

EmulatorException::EmulatorException(const std::string& iMsg, const char* iFile, int iLine) :
	std::runtime_error(iMsg)
{
	_message = iMsg + " : " + std::string(iFile) + " # " + std::to_string(iLine);
}

const char* EmulatorException::what() const noexcept
{
	return _message.c_str();
}

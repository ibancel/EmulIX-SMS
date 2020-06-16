#pragma once

#include <stdexcept>
#include <string>

#define EMULATOR_EXCEPTION(msg) EmulatorException(msg, __FILE__, __LINE__)

class EmulatorException : public std::runtime_error
{
public:
	EmulatorException(const std::string& iMsg, const char* iFile, int iLine);
	~EmulatorException() {}

	const char* what() const;

private:
	std::string _message;
};
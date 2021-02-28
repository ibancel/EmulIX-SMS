#pragma once

#include <stdexcept>
#include <string>

#define EMULATOR_EXCEPTION(msg) EmulatorException(msg, __FILE__, __LINE__)

class EmulatorException : public std::runtime_error
{
public:
	EmulatorException(const std::string& iMsg, const char* iFile, int iLine);
	virtual ~EmulatorException() noexcept { }

	virtual const char* what() const noexcept;

private:
	std::string _message;
};

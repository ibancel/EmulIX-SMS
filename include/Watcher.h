#pragma once

#include <cstdint>

class Watcher
{
public:
	Watcher(const uint_fast16_t iAddress);

	void setCurrentValue(uint8_t iValue);

	uint_fast16_t getAddress() const;
	uint8_t getCurrentValue() const;
	
private:
	uint_fast16_t _address;
	uint8_t _currentValue;
};
#pragma once

#include "types.h"
class Watcher
{
public:
	Watcher(const uint_fast16_t iAddress);

	void setCurrentValue(u8 iValue);

	uint_fast16_t getAddress() const;
	u8 getCurrentValue() const;

private:
	uint_fast16_t _address;
	u8 _currentValue;
};

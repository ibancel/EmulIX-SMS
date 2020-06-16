#include "Watcher.h"

#include <cstdint>

Watcher::Watcher(const uint_fast16_t iAddress) : 
	_address{ iAddress }, 
	_currentValue{ 0 }
{

}


void Watcher::setCurrentValue(uint8_t iValue)
{
	_currentValue = iValue;
}

uint_fast16_t Watcher::getAddress() const
{
	return _address;
}

uint8_t Watcher::getCurrentValue() const
{
	return _currentValue;
}
#pragma once

#include <stdexcept>

#include "types.h"

class MemoryBank
{
public:
	MemoryBank() : _data { nullptr }, _size { 0 }, _nullByte { 0 }
	{
		// IMPORTANT: Should an empty bank be filled with 0s?
	}

	MemoryBank(u8* const iBaseAddress, const size_t iSize)
		: _data { iBaseAddress }, _size { iSize }, _nullByte { 0 } { }

	inline u8* const data() { return _data; }

	inline size_t size() const noexcept { return _size; }

	u8& at(const size_t index);
	void removeStart(const size_t nbElementsToRemove);

private:
	u8* _data;
	size_t _size;

	u8 _nullByte;
};

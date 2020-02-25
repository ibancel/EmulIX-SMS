#pragma once

#include <cstdint>
#include <stdexcept>

class MemoryBank
{
public:
	MemoryBank() : _data{ nullptr }, _size{ 0 }, _nullByte{0}
	{
		// IMPORTANT: Should an empty bank be filled with 0s?
	}

	MemoryBank(uint8_t* const iBaseAddress, const size_t iSize) : _data{ iBaseAddress }, _size{ iSize }, _nullByte{ 0 }
	{

	}

	inline uint8_t* const data() {
		return _data;
	}

	inline size_t size() const noexcept {
		return _size;
	}

	uint8_t& at(const size_t index);
	void removeStart(const size_t nbElementsToRemove);

private:
	uint8_t* _data;
	size_t _size;

	uint8_t _nullByte;
};
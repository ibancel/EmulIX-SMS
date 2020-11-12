#include "MemoryBank.h"


u8& MemoryBank::at(const size_t index)
{
	if (index >= _size) {
		//throw std::out_of_range("Index out of range");
		return _nullByte;
	}

	return _data[index];
}

void MemoryBank::removeStart(const size_t nbElementsToRemove) {
	if (nbElementsToRemove < _size) {
		_data += nbElementsToRemove;
		_size -= nbElementsToRemove;
	} else {
		_data = nullptr;
		_size = 0;
	}
}

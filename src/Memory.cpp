#include "Memory.h"

Memory::Memory()
{
	init();
}

void Memory::init()
{
	for(int i = 0 ; i < MEMORY_SIZE ; i++)
		_memory[i] = 0;
}

void Memory::write(uint16_t address, uint8_t value)
{
	_memory[address] = value;
}

uint8_t Memory::read(uint16_t address)
{
	return _memory[address];
}

#ifndef _H_EMULATOR_MEMORY
#define _H_EMULATOR_MEMORY

#include <stdint.h>

#include "definitions.h"


class Memory
{

public:
	Memory();

	void init();

	void write(uint16_t address, uint8_t value);

	uint8_t read(uint16_t address);

private:
	uint8_t _memory[MEMORY_SIZE];

};

#endif

#ifndef _H_EMULATOR_CARTRIDGE
#define _H_EMULATOR_CARTRIDGE

#include <iostream>
#include <vector>

#include "Memory.h"

class Cartridge
{

public:
	Cartridge();

	void readFromFile(std::string filename);
	//void readFromBuffer()

	uint8_t getBlock(int address);
	int getSize();

private:
    std::vector<uint8_t> _memory;

};

#endif

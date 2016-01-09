#ifndef _H_EMULATOR_LOADER
#define _H_EMULATOR_LOADER

#include <iostream>

#include "Memory.h"

class Loader
{

public:
	Loader(Memory *m);

	void readFromFile(std::string filename);
	//void readFromBuffer()

	void setMemory(Memory *m);

private:
    Memory *_memory;

};

#endif

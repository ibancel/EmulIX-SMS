#ifndef _H_EMULATOR_STATS
#define _H_EMULATOR_STATS

#include <iostream>

class Stats
{

public:
	static float opcodeBySec;
	static std::string opcodeArray[256];
	static int opcodeOccur[256];

	static void add(uint8_t prefix, uint8_t opcode);

	static int *getMost();

private:

};

#endif


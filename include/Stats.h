#ifndef _H_EMULATOR_STATS
#define _H_EMULATOR_STATS

#include <iostream>

class Stats
{

public:
	static std::string opcodeArray[256];
	static int opcodeOccur[256];

	static void add(uint8_t prefix, uint8_t opcode);

	static int *getMost();

	static void addExecutionStat(uint8_t numberTStates, long double microsecondExecutionTime);
	static double getCpuExecSpeed();

private:
	static float opcodeBySec;
	static long double cpuExecSpeed;
};

#endif


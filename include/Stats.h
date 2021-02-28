#pragma once

#include <string>

#include "types.h"

class Stats
{

public:
	static std::string opcodeArray[256];
	static int opcodeOccur[256];

	static void add(u8 prefix, u8 opcode);

	static int* getMost();

	static void addExecutionStat(u8 numberTStates, long double microsecondExecutionTime);
	static double getCpuExecSpeed();

private:
	static float opcodeBySec;
	static long double cpuExecSpeed;
};

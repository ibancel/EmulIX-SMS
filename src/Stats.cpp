#include "Stats.h"

#include "CPU.h"
#include "definitions.h"

using namespace std;

float Stats::opcodeBySec = 0.0f;
long double Stats::cpuExecSpeed = 0.0;

int Stats::opcodeOccur[];

void Stats::add(u8 prefix, u8 opcode)
{
	if(prefix == 0)
		Stats::opcodeOccur[opcode]++;
}

int* Stats::getMost()
{
	int arrayCpy[256];
	int* res = new int[256];

	for(int i = 0; i < 256; i++)
		arrayCpy[i] = Stats::opcodeOccur[i];

	for(int n = 0; n < 256; n++) {
		int maxVal = 0;
		int iMax = 0;
		for(int i = 0; i < 256; i++) {
			if(opcodeOccur[i] > maxVal && arrayCpy[i] != -1) {
				maxVal = opcodeOccur[i];
				iMax = i;
			}
		}
		res[n] = iMax;
		arrayCpy[iMax] = -1;
	}

	return res;
}

void Stats::addExecutionStat(u8 numberTStates, long double microsecondExecutionTime)
{
	// if (microsecondExecutionTime <= 0) {
	//	microsecondExecutionTime = numeric_limits<double>::lowest();
	//}
	cpuExecSpeed
		= 0.99995 * cpuExecSpeed + 0.00005 * (microsecondExecutionTime / (numberTStates * CPU::MicrosecondPerState));
}

double Stats::getCpuExecSpeed() { return cpuExecSpeed; }
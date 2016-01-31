#include "Stats.h"

using namespace std;

float Stats::opcodeBySec = 0.0f;
int Stats::opcodeOccur[];

void Stats::add(uint8_t prefix, uint8_t opcode)
{
	if(prefix == 0)
		Stats::opcodeOccur[opcode]++;
}


int* Stats::getMost()
{
	int arrayCpy[256];
	int res[256] = { 0 };

	for(int i = 0 ; i < 256 ; i++)
		arrayCpy[i] = Stats::opcodeOccur[i];

	for(int n = 0 ; n < 256 ; n++)
	{
		int maxVal = 0;
		int iMax = 0;
		for(int i = 0 ; i < 256 ; i++)
		{
			if(opcodeOccur[i] > maxVal && arrayCpy[i] != -1)
			{
				maxVal = opcodeOccur[i];
				iMax = i;
			}
		}
		res[n] = iMax;
		arrayCpy[iMax] = -1;
	}


	return res;
}

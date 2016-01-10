#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "Cartridge.h"

using namespace std;


Cartridge::Cartridge()
{
	_memory.clear();
}

void Cartridge::readFromFile(string filename)
{
	_memory.clear();

	ifstream fichier(filename);

	if(!fichier) exit(EXIT_FAILURE);

	char h;
	uint8_t val;

	while(true)
	{
		fichier.read(&h, sizeof(char));
		val = static_cast<uint8_t>(h);
		//cout << hex << (unsigned int)val << " ";

		if(!fichier.good())
		{
			break;
			fichier.close();
		}

		_memory.push_back(val);
	}
}

uint8_t Cartridge::getBlock(int address)
{
	return _memory[address];
}

int Cartridge::getSize()
{
	return _memory.size();
}

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "Cartridge.h"
#include "Log.h"

using namespace std;


Cartridge::Cartridge()
{
	_data.clear();
}

void Cartridge::readFromFile(string filename)
{
	_data.clear();

	Memory *ram = Memory::instance();

	ifstream file(filename, ios_base::in | ios_base::binary);

	if(!file) {
		slog << lerror << "Cartridge loading failed." << endl;
		exit(EXIT_FAILURE);
	}

	char h;
	uint8_t val;
	uint16_t counter = 0x0;

	while(counter < MEMORY_SIZE)
	{
		file.read(&h, sizeof(char));
		val = static_cast<uint8_t>(h);
		//cout << hex << (unsigned int)val << " ";
		//cout << hex << counter << " : " << (unsigned int)val << endl;

		if(!file.good())
		{
			file.close();
			break;
		}

		// For the moment we just copy data in both memory
		_data.push_back(val);
		ram->write(counter, val);
		counter++;
	}
}

uint8_t Cartridge::getBlock(int address)
{
	return _data[address];
}

int Cartridge::getSize()
{
	return _data.size();
}

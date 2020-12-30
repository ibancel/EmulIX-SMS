#include "Cartridge.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "Log.h"
#include "Memory.h"
#include "System.h"

Cartridge::Cartridge(System& parent) : _system{parent}, _embeddedRam{0}, _header{}
{
	_data.clear();
	_isLoaded = false;
}

void Cartridge::insert(const std::string& filename)
{
	memset(_embeddedRam, 0, Cartridge::RamSize);
    readFromFile(filename);
}

bool Cartridge::isLoaded() const
{
    return _isLoaded;
}

void Cartridge::readFromFile(const std::string& filename)
{
	if (_isLoaded)
	{
        SLOG_THROW(lerror << "A cartridge is already IN!");
	}
	_data.clear();
	
	_isLoaded = false;

	std::ifstream file(filename, std::ios_base::in | std::ios_base::binary);

#if !DEBUG_MODE
	std::cout << "Open cartridge \"" << filename << "\"" << std::endl;
#else
	SLOG(lnotif << "Open cartridge \"" << filename << "\"");
#endif
	if(!file) {
        throw EMULATOR_EXCEPTION("Cartridge loading failed.");
	}

	char h;
	u8 val;
	int counter = 0x0;

	do
	{
		file.read(&h, sizeof(char));
		val = static_cast<u8>(h);
		// cout << hex << (unsigned int)val << " ";
		// SLOG(ldebug << hex << counter << " : " << (unsigned int)val << endl);

		if(!file.good())
		{
			file.close();
			break;
		}

		// For the moment we just copy data in both memory
		_data.push_back(val);
		if (counter < 0x400) {
            _system.getMemory().ptr()->write(counter, val);
		}
		counter++;
	} while (counter > 0); // Test 0 for overflow

	// Data needs to be filled to have block of 16kB banks
	int remainingElements = _data.size() % 0x4000 - 1;
	if (remainingElements > 0) {
		_data.insert(_data.end(), remainingElements, 0);
	}
	SLOG(lnotif << "Cartridge loaded " << std::dec << (static_cast<int>((_data.size() / 1024.0) * 10) / 10.0) << " kB");
#if !DEBUG_MODE
	std::cout << "Cartridge loaded " << std::dec << (static_cast<int>((_data.size() / 1024.0)*10)/10.0) << " kB" << std::endl;
#endif

	_isLoaded = true;
	readHeader();
}

void Cartridge::remove()
{
    memset(_embeddedRam, 0, Cartridge::RamSize);
    _data.clear();
    _header = {};
    _isLoaded = false;
}

u8 Cartridge::getBlock(int address)
{
	if (address < getSize()) {
		return _data.at(address);
	}

	return 0;
}


bool Cartridge::readHeader()
{
	_header = {};

	if (_data.size() == 0) {
		return false;
	}

	std::unique_ptr<u8> headerContent{ new u8[16] };
	bool retrieved = false;
	if (!retrieveHeaderContent(0x1ff0)) {
		if (!retrieveHeaderContent(0x3ff0)) {
			if (!retrieveHeaderContent(0x7ff0)) {
				return false;
			}
		}
	}

	return true;
}

bool Cartridge::retrieveHeaderContent(int iAddress)
{
	if (_data.size() < iAddress + size_t(16)) {
		return false;
	}

	u8 headerBuffer[8] { 0 };
	memcpy(headerBuffer, (_data.data()+iAddress), 8);

	if (static_cast<bool>(memcmp(headerBuffer, "TMR SEGA", 8) == 0)) {
		_header = {};
		_header.checksum = (_data[iAddress + size_t(10)] << 8) + _data[iAddress + size_t(11)];
		// TODO: transform BCD product code to integer
		_header.productCode = (_data[iAddress + size_t(12)] << 8) | _data[iAddress + size_t(13)] | (_data[iAddress + size_t(14)] & 0xF0) << 12;
		_header.version = _data[iAddress + size_t(14)] & 0x0F;
		_header.regionCode = (_data[iAddress + size_t(15)] & 0xF0) >> 4;
		_header.size = _data[iAddress + size_t(15)] & 0x0F;
		return true;
	}

	return false;
}

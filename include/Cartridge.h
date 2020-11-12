#ifndef _H_EMULATOR_CARTRIDGE
#define _H_EMULATOR_CARTRIDGE

#include <iostream>
#include <vector>
#include <algorithm>

#include "Log.h"
#include "Singleton.h"
#include "MemoryBank.h"

class Memory;

struct CartridgeHeader {
	u16 checksum;
	u16 productCode;
	u8 version;
	u8 regionCode;
	u8 size;
};

class Cartridge : public Singleton<Cartridge>
{
public:
	static constexpr u16 RamSize = 0x8000;

	Cartridge();

	void insert(const std::string& filename);
    bool isLoaded() const;
	void readFromFile(const std::string& filename);
    void remove();

	u8 getBlock(int address);

	inline size_t getSize() const noexcept {
		return _data.size();
	}

	inline MemoryBank getBank(int baseAddress) {
		if (baseAddress >= getSize()) {
			//SLOG(lwarning << "No ROM bank at this address (" << std::hex << baseAddress << ")");
			return MemoryBank{};
		}


		return MemoryBank(_data.data() + baseAddress, std::min(0x4000, static_cast<int>(getSize()) - baseAddress));
	}

	inline MemoryBank getRamBank(uint_fast8_t index) {
		if (index >= 2) {
			SLOG(lwarning << "No RAM bank at index " << std::dec << index);
			return MemoryBank{};
		}

		return MemoryBank(_embeddedRam + (static_cast<uintptr_t>(index) * 0x2000), 0x2000);
	}

private:
	bool _isLoaded;
    std::vector<u8> _data;
	u8 _embeddedRam[Cartridge::RamSize];
	CartridgeHeader _header;

	bool readHeader();
	bool retrieveHeaderContent(int iAddress);

};

#endif

#ifndef _H_EMULATOR_MEMORY
#define _H_EMULATOR_MEMORY

#include <cstdint>

#include "definitions.h"
#include "Singleton.h"
#include "Cartridge.h"
#include "Log.h"

class MemoryBank;
class Cartridge;


class Memory : public Singleton<Memory>
{
public:

	enum Paging_Reg { kRamSelect = 0xFFFC, kBank0 = 0xFFFD, kBank1 = 0xFFFE, kBank2 = 0xFFFF };

	Memory();

	void init();

	void write(u16 address, u8 value);

	u8 read(u16 address);

	void dumpRam();

private:
	Cartridge* _cartridge;
	u8 _memory[MEMORY_SIZE]; // Bank 0,1,2 empty here

	// TODO: keep bank by attribute
	MemoryBank _memoryBank0;
	MemoryBank _memoryBank1;
	MemoryBank _memoryBank2;
	MemoryBank _ramBank;

	MemoryBank getBank0();
	MemoryBank getBank1();
	MemoryBank getBank2();

	void switchBank(const int iBankIndex);
	void switchRamBank();
};

#endif

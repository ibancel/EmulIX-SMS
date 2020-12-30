#pragma once

#include <cstdint>

#include "definitions.h"
#include "Log.h"
#include "MemoryBank.h"
#include "System.h"
#include "SystemComponent.h"

class MemoryBank;
class System;

class Memory : public SystemComponent
{
public:

	enum Paging_Reg { kRamSelect = 0xFFFC, kBank0 = 0xFFFD, kBank1 = 0xFFFE, kBank2 = 0xFFFF };

    Memory(System& parent);

	void init();

    virtual void write(u16 address, u8 value) override;

    virtual u8 read(u16 address) override;

	void dumpRam();

private:
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

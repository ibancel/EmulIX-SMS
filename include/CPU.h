#ifndef _H_EMULATOR_CPU
#define _H_EMULATOR_CPU

#include <iostream>
#include <stdint.h>
#include <vector>
#include <bitset>

#include "Memory.h"
#include "Graphics.h"
#include "Audio.h"
#include "Cartridge.h"
#include "Stats.h"

struct resInstruction
{
	bool DBExist = false;
	bool IDExist = false;
};

enum R_NAME { R_B = 0, R_C = 1, R_D = 2, R_E = 3, R_H = 4, R_L = 5, R_A = 7 };
enum RP_NAME { RP_BC = 0, RP_DE = 1, RP_HL = 2, RP_SP = 3 };
enum RP2_NAME { RP2_BC = 0, RP2_DE = 1, RP2_HL = 2, RP2_AF = 3 };
enum F_NAME { F_C = 0, F_N = 1, F_P = 2, F_F3 = 3, F_H = 4, F_F5 = 5, F_Z = 6, F_S = 7 };

class CPU
{

public:
	CPU(Memory *m, Graphics *g, Cartridge *c);

	void cycle();
	resInstruction opcodeExecution(uint8_t prefix, uint8_t opcode);

	void aluOperation(uint8_t index, uint8_t value);
	void rotOperation(uint8_t index, uint8_t reg);

	// rw = true for write and false for read
	uint8_t portCommunication(bool rw, uint8_t address, uint8_t data = 0);

private:
	Memory *_memory;
	Graphics *_graphics;
	Cartridge *_cartridge;
	Audio *_audio;

	uint16_t _pc;
	uint16_t _sp;
	uint8_t _stack[MEMORY_SIZE]; // size unkown so very large size !

	uint8_t _register[REGISTER_SIZE];
	uint8_t _registerA[REGISTER_SIZE]; // alternate
	uint8_t _registerFlag;
	uint8_t _registerFlagA;

	uint8_t _modeInt;


	inline bool isPrefixByte(uint8_t byte);

	void opcode0(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q);
	void opcodeCB(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q);
	void opcodeDD(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q);
	void opcodeED(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q);

	bool condition(uint8_t code);

	void bliOperation(uint8_t x, uint8_t y);

	// swaps:
	void swapRegister(uint8_t code);
	void swapRegisterPair(uint8_t code);
	void swapRegisterPair2(uint8_t code);

	// set:
	void setRegisterPair(uint8_t code, uint16_t value, bool alternate = false);
	void setRegisterPair2(uint8_t code, uint16_t value, bool alternate = false);
	void setFlagBit(F_NAME f, uint8_t value);

	// get:
	uint16_t getRegisterPair(uint8_t code, bool alternate = false);
	uint16_t getRegisterPair2(uint8_t code, bool alternate = false);
	bool getFlagBit(F_NAME f);


};

#endif

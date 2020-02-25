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
#include "Inputs.h"
#include "Stats.h"
#include "Singleton.h"

struct resInstruction
{
	bool DBExist = false;
	bool IDExist = false;
};

enum R_NAME { R_B = 0, R_C = 1, R_D = 2, R_E = 3, R_H = 4, R_L = 5, R_A = 7 };
enum RP_NAME { RP_BC = 0, RP_DE = 1, RP_HL = 2, RP_SP = 3 };
enum RP2_NAME { RP2_BC = 0, RP2_DE = 1, RP2_HL = 2, RP2_AF = 3 };
enum F_NAME { F_C = 0, F_N = 1, F_P = 2, F_F3 = 3, F_H = 4, F_F5 = 5, F_Z = 6, F_S = 7 };

class Graphics;

class CPU : public Singleton<CPU>
{

public:

	CPU();
	CPU(Memory *m, Graphics *g, Cartridge *c);

	void init();
	void reset();

	void cycle();
	resInstruction opcodeExecution(const uint8_t prefix, const uint8_t opcode);

	void aluOperation(const uint8_t index, const uint8_t value);
	void rotOperation(const uint8_t index, const uint8_t reg);

	// rw = true for write and false for read
	uint8_t portCommunication(bool rw, uint8_t address, uint8_t data = 0);


	//** GET **//

	inline uint16_t getProgramCounter() const {
		return _pc;
	}
	inline uint8_t getRegisterFlag() const {
		return _registerFlag;
	}

private:
	Memory *_memory;
	Graphics *_graphics;
	Cartridge *_cartridge;
	Audio *_audio;
	Inputs *_inputs;

	bool _isInitialized;

    // special registers
	uint16_t _pc;
	uint16_t _sp;
	uint8_t _stack[MEMORY_SIZE]; // size unkown so very large size !
	uint16_t _registerR;
	uint16_t _registerI;
	uint16_t _registerIX;
	uint16_t _registerIY;

	uint8_t _register[REGISTER_SIZE];
	uint8_t _registerA[REGISTER_SIZE]; // Alternate register
	uint8_t _registerFlag;
	uint8_t _registerFlagA;
	uint8_t _ioPortControl;

	uint8_t _modeInt;

	bool _IFF1; // Interrupt flip-flop
	bool _IFF2; // Interrupt flip-flop
	bool _halt;
	int _enableInterruptWaiting;

	bool _isBlockInstruction;
	int _useRegisterIX;
	int _useRegisterIY;
	int8_t _cbDisplacement;


	inline bool isPrefixByte(uint8_t byte) const {
		return (byte == 0xCB || byte == 0xDD || byte == 0xED || byte == 0xFD);
	}

	void opcode0(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q);
	void opcodeCB(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q);
	void opcodeED(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q);
	void opcodeDD(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q);
	void opcodeFD(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q);

	bool condition(uint8_t code);

	void bliOperation(uint8_t x, uint8_t y);

	void interrupt(bool nonMaskable = false);

	void restart(uint_fast8_t p);

	inline void useRegisterIX() {
		_useRegisterIX = 2;
	}
	inline void consumeRegisterIX() {
		if (_useRegisterIX > 0) {
			_useRegisterIX--;
		}
	}
	inline void stopRegisterIX() {
		_useRegisterIX = 0;
	}
	inline void useRegisterIY() {
		_useRegisterIY = 2;
	}
	inline void consumeRegisterIY() {
		if (_useRegisterIY > 0) {
			_useRegisterIY--;
		}
	}
	inline void stopRegisterIY() {
		_useRegisterIY = 0;
	}

	// swaps:
	void swapRegister(uint8_t code);
	void swapRegisterPair(uint8_t code);
	void swapRegisterPair2(uint8_t code);

	// set:
	void setRegister(uint8_t code, uint8_t value, bool alternate = false, bool useIndex = true);
	void setRegisterPair(uint8_t code, uint16_t value, bool alternate = false);
	void setRegisterPair2(uint8_t code, uint16_t value, bool alternate = false);
	inline void setFlagBit(F_NAME f, uint8_t value) {
		if (value == 1) {
			_registerFlag |= 1 << (uint8_t)f;
		} else {
			_registerFlag &= ~(1 << (uint8_t)f);
		}
	}

	// get:
	const uint8_t getRegister(uint8_t code, bool alternate = false, bool useIndex = true);
	uint16_t getRegisterPair(uint8_t code, bool alternate = false);
	uint16_t getRegisterPair2(uint8_t code, bool alternate = false);

	inline bool getFlagBit(F_NAME f) const {
		return (_registerFlag >> (uint8_t)f) & 1;
	}


};

#endif

#ifndef _H_EMULATOR_CPU
#define _H_EMULATOR_CPU

#include <bitset>
#include <iostream>
#include <stdint.h>
#include <vector>

#include "Audio.h"
#include "Cartridge.h"
#include "Graphics.h"
#include "Inputs.h"
#include "Memory.h"
#include "Singleton.h"
#include "Stats.h"
#include "System.h"

struct resInstruction {
	bool DBExist = false;
	bool IDExist = false;
};

enum R_NAME { R_B = 0, R_C = 1, R_D = 2, R_E = 3, R_H = 4, R_L = 5, R_A = 7 };
enum RP_NAME { RP_BC = 0, RP_DE = 1, RP_HL = 2, RP_SP = 3 };
enum RP2_NAME { RP2_BC = 0, RP2_DE = 1, RP2_HL = 2, RP2_AF = 3 };
enum F_NAME { F_C = 0, F_N = 1, F_P = 2, F_F3 = 3, F_H = 4, F_F5 = 5, F_Z = 6, F_S = 7 };
enum F_NAME_MASK {
	F_C_MASK = 0x01,
	F_N_MASK = 0x02,
	F_P_MASK = 0x04,
	F_F3_MASK = 0x08,
	F_H_MASK = 0x10,
	F_F5_MASK = 0x20,
	F_Z_MASK = 0x40,
	F_S_MASK = 0x80
};

class Cartridge;
class Graphics;
class Inputs;
class Memory;

class CPU
{

public:
	static constexpr double Frequency = BaseFrequency / 3.0; // in Hz = cycle/s
	static constexpr long double MicrosecondPerState = 1.0 / (Frequency / 1'000'000.0);

	CPU(System& parent);

	void init();
	void reset();

	int cycle();
	int opcodeExecution(const u8 prefix, const u8 opcode);

	void aluOperation(const u8 index, const u8 value);
	void rotOperation(const u8 index, const u8 reg);

	// rw = true for write and false for read
	u8 portCommunication(bool rw, u8 address, u8 data = 0);

	//** GET **//

	inline u16 getProgramCounter() const { return _pc; }
	inline u8 getRegisterFlag() const { return _registerFlag; }

private:
	PtrRef<Memory> _memoryRef;
	PtrRef<Graphics> _graphicsRef;
	PtrRef<Cartridge> _cartridgeRef;
	Memory* _memory;
	Graphics* _graphics;
	Cartridge* _cartridge;
	Audio* _audio;
	Inputs* _inputs;

	bool _isInitialized;

	// special registers
	u16 _pc;
	u16 _sp;
	u16 _registerIX;
	u16 _registerIY;
	u8 _registerR;
	u8 _registerI;
	u8 _registerAluTemp;

	u8 _register[REGISTER_SIZE];
	u8 _registerA[REGISTER_SIZE]; // Alternate register
	u8 _registerFlag;
	u8 _registerFlagA;
	u8 _ioPortControl;

	u8 _modeInt;

	bool _IFF1; // Interrupt flip-flop
	bool _IFF2; // Interrupt flip-flop
	bool _halt;
	int _enableInterruptWaiting;

	bool _isBlockInstruction;
	int _useRegisterIX;
	int _useRegisterIY;
	s8 _cbDisplacement;
	bool _displacementForIndexUsed;
	s8 _displacementForIndex;

	int _cycleCount;

	inline bool isPrefixByte(u8 byte) const { return (byte == 0xCB || byte == 0xDD || byte == 0xED || byte == 0xFD); }

	int opcode0(u8 x, u8 y, u8 z, u8 p, u8 q);
	int opcodeCB(u8 x, u8 y, u8 z, u8 p, u8 q);
	int opcodeED(u8 x, u8 y, u8 z, u8 p, u8 q);
	int opcodeDD(u8 x, u8 y, u8 z, u8 p, u8 q);
	int opcodeFD(u8 x, u8 y, u8 z, u8 p, u8 q);

	bool condition(u8 code);

	int bliOperation(u8 x, u8 y);

	int interrupt(bool nonMaskable = false);

	void restart(uint_fast8_t p);

	void addNbStates(int iNbStates);

	inline void useRegisterIX() { _useRegisterIX = 2; }
	inline void consumeRegisterIX()
	{
		if(_useRegisterIX > 0) {
			_useRegisterIX--;
		}
	}
	inline void stopRegisterIX() { _useRegisterIX = 0; }
	inline void useRegisterIY() { _useRegisterIY = 2; }
	inline void consumeRegisterIY()
	{
		if(_useRegisterIY > 0) {
			_useRegisterIY--;
		}
	}
	inline void stopRegisterIY() { _useRegisterIY = 0; }

	inline bool isIndexUsed() { return (_useRegisterIX > 0) || (_useRegisterIY > 0); }

	u8 readMemoryHL(bool iUseIndex = true);
	void writeMemoryHL(u8 iNewValue, bool iUseIndex = true);
	s8 retrieveIndexDisplacement();
	void incrementRefreshRegister();

	// swaps:
	void swapRegister(u8 code);
	void swapRegisterPair(u8 code);
	void swapRegisterPair2(u8 code);

	// set:
	void setRegister(u8 code, u8 value, bool alternate = false, bool useIndex = true);
	void setRegisterPair(u8 code, u16 value, bool alternate = false, bool useIndex = true);
	void setRegisterPair2(u8 code, u16 value, bool alternate = false, bool useIndex = true);
	inline void setFlagBit(F_NAME f, u8 value)
	{
		if(value != 0) {
			_registerFlag |= 1 << static_cast<uint_fast8_t>(f);
		} else {
			_registerFlag &= ~(1 << static_cast<uint_fast8_t>(f));
		}
	}
	void setFlagMask(u8 mask, bool value)
	{
		if(value) {
			_registerFlag |= mask;
		} else {
			_registerFlag &= ~mask;
		}
	}
	void setFlagUndoc(u8 value)
	{
		setFlagBit(F_F3, value & 0x08);
		setFlagBit(F_F5, value & 0x20);
	}
	void setFlagUndocMethod2(u8 value)
	{
		setFlagBit(F_F3, value & 0x08);
		setFlagBit(F_F5, value & 0x02);
	}

	void setCBRegisterWithCopy(u8 iRegister, u8 iValue);

	// get:
	u8 getRegister(u8 code, bool alternate = false, bool useIndex = true);
	u16 getRegisterPair(u8 code, bool alternate = false, bool useIndex = true) const;
	u16 getRegisterPair2(u8 code, bool alternate = false, bool useIndex = true) const;

	inline u8 getAluTempByte() { return _registerAluTemp + _register[R_A]; }

	inline bool getFlagBit(F_NAME f) const { return (_registerFlag >> (u8)f) & 1; }
};

#endif

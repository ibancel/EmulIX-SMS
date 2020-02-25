#include "CPU.h"

#include <string>

#include <iomanip>
#include <sstream>

using namespace std;


CPU::CPU() : CPU(Memory::Instance(), Graphics::Instance(), Cartridge::Instance())
{

}

CPU::CPU(Memory* m, Graphics* g, Cartridge* c) :
	_IFF1{ false },
	_IFF2{ false },
	_pc{ 0 },
	_sp{ 0 },
	_modeInt{ 0 },
	_halt{ 0 },
	_register{ 0 },
	_stack{ 0 },
	_useRegisterIX{ 0 },
	_useRegisterIY{ 0 }
	
{
	_memory = m;
	_graphics = g;
	_cartridge = c;

	_audio = new Audio(); // for the moment !
	_inputs = Inputs::Instance();

	_isInitialized = false;
}

void CPU::init()
{
	_memory->init();

	_pc = 0x00; // reset (at $0000), IRQs (at $0038) and NMIs (at $0066)
	_sp = 0xFDD0;

	_modeInt = 0;
	_IFF1 = false;
	_IFF2 = false;
	_halt = false;
	_enableInterruptWaiting = 0;

	for(int i = 0 ; i < REGISTER_SIZE ; i++)
		_register[i] = 0;

	for(int i = 0 ; i < REGISTER_SIZE ; i++)
		_registerA[i] = 0;

	for(int i = 0 ; i < MEMORY_SIZE ; i++)
		_stack[i] = 0;

	_registerFlag = 0xFF;
	_registerFlagA = 0xFF;
	_registerIX = 0;
	_registerIY = 0;
	_registerI = 0;
	_registerR = 0;
	_ioPortControl = 0;

	stopRegisterIX();
	stopRegisterIY();

	_isBlockInstruction = false;


	// HACK FOR CERTAIN ROMS THAT NEED BIOS
	setRegisterPair2(RP2_AF, 0xFFFF);
	setRegisterPair(RP_SP, 0xDFEF);

	// HACK BIOS
	//setRegister(R_A, 0xFF);
	//_sp = 0xDFF0;

	_isInitialized = true;
}

void CPU::reset()
{
    _pc = 0;
    _IFF1 = false;
    _IFF2 = false;
	_halt = false;
	_enableInterruptWaiting = 0;
	_registerIX = 0;
	_registerIY = 0;
	stopRegisterIX();
	stopRegisterIY();
    _registerI = 0;
    _registerR = 0;
    _modeInt = 0;
    _registerFlag = 0xFF;
	_isBlockInstruction = false;
	_ioPortControl = 0;
}

void CPU::cycle()
{
	if (!_isInitialized) {
		SLOG_THROW(lerror << "CPU not initialized");
		return;
	}

	if(_enableInterruptWaiting == 0 && !_isBlockInstruction && _graphics->getIE()) {
		interrupt();
	}

	if (_enableInterruptWaiting > 0) {
		_enableInterruptWaiting--;
	}

	if (_halt) {
		return;
	}

	SLOG(ldebug << hex << "HL:" << getRegisterPair(RP_HL) << " SP:" << (uint16_t)(_sp) << " Stack:" << (uint16_t)_stack[_sp+1] << "," << (uint16_t)_stack[_sp] << endl);

	uint8_t prefix = _memory->read(_pc++);
	uint8_t opcode = prefix;

	if(prefix == 0xCB || prefix == 0xDD || prefix == 0xED || prefix == 0xED || prefix == 0xFD)
	{
		opcode = _memory->read(_pc++);
	}
	else
		prefix = 0;

	opcodeExecution(prefix, opcode);

	//_audio->run(); // to put in main loop
}

resInstruction CPU::opcodeExecution(const uint8_t prefix, const uint8_t opcode)
{
	SLOG(lnormal << hex << std::setfill('0') << std::uppercase
		<< "#" << (uint16_t)(_pc - 1 - (prefix != 0 ? 1 : 0)) << " "
		<< std::setw(4) << getRegisterPair2(RP2_AF) << " "
		<< std::setw(4) << getRegisterPair(RP_BC) << " "
		<< std::setw(4) << getRegisterPair(RP_DE) << " "
		<< std::setw(4) << getRegisterPair(RP_HL) << " "
		<< std::setw(4) << getRegisterPair(RP_SP) << " "
		<< endl);
	SLOG_NOENDL(ldebug << hex <<  "#" << (uint16_t)(_pc-1-(prefix!=0?1:0)) << " : " << (uint16_t) opcode);
	if (prefix != 0) {
		SLOG_NOENDL("(" << (uint16_t)prefix << ")");
	}
	SLOG(ldebug << " --" << getOpcodeName(prefix, opcode) << "--");

	Stats::add(prefix, opcode);

	resInstruction res;

	const uint8_t x = (opcode>>6) & 0b11;
	const uint8_t y = (opcode>>3) & 0b111;
	const uint8_t z = (opcode>>0) & 0b111;
	const uint8_t p = (y >> 1);
	const uint8_t q = y & 0b1;

	if(prefix == 0)
		opcode0(x,y,z,p,q);
	else if(prefix == 0xCB)
		opcodeCB(x,y,z,p,q);
	else if(prefix == 0xED)
		opcodeED(x,y,z,p,q);
    else if(prefix == 0xDD)
		opcodeDD(x,y,z,p,q);
    else if(prefix == 0xFD)
        opcodeFD(x,y,z,p,q);

	consumeRegisterIX();
	consumeRegisterIY();

 	return res; // TODO
}

void CPU::aluOperation(const uint8_t index, const uint8_t value)
{
	if (index == 0) // ADD A
	{
		uint8_t sum = (uint8_t)(_register[R_A] + value);
		setFlagBit(F_S, ((int8_t)(sum) < 0));
		setFlagBit(F_Z, (_register[R_A] == value));
		setFlagBit(F_H, ((_register[R_A] & 0xF) + (value & 0xF) < (_register[R_A] & 0xF)));
		setFlagBit(F_P, ((_register[R_A] >> 7) == (value >> 7) && (value >> 7) != (sum >> 7)));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, (sum < _register[R_A]));
		setFlagBit(F_F3, (sum >> 2) & 1);
		setFlagBit(F_F5, (sum >> 4) & 1);

		_register[R_A] = sum;
	}
	else if (index == 1) // ADC
	{
		// TODO: ADD act the same but with carry = 0
		int8_t registerValue = _register[R_A];
		int8_t addOp = value + getFlagBit(F_C);
		int8_t addOverflow = (value == 0x7F && getFlagBit(F_C));
		int8_t sum = (int8_t)(registerValue + addOp);
		setFlagBit(F_S, (sum < 0));
		setFlagBit(F_Z, (sum == 0));
		setFlagBit(F_H, (((_register[R_A] & 0xF) + (addOp & 0xF)) > 0xF));
		setFlagBit(F_P, addOverflow || (sign8(registerValue) == sign8(addOp) && sign8(addOp) != sign8(sum)));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, addOverflow || (sign8(registerValue) == sign8(addOp) && sign8(addOp) != sign8(sum)));
		setFlagBit(F_F3, (sum >> 2) & 1);
		setFlagBit(F_F5, (sum >> 4) & 1);

		_register[R_A] = sum;
	}
	else if(index == 2) // SUB
	{
	    /// TODO verify overflow
		uint8_t result = (uint8_t)(_register[R_A] - value);
		setFlagBit(F_S, ((int8_t)(result) < 0));
		setFlagBit(F_Z, (_register[R_A] == value));
		setFlagBit(F_H, ((_register[R_A]&0xF) - (value&0xF) < (_register[R_A]&0xF))); // ?
		setFlagBit(F_P, ((_register[R_A]>>7)==(value>>7) && (value>>7)!=(result>>7))); // ?
		setFlagBit(F_N, 1);
		setFlagBit(F_C, (result > _register[R_A])); // ?
		setFlagBit(F_F3, (result>>2)&1);
		setFlagBit(F_F5, (result>>4)&1);

		_register[R_A] = result;
	}
	else if (index == 3) // SBC
	{
		// TODO: SUB act the same but with borrow = 0
		uint8_t registerValue = _register[R_A];
		uint8_t minusOp = value + getFlagBit(F_C);
		uint8_t minusOverflow = value == 0xFF && getFlagBit(F_C);
		uint8_t result = static_cast<uint8_t>(registerValue - minusOp);
		setFlagBit(F_S, ((int8_t)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, minusOverflow || (registerValue&0xF) < (minusOp&0xF)); // NOT GOOD
		setFlagBit(F_P, (sign8(registerValue) != sign8(result)) && (sign8(minusOp) == sign8(result))); // verified

		setFlagBit(F_C, minusOverflow || (result > registerValue));
	}
	else if(index == 4) // AND
    {
        _register[R_A] = (_register[R_A] & value);
        setFlagBit(F_S, ((int8_t)(_register[R_A]) < 0));
        setFlagBit(F_Z, (_register[R_A] == 0));
        setFlagBit(F_H, 1);
        setFlagBit(F_P, nbBitsEven(_register[R_A]));
        setFlagBit(F_N, 0);
        setFlagBit(F_C, 0);
    }
	else if(index == 5) // XOR
	{
		_register[R_A] ^= value;
		setFlagBit(F_S, ((int8_t)(_register[R_A]) < 0));
		setFlagBit(F_Z, (_register[R_A] == 0));
		setFlagBit(F_H, 0);
		setFlagBit(F_P, nbBitsEven(_register[R_A]));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, 0);
		setFlagBit(F_F3, (_register[R_A]>>2)&1);
		setFlagBit(F_F5, (_register[R_A]>>4)&1);
	}
	else if(index == 6) // OR
	{
		_register[R_A] |= value;
		setFlagBit(F_S, ((int8_t)(_register[R_A]) < 0));
		setFlagBit(F_Z, (_register[R_A] == 0));
		setFlagBit(F_H, 0);
		setFlagBit(F_P, (_register[R_A]<value));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, 0);
		setFlagBit(F_F3, (_register[R_A]>>2)&1);
		setFlagBit(F_F5, (_register[R_A]>>4)&1);
	}
	else if(index == 7) // CP
	{
		/// TODO vérifier borrows & overflow
		uint8_t sum = _register[R_A] - value;
		setFlagBit(F_S, ((int8_t)(sum) < 0));
		setFlagBit(F_Z, (sum == 0));
		setFlagBit(F_H, ((_register[R_A]&0xF) - (value&0xF) < (_register[R_A]&0xF))); // ?
		setFlagBit(F_P, ((_register[R_A]>>7)==(value>>7) && (value>>7)!=(sum>>7))); // ?
		setFlagBit(F_N, 1);
		setFlagBit(F_C, (sum > _register[R_A])); // ?
		setFlagBit(F_F3, (value>>2)&1);
		setFlagBit(F_F5, (value>>4)&1);

		slog << ldebug << "CP(" << hex << (uint16_t)_register[R_A] << "," << (uint16_t)value << "): " << (uint16_t)(sum == 0) << endl;
	}
	//
	else
		slog << lwarning << "ALU " << (uint16_t)index << " is not implemented" << endl;
}

void CPU::rotOperation(const uint8_t index, const uint8_t reg)
{
	NOT_IMPLEMENTED("NOT USED ANYMORE, see rot[y] r[z]");
}

uint8_t CPU::portCommunication(bool rw, uint8_t address, uint8_t data)
{
	SLOG(ldebug << hex << (!rw ? "<" : "") << "-----------" << (rw ? ">" : "") << " rw:" << rw << " a:" << (uint16_t)address << " d:" << (uint16_t)data << endl);

	if (address == 0x3F)
	{
		if (rw) {
			_ioPortControl = data;
		} else {
			NOT_IMPLEMENTED("READ 0x3F port");
		}
	}
	else if(address == 0xBE || address == 0xBF)
	{
		/// TODO !
		if(rw)
			_graphics->write(address, data);
		else
			return _graphics->read(address);
			//_graphics->read(address, data);

	}
	else if(address == 0x7E || address == 0x7F)
	{
		if(rw)
			_audio->write(address, data);
		else
			return _graphics->read(address);
	}
	else if(address == 0xDE || address == 0xDF)
	{
		/// TODO: fully understand?
		return 0;
		//NOT_IMPLEMENTED("I18n");
	}
	else if(address == 0xDC || address == 0xC0)
    {
        /// TODO verification between Richard's and Marat's docs

        uint8_t returnCode = 0;
        setBit8(&returnCode, 7, !_inputs->controllerKeyPressed(1, CK_DOWN));
        setBit8(&returnCode, 6, !_inputs->controllerKeyPressed(1, CK_UP));
        setBit8(&returnCode, 5, !_inputs->controllerKeyPressed(0, CK_FIREB));
        setBit8(&returnCode, 4, !_inputs->controllerKeyPressed(0, CK_FIREA));
        setBit8(&returnCode, 3, !_inputs->controllerKeyPressed(0, CK_RIGHT));
        setBit8(&returnCode, 2, !_inputs->controllerKeyPressed(0, CK_LEFT));
        setBit8(&returnCode, 1, !_inputs->controllerKeyPressed(0, CK_DOWN));
        setBit8(&returnCode, 0, !_inputs->controllerKeyPressed(0, CK_UP));

        return returnCode;
    }
    else if(address == 0xDD || address == 0xC1)
    {
        uint8_t returnCode = 0;
		if (_ioPortControl != 0xF0) {
#if MACHINE_MODEL == 0
			setBit8(&returnCode, 7, 0);
			setBit8(&returnCode, 6, 0);
#else 
			setBit8(&returnCode, 7, getBit8(_ioPortControl, 7));
			setBit8(&returnCode, 6, getBit8(_ioPortControl, 5));
#endif
		} else {
			// TODO: lightgun
		}
        setBit8(&returnCode, 4, 1); // reset button
        setBit8(&returnCode, 3, !_inputs->controllerKeyPressed(0, CK_FIREB));
        setBit8(&returnCode, 2, !_inputs->controllerKeyPressed(0, CK_FIREA));
        setBit8(&returnCode, 1, !_inputs->controllerKeyPressed(0, CK_RIGHT));
        setBit8(&returnCode, 0, !_inputs->controllerKeyPressed(0, CK_LEFT));

        return returnCode;
    }
	else
	{
		slog << lwarning << hex << "Communication port 0x" << (uint16_t)address << " is not implemented (rw=" << rw << ")" << endl;
	}

	return 0;
}


/// PRIVATE :

void CPU::opcode0(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q)
{
	if(x == 0 && z == 0 && y == 0) // NOP
	{

	}
	else if(x == 0 && z == 0 && y == 1) // EX AF, AF'
	{
		swapRegisterPair2(RP2_AF);
	}
	else if(x == 0 && z == 0 && y == 2) // DJNZ d
	{
		_register[R_B]--;
		uint8_t d = _memory->read(_pc++);

		if(_register[R_B] != 0)
			_pc += (int8_t)d;
	}
	else if(x == 0 && z == 0 && y == 3) // JR d
	{
		_pc += (int8_t)(_memory->read(_pc)) + 1;
	}
	else if(x == 0 && z == 0 && y >= 4) // JR cc[y-4],d
	{
		SLOG(ldebug << "=> Condition " << y-4 << " : " << condition(y-4) << " (d=" << hex << (int16_t)((int8_t)_memory->read(_pc)) << ")");
		int8_t addrOffset = static_cast<int8_t>(_memory->read(_pc++));
		if (condition(y - 4)) {
			_pc += addrOffset;
		}
	}
	//
	else if(x == 0 && z == 1 && q == 0) // LD rp[p],nn
	{
		uint16_t val = _memory->read(_pc++);
		val += (_memory->read(_pc++)<<8);
		setRegisterPair(p, val);
	}
	else if(x == 0 && z == 1 && q == 1) // ADD HL,rp[p]
	{
		uint16_t oldVal = getRegisterPair(RP_HL);
		uint16_t newVal = getRegisterPair(RP_HL)+getRegisterPair(p);

		setFlagBit(F_H, ((oldVal&0xFFF) - (newVal&0xFFF) < (oldVal&0xFFF))); // ?
		setFlagBit(F_N, 0);
		setFlagBit(F_C, (newVal < oldVal));

		setRegisterPair(RP_HL, newVal);
	}
	//
	else if(x == 0 && z == 2 && q == 0 && p == 0) // LD (BC),A
	{
		_memory->write(getRegisterPair(RP_BC), _register[R_A]);
	}
	else if(x == 0 && z == 2 && q == 0 && p == 1) // LD (DE),A
	{
		_memory->write(getRegisterPair(RP_DE), _register[R_A]);
	}
	else if(x == 0 && z == 2 && q == 0 && p == 2) // LD (nn),HL
	{
		uint16_t addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		_memory->write(addr, getRegister(R_L));
		_memory->write(addr+1, getRegister(R_H));
	}
	else if(x == 0 && z == 2 && q == 0 && p == 3) // LD (nn),A
	{
		uint16_t addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		_memory->write(addr, _register[R_A]);
	}
	else if (x == 0 && z == 2 && q == 1 && p == 0) // LD A,(BC)
	{
		setRegister(R_A, _memory->read(getRegisterPair(RP_BC)));
	}
	else if(x == 0 && z == 2 && q == 1 && p == 1) // LD A,(DE)
	{
		_register[R_A] = _memory->read(getRegisterPair(RP_DE));
	}
	else if(x == 0 && z == 2 && q == 1 && p == 2) // LD HL,(nn)
	{
		uint16_t address = _memory->read(_pc++);
		address += (_memory->read(_pc++) << 8);
		setRegister(R_L, _memory->read(address));
		setRegister(R_H, _memory->read(address+1));
	}
	else if(x == 0 && z == 2 && q == 1 && p == 3) // LD A,(nn)
	{
		uint16_t val = _memory->read(_pc++);
		val += (_memory->read(_pc++)<<8);
		_register[R_A] = _memory->read(val);

		slog << ldebug << "addr: " << hex << val << " val: " << (uint16_t)_register[R_A] << endl;
	}
	//
	else if(x == 0 && z == 3 && q == 0) // INC rp[p]
	{
		setRegisterPair(p, getRegisterPair(p)+1);
	}
	else if(x == 0 && z == 3 && q == 1) // DEC rp[p]
	{
		setRegisterPair(p, getRegisterPair(p)-1);
	}
	else if(x == 0 && z == 4) // INC r[y]
	{
		// TODO: test flags
		const uint8_t value = getRegister(y);
		const uint8_t result = (uint8_t)(value + 1);
		setFlagBit(F_S, ((int8_t)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, (value & 0xF) == 0xF);
		setFlagBit(F_P, (value == 0x7F)); // from doc
		setFlagBit(F_N, 0);

		setRegister(y, result);
	}
	else if(x == 0 && z == 5) // DEC r[y]
	{
		// TODO: test flags
		const uint8_t value = getRegister(y);
		const uint8_t result = (uint8_t)(value - 1);
		setFlagBit(F_S, ((int8_t)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, (value & 0b11111) < 1);
		setFlagBit(F_P, (value == 0x80)); // from doc
		setFlagBit(F_N, 1);

		setRegister(y, result);
	}
	else if(x == 0 && z == 6) // LD r[y],n
	{
		setRegister(y, _memory->read(_pc++));
	}
	else if(x == 0 && z == 7 && y == 0) // RLCA
	{
		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		setFlagBit(F_C, (_register[R_A] >> 7));
		_register[R_A] <<= 1;
		_register[0] = getFlagBit(F_C);
	}
	else if(x == 0 && z == 7 && y == 1) // RRCA
    {
        setFlagBit(F_H, 0);
        setFlagBit(F_N, 0);
        uint8_t bit0 = getBit8(_register[R_A], 0);
        _register[R_A] >>= 1;
        setBit8(&_register[R_A], 7, bit0);
        setFlagBit(F_C, bit0);
    }
	else if (x == 0 && z == 7 && y == 2) // RLA
	{
		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		uint8_t carryBit = getFlagBit(F_C);
		setFlagBit(F_C, (_register[R_A] >> 7));
		_register[R_A] <<= 1;
		_register[0] = carryBit;
	}
	else if (x == 0 && z == 7 && y == 3) // RRA
	{
		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		uint_fast8_t bit0 = getBit8(_register[R_A], 0);
		_register[R_A] >>= 1;
		setBit8(&_register[R_A], 7, getFlagBit(F_C));
		setFlagBit(F_C, bit0);
	}
	else if (x == 0 && z == 7 && y == 4) // DAA
	{
		// Block simplified thanks to Eric Haskins
		// https://ehaskins.com/2018-01-30 Z80 DAA/
		uint8_t cBit = getFlagBit(F_C);
		uint8_t hBit = getFlagBit(F_H);
		uint8_t nBit = getFlagBit(F_N);
		uint8_t value = getRegister(R_A);
		uint8_t correction = 0;
		uint8_t newCBit = 0;
		if (hBit || (!nBit && (value & 0xF) > 0x9)) {
			correction = 0x06;
		}
		if (cBit || (!nBit && value > 0x99)) {
			correction |= 0x60;
			newCBit = 1;
		}

		value += (nBit ? -correction : correction);
		setRegister(R_A, value);

		setFlagBit(F_Z, value == 0);
		setFlagBit(F_H, 0); // to verify
		setFlagBit(F_C, newCBit);
		setFlagBit(F_S, getBit8(value, 7));
		setFlagBit(F_P, nbBitsEven(value));
	}
	else if(x == 0 && z == 7 && y == 5) // CPL
	{
		setFlagBit(F_H, 1);
		setFlagBit(F_N, 1);
		_register[R_A] = ~_register[R_A];
	}
	else if (x == 0 && z == 7 && y == 6) // SCF
	{
		setFlagBit(F_C, 1);
		setFlagBit(F_N, 0);
		setFlagBit(F_H, 0);
	}
	else if(x == 0 && z == 7 && y == 7) // CCF
	{
        setFlagBit(F_N, 0);
        setFlagBit(F_C, !getFlagBit(F_C));
	}
	else if(x == 1 && (z != 6 || y != 6)) // LD r[y],r[z]
	{
		setRegister(y, getRegister(z));
	}
	else if(x == 1 && z == 6 && y == 6) // HALT
	{
		_halt = true;
	}
	else if(x == 2) // alu[y] r[z]
	{
		aluOperation(y, getRegister(z));
	}
	else if(x == 3 && z == 0) // RET cc[y]
	{
		bool c = condition(y);
		if(c)
		{
			_pc = _stack[_sp++];
			_pc += (_stack[_sp++]<<8);
		}
		slog << ldebug << hex << "RET(" << (c?"true":"false") << ") to " << (uint16_t)_pc << endl;
	}
	else if(x == 3 && z == 1 && q == 0) // POP rp2[p]
	{
		uint16_t value = _stack[_sp++];
		value += (_stack[_sp++]<<8);
		setRegisterPair2(p, value);
	}
	else if(x == 3 && z == 1 && q == 1 && p == 0) // RET
	{
		uint16_t addr = _stack[_sp++];
		addr += (_stack[_sp++]<<8);
		_pc = addr;
		slog << ldebug << hex << "RET to " << (uint16_t)addr << endl;
	}
	else if(x == 3 && z == 1 && q == 1 && p == 1) // EXX
	{
		swapRegisterPair(RP_BC);
		swapRegisterPair(RP_DE);
		swapRegisterPair(RP_HL);
	}
	else if(x == 3 && z == 1 && q == 1 && p == 2) // JP HL
    {
        _pc = getRegisterPair(RP_HL);
        slog << ldebug << hex << "JP to " << (uint16_t)_pc << endl;
    }
	else if(x == 3 && z == 1 && q == 1 && p == 3) // LD SP, HL
	{
		_sp = getRegisterPair(RP_HL);
		slog << ldebug << hex << "SP: " << _sp << endl;
	}
	else if(x == 3 && z == 2) // JP cc[y],nn
	{
		uint16_t addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		if(condition(y))
			_pc = addr;
		slog << ldebug << "JP C,nn : " << (condition(y)?"true":"false") << " new addr: " << hex << _pc << endl;
	}
	//
	else if(x == 3 && z == 3 && y == 0) // JP nn
	{
		uint16_t val = _memory->read(_pc++);
		val += (_memory->read(_pc++) << 8);
		_pc = val;
		std::bitset<16> y(_pc);
		slog << ldebug << hex << "val: " << val << endl;
	}
	//
	else if(x == 3 && z == 3 && y == 2) // OUT (n),A
	{
		uint8_t val = _memory->read(_pc++);
		portCommunication(true, val, getRegister(R_A));
		//slog << ldebug << "OUT ====> " << hex << (uint16_t)val << " : " << (uint16_t)_register[R_A] << endl;
	}
	else if(x == 3 && z == 3 && y == 3) // IN A,(n)
	{
		uint8_t addr = _memory->read(_pc++);
		_register[R_A] = portCommunication(false, addr);
	}
	else if(x == 3 && z == 3 && y == 4) // EX (SP), HL
	{
		uint8_t tempL = _memory->read(_sp);
		uint8_t tempH = _memory->read(_sp+1);
		_memory->write(_sp, getRegister(R_L));
		_memory->write(_sp+1, getRegister(R_H));
		setRegister(R_L, tempL);
		setRegister(R_H, tempH);
	}
	else if (x == 3 && z == 3 && y == 5) // EX DE, HL
	{
		stopRegisterIX();
		stopRegisterIY();
		uint16_t temp = getRegisterPair(RP_HL);
		setRegisterPair(RP_HL, getRegisterPair(RP_DE));
		setRegisterPair(RP_DE, temp);
	}
	else if(x == 3 && z == 3 && y == 6) // DI
	{
		_IFF1 = false;
		_IFF2 = false;
	}
	else if(x == 3 && z == 3 && y == 7) // EI
    {
        /// TODO verify if it's working well
        _IFF1 = true;
        _IFF2 = true;
		_enableInterruptWaiting = 1;
    }
	else if(x == 3 && z == 4) // CALL cc[y],nn
	{
		uint16_t addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);

		bool c = condition(y);
		if(c)
		{
			_stack[--_sp] = (_pc >> 8);
			_stack[--_sp] = (_pc & 0xFF);
			_pc = addr;
		}

		slog << ldebug << "CALL(" << (c?"true":"false") << ") to " << hex << _pc << endl;
	}
	else if(x == 3 && z == 5 && q == 0) // PUSH rp2[p]
	{
		uint16_t val = getRegisterPair2(p);
		_stack[--_sp] = (val >> 8);
		_stack[--_sp] = (val & 0xFF);
	}
	//
	else if(x == 3 && z == 5 && q == 1 && p == 0) // CALL nn
	{
		uint16_t newPC = _memory->read(_pc++);
		newPC += (_memory->read(_pc++)<<8);

		_stack[--_sp] = (_pc >> 8);
		_stack[--_sp] = (_pc & 0xFF);

		_pc = newPC;

		slog << ldebug << "CALL to " << hex << newPC << endl;
	}
	//
	else if(x == 3 && z == 6) // alu[y] n
	{
		aluOperation(y, _memory->read(_pc++));
	}
	else if(x == 3 && z == 7) // RST y*8
	{
		restart(y * 8);
	}
	else
	{
		slog << lwarning << "Opcode : " << hex << (uint16_t)((x<<6)+(y<<3)+z) << " is not implemented" << endl;
		slog << ldebug << "# pc: " << hex << _pc << endl;
		//slog << ldebug << "x: " << (uint16_t)x << " | y: " << (uint16_t)y << " | z: " << (uint16_t)z << endl;
	}
}

void CPU::opcodeCB(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q)
{
	if(x == 0) // rot[y] r[z]
	{
		const uint8_t index = y;
		uint8_t value = getRegister(z, false, false);
		if (_useRegisterIX) {
			value = _memory->read(_registerIX + _cbDisplacement);
		} else if (_useRegisterIY) {
			value = _memory->read(_registerIY + _cbDisplacement);
		}

		if (index == 0) // RLC
		{
			setFlagBit(F_C, getBit8(value, 7));
			value <<= 1;
			setBit8(&value, 0, getFlagBit(F_C));
			setFlagBit(F_S, ((int8_t)(value) < 0));
			setFlagBit(F_Z, (value == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(value));
			setFlagBit(F_N, 0);
			setRegister(z, value, false, false);
		}
		//
		else if (index == 2) // RL
		{
			uint8_t valRot = (uint8_t)(value << 1);
			setBit8(&valRot, 0, getFlagBit(F_C));
			setFlagBit(F_S, ((int8_t)(valRot) < 0));
			setFlagBit(F_Z, (valRot == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(valRot));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, (value >> 7) & 1);
			setRegister(z, valRot, false, false);
		}
		else if (index == 3) // RR
		{
			uint8_t valRot = value >> 1;
			setBit8(&valRot, 7, getFlagBit(F_C));
			setFlagBit(F_S, ((int8_t)(valRot) < 0));
			setFlagBit(F_Z, (valRot == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(valRot));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, (value & 1));
			setRegister(z, valRot, false, false);
		}
		else if (index == 4) // SLA
		{
			uint8_t valRot = (uint8_t)(value << 1);
			setFlagBit(F_S, ((int8_t)(valRot) < 0));
			setFlagBit(F_Z, (valRot == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(valRot));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, (value >> 7) & 1);
			setRegister(z, valRot, false, false);
		}
		//
		else {
			slog << lwarning << "ROT " << (uint16_t)index << " is not implemented" << endl;
		}
	}
	else if(x == 1) // BIT y, r[z]
    {
		uint8_t value = getRegister(z, false, false);
		if (_useRegisterIX) {
			value = _memory->read(_registerIX + _cbDisplacement);
		} else if (_useRegisterIY) {
			value = _memory->read(_registerIY + _cbDisplacement);
		}
        setFlagBit(F_H, 1);
        setFlagBit(F_N, 0);
        setFlagBit(F_Z, (getBit8(value, y) == 0));
    }
	else if (x == 2) // RES y, r[z]
	{
		uint8_t value = getRegister(z, false, false);
		if (_useRegisterIX) {
			value = _memory->read(_registerIX + _cbDisplacement);
		} else if (_useRegisterIY) {
			value = _memory->read(_registerIY + _cbDisplacement);
		}

		setBit8(&value, y, 0);
		setRegister(z, value, false, false);
	}
	else if (x == 3) // SET y, r[z]
	{
		uint8_t value = getRegister(z, false, false);
		if (_useRegisterIX) {
			value = _memory->read(_registerIX + _cbDisplacement);
		} else if (_useRegisterIY) {
			value = _memory->read(_registerIY + _cbDisplacement);
		}

		setBit8(&value, y, 1);
		setRegister(z, value, false, false);
	}
	else
	{
		// Actually this should not occurs anymore :)
		slog << lwarning << "Opcode : " << hex << (uint16_t)((x<<6)+(y<<3)+z) << " (CB) is not implemented" << endl;
	}
}

void CPU::opcodeED(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q)
{
	if(x == 0 || x == 3) // NOP
	{

	}
	else if(x == 1 && z == 1 && y == 6) // OUT (C),0
	{
		portCommunication(true, _register[R_C], 0);
	}
	else if(x == 1 && z == 1 && y != 6) // OUT (C),r[y]
	{
		portCommunication(true, _register[R_C], getRegister(y));
	}
	else if(x == 1 && z == 2 && q == 0) // SBC HL, rp[p]
	{
		//uint16_t value = getRegisterPair(p) + getFlagBit(F_C);
		//uint16_t result = (uint16_t)(getRegisterPair(p) - value);
		//setFlagBit(F_S, ((int16_t)(result) < 0));
		//setFlagBit(F_Z, (getRegisterPair(p) == value));
		//setFlagBit(F_H, ((getRegisterPair(p)&0xF) - (value&0xF) < (getRegisterPair(p)&0xF))); // ?
		//setFlagBit(F_P, ((getRegisterPair(p)>>7)==(value>>7) && (value>>7)!=(result>>7))); // ?
		//setFlagBit(F_N, 1);
		//setFlagBit(F_C, (result > getRegisterPair(p))); // ?
		//setFlagBit(F_F3, (result>>2)&1);
		//setFlagBit(F_F5, (result>>4)&1);
		//_register[R_A] = result;

		uint16_t value = getRegisterPair(RP_HL);
		uint16_t minusOp = getRegisterPair(p) + getFlagBit(F_C);
		uint16_t minusOverflow = minusOp == 0xFFFF && getFlagBit(F_C);
		uint16_t result = (uint16_t)(value - minusOp);
		setFlagBit(F_S, ((int16_t)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		//setFlagBit(F_H, ((getRegisterPair(p)&0xF) - (value&0xF) < (getRegisterPair(p)&0xF))); // ?
		setFlagBit(F_H, minusOverflow || (value & 0xFFF) < (minusOp & 0xFFF)); // NOT GOOD
		//setFlagBit(F_P, ((getRegisterPair(p)>>7)==(value>>7) && (value>>7)!=(result>>7))); // ?
		setFlagBit(F_P, (isPositive16(value) != isPositive16(result)) && (isPositive16(result) == isPositive16(minusOp)));
		setFlagBit(F_N, 1);
		//setFlagBit(F_C, (result > getRegisterPair(p))); // ?
		setFlagBit(F_C, minusOverflow || (result > value));
		// setFlagBit(F_F3, (result>>2)&1); //__this is false, should be changed but understand why >>2 and >>4
		// setFlagBit(F_F5, (result>>4)&1); //_/
		setRegisterPair(RP_HL, result);
	}
	//
	else if(x == 1 && z == 3 && q == 0) // LD (nn), rp[p]
    {
        uint16_t registerValue = getRegisterPair(p);
        uint16_t addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		_memory->write(addr, getLowerByte(registerValue));
		_memory->write(addr+1, getHigherByte(registerValue));
    }
    else if(x == 1 && z == 3 && q == 1) // LD rp[p], (nn)
    {
        uint16_t addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		uint16_t registerValue = _memory->read(addr);
		registerValue += (_memory->read(addr)<<8);
		setRegisterPair(p, registerValue);
    }
	//
	else if(x == 1 && z == 5 && y != 1) // RETN
	{
		// TODO: verify
		_IFF1 = _IFF2;

		_pc = _stack[_sp++];
		_pc += (_stack[_sp++]<<8);
		//SLOG_THROW(lerror << "PC: " << _pc << endl);
	}
	else if(x == 1 && z == 5 && y == 1) // RETI
    {
        _pc = _stack[_sp++];
        _pc += (_stack[_sp++]<<8);

		_IFF1 = _IFF2;
        /// TODO complete this with IEO daisy chain

		//SLOG_THROW(lerror << "PC: " << _pc << endl);
    }
	//
	else if(x == 1 && z == 6) // IM im[y]
	{
		if(y == 0)
			_modeInt = 0;
		else if(y == 2)
			_modeInt = 1;
		else if(_modeInt == 3)
			_modeInt = 2;
	}
	//
	else if(x == 1 && z == 7 && y == 4) // RRD
    {
        uint8_t tempLowA = _register[R_A] & 0xF;
        uint8_t tempHighMem = (_memory->read(getRegisterPair(RP_HL)) >> 4) & 0xF;
        _register[R_A] = _memory->read(getRegisterPair(RP_HL)) & 0xF;
        _memory->write(getRegisterPair(RP_HL), tempLowA);
        _memory->write(getRegisterPair(RP_HL), tempHighMem);
        setFlagBit(F_S, ((int8_t)(_register[R_A]) < 0));
        setFlagBit(F_Z, (_register[R_A] == 0));
        setFlagBit(F_H, 0);
        setFlagBit(F_P, nbBitsEven(_register[R_A]));
        setFlagBit(F_N, 0);
    }
	//
	else if(x == 2 && (z > 3 || y < 4)) // NONI & NOP
	{
		/// TODO
	}
	else if(x == 2) //bli[y,z]
	{
		bliOperation(y, z);
	}
	else
	{
		slog << lwarning << "Opcode : " << hex << (uint16_t)((x<<6)+(y<<3)+z) << " (ED) is not implemented" << endl;
	}
}

void CPU::opcodeDD(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q)
{
    uint16_t prefixByte = (x<<6)+(y<<3)+z;

	if(prefixByte == 0xDD || prefixByte == 0xED || prefixByte == 0xFD) // NONI
	{
        /// TODO NONI
		_pc--;
	}
    else if(prefixByte == 0xCB) // DDCB prefix opcode
    {
		useRegisterIX();
		_cbDisplacement = static_cast<int8_t>(_memory->read(_pc++));
		const uint8_t opcode = _memory->read(_pc++);
		const uint8_t x = (opcode >> 6) & 0b11;
		const uint8_t y = (opcode >> 3) & 0b111;
		const uint8_t z = (opcode >> 0) & 0b111;
		const uint8_t p = (y >> 1);
		const uint8_t q = y & 0b1;
		opcodeCB(x, y, z, p, q);
		stopRegisterIX();
    }
	else
	{
		_pc--;
		useRegisterIX();
		// slog << lwarning << "Opcode : " << hex << (uint16_t)((x<<6)+(y<<3)+z) << " (DD) is not implemented" << endl;
	}
}

void CPU::opcodeFD(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q)
{
	uint16_t prefixByte = (x<<6)+(y<<3)+z;

	if(prefixByte == 0xDD || prefixByte == 0xED || prefixByte == 0xFD) // NONI
	{
        /// TODO NONI
		_pc--;
	}
    else if(prefixByte == 0xCB) // DDCB prefix opcode
    {
        /// TODO FDCB
        slog << lwarning << "FDCB prefix not implemented" << endl;
    }
	else
	{
		_pc--;
		useRegisterIY();
		// slog << lwarning << "Opcode : " << hex << (uint16_t)((x<<6)+(y<<3)+z) << " (FD) is not implemented" << endl;
	}
}

bool CPU::condition(uint8_t code)
{
	if(code == 0)
		return !getFlagBit(F_Z);
	else if(code == 1)
		return getFlagBit(F_Z);
	else if(code == 2)
		return !getFlagBit(F_C);
	else if(code == 3)
		return getFlagBit(F_C);

	return false;
}

void CPU::bliOperation(uint8_t x, uint8_t y)
{
	if (x == 4 && y == 0) // LDI
	{
		_memory->write(getRegisterPair(RP_DE), _memory->read(getRegisterPair(RP_HL)));
		setRegisterPair(RP_DE, getRegisterPair(RP_DE) + 1);
		setRegisterPair(RP_HL, getRegisterPair(RP_HL) + 1);
		setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		setFlagBit(F_P, getRegisterPair(RP_BC) != 0);
	}
	//
	else if(x == 6 && y == 0) // LDIR
	{
		/// TODO: test this block
		_memory->write(getRegisterPair(RP_DE), _memory->read(getRegisterPair(RP_HL)));
		setRegisterPair(RP_DE, getRegisterPair(RP_DE)+1);
		setRegisterPair(RP_HL, getRegisterPair(RP_HL)+1);
		setRegisterPair(RP_BC, getRegisterPair(RP_BC)-1);

		if(getRegisterPair(RP_BC) != 0)
			_pc -= 2;

		setFlagBit(F_H, 0);
		setFlagBit(F_P, 0);
		setFlagBit(F_N, 0);
	}
	//
	else if (x == 4 && y == 3) // OUTI
	{
		uint8_t value = _memory->read(getRegisterPair(RP_HL));
		//slog << ldebug << hex << "OUTI : R_C="<< (uint16_t)_register[R_C] << " R_B=" << (uint16_t)_register[R_B] << " RP_HL=" << (uint16_t)getRegisterPair(2) << " val=" << (uint16_t)value << endl;
		portCommunication(true, getRegister(R_C), value);

		setRegisterPair(RP_HL, getRegisterPair(RP_HL) + 1);
		setRegister(R_B, getRegister(R_B) - 1);

		setFlagBit(F_Z, (getRegister(R_B)==0));
		setFlagBit(F_N, 1);

		// From undocumented Z80 document
		uint16_t k = value + getRegister(R_L);
		setFlagBit(F_C, getRegisterPair(RP_HL) + k > 255);
		setFlagBit(F_H, getRegisterPair(RP_HL) + k > 255);
		setFlagBit(F_P, (((getRegisterPair(RP_HL) + k) & 7) ^ getRegister(R_L)) & 0b1);
	}
	//
	else if(x == 6 && y == 3) // OTIR
	{
		uint8_t value = _memory->read(getRegisterPair(RP_HL));
		//slog << ldebug << hex << "OTIR : R_C="<< (uint16_t)_register[R_C] << " R_B=" << (uint16_t)_register[R_B] << " RP_HL=" << (uint16_t)getRegisterPair(2) << " val=" << (uint16_t)value << endl;
		portCommunication(true, _register[R_C], value);

		setRegisterPair(RP_HL, getRegisterPair(RP_HL)+1);
		_register[R_B]--;

		if (_register[R_B] != 0) {
			_pc -= 2;
			_isBlockInstruction = true;
		} else {
			_isBlockInstruction = false;
		}

		setFlagBit(F_Z, 1);
		setFlagBit(F_N, 1);

		// From undocumented Z80 document
		uint8_t k = value + getRegister(R_L);
		setFlagBit(F_C, (k < value || k < getRegister(R_L)));
		setFlagBit(F_H, (k < value || k < getRegister(R_L)));
		setFlagBit(F_P, ((value&7) ^ getRegister(R_L)) & 0b1);
	}
	else
	{
		slog << lwarning << hex << "BLI (" << (uint16_t)x << ", " << (uint16_t)y << ") is not implemented !" << endl;
	}
}

void CPU::interrupt(bool nonMaskable)
{
	if (!nonMaskable && !_IFF1) {
		return;
	}

	_halt = false;

	if (nonMaskable) {
		restart(0x66);
		_IFF1 = false;
	} else {
		if (_modeInt == 0) {
			slog << lwarning << "TODO interrupt mode 0" << endl;
			// TODO: data bus
		} else if (_modeInt == 1) {
			restart(0x38);
		} else if (_modeInt == 2) {
			slog << lwarning << "TODO interrupt mode 2" << endl;
		} else {
			slog << lerror << "Undefined interrupt mode" << endl;
		}

		_IFF1 = false;
		_IFF2 = false;
	}
}

void CPU::restart(uint_fast8_t p)
{
	_stack[--_sp] = (_pc >> 8);
	_stack[--_sp] = (_pc & 0xFF);
	_pc = p;
	slog << ldebug << "RST to " << hex << (uint16_t)(p) << endl;
}

// swap:
void CPU::swapRegister(uint8_t code)
{
	uint8_t temp = _register[code];
	_register[code] = _registerA[code];
	_registerA[code] = temp;
}

// care ! code can only be 0, 1 or 2 ; 3 is not used
void CPU::swapRegisterPair(uint8_t code)
{
	uint16_t temp = getRegisterPair(code);
	setRegisterPair(code, getRegisterPair(code, true));
	setRegisterPair(code, temp, true);
}

void CPU::swapRegisterPair2(uint8_t code)
{
	uint16_t temp = getRegisterPair2(code);
	setRegisterPair2(code, getRegisterPair2(code, true));
	setRegisterPair2(code, temp, true);
}

// set:

void CPU::setRegister(uint8_t code, uint8_t value, bool alternate, bool useIndex)
{
	if (code == 6) {
		// TODO (IX/Y+d) for DD prefix
		_memory->write(getRegisterPair(RP_HL), value);
	} else if (code == R_H) {
		if (!useIndex || (!_useRegisterIX && !_useRegisterIY)) {
			_register[code] = value;
		} else if (_useRegisterIX) {
			setHigherByte(_registerIX, value);
		} else {
			setHigherByte(_registerIY, value);
		}
	} else if (code == R_L) {
		if (!useIndex || (!_useRegisterIX && !_useRegisterIY)) {
			_register[code] = value;
		} else if (_useRegisterIX) {
			setLowerByte(_registerIX, value);
		} else {
			setLowerByte(_registerIY, value);
		}
	} else {
		_register[code] = value;
	}
}

void CPU::setRegisterPair(uint8_t code, uint16_t value, bool alternate)
{
	uint8_t first = (value >> 8);
	uint8_t second = (value & 0xFF);

	uint8_t* r = _register;
	if(alternate)
		r = _registerA;

	switch(code)
	{
		case 0:
			r[R_B] = first;
			r[R_C] = second;
			break;
		case 1:
			r[R_D] = first;
			r[R_E] = second;
			break;
		case 2:
			if (code == 2 && _useRegisterIX) {
				_registerIX = value;
			} else if (code == 2 && _useRegisterIY) {
				_registerIY = value;
			} else {
				r[R_H] = first;
				r[R_L] = second;
			}
			break;
		case 3:
			_sp = value;
			break;
		default:
			break;
	}

	SLOG(ldebug << hex << "set RP[" << (uint16_t)code << "] = " << value);
}

void CPU::setRegisterPair2(uint8_t code, uint16_t value, bool alternate)
{
	uint8_t first = (value >> 8);
	uint8_t second = (value & 0xFF);

	uint8_t* r = _register;
	if(alternate)
		r = _registerA;

	switch(code)
	{
		case 0:
			r[R_B] = first;
			r[R_C] = second;
			break;
		case 1:
			r[R_D] = first;
			r[R_E] = second;
			break;
		case 2:
			if (code == 2 && _useRegisterIX) {
				_registerIX = value;
			} else if (code == 2 && _useRegisterIY) {
				_registerIY = value;
			} else {
				r[R_H] = first;
				r[R_L] = second;
			}
			break;
		case 3:
			r[R_A] = first;
			if(alternate)
				_registerFlagA = second;
			else
				_registerFlag = second;
			break;
		default:
			break;
	}

	SLOG(ldebug << hex << "set RP2[" << (uint16_t)code << "] = " << value);
}


// get:

const uint8_t CPU::getRegister(uint8_t code, bool alternate, bool useIndex)
{
	if (code == 6) {
		return _memory->read(getRegisterPair(RP_HL));
	} else if (code == R_H) {
		if (!useIndex || (!_useRegisterIX && !_useRegisterIY)) {
			return _register[code];
		} else if (_useRegisterIX) {
			return getHigherByte(_registerIX);
		} else {
			return getHigherByte(_registerIY);
		}
	} else if (code == R_L) {
		if (!useIndex || (!_useRegisterIX && !_useRegisterIY)) {
			return _register[code];
		} else if (_useRegisterIX) {
			return getLowerByte(_registerIX);
		} else {
			return getLowerByte(_registerIY);
		}
	}

	return _register[code];
}

uint16_t CPU::getRegisterPair(uint8_t code, bool alternate)
{
	// TODO verify HL
	uint8_t* r = _register;
	if(alternate)
		r = _registerA;

	if (code < 3) {
		if (code == 2 && _useRegisterIX) {
			return _registerIX;
		} else if (code == 2 && _useRegisterIY) {
			return _registerIY;
		}
		return ((r[code * 2] << 8) + r[code * 2 + 1]);
	}

	return _sp;
}

uint16_t CPU::getRegisterPair2(uint8_t code, bool alternate)
{
	uint8_t* r = _register;
	if(alternate)
		r = _registerA;

	if (code < 3) {
		if (code == 2 && _useRegisterIX) {
			return _registerIX;
		} else if (code == 2 && _useRegisterIY) {
			return _registerIY;
		}
		return ((r[code * 2] << 8) + r[code * 2 + 1]);
	}

	if(alternate)
		return ((r[R_A]<<8) + _registerFlagA);

	return ((r[R_A]<<8) + _registerFlag);
}
#include "CPU.h"

#include <string>

#include <iomanip>
#include <sstream>

#include "Cartridge.h"
#include "Memory.h"
#include "System.h"

using namespace std;

CPU::CPU(System& parent) :
    _cartridgeRef{parent.getCartridge()},
    _graphicsRef{parent.getGraphics()},
    _memoryRef{parent.getMemory()},
	_IFF1{ false },
	_IFF2{ false },
	_pc{ 0 },
	_sp{ 0 },
	_modeInt{ 0 },
	_halt{ 0 },
	_register{ 0 },
	_useRegisterIX{ 0 },
	_useRegisterIY{ 0 },
	_displacementForIndexUsed{ false },
	_displacementForIndex{ 0 },
	_registerAluTemp{ 0 }
	
{
    // TODO(ibancel): Audio
    // _audio = new Audio(); // for the moment !
	_inputs = Inputs::Instance();

	_isInitialized = false;
}

void CPU::init()
{
    _cartridge = _cartridgeRef.ptr();
    _graphics = _graphicsRef.ptr();
    _memory = _memoryRef.ptr();
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

	_registerFlag = 0xFF;
	_registerFlagA = 0x00;
	_registerIX = 0;
	_registerIY = 0;
	_registerI = 0;
	_registerR = 0;
	_ioPortControl = 0;

	_displacementForIndexUsed = false;
	_displacementForIndex = 0;

	stopRegisterIX();
	stopRegisterIY();

	_isBlockInstruction = false;

	_cycleCount = 5;
    _graphics->addCpuStates(_cycleCount);
    _graphics->syncThread();

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

	_cycleCount = 5;
    _graphics->addCpuStates(_cycleCount);
    _graphics->syncThread();

	_displacementForIndexUsed = false;
	_displacementForIndex = 0;
}

int CPU::cycle()
{
	if (!_isInitialized) {
		SLOG_THROW(lerror << "CPU not initialized");
		return 0;
	}

	int nbStates = 0;
	bool canExecute = true;

    if(_enableInterruptWaiting == 0 && !_isBlockInstruction && _graphics->getIE()) {
		nbStates = interrupt();
		if (nbStates > 0) {
			canExecute = false;
		}
	}
	if (_enableInterruptWaiting > 0) {
		_enableInterruptWaiting--;
	}

	if (_halt) {
		nbStates = 4;
		canExecute = false;
	}

	if (canExecute)
	{
		SLOG(ldebug << hex << "HL:" << getRegisterPair(RP_HL) << " SP:" << (u16)(_sp)/* << " Stack:" << (u16)_stack[_sp + 1] << "," << (u16)_stack[_sp]*/);

		u8 prefix = _memory->read(_pc++);
		u8 opcode = prefix;
		incrementRefreshRegister();

		if (prefix == 0xCB || prefix == 0xDD || prefix == 0xED || prefix == 0xED || prefix == 0xFD)
		{
			opcode = _memory->read(_pc++);
			incrementRefreshRegister();
		} else
			prefix = 0;

		nbStates = opcodeExecution(prefix, opcode);

		if (nbStates == 0) {
			SLOG_THROW(lerror << hex << "No T state given! #" << (u16)(_pc - 1 - (prefix != 0 ? 1 : 0)) << " : " << (u16)opcode << " (" << ((u16)prefix) << ")");
			nbStates = 4;
		}
	}

	if (nbStates == 0) {
		SLOG_THROW(lerror << hex << "No T state given!");
		nbStates = 4;
	}

	addNbStates(nbStates);

    //_audio->run(); // to put in main loop
	return nbStates;
}

int CPU::opcodeExecution(const u8 prefix, const u8 opcode)
{
	SLOG(lnormal << hex << std::setfill('0') << std::uppercase
		<< "#" << (u16)(_pc - 1 - (prefix != 0 ? 1 : 0)) << " "
		<< std::setw(4) << getRegisterPair2(RP2_AF) << " "
		<< std::setw(4) << getRegisterPair(RP_BC) << " "
		<< std::setw(4) << getRegisterPair(RP_DE) << " "
		<< std::setw(4) << getRegisterPair(RP_HL) << " "
		<< std::setw(4) << _registerIX << " "
		<< std::setw(4) << _registerIY << " "
		<< std::setw(4) << getRegisterPair(RP_SP) << " "
		<< std::dec << _cycleCount << " ");
	SLOG_NOENDL(ldebug << hex <<  "#" << (u16)(_pc-1-(prefix!=0?1:0)) << " : " << (u16) opcode);
	if (prefix != 0) {
		SLOG_NOENDL("(" << (u16)prefix << ")");
	}
	SLOG(ldebug << " --" << getOpcodeName(prefix, opcode) << "--");

	Stats::add(prefix, opcode);

	resInstruction res;
	int nbStates = 0;

	const u8 x = (opcode>>6) & 0b11;
	const u8 y = (opcode>>3) & 0b111;
	const u8 z = (opcode>>0) & 0b111;
	const u8 p = (y >> 1);
	const u8 q = y & 0b1;

	try {

		if (prefix == 0) {
			nbStates = opcode0(x, y, z, p, q);
		} else if (prefix == 0xCB) {
			nbStates = opcodeCB(x, y, z, p, q);
		} else if (prefix == 0xED) {
			nbStates = opcodeED(x, y, z, p, q);
		} else if (prefix == 0xDD) {
			nbStates = opcodeDD(x, y, z, p, q);
		} else if (prefix == 0xFD) {
			nbStates = opcodeFD(x, y, z, p, q);
		}

	} catch (const EmulatorException &e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Unknown Exception: " << e.what() << std::endl;
	}

	consumeRegisterIX();
	consumeRegisterIY();

 	return nbStates;
}

void CPU::aluOperation(const u8 index, const u8 value)
{
	if (index == 0) // ADD A
	{
		u8 sum = (u8)(_register[R_A] + value);
		setFlagBit(F_S, ((s8)(sum) < 0));
		setFlagBit(F_Z, (sum == 0));
		setFlagBit(F_H, (((_register[R_A] & 0x0F) + (value & 0x0F)) > 0x0F));
		setFlagBit(F_P, ((_register[R_A] >> 7) == (value >> 7) && (value >> 7) != (sum >> 7)));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, (sum < _register[R_A]));
		setFlagUndoc(sum);

		_register[R_A] = sum;
	}
	else if (index == 1) // ADC
	{
		// TODO: ADD act the same but with carry = 0
		s8 registerValue = _register[R_A];
		s8 addOp = value + getFlagBit(F_C);
		s8 addOpCarry = ((value == 0xFF) && getFlagBit(F_C));
		s8 sum = (s8)(registerValue + addOp);
		setFlagBit(F_S, (sum < 0));
		setFlagBit(F_Z, (sum == 0));
		setFlagBit(F_H, (((_register[R_A] & 0xF) + (addOp & 0xF)) > 0xF));
		setFlagBit(F_P, addOpCarry || (sign8(registerValue) == sign8(addOp) && sign8(addOp) != sign8(sum)));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, addOpCarry || (static_cast<u8>(sum) < static_cast<u8>(registerValue)));
		setFlagUndoc(sum);

		_register[R_A] = sum;
	}
	else if(index == 2) // SUB
	{
		u8 result = (u8)(_register[R_A] - value);
		bool carryOut = value > _register[R_A];
		bool carryIn = (value & 0x7F) > (_register[R_A] & 0x7F);
		setFlagBit(F_S, ((s8)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, ((_register[R_A] & 0xF) < (value & 0xF)));
		setFlagBit(F_P, (carryIn != carryOut)); // From UM0080 doc
		setFlagBit(F_N, 1);
		setFlagBit(F_C, (result > _register[R_A])); // ?
		setFlagUndoc(result);

		_register[R_A] = result;
	}
	else if (index == 3) // SBC
	{
		// TODO: SUB act the same but with borrow = 0
		u8 registerValue = _register[R_A];
		u8 minusOp = value + getFlagBit(F_C);
		u8 minusOverflow = value == 0xFF && getFlagBit(F_C);
		u8 result = static_cast<u8>(registerValue - minusOp);
		setFlagBit(F_S, ((s8)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, ((registerValue & 0xF) < (minusOp & 0xF)));
		setFlagBit(F_P, (sign8(registerValue) != sign8(result)) && (sign8(minusOp) == sign8(result))); // verified
		setFlagBit(F_C, minusOverflow || (result > registerValue));
		setFlagBit(F_N, 1);
		setFlagUndoc(result);

		_register[R_A] = result;
	}
	else if(index == 4) // AND
    {
        _register[R_A] = (_register[R_A] & value);
        setFlagBit(F_S, ((s8)(_register[R_A]) < 0));
        setFlagBit(F_Z, (_register[R_A] == 0));
        setFlagBit(F_H, 1);
        setFlagBit(F_P, nbBitsEven(_register[R_A]));
        setFlagBit(F_N, 0);
        setFlagBit(F_C, 0);
		setFlagUndoc(_register[R_A]);
    }
	else if(index == 5) // XOR
	{
		_register[R_A] ^= value;
		setFlagBit(F_S, ((s8)(_register[R_A]) < 0));
		setFlagBit(F_Z, (_register[R_A] == 0));
		setFlagBit(F_H, 0);
		setFlagBit(F_P, nbBitsEven(_register[R_A]));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, 0);
		setFlagUndoc(_register[R_A]);
	}
	else if(index == 6) // OR
	{
		_register[R_A] |= value;
		setFlagBit(F_S, ((s8)(_register[R_A]) < 0));
		setFlagBit(F_Z, (_register[R_A] == 0));
		setFlagBit(F_H, 0);
		setFlagBit(F_P, nbBitsEven(_register[R_A]));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, 0);
		setFlagUndoc(_register[R_A]);
	}
	else if(index == 7) // CP
	{
		/// TODO vérifier borrows & overflow
		u8 sum = _register[R_A] - value;
		setFlagBit(F_S, ((s8)(sum) < 0));
		setFlagBit(F_Z, (sum == 0));
		setFlagBit(F_H, ((_register[R_A]&0xF) < (value&0xF)));
		setFlagBit(F_P, ((sign8(_register[R_A]) != sign8(sum)) && (sign8(value)==sign8(sum))));
		setFlagBit(F_N, 1);
		setFlagBit(F_C, (sum > _register[R_A])); // ?
		setFlagUndoc(value);

		SLOG(ldebug << "CP(" << hex << (u16)_register[R_A] << "," << (u16)value << "): " << (u16)(sum == 0));
	}
	else
	{
		slog << lwarning << "ALU " << (u16)index << " is not implemented" << endl;
	}
}

void CPU::rotOperation(const u8 index, const u8 reg)
{
	NOT_IMPLEMENTED("NOT USED ANYMORE, see rot[y] r[z]");
}

u8 CPU::portCommunication(bool rw, u8 address, u8 data)
{
	SLOG(ldebug << hex << (!rw ? "<" : "") << "-----------" << (rw ? ">" : "") << " rw:" << rw << " a:" << (u16)address << " d:" << (u16)data);

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
		if (rw) {
			_graphics->write(address, data);
		} else {
			return _graphics->read(address);
		}
	}
	else if(address == 0x7E || address == 0x7F)
	{
		if (rw) {
            // TODO(ibancel): audio
            //_audio->write(address, data);
		} else {
			return _graphics->read(address);
		}
	}
	else if(address == 0xDE || address == 0xDF)
	{
		/// TODO: fully understand?
		if (!rw) {
			if (address == 0xDE) {
				return portCommunication(false, 0xDC);
			} else { // DF
				return portCommunication(false, 0xDD);
			}
		}
		//NOT_IMPLEMENTED("I18n");
	}
	else if(address == 0xDC || address == 0xC0)
    {
        /// TODO verification between Richard's and Marat's docs

        u8 returnCode = 0;
        setBit8(&returnCode, 7, !_inputs->controllerKeyPressed(Inputs::kJoypad2, Inputs::CK_DOWN));
        setBit8(&returnCode, 6, !_inputs->controllerKeyPressed(Inputs::kJoypad2, Inputs::CK_UP));
        setBit8(&returnCode, 5, !_inputs->controllerKeyPressed(Inputs::kJoypad1, Inputs::CK_FIREB));
        setBit8(&returnCode, 4, !_inputs->controllerKeyPressed(Inputs::kJoypad1, Inputs::CK_FIREA));
        setBit8(&returnCode, 3, !_inputs->controllerKeyPressed(Inputs::kJoypad1, Inputs::CK_RIGHT));
        setBit8(&returnCode, 2, !_inputs->controllerKeyPressed(Inputs::kJoypad1, Inputs::CK_LEFT));
        setBit8(&returnCode, 1, !_inputs->controllerKeyPressed(Inputs::kJoypad1, Inputs::CK_DOWN));
        setBit8(&returnCode, 0, !_inputs->controllerKeyPressed(Inputs::kJoypad1, Inputs::CK_UP));

        return returnCode;
    }
    else if(address == 0xDD || address == 0xC1)
    {
        u8 returnCode = 0;
		if (_ioPortControl == 0xF5) {
#if MACHINE_MODEL == 0
			setBit8(&returnCode, 7, 0);
			setBit8(&returnCode, 6, 0);
#else 
			setBit8(&returnCode, 7, getBit8(_ioPortControl, 7));
			setBit8(&returnCode, 6, getBit8(_ioPortControl, 5));
#endif
		} else {
			// TODO: lightgun
			setBit8(&returnCode, 7, 1);
			setBit8(&returnCode, 6, 1);
		}
#if MACHINE_MODEL == 0 || MACHINE_MODEL == 1
		setBit8(&returnCode, 5, 1);
#else
		NOT_IMPLEMENTED("Bit 5 of I/O port B");
#endif
        setBit8(&returnCode, 4, 1); // TODO: reset button
        setBit8(&returnCode, 3, !_inputs->controllerKeyPressed(Inputs::kJoypad2, Inputs::CK_FIREB));
        setBit8(&returnCode, 2, !_inputs->controllerKeyPressed(Inputs::kJoypad2, Inputs::CK_FIREA));
        setBit8(&returnCode, 1, !_inputs->controllerKeyPressed(Inputs::kJoypad2, Inputs::CK_RIGHT));
        setBit8(&returnCode, 0, !_inputs->controllerKeyPressed(Inputs::kJoypad2, Inputs::CK_LEFT));

        return returnCode;
    }
	else
	{
		slog << lwarning << hex << "Communication port 0x" << (u16)address << " is not implemented (rw=" << rw << ")" << endl;
	}

	return 0;
}


/// PRIVATE :

int CPU::opcode0(u8 x, u8 y, u8 z, u8 p, u8 q)
{
	int nbTStates = 0;

	if(x == 0 && z == 0 && y == 0) // NOP
	{
		nbTStates = 4;
	}
	else if(x == 0 && z == 0 && y == 1) // EX AF, AF'
	{
		swapRegisterPair2(RP2_AF);
		nbTStates = 4;
	}
	else if(x == 0 && z == 0 && y == 2) // DJNZ d
	{
		_register[R_B]--;
		u8 d = _memory->read(_pc++);

		if (getRegister(R_B) != 0) {
			_pc += (s8)d;
			nbTStates = 13;
		} else {
			nbTStates = 8;
		}

	}
	else if(x == 0 && z == 0 && y == 3) // JR d
	{
		_pc += (s8)(_memory->read(_pc)) + 1;
		nbTStates = 12;
	}
	else if(x == 0 && z == 0 && y >= 4) // JR cc[y-4],d
	{
		SLOG(ldebug << "=> Condition " << y-4 << " : " << condition(y-4) << " (d=" << hex << (s16)((s8)_memory->read(_pc)) << ")");
		s8 addrOffset = static_cast<s8>(_memory->read(_pc++));
		if (condition(y - 4)) {
			_pc += addrOffset;
			nbTStates = 12;
		} else {
			nbTStates = 7;
		}
	}
	//
	else if(x == 0 && z == 1 && q == 0) // LD rp[p],nn
	{
		u16 val = _memory->read(_pc++);
		val += (_memory->read(_pc++)<<8);
		setRegisterPair(p, val);
		
		if (p == 2 && (_useRegisterIX || _useRegisterIY)) {
			nbTStates = 14;
		} else {
			nbTStates = 10;
		}
	}
	else if(x == 0 && z == 1 && q == 1) // ADD HL,rp[p]
	{
		u16 oldVal = getRegisterPair(RP_HL);
		u16 addOp = getRegisterPair(p);
		u16 newVal = oldVal + addOp;

		setFlagBit(F_H, ((oldVal&0xFFF) + (addOp&0xFFF) > 0xFFF));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, (newVal < oldVal));

		setFlagUndoc(getHigherByte(newVal));

		setRegisterPair(RP_HL, newVal);

		if (_useRegisterIX || _useRegisterIY) {
			nbTStates = 15;
		} else {
			nbTStates = 11;
		}
	}
	//
	else if(x == 0 && z == 2 && q == 0 && p == 0) // LD (BC),A
	{
		_memory->write(getRegisterPair(RP_BC), _register[R_A]);
		nbTStates = 7;
	}
	else if(x == 0 && z == 2 && q == 0 && p == 1) // LD (DE),A
	{
		_memory->write(getRegisterPair(RP_DE), _register[R_A]);
		nbTStates = 7;
	}
	else if(x == 0 && z == 2 && q == 0 && p == 2) // LD (nn),HL
	{
		u16 addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		_memory->write(addr, getRegister(R_L));
		_memory->write(addr+1, getRegister(R_H));
		nbTStates = 16;
	}
	else if(x == 0 && z == 2 && q == 0 && p == 3) // LD (nn),A
	{
		u16 addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		_memory->write(addr, _register[R_A]);
		nbTStates = 13;
	}
	else if (x == 0 && z == 2 && q == 1 && p == 0) // LD A,(BC)
	{
		setRegister(R_A, _memory->read(getRegisterPair(RP_BC)));
		nbTStates = 7;
	}
	else if(x == 0 && z == 2 && q == 1 && p == 1) // LD A,(DE)
	{
		_register[R_A] = _memory->read(getRegisterPair(RP_DE));
		nbTStates = 7;
	}
	else if(x == 0 && z == 2 && q == 1 && p == 2) // LD HL,(nn)
	{
		u16 address = _memory->read(_pc++);
		address += (_memory->read(_pc++) << 8);
		setRegister(R_L, _memory->read(address));
		setRegister(R_H, _memory->read(address+1));
		nbTStates = 16;
	}
	else if(x == 0 && z == 2 && q == 1 && p == 3) // LD A,(nn)
	{
		u16 val = _memory->read(_pc++);
		val += (_memory->read(_pc++)<<8);
		_register[R_A] = _memory->read(val);

		nbTStates = 13;

		SLOG(ldebug << "addr: " << hex << val << " val: " << (u16)_register[R_A]);
	}
	//
	else if(x == 0 && z == 3 && q == 0) // INC rp[p]
	{
		setRegisterPair(p, getRegisterPair(p)+1);
		nbTStates = 6;
	}
	else if(x == 0 && z == 3 && q == 1) // DEC rp[p]
	{
		setRegisterPair(p, getRegisterPair(p)-1);
		nbTStates = 6;
	}
	else if(x == 0 && z == 4) // INC r[y]
	{
		// TODO: test flags
		const u8 value = getRegister(y);
		const u8 result = (u8)(value + 1);
		setFlagBit(F_S, ((s8)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, (value & 0xF) == 0xF);
		setFlagBit(F_P, (value == 0x7F)); // from doc
		setFlagBit(F_N, 0);
		setFlagUndoc(result);

		setRegister(y, result);
		
		nbTStates = 4;
	}
	else if(x == 0 && z == 5) // DEC r[y]
	{
		// TODO: test flags
		const u8 value = getRegister(y);
		const u8 result = (u8)(value - 1);

		setFlagBit(F_S, ((s8)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, (value & 0x0F) == 0);
		setFlagBit(F_P, (value == 0x80)); // from doc
		setFlagBit(F_N, 1);
		setFlagUndoc(result);

		setRegister(y, result);
		if (_useRegisterIX || _useRegisterIY) {
			nbTStates = 23;
		} else if (y == 6) {
			nbTStates = 11;
		} else {
			nbTStates = 4;
		}
	}
	else if(x == 0 && z == 6) // LD r[y],n
	{
		if (y == 6 && (_useRegisterIX || _useRegisterIY)) {
			retrieveIndexDisplacement();
		}

		setRegister(y, _memory->read(_pc++));
		if (_useRegisterIX || _useRegisterIY) {
			nbTStates = 19;
		} else if (y == 6) {
			nbTStates = 10;
		} else {
			nbTStates = 7;
		}
	}
	else if(x == 0 && z == 7 && y == 0) // RLCA
	{
		u8 valueRegister = getRegister(R_A, false, false);
		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		setFlagBit(F_C, (valueRegister >> 7));
		valueRegister <<= 1;
		setBit8(&valueRegister, 0, getFlagBit(F_C));

		setFlagUndoc(valueRegister);
		setRegister(R_A, valueRegister, false, false);
		nbTStates = 4;
	}
	else if(x == 0 && z == 7 && y == 1) // RRCA
    {
        setFlagBit(F_H, 0);
        setFlagBit(F_N, 0);
        u8 bit0 = getBit8(_register[R_A], 0);
        _register[R_A] >>= 1;
        setBit8(&_register[R_A], 7, bit0);
		setFlagUndoc(_register[R_A]);
        setFlagBit(F_C, bit0);
		nbTStates = 4;
    }
	else if (x == 0 && z == 7 && y == 2) // RLA
	{
	u8 valueRegister = getRegister(R_A, false, false);
		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		u8 carryBit = getFlagBit(F_C);
		setFlagBit(F_C, (valueRegister >> 7));
		valueRegister <<= 1;
		setBit8(&valueRegister, 0, carryBit);
		
		setFlagUndoc(valueRegister);
		setRegister(R_A, valueRegister, false, false);
		nbTStates = 4;
	}
	else if (x == 0 && z == 7 && y == 3) // RRA
	{
		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		uint_fast8_t bit0 = getBit8(_register[R_A], 0);
		_register[R_A] >>= 1;
		setBit8(&_register[R_A], 7, getFlagBit(F_C));
		setFlagUndoc(_register[R_A]);
		setFlagBit(F_C, bit0);
		nbTStates = 4;
	}
	else if (x == 0 && z == 7 && y == 4) // DAA
	{
		// Block simplified thanks to Eric Haskins
		// https://ehaskins.com/2018-01-30 Z80 DAA/
		u8 cBit = getFlagBit(F_C);
		u8 hBit = getFlagBit(F_H);
		u8 nBit = getFlagBit(F_N);
		u8 value = getRegister(R_A);
		u8 correction = 0;
		u8 newCBit = 0;
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
		setFlagBit(F_H, (!nBit && ((value&0x0F)>9)) || (nBit && hBit && ((value&0x0F)<6))); // to verify (or 0 as before?)
		setFlagBit(F_C, newCBit);
		setFlagBit(F_S, getBit8(value, 7));
		setFlagBit(F_P, nbBitsEven(value));
		setFlagUndoc(value);

		nbTStates = 4;
	}
	else if(x == 0 && z == 7 && y == 5) // CPL
	{
		_register[R_A] = ~_register[R_A];

		setFlagBit(F_H, 1);
		setFlagBit(F_N, 1);
		setFlagUndoc(_register[R_A]);

		nbTStates = 4;
	}
	else if (x == 0 && z == 7 && y == 6) // SCF
	{
		setFlagBit(F_C, 1);
		setFlagBit(F_N, 0);
		setFlagBit(F_H, 0);

		nbTStates = 4;
	}
	else if(x == 0 && z == 7 && y == 7) // CCF
	{
        setFlagBit(F_N, 0);
        setFlagBit(F_C, !getFlagBit(F_C));

		nbTStates = 4;
	}
	else if(x == 1 && (z != 6 || y != 6)) // LD r[y],r[z]
	{
		setRegister(y, getRegister(z, false, (y != 6)), false, (z != 6));
		if (_useRegisterIX || _useRegisterIY) {
			nbTStates = 19;
		} else if(y == 6 || z == 6) {
			nbTStates = 7;
		} else {
			nbTStates = 4;
		}
	}
	else if(x == 1 && z == 6 && y == 6) // HALT
	{
		_halt = true;
		nbTStates = 4;
	}
	else if(x == 2) // alu[y] r[z]
	{
		aluOperation(y, getRegister(z));
		if (z == 6) {
			if (_useRegisterIX || _useRegisterIY) {
				nbTStates = 19;
			} else {
				nbTStates = 7;
			}
		} else {
			nbTStates = 4;
		}
	}
	else if(x == 3 && z == 0) // RET cc[y]
	{
		bool c = condition(y);
		if(c) {
			_pc = _memory->read(_sp++);
			_pc += (_memory->read(_sp++)<<8);
			nbTStates = 11;
		} else {
			nbTStates = 5;
		}
		SLOG(ldebug << hex << "RET(" << (c?"true":"false") << ") to " << (u16)_pc);
	}
	else if(x == 3 && z == 1 && q == 0) // POP rp2[p]
	{
		u16 value = _memory->read(_sp++);
		value += (_memory->read(_sp++)<<8);
		setRegisterPair2(p, value);
		if (p == 2 && (_useRegisterIX || _useRegisterIY)) {
			nbTStates = 14;
		} else {
			nbTStates = 10;
		}
	}
	else if(x == 3 && z == 1 && q == 1 && p == 0) // RET
	{
		u16 addr = _memory->read(_sp++);
		addr += (_memory->read(_sp++)<<8);
		_pc = addr;
		nbTStates = 10;
		SLOG(ldebug << hex << "RET to " << (u16)addr);
	}
	else if(x == 3 && z == 1 && q == 1 && p == 1) // EXX
	{
		swapRegisterPair(RP_BC);
		swapRegisterPair(RP_DE);
		swapRegisterPair(RP_HL);
		nbTStates = 4;
	}
	else if(x == 3 && z == 1 && q == 1 && p == 2) // JP HL
    {
        _pc = getRegisterPair(RP_HL);
		nbTStates = 4;
        SLOG(ldebug << hex << "JP to " << (u16)_pc);
    }
	else if(x == 3 && z == 1 && q == 1 && p == 3) // LD SP, HL
	{
		_sp = getRegisterPair(RP_HL);
		nbTStates = 6;
		SLOG(ldebug << hex << "SP: " << _sp);
	}
	else if(x == 3 && z == 2) // JP cc[y],nn
	{
		u16 addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		if (condition(y)) {
			_pc = addr;
		}
		nbTStates = 10;
		SLOG(ldebug << "JP C,nn : " << (condition(y)?"true":"false") << " new addr: " << hex << _pc);
	}
	//
	else if(x == 3 && z == 3 && y == 0) // JP nn
	{
		u16 val = _memory->read(_pc++);
		val += (_memory->read(_pc++) << 8);
		_pc = val;
		nbTStates = 10;
		SLOG(ldebug << hex << "val: " << val);
	}
	//
	else if(x == 3 && z == 3 && y == 2) // OUT (n),A
	{
		addNbStates(5); // TODO: determine precisely the number of states before
		u8 val = _memory->read(_pc++);
		portCommunication(true, val, getRegister(R_A));
		nbTStates = 6;
		//slog << ldebug << "OUT ====> " << hex << (u16)val << " : " << (u16)_register[R_A] << endl;
	}
	else if(x == 3 && z == 3 && y == 3) // IN A,(n)
	{
		addNbStates(5); // TODO: determine precisely the number of states before
		u8 addr = _memory->read(_pc++);
		_register[R_A] = portCommunication(false, addr);
		// setFlagBit(F_H, 0); // Not sure about this
		nbTStates = 6;
	}
	else if(x == 3 && z == 3 && y == 4) // EX (SP), HL
	{
		u8 tempL = _memory->read(_sp);
		u8 tempH = _memory->read(_sp+1);
		_memory->write(_sp, getRegister(R_L));
		_memory->write(_sp+1, getRegister(R_H));
		setRegister(R_L, tempL);
		setRegister(R_H, tempH);
		if (_useRegisterIX || _useRegisterIY) {
			nbTStates = 23;
		} else {
			nbTStates = 19;
		}
	}
	else if (x == 3 && z == 3 && y == 5) // EX DE, HL
	{
		stopRegisterIX();
		stopRegisterIY();
		u16 temp = getRegisterPair(RP_HL);
		setRegisterPair(RP_HL, getRegisterPair(RP_DE));
		setRegisterPair(RP_DE, temp);
		nbTStates = 4;
	}
	else if(x == 3 && z == 3 && y == 6) // DI
	{
		_IFF1 = false;
		_IFF2 = false;
		nbTStates = 4;
	}
	else if(x == 3 && z == 3 && y == 7) // EI
    {
        /// TODO verify if it's working well
        _IFF1 = true;
        _IFF2 = true;
		_enableInterruptWaiting = 1;
		nbTStates = 4;
    }
	else if(x == 3 && z == 4) // CALL cc[y],nn
	{
		u16 addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);

		bool c = condition(y);
		if(c) {
			_memory->write(--_sp, (_pc >> 8));
			_memory->write(--_sp, (_pc & 0xFF));
			_pc = addr;
			nbTStates = 17;
		} else {
			nbTStates = 10;
		}

		SLOG(ldebug << "CALL(" << (c?"true":"false") << ") to " << hex << _pc);
	}
	else if(x == 3 && z == 5 && q == 0) // PUSH rp2[p]
	{
		u16 val = getRegisterPair2(p);
		_memory->write(--_sp, (val >> 8));
		_memory->write(--_sp, (val & 0xFF));
		if (p == 2 && (_useRegisterIX || _useRegisterIY)) {
			nbTStates = 15;
		} else {
			nbTStates = 11;
		}
	}
	//
	else if(x == 3 && z == 5 && q == 1 && p == 0) // CALL nn
	{
		u16 newPC = _memory->read(_pc++);
		newPC += (_memory->read(_pc++)<<8);

		_memory->write(--_sp, (_pc >> 8));
		_memory->write(--_sp, (_pc & 0xFF));

		_pc = newPC;

		nbTStates = 17;

		SLOG(ldebug << "CALL to " << hex << newPC);
	}
	//
	else if(x == 3 && z == 6) // alu[y] n
	{
		aluOperation(y, _memory->read(_pc++));
		nbTStates = 7;
	}
	else if(x == 3 && z == 7) // RST y*8
	{
		restart(y * 8);
		nbTStates = 11;
	}
	else
	{
		SLOG_THROW(lwarning << "Opcode : " << hex << (u16)((x<<6)+(y<<3)+z) << " is not implemented");
		SLOG(ldebug << "# pc: " << hex << _pc);
		//slog << ldebug << "x: " << (u16)x << " | y: " << (u16)y << " | z: " << (u16)z << endl;
	}

	return nbTStates;
}

int CPU::opcodeCB(u8 x, u8 y, u8 z, u8 p, u8 q)
{
	int nbTStates = 0;

	if(x == 0) // rot[y] r[z]
	{
		const u8 index = y;
		u8 value = getRegister(z, false, false);
		if (_useRegisterIX) {
			value = _memory->read(_registerIX + _cbDisplacement);
		} else if (_useRegisterIY) {
			value = _memory->read(_registerIY + _cbDisplacement);
		}

		if (_useRegisterIX || _useRegisterIY) {
			nbTStates = 6;
		} else if (z == 6) {
			nbTStates = 4;
		} else {
			nbTStates = 2;
		}

		if (index == 0) // RLC
		{
			setFlagBit(F_C, getBit8(value, 7));
			value <<= 1;
			setBit8(&value, 0, getFlagBit(F_C));
			setFlagBit(F_S, ((s8)(value) < 0));
			setFlagBit(F_Z, (value == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(value));
			setFlagBit(F_N, 0);
			setFlagUndoc(value);
			setCBRegisterWithCopy(z, value);
			
			if (_useRegisterIX || _useRegisterIY) {
				nbTStates = 23;
			} else if (z == 6) {
				nbTStates = 15;
			} else {
				nbTStates = 8;
			}
		}
		else if (index == 1) // RRC
		{
			setFlagBit(F_C, getBit8(value, 0));
			value >>= 1;
			setBit8(&value, 7, getFlagBit(F_C));
			setFlagBit(F_S, ((s8)(value) < 0));
			setFlagBit(F_Z, (value == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(value));
			setFlagBit(F_N, 0);
			setFlagUndoc(value);
			setCBRegisterWithCopy(z, value);

			if (_useRegisterIX || _useRegisterIY) {
				nbTStates = 23;
			} else if (z == 6) {
				nbTStates = 15;
			} else {
				nbTStates = 8;
			}
		}
		else if (index == 2) // RL
		{
			u8 valRot = (u8)(value << 1);
			setBit8(&valRot, 0, getFlagBit(F_C));

			setFlagBit(F_S, ((s8)(valRot) < 0));
			setFlagBit(F_Z, (valRot == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(valRot));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, (value >> 7) & 1);
			setFlagUndoc(valRot);

			setCBRegisterWithCopy(z, valRot);
		}
		else if (index == 3) // RR
		{
			u8 valRot = value >> 1;
			setBit8(&valRot, 7, getFlagBit(F_C));

			setFlagBit(F_S, ((s8)(valRot) < 0));
			setFlagBit(F_Z, (valRot == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(valRot));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, (value & 1));
			setFlagUndoc(valRot);

			setCBRegisterWithCopy(z, valRot);
		}
		else if (index == 4) // SLA
		{
			u8 valRot = (u8)(value << 1);

			setFlagBit(F_S, ((s8)(valRot) < 0));
			setFlagBit(F_Z, (valRot == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(valRot));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, (value >> 7) & 1);
			setFlagUndoc(valRot);

			setCBRegisterWithCopy(z, valRot);
		}
		else if (index == 5) // SRA
		{
			u8 valRot = (u8)(value >> 1);
			setBit8(&valRot, 7, getBit8(value, 7));

			setFlagBit(F_S, ((s8)(valRot) < 0));
			setFlagBit(F_Z, (valRot == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(valRot));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, (value & 1));
			setFlagUndoc(valRot);

			setCBRegisterWithCopy(z, valRot);
		}
		else if (index == 6) // SLL
		{
			u8 valRot = (u8)(value << 1);
			setBit8(&valRot, 0, 1);

			setFlagBit(F_S, ((s8)(valRot) < 0));
			setFlagBit(F_Z, (valRot == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(valRot));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, getBit8(value, 7));
			setFlagUndoc(valRot);

			setCBRegisterWithCopy(z, valRot);
		}
		else if (index == 7) // SRL
		{
			u8 valRot = (u8)(value >> 1);

			setFlagBit(F_S, 0);
			setFlagBit(F_Z, (valRot == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(valRot));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, (value & 1));
			setFlagUndoc(valRot);

			setCBRegisterWithCopy(z, valRot);
		}
		else {
			slog << lwarning << "ROT " << (u16)index << " is not implemented" << endl;
		}
	}
	else if(x == 1) // BIT y, r[z]
    {
		u8 value = getRegister(z, false, false);
		if (_useRegisterIX) {
			value = _memory->read(_registerIX + _cbDisplacement);
		} else if (_useRegisterIY) {
			value = _memory->read(_registerIY + _cbDisplacement);
		}

		bool isBitSet = getBit8(value, y);
		setFlagBit(F_S, ((y == 7) && isBitSet));
        setFlagBit(F_H, 1);
        setFlagBit(F_N, 0);
		setFlagBit(F_Z, (isBitSet == 0));
		setFlagBit(F_P, getFlagBit(F_Z));
		setFlagBit(F_F3, (y == 3 && isBitSet));
		setFlagBit(F_F5, (y == 5 && isBitSet));

		if (_useRegisterIX || _useRegisterIY) {
			nbTStates = 20;
		} else if (z == 6) {
			nbTStates = 12;
		} else {
			nbTStates = 8;
		}
    }
	else if (x == 2) // RES y, r[z]
	{
		u8 value = getRegister(z, false, false);
		if (_useRegisterIX) {
			value = _memory->read(_registerIX + _displacementForIndex);
		} else if (_useRegisterIY) {
			value = _memory->read(_registerIY + _displacementForIndex);
		}

		setBit8(&value, y, 0);

		setCBRegisterWithCopy(z, value);

		if (_useRegisterIX || _useRegisterIY) {
			nbTStates = 6;
		} else {
			nbTStates = 4;
		}
	}
	else if (x == 3) // SET y, r[z]
	{
		u8 value = getRegister(z, false, false);
		if (_useRegisterIX) {
			value = _memory->read(_registerIX + _displacementForIndex);
		} else if (_useRegisterIY) {
			value = _memory->read(_registerIY + _displacementForIndex);
		}

		setBit8(&value, y, 1);

		setCBRegisterWithCopy(z, value);

		if (_useRegisterIX || _useRegisterIY) {
			nbTStates = 23;
		} else if (z == 6) {
			nbTStates = 15;
		} else {
			nbTStates = 8;
		}
	}
	else
	{
		// Actually this should not occurs anymore :)
		slog << lwarning << "Opcode : " << hex << (u16)((x<<6)+(y<<3)+z) << " (CB) is not implemented" << endl;
	}

	return nbTStates;
}

int CPU::opcodeED(u8 x, u8 y, u8 z, u8 p, u8 q)
{
	int nbTStates = 0;

	if(x == 0 || x == 3) // NOP
	{
		nbTStates = 4;
	}
	else if(x == 1 && z == 1 && y == 6) // OUT (C),0
	{
		addNbStates(6); // TODO: determine precisely the number of states before
		portCommunication(true, _register[R_C], 0);
		nbTStates = 6; // to check!
	}
	else if(x == 1 && z == 1 && y != 6) // OUT (C),r[y]
	{
		addNbStates(5); // TODO: determine precisely the number of states before
		portCommunication(true, _register[R_C], getRegister(y));
		nbTStates = 6;
	}
	else if(x == 1 && z == 2 && q == 0) // SBC HL, rp[p]
	{
		u16 value = getRegisterPair(RP_HL);
		u16 minusOp = getRegisterPair(p) + getFlagBit(F_C);
		u16 minusOverflow = minusOp == 0xFFFF && getFlagBit(F_C);
		u16 result = (u16)(value - minusOp);
		setFlagBit(F_S, ((s16)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, minusOverflow || (value & 0xFFF) < (minusOp & 0xFFF)); // NOT GOOD
		setFlagBit(F_P, (isPositive16(value) != isPositive16(result)) && (isPositive16(result) == isPositive16(minusOp)));
		setFlagBit(F_N, 1);
		setFlagBit(F_C, minusOverflow || (result > value));
		// setFlagBit(F_F3, (result>>2)&1); //__this is false, should be changed but understand why >>2 and >>4
		// setFlagBit(F_F5, (result>>4)&1); //_/
		setRegisterPair(RP_HL, result);
		setFlagUndoc(getHigherByte(result));

		nbTStates = 15;
	}
	else if (x == 1 && z == 2 && q == 1) // ADC HL, rp[p]
	{
		u16 value = getRegisterPair(RP_HL);
		u16 addOp = getRegisterPair(p) + getFlagBit(F_C);
		u16 addOverflow = addOp == 0xFFFF && getFlagBit(F_C);
		u16 result = (u16)(value + addOverflow);

		setFlagBit(F_S, (static_cast<s16>(result) < 0));
		setFlagBit(F_Z, result == 0);
		setFlagBit(F_H, addOverflow || ((value & 0x0FFF) + (addOp & 0x0FFF) > 0x0FFF));
		setFlagBit(F_P, addOverflow || (sign16(value) == sign16(addOp) && sign16(addOp) != sign16(result)));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, (addOverflow || (result < value) || (result < addOp)));
		setFlagUndoc(getHigherByte(result));

		nbTStates = 15;
	}
	else if(x == 1 && z == 3 && q == 0) // LD (nn), rp[p]
    {
        u16 registerValue = getRegisterPair(p);
        u16 addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		_memory->write(addr, getLowerByte(registerValue));
		_memory->write(addr+1, getHigherByte(registerValue));
		nbTStates = 20;
    }
    else if(x == 1 && z == 3 && q == 1) // LD rp[p], (nn)
    {
        u16 addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		u16 registerValue = _memory->read(addr);
		registerValue += (_memory->read(addr+1)<<8);
		setRegisterPair(p, registerValue);
		nbTStates = 20;
    }
	else if (x == 1 && z == 4) // NEG
	{
		s8 registerValue = -getRegister(R_A);

		setFlagBit(F_P, getRegister(R_A) == 0x80);
		setFlagBit(F_C, getRegister(R_A) != 0);

		setFlagBit(F_S, registerValue < 0);
		setFlagBit(F_Z, registerValue == 0);
		setFlagBit(F_H, ((getRegister(R_A)&0x0F) > 0));
		setFlagBit(F_N, 1);

		setFlagUndoc(registerValue);

		setRegister(R_A, registerValue);
		nbTStates = 8;
	}
	else if(x == 1 && z == 5 && y != 1) // RETN
	{
		// TODO: verify
		_IFF1 = _IFF2;

		_pc = _memory->read(_sp++);
		_pc += (_memory->read(_sp++)<<8);

		nbTStates = 14;
	}
	else if(x == 1 && z == 5 && y == 1) // RETI
    {
        _pc = _memory->read(_sp++);
        _pc += (_memory->read(_sp++)<<8);

		_IFF1 = _IFF2;
        /// TODO complete this with IEO daisy chain

		nbTStates = 14;
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

		nbTStates = 8;
	}
	//
	else if (x == 1 && z == 7 && y == 1) // LD R, A
	{
		_registerR = getRegister(R_A);
	
		nbTStates = 9;
	}
	//
	else if (x == 1 && z == 7 && y == 3) // LD A, R
	{
		setRegister(R_A, _registerR);
		setFlagBit(F_S, ((s8)(_registerR) < 0));
		setFlagBit(F_Z, (_registerR == 0));
		setFlagBit(F_H, 0);
		setFlagBit(F_P, _IFF2);
		setFlagBit(F_N, 0);

		nbTStates = 9;
	}
	else if(x == 1 && z == 7 && y == 4) // RRD
    {
		u8 valueInMemory = readMemoryHL();
        u8 tempLowA = _register[R_A] & 0xF;
        _register[R_A] = (_register[R_A] & 0xF0) | (valueInMemory & 0x0F);
		writeMemoryHL((tempLowA << 4) | (valueInMemory >> 4));
        setFlagBit(F_S, ((s8)(_register[R_A]) < 0));
        setFlagBit(F_Z, (_register[R_A] == 0));
        setFlagBit(F_H, 0);
        setFlagBit(F_P, nbBitsEven(_register[R_A]));
        setFlagBit(F_N, 0);
		setFlagUndoc(getRegister(R_A));

		nbTStates = 18;
    }
	else if (x == 1 && z == 7 && y == 5) // RLD
	{
		u8 valueInMemory = readMemoryHL();
		u8 tempLowA = _register[R_A] & 0xF;
		_register[R_A] = (_register[R_A] & 0xF0) | ((valueInMemory & 0xF0) >> 4);
		writeMemoryHL(((valueInMemory & 0x0F) << 4) | tempLowA);
		setFlagBit(F_S, ((s8)(_register[R_A]) < 0));
		setFlagBit(F_Z, (_register[R_A] == 0));
		setFlagBit(F_H, 0);
		setFlagBit(F_P, nbBitsEven(_register[R_A]));
		setFlagBit(F_N, 0);
		setFlagUndoc(getRegister(R_A));

		nbTStates = 18;
	}
	else if (x == 1 && z == 7 && y > 5) // NOP
	{
		nbTStates = 4;
	}
	else if(x == 2 && (z > 3 || y < 4)) // NONI & NOP
	{
		/// TODO
		nbTStates = 8; // to check
	}
	else if(x == 2) //bli[y,z]
	{
		nbTStates = bliOperation(y, z);
	}
	else
	{
		SLOG_THROW(lwarning << "Opcode : " << hex << (u16)((x<<6)+(y<<3)+z) << " (ED) is not implemented");
	}

	return nbTStates;
}

int CPU::opcodeDD(u8 x, u8 y, u8 z, u8 p, u8 q)
{
	int nbTStates = 0;

    u16 prefixByte = (x<<6)+(y<<3)+z;

	if(prefixByte == 0xDD || prefixByte == 0xED || prefixByte == 0xFD) // NONI
	{
        /// TODO NONI
		_pc--;
		nbTStates = 8; // to check
	}
    else if(prefixByte == 0xCB) // DDCB prefix opcode
    {
		useRegisterIX();
		_cbDisplacement = static_cast<s8>(_memory->read(_pc++));
		_displacementForIndex = _cbDisplacement;
		_displacementForIndexUsed = true;
		const u8 opcode = _memory->read(_pc++);
		const u8 x = (opcode >> 6) & 0b11;
		const u8 y = (opcode >> 3) & 0b111;
		const u8 z = (opcode >> 0) & 0b111;
		const u8 p = (y >> 1);
		const u8 q = y & 0b1;
		nbTStates = opcodeCB(x, y, z, p, q);
		stopRegisterIX();
		_displacementForIndexUsed = false;
    }
	else
	{
		useRegisterIX();
		_displacementForIndexUsed = false;
		nbTStates = opcode0(x, y, z, p, q);
		stopRegisterIX();
		_displacementForIndexUsed = false;
	}

	return nbTStates;
}

int CPU::opcodeFD(u8 x, u8 y, u8 z, u8 p, u8 q)
{
	int nbTStates = 0;

	u16 prefixByte = (x<<6)+(y<<3)+z;

	if(prefixByte == 0xDD || prefixByte == 0xED || prefixByte == 0xFD) // NONI
	{
        /// TODO NONI
		_pc--;
	}
    else if(prefixByte == 0xCB) // DDCB prefix opcode
    {
		useRegisterIY();
		_cbDisplacement = static_cast<s8>(_memory->read(_pc++));
		_displacementForIndex = _cbDisplacement;
		_displacementForIndexUsed = true;
		const u8 opcode = _memory->read(_pc++);
		const u8 x = (opcode >> 6) & 0b11;
		const u8 y = (opcode >> 3) & 0b111;
		const u8 z = (opcode >> 0) & 0b111;
		const u8 p = (y >> 1);
		const u8 q = y & 0b1;
		nbTStates = opcodeCB(x, y, z, p, q);
		stopRegisterIY();
		_displacementForIndexUsed = false;
    }
	else
	{
		useRegisterIY();
		_displacementForIndexUsed = false;
		nbTStates = opcode0(x, y, z, p, q);
		stopRegisterIY();
		_displacementForIndexUsed = false;
	}

	return nbTStates;
}

bool CPU::condition(u8 code)
{
	if (code == 0) {
		return !getFlagBit(F_Z);
	} else if (code == 1) {
		return getFlagBit(F_Z);
	} else if (code == 2) {
		return !getFlagBit(F_C);
	} else if (code == 3) {
		return getFlagBit(F_C);
	} else if (code == 4) {
		return !getFlagBit(F_P);
	} else if (code == 5) {
		return getFlagBit(F_P);
	} else if (code == 6) {
		return !getFlagBit(F_S);
	}
	// else: code == 7

	return getFlagBit(F_S);
}

int CPU::bliOperation(u8 x, u8 y)
{
	int nbTStates = 0;
	if (y == 0 && x == 4) // LDI
	{
		_registerAluTemp = readMemoryHL();

		_memory->write(getRegisterPair(RP_DE), _registerAluTemp);
		setRegisterPair(RP_DE, getRegisterPair(RP_DE) + 1);
		setRegisterPair(RP_HL, getRegisterPair(RP_HL) + 1);
		setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		setFlagBit(F_P, getRegisterPair(RP_BC) != 0);
		setFlagUndocMethod2(getAluTempByte());

		nbTStates = 16;
	}
	else if (y == 0 && x == 5) // LDD
	{
		_registerAluTemp = readMemoryHL();

		_memory->write(getRegisterPair(RP_DE), _registerAluTemp);
		setRegisterPair(RP_DE, getRegisterPair(RP_DE) - 1);
		setRegisterPair(RP_HL, getRegisterPair(RP_HL) - 1);
		setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		setFlagBit(F_P, getRegisterPair(RP_BC) != 0);
		setFlagUndocMethod2(getAluTempByte());

		nbTStates = 16;
	}
	else if(y == 0 && x == 6) // LDIR
	{
		_registerAluTemp = readMemoryHL();

		_memory->write(getRegisterPair(RP_DE), _registerAluTemp);
		setRegisterPair(RP_DE, getRegisterPair(RP_DE)+1);
		setRegisterPair(RP_HL, getRegisterPair(RP_HL)+1);
		setRegisterPair(RP_BC, getRegisterPair(RP_BC)-1);

		if (getRegisterPair(RP_BC) != 0) {
			_pc -= 2;
			nbTStates = 21;
		} else {
			nbTStates = 16;
		}

		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		setFlagBit(F_P, getRegisterPair(RP_BC) != 0);
		setFlagUndocMethod2(getAluTempByte());
	}
	else if (y == 0 && x == 7) // LDDR
	{
		_registerAluTemp = readMemoryHL();

		_memory->write(getRegisterPair(RP_DE), _registerAluTemp);
		setRegisterPair(RP_DE, getRegisterPair(RP_DE) - 1);
		setRegisterPair(RP_HL, getRegisterPair(RP_HL) - 1);
		setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

		if (getRegisterPair(RP_BC) != 0) {
			_pc -= 2;
			nbTStates = 21;
		} else {
			nbTStates = 16;
		}

		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		setFlagBit(F_P, getRegisterPair(RP_BC) != 0);
		setFlagUndocMethod2(getAluTempByte());
	}
	else if (y == 1 && x == 4) // CPI
	{
		u8 memoryValue = readMemoryHL();
		u8 result = getRegister(R_A) - memoryValue;

		setRegisterPair(RP_HL, getRegisterPair(RP_HL) + 1);
		setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

		setFlagBit(F_S, ((s8)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, ((getRegister(R_A) & 0xF) < (memoryValue & 0xF)));
		setFlagBit(F_P, (getRegisterPair(RP_BC) != 0));
		setFlagBit(F_N, 1);
		setFlagUndocMethod2(result - static_cast<u8>(getFlagBit(F_H)));
		nbTStates = 16;
	}
	else if (y == 1 && x == 5) // CPD
	{
		u8 memoryValue = readMemoryHL();
		u8 result = getRegister(R_A) - memoryValue;

		setRegisterPair(RP_HL, getRegisterPair(RP_HL) - 1);
		setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

		setFlagBit(F_S, ((s8)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, ((getRegister(R_A) & 0xF) < (memoryValue & 0xF)));
		setFlagBit(F_P, (getRegisterPair(RP_BC)!=0));
		setFlagBit(F_N, 1);
		setFlagUndocMethod2(result - static_cast<u8>(getFlagBit(F_H)));
		nbTStates = 16;
	}
	else if (y == 1 && x == 6) // CPIR
	{
		u8 memoryValue = readMemoryHL();
		u8 result = getRegister(R_A) - memoryValue;

		setRegisterPair(RP_HL, getRegisterPair(RP_HL) + 1);
		setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

		if (getRegisterPair(RP_BC) != 0 && result != 0) {
			nbTStates = 21;
			_pc -= 2;
		} else {
			nbTStates = 16;
		}

		setFlagBit(F_S, ((s8)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, ((getRegister(R_A) & 0xF) < (memoryValue & 0xF)));
		setFlagBit(F_P, (getRegisterPair(RP_BC) != 0));
		setFlagBit(F_N, 1);
		setFlagUndocMethod2(result - static_cast<u8>(getFlagBit(F_H)));
	}
	else if (y == 1 && x == 7) // CPDR
	{
		u8 memoryValue = readMemoryHL();
		u8 result = getRegister(R_A) - memoryValue;

		setRegisterPair(RP_HL, getRegisterPair(RP_HL) - 1);
		setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

		if (getRegisterPair(RP_BC) != 0 && result != 0) {
			nbTStates = 21;
			_pc -= 2;
		} else {
			nbTStates = 16;
		}

		setFlagBit(F_S, ((s8)(result) < 0));
		setFlagBit(F_Z, (result == 0));
		setFlagBit(F_H, ((getRegister(R_A) & 0xF) < (memoryValue & 0xF)));
		setFlagBit(F_P, (getRegisterPair(RP_BC) != 0));
		setFlagBit(F_N, 1);
		setFlagUndocMethod2(result - static_cast<u8>(getFlagBit(F_H)));
	}
	//
	else if (y == 3 && x == 4) // OUTI
	{
		u8 value = readMemoryHL();
		//slog << ldebug << hex << "OUTI : R_C="<< (u16)_register[R_C] << " R_B=" << (u16)_register[R_B] << " RP_HL=" << (u16)getRegisterPair(2) << " val=" << (u16)value << endl;
		portCommunication(true, getRegister(R_C), value);

		setRegisterPair(RP_HL, getRegisterPair(RP_HL) + 1);
		setRegister(R_B, getRegister(R_B) - 1);

		nbTStates = 16;

		setFlagBit(F_S, (static_cast<s8>(getRegister(R_B)) < 0));
		setFlagBit(F_Z, (getRegister(R_B)==0));
		setFlagBit(F_N, getBit8(value, 7));
		setFlagUndoc(_register[R_B]);

		// From undocumented Z80 document
		u16 k = value + getRegister(R_L);
		setFlagBit(F_C, k > 255);
		setFlagBit(F_H, k > 255);
		setFlagBit(F_P, nbBitsEven((k & 7) ^ getRegister(R_B)));
	}
	else if (y == 3 && x == 5) // OUTD
	{
		s8 value = readMemoryHL();
		setRegister(R_B, getRegister(R_B) - 1);
		portCommunication(true, getRegister(R_C), value);

		setRegisterPair(RP_HL, getRegisterPair(RP_HL) - 1);

		nbTStates = 16;

		setFlagBit(F_S, (static_cast<s8>(getRegister(R_B)) < 0));
		setFlagBit(F_Z, (getRegister(R_B) == 0));
		setFlagBit(F_N, getBit8(value, 7));
		setFlagUndoc(_register[R_B]);

		// From undocumented Z80 document
		u16 k = value + getRegister(R_L);
		setFlagBit(F_C, k > 255);
		setFlagBit(F_H, k > 255);
		setFlagBit(F_P, nbBitsEven((k & 7) ^ getRegister(R_B)));
	}
	else if(y == 3 && x == 6) // OTIR
	{
		u8 value = readMemoryHL();
		//SLOG(ldebug << hex << "OTIR : R_C="<< (u16)_register[R_C] << " R_B=" << (u16)_register[R_B] << " RP_HL=" << (u16)getRegisterPair(2) << " val=" << (u16)value);
		portCommunication(true, _register[R_C], value);

		setRegisterPair(RP_HL, getRegisterPair(RP_HL)+1);
		_register[R_B]--;

		if (_register[R_B] != 0) {
			_pc -= 2;
			//_isBlockInstruction = true;
			nbTStates = 21;
		} else {
			//_isBlockInstruction = false;
			nbTStates = 16;
		}


		setFlagBit(F_N, getBit8(value, 7));
		setFlagBit(F_S, ((s8)(_register[R_B]) < 0));
		setFlagBit(F_Z, (_register[R_B] == 0));
		setFlagUndoc(_register[R_B]);

		// From undocumented Z80 document
		u16 k = value + getRegister(R_L);
		setFlagBit(F_C, k > 255);
		setFlagBit(F_H, k > 255);
		setFlagBit(F_P, nbBitsEven((k&7) ^ getRegister(R_B)));

		//Debugger::Instance()->pause();
	}
	//
	else
	{
		NOT_IMPLEMENTED("BLI (" << (u16)x << ", " << (u16)y << ")");
	}

	return nbTStates;
}

int CPU::interrupt(bool nonMaskable)
{
	if (!nonMaskable && !_IFF1) {
		return -1;
	}

	_halt = false;

	if (nonMaskable) {
		SLOG(lnormal << "=== NMI ===");
		restart(0x66);
		_IFF1 = false;
		return 11;
	} else {
		if (_modeInt == 0) {
			_IFF1 = false;
			_IFF2 = false;
			// TODO: data bus
			SLOG_THROW(lwarning << "TODO interrupt mode 0");
		} else if (_modeInt == 1) {
			SLOG(lnormal << "=== IRQ ===");
			restart(0x38);
			_IFF1 = false;
			_IFF2 = false;
			return 13;
		} else if (_modeInt == 2) {
			_IFF1 = false;
			_IFF2 = false;
			SLOG_THROW(lwarning << "TODO interrupt mode 0");
		} else {
			SLOG_THROW(lerror << "Undefined interrupt mode");
		}
	}

	return -1;
}

void CPU::restart(uint_fast8_t p)
{
	_memory->write(--_sp, (_pc >> 8));
	_memory->write(--_sp, (_pc & 0xFF));
	_pc = p;
	SLOG(ldebug << "RST to " << hex << (u16)(p));
}

void CPU::addNbStates(int iNbStates)
{
	_graphics->addCpuStates(iNbStates);
	_cycleCount += iNbStates;
}

u8 CPU::readMemoryHL(bool iUseIndex) {
	if (iUseIndex && _useRegisterIX) {
		return _memory->read(_registerIX + retrieveIndexDisplacement());
	} else if (iUseIndex && _useRegisterIY) {
		return _memory->read(_registerIY + retrieveIndexDisplacement());
	} else {
		return _memory->read(getRegisterPair(RP_HL));
	}
}

void CPU::writeMemoryHL(u8 iNewValue, bool iUseIndex) {
	if (iUseIndex && _useRegisterIX) {
		_memory->write(_registerIX + retrieveIndexDisplacement(), iNewValue);
	} else if (iUseIndex && _useRegisterIY) {
		_memory->write(_registerIY + retrieveIndexDisplacement(), iNewValue);
	} else {
		_memory->write(getRegisterPair(RP_HL), iNewValue);
	}
}

s8 CPU::retrieveIndexDisplacement() {
	if (!_displacementForIndexUsed) {
		_displacementForIndexUsed = true;
		_displacementForIndex = static_cast<s8>(_memory->read(_pc++));
	}
	return _displacementForIndex;
}

void CPU::incrementRefreshRegister() {
	u8 tempReg = ((_registerR+1) & 0b0111'1111);
	_registerR = (_registerR & 0b1000'0000) | tempReg;
}

// swap:
void CPU::swapRegister(u8 code)
{
	u8 temp = _register[code];
	_register[code] = _registerA[code];
	_registerA[code] = temp;
}

// care ! code can only be 0, 1 or 2 ; 3 is not used
void CPU::swapRegisterPair(u8 code)
{
	u16 temp = getRegisterPair(code, false, false);
	setRegisterPair(code, getRegisterPair(code, true, false), false, false);
	setRegisterPair(code, temp, true, false);
}

void CPU::swapRegisterPair2(u8 code)
{
	u16 temp = getRegisterPair2(code, false, false);
	setRegisterPair2(code, getRegisterPair2(code, true, false), false, false);
	setRegisterPair2(code, temp, true, false);
}

// set:

void CPU::setRegister(u8 code, u8 value, bool alternate, bool useIndex)
{
	if (code == 6) {
		// TODO (IX/Y+d) for DD prefix
		writeMemoryHL(value, useIndex);
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

void CPU::setRegisterPair(u8 code, u16 value, bool alternate, bool useIndex)
{
	u8 first = (value >> 8);
	u8 second = (value & 0xFF);

	u8* r = _register;
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
			if (useIndex && code == 2 && _useRegisterIX) {
				_registerIX = value;
			} else if (useIndex && code == 2 && _useRegisterIY) {
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

	SLOG(ldebug << hex << "set RP[" << (u16)code << "] = " << value);
}

void CPU::setRegisterPair2(u8 code, u16 value, bool alternate, bool useIndex)
{
	u8 first = (value >> 8);
	u8 second = (value & 0xFF);

	u8* r = _register;
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
			if (useIndex && code == 2 && _useRegisterIX) {
				_registerIX = value;
			} else if (useIndex && code == 2 && _useRegisterIY) {
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

	SLOG(ldebug << hex << "set RP2[" << (u16)code << "] = " << value);
}

void CPU::setCBRegisterWithCopy(u8 iRegister, u8 iValue) {
    if (_useRegisterIX) {
        _memory->write(_registerIX + _displacementForIndex, iValue);
    } else if (_useRegisterIY) {
        _memory->write(_registerIY + _displacementForIndex, iValue);
    }

    if (!isIndexUsed() || iRegister != 6) {
        setRegister(iRegister, iValue, false, false);
    }
}


// get:

const u8 CPU::getRegister(u8 code, bool alternate, bool useIndex)
{
	if (code == 6) {
		return readMemoryHL(useIndex);
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

u16 CPU::getRegisterPair(u8 code, bool alternate, bool useIndex)
{
	// TODO verify HL
	u8* r = _register;
	if(alternate)
		r = _registerA;

	if (code < 3) {
		if (useIndex && code == 2 && _useRegisterIX) {
			return _registerIX;
		} else if (useIndex && code == 2 && _useRegisterIY) {
			return _registerIY;
		}
		return ((r[code * 2] << 8) + r[code * 2 + 1]);
	}

	return _sp;
}

u16 CPU::getRegisterPair2(u8 code, bool alternate, bool useIndex)
{
	u8* r = _register;
	if(alternate)
		r = _registerA;

	if (code < 3) {
		if (useIndex && code == 2 && _useRegisterIX) {
			return _registerIX;
		} else if (useIndex && code == 2 && _useRegisterIY) {
			return _registerIY;
		}
		return ((r[code * 2] << 8) + r[code * 2 + 1]);
	}

	if(alternate)
		return ((r[R_A]<<8) + _registerFlagA);

	return ((r[R_A]<<8) + _registerFlag);
}

#include "CPU.h"

#include <functional>
#include <iomanip>
#include <sstream>
#include <string>

#include "Audio.h"
#include "Cartridge.h"
#include "Memory.h"
#include "System.h"

#include "CPU_opcode_0_cpp.inc"
#include "CPU_opcode_cb_cpp.inc"
#include "CPU_opcode_ed_cpp.inc"

using namespace std;

CPU::CPU(System& parent)
	: _audioRef { parent.getAudio() },
	  _cartridgeRef { parent.getCartridge() },
	  _graphicsRef { parent.getGraphics() },
	  _memoryRef { parent.getMemory() },
	  _IFF1 { false },
	  _IFF2 { false },
	  _pc { 0 },
	  _sp { 0 },
	  _modeInt { 0 },
	  _halt { 0 },
	  _register { 0 },
	  _useRegisterIX { 0 },
	  _useRegisterIY { 0 },
	  _displacementForIndexUsed { false },
	  _displacementForIndex { 0 },
	  _registerAluTemp { 0 }

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

	for(int i = 0; i < REGISTER_SIZE; i++)
		_register[i] = 0;

	for(int i = 0; i < REGISTER_SIZE; i++)
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
	// setRegister(R_A, 0xFF);
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
	if(!_isInitialized) {
		SLOG_THROW(lerror << "CPU not initialized");
		return 0;
	}

	int nbStates = 0;
	bool canExecute = true;

	if(_enableInterruptWaiting == 0 && !_isBlockInstruction && _graphics->getIE()) {
		nbStates = interrupt();
		if(nbStates > 0) {
			canExecute = false;
		}
	}
	if(_enableInterruptWaiting > 0) {
		_enableInterruptWaiting--;
	}

	if(_halt) {
		nbStates = 4;
		canExecute = false;
	}

	if(canExecute) {
		SLOG(ldebug << hex << "HL:" << getRegisterPair(RP_HL)
					<< " SP:" << (u16)(_sp) /* << " Stack:" << (u16)_stack[_sp + 1] << "," << (u16)_stack[_sp]*/);

		u8 prefix = _memory->read(_pc++);
		u8 opcode = prefix;
		incrementRefreshRegister();

		if(prefix == 0xCB || prefix == 0xDD || prefix == 0xED || prefix == 0xED || prefix == 0xFD) {
			opcode = _memory->read(_pc++);
			incrementRefreshRegister();
		} else
			prefix = 0;

		nbStates = opcodeExecution(prefix, opcode);

		if(nbStates == 0) {
			SLOG_THROW(lerror << hex << "No T state given! #" << (u16)(_pc - 1 - (prefix != 0 ? 1 : 0)) << " : "
							  << (u16)opcode << " (" << ((u16)prefix) << ")");
			nbStates = 4;
		}
	}

	if(nbStates == 0) {
		SLOG_THROW(lerror << hex << "No T state given!");
		nbStates = 4;
	}

	addNbStates(nbStates);

	//_audio->run(); // to put in main loop
	return nbStates;
}

int CPU::opcodeExecution(const u8 prefix, const u8 opcode)
{
	SLOG(lnormal << hex << std::setfill('0') << std::uppercase << "#" << (u16)(_pc - 1 - (prefix != 0 ? 1 : 0)) << " "
				 << std::setw(4) << getRegisterPair2(RP2_AF) << " " << std::setw(4) << getRegisterPair(RP_BC) << " "
				 << std::setw(4) << getRegisterPair(RP_DE) << " " << std::setw(4) << getRegisterPair(RP_HL) << " "
				 << std::setw(4) << _registerIX << " " << std::setw(4) << _registerIY << " " << std::setw(4)
				 << getRegisterPair(RP_SP) << " " << std::dec << _cycleCount << " ");
	SLOG_NOENDL(ldebug << hex << "#" << (u16)(_pc - 1 - (prefix != 0 ? 1 : 0)) << " : " << (u16)opcode);
	if(prefix != 0) {
		SLOG_NOENDL("(" << (u16)prefix << ")");
	}
	SLOG(ldebug << " --" << getOpcodeName(prefix, opcode) << "--");

	Stats::add(prefix, opcode);

	resInstruction res;
	int nbStates = 0;

	try {

		switch(prefix) {
			case 0:
				nbStates = opcode0(opcode);
				break;
			case 0xCB:
				nbStates = opcodeCB(opcode);
				break;
			case 0xED:
				nbStates = opcodeED(opcode);
				break;
			case 0xDD:
				nbStates = opcodeDD(opcode);
				break;
			case 0xFD:
				nbStates = opcodeFD(opcode);
				break;
		}

	} catch(const EmulatorException& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	} catch(const std::exception& e) {
		std::cerr << "Unknown Exception: " << e.what() << std::endl;
	}

	consumeRegisterIX();
	consumeRegisterIY();

	return nbStates;
}

void CPU::aluOperation(const u8 index, const u8 value)
{
	switch(index) {
		case 0: // ADD A
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
		} break;
		case 1: // ADC
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
		} break;
		case 2: // SUB
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
		} break;
		case 3: // SBC
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
		} break;
		case 4: // AND
		{
			_register[R_A] = (_register[R_A] & value);
			setFlagBit(F_S, ((s8)(_register[R_A]) < 0));
			setFlagBit(F_Z, (_register[R_A] == 0));
			setFlagBit(F_H, 1);
			setFlagBit(F_P, nbBitsEven(_register[R_A]));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, 0);
			setFlagUndoc(_register[R_A]);
		} break;
		case 5: // XOR
		{
			_register[R_A] ^= value;
			setFlagBit(F_S, ((s8)(_register[R_A]) < 0));
			setFlagBit(F_Z, (_register[R_A] == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(_register[R_A]));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, 0);
			setFlagUndoc(_register[R_A]);
		} break;
		case 6: // OR
		{
			_register[R_A] |= value;
			setFlagBit(F_S, ((s8)(_register[R_A]) < 0));
			setFlagBit(F_Z, (_register[R_A] == 0));
			setFlagBit(F_H, 0);
			setFlagBit(F_P, nbBitsEven(_register[R_A]));
			setFlagBit(F_N, 0);
			setFlagBit(F_C, 0);
			setFlagUndoc(_register[R_A]);
		} break;
		case 7: // CP
		{
			/// TODO vérifier borrows & overflow
			u8 sum = _register[R_A] - value;
			setFlagBit(F_S, ((s8)(sum) < 0));
			setFlagBit(F_Z, (sum == 0));
			setFlagBit(F_H, ((_register[R_A] & 0xF) < (value & 0xF)));
			setFlagBit(F_P, ((sign8(_register[R_A]) != sign8(sum)) && (sign8(value) == sign8(sum))));
			setFlagBit(F_N, 1);
			setFlagBit(F_C, (sum > _register[R_A])); // ?
			setFlagUndoc(value);

			SLOG(ldebug << "CP(" << hex << (u16)_register[R_A] << "," << (u16)value << "): " << (u16)(sum == 0));
		} break;
		default:
			slog << lwarning << "ALU " << (u16)index << " is not implemented" << endl;
			break;
	}
}

void CPU::rotOperation(const u8 index, const u8 reg) { NOT_IMPLEMENTED("NOT USED ANYMORE, see rot[y] r[z]"); }

u8 CPU::portCommunication(bool rw, u8 address, u8 data)
{
	SLOG(ldebug << hex << (!rw ? "<" : "") << "-----------" << (rw ? ">" : "") << " rw:" << rw << " a:" << (u16)address
				<< " d:" << (u16)data);

	switch(address) {
		case 0x3E:
			if(rw) {
				if((data >> 2) != 0b111011) {
					stringstream s;
					s << "Mapping other than ram (" << hex << (u16)(data & 0xF) << ")";
					NOT_IMPLEMENTED(s.str());
				}
			} else {
				NOT_IMPLEMENTED("READ 0x3E port");
			}
			break;
		case 0x3F:
			if(rw) {
				_ioPortControl = data;
			} else {
				NOT_IMPLEMENTED("READ 0x3F port");
			}
			break;
		case 0xBE:
		case 0xBF:
			if(rw) {
				_graphics->write(address, data);
			} else {
				return _graphics->read(address);
			}
			break;
		case 0x7E:
		case 0x7F:
			if(rw) {
				// TODO(ibancel): audio
				_audio->write(address, data);
			} else {
				return _graphics->read(address);
			}
			break;
		case 0xDE:
		case 0xDF:
			/// TODO: fully understand?
			NOT_IMPLEMENTED("No 8255 PPI (Used for SG-1000 and SC-3000)");
			if(!rw) {
				if(address == 0xDE) {
					return portCommunication(false, 0xDC);
				} else { // DF
					return portCommunication(false, 0xDD);
				}
			}
			// NOT_IMPLEMENTED("I18n");
			break;
		case 0xDC:
		case 0xC0: {
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
		} break;
		case 0xDD:
		case 0xC1: {
			u8 returnCode = 0;
			if(_ioPortControl == 0xF5) {
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
		} break;
		default:
			slog << lwarning << hex << "Communication port 0x" << (u16)address << " is not implemented (rw=" << rw
				 << ")" << endl;
			break;
	}

	return 0;
}

/// PRIVATE :

int CPU::opcode0(u8 opcode) { return Instructions_op_0[OPCODE_TO_INDEX(opcode)](this, opcode); }

int CPU::opcodeCB(u8 opcode) { return Instructions_op_cb[OPCODE_TO_INDEX(opcode)](this, opcode); }

int CPU::opcodeED(u8 opcode) { return Instructions_op_ed[OPCODE_TO_INDEX(opcode)](this, opcode); }

int CPU::opcodeDD(const u8 opOrPrefix)
{
	int nbTStates = 0;

	switch(opOrPrefix) {
		case 0xCB: {
			// DDCB prefix
			useRegisterIX();
			_cbDisplacement = static_cast<s8>(_memory->read(_pc++));
			_displacementForIndex = _cbDisplacement;
			_displacementForIndexUsed = true;
			const u8 opcode = _memory->read(_pc++);
			nbTStates = opcodeCB(opcode);
			stopRegisterIX();
			_displacementForIndexUsed = false;
		} break;
		case 0xDD:
		case 0xED:
		case 0xFD:
			/// TODO NONI
			_pc--;
			nbTStates = 4; // to check
			break;
		default: {
			useRegisterIX();
			_displacementForIndexUsed = false;
			nbTStates = opcode0(opOrPrefix);
			stopRegisterIX();
			_displacementForIndexUsed = false;
		}
	}

	return nbTStates;
}

int CPU::opcodeFD(const u8 opOrPrefix)
{
	int nbTStates = 0;

	switch(opOrPrefix) {
		case 0xCB: {
			useRegisterIY();
			_cbDisplacement = static_cast<s8>(_memory->read(_pc++));
			_displacementForIndex = _cbDisplacement;
			_displacementForIndexUsed = true;
			const u8 opcode = _memory->read(_pc++);
			nbTStates = opcodeCB(opcode);
			stopRegisterIY();
			_displacementForIndexUsed = false;
		} break;
		case 0xDD:
		case 0xED:
		case 0xFD:
			/// TODO NONI
			_pc--;
			nbTStates = 4; // to check
			break;
		default: {
			useRegisterIY();
			_displacementForIndexUsed = false;
			nbTStates = opcode0(opOrPrefix);
			stopRegisterIY();
			_displacementForIndexUsed = false;
		}
	}

	return nbTStates;
}

bool CPU::condition(u8 code)
{
	static const std::function<bool()> funcCondition[] = {
		[&]() -> bool { return !getFlagBit(F_Z); },
		[&]() -> bool { return getFlagBit(F_Z); },
		[&]() -> bool { return !getFlagBit(F_C); },
		[&]() -> bool { return getFlagBit(F_C); },
		[&]() -> bool { return !getFlagBit(F_P); },
		[&]() -> bool { return getFlagBit(F_P); },
		[&]() -> bool { return !getFlagBit(F_S); },
		[&]() -> bool { return getFlagBit(F_S); },
	};

	return funcCondition[code]();
}

// int CPU::bliOperation(u8 x, u8 y)
// {
// 	int nbTStates = 0;

// 	if(y == 0 && x == 4) // LDI
// 	{

// 	} else if(y == 0 && x == 5) // LDD
// 	{

// 	} else if(y == 0 && x == 6) // LDIR
// 	{

// 	} else if(y == 0 && x == 7) // LDDR
// 	{

// 	} else if(y == 1 && x == 4) // CPI
// 	{

// 	} else if(y == 1 && x == 5) // CPD
// 	{

// 	} else if(y == 1 && x == 6) // CPIR
// 	{

// 	} else if(y == 1 && x == 7) // CPDR
// 	{

// 	}
// 	//
// 	else if(y == 3 && x == 4) // OUTI
// 	{

// 	} else if(y == 3 && x == 5) // OUTD
// 	{

// 	} else if(y == 3 && x == 6) // OTIR
// 	{

// 	}
// 	//
// 	else {
// 	}

// 	return nbTStates;
// }

int CPU::interrupt(bool nonMaskable)
{
	if(!nonMaskable && !_IFF1) {
		return -1;
	}

	_halt = false;

	if(nonMaskable) {
		SLOG(lnormal << "=== NMI ===");
		restart(0x66);
		_IFF1 = false;
		return 11;
	} else {
		if(_modeInt == 0) {
			_IFF1 = false;
			_IFF2 = false;
			// TODO: data bus
			SLOG_THROW(lwarning << "TODO interrupt mode 0");
		} else if(_modeInt == 1) {
			SLOG(lnormal << "=== IRQ ===");
			restart(0x38);
			_IFF1 = false;
			_IFF2 = false;
			return 13;
		} else if(_modeInt == 2) {
			_IFF1 = false;
			_IFF2 = false;
			SLOG_THROW(lwarning << "TODO interrupt mode 2");
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

u8 CPU::readMemoryHL(bool iUseIndex)
{
	if(iUseIndex && _useRegisterIX) {
		return _memory->read(_registerIX + retrieveIndexDisplacement());
	} else if(iUseIndex && _useRegisterIY) {
		return _memory->read(_registerIY + retrieveIndexDisplacement());
	} else {
		return _memory->read(getRegisterPair(RP_HL));
	}
}

void CPU::writeMemoryHL(u8 iNewValue, bool iUseIndex)
{
	if(iUseIndex && _useRegisterIX) {
		_memory->write(_registerIX + retrieveIndexDisplacement(), iNewValue);
	} else if(iUseIndex && _useRegisterIY) {
		_memory->write(_registerIY + retrieveIndexDisplacement(), iNewValue);
	} else {
		_memory->write(getRegisterPair(RP_HL), iNewValue);
	}
}

s8 CPU::retrieveIndexDisplacement()
{
	if(!_displacementForIndexUsed) {
		_displacementForIndexUsed = true;
		_displacementForIndex = static_cast<s8>(_memory->read(_pc++));
	}
	return _displacementForIndex;
}

void CPU::incrementRefreshRegister()
{
	u8 tempReg = ((_registerR + 1) & 0b0111'1111);
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
	if(alternate) {
		NOT_IMPLEMENTED("alternate register");
	}

	switch(code) {
		case R_H:
			if(!useIndex || (!_useRegisterIX && !_useRegisterIY)) {
				_register[code] = value;
			} else if(_useRegisterIX) {
				setHigherByte(_registerIX, value);
			} else {
				setHigherByte(_registerIY, value);
			}
			break;
		case R_L:
			if(!useIndex || (!_useRegisterIX && !_useRegisterIY)) {
				_register[code] = value;
			} else if(_useRegisterIX) {
				setLowerByte(_registerIX, value);
			} else {
				setLowerByte(_registerIY, value);
			}
			break;
		case 6:
			// TODO (IX/Y+d) for DD prefix
			writeMemoryHL(value, useIndex);
			break;
		default:
			_register[code] = value;
			break;
	}
}

void CPU::setRegisterPair(u8 code, u16 value, bool alternate, bool useIndex)
{
	u8 first = (value >> 8);
	u8 second = (value & 0xFF);

	u8* r = _register;
	if(alternate)
		r = _registerA;

	switch(code) {
		case 0:
			r[R_B] = first;
			r[R_C] = second;
			break;
		case 1:
			r[R_D] = first;
			r[R_E] = second;
			break;
		case 2:
			if(useIndex && code == 2 && _useRegisterIX) {
				_registerIX = value;
			} else if(useIndex && code == 2 && _useRegisterIY) {
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

	switch(code) {
		case 0:
			r[R_B] = first;
			r[R_C] = second;
			break;
		case 1:
			r[R_D] = first;
			r[R_E] = second;
			break;
		case 2:
			if(useIndex && code == 2 && _useRegisterIX) {
				_registerIX = value;
			} else if(useIndex && code == 2 && _useRegisterIY) {
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

void CPU::setCBRegisterWithCopy(u8 iRegister, u8 iValue)
{
	if(_useRegisterIX) {
		_memory->write(_registerIX + _displacementForIndex, iValue);
	} else if(_useRegisterIY) {
		_memory->write(_registerIY + _displacementForIndex, iValue);
	}

	if(!isIndexUsed() || iRegister != 6) {
		setRegister(iRegister, iValue, false, false);
	}
}

// get:

u8 CPU::getRegister(u8 code, bool alternate, bool useIndex)
{
	if(alternate) {
		NOT_IMPLEMENTED("alternate register");
	}

	switch(code) {
		case R_H:
			if(!useIndex || (!_useRegisterIX && !_useRegisterIY)) {
				return _register[code];
			} else if(_useRegisterIX) {
				return getHigherByte(_registerIX);
			} else {
				return getHigherByte(_registerIY);
			}
			break;
		case R_L:
			if(!useIndex || (!_useRegisterIX && !_useRegisterIY)) {
				return _register[code];
			} else if(_useRegisterIX) {
				return getLowerByte(_registerIX);
			} else {
				return getLowerByte(_registerIY);
			}
			break;
		case 6:
			return readMemoryHL(useIndex);
			break;
	}

	return _register[code];
}

u16 CPU::getRegisterPair(u8 code, bool alternate, bool useIndex) const
{
	// TODO verify HL
	const u8* r = _register;
	if(alternate)
		r = _registerA;

	if(code < 3) {
		if(useIndex && code == 2 && _useRegisterIX) {
			return _registerIX;
		} else if(useIndex && code == 2 && _useRegisterIY) {
			return _registerIY;
		}
		return ((r[code * 2] << 8) + r[code * 2 + 1]);
	}

	return _sp;
}

u16 CPU::getRegisterPair2(u8 code, bool alternate, bool useIndex) const
{
	const u8* r = _register;
	if(alternate)
		r = _registerA;

	if(code < 3) {
		if(useIndex && code == 2 && _useRegisterIX) {
			return _registerIX;
		} else if(useIndex && code == 2 && _useRegisterIY) {
			return _registerIY;
		}
		return ((r[code * 2] << 8) + r[code * 2 + 1]);
	}

	if(alternate)
		return ((r[R_A] << 8) + _registerFlagA);

	return ((r[R_A] << 8) + _registerFlag);
}

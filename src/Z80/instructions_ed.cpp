#include "CPU.h"

using namespace std;

int CPU::_instruction_NOT_IMPLEMENTED(u8 opcode)
{
	SLOG_THROW(lwarning << "Opcode : " << hex << (u16)(opcode) << " (ED) is not implemented");
}

int CPU::_instruction_noni_nop(u8 opcode) { return 4 + 4; }

int CPU::_instruction_out16_0(u8 opcode)
{
	addNbStates(6); // TODO: determine precisely the number of states before
	portCommunication(true, _register[R_C], 0);
	return 6; // to check!
}

int CPU::_instruction_out16_r(u8 opcode)
{
	addNbStates(5); // TODO: determine precisely the number of states before
	portCommunication(true, _register[R_C], getRegister(OPCODE_Y(opcode)));
	return 6;
}

int CPU::_instruction_sbc_rp(u8 opcode)
{
	u16 value = getRegisterPair(RP_HL);
	u16 minusOp = getRegisterPair(OPCODE_P(opcode)) + getFlagBit(F_C);
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

	return 15;
}

int CPU::_instruction_adc_rp(u8 opcode)
{
	u16 value = getRegisterPair(RP_HL);
	u16 addOp = getRegisterPair(OPCODE_P(opcode)) + getFlagBit(F_C);
	u16 addOverflow = addOp == 0xFFFF && getFlagBit(F_C);
	u16 result = (u16)(value + addOverflow);

	setFlagBit(F_S, (static_cast<s16>(result) < 0));
	setFlagBit(F_Z, result == 0);
	setFlagBit(F_H, addOverflow || ((value & 0x0FFF) + (addOp & 0x0FFF) > 0x0FFF));
	setFlagBit(F_P, addOverflow || (sign16(value) == sign16(addOp) && sign16(addOp) != sign16(result)));
	setFlagBit(F_N, 0);
	setFlagBit(F_C, (addOverflow || (result < value) || (result < addOp)));
	setFlagUndoc(getHigherByte(result));

	return 15;
}

int CPU::_instruction_ld_nn_rp(u8 opcode)
{
	u16 registerValue = getRegisterPair(OPCODE_P(opcode));
	u16 addr = _memory->read(_pc++);
	addr += (_memory->read(_pc++) << 8);
	_memory->write(addr, getLowerByte(registerValue));
	_memory->write(addr + 1, getHigherByte(registerValue));
	return 20;
}

int CPU::_instruction_ld_rp_nn(u8 opcode)
{
	u16 addr = _memory->read(_pc++);
	addr += (_memory->read(_pc++) << 8);
	u16 registerValue = _memory->read(addr);
	registerValue += (_memory->read(addr + 1) << 8);
	setRegisterPair(OPCODE_P(opcode), registerValue);
	return 20;
}

int CPU::_instruction_neg(u8 opcode)
{
	s8 registerValue = -getRegister(R_A);

	setFlagBit(F_P, getRegister(R_A) == 0x80);
	setFlagBit(F_C, getRegister(R_A) != 0);

	setFlagBit(F_S, registerValue < 0);
	setFlagBit(F_Z, registerValue == 0);
	setFlagBit(F_H, ((getRegister(R_A) & 0x0F) > 0));
	setFlagBit(F_N, 1);

	setFlagUndoc(registerValue);

	setRegister(R_A, registerValue);
	return 8;
}

int CPU::_instruction_retn(u8 opcode)
{ // TODO: verify
	_IFF1 = _IFF2;

	_pc = _memory->read(_sp++);
	_pc += (_memory->read(_sp++) << 8);

	return 14;
}

int CPU::_instruction_reti(u8 opcode)
{
	_pc = _memory->read(_sp++);
	_pc += (_memory->read(_sp++) << 8);

	_IFF1 = _IFF2;
	/// TODO complete this with IEO daisy chain

	return 14;
}

int CPU::_instruction_im(u8 opcode)
{
	const u8 y = OPCODE_Y(opcode);
	if(y == 0)
		_modeInt = 0;
	else if(y == 2)
		_modeInt = 1;
	else if(_modeInt == 3)
		_modeInt = 2;

	return 8;
}

int CPU::_instruction_ld_r_a(u8 opcode)
{
	_registerR = getRegister(R_A);
	return 9;
}

int CPU::_instruction_ld_a_r(u8 opcode)
{
	setRegister(R_A, _registerR);
	setFlagBit(F_S, ((s8)(_registerR) < 0));
	setFlagBit(F_Z, (_registerR == 0));
	setFlagBit(F_H, 0);
	setFlagBit(F_P, _IFF2);
	setFlagBit(F_N, 0);
	return 9;
}

int CPU::_instruction_rrd(u8 opcode)
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
	return 18;
}

int CPU::_instruction_rld(u8 opcode)
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
	return 18;
}

int CPU::_instruction_bli_ldi(u8 opcode)
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

	return 16;
}

int CPU::_instruction_bli_ldd(u8 opcode)
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

	return 16;
}

int CPU::_instruction_bli_ldir(u8 opcode)
{
	int nbTStates = 16;

	_registerAluTemp = readMemoryHL();

	_memory->write(getRegisterPair(RP_DE), _registerAluTemp);
	setRegisterPair(RP_DE, getRegisterPair(RP_DE) + 1);
	setRegisterPair(RP_HL, getRegisterPair(RP_HL) + 1);
	setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

	if(getRegisterPair(RP_BC) != 0) {
		_pc -= 2;
		nbTStates = 21;
	}

	setFlagBit(F_H, 0);
	setFlagBit(F_N, 0);
	setFlagBit(F_P, getRegisterPair(RP_BC) != 0);
	setFlagUndocMethod2(getAluTempByte());

	return nbTStates;
}

int CPU::_instruction_bli_lddr(u8 opcode)
{
	int nbTStates = 16;

	_registerAluTemp = readMemoryHL();

	_memory->write(getRegisterPair(RP_DE), _registerAluTemp);
	setRegisterPair(RP_DE, getRegisterPair(RP_DE) - 1);
	setRegisterPair(RP_HL, getRegisterPair(RP_HL) - 1);
	setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

	if(getRegisterPair(RP_BC) != 0) {
		_pc -= 2;
		nbTStates = 21;
	}

	setFlagBit(F_H, 0);
	setFlagBit(F_N, 0);
	setFlagBit(F_P, getRegisterPair(RP_BC) != 0);
	setFlagUndocMethod2(getAluTempByte());

	return nbTStates;
}

int CPU::_instruction_bli_cpi(u8 opcode)
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
	return 16;
}

int CPU::_instruction_bli_cpd(u8 opcode)
{
	u8 memoryValue = readMemoryHL();
	u8 result = getRegister(R_A) - memoryValue;

	setRegisterPair(RP_HL, getRegisterPair(RP_HL) - 1);
	setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

	setFlagBit(F_S, ((s8)(result) < 0));
	setFlagBit(F_Z, (result == 0));
	setFlagBit(F_H, ((getRegister(R_A) & 0xF) < (memoryValue & 0xF)));
	setFlagBit(F_P, (getRegisterPair(RP_BC) != 0));
	setFlagBit(F_N, 1);
	setFlagUndocMethod2(result - static_cast<u8>(getFlagBit(F_H)));
	return 16;
}

int CPU::_instruction_bli_cpir(u8 opcode)
{
	int nbTStates = 16;
	u8 memoryValue = readMemoryHL();
	u8 result = getRegister(R_A) - memoryValue;

	setRegisterPair(RP_HL, getRegisterPair(RP_HL) + 1);
	setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

	if(getRegisterPair(RP_BC) != 0 && result != 0) {
		nbTStates = 21;
		_pc -= 2;
	}

	setFlagBit(F_S, ((s8)(result) < 0));
	setFlagBit(F_Z, (result == 0));
	setFlagBit(F_H, ((getRegister(R_A) & 0xF) < (memoryValue & 0xF)));
	setFlagBit(F_P, (getRegisterPair(RP_BC) != 0));
	setFlagBit(F_N, 1);
	setFlagUndocMethod2(result - static_cast<u8>(getFlagBit(F_H)));

	return nbTStates;
}

int CPU::_instruction_bli_cpdr(u8 opcode)
{
	int nbTStates = 16;

	u8 memoryValue = readMemoryHL();
	u8 result = getRegister(R_A) - memoryValue;

	setRegisterPair(RP_HL, getRegisterPair(RP_HL) - 1);
	setRegisterPair(RP_BC, getRegisterPair(RP_BC) - 1);

	if(getRegisterPair(RP_BC) != 0 && result != 0) {
		nbTStates = 21;
		_pc -= 2;
	}

	setFlagBit(F_S, ((s8)(result) < 0));
	setFlagBit(F_Z, (result == 0));
	setFlagBit(F_H, ((getRegister(R_A) & 0xF) < (memoryValue & 0xF)));
	setFlagBit(F_P, (getRegisterPair(RP_BC) != 0));
	setFlagBit(F_N, 1);
	setFlagUndocMethod2(result - static_cast<u8>(getFlagBit(F_H)));

	return nbTStates;
}

int CPU::_instruction_bli_outi(u8 opcode)
{
	u8 value = readMemoryHL();
	// slog << ldebug << hex << "OUTI : R_C="<< (u16)_register[R_C] << " R_B=" << (u16)_register[R_B] << " RP_HL="
	// << (u16)getRegisterPair(2) << " val=" << (u16)value << endl;
	portCommunication(true, getRegister(R_C), value);

	setRegisterPair(RP_HL, getRegisterPair(RP_HL) + 1);
	setRegister(R_B, getRegister(R_B) - 1);

	setFlagBit(F_S, (static_cast<s8>(getRegister(R_B)) < 0));
	setFlagBit(F_Z, (getRegister(R_B) == 0));
	setFlagBit(F_N, getBit8(value, 7));
	setFlagUndoc(_register[R_B]);

	// From undocumented Z80 document
	u16 k = value + getRegister(R_L);
	setFlagBit(F_C, k > 255);
	setFlagBit(F_H, k > 255);
	setFlagBit(F_P, nbBitsEven((k & 7) ^ getRegister(R_B)));

	return 16;
}

int CPU::_instruction_bli_outd(u8 opcode)
{
	s8 value = readMemoryHL();
	setRegister(R_B, getRegister(R_B) - 1);
	portCommunication(true, getRegister(R_C), value);

	setRegisterPair(RP_HL, getRegisterPair(RP_HL) - 1);

	setFlagBit(F_S, (static_cast<s8>(getRegister(R_B)) < 0));
	setFlagBit(F_Z, (getRegister(R_B) == 0));
	setFlagBit(F_N, getBit8(value, 7));
	setFlagUndoc(_register[R_B]);

	// From undocumented Z80 document
	u16 k = value + getRegister(R_L);
	setFlagBit(F_C, k > 255);
	setFlagBit(F_H, k > 255);
	setFlagBit(F_P, nbBitsEven((k & 7) ^ getRegister(R_B)));

	return 16;
}

int CPU::_instruction_bli_otir(u8 opcode)
{
	int nbTStates = 16;

	u8 value = readMemoryHL();
	// SLOG(ldebug << hex << "OTIR : R_C="<< (u16)_register[R_C] << " R_B=" << (u16)_register[R_B] << " RP_HL=" <<
	// (u16)getRegisterPair(2) << " val=" << (u16)value);
	portCommunication(true, _register[R_C], value);

	setRegisterPair(RP_HL, getRegisterPair(RP_HL) + 1);
	_register[R_B]--;

	if(_register[R_B] != 0) {
		_pc -= 2;
		//_isBlockInstruction = true;
		nbTStates = 21;
	} else {
		//_isBlockInstruction = false;
	}

	setFlagBit(F_N, getBit8(value, 7));
	setFlagBit(F_S, ((s8)(_register[R_B]) < 0));
	setFlagBit(F_Z, (_register[R_B] == 0));
	setFlagUndoc(_register[R_B]);

	// From undocumented Z80 document
	u16 k = value + getRegister(R_L);
	setFlagBit(F_C, k > 255);
	setFlagBit(F_H, k > 255);
	setFlagBit(F_P, nbBitsEven((k & 7) ^ getRegister(R_B)));

	// Debugger::Instance()->pause();
	return nbTStates;
}

int CPU::_instruction_bli_NOT_IMPL(u8 opcode)
{
	NOT_IMPLEMENTED("BLI (" << (u16)OPCODE_Y(opcode) << ", " << (u16)OPCODE_Z(opcode) << ")");
}

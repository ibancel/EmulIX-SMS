#include "CPU.h"

using namespace std;

int CPU::_instruction_rot_r(u8 opcode)
{
	int nbTStates = 0;

	const u8 y = OPCODE_Y(opcode);
	const u8 z = OPCODE_Z(opcode);

	const u8 index = y;
	u8 value = getRegister(z, false, false);
	if(_useRegisterIX) {
		value = _memory->read(_registerIX + _cbDisplacement);
	} else if(_useRegisterIY) {
		value = _memory->read(_registerIY + _cbDisplacement);
	}

	if(_useRegisterIX || _useRegisterIY) {
		nbTStates = 6;
	} else if(z == 6) {
		nbTStates = 4;
	} else {
		nbTStates = 2;
	}

	switch(index) {
		case 0: // RLC
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

			if(_useRegisterIX || _useRegisterIY) {
				nbTStates = 23;
			} else if(z == 6) {
				nbTStates = 15;
			} else {
				nbTStates = 8;
			}
		} break;
		case 1: // RRC
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

			if(_useRegisterIX || _useRegisterIY) {
				nbTStates = 23;
			} else if(z == 6) {
				nbTStates = 15;
			} else {
				nbTStates = 8;
			}
		} break;
		case 2: // RL
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
		} break;
		case 3: // RR
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
		} break;
		case 4: // SLA
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
		} break;
		case 5: // SRA
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
		} break;
		case 6: // SLL
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
		} break;
		case 7: // SRL
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
		} break;
		default:
			slog << lwarning << "ROT " << (u16)index << " is not implemented" << endl;
			break;
	}

	return nbTStates;
}

int CPU::_instruction_bit_r(u8 opcode)
{
	int nbTStates = 0;

	const u8 y = OPCODE_Y(opcode);
	const u8 z = OPCODE_Z(opcode);

	u8 value = getRegister(z, false, false);
	if(_useRegisterIX) {
		value = _memory->read(_registerIX + _cbDisplacement);
	} else if(_useRegisterIY) {
		value = _memory->read(_registerIY + _cbDisplacement);
	}

	bool isBitSet = static_cast<bool>(getBit8(value, y));
	bool flagZ = !isBitSet;
	setFlagBit(F_S, ((y == 7) && isBitSet));
	setFlagBit(F_H, 1);
	setFlagBit(F_N, 0);
	setFlagBit(F_Z, flagZ);
	setFlagBit(F_P, flagZ);
	if(_useRegisterIX) {
		setFlagBit(F_F3, getBit8(getHigherByte(_registerIX + _cbDisplacement), 3));
		setFlagBit(F_F5, getBit8(getHigherByte(_registerIX + _cbDisplacement), 5));
	} else if(_useRegisterIY) {
		setFlagBit(F_F3, getBit8(getHigherByte(_registerIY + _cbDisplacement), 3));
		setFlagBit(F_F5, getBit8(getHigherByte(_registerIY + _cbDisplacement), 5));
	} else {
		setFlagBit(F_F3, (y == 3 && isBitSet));
		setFlagBit(F_F5, (y == 5 && isBitSet));
	}

	if(_useRegisterIX || _useRegisterIY) {
		nbTStates = 20;
	} else if(z == 6) {
		nbTStates = 12;
	} else {
		nbTStates = 8;
	}

	return nbTStates;
}

int CPU::_instruction_res_r(u8 opcode)
{
	int nbTStates = 0;

	const u8 y = OPCODE_Y(opcode);
	const u8 z = OPCODE_Z(opcode);

	u8 value = getRegister(z, false, false);
	if(_useRegisterIX) {
		value = _memory->read(_registerIX + _displacementForIndex);
	} else if(_useRegisterIY) {
		value = _memory->read(_registerIY + _displacementForIndex);
	}

	setBit8(&value, y, 0);

	setCBRegisterWithCopy(z, value);

	if(_useRegisterIX || _useRegisterIY) {
		nbTStates = 6;
	} else {
		nbTStates = 4;
	}

	return nbTStates;
}

int CPU::_instruction_set_r(u8 opcode)
{
	int nbTStates = 0;

	const u8 y = OPCODE_Y(opcode);
	const u8 z = OPCODE_Z(opcode);

	u8 value = getRegister(z, false, false);
	if(_useRegisterIX) {
		value = _memory->read(_registerIX + _displacementForIndex);
	} else if(_useRegisterIY) {
		value = _memory->read(_registerIY + _displacementForIndex);
	}

	setBit8(&value, y, 1);

	setCBRegisterWithCopy(z, value);

	if(_useRegisterIX || _useRegisterIY) {
		nbTStates = 23;
	} else if(z == 6) {
		nbTStates = 15;
	} else {
		nbTStates = 8;
	}

	return nbTStates;
}

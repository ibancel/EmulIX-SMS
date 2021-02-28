#include "definitions.h"

std::string getOpcodeName(u8 prefix, u8 opcode)
{
	u8 x = ((opcode & 0b11000000) >> 6);
	u8 y = ((opcode & 0b00111000) >> 3);
	u8 z = (opcode & 0b00000111);
	if(prefix == 0xCB) {
		return OPCODE_CB_NAME[x];
	} else if(prefix == 0xED) {
		if(x == 2 && y >= 4) {
			return "bli[y,z]";
		} else if(x != 1) {
			return "NOP & NONI";
		}
		return OPCODE_ED_NAME[(z << 3) + y];
	}
	// else:
	u8 value = (x << 6) + (z << 3) + y;
	return OPCODE_NAME[value];
}

void setBit8(u8* value, u8 pos, bool newBit)
{
	if(!newBit)
		*value &= ~(1 << (u8)pos);
	else
		*value |= 1 << (u8)pos;
}

void setBit16(u16* value, u8 pos, bool newBit)
{
	if(!newBit) {
		*value &= ~(1 << (u8)pos);
	} else {
		*value |= 1 << (u8)pos;
	}
}

bool nbBitsEven(u8 byte)
{
	byte ^= byte >> 4;
	byte ^= byte >> 2;
	byte ^= byte >> 1;
	return (~byte) & 1;
}

#include "definitions.h"

std::string getOpcodeName(uint8_t prefix, uint8_t opcode)
{
	uint8_t x = ((opcode & 0b11000000) >> 6);
	uint8_t y = ((opcode & 0b00111000) >> 3);
	uint8_t z = (opcode & 0b00000111);
    if(prefix == 0xCB) {
        return OPCODE_CB_NAME[x];
	} else if (prefix == 0xED) {
		if (x == 2 && y >= 4) {
			return "bli[y,z]";
		} else if (x != 1) {
			return "NOP & NONI";
		}
		return OPCODE_ED_NAME[(z << 3) + y];
	}
    // else:
    uint8_t value = (x << 6) + (z << 3) + y;
    return OPCODE_NAME[value];
}


void setBit8(uint8_t* value, uint8_t pos, bool newBit)
{
	if(!newBit)
		*value &= ~(1 << (uint8_t)pos);
	else
		*value |= 1 << (uint8_t)pos;
}

bool nbBitsEven(uint8_t byte)
{
	byte ^= byte >> 4;
	byte ^= byte >> 2;
	byte ^= byte >> 1;
	return (~byte) & 1;
}

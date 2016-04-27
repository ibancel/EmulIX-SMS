#include "definitions.h"

std::string getOpcodeName(uint8_t opcode)
{
   uint8_t value = (opcode & 0b11000000) + ((opcode & 0b00111000) >> 3) + ((opcode & 0b00000111) << 3);
   return OPCODE_NAME[value];
}

uint8_t getBit8(uint8_t value, uint8_t pos)
{
	return (value >> pos) & 0b1;
}

bool nbBitsEven(uint8_t byte)
{
	byte ^= byte >> 4;
	byte ^= byte >> 2;
	byte ^= byte >> 1;
	return (~byte) & 1;
}

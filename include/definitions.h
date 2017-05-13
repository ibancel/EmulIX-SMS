#ifndef _H_EMULATOR_DEFINITIONS
#define _H_EMULATOR_DEFINITIONS

#include <iostream>

#define DEBUG_MODE true
#define STEP_BY_STEP false

#define BREAKPOINT_STYLE 1 // 0 for before execution, 1 for after

#define MEMORY_SIZE 65536
#define REGISTER_SIZE 8
#define CARTRIDGE_MAX_SIZE 524288

#define GRAPHIC_WIDTH 256
#define GRAPHIC_HEIGHT 240
#define GRAPHIC_VRAM_SIZE 16384
#define GRAPHIC_REGISTER_SIZE 8

extern bool systemPaused;
extern bool systemStepCalled;


const std::string OPCODE_NAME[] = {

	/// Important TODO: wrong name for p / q values

   // x = 0
   "NOP", "EX AF, AF'", "DJNZ d", "JR d", "JR cc[y-4],d", "JR cc[y-4],d", "JR cc[y-4],d", "JR cc[y-4],d",
   "LD rp[p], nn", "ADD HL, rp[p]", "LD rp[p], nn", "ADD HL, rp[p]", "LD rp[p], nn", "ADD HL, rp[p]", "LD rp[p], nn", "ADD HL, rp[p]",
   "LD (BC), A", "LD (DE), A", "LD (nn), HL", "LD (nn), A", "LD A, (BC)", "LD A, (DE)", "LD HL, (nn)", "LD A, (nn)",
   "INC rp[p]", "DEC rp[p]", "INC rp[p]", "DEC rp[p]", "INC rp[p]", "DEC rp[p]", "INC rp[p]", "DEC rp[p]",
   "INC r[y]", "INC r[y]", "INC r[y]", "INC r[y]", "INC r[y]", "INC r[y]", "INC r[y]", "INC r[y]",
   "DEC r[y]", "DEC r[y]", "DEC r[y]", "DEC r[y]", "DEC r[y]", "DEC r[y]", "DEC r[y]", "DEC r[y]",
   "LD r[y], n", "LD r[y], n", "LD r[y], n", "LD r[y], n", "LD r[y], n", "LD r[y], n", "LD r[y], n", "LD r[y], n",
   "RLCA", "RRCA", "RLA", "RRA", "DAA", "CPL", "SCF", "CCF",

   // x = 1
   "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]",
   "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]",
   "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]",
   "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]",
   "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]",
   "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]",
   "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "HALT", "LD r[y], r[z]",
   "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]", "LD r[y], r[z]",

   // x = 2
   "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]",
   "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]",
   "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]",
   "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]",
   "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]",
   "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]",
   "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]",
   "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]", "alu[y] r[z]",

   // x = 3
   "RET cc[y]", "RET cc[y]", "RET cc[y]", "RET cc[y]", "RET cc[y]", "RET cc[y]", "RET cc[y]", "RET cc[y]",
   "POP rp2[p]", "RET", "POP rp2[p]", "EXX", "POP rp2[p]", "JP HL", "POP rp2[p]", "LD SP, HL",
   "JP cc[y], nn", "JP cc[y], nn", "JP cc[y], nn", "JP cc[y], nn", "JP cc[y], nn", "JP cc[y], nn", "JP cc[y], nn", "JP cc[y], nn",
   "JP nn", "(CB prefix)", "OUT (n), A", "IN A, (n)", "EX (SP), HL", "EX DE, HL", "DI", "EI",
   "CALL cc[y], nn", "CALL cc[y], nn", "CALL cc[y], nn", "CALL cc[y], nn", "CALL cc[y], nn", "CALL cc[y], nn", "CALL cc[y], nn", "CALL cc[y], nn",
   "PUSH rp2[p]", "CALL nn", "PUSH rp2[p]", "(DD prefix)", "PUSH rp2[p]", "(ED prefix)", "PUSH rp2[p]", "(FD prefix)",
   "alu[y] n", "alu[y] n", "alu[y] n", "alu[y] n", "alu[y] n", "alu[y] n", "alu[y] n", "alu[y] n",
   "RST y*8", "RST y*8", "RST y*8", "RST y*8", "RST y*8", "RST y*8", "RST y*8", "RST y*8",
};

const std::string OPCODECB_NAME[] = { "rot[y] r[z]", "BIT y, r[z]", "RES y, r[z]", "SET y, r[z]" };

std::string getOpcodeName(uint8_t prefix, uint8_t opcode);

// pos from 0 to 7 (with 7 MSB)
uint8_t getBit8(uint8_t value, uint8_t pos);

// value is the variable that will be changed
// pos from 0 to 7 (with 7 MSB)
// newBit can be 0 or 1 else can be true or false
void setBit8(uint8_t* value, uint8_t pos, bool newBit);


// count of bits is even or odd ? (true=even)
bool nbBitsEven(uint8_t byte);

#endif

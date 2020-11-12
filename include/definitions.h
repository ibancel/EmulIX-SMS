#pragma once

#include <string>

#include "types.h"

#define DEBUG_MODE false || _DEBUG

#define BREAKPOINT_STYLE 1 // 0 for before execution, 1 for after

#define TIME_SCALE 1.00

#define MEMORY_SIZE 65536
#define REGISTER_SIZE 8
#define CARTRIDGE_MAX_SIZE 524288

#define GRAPHIC_WIDTH 256
#define GRAPHIC_HEIGHT 192
#define GRAPHIC_VRAM_SIZE 16384
#define GRAPHIC_CRAM_SIZE 32
#define GRAPHIC_REGISTER_SIZE 11
#define GRAPHIC_PRECISE_TIMING true
#define GRAPHIC_THREADING true

#define MACHINE_MODEL 1 // 0: domestic (JP) / 1: export (US, EU, ...)
#define MACHINE_VERSION 1 // 0: SMS1 / 1: SMS2

#define NOT_IMPLEMENTED(string) SLOG_THROW(lwarning << std::dec << __FILE__ << "#" << __LINE__ << " NOT_IMPLEMENTED: " << string << std::endl);

constexpr long double BaseFrequency = 10'738'635.0 * TIME_SCALE; // in Hz = cycle/s (10'738'580.0 OR 10'738'635.0?)
constexpr int DumpMode = 0;

namespace VDP {
    enum StatusBit { S_F = 7, S_OVR = 6, S_C = 5 };
    enum ColorBank { kFirst = 0, kSecond = 16 };
}
enum class SpriteSize : u8 { k8 = 0, k16 = 1 };


const std::string OPCODE_NAME[] = {

	/// Important TODO: wrong name for p / q values (x0 done)

   // x = 0
   "NOP", "EX AF, AF'", "DJNZ d", "JR d", "JR cc[y-4],d", "JR cc[y-4],d", "JR cc[y-4],d", "JR cc[y-4],d",
   "LD rp[p], nn", "ADD HL, rp[p]", "LD rp[p], nn", "ADD HL, rp[p]", "LD rp[p], nn", "ADD HL, rp[p]", "LD rp[p], nn", "ADD HL, rp[p]",
   "LD (BC), A", "LD (A), BC", "LD (DE), A", "LD (A), DE", "LD (nn), HL", "LD HL, (nn)", "LD (nn), A", "LD A, (nn)",
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

const std::string OPCODE_CB_NAME[] = { "rot[y] r[z]", "BIT y, r[z]", "RES y, r[z]", "SET y, r[z]" };


// Only for x == 1:
const std::string OPCODE_ED_NAME[] = { 
    "IN r[y], (C)", "IN r[y], (C)", "IN r[y], (C)", "IN r[y], (C)", "IN r[y], (C)", "IN r[y], (C)", "IN (C)", "IN r[y], (C)",
    "OUT (C), r[y]", "OUT (C), r[y]", "OUT (C), r[y]", "OUT (C), r[y]", "OUT (C), r[y]", "OUT (C), r[y]", "OUT (C), 0", "OUT (C), r[y]",
    "SBC HL, rp[p]", "ADC HL, rp[p]", "-", "-", "-", "-", "-", "-",
    "LD (nn), rp[p]", "LD rp[p], (nn)", "-", "-", "-", "-", "-", "-",
    "NEG", "NEG", "NEG", "NEG", "NEG", "NEG", "NEG", "NEG",
    "RETN", "RETI", "RETN", "RETN", "RETN", "RETN", "RETN", "RETN",
    "IM im[y]", "IM im[y]", "IM im[y]", "IM im[y]", "IM im[y]", "IM im[y]", "IM im[y]", "IM im[y]",
    "LD I, A", "LD R,A", "LD A, I", "LD A, R", "RRD", "RLD", "NOP", "NOP"
};

std::string getOpcodeName(u8 prefix, u8 opcode);

// pos from 0 to 7 (with 7 MSB)
inline u8 getBit8(u8 value, u8 pos) {
    return (value >> pos) & 0b1;
}

// value is the variable that will be changed
// pos from 0 to 7 (with 7 MSB)
// newBit can be 0 or 1 else can be true or false
void setBit8(u8* value, u8 pos, bool newBit);

void setBit16(u16* value, u8 pos, bool newBit);


// extract 8 lowest bits of a 16 bits value
inline u8 getLowerByte(u16 value) {
    return (value & 0xFF);
}

inline void setLowerByte(u16 &ioToSet, u8 byte) {
    ioToSet = (ioToSet & 0xFF00) | byte;
}

// extract 8 hight bits of a 16 bits value
inline u8 getHigherByte(u16 value) {
    return (value >> 8) & 0xFF;
}

inline void setHigherByte(u16& ioToSet, u8 byte) {
    ioToSet = (ioToSet & 0x00FF) | (byte<<8);
}


// count of bits is even or odd ? (true=even)
bool nbBitsEven(u8 byte);

inline u8 sign8(u8 byte) {
    return byte >> 7;
}
inline u16 sign16(u16 word) {
    return word >> 15;
}

inline bool isPositive8(u8 byte) {
    return static_cast<bool>(sign8(byte));
}
inline bool isPositive16(u16 word) {
    return static_cast<bool>(sign16(word));
}

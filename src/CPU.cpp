#include <fstream>
#include <string>
#include <unistd.h>

#include "CPU.h"

using namespace std;


CPU::CPU()
{
	init();
}

CPU::CPU(Memory *m, Graphics *g, Cartridge *c)
{
	init();
}

void CPU::init()
{
	_memory = Memory::instance();
	_graphics = Graphics::instance();
	_cartridge = Cartridge::instance();
	_audio = new Audio(); // for the moment !

	_pc = 0x00; // reset (at $0000), IRQs (at $0038) and NMIs (at $0066)
	_sp = 0xFDD0;

	_modeInt = 0;
	_IFF1 = false;
	_IFF2 = false;

	for(int i = 0 ; i < REGISTER_SIZE ; i++)
		_register[i] = 0xFF;

	for(int i = 0 ; i < REGISTER_SIZE ; i++)
		_registerA[i] = 0xFF;

	for(int i = 0 ; i < MEMORY_SIZE ; i++)
		_stack[i] = 0;

	_registerFlag = 0xFF;
	_registerFlagA = 0xFF;

	/// vv A ENLEVER vv
	_memory->init();

	ifstream fichier("ROMS/zexall.sms", ios_base::in | ios_base::binary);
	//ifstream fichier("ROMS/Trans-Bot (UE).sms", ios_base::in | ios_base::binary);
	//ifstream fichier("ROMS/Astro Flash (J) [!].sms", ios_base::in | ios_base::binary);
	//ifstream fichier("ROMS/Color & Switch Test (Unknown).sms", ios_base::in | ios_base::binary);
	//ifstream fichier("ROMS/F-16 Fighter (USA, Europe).sms", ios_base::in | ios_base::binary);
	//ifstream fichier("ROMS/Ghost House (USA, Europe).sms", ios_base::in | ios_base::binary);
	//ifstream fichier("ROMS/Black Onyx, The (SG-1000) [!].sg", ios_base::in | ios_base::binary);
	//ifstream fichier("ROMS/Championship Golf (SG-1000).sg", ios_base::in | ios_base::binary);
	//ifstream fichier("ROMS/Elevator Action (SG-1000) [!].sg", ios_base::in | ios_base::binary);


	if(!fichier) {
		slog << lerror << "ROM loading failed." << endl;
		exit(EXIT_FAILURE);
	}

	char h;
	uint8_t val;
	uint16_t compteur = 0x0;
	while(compteur < MEMORY_SIZE)
	{
		fichier.read(&h, sizeof(char));
		val = static_cast<uint8_t>(h);
		//cout << hex << (unsigned int)val << " ";
		//cout << hex << compteur << " : " << (unsigned int)val << endl;

		if(!fichier.good())
		{
			fichier.close();
			break;
		}

		_memory->write(compteur++, val);
	}
	/// ^^ A ENLEVER ^^
}

void CPU::reset()
{
    _IFF1 = false;
    _IFF2 = false;
    _pc = 0;
    _registerI = 0;
    _registerR = 0;
    _modeInt = 0;
    _registerFlag = 0xFF;
}

void CPU::cycle()
{
	if(STEP_BY_STEP && !systemStepCalled)
		return;

	//while(true)
	{
		// TODO: for the moment
		if(_graphics->getIE()) {
			_stack[--_sp] = (_pc >> 8);
			_stack[--_sp] = (_pc & 0xFF);
			_pc = 0x38;
		}

		slog << ldebug << "HL: " << getRegisterPair(RP_HL) << endl;
		slog << ldebug << "Stack: " << (uint16_t)_stack[_sp] << ", " << (uint16_t)_stack[_sp+1] << endl;

		uint8_t prefix = _memory->read(_pc++);
		uint8_t opcode = prefix;

		if(prefix == 0xCB || prefix == 0xDD || prefix == 0xED || prefix == 0xED || prefix == 0xFD)
		{
			opcode = _memory->read(_pc++);
		}
		else
			prefix = 0;

		opcodeExecution(prefix, opcode);

		_audio->run(); // to put in main loop

		//usleep(500 * 1000);

		if(_pc > 0x8000) exit(8);
	}

	systemStepCalled = false;
}

resInstruction CPU::opcodeExecution(uint8_t prefix, uint8_t opcode)
{
	/// TODO: à prendre en compte les changements pour ALU
	/// TODO: gérer les interruptions

	slog << ldebug << hex <<  "#" << (uint16_t)(_pc-1) << " : " << (uint16_t) opcode;
	if(prefix != 0)
			slog << "(" << (uint16_t)prefix << ")";
	slog << " --" << getOpcodeName(opcode) << "--" << endl;
	//cout << "regA : " << (uint16_t)_register[R_A] << endl;
	Stats::add(prefix, opcode);

	resInstruction res;

	uint8_t x = (opcode>>6) & 0b11;
	uint8_t y = (opcode>>3) & 0b111;
	uint8_t z = (opcode>>0) & 0b111;
	uint8_t p = (y >> 1);
	uint8_t q = y & 0b1;

	if(prefix == 0)
		opcode0(x,y,z,p,q);
	else if(prefix == 0xCB)
		opcodeCB(x,y,z,p,q);
	else if(prefix == 0xDD)
		opcodeDD(x,y,z,p,q);
	else if(prefix == 0xED)
		opcodeED(x,y,z,p,q);
}

void CPU::aluOperation(uint8_t index, uint8_t value)
{
	if(index == 0) // ADD A
	{
		uint8_t sum = (uint8_t)(_register[R_A] + value);
		setFlagBit(F_S, ((int8_t)(sum) < 0));
		setFlagBit(F_Z, (_register[R_A] == value));
		setFlagBit(F_H, ((_register[R_A]&0xF) + (value&0xF) < (_register[R_A]&0xF)));
		setFlagBit(F_P, ((_register[R_A]>>7)==(value>>7) && (value>>7)!=(sum>>7)));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, (sum < _register[R_A]));
		setFlagBit(F_F3, (sum>>2)&1);
		setFlagBit(F_F5, (sum>>4)&1);

		_register[R_A] = sum;
	}
	//
	else if(index == 2) // SUB
	{
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
	//
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

void CPU::rotOperation(uint8_t index, uint8_t reg)
{
	if(index == 0) // RLC
	{
		/// TODO
	}
	//
	else if(index == 4) // SLA
	{
		uint8_t val = (uint8_t)(_register[reg] << 1);
		setFlagBit(F_S, ((int8_t)(val) < 0));
		setFlagBit(F_Z, (val == 0));
		setFlagBit(F_H, 0);
		setFlagBit(F_P, nbBitsEven(val));
		setFlagBit(F_N, 0);
		setFlagBit(F_C, (_register[reg]>>7)&1);
		_register[reg] = val;
	}
	//
	else
		slog << lwarning << "ROT " << (uint16_t)index << " is not implemented" << endl;
}

uint8_t CPU::portCommunication(bool rw, uint8_t address, uint8_t data)
{
    slog << ldebug << hex << "<-----------> rw:" << rw << " a:" << (uint16_t)address << " d:" << (uint16_t)data << endl;
	if(address == 0xBE || address == 0xBF)
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
		/// TODO understand (nationalisation ?)
	}
	else
	{
		slog << lwarning << hex << "Communication port 0x" << (uint16_t)address << " is not implemented" << endl;
	}
}


uint16_t CPU::getProgramCounter() const
{
	return _pc;
}

uint8_t CPU::getRegisterFlag() const
{
	return _registerFlag;
}


/// PRIVATE :

bool CPU::isPrefixByte(uint8_t byte)
{
	return (byte == 0xCB || byte == 0xDD || byte == 0xED || byte == 0xFD);
}

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
		slog << ldebug << "=> Condition " << y-4 << " : " << condition(y-4) << " (d=" << hex << (int16_t)_memory->read(_pc) << ")" << endl;
		if(condition(y-4))
			_pc += (int8_t)_memory->read(_pc) + 1;
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
	else if(x == 0 && z == 2 && q == 0 && p == 0) // LD (DE),A
	{
		_memory->write(getRegisterPair(RP_DE), _register[R_A]);
	}
	else if(x == 0 && z == 2 && q == 0 && p == 2) // LD (nn),HL
	{
		uint16_t addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		_memory->write(addr, _register[R_L]);
		_memory->write(addr+1, _register[R_H]);
	}
	else if(x == 0 && z == 2 && q == 0 && p == 3) // LD (nn),A
	{
		uint16_t addr = _memory->read(_pc++);
		addr += (_memory->read(_pc++)<<8);
		_memory->write(addr, _register[R_A]);
	}
	//
	else if(x == 0 && z == 2 && q == 1 && p == 1) // LD A,(DE)
	{
		_register[R_A] = _memory->read(getRegisterPair(RP_DE));
	}
	else if(x == 0 && z == 2 && q == 1 && p == 2) // LD HL,(nn)
	{
		_register[R_L] = _memory->read(_pc++);
		_register[R_H] = _memory->read(_pc++);
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
		/// TODO flags
		_register[y]++;
	}
	else if(x == 0 && z == 5) // DEC r[y]
	{
		/// TODO flags
		_register[y]--;
	}
	else if(x == 0 && z == 6) // LD r[y],n
	{
		_register[y] = _memory->read(_pc++);
	}
	else if(x == 0 && z == 7 && y == 0) // RLCA
	{
		setFlagBit(F_H, 0);
		setFlagBit(F_N, 0);
		setFlagBit(F_C, (_register[R_A] >> 7));
		_register[R_A] = _register[R_A] << 1;
		_register[0] = getFlagBit(F_C);
	}
	//
	else if(x == 0 && z == 7 && y == 5) // CPL
	{
		setFlagBit(F_H, 1);
		setFlagBit(F_N, 1);
		_register[R_A] = ~_register[R_A];
	}
	//
	else if(x == 0 && z == 7 && y == 7) // CCF
	{
        setFlagBit(F_N, 0);
        setFlagBit(F_C, !getFlagBit(F_C));
	}
	else if(x == 1 && (z != 6 || y != 6))
	{
		_register[y] = _register[z];
	}
	else if(x == 1 && z == 6 && y == 6) // HALT
	{
		/// TODO
	}
	else if(x == 2) // alu[y] r[z]
	{
		aluOperation(y, _register[z]);
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
	//
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
		/// TODO voir les sorties
		uint8_t val = _memory->read(_pc++);
		portCommunication(true, val, _register[R_A]);
		slog << ldebug << "OUT ====> " << hex << (uint16_t)val << " : " << (uint16_t)_register[R_A] << endl;
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
		_memory->write(_sp, _register[R_L]);
		_memory->write(_sp+1, _register[R_H]);
		_register[R_L] = tempL;
		_register[R_H] = tempH;
	}
	//
	else if(x == 3 && z == 3 && y == 6) // DI
	{
		_IFF1 = false;
		_IFF2 = false;
	}
	//
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
      _stack[--_sp] = (_pc >> 8);
      _stack[--_sp] = (_pc & 0xFF);
      _pc = y*8;
      slog << ldebug << "RST to " << hex << (uint8_t)(y*8) << endl;
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
		rotOperation(y,z);
	}
	//
	else
	{
		slog << lwarning << "Opcode : " << hex << (uint16_t)((x<<6)+(y<<3)+z) << " (CB) is not implemented" << endl;
	}
}

void CPU::opcodeDD(uint8_t x, uint8_t y, uint8_t z, uint8_t p, uint8_t q)
{
	if(false)
	{

	}
	else
	{
		slog << lwarning << "Opcode : " << hex << (uint16_t)((x<<6)+(y<<3)+z) << " (DD) is not implemented" << endl;
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
		portCommunication(true, _register[R_C], _register[y]);
	}
	else if(x == 1 && z == 2 && q == 0) // SBC HL, rp[p]
	{
      uint16_t value = getRegisterPair(p) + getFlagBit(F_C);
      uint16_t result = (uint16_t)(getRegisterPair(p) - value);
		setFlagBit(F_S, ((int16_t)(result) < 0));
		setFlagBit(F_Z, (getRegisterPair(p) == value));
		setFlagBit(F_H, ((getRegisterPair(p)&0xF) - (value&0xF) < (getRegisterPair(p)&0xF))); // ?
		setFlagBit(F_P, ((getRegisterPair(p)>>7)==(value>>7) && (value>>7)!=(result>>7))); // ?
		setFlagBit(F_N, 1);
		setFlagBit(F_C, (result > getRegisterPair(p))); // ?
		setFlagBit(F_F3, (result>>2)&1);
		setFlagBit(F_F5, (result>>4)&1);

		_register[R_A] = result;
	}
	//
	else if(x == 1 && z == 5 && y != 1) // RETN
	{
		// TODO: verify
		_IFF1 = _IFF2;

		_pc = _stack[_sp++];
		_pc += (_stack[_sp++]<<8);
		slog << lerror << "PC: " << _pc << endl;
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

		interrupt();
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
	if(x == 6 && y == 0) // LDIR
	{
		/// TODO interrupts et tester ce bloc !
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
	else if(x == 6 && y == 3) // OTIR
	{
		uint16_t value = _memory->read(getRegisterPair(RP_HL));
		//slog << ldebug << hex << "OTIR : R_C="<< (uint16_t)_register[R_C] << " R_B=" << (uint16_t)_register[R_B] << " RP_HL=" << (uint16_t)getRegisterPair(2) << " val=" << (uint16_t)value << endl;
		portCommunication(true, _register[R_C], value);

		setRegisterPair(RP_HL, getRegisterPair(RP_HL)+1);
		_register[R_B]--;

		if(_register[R_B] != 0)
			_pc -= 2;

		setFlagBit(F_Z, 1);
		setFlagBit(F_N, 1);
	}
	else
	{
		slog << lwarning << hex << "BLI (" << (uint16_t)x << ", " << (uint16_t)y << ") is not implemented !" << endl;
	}
}

void CPU::interrupt(bool nonMaskable)
{
    if(!_IFF1) return;

    if(_modeInt == 0) {
        slog << ldebug << "TODO interrupt mode 0" << endl;
     }
     else if(_modeInt == 1) {
        reset();
        _pc = 0x38;
    }
    else if(_modeInt == 2) {
         slog << ldebug << "TODO interrupt mode 2" << endl;
    }
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
			r[R_H] = first;
			r[R_L] = second;
			break;
		case 3:
			_sp = value;
			break;
		default:
			break;
	}

	slog << ldebug << hex << "set RP[" << (uint16_t)code << "] = " << value << endl;
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
			r[R_H] = first;
			r[R_L] = second;
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

	slog << ldebug << hex << "set RP2[" << (uint16_t)code << "] = " << value << endl;
}

void CPU::setFlagBit(F_NAME f, uint8_t value)
{
	if(value == 1)
		_registerFlag |= 1 << (uint8_t)f;
	else
		_registerFlag &= ~(1 << (uint8_t)f);
}


// get:

uint16_t CPU::getRegisterPair(uint8_t code, bool alternate)
{
	// TODO verify HL
	uint8_t* r = _register;
	if(alternate)
		r = _registerA;

	if(code < 3)
		return ((r[code*2]<<8) + r[code*2+1]);

	return _sp;
}

uint16_t CPU::getRegisterPair2(uint8_t code, bool alternate)
{
	uint8_t* r = _register;
	if(alternate)
		r = _registerA;

	if(code < 3)
		return ((r[code*2]<<8) + r[code*2+1]);

	if(alternate)
		return ((r[R_A]<<8) + _registerFlagA);

	return ((r[R_A]<<8) + _registerFlag);
}

bool CPU::getFlagBit(F_NAME f)
{
	return (_registerFlag >> (uint8_t)f) & 1;
}

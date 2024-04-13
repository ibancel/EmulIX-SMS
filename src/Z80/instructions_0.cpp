#include "CPU.h"

using namespace std;

int CPU::_instruction_INVALID(u8 opcode)
{
	SLOG_THROW(lerror << "INSTRUCTION INVALID " << hex << opcode);
	return -1;
}

int CPU::_instruction_nop(u8 opcode) { return 4; }

int CPU::_instruction_ex(u8 opcode)
{
	swapRegisterPair2(RP2_AF);
	return 4;
}

int CPU::_instruction_djnz(u8 opcode)
{
	_register[R_B]--;
	u8 d = _memory->read(_pc++);

	if(getRegister(R_B) != 0) {
		_pc += (s8)d;
		return 13;
	} else {
		return 8;
	}
}

int CPU::_instruction_jr(u8 opcode)
{
	_pc += (s8)(_memory->read(_pc)) + 1;
	return 12;
}

int CPU::_instruction_jr_cond(u8 opcode)
{
	const u8 y = OPCODE_Y(opcode);
	SLOG(ldebug << "=> Condition " << y - 4 << " : " << condition(y - 4) << " (d=" << hex
				<< (s16)((s8)_memory->read(_pc)) << ")");
	s8 addrOffset = static_cast<s8>(_memory->read(_pc++));
	if(condition(y - 4)) {
		_pc += addrOffset;
		return 12;
	} else {
		return 7;
	}
}

int CPU::_instruction_ld(u8 opcode)
{
	u16 val = _memory->read(_pc++);
	val += (_memory->read(_pc++) << 8);
	setRegisterPair(OPCODE_P(opcode), val);

	if(OPCODE_P(opcode) == 2 && (_useRegisterIX || _useRegisterIY)) {
		return 14;
	} else {
		return 10;
	}
}

int CPU::_instruction_add_hl(u8 opcode)
{
	u16 oldVal = getRegisterPair(RP_HL);
	u16 addOp = getRegisterPair(OPCODE_P(opcode));
	u16 newVal = oldVal + addOp;

	setFlagBit(F_H, ((oldVal & 0xFFF) + (addOp & 0xFFF) > 0xFFF));
	setFlagBit(F_N, 0);
	setFlagBit(F_C, (newVal < oldVal));

	setFlagUndoc(getHigherByte(newVal));

	setRegisterPair(RP_HL, newVal);

	if(_useRegisterIX || _useRegisterIY) {
		return 15;
	} else {
		return 11;
	}
}

int CPU::_instruction_ld_bc_acc(u8 opcode)
{
	_memory->write(getRegisterPair(RP_BC), _register[R_A]);
	return 7;
}

int CPU::_instruction_ld_acc_bc(u8 opcode)
{
	setRegister(R_A, _memory->read(getRegisterPair(RP_BC)));
	return 7;
}

int CPU::_instruction_ld_de_acc(u8 opcode)
{
	_memory->write(getRegisterPair(RP_DE), _register[R_A]);
	return 7;
}

int CPU::_instruction_ld_acc_de(u8 opcode)
{
	_register[R_A] = _memory->read(getRegisterPair(RP_DE));
	return 7;
}

int CPU::_instruction_ld_nn_hl(u8 opcode)
{
	u16 addr = _memory->read(_pc++);
	addr += (_memory->read(_pc++) << 8);
	_memory->write(addr, getRegister(R_L));
	_memory->write(addr + 1, getRegister(R_H));
	return 16;
}

int CPU::_instruction_ld_hl_nn(u8 opcode)
{
	u16 address = _memory->read(_pc++);
	address += (_memory->read(_pc++) << 8);
	setRegister(R_L, _memory->read(address));
	setRegister(R_H, _memory->read(address + 1));
	return 16;
}

int CPU::_instruction_ld_nn_acc(u8 opcode)
{
	u16 addr = _memory->read(_pc++);
	addr += (_memory->read(_pc++) << 8);
	_memory->write(addr, _register[R_A]);
	return 13;
}

int CPU::_instruction_ld_acc_nn(u8 opcode)
{
	u16 val = _memory->read(_pc++);
	val += (_memory->read(_pc++) << 8);
	_register[R_A] = _memory->read(val);

	return 13;

	SLOG(ldebug << "addr: " << hex << val << " val: " << (u16)_register[R_A]);
}

int CPU::_instruction_inc_rp(u8 opcode)
{
	u8 p = OPCODE_P(opcode);
	setRegisterPair(p, getRegisterPair(p) + 1);
	return 6;
}

int CPU::_instruction_dec_rp(u8 opcode)
{
	u8 p = OPCODE_P(opcode);
	setRegisterPair(p, getRegisterPair(p) - 1);
	return 6;
}

int CPU::_instruction_inc(u8 opcode)
{
	u8 y = OPCODE_Y(opcode);
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

	return 4;
}

int CPU::_instruction_dec(u8 opcode)
{
	u8 y = OPCODE_Y(opcode);
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
	if(_useRegisterIX || _useRegisterIY) {
		return 23;
	} else if(y == 6) {
		return 11;
	} else {
		return 4;
	}
}

int CPU::_instruction_ld_r_n(u8 opcode)
{
	u8 y = OPCODE_Y(opcode);
	if(y == 6 && (_useRegisterIX || _useRegisterIY)) {
		retrieveIndexDisplacement();
	}

	setRegister(y, _memory->read(_pc++));
	if(_useRegisterIX || _useRegisterIY) {
		return 19;
	} else if(y == 6) {
		return 10;
	} else {
		return 7;
	}
}

int CPU::_instruction_rlca(u8 opcode)
{
	u8 valueRegister = getRegister(R_A, false, false);
	setFlagBit(F_H, 0);
	setFlagBit(F_N, 0);
	setFlagBit(F_C, (valueRegister >> 7));
	valueRegister <<= 1;
	setBit8(&valueRegister, 0, getFlagBit(F_C));

	setFlagUndoc(valueRegister);
	setRegister(R_A, valueRegister, false, false);
	return 4;
}

int CPU::_instruction_rrca(u8 opcode)
{
	setFlagBit(F_H, 0);
	setFlagBit(F_N, 0);
	u8 bit0 = getBit8(_register[R_A], 0);
	_register[R_A] >>= 1;
	setBit8(&_register[R_A], 7, bit0);
	setFlagUndoc(_register[R_A]);
	setFlagBit(F_C, bit0);
	return 4;
}

int CPU::_instruction_rla(u8 opcode)
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
	return 4;
}

int CPU::_instruction_rra(u8 opcode)
{
	setFlagBit(F_H, 0);
	setFlagBit(F_N, 0);
	uint_fast8_t bit0 = getBit8(_register[R_A], 0);
	_register[R_A] >>= 1;
	setBit8(&_register[R_A], 7, getFlagBit(F_C));
	setFlagUndoc(_register[R_A]);
	setFlagBit(F_C, bit0);
	return 4;
}

int CPU::_instruction_daa(u8 opcode)
{
	// Block simplified thanks to Eric Haskins
	// https://ehaskins.com/2018-01-30 Z80 DAA/
	u8 cBit = getFlagBit(F_C);
	u8 hBit = getFlagBit(F_H);
	u8 nBit = getFlagBit(F_N);
	u8 value = getRegister(R_A);
	u8 correction = 0;
	u8 newCBit = 0;
	if(hBit || (!nBit && (value & 0xF) > 0x9)) {
		correction = 0x06;
	}
	if(cBit || (!nBit && value > 0x99)) {
		correction |= 0x60;
		newCBit = 1;
	}

	value += (nBit ? -correction : correction);
	setRegister(R_A, value);

	setFlagBit(F_Z, value == 0);
	setFlagBit(
		F_H, (!nBit && ((value & 0x0F) > 9)) || (nBit && hBit && ((value & 0x0F) < 6))); // to verify (or 0 as before?)
	setFlagBit(F_C, newCBit);
	setFlagBit(F_S, getBit8(value, 7));
	setFlagBit(F_P, nbBitsEven(value));
	setFlagUndoc(value);

	return 4;
}

int CPU::_instruction_cpl(u8 opcode)
{
	_register[R_A] = ~_register[R_A];

	setFlagBit(F_H, 1);
	setFlagBit(F_N, 1);
	setFlagUndoc(_register[R_A]);

	return 4;
}

int CPU::_instruction_scf(u8 opcode)
{
	setFlagBit(F_C, 1);
	setFlagBit(F_N, 0);
	setFlagBit(F_H, 0);

	return 4;
}

int CPU::_instruction_ccf(u8 opcode)
{
	u8 previousCarry = getFlagBit(F_C);
	setFlagBit(F_N, 0);
	setFlagBit(F_C, !previousCarry);
	setFlagBit(F_H, previousCarry);

	return 4;
}

int CPU::_instruction_ld_r_r(u8 opcode)
{
	u8 y = OPCODE_Y(opcode);
	u8 z = OPCODE_Z(opcode);
	setRegister(y, getRegister(z, false, (y != 6)), false, (z != 6));
	if(_useRegisterIX || _useRegisterIY) {
		return 19;
	} else if(y == 6 || z == 6) {
		return 7;
	} else {
		return 4;
	}
}

int CPU::_instruction_halt(u8 opcode)
{
	_halt = true;
	return 4;
}

int CPU::_instruction_alu_r(u8 opcode)
{
	u8 y = OPCODE_Y(opcode);
	u8 z = OPCODE_Z(opcode);
	aluOperation(y, getRegister(z));
	if(z == 6) {
		if(_useRegisterIX || _useRegisterIY) {
			return 19;
		} else {
			return 7;
		}
	} else {
		return 4;
	}
}

int CPU::_instruction_retcc(u8 opcode)
{
	u8 y = OPCODE_Y(opcode);
	bool c = condition(y);
	if(c) {
		_pc = _memory->read(_sp++);
		_pc += (_memory->read(_sp++) << 8);
		return 11;
	} else {
		return 5;
	}
	SLOG(ldebug << hex << "RET(" << (c ? "true" : "false") << ") to " << (u16)_pc);
}

int CPU::_instruction_pop(u8 opcode)
{
	u8 p = OPCODE_P(opcode);
	u16 value = _memory->read(_sp++);
	value += (_memory->read(_sp++) << 8);
	setRegisterPair2(p, value);
	if(p == 2 && (_useRegisterIX || _useRegisterIY)) {
		return 14;
	} else {
		return 10;
	}
}

int CPU::_instruction_ret(u8 opcode)
{
	u16 addr = _memory->read(_sp++);
	addr += (_memory->read(_sp++) << 8);
	_pc = addr;
	return 10;
	SLOG(ldebug << hex << "RET to " << (u16)addr);
}

int CPU::_instruction_exx(u8 opcode)
{
	swapRegisterPair(RP_BC);
	swapRegisterPair(RP_DE);
	swapRegisterPair(RP_HL);
	return 4;
}

int CPU::_instruction_jp_hl(u8 opcode)
{
	_pc = getRegisterPair(RP_HL);
	return 4;
	SLOG(ldebug << hex << "JP to " << (u16)_pc);
}

int CPU::_instruction_ld_sp_hl(u8 opcode)
{
	_sp = getRegisterPair(RP_HL);
	return 6;
	SLOG(ldebug << hex << "SP: " << _sp);
}

int CPU::_instruction_jpcc_nn(u8 opcode)
{
	u16 addr = _memory->read(_pc++);
	addr += (_memory->read(_pc++) << 8);
	if(condition(OPCODE_Y(opcode))) {
		_pc = addr;
	}
	return 10;
	SLOG(ldebug << "JP C,nn : " << (condition(OPCODE_Y(opcode)) ? "true" : "false") << " new addr: " << hex << _pc);
}

int CPU::_instruction_jp_nn(u8 opcode)
{
	u16 val = _memory->read(_pc++);
	val += (_memory->read(_pc++) << 8);
	_pc = val;
	return 10;
	SLOG(ldebug << hex << "val: " << val);
}

int CPU::_instruction_out(u8 opcode)
{
	addNbStates(5); // TODO: determine precisely the number of states before
	u8 val = _memory->read(_pc++);
	portCommunication(true, val, getRegister(R_A));
	return 6;
	// slog << ldebug << "OUT ====> " << hex << (u16)val << " : " <<
	// (u16)_register[R_A] << endl;
}

int CPU::_instruction_in(u8 opcode)
{
	addNbStates(5); // TODO: determine precisely the number of states before
	u8 addr = _memory->read(_pc++);
	_register[R_A] = portCommunication(false, addr);
	// setFlagBit(F_H, 0); // Not sure about this
	return 6;
}

int CPU::_instruction_ex_sp_hl(u8 opcode)
{
	u8 tempL = _memory->read(_sp);
	u8 tempH = _memory->read(_sp + 1);
	_memory->write(_sp, getRegister(R_L));
	_memory->write(_sp + 1, getRegister(R_H));
	setRegister(R_L, tempL);
	setRegister(R_H, tempH);
	if(_useRegisterIX || _useRegisterIY) {
		return 23;
	} else {
		return 19;
	}
}

int CPU::_instruction_ex_de_hl(u8 opcode)
{
	stopRegisterIX();
	stopRegisterIY();
	u16 temp = getRegisterPair(RP_HL);
	setRegisterPair(RP_HL, getRegisterPair(RP_DE));
	setRegisterPair(RP_DE, temp);
	return 4;
}

int CPU::_instruction_di(u8 opcode)
{
	_IFF1 = false;
	_IFF2 = false;
	return 4;
}

int CPU::_instruction_ei(u8 opcode)
{
	/// TODO verify if it's working well
	_IFF1 = true;
	_IFF2 = true;
	_enableInterruptWaiting = 1;
	return 4;
}

int CPU::_instruction_call_cc(u8 opcode)
{
	u16 addr = _memory->read(_pc++);
	addr += (_memory->read(_pc++) << 8);

	bool c = condition(OPCODE_Y(opcode));
	if(c) {
		_memory->write(--_sp, (_pc >> 8));
		_memory->write(--_sp, (_pc & 0xFF));
		_pc = addr;
		return 17;
	} else {
		return 10;
	}

	SLOG(ldebug << "CALL(" << (c ? "true" : "false") << ") to " << hex << _pc);
}

int CPU::_instruction_push(u8 opcode)
{
	u8 p = OPCODE_P(opcode);
	u16 val = getRegisterPair2(p);
	_memory->write(--_sp, (val >> 8));
	_memory->write(--_sp, (val & 0xFF));
	if(p == 2 && (_useRegisterIX || _useRegisterIY)) {
		return 15;
	} else {
		return 11;
	}
}

int CPU::_instruction_call(u8 opcode)
{
	u16 newPC = _memory->read(_pc++);
	newPC += (_memory->read(_pc++) << 8);

	_memory->write(--_sp, (_pc >> 8));
	_memory->write(--_sp, (_pc & 0xFF));

	_pc = newPC;

	return 17;

	SLOG(ldebug << "CALL to " << hex << newPC);
}

int CPU::_instruction_alu_n(u8 opcode)
{
	aluOperation(OPCODE_Y(opcode), _memory->read(_pc++));
	return 7;
}

int CPU::_instruction_rst(u8 opcode)
{
	restart(OPCODE_Y(opcode) * 8);
	return 11;
}

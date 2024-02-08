int _instruction_NOT_IMPLEMENTED(u8 opcode);
int _instruction_noni_nop(u8 opcode);
int _instruction_out16_0(u8 opcode);
int _instruction_out16_r(u8 opcode);
int _instruction_sbc_rp(u8 opcode);
int _instruction_adc_rp(u8 opcode);
int _instruction_ld_nn_rp(u8 opcode);
int _instruction_ld_rp_nn(u8 opcode);
int _instruction_neg(u8 opcode);
int _instruction_retn(u8 opcode);
int _instruction_reti(u8 opcode);
int _instruction_im(u8 opcode);
int _instruction_ld_r_a(u8 opcode);
int _instruction_ld_a_r(u8 opcode);
int _instruction_rrd(u8 opcode);
int _instruction_rld(u8 opcode);

int _instruction_bli_ldi(u8 opcode);
int _instruction_bli_ldd(u8 opcode);
int _instruction_bli_ldir(u8 opcode);
int _instruction_bli_lddr(u8 opcode);
int _instruction_bli_cpi(u8 opcode);
int _instruction_bli_cpd(u8 opcode);
int _instruction_bli_cpir(u8 opcode);
int _instruction_bli_cpdr(u8 opcode);
//
int _instruction_bli_outi(u8 opcode);
int _instruction_bli_outd(u8 opcode);
int _instruction_bli_otir(u8 opcode);
//
int _instruction_bli_NOT_IMPL(u8 opcode);

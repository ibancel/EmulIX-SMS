std::function<int(CPU*, u8)> CPU::Instructions_op_cb[] = {
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_rot_r),
    // X == 1
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_bit_r),
    // X == 2
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_res_r),
    // X == 3
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
    std::function<int(CPU*, u8)>(&CPU::_instruction_set_r),
};

//
// Created by Bas du Pré on 21-01-18.
//

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "instructions.h"
#include "../error_handling.h"

void NOT_IMPLEMENTED(CPU_State* s) {
    int opcode = 0; // TODO
    vm_error(s, "Fatal: opcode %#02X is not implemented", opcode);
    vm_exit(s, EXIT_FAILURE);
}

Instruction_Implementation instruction_implementations[256] = {
        /* 0x00 */    &instr_nop,
        /* 0x01 */    &instr_halt,
        /* 0x02 */    &instr_trap,
        /* 0x03 */    &instr_int,
        /* 0x04 */    &instr_link,
        /* 0x05 */    &instr_debug_break,
        /* 0x06 */    &instr_debug_setcontext,
        /* 0x07 */    &instr_debug_enterscope,
        /* 0x08 */    &instr_debug_leavescope,
        /* 0x09 */    &instr_unlink,
        /* 0x0A */    &NOT_IMPLEMENTED,
        /* 0x0B */    &NOT_IMPLEMENTED,
        /* 0x0C */    &instr_syscall_getindex,
        /* 0x0D */    &instr_syscall_byname,
        /* 0x0E */    &instr_syscall,
        /* 0x0F */    &instr_syscall_pop,
        /* 0x10 */    &instr_ld_int,
        /* 0x11 */    &instr_ld_uint,
        /* 0x12 */    &instr_ld_float,
        /* 0x13 */    &instr_ld_str,
        /* 0x14 */    &instr_ld_map,
        /* 0x15 */    &instr_ld_local,
        /* 0x16 */    &instr_ld_reg,
        /* 0x17 */    &instr_ld_stack,
        /* 0x18 */    &instr_ld_sref,
        /* 0x19 */    &instr_st_stack,
        /* 0x1A */    &instr_ld_lref,
        /* 0x1B */    &instr_ld_ref,
        /* 0x1C */    &instr_pop,
        /* 0x1D */    &instr_st_reg,
        /* 0x1E */    &instr_st_local,
        /* 0x1F */    &instr_st_ref,
        /* 0x20 */    &instr_conv_int,
        /* 0x21 */    &instr_conv_uint,
        /* 0x22 */    &instr_conv_float,
        /* 0x23 */    &instr_conv_str,
        /* 0x24 */    &instr_cast_int,
        /* 0x25 */    &instr_cast_uint,
        /* 0x26 */    &instr_cast_float,
        /* 0x27 */    &instr_cast_str,
        /* 0x28 */    &instr_cast_ref,
        /* 0x29 */    &instr_ajs,
        /* 0x2A */    &instr_locals_res,
        /* 0x2B */    &instr_locals_cleanup,
        /* 0x2C */    &instr_dup,
        /* 0x2D */    &instr_deref,
        /* 0x2E */    &instr_var,
        /* 0x2F */    &instr_ld_deref,
        /* 0x30 */    &instr_add,
        /* 0x31 */    &instr_sub,
        /* 0x32 */    &instr_mul,
        /* 0x33 */    &instr_div,
        /* 0x34 */    &instr_mod,
        /* 0x35 */    &instr_neg,
        /* 0x36 */    &instr_and,
        /* 0x37 */    &instr_or,
        /* 0x38 */    &instr_xor,
        /* 0x39 */    &instr_not,
        /* 0x3A */    &instr_cmp,
        /* 0x3B */    &instr_eq,
        /* 0x3C */    &instr_ne,
        /* 0x3D */    &instr_lt,
        /* 0x3E */    &instr_gt,
        /* 0x3F */    &instr_le,
        /* 0x40 */    &instr_ge,
        /* 0x41 */    &instr_pow,
        /* 0x42 */    &instr_lsh,
        /* 0x43 */    &instr_rsh,
        /* 0x44 */    &NOT_IMPLEMENTED,
        /* 0x45 */    &NOT_IMPLEMENTED,
        /* 0x46 */    &NOT_IMPLEMENTED,
        /* 0x47 */    &NOT_IMPLEMENTED,
        /* 0x48 */    &NOT_IMPLEMENTED,
        /* 0x49 */    &NOT_IMPLEMENTED,
        /* 0x4A */    &NOT_IMPLEMENTED,
        /* 0x4B */    &NOT_IMPLEMENTED,
        /* 0x4C */    &NOT_IMPLEMENTED,
        /* 0x4D */    &NOT_IMPLEMENTED,
        /* 0x4E */    &NOT_IMPLEMENTED,
        /* 0x4F */    &NOT_IMPLEMENTED,
        /* 0x50 */    &instr_beq,
        /* 0x51 */    &instr_bne,
        /* 0x52 */    &instr_blt,
        /* 0x53 */    &instr_bgt,
        /* 0x54 */    &instr_ble,
        /* 0x55 */    &instr_bge,
        /* 0x56 */    &instr_jmp,
        /* 0x57 */    &instr_brfalse,
        /* 0x58 */    &instr_brtrue,
        /* 0x59 */    &instr_call,
        /* 0x5A */    &instr_call_pop,
        /* 0x5B */    &instr_jmp_pop,
        /* 0x5C */    &instr_ret,
        /* 0x5D */    &instr_args_accept,
        /* 0x5E */    &instr_args_cleanup,
        /* 0x5F */    &instr_ld_arg,
        /* 0x60 */    &instr_strcat,
        /* 0x61 */    &instr_substr,
        /* 0x62 */    &instr_strlen,
        /* 0x63 */    &NOT_IMPLEMENTED,
        /* 0x64 */    &NOT_IMPLEMENTED,
        /* 0x65 */    &NOT_IMPLEMENTED,
        /* 0x66 */    &NOT_IMPLEMENTED,
        /* 0x67 */    &instr_arr_copy,
        /* 0x68 */    &instr_ld_arr,
        /* 0x69 */    &instr_ld_arrelem,
        /* 0x6A */    &instr_st_arrelem,
        /* 0x6B */    &instr_del_arrelem,
        /* 0x6C */    &instr_arr_len,
        /* 0x6D */    &instr_arr_insert,
        /* 0x6E */    &instr_arr_slice,
        /* 0x6F */    &instr_arr_concat,
        /* 0x70 */    &instr_cmp_id,
        /* 0x71 */    &instr_eq_id,
        /* 0x72 */    &instr_ne_id,
        /* 0x73 */    &instr_lt_id,
        /* 0x74 */    &instr_gt_id,
        /* 0x75 */    &instr_le_id,
        /* 0x76 */    &instr_ge_id,
        /* 0x77 */    &instr_st_addr,
        /* 0x78 */    &instr_swp,
        /* 0x79 */    &instr_ld_addr,
        /* 0x7A */    &instr_st_arg,
        /* 0x7B */    &NOT_IMPLEMENTED,
        /* 0x7C */    &NOT_IMPLEMENTED,
        /* 0x7D */    &NOT_IMPLEMENTED,
        /* 0x7E */    &NOT_IMPLEMENTED,
        /* 0x7F */    &NOT_IMPLEMENTED,
        /* 0x80 */    &instr_conv_arr,
        /* 0x81 */    &instr_arr_range,
        /* 0x82 */    &NOT_IMPLEMENTED,
        /* 0x83 */    &NOT_IMPLEMENTED,
        /* 0x84 */    &NOT_IMPLEMENTED,
        /* 0x85 */    &NOT_IMPLEMENTED,
        /* 0x86 */    &NOT_IMPLEMENTED,
        /* 0x87 */    &NOT_IMPLEMENTED,
        /* 0x88 */    &NOT_IMPLEMENTED,
        /* 0x89 */    &NOT_IMPLEMENTED,
        /* 0x8A */    &NOT_IMPLEMENTED,
        /* 0x8B */    &NOT_IMPLEMENTED,
        /* 0x8C */    &NOT_IMPLEMENTED,
        /* 0x8D */    &NOT_IMPLEMENTED,
        /* 0x8E */    &NOT_IMPLEMENTED,
        /* 0x8F */    &NOT_IMPLEMENTED,
        /* 0x90 */    &instr_ld_extern,
        /* 0x91 */    &instr_ld_empty,
        /* 0x92 */    &instr_st_stack_pop,
        /* 0x93 */    &instr_st_arg_pop,
        /* 0x94 */    &NOT_IMPLEMENTED,
        /* 0x95 */    &NOT_IMPLEMENTED,
        /* 0x96 */    &NOT_IMPLEMENTED,
        /* 0x97 */    &NOT_IMPLEMENTED,
        /* 0x98 */    &NOT_IMPLEMENTED,
        /* 0x99 */    &NOT_IMPLEMENTED,
        /* 0x9A */    &NOT_IMPLEMENTED,
        /* 0x9B */    &NOT_IMPLEMENTED,
        /* 0x9C */    &NOT_IMPLEMENTED,
        /* 0x9D */    &NOT_IMPLEMENTED,
        /* 0x9E */    &NOT_IMPLEMENTED,
        /* 0x9F */    &NOT_IMPLEMENTED,
        /* 0xA0 */    &instr_is_int,
        /* 0xA1 */    &instr_is_uint,
        /* 0xA2 */    &instr_is_float,
        /* 0xA3 */    &instr_is_str,
        /* 0xA4 */    &instr_is_arr,
        /* 0xA5 */    &instr_is_map,
        /* 0xA6 */    &instr_is_ref,
        /* 0xA7 */    &instr_is_empty,
        /* 0xA8 */    &NOT_IMPLEMENTED,
        /* 0xA9 */    &NOT_IMPLEMENTED,
        /* 0xAA */    &NOT_IMPLEMENTED,
        /* 0xAB */    &NOT_IMPLEMENTED,
        /* 0xAC */    &NOT_IMPLEMENTED,
        /* 0xAD */    &NOT_IMPLEMENTED,
        /* 0xAE */    &NOT_IMPLEMENTED,
        /* 0xAF */    &NOT_IMPLEMENTED,
        /* 0xB0 */    &instr_ld_mapitem,
        /* 0xB1 */    &instr_ld_mapitem_pop,
        /* 0xB2 */    &instr_st_mapitem,
        /* 0xB3 */    &instr_st_mapitem_pop,
        /* 0xB4 */    &instr_del_mapitem,
        /* 0xB5 */    &instr_del_mapitem_pop,
        /* 0xB6 */    &instr_has_mapitem,
        /* 0xB7 */    &instr_has_mapitem_pop,
        /* 0xB8 */    &instr_map_len,
        /* 0xB9 */    &instr_map_merge,
        /* 0xBA */    &instr_map_copy,
        /* 0xBB */    &instr_map_getprototype,
        /* 0xBC */    &instr_map_setprototype,
        /* 0xBD */    &instr_box,
        /* 0xBE */    &instr_unbox,
        /* 0xBF */    &instr_ld_boxingproto,
        /* 0xC0 */    &instr_map_renamekey,
        /* 0xC1 */    &instr_map_renamekey_pop,
        /* 0xC2 */    &instr_map_getkeys,
        /* 0xC3 */    &NOT_IMPLEMENTED,
        /* 0xC4 */    &NOT_IMPLEMENTED,
        /* 0xC5 */    &NOT_IMPLEMENTED,
        /* 0xC6 */    &NOT_IMPLEMENTED,
        /* 0xC7 */    &NOT_IMPLEMENTED,
        /* 0xC8 */    &NOT_IMPLEMENTED,
        /* 0xC9 */    &NOT_IMPLEMENTED,
        /* 0xCA */    &NOT_IMPLEMENTED,
        /* 0xCB */    &NOT_IMPLEMENTED,
        /* 0xCC */    &NOT_IMPLEMENTED,
        /* 0xCD */    &NOT_IMPLEMENTED,
        /* 0xCE */    &NOT_IMPLEMENTED,
        /* 0xCF */    &NOT_IMPLEMENTED,
        /* 0xD0 */    &NOT_IMPLEMENTED,
        /* 0xD1 */    &NOT_IMPLEMENTED,
        /* 0xD2 */    &NOT_IMPLEMENTED,
        /* 0xD3 */    &NOT_IMPLEMENTED,
        /* 0xD4 */    &NOT_IMPLEMENTED,
        /* 0xD5 */    &NOT_IMPLEMENTED,
        /* 0xD6 */    &NOT_IMPLEMENTED,
        /* 0xD7 */    &NOT_IMPLEMENTED,
        /* 0xD8 */    &NOT_IMPLEMENTED,
        /* 0xD9 */    &NOT_IMPLEMENTED,
        /* 0xDA */    &NOT_IMPLEMENTED,
        /* 0xDB */    &NOT_IMPLEMENTED,
        /* 0xDC */    &NOT_IMPLEMENTED,
        /* 0xDD */    &NOT_IMPLEMENTED,
        /* 0xDE */    &NOT_IMPLEMENTED,
        /* 0xDF */    &NOT_IMPLEMENTED,
        /* 0xE0 */    &NOT_IMPLEMENTED,
        /* 0xE1 */    &NOT_IMPLEMENTED,
        /* 0xE2 */    &NOT_IMPLEMENTED,
        /* 0xE3 */    &NOT_IMPLEMENTED,
        /* 0xE4 */    &NOT_IMPLEMENTED,
        /* 0xE5 */    &NOT_IMPLEMENTED,
        /* 0xE6 */    &NOT_IMPLEMENTED,
        /* 0xE7 */    &NOT_IMPLEMENTED,
        /* 0xE8 */    &NOT_IMPLEMENTED,
        /* 0xE9 */    &NOT_IMPLEMENTED,
        /* 0xEA */    &NOT_IMPLEMENTED,
        /* 0xEB */    &NOT_IMPLEMENTED,
        /* 0xEC */    &NOT_IMPLEMENTED,
        /* 0xED */    &NOT_IMPLEMENTED,
        /* 0xEE */    &NOT_IMPLEMENTED,
        /* 0xEF */    &NOT_IMPLEMENTED,
        /* 0xF0 */    &NOT_IMPLEMENTED,
        /* 0xF1 */    &NOT_IMPLEMENTED,
        /* 0xF2 */    &NOT_IMPLEMENTED,
        /* 0xF3 */    &NOT_IMPLEMENTED,
        /* 0xF4 */    &NOT_IMPLEMENTED,
        /* 0xF5 */    &NOT_IMPLEMENTED,
        /* 0xF6 */    &NOT_IMPLEMENTED,
        /* 0xF7 */    &NOT_IMPLEMENTED,
        /* 0xF8 */    &NOT_IMPLEMENTED,
        /* 0xF9 */    &NOT_IMPLEMENTED,
        /* 0xFA */    &NOT_IMPLEMENTED,
        /* 0xFB */    &NOT_IMPLEMENTED,
        /* 0xFC */    &NOT_IMPLEMENTED,
        /* 0xFD */    &NOT_IMPLEMENTED,
        /* 0xFE */    &NOT_IMPLEMENTED,
        /* 0xFF */    &NOT_IMPLEMENTED
};


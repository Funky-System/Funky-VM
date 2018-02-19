#ifndef PROCESSOR_INSTRUCTIONS_H
#define PROCESSOR_INSTRUCTIONS_H

#include <stdio.h>

#include "../../../include/funkyvm/cpu.h"
#include "../error_handling.h"

typedef void (*Instruction_Implementation)(CPU_State *state);    /* A pointer to a handler function */

extern Instruction_Implementation instruction_implementations[256];

#define INSTR_NOT_IMPLEMENTED(name) void instr_##name (CPU_State* s) { vm_error(s, "Fatal: opcode '%s' is not implemented", #name); vm_exit(s, EXIT_FAILURE); }
#define INSTR(name) void instr_##name (CPU_State* state)
#define GET_OPERAND() (state->pc += sizeof(vm_type_t), *(vm_type_t*)(state->memory->main_memory + state->pc - sizeof(vm_type_t)))
#define GET_OPERAND_SIGNED() (state->pc += sizeof(vm_type_signed_t), *(vm_type_signed_t*)(state->memory->main_memory + state->pc - sizeof(vm_type_signed_t)))
#define GET_OPERAND_FLOAT() (state->pc += sizeof(vm_type_float_t), *(vm_type_float_t*)(state->memory->main_memory + state->pc - sizeof(vm_type_float_t)))

#define USE_STACK() vm_value_t *stack = ((vm_value_t *)(state->memory->main_memory + state->sp))
#define AJS_STACK(n) { state->sp += sizeof(vm_value_t) * (n); }
#define USE_MARK() vm_value_t *mark = ((vm_value_t *)(state->memory->main_memory + state->mp))
#define USE_ARGS() vm_value_t *args = ((vm_value_t *)(state->memory->main_memory + state->ap))

int is_ptr_in_static_memory(CPU_State *state, vm_value_t *val);
int conv_str_rel(CPU_State *state, vm_type_signed_t rel);
void str_eq(CPU_State *state);
void str_ne(CPU_State *state);
void ld_arrelem_str(CPU_State *state);
void st_arrelem_str(CPU_State *state);
void arr_slice_str(CPU_State *state);

void arr_release(CPU_State* state, vm_pointer_t ptr);
void arr_insert_at(CPU_State *state, vm_value_t *arrayval, vm_value_t *value, vm_type_signed_t index);
vm_type_t arr_len(CPU_State *state, vm_value_t *arrayval);
void instr_conv_arr_rel(CPU_State* state, vm_type_signed_t rel);
void arr_eq(CPU_State *state);
void arr_ne(CPU_State *state);

void map_release(CPU_State* state, vm_pointer_t ptr);
vm_type_t map_contains_key(CPU_State *state, vm_pointer_t map_ptr, const char* name);
vm_map_elem_t* ld_mapitem(CPU_State *state, vm_pointer_t map_ptr, const char* name);
void ld_arrelem_map(CPU_State *state);
void st_arrelem_map(CPU_State *state);

INSTR(nop);
INSTR(halt);
INSTR(trap);
INSTR(int);
INSTR(link);
INSTR(unlink);
INSTR(debug_break);
INSTR(debug_setcontext);
INSTR(debug_enterscope);
INSTR(debug_leavescope);
INSTR(syscall);
INSTR(syscall_getindex);
INSTR(syscall_getindex_pop);
INSTR(syscall_byname);
INSTR(syscall_pop);

INSTR(ld_int);
INSTR(ld_uint);
INSTR(ld_float);
INSTR(ld_str);
INSTR(ld_map);
INSTR(ld_local);
INSTR(ld_reg);
INSTR(ld_stack);
INSTR(ld_sref);
INSTR(ld_lref);
INSTR(ld_ref);
INSTR(ld_addr);
INSTR(ld_empty);
INSTR(pop);
INSTR(st_reg);
INSTR(st_stack);
INSTR(st_stack_pop);
INSTR(st_arg_pop);
INSTR(st_local);
INSTR(st_ref);
INSTR(st_addr);

INSTR(conv_int);
INSTR(conv_uint);
INSTR(conv_float);
INSTR(conv_str);
INSTR(cast_int);
INSTR(cast_uint);
INSTR(cast_float);
INSTR(cast_str);
INSTR(cast_ref);

INSTR(ajs);
INSTR(locals_res);
INSTR(locals_cleanup);
INSTR(dup);
INSTR(deref);
INSTR(var);
INSTR(ld_deref);

INSTR(add);
INSTR(sub);
INSTR(mul);
INSTR(div);
INSTR(mod);
INSTR(neg);
INSTR(and);
INSTR(or);
INSTR(xor);
INSTR(not);
INSTR(not_bitwise);
INSTR(cmp);
INSTR(eq);
INSTR(ne);
INSTR(lt);
INSTR(gt);
INSTR(le);
INSTR(ge);
INSTR(cmp_id);
INSTR(eq_id);
INSTR(ne_id);
INSTR(lt_id);
INSTR(gt_id);
INSTR(le_id);
INSTR(ge_id);
INSTR(pow);
INSTR(lsh);
INSTR(rsh);
INSTR(swp);
INSTR(beq);
INSTR(bne);
INSTR(blt);
INSTR(bgt);
INSTR(ble);
INSTR(bge);
INSTR(jmp);
INSTR(brfalse);
INSTR(brtrue);
INSTR(call);
INSTR(call_pop);
INSTR(jmp_pop);
INSTR(ret);
INSTR(args_accept);
INSTR(args_cleanup);
INSTR(ld_arg);
INSTR(st_arg);

INSTR(strcat);
INSTR(substr);
INSTR(strlen);

INSTR(ld_arr);
INSTR(ld_arrelem);
INSTR(st_arrelem);
INSTR(del_arrelem);
INSTR(arr_len);
INSTR(arr_insert);
INSTR(arr_slice);
INSTR(arr_concat);
INSTR(arr_copy);
INSTR(conv_arr);
INSTR(arr_range);

INSTR(ld_extern);

INSTR(is_int);
INSTR(is_uint);
INSTR(is_float);
INSTR(is_str);
INSTR(is_arr);
INSTR(is_map);
INSTR(is_ref);
INSTR(is_empty);

INSTR(ld_mapitem);
INSTR(ld_mapitem_pop);
INSTR(st_mapitem);
INSTR(st_mapitem_pop);
INSTR(del_mapitem);
INSTR(del_mapitem_pop);
INSTR(has_mapitem);
INSTR(has_mapitem_pop);
INSTR(map_len);
INSTR(map_merge);
INSTR(map_copy);
INSTR(map_getprototype);
INSTR(map_setprototype);
INSTR(box);
INSTR(unbox);
INSTR(ld_boxingproto);
INSTR(map_renamekey);
INSTR(map_renamekey_pop);
INSTR(map_getkeys);

#endif //PROCESSOR_INSTRUCTIONS_H

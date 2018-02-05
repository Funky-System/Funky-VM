#ifndef PROCESSOR_INSTRUCTIONS_H
#define PROCESSOR_INSTRUCTIONS_H

#include <stdio.h>

#include "cpu.h"

typedef void (*Instruction_Implementation)(CPU_State *state);    /* A pointer to a handler function */

extern Instruction_Implementation instruction_implementations[256];

#define INSTR_NOT_IMPLEMENTED(name) void instr_##name (CPU_State* s) { printf("Fatal: opcode '%s' is not implemented\n", #name); exit(EXIT_FAILURE); }
#define INSTR(name) void instr_##name (CPU_State* state)
#define GET_OPERAND() (state->pc += sizeof(vm_type_t), *(vm_type_t*)(state->memory->main_memory + state->pc - sizeof(vm_type_t)))
#define GET_OPERAND_SIGNED() (state->pc += sizeof(vm_type_signed_t), *(vm_type_signed_t*)(state->memory->main_memory + state->pc - sizeof(vm_type_signed_t)))
#define GET_OPERAND_FLOAT() (state->pc += sizeof(vm_type_float_t), *(vm_type_float_t*)(state->memory->main_memory + state->pc - sizeof(vm_type_float_t)))

#define USE_STACK() vm_value_t *stack = ((vm_value_t *)(state->memory->main_memory + state->sp))
#define AJS_STACK(n) { state->sp += sizeof(vm_value_t) * (n); }
#define USE_MARK() vm_value_t *mark = ((vm_value_t *)(state->memory->main_memory + state->mp))

int is_ptr_in_static_memory(CPU_State *state, vm_value_t *val);
void retain(CPU_State *state, vm_value_t *ptr);
void release(CPU_State *state, vm_value_t *ptr);
char *cstr_pointer_from_vm_value(CPU_State* state, vm_value_t* val);
void instr_conv_str_rel(CPU_State* state, vm_type_signed_t rel);

void arr_release(CPU_State* state, vm_value_t *ptr);
void arr_insert_at(CPU_State *state, vm_value_t *arrayval, vm_value_t *value, vm_type_signed_t index);
vm_type_t arr_len(CPU_State *state, vm_value_t *arrayval);
void instr_conv_arr_rel(CPU_State* state, vm_type_signed_t rel);


INSTR(nop);
INSTR(halt);
INSTR(trap);
INSTR(int);
INSTR(break);
INSTR(link);

INSTR(ld_int);
INSTR(ld_uint);
INSTR(ld_float);
INSTR(ld_str);
INSTR(ld_obj);
INSTR(ld_local);
INSTR(ld_reg);
INSTR(ld_stack);
INSTR(ld_sref);
INSTR(ld_lref);
INSTR(ld_ref);
INSTR(pop);
INSTR(st_reg);
INSTR(st_stack);
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
INSTR(is_obj);
INSTR(is_ref);
INSTR(is_empty);

#endif //PROCESSOR_INSTRUCTIONS_H

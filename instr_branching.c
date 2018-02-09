#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "vm.h"

#include "instructions.h"

/// Branch Always. Jumps to the destination. Replaces the PC with the destination address.
INSTR(jmp) {
    // PC_post = PC_pre + M_pre[PC_pre + 1] + 2
    state->pc = get_current_module(state)->addr + GET_OPERAND();
}

#define CMP_BRANCH(op) { \
    USE_STACK(); \
    AJS_STACK(-1); \
    vm_type_t jmp_addr = GET_OPERAND(); \
    if (stack->int_value op 0) { state->pc = get_current_module(state)->addr + jmp_addr; } \
}

INSTR(beq) {
    CMP_BRANCH(==);
}
INSTR(bne) {
    CMP_BRANCH(!=);
}
INSTR(blt) {
    CMP_BRANCH(<);
}
INSTR(bgt) {
    CMP_BRANCH(>);
}
INSTR(ble) {
    CMP_BRANCH(<=);
}
INSTR(bge) {
    CMP_BRANCH(>=);
}

INSTR(brfalse) {
    USE_STACK();
    vm_type_t jmp_addr = GET_OPERAND();
    if (stack->uint_value == 0)
        state->pc = get_current_module(state)->addr + jmp_addr;
    AJS_STACK(-1);
}

INSTR(brtrue) {
    USE_STACK();
    vm_type_t jmp_addr = GET_OPERAND();
    if (stack->uint_value != 0)
        state->pc = get_current_module(state)->addr + jmp_addr;
    AJS_STACK(-1);
}

/// Branch to subroutine. Pushes the PC on the stack and jumps to the subroutine.
INSTR(call) {
    // SP_post = SP_pre + 1
    // M_post[SP_post] = PC_pre + 2
    // PC_post = PC_pre + M_pre[PC_pre + 1] + 2


    AJS_STACK(+2);
    USE_STACK();
    vm_type_t addr = GET_OPERAND();
    vm_type_t num_args = GET_OPERAND();
    (stack - 1)->uint_value = state->pc;
    (stack - 1)->type = VM_TYPE_REF;

    state->pc = get_current_module(state)->addr + addr;

    *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = num_args };
}

/// Branch to subroutine. Pops a destination from the stack, pushes the PC on the stack and jumps to the destination.
INSTR(call_pop) {
    // SP_post = SP_pre
    // PC_post = M_pre[SP_pre]
    // M_post[SP_post] = PC_pre + 1

    vm_type_t num_args = GET_OPERAND();

    AJS_STACK(+1);
    USE_STACK();
    assert((stack - 1)->type == VM_TYPE_REF);
    vm_type_t jmp_addr = (stack - 1)->uint_value;

    (stack - 1)->uint_value = state->pc;
    (stack - 1)->type = VM_TYPE_REF;

    state->pc = jmp_addr;

    *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = num_args };

}

/// Jump. Pops a destination from the stack and jumps to the destination.
INSTR(jmp_pop) {
    // SP_post = SP_pre
    // PC_post = M_pre[SP_pre]
    // M_post[SP_post] = PC_pre + 1

    USE_STACK();
    assert(stack->type == VM_TYPE_REF);
    state->pc = stack->uint_value;

    AJS_STACK(-1);
}

/// Return from subroutine. Pops a previously pushed PC from the stack and jumps to it.
INSTR(ret) {
    // SP_post = SP_pre - 1
    // PC_post = M_pre[SP_pre]

    if (state->sp <= state->stack_base) {
        instr_halt(state);
        return;
    }

    AJS_STACK(-1); // num_args, we do nothing with it
    USE_STACK();
    assert(stack->type == VM_TYPE_REF);
    state->pc = stack->uint_value;
    AJS_STACK(-1);
}

INSTR(args_accept) {
    USE_STACK();
    assert(stack->type == VM_TYPE_UINT); // num passed

    vm_value_t AP_pre = { .uint_value = state->ap, .type = VM_TYPE_REF };

    vm_type_signed_t num_args = GET_OPERAND();
    vm_type_signed_t passed_args = stack->uint_value;
    vm_value_t ret_ref = *(stack - 1);

    for (vm_type_signed_t i = 0; i < (num_args - passed_args); i++) {
        *(stack - 1 + i) = (vm_value_t) { .type = VM_TYPE_EMPTY };
    }

    *(stack - 1 + (num_args - passed_args)) = ret_ref;
    *(stack - 0 + (num_args - passed_args)) = AP_pre;
    *(stack + 1 + (num_args - passed_args)) = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = num_args };

    state->ap = state->sp - sizeof(vm_value_t) * (passed_args + 1);

    AJS_STACK(num_args - passed_args + 1);
}

INSTR(args_cleanup) {
    USE_STACK();
    assert(stack->type == VM_TYPE_UINT);      // num_args
    assert((stack - 1)->type == VM_TYPE_REF); // old AP
    assert((stack - 2)->type == VM_TYPE_REF); // return address

    vm_type_t num_args = stack->uint_value;
    state->ap = (stack - 1)->uint_value;
    vm_value_t ret_ref = *(stack - 2);

    for (int i = 0; i < num_args; i++) {
        release(state, stack - 3 - i);
    }

    *(stack - num_args - 1) = *stack;
    *(stack - num_args - 2) = ret_ref;

    AJS_STACK(-num_args - 1);
}

INSTR(ld_arg) {
    AJS_STACK(+1);
    USE_STACK();
    USE_ARGS();
    *stack = *(args + GET_OPERAND_SIGNED());

    retain(state, stack);
}

INSTR(st_arg) {
    USE_STACK();
    USE_ARGS();
    vm_value_t *dst = args + GET_OPERAND_SIGNED();
    release(state, dst);
    *dst = *stack;
    AJS_STACK(-1);
}
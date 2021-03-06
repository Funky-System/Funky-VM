#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <funkyvm/funkyvm.h>

#include "instructions.h"
#include "../../../include/funkyvm/cpu.h"
#include "../../../include/funkyvm/funkyvm.h"
#include "../../../include/funkyvm/memory.h"
#include "../error_handling.h"

void release_pointer(CPU_State *state, enum vm_value_type_t type, vm_pointer_t ptr) {
    vm_type_t *ref_count = vm_pointer_to_native(state->memory, ptr, vm_type_t*);

    // do not free constants from code or null pointers
    if (*ref_count == VM_UNSIGNED_MAX) {
        //printf("Not released string \"%s\", it is in static memory.\n", cstr_pointer_from_vm_value(state, val));
        return;
    }

    (*ref_count)--;

    //printf("Released string \"%s\". Refcount from %d to %d.", cstr_pointer_from_vm_value(state, val), *ref_count + 1, *ref_count);

    if (*ref_count == 0) {
        if (type == VM_TYPE_ARRAY) {
            arr_release(state, ptr);
        } else if (type == VM_TYPE_MAP) {
            map_release(state, ptr);
        }

        //printf(" Refcount is 0, so free memory.");
        vm_free(state->memory, ptr);
    }

}

void release(CPU_State *state, vm_value_t *val) {
    if (val->type != VM_TYPE_STRING && val->type != VM_TYPE_MAP && val->type != VM_TYPE_ARRAY) {
        return;
    }

    vm_type_t *ref_count = vm_pointer_to_native(state->memory, val->pointer_value, vm_type_t*);

    // do not free constants from code or null pointers
    if (*ref_count == VM_UNSIGNED_MAX) {
        //printf("Not released string \"%s\", it is in static memory.\n", cstr_pointer_from_vm_value(state, val));
        return;
    }

    (*ref_count)--;

    //printf("Released string \"%s\". Refcount from %d to %d.", cstr_pointer_from_vm_value(state, val), *ref_count + 1, *ref_count);

    if (*ref_count == 0) {
        if (val->type == VM_TYPE_ARRAY) {
            arr_release(state, val->pointer_value);
        } else if (val->type == VM_TYPE_MAP) {
            map_release(state, val->pointer_value);
        }

        //printf(" Refcount is 0, so free memory.");
        vm_free(state->memory, val->pointer_value);
    }

    //printf("\n");
}

void retain_pointer(CPU_State *state, enum vm_value_type_t type, vm_pointer_t ptr) {
    if (type != VM_TYPE_STRING && type != VM_TYPE_MAP && type != VM_TYPE_ARRAY) {
        return;
    }

    vm_type_t *ref_count = vm_pointer_to_native(state->memory, ptr, vm_type_t*);

    // do not retain constants from code or null pointers
    if (*ref_count == VM_UNSIGNED_MAX) {
        return;
    }

    (*ref_count)++;
}

void retain(CPU_State *state, vm_value_t *val) {
    if (val->type != VM_TYPE_STRING && val->type != VM_TYPE_MAP && val->type != VM_TYPE_ARRAY) {
        return;
    }

    vm_type_t *ref_count = vm_pointer_to_native(state->memory, val->pointer_value, vm_type_t*);

    // do not retain constants from code or null pointers
    if (*ref_count == VM_UNSIGNED_MAX) {
        //printf("Not retaining string \"%s\", it is in static memory.\n", cstr_pointer_from_vm_value(state, val));
        return;
    }

    (*ref_count)++;

    //printf("Retained string \"%s\". Refcount from %d to %d.\n", cstr_pointer_from_vm_value(state, val), *ref_count - 1, *ref_count);

}

/// Load Constant. Pushes the inline constant on the stack.
INSTR(ld_int) {
    AJS_STACK(+1);
    USE_STACK();
    stack->int_value = GET_OPERAND_SIGNED();
    stack->type = VM_TYPE_INT;
}

INSTR(ld_uint) {
    AJS_STACK(+1);
    USE_STACK();
    stack->uint_value = GET_OPERAND();
    stack->type = VM_TYPE_UINT;
}

INSTR(ld_float) {
    AJS_STACK(+1);
    USE_STACK();
    stack->float_value = GET_OPERAND_FLOAT();
    stack->type = VM_TYPE_FLOAT;
}

INSTR(ld_str) {
    AJS_STACK(+1);
    USE_STACK();

    stack->pointer_value = get_current_module(state)->addr + GET_OPERAND();
    stack->type = VM_TYPE_STRING;
}

INSTR(ld_empty) {
    AJS_STACK(+1);
    USE_STACK();
    stack->type = VM_TYPE_EMPTY;
    stack->int_value = 0;
}

/// Load Local. Pushes a value relative to the markpointer.
INSTR(ld_local) {
    // SP_post = SP_pre + 1
    // M_post[SP_post] = M_pre[MP_pre + M_pre[PC_pre+1]]

    AJS_STACK(+1);
    USE_STACK();
    USE_MARK();
    *stack = *(mark + 1 + GET_OPERAND_SIGNED());

    retain(state, stack);
}

/// Load Register. Pushes a value from a register.
INSTR(ld_reg) {
    AJS_STACK(+1);
    USE_STACK();
    vm_type_t rid = GET_OPERAND();
    switch (rid) {
        case 0: *stack = (vm_value_t) { .uint_value = state->pc, .type = VM_TYPE_INT }; break;
        case 1: *stack = (vm_value_t) { .uint_value = state->sp, .type = VM_TYPE_INT }; break;
        case 2: *stack = (vm_value_t) { .uint_value = state->mp, .type = VM_TYPE_INT }; break;
        case 3: *stack = (vm_value_t) { .uint_value = state->ap, .type = VM_TYPE_INT }; break;
        case 4: *stack = state->rr; break;
        case 5: *stack = state->r0; break;
        case 6: *stack = state->r1; break;
        case 7: *stack = state->r2; break;
        case 8: *stack = state->r3; break;
        case 9: *stack = state->r4; break;
        case 10: *stack = state->r5; break;
        case 11: *stack = state->r6; break;
        case 12: *stack = state->r7; break;
        default:
            vm_error(state, "Register id %d is not defined", rid);
            vm_exit(state, EXIT_FAILURE);
            break;
    }

    retain(state, stack);
}

/// Load from Stack. Pushes a value relative to the top of the stack.
INSTR(ld_stack) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = *(stack - 1 + GET_OPERAND_SIGNED());

    retain(state, stack);
}

/// Load Stack Address. Pushes the address of a value relative to the stackpointer.
INSTR(ld_sref) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) {
            .uint_value = state->sp + (GET_OPERAND_SIGNED() - 1) * sizeof(vm_value_t),
            .type = VM_TYPE_REF
    };
}

INSTR_NOT_IMPLEMENTED(ld_href);

/// Load Local Address. Pushes the address of a value relative to the markpointer.
INSTR(ld_lref) {
    AJS_STACK(+1);
    USE_STACK();
    USE_MARK();
    *stack = (vm_value_t) {
            .uint_value = state->mp + (GET_OPERAND_SIGNED() + 1) * sizeof(vm_value_t),
            .type = VM_TYPE_REF
    };
}

INSTR(ld_ref) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) {
            .pointer_value = get_current_module(state)->addr + GET_OPERAND(),
            .type = VM_TYPE_REF
    };
}

INSTR(ld_addr) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) {
            .pointer_value = GET_OPERAND(),
            .type = VM_TYPE_REF
    };
}

INSTR(deref) {
    USE_STACK();

    vm_assert(state, stack->type == VM_TYPE_REF, "Can't dereference value");
    //*stack = *((vm_value_t*)(state->memory->main_memory + stack->uint_value));
    *stack = *vm_pointer_to_native(state->memory, stack->pointer_value, vm_value_t*);

    retain(state, stack);
}

INSTR(ld_deref) {
    AJS_STACK(+1);
    USE_STACK();

    *stack = *vm_pointer_to_native(state->memory, get_current_module(state)->addr + GET_OPERAND(), vm_value_t*);
    retain(state, stack);
}

INSTR(pop) {
    USE_STACK();
    release(state, stack);

    AJS_STACK(-1);
}

INSTR(st_reg) {
    USE_STACK();
    vm_type_t rid = GET_OPERAND();
    switch (rid) {
        case 0: state->pc = stack->uint_value; break;
        case 1: state->sp = stack->uint_value; break;
        case 2: state->mp = stack->uint_value; break;
        case 3: state->ap = stack->uint_value; break;
        case 4: release(state, &state->rr); state->rr = *stack; break;
        case 5: release(state, &state->r0); state->r0 = *stack; break;
        case 6: release(state, &state->r1); state->r1 = *stack; break;
        case 7: release(state, &state->r2); state->r2 = *stack; break;
        case 8: release(state, &state->r3); state->r3 = *stack; break;
        case 9: release(state, &state->r4); state->r4 = *stack; break;
        case 10: release(state, &state->r5); state->r5 = *stack; break;
        case 11: release(state, &state->r6); state->r6 = *stack; break;
        case 12: release(state, &state->r7); state->r7 = *stack; break;
        default:
            vm_error(state, "Register id %d is not defined", rid);
            vm_exit(state, EXIT_FAILURE);
            break;
    }

    AJS_STACK(-1);
}

/// Store into Stack. Pops a value from the stack and stores it in a location relative to the top of the stack.
INSTR(st_stack) {
    USE_STACK();
    vm_value_t *dst = stack + GET_OPERAND_SIGNED();
    if (dst == stack) return;
    release(state, dst);
    *dst = *stack;
    AJS_STACK(-1);
}

INSTR(st_stack_pop) {
    AJS_STACK(-1);
    USE_STACK();
    vm_type_signed_t rel = (stack + 1)->int_value;

    vm_value_t *dst = stack + rel;
    if (dst == stack) return;
    release(state, dst);
    *dst = *stack;
    AJS_STACK(-1);
}

/// Store into local. Pops a value from the stack and stores it in a location relative to the markpointer.
INSTR(st_local) {
    USE_STACK();
    USE_MARK();
    vm_value_t *dst = mark + 1 + GET_OPERAND_SIGNED();
    release(state, dst);
    *dst = *stack;
    AJS_STACK(-1);
}

INSTR(st_addr) {
    USE_STACK();
    vm_value_t *dst = vm_pointer_to_native(state->memory, GET_OPERAND(), vm_value_t*);
    release(state, dst);
    *dst = *stack;
    AJS_STACK(-1);
}

INSTR(st_ref) {
    USE_STACK();
    vm_value_t *dst = vm_pointer_to_native(state->memory, get_current_module(state)->addr + GET_OPERAND(), vm_value_t*);
    release(state, dst);
    *dst = *stack;
    AJS_STACK(-1);
}

/// Adjust Stack. Adjusts the stack pointer with fixed amount.
INSTR(ajs) {
    vm_type_signed_t operand = GET_OPERAND_SIGNED();
    if (operand < 0) {
        for (vm_type_signed_t i = operand; i < 0; i++) {
            instr_pop(state);
        }
    } else {
        AJS_STACK(operand);
    }
}

/// Reserve memory for locals. Convenience instruction combining the push of MP and the adjustment of the SP.
INSTR(locals_res) {
    // MP_post = SP_pre + 1
    // M_post[MP_post] = MP_pre
    // SP_post = MP_post + M_pre[PC_pre+1]

    vm_value_t MP_pre = { .uint_value = state->mp, .type = VM_TYPE_REF };
    state->mp = state->sp + sizeof(vm_value_t);

    USE_MARK();

    vm_type_signed_t num = GET_OPERAND_SIGNED();

    *mark = MP_pre;
    state->sp = state->mp + sizeof(vm_value_t) * num;

    for (int i = 1; i <= num; i++) {
        (mark + i)->type = VM_TYPE_INT;
        (mark + i)->int_value = 0;
    }
}

/// Free memory for locals. Convenience instruction combining the push of MP and the adjustment of the SP.
INSTR(locals_cleanup) {
    // MP_post = M_pre[MP_pre]
    // SP_post = MP_pre - 1

    USE_MARK();
    unsigned int num_lost = (state->sp - state->mp) / sizeof(vm_value_t);
    state->sp = state->mp - sizeof(vm_value_t);
    vm_assert(state, mark->type == VM_TYPE_REF, "Junk on stack, MP lost.");
    state->mp = mark->uint_value;

    for (int i = 1; i <= num_lost; i++) {
        release(state, mark + i);
    }
}

/// Duplicate top of stack
INSTR(dup) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = *(stack - 1);
    retain(state, stack);
}

/// Swap values. Swaps the 2 topmost values on the stack.
INSTR(swp) {
    USE_STACK();
    vm_value_t tmp = *stack;
    *stack = *(stack - 1);
    *(stack - 1) = tmp;
}

/// Reserve data
INSTR(var) {
    state->pc += sizeof(vm_value_t) - 1;
}

INSTR(is_int) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = (stack - 1)->type == VM_TYPE_INT };
}

INSTR(is_uint) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = (stack - 1)->type == VM_TYPE_UINT };
}

INSTR(is_float) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = (stack - 1)->type == VM_TYPE_FLOAT };
}

INSTR(is_str) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = (stack - 1)->type == VM_TYPE_STRING };
}

INSTR(is_arr) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = (stack - 1)->type == VM_TYPE_ARRAY };
}

INSTR(is_map) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = (stack - 1)->type == VM_TYPE_MAP };
}

INSTR(is_ref) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = (stack - 1)->type == VM_TYPE_REF };
}

INSTR(is_empty) {
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = (stack - 1)->type == VM_TYPE_EMPTY };
}
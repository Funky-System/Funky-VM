#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "instructions.h"
#include "funkyvm/funkyvm.h"
#include "../error_handling.h"

INSTR(conv_int) {
    USE_STACK();
    switch (stack->type) {
        case VM_TYPE_UINT: {
            stack->int_value = stack->uint_value;
            stack->type = VM_TYPE_INT;
            break;
        }
        case VM_TYPE_INT: {
            //CONVERT_NATIVE(vm_type_signed_t, vm_type_signed_t);
            //stack->type = VM_TYPE_INT;
            // do nothing
            break;
        }
        case VM_TYPE_FLOAT: {
            stack->int_value = (vm_type_signed_t)stack->float_value;
            stack->type = VM_TYPE_INT;
            break;
        }
        case VM_TYPE_STRING: {
            vm_value_t strval = *stack;
            stack->int_value = (vm_type_signed_t)strtol(cstr_pointer_from_vm_value(state, stack), NULL, 0);
            stack->type = VM_TYPE_INT;
            release(state, &strval);
            break;
        }
        case VM_TYPE_EMPTY: {
            stack->int_value = 0;
            stack->type = VM_TYPE_INT;
            break;
        }
        case VM_TYPE_REF:
            // TODO
            //break;
        case VM_TYPE_MAP:
            // TODO
            //break;
        case VM_TYPE_UNKNOWN:
        default:
            vm_error(state, "Top of stack is of unknown type, can't convert to INT");
            vm_exit(state, EXIT_FAILURE);
    }
}

INSTR(conv_uint) {
    USE_STACK();
    switch (stack->type) {
        case VM_TYPE_UINT: {
            //CONVERT_NATIVE(vm_type_t, vm_type_t);
            //stack->type = VM_TYPE_UINT;
            // Do nothing
            break;
        }
        case VM_TYPE_INT: {
            stack->uint_value = (vm_type_t) stack->int_value;
            stack->type = VM_TYPE_UINT;
            break;
        }
        case VM_TYPE_FLOAT: {
            stack->uint_value = (vm_type_t) stack->float_value;
            stack->type = VM_TYPE_UINT;
            break;
        }
        case VM_TYPE_STRING: {
            vm_value_t strval = *stack;
            stack->uint_value = (vm_type_signed_t)strtoul(cstr_pointer_from_vm_value(state, stack), NULL, 0);
            stack->type = VM_TYPE_UINT;
            release(state, &strval);
            break;
        }
        case VM_TYPE_EMPTY: {
            stack->uint_value = 0;
            stack->type = VM_TYPE_UINT;
            break;
        }
        case VM_TYPE_REF:
            // TODO
            //break;
        case VM_TYPE_MAP:
            // TODO
            //break;
        case VM_TYPE_UNKNOWN:
        default:
            vm_error(state, "Top of stack is of unknown type, can't convert to INT");
            vm_exit(state, EXIT_FAILURE);
    }
}

INSTR(conv_float) {
    USE_STACK();
    switch (stack->type) {
        case VM_TYPE_UINT: {
            stack->float_value = stack->uint_value;
            stack->type = VM_TYPE_FLOAT;
            break;
        }
        case VM_TYPE_INT: {
            stack->float_value = stack->int_value;
            stack->type = VM_TYPE_FLOAT;
            break;
        }
        case VM_TYPE_FLOAT: {
            //CONVERT_NATIVE(vm_type_float_t, vm_type_float_t);
            //stack->type = VM_TYPE_INT;
            // do nothing
            break;
        }
        case VM_TYPE_STRING: {
            vm_value_t strval = *stack;
            stack->float_value = (vm_type_float_t)strtof(cstr_pointer_from_vm_value(state, stack), NULL);
            stack->type = VM_TYPE_FLOAT;
            release(state, &strval);
            break;
        }
        case VM_TYPE_EMPTY: {
            stack->float_value = .0;
            stack->type = VM_TYPE_FLOAT;
            break;
        }
        case VM_TYPE_REF:
            // TODO
            //break;
        case VM_TYPE_MAP:
            // TODO
            //break;
        case VM_TYPE_UNKNOWN:
        default:
            vm_error(state, "Top of stack is of unknown type, can't convert to INT");
            vm_exit(state, EXIT_FAILURE);
    }
}

// conv_str in instr_string.h

INSTR(cast_int) {
    USE_STACK();
    stack->type = VM_TYPE_INT;
}

INSTR(cast_uint) {
    USE_STACK();
    stack->type = VM_TYPE_UINT;
}

INSTR(cast_float) {
    USE_STACK();
    stack->type = VM_TYPE_FLOAT;
}

INSTR(cast_ref) {
    USE_STACK();
    stack->type = VM_TYPE_REF;
}

INSTR(cast_str) {
    USE_STACK();
    stack->type = VM_TYPE_STRING;
}

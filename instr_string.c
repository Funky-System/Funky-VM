#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include "instructions.h"
#include "vm.h"
#include "cpu.h"
#include "error_handling.h"

/* -- Strings --
 * Arrays are saved in memory as a packed tuple:
 *   vm_type_t length
 *   char*     pointer to char* (c string)
 */

// Strings are refcounted
// Strings are immutable, e.g. when changing a string or extracting a portion, a new copy is created

char *cstr_pointer_from_vm_pointer_t(CPU_State* state, vm_pointer_t ptr) {
    return vm_pointer_to_native(state->memory, ptr, char*);
}

char *cstr_pointer_from_vm_value(CPU_State* state, vm_value_t* val) {
    return cstr_pointer_from_vm_pointer_t(state, val->pointer_value + sizeof(vm_type_t));
}

INSTR(strcat) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_STRING, "String concatenation with non-string left operand");
    vm_assert(state, (stack - 1)->type == VM_TYPE_STRING, "String concatenation with non-string right operand");

    vm_pointer_t reserved_mem = vm_malloc(state->memory,
            sizeof(vm_type_t)
            + strlen(cstr_pointer_from_vm_value(state, stack))
            + strlen(cstr_pointer_from_vm_value(state, stack - 1))
            + 1);
    vm_type_t *ref_count = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    char *str = vm_pointer_to_native(state->memory, reserved_mem + sizeof(vm_type_t), char*);

    *ref_count = 1;
    strcpy(str, cstr_pointer_from_vm_value(state, stack-1));
    strcat(str, cstr_pointer_from_vm_value(state, stack));

    release(state, stack);
    release(state, stack - 1);

    (stack-1)->pointer_value = reserved_mem;

    AJS_STACK(-1);
}
INSTR(substr) {
    USE_STACK();
    vm_assert(state, (stack - 2)->type == VM_TYPE_STRING, "Substring on non-string value");

    vm_type_signed_t start = (stack - 1)->int_value;
    vm_type_signed_t length = stack->int_value;

    vm_type_signed_t orig_length = (vm_type_signed_t) strlen(cstr_pointer_from_vm_value(state, stack - 2));

    if (length < 0) {
        length = orig_length - start + length;
        if (length < 0) length = 0;
    }

    vm_pointer_t reserved_mem = vm_malloc(state->memory,
                                          sizeof(vm_type_t)
                                          + length
                                          + 1);
    vm_type_t *ref_count = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    char *str = vm_pointer_to_native(state->memory, reserved_mem + sizeof(vm_type_t), char*);

    *ref_count = 1;

    str[0] = '\0';
    if (orig_length - start > 0) {
        strncat(str, cstr_pointer_from_vm_value(state, stack - 2) + start, length);
    }

    release(state, stack - 2);

    (stack-2)->pointer_value = reserved_mem;

    AJS_STACK(-2);
}

INSTR(strlen) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_STRING, "Can't get string length from non-string value");

    vm_type_t len = (vm_type_t) strlen(cstr_pointer_from_vm_value(state, stack));
    release(state, stack);
    stack->uint_value = len;
    stack->type = VM_TYPE_UINT;
}

void instr_conv_str_rel(CPU_State* state, vm_type_signed_t rel) {
    USE_STACK();
    vm_pointer_t reserved_mem = vm_malloc(state->memory,
                                          sizeof(vm_type_t)
                                          + 32
                                          + 1);
    vm_type_t *ref_count = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    char *str = vm_pointer_to_native(state->memory, reserved_mem + sizeof(vm_type_t), char*);

    *ref_count = 1;

    switch ((stack + rel)->type) {
        case VM_TYPE_UINT:
            sprintf(str, "%u", (stack + rel)->uint_value);
            (stack + rel)->pointer_value = reserved_mem;
            break;
        case VM_TYPE_INT:
            sprintf(str, "%d", (stack + rel)->int_value);
            (stack + rel)->pointer_value = reserved_mem;
            break;
        case VM_TYPE_FLOAT: {
            sprintf(str, "%f", (stack + rel)->float_value);
            (stack + rel)->pointer_value = reserved_mem;
            break;
        }
        case VM_TYPE_STRING:
            vm_free(state->memory, reserved_mem);
            break;
        case VM_TYPE_EMPTY: {
            strcpy(str, "");
            (stack + rel)->pointer_value = reserved_mem;
            break;
        }
        case VM_TYPE_REF:
            // TODO
            //break;
        case VM_TYPE_MAP:
            // TODO
            //break;
        case VM_TYPE_ARRAY:
            // TODO
            //break;
        case VM_TYPE_UNKNOWN:
        default:
            vm_free(state->memory, reserved_mem);
            vm_error(state, "Top of stack is of unknown type, can't convert to INT");
            vm_exit(state, EXIT_FAILURE);
    }
    (stack + rel)->type = VM_TYPE_STRING;
}

INSTR(conv_str) {
    instr_conv_str_rel(state, 0);
}

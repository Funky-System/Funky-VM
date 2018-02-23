#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <funkyvm/funkyvm.h>
#include "instructions.h"
#include "../../../include/funkyvm/funkyvm.h"
#include "../../../include/funkyvm/cpu.h"
#include "../error_handling.h"

#include "../../../include/funkyvm/os.h"
#ifdef FUNKY_VM_OS_EMSCRIPTEN
#pragma pack(1)
#endif

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

/**!
 * instruction: strcat
 * category: strings
 * opcode: "0x60"
 * description: Concatenate two strings.
 * stack_pre:
 *   - type: string
 *     description: the second string
 *   - type: string
 *     description: the first string
 * stack_post:
 *   - type: string
 *     description: Concatenation of first + second string
 */
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
/**!
 * instruction: substr
 * category: strings
 * opcode: "0x61"
 * description: Pushes a substring of string on top of stack.
 * stack_pre:
 *   - type: int
 *     description: length (negative length counts from end)
 *   - type: int
 *     description: start index
 *   - type: string
 *     description: the string
 * stack_post:
 *   - type: string
 *     description: substring result
 */
INSTR(substr) {
    USE_STACK();
    vm_assert(state, (stack - 2)->type == VM_TYPE_STRING, "Substring on non-string value");

    vm_type_signed_t start = (stack - 1)->int_value;
    vm_type_signed_t length = stack->int_value;

    vm_type_signed_t orig_length = (vm_type_signed_t) strlen(cstr_pointer_from_vm_value(state, stack - 2));

    if (start < 0) {
        start = orig_length + start + 1;
    }

    if (length < 0) {
        length = orig_length - start + length + 1;
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

/**!
 * instruction: strlen
 * category: strings
 * opcode: "0x62"
 * description: Puts length of string on the stack.
 * stack_pre:
 *   - type: string
 *     description: the string
 * stack_post:
 *   - type: uint
 *     description: length of string
 */
INSTR(strlen) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_STRING, "Can't get string length from non-string value");

    vm_type_t len = (vm_type_t) strlen(cstr_pointer_from_vm_value(state, stack));
    release(state, stack);
    stack->uint_value = len;
    stack->type = VM_TYPE_UINT;
}

/**
 * Converts a value on the stack to string. Do not call this function after calls to GET_OPERAND() and such, which alter
 * the Program Counter (PC). If you must, rewind the PC to the first byte right after the current instruction before
 * calling this function.
 * @param the current CPU state
 * @param rel the offset from the top of the stack to the value to convert
 * @return 1 if the exection of the current instruction needs to be aborted A.S.A.P. It will be called again once
 *         the value has been converted to a string.
 *         0 if the value has been converted, execution may resume.
 */
int conv_str_rel(CPU_State *state, vm_type_signed_t rel) {
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
            sprintf(str, "%llu", (stack + rel)->uint_value);
            (stack + rel)->pointer_value = reserved_mem;
            break;
        case VM_TYPE_INT:
            sprintf(str, "%lld", (stack + rel)->int_value);
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
        {
            vm_map_elem_t *elem = ld_mapitem(state, (stack + rel)->pointer_value, "string");
            if (elem != NULL) {
                state->r1 = *(stack + rel);
                retain(state, stack + rel);

                state->r7.type = VM_TYPE_INT;
                state->r7.int_value = rel;

                AJS_STACK(+2);
                USE_STACK();
                vm_type_t addr = elem->value.pointer_value;
                vm_type_t num_args = 0;
                (stack - 1)->uint_value = state->pc - 1;
                (stack - 1)->type = VM_TYPE_REF;

                state->pc = addr;

                *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = num_args };
                vm_free(state->memory, reserved_mem);
                return 1;
            } else {
                strcpy(str, "(map)");
                (stack + rel)->pointer_value = reserved_mem;
            }
        }
            break;
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

    return 0;
}

/**!
 * instruction: conv.str
 * category: strings
 * opcode: "0x23"
 * description: Convert top of stack to string.
 * stack_pre:
 *   - type: any
 *     description: any value convertable to string
 * stack_post:
 *   - type: string
 *     description: converted string
 */
INSTR(conv_str) {
    conv_str_rel(state, 0);
}

void str_eq(CPU_State *state) {
    USE_STACK();
    if (stack->type != VM_TYPE_STRING) {
        if (conv_str_rel(state, 0)) return;
    }

    if ((stack - 1)->type != VM_TYPE_STRING) {
        if (conv_str_rel(state, -1)) return;
    }

    const char *str1 = cstr_pointer_from_vm_value(state, stack - 1);
    const char *str2 = cstr_pointer_from_vm_value(state, stack);

    vm_type_signed_t eq = strcmp(str1, str2) == 0;

    release(state, stack);
    release(state, stack - 1);

    (stack - 1)->type = VM_TYPE_INT;
    (stack - 1)->int_value = eq;
    AJS_STACK(-1);
}

void str_ne(CPU_State *state) {
    USE_STACK();
    if (stack->type != VM_TYPE_STRING) {
        if (conv_str_rel(state, 0)) return;
    }

    if ((stack - 1)->type != VM_TYPE_STRING) {
        if (conv_str_rel(state, -1)) return;
    }
    const char *str1 = cstr_pointer_from_vm_value(state, stack - 1);
    const char *str2 = cstr_pointer_from_vm_value(state, stack);

    vm_type_signed_t ne = strcmp(str1, str2) != 0;

    release(state, stack);
    release(state, stack - 1);

    (stack - 1)->type = VM_TYPE_INT;
    (stack - 1)->int_value = ne;
    AJS_STACK(-1);
}

void ld_arrelem_str(CPU_State *state) {
    USE_STACK();

    vm_assert(state, (stack-1)->type == VM_TYPE_STRING, "value is not a string");
    instr_conv_int(state); // ensure top of stack is an unsigned integer, aka: the index

    vm_type_signed_t index = stack->int_value;

    const char *str = (const char*)cstr_pointer_from_vm_value(state, stack - 1);
    vm_type_t len = (vm_type_t)strlen(str);

    // Negative index is index from end
    if (index < 0) index = len + index;

    if (index > len - 1 || index < 0) {
        vm_error(state, "Error: index is out of range");
        vm_exit(state, EXIT_FAILURE);
    }

    vm_value_t val = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = ((const unsigned char *)str)[index] };
    release(state, stack - 1); // release the string

    *(stack - 1) = val;

    AJS_STACK(-1);
}

void st_arrelem_str(CPU_State *state) {
    USE_STACK();

    vm_assert(state, (stack-1)->type == VM_TYPE_STRING, "value is not a string");
    instr_conv_int(state); // ensure top of stack is an unsigned integer, aka: the index

    vm_type_signed_t index = stack->int_value;

    char *str = cstr_pointer_from_vm_value(state, stack - 1);
    vm_type_t len = (vm_type_t)strlen(str);

    // Negative index is index from end
    if (index < 0) index = len + index;

    if (index < 0) {
        vm_error(state, "Error: index is out of range");
        vm_exit(state, EXIT_FAILURE);
    }

    if (index > len - 1) {
        vm_pointer_t reserved_mem = vm_malloc(state->memory,
                                              sizeof(vm_type_t)
                                              + index + 1
                                              + 1);
        vm_type_t *ref_count = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
        char *str2 = vm_pointer_to_native(state->memory, reserved_mem + sizeof(vm_type_t), char*);

        *ref_count = 1;
        strcpy(str2, str);
        for (int i = 0; i < index - (len - 1); i++) {
            strcat(str2, " ");
        }
        str = str2;
        release(state, stack - 1);

        (stack-1)->pointer_value = reserved_mem;
    }

    str[index] = (char)((stack - 2)->uint_value);

    release(state, stack - 1); // release the string
    AJS_STACK(-3);

}

void arr_slice_str(CPU_State *state) {
    // str.substr expects a start position and length, but arr.slice expects a start position and an inclusive
    // end position

    USE_STACK();

    vm_value_t* start = stack - 1;
    vm_value_t* end = stack;

    if (end->int_value >= 0) {
        end->int_value -= start->int_value;
    }

    instr_substr(state);
}

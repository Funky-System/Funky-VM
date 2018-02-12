#include <assert.h>
#include "instructions.h"
#include "vm.h"

/* -- Arrays --
 * Arrays are saved in memory as a packed tuple:
 *   vm_type_t length
 *   vm_type_t ref_count
 *   vm_type_t pointer to array of multiple vm_value_t
 */

// Arrays are refcounted
// Arrays are mutable. That means that arrays are changed in-place

INSTR(ld_arr) {
    USE_STACK();

    vm_value_t arrayval;
    arrayval.type = VM_TYPE_ARRAY;

    vm_pointer_t reserved_mem = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for length
    vm_type_t *ref_count      = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_type_t *length         = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*) + 1;
    vm_pointer_t *array_ptr   = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    *ref_count = 1;
    *length = GET_OPERAND();

    *array_ptr = vm_malloc(state->memory, *length * sizeof(vm_value_t));
    vm_value_t *array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

    for (int i = 0; i < *length; i++) {
        array[i] = *(stack - *length + 1 + i);
        retain(state, &array[i]);
    }

    arrayval.pointer_value = reserved_mem;

    *(stack - *length + 1) = arrayval;

    AJS_STACK(-(*length) + 1);
}

INSTR(ld_arrelem) {
    USE_STACK();
    vm_assert(state, (stack-1)->type == VM_TYPE_ARRAY, "value is not an array");
    instr_conv_int(state); // ensure top of stack is an unsigned integer, aka: the index

    vm_type_signed_t index = stack->int_value;

    vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, (stack - 1)->pointer_value, vm_type_t*);
    vm_type_t len = *(reserved_mem + 1);

    // Negative index is index from end
    if (index < 0) index = len + index;

    if (index > len - 1 || index < 0) {
        vm_error(state, "Error: index is out of range");
        vm_exit(state, EXIT_FAILURE);
    }
    vm_pointer_t *array_ptr = reserved_mem + 2;
    vm_value_t *array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

    vm_value_t val = *(array + index);
    retain(state, &val); // retain value
    release(state, stack - 1); // release the array

    *(stack - 1) = val;

    AJS_STACK(-1);
}

INSTR(st_arrelem) {
    USE_STACK();
    vm_assert(state, (stack - 1)->type == VM_TYPE_ARRAY, "value is not an array");
    instr_conv_int(state); // ensure top of stack is an unsigned integer, aka: the index

    vm_type_signed_t index = stack->int_value;

    vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, (stack - 1)->pointer_value, vm_type_t*);
    vm_type_t *len = reserved_mem + 1;

    // Negative index is index from end
    if (index < 0) index = *len + index;
    if (index < 0) {
        vm_error(state, "Error: index is out of range");
        vm_exit(state, EXIT_FAILURE);
    }

    vm_pointer_t *array_ptr = reserved_mem + 2;
    vm_value_t *array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

    if (index > (vm_type_signed_t)*len - 1) {
        *array_ptr = vm_realloc(state->memory, *array_ptr, (index + 1) * sizeof(vm_value_t));
        array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);
        for (int i = (vm_type_signed_t)*len; i < index; i++) {
            array[i] = (vm_value_t) { .type = VM_TYPE_EMPTY };
        }
        *len = (vm_type_t) (index + 1);
    }

    array[index] = *(stack - 2);

    release(state, stack - 1); // release the array
    // no release/retain for value, as it is reduced by one because of stack pop, but added by one because of array storage
    AJS_STACK(-3);
}

INSTR(del_arrelem) {
    USE_STACK();
    vm_assert(state, (stack - 1)->type == VM_TYPE_ARRAY, "value is not an array");
    instr_conv_int(state); // ensure top of stack is an unsigned integer, aka: the index

    vm_type_signed_t index = stack->int_value;

    vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, (stack - 1)->pointer_value, vm_type_t*);
    vm_type_t *len = reserved_mem + 1;

    // Negative index is index from end
    if (index < 0) index = *len + index;

    if (index > *len - 1 || index < 0) {
        vm_error(state, "Error: index is out of range");
        vm_exit(state, EXIT_FAILURE);
    }

    vm_pointer_t *array_ptr = reserved_mem + 2;
    vm_value_t *array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

    if (index == *len - 1) {
        // last element
        (*len)--;
        release(state, &(array[index])); // release the value
        release(state, stack - 1); // release the array
        AJS_STACK(-2);
        return;
    } else {
        // element somewhere in the middle or front
        release(state, &(array[index])); // release the value

        // move all values after the index to fill the gap
        for (vm_type_signed_t i = index + 1; i < *len; i++) {
            array[i - 1] = array[i];
        }
        (*len)--;

        release(state, stack - 1); // release the array
    }

    AJS_STACK(-2);
}

INSTR(arr_len) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_ARRAY, "value is not an array");

    vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, stack->pointer_value, vm_type_t*);
    vm_type_t len = *(reserved_mem + 1);
    release(state, stack);
    stack->uint_value = len;
    stack->type = VM_TYPE_UINT;
}

vm_type_t arr_len(CPU_State *state, vm_value_t *arrayval) {
    vm_assert(state, arrayval->type == VM_TYPE_ARRAY, "value is not an array");

    vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, arrayval->pointer_value, vm_type_t*);
    vm_type_t *len = reserved_mem + 1;

    return *len;
}


void arr_insert_at(CPU_State *state, vm_value_t *arrayval, vm_value_t *value, vm_type_signed_t index) {
    vm_assert(state, arrayval->type == VM_TYPE_ARRAY, "value is not an array");

    vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, arrayval->pointer_value, vm_type_t*);
    vm_type_t *len = reserved_mem + 1;

    // Negative index is index from end
    if (index < 0) index = *len + index;
    if (index < 0) {
        vm_error(state, "Error: index is out of range");
        vm_exit(state, EXIT_FAILURE);
    }

    if (index > *len - 1) {
        vm_pointer_t *array_ptr = reserved_mem + 2;
        vm_value_t *array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

        if (index > (vm_type_signed_t)*len - 1) {
            *array_ptr = vm_realloc(state->memory, *array_ptr, (index + 1) * sizeof(vm_value_t));
            array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

            for (int i = (vm_type_signed_t)*len; i < index; i++) {
                array[i] = (vm_value_t) { .type = VM_TYPE_EMPTY };
            }
            *len = (vm_type_t) (index + 1);
        }
        array[index] = *value;

        return;
    }

    vm_pointer_t *array_ptr = reserved_mem + 2;
    (*len)++;
    *array_ptr = vm_realloc(state->memory, *array_ptr, *len * sizeof(vm_value_t));
    vm_value_t *array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);
    // move all values after the index up one place to create a gap at index
    for (vm_type_signed_t i = *len - 1; i > index; i--) {
        array[i] = array[i - 1];
    }

    array[index] = *value;

}

INSTR(arr_insert) {
    USE_STACK();
    vm_assert(state, (stack - 1)->type == VM_TYPE_ARRAY, "value is not an array");
    instr_conv_int(state); // ensure top of stack is an unsigned integer, aka: the index

    vm_type_signed_t index = stack->int_value;

    arr_insert_at(state, stack - 1, stack - 2, index);

    release(state, stack - 1); // release the array

    AJS_STACK(-3);
}

INSTR(arr_slice) {
    USE_STACK();
    vm_assert(state, (stack - 2)->type == VM_TYPE_ARRAY, "value is not an array");

    vm_type_t *orig_reserved_mem = vm_pointer_to_native(state->memory, (stack - 2)->pointer_value, vm_type_t*);
    vm_type_t *orig_len = orig_reserved_mem + 1;
    vm_pointer_t *orig_array_ptr = orig_reserved_mem + 2;
    vm_value_t *orig_array = vm_pointer_to_native(state->memory, *orig_array_ptr, vm_value_t*);

    instr_conv_int(state); // ensure top of stack is an unsigned integer, aka: the start
    AJS_STACK(-1); instr_conv_int(state); AJS_STACK(+1); // ensure stack - 1 is an unsigned integer, aka: the end

    vm_type_signed_t start = (stack - 1)->int_value;
    vm_type_signed_t end = (stack)->int_value;

    if (start < 0) start = *orig_len + start;
    if (start < 0 || start > *orig_len - 1) {
        vm_error(state, "Error: index is out of range");
        vm_exit(state, EXIT_FAILURE);
    }
    if (end < 0) end = *orig_len + end;
    if (end < 0 || end > *orig_len - 1) {
        vm_error(state, "Error: index is out of range");
        vm_exit(state, EXIT_FAILURE);
    }

    if (start > end) {
        vm_error(state, "Error: slice has negative length");
        vm_exit(state, EXIT_FAILURE);
    }

    vm_type_t reserved_mem = vm_malloc(state->memory, sizeof(vm_type_t) * 3);
    vm_type_t *ref_count      = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_type_t *length         = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*) + 1;
    vm_pointer_t *array_ptr   = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    *ref_count = 1;
    *length = (vm_type_t) (end - start  + 1);

    *array_ptr = vm_malloc(state->memory, *length * sizeof(vm_value_t));
    vm_value_t *new_array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);


    for (int i = 0; i < *length; i++) {
        new_array[i] = orig_array[start + i];
        retain(state, &new_array[i]);
    }

    release(state, stack - 2);

    (stack - 2)->pointer_value = reserved_mem;
    AJS_STACK(-2);
}

INSTR(arr_concat) {
    USE_STACK();
    vm_assert(state, (stack - 1)->type == VM_TYPE_ARRAY, "left operand is not an array");
    vm_assert(state, stack->type == VM_TYPE_ARRAY, "right operand is not an array");

    vm_type_t *first_reserved_mem = vm_pointer_to_native(state->memory, (stack - 1)->pointer_value, vm_type_t*);
    vm_type_t *first_len = first_reserved_mem + 1;
    vm_pointer_t *first_array_ptr = first_reserved_mem + 2;
    vm_value_t *first_array = vm_pointer_to_native(state->memory, *first_array_ptr, vm_value_t*);

    vm_type_t *second_reserved_mem = vm_pointer_to_native(state->memory, (stack)->pointer_value, vm_type_t*);
    vm_type_t *second_len = second_reserved_mem + 1;
    vm_pointer_t *second_array_ptr = second_reserved_mem + 2;
    vm_value_t *second_array = vm_pointer_to_native(state->memory, *second_array_ptr, vm_value_t*);

    vm_pointer_t reserved_mem = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for length
    vm_type_t *ref_count      = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_type_t *length         = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*) + 1;
    vm_pointer_t *array_ptr   = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    *ref_count = 1;
    *length = (vm_type_t) (*first_len + *second_len);

    *array_ptr = vm_malloc(state->memory, *length * sizeof(vm_value_t));
    vm_value_t *new_array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

    // copy first array
    for (int i = 0; i < *first_len; i++) {
        new_array[i] = first_array[i];
        retain(state, &new_array[i]);
    }
    // copy second array
    for (int i = 0; i < *second_len; i++) {
        new_array[*first_len + i] = second_array[i];
        retain(state, &new_array[*first_len + i]);
    }

    release(state, stack - 1);
    release(state, stack);

    (stack - 1)->pointer_value = reserved_mem;
    AJS_STACK(-1);
}

INSTR(arr_copy) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_ARRAY, "value is not an array");

    vm_type_t *orig_reserved_mem = vm_pointer_to_native(state->memory, (stack)->pointer_value, vm_type_t*);
    vm_type_t *orig_len = orig_reserved_mem + 1;
    vm_pointer_t *orig_array_ptr = orig_reserved_mem + 2;
    vm_value_t *orig_array = vm_pointer_to_native(state->memory, *orig_array_ptr, vm_value_t*);

    vm_pointer_t reserved_mem = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for length
    vm_type_t *ref_count      = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_type_t *length         = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*) + 1;
    vm_pointer_t *array_ptr   = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    *ref_count = 1;
    *length = *orig_len;

    *array_ptr = vm_malloc(state->memory, *orig_len * sizeof(vm_value_t));
    vm_value_t *new_array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

    for (int i = 0; i < *length; i++) {
        new_array[i] = orig_array[i];
        retain(state, &new_array[i]);
    }

    release(state, stack);
    stack->pointer_value = reserved_mem;
}

void arr_release(CPU_State* state, vm_pointer_t ptr) {
    vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, ptr, vm_type_t*);
    vm_type_t *len = reserved_mem + 1;
    vm_pointer_t *array_ptr = reserved_mem + 2;
    vm_value_t *array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

    for (int i = 0; i < *len; i++) {
        release(state, &array[i]);
    }
}

void instr_conv_arr_rel(CPU_State* state, vm_type_signed_t rel) {
    USE_STACK();

    if ((stack + rel)->type == VM_TYPE_ARRAY) return;

    vm_value_t arrayval;
    arrayval.type = VM_TYPE_ARRAY;

    vm_pointer_t reserved_mem = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for length
    vm_type_t *ref_count      = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_type_t *length         = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*) + 1;
    vm_pointer_t *array_ptr   = (vm_pointer_t *)vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*) + 2;

    *ref_count = 1;
    *length = 1;

    *array_ptr = vm_malloc(state->memory, sizeof(vm_value_t));
    vm_value_t *array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

    arrayval.pointer_value = reserved_mem;

    array[0] = *(stack + rel);
    retain(state, &array[0]);

    *(stack + rel) = arrayval;
}

INSTR(conv_arr) {
    instr_conv_arr_rel(state, 0);
}

INSTR(arr_range) {
    USE_STACK();

    vm_value_t arrayval;
    arrayval.type = VM_TYPE_ARRAY;

    vm_pointer_t reserved_mem = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for length
    vm_type_t *ref_count      = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_type_t *length         = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*) + 1;
    vm_pointer_t *array_ptr   = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    vm_type_signed_t start = (stack - 1)->int_value;
    vm_type_signed_t end = (stack)->int_value;

    vm_type_signed_t step = 1;

    if (start > end) {
        step = -1;
    }

    *ref_count = 1;
    *length = (vm_type_t) ((end - start + step) / step);

    *array_ptr = vm_malloc(state->memory, *length * sizeof(vm_value_t));
    vm_value_t *array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

    for (int i = 0; i < *length; i++) {
        array[i] = (vm_value_t) { .type = VM_TYPE_INT, .int_value = start + i * step };
    }

    arrayval.pointer_value = reserved_mem;

    *(stack - 1) = arrayval;

    AJS_STACK(-1);
}

void arr_compare(CPU_State *state, Instruction_Implementation compare_instr) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_ARRAY, "right operand is not an array");
    vm_assert(state, (stack - 1)->type == VM_TYPE_ARRAY, "left operand is not an array");

    vm_type_t *a_reserved_mem = vm_pointer_to_native(state->memory, (stack - 1)->pointer_value, vm_type_t*);
    vm_type_t *a_len = a_reserved_mem + 1;
    vm_pointer_t *a_array_ptr = a_reserved_mem + 2;
    vm_value_t *a_array = vm_pointer_to_native(state->memory, *a_array_ptr, vm_value_t*);

    vm_type_t *b_reserved_mem = vm_pointer_to_native(state->memory, (stack)->pointer_value, vm_type_t*);
    vm_type_t *b_len = b_reserved_mem + 1;
    vm_pointer_t *b_array_ptr = b_reserved_mem + 2;
    vm_value_t *b_array = vm_pointer_to_native(state->memory, *b_array_ptr, vm_value_t*);

    vm_type_t eq = 1;

    if (*a_len != *b_len) {
        eq = 0;
    } else {
        for (int i = 0; i < *a_len; i++) {
            *(stack + 1) = a_array[i];
            *(stack + 2) = b_array[i];
            retain(state, &a_array[i]);
            retain(state, &b_array[i]);
            AJS_STACK(+2);
            compare_instr(state);
            vm_type_signed_t res = (stack + 1)->int_value;
            AJS_STACK(-1);
            if (!res) {
                eq = 0;
                break;
            }
        }
    }

    release(state, stack);
    release(state, stack - 1);

    (stack - 1)->type = VM_TYPE_INT;
    (stack - 1)->int_value = eq;
    AJS_STACK(-1);
}

void arr_eq(CPU_State *state) {
    arr_compare(state, instr_eq);
}

void arr_ne(CPU_State *state) {
    arr_compare(state, instr_ne);
}

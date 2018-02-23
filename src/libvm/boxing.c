#include <assert.h>
#include "funkyvm/funkyvm.h"
#include "instructions/instructions.h"
#include "boxing.h"
#include "error_handling.h"

/*
 * Boxing promotes a native type (int, uint, float, string, array, map) to an object that contains the value.
 * Unboxing unwraps that value back again.
 */

vm_pointer_t create_ptype(CPU_State *state) {
    vm_pointer_t reserved_mem     = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for first item
    vm_type_t *ref_count          = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_pointer_t *first_ptr       = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 1;
    vm_pointer_t *prototype_ptr   = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    *ref_count = VM_UNSIGNED_MAX;
    *first_ptr = 0;
    *prototype_ptr = 0;

    return reserved_mem;
}

void initialize_boxing_prototypes(CPU_State *state) {
    state->boxing.proto_int = create_ptype(state);
    state->boxing.proto_uint = create_ptype(state);
    state->boxing.proto_float = create_ptype(state);
    state->boxing.proto_string = create_ptype(state);
    state->boxing.proto_array = create_ptype(state);
    state->boxing.proto_map = create_ptype(state);
}

void destroy_boxing_prototypes(CPU_State *state) {
    map_release(state, state->boxing.proto_int);
    vm_free(state->memory, state->boxing.proto_int);

    map_release(state, state->boxing.proto_uint);
    vm_free(state->memory, state->boxing.proto_uint);

    map_release(state, state->boxing.proto_float);
    vm_free(state->memory, state->boxing.proto_float);

    map_release(state, state->boxing.proto_string);
    vm_free(state->memory, state->boxing.proto_string);

    map_release(state, state->boxing.proto_array);
    vm_free(state->memory, state->boxing.proto_array);

    map_release(state, state->boxing.proto_map);
    vm_free(state->memory, state->boxing.proto_map);
}

INSTR(box) {
    vm_pointer_t reserved_mem     = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for first item
    vm_type_t *ref_count          = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_pointer_t *first_ptr       = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 1;
    vm_pointer_t *prototype_ptr   = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    *ref_count = 1;
    *first_ptr = 0;
    *prototype_ptr = 0;

    USE_STACK();

    vm_value_t unboxed_value = *stack;

    if (stack->type == VM_TYPE_INT) *prototype_ptr = state->boxing.proto_int;
    else if (stack->type == VM_TYPE_UINT) *prototype_ptr = state->boxing.proto_uint;
    else if (stack->type == VM_TYPE_FLOAT) *prototype_ptr = state->boxing.proto_float;
    else if (stack->type == VM_TYPE_STRING) *prototype_ptr = state->boxing.proto_string;
    else if (stack->type == VM_TYPE_ARRAY) *prototype_ptr = state->boxing.proto_array;
    else if (stack->type == VM_TYPE_MAP) *prototype_ptr = state->boxing.proto_map;
    else {
        vm_error(state, "Can't box type %d", stack->type);
        vm_exit(state, EXIT_FAILURE);
    }

    st_mapitem(state, reserved_mem, "value", &unboxed_value);

    *stack = (vm_value_t) { .type = VM_TYPE_MAP, .pointer_value = reserved_mem };
}

INSTR(unbox) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_MAP, "value is not an unboxable type");
    vm_map_elem_t *box_value = ld_mapitem(state, stack->pointer_value, "value");
    if (box_value == NULL) {
        vm_error(state, "Value is not a boxed type");
        vm_exit(state, EXIT_FAILURE);
    }
    vm_value_t unboxed = box_value->value;
    release(state, stack);
    *stack = unboxed;
}

INSTR(ld_boxingproto) {
    vm_type_signed_t type = GET_OPERAND_SIGNED();
    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) { .type = VM_TYPE_MAP };
    if (type == VM_TYPE_INT) stack->pointer_value = state->boxing.proto_int;
    else if (type == VM_TYPE_UINT) stack->pointer_value = state->boxing.proto_uint;
    else if (type == VM_TYPE_FLOAT) stack->pointer_value = state->boxing.proto_float;
    else if (type == VM_TYPE_STRING) stack->pointer_value = state->boxing.proto_string;
    else if (type == VM_TYPE_ARRAY) stack->pointer_value = state->boxing.proto_array;
    else if (type == VM_TYPE_MAP) stack->pointer_value = state->boxing.proto_map;
    else {
        vm_error(state, "Can't get boxing prototype for type %d", type);
        vm_exit(state, EXIT_FAILURE);
    }
}
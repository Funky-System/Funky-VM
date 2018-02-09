#include <assert.h>
#include "vm.h"
#include "instructions.h"
#include "boxing.h"

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
        fprintf(stderr, "Can't box type %d\n", stack->type);
        exit(EXIT_FAILURE);
    }

    st_mapitem(state, reserved_mem, "@value", &unboxed_value);

    *stack = (vm_value_t) { .type = VM_TYPE_MAP, .pointer_value = reserved_mem };
}

INSTR(unbox) {
    USE_STACK();
    assert(stack->type == VM_TYPE_MAP);
    vm_map_elem_t *box_value = ld_mapitem(state, stack->pointer_value, "@value");
    if (box_value == NULL) {
        fprintf(stderr, "Value is not a boxed type\n");
        exit(EXIT_FAILURE);
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
        fprintf(stderr, "Can't get boxing prototype for type %d\n", type);
        exit(EXIT_FAILURE);
    }
}
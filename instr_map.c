#include <memory.h>
#include <assert.h>
#include "instructions.h"
#include "vm.h"
#include "error_handling.h"

INSTR(ld_map) {
    USE_STACK();

    vm_value_t mapval;
    mapval.type = VM_TYPE_MAP;

    vm_pointer_t reserved_mem     = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for first item
    vm_type_t *ref_count          = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_pointer_t *first_ptr       = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 1;
    vm_pointer_t *prototype_ptr   = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    *ref_count = 1;
    *first_ptr = 0;
    *prototype_ptr = 0;

    mapval.pointer_value = reserved_mem;

    *(stack + 1) = mapval;

    AJS_STACK(+1);
}

vm_map_elem_t* ld_mapitem(CPU_State *state, vm_pointer_t map_ptr, const char* name) {
    vm_pointer_t *first_ptr = vm_pointer_to_native(state->memory, map_ptr, vm_pointer_t*) + 1;
    vm_pointer_t *prototype_ptr = vm_pointer_to_native(state->memory, map_ptr, vm_pointer_t*) + 2;

    if (*first_ptr == 0) {
        if (*prototype_ptr == 0) {
            return NULL;
        } else {
            return ld_mapitem(state, *prototype_ptr, name);
        }
    } else {
        vm_map_elem_t *item = vm_pointer_to_native(state->memory, *first_ptr, vm_map_elem_t*);
        while (1) {
            if (strcmp(name, vm_pointer_to_native(state->memory, item->name, char*)) == 0) {
                return item;
            }
            if (item->next == 0) break;
            item = vm_pointer_to_native(state->memory, item->next, vm_map_elem_t*);
        }

        if (*prototype_ptr == 0) {
            return NULL;
        } else {
            return ld_mapitem(state, *prototype_ptr, name);
        }
    }
}


INSTR(ld_mapitem) {
    USE_STACK();
    if (stack->type != VM_TYPE_MAP) {
        instr_box(state);
        //vm_error(state, "Error: Not a map type\n");
        //exit(EXIT_FAILURE);
    }
    vm_pointer_t name_ptr = get_current_module(state)->addr + GET_OPERAND();
    const char *name = cstr_pointer_from_vm_pointer_t(state, name_ptr) + sizeof(vm_type_t);
    vm_value_t val;

    vm_map_elem_t *elem = ld_mapitem(state, stack->pointer_value, name);
    if (elem == NULL) {
        val.type = VM_TYPE_EMPTY;
    } else {
        val = elem->value;
        retain(state, &val);
    }

    release(state, stack); // release the map
    *stack = val;
}

INSTR(ld_mapitem_pop) {
    USE_STACK();
    vm_assert(state, (stack - 1)->type == VM_TYPE_MAP, "value is not a map type");
    vm_assert(state, stack->type == VM_TYPE_STRING, "map reference is not of string type");
    const char *name = cstr_pointer_from_vm_value(state, stack);

    vm_value_t val;

    vm_map_elem_t *elem = ld_mapitem(state, stack->pointer_value, name);
    if (elem == NULL) {
        val.type = VM_TYPE_EMPTY;
    } else {
        val = elem->value;
        retain(state, &val);
    }

    release(state, stack - 1); // release the map
    release(state, stack); // release the name
    *(stack - 1) = val;
    AJS_STACK(-1);
}

void st_mapitem(CPU_State *state, vm_pointer_t map_ptr, const char* name, vm_value_t* value) {
    vm_pointer_t *first_ptr = vm_pointer_to_native(state->memory, map_ptr, vm_pointer_t*) + 1;

    if (*first_ptr == 0) {
        vm_pointer_t name_ptr = vm_malloc(state->memory, strlen(name) + 1);
        strcpy(vm_pointer_to_native(state->memory, name_ptr, char*), name);
        vm_pointer_t mem = vm_malloc(state->memory, sizeof(vm_map_elem_t));
        vm_map_elem_t *elem = vm_pointer_to_native(state->memory, mem, vm_map_elem_t*);
        elem->name = name_ptr;
        elem->value = *value;

        elem->next = 0;
        elem->prev = 0;
        *first_ptr = mem;
        return;
    }

    vm_map_elem_t *item = vm_pointer_to_native(state->memory, *first_ptr, vm_map_elem_t*);
    while (1) {
        if (strcmp(name, vm_pointer_to_native(state->memory, item->name, char*)) == 0) {
            vm_value_t oldval = item->value;
            item->value = *value;
            release(state, &oldval);
            return;
        }
        if (item->next == 0) break;
        item = vm_pointer_to_native(state->memory, item->next, vm_map_elem_t*);
    }

    // If we're here, this is a new item
    vm_pointer_t name_ptr = vm_malloc(state->memory, strlen(name) + 1);
    strcpy(vm_pointer_to_native(state->memory, name_ptr, char*), name);
    vm_pointer_t mem = vm_malloc(state->memory, sizeof(vm_map_elem_t));
    vm_map_elem_t *elem = vm_pointer_to_native(state->memory, mem, vm_map_elem_t*);
    elem->name = name_ptr;
    elem->value = *value;

    elem->next = *first_ptr;
    elem->prev = 0;
    vm_map_elem_t *next_elem = vm_pointer_to_native(state->memory, *first_ptr, vm_map_elem_t*);
    next_elem->prev = mem;
    *first_ptr = mem;
}

INSTR(st_mapitem) {
    USE_STACK();
    vm_assert(state, (stack)->type == VM_TYPE_MAP, "value is not a map type");
    vm_pointer_t name_ptr = get_current_module(state)->addr + GET_OPERAND() + sizeof(vm_type_t);
    const char *name = cstr_pointer_from_vm_pointer_t(state, name_ptr);
    st_mapitem(state, stack->pointer_value, name, stack - 1);
    release(state, stack); // release the map
    // no release/retain for value, as it is reduced by one because of stack pop, but added by one because of array storage
    AJS_STACK(-2);
}

INSTR(st_mapitem_pop) {
    USE_STACK();
    vm_assert(state, (stack - 1)->type == VM_TYPE_MAP, "value is not a map type");
    vm_assert(state, (stack)->type == VM_TYPE_STRING, "map reference is not of string type");
    const char *name = cstr_pointer_from_vm_value(state, stack);
    st_mapitem(state, (stack - 1)->pointer_value, name, stack - 2);
    release(state, stack - 1); // release the map
    release(state, stack); // release the name
    // no release/retain for value, as it is reduced by one because of stack pop, but added by one because of array storage
    AJS_STACK(-3);
}

void del_mapitem(CPU_State *state, vm_pointer_t map_ptr, const char* name) {
    vm_pointer_t *first_ptr = vm_pointer_to_native(state->memory, map_ptr, vm_pointer_t*) + 1;

    if (*first_ptr == 0) {
        vm_error(state, "map is empty");
        vm_exit(state, EXIT_FAILURE);
    }

    vm_map_elem_t *item = vm_pointer_to_native(state->memory, *first_ptr, vm_map_elem_t*);
    while (1) {
        if (strcmp(name, vm_pointer_to_native(state->memory, item->name, char*)) == 0) {
            if (item->prev) {
                vm_map_elem_t *prev = vm_pointer_to_native(state->memory, item->prev, vm_map_elem_t*);
                prev->next = item->next;
            }
            if (item->next) {
                vm_map_elem_t *next = vm_pointer_to_native(state->memory, item->next, vm_map_elem_t*);
                next->prev = item->prev;
            }
            if (*first_ptr == native_to_vm_pointer(state->memory, item)) {
                *first_ptr = item->next;
            }
            release(state, &item->value);
            vm_free(state->memory, item->name);
            k_free(state->memory, item); // use k_free because we only have a native pointer at this moment
            return;
        }
        if (item->next == 0) break;
        item = vm_pointer_to_native(state->memory, item->next, vm_map_elem_t*);
    }

    // If we're here, this is a new item
    vm_error(state, "map does not contain element with key '%s'", name);
    vm_exit(state, EXIT_FAILURE);
}

INSTR(del_mapitem) {
    USE_STACK();
    vm_assert(state, (stack)->type == VM_TYPE_MAP, "value is not a map type");
    vm_pointer_t name_ptr = get_current_module(state)->addr + GET_OPERAND() + sizeof(vm_type_t);
    const char *name = cstr_pointer_from_vm_pointer_t(state, name_ptr);
    del_mapitem(state, stack->pointer_value, name);
    release(state, stack); // release the map
    AJS_STACK(-1);
}

INSTR(del_mapitem_pop) {
    USE_STACK();
    vm_assert(state, (stack - 1)->type == VM_TYPE_MAP, "value is not a map type");
    vm_assert(state, (stack)->type == VM_TYPE_STRING, "map reference is not of string type");
    const char *name = cstr_pointer_from_vm_value(state, stack);
    del_mapitem(state, (stack - 1)->pointer_value, name);
    release(state, stack - 1); // release the map
    release(state, stack); // release the name
    // no release/retain for value, as it is reduced by one because of stack pop, but added by one because of array storage
    AJS_STACK(-2);
}

vm_type_t map_contains_key(CPU_State *state, vm_pointer_t map_ptr, const char* name) {
    vm_pointer_t *item_ptr = vm_pointer_to_native(state->memory, map_ptr, vm_pointer_t*) + 1;

    if (*item_ptr == 0) return 0;

    vm_map_elem_t *item = vm_pointer_to_native(state->memory, *item_ptr, vm_map_elem_t*);

    while (item != 0) {
        if (strcmp(name, vm_pointer_to_native(state->memory, item->name, const char*)) == 0) {
            return 1;
        }
        if (item->next == 0) break;
        item = vm_pointer_to_native(state->memory, item->next, vm_map_elem_t*);
    }

    return 0;
}

INSTR(has_mapitem) {
    USE_STACK();
    vm_assert(state, (stack)->type == VM_TYPE_MAP, "value is not a map type");

    vm_pointer_t name_ptr = get_current_module(state)->addr + GET_OPERAND() + sizeof(vm_type_t);
    const char *name = cstr_pointer_from_vm_pointer_t(state, name_ptr);

    vm_type_t contains = map_contains_key(state, stack->pointer_value, name);
    release(state, stack); // release the map

    stack->type = VM_TYPE_UINT;
    stack->uint_value = contains;
}

INSTR(has_mapitem_pop) {
    USE_STACK();
    vm_assert(state, (stack - 1)->type == VM_TYPE_MAP, "value is not a map type");
    vm_assert(state, (stack)->type == VM_TYPE_STRING, "map reference is not of string type");
    const char *name = cstr_pointer_from_vm_value(state, stack);

    vm_type_t contains = map_contains_key(state, (stack - 1)->pointer_value, name);
    release(state, stack); // release the string
    release(state, stack - 1); // release the map

    (stack - 1)->type = VM_TYPE_UINT;
    (stack - 1)->uint_value = contains;
    AJS_STACK(-1);
}

INSTR(map_copy) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_MAP, "value is not a map type");

    vm_value_t mapval;
    mapval.type = VM_TYPE_MAP;

    vm_pointer_t reserved_mem     = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for first item
    vm_type_t *ref_count          = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_pointer_t *first_ptr       = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 1;
    vm_pointer_t *prototype_ptr   = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    *ref_count = 1;
    *first_ptr = 0;

    mapval.pointer_value = reserved_mem;

    *prototype_ptr = *(vm_pointer_to_native(state->memory, stack->pointer_value, vm_pointer_t*) + 2);

    vm_pointer_t *item_ptr = vm_pointer_to_native(state->memory, stack->pointer_value, vm_pointer_t*) + 1;

    if (*item_ptr != 0) {
        vm_map_elem_t *item = vm_pointer_to_native(state->memory, *item_ptr, vm_map_elem_t*);

        while (1) {
            const char *name = cstr_pointer_from_vm_pointer_t(state, item->name);
            st_mapitem(state, reserved_mem, name, &item->value);
            retain(state, &item->value);
            if (item->next == 0) break;
            item = vm_pointer_to_native(state->memory, item->next, vm_map_elem_t*);
        }
    }

    release(state, stack); // release original map

    *stack = mapval;
}

INSTR(map_len) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_MAP, "value is not a map type");

    vm_pointer_t *item_ptr = vm_pointer_to_native(state->memory, stack->pointer_value, vm_pointer_t*) + 1;
    vm_type_t len = 0;

    if (*item_ptr != 0) {
        vm_map_elem_t *item = vm_pointer_to_native(state->memory, *item_ptr, vm_map_elem_t*);

        while (1) {
            len++;

            if (item->next == 0) break;
            item = vm_pointer_to_native(state->memory, item->next, vm_map_elem_t*);
        }
    }

    release(state, stack); // release original map

    *stack = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = len };
}

INSTR(map_merge) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_MAP, "right operand value is not a map type");
    vm_assert(state, (stack - 1)->type == VM_TYPE_MAP, "left operand value is not a map type");

    vm_value_t mapval;
    mapval.type = VM_TYPE_MAP;

    vm_pointer_t reserved_mem   = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for first item
    vm_type_t *ref_count        = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_pointer_t *first_ptr     = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 1;
    vm_pointer_t *prototype_ptr = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    *ref_count = 1;
    *first_ptr = 0;
    *prototype_ptr = 0;

    mapval.pointer_value = reserved_mem;

    vm_pointer_t *item_ptr = vm_pointer_to_native(state->memory, (stack - 1)->pointer_value, vm_pointer_t*) + 1;
    if (*item_ptr != 0) {
        vm_map_elem_t *item = vm_pointer_to_native(state->memory, *item_ptr, vm_map_elem_t*);

        while (1) {
            const char *name = cstr_pointer_from_vm_pointer_t(state, item->name);
            st_mapitem(state, reserved_mem, name, &item->value);
            retain(state, &item->value);
            if (item->next == 0) break;
            item = vm_pointer_to_native(state->memory, item->next, vm_map_elem_t*);
        }
    }

    item_ptr = vm_pointer_to_native(state->memory, stack->pointer_value, vm_pointer_t*) + 1;
    if (*item_ptr != 0) {
        vm_map_elem_t *item = vm_pointer_to_native(state->memory, *item_ptr, vm_map_elem_t*);

        while (1) {
            const char *name = cstr_pointer_from_vm_pointer_t(state, item->name);
            st_mapitem(state, reserved_mem, name, &item->value);
            retain(state, &item->value);
            if (item->next == 0) break;
            item = vm_pointer_to_native(state->memory, item->next, vm_map_elem_t*);
        }
    }

    release(state, stack - 1); // release original map
    release(state, stack); // release original map

    *(stack - 1) = mapval;
    AJS_STACK(-1);
}

void map_release(CPU_State* state, vm_pointer_t ptr) {
    vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, ptr, vm_type_t*);
    vm_pointer_t *first_ptr = reserved_mem + 1;
    vm_pointer_t *prototype_ptr = reserved_mem + 2;
    vm_map_elem_t *elem = vm_pointer_to_native(state->memory, *first_ptr, vm_map_elem_t*);

    while (1) {
        vm_free(state->memory, elem->name);
        release(state, &elem->value);
        vm_pointer_t next = elem->next;
        k_free(state->memory, elem);
        if (next == 0) break;
        elem = vm_pointer_to_native(state->memory, next, vm_map_elem_t*);
    }

    if (*prototype_ptr != 0) {
        release_pointer(state, VM_TYPE_MAP, *prototype_ptr);
    }
}

INSTR(map_setprototype) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_MAP, "value is not a map type"); // the map
    vm_assert(state, (stack - 1)->type == VM_TYPE_MAP, "value is not a map type"); // the prototype

    vm_pointer_t *prototype_ptr = vm_pointer_to_native(state->memory, stack->pointer_value, vm_pointer_t*) + 2;

    if (*prototype_ptr != 0 && *prototype_ptr != (stack - 1)->pointer_value) {
        release_pointer(state, VM_TYPE_MAP, *prototype_ptr);
    }

    *prototype_ptr = (stack - 1)->pointer_value;

    // do not release/retain prototype, as it stays equal
    release(state, stack);

    AJS_STACK(-2);
}

INSTR(map_getprototype) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_MAP, "value is not a map type"); // the map

    vm_pointer_t *prototype_ptr = vm_pointer_to_native(state->memory, stack->pointer_value, vm_pointer_t*) + 2;

    if (*prototype_ptr == 0) {
        release(state, stack);
        *stack = (vm_value_t) { .type = VM_TYPE_EMPTY };
    } else {
        retain_pointer(state, VM_TYPE_MAP, *prototype_ptr);
        release(state, stack);
        *stack = (vm_value_t) { .type = VM_TYPE_MAP, .pointer_value = *prototype_ptr };
    }
}

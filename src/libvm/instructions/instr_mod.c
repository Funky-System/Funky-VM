//
// Created by Bas du Pré on 01-02-18.
//

#include <string.h>

#include "instructions.h"
#include "funkyvm/funkyvm.h"
#include "../boxing.h"

void link(CPU_State *state, const char* name) {
    Module *existing = module_get(state, name);
    Module *module = calloc(1, sizeof(Module));
    Module *orig_module = module;
    if (existing != NULL) {
        if (existing->ref_map != 0) {
            AJS_STACK(+1);
            USE_STACK();
            *stack = (vm_value_t) {.type = VM_TYPE_MAP, .pointer_value = existing->ref_map};
            free(orig_module);
            existing->num_links++;
            return;
        }
        module = existing;
    } else {
        *module = module_load_name(state, name);
    }

    vm_pointer_t reserved_mem     = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for first item
    vm_type_t *ref_count          = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_pointer_t *first_ptr       = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 1;
    vm_pointer_t *prototype_ptr   = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    *ref_count = VM_UNSIGNED_MAX;
    *first_ptr = 0;
    *prototype_ptr = 0;

    char *addr = vm_pointer_to_native(state->memory, module->addr, char*);
    int num_found = 0;
    while (num_found < module->num_exports) {
        const char *curname = addr;
        while (*(addr++) != '\0') {}
        vm_type_t ref = *((vm_type_t *) addr);
        vm_value_t refval = (vm_value_t) {.type = VM_TYPE_REF, .pointer_value = module->addr + ref};
        st_mapitem(state, reserved_mem, curname, &refval);

        num_found++;
        addr += sizeof(vm_type_t);
    }

    vm_value_t refval = (vm_value_t) {.type = VM_TYPE_REF, .pointer_value = module->addr + module->start_of_code};
    st_mapitem(state, reserved_mem, "@init", &refval);

    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) {.type = VM_TYPE_MAP, .pointer_value = reserved_mem};

    module->ref_map = reserved_mem;

    if (!existing) {
        module->num_links = 1;
        module_register(state, *module);
    } else {
        existing->num_links++;
    }

    free(orig_module);
}

/**!
 * instruction: link
 * category: modules
 * opcode: "0x04"
 * description: Loads a funk module into memory and pushes a map with named references.
 * extra_info: If the name does not end in <code>.funk</code>, the VM adds it.
 * operands:
 *   - type: string
 *     description: Name of module
 * stack_post:
 *   - type: map
 *     description: Named references that have been exported from module
 */
INSTR(link) {
    char *name = (char*)state->memory->main_memory + get_current_module(state)->addr + GET_OPERAND() + sizeof(vm_type_t);

    link(state, name);
}

INSTR(link_pop) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_STRING, "map reference is not of string type");
    const char *name = cstr_pointer_from_vm_value(state, stack);
    AJS_STACK(-1);
    link(state, name);
}

void unlink(CPU_State *state, const char *name) {
    Module *existing = module_get(state, name);
    if (existing != NULL) {
        if (existing->num_links > 1) {
            existing->num_links--;
        } else {
            module_release(state, name);
            if (existing->ref_map != 0) {
                vm_type_t *ref_count = vm_pointer_to_native(state->memory, existing->ref_map, vm_type_t*);
                *ref_count = 1;
                release_pointer(state, VM_TYPE_MAP, existing->ref_map);
            }
            module_unload(state->memory, existing);
        }
    } else {
        vm_error(state, "Error: Module not loaded: %s\n", name);
        vm_exit(state, EXIT_FAILURE);
    }
}

INSTR(unlink) {
    char *name = (char*)state->memory->main_memory + get_current_module(state)->addr + GET_OPERAND() + sizeof(vm_type_t);
    unlink(state, name);
}

INSTR(unlink_pop) {
    USE_STACK();
    vm_assert(state, stack->type == VM_TYPE_STRING, "map reference is not of string type");
    const char *name = cstr_pointer_from_vm_value(state, stack);
    AJS_STACK(-1);
    unlink(state, name);
}

/**!
 * instruction: ld.extern
 * category: modules
 * opcode: "0x90"
 * description: Load a reference to an exported symbol from a module.
 * extra_info: If the name does not end in <code>.funk</code>, the VM adds it.
 * operands:
 *   - type: string
 *     description: Name of module
 *   - type: string
 *     description: Name of symbol
 * stack_post:
 *   - type: reference
 *     description: Reference to symbol
 */
INSTR(ld_extern) {
    char *module = (char*)state->memory->main_memory + get_current_module(state)->addr + GET_OPERAND() + sizeof(vm_type_t);
    char *name = (char*)state->memory->main_memory + get_current_module(state)->addr + GET_OPERAND() + sizeof(vm_type_t);

    for (int i = 0; i < state->num_modules; i++) {
        if (strcmp(state->modules[i].name, module) == 0) {
            char *addr = (char *) state->memory->main_memory + state->modules[i].addr;
            int num_found = 0;
            while (num_found < state->modules[i].num_exports) {
                char c, *curname = addr;
                while (*(addr++) != '\0') {}
                vm_type_t ref = *((vm_type_t *) addr);
                addr += sizeof(vm_type_t);
                if (strcmp(name, curname) == 0) {
                    AJS_STACK(+1);
                    USE_STACK();
                    stack->uint_value = ref + state->modules[i].addr;
                    stack->type = VM_TYPE_REF;
                }
                num_found++;
            }
        }
    }
}
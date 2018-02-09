//
// Created by Bas du Pré on 01-02-18.
//

#include <strings.h>

#include "instructions.h"
#include "modules.h"
#include "memory.h"
#include "vm.h"
#include "boxing.h"

INSTR(link) {
    char *name = (char*)state->memory->main_memory + get_current_module(state)->addr + GET_OPERAND() + sizeof(vm_type_t);
    Module module = module_load(state->memory, name);
    module_register(state, module);

    vm_pointer_t reserved_mem     = vm_malloc(state->memory, sizeof(vm_type_t) * 3); // first for refcount, second for first item
    vm_type_t *ref_count          = vm_pointer_to_native(state->memory, reserved_mem, vm_type_t*);
    vm_pointer_t *first_ptr       = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 1;
    vm_pointer_t *prototype_ptr   = vm_pointer_to_native(state->memory, reserved_mem, vm_pointer_t*) + 2;

    *ref_count = VM_UNSIGNED_MAX;
    *first_ptr = 0;
    *prototype_ptr = 0;

    char *addr = vm_pointer_to_native(state->memory, module.addr, char*);
    int num_found = 0;
    while (num_found < module.num_exports) {
        const char *curname = addr;
        while (*(addr++) != '\0') {}
        vm_type_t ref = *((vm_type_t *) addr);
        vm_value_t refval = (vm_value_t) {.type = VM_TYPE_REF, .pointer_value = module.addr + ref};
        st_mapitem(state, reserved_mem, curname, &refval);

        num_found++;
        addr += sizeof(vm_type_t);
    }

    vm_value_t refval = (vm_value_t) {.type = VM_TYPE_REF, .pointer_value = module.addr + module.start_of_code};
    st_mapitem(state, reserved_mem, "@init", &refval);

    AJS_STACK(+1);
    USE_STACK();
    *stack = (vm_value_t) {.type = VM_TYPE_MAP, .pointer_value = reserved_mem};
}

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
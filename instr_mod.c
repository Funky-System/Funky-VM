//
// Created by Bas du Pré on 01-02-18.
//

#include <strings.h>

#include "instructions.h"
#include "modules.h"
#include "memory.h"
#include "vm.h"

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
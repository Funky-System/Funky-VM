#include <string.h>
#include "instructions.h"

int register_syscall(CPU_State* state, const char* name, vm_syscall_t fn) {
    state->num_syscalls++;
    state->syscall_table = k_realloc(state->memory, state->syscall_table, sizeof(vm_syscall_table_t) * state->num_syscalls);
    state->syscall_table[state->num_syscalls - 1].name = name;
    state->syscall_table[state->num_syscalls - 1].fn = fn;
    return state->num_syscalls;
}

int release_syscall(CPU_State* state, const char* name) {
    for (int i = 0; i < state->num_syscalls; i++) {
        if (strcmp(state->syscall_table[i].name, name) == 0) {
            for (int j = i + 1; j < state->num_modules; j++) {
                state->modules[j - 1] = state->modules[j];
            }
            state->num_modules--;
            state->modules = k_realloc(state->memory, state->modules, sizeof(Module) * state->num_modules);
            return 1;
        }
    }

    return 0;
}

INSTR(syscall) {
    state->syscall_table[GET_OPERAND()].fn(state);
}

INSTR(syscall_getindex) {
    char *name = (char*)state->memory->main_memory + get_current_module(state)->addr + GET_OPERAND() + sizeof(vm_type_t);

    AJS_STACK(+1);
    USE_STACK();

    for (int i = 0; i < state->num_syscalls; i++) {
        if (strcmp(state->syscall_table[i].name, name) == 0) {
            *stack = (vm_value_t) {
                    .type = VM_TYPE_INT,
                    .int_value = i
            };
            return;
        }
    }

    *stack = (vm_value_t) {
            .type = VM_TYPE_INT,
            .int_value = -1
    };
}
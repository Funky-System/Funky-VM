//
// Created by Bas du Pré on 10-02-18.
//

#ifndef FUNKY_VM_SYSCALL_H
#define FUNKY_VM_SYSCALL_H

#include "funkyvm.h"

#define STACK_VALUE(STATE, i) ((vm_value_t *)(STATE->memory->main_memory + STATE->sp + i * sizeof(vm_value_t)))

typedef void (*vm_syscall_t)(CPU_State *state);

typedef struct vm_syscall_table_t {
    const char* name;
    vm_syscall_t fn;
} vm_syscall_table_t;

int register_syscall(CPU_State* state, const char* name, vm_syscall_t fn);
int release_syscall(CPU_State* state, const char* name);

#endif //FUNKY_VM_SYSCALL_H

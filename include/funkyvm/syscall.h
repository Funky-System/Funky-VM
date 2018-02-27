//
// Created by Bas du Pré on 10-02-18.
//

#ifndef FUNKY_VM_SYSCALL_H
#define FUNKY_VM_SYSCALL_H

#include "funkyvm.h"

#define STACK_VALUE(STATE, i) ((vm_value_t *)(STATE->memory->main_memory + STATE->sp + i * sizeof(vm_value_t)))

#define VM_RETURN_INT(STATE, VAL) { \
    STATE->rr = (vm_value_t) { .type = VM_TYPE_INT, .int_value = VAL }; \
    return; \
}
#define VM_RETURN_UINT(STATE, VAL) { \
    STATE->rr = (vm_value_t) { .type = VM_TYPE_UINT, .uint_value = VAL }; \
    return; \
}
#define VM_RETURN_FLOAT(STATE, VAL) { \
    STATE->rr = (vm_value_t) { .type = VM_TYPE_FLOAT, .float_value = VAL }; \
    return; \
}
#define VM_RETURN_STRING(STATE, VAL) { \
    STATE->rr = (vm_value_t) { .type = VM_TYPE_STRING, .pointer_value = vm_create_string(STATE, VAL) }; \
    return; \
}

typedef void (*vm_syscall_t)(CPU_State *state);

typedef __attribute__((aligned(1))) struct vm_syscall_table_t {
    const char* name;
    vm_syscall_t fn;
} vm_syscall_table_t;

int register_syscall(CPU_State* state, const char* name, vm_syscall_t fn);
int release_syscall(CPU_State* state, const char* name);

vm_value_t vm_create_string(CPU_State* state, const char* c_str);

#endif //FUNKY_VM_SYSCALL_H

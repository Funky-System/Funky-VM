//
// Created by Bas du Pré on 12-02-18.
//

#include <stdio.h>

#include "bindings.h"

void print(CPU_State *state) {
    const char* str = cstr_pointer_from_vm_value(state, STACK_VALUE(state, 0));
    printf("%s", str);
}

void register_bindings(CPU_State *state) {
    register_syscall(state, "print", print);
}

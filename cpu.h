#ifndef PROCESSOR_CPU_H
#define PROCESSOR_CPU_H

typedef struct CPU_State CPU_State;

#include <stdint.h>
#include "vm.h"
#include "memory.h"
#include "modules.h"

typedef struct Debug_Context {
    const char* filename;
    int line;
    int col;

    const char** stacktrace;
    int num_stacktrace;
} Debug_Context;

typedef struct CPU_State {
    // registers
    vm_type_t pc, sp, mp, ap;
    vm_value_t rr, r0, r1, r2, r3, r4, r5, r6, r7;
    vm_type_t stack_base;

    // state
    int running;

    Memory* memory;

    Module* modules;
    vm_type_t num_modules;

    Debug_Context debug_context;

} CPU_State;

CPU_State cpu_init(Memory* memory);
void cpu_set_entry_to_module(CPU_State *state, Module *mod);
vm_type_t cpu_run(CPU_State *state);

#endif //PROCESSOR_CPU_H

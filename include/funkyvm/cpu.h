#ifndef PROCESSOR_CPU_H
#define PROCESSOR_CPU_H

#include "os.h"
#ifdef FUNKY_VM_OS_EMSCRIPTEN
#pragma pack(1)
#endif

typedef struct CPU_State CPU_State;

#include <stdint.h>
#include "funkyvm.h"
#include "memory.h"
#include "modules.h"
#include "syscall.h"

typedef struct Stacktrace_Frame {
    const char* name;
    const char* filename;
    int line;
    int col;
} Stacktrace_Frame;

typedef struct Debug_Context {
    const char* filename;
    int line;
    int col;

    Stacktrace_Frame* stacktrace;
    int num_stacktrace;
} Debug_Context;

typedef struct CPU_State {
    // registers
    vm_type_t pc, sp, mp, ap;
    vm_value_t rr, r0, r1, r2, r3, r4, r5, r6, r7;
    vm_type_t stack_base;

    // state
    int running;
    int in_error_state;

    Memory* memory;

    Module* modules;
    vm_type_t num_modules;
    char** module_paths;
    vm_type_t num_module_paths;

    vm_syscall_table_t* syscall_table;
    vm_type_t num_syscalls;

    #ifdef FUNKY_VM_OS_EMSCRIPTEN
    int emscripten_yield;
    #endif

    struct boxing {
        vm_pointer_t proto_int;
        vm_pointer_t proto_uint;
        vm_pointer_t proto_float;
        vm_pointer_t proto_string;
        vm_pointer_t proto_array;
        vm_pointer_t proto_map;
    } boxing;

    Debug_Context debug_context;

} CPU_State;

CPU_State cpu_init(Memory* memory);
void cpu_destroy(CPU_State *state);
void cpu_set_entry_to_module(CPU_State *state, Module *mod);
vm_type_t cpu_run(CPU_State *state);

#ifdef FUNKY_VM_OS_EMSCRIPTEN
void cpu_emscripten_yield(CPU_State *state);
#endif

#endif //PROCESSOR_CPU_H

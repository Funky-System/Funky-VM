//
// Created by Bas du Pré on 01-02-18.
//

#ifndef VM_MODULES_H
#define VM_MODULES_H

typedef struct Module Module;

#include "funkyvm.h"
#include "memory.h"
#include "cpu.h"

typedef struct Module {
    char* name;
    vm_pointer_t addr;
    vm_type_t num_exports;
    vm_type_t start_of_code;
    vm_type_t size;
    vm_pointer_t ref_map;
} Module;

Module module_load_name(Memory *mem, const char* name);
Module module_load(Memory *mem, const char* name, funky_bytecode_t bc);
void module_unload(Memory *mem, Module* module);
int module_register(CPU_State *state, Module module);
int module_release(CPU_State *state, const char* name);
Module *module_get(CPU_State *state, const char* name);

Module* get_current_module(CPU_State *state);

#endif //VM_MODULES_H

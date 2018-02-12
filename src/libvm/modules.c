//
// Created by Bas du Pré on 01-02-18.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "funkyvm/modules.h"
#include "funkyvm/memory.h"
#include "funkyvm/cpu.h"
#include "instructions/instructions.h"
#include "error_handling.h"

#define IS_BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})
#define FLAG_LITTLE_ENDIAN 1

Module module_load(Memory *mem, const char* name) {
    Module module;

    char* filename = malloc(strlen(name) + 1 + 5);
    strcpy(filename, name);
    strcat(filename, ".funk");

    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        int errnum = errno;
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        vm_error(NULL, "%s", strerror(errnum));
        vm_exit(state, EXIT_FAILURE);
    }

    module.name = k_malloc(mem, strlen(name) + 1);
    strcpy(module.name, name);

    // Get the number of bytes
    fseek(fp, 0L, SEEK_END);
    size_t numbytes = (size_t)ftell(fp);

    // reset the file position indicator to the beginning of the file
    fseek(fp, 0L, SEEK_SET);

    char header[6];

    if (numbytes < (long) sizeof(header)) {
        printf("%s is not a valid BSMB file\n", filename);
        vm_exit(state, EXIT_FAILURE);
    }

    fread(&header, sizeof(char), sizeof(header), fp);

    if (header[0] != 'f' || header[1] != 'u' || header[2] != 'n' | header[3] != 'k') {
        printf("%s is not a valid Funky bytecode file\n", filename);
        vm_exit(state, EXIT_FAILURE);
    }

    if ((header[4] & FLAG_LITTLE_ENDIAN) == 0 && !IS_BIG_ENDIAN) {
        printf("%s is compiled for big endian systems. This system is little endian.\n", filename);
        vm_exit(state, EXIT_FAILURE);
    }

    if (header[5] != sizeof(vm_type_t)) {
        printf("%s is compiled for %d virtual bits. This system is %u virtual bits.\n", filename, header[5] * 8,
               (unsigned int) sizeof(vm_type_t) * 8);
        vm_exit(state, EXIT_FAILURE);
    }

    free(filename);

    vm_type_t num_exports, start_of_code;
    fread(&num_exports, sizeof(vm_type_t), 1, fp);
    fread(&start_of_code, sizeof(vm_type_t), 1, fp);

    module.num_exports = num_exports;
    module.start_of_code = start_of_code;

    numbytes -= sizeof(header) + sizeof(vm_type_t) * 2;

    // grab sufficient memory for the buffer to hold the text
    //unsigned char *module_addr = k_calloc(mem, numbytes, sizeof(unsigned char));
    //module.addr = (vm_type_t) (module_addr - mem->main_memory);
    vm_pointer_t module_addr = vm_calloc(mem, numbytes + 1, sizeof(unsigned char));
    unsigned char* native_module_addr = vm_pointer_to_native(mem, module_addr, unsigned char*);
    module.addr = module_addr;

    if (module_addr == 0) {
        fprintf(stderr, "Memory allocation error\nDo you have enough free memory?\n");
        vm_exit(state, EXIT_FAILURE);
    }

    fread(native_module_addr, sizeof(char), numbytes, fp);
    fclose(fp);
    native_module_addr[numbytes] = 0x5C; // ret
    numbytes++;

    module.size = (vm_type_t)numbytes;

    return module;
}

void module_unload(Memory *mem, Module* module) {
    vm_free(mem, module->addr);
}

int module_register(CPU_State *state, Module module) {
    state->num_modules++;
    state->modules = k_realloc(state->memory, state->modules, sizeof(Module) * state->num_modules);
    state->modules[state->num_modules - 1] = module;
    return state->num_modules;
}

int module_release(CPU_State *state, const char* name) {
    for (int i = 0; i < state->num_modules; i++) {
        if (strcmp(state->modules[i].name, name) == 0) {
            k_free(state->memory, state->modules[i].name);
            vm_free(state->memory, state->modules[i].addr);
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

Module *module_get(CPU_State *state, const char* name) {
    for (int i = 0; i < state->num_modules; i++) {
        if (strcmp(state->modules[i].name, name) == 0) {
            return &state->modules[i];
        }
    }

    return NULL;

}

Module* get_current_module(CPU_State *state) {
    for (int i = 0; i < state->num_modules; i++) {
        if (state->pc >= state->modules[i].addr && state->pc < state->modules[i].addr + state->modules[i].size) {
            return &state->modules[i];
        }
    }
    return NULL;
}
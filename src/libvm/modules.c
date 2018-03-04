//
// Created by Bas du Pré on 01-02-18.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <funkyvm/funkyvm.h>

#include "funkyvm/modules.h"
#include "funkyvm/memory.h"
#include "funkyvm/cpu.h"
#include "instructions/instructions.h"
#include "error_handling.h"

#define IS_BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})
#define FLAG_LITTLE_ENDIAN 1u

static char *vm_strlwr(char *s) {
    char *tmp = s;

    for (; *tmp; ++tmp) {
        *tmp = (char) tolower((unsigned char) *tmp);
    }

    return s;
}

int module_register_path(CPU_State *state, const char* path) {
    state->num_module_paths++;
    state->module_paths = realloc(state->module_paths, sizeof(char*) * state->num_module_paths);
    state->module_paths[state->num_module_paths - 1] = strdup(path);
    return state->num_module_paths;
}

static const char* path_separator =
#ifdef _WIN32
        "\\";
#else
        "/";
#endif

char* module_find_filename(CPU_State *state, const char* name) {
    char* filename = malloc(strlen(name) + 1 + 5);
    strcpy(filename, name);

    vm_strlwr(filename);
    char *dot = strrchr(filename, '.');
    if (dot && !strcmp(dot, ".funk")) {
        strcpy(filename, name); // restore original casing
    } else {
        strcpy(filename, name); // restone original casing
        strcat(filename, ".funk");
    }

    if (access(filename, F_OK) != -1) {
        // file exists
        return filename;
    }

    for (int i = 0; i < state->num_module_paths; i++) {
        char *path = malloc(strlen(filename) + strlen(state->module_paths[i]) + 16);
        strcpy(path, state->module_paths[i]);
        strcat(path, path_separator);
        strcat(path, filename);
        if (access(path, F_OK) != -1) {
            // file exists
            free(filename);
            return path;
        }
        free(path);
    }
    free(filename);
    return NULL;
}

int module_exists(CPU_State *state, const char* name) {
    char *filename = module_find_filename(state, name);
    if (filename != NULL) {
        free(filename);
        return 1;
    } else {
        return 0;
    }
}

Module module_load_name(CPU_State* state, const char* name) {
    char *filename = module_find_filename(state, name);

    if (filename == NULL) {
        fprintf(stderr, "Error: Module not found: %s\n", name);
        vm_exit(state, EXIT_FAILURE);
        return (Module) { .name = strdup(name), .addr = 0, .size = 0, .num_exports = 0, .start_of_code = 0 };
    }

    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        int errnum = errno;
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        vm_error(state, "%s", strerror(errnum));
        vm_exit(state, EXIT_FAILURE);
        free(filename);
        return (Module) { .name = strdup(name), .addr = 0, .size = 0, .num_exports = 0, .start_of_code = 0 };
    }

    free(filename);

    // Get the number of bytes
    fseek(fp, 0L, SEEK_END);
    size_t numbytes = (size_t)ftell(fp);

    // reset the file position indicator to the beginning of the file
    fseek(fp, 0L, SEEK_SET);

    byte_t *bytes = malloc(numbytes);
    fread(bytes, sizeof(byte_t), numbytes, fp);
    fclose(fp);

    funky_bytecode_t bytecode = (funky_bytecode_t) {
            .bytes = bytes,
            .length = numbytes
    };

    Module module = module_load(state->memory, name, bytecode);

    module.num_links = 0;

    free(bytes);

    return module;
}

Module module_load(Memory *mem, const char* name, funky_bytecode_t bc) {
    Module module;

    module.name = strdup(name);

    if (bc.length < 6 + 2 * sizeof(vm_type_t)) {
        printf("%s is not a valid BSMB file\n", name);
        vm_exit(NULL, EXIT_FAILURE);
    }

    if (bc.bytes[0] != 'f' || bc.bytes[1] != 'u' || bc.bytes[2] != 'n' || bc.bytes[3] != 'k') {
        printf("%s is not a valid Funky bytecode file\n", name);
        vm_exit(NULL, EXIT_FAILURE);
    }

    if ((bc.bytes[4] & FLAG_LITTLE_ENDIAN) == 0 && !IS_BIG_ENDIAN) {
        printf("%s is compiled for big endian systems. This system is little endian.\n", name);
        vm_exit(NULL, EXIT_FAILURE);
    }

    if (bc.bytes[5] != sizeof(vm_type_t)) {
        printf("%s is compiled for %d virtual bits. This system is %u virtual bits.\n", name, bc.bytes[5] * 8,
               (unsigned int) sizeof(vm_type_t) * 8);
        vm_exit(NULL, EXIT_FAILURE);
    }

    module.num_exports = *(vm_type_t*)(bc.bytes + 6);
    module.start_of_code = *(((vm_type_t*)(bc.bytes + 6)) + 1);
    module.size = (vm_type_t)bc.length - (6 + sizeof(vm_type_t) * 2);

    // grab sufficient memory for the buffer to hold the text
    //unsigned char *module_addr = k_calloc(mem, numbytes, sizeof(unsigned char));
    //module.addr = (vm_type_t) (module_addr - mem->main_memory);
    vm_pointer_t module_addr = vm_calloc(mem, module.size + 1, sizeof(byte_t));
    byte_t* native_module_addr = vm_pointer_to_native(mem, module_addr, byte_t*);
    module.addr = module_addr;

    if (module_addr == 0) {
        fprintf(stderr, "Memory allocation error\nDo you have enough free memory?\n");
        vm_exit(NULL, EXIT_FAILURE);
    }

    memcpy(native_module_addr, bc.bytes + 6 + 2 * sizeof(vm_type_t), module.size);

    native_module_addr[module.size] = 0x5C; // ret
    module.size++;

    module.ref_map = 0;

    return module;
}

void module_unload(Memory *mem, Module module) {
    vm_free(mem, module.addr);
    free(module.name);
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "vm.h"

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"
#include "bindings.h"

int main(int argc, char **argv) {
    static_assert(sizeof(vm_type_t) == sizeof(vm_type_signed_t), "vm_type_t and vm_type_signed_t must be of equal size");
    static_assert(sizeof(vm_type_t) == sizeof(vm_type_float_t), "vm_type_float_t and vm_type_t must be of equal size");
    static_assert(sizeof(vm_pointer_t) <= sizeof(vm_type_t), "vm_pointer_t must be equal or smaller than vm_type_t");
    static_assert(sizeof(enum vm_value_type_t) <= sizeof(vm_type_t), "vm_value_type_t must be equal or smaller than vm_type_t");

    struct optparse_long longopts[] = {
            {"amend", 'a', OPTPARSE_NONE},
            {"brief", 'b', OPTPARSE_NONE},
            {"color", 'c', OPTPARSE_REQUIRED},
            {"delay", 'd', OPTPARSE_OPTIONAL},
            {0}
    };

    int amend = 0;
    int brief = 0;
    const char *color = "white";
    int delay = 0;

    int option;
    struct optparse options;

    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
        switch (option) {
            case 'a':
                amend = 1;
                break;
            case 'b':
                brief = 1;
                break;
            case 'c':
                color = options.optarg;
                break;
            case 'd':
                delay = options.optarg ? atoi(options.optarg) : 1;
                break;
            case '?':
                fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
                exit(EXIT_FAILURE);
        }
    }

    if (options.optind >= argc) {
        printf("Usage: %s [kernel]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    unsigned char *main_memory = malloc(VM_MEMORY_LIMIT);
    Memory memory;
    memory_init(&memory, main_memory);
    Module kernel;
    int kernel_set = 0;

    CPU_State state = cpu_init(&memory);

    register_bindings(&state);

    char *filename;
    while ((filename = optparse_arg(&options))) {
        Module module = module_load(&memory, filename);
        if (!kernel_set) {
            kernel = module;
            kernel_set = 1;
        }
        module_register(&state, module);
    }

    cpu_set_entry_to_module(&state, &kernel);

    vm_type_t ret = cpu_run(&state);

    free(main_memory);

    return ret;
}

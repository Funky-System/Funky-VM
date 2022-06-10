#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "funkyvm/funkyvm.h"
#include "libvm/os.h"
#include "version.h"

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"
#include "bindings.h"
#include "performance.h"

int main(int argc, char **argv) {
    static_assert(sizeof(vm_type_t) == sizeof(vm_type_signed_t), "vm_type_t and vm_type_signed_t must be of equal size");
    static_assert(sizeof(vm_type_t) == sizeof(vm_type_float_t), "vm_type_float_t and vm_type_t must be of equal size");
    static_assert(sizeof(vm_pointer_t) <= sizeof(vm_type_t), "vm_pointer_t must be equal or smaller than vm_type_t");
    static_assert(sizeof(enum vm_value_type_t) <= sizeof(vm_type_t), "vm_value_type_t must be equal or smaller than vm_type_t");

#if defined(VM_NATIVE_MALLOC) && VM_NATIVE_MALLOC
    unsigned char *main_memory = 0;
#else
    unsigned char *main_memory = malloc(VM_MEMORY_LIMIT);
#endif
    Memory memory;
    memory_init(&memory, main_memory);
    Module kernel;
    int kernel_set = 0;

    CPU_State state = cpu_init(&memory);

    struct optparse_long longopts[] = {
            {"amend", 'a', OPTPARSE_NONE},
            {"brief", 'b', OPTPARSE_NONE},
            {"color", 'c', OPTPARSE_REQUIRED},
            {"performance-test", 'P', OPTPARSE_REQUIRED},
            {"delay", 'd', OPTPARSE_OPTIONAL},
            {"library-search-path", 'L', OPTPARSE_REQUIRED},
            {"version", 'v', OPTPARSE_NONE},
            {0}
    };

    int amend = 0;
    int brief = 0;
    const char *color = "white";
    int delay = 0;
    int performance_test = 0;

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
            case 'L':
                module_register_path(&state, options.optarg);
                break;
            case 'd':
                delay = options.optarg ? atoi(options.optarg) : 1;
                break;
            case 'P':
                performance_test = atoi(options.optarg);
                break;
            case 'v':
                printf("Funky VM version %s.%s.%s\nBuilt on %s %s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, __DATE__, __TIME__);
                return 0;
            case '?':
                fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
                exit(EXIT_FAILURE);
        }
    }

    if (options.optind >= argc) {
        printf("Usage: %s [kernel]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

#if defined(FUNKY_VM_OS_MACOS)
    module_register_path(&state, "/usr/local/lib/funky");
    module_register_path(&state, "/usr/lib/funky");
    module_register_path(&state, "/lib/funky");
    module_register_path(&state, "/System/Library/Funky");
    module_register_path(&state, "/Library/Funky");
    module_register_path(&state, "~/Library/Funky");
#elif defined(FUNKY_VM_OS_LINUX)
    module_register_path(&state, "/usr/local/lib/funky");
    module_register_path(&state, "/usr/lib/funky");
    module_register_path(&state, "/lib/funky");
#elif defined(FUNKY_VM_OS_WINDOWS)

#endif

    char *stdlibpath = get_executable_path("stdlib");
    module_register_path(&state, stdlibpath);
    free(stdlibpath);

    register_bindings(&state);

    char *filename;
    while ((filename = optparse_arg(&options))) {
        Module module = module_load_name(&state, filename);
        if (!kernel_set) {
            kernel = module;
            kernel_set = 1;
        }
        module_register(&state, module);
    }

    cpu_set_entry_to_module(&state, &kernel);

    vm_type_t ret = 0;
    if (performance_test) {
        double duration = performance_test_run(&state, performance_test);
        printf("%9.6f", duration);
    } else {
         ret = cpu_run(&state);
    }

    cpu_destroy(&state);
    memory_destroy(&memory);
    free(main_memory);

    return ret;
}

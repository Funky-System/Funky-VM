#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "instructions.h"
#include "../../../include/funkyvm/cpu.h"
#include "../../../include/funkyvm/funkyvm.h"
#include "../../../include/funkyvm/memory.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

INSTR(nop) {
    // literally do nothing
}

INSTR(halt) {
    state->running = 0;
}

INSTR(trap) {
    vm_type_t operand = GET_OPERAND();
    USE_STACK();

    switch (operand) {
        case 0:
            printf("Trap 0: Print registers\n");
            printf(" pc: 0x%04X\n sp: 0x%04X\n mp: 0x%04X\n rr: 0x%04X\n r0: %d\n r1: %d\n r2: %d\n r3: %d\n r4: %d\n r5: %d\n r6: %d\n r7: %d\n",
                state->pc, state->sp, state->mp, state->rr.uint_value, state->r0.uint_value, state->r1.uint_value, state->r2.uint_value, state->r3.uint_value,
                state->r4.uint_value, state->r5.uint_value, state->r6.uint_value, state->r7.uint_value);
            break;

        case 1:
            printf("Trap 1: Print stack\n");
            printf("+--------+----+------+---------------+-------------------------------+\n");
            printf("|  addr  |    |  rel | type          | value                         |\n");
            printf("|--------|----|------|---------------|-------------------------------|\n");
            vm_type_signed_t stack_size = (vm_type_signed_t)(((vm_type_signed_t)state->sp - (vm_type_signed_t)state->stack_base) / sizeof(vm_value_t) + 1);
            for (int i = 0; i < MIN(stack_size, 16); i++) {
                char* note = "";
                vm_type_t addr = (state->sp - i * sizeof(vm_value_t));
                if (addr == state->sp) note = "SP";
                if (addr == state->mp) note = "MP";
                if (addr == state->ap) note = "AP";
                printf("| 0x%04X | %2s | %4d ", addr, note, -i);
                vm_value_t val = *(stack - i);
                if (val.type == VM_TYPE_UINT) {
                    printf("| %-13s | %-30u|\n", "unsigned", val.uint_value);
                } else if (val.type == VM_TYPE_INT) {
                    printf("| %-13s | %-30i|\n", "integer", val.int_value);
                } else if (val.type == VM_TYPE_FLOAT) {
                    printf("| %-13s | %-30f|\n", "float", val.float_value);
                } else if (val.type == VM_TYPE_REF) {
                    printf("| %-13s | %-#30x|\n", "reference", val.uint_value);
                }  else if (val.type == VM_TYPE_STRING) {
                    vm_type_t ref_count = *vm_pointer_to_native(state->memory, val.pointer_value, vm_type_t*);
                    char *str = vm_pointer_to_native(state->memory, val.pointer_value + sizeof(vm_type_t), char*);
                    if (ref_count == VM_UNSIGNED_MAX) ref_count = 0;

                    char prstr[strlen(str) + 3];
                    strcpy(prstr, "\"");
                    strcat(prstr, str);
                    strcat(prstr, "\"");
                    printf("| string (%4d) | %-29.29s |\n", ref_count, prstr);
                } else if (val.type == VM_TYPE_ARRAY) {
                    vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, val.pointer_value, vm_type_t*);
                    vm_type_t ref_count = *(reserved_mem);
                    vm_type_t len = *(reserved_mem + 1);

                    printf("| array (%4d)  | Array of length %-4d          |\n", ref_count, len);
                } else if (val.type == VM_TYPE_MAP) {
                    vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, val.pointer_value, vm_type_t*);
                    vm_type_t ref_count = *(reserved_mem);

                    printf("| map (%4d)    | Map                           |\n", ref_count);
                } else if (val.type == VM_TYPE_EMPTY) {
                    printf("| Empty         |                               |\n");
                } else {
                    printf("| %-10s | %-#30x|\n", "CORRUPTED!   ", val.uint_value);
                }
            }
            printf("+--------+----+------+---------------+-------------------------------+\n\n");
            break;

        case 2:
            if (stack->type == VM_TYPE_UINT) {
                printf("%u\n", stack->uint_value);
            } else if (stack->type == VM_TYPE_INT) {
                printf("%i\n", stack->int_value);
            } else if (stack->type == VM_TYPE_FLOAT) {
                printf("%f\n", stack->float_value);
            } else if (stack->type == VM_TYPE_STRING) {
                printf("%s\n", cstr_pointer_from_vm_value(state, stack));
            }
            break;

        case 3:
            printf("Trap 3: Print array\n");
            if (stack->type != VM_TYPE_ARRAY) {
                printf("Top of stack is not of type array!\n");
                break;
            }
            printf("+------+---------------+-------------------------------+\n");
            printf("| ind  | type          | value                         |\n");
            printf("|------|---------------|-------------------------------|\n");
            vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, stack->pointer_value, vm_type_t*);
            vm_type_t len = *(reserved_mem + 1);
            vm_pointer_t *array_ptr = reserved_mem + 2;
            vm_value_t *array = vm_pointer_to_native(state->memory, *array_ptr, vm_value_t*);

            for (int i = 0; i < len; i++) {
                printf("| %4d | ", i);
                vm_value_t val = array[i];
                if (val.type == VM_TYPE_UINT) {
                    printf("%-13s | %-30u|\n", "unsigned", val.uint_value);
                } else if (val.type == VM_TYPE_INT) {
                    printf("%-13s | %-30i|\n", "integer", val.int_value);
                } else if (val.type == VM_TYPE_FLOAT) {
                    printf("%-13s | %-30f|\n", "float", val.float_value);
                } else if (val.type == VM_TYPE_REF) {
                    printf("%-13s | %-#30x|\n", "reference", val.uint_value);
                }  else if (val.type == VM_TYPE_STRING) {
                    int offset = 0;
                    offset = sizeof(vm_type_t);
                    vm_type_t *str_reserved_mem = vm_pointer_to_native(state->memory, val.pointer_value, vm_type_t*);
                    vm_type_t ref_count = *(str_reserved_mem);
                    if (ref_count == VM_UNSIGNED_MAX) ref_count = 0;
                    char str[strlen(cstr_pointer_from_vm_value(state, &val)) + 3];
                    strcpy(str, "\"");
                    strcat(str, cstr_pointer_from_vm_value(state, &val));
                    strcat(str, "\"");
                    printf("string (%4d) | %-29.29s |\n", ref_count, str);
                } else if (val.type == VM_TYPE_ARRAY) {
                    vm_type_t *arr_reserved_mem = vm_pointer_to_native(state->memory, val.pointer_value, vm_type_t*);
                    vm_type_t arr_ref_count = *(arr_reserved_mem);
                    vm_type_t arr_len = *(arr_reserved_mem + 1);

                    printf("array (%4d)  | Array of length %-4d          |\n", arr_ref_count, arr_len);
                } else if (val.type == VM_TYPE_EMPTY) {
                    printf("Empty         |                               |\n");
                } else {
                    printf("%-10s | %-#30x|\n", "CURRUPTED!", val.uint_value);
                }
            }
            printf("+------+---------------+-------------------------------+\n\n");
            break;

        case 5: {
            printf("Trap 5: Print map\n");
            if (stack->type != VM_TYPE_MAP) {
                printf("Top of stack is not of type map!\n");
                break;
            }
            printf("+---------------------+---------------+-------------------------------+\n");
            printf("| name                | type          | value                         |\n");
            printf("|---------------------|---------------|-------------------------------|\n");
            vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, stack->pointer_value, vm_type_t*);
            vm_pointer_t *first_ptr = reserved_mem + 1;
            if (*first_ptr > 0) {
                vm_map_elem_t *item = vm_pointer_to_native(state->memory, *first_ptr, vm_map_elem_t*);

                while (1) {
                    char *name = cstr_pointer_from_vm_pointer_t(state, item->name);
                    printf("| %-19.19s | ", name);
                    vm_value_t val = item->value;
                    if (val.type == VM_TYPE_UINT) {
                        printf("%-13s | %-30u|\n", "unsigned", val.uint_value);
                    } else if (val.type == VM_TYPE_INT) {
                        printf("%-13s | %-30i|\n", "integer", val.int_value);
                    } else if (val.type == VM_TYPE_FLOAT) {
                        printf("%-13s | %-30f|\n", "float", val.float_value);
                    } else if (val.type == VM_TYPE_REF) {
                        printf("%-13s | %-#30x|\n", "reference", val.uint_value);
                    } else if (val.type == VM_TYPE_STRING) {
                        int offset = 0;
                        offset = sizeof(vm_type_t);
                        vm_type_t *str_reserved_mem = vm_pointer_to_native(state->memory, val.pointer_value,
                                                                           vm_type_t*);
                        vm_type_t ref_count = *(str_reserved_mem);
                        if (ref_count == VM_UNSIGNED_MAX) ref_count = 0;
                        char str[strlen(cstr_pointer_from_vm_value(state, &val)) + 3];
                        strcpy(str, "\"");
                        strcat(str, cstr_pointer_from_vm_value(state, &val));
                        strcat(str, "\"");
                        printf("string (%4d) | %-29.29s |\n", ref_count, str);
                    } else if (val.type == VM_TYPE_ARRAY) {
                        vm_type_t *arr_reserved_mem = vm_pointer_to_native(state->memory, val.pointer_value,
                                                                           vm_type_t*);
                        vm_type_t arr_ref_count = *(arr_reserved_mem);
                        vm_type_t arr_len = *(arr_reserved_mem + 1);

                        printf("array (%4d)  | Array of length %-4d          |\n", arr_ref_count, arr_len);
                    } else if (val.type == VM_TYPE_MAP) {
                        vm_type_t *reserved_mem = vm_pointer_to_native(state->memory, val.pointer_value, vm_type_t*);
                        vm_type_t ref_count = *(reserved_mem);

                        printf("map (%4d)    | Map                           |\n", ref_count);
                    } else if (val.type == VM_TYPE_EMPTY) {
                        printf("Empty         |                               |\n");
                    } else {
                        printf("%-10s | %-#30x|\n", "CURRUPTED!", val.uint_value);
                    }

                    if (item->next == 0) break;
                    item = vm_pointer_to_native(state->memory, item->next, vm_map_elem_t*);
                }
            }
            printf("+---------------------+---------------+-------------------------------+\n\n");
            break;
        }
        case 4:
            printf("Trap 4: Print modules\n");
            for (int i = 0; i < state->num_modules; i++) {
                printf("Module %d: %s\n", i, state->modules[i].name);
                char *addr = (char*)state->memory->main_memory + state->modules[i].addr;
                int num_found = 0;
                while (num_found < state->modules[i].num_exports) {
                    char *name = addr;
                    char c;
                    while (*(addr++) != '\0') { }
                    vm_type_t ref = *((vm_type_t*)addr);
                    addr += sizeof(vm_type_t);
                    printf("  %d: %s (0x%x)\n", num_found, name, ref);
                    num_found++;
                }
            }
            break;

        default:
            printf("Trap %d is not defined\n", operand);
            break;
    }
}

INSTR(int) {
    vm_type_t operand = GET_OPERAND();

    if (operand == 10) {
        USE_STACK();
        vm_value_t *string_addr = (vm_value_t*)(state->memory->main_memory + stack->uint_value);
        AJS_STACK(-1);
        char c;
        do {
            c = (char)string_addr->uint_value;
            if (c != '\0') putchar(c);
            string_addr++;
        } while (c != '\0');
    }

}

INSTR(debug_break) {
    printf("breakpoint!\n");
}

INSTR(debug_setcontext) {
    state->debug_context.filename = vm_pointer_to_native(state->memory, get_current_module(state)->addr + GET_OPERAND() + sizeof(vm_type_t), const char*);
    state->debug_context.line = GET_OPERAND_SIGNED();
    state->debug_context.col = GET_OPERAND_SIGNED();
}

INSTR(debug_enterscope) {
    state->debug_context.num_stacktrace++;
    state->debug_context.stacktrace = realloc(state->debug_context.stacktrace, sizeof(struct Stacktrace_Frame) * state->debug_context.num_stacktrace);
    state->debug_context.stacktrace[state->debug_context.num_stacktrace - 1] = (struct Stacktrace_Frame) {
            .name = vm_pointer_to_native(state->memory, get_current_module(state)->addr + GET_OPERAND() + sizeof(vm_type_t), const char*),
            .filename = state->debug_context.filename,
            .col = state->debug_context.col,
            .line = state->debug_context.line
    };
}

INSTR(debug_leavescope) {
    state->debug_context.num_stacktrace--;
    if (state->debug_context.num_stacktrace < 0)
        state->debug_context.num_stacktrace = 0;
    state->debug_context.stacktrace = realloc(state->debug_context.stacktrace, sizeof(struct Stacktrace_Frame) * state->debug_context.num_stacktrace);
}

#include "funkyvm/cpu.h"
#include "instructions/instructions.h"
#include "liballoc_1_1.h"
#include "funkyvm/memory.h"
#include "boxing.h"

#ifdef FUNKY_VM_OS_EMSCRIPTEN
#include <emscripten/emscripten.h>
#pragma pack(1)
#endif

CPU_State cpu_init(Memory* memory) {
    CPU_State state;
    state.memory = memory;

    state.stack_base = vm_malloc(memory, VM_STACK_SIZE);
    state.pc = 0;
    state.mp = state.stack_base - sizeof(vm_type_signed_t);
    state.sp = state.stack_base - sizeof(vm_type_signed_t);
    state.ap = state.stack_base - sizeof(vm_type_signed_t);
    state.rr = (vm_value_t) { 0 };
    state.r0 = (vm_value_t) { 0 };
    state.r1 = (vm_value_t) { 0 };
    state.r2 = (vm_value_t) { 0 };
    state.r3 = (vm_value_t) { 0 };
    state.r4 = (vm_value_t) { 0 };
    state.r5 = (vm_value_t) { 0 };
    state.r6 = (vm_value_t) { 0 };
    state.r7 = (vm_value_t) { 0 };

    state.debug_context = (Debug_Context) { .filename = NULL, .line = -1, .col = -1, .num_stacktrace = 0 };
    state.debug_context.stacktrace = malloc(0);

    state.in_error_state = 0;

    state.modules = k_malloc(memory, 0);
    state.num_modules = 0;

    state.module_paths = malloc(0);
    state.num_module_paths = 0;

    state.syscall_table = k_malloc(memory, 0);
    state.num_syscalls = 0;

    state.running = 1;

    initialize_boxing_prototypes(&state);

    return state;
}

void cpu_destroy(CPU_State *state) {
    for (int i = 0; i < state->num_module_paths; i++) {
        free (state->module_paths[i]);
    }
    free(state->module_paths);
}

void cpu_set_entry_to_module(CPU_State *state, Module *mod) {
    state->pc = mod->addr + mod->start_of_code;
}

#ifdef FUNKY_VM_OS_EMSCRIPTEN
void cpu_emscripten_yield(CPU_State *state) {
    state->emscripten_yield = 1;
}

void emscripten_loop(void *arg) {
    CPU_State *state = (CPU_State *) arg;
    state->emscripten_yield = 0;
    while (!state->emscripten_yield && state->running) {
        unsigned char opcode = *(state->memory->main_memory + state->pc);
        state->pc++;
        instruction_implementations[opcode](state);
    }
    state->emscripten_yield = 1;
}
#endif

vm_type_t cpu_run(CPU_State *state) {
#ifdef FUNKY_VM_OS_EMSCRIPTEN
    emscripten_set_main_loop_arg(emscripten_loop, state, 0, 0);
    return 0;
#else
    while (state->running) {
        unsigned char opcode = *(state->memory->main_memory + state->pc);
        state->pc++;
        instruction_implementations[opcode](state);
    }

    return state->rr.uint_value;
#endif
}
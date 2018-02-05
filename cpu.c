#include "cpu.h"
#include "instructions.h"
#include "liballoc_1_1.h"
#include "memory.h"

CPU_State cpu_init(Memory* memory) {
    CPU_State state;
    state.memory = memory;

    state.stack_base = vm_malloc(memory, VM_STACK_SIZE);
    state.pc = 0;
    state.mp = state.stack_base - sizeof(vm_type_signed_t);
    state.sp = state.stack_base - sizeof(vm_type_signed_t);
    state.rr = (vm_value_t) { 0 };
    state.r0 = (vm_value_t) { 0 };
    state.r1 = (vm_value_t) { 0 };
    state.r2 = (vm_value_t) { 0 };
    state.r3 = (vm_value_t) { 0 };
    state.r4 = (vm_value_t) { 0 };
    state.r5 = (vm_value_t) { 0 };
    state.r6 = (vm_value_t) { 0 };
    state.r7 = (vm_value_t) { 0 };

    state.modules = k_malloc(memory, 0);
    state.num_modules = 0;

    return state;
}

void cpu_set_entry_to_module(CPU_State *state, Module *mod) {
    state->pc = mod->addr + mod->start_of_code;
}

vm_type_t cpu_run(CPU_State *state) {
    state->running = 1;

    while (state->running) {
        unsigned char opcode = *(state->memory->main_memory + state->pc);
        state->pc++;
        instruction_implementations[opcode](state);
    }

    return state->rr.uint_value;
}
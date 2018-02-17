//
// Created by Bas du Pré on 10-02-18.
//

#include <stdio.h>
#include <stdarg.h>

#include "error_handling.h"

static CPU_State unknown_state = {0};

void vm_exit(CPU_State* state, int res) {
    state->running = 0;
}

void vm_vaerror(CPU_State* state, const char* error_message, va_list vl) {
    if (state == NULL) {
        if (unknown_state.debug_context.filename == 0) {
            unknown_state = (CPU_State) {
                    .debug_context = (Debug_Context) {
                            .filename = "<unknown>",
                            .col = 0,
                            .line = 0,
                            .num_stacktrace = 0
                    }
            };
        }
        state = &unknown_state;
    }
    fprintf(stderr, "Error: ");
    vfprintf(stderr, error_message, vl);
    fprintf(stderr, "\n");

    for (int i = state->debug_context.num_stacktrace; i >= 0; i--) {
        if (i == 0) {
            fprintf(stderr, "  at %s:%d:%d\n",
                    state->debug_context.filename, state->debug_context.line, state->debug_context.col);
        } else if (i == state->debug_context.num_stacktrace) {
            fprintf(stderr, "  at %s (%s:%d:%d)\n",
                    state->debug_context.stacktrace[state->debug_context.num_stacktrace - 1].name,
                    state->debug_context.filename, state->debug_context.line, state->debug_context.col);
        } else {
            struct Stacktrace_Frame *frame = &state->debug_context.stacktrace[i];
            fprintf(stderr, "  at %s (%s:%d:%d)\n", state->debug_context.stacktrace[i - 1].name, frame->filename,
                    frame->line, frame->col);
        }
    }

    va_end(vl);
}

void vm_error(CPU_State* state, const char* error_message, ...) {
    va_list vl;
    va_start(vl, error_message);
    vm_vaerror(state, error_message, vl);
    va_end(vl);
}

void vm_assert(CPU_State* state, int res, const char* error_message, ...) {
    if (!res) {
        va_list vl;
        va_start(vl, error_message);
        vm_vaerror(state, error_message, vl);
        va_end(vl);
        vm_exit(state, EXIT_FAILURE);
    }
}


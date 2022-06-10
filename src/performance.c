//
// Created by basdu on 10-6-2022.
//

#include <stdio.h>

#include "funkyvm/funkyvm.h"
#include "performance.h"

#ifdef FUNKY_VM_OS_WINDOWS
#include <Windows.h>
double get_wall_time(){
    LARGE_INTEGER time,freq;
    if (!QueryPerformanceFrequency(&freq)){
        //  Handle error
        return 0;
    }
    if (!QueryPerformanceCounter(&time)){
        //  Handle error
        return 0;
    }
    return (double)time.QuadPart / freq.QuadPart;
}
double get_cpu_time(){
    FILETIME a,b,c,d;
    if (GetProcessTimes(GetCurrentProcess(),&a,&b,&c,&d) != 0){
        //  Returns total user time.
        //  Can be tweaked to include kernel times as well.
        return
                (double)(d.dwLowDateTime |
                         ((unsigned long long)d.dwHighDateTime << 32)) * 0.0000001;
    }else{
        //  Handle error
        return 0;
    }
}

#else

#include <time.h>
#include <sys/time.h>
double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
double get_cpu_time(){
    return (double)clock() / CLOCKS_PER_SEC;
}

#endif

#define cpu_reset_quick(state) {\
    state->pc = 0;\
    state->mp = state->stack_base - sizeof(vm_type_signed_t);\
    state->sp = state->stack_base - sizeof(vm_type_signed_t);\
    state->ap = state->stack_base - sizeof(vm_type_signed_t);\
    state->rr = (vm_value_t) { 0 };\
    state->r0 = (vm_value_t) { 0 };\
    state->r1 = (vm_value_t) { 0 };\
    state->r2 = (vm_value_t) { 0 };\
    state->r3 = (vm_value_t) { 0 };\
    state->r4 = (vm_value_t) { 0 };\
    state->r5 = (vm_value_t) { 0 };\
    state->r6 = (vm_value_t) { 0 };\
    state->r7 = (vm_value_t) { 0 };\
    state->in_error_state = 0;\
    state->running = 1;\
}

double performance_test_run(CPU_State *state, int iterations) {
    double start = get_wall_time();
    while (iterations > 0) {
        cpu_reset_quick(state);
        cpu_run(state);
        iterations--;
    }
    return get_wall_time() - start;
}


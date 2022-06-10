//
// Created by basdu on 10-6-2022.
//

#ifndef FUNKY_VM_PERFORMANCE_H
#define FUNKY_VM_PERFORMANCE_H

#include "funkyvm/funkyvm.h"

double get_cpu_time();
double performance_test_run(CPU_State *state, int iterations);

#endif //FUNKY_VM_PERFORMANCE_H

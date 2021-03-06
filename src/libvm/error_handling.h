//
// Created by Bas du Pré on 10-02-18.
//

#ifndef FUNKY_VM_ERROR_HANDLING_H
#define FUNKY_VM_ERROR_HANDLING_H

#include "funkyvm/funkyvm.h"

void vm_exit(CPU_State* state, int res);
void vm_assert(CPU_State* state, int res, const char* error_message, ...);
void vm_error(CPU_State* state, const char* error_message, ...);

#endif //FUNKY_VM_ERROR_HANDLING_H

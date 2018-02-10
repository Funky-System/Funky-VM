//
// Created by Bas du Pré on 10-02-18.
//

#ifndef FUNKY_VM_ERROR_HANDLING_H
#define FUNKY_VM_ERROR_HANDLING_H

#include "vm.h"

#define vm_exit(state, res) exit(res)

void vm_assert(CPU_State* state, int res, const char* error_message, ...);
void vm_error(CPU_State* state, const char* error_message, ...);

#endif //FUNKY_VM_ERROR_HANDLING_H

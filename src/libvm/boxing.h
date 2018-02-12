//
// Created by Bas du Pré on 09-02-18.
//

#ifndef FUNKY_VM_BOXING_H
#define FUNKY_VM_BOXING_H

#include "../../include/funkyvm/cpu.h"

void initialize_boxing_prototypes(CPU_State *state);
void st_mapitem(CPU_State *state, vm_pointer_t map_ptr, const char* name, vm_value_t* value);
vm_map_elem_t* ld_mapitem(CPU_State *state, vm_pointer_t map_ptr, const char* name);

#endif //FUNKY_VM_BOXING_H

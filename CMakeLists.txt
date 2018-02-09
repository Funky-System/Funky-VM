cmake_minimum_required(VERSION 3.5)
project(funky-vm)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DVM_ARCH_BITS=32)

add_library(funky-vm
        cpu.c  instructions.c instr_cpu.c instr_mem.c instr_computation.c instr_branching.c
        instr_convert.c instr_string.c memory.c instr_array.c
        liballoc_1_1.c modules.c instr_mod.c instr_map.c boxing.c boxing.h)

add_executable(funky-vm-bin vm.c)
target_link_libraries(funky-vm-bin m funky-vm)
target_link_libraries(funky-vm m)
set_target_properties(funky-vm-bin PROPERTIES OUTPUT_NAME funky-vm)

configure_file(test_vm.sh ${CMAKE_BINARY_DIR}/bin/test_vm.sh COPYONLY)

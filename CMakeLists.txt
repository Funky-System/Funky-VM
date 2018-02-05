cmake_minimum_required(VERSION 3.5)
project(vm)

set(CMAKE_CXX_STANDARD 11)

add_library(vm
        cpu.c  instructions.c instr_cpu.c instr_mem.c instr_computation.c instr_branching.c
        instr_convert.c instr_string.c memory.c instr_array.c
        liballoc_1_1.c modules.c instr_mod.c)

add_executable(vm-bin vm.c)
target_link_libraries(vm-bin m vm)
target_link_libraries(vm m)
set_target_properties(vm-bin PROPERTIES OUTPUT_NAME vm)

configure_file(test_vm.sh ${CMAKE_BINARY_DIR}/bin/test_vm.sh COPYONLY)

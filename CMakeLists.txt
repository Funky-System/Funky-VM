cmake_minimum_required(VERSION 3.5)
project(funky-vm)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_definitions(-DVM_ARCH_BITS=32)

add_library(funky-vm
        src/libvm/cpu.c src/libvm/instructions/instructions.c src/libvm/instructions/instr_cpu.c src/libvm/instructions/instr_mem.c src/libvm/instructions/instr_computation.c src/libvm/instructions/instr_branching.c
        src/libvm/instructions/instr_convert.c src/libvm/instructions/instr_string.c include/funkyvm/memory.c src/libvm/instructions/instr_array.c
        src/libvm/liballoc_1_1.c src/libvm/modules.c src/libvm/instructions/instr_mod.c src/libvm/instructions/instr_map.c src/libvm/boxing.c src/libvm/boxing.h src/libvm/error_handling.c src/libvm/error_handling.h src/libvm/syscall.c include/funkyvm/syscall.h)

add_executable(funky-vm-bin src/funkyvm.c src/bindings.c src/bindings.h)
target_link_libraries(funky-vm-bin m funky-vm)
target_link_libraries(funky-vm m)
set_target_properties(funky-vm-bin PROPERTIES OUTPUT_NAME funky-vm)

configure_file(test/test_vm.sh ${CMAKE_BINARY_DIR}/bin/test_vm.sh COPYONLY)

file(COPY include DESTINATION ${CMAKE_BINARY_DIR})

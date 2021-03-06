cmake_minimum_required(VERSION 3.5)
project(funky-vm)

set(CMAKE_C_STANDARD 11)

include_directories(include)

if (NOT DEFINED VM_ARCH_BITS)
    set(VM_ARCH_BITS 32)
endif()
add_definitions(-DVM_ARCH_BITS=${VM_ARCH_BITS})

if (NOT DEFINED VM_NATIVE_MALLOC)
    set(VM_NATIVE_MALLOC 0)
endif()
add_definitions(-DVM_NATIVE_MALLOC=${VM_NATIVE_MALLOC})

add_library(funky-vm
        src/libvm/cpu.c src/libvm/instructions/instructions.c src/libvm/instructions/instr_cpu.c src/libvm/instructions/instr_mem.c src/libvm/instructions/instr_computation.c src/libvm/instructions/instr_branching.c
        src/libvm/instructions/instr_convert.c src/libvm/instructions/instr_string.c src/libvm/memory.c src/libvm/instructions/instr_array.c
        src/libvm/liballoc_1_1.c src/libvm/modules.c src/libvm/instructions/instr_mod.c src/libvm/instructions/instr_map.c src/libvm/boxing.c src/libvm/boxing.h src/libvm/error_handling.c src/libvm/error_handling.h src/libvm/syscall.c include/funkyvm/syscall.h src/libvm/os.c src/libvm/os.h include/funkyvm/os.h)

add_executable(funky-vm-bin src/funkyvm.c src/bindings.c src/bindings.h)
target_link_libraries(funky-vm-bin m funky-vm)
target_link_libraries(funky-vm m)
set_target_properties(funky-vm-bin PROPERTIES OUTPUT_NAME funky-vm)

configure_file(test/test_vm.sh ${CMAKE_BINARY_DIR}/bin/test_vm.sh COPYONLY)

file(COPY include DESTINATION ${CMAKE_BINARY_DIR})

install(TARGETS funky-vm-bin DESTINATION bin)
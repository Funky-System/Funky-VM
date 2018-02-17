//
// Created by Bas du Pré on 21-01-18.
//

#ifndef VM_H
#define VM_H

#include <stdlib.h>
#include <stdint.h>

#ifndef VM_ARCH_BITS
#define VM_ARCH_BITS 32
#endif

#if VM_ARCH_BITS == 16
typedef uint16_t vm_type_t;
    typedef uint16_t vm_type_t;
    typedef int16_t vm_type_signed_t;
    typedef __fp16 vm_type_float_t;
    typedef vm_type_t vm_pointer_t;
    #define VM_UNSIGNED_MAX UINT16_MAX
    #define VM_SIGNED_MAX INT16_MAX
#elif VM_ARCH_BITS == 32
    typedef uint32_t vm_type_t;
    typedef int32_t vm_type_signed_t;
    typedef float vm_type_float_t;
    typedef vm_type_t vm_pointer_t;
    #define VM_UNSIGNED_MAX UINT32_MAX
    #define VM_SIGNED_MAX INT32_MAX
#elif VM_ARCH_BITS == 64
    typedef uint64_t vm_type_t;
    typedef int64_t vm_type_signed_t;
    typedef float vm_type_float_t;
    typedef vm_type_t vm_pointer_t;
    #define VM_UNSIGNED_MAX UINT64_MAX
    #define VM_SIGNED_MAX INT64_MAX
#else
    #error Value for VM_ARCH_BITS is unsupported
#endif

#define VM_MEMORY_LIMIT (1024 * 1024) // one whole meg!
#define VM_STACK_SIZE (sizeof(vm_value_t) * 1024)

enum vm_value_type_t {
    VM_TYPE_INT = 0,
    VM_TYPE_UINT,
    VM_TYPE_FLOAT,
    VM_TYPE_STRING,
    VM_TYPE_REF,      // address of a vm_value_t somewhere
    VM_TYPE_MAP,      // address of a custom type (vm_obj_t)
    VM_TYPE_ARRAY,
    VM_TYPE_EMPTY,
    VM_TYPE_UNKNOWN
};

typedef struct {
    union {
        enum vm_value_type_t type;
        vm_type_t __padding1;
    };
    union {
        vm_type_t uint_value;
        vm_type_signed_t int_value;
        vm_type_float_t float_value;
        vm_pointer_t pointer_value;
    };
} vm_value_t;

typedef struct {
    vm_pointer_t name;
    vm_value_t value;

    vm_pointer_t next;
    vm_pointer_t prev;
} vm_map_elem_t;

typedef unsigned char byte_t;
typedef struct {
    byte_t* bytes;
    vm_type_t length;
} funky_bytecode_t;

#include "cpu.h"
#include "modules.h"
#include "memory.h"
#include "syscall.h"

#endif //VM_H

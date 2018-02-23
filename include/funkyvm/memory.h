#ifndef VM_MEMORY_H
#define VM_MEMORY_H

#include <stdlib.h>

#define PAGE_SIZE 4096

typedef __attribute__((aligned(1))) struct {
    __attribute__((aligned(1))) unsigned char* main_memory;
    __attribute__((aligned(1))) unsigned char* bitmap;
    vm_type_t bitmap_size;
    int lock;
} Memory;

void memory_init(Memory *mem, unsigned char *main_memory);
void memory_destroy(Memory *mem);
void memory_print_bitmap_debug(Memory *mem);

#if defined(VM_NATIVE_MALLOC) && VM_NATIVE_MALLOC

    #define k_malloc(mem, size) (void*)malloc(size)
    #define k_realloc(mem, ptr, size) (void*)realloc((void*)ptr, size)
    #define k_calloc(mem, n, size) (void*)calloc(n, size)
    #define k_free(mem, ptr) free((void*)ptr)

    #define vm_pointer_to_native(MEMORY, POINTER, TYPE) ( (TYPE) ((vm_pointer_t)(POINTER)) )
    #define native_to_vm_pointer(MEMORY, POINTER) ( (vm_pointer_t) ((unsigned char*)(POINTER)) )
    #define vm_malloc(MEMORY, SIZE) ((vm_pointer_t) ((unsigned char *)k_malloc(MEMORY, SIZE)))
    #define vm_realloc(MEMORY, POINTER, SIZE) ((vm_pointer_t) ((unsigned char*)k_realloc(MEMORY, vm_pointer_to_native(MEMORY, POINTER, void*), SIZE)))
    #define vm_calloc(MEMORY, NUM, SIZE) ((vm_pointer_t) ((unsigned char *)k_calloc(MEMORY, NUM, SIZE)))
    #define vm_free(MEMORY, POINTER) k_free(MEMORY, POINTER)

#else

    extern void    *k_malloc(Memory *,size_t);
    extern void    *k_realloc(Memory *,void *, size_t);
    extern void    *k_calloc(Memory *,size_t, size_t);
    extern void     k_free(Memory *,void *);

    #define vm_pointer_to_native(MEMORY, POINTER, TYPE) ( (TYPE) (MEMORY->main_memory + (vm_pointer_t)(POINTER)) )
    #define native_to_vm_pointer(MEMORY, POINTER) ( (vm_pointer_t) ((unsigned char*)(POINTER) - MEMORY->main_memory) )
    #define vm_malloc(MEMORY, SIZE) ((vm_pointer_t) ((unsigned char *)k_malloc(MEMORY, SIZE) - MEMORY->main_memory))
    #define vm_realloc(MEMORY, POINTER, SIZE) ((vm_pointer_t) ((unsigned char*)k_realloc(MEMORY, vm_pointer_to_native(MEMORY, POINTER, void*), SIZE) - MEMORY->main_memory))
    #define vm_calloc(MEMORY, NUM, SIZE) ((vm_pointer_t) ((unsigned char *)k_calloc(MEMORY, NUM, SIZE) - MEMORY->main_memory))
    #define vm_free(MEMORY, POINTER) k_free(MEMORY, MEMORY->main_memory + POINTER)

#endif

void retain(CPU_State *state, vm_value_t *ptr);
void release(CPU_State *state, vm_value_t *ptr);
void release_pointer(CPU_State *state, enum vm_value_type_t type, vm_pointer_t ptr);
void retain_pointer(CPU_State *state, enum vm_value_type_t type, vm_pointer_t ptr);
char *cstr_pointer_from_vm_pointer_t(CPU_State* state, vm_pointer_t ptr);
char *cstr_pointer_from_vm_value(CPU_State* state, vm_value_t* val);

#endif //VM_MEMORY_H

#ifndef VM_MEMORY_H
#define VM_MEMORY_H

#include <stdlib.h>

#define PAGE_SIZE 4096

typedef struct {
    unsigned char* main_memory;
    unsigned char* bitmap;
    vm_type_t bitmap_size;
    int lock;
} Memory;

void memory_init(Memory *mem, unsigned char *main_memory);
void memory_print_bitmap_debug(Memory *mem);

extern void    *k_malloc(Memory *,size_t);				///< The standard function.
extern void    *k_realloc(Memory *,void *, size_t);		///< The standard function.
extern void    *k_calloc(Memory *,size_t, size_t);		///< The standard function.
extern void     k_free(Memory *,void *);	        	///< The standard function.

#define vm_pointer_to_native(MEMORY, POINTER, TYPE) ( (TYPE) (MEMORY->main_memory + (vm_pointer_t)(POINTER)) )
#define native_to_vm_pointer(MEMORY, POINTER) ( (vm_pointer_t) ((unsigned char*)(POINTER) - MEMORY->main_memory) )
#define vm_malloc(MEMORY, SIZE) ((vm_pointer_t) ((unsigned char *)k_malloc(MEMORY, SIZE) - MEMORY->main_memory))
#define vm_realloc(MEMORY, POINTER, SIZE) ((vm_pointer_t) ((unsigned char*)k_realloc(MEMORY, vm_pointer_to_native(MEMORY, POINTER, void*), SIZE) - MEMORY->main_memory))
#define vm_calloc(MEMORY, NUM, SIZE) ((vm_pointer_t) ((unsigned char *)k_calloc(MEMORY, NUM, SIZE) - MEMORY->main_memory))
#define vm_free(MEMORY, POINTER) k_free(MEMORY, MEMORY->main_memory + POINTER)

#endif //VM_MEMORY_H

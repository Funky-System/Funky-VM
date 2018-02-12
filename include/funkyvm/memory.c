#include <memory.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "funkyvm.h"
#include "memory.h"
#include "../../src/libvm/liballoc_1_1.h"
#include "cpu.h"
#include "../../src/libvm/error_handling.h"

#define SET_BIT(var, bit) var |= (1 << bit)
#define CLEAR_BIT(var, bit) var &= ~(1 << bit)
#define IS_BIT_1(var, bit) ((var >> bit) & 1)
#define IS_BIT_0(var, bit) (!IS_BIT_1(var, bit))

void memory_print_bitmap_debug(Memory *mem) {
    for (unsigned int i = 0; i < mem->bitmap_size / sizeof (unsigned long); i++) {
        printf("%4d: %016lX\n", i * 8, ((unsigned long*)mem->bitmap)[i]);
    }
}

vm_type_t bitshift = 0;

void memory_set_used(Memory* mem, vm_type_t addr) {
    vm_type_t i = addr >> bitshift;
    vm_type_t bit = addr / PAGE_SIZE % 8;

    CLEAR_BIT(mem->bitmap[i], bit);
}

void memory_set_unused(Memory* mem, vm_type_t addr) {
    vm_type_t i = addr >> bitshift;
    vm_type_t bit = addr / PAGE_SIZE % 8;

    SET_BIT(mem->bitmap[i], bit);
}

void memory_init(Memory *mem, unsigned char *main_memory) {
    assert(VM_MEMORY_LIMIT % PAGE_SIZE == 0); // multiple of page size
    assert(sizeof(vm_value_t) < PAGE_SIZE); // at least one vm_value_t must fit in a page

    mem->main_memory = main_memory;

    mem->bitmap_size = VM_MEMORY_LIMIT / 8 / PAGE_SIZE;

    unsigned char *bitmap = malloc(mem->bitmap_size);

    bitshift = (vm_type_t)log2(PAGE_SIZE * 8);

    mem->bitmap = bitmap;
    memset(bitmap, 0xFF, mem->bitmap_size);

    /*for (vm_type_t addr = 0; addr < kernel_size + PAGE_SIZE - 1; addr += PAGE_SIZE) {
        memory_set_used(mem, addr);
    }*/

    mem->lock = 0;
}

void memory_destroy(Memory *mem) {
    free(mem->bitmap);
}

int memory_is_free(Memory* mem, vm_type_t addr) {
    vm_type_t i = addr >> bitshift;
    vm_type_t bit = addr / PAGE_SIZE % 8;
    return IS_BIT_1(mem->bitmap[i], bit);
}

vm_type_t memory_alloc(Memory* mem, vm_type_t num_pages) {
    for (vm_type_t addr = 0; addr < VM_MEMORY_LIMIT; addr += PAGE_SIZE) {
        int found_chunk = 1;
        for (vm_type_t i = 0; i < num_pages; i++) {
            if (!memory_is_free(mem, addr + i * PAGE_SIZE)) {
                found_chunk = 0;
                break;
            }
        }
        if (found_chunk) {
            for (vm_type_t i = 0; i < num_pages; i++) {
                memory_set_used(mem, addr + i * PAGE_SIZE);
            }
            return addr;
        }
    }

    vm_error(NULL, "Memory exhausted");
    vm_exit(NULL, EXIT_FAILURE);
}

void memory_free(Memory *mem, vm_type_t addr, vm_type_t num_pages) {
    for (vm_type_t i = 0; i < num_pages; i++) {
        memory_set_unused(mem, addr + i * PAGE_SIZE);
    }
}

#define TRUE 1
#define FALSE 0

/** This function is supposed to lock the memory data structures. It
 * could be as simple as disabling interrupts or acquiring a spinlock.
 * It's up to you to decide.
 *
 * \return 0 if the lock was acquired successfully. Anything else is
 * failure.
 */
int liballoc_lock(Memory *mem) {
    if (mem->lock) {
        return FALSE;
    } else {
        mem->lock = 1;
        return TRUE;
    }
}

/** This function unlocks what was previously locked by the liballoc_lock
 * function.  If it disabled interrupts, it enables interrupts. If it
 * had acquiried a spinlock, it releases the spinlock. etc.
 *
 * \return 0 if the lock was successfully released.
 */
int liballoc_unlock(Memory *mem) {
    if (!mem->lock) {
        return FALSE;
    } else {
        mem->lock = 0;
        return TRUE;
    }
}

/** This is the hook into the local system which allocates pages. It
 * accepts an integer parameter which is the number of pages
 * required.  The page size was set up in the liballoc_init function.
 *
 * \return NULL if the pages were not allocated.
 * \return A pointer to the allocated memory.
 */
void* liballoc_alloc(Memory *mem, size_t num_pages) {
    return mem->main_memory + memory_alloc(mem, (vm_type_t)num_pages);
}

/** This frees previously allocated memory. The void* parameter passed
 * to the function is the exact same value returned from a previous
 * liballoc_alloc call.
 *
 * The integer value is the number of pages to free.
 *
 * \return 0 if the memory was successfully freed.
 */
int liballoc_free(Memory *mem, void* ptr, size_t num_pages) {
    memory_free(mem, (vm_type_t) (((unsigned char*)ptr) - mem->main_memory), (vm_type_t)num_pages);
    return 0;
}



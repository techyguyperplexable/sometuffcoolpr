#include "heap.h"
#include "mm/vm.h"

extern uint32_t kernel_end;   // defined in linker

static uint32_t heap_start;
static uint32_t heap_end;
static uint32_t heap_current;

#define HEAP_SIZE 1024 * 1024  // 1MB heap for now

typedef struct block_header
{
    size_t size;
    int free;
    struct block_header* next;
} block_header_t;

static block_header_t* heap_head = NULL;

void heap_init()
{
    heap_start   = KERNEL_HEAP_START;
    heap_end     = KERNEL_HEAP_START;
    heap_current = KERNEL_HEAP_START;

}


void* kmalloc(size_t size)
{
    // Align to 8 bytes
    size = (size + 7) & ~7;

    block_header_t* current = heap_head;

    if (!heap_head)
    {
        heap_head = (block_header_t*)heap_end;

        heap_head->size = size;
        heap_head->free = 0;
        heap_head->next = NULL;

        heap_end += sizeof(block_header_t) + size;

        if (heap_end >= KERNEL_HEAP_END)
            return NULL;

        return (void*)(heap_head + 1);
    }



    while (current)
    {
        if (current->free && current->size >= size)
        {
            current->free = 0;
            return (void*)(current + 1);
        }

        if (!current->next)
            break;

        current = current->next;
    }

    block_header_t* new_block = (block_header_t*)heap_end;

    new_block->size = size;
    new_block->free = 0;
    new_block->next = NULL;

    current->next = new_block;

    uint32_t new_end = heap_end + sizeof(block_header_t) + size;

    if (new_end < heap_end)   // overflow check
        return NULL;

    if (new_end >= KERNEL_HEAP_END)
        return NULL;

    heap_end = new_end;

}


void kfree(void* ptr)
{
    if (!ptr)
        return;

    block_header_t* block = (block_header_t*)ptr - 1;
    block->free = 1;
}



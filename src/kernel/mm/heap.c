#include "heap.h"
#include "mm/vm.h"
#include "video/vga.h"


extern uint32_t kernel_end;   // defined in linker

static uint32_t heap_start;
static uint32_t heap_end;
static uint32_t heap_current;

#define HEAP_SIZE 1024 * 1024  // 1MB heap for now
#define MIN_BLOCK_SIZE 8

typedef struct block_header
{
    size_t size;
    int free;
    struct block_header* next;
    struct block_header* prev;
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
    size = (size + 7) & ~7;  // 8-byte alignment

    block_header_t* current = heap_head;

    // First allocation
    if (!heap_head)
    {
        heap_head = (block_header_t*)heap_end;

        heap_head->size = size;
        heap_head->free = 0;
        heap_head->next = NULL;
        heap_head->prev = NULL;

        heap_end += sizeof(block_header_t) + size;

        if (heap_end >= KERNEL_HEAP_END)
            return NULL;

        return (void*)(heap_head + 1);
    }

    // Search free block
    while (current)
    {
        if (current->free && current->size >= size)
        {
            // Split if large enough
            if (current->size >= size + sizeof(block_header_t) + MIN_BLOCK_SIZE)
            {
                block_header_t* new_block =
                    (block_header_t*)((char*)(current + 1) + size);

                new_block->size = current->size - size - sizeof(block_header_t);
                new_block->free = 1;
                new_block->next = current->next;
                new_block->prev = current;

                if (new_block->next)
                    new_block->next->prev = new_block;

                current->size = size;
                current->next = new_block;
            }

            current->free = 0;
            return (void*)(current + 1);
        }

        if (!current->next)
            break;

        current = current->next;
    }

    // Append new block
    block_header_t* new_block = (block_header_t*)heap_end;

    new_block->size = size;
    new_block->free = 0;
    new_block->next = NULL;
    new_block->prev = current;

    current->next = new_block;

    uint32_t new_end = heap_end + sizeof(block_header_t) + size;

    if (new_end < heap_end)
        return NULL;

    if (new_end >= KERNEL_HEAP_END)
        return NULL;

    heap_end = new_end;

    return (void*)(new_block + 1);
}



void kfree(void* ptr)
{
    if (!ptr)
        return;

    block_header_t* block = (block_header_t*)ptr - 1;

    if (block->free)
    {
        vga_print("Double free detected!\n");
        return;
    }

    block->free = 1;

    // Merge with next
    if (block->next && block->next->free)
    {
        block->size += sizeof(block_header_t) + block->next->size;
        block->next = block->next->next;

        if (block->next)
            block->next->prev = block;
    }

    // Merge with previous
    if (block->prev && block->prev->free)
    {
        block->prev->size += sizeof(block_header_t) + block->size;
        block->prev->next = block->next;

        if (block->next)
            block->next->prev = block->prev;
    }
}




void heap_dump(void)
{
    block_header_t* current = heap_head;
    uint32_t index = 0;

    if (!current)
    {
        vga_print("Heap empty.\n");
        return;
    }

    vga_print("Heap blocks:\n");

    while (current)
    {
        vga_print("  Block ");
        vga_print_dec(index);
        vga_print(": addr=");
        vga_print_hex((uint32_t)current);

        vga_print(" size=");
        vga_print_dec(current->size);

        vga_print(" header=");
        vga_print_hex((uint32_t)current);
        vga_print(" payload=");
        vga_print_hex((uint32_t)(current + 1));


        vga_print(" ");
        if (current->free)
            vga_print("FREE");
        else
            vga_print("USED");

        vga_print("\n");

        current = current->next;
        index++;
    }
}

void heap_validate(void)
{
    block_header_t* current = heap_head;

    while (current)
    {
        if (current->next && current->next->prev != current)
        {
            vga_print("Heap corruption detected!\n");
            return;
        }

        current = current->next;
    }

    vga_print("Heap structure valid.\n");
}

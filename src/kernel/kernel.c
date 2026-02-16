#include "core/boot.h"
#include "drivers/keyboard.h"

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_addr)
{
    kernel_boot(multiboot_magic, multiboot_addr);

    for (int i = 0; i < 1000; i++)
    {
        int* x = kmalloc(sizeof(int));
        *x = i;
    }


    vga_print("We survived demand paging.\n");

    while (1)
    {
        asm volatile("hlt");
    }
}

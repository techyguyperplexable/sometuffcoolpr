#include "core/boot.h"
#include "drivers/keyboard.h"
#include "core/debug.h"

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_addr)
{
    kernel_boot(multiboot_magic, multiboot_addr);

    int x = *(int*)0;


    while (1)
    {
        asm volatile("hlt");
    }
}

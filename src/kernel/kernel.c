#include "core/boot.h"
#include "drivers/keyboard.h"
#include "core/debug.h"
#include "shell/shell.h"

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_addr)
{
    kernel_boot(multiboot_magic, multiboot_addr);

    shell_init();

    void* ptrs[128];

    for (int i = 0; i < 128; i++)
    {
        ptrs[i] = kmalloc(64);
    }

    for (int i = 0; i < 128; i += 2)
    {
        kfree(ptrs[i]);   // free every other block
    }

    for (int i = 0; i < 64; i++)
    {
        kmalloc(32);      // try to reuse fragmented space
    }


    while (1)
    {   
        shell_update();
        asm volatile("hlt");
    }
}

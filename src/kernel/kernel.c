#include "core/boot.h"
#include "drivers/keyboard.h"
#include "core/debug.h"
#include "shell/shell.h"

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_addr)
{
    kernel_boot(multiboot_magic, multiboot_addr);

    shell_init();

    while (1)
    {   
        shell_update();
        asm volatile("hlt");
    }
}

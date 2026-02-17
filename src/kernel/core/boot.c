#include "boot.h"

#include "core/idt.h"
#include "core/debug.h"
#include "core/console.h"
#include "core/multiboot.h"
#include "core/status.h"
#include "time.h"

#include "drivers/pic.h"
#include "drivers/pit.h"
#include "drivers/keyboard.h"

#include "mm/heap.h"
#include "mm/paging.h"
#include "mm/pmm.h"

#include "include/system.h"

#include "video/vga.h"

void kernel_boot(uint32_t multiboot_magic, uint32_t multiboot_addr)
{
    vga_clear();
    multiboot_print_memory(multiboot_addr);                


    status_begin("Initializing PMM");

    int pmm_result = pmm_init(multiboot_addr);

    if (pmm_result == 0)
    {
        status_end_ok();
    }
    else
    {
        status_end_error();

        if (pmm_result == 1) // Missing memory map
            debug_panic("PMM failed: Missing memory map");

        if (pmm_result == 2) // Out of physical Memory
            debug_panic("PMM failed: Out of physical memory");

        debug_panic("PMM failed: Unknown error");
    }

    status_begin("Initializing IDT");
    idt_init();
    status_end_ok();

    status_begin("Remapping PIC");
    pic_remap();
    status_end_ok();

    status_begin("Setting hardware time frequency to 100Hz");
    pit_init(100);
    status_end_ok();

    status_begin("Initializing Paging");
    paging_init();
    status_end_ok();

    status_begin("Initializing Heap");
    heap_init();
    status_end_ok();

    status_begin("Enabling Interrupts");
    asm volatile("sti");
    status_end_ok();

    status_begin("Initializing Console");
    console_init();
    status_end_ok();

    status_begin("Initializing Keyboard");
    keyboard_init();
    status_end_ok();

    vga_set_color(0xC);
    vga_print(boot_logo);
    vga_set_color(0x07);


}

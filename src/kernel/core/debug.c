#include "debug.h"
#include "video/vga.h"

static void print_hex16(uint16_t val) {
    char hex[] = "0123456789ABCDEF";
    char buf[7];

    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = hex[(val >> 12) & 0xF];
    buf[3] = hex[(val >> 8) & 0xF];
    buf[4] = hex[(val >> 4) & 0xF];
    buf[5] = hex[val & 0xF];
    buf[6] = '\0';

    vga_print(buf);
}

uint16_t debug_get_cs() {
    uint16_t val;
    asm volatile("mov %%cs, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_ds() {
    uint16_t val;
    asm volatile("mov %%ds, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_es() {
    uint16_t val;
    asm volatile("mov %%es, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_fs() {
    uint16_t val;
    asm volatile("mov %%fs, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_gs() {
    uint16_t val;
    asm volatile("mov %%gs, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_ss() {
    uint16_t val;
    asm volatile("mov %%ss, %0" : "=r"(val));
    return val;
}

uint32_t debug_get_esp()
{
    uint32_t val;
    asm volatile("mov %%esp, %0" : "=r"(val));

    return val;
}

uint32_t debug_get_eflags()
{
    uint32_t val;
    // asm volatile("pushf; pop %0" : "=r"(val));
    return val; 
}


uint32_t debug_get_cr2()
{
    uint32_t val;
    asm volatile("mov %%cr2, %0" : "=r"(val));
    return val;
}

uint32_t debug_get_cr3()
{
    uint32_t val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}


void debug_print_segments()
{
    vga_print("=========== CPU STATE ===========\n");

    // Segment registers (2-line compact layout)
    vga_print("Segments:\n");

    vga_print("  CS: "); print_hex16(debug_get_cs());
    vga_print("  DS: "); print_hex16(debug_get_ds());
    vga_print("  ES: "); print_hex16(debug_get_es());
    vga_print("\n");

    vga_print("  FS: "); print_hex16(debug_get_fs());
    vga_print("  GS: "); print_hex16(debug_get_gs());
    vga_print("  SS: "); print_hex16(debug_get_ss());
    vga_print("\n\n");

    // Stack pointer
    vga_print("ESP: ");
    vga_print_hex(debug_get_esp());
    vga_print("\n");

    // Flags
    // vga_print("EFLAGS: ");
    // vga_print_hex(debug_get_eflags());
    // vga_print("\n");

    // Paging info
    vga_print("CR2 (Fault Addr): ");
    vga_print_hex(debug_get_cr2());
    vga_print("\n");

    vga_print("CR3 (Page Dir):  ");
    vga_print_hex(debug_get_cr3());
    vga_print("\n");

    vga_print("=================================\n");
}



void debug_panic(const char* message)
{
    asm volatile("cli");   // Disable interrupts

    vga_set_color(0x4F);   // White on red 
    vga_clear();

    vga_print("========================================\n");
    vga_set_color(0xCF); // white on red + blinking
    vga_print("             KERNEL PANIC               \n");
    vga_set_color(0x4F);
    vga_print("========================================\n\n");

    vga_set_color(0x4F); 

    vga_print("Reason: ");
    vga_print(message);
    vga_print("\n\n");

    vga_print("CPU Segment Registers:\n");
    debug_print_segments();
    vga_print("\n");

    vga_print("System Halted.\n");

    while (1)
        asm volatile("hlt");
}
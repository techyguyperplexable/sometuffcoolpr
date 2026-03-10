#include "debug.h"
#include "video/vga.h"

// ----------------------------------------------------------------------------
// CPU Register Getters (Forced Inline for Accuracy)
// ----------------------------------------------------------------------------

static inline __attribute__((always_inline)) uint16_t debug_get_cs() {
    uint16_t val;
    asm volatile("mov %%cs, %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) uint16_t debug_get_ds() {
    uint16_t val;
    asm volatile("mov %%ds, %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) uint16_t debug_get_es() {
    uint16_t val;
    asm volatile("mov %%es, %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) uint16_t debug_get_fs() {
    uint16_t val;
    asm volatile("mov %%fs, %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) uint16_t debug_get_gs() {
    uint16_t val;
    asm volatile("mov %%gs, %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) uint16_t debug_get_ss() {
    uint16_t val;
    asm volatile("mov %%ss, %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) uint32_t debug_get_esp() {
    uint32_t val;
    asm volatile("mov %%esp, %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) uint32_t debug_get_eip() {
    // Grabs the return address of the caller
    return (uint32_t)__builtin_return_address(0);
}

static inline __attribute__((always_inline)) uint32_t debug_get_cr2() {
    uint32_t val;
    asm volatile("mov %%cr2, %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) uint32_t debug_get_cr3() {
    uint32_t val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) uint16_t debug_get_tr() {
    uint16_t val;
    asm volatile("str %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) void debug_get_gdtr(uint32_t* base, uint16_t* limit) {
    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) gdtr;

    asm volatile("sgdt %0" : "=m"(gdtr));

    *base = gdtr.base;
    *limit = gdtr.limit;
}

static inline __attribute__((always_inline)) void debug_get_idtr(uint32_t* base, uint16_t* limit) {
    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) idtr;

    asm volatile("sidt %0" : "=m"(idtr));

    *base = idtr.base;
    *limit = idtr.limit;
}

static inline __attribute__((always_inline)) uint32_t debug_get_cr0() {
    uint32_t val;
    asm volatile("mov %%cr0, %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) uint32_t debug_get_cr4() {
    uint32_t val;
    asm volatile("mov %%cr4, %0" : "=r"(val));
    return val;
}

static inline __attribute__((always_inline)) uint32_t debug_get_eflags() {
    uint32_t val;
    // Use pushfd for full 32-bit flags
    asm volatile(
        "pushfd\n\t"
        "pop %0"
        : "=r"(val)
    );
    return val;
}

// ----------------------------------------------------------------------------
// General Purpose Debug Output
// ----------------------------------------------------------------------------

void debug_print_segments() {
    vga_print("==== CPU STATE ====\n");

    uint16_t cs = debug_get_cs();
    uint16_t ds = debug_get_ds();
    uint16_t es = debug_get_es();
    uint16_t fs = debug_get_fs();
    uint16_t gs = debug_get_gs();
    uint16_t ss = debug_get_ss();
    uint16_t tr = debug_get_tr();

    uint32_t cr0 = debug_get_cr0();
    uint32_t cr2 = debug_get_cr2();
    uint32_t cr3 = debug_get_cr3();
    uint32_t cr4 = debug_get_cr4();
    uint32_t eflags = debug_get_eflags();
    uint32_t esp = debug_get_esp();
    uint32_t eip = debug_get_eip();

    uint32_t gdtr_base, idtr_base;
    uint16_t gdtr_limit, idtr_limit;

    debug_get_gdtr(&gdtr_base, &gdtr_limit);
    debug_get_idtr(&idtr_base, &idtr_limit);

    // Segments (Using the standard vga_print_hex)
    vga_print("CS: "); vga_print_hex((uint32_t)cs);
    vga_print(" DS: "); vga_print_hex((uint32_t)ds);
    vga_print(" ES: "); vga_print_hex((uint32_t)es);
    vga_print(" FS: "); vga_print_hex((uint32_t)fs);
    vga_print(" GS: "); vga_print_hex((uint32_t)gs);
    vga_print(" SS: "); vga_print_hex((uint32_t)ss);
    vga_print(" TR: "); vga_print_hex((uint32_t)tr);
    vga_print("\n");

    // Code & Stack + Flags
    vga_print("EIP: "); vga_print_hex(eip);
    vga_print(" ESP: "); vga_print_hex(esp);
    vga_print(" EFLAGS: "); vga_print_hex(eflags);
    vga_print(" CPL: "); vga_print_dec(cs & 0x3);
    vga_print("\n");

    // Control Registers
    vga_print("CR0: "); vga_print_hex(cr0);
    vga_print(" CR2: "); vga_print_hex(cr2);
    vga_print(" CR3: "); vga_print_hex(cr3);
    vga_print(" CR4: "); vga_print_hex(cr4);
    vga_print("\n");

    // Descriptor Tables
    vga_print("GDTR: "); vga_print_hex(gdtr_base);
    vga_print("/"); vga_print_hex((uint32_t)gdtr_limit);

    vga_print(" IDTR: "); vga_print_hex(idtr_base);
    vga_print("/"); vga_print_hex((uint32_t)idtr_limit);
    vga_print("\n");

    // Mode flags
    vga_print("PM:");
    vga_print((cr0 & 0x1) ? "1 " : "0 ");

    vga_print("PG:");
    vga_print((cr0 & 0x80000000) ? "1" : "0");

    vga_print("\n====================\n");
}

// ----------------------------------------------------------------------------
// Standard Panic (Called manually by the kernel)
// ----------------------------------------------------------------------------

void debug_panic(const char* message) {
    asm volatile("cli");   // Disable interrupts

    vga_set_color(0x4F);   // White on red 
    vga_clear();

    vga_print("================================================================================\n");
    vga_set_color(0xCF); 
    vga_print("                                 KERNEL PANIC                                   \n");
    vga_set_color(0x4F);
    vga_print("================================================================================\n\n");

    vga_print("Reason: ");
    vga_print(message);
    vga_print("\n\n");

    vga_print("CPU Registers at panic call:\n");
    debug_print_segments();
    vga_print("\n");

    vga_print("System Halted.\n");

    // Ensure interrupts stay disabled if an NMI wakes the CPU
    while (1) {
        asm volatile("cli; hlt");
    }
}

// ----------------------------------------------------------------------------
// Interrupt Panic (Called by ISRs, uses strict stack state)
// ----------------------------------------------------------------------------
/* * NOTE: Move this struct to a header file (like isr.h or debug.h) 
 * so your interrupt handlers can see it. 
 */
typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;          // Pushed by CPU
} registers_t;

void debug_panic_from_exception(registers_t *regs, const char* message) {
    asm volatile("cli");

    vga_set_color(0x4F);
    vga_clear();

    vga_print("================================================================================\n");
    vga_set_color(0xCF); 
    vga_print("                             UNHANDLED EXCEPTION                                \n");
    vga_set_color(0x4F);
    vga_print("================================================================================\n\n");

    vga_print("Exception: ");
    vga_print(message);
    vga_print(" (Int No: "); vga_print_hex(regs->int_no);
    vga_print(" Err Code: "); vga_print_hex(regs->err_code);
    vga_print(")\n\n");

    vga_print("EIP: "); vga_print_hex(regs->eip);
    vga_print(" CS: "); vga_print_hex(regs->cs);
    vga_print(" EFLAGS: "); vga_print_hex(regs->eflags);
    vga_print("\n");

    vga_print("EAX: "); vga_print_hex(regs->eax);
    vga_print(" EBX: "); vga_print_hex(regs->ebx);
    vga_print(" ECX: "); vga_print_hex(regs->ecx);
    vga_print(" EDX: "); vga_print_hex(regs->edx);
    vga_print("\n");
    
    // Add more register prints as needed based on the struct!

    vga_print("\nSystem Halted.\n");

    while (1) {
        asm volatile("cli; hlt");
    }
}
    asm volatile("mov %%cr2, %0" : "=r"(val));
    return val;
}

uint32_t debug_get_cr3()
{
    uint32_t val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

uint16_t debug_get_tr()
{
    uint16_t val;
    asm volatile("str %0" : "=r"(val));
    return val;
}

void debug_get_gdtr(uint32_t* base, uint16_t* limit)
{
    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) gdtr;

    asm volatile("sgdt %0" : "=m"(gdtr));

    *base = gdtr.base;
    *limit = gdtr.limit;
}

void debug_get_idtr(uint32_t* base, uint16_t* limit)
{
    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) idtr;

    asm volatile("sidt %0" : "=m"(idtr));

    *base = idtr.base;
    *limit = idtr.limit;
}

uint32_t debug_get_cr0()
{
    uint32_t val;
    asm volatile("mov %%cr0, %0" : "=r"(val));
    return val;
}

uint32_t debug_get_cr4()
{
    uint32_t val;
    asm volatile("mov %%cr4, %0" : "=r"(val));
    return val;
}

uint32_t debug_get_eflags()
{
    uint32_t val;

    asm volatile(
        "pushf\n\t"
        "pop %0"
        : "=r"(val)
    );

    return val;
}



void debug_print_segments()
{
    vga_print("==== CPU STATE ====\n");

    uint16_t cs = debug_get_cs();
    uint16_t ds = debug_get_ds();
    uint16_t es = debug_get_es();
    uint16_t fs = debug_get_fs();
    uint16_t gs = debug_get_gs();
    uint16_t ss = debug_get_ss();
    uint16_t tr = debug_get_tr();

    uint32_t cr0 = debug_get_cr0();
    uint32_t cr2 = debug_get_cr2();
    uint32_t cr3 = debug_get_cr3();
    uint32_t cr4 = debug_get_cr4();
    uint32_t eflags = debug_get_eflags();
    uint32_t esp = debug_get_esp();

    uint32_t gdtr_base, idtr_base;
    uint16_t gdtr_limit, idtr_limit;

    debug_get_gdtr(&gdtr_base, &gdtr_limit);
    debug_get_idtr(&idtr_base, &idtr_limit);

    // Segments
    vga_print("CS: "); print_hex16(cs);
    vga_print(" DS: "); print_hex16(ds);
    vga_print(" ES: "); print_hex16(es);
    vga_print(" FS: "); print_hex16(fs);
    vga_print(" GS: "); print_hex16(gs);
    vga_print(" SS: "); print_hex16(ss);
    vga_print(" TR: "); print_hex16(tr);
    vga_print("\n");

    // Stack + Flags
    vga_print("ESP: "); vga_print_hex(esp);
    vga_print(" EFLAGS: "); vga_print_hex(eflags);
    vga_print(" CPL: "); vga_print_dec(cs & 0x3);
    vga_print("\n");

    // Control Registers
    vga_print("CR0: "); vga_print_hex(cr0);
    vga_print(" CR2: "); vga_print_hex(cr2);
    vga_print(" CR3: "); vga_print_hex(cr3);
    vga_print(" CR4: "); vga_print_hex(cr4);
    vga_print("\n");

    // Descriptor Tables
    vga_print("GDTR: "); vga_print_hex(gdtr_base);
    vga_print("/"); vga_print_hex(gdtr_limit);

    vga_print(" IDTR: "); vga_print_hex(idtr_base);
    vga_print("/"); vga_print_hex(idtr_limit);
    vga_print("\n");

    // Mode flags (super compact)
    vga_print("PM:");
    vga_print((cr0 & 0x1) ? "1 " : "0 ");

    vga_print("PG:");
    vga_print((cr0 & 0x80000000) ? "1" : "0");

    vga_print("\n====================\n");
}





void debug_panic(const char* message)
{
    asm volatile("cli");   // Disable interrupts

    vga_set_color(0x4F);   // White on red 
    vga_clear();

    vga_print("================================================================================\n");
    vga_set_color(0xCF); // white on red + blinking (or bright bg on QEMU)
    vga_print("                                 KERNEL PANIC                                   \n");
    vga_set_color(0x4F);
    vga_print("================================================================================\n\n");

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

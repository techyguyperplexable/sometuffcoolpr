#include "shell.h"
#include "drivers/keyboard.h"
#include "core/console.h"
#include "video/vga.h"
#include "core/debug.h"
#include "lib/string.h"
#include <stddef.h>
#include "core/time.h"
#include "arch/x86/cpuid.h"
#include "mm/pmm.h"
#include "mm/heap.h"
#include "include/system.h"

#define MAX_ARGS 8

static void execute_command(char* input);
static void parse_arguments(char* input, char* argv[], int* argc);

static void* last_alloc = NULL;

int mode = 0; // 0 = kernel, 1 = root, 2 = user

static void cmd_heapval(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    heap_validate();
}

static void cmd_clear(int argc, char* argv[])
{
    vga_clear();
}

static void cmd_memdump(int argc, char* argv[])
{
    if (argc < 3)
    {
        vga_print("Usage: memdump <addr> <bytes>\n");
        return;
    }

    uint32_t addr = atoi_hex(argv[1]);
    int bytes = atoi(argv[2]);

    if (bytes <= 0)
    {
        vga_print("Invalid byte count\n");
        return;
    }

    uint8_t* ptr = (uint8_t*)addr;

    for (int i = 0; i < bytes; i++)
    {
        if (i % 16 == 0)
        {
            vga_print("\n");
            vga_print_hex((uint32_t)(ptr + i));
            vga_print(": ");
        }

        uint8_t val = ptr[i];

        if (val < 16)
            vga_print("0");

        vga_print_hex(val);
        vga_print(" ");
    }

    vga_print("\n");
}

static void cmd_free(int argc, char* argv[])
{
    if (argc < 2)
    {
        vga_print("Usage: free <address>\n");
        return;
    }

    void* ptr = (void*)atoi_hex(argv[1]);

    if (!ptr)
    {
        vga_print("Invalid address\n");
        return;
    }

    kfree(ptr);

    vga_print("Freed: ");
    vga_print_hex((uint32_t)ptr);
    vga_print("\n");
}


static void cmd_freelast(int argc, char* argv[])
{
    (void)argc; (void)argv;

    if (!last_alloc)
    {
        vga_print("No previous allocation\n");
        return;
    }

    kfree(last_alloc);

    vga_print("Freed: ");
    vga_print_hex((uint32_t)last_alloc);
    vga_print("\n");

    last_alloc = NULL;
}


static void cmd_alloc(int argc, char* argv[])
{
    if (argc < 2)
    {
        vga_print("Usage: alloc <size>\n");
        return;
    }

    int size = atoi(argv[1]);

    if (size <= 0)
    {
        vga_print("Invalid size\n");
        return;
    }

    void* ptr = kmalloc((size_t)size);

    if (!ptr)
    {
        vga_print("Allocation failed\n");
        return;
    }

    last_alloc = ptr;

    vga_print("Allocated at: ");
    vga_print_hex((uint32_t)ptr);
    vga_print("\n");
}





static void cmd_heapinfo(int argc, char* argv[])
{
    (void)argc; (void)argv;
    heap_dump();
}


static void cmd_meminfo(int argc, char* argv[])
{
    (void)argc; (void)argv;

    uint32_t total = pmm_get_total_frames();
    uint32_t free  = pmm_get_free_frames();

    vga_print("Physical Memory:\n");
    vga_print("  Total frames: ");
    vga_print_dec(total);
    vga_print("\n");

    vga_print("  Free frames:  ");
    vga_print_dec(free);
    vga_print("\n");

    vga_print("  Used frames:  ");
    vga_print_dec(total - free);
    vga_print("\n");
}


static void cmd_cpuid(int argc, char* argv[])
{
    (void)argc; (void)argv;

    static char brand[49];

    cpu_get_brand(brand);

    vga_print("CPU: ");
    vga_print(brand);
    vga_print("\n");
}


static void cmd_uptime(int argc, char* argv[])
{
    (void)argc; (void)argv;

    uint32_t secs = timer_get_seconds();

    vga_print("Uptime: ");
    vga_print_dec(secs);
    vga_print(" seconds\n");
}


static void cmd_help(int argc, char* argv[])
{
    (void)argc; (void)argv;

    vga_print("Available commands:\n\n");

    vga_print("System:\n");
    vga_print("  help            - Show this message\n");
    vga_print("  panic           - Trigger kernel panic\n");
    vga_print("  state           - Print CPU state\n");
    vga_print("  cpuid           - Print CPU info\n");
    vga_print("  uptime          - Print OS uptime\n\n");

    vga_print("Memory:\n");
    vga_print("  meminfo         - Show physical memory info\n");
    vga_print("  heapinfo        - Show heap layout\n");
    vga_print("  heapval         - Validate heap integrity\n");
    vga_print("  alloc <size>    - Allocate memory\n");
    vga_print("  free <addr>     - Free memory at address\n");
    vga_print("  freelast        - Free last allocation\n");
    vga_print("  memdump <a> <b> - Dump memory\n");
}


static void cmd_sysfetch(int argc, char* argv[])
{
    (void)argc; (void)argv;

    static char brand[49];
    cpu_get_brand(brand);

    uint32_t total = pmm_get_total_frames();
    uint32_t free  = pmm_get_free_frames();
    uint32_t secs  = timer_get_seconds();

    vga_print_color(boot_logo, 0xC);

    vga_print_color("Kernel build date: ", 0xC);
    vga_print_color(KERNEL_BUILD_DATE, 0xC);
    vga_print_color("\nVersion: ", 0xC);
    vga_print_color(KERNEL_VERSION, 0xC);

    vga_print("\n\nCPU:        ");
    vga_print(brand);
    vga_print("\n");

    vga_print("Uptime:     ");
    vga_print_dec(secs);
    vga_print(" seconds\n");

    vga_print("Memory:     ");
    vga_print_dec(total - free);
    vga_print("/");
    vga_print_dec(total);
    vga_print(" frames used\n");

    vga_print("Mode:       ");

    if (mode == 0)
        vga_print("kernel\n");
    else if (mode == 1)
        vga_print("root\n");
    else
        vga_print("user\n");
}



static void cmd_panic(int argc, char* argv[])
{
    if (argc < 2)
    {
        debug_panic("Manual panic triggered");
        return;
    }

    static char buffer[128];
    buffer[0] = '\0';

    for (int i = 1; i < argc; i++)
    {
        if (i > 1)
            strcat(buffer, " ");

        strcat(buffer, argv[i]);
    }

    debug_panic(buffer);
}


static void cmd_state(int argc, char* argv[])
{
    (void)argc; (void)argv;
    debug_print_segments();
}

static void cmd_shutdown(int argc, char* argv[])
{
    (void)argc; (void)argv;

    vga_print("Shutting down...\n");

    // QEMU poweroff
    outw(0x604, 0x2000);

    sleep(1000);


    vga_print_color("Unable to shutdown\n", 0xC);
}

static void cmd_reboot(int argc, char* argv[])
{
    (void)argc; (void)argv;

    vga_print("Rebooting...\n");

    // QEMU reboot
    outb(0x64, 0xFE);

    sleep(1000);

    vga_print_color("Unable to reboot\n", 0xC);
}


typedef struct {
    const char* name;
    void (*handler)(int argc, char* argv[]);
} command_t;

static command_t commands[] = {
    {"help",  cmd_help},
    {"panic", cmd_panic},
    {"state", cmd_state},
    {"uptime", cmd_uptime},
    {"cpuid", cmd_cpuid},
    {"meminfo", cmd_meminfo},
    {"heapinfo", cmd_heapinfo},
    {"alloc", cmd_alloc},
    {"free", cmd_free},
    {"freelast", cmd_freelast},
    {"memdump", cmd_memdump},
    {"heapval", cmd_heapval},
    {"sysfetch", cmd_sysfetch},
    {"shutdown", cmd_shutdown},  // FIXME: Only for QEMU
    {"reboot", cmd_reboot},      // FIXME: Only for QEMU
    {"clear", cmd_clear},
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))


void shell_init(void)
{
    if (mode == 0)
    {
        vga_print_color("kernel", 0xC);
        vga_print("> ");
    }
    else if (mode == 1)
    {
        vga_print_color("root", 0xE);
        vga_print("> ");
    }
    else if (mode == 2)
    {
        vga_print_color("user", 0x9);
        vga_print("> ");
    }
}

void shell_update(void)
{
    while (keyboard_has_input())
    {
        char c = keyboard_getchar();

        if (c == '\n')
        {
            char* input = (char*)console_get_line();

            vga_putchar('\n');

            execute_command(input);

            console_clear_line();
            if (mode == 0)
            {
                vga_print_color("kernel", 0xC);
                vga_print("> ");
            }
            else if (mode == 1)
            {
                vga_print_color("root", 0xE);
                vga_print("> ");
            }
            else if (mode == 2)
            {
                vga_print_color("user", 0x9);
                vga_print("> ");
            }
        }
        else
        {
            console_handle_char(c);
        }
    }
}


static void execute_command(char* input)
{
    char local_buffer[256];

    strcpy(local_buffer, input);

    char* argv[MAX_ARGS];
    int argc = 0;

    parse_arguments(local_buffer, argv, &argc);

    if (argc == 0)
        return;

    for (size_t i = 0; i < COMMAND_COUNT; i++)
    {
        if (strcmp(argv[0], commands[i].name) == 0)
        {
            commands[i].handler(argc, argv);
            return;
        }
    }

    vga_print("Unknown command\n");
}


static void parse_arguments(char* input, char* argv[], int* argc)
{
    *argc = 0;

    while (*input && *argc < MAX_ARGS)
    {
        // Skip leading spaces
        while (*input == ' ')
            input++;

        if (*input == '\0')
            break;

        argv[*argc] = input;
        (*argc)++;

        // Move until next space
        while (*input && *input != ' ')
            input++;

        if (*input)
        {
            *input = '\0';
            input++;
        }
    }
}

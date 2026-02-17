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


#define MAX_ARGS 8

static void execute_command(char* input);
static void parse_arguments(char* input, char* argv[], int* argc);

static void* last_alloc = NULL;

static void cmd_heapval(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    heap_validate();
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

    vga_print("Available commands:\n");
    vga_print("   help   - Show this message\n");
    vga_print("   panic  - Trigger kernel panic\n");
    vga_print("   state  - Print CPU state\n");
    vga_print("   cpuid  - Print Hardware info\n");
    vga_print("   uptime - Print OS uptime\n");
    vga_print("  meminfo - Print memory info\n");
    vga_print(" heapinfo - Print heap info\n");
    vga_print("alloc <size> - Allocate memory\n");
    vga_print("freelast  - frees last alloc\n");
    vga_print("free <addr>  - frees address\n");
    vga_print("memdump <addr> <bytes> - Dump raw memory\n");
    vga_print("  heapval - Prints validation of heap\n");
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
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))


void shell_init(void)
{
    vga_print("kernel> ");
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
            vga_print("kernel> ");
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

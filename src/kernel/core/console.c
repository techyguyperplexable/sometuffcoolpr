#include "console.h"
#include "video/vga.h"

/*
    This is your TTY input buffer.

    It stores what the user is typing.
    VGA just displays it.
*/

#define CONSOLE_BUFFER_SIZE 256

static char line_buffer[CONSOLE_BUFFER_SIZE];
static uint32_t line_length = 0;

void console_init(void)
{
    line_length = 0;

    for (uint32_t i = 0; i < CONSOLE_BUFFER_SIZE; i++) {
        line_buffer[i] = 0;
    }
}

/*
    This function interprets input characters.

    THIS is where backspace belongs.
    Not in keyboard driver.
    Not in VGA.
*/

void console_handle_char(char c)
{
    // =============================
    // BACKSPACE
    // =============================
    if (c == '\b')
    {
        if (line_length > 0)
        {
            line_length--;
            line_buffer[line_length] = 0;

            // Tell VGA to visually remove last char
            vga_putchar('\b');
        }
        return;
    }

    // =============================
    // NEWLINE (ENTER)
    // =============================
    if (c == '\n')
    {
        vga_putchar('\n');

        // Here later we can process command

        line_length = 0;
        return;
    }

    // =============================
    // NORMAL CHARACTER
    // =============================
    if (line_length < CONSOLE_BUFFER_SIZE - 1)
    {
        line_buffer[line_length++] = c;
        vga_putchar(c);
    }
}

const char* console_get_line(void)
{
    return line_buffer;
}

void console_clear_line(void)
{
    line_length = 0;
}

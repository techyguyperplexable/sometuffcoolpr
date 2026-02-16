#ifndef VGA_H
#define VGA_H

#include <stddef.h>
#include <stdint.h>

void vga_clear();
void vga_set_color(uint8_t color);
void vga_putchar(char c);
void vga_print(const char* str);
void vga_print_hex(uint32_t value); 
void vga_print_dec(uint32_t num);
void vga_print_color(const char* str, unsigned char color); 
size_t vga_get_column(void);


#endif

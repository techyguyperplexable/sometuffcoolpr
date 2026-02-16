#ifndef PMM_H
#define PMM_H

#include <stdint.h>

int pmm_init(uint32_t multiboot_addr);
void* pmm_alloc_frame();
void  pmm_free_frame(void* frame);

#endif

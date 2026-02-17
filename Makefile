BUILD_DIR = build
SRC_DIR   = src

CFLAGS  = -ffreestanding -m32 -Wall -Wextra -std=gnu99 -I$(SRC_DIR)/kernel
LDFLAGS = -m elf_i386 -T linker.ld

BOOT_SRC    = $(SRC_DIR)/boot
KERNEL_SRC  = $(SRC_DIR)/kernel
GRUB_SRC    = $(SRC_DIR)/grub

CORE_SRC    = $(KERNEL_SRC)/core
DRIVER_SRC  = $(KERNEL_SRC)/drivers
VIDEO_SRC   = $(KERNEL_SRC)/video
MM_SRC      = $(KERNEL_SRC)/mm
SHELL_SRC   = $(KERNEL_SRC)/shell
LIB_SRC     = $(KERNEL_SRC)/lib
ARCH_SRC	= $(KERNEL_SRC)/arch
X86_SRC     = $(ARCH_SRC)/x86

# =========================
# Boot Assembly Objects
# =========================
BOOT_OBJS = \
	$(BUILD_DIR)/boot.o \
	$(BUILD_DIR)/idt_load.o \
	$(BUILD_DIR)/isr_stubs.o \
	$(BUILD_DIR)/isr_common_stub.o

# =========================
# Kernel C Objects
# =========================
KERNEL_OBJS = \
	$(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/boot_core.o \
	$(BUILD_DIR)/idt.o \
	$(BUILD_DIR)/multiboot.o \
	$(BUILD_DIR)/debug.o \
	$(BUILD_DIR)/time.o \
	$(BUILD_DIR)/console.o \
	$(BUILD_DIR)/heap.o \
	$(BUILD_DIR)/pmm.o \
	$(BUILD_DIR)/paging.o \
	$(BUILD_DIR)/pic.o \
	$(BUILD_DIR)/pit.o \
	$(BUILD_DIR)/io.o \
	$(BUILD_DIR)/keyboard.o \
	$(BUILD_DIR)/vga.o \
	$(BUILD_DIR)/status.o \
	$(BUILD_DIR)/shell.o \
	$(BUILD_DIR)/string.o \
	$(BUILD_DIR)/cpuid.o


all: $(BUILD_DIR)/myos.iso

.PHONY: all clean

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# =========================
# Assembly
# =========================
$(BUILD_DIR)/%.o: $(BOOT_SRC)/%.s | $(BUILD_DIR)
	nasm -f elf32 $< -o $@


# =========================
# Libraries
# =========================

$(BUILD_DIR)/string.o: $(LIB_SRC)/string.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# =========================
# Core
# =========================
$(BUILD_DIR)/kernel.o: $(KERNEL_SRC)/kernel.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/idt.o: $(CORE_SRC)/idt.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/debug.o: $(CORE_SRC)/debug.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/time.o: $(CORE_SRC)/time.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/console.o: $(CORE_SRC)/console.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/multiboot.o: $(CORE_SRC)/multiboot.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/status.o: $(CORE_SRC)/status.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/boot_core.o: $(CORE_SRC)/boot.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@


# =========================
# Memory Management
# =========================
$(BUILD_DIR)/heap.o: $(MM_SRC)/heap.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/pmm.o: $(MM_SRC)/pmm.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/paging.o: $(MM_SRC)/paging.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# =========================
# Drivers
# =========================
$(BUILD_DIR)/pic.o: $(DRIVER_SRC)/pic.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/pit.o: $(DRIVER_SRC)/pit.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/io.o: $(DRIVER_SRC)/io.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/keyboard.o: $(DRIVER_SRC)/keyboard.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# =========================
# Video
# =========================
$(BUILD_DIR)/vga.o: $(VIDEO_SRC)/vga.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# =========================
# Arch x86
# =========================
$(BUILD_DIR)/cpuid.o: $(X86_SRC)/cpuid.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# =========================
# Shell
# =========================
$(BUILD_DIR)/shell.o: $(SHELL_SRC)/shell.c | $(BUILD_DIR)
	gcc $(CFLAGS) -c $< -o $@

# =========================
# Linking
# =========================
$(BUILD_DIR)/kernel.bin: $(BOOT_OBJS) $(KERNEL_OBJS)
	ld $(LDFLAGS) $^ -o $@

# =========================
# ISO
# =========================
$(BUILD_DIR)/myos.iso: $(BUILD_DIR)/kernel.bin
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	cp $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/iso/boot/
	cp $(GRUB_SRC)/grub.cfg $(BUILD_DIR)/iso/boot/grub/
	grub-mkrescue -o $@ $(BUILD_DIR)/iso

clean:
	rm -rf $(BUILD_DIR)

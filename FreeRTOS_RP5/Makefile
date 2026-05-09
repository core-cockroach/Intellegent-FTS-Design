#
#	Makefile for FreeRTOS demo on Raspberry Pi
#
BASE=$(shell pwd)/
BUILD_DIR=$(shell pwd)/build/

MODULE_NAME="RaspberryPi BSP"

TARGETS=kernel.img kernel.list kernel.syms kernel.elf rtos.bin
LINKER_SCRIPT=raspberrypi.ld

-include .dbuild/dbuild.mk


all: kernel.list kernel.img kernel.syms
	@$(SIZE) kernel.elf

kernel.img: kernel.elf
	$(Q)$(PRETTY) IMAGE $(MODULE_NAME) $@
	$(Q)$(OBJCOPY) kernel.elf -O binary $@

kernel.list: kernel.elf
	$(Q)$(PRETTY) LIST $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -D -S  kernel.elf > $@

kernel.syms: kernel.elf
	$(Q)$(PRETTY) SYMS $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -t kernel.elf > $@

# rtos.bin: kernel.elf
# 	$(Q)$(PRETTY) BINARY $(MODULE_NAME) $@
# 	$(Q)$(OBJDUMP) -O kernel.elf > $@

rtos.bin: kernel.elf
	$(Q)$(PRETTY) BINARY $(MODULE_NAME) $@
	$(Q)$(OBJCOPY) kernel.elf -O binary $@

LIBC_DIR  = $(dir $(shell arm-none-eabi-gcc -print-file-name=libc.a))
LIBGCC_DIR = $(dir $(shell arm-none-eabi-gcc -print-libgcc-file-name))

kernel.elf: LDFLAGS += -L  $(LIBGCC_DIR) -lgcc
kernel.elf: LDFLAGS += -L $(LIBC_DIR) -lc
kernel.elf: $(OBJECTS)
	$(Q)$(LD) $(OBJECTS) -Map kernel.map -o $@ -T $(LINKER_SCRIPT) $(LDFLAGS)


#kernel.elf: LDFLAGS += -L"c:/yagarto/lib/gcc/arm-none-eabi/4.7.1/" -lgcc
#kernel.elf: LDFLAGS += -L"c:/yagarto/arm-none-eabi/lib/" -lc
#"/usr/lib/gcc/arm-none-eabi/14.2.1" 
#"/usr/arm-none-eabi/lib"
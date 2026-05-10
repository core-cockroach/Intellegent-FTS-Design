# #
# #	Makefile for FreeRTOS demo on Raspberry Pi
# #
# BASE=$(shell pwd)/
# BUILD_DIR=$(shell pwd)/build/

# MODULE_NAME="RaspberryPi BSP"

# TARGETS=kernel.img kernel.list kernel.syms kernel.elf rtos.bin
# LINKER_SCRIPT=raspberrypi.ld

# -include .dbuild/dbuild.mk

# LDFLAGS = -T raspberrypi.ld -lc -lgcc -specs=nosys.specs

# all: kernel.list kernel.img kernel.syms
# 	@$(SIZE) kernel.elf

# kernel.img: kernel.elf
# 	$(Q)$(PRETTY) IMAGE $(MODULE_NAME) $@
# 	$(Q)$(OBJCOPY) kernel.elf -O binary $@

# kernel.list: kernel.elf
# 	$(Q)$(PRETTY) LIST $(MODULE_NAME) $@
# 	$(Q)$(OBJDUMP) -D -S  kernel.elf > $@

# kernel.syms: kernel.elf
# 	$(Q)$(PRETTY) SYMS $(MODULE_NAME) $@
# 	$(Q)$(OBJDUMP) -t kernel.elf > $@

# # rtos.bin: kernel.elf
# # 	$(Q)$(PRETTY) BINARY $(MODULE_NAME) $@
# # 	$(Q)$(OBJDUMP) -O kernel.elf > $@

# rtos.bin: kernel.elf
# 	$(Q)$(PRETTY) BINARY $(MODULE_NAME) $@
# 	$(Q)$(OBJCOPY) kernel.elf -O binary $@

# LIBC_DIR  = $(dir $(shell arm-none-eabi-gcc -print-file-name=libc.a))
# LIBGCC_DIR = $(dir $(shell arm-none-eabi-gcc -print-libgcc-file-name))

# kernel.elf: LDFLAGS += -L  $(LIBGCC_DIR) -lgcc
# kernel.elf: LDFLAGS += -L $(LIBC_DIR) -lc
# kernel.elf: $(OBJECTS)
# 	$(Q)$(LD) $(OBJECTS) -Map kernel.map -o $@ -T $(LINKER_SCRIPT) $(LDFLAGS)


# #kernel.elf: LDFLAGS += -L"c:/yagarto/lib/gcc/arm-none-eabi/4.7.1/" -lgcc
# #kernel.elf: LDFLAGS += -L"c:/yagarto/arm-none-eabi/lib/" -lc
# #"/usr/lib/gcc/arm-none-eabi/14.2.1" 
# #"/usr/arm-none-eabi/lib"


# #
# #   Makefile for FreeRTOS demo on Raspberry Pi
# #
# BASE=$(shell pwd)/
# BUILD_DIR=$(shell pwd)/build/

# MODULE_NAME="RaspberryPi BSP"

# TARGETS=kernel.img kernel.list kernel.syms kernel.elf rtos.bin
# LINKER_SCRIPT=raspberrypi.ld

# # Include the dark builder system (defines OBJECTS, Q, PRETTY, OBJCOPY, etc.)
# -include .dbuild/dbuild.mk

# # -----------------------------------------------------------------
# # Main targets
# # -----------------------------------------------------------------
# all: kernel.list kernel.img kernel.syms
# 	@$(SIZE) kernel.elf

# kernel.img: kernel.elf
# 	$(Q)$(PRETTY) IMAGE $(MODULE_NAME) $@
# 	$(Q)$(OBJCOPY) kernel.elf -O binary $@

# kernel.list: kernel.elf
# 	$(Q)$(PRETTY) LIST $(MODULE_NAME) $@
# 	$(Q)$(OBJDUMP) -D -S kernel.elf > $@

# kernel.syms: kernel.elf
# 	$(Q)$(PRETTY) SYMS $(MODULE_NAME) $@
# 	$(Q)$(OBJDUMP) -t kernel.elf > $@

# rtos.bin: kernel.elf
# 	$(Q)$(PRETTY) BINARY $(MODULE_NAME) $@
# 	$(Q)$(OBJCOPY) kernel.elf -O binary $@

# # -----------------------------------------------------------------
# # Link step – use gcc, not raw ld.  
# # The linker script must define the symbol 'end' after the .bss section.
# # -----------------------------------------------------------------
# kernel.elf: $(OBJECTS)
# 	$(Q)arm-none-eabi-gcc \
# 		-T $(LINKER_SCRIPT) \
# 		-Xlinker -Map -Xlinker kernel.map \
# 		-specs=nosys.specs \
# 		-nostartfiles \
# 		$(OBJECTS) \
# 		-lc -lgcc \
# 		-o $@

# # -----------------------------------------------------------------
# # Clean – ignore the harmless override warning
# # -----------------------------------------------------------------
# clean:
# 	rm -f kernel.elf kernel.img kernel.list kernel.syms rtos.bin kernel.map


#
#   Makefile for FreeRTOS on Raspberry Pi (secondary core, 0xF0000000)
#   Integration with dark builder system.
#

BASE = $(shell pwd)/
BUILD_DIR = $(shell pwd)/build/
MODULE_NAME = "RaspberryPi BSP"
TARGETS = kernel.img kernel.list kernel.syms kernel.elf rtos.bin
LINKER_SCRIPT = raspberrypi.ld

# Include the dark builder system (sets OBJECTS, CC, CFLAGS, Q, PRETTY, OBJCOPY, OBJDUMP, SIZE, etc.)
-include .dbuild/dbuild.mk

# -----------------------------------------------------------------
# Add syscalls.c to the build (startup.s is already in OBJECTS via .dbuild)
# -----------------------------------------------------------------
EXTRA_OBJ = syscalls.o
OBJECTS += $(EXTRA_OBJ)


#OBJECTS += $(addprefix $(BUILD_DIR)Demo/, $(EXTRA_OBJS))

# Rule to compile syscalls.c (uses the same compiler and flags)
syscalls.o: syscalls.c
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

# -----------------------------------------------------------------
# Main targets
# -----------------------------------------------------------------
all: kernel.list kernel.img kernel.syms
	@$(SIZE) kernel.elf

kernel.img: kernel.elf
	$(Q)$(PRETTY) IMAGE $(MODULE_NAME) $@
	$(Q)$(OBJCOPY) kernel.elf -O binary $@

kernel.list: kernel.elf
	$(Q)$(PRETTY) LIST $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -D -S kernel.elf > $@

kernel.syms: kernel.elf
	$(Q)$(PRETTY) SYMS $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -t kernel.elf > $@

rtos.bin: kernel.elf
	$(Q)$(PRETTY) BINARY $(MODULE_NAME) $@
	$(Q)$(OBJCOPY) kernel.elf -O binary $@

# -----------------------------------------------------------------
# Link step – using arm-none-eabi-gcc, with -nostartfiles and our own syscalls
# -----------------------------------------------------------------
kernel.elf: $(OBJECTS)
	$(Q)arm-none-eabi-gcc \
		-T $(LINKER_SCRIPT) \
		-Xlinker -Map -Xlinker kernel.map \
		-nostartfiles \
		$(OBJECTS) \
		-lc -lgcc \
		-o $@

# -----------------------------------------------------------------
# Clean – ignore harmless override warning
# -----------------------------------------------------------------
clean:
	rm -f kernel.elf kernel.img kernel.list kernel.syms rtos.bin kernel.map $(EXTRA_OBJ)
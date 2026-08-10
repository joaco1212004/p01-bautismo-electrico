# DM2 — Makefile bare minimum para STM32F103C8T6 (Blue Pill)
# Toolchain: arm-none-eabi-gcc
# Flasher:   openocd (ST-Link v2) o st-flash

TARGET  = blinky
MCU     = cortex-m3   # Cortex-M3: sin FPU

CROSS   = arm-none-eabi-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
SIZE    = $(CROSS)size

# ── Flags de compilación ──────────────────────────────────────────────────
CFLAGS  = -mcpu=$(MCU) -mthumb
CFLAGS += -O0 -g3                   # sin optimización, debug completo
CFLAGS += -Wall -Wextra -std=c11
CFLAGS += -ffreestanding -nostdlib  # sin libc, sin startup de GCC

LDFLAGS = -T src/linker.ld -nostdlib -Wl,-Map=$(TARGET).map

# ── Fuentes ───────────────────────────────────────────────────────────────
SRCS = src/main.c src/startup.s

# ── Targets ───────────────────────────────────────────────────────────────
.PHONY: all flash flash-stlink clean size

all: $(TARGET).elf $(TARGET).bin
	@echo ""
	@$(SIZE) $(TARGET).elf

$(TARGET).elf: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# OpenOCD con ST-Link v2 (el programador chino del Blue Pill)
flash: $(TARGET).bin
	openocd \
	  -f interface/stlink.cfg \
	  -f target/stm32f1x.cfg \
	  -c "program $(TARGET).bin verify reset exit 0x08000000"

# Alternativa: st-flash (más simple)
flash-stlink: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000

size: $(TARGET).elf
	$(SIZE) -A $(TARGET).elf

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).map

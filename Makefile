CC      = aarch64-linux-musl-gcc
LD      = aarch64-linux-musl-ld
OBJCOPY = aarch64-linux-musl-objcopy

SOURCES = $(wildcard *.c)
OBJS    = $(SOURCES:.c=.o)
CFLAGS  = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-stack-protector

.PHONY: clean

all: clean kernel8.img

start.o: start.S
	$(CC) $(CFLAGS) -c start.S -o start.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

font.o: font.psf
	$(LD) -r -b binary -o font.o font.psf

kernel8.img: start.o font.o $(OBJS)
	$(LD) -nostdlib -nostartfiles start.o font.o $(OBJS) -T link.ld -o kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf *.o >/dev/null 2>/dev/null || true

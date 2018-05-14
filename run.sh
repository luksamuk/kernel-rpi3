#!/bin/bash

# 05_uart0
# https://github.com/bztsrc/raspi3-tutorial

#qemu-system-aarch64 -M raspi3 -kernel kernel8.img -d in_asm
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio


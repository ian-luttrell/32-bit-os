#!/bin/bash

nasm -f bin boot.asm -o boot.bin &&
i686-elf-gcc -c -nostdlib -ffreestanding -masm=intel kernel32.c -o kernel32.o &&
i686-elf-ld -T link.ld kernel32.o -o kernel32.img &&

dd if=boot.bin of=os.flp &&
dd oflag=append conv=notrunc if=kernel32.img of=os.flp &&
#/usr/bin/bochs 'boot:floppy' 'floppya: 1_44=os.flp, status=inserted'
qemu-system-i386 -fda os.flp -monitor stdio

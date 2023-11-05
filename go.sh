#!/bin/bash

nasm -f bin boot.asm -o boot.bin &&
i686-elf-gcc -c -nostdlib -ffreestanding -mno-80387 -masm=intel kernel32.c -o kernel32.o &&
i686-elf-gcc -c -nostdlib -ffreestanding -mno-80387 -masm=intel terminal.c -o terminal.o &&
i686-elf-gcc -c -nostdlib -ffreestanding -mno-80387 -masm=intel print.c -o print.o &&
i686-elf-gcc -c -nostdlib -ffreestanding -mno-80387 -masm=intel util.c -o util.o &&
i686-elf-gcc -c -nostdlib -ffreestanding -mno-80387 -masm=intel pic.c -o pic.o &&
i686-elf-gcc -c -nostdlib -ffreestanding -mno-80387 -masm=intel paging.c -o paging.o &&
i686-elf-ld -T link.ld kernel32.o terminal.o print.o util.o pic.o paging.o -o kernel32.img &&

dd if=boot.bin of=os.flp &&
dd oflag=append conv=notrunc if=kernel32.img of=os.flp &&
#/usr/bin/bochs 'boot:floppy' 'floppya: 1_44=os.flp, status=inserted'
qemu-system-i386 -cpu pentium3 -fda os.flp -monitor stdio


#include <stdint.h>
#include <stddef.h>
#include "util.h"


size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

void memset(void *dest, uint8_t value, uint32_t byte_count) {
	char *tmp = (char *)dest;
	for ( ; byte_count != 0; byte_count--) {
		*tmp++ = value;
	}
}

void outb(uint16_t port, uint8_t value) {
	asm volatile ("outb %0, %1\n" : : "Nd" (port), "a" (value): "memory");
}

uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile("inb %0, %1"
					: "=a"(ret)
					: "Nd"(port)
					: "memory");

	return ret;
}

void outw(uint32_t port, uint32_t value) {
	asm volatile ("mov EAX, %0\n" : : "m"(value));
	asm volatile ("mov EDX, %0\n" : : "m"(port));
	asm volatile ("out DX, EAX\n");
}

uint16_t inw(uint16_t port) {
	uint16_t ret;
	asm volatile ("mov DX, %0\n" : : "m"(port));
	asm volatile ("in AX, DX\n");
	asm volatile("mov %0, AX\n" : "=r"(ret) : : "memory");

	return ret;
}


void io_wait() {
	for (int i = 0; i < 100; i++) {	
		outb(0x80, 0x00);  // port 0x80 is unused after BIOS POST
	}
}

#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define BYTES_PER_WORD 2

size_t strlen(const char* str);
void memset(void *dest, uint8_t value, uint32_t byte_count);
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void outw(uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);
void io_wait();

#endif

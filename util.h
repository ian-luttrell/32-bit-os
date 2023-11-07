#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stddef.h>

size_t strlen(const char* str);
void memset(void *dest, uint8_t value, uint32_t byte_count);
void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void io_wait();

#endif
#ifndef TERMINAL_H
#define TERMINAL_H

void terminal_initialize();
void terminal_putchar(uint8_t);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);

#endif

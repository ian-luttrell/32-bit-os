#ifndef PRINT_H
#define PRINT_H

typedef enum
{
	ZERO_X,
	SPACE,
} hex_option;

void print(const char *);
void print_uint32(uint32_t);
void print_uint32_hex(uint32_t);
void print_uint16_hex(uint16_t input, hex_option option);
void print_uint16_bin(uint16_t input);
void print_reg_cs();
void print_reg_esp();

#endif


#include <stdint.h>
#include <stddef.h>
#include "print.h"
#include "terminal.h"
#include "util.h"


void print(const char *string) {
	terminal_write(string, strlen(string));
}

uint32_t remainder;
void print_uint32(uint32_t input)
{
	if (input == 0)
		terminal_putchar(48);
	else
	{
		uint32_t curr = input;
		size_t digit_counter = 0;	
		while (curr != 0)
		{
			remainder = curr % 10;
			asm("push remainder");
			digit_counter++;

			curr -= remainder;
			curr /= 10;		
		}
	
		while (digit_counter > 0)
		{
			asm("pop remainder");
			terminal_putchar(remainder + 48);
			digit_counter--;
		}
	}
}

void print_uint32_hex(uint32_t input)
{

	if (input == 0)
	{
		terminal_writestring("0x00000000");
	}
	else
	{
		uint32_t curr = input;
		size_t digit_counter = 0;	
		while (curr != 0)
		{
			remainder = curr % 16;
			asm("push remainder");
			digit_counter++;

			curr -= remainder;
			curr /= 16;		
		}
	
		terminal_writestring("0x");
		for(uint8_t i = 0; i < 8 - digit_counter; i++) {
			terminal_writestring("0");
		}

		while (digit_counter > 0)
		{
			asm("pop remainder");
			switch(remainder) {
				case 10:
					terminal_putchar('A');
					break;
				case 11:
					terminal_putchar('B');
					break;
				case 12:
					terminal_putchar('C');
					break;
				case 13:
					terminal_putchar('D');
					break;				
				case 14:
					terminal_putchar('E');
					break;
				case 15:
					terminal_putchar('F');
					break;
				default:
					terminal_putchar(remainder + 48);
			}

			digit_counter--;
		}
	}
}

void print_uint16_hex(uint16_t input, hex_option option)
{

	if (input == 0)
	{
		switch(option) {
			case ZERO_X: terminal_writestring("0x0000 "); break;
			case SPACE: terminal_writestring("0000 "); break;
			default: terminal_writestring("0x0000 "); break;
		}
	}
	else
	{
		uint16_t curr = input;
		size_t digit_counter = 0;	
		while (curr != 0)
		{
			remainder = curr % 16;
			asm("push remainder");
			digit_counter++;

			curr -= remainder;
			curr /= 16;		
		}
	
		switch(option) {
			case ZERO_X: terminal_writestring("0x"); break;
			default: break;
		}
		for(uint8_t i = 0; i < 4 - digit_counter; i++) {
			terminal_writestring("0");
		}

		while (digit_counter > 0)
		{
			asm("pop remainder");
			switch(remainder) {
				case 10:
					terminal_putchar('A');
					break;
				case 11:
					terminal_putchar('B');
					break;
				case 12:
					terminal_putchar('C');
					break;
				case 13:
					terminal_putchar('D');
					break;				
				case 14:
					terminal_putchar('E');
					break;
				case 15:
					terminal_putchar('F');
					break;
				default:
					terminal_putchar(remainder + 48);
			}

			digit_counter--;
		}
		terminal_writestring(" ");
	}
}

void print_uint16_bin(uint16_t input)
{
	uint16_t mask = 0x8000;
	while (mask) {
		if (mask & input) {
			terminal_putchar('1');
		}
		else {
			terminal_putchar('0');
		}

		if (mask & 0x1110) {  // print a space after each nibble
			terminal_putchar(' ');
		} 

		mask >>= 1;
	}
}

void print_uint16_bits(uint16_t input)
{
	uint16_t mask = 0x8000;
	uint32_t bit_pos = 15;
	while (mask) {
		print("\nBit ");
		if (bit_pos >= 10) {
			print_uint32(bit_pos);
		}
		else {
			print("0");
			print_uint32(bit_pos);
		}
		print(": ");
	
		if (mask & input) {
			terminal_putchar('1');
		}
		else {
			terminal_putchar('0');
		}

		mask >>= 1;
		bit_pos--;
	}
}

void print_reg_cs() {
print("\nThe CS register currently contains: ");
	uint32_t cs_contents;
	asm("mov ebx, cs");
	asm("mov %0, ebx" : "=r"(cs_contents));
	print_uint32_hex(cs_contents);
}

void print_reg_esp() {
	print("\nThe ESP register currently contains: ");
	uint32_t esp_contents;
	asm("mov ebx, esp");
	asm("mov %0, ebx" : "=r"(esp_contents));
	print_uint32_hex(esp_contents);
}

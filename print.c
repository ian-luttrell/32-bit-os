
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

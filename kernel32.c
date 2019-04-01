#include <stdbool.h> 
#include <stddef.h>
#include <stdint.h>

void terminal_initialize();
void print(const char *);
void terminal_putchar(uint8_t);
size_t strlen(const char *);
void print(const char *);
void print_uint32_dec(uint32_t);
void print_uint32_hex(uint32_t);
void user_mode();
void load_idt();
void init_paging();


uint32_t kernel_main(void) {
	terminal_initialize();

	print("Loading IDT...\n");
	load_idt();
	print("Done loading IDT.\n");

	print("Initializing paging...\n");
	init_paging();
	print("Done initializing paging.\n");

	// uncomment to make sure we are running in ring 0 (i.e. halt allowed)
	//asm("HLT");	

	/* uncomment to go into user mode (i.e. the function user_mode())
	asm("MOV AX, 0x23\n"	
		"MOV DS, AX\n"
		"MOV ES, AX\n"
		"MOV FS, AX\n"
		"MOV GS, AX\n"
		"PUSH 0x23\n"		
		"PUSH ESP\n"		
		"PUSHFD\n"			
		"PUSH 0x1B\n"		
		"LEA EAX, [user_mode]\n"	
		"PUSH EAX\n"
		"IRETD");
	*/

	return 0xDEADBEEF;
}


// NEEDS TO BE 4K ALIGNED WHEN ADDRESSES GET LARGER (but gcc ignores directive)
typedef struct {
	uint32_t entry[1024];
} page_dir_t;

// NEEDS TO BE 4K ALIGNED WHEN ADDRESSES GET LARGER (but gcc ignores directive)
typedef struct {
	uint32_t page[1024];
} page_table_t;

page_dir_t page_dir;
page_table_t kernel_page_table_1;
// NEED TO READ NEXT TRACK OF FLOPPY TO GET THESE INTO RAM
//page_table_t kernel_page_table_2, kernel_page_table_3, kernel_page_table_4, kernel_page_table_5;

void enable_paging()
{
	asm("lea eax, [page_dir]\n"
		"mov cr3, eax\n"
		"mov eax, cr0\n"
		"or eax, 0x80000001\n"
		"mov cr0, eax");
}

void init_paging()
{
	// KERNEL'S FIRST PAGE
	// start with nonzero bits of the 4KB-aligned page table address
	page_dir.entry[0] = ((uint32_t)&kernel_page_table_1) << 12;
	// "Avail" field not used, so we leave it as zero
	// G is ignored as well
	// Low byte: S = 0 (for 4KB), next bit always 0, A = 0 (not accessed/maybe irrelevant),
	//     D = 0 (page will not be cached), W = 0 (write-back caching), U = 0 (supervisor-only access),
	//     R = 1 (read/write), P = 1 (present)
	page_dir.entry[0] |= 0x00000003;

/*
	// KERNEL'S SECOND PAGE
	page_dir.entry[1] |= ((uint32_t)&kernel_page_table_2) << 12;
	page_dir.entry[1] |= 0x00000003;

	// KERNEL'S THIRD PAGE
	page_dir.entry[2] |= ((uint32_t)&kernel_page_table_3) << 12;
	page_dir.entry[2] |= 0x00000003;

	// KERNEL'S FOURTH PAGE
	page_dir.entry[3] |= ((uint32_t)&kernel_page_table_4) << 12;
	page_dir.entry[3] |= 0x00000003;
	
	// KERNEL'S FIFTH PAGE
	page_dir.entry[4] |= ((uint32_t)&kernel_page_table_5) << 12;
	page_dir.entry[4] |= 0x00000003;
*/
	print("Set up page directory kernel entries.\n");

	while(1);
	enable_paging();
}


/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

 
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
 
 
void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

void print(const char *string) {
	terminal_write(string, strlen(string));
}

void terminal_putchar(uint8_t byte) {
	if (byte == '\n') {
		terminal_row++;
		terminal_column = 0;
		return;
	}
	else
		terminal_putentryat(byte, terminal_color, terminal_column, terminal_row);

	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
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


void isr_0x60()
{
	// Probably needs to be moved to an .asm file, because compiler
	//     does not handle the stack in the way we require
	//     (and that .asm file will need to be linked properly)
	print("This is the system call interrupt.\n");
}

typedef struct {
	uint16_t offset_low;  // bits 0 through 15 of ISR address
	uint16_t selector;  // code segment selector in GDT or LDT
	uint8_t zero;       // unused, must be set to 0
	uint8_t type_attr;  // type and attributes of this IDT entry
	uint16_t offset_high;  // bits 16 through 31 of ISR address
} __attribute__((packed)) idt_entry_t;

// the actual IDT
idt_entry_t idt[256];

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) idtr_contents_t; 

idtr_contents_t idtr_contents;


void load_idt()
{
	idtr_contents.limit = (256 * sizeof(idt_entry_t)) - 1;
	idtr_contents.base = (uint32_t)idt;

	idt[0x60].offset_low = ((uint32_t)isr_0x60) & 0x0000FFFF;
	idt[0x60].offset_high = ((uint32_t)isr_0x60 >> 16) & 0x0000FFFF;
	idt[0x60].selector = 0x0008;
	idt[0x60].zero = 0x00;
	// present, caller DPL <= 3, type = 14 (32-bit interrupt)
	idt[0x60].type_attr = 0xEE;

	asm volatile("lidt [idtr_contents]");
}




void user_mode()
{
	asm("ADD ESP, 4");
	
	print("Now we're in user mode.\n");

	// uncomment to make sure we are running in ring 0 (i.e. halt NOT allowed)
	//asm("HLT");	

	while(1);
}



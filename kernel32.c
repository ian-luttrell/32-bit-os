#include <stdbool.h> 
#include <stddef.h>
#include <stdint.h>
#include "terminal.h"
#include "print.h"
#include "util.h"
#include "pic.h"
#include "paging.h"


void user_mode();
void load_idt();
void init_paging();


uint32_t kernel_main(void) {
	terminal_initialize();

	print("Loading IDT...\n");
	load_idt();
	print("Done loading IDT.\n");
	/*
	// set timer frequency to "Hz" ticks per second
	uint32_t Hz = 1000;	
	uint32_t divisor = 1193180 / Hz;
	uint8_t divisor_low_byte = divisor & 0xFF;
	uint8_t divisor_high_byte = divisor >> 8;
	asm (        "mov al, 0x36;\
				  outb 0x43, al;\
				  mov al, %0;\
				  outb 0x40, al;\
				  mov al, %1;\
				  outb 0x40, al"
				 :
				 : "m" (divisor_low_byte), "m" (divisor_high_byte)
		);
	*/

	// test "general protection fault" exception handler (in kernel mode)
	//   (selector in interrupt/exception handler would need to be 0x0008, not 0x0018)
	//asm("int 0x10");


	//print("Initializing paging...\n");
	//init_paging();
	//print("Done initializing paging.\n");

	// uncomment to make sure we are running in ring 0 (i.e. halt allowed)
	//asm("HLT");	

	// uncomment to go into user mode (i.e. the function user_mode())
	
	print("\nAbout to jump to user mode. The ESP register currently contains: ");
	uint32_t esp_contents;
	asm("mov ebx, esp");
	asm("mov %0, ebx" : "=r"(esp_contents));
	print_uint32_hex(esp_contents);

	vmmngr_initialize();

	asm("CLI\n"
		"MOV AX, 0x23\n"	
		"MOV DS, AX\n"	// data segment selectors for user mode - 0x23 = 0x20 | 0x03 (selector in GDT ORed with Requested Privilege Level (RPL) of 3, for Ring 3)
		"MOV ES, AX\n"
		"MOV FS, AX\n"
		"MOV GS, AX\n"
		"MOV EAX, 0x000C0000\n"  // top of user mode stack
		"PUSH 0x23\n"	// stack segment selector for user mode
		"PUSH EAX\n"	// value of stack pointer upon IRET
		"PUSHFD\n"		// value of EFLAGS register to be loaded upon IRET
		"POP EAX\n"		// pop value of EFLAGS into EAX
		"OR EAX, 0x200\n"  // enable interupt flag (IF) by ORing current EFLAGS with 0x200 (1000000000)
		"PUSH EAX\n"	// now interrupts will be reenabled atomically (i.e. upon IRET, so we will already be safely in user mode)
		"PUSH 0x1B\n"	// code segment selector for user mode (again, GDT selector is ORed with RPL of 3, i.e. 0x1B = 0x18 | 0x03)
		"LEA EAX, [user_mode]\n"	
		"PUSH EAX\n"	// top of stack (lowest address) is set to the address where we will continue execution upon IRET
		"IRETD");		// interrupt "return" to user mode

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


/*
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
/*
	print("Set up page directory kernel entries.\n");

	enable_paging();
}
*/
 
__attribute__((interrupt)) void isr_0x00(uint32_t *p)
{
	print("\n\nYou divided by zero! This is undefined in any (algebraic) ring.\nThe processor has been halted.");

	asm("hlt");
}

__attribute__((interrupt)) void isr_0x0A(uint32_t *p)
{
	print("\n\nInvalid TSS (task state segment)!\nThe processor has been halted.");
	
	asm("hlt");
}


__attribute__((interrupt)) void isr_0x0D(uint32_t *p)
{
	print("\n\nGeneral protection fault!\nThe processor has been halted.");

	asm("mov eax, 0x0badc0de");
	
	asm("hlt");
}


__attribute__((interrupt)) void isr_0x80(uint32_t *ptr)
{
	print("\n\nSystem call!");
	//print_reg_cs();
	//print_reg_esp();

	//memset((void *)0xB8000, 0x0010, 1000);  // testing the memset utility function on VGA RAM

	print("\nReturning to user mode.");
}


typedef struct
{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
} regs_t;


__attribute__((interrupt)) void irq_0x00(regs_t *regs)
{
	asm("cli");
	//print("\n\nTimer interrupt generated on 10/28/23.");
	//print_reg_esp();
	//print_reg_cs();
	//send "end of interrupt" (EOI) signal to master PIC
	PIC_sendEOI(0x00);
	asm("sti");
}

__attribute__((interrupt)) void irq_0x01(regs_t *regs)
{
	asm("cli");
	//print("\n\nKeyboard interrupt generated!");
	uint8_t byte;
	byte = inb(0x60);
	print("\nGot \n");	
	print_uint32_hex(byte);
	//print_reg_esp();
	//print_reg_cs();

	//send "end of interrupt" (EOI) signal to master PIC
	PIC_sendEOI(0x01);
	asm("sti");
}

__attribute__((interrupt)) void irq_0x0C(regs_t *regs)
{
	asm("cli");
	print("\n\nMouse interrupt generated!");
	print_reg_esp();
	print_reg_cs();
	//send "end of interrupt" (EOI) signal to master PIC
	PIC_sendEOI(0x0C);
	asm("sti");
}

__attribute__((interrupt)) void isr_0x0E(regs_t *regs)
{
	asm("cli");
	print("\n\nPage fault exception!");
	print_reg_esp();
	print_reg_cs();

	asm("hlt");  // need to do something instead of halting

	asm("sti");
}


typedef struct {
	uint16_t offset_low;  // bits 0 through 15 of ISR address
	uint16_t selector;  // code segment selector in GDT or LDT
	uint8_t zero;       // unused, must be set to 0
	uint8_t type_attr;  // type and attributes of this IDT entry
	uint16_t offset_high;  // bits 16 through 31 of ISR address
} idt_entry_t;

// the actual IDT
idt_entry_t idt[256];

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) idtr_contents_t; 

idtr_contents_t idtr_contents;


void set_idt_entry(uint16_t num, uint32_t *base, uint16_t selector, uint8_t flags)
{
	idt[num].offset_low = (uint32_t)base & 0x0000FFFF;
	idt[num].offset_high = ((uint32_t)base >> 16) & 0x0000FFFF;	
	idt[num].selector = selector;
	idt[num].zero = 0x00;
	idt[num].type_attr = flags;
}

void load_idt()
{
	idtr_contents.limit = (256 * sizeof(idt_entry_t)) - 1;
	idtr_contents.base = (uint32_t)idt;

	// remap PIC IRQ entry numbers to avoid overlap with 80386 processor interrupts
	PIC_remap(0x20, 0x28);
	// hardware interrupts (IRQs)
	set_idt_entry(0x20, (uint32_t *)irq_0x00, 0x0008, 0x8E);
	set_idt_entry(0x21, (uint32_t *)irq_0x01, 0x0008, 0x8E);
	set_idt_entry(0x2B, (uint32_t *)irq_0x0C, 0x0008, 0x8E);

	// exceptions (software interrupts)
	set_idt_entry(0x00, (uint32_t *)isr_0x00, 0x0008, 0xEE);
	set_idt_entry(0x0A, (uint32_t *)isr_0x0A, 0x0008, 0xEE);
	set_idt_entry(0x0E, (uint32_t *)isr_0x0E, 0x0008, 0xEE);

	// system call (software interrupt)
	set_idt_entry(0x80, (uint32_t *)isr_0x80, 0x0008, 0xEE);

	asm volatile("lidt [idtr_contents]");

	asm volatile("sti");
}




void user_mode()
{
	asm("add esp, 4");

	print("\nNow we're in user mode.");

	asm("int 0x80");  // system call

	//print("\nNow we're back in user mode.");
	//print_reg_cs();
	//print_reg_esp();

	asm("int 0x80");

	//print("\nNow we're back in user mode.");
	//print_reg_cs();
	//print_reg_esp();
	
/*
	uint8_t byte;
	byte = inb(0x64);
	print("\nPort 0x64 reads:\n");
	print_uint32_hex(byte);

	outb(0x60, 0xF5);
	byte = inb(0x60);
	while (byte != 0xFA);
	print("\nGot byte ");
	print_uint32_hex(byte);
	outb(0x60, 0xF2);
	byte = inb(0x60);
	while (byte != 0xFA);
print("\nGot byte ");
	print_uint32_hex(byte);
byte = inb(0x60);
print("\nGot byte ");
	print_uint32_hex(byte);
	byte = inb(0x60);
print("\nGot byte ");
	print_uint32_hex(byte);
*/

	while(1);
}



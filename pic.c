
#include <stdint.h>
#include "util.h"
#include "pic.h"


void PIC_remap(uint8_t offset1, uint8_t offset2)
{
	uint8_t a1, a2;
	a1 = inb(PIC1_DATA_PORT);	// preserve masks to restore after remap
	a2 = inb(PIC2_DATA_PORT);

	outb(PIC1_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA_PORT, offset1);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_DATA_PORT, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_DATA_PORT, 0x04);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_DATA_PORT, 0x02);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
 
	outb(PIC1_DATA_PORT, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	outb(PIC2_DATA_PORT, ICW4_8086);
	io_wait();
 
	outb(PIC1_DATA_PORT, a1);   // restore saved masks.
	io_wait();	
	outb(PIC2_DATA_PORT, a2);
}

void PIC_sendEOI(uint8_t irq) {
	if(irq >= 8) {  // if interrupt >=8, then the IRQ came from slave PIC
		outb(PIC2_COMMAND_PORT, PIC_EOI);
	}
	outb(PIC1_COMMAND_PORT, PIC_EOI);
}

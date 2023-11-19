#include "ata.h"
#include "print.h"


bool ata_wait(uint32_t ticks, uint8_t condition_mask) {
	while (ticks) {
		if (!(ata_read_status() & condition_mask)) {
			io_wait();

			ticks--;
			continue;
		}		
		if (ata_read_status() & condition_mask) {
			return true;
		}
	}

	return false;
}

bool ata_wait_busy(uint32_t ticks) {
	while (ticks) {
		if (ata_wait(1, ATA_DRIVE_BUSY)) {
			io_wait();
			ticks--;

			continue;
		}
		return true;
	}

	return false;
}

void ata_select_drive(uint8_t drive) {
	if (!ata_wait_busy(1)) {
		print("\nATA controller timed out on drive select. Halting.");
		asm("cli");
		asm("hlt");
	}
	
	outb(ATA_HEAD, 0xA0 | (drive << 4));
	
	if (!ata_wait_busy(1)) {
		print("\nATA controller timed out on drive select. Halting.");
		asm("cli");
		asm("hlt");
	}
}

void ata_reset(uint8_t drive) {
	ata_select_drive(drive);

	outb(ATA_DEV_CONTROL, ATA_COMMAND_RESET);
	for (int i = 0; i < 10000; i++) {
		io_wait();
	}
	print("\nImmediately after reset, status is ");
	int status = ata_read_status();
	print_uint32_hex(status);
	print_uint16_bin(status);
	
	print("\nControl is: ");
	print_uint32_hex(inb(ATA_DEV_CONTROL));
	outb(ATA_DEV_CONTROL, 0x02);  // clear SRST and disable interrupts (i.e. set nINT bit)
	print("\nAfter SRST clear, control is: ");
	print_uint32_hex(inb(ATA_DEV_CONTROL));

	if (!ata_wait_busy(100000)) {
		print("\nATA controller timed out on drive reset. Halting.");
		asm("cli");
		asm("hlt");
	}
}

uint8_t ata_read_status() {
	for (uint8_t i = 0; i < 14; i++) {
		inb(ATA_ALT_STATUS);
	}
	return inb(ATA_ALT_STATUS);
}

uint8_t ata_read_error() {
	for (uint8_t i = 0; i < 14; i++) {
		inb(ATA_ALT_STATUS);
	}
	return inb(ATA_ERROR);
}

uint8_t ata_read_sect_count(uint8_t drive) {
	ata_select_drive(drive);
	io_wait();
	return inb(ATA_SECT_COUNT);
}

uint8_t ata_read_sect_number(uint8_t drive) {
	ata_select_drive(drive);
	io_wait();
	return inb(ATA_SECT_NUMBER);
}

#include "ata.h"


bool ata_wait(uint32_t ticks, uint8_t condition_mask) {
	while (ticks) {
		uint8_t status = (uint8_t)inw(ATA_ALT_STATUS);
		if (status & condition_mask) {
			return true;
		}

		io_wait();
		ticks--;
	}

	return false;
}

uint8_t ata_read_status() {
	return inb(ATA_ALT_STATUS);
}

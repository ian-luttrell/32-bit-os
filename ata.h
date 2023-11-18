#ifndef ATA_H
#define ATA_H

#include "util.h"

#define ATA_COMMAND    0x01F0
#define ATA_CONTROL    0x03F4

#define ATA_STATUS     ATA_COMMAND + 7
#define ATA_ALT_STATUS ATA_CONTROL + 2

#define ATA_BUSY       0x80

bool ata_wait(uint32_t ticks, uint8_t condition_mask);
uint8_t ata_read_status();


#endif

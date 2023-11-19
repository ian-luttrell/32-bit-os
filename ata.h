#ifndef ATA_H
#define ATA_H

#include "util.h"

#define ATA_COMMAND    0x01F0
#define ATA_CONTROL    0x03F4

#define ATA_DATA         ATA_COMMAND + 0
#define ATA_ERROR        ATA_COMMAND + 1
#define ATA_FEATURES     ATA_COMMAND + 1
#define ATA_CYL_LOW      ATA_COMMAND + 4
#define ATA_CYL_HIGH     ATA_COMMAND + 5
#define ATA_HEAD         ATA_COMMAND + 6
#define ATA_STATUS       ATA_COMMAND + 7
#define ATA_SECT_COUNT   ATA_COMMAND + 2
#define ATA_SECT_NUMBER  ATA_COMMAND + 3
#define ATA_ALT_STATUS   ATA_CONTROL + 2  // for reads
#define ATA_DEV_CONTROL  ATA_CONTROL + 2  // for writes

#define ATA_DRIVE_BUSY        0x80
#define ATA_DRIVE_READY       0x40
#define ATA_DATA_READY        0x08

#define ATA_COMMAND_IDENTIFY 0xEC
#define ATA_COMMAND_RESET    0x04

bool ata_wait(uint32_t ticks, uint8_t condition_mask);
bool ata_wait_busy(uint32_t ticks);
void ata_select_drive(uint8_t drive);
void ata_reset(uint8_t drive);
uint8_t ata_read_status();
uint8_t ata_read_error();
uint8_t ata_read_sect_count(uint8_t drive);
uint8_t ata_read_sect_number(uint8_t drive);


#endif

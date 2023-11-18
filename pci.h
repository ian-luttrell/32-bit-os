#include <stdint.h>
#include "print.h"

#define PCI_CONFIG_ADDR    0xCF8
#define PCI_CONFIG_DATA    0xCFC
#define BYTES_PER_REGISTER 0x04
#define BITS_PER_BYTE      8


uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg_offset);

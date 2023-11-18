#include "pci.h"
#include "util.h"


uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t register_index)
{	
	uint32_t config_address;
	uint32_t enable_word = 0x80000000;  // bits 24-31 of address must be 10000000 to enable reads/writes (I/O port 0xCFC) for this address
	uint32_t bus_word = (uint32_t)bus << 16;  // bus number in bits 16-23 of address
	uint32_t device_word = (uint32_t)device << 11;  // device number in bits 11-15 of address
	uint32_t func_word = (uint32_t)function << 8;   // function number in bits 8-10 of address
	uint32_t reg_word = (uint32_t)(register_index * BYTES_PER_REGISTER) & 0xFC;  // offset into 256-byte configuration space must be 4-byte aligned, so mask with 11111100 to address the 32-bit word at reg_offset  
	
	config_address = enable_word | bus_word | device_word | func_word | reg_word;
	outw(PCI_CONFIG_ADDR, config_address);

	uint32_t result = inw(PCI_CONFIG_DATA);
	if(result != 0xFFFFFFFF) {
		print("\nFrom PCI bus ");
		print_uint32(bus);
		print(", dev ");
		print_uint32(device);
		print(", func ");
		print_uint32(function);
		print(", reg ");
		print_uint32(register_index);
		print(" (bits ");
		print_uint32(register_index * BYTES_PER_REGISTER * BITS_PER_BYTE + 31);
		print(" - ");
		print_uint32(register_index * BYTES_PER_REGISTER * BITS_PER_BYTE);
		print("): ");
		print_uint32_hex(result);
		print(": ");
		print("\n");
		print_uint16_bin((uint16_t)(result >> 16));
		print("\n");
		print_uint16_bin((uint16_t)result);
	}

	return result;
}

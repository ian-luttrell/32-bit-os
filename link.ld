
ENTRY(kernel_main)

SECTIONS
{
	. = 0x8400;
	.text : { *(.text) }
	.data : { *(.data) }
	.bss : { *(.bss) }
	.rodata : { *(.rodata) }
}

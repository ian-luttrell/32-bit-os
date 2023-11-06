
#include <stdint.h>
#include "paging.h"
#include "util.h"
//
#include "print.h"
 

//uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t *page_directory = (uint32_t *)0x00012000;
//uint32_t kernel_page_table[1024] __attribute__((aligned(4096)));
uint32_t *shared_page_table = (uint32_t *)0x00010000;
uint32_t *user_code_page_table = (uint32_t *)0x00410000;
uint32_t *user_data_page_table = (uint32_t *)0x00810000;
uint32_t *kernel_code_page_table = (uint32_t *)0x00C10000;
uint32_t *kernel_data_page_table = (uint32_t *)0x01010000;

void vmmngr_initialize () {
/*
	-- Commented out this section for now - using above global variables until I understand more
	// kernel page table
	ptable* table = (ptable*) 0x010000;   // arbitrary address, but 4K aligned
	if (!table)
		return;

	physical_addr frame;
	uint32_t virt;
	for (int i = 0, frame = 0x00000000,  virt = 0x00000000; i < PAGES_PER_TABLE; i++, frame += PAGE_SIZE, virt += PAGE_SIZE) {
		pt_entry kernel_page = 0;
		pt_entry_add_attrib(&kernel_page, I86_PTE_PRESENT);
		pt_entry_add_attrib(&kernel_page, I86_PTE_KERNEL);
		pt_entry_set_frame(&kernel_page, frame);  // identity map these kernel pages
//print("\n\n");
	//print_uint32_hex(kernel_page);

		table->m_entries[PAGE_TABLE_INDEX(virt)] = kernel_page;
	}

	//! create default directory table
	pdirectory*	dir = (pdirectory*) 0x012000;  // arbitrary address, but 4K aligned
	if (!dir)
		return;
 
	// clear directory table
	memset (dir, 0, sizeof (pdirectory));

	pd_entry* entry = &dir->m_entries[PAGE_DIRECTORY_INDEX(0x00000000)];
	pd_entry_add_attrib (entry, I86_PDE_PRESENT);
	pd_entry_add_attrib(entry, I86_PTE_KERNEL);
	pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
	pd_entry_set_frame (entry, (physical_addr)table);
*/


	int i;
	for (i = 0; i < 1024; i++)
	{
		memset((uint32_t *)page_directory[i], 0, sizeof(uint32_t));
	}

	for (i = 0; i < 1024; i++)
	{
		shared_page_table[i] = (i * 0x1000) | (I86_PTE_USER | I86_PTE_PRESENT | I86_PTE_WRITABLE);
	}
	page_directory[0] = (uint32_t)shared_page_table | (I86_PDE_USER | I86_PDE_PRESENT | I86_PDE_WRITABLE);
	for (i = 0; i < 1024; i++)
	{
		user_code_page_table[i] = (i * 0x1000) | (I86_PTE_USER | I86_PTE_PRESENT);
	}
	page_directory[1] = (uint32_t)user_code_page_table | (I86_PDE_USER | I86_PDE_PRESENT);
	for (i = 0; i < 1024; i++)
	{
		user_data_page_table[i] = (i * 0x1000) | (I86_PTE_USER | I86_PTE_PRESENT | I86_PTE_WRITABLE);
	}
	page_directory[2] = (uint32_t)user_data_page_table | (I86_PDE_USER | I86_PDE_PRESENT | I86_PDE_WRITABLE);
	for (i = 0; i < 1024; i++)
	{
		kernel_code_page_table[i] = (i * 0x1000) | (I86_PTE_KERNEL | I86_PTE_PRESENT);
	}
	page_directory[3] = (uint32_t)kernel_code_page_table | (I86_PDE_KERNEL | I86_PDE_PRESENT);	

	// rest of the 4GiB of RAM will be kernel data for now
	int dir_index;
	for (dir_index = 4; dir_index < 1024; dir_index++)
	{
		for (i = 0; i < 1024; i++)
		{
			kernel_data_page_table[i] = (i * 0x1000) | (I86_PTE_KERNEL | I86_PTE_PRESENT | I86_PTE_WRITABLE);
		}
		page_directory[dir_index] = (uint32_t)kernel_data_page_table | (I86_PDE_KERNEL | I86_PDE_PRESENT | I86_PDE_WRITABLE);
	}

	enable_paging(page_directory);
}

extern void pt_entry_add_attrib (pt_entry* e, uint32_t attrib) {
	*e = *e | attrib;

	return;
}

extern void pt_entry_set_frame (pt_entry* e, physical_addr frame) {
	*e = *e | (frame << 12);
}

extern void pd_entry_add_attrib (pd_entry* e, uint32_t attrib) {
	*e = *e | attrib;

	return;
}

extern void	pd_entry_set_frame (pd_entry* e, physical_addr frame) {
	*e = *e | (frame << 12);
}

void enable_paging(uint32_t *dir)
{
	asm volatile("mov eax, %0\n"
		"mov cr3, eax\n"
		"mov eax, cr0\n"
		"or eax, 0x80000000\n"
		"mov cr0, eax\n" : "=m"(dir)
		);
}


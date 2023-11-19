
#include <stdint.h>
#include "paging.h"
#include "util.h"
//
#include "print.h"
 

uint32_t *page_directory_kernel_mode = (uint32_t *)  0x00040000;  // needs to be in kernel space
uint32_t **kernel_data_pages = (uint32_t **)         0x00060004;  // do not overwrite when populating page directory (and needs to be in kernel space)
uint32_t *shared_page_table = (uint32_t *)       	 0x00000000;
uint32_t *user_code_page_table = (uint32_t *)    	 0x00400000;
uint32_t *user_data_page_table = (uint32_t *)    	 0x00800000;
uint32_t *kernel_code_page_table = (uint32_t *)  	 0x00C00000;

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

	pd_entry* entry = &dir->m_entries[page_directory_kernel_mode_INDEX(0x00000000)];
	pd_entry_add_attrib (entry, I86_PDE_PRESENT);
	pd_entry_add_attrib(entry, I86_PTE_KERNEL);
	pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
	pd_entry_set_frame (entry, (physical_addr)table);
*/


	unsigned int i;
	for (i = 0; i < 1024; i++)
	{
		memset((uint32_t *)page_directory_kernel_mode[i], 0, sizeof(uint32_t));
	}

	for (i = 0; i < 1024; i++)
	{
		shared_page_table[i] = (i * 0x1000) | (I86_PTE_USER | I86_PTE_PRESENT | I86_PTE_WRITABLE);
	}

	page_directory_kernel_mode[0] = (uint32_t)shared_page_table | (I86_PDE_USER | I86_PDE_PRESENT | I86_PDE_WRITABLE);
	for (i = 0; i < 1024; i++)
	{
		user_code_page_table[i] = ((uint32_t)user_code_page_table + i * 0x1000) | (I86_PTE_USER | I86_PTE_PRESENT | I86_PDE_WRITABLE);
	}
	page_directory_kernel_mode[1] = (uint32_t)user_code_page_table | (I86_PDE_USER | I86_PDE_PRESENT | I86_PDE_WRITABLE);

	for (i = 0; i < 1024; i++)
	{
		user_data_page_table[i] = ((uint32_t)user_data_page_table + i * 0x1000) | (I86_PTE_USER | I86_PTE_PRESENT | I86_PTE_WRITABLE);
	}
	page_directory_kernel_mode[2] = (uint32_t)user_data_page_table | (I86_PDE_USER | I86_PDE_PRESENT | I86_PDE_WRITABLE);
	for (i = 0; i < 1024; i++)
	{
		kernel_code_page_table[i] = ((uint32_t)kernel_code_page_table + i * 0x1000) | (I86_PTE_KERNEL | I86_PTE_PRESENT);
	}
	page_directory_kernel_mode[3] = (uint32_t)kernel_code_page_table | (I86_PDE_KERNEL | I86_PDE_PRESENT);	

	unsigned int dir_index;
	*kernel_data_pages = (uint32_t *)0x01000000;
	uint32_t *kernel_data_frame_start = *kernel_data_pages;  // initialize first frame to start of kernel data range
	for (dir_index = 4; dir_index < 120; dir_index++)  // allocate 112 MiB of kernel data pages (do not overwrite video RAM), so we have 128 MiB mapped in total
	{
		for (i = 0; i < 1024; i++)
		{
			kernel_data_frame_start[i] = ((uint32_t)kernel_data_frame_start + i * 0x1000) | (I86_PTE_KERNEL | I86_PTE_PRESENT | I86_PTE_WRITABLE);
		}
		page_directory_kernel_mode[dir_index] = (uint32_t)kernel_data_frame_start | (I86_PDE_KERNEL | I86_PDE_PRESENT | I86_PDE_WRITABLE);
		
		kernel_data_frame_start += 0x00400000 / sizeof(kernel_data_frame_start);  // 4MiB between each page directory entry (1024 frames * 4096 bytes/frame = 4 MiB)
	}

	enable_paging(page_directory_kernel_mode);
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


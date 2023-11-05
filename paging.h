
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t physical_addr;
#define PAGE_SIZE 4096
//! i86 architecture defines 1024 entries per table--do not change
#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR	1024
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3FF)  // macro to shift virtual address right by 12 bits and mask with 00000000000000000000001111111111, so e.g. virt. 												   //   address 4096 gets an index of 1 in the PT, 8192 gets an offset of 2, etc.
#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3FF)  // similar for indexing page directory entries

// page directory entry
typedef uint32_t pd_entry;
// page table entry
typedef uint32_t pt_entry;

//! page table
typedef struct ptable {
 
	pt_entry m_entries[PAGES_PER_TABLE];
} ptable;
 
//! page directory
typedef struct pdirectory {
 
	pd_entry m_entries[PAGES_PER_DIR];
} pdirectory;


enum PAGE_PDE_FLAGS {
 
	I86_PDE_PRESENT			=	0x01,		//0000000000000000000000000000001
	I86_PDE_WRITABLE		=	0x02,		//0000000000000000000000000000010
	I86_PDE_KERNEL			=	0x00,		//0000000000000000000000000000000
	I86_PDE_USER			=	0x04,		//0000000000000000000000000000100
	I86_PDE_PWT			    =	0x08,		//0000000000000000000000000001000
	I86_PDE_PCD			    =	0x10,		//0000000000000000000000000010000
	I86_PDE_ACCESSED		=	0x20,		//0000000000000000000000000100000
	I86_PDE_DIRTY			=	0x40,		//0000000000000000000000001000000
	I86_PDE_4MB			    =	0x80,		//0000000000000000000000010000000
	I86_PDE_CPU_GLOBAL		=	0x100,		//0000000000000000000000100000000
	I86_PDE_LV4_GLOBAL		=	0x200,		//0000000000000000000001000000000
   	I86_PDE_FRAME			=	0x7FFFF000 	//1111111111111111111000000000000
};

enum PAGE_PTE_FLAGS {
 
	I86_PTE_PRESENT			=	0x01,		//0000000000000000000000000000001
	I86_PTE_WRITABLE		=	0x02,		//0000000000000000000000000000010
	I86_PTE_KERNEL			=	0x00,		//0000000000000000000000000000000	
	I86_PTE_USER			=	0x04,		//0000000000000000000000000000100
	I86_PTE_WRITETHOUGH		=	0x08,		//0000000000000000000000000001000
	I86_PTE_NOT_CACHEABLE   =	0x10,		//0000000000000000000000000010000
	I86_PTE_ACCESSED		=	0x20,		//0000000000000000000000000100000
	I86_PTE_DIRTY			=	0x40,		//0000000000000000000000001000000
	I86_PTE_PAT			    =	0x80,		//0000000000000000000000010000000
	I86_PTE_CPU_GLOBAL		=	0x100,		//0000000000000000000000100000000
	I86_PTE_LV4_GLOBAL		=	0x200,		//0000000000000000000001000000000
   	I86_PTE_FRAME			=	0x7FFFF000 	//1111111111111111111000000000000
};

extern void				pd_entry_add_attrib (pd_entry* e, uint32_t attrib);
extern void				pd_entry_del_attrib (pd_entry* e, uint32_t attrib);
extern void				pd_entry_set_frame (pd_entry*, physical_addr);
extern bool				pd_entry_is_present (pd_entry e);
extern bool				pd_entry_is_user (pd_entry);
extern bool				pd_entry_is_4mb (pd_entry);
extern bool				pd_entry_is_writable (pd_entry e);
extern physical_addr	pd_entry_pfn (pd_entry e);
extern void				pd_entry_enable_global (pd_entry e);

extern void 			pt_entry_add_attrib (pt_entry* e, uint32_t attrib);
extern void 			pt_entry_del_attrib (pt_entry* e, uint32_t attrib);
extern void 			pt_entry_set_frame (pt_entry*, physical_addr);
extern bool 			pt_entry_is_present (pt_entry e);
extern bool 			pt_entry_is_writable (pt_entry e);
extern physical_addr	pt_entry_pfn (pt_entry e);

void 					vmmngr_initialize ();
void 					enable_paging(uint32_t *dir);













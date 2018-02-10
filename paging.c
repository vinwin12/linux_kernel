/*
	paging.c


*/

#include "paging.h"

/*
		paging_init()

		Description: initializes paging
		Inputs: None
		Outputs: None
		Side Effects: initializes paging
		Inspiration: OSDev.org

		http://wiki.osdev.org/Setting_Up_Paging

		Note: Video Memory address is x
*/
void paging_init()
{
	//set each entry to not present
	int i;
	for(i = 0; i < ONE_KB; i++) // 1KB entries of 4KB size
	{
    	//this sets up each index in the directory, allowing R/W but marked as NOT present for now
    	page_directory[i] = 0x00000002;
    	// 0x3 tells it to enable R/W, since page table is not present.. hence OR with 0x2
    	// first 4 MB broken down into 4kb pages and R/W is set, but marked as not present
		// 0x1000 shifts it to ignore first 12 bits, where i gives physical page address
    	page_table[i] = (i * 0x1000) | 2;
	}

	// attributes: supervisor level (bit 3, not set = not user access), read/write, present
	page_directory[0] = ((unsigned int)page_table | 3);

	/* Map Kernel from Phys to Virt Memory. Turn on R/W and Present bits, set Page Size to 4MB */
	page_directory[1] = (KERNEL_START_ADDR | 0x83);

	// put video memory pointer in the correct page_table entry
	page_table[VIDMEM_START_ADDR>>12] = (VIDMEM_START_ADDR + 3);

	/* set up 3 pages for each terminal's video memory */
	page_table[TERM_1_VIDEO>>12] = (TERM_1_VIDEO + 3);
	page_table[TERM_2_VIDEO>>12] = (TERM_2_VIDEO + 3);
	page_table[TERM_3_VIDEO>>12] = (TERM_3_VIDEO + 3);

	asm volatile("						\n\
	movl %0, %%eax						\n\
	movl %%eax, %%cr3 					\n\
	movl %%cr4, %%eax					\n\
	orl $0x00000010, %%eax				\n\
	movl %%eax, %%cr4					\n\
	movl %%cr0, %%eax					\n\
	orl $0x80000000, %%eax				\n\
	movl %%eax, %%cr0					\n\
	"
	:
	: "r"(page_directory)
	: "eax"
	);
}


/*
Unlike Linux, we will provide you with set physical addresses
for the images of the two tasks, and will stipulate that they
require no more than 4 MB each, so you need only allocate a
single page for each task's user-level memory
*/

/*

		map_task();

		Description: Maps program image
		Inputs: Physical and virtual address
		Outputs: None
		Side Effects: Maps program at virtual address into physical address

*/
void map_task(uint32_t virtual_address, uint32_t physical_address)
{
	uint32_t pd_entry = virtual_address / FOUR_MB ; // 128 MB / 4 MB to find index 32
	page_directory[pd_entry] = (physical_address | 0x87); // sets Present bit, User-level, R/W and 4 MB page size
	flush_tlb();

}

/*

		map_vidmem()

		Description: Maps video memory
		Inputs: Physical and virtual address, as well as the terminal number for this process
		Outputs: None
		Side Effects: Maps vidmem into user space (pre-set virtual address per terminal)

*/
void map_vidmem(uint32_t virtual_address, uint32_t physical_address)
{
	uint32_t pd_entry = virtual_address / FOUR_MB ;
	page_directory[pd_entry] = (unsigned int)vidmap_page_table | 0x7 ; //sets present bit, user-level, R/W
	vidmap_page_table[0] = physical_address | 0x7;
	flush_tlb();
}

/*

		display_terminal()

		Description: Each terminal writes to a virtual page which points to the same
								 address in physical memory. This function will be called whenever
								 ALT + F# is pressed, so it will not only copy its data over to
								 physical Video Memory, but will then point the terminal's virtual
								 address to physical Video Memory, to then write to the screen.

		Inputs: current terminal number, previous terminal number
		Outputs: None

*/
void display_terminal(int curr_term_num, int prev_term_num)
{
	if( curr_term_num == prev_term_num )
		return;

	uint32_t num_bytes = (NUM_COLS * NUM_ROWS * 2);

	int curr_pt_index = (terminals[curr_term_num].vidmem_addr >> 12);

	if( prev_term_num != -1 )
	{
		int prev_pt_index = (terminals[prev_term_num].vidmem_addr >> 12);
		/* restore previous terminal's pointer and video memory */
		page_table[prev_pt_index] = (terminals[prev_term_num].vidmem_addr + 3);
		flush_tlb();
		/* map user video memory back to the specific terminal's page */
		memcpy((uint8_t*)(terminals[prev_term_num].vidmem_addr), (uint8_t*)(VIDMEM_START_ADDR), num_bytes);
	}

	/* copy visible terminal's data over and point to physical video memory */
	memcpy((uint8_t*)(VIDMEM_START_ADDR), (uint8_t*)(terminals[curr_term_num].vidmem_addr), num_bytes);
	page_table[curr_pt_index] = (VIDMEM_START_ADDR + 3);
	flush_tlb();
}

/*
	flush_tlb()

	Description: flushes tlb
	Inputs: None
	Outputs: None
	Side Effects: reloads cr3 register with page directory
	Inspiration: OSDev.org

*/
void flush_tlb()
{
	asm volatile ("				\n\
	movl %%cr3, %%eax			\n\
	movl %%eax, %%cr3			\n\
	"
	:
	:
	:"eax"
	);
}

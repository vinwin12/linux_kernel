#ifndef _PAGING_H
#define _PAGING_H

//#include libraries
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"

/* page/directory/table relevant constants */
#define FOUR_MB 			0x400000
#define ONE_KB				1024
#define FOUR_KB				4096
#define EIGHT_MB      0x800000
#define OFFSET        0x48000
#define ONE_GB        0x40000000


#define KERNEL_START_ADDR 	0x400000
#define VIDMEM_START_ADDR   0xB8000

//pulled from http://wiki.osdev.org/Setting_Up_Paging
//every index in directory is a pointer to a separate page table
//page directory describes entry format http://wiki.osdev.org/Paging
uint32_t page_directory[ONE_KB] __attribute__((aligned(FOUR_KB)));
uint32_t page_table[ONE_KB] __attribute__((aligned(FOUR_KB)));

uint32_t vidmap_page_table[ONE_KB] __attribute__((aligned(FOUR_KB)));

/* Initializes Paging */
void paging_init();
/* Maps from virtual to physical */
void map_task(uint32_t virtual_address, uint32_t physical_address);
/* Maps vidmem into user space (pre-set virtual address) */
void map_vidmem(uint32_t virtual_address, uint32_t physical_address);
/* displays terminal based on ALT + F# */
void display_terminal(int curr_term_num, int prev_term_num);
/* Flushes TLB */
void flush_tlb();
#endif

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "lib.h"
#include "i8259.h"
#include "system_calls.h"

#define SET_PIT_1 0x36
#define SET_PIT_2 0x30
#define PIT_IRQ 0
#define PIT_REG 0x43
#define CH_0_REG 0x40

#define HZ_31 0x965A
#define HZ_18 0xFFFF
#define HZ_40 0x7486

/* stores previous value of curr_idx to restore if shell execution fails */
int restore_curr_idx;

/* variable used to determine which scheduled process to switch to */
int curr_idx;

/* initializes scheduler variables and the PIT */
void scheduler_init();
/* function which performs the context switch between processes */
void scheduler();
/* finds the next terminal index for whichever process should be switched to */
int find_next_process();
/* initializes the PIT device */
void pit_init();
void pit_handler();
int isEmpty();


#endif

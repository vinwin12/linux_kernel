/*
    scheduler.c



 */

#include "scheduler.h"

/*
	pit_init()

	Description: initializes pit
	Inputs: None
	Outputs: None
	Side Effects: Enables pit IRQ, writes to pit ports
  Inspiration: OSDev
*/


void pit_init(){


    // uses not-square-wave-generator mode
    outb(SET_PIT_1, PIT_REG); //0x36

    // uses square wave generator
    // outb(SET_PIT_2,PIT_REG);

    /* using outb twice as the PIT ports for channel 0 are,for write and read, size 8-bits */

    // 31 Hz
    // outb(HZ_31, CH_0_REG);
    // outb((HZ_31 >> 8), CH_0_REG);

    // 18 Hz
    // outb(HZ_18,CH_0_REG);
    // outb(HZ_18 >> 8,CH_0_REG);

    /* THIS ONE WORKS BEST */
    // 40 Hz
    outb(HZ_40, CH_0_REG);
    outb((HZ_40 >> 8), CH_0_REG);


    enable_irq(PIT_IRQ);
}


/*
	pit_handler()

	Description: handler for a PIT interrupt
	Inputs: None
	Outputs: None
	Side Effects: handles PIT interrupt
*/

void pit_handler(){

  send_eoi(PIT_IRQ);

  cli();

  /* condition to prevent Terminal 1 from producing errors */
  if( terminals[curr_idx].current_process == -1 )
  {
      sti();
      return;
  }

  pcb_t * old_pcb = (pcb_t*)(MB8 - (KB8 * (terminals[curr_idx].current_process + 1)));

  //pre-context switch, storing the important information of the process
  asm volatile("  \n\
   movl %%esp, %0 \n\
   movl %%ebp, %1 \n\
  "
  : "=g"(old_pcb->esp), "=g"(old_pcb->ebp)
  );

  scheduler();

  /* HANDLER SHOULD NEVER REACH THIS POINT. THIS IS CONSIDERED A FAILURE*/

  sti();

  return;
}


/*
  scheduler()

  Description: This function switches from one process to another process
  Inputs: None
  Outputs: None
  Side Effects: Context switches to the next scheduled process

*/

void scheduler()
{
    /* update curr_idx */
    curr_idx = find_next_process();

    /* determine the process ID of this terminal's current process */
    int term_process = terminals[curr_idx].current_process;

    /* find the next PCB so we can switch the ESP and EBP */
    pcb_t * next_pcb = (pcb_t*)(MB8 - (KB8 * (term_process + 1)));

    /* set necessary TSS information (really only esp0 matters) */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = (MB8 - (KB8 * term_process)) - 4;

    /* restore paging and user video memory mapping for the new process */
    map_task(MB128, (MB8 + (term_process * MB4)));
    if( terminals[curr_idx].is_visible )
        map_vidmem(terminals[curr_idx].user_vidmem_addr, VIDMEM_START_ADDR);
    else
        map_vidmem(terminals[curr_idx].user_vidmem_addr, terminals[curr_idx].vidmem_addr);

    sti();

    asm volatile("							                   \n\
    mov %0, %%esp							                     \n\
	  mov %1, %%ebp							                     \n\
    # jump back to current process program counter \n\
	  leave 						                             \n\
    ret                                            \n\
   	"
    :
    : "r"(next_pcb->esp), "r"(next_pcb->ebp)
    );
}

/*

  find_next_process()

  Description: finds the terminal index of the next scheduled process
  Inputs: None
  Outputs: index to the terminal array

*/
int find_next_process()
{
  int temp = curr_idx;
  int i;
  for( i = ((curr_idx + 1) % NUM_TERMINALS); i != curr_idx; i = ((i + 1) % NUM_TERMINALS) )
  {
      /* check to see if the terminal is even running */
      if( terminals[i].has_been_launched )
      {
          temp = i;
          break;
      }
  }
  return temp;

}

/*
  scheduler_init()

  Description: initializes scheduling / PIT device
  Inputs: none
  Outputs: none

*/
void scheduler_init()
{
    curr_idx = 0;
    pit_init();
}

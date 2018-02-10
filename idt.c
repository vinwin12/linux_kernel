/*

    idt.c

*/

#include "idt.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
#include "mouse.h"
#include "scheduler.h"

#define SYSCALL_INDEX 0x80

/*IDT_init
* Sets all idt entries
* INPUT: none
* OUTPUT: none
* SIDE EFFECT: fills idt
*/
void IDT_init()
{
  int i;

  lidt(idt_desc_ptr);

  SET_IDT_ENTRY(idt[0], DIVIDE_ERROR_EXCEPTION);
  SET_IDT_ENTRY(idt[1], DEBUG_EXCEPTION);
  SET_IDT_ENTRY(idt[2], NMI_INTERRUPT);
  SET_IDT_ENTRY(idt[3], BREAKPOINT_EXCEPTION);
  SET_IDT_ENTRY(idt[4], OVERFLOW_EXCEPTION);
  SET_IDT_ENTRY(idt[5], BOUND_RANGE_EXCEEDED_EXCEPTION);
  SET_IDT_ENTRY(idt[6], INVALID_OPCODE_EXCEPTION);
  SET_IDT_ENTRY(idt[7], DEVICE_NOT_AVAILABLE_EXCEPTION);
  SET_IDT_ENTRY(idt[8], DOUBLE_FAULT_EXCEPTION);
  SET_IDT_ENTRY(idt[9], COPROCESSOR_SEGMENT_OVERRUN);
  SET_IDT_ENTRY(idt[10], INVALID_TSS_EXCEPTION);
  SET_IDT_ENTRY(idt[11], SEGMENT_NOT_PRESENT);
  SET_IDT_ENTRY(idt[12], STACK_FAULT_EXCEPTION);
  SET_IDT_ENTRY(idt[13], GENERAL_PROTECTION_EXCEPTION);
  SET_IDT_ENTRY(idt[14], PAGE_FAULT_EXCEPTION);
  // idt[15] reserved
  SET_IDT_ENTRY(idt[16], X87_FPU_FLOATING_POINT_ERROR);
  SET_IDT_ENTRY(idt[17], ALIGNMENT_CHECK_EXCEPTION);
  SET_IDT_ENTRY(idt[18], MACHINE_CHECK_EXCEPTION);
  SET_IDT_ENTRY(idt[19], SIMD_FLOATING_POINT_EXCEPTION);

  for(i = 0; i < NUM_VEC; i++)
  {
		// Set interruption vector present
		idt[i].seg_selector = KERNEL_CS;
		idt[i].reserved4 = 0;
    // first 32 descriptors are traps, rest are interrupts
    if(i > 31)
    {
			idt[i].reserved3 = 0;
			SET_IDT_ENTRY(idt[i], GENERAL_INTERRUPT);
		}
    else
    {
      idt[i].reserved3 = 1;
    }
		idt[i].reserved2 = 1;
		idt[i].reserved1 = 1;
		idt[i].size = 1;
		idt[i].reserved0 = 0;
		idt[i].dpl = 0;
    idt[i].present = 1;

		// syscall comes from user privilege (3)
		if(SYSCALL_INDEX == i)
    {
			idt[i].dpl = 3;
		}
  }
  /*
    Keyboard, RTC, mouse (not implemented) and PIT interrupt handlers

    NOTE: These function names are declared in
          inthandler.c/.h, since they use assembly
          to return from interrupts correctly
  */
  SET_IDT_ENTRY(idt[0x21], INT_HANDLER_33);  //keyboard
  SET_IDT_ENTRY(idt[0x28], INT_HANDLER_40);  //rtc
  SET_IDT_ENTRY(idt[0x2C], INT_HANDLER_44);  //mouse
  SET_IDT_ENTRY(idt[0x20], INT_HANDLER_32);  //pit

  /* Set up System Call Interrupt Handler */
  SET_IDT_ENTRY(idt[0x80], SYSCALL_INTERRUPT);
}


/*

    Exception/Interrupt Handlers

    The following functions all follow the
    general form:

    Inputs: None
    Outputs: None
    Side Effects: Either causes "blue screen of death"
                  or faults, in which case it will fix
                  the error and continue execution

*/

// idt[32-255]
void GENERAL_INTERRUPT()
{
  printf("GENERAL_INTERRUPT\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[0]
void DIVIDE_ERROR_EXCEPTION()
{
  printf("DIVIDE_ERROR_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[1]
void DEBUG_EXCEPTION()
{
  printf("DEBUG_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[2]
void NMI_INTERRUPT()
{
  printf("NMI_INTERRUPT\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[3]
void BREAKPOINT_EXCEPTION()
{
  printf("BREAKPOINT_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[4]
void OVERFLOW_EXCEPTION()
{
  printf("OVERFLOW_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[5]
void BOUND_RANGE_EXCEEDED_EXCEPTION()
{
  printf("BOUND_RANGE_EXCEEDED_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[6]
void INVALID_OPCODE_EXCEPTION()
{
  printf("INVALID_OPCODE_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[7]
void DEVICE_NOT_AVAILABLE_EXCEPTION()
{
  printf("DEVICE_NOT_AVAILABLE_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[8]
void DOUBLE_FAULT_EXCEPTION()
{
  printf("DOUBLE_FAULT_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[9]
void COPROCESSOR_SEGMENT_OVERRUN()
{
  printf("COPROCESSOR_SEGMENT_OVERRUN\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[10]
void INVALID_TSS_EXCEPTION()
{
  printf("INVALID_TSS_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[11]
void SEGMENT_NOT_PRESENT()
{
  printf("SEGMENT_NOT_PRESENT\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[12]
void STACK_FAULT_EXCEPTION()
{
  printf("STACK_FAULT_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[13]
void GENERAL_PROTECTION_EXCEPTION()
{
  printf("GENERAL_PROTECTION_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[14]
void PAGE_FAULT_EXCEPTION()
{
  printf("PAGE_FAULT_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[15] is reserved by Intel

// idt[16]
void X87_FPU_FLOATING_POINT_ERROR()
{
  printf("X87_FPU_FLOATING_POINT_ERROR\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[17]
void ALIGNMENT_CHECK_EXCEPTION()
{
  printf("ALIGNMENT_CHECK_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[18]
void MACHINE_CHECK_EXCEPTION()
{
  printf("MACHINE_CHECK_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

// idt[19]
void SIMD_FLOATING_POINT_EXCEPTION()
{
  printf("SIMD_FLOATING_POINT_EXCEPTION\n");
  halt(HALT_BY_EXCEPTION);
}

/*
    void error_hang()

    Description: this function disables all interrupts, so that
                 when a "blue screen of death" instance occurs,
                 NO other interrupts are allowed to occur.
    Inputs: None
    Outputs: None
    Side Effects: disables all interrupts

    USED PRE-CHECKPOINT 4/5
*/
void error_hang()
{
  /* disable keyboard interrupts */
  disable_irq(KEYBOARD_INT_NUM);
  /* disable RTC interrupts */
  disable_irq(RTC_INT_NUM);
  /* disable mouse interrupts */
  disable_irq(MOUSE_INT_NUM);
  /* disable mouse interrupts */
  disable_irq(PIT_IRQ);
  /* hang */
  while(1);
}


#ifndef _IDT_H
#define _IDT_H


#include "x86_desc.h"
#include "types.h"
#include "lib.h"
#include "int_handler.h"
#include "system_calls.h"
#include "keyboard.h"
#include "rtc.h"

#define HALT_BY_EXCEPTION 255

void IDT_init();

/* Exeption handlers for traps */
void GENERAL_INTERRUPT();
void DIVIDE_ERROR_EXCEPTION();
void DEBUG_EXCEPTION();
void NMI_INTERRUPT();
void BREAKPOINT_EXCEPTION();
void OVERFLOW_EXCEPTION();
void BOUND_RANGE_EXCEEDED_EXCEPTION();
void INVALID_OPCODE_EXCEPTION();
void DEVICE_NOT_AVAILABLE_EXCEPTION();
void DOUBLE_FAULT_EXCEPTION();
void COPROCESSOR_SEGMENT_OVERRUN();
void INVALID_TSS_EXCEPTION();
void SEGMENT_NOT_PRESENT();
void STACK_FAULT_EXCEPTION();
void GENERAL_PROTECTION_EXCEPTION();
void PAGE_FAULT_EXCEPTION();
void X87_FPU_FLOATING_POINT_ERROR();
void ALIGNMENT_CHECK_EXCEPTION();
void MACHINE_CHECK_EXCEPTION();
void SIMD_FLOATING_POINT_EXCEPTION();

/*
	Function which disables Keyboard and RTC interrupts
	when an error occurs: a "blue screen of death"
*/
void error_hang();

#endif

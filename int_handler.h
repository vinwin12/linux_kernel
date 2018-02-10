/*
	int_handler.h

	Created by: Thomas Korenchan and Vinay Vemuri
*/

#ifndef _INT_HANDLER_H
#define _INT_HANDLER_H

#include "x86_desc.h"
#include "system_calls.h"

/* Interrupt Handler for the Keyboard */
void INT_HANDLER_33();

/* Interrupt Handler for the RTC */
void INT_HANDLER_40();

/* Interrupt Handler for the mouse */
void INT_HANDLER_44();

/* Interrupt Handler for the PIT */
void INT_HANDLER_32();

/* System Call Interrup Handler */
void SYSCALL_INTERRUPT();

#endif

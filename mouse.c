#include "mouse.h"



/*
	mouse_init()

	Description: Initializes mouse use
	Inputs: None
	Outputs: None
	Side Effects: enables mouse IRQs
*/
void mouse_init()
{
    /* enable (unmask) IRQ1 for keyboard interrupts */
    printf("mouse initialization");

    enable_irq(MOUSE_INT_NUM);
}


void mouse_handler() {
    cli();
    printf("mouse interrupt");
    send_eoi(MOUSE_INT_NUM);
    sti();
}

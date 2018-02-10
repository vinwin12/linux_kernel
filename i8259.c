/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

#define INIT_MASK 	0xFF
#define SLAVE_IRQ 	2

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void)
{
	/* Mask All IRQs */
	outb(INIT_MASK, MASTER_8259_PORT + 1);

	outb(ICW1, MASTER_8259_PORT);
	outb(ICW1, SLAVE_8259_PORT);

	outb(ICW2_MASTER, MASTER_8259_PORT + 1);
	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);

	outb(ICW3_MASTER, MASTER_8259_PORT + 1);
	outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);

	outb(ICW4, MASTER_8259_PORT + 1);
	outb(ICW4, SLAVE_8259_PORT + 1);

	/* Enable IRQs on the Slave PIC */
	enable_irq(SLAVE_IRQ);
}

/*
	Enable (unmask) the specified IRQ

	Note: IRQs are ACTIVE LOW

	Code is based off of Linux's
	/x86/kernel/i8259.c
*/
void enable_irq(uint32_t irq_num)
{
	unsigned int mask;

	/* Check if valid IRQ number */
	if( (irq_num < 0) || (irq_num > 15) )
		return;

	/* Create mask based on either Master or Slave PIC */
	if( irq_num >= 8 )
		mask = ~(1 << (irq_num - 8));
	else
		mask = ~(1 << irq_num);

	/* IRQ belongs to Slave PIC */
	if(irq_num & 8)
	{
		slave_mask &= mask;
		outb(slave_mask, SLAVE_8259_PORT + 1);
	}
	/* IRQ belongs to Master PIC */
	else
	{
		master_mask &= mask;
		outb(master_mask, MASTER_8259_PORT + 1);
	}
}

/*
	Disable (mask) the specified IRQ

	Note: IRQs are INACTIVE HIGH

	Code is based off of Linux's
	/x86/kernel/i8259.c
*/
void disable_irq(uint32_t irq_num)
{
	unsigned int mask;

	/* Check if valid IRQ number */
	if( (irq_num < 0) || (irq_num > 15) )
		return;

	/* Create mask based on either Master or Slave PIC */
	if( irq_num >= 8 )
		mask = (1 << (irq_num - 8));
	else
		mask = (1 << irq_num);

	/* IRQ belongs to Slave PIC */
	if(irq_num & 8)
	{
		slave_mask |= mask;
		outb(slave_mask, SLAVE_8259_PORT + 1);
	}
	/* IRQ belongs to Master PIC */
	else
	{
		master_mask |= mask;
		outb(master_mask, MASTER_8259_PORT + 1);
	}
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num)
{
	/* IRQ belongs to Master PIC */
	if( (irq_num >= 0) && (irq_num <= 7) )
	{
		outb((EOI | irq_num), MASTER_8259_PORT);
	}

	/* IRQ belongs to Slave PIC */
	if( (irq_num >= 8) && (irq_num <= 15) )
	{
		outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
		/* Must write to the Master PIC too (IRQ2) */
		outb(EOI | 2, MASTER_8259_PORT);
	}
}

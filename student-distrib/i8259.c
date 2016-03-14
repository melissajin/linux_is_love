/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void
i8259_init(void)
{

	/* Save masks */
	master_mask = inb(MASTER_8259_PORT_DATA); 
	slave_mask = inb(SLAVE_8259_PORT_DATA);

	/* ICW1: Start PIC initization sequence, no ICW4, 2 PICs, edge triggered */
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW1, SLAVE_8259_PORT);

	/* ICW2: map master IRQ0-7 to 0x20-0x27, slave IRQ0-7 to 0x28-0x2f for IRQs */
	outb(ICW2_MASTER, MASTER_8259_PORT_DATA);
	outb(ICW2_SLAVE, SLAVE_8259_PORT_DATA);

	/* ICW3: tell master that slave is IRQ2, tell slave its identity */
	outb(ICW3_MASTER, MASTER_8259_PORT_DATA);
	outb(ICW3_SLAVE, SLAVE_8259_PORT_DATA);

	/* ICW4 */
	outb(ICW4, MASTER_8259_PORT_DATA);
	outb(ICW4, SLAVE_8259_PORT_DATA);

	/* Restore masks */
	outb(master_mask, MASTER_8259_PORT_DATA);
	outb(slave_mask, SLAVE_8259_PORT_DATA);
}

/* Enable (unmask) the specified IRQ */
/* Source: wiki.osdev.org/8259_PIC#Initialisation */
void
enable_irq(uint32_t irq_num)
{
	uint16_t port;
	uint8_t value;

	if(irq_num < 8){
		port = MASTER_8259_PORT_DATA;
	}
	else{
		port = SLAVE_8259_PORT_DATA;
		irq_num -= 8;
	}

	value = inb(port) & ~(1 << irq_num);
	outb(value, port);
}

/* Disable (mask) the specified IRQ */
/* Source: wiki.osdev.org/8259_PIC#Initialisation */
void
disable_irq(uint32_t irq_num)
{
	uint16_t port;
	uint8_t value;

	if(irq_num < 8){
		port = MASTER_8259_PORT_DATA;
	}
	else{
		port = SLAVE_8259_PORT_DATA;
		irq_num -= 8;
	}

	value = inb(port) | (1 << irq_num);
	outb(value, port);
}

/* Send end-of-interrupt signal for the specified IRQ */
/* Source: wiki.osdev.org/8259_PIC#Initialisation */
void
send_eoi(uint32_t irq_num)
{
	if(irq_num >= 8){
		outb(EOI, SLAVE_8259_PORT); /* Send EOI to slave pic */
	}
	outb(EOI, MASTER_8259_PORT); /* Send EOI to master pic */
}


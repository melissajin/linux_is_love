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
	outb(MASTER_8259_PORT, ICW1);
	outb(SLAVE_8259_PORT, ICW1);

	/* ICW2: map master IRQ0-7 to 0x20-0x27, slave IRQ0-7 to 0x28-0x2f for IRQs */
	outb(MASTER_8259_PORT_DATA, ICW2_MASTER);
	outb(SLAVE_8259_PORT_DATA, ICW2_SLAVE);

	/* ICW3: tell master that slave is IRQ2, tell slave its identity */
	outb(MASTER_8259_PORT_DATA, ICW3_MASTER);
	outb(SLAVE_8259_PORT_DATA, ICW3_SLAVE);

	/* ICW4 */
	outb(MASTER_8259_PORT_DATA, ICW4);
	outb(SLAVE_8259_PORT_DATA, ICW4);

	/* Restore masks */
	outb(MASTER_8259_PORT_DATA, master_mask);
	outb(SLAVE_8259_PORT_DATA, slave_mask);
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
	outb(port, value);
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
	outb(port, value);
}

/* Send end-of-interrupt signal for the specified IRQ */
/* Source: wiki.osdev.org/8259_PIC#Initialisation */
void
send_eoi(uint32_t irq_num)
{
	if(irq_num >= 8){
		outb(SLAVE_8259_PORT, EOI); /* Send EOI to slave pic */
	}
	outb(MASTER_8259_PORT, EOI) /* Send EOI to master pic */
}


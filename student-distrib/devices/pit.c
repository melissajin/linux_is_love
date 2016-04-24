#include "pit.h"
#include "../i8259.h"
#include "../idt_set.h"
#include "../lib.h"
#include "../fs.h"
#include "../process.h"
#include "../sys_calls.h"

#define PIT_CMD_PORT 0x40
#define PIT0_DATA_PORT 0x43
#define PIT_0_RESET 0x31
#define PIT_0_LATCH 0x00
#define PIT_IRQ_NUM 0
#define DEFAULT_RATE 100
#define INPUT_CLK 1193180

#define LOWER_B 0xFF
#define UPPER_B 8

extern void pit_isr();

uint8_t pit_ticks = 0;
uint16_t pit_rate = 0;

//initializes the pit
void pit_init(){
	set_int_gate(0x20, (unsigned long) pit_isr);
	
	pit_rate = INPUT_CLK / DEFAULT_RATE;

	cli();
	outb(PIT_0_RESET,PIT_CMD_PORT);
	outb((uint8_t)(pit_rate & LOWER_B), PIT0_DATA_PORT);
	outb((uint8_t)(pit_rate >> UPPER_B), PIT0_DATA_PORT);
	sti();

	enable_irq(PIT_IRQ_NUM);
}

//pit interrupt handler
void PIT_handler_main(){
	if(pit_ticks++ == 9){ //10 ticks is an arbitrary number maybe 5ish?
		pit_ticks = 0;
	}
	//resets the counter for the timer
	cli();
	outb(PIT_0_RESET, PIT_CMD_PORT);
	outb((uint8_t)(pit_rate & LOWER_B), PIT0_DATA_PORT);
	outb((uint8_t)(pit_rate >> UPPER_B), PIT0_DATA_PORT);
	sti();
	
  	send_eoi(PIT_IRQ_NUM);
}

//sets the counter to rate(hz), change will happen after the most recent tick is done
void pit_set_rate(uint16_t rate){
	
	pit_rate =  INPUT_CLK/rate;
}

//gets the current count
uint16_t pit_get_count(){
	uint16_t retval;
	uint16_t temp;
	
	cli();
	outb(PIT_CMD_PORT, PIT_0_LATCH);
	retval = (uint16_t)inb(PIT0_DATA_PORT);
	temp = (uint16_t)inb(PIT0_DATA_PORT) << 8;
	sti();
	
	return retval | temp;
}

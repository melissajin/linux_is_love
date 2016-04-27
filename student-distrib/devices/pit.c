#include "pit.h"
#include "../i8259.h"
#include "../idt_set.h"
#include "../lib.h"
#include "../fs.h"
#include "../process.h"
#include "../sys_calls.h"
#include "../virtualmem.h"
#include "../x86_desc.h"
#include "keyboard.h"

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

static void run(pcb_t * prev, uint8_t * command);

uint16_t pit_rate = 0;
uint8_t * next_execute = NULL;

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
void pit_handler_main(){
	int32_t prev_pid, next_pid, running_term, i;
	pcb_t * prev, * next;
	uint8_t * command;

	send_eoi(PIT_IRQ_NUM);

	pcb(prev);

	/* if execution scheduled */
	if(next_execute != NULL) {
		command = next_execute;
		next_execute = NULL;

		//resets the counter for the timer
		outb(PIT_0_RESET, PIT_CMD_PORT);
		outb((uint8_t)(pit_rate & LOWER_B), PIT0_DATA_PORT);
		outb((uint8_t)(pit_rate >> UPPER_B), PIT0_DATA_PORT);

		run(prev, command);

		return;
	}

	/* scheduling logic */
  	prev_pid = prev -> pid;
  	running_term = prev -> term_num;

  	for(i = 1; i <= MAX_TERMINALS; i++) {
  		next_pid = get_active_process((running_term + i) % MAX_TERMINALS);
  		if(next_pid != -1) break;
  	}

  	next = (pcb_t *) (KERNEL_MEM_END - PCB_SIZE * (next_pid + 1));

	//resets the counter for the timer
	outb(PIT_0_RESET, PIT_CMD_PORT);
	outb((uint8_t)(pit_rate & LOWER_B), PIT0_DATA_PORT);
	outb((uint8_t)(pit_rate >> UPPER_B), PIT0_DATA_PORT);
	
  	// send_eoi(PIT_IRQ_NUM);

  	if(!processes()) return;
  	if(next_pid == -1) return;
  	if(prev_pid == next_pid) return;

	context_switch(prev, next);
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

void schedule_for_execution(uint8_t * command) {
	next_execute = command;
}

void run(pcb_t * prev, uint8_t * command) {
	asm volatile("					\n\
		movl	%%ebp, %[prev_ebp]	\n\
		movl	%%esp, %[prev_esp]	\n\
		movl	$1f, %[prev_eip]	\n\
		pushl	%[cmd_ptr]			\n\
		exec:						\n\
		call	execute				\n\
		jmp		exec				\n\
									\n\
		1:							\n\
		# movl	%[prev_ebp], %%ebp	\n\
		"
		: [prev_esp] "=m" (prev -> context.esp),
		  [prev_eip] "=m" (prev -> context.eip),
		  [prev_ebp] "=m" (prev -> context.ebp)
		: [cmd_ptr] "r" (command)
	);
}

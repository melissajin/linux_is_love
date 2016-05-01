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

/*helper function to reset count in channel 0 mode 0*/
static void pit_reset_count();

uint16_t pit_rate = 0; //global variable for the pit rate in hz
uint8_t * next_execute = NULL; //next process to execute

/* 
void pit_init()
DESCRIPTION:
	initializes the programmable interval timer
INPUT: none
OUTPUT: none
RETURN VALUES: none
SIDE EFFECTS:
	pit interrupt is turned on
*/
void pit_init(){
	//setting up interrupt gate to proper handler
	set_int_gate(0x20, (unsigned long) pit_isr);
	
	//proper conversion from the input_clk counter to hertz
	pit_rate = INPUT_CLK / DEFAULT_RATE;

	cli();
	pit_reset_count();
	sti();
	
	//enabling
	enable_irq(PIT_IRQ_NUM);
}

/* 
void pit_handler_main()
DESCRIPTION:
	the interrupt handler for the programmable interval timer, also functions as the round-robin scheduler
INPUT: none
OUTPUT: none
RETURN VALUES: none
SIDE EFFECTS:
	Once pit fires, initiates a context switch to run another program on a different terminal to run in a round robin fashion
*/
void pit_handler_main(){
	int32_t prev_pid, next_pid, running_term, i;
	pcb_t * prev, * next;
	uint8_t * command;
	//reset early so that we do not miss any interrupts
	send_eoi(PIT_IRQ_NUM);
	
	//no other processses so no context switch
	if(!processes()) {
		pit_reset_count();
		return;
	}

	pcb(prev);
	
	//storing the process switching from's esp and ebp into the pcb
	asm volatile("						\n\
		movl	%%esp, %[prev_esp]			\n\
		movl	%%ebp, %[prev_ebp]			\n\
		"
		: [prev_esp] "=m" (prev -> context.esp),
		  [prev_ebp] "=m" (prev -> context.ebp)
	);

	/* if execution scheduled */
	if(next_execute != NULL) {
		command = next_execute;
		next_execute = NULL;

		pit_reset_count();

		execute((uint8_t *) "shell");
	}

	/* scheduling logic */
  	prev_pid = prev -> pid;
  	running_term = prev -> term_num;

  	for(i = 1; i <= MAX_TERMINALS; i++) {
  		next_pid = get_active_process((running_term + i) % MAX_TERMINALS);
  		if(next_pid != -1) break;
  	}

  	next = (pcb_t *) (KERNEL_MEM_END - KERNEL_STACK_SIZE * (next_pid + 1));

	pit_reset_count();
	
	//case there is only one process running
  	if(prev_pid == next_pid) return;

	set_pd(next -> pd);

	//setting the esp and ebp and tss_esp0 to contain that of the process switching to
	asm volatile("							\n\
		movl	%[next_esp], %%esp			\n\
		movl	%[next_ebp], %%ebp 			\n\
		movl	%[next_esp0], %[tss_esp0]	\n\
		"
		: [tss_esp0] "=m" (tss.esp0)
		: [next_esp] "rm" (next -> context.esp),
		  [next_ebp] "rm" (next -> context.ebp),
		  [next_esp0] "r" (next -> context.esp0)
	);
}

/*
void pit_set_rate(uint16_t rate)
DESCRIPTION: sets the rate of the PIT firing to the argument (in hertz
INPUT:
	uint16_t rate - the rate of firing to set to in hertz
OUTPUT:none
RETURN: none
SIDE EFFECTS: 
	changes the pit rate of firing
*/
void pit_set_rate(uint16_t rate){
	
	pit_rate =  INPUT_CLK/rate;
}

/* 
uint16_t pit_get_count()
DESCRIPTION: gets the current count of the PIT (amount of cycles before it fires)
INPUT:none
OUTPUT: none
RETURN: the current count of the PIT
SIDE EFFECTS:
	will pause the PIT until both bytes are read
*/
uint16_t pit_get_count(){
	uint16_t retval;
	uint16_t temp;
	
	cli();
	//pauses the pit so that the count does not change while reading
	outb(PIT_CMD_PORT, PIT_0_LATCH);

	//reads are 8 byte each so have to do some bit work (lowbyte then hibyte)
	retval = (uint16_t)inb(PIT0_DATA_PORT);
	temp = (uint16_t)inb(PIT0_DATA_PORT) << 8;
	sti();
	
	return retval | temp;
}

/*
void pit_reset_count()
DESCRIPTION: resets the count of the pit (necessary because we are using mode 0)
INPUT: 
	int pit_rate - the global variable that represents the rate at which the PIT fires in hz
OUTPUT: none
RETURN: none
SIDE EFFECTS:
	Once the PIT sends an interrupt, it will not send another until the pit is reset
*/
void pit_reset_count() {
	//sends proper command to indicate that we are sending the count as lo/hi, for channel 3 in mode 0
	outb(PIT_0_RESET, PIT_CMD_PORT);
	
	//two bytes that represent count
	outb((uint8_t)(pit_rate & LOWER_B), PIT0_DATA_PORT);
	outb((uint8_t)(pit_rate >> UPPER_B), PIT0_DATA_PORT);
}

/*
void schedule_for_execution(uint8_t * command)
DESCRIPTION: schedules the command for execution in the next scheduler cycle
INPUT:
	uint8_t* command - the command to be executed
OUTPUT: none
RETURN: none
SIDE EFFECTS:
	next scheduler cycle will execute this command
*/
void schedule_for_execution(uint8_t * command) {
	next_execute = command;
}

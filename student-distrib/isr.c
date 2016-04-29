#include "isr.h"
#include "lib.h"
#include "idt_set.h"
#include "sys_calls.h"
#include "process.h"
#include "devices/keyboard.h"
#include "virtualmem.h"

#define NUM_IRQS 16

struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
};

/* Sets error message to print out when exception is recieved */
static char * exception_messages[] = 
{
	"Division by zero",
	"Debugger",
	"NMI",
	"Breakpoint",
	"Overflow",
	"Bounds",
	"Invalid Opcode",
	"Coprocessor not availible",
	"Double fault",
	"Coprocessor segment overrun",
	"Invalid TSS",
	"Segment not present",
	"Stack fault",
	"General protection fault",
	"Page fault",
	"Unknown interrupt",
	"Math fault",
	"Alignment Check",
	"Machine check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
};

extern void isr0(); //Division by zero
extern void isr1(); //Debugger
extern void isr2(); //Non Maskable Interrupt(NMI)
extern void isr3(); //Breakpoint
extern void isr4(); //Overflow
extern void isr5(); //Bounds
extern void isr6(); //Invalid Opcode
extern void isr7(); //Coprocessor not available
extern void isr8(); //Double fault
extern void isr9(); //Coprocessor segment Overrun
extern void isr10(); //invalid TSS
extern void isr11(); //segment not present
extern void isr12(); //stack fault
extern void isr13(); //general protection fault
extern void isr14(); //page fault
extern void isr15(); //unknown interrupt
extern void isr16(); //math fault
extern void isr17(); //Alignment check
extern void isr18(); //Machine check
extern void isr19(); //REST ARE RESERVED
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void sys_call();

static void irq_handler_default();

uint32_t irq_table[NUM_IRQS];

/* isrs_install
 * DESC: C-function that initializes the first 32 gates int the IDT for the exceptions
 * INPUT: none
 * OUTPUT: none
 * RETURN: none
 * SIDE EFFECTS: Sets the first 32 spots on the idt table
 */
void isrs_install(){
	int i;

	set_trap_gate(0,  (unsigned long) isr0);
	set_trap_gate(1,  (unsigned long) isr1);
	set_int_gate(2,  (unsigned long) isr2);
	set_sys_int_gate(3,  (unsigned long) isr3);
	set_sys_gate(4,  (unsigned long) isr4);
	set_sys_gate(5,  (unsigned long) isr5);
	set_trap_gate(6,  (unsigned long) isr6);
	set_trap_gate(7,  (unsigned long) isr7);
	set_trap_gate(8,  (unsigned long) isr8);
	set_trap_gate(9,  (unsigned long) isr9);
	set_trap_gate(10,  (unsigned long) isr10);
	set_trap_gate(11,  (unsigned long) isr11);
	set_trap_gate(12,  (unsigned long) isr12);
	set_trap_gate(13,  (unsigned long) isr13);
	set_int_gate(14,  (unsigned long) isr14);
	set_trap_gate(15,  (unsigned long) isr15);
	set_trap_gate(16,  (unsigned long) isr16);
	set_trap_gate(17,  (unsigned long) isr17);
	set_trap_gate(18,  (unsigned long) isr18);
	set_trap_gate(19,  (unsigned long) isr19);
	set_trap_gate(20,  (unsigned long) isr20);
	set_trap_gate(21,  (unsigned long) isr21);
	set_trap_gate(22,  (unsigned long) isr22);
	set_trap_gate(23,  (unsigned long) isr23);
	set_trap_gate(24,  (unsigned long) isr24);
	set_trap_gate(25,  (unsigned long) isr25);
	set_trap_gate(26,  (unsigned long) isr26);
	set_trap_gate(27,  (unsigned long) isr27);
	set_trap_gate(28,  (unsigned long) isr28);
	set_trap_gate(29,  (unsigned long) isr29);
	set_trap_gate(30,  (unsigned long) isr30);
	set_trap_gate(31,  (unsigned long) isr31);
	set_sys_gate(0x80, (unsigned long) sys_call);

	/* start irqs at idt entry 32 */
	set_int_gate(32,  (unsigned long) irq0);
	set_int_gate(33,  (unsigned long) irq1);
	set_int_gate(34,  (unsigned long) irq2);
	set_int_gate(35,  (unsigned long) irq3);
	set_int_gate(36,  (unsigned long) irq4);
	set_int_gate(37,  (unsigned long) irq5);
	set_int_gate(38,  (unsigned long) irq6);
	set_int_gate(39,  (unsigned long) irq7);
	set_int_gate(40,  (unsigned long) irq8);
	set_int_gate(41,  (unsigned long) irq9);
	set_int_gate(42,  (unsigned long) irq10);
	set_int_gate(43,  (unsigned long) irq11);
	set_int_gate(44,  (unsigned long) irq12);
	set_int_gate(45,  (unsigned long) irq13);
	set_int_gate(46,  (unsigned long) irq14);
	set_int_gate(47,  (unsigned long) irq15);

	for(i = 0; i < NUM_IRQS; i++) {
		irq_table[i] = (uint32_t) irq_handler_default;
	}
}
/*fault_handler
 *DESC: C-function that handles any exceptions, called by an assembly linkage
 *INPUT: a register structure that has the state of the machine and which error included
 *OUTPUT: prints "Exception <exception#>: <exception message> " on a known exception(one of the first 32)
 *	  prints "Unknown exception <exception#>" on an unknown exception
 *RETURN: none
 *SIDE EFFECT: Spins indefinately at aka and blue screens
 */
void fault_handler(struct regs * r){
	pcb_t * pcb;
	terminal_t * terminal;

	if(processes()) {
		pcb(pcb);
		terminal = get_terminal(pcb -> term_num);

		set_screen_x(terminal -> screen.x);
		set_screen_y(terminal -> screen.y);
		set_video_mem(terminal -> screen.video_mem + PAGE_SIZE * (pcb -> term_num + 1));

		printf("Process terminated with exception %d: %s (%d)\n",
			r -> int_no,
			exception_messages[r -> int_no],
			r -> err_code);

		terminal -> screen.x = get_screen_x();
		terminal -> screen.y = get_screen_y();
		terminal -> screen.video_mem = get_video_mem() - PAGE_SIZE * (pcb -> term_num + 1);

		halt(1);
	} else {
		printf("Process terminated with exception %d: %s (%d)\n",
			r -> int_no,
			exception_messages[r -> int_no],
			r -> err_code);

		while(1);
	}
	
}

/*sys_call_handler
 *DESC: C-function that handles any sys_call, called by an assembly linkage
 *INPUT: int syscall- which syscall number it is
 *OUTPUT: prints "SYSTEM CALL <syscall#> "
 *RETURN: none
 *SIDE EFFECT: Spins indefinately at end aka blue screens
 */
void sys_call_handler(int syscall){
	printf("SYSTEM CALL %d\n", syscall);
	while(1);
}

void irq_handler_default() {}

void add_irq(uint32_t irq, uint32_t handler_addr) {
	if(irq < 0 || irq >= NUM_IRQS) return;
	irq_table[irq] = handler_addr;
}

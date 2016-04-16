#include "isr.h"
#include "lib.h"
#include "idt_set.h"

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
extern void sys_call();

/* isrs_install
 * DESC: C-function that initializes the first 32 gates int the IDT for the exceptions
 * INPUT: none
 * OUTPUT: none
 * RETURN: none
 * SIDE EFFECTS: Sets the first 32 spots on the idt table
 */
void isrs_install(){
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
	if(r -> int_no < 32)
	{
		printf("Exception %d: ", r -> int_no);
		printf(exception_messages[r -> int_no]);
		printf("\n");
		printf("Code: %d\n", r -> err_code);
	}
	else
	{
		printf("Unknown exception %d\n", r -> int_no);
	}

	while(1);
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

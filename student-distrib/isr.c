#include "isr_init.S"

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr31();
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

/* Set first 32 entries in IDT to Intel's defined exceptions */
void isrs_install(){
	set_trap_gate(0, (unsigned) isr0);
	set_trap_gate(1, (unsigned) isr1);
	set_trap_gate(2, (unsigned) isr2);
	set_trap_gate(3, (unsigned) isr3);
	set_trap_gate(4, (unsigned) isr4);
	set_trap_gate(5, (unsigned) isr5);
	set_trap_gate(6, (unsigned) isr6);
	set_trap_gate(7, (unsigned) isr7);
	set_trap_gate(8, (unsigned) isr8);
	set_trap_gate(9, (unsigned) isr9);
	set_trap_gate(10, (unsigned) isr10);
	set_trap_gate(11, (unsigned) isr11);
	set_trap_gate(12, (unsigned) isr12);
	set_trap_gate(13, (unsigned) isr13);
	set_trap_gate(14, (unsigned) isr14);
	set_trap_gate(15, (unsigned) isr15);
	set_trap_gate(16, (unsigned) isr16);
	set_trap_gate(17, (unsigned) isr17);
	set_trap_gate(18, (unsigned) isr18);
	set_trap_gate(19, (unsigned) isr19);
	set_trap_gate(20, (unsigned) isr20);
	set_trap_gate(21, (unsigned) isr21);
	set_trap_gate(22, (unsigned) isr22);
	set_trap_gate(23, (unsigned) isr23);
	set_trap_gate(24, (unsigned) isr24);
	set_trap_gate(25, (unsigned) isr25);
	set_trap_gate(26, (unsigned) isr26);
	set_trap_gate(27, (unsigned) isr27);
	set_trap_gate(28, (unsigned) isr28);
	set_trap_gate(29, (unsigned) isr29);
	set_trap_gate(30, (unsigned) isr30);
	set_trap_gate(31, (unsigned) isr31);
}

/* Sets error message to print out when exception is recieved */
unsigned char * excetion_messages[] = 
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
}

void fault_handler(){
	
	/* prints excetion message */
	puts(excetion_messages[]);
	
	/* infinite loop */
	for(;;); 	
}





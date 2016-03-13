#include "isr.h"


/* Sets error message to print out when exception is recieved */
unsigned char * exception_messages[] = 
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

void isr0()
{

	printf("%s",exception_messages[0]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr1()
{

	printf("%s",exception_messages[1]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr2()
{

	printf("%s",exception_messages[2]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr3()
{

	printf("%s",exception_messages[3]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr4()
{

	printf("%s",exception_messages[4]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr5()
{

	printf("%s",exception_messages[5]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr6()
{

	printf("%s",exception_messages[6]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr7()
{

	printf("%s",exception_messages[7]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr8()
{

	printf("%s",exception_messages[8]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr9()
{

	printf("%s",exception_messages[9]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr10()
{

	printf("%s",exception_messages[10]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr11()
{

	printf("%s",exception_messages[11]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr12()
{

	printf("%s",exception_messages[12]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr13()
{

	printf("%s",exception_messages[13]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr14()
{

	printf("%s",exception_messages[14]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr15()
{

	printf("%s",exception_messages[15]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr16()
{

	printf("%s",exception_messages[16]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr17()
{

	printf("%s",exception_messages[17]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}
void isr18()
{

	printf("%s",exception_messages[18]);

	while(1){
	//spinning to produce blue screen kinda deal
	}
}

/* Set first 32 entries in IDT to Intel's defined exceptions */
void isrs_install(){
	set_trap_gate(0,  isr0);
	set_trap_gate(1,  isr1);
	set_trap_gate(2,  isr2);
	set_trap_gate(3,  isr3);
	set_trap_gate(4,  isr4);
	set_trap_gate(5,  isr5);
	set_trap_gate(6,  isr6);
	set_trap_gate(7,  isr7);
	set_trap_gate(8,  isr8);
	set_trap_gate(9,  isr9);
	set_trap_gate(10,  isr10);
	set_trap_gate(11,  isr11);
	set_trap_gate(12,  isr12);
	set_trap_gate(13,  isr13);
	set_trap_gate(14,  isr14);
	set_trap_gate(15,  isr15);
	set_trap_gate(16,  isr16);
	set_trap_gate(17,  isr17);
	set_trap_gate(18,  isr18);
}






/*void fault_handler(){
	
	puts(excetion_messages[]);
	
	for(;;); 	
}
*/




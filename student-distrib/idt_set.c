#include "idt_set.h"
#include "x86_desc.h"


void set_int_gate(unsigned long n, unsigned long addr){
	SET_IDT_ENTRY(idt[n], addr);
	idt[n].seg_selector = KERNEL_CS;
	idt[n].reserved0 = 0;
	idt[n].reserved1 = 1;
	idt[n].reserved2 = 1;
	idt[n].reserved3 = 0;
	idt[n].size = 1;
	idt[n].dpl = 0;
	
	idt[n].present = 1;
}


void set_sys_gate(unsigned long n, unsigned long addr){
	SET_IDT_ENTRY(idt[n], addr);
	idt[n].seg_selector = KERNEL_CS;
	idt[n].reserved0 = 0;
	idt[n].reserved1 = 1;
	idt[n].reserved2 = 1;
	idt[n].reserved3 = 1;
	idt[n].size = 1;
	idt[n].dpl = 3;
	
	idt[n].present = 1;
}


void set_sys_int_gate(unsigned long n, unsigned long addr){
	SET_IDT_ENTRY(idt[n], addr);
	idt[n].seg_selector = KERNEL_CS;
	idt[n].reserved0 = 0;
	idt[n].reserved1 = 1;
	idt[n].reserved2 = 1;
	idt[n].reserved3 = 0;
	idt[n].size = 1;
	idt[n].dpl = 3;
	
	idt[n].present = 1;}


void set_trap_gate(unsigned long n, unsigned long addr){
	SET_IDT_ENTRY(idt[n], addr);
	idt[n].seg_selector = KERNEL_CS;
	idt[n].reserved0 = 0;
	idt[n].reserved1 = 1;
	idt[n].reserved2 = 1;
	idt[n].reserved3 = 1;
	idt[n].size = 1;
	idt[n].dpl = 0;
	
	idt[n].present = 1;
}

void set_task_gate(unsigned long n, unsigned long gdt){
	idt[n].offset_15_00 = 0;
	idt[n].seg_selector = gdt & 0xFFFF;
	idt[n].reserved0 = 0;
	idt[n].reserved1 = 0;
	idt[n].reserved2 = 1;
	idt[n].reserved3 = 1;
	idt[n].dpl = 3;
	idt[n].offset_31_16 = 0;
	
	idt[n].present = 1;
}
	

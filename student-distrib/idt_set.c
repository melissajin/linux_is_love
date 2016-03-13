#include "idt_set.h"


void set_int_gate(unsigned long n, unsigned long addr){
	idt_desc_t newidt;
	newidt.offset_15_00 = 0xFF & addr;
	newidt.seg_selector = KERNEL_CS;
	newidt.reserved2 = 1;
	newidt.reserved1 = 1;
	newidt.size = 1;
	newidt.dpl = 0;
	newidt.offset_31_16 = (0xFF00 & addr)>>16;
	
	newidt.present = 1;
	idt[n] =  newidt;
}


void set_sys_gate(unsigned long n, unsigned long addr){
	idt_desc_t newidt;
	newidt.offset_15_00 = 0xFF & addr;
	newidt.seg_selector = KERNEL_CS;
	newidt.reserved3 =1;
	newidt.reserved2 =1;
	newidt.reserved1 = 1;
	newidt.size = 1;
	newidt.dpl = 3;
	newidt.offset_31_16 = (0xFF00 & addr)>>16;
	
	newidt.present = 1;
	idt[n] =  newidt;
}


void set_sys_int_gate(unsigned long n, unsigned long addr){
	idt_desc_t newidt;
	newidt.offset_15_00 = 0xFF & addr;
	newidt.seg_selector = KERNEL_CS;
	newidt.reserved2 =1;
	newidt.reserved1 = 1;
	newidt.size = 1;
	newidt.dpl = 3;
	newidt.offset_31_16 = (0xFF00 & addr)>>16;
	
	newidt.present = 1;
	idt[n] + =  newidt;
}


void set_trap_gate(unsigned long n, unsigned long addr){
	idt_desc_t newidt;
	newidt.offset_15_00 = 0xFF & addr;
	newidt.seg_selector = KERNEL_CS;
	newidt.reserved3 =1;
	newidt.reserved2 =1;
	newidt.reserved1 = 1;
	newidt.size = 1;
	newidt.dpl = 0;
	newidt.offset_31_16 = (0xFF00 & addr)>>16;
	
	
	newidt.present = 1;
	idt[n] =  newidt;
}

void set_task_gate(unsigned long n, unsigned long gdt){
	idt_desc_t newidt;
	newidt.offset_15_00 = 0;
	newidt.seg_selector = gdt & 0xFFFF;
	newidt.reserved3 = 1;
	newidt.reserved1 = 1;
	newidt.dpl = 3;
	newidt.offset_31_16 = 0;
	
	newidt.present = 1;
	idt[n] = newidt;
}
	

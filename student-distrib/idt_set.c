#include "idt_set.h"
#include "x86_desc.h"

/* set_int_gate
 * DESC:sets an interrupt gate in the IDT, cannot be accessed by user
 * INPUTS: n - the index of the IDT that we are putting this ISR in
 * 	   addr - the address of the corresponding handler
 * OUTPUTS: none
 * SIDE EFFECTS: IDT[n] will be written to
 */
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


/* set_sys_gate
 * DESC:sets an system gate in the IDT, can be accessed by user
 * INPUTS: n - the index of the IDT that we are putting this ISR in
 * 	   addr - the address of the corresponding handler
 * OUTPUTS: none
 * SIDE EFFECTS: IDT[n] will be written to
 */
void set_sys_gate(unsigned long n, unsigned long addr){
	SET_IDT_ENTRY(idt[n], addr);
	idt[n].seg_selector = KERNEL_CS;
	idt[n].reserved0 = 0;
	idt[n].reserved1 = 1;
	idt[n].reserved2 = 1;
	idt[n].reserved3 = 1;
	idt[n].size = 1;
	idt[n].dpl = 3;	//dpl = 3 makes this table member not accessable by user
	
	idt[n].present = 1;
}


/* set_sys_int_gate
 * DESC:sets an int gate in the IDT, can be accessed by user
 * INPUTS: n - the index of the IDT that we are putting this ISR in
 * 	   addr - the address of the corresponding handler
 * OUTPUTS: none
 * SIDE EFFECTS: IDT[n] will be written to
 */
void set_sys_int_gate(unsigned long n, unsigned long addr){
	SET_IDT_ENTRY(idt[n], addr);
	idt[n].seg_selector = KERNEL_CS;
	idt[n].reserved0 = 0;
	idt[n].reserved1 = 1;
	idt[n].reserved2 = 1;
	idt[n].reserved3 = 1;
	idt[n].size = 1;
	idt[n].dpl = 3;//dpl = 3 makes this table memebr not accessable by user
	
	idt[n].present = 1;}


/* set_trap_gate
 * DESC:sets an trap gate in the IDT, cannot be accessed by user
 * INPUTS: n - the index of the IDT that we are putting this ISR in
 * 	   addr - the address of the corresponding handler
 * OUTPUTS: none
 * SIDE EFFECTS: IDT[n] will be written to
 */
void set_trap_gate(unsigned long n, unsigned long addr){
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

/* set_task_gate
 * DESC:sets a task gate in the IDT, cannot be accessed by user
 * INPUTS: n - the index of the IDT that we are putting this ISR in
 * 	   addr - the address of the corresponding handler
 * OUTPUTS: none
 * SIDE EFFECTS: IDT[n] will be written to
 * NOTES: double fault handler uses this 
 */
void set_task_gate(unsigned long n, unsigned long gdt){
	SET_IDT_ENTRY(idt[n], 0); //the task gate has its offsets set to 0
	idt[n].seg_selector = gdt & 0xFFFF; // we use FFFF here because we only want bottom 16 bits
	idt[n].reserved0 = 0;
	idt[n].reserved1 = 0;
	idt[n].reserved2 = 1;
	idt[n].reserved3 = 1;
	idt[n].dpl = 3; //dpl = 3 makes this table member not accessable by user
	
	idt[n].present = 1;
}
	

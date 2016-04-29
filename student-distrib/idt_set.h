#ifndef _IDT_SET_FUNC_
#define _IDT_SET_FUNC_

/*sets an interrupt gate in the IDT*/
extern void set_int_gate(unsigned long n, unsigned long addr);

/*sets a system trap gate in the IDT*/
extern void set_sys_gate(unsigned long n, unsigned long addr);

/*sets an system interrupt gate in the IDT*/
extern void set_sys_int_gate(unsigned long n, unsigned long addr);

/*sets a trap gate in the IDT*/
extern void set_trap_gate(unsigned long n, unsigned long addr);

/*sets a task gate in the IDT*/
extern void set_task_gate(unsigned long n, unsigned long gdt);

#endif //_IDT_SET_FUNC_

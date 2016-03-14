#ifndef _IDT_SET_FUNC_
#define _IDT_SET_FUNC_

extern void set_int_gate(unsigned long n, unsigned long addr);

extern void set_sys_gate(unsigned long n, unsigned long addr);

extern void set_sys_int_gate(unsigned long n, unsigned long addr);

extern void set_trap_gate(unsigned long n, unsigned long addr);

extern void set_task_gate(unsigned long n, unsigned long gdt);

#endif //_IDT_SET_FUNC_

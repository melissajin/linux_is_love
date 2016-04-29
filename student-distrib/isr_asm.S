#define USER_CS 0x0023
#define USER_DS 0x002B

.text

.global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7 //provides all the assembly linkages to be called by c functions
.global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
.global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
.global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
.global irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
.global irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15
.global sys_call
.global kybd_isr, rtc_isr, pit_isr

.extern fault_handler		#assembly linkage for all our exceptions
.extern sys_call_handler	#assembly linkage for sys calls
.extern keyboard_handler_main	#assembly linkage for keyboard
.extern rtc_handler_main	#assembly linkage for rtc
.extern halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn #system calls

.extern irq_table

sys_table:
	.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

/*
 * void handle
 *   Description: Generic stub for handling faults
 *   Inputs: none
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Halts the process that faulted.
 */
handle:
	pusha
	push	%ds		#here we store the data segment registers
	push	%es
	push	%fs
	push	%gs

	movw	$0x10, %ax	#0x10 points to the new data selector
	movw	%ax, %ds	#we fill all the segment registers with the new data selector
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	movl	%esp, %eax
	pushl	%eax		#pushing the isr number for the fault handler

	call	fault_handler

	popl	%eax		#restoring all our registers
	pop		%gs
	pop		%fs
	pop		%es
	pop		%ds
	popa

	add		$8, %esp #clearing stack
	iret

/*
 * void handle_syscall
 *   Description: Generic stub for handling system calls.
 *   Inputs: eax - syscall number (1-10)
 *           ebx, ecx, edx - args
 *   Outputs: Dependent on system call
 *   Return Value: Dependent on system call
 */
handle_syscall:		#similar functionality as above except for system calls
	/* test validity of syscall number */
	cmpl	$1, %eax
	jb		handle_syscall_error
	cmpl	$10, %eax
	ja		handle_syscall_error

	/* save regs */
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%esi
	pushl	%edi

	/* push args */
	pushl	%edx
	pushl	%ecx
	pushl	%ebx

	decl	%eax	/* correct numbering 1-10 to 0-9 */
	call	*sys_table(,%eax, 4)

	/* pop args */
	popl	%ebx
	popl	%ecx
	popl	%edx
	
	/* restore regs */
	popl	%edi
	popl	%esi
	popl	%edx
	popl	%ecx
	popl	%ebx

handle_syscall_iret:
	iret
handle_syscall_error:
	movl	$-1, %eax
	jmp		handle_syscall_iret

/* stub for ISR 0 */
isr0:
	cli
	pushl $0  /* error code repeats for all isr*/
	pushl $0  /* isr number repeats for all isr*/
	jmp handle

/* stub for ISR 1 */
isr1:
	cli
	pushl $0
	pushl $1
	jmp handle

/* stub for ISR 2 */
isr2:
	cli
	pushl $0
	pushl $2
	jmp handle

/* stub for ISR 3 */
isr3:
	cli
	pushl $0
	pushl $3
	jmp handle

/* stub for ISR 4 */
isr4:
	cli
	pushl $0
	pushl $4
	jmp handle

/* stub for ISR 5 */
isr5:
	cli
	pushl $0
	pushl $5
	jmp handle

/* stub for ISR 6 */
isr6:
	cli
	pushl $0
	pushl $6
	jmp handle

/* stub for ISR 7 */
isr7:
	cli
	pushl $0
	pushl $7
	jmp handle

/* stub for ISR 8 */
isr8:
	cli
	pushl $8  /* no error code because processor pushes one */
	jmp handle

/* stub for ISR 9 */
isr9:
	cli
	pushl $0
	pushl $9
	jmp handle

/* stub for ISR 10 */
isr10:
	cli
	pushl $10
	jmp handle

/* stub for ISR 11 */
isr11:
	cli
	pushl $11
	jmp handle

/* stub for ISR 12 */
isr12:
	cli
	pushl $12
	jmp handle

/* stub for ISR 13 */
isr13:
	cli
	pushl $13
	jmp handle

/* stub for ISR 14 */
isr14:
	cli
	pushl $14
	jmp handle

/* stub for ISR 15 */
isr15:
	cli
	pushl $0
	pushl $15
	jmp handle

/* stub for ISR 16 */
isr16:
	cli
	pushl $0
	pushl $16
	jmp handle

/* stub for ISR 17 */
isr17:
	cli
	pushl $17
	jmp handle

/* stub for ISR 18 */
isr18:
	cli
	pushl $0
	pushl $18
	jmp handle

/* stub for ISR 19 */
isr19:
	cli
	pushl $0
	pushl $19
	jmp handle

/* stub for ISR 20 */
isr20:
	cli
	pushl $0
	pushl $20
	jmp handle

/* stub for ISR 21 */
isr21:
	cli
	pushl $0
	pushl $21
	jmp handle

/* stub for ISR 22 */
isr22:
	cli
	pushl $0
	pushl $22
	jmp handle

/* stub for ISR 23 */
isr23:
	cli
	pushl $0
	pushl $23
	jmp handle

/* stub for ISR 24 */
isr24:
	cli
	pushl $0
	pushl $24
	jmp handle

/* stub for ISR 25 */
isr25:
	cli
	pushl $0
	pushl $25
	jmp handle

/* stub for ISR 26 */
isr26:
	cli
	pushl $0
	pushl $26
	jmp handle

/* stub for ISR 27 */
isr27:
	cli
	pushl $0
	pushl $27
	jmp handle

/* stub for ISR 28 */
isr28:
	cli
	pushl $0
	pushl $28
	jmp handle

/* stub for ISR 29 */
isr29:
	cli
	pushl $0
	pushl $29
	jmp handle

/* stub for ISR 30 */
isr30:
	cli
	pushl $0
	pushl $30
	jmp handle

/* stub for ISR 31 */
isr31:
	cli
	pushl $0
	pushl $31
	jmp handle

irq0:
	pusha
	call *irq_table+4*0
	popa
	iret

irq1:
	pusha
	call *irq_table+4*1
	popa
	iret

irq2:
	pusha
	call *irq_table+4*2
	popa
	iret

irq3:
	pusha
	call *irq_table+4*3
	popa
	iret

irq4:
	pusha
	call *irq_table+4*4
	popa
	iret

irq5:
	pusha
	call *irq_table+4*5
	popa
	iret

irq6:
	pusha
	call *irq_table+4*6
	popa
	iret

irq7:
	pusha
	call *irq_table+4*7
	popa
	iret

irq8:
	pusha
	call *irq_table+4*8
	popa
	iret

irq9:
	pusha
	call *irq_table+4*9
	popa
	iret

irq10:
	pusha
	call *irq_table+4*10
	popa
	iret

irq11:
	pusha
	call *irq_table+4*11
	popa
	iret

irq12:
	pusha
	call *irq_table+4*12
	popa
	iret

irq13:
	pusha
	call *irq_table+4*13
	popa
	iret

irq14:
	pusha
	call *irq_table+4*14
	popa
	iret

irq15:
	pusha
	call *irq_table+4*15
	popa
	iret

/* stub for system calls */
sys_call:		#preparing argument for handle_syscall
	jmp handle_syscall

.end

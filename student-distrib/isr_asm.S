#define USER_CS 0x0023
#define USER_DS 0x002B

.text

.global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7 //provides all the assembly linkages to be called by c functions
.global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
.global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
.global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
.global sys_call
.global kybd_isr, rtc_isr

.extern fault_handler		#assembly linkage for all our exceptions
.extern sys_call_handler	#assembly linkage for sys calls
.extern keyboard_handler_main	#assembly linkage for keyboard
.extern rtc_handler_main	#assembly linkage for rtc
.extern halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn #system calls


sys_table:
	.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn


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
	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popa

	add		$8, %esp #clearing stack
	iret

handle_syscall:		#similar functionality as above except for system calls
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
	call *sys_table(,%eax, 4)

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

	iret

isr0:
	cli
	pushl $0  /* error code repeats for all isr*/
	pushl $0  /* isr number repeats for all isr*/
	jmp handle

isr1:
	cli
	pushl $0
	pushl $1
	jmp handle

isr2:
	cli
	pushl $0
	pushl $2
	jmp handle

isr3:
	cli
	pushl $0
	pushl $3
	jmp handle

isr4:
	cli
	pushl $0
	pushl $4
	jmp handle

isr5:
	cli
	pushl $0
	pushl $5
	jmp handle

isr6:
	cli
	pushl $0
	pushl $6
	jmp handle

isr7:
	cli
	pushl $0
	pushl $7
	jmp handle

isr8:
	cli
	pushl $8  /* no error code because processor pushes one */
	jmp handle

isr9:
	cli
	pushl $0
	pushl $9
	jmp handle

isr10:
	cli
	pushl $10
	jmp handle

isr11:
	cli
	pushl $11
	jmp handle

isr12:
	cli
	pushl $12
	jmp handle

isr13:
	cli
	pushl $13
	jmp handle

isr14:
	cli
	pushl $14
	jmp handle

isr15:
	cli
	pushl $0
	pushl $15
	jmp handle

isr16:
	cli
	pushl $0
	pushl $16
	jmp handle

isr17:
	cli
	pushl $17
	jmp handle

isr18:
	cli
	pushl $0
	pushl $18
	jmp handle

isr19:
	cli
	pushl $0
	pushl $19
	jmp handle

isr20:
	cli
	pushl $0
	pushl $20
	jmp handle

isr21:
	cli
	pushl $0
	pushl $21
	jmp handle

isr22:
	cli
	pushl $0
	pushl $22
	jmp handle

isr23:
	cli
	pushl $0
	pushl $23
	jmp handle

isr24:
	cli
	pushl $0
	pushl $24
	jmp handle

isr25:
	cli
	pushl $0
	pushl $25
	jmp handle

isr26:
	cli
	pushl $0
	pushl $26
	jmp handle

isr27:
	cli
	pushl $0
	pushl $27
	jmp handle

isr28:
	cli
	pushl $0
	pushl $28
	jmp handle

isr29:
	cli
	pushl $0
	pushl $29
	jmp handle

isr30:
	cli
	pushl $0
	pushl $30
	jmp handle

isr31:
	cli
	pushl $0
	pushl $31
	jmp handle

kybd_isr:		#A handler/assembly linkage for the keyboard just saves state of machine
	cli
	pusha		
	call keyboard_handler_main
	popa
	iret

rtc_isr:		#A handler/assembly linkage for the rtc just saves state of machine
	cli
	pusha
	call rtc_handler_main
	popa
	iret

sys_call:		#preparing argument for handle_syscall
	cli
	jmp handle_syscall

.end

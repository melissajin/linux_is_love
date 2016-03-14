.text

.global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
.global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
.global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
.global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31

.extern fault_handler

handle:
	pusha
	push	%ds
	push	%es
	push	%fs
	push	%gs

	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	movl	%esp, %eax
	pushl	%eax

	movl	fault_handler, %eax
	call	%eax

	popl	%eax
	pop		%gs
	pop		%fs
	pop		%es
	pop		%ds
	popa

	add		$8, %esp
	iret

isr0:
	cli
	pushb $0  /* error code */
	pushb $0  /* isr number */
	jmp handle

isr1:
	cli
	pushb $0
	pushb $1
	jmp handle

isr2:
	cli
	pushb $0
	pushb $2
	jmp handle

isr3:
	cli
	pushb $0
	pushb $3
	jmp handle

isr4:
	cli
	pushb $0
	pushb $4
	jmp handle

isr5:
	cli
	pushb $0
	pushb $5
	jmp handle

isr6:
	cli
	pushb $0
	pushb $6
	jmp handle

isr7:
	cli
	pushb $0
	pushb $7
	jmp handle

isr8:
	cli
	pushb $8  /* no error code because processor pushes one */
	jmp handle

isr9:
	cli
	pushb $0
	pushb $9
	jmp handle

isr10:
	cli
	pushb $10
	jmp handle

isr11:
	cli
	pushb $11
	jmp handle

isr12:
	cli
	pushb $12
	jmp handle

isr13:
	cli
	pushb $13
	jmp handle

isr14:
	cli
	pushb $14
	jmp handle

isr15:
	cli
	pushb $0
	pushb $15
	jmp handle

isr16:
	cli
	pushb $0
	pushb $16
	jmp handle

isr17:
	cli
	pushb $17
	jmp handle

isr18:
	cli
	pushb $0
	pushb $18
	jmp handle

isr19:
	cli
	pushb $0
	pushb $19
	jmp handle

isr20:
	cli
	pushb $0
	pushb $20
	jmp handle

isr21:
	cli
	pushb $0
	pushb $21
	jmp handle

isr22:
	cli
	pushb $0
	pushb $22
	jmp handle

isr23:
	cli
	pushb $0
	pushb $23
	jmp handle

isr24:
	cli
	pushb $0
	pushb $24
	jmp handle

isr25:
	cli
	pushb $0
	pushb $25
	jmp handle

isr26:
	cli
	pushb $0
	pushb $26
	jmp handle

isr27:
	cli
	pushb $0
	pushb $27
	jmp handle

isr28:
	cli
	pushb $0
	pushb $28
	jmp handle

isr29:
	cli
	pushb $0
	pushb $29
	jmp handle

isr30:
	cli
	pushb $0
	pushb $30
	jmp handle

isr31:
	cli
	pushb $0
	pushb $31
	jmp handle

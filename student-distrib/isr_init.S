# Initialize Interrupt Service Routines (ISRs)

.global isr0
.global isr1
.global isr3
.global isr4
.global isr5
.global isr6
.global isr7
.global isr8
.global isr9
.global isr10
.global isr11
.global isr12
.global isr13
.global isr14
.global isr15
.global isr16
.global isr17
.global isr18
.global isr19
.global isr20
.global isr21
.global isr22
.global isr23
.global isr24
.global isr25
.global isr26
.global isr27
.global isr28
.global isr29
.global isr30
.global isr31

# C function called from isr_common_stub
.extern fault_handler

isr0: 			#Divide by zero exception
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 0
	jmp isr_common_stub

isr1:			#Debug exception
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 1
	jmp isr_common_stub

isr2:			#Non maskable interrupt exception
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 2
	jmp isr_common_stub

isr3:			#Breakpoint exception
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 3
	jmp isr_common_stub

isr4:			#Into detected overflow error
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 4
	jmp isr_common_stub

isr5:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 5
	jmp isr_common_stub

isr6:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 6
	jmp isr_common_stub

isr7:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 7
	jmp isr_common_stub

isr8:
	cli
	pushb 8		#don't push dummy error code
	jmp isr_common_stub

isr9:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 9
	jmp isr_common_stub

isr10:
	cli
	pushb 10		#don't push dummy error code
	jmp isr_common_stub

isr11:
	cli
	pushb 11		#don't push dummy error code
	jmp isr_common_stub

isr12:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 12
	jmp isr_common_stub

isr13:
	cli
	pushb 13		#don't push dummy error code
	jmp isr_common_stub

isr14:
	cli
	pushb 14		#don't push dummy error code
	jmp isr_common_stub

isr15:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 15
	jmp isr_common_stub

isr16:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 16
	jmp isr_common_stub

isr17:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 17
	jmp isr_common_stub

isr18:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 18
	jmp isr_common_stub

isr19:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 19
	jmp isr_common_stub

isr20:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 20
	jmp isr_common_stub

isr21:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 21
	jmp isr_common_stub

isr22:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 22
	jmp isr_common_stub

isr23:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 23
	jmp isr_common_stub

isr24:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 24
	jmp isr_common_stub

isr25:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 25
	jmp isr_common_stub

isr26:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 26
	jmp isr_common_stub

isr27:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 27
	jmp isr_common_stub

isr28:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 28
	jmp isr_common_stub

isr29:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 29
	jmp isr_common_stub

isr30:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 30
	jmp isr_common_stub

isr31:
	cli 
	pushb 0		#push dummy error code to keep uniform stack frame
	pushb 31
	jmp isr_common_stub

isr_common_stub:
	pusha		#push all general purpose registers
	push %ds 
	push %es
	push %fs
	push %gs 
	movw $KERNEL_DS, %cx
	movw %cx, %ss
	movw %cx, %ds
	movw %cx, %es
	movw %cx, %fs
	movw %cx, %gs
	pushl %esp
	movl fault_handler, %eax
	call %eax
	pop %eax
	pop %gs
	pop %fs
	pop %es
	pop %ds
	popa
	add %esp, 8 #cleans up pushed error code and pushed ISR number
	iret

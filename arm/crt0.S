
.text
.align

.extern main
.extern sh4_fiq_hdl
.extern bus_fiq_hdl
.extern timer_fiq_hdl

#include "../aica_registers.h"

.globl reset
reset:
	# Exception vectors
	b start
	b undef
	b softint
	b pref_abort
	b data_abort
	b rsrvd
	b irq_hdl

	# FIQ handling: load the FIQ code, and jump to
	# the corresponding function.
	ldr r8, reg_arm_fiq_code
	ldr r8, [r8]
	and r8, r8, #7

	mov r9,#SH4_INTERRUPT_INT_CODE
	cmp r8,r9
	b sh4_fiq_hdl

	mov r9,#TIMER_INTERRUPT_INT_CODE
	cmp r8,r9
	beq timer_fiq_hdl

	mov r9,#BUS_REQUEST_INT_CODE
	cmp r8,r9
	beq bus_fiq_hdl

undef:
softint:
	movs pc,lr

data_abort:
	subs pc,lr,#8

pref_abort:
rsrvd:
irq_hdl:
	subs pc,lr,#4

start:
	# Setup a basic stack
	ldr sp,=__stack

	# Switch to FIQ mode
	mrs r0,CPSR
	bic r0,r0,#0x1f
	orr r0,r0,#0x12
	msr CPSR_c,r0
	nop
	nop
	nop
	nop

	# Setup the FIQ stack
	ldr sp,=__fiq_stack

	# Switch back to supervisor mode
	orr r0,r0,#0x13
	msr CPSR_c,r0
	nop
	nop
	nop
	nop

	ldr r0,=__bss_end
	ldr r1,=__bss_start
	mov r2,#0

	clear_bss_loop:
		str r2,[r0,#-4]!
		cmp r0,r1
		bhi clear_bss_loop

	# Call the C "main" function
	mov r0,#0
	bl main

program_end:
	# Loop infinitely when we get here
	b program_end

reg_arm_fiq_code:
	.word REG_ARM_FIQ_CODE

.end


# KallistiOS ##version##
#
#  crt0.s
#  (c)2000-2002 Dan Potter
#  (c)2011 Paul Cercueil
#
#  Startup for ARM program
#  Adapted from Marcus' AICA example among a few other sources =)


.text

.align

.globl  fiq_enable
.globl  fiq_disable
.globl  irq_enable
.globl  irq_disable
.globl  inside_interrupt
.extern aica_sh4_fiq_hdl
.extern main

# Meaningless but makes the linker shut up
.globl	reset
reset:

# Exception vectors
    b   start
    b   undef
    b   softint
    b   pref_abort
    b   data_abort
    b   rsrvd
    b   irq


# FIQ code adapted from the Marcus AICA example
fiq:

	mov r8,#1
	str r8,in_fiq

	# Grab interrupt type (store as parameter)
	ldr	r8,intreq
	ldr	r9,[r8]
	and	r9,r9,#7

	# Timer interupt?
	cmp	r9,#2
	beq	fiq_timer

	# Bus request?
	cmp	r9,#5
	beq	fiq_busreq

    # SH-4 interrupt?
    cmp r9,#6

	# Dunno -- ack and skip
	bne fiq_done


fiq_sh4:

    # Aknowledgement
    ldr r9,scpu_interrupt
    mov r8,#0x20
    str r8,[r9,#8]
    str r8,[r9,#8]
    str r8,[r9,#8]
    str r8,[r9,#8]

    bl aica_sh4_fiq_hdl
    b fiq_done


fiq_busreq:
	# Type 5 is bus request. Wait until the INTBusRequest register
	# goes back from 0x100.
	ldr	r8,busreq_control
fiq_busreq_loop:
	# This could probably be done more efficiently, but I'm
	# no ARM assembly expert...
	ldr	r9,[r8]
	and	r9,r9,#0x100
	cmp	r9,#0
	bne	fiq_busreq_loop

	b	fiq_done


fiq_timer:
	# Type 2 is timer interrupt. Increment timer variable.
	# Update the next line to AICA_MEM_CLOCK if you change AICA_CMD_IFACE
	mov	r8,#0x21000
	ldr	r9,[r8]
	add	r9,r9,#1
	str	r9,[r8]
	
	# Request a new timer interrupt. We'll calculate the number
	# put in here based on the "jps" (jiffies per second). 
	ldr	r8, timer_control
	mov	r9,#256-(44100/4410)
	str	r9,[r8,#0x10]
	mov	r9,#0x40
	str	r9,[r8,#0x24]
	
	# Return from interrupt

fiq_done:
	# Clear interrupt
	ldr	r8,intclr
	mov	r9,#1
	str	r9,[r8]
	str	r9,[r8]
	str	r9,[r8]
	str	r9,[r8]

	mov r8,#0
	str r8,in_fiq

	subs pc,r14,#4

irq_enable:
	mrs r0,CPSR
	bic r0,r0,#0x80
	msr CPSR_all,r0
	mov pc,lr

irq_disable:
	mrs r0,CPSR
	orr r0,r0,#0x80
	msr CPSR_all,r0
	mov pc,lr

fiq_enable:
	mrs r0,CPSR
	bic r0,r0,#0x40
	msr CPSR_all,r0
	mov pc,lr


fiq_disable:
	mrs r0,CPSR
	orr r0,r0,#0x40
	msr CPSR_all,r0
	mov pc,lr


inside_interrupt:
    ldr r0,in_fiq
    mov pc,lr


start:
	# Setup a basic stack
    ldr sp,=__stack

	ldr r0,=__bss_end__
	ldr r1,=__bss_start
	mov r2,#0

	clear_bss_loop:
		sub r0,r0,#4
		str r2,[r0]
		cmp r0,r1
		bls clear_bss_loop

    # Set the SH-4 interrupt code to 6
	ldr r10,scpu_interrupt
    mov r11,#0x20
    str r11,[r10,#16]
    str r11,[r10,#20]

    # Enable interrupts from SH-4
    str r11,[r10]

	# Call the C "main" function
	mov r0,#0
	bl main

program_end:
	# Loop infinitely when we get here
    b program_end
    

# Handlers we don't bother to catch
undef:
softint:
    mov	pc,r14

data_abort:
pref_abort:
irq:
rsrvd:
    subs pc,r14,#4

.align

in_fiq:
	.long 0x0
intreq:
	.long 0x00802d00
intclr:
	.long 0x00802d04
timer_control:
	.long 0x00802880
busreq_control:
	.long 0x00802808
scpu_interrupt:
    .long 0x0080289c

.end


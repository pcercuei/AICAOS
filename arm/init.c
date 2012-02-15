
#include <stdint.h>

#include "init.h"
#include "../aica_registers.h"

/* When F bit (resp. I bit) is set, FIQ (resp. IRQ) is disabled. */
#define F_BIT 0x40
#define I_BIT 0x80

void int_restore(uint32_t context)
{
	asm volatile("msr CPSR_all,%0" : : "r"(context));
}

uint32_t int_disable(void)
{
	register uint32_t cpsr;
	asm volatile("mrs %0,CPSR" : "=r"(cpsr) :);

	int_restore(cpsr | I_BIT | F_BIT);
	return cpsr;
}

uint32_t int_enable(void)
{
	register uint32_t cpsr;
	asm volatile("mrs %0,CPSR" : "=r"(cpsr) :);

	int_restore(cpsr & ~(I_BIT | F_BIT));
	return cpsr;
}

void aica_sh4_fiq_hdl(void)
{
}

void __attribute__((interrupt ("FIQ"))) fiq_hdl(void)
{
	uint32_t i;

	switch (*(unsigned int *) REG_ARM_FIQ_CODE & 0x7) {
		case TIMER_INTERRUPT_INT_CODE:
			break;
		case BUS_REQUEST_INT_CODE:
			while(0x100 & *(volatile unsigned int *) REG_BUS_REQUEST);
			break;
		case SH4_INTERRUPT_INT_CODE:
			aica_sh4_fiq_hdl();
			break;
	}

	/* Acknowledge */
	for (i=0; i<4; i++)
		*(unsigned int *) REG_ARM_FIQ_ACK = 1;
}


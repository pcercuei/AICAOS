
#include <stdint.h>

#include "../aica_registers.h"

/* When F bit (resp. I bit) is set, FIQ (resp. IRQ) is disabled. */
#define F_BIT 0x40
#define I_BIT 0x80

void int_toggle(int enable)
{
	register uint32_t tmp;
	asm volatile("mrs %0,CPSR" : "=r"(tmp) :);
	if (enable)
		tmp &= ~(I_BIT | F_BIT);
	else
		tmp |= (I_BIT | F_BIT);
	asm volatile("msr CPSR_all,%0" : : "r"(tmp));
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


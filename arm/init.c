
#include <stdint.h>

/* When F bit (resp. I bit) is set, FIQ (resp. IRQ) is disabled. */
#define F_BIT 0x40
#define I_BIT 0x80

/* AICA registers */
#define INTBusRequest  0x00802808
#define INTTimerStart  0x00802880
#define INTCtrlSCPU    0x0080289c
#define INTRequest     0x00802d00
#define INTClear       0x00802d04

/* Interruption codes */
#define TIMER_INTERRUPT_INT_CODE 2
#define BUS_REQUEST_INT_CODE 5
#define SH4_INTERRUPT_INT_CODE 6

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
	uint32_t code = 0x7 & *(uint32_t *) INTRequest;
	volatile uint32_t *intclr = (uint32_t *) INTClear;
	uint32_t i;

	switch (code) {
		case TIMER_INTERRUPT_INT_CODE:
			break;
		case BUS_REQUEST_INT_CODE:
			while(0x100 & *(volatile uint32_t *) INTBusRequest);
			break;
		case SH4_INTERRUPT_INT_CODE:
			aica_sh4_fiq_hdl();
			break;
	}

	/* Acknowledge */
	for (i=0; i<4; i++)
		*intclr = 1;
}


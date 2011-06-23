
typedef int size_t;

#include "../aica_common.h"
#include "crt0.h"

static struct io_channel *io_addr;

static SHARED(get_arm_func_id)
{
	return aica_find_id((unsigned int *)out, (char *)in);
}

int aica_init(char *fn)
{
	/* Discard GCC warnings about unused parameter */
	(void)fn;

	aica_clear_handler_table();

	/* That function will be used by the remote processor to get IDs
	 * from the names of the functions to call. */
	AICA_SHARE(get_arm_func_id, FUNCNAME_MAX_LENGTH, sizeof(unsigned int));

	fiq_enable();
	aica_interrupt_init();
	return 0;
}

void aica_exit(void)
{
	aica_clear_handler_table();
}


int __aica_call(unsigned int id, void *in, void *out, unsigned short prio)
{
	/* Protect from context changes. */
	fiq_disable();

	while(*(volatile unsigned char *) &io_addr[ARM_TO_SH].cparams.sync);
	io_addr[ARM_TO_SH].cparams.id = id;
	io_addr[ARM_TO_SH].cparams.prio = prio;
	io_addr[ARM_TO_SH].cparams.in = in;
	io_addr[ARM_TO_SH].cparams.out = out;

	aica_interrupt();
	fiq_enable();
	return 0;
}


void aica_interrupt_init(void)
{
	*(unsigned int *)0x008028b4 = 0x20;
}

void aica_interrupt(void)
{
	*(unsigned int *)0x008028b8 = 0x20;
}


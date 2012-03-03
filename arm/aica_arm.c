
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aica_registers.h"
#include "../aica_common.h"
#include "interrupt.h"
#include "task.h"

extern struct io_channel *__io_init;
static struct io_channel *io_addr;

static AICA_SHARED(get_arm_func_id)
{
	return aica_find_id((unsigned int *)out, (char *)in);
}

static void __main(void)
{
	extern int main(int, char**) __attribute__((weak));

	if (main) {
		int err = main(0, 0);
		printf("ARM main terminated with status %i\n", err);
	}

	while(1)
		task_reschedule();
}

static struct task * task_create_main(void)
{
	struct context cxt = {
		.pc = (aica_funcp_t) __main,
		.cpsr = 0x13, /* supervisor */
	};

	return task_create(&cxt);
}

int aica_init(char *fn)
{
	struct task *main_task;

	/* Discard GCC warnings about unused parameter */
	(void)fn;

	io_addr = (struct io_channel *) calloc(2, sizeof(struct io_channel));

	aica_clear_handler_table();

	/* That function will be used by the remote processor to get IDs
	 * from the names of the functions to call. */
	AICA_SHARE(get_arm_func_id, FUNCNAME_MAX_LENGTH, sizeof(unsigned int));

	aica_interrupt_init();
	__io_init = io_addr;

	/* We will continue when the SH-4 will decide so. */
	while (*(volatile int *) &__io_init != 0);

	/* Create the main thread, that should start right after aica_init(). */
	main_task = task_create_main();
	if (!main_task)
		printf("Unable to create main task.\n");

	task_add_to_runnable(main_task, PRIORITY_MAX);
	task_select(main_task);

	/* Never reached */
	return -1;
}

void aica_exit(void)
{
	aica_clear_handler_table();
	free(io_addr);
}


int __aica_call(unsigned int id, void *in, void *out, unsigned short prio)
{
	uint32_t int_context;

	/* Wait here if a previous call is pending. */
	while((*(volatile unsigned char *) &io_addr[ARM_TO_SH].cparams.sync)
				|| (*(volatile unsigned int *) &io_addr[ARM_TO_SH].fparams[id].call_status != FUNCTION_CALL_AVAIL))
	{
		/* TODO: yield the thread (when there'll be one... */
	}

	/* Protect from context changes. */
	int_context = int_disable();

	io_addr[ARM_TO_SH].cparams.id = id;
	io_addr[ARM_TO_SH].cparams.prio = prio;
	io_addr[ARM_TO_SH].cparams.in = in;
	io_addr[ARM_TO_SH].cparams.out = out;
	io_addr[ARM_TO_SH].cparams.sync = 255;
	io_addr[ARM_TO_SH].fparams[id].call_status = FUNCTION_CALL_PENDING;

	aica_interrupt();
	int_restore(int_context);

	/* We will wait until the call completes. */
	while( *(volatile unsigned int *) &io_addr[ARM_TO_SH].fparams[id].call_status != FUNCTION_CALL_AVAIL) {
		/* TODO: yield the thread */
	}

	return 0;
}


void aica_interrupt_init(void)
{
	/* Set the FIQ code */
	*(unsigned int *) REG_ARM_FIQ_BIT_2  = (SH4_INTERRUPT_INT_CODE & 4) ? MAGIC_CODE : 0;
	*(unsigned int *) REG_ARM_FIQ_BIT_1  = (SH4_INTERRUPT_INT_CODE & 2) ? MAGIC_CODE : 0;
	*(unsigned int *) REG_ARM_FIQ_BIT_0  = (SH4_INTERRUPT_INT_CODE & 1) ? MAGIC_CODE : 0;

	/* Allow the SH4 to raise interrupts on the ARM */
	*(unsigned int *) REG_ARM_INT_ENABLE = MAGIC_CODE;

	/* Allow the ARM to raise interrupts on the SH4 */
	*(unsigned int *) REG_SH4_INT_ENABLE = MAGIC_CODE;
}


void aica_interrupt(void)
{
	*(unsigned int *) REG_SH4_INT_SEND = MAGIC_CODE;
}


void aica_update_fparams_table(unsigned int id, struct function_params *fparams)
{
	memcpy(&io_addr[SH_TO_ARM].fparams[id], fparams, sizeof(struct function_params));
}


static void task_birth(aica_funcp_t func, struct function_params *fparams)
{
	func(fparams->out.ptr, fparams->in.ptr);
	fparams->call_status = FUNCTION_CALL_AVAIL;
	task_exit();
}


static struct task * create_handler(aica_funcp_t func, struct function_params *fparams)
{
	struct context cxt = {
		.r0_r7 = {
			(uint32_t) func,
			(uint32_t) fparams,
			0, 0, 0, 0, 0, 0,
		},
		.pc = (aica_funcp_t) task_birth,
		.cpsr = 0x13, /* supervisor */
	};

	return task_create(&cxt);
}


/* Called from crt0.S */
void aica_sh4_fiq_hdl(void)
{
	struct call_params cparams;
	struct task *task;
	aica_funcp_t func;

	/* Switch back to newlib's reent structure */
	_impure_ptr = _global_impure_ptr;

	/* Retrieve the call parameters */
	memcpy(&cparams, &io_addr[SH_TO_ARM].cparams, sizeof(struct call_params));

	/* The call data has been read, clear the sync flag and acknowledge */
	io_addr[SH_TO_ARM].cparams.sync = 0;

	func = aica_get_func_from_id(cparams.id);
	if (!func) {
		fprintf(stderr, "No function found for ID %i.\n", cparams.id);
		__task_reschedule();
	}

	task = create_handler(func,
				&io_addr[SH_TO_ARM].fparams[cparams.id]);

	task_add_to_runnable(task, cparams.prio);
	task_select(task);
}


#ifndef _TASK_H
#define _TASK_H

#include <stdint.h>
#include <sys/reent.h>

#include "../aica_common.h"

#define DEFAULT_STACK_SIZE 4096

struct context {
	/* XXX: don't change the order */
	uint32_t r0_r7[8];
	aica_funcp_t pc;
	uint32_t r8_r14[7];
	uint32_t cpsr;
};

struct task {
	struct context context;
	struct _reent reent;
	char * stack;
	size_t stack_size;
	uint32_t id;
};

/* Pointer to the current task */
extern struct task *current_task;

/* Create a new task, with the corresponding context.
 * At least cxt->pc should be set to the function to execute.
 * XXX: The stack pointer (r13) is overwritten. */
struct task * task_create(struct context *cxt);

/* Add the task to the runnable queue */
void task_add_to_runnable(struct task *task, unsigned char prio);

/* Execute the given task.
 * XXX: This doesn't save the current context. */
void task_select(struct task *task);

/* Reschedule (yield the task).
 * Located in task_asm.S */
void task_reschedule(void);

/* Switch to another task.
 * Called from task_asm.S
 * XXX: Does not save the current task's context! */
void __task_reschedule(void);

/* Exit the currently running task */
void task_exit(void);

#endif

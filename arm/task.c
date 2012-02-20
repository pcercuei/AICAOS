
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <reent.h>
#include <sys/queue.h>

#include "../aica_common.h"
#include "task.h"

struct task *current_task;
static unsigned int id = 0;

struct TaskHandler {
	struct task * task;
	SLIST_ENTRY(TaskHandler) next;
};

static SLIST_HEAD(Head, TaskHandler) tasks [PRIORITY_MAX+1];

void task_select(struct task *task)
{
	/* Inside task_arm.S */
	void __task_select(struct context *context);

	current_task = task;
	_impure_ptr = &task->reent;
	__task_select(&task->context);
}

/* Called from task_arm.S */
void __task_reschedule()
{
	uint32_t i;
	struct TaskHandler *hdl;

	for (i = 0; i <= PRIORITY_MAX; i++) {
		hdl = SLIST_FIRST(&tasks[i]);
		if (!hdl || hdl->task == current_task)
			continue;
		task_select(hdl->task);
	}

	task_select(current_task);
}

void task_exit(void)
{
	struct TaskHandler *hdl;
	unsigned int i;

	for (i = 0; i <= PRIORITY_MAX; i++) {
		SLIST_FOREACH(hdl, &tasks[i], next) {
			if (hdl->task == current_task) {

				/* Revert to the main stack */
				asm volatile("ldr sp,=__stack");

				SLIST_REMOVE(&tasks[i], hdl, TaskHandler, next);
				free(hdl->task->stack);
				free(hdl->task);
				free(hdl);

				__task_reschedule();
			}
		}
	}
}

struct task * task_create(struct context *cxt)
{
	struct task *task = malloc(sizeof(struct task));
	if (!task)
		return NULL;

	memcpy(&task->context, cxt, sizeof(struct context));

	/* Init the stack */
	task->stack_size = DEFAULT_STACK_SIZE;
	task->stack = malloc(DEFAULT_STACK_SIZE);
	task->context.r8_r14[5] = (uint32_t) task->stack + DEFAULT_STACK_SIZE;

	/* Init newlib's reent structure */
	_REENT_INIT_PTR(&task->reent);

	task->id = id++;
	return task;
}

void task_add_to_runnable(struct task *task, unsigned char prio)
{
	struct TaskHandler *old = NULL,
					   *new = malloc(sizeof(struct TaskHandler)),
					   *hdl = SLIST_FIRST(&tasks[prio]);
	new->task = task;

	SLIST_FOREACH(hdl, &tasks[prio], next)
		old = hdl;

	if (old)
		SLIST_INSERT_AFTER(old, new, next);
	else
		SLIST_INSERT_HEAD(&tasks[prio], new, next);
}


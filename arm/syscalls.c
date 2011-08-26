
/* errno variable definition */
#include <errno.h>
#undef errno
extern int errno;

/* environment definition */
char *__env[1] = { 0 };
char **environ = __env;

int execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}

int fork(void)
{
	errno = EAGAIN;
	return -1;
}

int getpid(void)
{
	return 1;
}

int isatty(int file)
{
	return 1;
}

int kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

int times(struct tms *buf)
{
	return -1;
}

int wait(int *status)
{
	errno = ECHILD;
	return -1;
}


register char * stack_ptr asm ("sp");
static char *heap_end = (char*) 0;

caddr_t sbrk(int incr)
{
	extern char _end;		/* Defined by the linker */
	char *prev_heap_end;

	if (heap_end == 0) {
		heap_end = &_end;
	}
	prev_heap_end = heap_end;
	if (heap_end + incr > stack_ptr) {
		write (1, "Heap and stack collision\n", 25);
		abort ();
	}

	heap_end += incr;
	return (caddr_t) prev_heap_end;
}


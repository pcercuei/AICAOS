
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* errno variable definition */
#include <errno.h>
#undef errno
extern int errno;

/* environment definition */
char *__env[1] = { 0 };
char **environ = __env;

int execve(const char *name, char * const argv[], char * const env[])
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

int kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

clock_t times(struct tms *buf)
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

void * sbrk(ptrdiff_t incr)
{
	extern char _end;		/* Defined by the linker */
	char *prev_heap_end;

	if (heap_end == 0) {
		heap_end = &_end;
	}
	prev_heap_end = heap_end;
	if (heap_end + incr > stack_ptr) {
		write (1, "Heap and stack collision\n", 25);
		abort();
	}

	heap_end += incr;
	return prev_heap_end;
}



/* AICA-specific syscalls */

#include "../aica_syscalls.h"

AICA_ADD_REMOTE(sh4_open, PRIORITY_DEFAULT);
AICA_ADD_REMOTE(sh4_close, PRIORITY_DEFAULT);
AICA_ADD_REMOTE(sh4_fstat, PRIORITY_DEFAULT);
AICA_ADD_REMOTE(sh4_stat, PRIORITY_DEFAULT);
AICA_ADD_REMOTE(sh4_isatty, PRIORITY_DEFAULT);
AICA_ADD_REMOTE(sh4_link, PRIORITY_DEFAULT);
AICA_ADD_REMOTE(sh4_lseek, PRIORITY_DEFAULT);
AICA_ADD_REMOTE(sh4_read, PRIORITY_DEFAULT);
AICA_ADD_REMOTE(sh4_write, PRIORITY_DEFAULT);

int open(const char *name, int flags, int mode)
{
	int result;
	struct open_param params = { name, strlen(name), flags, mode, };

	if ( sh4_open(&result, &params) != 0 ) {
		errno = EAICA;
		return -1;
	}

	return result;
}

int close(int file)
{
	int result;

	if ( sh4_close(&result, &file) != 0 ) {
		errno = EAICA;
		return -1;
	}

	return result;
}

int fstat(int file, struct stat *st)
{
	int result;
	struct fstat_param params = { file, st, };

	if ( sh4_fstat(&result, &params) != 0 ) {
		errno = EAICA;
		return -1;
	}

	return result;
}

int stat(const char *file, struct stat *st)
{
	int result;
	struct stat_param params = { file, strlen(file), st, };

	if ( sh4_stat(&result, &params) != 0 ) {
		errno = EAICA;
		return -1;
	}

	return result;
}

int isatty(int file)
{
	int result;

	if ( sh4_isatty(&result, &file) != 0 ) {
		errno = EAICA;
		return 0;
	}

	return result;
}

int link(const char *old, const char *new)
{
	int result;
	struct link_param params = { old, strlen(old), new, strlen(new), };

	if ( sh4_link(&result, &params) != 0 ) {
		errno = EAICA;
		return -1;
	}

	return result;
}

off_t lseek(int file, off_t ptr, int dir)
{
	int result;
	struct lseek_param params = { file, ptr, dir, };

	if ( sh4_lseek(&result, &params) != 0 ) {
		errno = EAICA;
		return -1;
	}

	return result;
}

_READ_WRITE_RETURN_TYPE read(int file, void *ptr, size_t len)
{
	int result;
	struct read_param params = { file, ptr, len, };

	if ( sh4_read(&result, &params) != 0 ) {
		errno = EAICA;
		return -1;
	}

	return result;
}

_READ_WRITE_RETURN_TYPE write(int file, const void *ptr, size_t len)
{
	int result;
	struct write_param params = { file, ptr, len, };

	if ( sh4_write(&result, &params) != 0 ) {
		errno = EAICA;
		return -1;
	}

	return result;
}


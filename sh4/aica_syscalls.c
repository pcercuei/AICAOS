
#include <kos.h>
#include <alloca.h>

#include "../aica_syscalls.h"

static SHARED(sh4_open)
{
	int *result = (int *) out;
	char *fn;
	struct open_param *p = (struct open_param *) in;

	fn = alloca(p->namelen+1);
	aica_download(fn, p->name, p->namelen);
	fn[p->namelen] = '\0';

	*result = open(fn, p->flags, p->mode);
	return 0;
}

static SHARED(sh4_close)
{
	int *result = (int *) out;
	int *file = (int *) in;

	*result = close(*file);
	return 0;
}

static SHARED(sh4_fstat)
{
	int *result = (int *) out;
	struct stat st;
	struct fstat_param *p = (struct fstat_param *) in;

	*result = fstat(p->file, &st);
	aica_upload(p->st, &st, sizeof(struct stat));
	return 0;
}

static SHARED(sh4_stat)
{
	int *result = (int *) out;
	struct stat st;
	char *fn;
	struct stat_param *p = (struct stat_param *) in;

	fn = alloca(p->namelen+1);
	aica_download(fn, p->name, p->namelen);
	fn[p->namelen] = '\0';

	*result = stat(fn, &st);
	aica_upload(p->st, &st, sizeof(struct stat));
	return 0;
}

static SHARED(sh4_isatty)
{
	int *result = (int *) out;
	int *file = (int *) in;

	*result = isatty(*file);
	return 0;
}

static SHARED(sh4_link)
{
	int *result = (int *) out;
	struct link_param *p = (struct link_param *) in;
	char *fn_old, *fn_new;

	fn_old = alloca(p->namelen_old+1);
	fn_new = alloca(p->namelen_new+1);
	aica_download(fn_old, p->old, p->namelen_old);
	aica_download(fn_new, p->new, p->namelen_new);
	fn_old[p->namelen_old] = '\0';
	fn_new[p->namelen_new] = '\0';

	*result = link(fn_old, fn_new);
	return 0;
}

static SHARED(sh4_lseek)
{
	off_t *result = (off_t *) out;
	struct lseek_param *p = (struct lseek_param *) in;

	*result = lseek(p->file, p->ptr, p->dir);
	return 0;
}

/* TODO: optimize... */
static SHARED(sh4_read)
{
	_READ_WRITE_RETURN_TYPE *result = (_READ_WRITE_RETURN_TYPE *) out;
	struct read_param *p = (struct read_param *) in;
	void *buf = malloc(p->len);

	*result = read(p->file, buf, p->len);
	aica_upload(p->ptr, buf, p->len);
	free(buf);
	return 0;
}

/* TODO: optimize... */
static SHARED(sh4_write)
{
	_READ_WRITE_RETURN_TYPE *result = (_READ_WRITE_RETURN_TYPE *) out;
	struct write_param *p = (struct write_param *) in;
	void *buf = malloc(p->len);

	aica_download(buf, p->ptr, p->len);
	*result = write(p->file, buf, p->len);
	free(buf);
	return 0;
}


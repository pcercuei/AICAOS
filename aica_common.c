
#include <stdlib.h>
#include <string.h>

#include "aica_common.h"

static struct Handler {
	aica_funcp_t handler;
	const char *funcname;
	unsigned int id;
	struct Handler *next;
} *handlers = NULL;


void __aica_share(aica_funcp_t func, const char *funcname, size_t sz_in, size_t sz_out)
{
	unsigned int id;
	struct Handler *hdl = malloc(sizeof(struct Handler));

	if (handlers == NULL)
	  id = 0;
	else
	  id = handlers->id +1;

	struct function_params fparams = {
		{ sz_in, sz_in ? malloc(sz_in) : NULL, },
		{ sz_out, sz_out ? malloc(sz_out) : NULL, },
		FUNCTION_CALL_AVAIL,
	};

	aica_update_fparams_table(id, &fparams);

	hdl->id = id;
	hdl->handler = func;
	hdl->funcname = funcname;
	hdl->next = handlers;

	handlers = hdl;
}


int aica_clear_handler(unsigned int id)
{
	struct Handler *hdl, *prev;

	for (hdl = handlers, prev = NULL; hdl; prev = hdl, hdl = hdl->next) {
		if (hdl->id == id) {
			if (prev)
			  prev->next = hdl->next;
			else
			  handlers = handlers->next;

			free(hdl);
			return 0;
		}
	}

	return -1;
}

void aica_clear_handler_table(void)
{
	struct Handler *next;
	while (handlers) {
		next = handlers->next;
		free(handlers);
		handlers = next;
	}
}

int aica_find_id(unsigned int *id, char *funcname)
{
	struct Handler *hdl = handlers;

	while(hdl) {
		if (strcmp(hdl->funcname, funcname) == 0) {
			*id = hdl->id;
			return 0;
		}
		hdl = hdl->next;
	}

	return -1;
}

aica_funcp_t aica_get_func_from_id(unsigned int id)
{
	struct Handler *hdl = handlers;

	while(hdl) {
		if (hdl->id == id)
		  return hdl->handler;
		hdl = hdl->next;
	}

	return NULL;
}

const char * aica_get_funcname_from_id(unsigned int id)
{
	struct Handler *hdl = handlers;

	while(hdl) {
		if (hdl->id == id)
		  return hdl->funcname;
		hdl = hdl->next;
	}

	return NULL;
}


#include <stdio.h>
#include <stdint.h>

#include "../aica_common.h"

AICA_ADD_REMOTE(sh4_puts, 0);


int main(int argc, char **argv)
{
	aica_init(NULL);

	printf("Hello\n");
	sh4_puts(NULL, "world!");

	aica_exit();
	return 0;
}


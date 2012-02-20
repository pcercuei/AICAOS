
#include <stdio.h>
#include <stdint.h>

#include "../aica_common.h"

AICA_ADD_REMOTE(sh4_puts, 0);


int main(int argc, char **argv)
{
	printf("Hello\n");
	sh4_puts(NULL, "world!");

	return 0;
}


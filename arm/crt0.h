#ifndef _CRT0_S
#define _CRT0_S

#include <stdint.h>

void reset(void);

uint32_t int_enable(void);
uint32_t int_disable(void);
void int_restore(uint32_t context);

#endif

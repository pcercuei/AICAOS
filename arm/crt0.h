#ifndef _CRT0_S
#define _CRT0_S

void reset(void);

void int_toggle(int enable);

inline void int_enable(void)
{
	int_toggle(1);
}

inline void int_disable(void)
{
	int_toggle(0);
}

#endif

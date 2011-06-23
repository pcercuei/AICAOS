#ifndef _CRT0_S
#define _CRT0_S

void reset(void);

void fiq_enable(void);
void fiq_disable(void);

void irq_enable(void);
void irq_disable(void);

int inside_interrupt(void);

/* Called from crt0.s */
void aica_sh4_fiq_hdl(void);

/* Entry point */
int main(int argc, char **argv);

#endif

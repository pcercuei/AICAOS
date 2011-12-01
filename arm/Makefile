
include ../Makefile.inc

ELFTARGET=aicaos.elf

MY_CFLAGS = $(DC_ARM_CFLAGS) -fomit-frame-pointer

OBJS = aica_syscalls.o aica_arm.o main.o aica_common.o
CRT0 = crt0.o

all: clean $(SPUTARGET)

$(SPUTARGET): $(ELFTARGET)
	$(DC_ARM_OBJCOPY) -O binary $(ELFTARGET) $(SPUTARGET)

$(ELFTARGET): $(OBJS) $(CRT0)
	$(DC_ARM_CC) -Wl,-Tscript.ld -nostartfiles -nostdlib -o $(ELFTARGET) $(OBJS) -lc

aica_common.o: ../aica_common.c
	$(DC_ARM_CC) $(MY_CFLAGS) -I.. $(DC_ARM_INCS) $< -c -o $@

%.o: %.c
	$(DC_ARM_CC) $(MY_CFLAGS) -I. $(DC_ARM_INCS) $< -c -o $@

%.o: %.s
	$(DC_ARM_CC) $(MY_CFLAGS) -I. $(DC_ARM_INCS) $< -c -o $@

clean:
	-rm -f $(OBJS) $(CRT0) $(ELFTARGET) $(SPUTARGET) ../romdisk/$(SPUTARGET)

install: $(SPUTARGET)
	cp $(SPUTARGET) ../romdisk


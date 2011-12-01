
STARTUP(crt0.o)

SECTIONS
{
	.text 0x00000000 : {
		*(.text)
		*(.rodata)
	}

	.data : {
		*(.data)
	}

	__bss_start = .;
	.bss : {
		*(.bss)
		*(COMMON)
	}

	__bss_end__ = ALIGN(0x4);
	_end = __bss_end__;
	end = _end;

	__stack = 0x1FFFFC;
	__brk = __stack;
	_brk = __brk;
	brk = __brk;
}

#include "quadrocore.h"

void ccp_io_set(volatile uint8_t *address, volatile uint8_t value)
{
    asm volatile
	(
		"movw r30,  %0"       "\n\t"
		"ldi  r16,  %2"       "\n\t"
		"out   %3, r16"       "\n\t"
		"st     Z,  %1"       "\n\t"
		:
		: "r" (address), "r" (value), "M" (CCP_IOREG_gc), "i" (&CCP)
		: "r16", "r30", "r31"
     );
}
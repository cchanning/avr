#include "quadrocore.h"

void SetPMICLevel(uint8_t level)
{
	PMIC.CTRL = level;
}

void EnableGlobalInterrupts(void)
{
	sei();
}
	
void DisableGlobalInterrupts(void)
{
	cli();
}
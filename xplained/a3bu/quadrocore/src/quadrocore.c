#include "quadrocore.h"

int main(void)
{
	SystemClockInit();
	USBModuleInit(1, 32);
	
	for ( ; ; );
	
	return 0;
}
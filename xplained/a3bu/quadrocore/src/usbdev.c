#include "quadrocore.h"

void USBDeviceReset(void)
{	
	USBEndpointResetAll();
	USBDeviceSetAddress(0);	
}

void USBDeviceGetDescriptor(void)
{
	
}

void USBDeviceSetAddress(uint8_t address)
{
	
}
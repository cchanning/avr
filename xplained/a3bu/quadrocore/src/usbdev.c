#include "quadrocore.h"

void USBDeviceReset(void)
{
	USBEndpointTable_t *usbEndpointTableP = USBEndpointTableGet();
	
	if (! usbEndpointTableP)
	{
		return;
	}
	
	
}
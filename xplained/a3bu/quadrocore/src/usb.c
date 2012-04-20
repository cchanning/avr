#include "quadrocore.h"

static USBEndpointTable_t *usbEndpointTableP = NULL;

bool USBModuleInit(short endpointCount, uint8_t endpointBufferSize)
{
	if (! (usbEndpointTableP = USBEndpointTableAlloc(endpointCount, endpointBufferSize)))
	{
		return false;	
	}
	
	return true;
}
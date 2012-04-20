#include "quadrocore.h"

static USBEndpointTable_t *usbEndpointTableP = NULL;

bool USBModuleInit(short endpointCount, uint8_t endpointBufferSize)
{
	if (! (usbEndpointTableP = USBEndpointTableAlloc(endpointCount, endpointBufferSize)))
	{
		return false;	
	}
	
	//set the USB module EPPTR register (so it will know where the endpoint configuration lives)
	USB.EPPTR = (word_t)usbEndpointTableP->usbEndpointP;
	
	return true;
}

ISR(USB_BUSEVENT_vect)
{
	
}

ISR(USB_TRNCOMPL_vect)
{
	
}
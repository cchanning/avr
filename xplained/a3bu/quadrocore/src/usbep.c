#include "quadrocore.h"

bool USBEndpointInit(USBEndpoint_t *usbEndpointP, uint8_t endpointBufferSize)
{
	if (! usbEndpointP)
	{
		return false;
	}
	
	usbEndpointP->status = 0;
	usbEndpointP->ctrl = 0;
	usbEndpointP->cnt = 0;
	usbEndpointP->auxData = 0;
	
	if (! (usbEndpointP->dataP = calloc(1, endpointBufferSize * sizeof(byte_t))))
	{
		return false;
	}
	
	return true;
}

void USBEndpointFree(USBEndpoint_t *usbEndpointP)
{
	if (! usbEndpointP)
	{
		return;
	}
	
	if (usbEndpointP->dataP)
	{
		free(usbEndpointP->dataP);
	}
	
	usbEndpointP->status = 0;
	usbEndpointP->ctrl = 0;
	usbEndpointP->cnt = 0;
	usbEndpointP->auxData = 0;
	usbEndpointP->dataP = NULL;
}

void USBEndpointTableFree(USBEndpointTable_t *usbEndpointTableP)
{
	if (! usbEndpointTableP)
	{
		return;
	}
	
	if (usbEndpointTableP->usbEndpointP)
	{
		uint8_t endpointNumber = 0;
		
		for (endpointNumber = 0; endpointNumber < usbEndpointTableP->endpointCount; endpointNumber++)
		{
			USBEndpointFree(USBEndpointGet(usbEndpointTableP, endpointNumber, OUT));
			USBEndpointFree(USBEndpointGet(usbEndpointTableP, endpointNumber, IN));
		}
		
		free(usbEndpointTableP->usbEndpointP);
	}
	
	free(usbEndpointTableP);
}

USBEndpointTable_t* USBEndpointTableAlloc(short endpointCount, uint8_t endpointBufferSize)
{
	USBEndpointTable_t *usbEndpointTableP = NULL;
	
	if (endpointCount <= 0)
	{
		return NULL;
	}
			
	if (! (usbEndpointTableP = calloc(1, sizeof(USBEndpointTable_t))))
	{
		return NULL;
	}	
				
	usbEndpointTableP->endpointCount = endpointCount;
			
	/*
		Note that this is multiplied by 2 as each endpoint has two pipes (OUT/IN). The usbEndpointP will
		automatically point at the output pipe, adding 1 to the pointer will move it to the IN pipe as the
		endpoint structure is 8 bytes.
	*/
	if (! (usbEndpointTableP->usbEndpointP = calloc(endpointCount, sizeof(USBEndpoint_t) * 2)))
	{
		USBEndpointTableFree(usbEndpointTableP);
		return NULL;
	}
			
	{	
		uint8_t endpointNumber = 0;
		
		for (endpointNumber = 0; endpointNumber < endpointCount; endpointNumber++)
		{
			if ((! USBEndpointInit(USBEndpointGet(usbEndpointTableP, endpointNumber, OUT), endpointBufferSize))
				|| (! USBEndpointInit(USBEndpointGet(usbEndpointTableP, endpointNumber, IN), endpointBufferSize)))
			{
					USBEndpointTableFree(usbEndpointTableP);
					return NULL;
			}
		}	
	}						
	
	return usbEndpointTableP;
}

USBEndpoint_t* USBEndpointGet(USBEndpointTable_t *usbEndpointTableP, uint8_t endpointNumber, EndpointDirection endpointDirection)
{
	USBEndpoint_t *usbEndpointP = NULL;
	
	if ((! usbEndpointTableP) || (endpointNumber < 0) || (endpointNumber > usbEndpointTableP->endpointCount))
	{
		return NULL;
	}
	
	//we're already going to point at the OUT section for the EP
	usbEndpointP = usbEndpointTableP->usbEndpointP + (endpointNumber * 2);
	
	if (IN == endpointDirection)
	{
		usbEndpointP++;
	}
	
	return usbEndpointP;
}
#include "quadrocore.h"

bool USBEndpointInit(USBEndpoint_t *usbEndpointP)
{
	return true;
}

void USBEndpointFree(USBEndpoint_t *usbEndointP)
{
		
}

void USBEndpointTableFree(USBEndpointTable_t *usbEndpointTableP)
{
	if (! usbEndpointTableP)
	{
		return;
	}
	
	if (usbEndpointTableP->usbEndpointP)
	{
		USBEndpoint_t *usbEndpointP = usbEndpointTableP->usbEndpointP;
		
		for ( ; usbEndpointP < (usbEndpointTableP->usbEndpointP + usbEndpointTableP->endpointCount); usbEndpointP++)
		{
			if (usbEndpointP->dataP)
			{
				free(usbEndpointP->dataP);
			}
		}
		
		free(usbEndpointTableP->usbEndpointP);
	}
	
	free(usbEndpointTableP);
}

USBEndpointTable_t* USBEndpointTableAlloc(short endpointCount, short endpointBufferSize)
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
			
	if (! (usbEndpointTableP->usbEndpointP = calloc(endpointCount, sizeof(USBEndpoint_t))))
	{
		USBEndpointTableFree(usbEndpointTableP);
		return NULL;
	}
			
	{		
		USBEndpoint_t *usbEndpointP = usbEndpointTableP->usbEndpointP;
				
		for ( ; usbEndpointP < (usbEndpointTableP->usbEndpointP + endpointCount); usbEndpointP++)
		{
			if (! USBEndpointInit(usbEndpointP))
			{
				USBEndpointTableFree(usbEndpointTableP);
				return NULL;
			}
		}
	}						
	
	return usbEndpointTableP;
}
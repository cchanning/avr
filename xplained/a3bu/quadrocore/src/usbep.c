#include "quadrocore.h"

static USBEndpointTable_t *usbEndpointTableP = NULL;

void USBEndpointReset(USBEndpoint_t *usbEndpointP, uint8_t endpointBufferSize, uint8_t endpointType)
{
	if (! usbEndpointP)
	{
		return;
	}
	
	usbEndpointP->status = 0;
	usbEndpointP->ctrl = endpointType;
	usbEndpointP->cnt = 0;
	usbEndpointP->auxData = 0;
	
	// zero out the data buffer
	for (volatile uint8_t *dataBufferP = usbEndpointP->dataBufferP; dataBufferP < (usbEndpointP->dataBufferP) + endpointBufferSize; dataBufferP++)
	{
		*dataBufferP = 0;
	}
}

bool USBEndpointInit(USBEndpoint_t *usbEndpointP, uint8_t endpointBufferSize, uint8_t endpointType)
{
	if (! usbEndpointP)
	{
		return false;
	}
		
	if (! (usbEndpointP->dataBufferP = calloc(1, endpointBufferSize * sizeof(uint8_t))))
	{
		return false;
	}
	
	USBEndpointReset(usbEndpointP, endpointBufferSize, endpointType);
	
	return true;
}

void USBEndpointFree(USBEndpoint_t *usbEndpointP)
{
	if (! usbEndpointP)
	{
		return;
	}
	
	if (usbEndpointP->dataBufferP)
	{
		free(usbEndpointP->dataBufferP);
	}
	
	usbEndpointP->status = 0;
	usbEndpointP->ctrl = 0;
	usbEndpointP->cnt = 0;
	usbEndpointP->auxData = 0;
	usbEndpointP->dataBufferP = NULL;
}

void USBEndpointTableFree(void)
{
	if (! usbEndpointTableP)
	{
		return;
	}
	
	if (usbEndpointTableP->usbEndpointP)
	{
		uint8_t endpointNumber = 0;
		
		for (endpointNumber = 0; endpointNumber < usbEndpointTableP->usbEndpointTableConfigurationP->endpointCount; endpointNumber++)
		{
			USBEndpointFree(USBEndpointGet(endpointNumber, OUT));
			USBEndpointFree(USBEndpointGet(endpointNumber, IN));
		}
		
		free(usbEndpointTableP->baseP);
	}
	
	free(usbEndpointTableP);
	usbEndpointTableP = NULL;
}

USBEndpointTable_t* USBEndpointTableGet(void)
{
	return usbEndpointTableP;
}

bool USBEndpointTableAlloc(USBEndpointTableConfiguration_t *usbEndpointTableConfigurationP)
{
	const uint8_t FIFO_SIZE = (usbEndpointTableConfigurationP->endpointCount + 1) * 4;
	const uint16_t ENDPOINT_TABLE_SIZE = (sizeof(USBEndpoint_t) * 2) ;
		
	if (usbEndpointTableP)
	{
		return true;
	}		
	
	if (usbEndpointTableConfigurationP->endpointCount <= 0)
	{
		return false;
	}
			
	if (! (usbEndpointTableP = calloc(1, sizeof(USBEndpointTable_t))))
	{
		return false;
	}	
				
	usbEndpointTableP->usbEndpointTableConfigurationP = usbEndpointTableConfigurationP;
			
	/*
		Note that as we allocate the endpoint table dynamically we also need to take into account the FIFO memory block
		used to contain the addresses of the last "touched" endpoint. The FIFO is handled directly by the USB module and it
		appears before the endpoint table. If we were statically allocating this then we could have used a structure and its
		side by side elements but it seems restrictive as it would mean defining all of the possible endpoints up front.
	 */
	if (! (usbEndpointTableP->baseP = calloc(usbEndpointTableConfigurationP->endpointCount, FIFO_SIZE + ENDPOINT_TABLE_SIZE)))
	{
		USBEndpointTableFree();
		return false;
	}
	
	usbEndpointTableP->fifoP = usbEndpointTableP->baseP;
	usbEndpointTableP->usbEndpointP = (USBEndpoint_t *)(((uint8_t *)usbEndpointTableP->fifoP) + FIFO_SIZE);
			
	for (uint8_t endpointNumber = 0; endpointNumber < usbEndpointTableConfigurationP->endpointCount; endpointNumber++)
	{
		if ((! USBEndpointInit(USBEndpointGet(endpointNumber, OUT), usbEndpointTableConfigurationP->endpointBufferSize, usbEndpointTableConfigurationP->endpointType[endpointNumber]))
			|| (! USBEndpointInit(USBEndpointGet(endpointNumber, IN), usbEndpointTableConfigurationP->endpointBufferSize, usbEndpointTableConfigurationP->endpointType[endpointNumber])))
		{
				USBEndpointTableFree();
				return false;
		}
	}						
	
	return usbEndpointTableP;
}

USBEndpoint_t* USBEndpointGet(uint8_t endpointNumber, EndpointDirection endpointDirection)
{
	USBEndpoint_t *usbEndpointP = NULL;
	
	if ((! usbEndpointTableP) || (endpointNumber < 0) || (endpointNumber > usbEndpointTableP->usbEndpointTableConfigurationP->endpointCount))
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

USBEndpoint_t* USBEndpointGetFIFO(void)
{
	/*
		treat the usbEndpointP as just another 16bit number, add the negative fiforp value to it. This
		should cause the calculated value to be the address of the endpoint. So just cast it as a 
		pointer to the endpoint. Remember that the usbEndpointP will always point to the start of the
		endpoint table.
	 */
	return (USBEndpoint_t *)(USB.FIFORP + ((uint16_t)usbEndpointTableP->usbEndpointP));
}
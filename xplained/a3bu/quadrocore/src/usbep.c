#include "quadrocore.h"

static USBEndpointTable_t *usbEndpointTableP = NULL;

void USBEndpointResetAll(void)
{
	USBEndpointTable_t *usbEndpointTableP = USBEndpointTableGet();
	
	if (! usbEndpointTableP)
	{
		return;
	}
	
	{
		uint8_t endpointNumber = 0;
		
		for (endpointNumber = 0; endpointNumber < usbEndpointTableP->usbEndpointTableConfigurationP->endpointCount; endpointNumber++)
		{
			const USBEndpointConfiguration_t const *endpointConfigurationP = &usbEndpointTableP->usbEndpointTableConfigurationP->endpointConfiguration[endpointNumber];
			USBEndpointReset(USBEndpointGet(endpointNumber, OUT), endpointConfigurationP);
			USBEndpointReset(USBEndpointGet(endpointNumber, IN), endpointConfigurationP);
		}	
	}
}

void USBEndpointReset(USBEndpoint_t *usbEndpointP, const USBEndpointConfiguration_t const *usbEndpointConfigurationP)
{
	if (! usbEndpointP)
	{
		return;
	}
	
	usbEndpointP->status = 0;
	usbEndpointP->ctrl = usbEndpointConfigurationP->type | usbEndpointConfigurationP->bufferType;
	usbEndpointP->cnt = 0;
	usbEndpointP->auxData = 0;
	
	if (usbEndpointP->dataBufferP)
	{
		uint8_t *dataBufferP = NULL;
		
		// zero out the data buffer
		for (dataBufferP = (uint8_t *)usbEndpointP->dataBufferP; dataBufferP < ((uint8_t *)usbEndpointP->dataBufferP + usbEndpointConfigurationP->bufferSize); dataBufferP++)
		{
			*dataBufferP = 0;
		}	
	}
}

bool USBEndpointInit(USBEndpoint_t *usbEndpointP, const USBEndpointConfiguration_t const *usbEndpointConfigurationP)
{
	if (! usbEndpointP)
	{
		return false;
	}
		
	if (! (usbEndpointP->dataBufferP = calloc(1, usbEndpointConfigurationP->bufferSize * sizeof(uint8_t))))
	{
		return false;
	}
	
	USBEndpointReset(usbEndpointP, usbEndpointConfigurationP);
	
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
		free((void *)usbEndpointP->dataBufferP);
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
	
	free((void *)usbEndpointTableP);
	usbEndpointTableP = NULL;
}

USBEndpointTable_t* USBEndpointTableGet(void)
{
	return usbEndpointTableP;
}

bool USBEndpointTableAlloc(const USBEndpointTableConfiguration_t const *usbEndpointTableConfigurationP)
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
		if ((! USBEndpointInit(USBEndpointGet(endpointNumber, OUT), &usbEndpointTableConfigurationP->endpointConfiguration[endpointNumber]))
			|| (! USBEndpointInit(USBEndpointGet(endpointNumber, IN), &usbEndpointTableConfigurationP->endpointConfiguration[endpointNumber])))
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

USBEndpoint_t* USBEndpointGetDefault(EndpointDirection endpointDirection)
{
	return 	USBEndpointGet(0, endpointDirection);
}

USBEndpoint_t* USBEndpointTxQueueGetNext(void)
{
	// note that reading this register causes the USB module adjust the pointer in hardware
	sint8_t fifoRP = (sint8_t)USB.FIFORP;
	//note that reading this register has no affect on the pointer
	sint8_t fifoWP = (sint8_t)USB.FIFOWP;
	
	if (fifoRP == fifoWP)
	{
		return NULL;
	}
	
	return (USBEndpoint_t *)((((uint16_t)usbEndpointTableP->usbEndpointP) + 2) * fifoRP);
}
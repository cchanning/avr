/***********************************************************************************************************************
 * 
 * > QuadroCore <
 * 
 * Copyright (C) 2012 by Chris Channing
 *
 ***********************************************************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 ***********************************************************************************************************************/

#include "quadrocore.h"

static USBEndpointTable_t *usbEndpointTableP = NULL;

USBEndpointPipe_t* USBEndpointPipeGetBase(void);
void USBEndpointPipeReset(USBEndpointPipe_t *usbEndpointPipeP, const USBEndpointConfiguration_t const *usbEndpointConfigurationP);
bool_t USBEndpointPipeInit(USBEndpointPipe_t *usbEndpointPipeP, const USBEndpointConfiguration_t const *usbEndpointConfigurationP);
void USBEndpointPipeFree(USBEndpointPipe_t *usbEndpointPipeP);
bool_t USBEndpointInit(USBEndpoint_t *usbEndpointP);
void USBEndpointTableFree(void);
void USBEndpointFree(USBEndpoint_t *usbEndpointP);

bool_t USBEndpointTableInit(const USBEndpointTableConfiguration_t const *usbEndpointTableConfigurationP)
{		
	if (! usbEndpointTableConfigurationP) return false;
	
	if (usbEndpointTableP) return true;	
	
	if (usbEndpointTableConfigurationP->endpointCount <= 0) return false;
			
	if (! (usbEndpointTableP = calloc(1, sizeof(USBEndpointTable_t)))) return false;
		
	{
		uint8_t endpointNumber = 0;
		USBEndpointPipe_t *usbEndpointPipeP = NULL;
		const uint8_t FIFO_SIZE = (usbEndpointTableConfigurationP->endpointCount + 1) * 4;
		const uint16_t ENDPOINT_TABLE_SIZE = usbEndpointTableConfigurationP->endpointCount * (sizeof(USBEndpointPipe_t) * 2);
		const uint8_t ENDPOINT_PADDING_SIZE = 1;
		
		/*
			Note that as we allocate the endpoint table dynamically we also need to take into account the FIFO memory block
			used to contain the addresses of the last "touched" endpoint. The FIFO is handled directly by the USB module and it
			appears before the endpoint table. If we were statically allocating this then we could have used a structure and its
			side by side elements but it seems restrictive as it would mean defining all of the possible endpoints up front.
		 */
		if (! (usbEndpointTableP->blockP = calloc(1, FIFO_SIZE + ENDPOINT_TABLE_SIZE + ENDPOINT_PADDING_SIZE)))
		{
			USBEndpointTableFree();
			return false;
		}
		
		usbEndpointTableP->usbEndpointTableConfigurationP = usbEndpointTableConfigurationP;
		usbEndpointPipeP = (USBEndpointPipe_t *)(((uint8_t *)usbEndpointTableP->blockP) + FIFO_SIZE);
		usbEndpointTableP->usbEndpointListP = VectorAlloc(1, sizeof(USBEndpoint_t));
		
		/*
			Note that there appears to be a bug on the XMega256A3BU that if the USB.EPPTR is not given an even address,
			the chip seems to decrement it upon being set. However, the table is then never found during runtime.
			The only way around this is to make sure the address is even, hence the additional "padding" added to the
			memory allocation. If the address is odd then we simply increment it by one byte. 
		 */
		if (((uint16_t)usbEndpointPipeP % 2) != 0)
		{
			usbEndpointPipeP = (USBEndpointPipe_t *)(((uint8_t *)usbEndpointPipeP) + 1);
		}
		
		for (endpointNumber = 0; endpointNumber < usbEndpointTableConfigurationP->endpointCount; endpointNumber++)
		{
			USBEndpoint_t *usbEndpointP = (USBEndpoint_t *)VectorCreateRow(usbEndpointTableP->usbEndpointListP);
			usbEndpointP->endpointNumber = endpointNumber;
			usbEndpointP->usbEndpointConfigurationP =  &usbEndpointTableConfigurationP->endpointConfiguration[endpointNumber];
			usbEndpointP->endpointType = usbEndpointP->usbEndpointConfigurationP->type;
			usbEndpointP->usbEndpointOutPipeP = usbEndpointPipeP + (endpointNumber * 2);
			usbEndpointP->usbEndpointInPipeP = usbEndpointP->usbEndpointOutPipeP + 1;
		
			if (! USBEndpointInit(usbEndpointP))
			{
				USBEndpointTableFree();
				return false;
			}
		}
	}	
				
	return usbEndpointTableP;
}

void USBEndpointTableFree(void)
{
	if (! usbEndpointTableP) return;
	
	{
		uint8_t endpointNumber = 0;
		
		for (endpointNumber = 0; endpointNumber < usbEndpointTableP->usbEndpointTableConfigurationP->endpointCount; endpointNumber++)
		{
			USBEndpointFree(USBEndpointGetByNumber(endpointNumber));
		}
		
		VectorFree(usbEndpointTableP->usbEndpointListP);
		free(usbEndpointTableP->blockP);
		free((void *)usbEndpointTableP);
		usbEndpointTableP = NULL;
	}
}

uint16_t USBEndpointTableGetBaseAddress(void)
{
	return (uint16_t)(USBEndpointGetDefault()->usbEndpointOutPipeP);
}

void USBEndpointResetAll(void)
{
	uint8_t endpointNumber = 0;
	
	for (endpointNumber = 0; endpointNumber < usbEndpointTableP->usbEndpointTableConfigurationP->endpointCount; endpointNumber++)
	{
		USBEndpointReset(USBEndpointGetByNumber(endpointNumber));
	}
}

void USBEndpointReset(USBEndpoint_t *usbEndpointP)
{
	if (! usbEndpointP) return;
	
	USBEndpointPipeReset(usbEndpointP->usbEndpointOutPipeP, usbEndpointP->usbEndpointConfigurationP);
	USBEndpointPipeReset(usbEndpointP->usbEndpointInPipeP, usbEndpointP->usbEndpointConfigurationP);
}

bool_t USBEndpointInit(USBEndpoint_t *usbEndpointP)
{
	if (! usbEndpointP) return false;
	
	return (USBEndpointPipeInit(usbEndpointP->usbEndpointOutPipeP, usbEndpointP->usbEndpointConfigurationP) && USBEndpointPipeInit(usbEndpointP->usbEndpointInPipeP, usbEndpointP->usbEndpointConfigurationP));
}

void USBEndpointFree(USBEndpoint_t *usbEndpointP)
{
	if (! usbEndpointP) return;
	
	USBEndpointPipeFree(usbEndpointP->usbEndpointOutPipeP);
	USBEndpointPipeFree(usbEndpointP->usbEndpointInPipeP);
}

void USBEndpointPipeReset(USBEndpointPipe_t *usbEndpointPipeP, const USBEndpointConfiguration_t const *usbEndpointConfigurationP)
{
	if (! usbEndpointPipeP) return;
	
	usbEndpointPipeP->status = 0;
	usbEndpointPipeP->ctrl = usbEndpointConfigurationP->type | usbEndpointConfigurationP->bufferType;
	usbEndpointPipeP->cnt = 0;
	usbEndpointPipeP->auxData = 0;
	
	if (usbEndpointPipeP->dataBufferP)
	{
		memset((void *)usbEndpointPipeP->dataBufferP, 0, usbEndpointConfigurationP->bufferSize);	
	}
}

bool_t USBEndpointPipeInit(USBEndpointPipe_t *usbEndpointPipeP, const USBEndpointConfiguration_t const *usbEndpointConfigurationP)
{
	if (! usbEndpointPipeP)
	{
		return false;
	}
	
	if (! (usbEndpointPipeP->dataBufferP = calloc(1, usbEndpointConfigurationP->bufferSize * sizeof(uint8_t))))
	{
		return false;
	}
	
	USBEndpointPipeReset(usbEndpointPipeP, usbEndpointConfigurationP);
	
	return true;
}

void USBEndpointPipeFree(USBEndpointPipe_t *usbEndpointPipeP)
{
	if (! usbEndpointPipeP)
	{
		return;
	}
	
	if (usbEndpointPipeP->dataBufferP)
	{
		free((void *)usbEndpointPipeP->dataBufferP);
	}
	
	usbEndpointPipeP->status = 0;
	usbEndpointPipeP->ctrl = 0;
	usbEndpointPipeP->cnt = 0;
	usbEndpointPipeP->auxData = 0;
	usbEndpointPipeP->dataBufferP = NULL;
}

USBEndpoint_t* USBEndpointGetByPipe(USBEndpointPipe_t *usbEndpointPipeP)
{
	uint8_t endpointNumber = 0;
	
	if (! usbEndpointPipeP) return NULL;
	
	//use integer division (take only the integer part of the result) to get the endpoint number
	endpointNumber = (uint8_t)((((uint16_t)usbEndpointPipeP - USBEndpointTableGetBaseAddress()) / (uint16_t)sizeof(USBEndpointPipe_t))) / (uint8_t)2;
	
	return USBEndpointGetByNumber(endpointNumber);
}

inline USBEndpoint_t* USBEndpointGetByNumber(uint8_t endpointNumber)
{
	return VectorGetRow(usbEndpointTableP->usbEndpointListP, endpointNumber, USBEndpoint_t*);
}

inline USBEndpoint_t* USBEndpointGetDefault(void)
{
	return USBEndpointGetByNumber(0);
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
	
	return (USBEndpoint_t *)((USBEndpointTableGetBaseAddress() + 2) * fifoRP);
}
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

static Vector_t *usbTransferTableP = NULL;

bool_t USBTransferInitialized(USBTransfer_t *usbTransferP)
{
	return (usbTransferP) && (usbTransferP->usbEndpointP);
}

void USBTransferInit(USBTransfer_t *usbTransferP, uint8_t endpointNumber)
{
	if (! usbTransferP) return;
	
	usbTransferP->usbEndpointP = USBEndpointGetByNumber(endpointNumber);
	usbTransferP->callbackDataP = NULL;
	usbTransferP->callbackFuncP = NULL;
}

bool_t USBTransferTableInit(USBEndpointTableConfiguration_t *usbEndpointTableConfigurationP)
{
	if (! usbEndpointTableConfigurationP) return false;
	
	if (usbTransferTableP) return true;
	
	if (! (usbTransferTableP = VectorAlloc(1, sizeof(USBTransfer_t)))) return false;
	
	{
		uint8_t endpoint = 0;
		
		for (endpoint = 0; endpoint < usbEndpointTableConfigurationP->endpointCount; endpoint++)
		{
			VectorCreateRow(usbTransferTableP);
		}
	}
	
	return true;
}

void USBEndpointTransmit(USBEndpoint_t *usbEndpointP, size_t byteCount)
{
	USBEndpointPipe_t *usbEndpointInPipeP = usbEndpointP->usbEndpointInPipeP;
	usbEndpointInPipeP->auxData = 0;
	usbEndpointInPipeP->cnt = byteCount;
	usbEndpointInPipeP->status &= ~(1 << 1);;
}

USBTransfer_t* USBGetTransfer(USBEndpoint_t *usbEndpointP)
{
	USBTransfer_t *usbTransferP = NULL;
	
	if (! usbEndpointP) return NULL;
	
	{		
		uint8_t endpointNumber = usbEndpointP->endpointNumber;
		
		if (! (usbTransferP = VectorGetRow(usbTransferTableP, endpointNumber, USBTransfer_t*)))
		{
			return NULL;
		}
		
		if (! USBTransferInitialized(usbTransferP))
		{
			USBTransferInit(usbTransferP, endpointNumber);
		}
	}
	
	return usbTransferP;
}
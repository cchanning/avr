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

int main(void)
{
	const USBConfiguration_t const usbConfiguration = 
	{
		.usbControlTransferBufferSize = 64,
		.usbEndpointTableConfiguration = 
		{
			.endpointCount = 1,
			.maxPacketSize = 8,
			.endpointConfiguration[0] = 
			{
				.type = USB_ENDPOINT_TYPE_CONTROL,
				.bufferSize = 8,
				.bufferType = USB_EP_BUFSIZE_8_gc,
				.maxPacketSize = 8
			}
		}			
	};
	
	DisableGlobalInterrupts();
	{
		SystemClockInit();
		PMICInit(PMIC_HILVLEN_bm);
		USBModuleInit(&usbConfiguration);
		
		
		USBStandardRequest_t *usbStandardRequestP = (USBStandardRequest_t *)USBEndpointGetDefault()->usbEndpointOutPipeP->dataBufferP;
		usbStandardRequestP->index = 0;
		usbStandardRequestP->length = 10;
		usbStandardRequestP->request = 0x06;
		usbStandardRequestP->requestType = 0x80;
		usbStandardRequestP->value = 0x0600;
	}
	EnableGlobalInterrupts();
	
	for ( ; ; )
	{
		// keep looping forever (we are interrupt driven)
	}
	
	return 0;
}
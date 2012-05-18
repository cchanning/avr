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
		.usbEndpointTableConfiguration = 
		{
			.endpointCount = 1,
			.maxPacketSize = 32,
			.endpointConfiguration[0] = 
			{
				.type = USB_EP_TYPE_CONTROL_gc,
				.bufferSize = 32,
				.bufferType = USB_EP_BUFSIZE_32_gc
			}
		}
	};
	
	//vector to hold pointers
	uint8_t v1 = 1;
	uint8_t *vP1 = &v1;
	uint8_t v2 = 2;
	uint8_t *vP2 = &v2;
	
	Vector_t *vectorP = VectorAlloc(1, sizeof(ptr_t));
	
	//note we use the address of the pointer as memcpy will deference the pointer and copy the bytes, so we need a pointer to a pointer so when it's dereferenced the address is copied into the vector space
	VectorAddRow(vectorP, &vP1);
	VectorAddRow(vectorP, &vP2);
	VectorAddRow(vectorP, &vP1);
	VectorAddRow(vectorP, &vP2);
	VectorAddRow(vectorP, &vP1);
	VectorAddRow(vectorP, &vP2);

	VectorRemoveRow(vectorP, 5);
	VectorRemoveRow(vectorP, 0);
	VectorRemoveRow(vectorP, 0);
	VectorRemoveRow(vectorP, 0);
	VectorRemoveRow(vectorP, 0);
	VectorRemoveRow(vectorP, 0);
	VectorRemoveRow(vectorP, 0);
	
	Vector_t *vector2P = VectorAlloc(1, sizeof(USBTransfer_t));
	USBTransfer_t *usbTransfer1P = VectorCreateRow(vector2P);
	usbTransfer1P->transactionCount = 2;
	USBTransfer_t *usbTransfer2P = VectorCreateRow(vector2P);
	usbTransfer2P->transactionCount = 4;	
	
	VectorRemoveRow(vector2P, 1);
	VectorRemoveRow(vector2P, 0);
	
	DisableGlobalInterrupts();
	{
		SystemClockInit();
		PMICInit(PMIC_HILVLEN_bm);
		USBModuleInit(&usbConfiguration);
	}
	EnableGlobalInterrupts();
	
	for ( ; ; )
	{
		// keep looping forever (we are interrupt driven)
	}
	
	return 0;
}
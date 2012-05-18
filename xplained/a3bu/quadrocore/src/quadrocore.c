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
	uint8_t v = 1;
	uint8_t *vP = &v;
	
	Vector_t *vectorP = VectorAlloc(1, sizeof(ptr_t));
	
	//note we use the address of the pointer as memcpy will deference the pointer and copy the bytes, so we need a pointer to a pointer so when it's dereferenced the address is copied into the vector space
	ptr_t p1 = VectorAddRow(vectorP, &vP);
	ptr_t p2 = VectorAddRow(vectorP, &vP);
	
	Vector_t *vector2P = VectorAlloc(1, sizeof(USBTransfer_t));
	ptr_t p3 = VectorCreateRow(vector2P);
	USBTransfer_t *usbTransferP = VectorGetRow(vector2P, p3 , USBTransfer_t *);
	usbTransferP->transactionCount = 2;
	
	// add row gives us back a void pointer to a pointer of type X. We know that a pointer is 2 bytes wide on the 8bit avr. So we need to cast the original void pointer to a uint16_t pointer type
	// to allow use to deference the void pointer and read the address bytes (2 of them) that the void * points at. This value will be the deferenced value of the pointer we added to the vector (in this case the address of the original pointer). Once we have the address value we can then
	// cast it to the original pointer type...in this case a uint8_t*.
	uint8_t pv = *((uint8_t*)(*((uint16_t*)p1)));
	uint8_t *fun = VectorGetDeferencedRowByIndex(vectorP, 0, uint8_t*);
	fun = VectorGetDeferencedRow(vectorP, p1, uint8_t*);
	
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
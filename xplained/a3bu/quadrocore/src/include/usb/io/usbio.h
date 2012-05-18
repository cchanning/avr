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

#ifndef USBIO_H_
#define USBIO_H_

#include "type/type.h"
#include "usb/io/usbep.h"

typedef struct _USBTransfer
{
	uint8_t maxTransactionCount;
	uint8_t transactionCount;
	USBEndpoint_t *usbEndpointInP;
	USBEndpoint_t *usbEndpointOutP;
	CALLBACK_FUNC *callbackFuncP;
	void *callbackDataP;
	
} USBTransfer_t;

typedef struct _USBTransferTable
{
	uint8_t maxCount;
	uint8_t count;
	uint8_t incUnit;
	USBTransfer_t *usbTransferP;
} USBTransferTable_t;

USBTransfer_t* USBTransferAlloc(void);
void USBTransferFree(USBTransfer_t *usbTransferP);

USBTransferTable_t* USBTransferTableAlloc(uint8_t incUnit);
USBTransferTable_t* USBTransferTableFree(USBTransferTable_t *usbTransferTableP);
USBTransfer_t* USBTransferTableAdd(USBTransferTable_t *usbTransferTableP);
void USBTransferTableRemove(USBTransferTable_t *usbTransferTableP, USBTransfer_t *usbTransferP);

USBTransfer_t* USBGetTransfer(USBEndpoint_t *usbEndpointP);
USBTransfer_t USBBeginTransfer(USBEndpoint_t *usbEndpointP);
void USBEndTransfer(USBTransfer_t *usbTransferP);

void USBEndpointTransmit(USBEndpoint_t *usbEndpointP, size_t byteCount);

#endif /* USBIO_H_ */
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

typedef enum _USBTransferDirection
{
	USB_TRANSFER_DIRECTION_IN = 0,
	USB_TRANSFER_DIRECTION_OUT = 1
} USBTransferDirection_t;

typedef enum _USBTransferStage
{
	USB_TRANSFER_STAGE_SETUP = 0,
	USB_TRANSFER_STAGE_DATA = 1,
	USB_TRANSFER_STAGE_STATUS = 2
} USBTransferStage_t;

typedef struct _USBTransfer
{
	USBTransferDirection_t usbTransferDirection;
	USBTransferStage_t usbTransferStage;
	USBEndpoint_t *usbEndpointP;
	CALLBACK_FUNC callbackFuncP;
	void *callbackDataP;
	
} USBTransfer_t;

bool_t USBTransferTableInit(USBEndpointTableConfiguration_t *usbEndpointTableConfigurationP);
USBTransfer_t* USBGetTransfer(USBEndpoint_t *usbEndpointP);

bool_t USBEndpointHasOutputData(USBEndpoint_t *usbEndpointP);
void USBEndpointResetOutputBuffer(USBEndpoint_t *usbEndpointP);
void USBEndpointResetStatus(USBEndpoint_t *usbEndpointP);
void USBEndpointTransmit(USBEndpoint_t *usbEndpointP, size_t requestedByteCount, size_t byteCount);
bool_t USBEndpointIsWritable(USBEndpoint_t *usbEndpointP);
bool_t USBEndpointIsReadable(USBEndpoint_t *usbEndpointP);

#endif /* USBIO_H_ */
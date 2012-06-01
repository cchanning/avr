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

typedef enum _USBTransferType
{
	USB_TRANSFER_TYPE_CONTROL = 0
} USBTransferType_t;

typedef enum _USBControlTransferDirection
{
	USB_TRANSFER_DIRECTION_IN = 0,
	USB_TRANSFER_DIRECTION_OUT = 1
} USBControlTransferDirection_t;

typedef enum _USBControlTransferStage
{
	USB_TRANSFER_STAGE_INITIAL = 0,
	USB_TRANSFER_STAGE_DATA = 1,
	USB_TRANSFER_STAGE_AKNOWLEDGED = 2
} USBTransferStage_t;

typedef struct _USBControlTransfer
{
	ptr_t usbRequestP;
	ptr_t usbDataBufferOutP;
	ptr_t usbDataBufferInP;
	uint16_t usbBufferSize;
	USBTransferType_t usbTransferType;
	USBControlTransferDirection_t usbTransferDirection;
	USBTransferStage_t usbTransferStage;
	USBEndpoint_t *usbEndpointP;
	uint16_t requestedLength;
	uint16_t actualLength;
	uint16_t transmittedLength;
	CALLBACK_FUNC completionStageFuncP;
	void *completionStageDataP;
	
} USBControlTransfer_t;

bool_t USBTransferTableInit(uint16_t usbControlTransferBufferSize, uint8_t endpointCount);
void USBControlTransferResetAll(void);
bool_t USBControlTransferStarted(USBEndpoint_t *usbEndpointP);
USBControlTransfer_t* USBBeginControlTransfer(USBEndpoint_t *usbEndpointP);
void USBEndControlTransfer(USBEndpoint_t *usbEndpointP);
USBControlTransfer_t* USBGetControlTransfer(USBEndpoint_t *usbEndpointP);
void USBResetControlTransfer(USBControlTransfer_t *usbControlTransferP);
void USBControlTransferReportStatus(USBControlTransfer_t *usbControlTransferP);
void USBEndpointResetStatus(USBEndpoint_t *usbEndpointP);
void USBEndpointSetStalled(USBEndpoint_t *usbEndpointP);
USBControlTransferDirection_t USBGetUSBControlTransferDirection(USBControlTransfer_t *usbControlTransferP);
void USBProcessControlTransfer(USBEndpoint_t *usbEndpointP);

#endif /* USBIO_H_ */
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

void USBDeviceReset(void)
{
	USBEndpointResetAll();
	USBDeviceSetAddress(NULL, NULL, NULL);
	USB.STATUS &= ~USB_BUSRST_bm;
}

void USBDeviceGetDescriptor(USBStandardRequest_t *usbStandardRequestP, USBResponse_t *usbResponseP, USBTransfer_t *usbTransferP)
{
	USBStandardDeviceDescriptor_t *usbStandardDeviceDescriptorP = (USBStandardDeviceDescriptor_t *)usbTransferP->usbEndpointP->usbEndpointInPipeP->dataBufferP;
	
	usbStandardDeviceDescriptorP->length = sizeof(USBStandardDeviceRequest_t);
	usbStandardDeviceDescriptorP->descriptorType = DEVICE;
	usbStandardDeviceDescriptorP->deviceClass = 0x00;
	usbStandardDeviceDescriptorP->deviceSubClass = 0x00;
	usbStandardDeviceDescriptorP->deviceProtocol = 0x00;
	usbStandardDeviceDescriptorP->maxPacketSize = usbTransferP->usbEndpointP->usbEndpointConfigurationP->maxPacketSize;
	usbStandardDeviceDescriptorP->vendorId = 0x03EB;
	usbStandardDeviceDescriptorP->productId = 0x2FE2;
	usbStandardDeviceDescriptorP->deviceVersion = 0x0002;
	usbStandardDeviceDescriptorP->manufacturerIndex = 0x01;
	usbStandardDeviceDescriptorP->productIndex = 0x01;
	usbStandardDeviceDescriptorP->serialNumberIndex = 0x00;
	usbStandardDeviceDescriptorP->numberOfConfigurations = 0x01;
	usbStandardDeviceDescriptorP->usbVersion = 0x0200;
	
	usbResponseP->byteCount = usbStandardDeviceDescriptorP->length;
	PORTR.DIR = 0xff;
	PORTR.OUTSET = 0x00;
}

void USBDeviceSetAddress(USBStandardRequest_t *usbStandardRequestP, USBResponse_t *usbResponseP, USBTransfer_t *usbTransferP)
{
	usbResponseP->byteCount = 0;
}
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

void USBDeviceSetAddressCallback(ptr_t dataP);

void USBDeviceReset(void)
{
	USBControlTransferResetAll();
	USBEndpointResetAll();
	USBDeviceSetAddress(0);
	USB.STATUS &= ~USB_BUSRST_bm;
}

void USBDeviceGetDescriptor(USBControlTransfer_t *usbControlTransferP)
{
	USBStandardDeviceDescriptor_t *usbStandardDeviceDescriptorP = (USBStandardDeviceDescriptor_t *)usbControlTransferP->usbDataBufferInP;
	
	usbStandardDeviceDescriptorP->length = sizeof(USBStandardDeviceDescriptor_t);
	usbStandardDeviceDescriptorP->descriptorType = USB_STANDARD_DESCRIPTOR_TYPE_DEVICE;
	usbStandardDeviceDescriptorP->deviceClass = 0x00;
	usbStandardDeviceDescriptorP->deviceSubClass = 0x00;
	usbStandardDeviceDescriptorP->deviceProtocol = 0x00;
	usbStandardDeviceDescriptorP->maxPacketSize = usbControlTransferP->usbEndpointP->usbEndpointConfigurationP->maxPacketSize;
	usbStandardDeviceDescriptorP->vendorId = 0x03EB;
	usbStandardDeviceDescriptorP->productId = 0x2FE2;
	usbStandardDeviceDescriptorP->deviceVersion = 0x0002;
	usbStandardDeviceDescriptorP->manufacturerIndex = 0x01;
	usbStandardDeviceDescriptorP->productIndex = 0x01;
	usbStandardDeviceDescriptorP->serialNumberIndex = 0x00;
	usbStandardDeviceDescriptorP->numberOfConfigurations = 0x01;
	usbStandardDeviceDescriptorP->usbVersion = 0x0200;
	
	usbControlTransferP->actualLength = usbStandardDeviceDescriptorP->length;
}

void USBDeviceSetDeferredAddress(USBControlTransfer_t *usbControlTransferP)
{
	//a usb address is only 7 bits long, guarantee we don't get any other garbage in the 16bit request value
	uint8_t *addressP = calloc(1, sizeof(uint8_t));
	*addressP = ((USBStandardRequest_t*)usbControlTransferP->usbRequestP)->value & 0x007F;
	
	usbControlTransferP->completionStageFuncP = &USBDeviceSetAddressCallback;
	usbControlTransferP->completionStageDataP = (ptr_t)addressP;
}

void USBDeviceSetAddressCallback(ptr_t addressP)
{
	USBDeviceSetAddress(*((uint8_t *)addressP));
}

void USBDeviceSetAddress(uint8_t address)
{
	USB.ADDR = (register8_t)address;	
}
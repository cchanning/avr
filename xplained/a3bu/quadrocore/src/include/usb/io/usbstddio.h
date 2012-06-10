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

#ifndef USBSTDDREQ_H_
#define USBSTDDREQ_H_

#include "usb/io/usbstdio.h"

typedef struct _USBStandardDeviceDescriptor
{
	uint8_t length;
	uint8_t descriptorType;
	uint16_t usbVersion;
	uint8_t deviceClass;
	uint8_t deviceSubClass;
	uint8_t deviceProtocol;
	uint8_t maxPacketSize;
	uint16_t vendorId;
	uint16_t productId;
	uint16_t deviceVersion;
	uint8_t manufacturerIndex;
	uint8_t productIndex;
	uint8_t serialNumberIndex;
	uint8_t numberOfConfigurations;
} USBStandardDeviceDescriptor_t;

typedef struct _USBStandardDeviceConfigurationDescriptor
{
	uint8_t length;
	uint8_t descriptorType;
	uint16_t totalLength;
	uint8_t numInterfaces;
	uint8_t configurationValue;
	uint8_t configurationIndex;
	uint8_t attributes;
	uint8_t maxPower;
} USBStandardDeviceConfigurationDescriptor_t;

typedef struct _USBStandardInterfaceConfigurationDescriptor
{
	uint8_t length;
	uint8_t descriptorType;
	uint8_t interfaceNumber;
	uint8_t alternateSetting;
	uint8_t numberOfEndpoints;
	uint8_t interfaceClass;
	uint8_t interfaceSubClass;
	uint8_t interfaceProtocol;
	uint8_t interfaceIndex;	
} USBStandardInterfaceConfigurationDescriptor_t;

typedef struct _USBStandardHIDConfigurationDescriptor
{
	uint8_t length;
	uint8_t descriptorType;
	uint16_t version;
	uint8_t countryCode;
	uint8_t numberOfDescriptors;
	uint8_t nestedDescriptorType;
	uint16_t nestedDesciptorLength;
} USBStandardHIDConfigurationDescriptor_t;

typedef struct _USBStandardEndpointConfigurationDescriptor
{
	uint8_t length;
	uint8_t descriptorType;
	uint8_t endpointAddress;
	uint8_t attributes;
	uint16_t maxPacketSize;
	uint8_t interval;
} USBStandardEndpointConfigurationDescriptor_t;

void USBDeviceReset(void);
void USBDeviceGetDescriptor(USBControlTransfer_t *usbControlTransferP);
void USBDeviceGetConfigurationDescriptor(USBControlTransfer_t *usbControlTransferP);
void USBDeviceSetAddress(uint8_t address);
void USBDeviceSetDeferredAddress(USBControlTransfer_t *usbControlTransferP);
void USBDeviceGetString(USBControlTransfer_t *usbControlTransferP);
void USBDeviceGetConfiguration(USBControlTransfer_t *usbControlTransferP);
void USBDeviceSetConfiguration(USBControlTransfer_t * usbControlTransferP);
void USBDeviceGetHIDReportDescriptor(USBControlTransfer_t *usbControlTransferP);

#endif /* USBSTDDREQ_H_ */
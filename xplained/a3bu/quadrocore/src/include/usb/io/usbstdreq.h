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

#ifndef USBSTDREQ_H_
#define USBSTDREQ_H_

#include "usb/io/usbio.h"

#define USB_REQUEST_TYPE_FLD_TYPE_bm 0x60
#define USB_REQUEST_TYPE_FLD_DPTFD_HD_bm (0 << 7)
#define USB_REQUEST_TYPE_FLD_DPTFD_DH_bm (1 << 7)
#define USB_REQUEST_TYPE_FLD_TYPE_STANDARD_bm (0 << 6)
#define USB_REQUEST_TYPE_FLD_TYPE_CLASS_bm (1 << 6)
#define USB_REQUEST_TYPE_FLD_TYPE_VENDOR_bm (2 << 5)
#define USB_REQUEST_TYPE_FLD_TYPE_RESERVED_bm (3 << 5)
#define USB_REQUEST_TYPE_FLD_RECIPIENT_bm 0x1F
#define USB_REQUEST_TYPE_FLD_RECIPIENT_DEVICE_bm (0 << 4)
#define USB_REQUEST_TYPE_FLD_RECIPIENT_INTERFACE_bm (1 << 4)
#define USB_REQUEST_TYPE_FLD_RECIPIENT_ENDPOINT_bm (2 << 3)
#define USB_REQUEST_TYPE_FLD_RECIPIENT_OTHER_bm (3 << 3)

#define USB_REQUEST_DEVICE_GET_DESCRIPTOR 0x06
#define USB_REQUEST_DEVICE_SET_ADDRESS 0x05

#define USB_REQUEST_TYPE_HANDLER_COUNT 2

typedef struct _USBResponse
{
	uint16_t byteCount;
} USBResponse_t;

void USBProcessStandardRequest(USBTransfer_t *usbTransferP);

enum DescriptorType
{
	DEVICE = 0x01	
};

typedef struct _USBStandardRequest
{
	uint8_t requestType;
	uint8_t request;
} USBStandardRequest_t;

typedef void (*USB_STANDARD_REQUEST_HANDLER_FUNC)(USBStandardRequest_t *usbStandardRequestP, USBResponse_t *usbResponseP, USBTransfer_t *usbTransferP);

typedef struct _USBStandardRequestHandler
{
	uint8_t type;
	uint8_t recipient;
	uint8_t id;
	USB_STANDARD_REQUEST_HANDLER_FUNC handlerFuncP;
} USBStandardRequestHandler_t;

#endif /* USBSTDREQ_H_ */
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

static bool_t usbSetupRequestHandlerTableInitialized = false;
static USBStandardRequestHandler_t USB_STANDARD_REQUEST_HANDLER_TABLE[USB_REQUEST_TYPE_HANDLER_COUNT];

USBStandardRequestHandler_t* USBStandardRequestHandlerTableGet(void)
{
	if (! usbSetupRequestHandlerTableInitialized)
	{
		int i = 0;
		USB_STANDARD_REQUEST_HANDLER_TABLE[i].type = USB_REQUEST_TYPE_FLD_TYPE_STANDARD_bm;
		USB_STANDARD_REQUEST_HANDLER_TABLE[i].recipient = USB_REQUEST_TYPE_FLD_RECIPIENT_DEVICE_bm;
		USB_STANDARD_REQUEST_HANDLER_TABLE[i].id = USB_REQUEST_DEVICE_SET_ADDRESS;
		USB_STANDARD_REQUEST_HANDLER_TABLE[i].handlerFuncP = &USBDeviceSetDeferredAddress;
		
		i++;
		USB_STANDARD_REQUEST_HANDLER_TABLE[i].type = USB_REQUEST_TYPE_FLD_TYPE_STANDARD_bm;
		USB_STANDARD_REQUEST_HANDLER_TABLE[i].recipient = USB_REQUEST_TYPE_FLD_RECIPIENT_DEVICE_bm;
		USB_STANDARD_REQUEST_HANDLER_TABLE[i].id = USB_REQUEST_DEVICE_GET_DESCRIPTOR;
		USB_STANDARD_REQUEST_HANDLER_TABLE[i].handlerFuncP = &USBDeviceGetDescriptor;
		
		usbSetupRequestHandlerTableInitialized = true;
	}
	
	return USB_STANDARD_REQUEST_HANDLER_TABLE;
}

USBStandardRequestHandler_t* USBStandardRequestResolveHandler(USBStandardRequest_t *usbStandardRequestP)
{
	USBStandardRequestHandler_t *usbStandardRequestHandlerTableP = USBStandardRequestHandlerTableGet();
		
	if (! usbStandardRequestHandlerTableP)
	{
		return NULL;
	}
		
	//decode the type and recipient from the request
	{
		uint8_t type = usbStandardRequestP->requestType & USB_REQUEST_TYPE_FLD_TYPE_bm;
		uint8_t recipient = usbStandardRequestP->requestType & USB_REQUEST_TYPE_FLD_RECIPIENT_bm;
		
		for (USBStandardRequestHandler_t *usbStandardRequestHandlerP = usbStandardRequestHandlerTableP; usbStandardRequestHandlerP < (usbStandardRequestHandlerTableP + USB_REQUEST_TYPE_HANDLER_COUNT); usbStandardRequestHandlerP++)
		{
			// if we get match on the id, just double check that we're scoped properly for request just in case request ids are not unique
			if ((usbStandardRequestHandlerP->id == usbStandardRequestP->request) && (usbStandardRequestHandlerP->recipient == recipient) && (usbStandardRequestHandlerP->type == type))
			{
				return usbStandardRequestHandlerP;
			}
		}			
	}
				
	return NULL;
}

void USBParseStandardRequestMetaData(USBControlTransfer_t *usbControlTransferP)
{
	USBStandardRequest_t *usbStandardRequestP = (USBStandardRequest_t *)usbControlTransferP->usbRequestP;
	usbControlTransferP->requestedLength = usbStandardRequestP->length;
	usbControlTransferP->usbTransferDirection = ((usbStandardRequestP->requestType >> 7) > 0 ? USB_TRANSFER_DIRECTION_IN : USB_TRANSFER_DIRECTION_OUT);
}

bool_t USBProcessStandardRequest(USBControlTransfer_t *usbControlTransferP)
{
	USBStandardRequestHandler_t *usbStandardRequestHandlerP = USBStandardRequestResolveHandler((USBStandardRequest_t *)usbControlTransferP->usbRequestP);
		
	// this isn't a standard request type
	if (! usbStandardRequestHandlerP)
	{
		return false;
	}
	
	// delegate to the request handler
	(*usbStandardRequestHandlerP->handlerFuncP)(usbControlTransferP);
	
	return true;
}
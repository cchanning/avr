#include "quadrocore.h"

static bool setupRequestHandlerTableInitialized = false;
static SetupRequestHandler_t SETUP_REQUEST_HANDLER_TABLE[USB_REQUEST_TYPE_HANDLER_COUNT];

SetupRequestHandler_t* SetupRequestHandlerTableGet(void)
{
	if (! setupRequestHandlerTableInitialized)
	{
		int i = 0;
		SETUP_REQUEST_HANDLER_TABLE[i].type = USB_REQUEST_TYPE_FLD_TYPE_STANDARD_bm;
		SETUP_REQUEST_HANDLER_TABLE[i].recipient = USB_REQUEST_TYPE_FLD_RECIPIENT_DEVICE_bm;
		SETUP_REQUEST_HANDLER_TABLE[i].id = USB_REQUEST_DEVICE_SET_ADDRESS;
		SETUP_REQUEST_HANDLER_TABLE[i].handlerFuncP = &USBDeviceSetAddress;
		
		i++;
		SETUP_REQUEST_HANDLER_TABLE[i].type = USB_REQUEST_TYPE_FLD_TYPE_STANDARD_bm;
		SETUP_REQUEST_HANDLER_TABLE[i].recipient = USB_REQUEST_TYPE_FLD_RECIPIENT_DEVICE_bm;
		SETUP_REQUEST_HANDLER_TABLE[i].id = USB_REQUEST_DEVICE_GET_DESCRIPTOR;
		SETUP_REQUEST_HANDLER_TABLE[i].handlerFuncP = &USBDeviceGetDescriptor;
		
		setupRequestHandlerTableInitialized = true;
	}
	
	return SETUP_REQUEST_HANDLER_TABLE;
}

SetupRequestHandler_t* SetupRequestResolveHandler(USBRequest_t *usbRequestP)
{
	if (! usbRequestP)
	{
		return NULL;
	}

	{
		SetupRequestHandler_t *setupRequestHandlerTableP = SetupRequestHandlerTableGet();
		
		if (! setupRequestHandlerTableP)
		{
			return NULL;
		}
		
		//decode the type and recipient from the request
		uint8_t type = usbRequestP->requestType & USB_REQUEST_TYPE_FLD_TYPE_bm;
		uint8_t recipient = usbRequestP->requestType & USB_REQUEST_TYPE_FLD_RECIPIENT_bm;
		
		{
			SetupRequestHandler_t *setupRequestHandlerP = NULL;
		
			for (setupRequestHandlerP = setupRequestHandlerTableP; setupRequestHandlerP < (setupRequestHandlerTableP + USB_REQUEST_TYPE_HANDLER_COUNT); setupRequestHandlerP++)
			{
				// if we get match on the id, just double check that we're scoped properly for request just in case request ids are not unique
				if ((setupRequestHandlerP->id == usbRequestP->request) && (setupRequestHandlerP->recipient == recipient) && (setupRequestHandlerP->type == type))
				{
					return setupRequestHandlerP;
				}
			}
		}			
	}
		
	return NULL;
}

void ProcessSetupRequest(USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP)
{			
	if (! usbEndpointOutP->cnt)
	{
		return;
	}
	
	{
		USBRequest_t *usbRequestP = (USBRequest_t *)usbEndpointOutP->dataBufferP;
		SetupRequestHandler_t *setupRequestHandlerP = SetupRequestResolveHandler(usbRequestP);
		
		if (! setupRequestHandlerP)
		{
			return;
		}
		
		// handle the request and transmit the response
		{
			USBResponse_t *usbResponseP = NULL;
			
			if (! (usbResponseP = calloc(1, sizeof(USBResponse_t))))
			{
				return;
			}
			
			(*setupRequestHandlerP->handlerFuncP)(usbRequestP, usbResponseP, usbEndpointOutP, usbEndpointInP);
			USBEndpointTransmit(usbEndpointInP, usbResponseP->byteCount);
			free(usbResponseP);
		}
	}
}
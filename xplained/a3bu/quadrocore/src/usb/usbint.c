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

static USBTransfer_t *usbTransferP = NULL;

ISR(USB_BUSEVENT_vect)
{
	DisableGlobalInterrupts();
	{		
		if (USB.INTFLAGSASET & USB_SOFIF_bm)
		{
			USB.INTFLAGSACLR = USB_SOFIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_SUSPENDIF_bm)
		{
			USB.INTFLAGSACLR = USB_SUSPENDIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_RESUMEIF_bm)
		{
			USB.INTFLAGSACLR = USB_RESUMEIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_RSTIF_bm)
		{
			USBDeviceReset();
			USB.INTFLAGSACLR = USB_RSTIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_CRCIF_bm)
		{	
			USB.INTFLAGSACLR = USB_CRCIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_UNFIF_bm)
		{
			USB.INTFLAGSACLR = USB_UNFIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_OVFIF_bm)
		{
			USB.INTFLAGSACLR = USB_OVFIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_STALLIF_bm)
		{
			USB.INTFLAGSACLR = USB_STALLIF_bm;
		}
	}	
	EnableGlobalInterrupts();
}

ISR(USB_TRNCOMPL_vect)
{	
	DisableGlobalInterrupts();
	{	
		//temp until we setup the transfer table
		if (! usbTransferP)
		{
			usbTransferP = calloc(1, sizeof(USBTransfer_t));
			usbTransferP->usbEndpointP = USBEndpointGetDefault();
			usbTransferP->callbackDataP = NULL;
			usbTransferP->callbackFuncP = NULL;
		}
		
		if (USB.INTFLAGSBSET & USB_SETUPIF_bm)
		{
			USBProcessStandardRequest(usbTransferP);
			usbTransferP->usbEndpointP->usbEndpointOutPipeP->status &= ~(USB_EP_SETUP_bm | USB_EP_TRNCOMPL0_bm | USB_EP_BUSNACK0_bm | USB_EP_OVF_bm);
			usbTransferP->usbEndpointP->usbEndpointInPipeP->status &= ~(USB_EP_TRNCOMPL0_bm | USB_EP_BUSNACK0_bm | USB_EP_OVF_bm);
			USB.INTFLAGSBCLR = USB_SETUPIF_bm;	
		}
		
		if (USB.INTFLAGSBSET & USB_TRNIF_bm)
		{
			if (usbTransferP->callbackFuncP)
			{
				(*usbTransferP->callbackFuncP)(usbTransferP->callbackDataP);
				free(usbTransferP->callbackDataP);
				usbTransferP->callbackFuncP = NULL;
				usbTransferP->callbackDataP = NULL;
			}
			
			usbTransferP->usbEndpointP->usbEndpointOutPipeP->status &= ~(USB_EP_TRNCOMPL0_bm | USB_EP_BUSNACK0_bm | USB_EP_OVF_bm);
			usbTransferP->usbEndpointP->usbEndpointInPipeP->status &= ~(USB_EP_TRNCOMPL0_bm | USB_EP_BUSNACK0_bm | USB_EP_OVF_bm);
			USB.INTFLAGSBCLR = USB_TRNIF_bm;
		}
	}	
	EnableGlobalInterrupts();	
}
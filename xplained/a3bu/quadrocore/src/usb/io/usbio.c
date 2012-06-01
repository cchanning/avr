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

static Vector_t *usbTransferTableP = NULL;

bool_t USBTransferTableInit(uint16_t usbControlTransferBufferSize, uint8_t endpointCount)
{
	if (usbTransferTableP) return true;
	
	if (! (usbTransferTableP = VectorAlloc(1, sizeof(USBControlTransfer_t)))) return false;
	
	for (uint8_t endpointNumber = 0; endpointNumber < endpointCount; endpointNumber++)
	{
		USBControlTransfer_t *usbControlTransferP = (USBControlTransfer_t *)VectorCreateRow(usbTransferTableP);
		usbControlTransferP->usbRequestP = calloc(1, usbControlTransferBufferSize);
		usbControlTransferP->usbDataBufferInP = calloc(1, usbControlTransferBufferSize);
		usbControlTransferP->usbDataBufferOutP = calloc(1, usbControlTransferBufferSize);
	}
	
	return true;
}

void USBEndpointResetStatus(USBEndpoint_t *usbEndpointP)
{
	if (! usbEndpointP)
	{
		return;
	}
	
	usbEndpointP->usbEndpointOutPipeP->status &= ~(USB_EP_SETUP_bm | USB_EP_TRNCOMPL0_bm | USB_EP_BUSNACK0_bm | USB_EP_OVF_bm | USB_EP_STALLF_bm);
	usbEndpointP->usbEndpointInPipeP->status &= ~(USB_EP_TRNCOMPL0_bm | USB_EP_BUSNACK0_bm | USB_EP_OVF_bm | USB_EP_STALLF_bm);
}

void USBControlTransferReportStatus(USBControlTransfer_t *usbControlTransferP)
{
	usbControlTransferP->usbTransferStage = USB_TRANSFER_STAGE_AKNOWLEDGED;
	
	/**
		When the direction is OUT then the host will be expecting a ZLP packet to be sent back when it sends an IN token. When the direction is IN, 
		the host will just expect an ACK packet which will be sent by the USB module as once the status of the out endpoint pipe is cleared an OUT
		token will be handled automatically.
		*/
			
	if (usbControlTransferP->usbTransferDirection == USB_TRANSFER_DIRECTION_OUT)
	{
		usbControlTransferP->usbEndpointP->usbEndpointInPipeP->auxData = 0;
		usbControlTransferP->usbEndpointP->usbEndpointInPipeP->cnt = 0;				
	}
	
	USBEndpointResetStatus(usbControlTransferP->usbEndpointP);
}

USBControlTransfer_t* USBGetControlTransfer(USBEndpoint_t *usbEndpointP)
{
	USBControlTransfer_t *usbControlTransferP = NULL;
	uint8_t endpointNumber = usbEndpointP->endpointNumber;
	
	if (! (usbControlTransferP = VectorGetRow(usbTransferTableP, endpointNumber, USBControlTransfer_t*)))
	{
		return NULL;
	}
	
	return usbControlTransferP;	
}

void USBResetControlTransfer(USBControlTransfer_t *usbControlTransferP)
{
	/**
		Zero out all of the various buffers for the control transfer
	 */
	memset(usbControlTransferP->usbRequestP, 0, usbControlTransferP->usbEndpointP->usbEndpointConfigurationP->bufferSize);
	memset(usbControlTransferP->usbDataBufferOutP, 0, usbControlTransferP->usbEndpointP->usbEndpointConfigurationP->bufferSize);
	memset(usbControlTransferP->usbDataBufferInP, 0, usbControlTransferP->usbEndpointP->usbEndpointConfigurationP->bufferSize);
	
	usbControlTransferP->completionStageDataP = NULL;
	usbControlTransferP->completionStageFuncP = NULL;
	usbControlTransferP->requestedLength = 0;
	usbControlTransferP->transmittedLength = 0;
	usbControlTransferP->actualLength = 0;
	usbControlTransferP->usbEndpointP = NULL;
	usbControlTransferP->usbTransferDirection = 0;
	usbControlTransferP->usbTransferStage = 0;
	usbControlTransferP->usbTransferType = 0;
}

bool_t USBControlTransferStarted(USBEndpoint_t *usbEndpointP)
{	
	USBControlTransfer_t *usbTransferP = USBGetControlTransfer(usbEndpointP);
		
	if ((! usbEndpointP) || (! usbTransferP->usbEndpointP))
	{
		return false;
	}
	
	return true;
}

USBControlTransfer_t* USBBeginControlTransfer(USBEndpoint_t *usbEndpointP)
{
	USBControlTransfer_t *usbTransferP = USBGetControlTransfer(usbEndpointP);
	
	if (! usbTransferP)
	{
		return NULL;
	}
	
	USBResetControlTransfer(usbTransferP);
	usbTransferP->usbEndpointP = usbEndpointP;
	usbTransferP->usbTransferStage = USB_TRANSFER_STAGE_INITIAL;
	memcpy((ptr_t)usbTransferP->usbRequestP, (ptr_t)usbTransferP->usbEndpointP->usbEndpointOutPipeP->dataBufferP, usbTransferP->usbEndpointP->usbEndpointConfigurationP->bufferSize);
	USBParseStandardRequestMetaData(usbTransferP);
		
	return usbTransferP;	
}

void USBEndControlTransfer(USBEndpoint_t *usbEndpointP)
{
	USBControlTransfer_t *usbTransferP = USBGetControlTransfer(usbEndpointP);
	
	if (! usbTransferP)
	{
		return;
	}
	
	USBResetControlTransfer(usbTransferP);
}

void USBProcessControlTransfer(USBEndpoint_t *usbEndpointP)
{
	USBControlTransfer_t *usbControlTransferP = (! USBControlTransferStarted(usbEndpointP) ? USBBeginControlTransfer(usbEndpointP) : USBGetControlTransfer(usbEndpointP));
	
	/**
		The host has acknowledged our status response, so lets make sure we finish any outstanding work.
	 */
	if (usbControlTransferP->usbTransferStage == USB_TRANSFER_STAGE_AKNOWLEDGED)
	{
		if (usbControlTransferP->completionStageFuncP)
		{
			(*usbControlTransferP->completionStageFuncP)(usbControlTransferP->completionStageDataP);
			free(usbControlTransferP->completionStageDataP);
		}
		
		USBEndControlTransfer(usbEndpointP);
		
		return;
	}
	
	/**
		Data needs to be moved between the host and the device (data stage), work out which way and buffer the data until we have it all if required.
	 */
	if (usbControlTransferP->requestedLength > 0)
	{
		usbControlTransferP->usbTransferStage = USB_TRANSFER_STAGE_DATA;
		
		/**
			The host wants to send us some data, make sure we have it all before delegating to the request handler for processing. Each request handler requiring data from the host
			expects the complete picture before it will process the request.
		 */
		if (usbControlTransferP->usbTransferDirection == USB_TRANSFER_DIRECTION_OUT)
		{
			usbControlTransferP->transmittedLength += usbControlTransferP->usbEndpointP->usbEndpointOutPipeP->cnt;
			
			/**
				We have all of the data we need from the host, so process the request
			 */
			if (usbControlTransferP->transmittedLength >= usbControlTransferP->requestedLength)
			{
				USBProcessStandardRequest(usbControlTransferP);
				USBControlTransferReportStatus(usbControlTransferP);
			}
			else
			{
				/**
					The host has sent us some data, copy it in to the control transfer OUT data buffer.
				 */
				if (usbControlTransferP->usbEndpointP->usbEndpointOutPipeP->cnt > 0)
				{
					memcpy((ptr_t)usbControlTransferP->usbDataBufferOutP + usbControlTransferP->transmittedLength, (ptr_t)usbControlTransferP->usbEndpointP->usbEndpointOutPipeP->dataBufferP, usbControlTransferP->usbEndpointP->usbEndpointOutPipeP->cnt);	
				}
				
				/**
					We don't have all of the data yet, so clear the endpoint status to allow the host to keep sending us OUT tokens.
				 */
				USBEndpointResetStatus(usbControlTransferP->usbEndpointP);
			}
		}
		
		/**
			The host wants us to send it data, process the request in order to have it dumped into the internal control transfer IN buffer and then split the buffer into multiple IN
			tokens.
		 */
		if (usbControlTransferP->usbTransferDirection == USB_TRANSFER_DIRECTION_IN)
		{
			bool_t transmitRequired = true;
			
			if (usbControlTransferP->usbEndpointP->usbEndpointInPipeP->cnt == 0)
			{
				/**
					Populate the control transfer IN buffer with the response, we'll chunk this up into multiple (if required) IN tokens later.
				 */
				USBProcessStandardRequest(usbControlTransferP);			
			}
			else
			{
				/**
					The IN based endpoint pipe will reflect the data sent in the last transaction, we need to populate it ready for the next IN token from the host.
				 */
				usbControlTransferP->transmittedLength += usbControlTransferP->usbEndpointP->usbEndpointInPipeP->cnt;
				
				if (usbControlTransferP->transmittedLength >= usbControlTransferP->actualLength)
				{
					USBControlTransferReportStatus(usbControlTransferP);
					transmitRequired = false;
				}
			}
			
			/**
				Load the IN endpoint data buffer with the next chunk of data from the control transfer IN data buffer (holding the full response)
			 */	
			if (transmitRequired)
			{
				uint16_t maxPacketSize = usbControlTransferP->usbEndpointP->usbEndpointConfigurationP->maxPacketSize;
				uint16_t remainingSize = usbControlTransferP->actualLength - usbControlTransferP->transmittedLength;
				uint16_t size = (remainingSize > maxPacketSize ? maxPacketSize : remainingSize);
				
				usbControlTransferP->usbEndpointP->usbEndpointInPipeP->cnt = size;
				usbControlTransferP->usbEndpointP->usbEndpointInPipeP->auxData = 0;
				memcpy((ptr_t)usbControlTransferP->usbEndpointP->usbEndpointInPipeP->dataBufferP, (ptr_t)usbControlTransferP->usbDataBufferInP + usbControlTransferP->transmittedLength, size);
				
				USBEndpointResetStatus(usbControlTransferP->usbEndpointP);
			}
		}	
	}
	else
	{
		/**
			There is no data stage, so just process the request
		 */
		USBProcessStandardRequest(usbControlTransferP);
		USBControlTransferReportStatus(usbControlTransferP);
	}
}

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

bool_t USBModuleInit(const USBConfiguration_t *usbConfigurationP)
{
	if (! USBEndpointTableInit(&usbConfigurationP->usbEndpointTableConfiguration))
	{
		return false;	
	}
	
	if (! USBTransferTableInit(usbConfigurationP->usbControlTransferBufferSize, usbConfigurationP->usbEndpointTableConfiguration.endpointCount))
	{
		return false;
	}		
		
	//set the USB module EPPTR register (so it will know where the endpoint table configuration lives)
	USB.EPPTR = USBEndpointTableGetBaseAddress();
	
	//load the USB calibration data from the device production row into the USB calibration registers
	NVM.CMD = NVM_CMD_READ_CALIB_ROW_gc; 
	USB.CAL0 = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, USBCAL0));
	USB.CAL1 = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, USBCAL1));
	NVM.CMD = NVM_CMD_NO_OPERATION_gc; 
	
	USB.INTCTRLA = USB_INTLVL_HI_gc | USB_BUSEVIE_bm;
	USB.INTCTRLB = USB_TRNIE_bm | USB_SETUPIE_bm;
	
	USB.STATUS = 0;
	
	// set the PLL as the USB clock source, enable the USB clock source (start feeding the USB module clock signals)
	SetProtectedMemory(&CLK.USBCTRL, CLK_USBSRC_PLL_gc | CLK_USBSEN_bm);
	
	USB.CTRLA = USB_ENABLE_bm | USB_SPEED_bm | usbConfigurationP->usbEndpointTableConfiguration.endpointCount;
	
	// attach the USB module to the bus (allows the host to start the enumeration process)
	USB.CTRLB = USB_ATTACH_bm;
	
	return true;
}
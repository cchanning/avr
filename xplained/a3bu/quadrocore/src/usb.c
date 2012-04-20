#include "quadrocore.h"

static USBEndpointTable_t *usbEndpointTableP = NULL;

bool USBModuleInit(short endpointCount, uint8_t endpointBufferSize)
{
	if (! (usbEndpointTableP = USBEndpointTableAlloc(endpointCount, endpointBufferSize)))
	{
		return false;	
	}
		
	//set the USB module EPPTR register (so it will know where the endpoint configuration lives)
	USB.EPPTR = (word_t)usbEndpointTableP->usbEndpointP;
	
	//load the USB calibration data from the device production row into the USB calibration registers
	USB.CAL0 = ReadCalibrationByte(USB_CAL0);
	USB.CAL1 = ReadCalibrationByte(USB_CAL1);
	
	// set the PLL as the USB clock source, enable the USB clock source (start feeding the USB module clock signals)
	CLK.USBCTRL = CLK_USBSRC_PLL_gc | CLK_USBSEN_bm;
	
	return true;
}

ISR(USB_BUSEVENT_vect)
{
	
}

ISR(USB_TRNCOMPL_vect)
{
	
}
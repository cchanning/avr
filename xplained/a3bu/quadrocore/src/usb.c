#include "quadrocore.h"

bool USBModuleInit(const USBConfiguration_t *usbConfigurationP)
{
	if (! USBEndpointTableAlloc(&usbConfigurationP->usbEndpointTableConfiguration))
	{
		return false;	
	}
		
	//set the USB module EPPTR register (so it will know where the endpoint table configuration lives)
	USB.EPPTR = ((uint16_t)USBEndpointTableGet()->usbEndpointP);
	
	//load the USB calibration data from the device production row into the USB calibration registers
	NVM.CMD = NVM_CMD_READ_CALIB_ROW_gc; 
	USB.CAL0 = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, USBCAL0));
	USB.CAL1 = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, USBCAL1));
	NVM.CMD = NVM_CMD_NO_OPERATION_gc; 
	
	USB.INTCTRLA = usbConfigurationP->usbInterruptLevel | USB_BUSEVIE_bm;
	USB.INTCTRLB = USB_TRNIE_bm | USB_SETUPIE_bm;
	
	USB.STATUS = 0;
	
	// set the PLL as the USB clock source, enable the USB clock source (start feeding the USB module clock signals)
	CLK.USBCTRL = CLK_USBSRC_PLL_gc | CLK_USBSEN_bm;
	
	USB.CTRLA = USB_ENABLE_bm | USB_SPEED_bm | USB_FIFOEN_bm | usbConfigurationP->usbEndpointTableConfiguration.endpointCount;
	
	// attach the USB module to the bus (allows the host to start the enumeration process)
	USB.CTRLB = USB_ATTACH_bm;
	
	return true;
}
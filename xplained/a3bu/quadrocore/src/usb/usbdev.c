#include "quadrocore.h"

void USBDeviceReset(void)
{	
	USBEndpointResetAll();
	USBDeviceSetAddress(NULL, NULL, NULL);
	USB.STATUS ^= USB_BUSRST_bm;
}

void USBDeviceGetDescriptor(SetupRequestDescriptor_t *setupRequestDescriptorP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP)
{
	PORTR.DIR = 0xFF;
	PORTR.OUTSET = 0x00;
}

void USBDeviceSetAddress(SetupRequestDescriptor_t *setupRequestDescriptorP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP)
{
	if (! setupRequestDescriptorP)
	{
		return;
	}
	
	PORTR.DIR = 0xFF;
	PORTR.OUTSET = 0x01;	
}
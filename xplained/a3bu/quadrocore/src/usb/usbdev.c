#include "quadrocore.h"

void USBDeviceReset(void)
{	
	USBEndpointResetAll();
	USBDeviceSetAddress(NULL, NULL, NULL);
	USB.STATUS ^= USB_BUSRST_bm;
}

void USBDeviceGetDescriptor(SetupRequestDescriptor_t *setupRequestDescriptorP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP)
{

}

void USBDeviceSetAddress(SetupRequestDescriptor_t *setupRequestDescriptorP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP)
{
	
}
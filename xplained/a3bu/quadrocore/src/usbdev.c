#include "quadrocore.h"

void USBDeviceReset(void)
{	
	USBEndpointResetAll();
	USBDeviceSetAddress(NULL, NULL, NULL);
}

void USBDeviceGetDescriptor(SetupRequestDescriptor_t *setupRequestDescriptorP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP)
{

}

void USBDeviceSetAddress(SetupRequestDescriptor_t *setupRequestDescriptorP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP)
{
	
}
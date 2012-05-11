#include "quadrocore.h"

void USBDeviceReset(void)
{	
	USBEndpointResetAll();
	USBDeviceSetAddress(NULL, NULL, NULL, NULL);
	USB.STATUS ^= USB_BUSRST_bm;
}

void USBDeviceGetDescriptor(USBRequest_t *usbRequestP, USBResponse_t *usbResponseP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP)
{
	USBDeviceDescriptor_t *usbDeviceDescriptorP = (USBDeviceDescriptor_t *)usbEndpointInP->dataBufferP;	
	
	usbDeviceDescriptorP->length = sizeof(USBStandardDeviceRequest_t);
	usbDeviceDescriptorP->descriptorType = DEVICE;
	usbDeviceDescriptorP->deviceClass = 0x00;
	usbDeviceDescriptorP->deviceSubClass = 0x00;
	usbDeviceDescriptorP->deviceProtocol = 0x00;
	usbDeviceDescriptorP->maxPacketSize = USBEndpointTableGet()->usbEndpointTableConfigurationP->maxPacketSize;
	usbDeviceDescriptorP->vendorId = 0x03EB;
	usbDeviceDescriptorP->productId = 0x2FE2;
	usbDeviceDescriptorP->deviceVersion = 0x0002;
	usbDeviceDescriptorP->manufacturerIndex = 0x01;
	usbDeviceDescriptorP->productIndex = 0x01;
	usbDeviceDescriptorP->serialNumberIndex = 0x00;
	usbDeviceDescriptorP->numberOfConfigurations = 0x01;
	usbDeviceDescriptorP->usbVersion = 0x0200;
	
	usbResponseP->byteCount = usbDeviceDescriptorP->length;
}

void USBDeviceSetAddress(USBRequest_t *usbRequestP, USBResponse_t *usbResponseP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP)
{	
	usbResponseP->byteCount = 0;
}
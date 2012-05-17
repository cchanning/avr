#include "quadrocore.h"

void USBDeviceReset(void)
{
	USBEndpointResetAll();
	USBDeviceSetAddress(NULL, NULL, NULL);
	USB.STATUS ^= USB_BUSRST_bm;
}

void USBDeviceGetDescriptor(USBStandardRequest_t *usbStandardRequestP, USBResponse_t *usbResponseP, USBTransfer_t *usbTransferP)
{
	USBStandardDeviceDescriptor_t *usbStandardDeviceDescriptorP = (USBStandardDeviceDescriptor_t *)usbTransferP->usbEndpointInP->dataBufferP;
	
	usbStandardDeviceDescriptorP->length = sizeof(USBStandardDeviceRequest_t);
	usbStandardDeviceDescriptorP->descriptorType = DEVICE;
	usbStandardDeviceDescriptorP->deviceClass = 0x00;
	usbStandardDeviceDescriptorP->deviceSubClass = 0x00;
	usbStandardDeviceDescriptorP->deviceProtocol = 0x00;
	usbStandardDeviceDescriptorP->maxPacketSize = USBEndpointTableGet()->usbEndpointTableConfigurationP->maxPacketSize;
	usbStandardDeviceDescriptorP->vendorId = 0x03EB;
	usbStandardDeviceDescriptorP->productId = 0x2FE2;
	usbStandardDeviceDescriptorP->deviceVersion = 0x0002;
	usbStandardDeviceDescriptorP->manufacturerIndex = 0x01;
	usbStandardDeviceDescriptorP->productIndex = 0x01;
	usbStandardDeviceDescriptorP->serialNumberIndex = 0x00;
	usbStandardDeviceDescriptorP->numberOfConfigurations = 0x01;
	usbStandardDeviceDescriptorP->usbVersion = 0x0200;
	
	usbResponseP->byteCount = usbStandardDeviceDescriptorP->length;
}

void USBDeviceSetAddress(USBStandardRequest_t *usbStandardRequestP, USBResponse_t *usbResponseP, USBTransfer_t *usbTransferP)
{
	usbResponseP->byteCount = 0;
}
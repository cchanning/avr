#ifndef USBDEV_H_
#define USBDEV_H_

#include "usbreq.h"

typedef struct _USBDeviceDescriptor
{
	uint8_t length;
	uint8_t descriptorType;
	uint16_t usbVersion;
	uint8_t deviceClass;
	uint8_t deviceSubClass;
	uint8_t deviceProtocol;
	uint8_t maxPacketSize;
	uint16_t vendorId;
	uint16_t productId;
	uint16_t deviceVersion;
	uint8_t manufacturerIndex;
	uint8_t productIndex;
	uint8_t serialNumberIndex;
	uint8_t numberOfConfigurations;
} USBDeviceDescriptor_t;

void USBDeviceReset(void);
void USBDeviceGetDescriptor(USBRequest_t *usbRequestP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP);
void USBDeviceSetAddress(USBRequest_t *usbRequestP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP);


#endif /* USBDEV_H_ */
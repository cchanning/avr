#ifndef USBSTDDREQ_H_
#define USBSTDDREQ_H_

#include "usb/io/usbstdreq.h"

typedef struct _USBStandardDeviceRequest
{
	uint8_t requestType;
	uint8_t request;
	uint16_t value;
	uint16_t index;
	uint16_t length;
} USBStandardDeviceRequest_t;

typedef struct _USBStandardDeviceDescriptor
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
} USBStandardDeviceDescriptor_t;

void USBDeviceReset(void);
void USBDeviceGetDescriptor(USBStandardRequest_t *usbStandardRequestP, USBResponse_t *usbResponseP, USBTransfer_t *usbTransferP);
void USBDeviceSetAddress(USBStandardRequest_t *usbStandardRequestP, USBResponse_t *usbResponseP, USBTransfer_t *usbTransferP);


#endif /* USBSTDDREQ_H_ */
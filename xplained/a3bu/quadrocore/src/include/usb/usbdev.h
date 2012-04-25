#ifndef USBDEV_H_
#define USBDEV_H_

#include "usbreq.h"

void USBDeviceReset(void);
void USBDeviceGetDescriptor(SetupRequestDescriptor_t *setupRequestDescriptorP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP);
void USBDeviceSetAddress(SetupRequestDescriptor_t *setupRequestDescriptorP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP);


#endif /* USBDEV_H_ */
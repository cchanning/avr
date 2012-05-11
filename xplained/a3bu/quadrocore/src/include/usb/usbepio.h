#ifndef USBEPIO_H_
#define USBEPIO_H_

#include "usbep.h"

void USBEndpointTransmit(USBEndpoint_t *usbEndpointP, size_t byteCount);
void USBEndpointAcknowledge(USBEndpoint_t *usbEndpointP);


#endif /* USBEPIO_H_ */
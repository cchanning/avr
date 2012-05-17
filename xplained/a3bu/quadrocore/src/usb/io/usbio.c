#include "quadrocore.h"

void USBEndpointTransmit(USBEndpoint_t *usbEndpointP, size_t byteCount)
{
	usbEndpointP->auxData = 0;
	usbEndpointP->cnt = byteCount;
	usbEndpointP->status &= ~(1 << 1);;
}
#ifndef USB_H_
#define USB_H_

#include "type.h"

typedef struct struct_USBEnpoint
{
	byte_t status;
	byte_t ctrl;
	word_t cnt;
	byte_t *dataP;
	word_t auxData;
	
} USBEndpoint_t;

typedef struct struct_USBEnpointTable
{
	USBEndpoint_t *usbEndpointP;
	uint8_t endpointCount;
} USBEndpointTable_t;

USBEndpointTable_t* USBEndpointTableAlloc(short endpointCount, short endpointBufferSize);

#endif /* USB_H_ */
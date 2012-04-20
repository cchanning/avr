#ifndef USBEP_H_
#define USBEP_H_

typedef struct _USBEndpoint
{
	byte_t status;
	byte_t ctrl;
	word_t cnt;
	byte_t *dataP;
	word_t auxData;
	
} USBEndpoint_t;

typedef struct _USBEndpointTable
{
	USBEndpoint_t *usbEndpointP;
	uint8_t endpointCount;
} USBEndpointTable_t;

typedef enum _EndpointDirection
{
	OUT,
	IN
} EndpointDirection;

USBEndpointTable_t* USBEndpointTableAlloc(short endpointCount, uint8_t endpointBufferSize);

USBEndpoint_t* USBEndpointGet(USBEndpointTable_t *usbEndpointTableP, uint8_t endpointNumber, EndpointDirection endpointDirection);


#endif /* USBEP_H_ */
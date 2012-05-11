#ifndef USBEP_H_
#define USBEP_H_

typedef struct _USBEndpoint
{
	volatile uint8_t status;
	volatile uint8_t ctrl;
	volatile uint16_t cnt;
	volatile void *dataBufferP;
	volatile uint16_t auxData;
	
} USBEndpoint_t;

typedef struct _USBEndpointConfiguration
{
	uint8_t type;
	uint8_t bufferSize;
	uint8_t bufferType;
} USBEndpointConfiguration_t;

typedef struct _USBEndpointTableConfiguration
{
	uint8_t endpointCount;
	uint8_t maxPacketSize;
	USBEndpointConfiguration_t endpointConfiguration[1];
} USBEndpointTableConfiguration_t;

typedef struct _USBEndpointTable
{
	/*
		pointer to block of memory used for FIFO and the endpoint table. This should never be used directly
		except to free it.
	 */
	void *baseP;
	
	/*
		pointer to the block of memory used as the endpoint FIFO. Note that it will never to be used in code,
		it just serves as a reminder that the USB module expects this memory to be allocated.  
	 */ 
	void *fifoP;
	
	/* 
		pointer to the endpoint table memory used to hold the USB endpoint configuration etc.
	 */
	USBEndpoint_t *usbEndpointP;
	
	const USBEndpointTableConfiguration_t const *usbEndpointTableConfigurationP;

} USBEndpointTable_t;

typedef enum _EndpointDirection
{
	OUT,
	IN
} EndpointDirection;


bool USBEndpointTableAlloc(const USBEndpointTableConfiguration_t const *usbEndpointTableConfigurationP);

USBEndpointTable_t* USBEndpointTableGet(void);
USBEndpoint_t* USBEndpointGet(uint8_t endpointNumber, EndpointDirection endpointDirection);
USBEndpoint_t* USBEndpointGetDefault(EndpointDirection endpointDirection);
USBEndpoint_t* USBEndpointTxQueueGetNext(void);

void USBEndpointReset(USBEndpoint_t *usbEndpointP, const USBEndpointConfiguration_t const *usbEndpointConfigurationP);
void USBEndpointResetAll(void);
void USBEndpointAcknowledge(USBEndpoint_t *usbEndpointP);

#endif /* USBEP_H_ */
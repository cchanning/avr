#ifndef USBIO_H_
#define USBIO_H_

#include "type/type.h"
#include "usb/io/usbep.h"

typedef struct _USBTransfer
{
	uint8_t maxTransactionCount;
	uint8_t transactionCount;
	USBEndpoint_t *usbEndpointInP;
	USBEndpoint_t *usbEndpointOutP;
	CALLBACK_FUNC *callbackFuncP;
	void *callbackDataP;
	
} USBTransfer_t;

typedef struct _USBTransferTable
{
	uint8_t maxCount;
	uint8_t count;
	uint8_t incUnit;
	USBTransfer_t *usbTransferP;
} USBTransferTable_t;

USBTransfer_t* USBTransferAlloc(void);
void USBTransferFree(USBTransfer_t *usbTransferP);

USBTransferTable_t* USBTransferTableAlloc(uint8_t incUnit);
USBTransferTable_t* USBTransferTableFree(USBTransferTable_t *usbTransferTableP);
USBTransfer_t* USBTransferTableAdd(USBTransferTable_t *usbTransferTableP);
void USBTransferTableRemove(USBTransferTable_t *usbTransferTableP, USBTransfer_t *usbTransferP);

USBTransfer_t* USBGetTransfer(USBEndpoint_t *usbEndpointP);
USBTransfer_t USBBeginTransfer(USBEndpoint_t *usbEndpointP);
void USBEndTransfer(USBTransfer_t *usbTransferP);

void USBEndpointTransmit(USBEndpoint_t *usbEndpointP, size_t byteCount);

#endif /* USBIO_H_ */
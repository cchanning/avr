#ifndef USBREQ_H_
#define USBREQ_H_

#define USB_REQUEST_TYPE_FLD_TYPE_bm 0x60
#define USB_REQUEST_TYPE_FLD_DPTFD_HD_bm (0 << 7)
#define USB_REQUEST_TYPE_FLD_DPTFD_DH_bm (1 << 7)
#define USB_REQUEST_TYPE_FLD_TYPE_STANDARD_bm (0 << 6)
#define USB_REQUEST_TYPE_FLD_TYPE_CLASS_bm (1 << 6)
#define USB_REQUEST_TYPE_FLD_TYPE_VENDOR_bm (2 << 5)
#define USB_REQUEST_TYPE_FLD_TYPE_RESERVED_bm (3 << 5)
#define USB_REQUEST_TYPE_FLD_RECIPIENT_bm 0x1F
#define USB_REQUEST_TYPE_FLD_RECIPIENT_DEVICE_bm (0 << 4)
#define USB_REQUEST_TYPE_FLD_RECIPIENT_INTERFACE_bm (1 << 4)
#define USB_REQUEST_TYPE_FLD_RECIPIENT_ENDPOINT_bm (2 << 3)
#define USB_REQUEST_TYPE_FLD_RECIPIENT_OTHER_bm (3 << 3)

#define USB_REQUEST_DEVICE_GET_DESCRIPTOR 0x06
#define USB_REQUEST_DEVICE_SET_ADDRESS 0x05

#define USB_REQUEST_TYPE_HANDLER_COUNT 2

typedef struct _SetupRequestDescriptor
{
	uint8_t requestType;
	uint8_t request;
} SetupRequestDescriptor_t;

typedef void (*SETUP_REQUEST_HANDLER_FUNC)(SetupRequestDescriptor_t *setupRequestDescriptorP, USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP);

typedef struct _SetupRequestHandler
{
	uint8_t type;
	uint8_t recipient;
	uint8_t id;
	SETUP_REQUEST_HANDLER_FUNC handlerFuncP;
} SetupRequestHandler_t;

void ProcessSetupRequest(USBEndpoint_t *usbEndpointOutP, USBEndpoint_t *usbEndpointInP);


#endif /* USBREQ_H_ */
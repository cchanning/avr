#ifndef USB_H_
#define USB_H_

#include "usb/io/usbep.h"
#include "usb/io/usbep.h"
#include "usb/io/usbio.h"
#include "usb/io/usbstdreq.h"
#include "usb/io/usbstddreq.h"
#include "type/type.h"

typedef struct _USBConfiguration
{
	USBEndpointTableConfiguration_t usbEndpointTableConfiguration;
} USBConfiguration_t;

bool_t USBModuleInit(const USBConfiguration_t *usbConfigurationP);

#endif /* USB_H_ */
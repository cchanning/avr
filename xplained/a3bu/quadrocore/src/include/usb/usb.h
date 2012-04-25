#ifndef USB_H_
#define USB_H_

#include "usb/usbep.h"
#include "type.h"

typedef struct _USBConfiguration
{
	uint8_t usbInterruptLevel;
	USBEndpointTableConfiguration_t usbEndpointTableConfiguration;
} USBConfiguration_t;

bool USBModuleInit(const USBConfiguration_t *usbConfigurationP);

#endif /* USB_H_ */
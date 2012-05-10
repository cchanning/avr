#include "quadrocore.h"

int main(void)
{
	const USBConfiguration_t const usbConfiguration = 
	{
		.usbEndpointTableConfiguration = 
		{
			.endpointCount = 1,
			
			// default endpoint configuration
			.endpointConfiguration[0] = 
			{
				.type = USB_EP_TYPE_CONTROL_gc,
				.bufferSize = 32,
				.bufferType = USB_EP_BUFSIZE_32_gc
			}
		}
	};
	
	DisableGlobalInterrupts();
	{
		SystemClockInit();
		PMICInit(PMIC_HILVLEN_bm);
		USBModuleInit(&usbConfiguration);		
	}
	EnableGlobalInterrupts();
	
	for ( ; ; )
	{
		// keep looping forever (we are interrupt driven)
	}
	
	return 0;
}
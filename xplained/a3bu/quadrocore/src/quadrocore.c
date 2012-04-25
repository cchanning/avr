#include "quadrocore.h"

int main(void)
{
	const USBConfiguration_t const usbConfiguration = 
	{
		.usbInterruptLevel = USB_INTLVL_HI_gc,
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
	
	/*
		Configure the system clock to be 48mhz for the PLL and set the CPU/RAM/etc to run at 24mhz
	 */
	SystemClockInit();
	USBModuleInit(&usbConfiguration);
	PMICInit(PMIC_HILVLEN_bm);
	EnableGlobalInterrupts();
	
	for ( ; ; );
	
	return 0;
}
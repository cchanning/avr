#include "quadrocore.h"

int main(void)
{
	USBConfiguration_t usbConfiguration = 
	{
		.usbEndpointTableConfiguration = 
		{
			.endpointCount = 1,
			.endpointBufferSize = 32,
			.endpointType[0] = USB_EP_TYPE_CONTROL_gc
		}
	};
	
	SystemClockInit();
	USBModuleInit(&usbConfiguration);
	SetPMICLevel(PMIC_HILVLEN_bm);
	EnableGlobalInterrupts();
	
	for ( ; ; );
	
	return 0;
}
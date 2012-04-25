#include "quadrocore.h"

ISR(USB_BUSEVENT_vect)
{
	DisableGlobalInterrupts();
	{		
		if (USB.INTFLAGSASET & USB_RSTIF_bm)
		{
			USBDeviceReset();
			
			USB.INTFLAGSACLR = USB_RSTIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_SUSPENDIF_bm)
		{
			USB.INTFLAGSACLR = USB_SUSPENDIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_RESUMEIF_bm)
		{
			USB.INTFLAGSACLR = USB_RESUMEIF_bm;
		}
	}	
	EnableGlobalInterrupts();
}

ISR(USB_TRNCOMPL_vect)
{
	DisableGlobalInterrupts();
	{		
		PORTR.DIR = 0xFF;
		PORTR.OUTSET = 0x00;
			
		if (USB.INTFLAGSBSET & USB_SETUPIF_bm)
		{
			//for now assume the only endpoint that can handle SETUP packets is the default
			ProcessSetupRequest(USBEndpointGetDefault(OUT), USBEndpointGetDefault(IN));
			USB.INTFLAGSBCLR = USB_SETUPIF_bm;	
		}
		
		if (USB.INTFLAGSBSET & USB_TRNIF_bm)
		{
			USB.INTFLAGSBCLR = USB_TRNIF_bm;
		}
	}	
	EnableGlobalInterrupts();	
}
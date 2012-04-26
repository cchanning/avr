#include "quadrocore.h"

ISR(USB_BUSEVENT_vect)
{
	DisableGlobalInterrupts();
	{		
		if (USB.INTFLAGSASET & USB_SOFIF_bm)
		{
			USB.INTFLAGSACLR = USB_SOFIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_SUSPENDIF_bm)
		{
			USB.INTFLAGSACLR = USB_SUSPENDIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_RESUMEIF_bm)
		{
			USB.INTFLAGSACLR = USB_RESUMEIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_RSTIF_bm)
		{
			USBDeviceReset();
			USB.INTFLAGSACLR = USB_RSTIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_CRCIF_bm)
		{
			USB.INTFLAGSACLR = USB_CRCIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_UNFIF_bm)
		{
			USB.INTFLAGSACLR = USB_UNFIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_OVFIF_bm)
		{
			USB.INTFLAGSACLR = USB_OVFIF_bm;
		}
		
		if (USB.INTFLAGSASET & USB_STALLIF_bm)
		{
			USB.INTFLAGSACLR = USB_STALLIF_bm;
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
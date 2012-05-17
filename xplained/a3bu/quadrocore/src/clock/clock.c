#include "quadrocore.h"

bool R32OscillatorInit(void)
{
	//switch on the RC32Mhz internal oscillator
	OSC.CTRL |= OSC_RC32MEN_bm;
	
	while (OSC_RC32MRDY_bm != (OSC.STATUS & OSC_RC32MRDY_bm))
	{
		//wait until the oscillator has stabilized
	}
	
	
	return true;
}

 /**
  * Configures the PLL, this requires the 32Mhz internal oscillator to be the reference input, note that the hardware will automatically scale down the input
  * to the PLL to 8mhz (div 4) before we even get to touch it. The PLL multiplication factor is set at 6 to scale the required clock frequency up to 48mhz. 
  * This will allow the PLL to be used as a clock source for the USB.
  */
bool PLLInit(void)
{
	if (! R32OscillatorInit())
	{
		return false;
	}
	
	// set the PLL source as the R32 internal osc. and set the multiplier as 6 (scales up to 48mhz for the USB module)
	OSC.PLLCTRL |= OSC_PLLSRC_RC32M_gc | 0x06;
	
	//enable the PLL
	OSC.CTRL |= OSC_PLLEN_bm;
	
	while (OSC_PLLRDY_bm != (OSC.STATUS & OSC_PLLRDY_bm))
	{
		//wait until the PLL has stabilized	
	}
	
	return true;
}

bool SystemClockInit(void)
{
	if (! PLLInit())
	{
		return false;
	}
	
	// set prescaler A to div by 2, disable prescalers C/B (CPU etc will run at 24Mhz)
	SetProtectedMemory(&CLK.PSCTRL, 0b00000100);
	
	// set the clock source as PLL
	SetProtectedMemory(&CLK.CTRL, 0b00000100);
}
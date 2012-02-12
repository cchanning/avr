/**
 *
 * Notes:
 *				- We can only have 16k of memory to play with, it starts at 0x2000 and finishes at 0x5FFF
				
				- Each endpoint address consists of both out and in endpoints. Each endpoint needs 8 bytes of memory as well as its own 
					data buffer in RAM. Endpoint 0 is first, and so the first 8 bytes is for EP0 OUT and the next 8 bytes is for EP0 IN.
					So just remember to get to the next endpoint address, you'll need to add 16 to the last endpoint address. Or better
					still use this formula >> ep_base_ram_addr + (ep_number (e.g. 0/1/2/3 etc) * 16)

				- when a setup token comes in for EP 0, our data ptr will already be configured to point at our buffer containing the
					harcoded device information. This needs to be ready before we switch on the USB module.

 */

.cseg

#define	TEMP	R26
#define TEMP1	R27
#define	TEMP2	R28
#define	TEMP3	R29

.equ	ENDPOINT_COUNT					=	0x02
.equ	USB_EPPTRL						=	USB_EPPTR
.equ	USB_EPPTRH						=	USB_EPPTR + 1

.equ	ENDPOINT_BASE					=	SRAM_START + ((ENDPOINT_COUNT + 1) * 4)
.equ	ENDPOINT_DATA_BUFFER_SIZE		=	8
.equ	ENDPOINT_DATA_BUFFER_BASE		=	ENDPOINT_BASE + ((ENDPOINT_COUNT + 1) * 16) + 2	; assume we have FIFO and frame number tracking on

// begin: configure endpoint 0 (in/out)
.equ	ENDPOINT_0_OUT_BASE				=	ENDPOINT_BASE
.equ	ENDPOINT_0_OUT_DATA_BUFFER_BASE	=	ENDPOINT_DATA_BUFFER_BASE
.equ	ENDPOINT_0_OUT_STATUS			=	ENDPOINT_0_OUT_BASE
.equ	ENDPOINT_0_OUT_CTRL				=	ENDPOINT_0_OUT_BASE + 1
.equ	ENDPOINT_0_OUT_CNTL				=	ENDPOINT_0_OUT_BASE + 2
.equ	ENDPOINT_0_OUT_CNTH				=	ENDPOINT_0_OUT_BASE + 3
.equ	ENDPOINT_0_OUT_DATAPTRL			=	ENDPOINT_0_OUT_BASE + 4
.equ	ENDPOINT_0_OUT_DATAPTRH			=	ENDPOINT_0_OUT_BASE + 5
.equ	ENDPOINT_0_OUT_AUXDATAL			=	ENDPOINT_0_OUT_BASE + 6
.equ	ENDPOINT_0_OUT_AUXDATAH			=	ENDPOINT_0_OUT_BASE + 7

.equ	ENDPOINT_0_IN_BASE				=	ENDPOINT_BASE + 8
.equ	ENDPOINT_0_IN_DATA_BUFFER_BASE	=	ENDPOINT_DATA_BUFFER_BASE + ENDPOINT_DATA_BUFFER_SIZE
.equ	ENDPOINT_0_IN_STATUS			=	ENDPOINT_0_IN_BASE
.equ	ENDPOINT_0_IN_CTRL				=	ENDPOINT_0_IN_BASE + 1
.equ	ENDPOINT_0_IN_CNTL				=	ENDPOINT_0_IN_BASE + 2
.equ	ENDPOINT_0_IN_CNTH				=	ENDPOINT_0_IN_BASE + 3
.equ	ENDPOINT_0_IN_DATAPTRL			=	ENDPOINT_0_IN_BASE + 4
.equ	ENDPOINT_0_IN_DATAPTRH			=	ENDPOINT_0_IN_BASE + 5
.equ	ENDPOINT_0_IN_AUXDATAL			=	ENDPOINT_0_IN_BASE + 6
.equ	ENDPOINT_0_IN_AUXDATAH			=	ENDPOINT_0_IN_BASE + 7
// end: configure endpoint 0 (in/out)

.org 0x0000
	jmp reset
.org USB_TRNCOMPL_vect
	reti												; source = TRNIF (An IN/OUT transaction has completed)
	jmp isr_device_handle_setup_request					; source = SETUPIF (A SETUP transaction has completed)

reset:
	ldi R29, low(RAMEND)
	sts CPU_SPL, R29									; configure low byte of stack pointer
	ldi R29, high(RAMEND)
	sts CPU_SPH, R29									; configure high byte of stack pointer
	call configure_system_clock
	call configure_usb
	call enable_interrupts
	call enable_usb

	jmp main											; jump to the main program loop

main:
	jmp main

/****************************************************************************************
 * Clock Functions
 ****************************************************************************************/

/*
 * By default the system clock will be using the 2Mhz internal oscillator (after hardware reset). This function will cause the PLL to 
 * be configured as the system clock source. Prescaler A will be configured to reduce the output from the PLL to 3Mhz rather than 48Mhz.
 */
configure_system_clock:
	call configure_32mhz_int_osc						; make sure the 32Mhz internal oscillator is configured and is stable for use
	call configure_pll									; configure the PLL
	ldi TEMP, 0b00011100
	sts CLK_PSCTRL, TEMP								; set prescaler A to div by 16, disable prescalers C/B (CPU etc will run at 3Mhz)
	ldi TEMP, 0b00000100
	sts CLK_CTRL, TEMP									; set the clock source as PLL
	ret

/*
 * Switch on the 32mhz internal oscillator and wait for it to become stable before returning.
 */
configure_32mhz_int_osc:
	lds	TEMP1, OSC_CTRL									; load up the existing oscillator configuration (by default the 2mhz oscillator should be enabled)
	ldi TEMP2, 0b00000010
	or TEMP1, TEMP2										; this will make sure we keep any previously enabled oscillators alive :)
	sts OSC_CTRL, TEMP1									; enable the 32mhz internal oscillator

	IOSC_NOT_READY:										; loop until the hardware has told us the 32mhz internal oscillator is stable
		ldi TEMP1, 0b00000010
		mov TEMP2, TEMP1
		lds TEMP3, OSC_STATUS
		and TEMP2, TEMP3
		cpse TEMP1, TEMP2								; when TEMP1 == TEMP2 the next instruction will be skipped, causing "ret" to be the next instruction to be executed
		jmp IOSC_NOT_READY
	ret

 /**
  * Configures the PLL, this requires the 32Mhz internal oscillator to be the reference input, note that the hardware will automatically scale down the input
  * from the PLL to 8mhz (div 4) before we even get to touch it. The PLL multiplaction factor is set at 6 to scale the required clock frequency up to 48mhz. 
  * This will allow the PLL to be used as a clock source for the USB. Note that the 32mhz internal oscillator must be running and stable before calling this 
  * function.
  */
configure_pll:
	ldi TEMP, 0b10000110
	sts OSC_PLLCTRL, TEMP								; configure the PLL input clock source as the 32Mhz internal oscillator (scaled down automatically to 8mhz by hardware), set the PLL frequency multiplication factor to 6 to scale the PLL output up to 48mhz
	lds	TEMP1, OSC_CTRL									; copy the current oscillator configuration (we dont' want to accidentally turn off the 32mhz internal osscillator etc!)
	ldi TEMP2, 0b00010000								; load the bitmask that will enable the PLL
	or TEMP1, TEMP2
	sts OSC_CTRL, TEMP1									; enable the PLL (whilst preserving any previously enabled osscillators)
	
	PLL_NOT_READY:										; loop until the hardware has told us the PLL is stable
		ldi TEMP1, 0b00010000
		mov TEMP2, TEMP1
		lds TEMP3, OSC_STATUS
		and TEMP2, TEMP3
		cpse TEMP1, TEMP2								; when TEMP1 == TEMP2 the next instruction will be skipped, causing "ret" to be the next instruction to be executed
		jmp PLL_NOT_READY

	ret

/*
 * Enable the USB clock source as the PLL, note that the PLL must be configured and running before calling this function
 */
configure_usb_clock:
	ldi TEMP, 0b00000001
	sts CLK_USBCTRL, TEMP								; set the PLL as the USB clock source, enable the USB clock source (start feeding the USB module clock signals)	
	ret

/****************************************************************************************
 * General USB Functions
 ****************************************************************************************/

enable_interrupts:
	ldi TEMP, 0b00000100
	sts PMIC_CTRL, TEMP								; enable high level in PMIC
	ldi TEMP, 0b00000011
	sts USB_INTCTRLA, TEMP								; enable interrupts to fire for USB
	ldi TEMP, 0b00000001								
	sts USB_INTCTRLB, TEMP								; enable interrupts to fire when SETUP transactions complete on USB
	sei													; enable global interrupts

	ret

enable_usb:
	ldi TEMP1, 0b11110000								; enables the following usb port, full speed, FIFO, frame number tracking
	ldi TEMP2, ENDPOINT_COUNT
	or	TEMP1, TEMP2									; enable the required number of endpoints
	sts USB_CTRLA, TEMP1								; activate the USB port with the configuration
	ret

disable_usb:
	ldi TEMP, 0x00
	sts USB_CTRLA, TEMP
	ret

configure_usb:
	ldi TEMP, 0x00
	sts USB_ADDR, TEMP									; reset device address
	call configure_usb_clock
	call configure_usb_endpoints
	ret

/****************************************************************************************
 * USB Endpoint Functions
 ****************************************************************************************/
configure_usb_endpoints:
	ldi TEMP, low(SRAM_START)
	sts USB_EPPTRL, TEMP								; configure low byte of endpoint table pointer
	ldi TEMP, high(SRAM_START)
	sts USB_EPPTRH, TEMP								; configure high byte of endpoint table pointer

	ldi	TEMP, 0b01000000;		
	sts ENDPOINT_0_OUT_CTRL, TEMP						; configure endpoint 0 (out) be a control type with interrupts enabled for FIFO
	ldi TEMP, low(ENDPOINT_0_OUT_DATA_BUFFER_BASE)
	sts ENDPOINT_0_OUT_DATAPTRL, TEMP					; configure the dataptr low byte for EP 0 out
	ldi TEMP, high(ENDPOINT_0_OUT_DATA_BUFFER_BASE)
	sts ENDPOINT_0_OUT_DATAPTRH, TEMP					; configure the dataptr high byte for EP 0 out
	ldi TEMP, 0x00
	sts ENDPOINT_0_OUT_STATUS, TEMP					; reset EP 0 out status byte
	sts ENDPOINT_0_OUT_CNTL, TEMP						; reset EP 0 out count low byte
	sts ENDPOINT_0_OUT_CNTH, TEMP						; reset EP 0 out count high byte
	sts ENDPOINT_0_OUT_AUXDATAL, TEMP					; reset EP 0 out auxdata low byte
	sts ENDPOINT_0_OUT_AUXDATAH, TEMP					; reset EP 0 out auxdata high byte

	ldi	TEMP, 0b01000000;		
	sts ENDPOINT_0_IN_CTRL, TEMP						; configure endpoint 0 (in) be a control type with interrupts enabled for FIFO
	ldi TEMP, low(ENDPOINT_0_IN_DATA_BUFFER_BASE)
	sts ENDPOINT_0_IN_DATAPTRL, TEMP					; configure the dataptr low byte for EP 0 in
	ldi TEMP, high(ENDPOINT_0_IN_DATA_BUFFER_BASE)
	sts ENDPOINT_0_IN_DATAPTRH, TEMP					; configure the dataptr high byte for EP 0 in
	ldi TEMP, 0x00
	sts ENDPOINT_0_IN_STATUS, TEMP						; reset EP 0 out status byte
	sts ENDPOINT_0_IN_CNTL, TEMP						; reset EP 0 in count low byte
	sts ENDPOINT_0_IN_CNTH, TEMP						; reset EP 0 in count high byte
	sts ENDPOINT_0_IN_AUXDATAL, TEMP					; reset EP 0 in auxdata low byte
	sts ENDPOINT_0_IN_AUXDATAH, TEMP					; reset EP 0 in auxdata high byte
	ret

/****************************************************************************************
 * USB Request Token Handling Functions e.g. SETUP
 ****************************************************************************************/

isr_device_handle_setup_request:
	reti

device_clear_feature:
	ret

device_get_configuration:
	ret

device_get_descriptor:
	ret

device_get_interface:
	ret

device_get_status:
	ret

device_set_address:
	ret

device_set_configuration:
	ret

device_set_descriptor:
	ret

device_set_feature:
	ret

device_set_interface:
	ret

device_sync_frame:
	ret
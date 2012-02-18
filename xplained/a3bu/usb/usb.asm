/*********************************************************************************************
 * 
 * Atmel ATXMEGA256A3BU USB Firmware
 *
 * Author: Chris Channing
 * 
 *********************************************************************************************
 *
 *
 *	+--------------------------------------+
 *	+ Application Stack
 *	+--------------------------------------+
 *	+ Endpoint Configuration Table
 *	+--------------------------------------+
 *	+ Endpoint Data Heap
 *	+--------------------------------------+
 *
 *
 *********************************************************************************************/
 
 .include "ATxmega256A3BUdef.inc"

 /*********************************************************************************************
 * Register Aliases
 *********************************************************************************************/

.def TEMP = R24
.def TEMP1 = R23
.def TEMP2 = R22
.def TEMP3 = R21
.def TEMP4 = R20

 /*********************************************************************************************
 * Main
 *********************************************************************************************/

.cseg
.org 0x0000
	jmp reset
.org USB_TRNCOMPL_vect
	reti										; source = TRNIF (An IN/OUT transaction has completed)
	jmp isr_device_handle_setup_request						; source = SETUPIF (A SETUP transaction has completed)

reset:
	ldi TEMP, low(RAMEND)
	sts CPU_SPL, TEMP								; configure low byte of CPU stack pointer
	ldi TEMP, high(RAMEND)
	sts CPU_SPH, TEMP								; configure high byte of CPU stack pointer
	ldi ZL, low(APPLICATION_STACK_START)						; configure low byte of application stack pointer
	ldi ZH, high(APPLICATION_STACK_START)						; configure high byte of application stack pointer
	sbiw Z, 1									; cause the SP to be one before the stack start address, this is required as a pusha will first increment Z. In the first usage scenario it will make sure the data is placed at the stack start address.	

	call configure_system_clock
	call configure_usb
	call enable_interrupts
	call enable_usb

	jmp main									; jump to the main program loop

main:
	jmp main									; keep the CPU busy forever

/*********************************************************************************************
 * Application Stack
 *********************************************************************************************/

.equ APPLICATION_STACK_START = SRAM_START
.equ APPLICATION_STACK_SIZE = 16

.macro pusha
	adiw Z, 1
	ldi TEMP, @0
	st Z, TEMP
.endm

.macro popa
	ld @0, Z
	ldi R16, 0
	st Z, R16
	sbiw Z, 1
.endm


/*********************************************************************************************
 * USB Endpoint
 *********************************************************************************************/

.equ ENDPOINT_COUNT = 2
.equ ENDPOINT_DATA_BUFFER_SIZE = 32
.equ ENDPOINT_CFG_TBL_START = APPLICATION_STACK_START + APPLICATION_STACK_SIZE
.equ ENDPOINT_START = ENDPOINT_CFG_TBL_START + ((ENDPOINT_COUNT + 1) * 4)
.equ ENDPOINT_OFFSET_STATUS = 0
.equ ENDPOINT_OFFSET_CTRL = 1
.equ ENDPOINT_OFFSET_CNTL = 2
.equ ENDPOINT_OFFSET_CNTH = 3
.equ ENDPOINT_OFFSET_DATAPTRL = 4
.equ ENDPOINT_OFFSET_DATAPTRH = 5
.equ ENDPOINT_OFFSET_AUXDATAL = 6
.equ ENDPOINT_OFFSET_AUXDATAH = 7


/*
 * This macro will calculate the pointer to the start of the output portion of endpoint and place the pointer in Y
 *
 * Parameter 0 - the endpoint number ranging from 0 - n
 */
.macro calculateOutputEndpointPtr
		ldi YL, low(ENDPOINT_START)						; configure low byte of pointer to the first endpoint
		ldi YH, high(ENDPOINT_START)					; configure high byte of pointer to the first endpoint
		mov R19, @0
		ldi R18, 16
		mul R19, R18									; MUL saves result in R1:R0 where R1 == HIGH and R0 == low
		add YL, R0
		adc YH, R1
.endm

/*
 * This macro will calculate the pointer to the start of the input portion of the endpoint and place the pointer in Y
 *
 * Parameter 0 - the endpoint number ranging from 0 - n
 */
.macro calculateInputEndpointPtr
	calculateOutputEndpointPtr @0						; configure Y to point at the output portion of the endpoint
	adiw Y, 8											; the input portion of the endpoint is always 8 bytes from the start of the output portion
.endm

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
	ldi TEMP, 0b00001100
	sts CLK_PSCTRL, TEMP								; set prescaler A to div by 4, disable prescalers C/B (CPU etc will run at 12Mhz)
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
	ldi TEMP, low(ENDPOINT_CFG_TBL_START)
	sts USB_EPPTR, TEMP									; configure low byte of endpoint config table pointer
	ldi TEMP, high(ENDPOINT_CFG_TBL_START)
	sts USB_EPPTR + 1, TEMP								; configure high byte of endpoint config table pointer

	pusha 0b01000000									; push endpoint config byte, control type with interrupts enabled for FIFO
	pusha 0b10000000									; push endpoint config byte, bulk type with interrupts enabled for FIFO
	ldi TEMP1, 0

	//for each offset we need to reset Y to point at the start of the output endpoint, as the offsets will always be relevant to the start (Y + offset)
	ENDPOINT_OUTPUT_CONFIG_LOOP:
		eor TEMP2, TEMP2							
		calculateOutputEndpointPtr TEMP1					
		adiw Y, ENDPOINT_OFFSET_STATUS
		st Y, TEMP2
		calculateOutputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_CTRL
		popa TEMP2
		st Y, TEMP2
		eor TEMP2, TEMP2
		calculateOutputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_CNTL
		st Y, TEMP2
		calculateOutputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_CNTH
		st Y, TEMP2
		calculateOutputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_DATAPTRL
		st Y, TEMP2
		calculateOutputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_DATAPTRH
		st Y, TEMP2
		calculateOutputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_AUXDATAL
		st Y, TEMP2
		calculateOutputEndpointPtr TEMP1
		adiw Y, ENDPOINT_OFFSET_AUXDATAH
		st Y, TEMP2

		inc TEMP1
		ldi TEMP2, ENDPOINT_COUNT
		cpse TEMP1, TEMP2
		jmp ENDPOINT_OUTPUT_CONFIG_LOOP

	pusha 0b01000000									; push endpoint config byte, control type with interrupts enabled for FIFO
	pusha 0b10000000									; push endpoint config byte, bulk type with interrupts enabled for FIFO
	ldi TEMP1, 0

	//for each offset we need to reset Y to point at the start of the input endpoint, as the offsets will always be relevant to the start (Y + offset)
	ENDPOINT_INPUT_CONFIG_LOOP:
		eor TEMP2, TEMP2							
		calculateInputEndpointPtr TEMP1					
		adiw Y, ENDPOINT_OFFSET_STATUS
		st Y, TEMP2
		calculateInputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_CTRL
		popa TEMP2
		st Y, TEMP2
		eor TEMP2, TEMP2
		calculateInputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_CNTL
		st Y, TEMP2
		calculateInputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_CNTH
		st Y, TEMP2
		calculateInputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_DATAPTRL
		st Y, TEMP2
		calculateInputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_DATAPTRH
		st Y, TEMP2
		calculateInputEndpointPtr TEMP1						
		adiw Y, ENDPOINT_OFFSET_AUXDATAL
		st Y, TEMP2
		calculateInputEndpointPtr TEMP1
		adiw Y, ENDPOINT_OFFSET_AUXDATAH
		st Y, TEMP2

		inc TEMP1
		ldi TEMP2, ENDPOINT_COUNT
		cpse TEMP1, TEMP2
		jmp ENDPOINT_INPUT_CONFIG_LOOP

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

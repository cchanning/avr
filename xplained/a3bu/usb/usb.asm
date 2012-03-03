/*********************************************************************************************
 * 
 * Atmel ATXMEGA256A3BU USB Firmware
 *
 * Author: Chris Channing
 * 
 *********************************************************************************************
 *
 *
 *	+------------------------------------------------------------------------------+
 *	+ Application Stack (maintained by Z register, grows downwards from RAMSTART)
 *	+------------------------------------------------------------------------------+
 *	+ Application Heap (First two bytes are reserved for the free byte pointer)
 *	+------------------------------------------------------------------------------+
 *	+ Endpoint Configuration Table
 *	+------------------------------------------------------------------------------+
 *	+ CPU Stack (grows upwards from RAMEND)
 *	+------------------------------------------------------------------------------+
 *
 *
 *********************************************************************************************/
 
 .include "ATxmega256A3BUdef.inc"

 /*********************************************************************************************
 * Register Aliases
 *********************************************************************************************/

.def TEMP0 = R21
.def TEMP1 = R22
.def TEMP2 = R23
.def TEMP3 = R24
.def TEMP4 = R25

 /*********************************************************************************************
 * Main
 *********************************************************************************************/

.cseg

.org 0x0000
	jmp reset
.org USB_BUSEVENT_vect
	jmp isr_device_bus_event
.org USB_TRNCOMPL_vect
	jmp isr_device_transaction_complete

reset:
	ldi TEMP0, low(RAMEND)
	sts CPU_SPL, TEMP0											; configure low byte of CPU stack pointer
	ldi TEMP0, high(RAMEND)
	sts CPU_SPH, TEMP0											; configure high byte of CPU stack pointer
	ldi ZL, low(APPLICATION_STACK_START)						; configure low byte of application stack pointer
	ldi ZH, high(APPLICATION_STACK_START)						; configure high byte of application stack pointer
	sbiw Z, 1													; cause the SP to be one before the stack start address, this is required as a pusha will first increment Z. In the first usage scenario it will make sure the data is placed at the stack start address.	

	call configure_heap
	call configure_system_clock
	call configure_usb_clock
	call configure_usb_io
	call enable_interrupts
	call enable_usb

	jmp main													; jump to the main program loop

main:
	jmp main													; keep the CPU busy forever

/*********************************************************************************************
 * Application Memory
 *********************************************************************************************/

.equ APPLICATION_STACK_START = SRAM_START
.equ APPLICATION_STACK_SIZE = 32

.macro pushai
	adiw Z, 1
	ldi R16, @0
	st Z, R16
.endm

.macro pusha
	adiw Z, 1
	mov R16, @0
	st Z, R16
.endm

.macro popa
	ld @0, Z
	ldi R16, 0
	st Z, R16
	sbiw Z, 1
.endm

/**
 * Saves the application registers to the CPU stack. This macro should be called at the start of a function
 * to avoid tromping on memory used by other parent functions etc. It must be restored using the ctxswib macro.
 */
.macro ctxswi
	push TEMP0
	push TEMP1
	push TEMP2
	push TEMP3
	push TEMP4
	push YL
	push YH
	push XL
	push XH

	//now clear the registers
	clr TEMP0
	clr TEMP1
	clr TEMP2
	clr TEMP3
	clr TEMP4
	clr YL
	clr YH
	clr XL
	clr XH
.endm

/**
 * Restores/switches back the persisted application registers from the CPU stack, this macro should only be used when ctxswi is used.
 */
.macro ctxswib
	pop XH
	pop XL
	pop YH
	pop YL
	pop TEMP4
	pop TEMP3
	pop TEMP2
	pop TEMP1
	pop TEMP0
.endm

.equ APPLICATION_HEAP_START = APPLICATION_STACK_START + APPLICATION_STACK_SIZE
.equ APPLICATION_HEAP_SIZE = 1024

/**
 * Loads the heap free byte pointer address into the given register pair
 */
.macro lhfbp
	lds R16, APPLICATION_HEAP_START
	lds R17, APPLICATION_HEAP_START + 1
	movw @0, R17:R16
.endm

/**
 * Increments the heap free byte pointer by the given number
 */
.macro ahfbp
	lhfbp R17:R16
	ldi R18, @0
	add R16, R18
	clr R18
	adc R17, R18
	sts APPLICATION_HEAP_START, R16
	sts APPLICATION_HEAP_START + 1, R17
.endm

configure_heap:
	ctxswi

	ldi TEMP0, low(APPLICATION_HEAP_START + 2)		
	sts APPLICATION_HEAP_START, TEMP0									; store LSB of heap free byte pointer
	ldi TEMP0, high(APPLICATION_HEAP_START + 2)		
	sts APPLICATION_HEAP_START + 1, TEMP0								; store MSB of heap free byte pointer

	//now zero out the heap memory for cleanliness
	clr TEMP0
	lhfbp X																; load the free heap byte pointer into X
	ldi YL, low(APPLICATION_HEAP_START + APPLICATION_HEAP_SIZE)			; load Y with address of last useable heap byte + 1
	ldi YH, high(APPLICATION_HEAP_START + APPLICATION_HEAP_SIZE)

	//keep zero'ing each byte of the heap while X < Y 
	CONFIGURE_HEAP_ZERO_LOOP:
		st	X, TEMP0
		adiw X, 1														; increment X by 1
		cp XL, YL
		cpc XH, YH
		brlt CONFIGURE_HEAP_ZERO_LOOP									; if X is still lower then Y keep going

	ctxswib
	ret

/*********************************************************************************************
 * USB Endpoint
 *********************************************************************************************/

.equ ENDPOINT_COUNT = 2 ; Each endpoint has two pipes (OUT/IN)
.equ ENDPOINT_CFG_TBL_START = APPLICATION_HEAP_START + APPLICATION_HEAP_SIZE
.equ ENDPOINT_START = ENDPOINT_CFG_TBL_START //in the future FIFO maybe enabled meaning the ENDPOINT_START is further from the cfg table start

.equ ENDPOINT_PIPE_OFFSET_STATUS = 0
.equ ENDPOINT_PIPE_OFFSET_CTRL = 1
.equ ENDPOINT_PIPE_OFFSET_CNTL = 2
.equ ENDPOINT_PIPE_OFFSET_CNTH = 3
.equ ENDPOINT_PIPE_OFFSET_DATAPTRL = 4
.equ ENDPOINT_PIPE_OFFSET_DATAPTRH = 5
.equ ENDPOINT_PIPE_OFFSET_AUXDATAL = 6
.equ ENDPOINT_PIPE_OFFSET_AUXDATAH = 7

.equ ENDPOINT_PIPE_MASK_TYPE_CONTROL = 0b01000000
.equ ENDPOINT_PIPE_MASK_TYPE_BULK = 0b10000000

.equ ENDPOINT_PIPE_MASK_BUFFER_SIZE_32 = 0b00000010
.equ ENDPOINT_PIPE_BUFFER_SIZE = 32

/*
 * This macro will calculate and store in register Y the start address of the output pipe for the supplied endpoint.
 *
 * Parameter 0 - the endpoint number ranging from 0 - n
 */
.macro coep
	ldi YL, low(ENDPOINT_START)									; configure low byte of pointer to the first endpoint
	ldi YH, high(ENDPOINT_START)								; configure high byte of pointer to the first endpoint
	mov R19, @0
	ldi R18, 16
	mul R19, R18												; MUL saves result in R1:R0 where R1 == HIGH and R0 == low
	add YL, R0
	adc YH, R1
.endm

/*
 * This macro will calculate and store in register Y the start address of the input pipe for the supplied endpoint.
 *
 * Parameter 0 - the endpoint number ranging from 0 - n
 */
.macro ciep
	coep @0														; configure Y to point at the output portion of the endpoint
	adiw Y, 8													; the input portion of the endpoint is always 8 bytes from the start of the output portion
.endm

/****************************************************************************************
 * USB Bus Event Constants
 ****************************************************************************************/

 .equ BUS_EVENT_MASK_RESET = 0b00010000

/****************************************************************************************
 * USB Request/Descriptor Constants
 ****************************************************************************************/
 .equ REQUEST_MASK_TYPE_STANDARD = 0b00000000
 .equ REQUEST_MASK_TYPE_CLASS = 0b00100000
 .equ REQUEST_MASK_TYPE_VENDOR = 0b01000000
 .equ REQUEST_MASK_TYPE_RESERVED = 0b01100000
 .equ REQUEST_MASK_TYPE_RECIPIENT_DEVICE = 0b00000000
 .equ REQUEST_MASK_TYPE_RECIPIENT_INTERFACE = 0b00000001
 .equ REQUEST_MASK_TYPE_RECIPIENT_ENDPOINT = 0b00000010
 .equ REQUEST_MASK_TYPE_RECIPIENT_OTHER = 0b00000011

.equ REQUEST_DEVICE_MASK_GET_STATUS = 0b00000000
.equ REQUEST_DEVICE_MASK_CLEAR_FEATURE = 0b00000001
.equ REQUEST_DEVICE_MASK_SET_FEATURE = 0b00000011
.equ REQUEST_DEVICE_MASK_SET_ADDRESS = 0b00000101
.equ REQUEST_DEVICE_MASK_GET_DESCRIPTOR = 0b00000110
.equ REQUEST_DEVICE_MASK_SET_DESCRIPTOR = 0b00000111
.equ REQUEST_DEVICE_MASK_GET_CONFIGURATION = 0b00001000
.equ REQUEST_DEVICE_MASK_SET_CONFIGURATION = 0b00001001

.equ DEVICE_DESCRIPTOR_LENGTH = 0x12							; 18 bytes long
.equ DEVICE_DESCRIPTOR_TYPE = 0x01								; device descriptor
.equ DEVICE_DESCRIPTOR_BCD_USB = 0x0200							; support for USB 2.0 
.equ DEVICE_DESCRIPTOR_DEVICE_CLASS = 0x02						; pretend we're (Communication Device Class) CDC based
.equ DEVICE_DESCRIPTOR_DEVICE_SUB_CLASS = 0x00 
.equ DEVICE_DESCRIPTOR_DEVICE_PROTOCOL = 0x00
.equ DEVICE_DESCRIPTOR_MAX_PACKET_SIZE = 0x08
.equ DEVICE_DESCRIPTOR_ID_VENDOR = 0x03EB						; pretend we're Atmel for now :)						
.equ DEVICE_DESCRIPTOR_ID_PRODUCT = 0x2FE2						; set the Amtel product ID to be the A3BU Xplained board
.equ DEVICE_DESCRIPTOR_BCD_DEVICE = 0x0100						; our product version is set at 1.0
.equ DEVICE_DESCRIPTOR_MANUFACTURER = 0x00
.equ DEVICE_DESCRIPTOR_PRODUCT = 0x00
.equ DEVICE_DESCRIPTOR_SERIAL_NUMBER = 0x00
.equ DEVICE_DESCRIPTOR_NUM_CONFIGURATIONS = 0x01 

/****************************************************************************************
 * Clock Functions
 ****************************************************************************************/

/*
 * By default the system clock will be using the 2Mhz internal oscillator (after hardware reset). This function will cause the PLL to 
 * be configured as the system clock source. Prescaler A will be configured to reduce the output from the PLL to 12Mhz rather than 48Mhz.
 */
configure_system_clock:
	ctxswi
	
	call configure_32mhz_int_osc								; make sure the 32Mhz internal oscillator is configured and is stable for use
	call configure_pll											; configure the PLL
	ldi TEMP0, 0b00001100
	sts CLK_PSCTRL, TEMP0										; set prescaler A to div by 4, disable prescalers C/B (CPU etc will run at 12Mhz)
	ldi TEMP0, 0b00000100
	sts CLK_CTRL, TEMP0											; set the clock source as PLL
	
	ctxswib
	ret

/*
 * Switch on the 32mhz internal oscillator and wait for it to become stable before returning.
 */
configure_32mhz_int_osc:
	ctxswi
	
	lds TEMP0, OSC_CTRL											; load up the existing oscillator configuration (by default the 2mhz oscillator should be enabled)
	ldi TEMP1, 0b00000010
	or TEMP0, TEMP1												; this will make sure we keep any previously enabled oscillators alive :)
	sts OSC_CTRL, TEMP0											; enable the 32mhz internal oscillator

	IOSC_NOT_READY:												; loop until the hardware has told us the 32mhz internal oscillator is stable
		ldi TEMP0, 0b00000010
		mov TEMP1, TEMP0
		lds TEMP2, OSC_STATUS
		and TEMP1, TEMP2
		cpse TEMP0, TEMP1										; when TEMP0 == TEMP1 the next instruction will be skipped, causing "ret" to be the next instruction to be executed
		jmp IOSC_NOT_READY
	
	ctxswib
	ret

 /**
  * Configures the PLL, this requires the 32Mhz internal oscillator to be the reference input, note that the hardware will automatically scale down the input
  * to the PLL to 8mhz (div 4) before we even get to touch it. The PLL multiplaction factor is set at 6 to scale the required clock frequency up to 48mhz. 
  * This will allow the PLL to be used as a clock source for the USB. Note that the 32mhz internal oscillator must be running and stable before calling this 
  * function.
  */
configure_pll:
	ctxswi
	
	ldi TEMP0, 0b10000110
	sts OSC_PLLCTRL, TEMP0										; configure the PLL input clock source as the 32Mhz internal oscillator (scaled down automatically to 8mhz by hardware), set the PLL frequency multiplication factor to 6 to scale the PLL output up to 48mhz
	lds	TEMP1, OSC_CTRL											; copy the current oscillator configuration (we dont' want to accidentally turn off the 32mhz internal osscillator etc!)
	ldi TEMP2, 0b00010000										; load the bitmask that will enable the PLL
	or TEMP1, TEMP2
	sts OSC_CTRL, TEMP1											; enable the PLL (whilst preserving any previously enabled osscillators)
	
	PLL_NOT_READY:												; loop until the hardware has told us the PLL is stable
		ldi TEMP1, 0b00010000
		mov TEMP2, TEMP1
		lds TEMP3, OSC_STATUS
		and TEMP2, TEMP3
		cpse TEMP1, TEMP2										; when TEMP1 == TEMP2 the next instruction will be skipped, causing "ret" to be the next instruction to be executed
		jmp PLL_NOT_READY
	
	ctxswib
	ret

/*
 * Enable the USB clock source as the PLL, note that the PLL must be configured and running before calling this function
 */
configure_usb_clock:
	ctxswi
	
	ldi TEMP0, 0b00000001
	sts CLK_USBCTRL, TEMP0										; set the PLL as the USB clock source, enable the USB clock source (start feeding the USB module clock signals)	
	
	ctxswib
	ret

/****************************************************************************************
 * General USB Functions
 ****************************************************************************************/

 /**
  * Enables interrupts for PMIC, USB and global.
  */
enable_interrupts:
	ctxswi

	ldi TEMP0, 0b00000100
	sts PMIC_CTRL, TEMP0										; enable high level in PMIC
	ldi TEMP0, 0b01000011
	sts USB_INTCTRLA, TEMP0										; enable txn and bus (BUSEVIE) interrupts to fire for USB
	ldi TEMP0, 0b00000001								
	sts USB_INTCTRLB, TEMP0										; enable interrupts to fire when SETUP transactions complete on USB
	sei															; enable global interrupts

	ctxswib
	ret

/**
 * Configures the USB module registers with required operating speed, endpoint count. Once configured, the USB module is activated.
 */
enable_usb:
	ctxswi

	call calibrate_usb											; load calibrated USB data from device production row

	ldi TEMP0, 0b11010000										; enables the following usb port, full speed, frame number tracking
	ldi TEMP1, ENDPOINT_COUNT
	or TEMP0, TEMP1												; enable the required number of endpoints
	sts USB_CTRLA, TEMP0										; activate the USB port with the configuration

	ldi TEMP0, 0b00000001										
	sts USB_CTRLB, TEMP0										; set the attach mode to bind USB module to USB tx/rx lines

	ctxswib
	ret

/**
 * Resets USB address and configures USB endpoints
 */
configure_usb_io:
	ctxswi

	clr TEMP0
	sts USB_ADDR, TEMP0											; reset device address
	call configure_usb_endpoints								; reset endpoints

	ctxswib
	ret

/**
 * Loads the USB calibration data from the device production row into the USB calibration registers
 */
calibrate_usb:
	ctxswi

	movw X, Z												; save app stack pointer as LPM needs the 16bit Z register

	ldi TEMP0, NVM_CMD_READ_CALIB_ROW_gc															
	sts NVM_CMD, TEMP0										; load nvm command register for reading calibration row	

	ldi TEMP0, PROD_SIGNATURES_START + NVM_PROD_SIGNATURES_USBCAL0_offset
	clr ZH
	mov ZL, TEMP0											; set ZL to be the USBCAL0 address
	lpm TEMP0, Z											; load the USBCAL0 value from NVM into TEMP0
	sts USB_CAL0, TEMP0										; store USBCAL0 value to USB module CAL0 register
	
	ldi TEMP0, PROD_SIGNATURES_START + NVM_PROD_SIGNATURES_USBCAL1_offset
	clr ZH
	mov ZL, TEMP0											; set ZL to be the USBCAL1 address
	lpm TEMP0, Z											; load the USBCAL1 value from NVM into TEMP0
	sts USB_CAL1, TEMP0										; store USBCAL1 value to USB module CAL1 register

	ldi TEMP0, NVM_CMD_NO_OPERATION_gc
	sts NVM_CMD, TEMP0										; clear NVM command register
	
	movw Z, X												; restore app stack pointer

	ctxswib
	ret

/****************************************************************************************
 * USB Endpoint Functions
 ****************************************************************************************/

configure_usb_endpoints:
	ctxswi

	ldi TEMP0, low(ENDPOINT_CFG_TBL_START)
	sts USB_EPPTR, TEMP0										; configure low byte of endpoint config table pointer
	ldi TEMP0, high(ENDPOINT_CFG_TBL_START)
	sts USB_EPPTR + 1, TEMP0									; configure high byte of endpoint config table pointer
	ldi TEMP0, 0												; configure endpoint counter

	//configure endpoint 0 out pipe
	coep TEMP0
	pushai ENDPOINT_PIPE_MASK_TYPE_CONTROL
	pusha YH 
	pusha YL
	call configure_usb_endpoint_pipe

	//configure endpoint 0 in pipe
	ciep TEMP0
	pushai ENDPOINT_PIPE_MASK_TYPE_CONTROL
	pusha YH 
	pusha YL
	call configure_usb_endpoint_pipe
	inc TEMP0

	ENDPOINT_CONFIG_LOOP: 
		//configure endpoint n output pipe							
		coep TEMP0
		pushai ENDPOINT_PIPE_MASK_TYPE_BULK
		pusha YH 
		pusha YL
		call configure_usb_endpoint_pipe

		//configure endpoint n input pipe
		ciep TEMP0
		pushai ENDPOINT_PIPE_MASK_TYPE_BULK
		pusha YH 
		pusha YL
		call configure_usb_endpoint_pipe

		inc TEMP0
		ldi TEMP2, ENDPOINT_COUNT
		cpse TEMP0, TEMP2
		jmp ENDPOINT_CONFIG_LOOP

	ctxswib
	ret

configure_usb_endpoint_pipe:
	ctxswi

	popa YL														; pop the low byte of the endpoint address from the application stack
	popa YH														; pop the high byte of the endpoint address from the application stack

	clr TEMP0							
	std Y + ENDPOINT_PIPE_OFFSET_STATUS, TEMP0
	popa TEMP0													; pop the type of endpoint to use from the application stack
	ori TEMP0, ENDPOINT_PIPE_MASK_BUFFER_SIZE_32				; set the buffer size for the endpoint
	std Y + ENDPOINT_PIPE_OFFSET_CTRL, TEMP0
	clr TEMP0
	std Y + ENDPOINT_PIPE_OFFSET_CNTL, TEMP0
	std Y + ENDPOINT_PIPE_OFFSET_CNTH, TEMP0
	lhfbp X														; load the heap free byte pointer address into X (XH:XL)
	std Y + ENDPOINT_PIPE_OFFSET_DATAPTRL, XL					; store the low byte of the HFBPtr address
	std Y + ENDPOINT_PIPE_OFFSET_DATAPTRH, XH					; store the high byte of the HFBPtr address
	ahfbp ENDPOINT_PIPE_BUFFER_SIZE								; now increment the HFBPtr by 32 to point at the next free byte					
	std Y + ENDPOINT_PIPE_OFFSET_AUXDATAL, TEMP0
	std Y + ENDPOINT_PIPE_OFFSET_AUXDATAH, TEMP0

	ctxswib
	ret

clear_usb_interrupt_flags_a:
	ctxswi

	ldi TEMP0, 0b11111111
	sts USB_INTFLAGSACLR, TEMP0

	ctxswib
	ret

clear_usb_interrupt_flags_b:
	ctxswi

	ldi TEMP0, 0b11111111
	sts USB_INTFLAGSBCLR, TEMP0

	ctxswib
	ret

handle_usb_setup_request:
	ctxswi

	ldi TEMP0, 0												; initialize the endpoint counter

	//loop through all output type endpoints (SETUP packets are always sent from host to client, making it an output transfer type)
	HANDLE_USB_SETUP_REQUEST_ENDPOINT_LOOP:
		//check if the endpoint is a control type
		coep TEMP0
		ldd TEMP1, Y + ENDPOINT_PIPE_OFFSET_CTRL
		ldi TEMP2, ENDPOINT_PIPE_MASK_TYPE_CONTROL
		and TEMP1, TEMP2										; bitwise "and" with bitmask to check if the endpoint is actually a "control" type
		cpse TEMP1, TEMP2										; if the endpoint isn't a control type, continue in the loop. If it is, the jmp instruction is skipped
		jmp HANDLE_USB_SETUP_REQUEST_ENDPOINT_LOOP_CONTINUE
		
		//now check if the endpoints status register reflects the setup txn
		ldd TEMP1, Y + ENDPOINT_PIPE_OFFSET_STATUS											; load status of EP output pipe into register
		ldi TEMP2, 0b00010000
		and TEMP1, TEMP2										; bitwise "and" with bitmask to check if the endpoint is reporting a SETUP txn completing
		cpse TEMP1, TEMP2										; if the endpoint hasn't had a SETUP txn complete, continue in the loop. If it has, the jmp instruction is skipped
		jmp HANDLE_USB_SETUP_REQUEST_ENDPOINT_LOOP_CONTINUE

		//now delegate to the parsing handler to process the request
		pusha TEMP0
		call process_usb_setup_request

		//now check cleanup status register for endpoint
		clr TEMP1												; the doc says write 1's to clear the endpoint status but I think this is a bug?
		std Y + ENDPOINT_PIPE_OFFSET_STATUS, TEMP1 

		HANDLE_USB_SETUP_REQUEST_ENDPOINT_LOOP_CONTINUE:
		inc TEMP0
		ldi TEMP1, ENDPOINT_COUNT
		cpse TEMP0, TEMP1
		jmp HANDLE_USB_SETUP_REQUEST_ENDPOINT_LOOP

	ctxswib
	ret

handle_usb_io_request:
	ctxswi
	ctxswib
	ret

/*
	This function will parse the SETUP request and invoke the appropriate handler. We need the endpoint number so we have
	the flexibility of using both the OUT and IN data pipes.

	@param endpoint number
 */
process_usb_setup_request:
	ctxswi

	//fetch the data pointer for the endpoint pipe
	popa TEMP0															; pop the endpoint number from the app stack
	coep TEMP0															; calculate and store endpoint output pipe address in Y
	ldd TEMP1, Y + ENDPOINT_PIPE_OFFSET_DATAPTRL														
	ldd TEMP2, Y + ENDPOINT_PIPE_OFFSET_DATAPTRH						; temp2:temp1 now holds the data pointer for the endpoint output pipe
	pusha YH															
	pusha YL															; backup endpoint output pipe address to the app stack so we can restore it later
	movw Y, TEMP2:TEMP1													; copy endpoint output pipe data pointer address in to Y
	movw X, Y															; copy Y into X (backup address so we can free TEMP1 and TEMP2 for other usage) 


	//load the request type here and the invoke the associated handler
	ld TEMP0, Y														; load byte from data buffer *(ptr + 0)
	
	//check if we're dealing with standard requests
	mov TEMP1, TEMP0
	andi TEMP1, REQUEST_MASK_TYPE_STANDARD
	breq PROCESS_USB_SETUP_REQUEST_STANDARD

	//finish up
	jmp PROCESS_USB_SETUP_REQUEST_RETURN

	//check what type of standard request it was e.g. device level
	PROCESS_USB_SETUP_REQUEST_STANDARD:
		mov TEMP1, TEMP0
		andi TEMP1, REQUEST_MASK_TYPE_RECIPIENT_DEVICE
		breq PROCESS_USB_SETUP_REQUEST_STANDARD_DEVICE
		mov TEMP1, TEMP0
		andi TEMP1, REQUEST_MASK_TYPE_RECIPIENT_INTERFACE 
		breq PROCESS_USB_SETUP_REQUEST_STANDARD_INTERFACE
		mov TEMP1, TEMP0
		andi TEMP1, REQUEST_MASK_TYPE_RECIPIENT_ENDPOINT 
		breq PROCESS_USB_SETUP_REQUEST_STANDARD_ENDPOINT
		mov TEMP1, TEMP0
		andi TEMP1, REQUEST_MASK_TYPE_RECIPIENT_OTHER 
		breq PROCESS_USB_SETUP_REQUEST_STANDARD_OTHER

		//if we get here, we don't have any handlers to support this type so skip on
		jmp PROCESS_USB_SETUP_REQUEST_RETURN

	PROCESS_USB_SETUP_REQUEST_STANDARD_DEVICE:
		jmp PROCESS_USB_SETUP_REQUEST_RETURN

	PROCESS_USB_SETUP_REQUEST_STANDARD_INTERFACE:
		jmp PROCESS_USB_SETUP_REQUEST_RETURN

	PROCESS_USB_SETUP_REQUEST_STANDARD_ENDPOINT:
		jmp PROCESS_USB_SETUP_REQUEST_RETURN

	PROCESS_USB_SETUP_REQUEST_STANDARD_OTHER:
		jmp PROCESS_USB_SETUP_REQUEST_RETURN

	PROCESS_USB_SETUP_REQUEST_RETURN:
		ctxswib
	ret

handle_usb_bus_event:
	ctxswi
	lds TEMP0, USB_INTFLAGSASET

	//check for reset
	mov TEMP1, TEMP0
	andi TEMP1, BUS_EVENT_MASK_RESET
	cpi TEMP1, BUS_EVENT_MASK_RESET
	breq HANDLE_USB_BUS_EVENT_RESET

	//finish
	jmp HANDLE_USB_BUS_EVENT_RETURN

	HANDLE_USB_BUS_EVENT_RESET:
		call process_usb_bus_event_reset
		jmp HANDLE_USB_BUS_EVENT_RETURN

	HANDLE_USB_BUS_EVENT_RETURN:
		ctxswib
	ret

process_usb_bus_event_reset:
	ctxswi
	
	call configure_heap
	call configure_usb_io

	ctxswib
	ret

/****************************************************************************************
 * USB ISRs
 ****************************************************************************************/

isr_device_bus_event:
	ctxswi
	
	cli													; prevent further interrupts occuring

	call handle_usb_bus_event
	call clear_usb_interrupt_flags_a
	call clear_usb_interrupt_flags_b

	sei													; enable interrupts again

	ctxswib
	reti

/*
	This ISR will examine each endpoint to determine which endpoint recieved data or 
	needs to respond to a SETUP request. Once found the ISR will push the address of
	the endpoint on the application stack and invoke the relevant handler.

	The USB interrupt flags will be cleared before returning.
 */
isr_device_transaction_complete:
	ctxswi

	cli												; prevent further interrupts occurring

	ldi TEMP0, 0b00000011
	sts PORTR_DIR, TEMP0
	ldi TEMP0, 0x00000010
	sts PORTR_OUT, TEMP0

	lds TEMP0, USB_INTFLAGSBSET						; read current interrupt flag to check if this txn was setup or IO
	andi TEMP0, 0b00000001							; bitwise "and" with bitmask to check if the transaction type was SETUP
	sbrc TEMP0, 0									; skip next instruction if bit 0 is != 1
	call handle_usb_setup_request
	sbrs TEMP0, 0									; skip next instruction if bit 0 is != 0
	call handle_usb_io_request
	call clear_usb_interrupt_flags_a
	call clear_usb_interrupt_flags_b

	sei												; enable interrupts again

	ctxswib
	reti

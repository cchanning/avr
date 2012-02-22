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
	ldi TEMP0, low(APPLICATION_HEAP_START + 2)		
	sts APPLICATION_HEAP_START, TEMP0							; store LSB of heap free byte pointer
	ldi TEMP0, high(APPLICATION_HEAP_START + 2)		
	sts APPLICATION_HEAP_START + 1, TEMP0						; store MSB of heap free byte pointer

	call configure_system_clock
	call configure_usb
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
 * This macro will calculate the pointer to the start of the output portion of endpoint and place the pointer in Y
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
 * This macro will calculate the pointer to the start of the input portion of the endpoint and place the pointer in Y
 *
 * Parameter 0 - the endpoint number ranging from 0 - n
 */
.macro ciep
	coep @0														; configure Y to point at the output portion of the endpoint
	adiw Y, 8													; the input portion of the endpoint is always 8 bytes from the start of the output portion
.endm

/****************************************************************************************
 * Clock Functions
 ****************************************************************************************/

/*
 * By default the system clock will be using the 2Mhz internal oscillator (after hardware reset). This function will cause the PLL to 
 * be configured as the system clock source. Prescaler A will be configured to reduce the output from the PLL to 3Mhz rather than 48Mhz.
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
  * from the PLL to 8mhz (div 4) before we even get to touch it. The PLL multiplaction factor is set at 6 to scale the required clock frequency up to 48mhz. 
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

enable_usb:
	ctxswi

	ldi TEMP0, 0b11010000										; enables the following usb port, full speed, frame number tracking
	ldi TEMP1, ENDPOINT_COUNT
	or TEMP0, TEMP1												; enable the required number of endpoints
	sts USB_CTRLA, TEMP0										; activate the USB port with the configuration

	ctxswib
	ret

disable_usb:
	ctxswi

	ldi TEMP0, 0x00
	sts USB_CTRLA, TEMP0

	ctxswib
	ret

configure_usb:
	ctxswi

	ldi TEMP0, 0x00
	sts USB_ADDR, TEMP0											; reset device address
	call configure_usb_clock
	call configure_usb_endpoints

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

	popa TEMP1													; pop the low byte of the endpoint address from the application stack
	popa TEMP2													; pop the high byte of the endpoint address from the application stack

	movw Y, TEMP2:TEMP1
	clr TEMP3							
	adiw Y, ENDPOINT_PIPE_OFFSET_STATUS
	st Y, TEMP3
	movw Y, TEMP2:TEMP1				
	adiw Y, ENDPOINT_PIPE_OFFSET_CTRL
	popa TEMP3													; pop the type of endpoint to use from the application stack
	ori TEMP3, ENDPOINT_PIPE_MASK_BUFFER_SIZE_32				; set the buffer size for the endpoint
	st Y, TEMP3
	clr TEMP3
	movw Y, TEMP2:TEMP1						
	adiw Y, ENDPOINT_PIPE_OFFSET_CNTL
	st Y, TEMP3
	movw Y, TEMP2:TEMP1					
	adiw Y, ENDPOINT_PIPE_OFFSET_CNTH
	st Y, TEMP3
	lhfbp X														; load the heap free byte pointer address into X (XH:XL)
	movw Y, TEMP2:TEMP1							
	adiw Y, ENDPOINT_PIPE_OFFSET_DATAPTRL
	st Y, XL													; store the low byte of the HFBPtr address
	movw Y, TEMP2:TEMP1					
	adiw Y, ENDPOINT_PIPE_OFFSET_DATAPTRH
	st Y, XH													; store the high byte of the HFBPtr address
	ahfbp ENDPOINT_PIPE_BUFFER_SIZE								; now increment the HFBPtr by 32 to point at the next free byte
	movw Y, TEMP2:TEMP1						
	adiw Y, ENDPOINT_PIPE_OFFSET_AUXDATAL
	st Y, TEMP3
	movw Y, TEMP2:TEMP1	
	adiw Y, ENDPOINT_PIPE_OFFSET_AUXDATAH
	st Y, TEMP3

	ctxswib
	ret

clear_usb_interrupt_flags_a:
	ctxswi

	ldi TEMP0, 0b11111111
	sts USB_INTFLAGSACLR, TEMP0

	ctxswib
	ret

handle_usb_setup_request:
	ctxswi

	ldi TEMP0, 0												; initialize the endpoint counter

	//loop through all output type endpoints (SETUP packets are always sent from host to client, making it an output transfer type)
	HANDLE_USB_SETUP_REQUEST_ENDPOINT_LOOP:
		//check if the endpoint is a control type
		coep TEMP0
		movw X, Y
		adiw Y, ENDPOINT_PIPE_OFFSET_CTRL
		ld TEMP1, Y
		ldi TEMP2, ENDPOINT_PIPE_MASK_TYPE_CONTROL
		and TEMP1, TEMP2										; bitwise "and" with bitmask to check if the endpoint is actually a "control" type
		cpse TEMP1, TEMP2										; if the endpoint isn't a control type, continue in the loop. If it is, the jmp instruction is skipped
		jmp HANDLE_USB_SETUP_REQUEST_ENDPOINT_LOOP_CONTINUE
		
		//now check if the endpoints status register reflects the setup txn
		movw Y, X
		adiw Y, ENDPOINT_PIPE_OFFSET_STATUS
		ld TEMP1, Y												; load status of EP output pipe into register
		ldi TEMP2, 0b00010000
		and TEMP1, TEMP2										; bitwise "and" with bitmask to check if the endpoint is reporting a SETUP txn completing
		cpse TEMP1, TEMP2										; if the endpoint hasn't had a SETUP txn complete, continue in the loop. If it has, the jmp instruction is skipped
		jmp HANDLE_USB_SETUP_REQUEST_ENDPOINT_LOOP_CONTINUE

		//now delegate to the parsing handler to process the request
		pusha TEMP0
		call process_usb_setup_request

		//now check cleanup status register for endpoint
		clr TEMP1												; the doc says write 1's to clear the endpoint status but I think this is a bug?
		movw Y, X
		adiw Y, ENDPOINT_PIPE_OFFSET_STATUS
		st Y, TEMP1 

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

	popa TEMP0
	coep TEMP0
	movw X, Y
	adiw Y, ENDPOINT_PIPE_OFFSET_DATAPTRL
	ld TEMP1, Y
	movw Y, X
	adiw Y, ENDPOINT_PIPE_OFFSET_DATAPTRH
	ld TEMP2, Y															; temp2:temp1 now holds the DATAPTR for the endpoint
	movw TEMP4:TEMP3, X													; backup X as it holds the ptr to the endpoint output pipe
	movw X, TEMP2:TEMP1													; copy the pipe data ptr into X so we can use it

	//decode the request type here and the invoke the associated handler

	ctxswib
	ret

/****************************************************************************************
 * USB ISRs
 ****************************************************************************************/

isr_device_bus_event:
	ctxswi

	call clear_usb_interrupt_flags_a

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

	lds TEMP0, USB_INTFLAGSBSET						; read current interrupt flag to check if this txn was setup or IO
	andi TEMP0, 0b00000001							; bitwise "and" with bitmask to check if the transaction type was SETUP
	sbrc TEMP0, 0									; skip next instruction if bit 0 is != 1
	call handle_usb_setup_request
	sbrs TEMP0, 0									; skip next instruction if bit 0 is != 0
	call handle_usb_io_request
	call clear_usb_interrupt_flags_a

	ctxswib
	reti

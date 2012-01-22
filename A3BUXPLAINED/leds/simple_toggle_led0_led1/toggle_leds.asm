/*
 * Summary:	This program will toggle both LED0 and LED1 of an Atmel A3BU Xplained Evaluation board by clicking SW0.
 *
 * Description: The reset code will configure an interrupt to fire when PE5 goes to logic level low (when the button is pressed). This will cause
 *				the "toggle_leds_isr" ISR to be invoked that will toggle LED0 and LED1. This is achieved by first reading PORTR (LED0/LED1 are attached)
 *				and then XOR'ing the value with the mask 0x03 (remember that PORTR only has two pins). The toggled value is then written back out to PORTR.
 *
 *				Key points to remember:
 *											- each interrupt must be configured to use the right level and sensing values
 *											- the PMIC must be configured to use the right level otherwise the interrupts won't fire
 *											- enable global interrupts otherwise the interrupts won't fire
 */ 

 .cseg

 .org 0x0000
	jmp reset

.org PORTE_INT0_vect			; configure PORTE INT0 to jump to the toggle_leds ISR
	jmp toggle_leds_isr

reset:
	ldi R29, low(RAMEND)
	sts CPU_SPL, R29			; configure low byte of stack pointer
	ldi R29, high(RAMEND)
	sts CPU_SPH, R29			; configure high byte of stack pointer

	call toggle_leds			; switch on LED0/LED1

	ldi R29, 0b00000011 
	sts PORTE_INTCTRL, R29		; enable INT0 only for PORTE with the highest level
	ldi R29, 0b00100000
	sts PORTE_INT0MASK, R29		; bind PR5 (remember pins start at 0) of PORTE to INT0
	ldi R29, 0b00000110
	sts PORTE_PIN5CTRL, R29		; configure pin 5 of PORTE to use low level sensing

	ldi R29, 0b00000100
	sts PMIC_CTRL, R29			; enable high level in PMIC

	sei							; enable all interupts (global enable)

	jmp loop					; kick off the program loop

loop:
	jmp loop

toggle_leds_isr:
	call toggle_leds
	reti

toggle_leds:
	ldi R29, 0b00000000
	sts PORTR_DIR, R29			; configure PORTR for input
	lds R28, PORTR_IN			; read current value of PORTR

	ldi R29, 0b00000011 
	sts PORTR_DIR, R29			; configure PORTR for output
	ldi R27, 0b00000011
	eor R28, R27				; inverse the value of R28 
	sts PORTR_OUT, R28			; toggle the LEDs
	ret
/*
 * assembly.s
 *
 */
 
 @ DO NOT EDIT
	.syntax unified
    .text
    .global ASM_Main
    .thumb_func

@ DO NOT EDIT
vectors:
	.word 0x20002000
	.word ASM_Main + 1

@ DO NOT EDIT label ASM_Main
ASM_Main:

	@ Some code is given below for you to start with
	LDR R0, RCC_BASE  		@ Enable clock for GPIOA and B by setting bit 17 and 18 in RCC_AHBENR
	LDR R1, [R0, #0x14]
	LDR R2, AHBENR_GPIOAB	@ AHBENR_GPIOAB is defined under LITERALS at the end of the code
	ORRS R1, R1, R2
	STR R1, [R0, #0x14]

	LDR R0, GPIOA_BASE		@ Enable pull-up resistors for pushbuttons
	MOVS R1, #0b01010101
	STR R1, [R0, #0x0C]
	LDR R1, GPIOB_BASE  	@ Set pins connected to LEDs to outputs
	LDR R2, MODER_OUTPUT
	STR R2, [R1, #0]
	MOVS R2, #0         	@ NOTE: R2 will be dedicated to holding the value on the LEDs

@ TODO: Add code, labels and logic for button checks and LED patterns
LDR R0, GPIOA_BASE

Long_Delay:
	LDR R4, =LONG_DELAY_CNT @ The number we have to count down from to get a 0.7s delay is stored in R4
	LDR R4, [R4]
	B main_loop

Short_Delay:
	LDR R4, =SHORT_DELAY_CNT @ Replace the number we have to count down from with the smaller number]
	LDR R4, [R4]
	B main_loop

write_leds:
	LDR R3, [R0, #0x10]    @ Read GPIOA IDR (input data register) into R3
	MOVS R6, #8 @ check if switch 3 is pressed
	TST R3, R6
	BEQ sw3_pressed @Branch to the correct section if switch 3 is pressed

	STR R2, [R1, #0x14] @ Store the current led pattern in the ODR (Output Data Register)
	ADD R2, R2, R5	@ Increment the register by the incrementer value set in R5

	MOVS R6, #4 @ check if switch 2 is pressed
	TST R3, R6
	BEQ sw2_pressed @Branch to the correct section if switch 2 is pressed

	MOVS R6, #2 @ Set R1 to 0x2 to check if switch 1 is on
	TST R3, R6  @ Check if bit 1 is set (switch 1)
	BEQ Short_Delay @ If bit 1 is set go to short delay
	B Long_Delay


main_loop:
	LDR R3, [R0, #0x10]    @ Read GPIOA IDR (input data register) into R3
	MOVS R6, #1 @ Set R6 to 1 so we can do a bitwise and with it
	TST R3, R6  @ Check if bit 0 is set (switch 0)
	BNE sw0_not_pressed

sw0_pressed:
	MOVS R5, #2
	B check_Delay

sw0_not_pressed:
	MOVS R5, #1

check_Delay:
	SUBS R4, R4, #1
	BEQ write_leds @Branch to write to the leds when we have counted down to
	B main_loop

sw2_pressed:
	MOVS R2, #0xaa
	STR R2, [R1, #0x14]
	LDR R3, [R0, #0x10]    @ Read GPIOA IDR (input data register) into R3
	TST R3, R6
	BEQ sw2_pressed
	B write_leds

sw3_pressed:
	LDR R3, [R0, #0x10]    @ Read GPIOA IDR (input data register) into R3
	TST R3, R6
	BEQ sw3_pressed
	B write_leds

@ LITERALS; DO NOT EDIT
	.align
RCC_BASE: 			.word 0x40021000
AHBENR_GPIOAB: 		.word 0b1100000000000000000
GPIOA_BASE:  		.word 0x48000000
GPIOB_BASE:  		.word 0x48000400
MODER_OUTPUT: 		.word 0x5555

@ TODO: Add your own values for these delays
LONG_DELAY_CNT: 	.word 300000
SHORT_DELAY_CNT: 	.word 100000

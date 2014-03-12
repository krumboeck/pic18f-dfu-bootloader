/*
 * PIC18F DFU Bootloader
 *
 * Author: Bernd Krumböck <krumboeck@universalnet.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

// #include "pic18fregs.h"
#include <pic18f2550.h>
#include <stdio.h>
#include <usart.h>

#include "debug.h"
#include "typedef.h"
#include "usb/usb_descriptors.h"
#include "usb/usb_std_req.h"
#include "usb/usb.h"
#include "dfu/dfu.h"

#pragma stack 0x200 255

static __code char __at(__CONFIG1H) conf1H = _OSC_HS__HS_PLL__USB_HS_1H & _FCMEN_OFF_1H & _IESO_ON_1H;
static __code char __at(__CONFIG1L) conf1L = _USBPLL_CLOCK_SRC_FROM_96MHZ_PLL_2_1L & _CPUDIV__OSC1_OSC2_SRC___1__96MHZ_PLL_SRC___2__1L &_PLLDIV_DIVIDE_BY_5__20MHZ_INPUT__1L;
static __code char __at(__CONFIG2H) conf2H = _WDT_DISABLED_CONTROLLED_2H;
static __code char __at(__CONFIG2L) conf2L = _BODEN_ON_2L & _BODENV_4_2V_2L;
static __code char __at(__CONFIG3H) conf3H = _PBADEN_PORTB_4_0__CONFIGURED_AS_DIGITAL_I_O_ON_RESET_3H;
static __code char __at(__CONFIG4L) conf4L = _LVP_OFF_4L & _ENHCPU_OFF_4L; // & _BACKBUG_OFF_4L & _STVR_OFF_4L;

#define LED LATBbits.LATB0
#define NOISE LATBbits.LATB2
#define BUTTON LATBbits.LATB4


void jump_to_app() {
	debug("Reset all flags\n");
    RCON |= 0x93;     // reset all reset flag
	debug("Jump to app\n");
	__asm
		goto ENTRY
    __endasm;
}

void main(void) {
	unsigned char oldValue = 0;
	unsigned int counter = 0;
	TRISBbits.TRISB4 = 1;  // RB4 input
	TRISBbits.TRISB2 = 1;  // RB2 input
	TRISBbits.TRISB0 = 0;  // RB0 output
	LED = 1;


	// serial communication

	TRISCbits.TRISC6 = 0; //TX pin set as output
	TRISCbits.TRISC7 = 1; //RX pin set as input

	usart_open(
			USART_TX_INT_OFF & USART_RX_INT_OFF & USART_BRGH_HIGH
					& USART_EIGHT_BIT & USART_ASYNCH_MODE, 25 // BAUD_RATE_GEN is calculated as = [Fosc / (16 * Desired Baudrate)] - 1
			);

	stdout = STREAM_USART;

	debug("Serial interface started\n");

	if ((PORTB & (1<<4)) > 0) {
		jump_to_app();
	}

	// Initialise USB

	debug("use boot descriptors\n");
	device_descriptor = &boot_device_descriptor;
	configuration_descriptor = (const void **) boot_configuration_descriptor;
	string_descriptor = boot_string_descriptor;

	ep_init = boot_ep_init;
	ep_in = boot_ep_in;
	ep_out = boot_ep_out;
	ep_setup = boot_ep_setup;

	init_usb();
	debug("USB interface started\r\n");

	init_dfu();
	debug("DFU started\r\n");

	LED = 0;

	while (1) {
		enable_usb();
		dispatch_usb_event();
		if (dfuOperationStarted()) {
			dfuFinishOperation();
		}
	}

}


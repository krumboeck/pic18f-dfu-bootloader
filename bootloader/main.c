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

#include "pic18fregs.h"

#ifdef _DEBUG
	#include <stdio.h>
	#include <usart.h>
#endif

#include "debug.h"
#include "typedef.h"
#include "fuses.h"
#include "config.h"
#include "usb/usb_descriptors.h"
#include "usb/usb_std_req.h"
#include "usb/usb.h"
#include "dfu/dfu.h"
#include "led.h"

#pragma stack 0x200 255

void main(void) {

	u16 reset_timeout = 0;
	/*
	 * Initialize Ports
	 */

	BUTTON_TRIS = 1;
	led_init();
	led_off();

#ifdef _DEBUG
	TRISCbits.TRISC6 = 0; //TX pin set as output
	TRISCbits.TRISC7 = 1; //RX pin set as input

	usart_open(
			USART_TX_INT_OFF & USART_RX_INT_OFF & USART_BRGH_HIGH
					& USART_EIGHT_BIT & USART_ASYNCH_MODE, 25 // BAUD_RATE_GEN is calculated as = [Fosc / (16 * Desired Baudrate)] - 1
			);

	stdout = STREAM_USART;
	debug("Serial interface started\n");
#endif

	/*
	 * Check if button is pressed
	 */
#ifdef BUTTON_PORT
	if ((BUTTON_PORT & (1<<BUTTON_PIN)) > 0) {
		jump_to_app();
	}
#endif

	/*
	 * Configure USB and DFU
	 */
	debug("Configure usb descriptors\n");
	device_descriptor = &boot_device_descriptor;
	configuration_descriptor = (const void **) boot_configuration_descriptor;
	string_descriptor = boot_string_descriptor;

	ep_init = boot_ep_init;
	ep_in = boot_ep_in;
	ep_out = boot_ep_out;
	ep_setup = boot_ep_setup;

	init_usb();
	debug("USB interface started\n");

	init_dfu();

	/*
	 * Run USB, DFU, ...
	 */
	led_on();
	while (1) {
		enable_usb();
		dispatch_usb_event();
		if (dfuOperationStarted()) {
			dfuFinishOperation();
		}
		if (dfuWaitReset()) {
			setManifest();
			reset_timeout++;
		}
		if (reset_timeout > 0) {
			reset_timeout++;
		}
		if (reset_timeout > 10000) {
			close_usb();
			jump_to_app();
		}
	}

}

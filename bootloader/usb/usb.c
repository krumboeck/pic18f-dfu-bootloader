/*
 * PIC18F DFU Bootloader
 *
 * Author: Bernd Krumböck <krumboeck@universalnet.at>
 *         Based on André Gentric's and
 *         Régis Blanchot's <rblanchot@gmail.com> USB Pinguino Bootloader,
 *         based on Pierre Gaufillet's <pierre.gaufillet@magic.fr> PUF,
 *         based on Albert Faber's JAL bootloader
 *         and Alexander Enzmann's USB Framework
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

#include <pic18fregs.h>

#include "debug.h"
#include "typedef.h"
#include "usb/usb_descriptors.h"
#include "usb/usb.h"

/* Buffer descriptors Table */
volatile BufferDescriptorTable __at (0x400) ep_bdt[32];
const USB_Device_Descriptor *device_descriptor;
const void **configuration_descriptor;
const u8* const *string_descriptor;

void (***ep_init)(void);
void (***ep_in)(void);
void (***ep_out)(void);
void (***ep_setup)(void);

#pragma udata access usb_device_state
u8 __at(0x005f) usb_device_state;
#pragma udata access usb_active_cfg
u8 __at(0x005e) usb_active_cfg;
#pragma udata access usb_active_alt_setting
u8 __at(0x005d) usb_active_alt_setting;

void init_usb(void) {
	debug_usb("USB Init\r\n");
	UIE = 0;
	UCFG = 0x14;
	UCON = 0x00;
	SET_DEVICE_STATE(DETACHED_STATE);

	// Clear active configuration and alternate setting
	SET_ACTIVE_CONFIGURATION(DEFAULT_CONFIGURATION);
	SET_ACTIVE_ALTERNATE_SETTING(DEFAULT_CONFIGURATION);
}

void enable_usb(void) {

	// TBD: Check for voltage coming from the USB cable and use that
	// as an indication we are attached.
	if (UCONbits.USBEN == 0) {
		debug_usb("Device attached\r\n");
		UCON = 0;
		UIE = 0;
		UCONbits.USBEN = 1;
		SET_DEVICE_STATE(ATTACHED_STATE);
	}

	// If we are attached and no single-ended zero is detected, then
	// we can move to the Powered state.
	if ((GET_DEVICE_STATE() == ATTACHED_STATE) && !UCONbits.SE0) {
		debug_usb("Device powered\r\n");
		UIR = 0;
		UIE = 0;
		UIEbits.URSTIE = 1;
		UIEbits.IDLEIE = 1;
		SET_DEVICE_STATE(POWERED_STATE);
	}

}

void unsuspend(void) {
	debug_usb("UnSuspend\r\n");
	UCONbits.SUSPND = 0;
	UIEbits.ACTVIE = 0;
	UIRbits.ACTVIF = 0;
}

void close_usb(void) {
	// Flush any pending transactions
	while (UIRbits.TRNIF == 1)
		UIRbits.TRNIF = 0;

	// Disable packet processing
	UCONbits.PKTDIS = 1;

	UCON = 0;
	UIE = 0;
	UCONbits.USBEN = 0;
}

void bus_reset() {
	u8 i;

	debug_usb("Bus reset\r\n");
	UEIR = 0x00;
	UIR = 0x00;
	UEIE = 0x9f;
	UIE = 0x7b;
	UADDR = 0x00;

	// Set endpoint 0 as a control pipe
	UEP0 = EP_CTRL | HSHK_EN;

	// Flush any pending transactions
	while (UIRbits.TRNIF == 1)
		UIRbits.TRNIF = 0;

	// Enable packet processing
	UCONbits.PKTDIS = 0;

	UEP0 = 0;
	UEP1 = 0;
	UEP2 = 0;
	UEP3 = 0;
	UEP4 = 0;
	UEP5 = 0;
	UEP6 = 0;
	UEP7 = 0;
	UEP8 = 0;
	UEP9 = 0;
	UEP10 = 0;
	UEP11 = 0;
	UEP12 = 0;
	UEP13 = 0;
	UEP14 = 0;
	UEP15 = 0;

	// remoteWakeup = 0;                     // Remote wakeup is off by default
	// selfPowered = 0;                      // Self powered is off by default
	SET_DEVICE_STATE(DEFAULT_STATE);
	SET_ACTIVE_CONFIGURATION(0);
	SET_ACTIVE_ALTERNATE_SETTING(0);

	debug_usb("Start ep_init\r\n");
	for (i = 0; i < 16; i++) {
		ep_init[0][i]();
	}

}

void suspend(void) {
	debug_usb("Suspend\r\n");
	UIEbits.ACTVIE = 1;
	UIRbits.IDLEIF = 0;
	UCONbits.SUSPND = 1;

	PIR2bits.USBIF = 0;
	INTCONbits.RBIF = 0;
	PIE2bits.USBIE = 1;
	INTCONbits.RBIE = 1;

	// disable the USART
	RCSTAbits.CREN = 0;
	TXSTAbits.TXEN = 0;

	Sleep();

	// enable the USART
	RCSTAbits.CREN = 1;
	TXSTAbits.TXEN = 1;

	PIE2bits.USBIE = 0;
	INTCONbits.RBIE = 0;
}

// Full speed devices get a Start Of Frame (SOF) packet every 1 millisecond.
// Nothing is currently done with this interrupt (it is simply masked out).
void start_of_frame(void) {
	// debug("Start of Frame (ignored)\r\n");
	// TBD: Add a callback routine to do something
	UIRbits.SOFIF = 0;
}

// This routine is called in response to the code stalling an endpoint.
void stall(void) {
	debug_usb("Stall\r\n");
	if (UEP0bits.EPSTALL == 1) {
		// Prepare for the Setup stage of a control transfer
		ep_init[GET_ACTIVE_CONFIGURATION()][0]();
		UEP0bits.EPSTALL = 0;
	}
	UIRbits.STALLIF = 0;
}

void process_event(void) {
	// Process event for endpoint EPx
	if (USTATbits.DIR == OUT) {
		if (EP_OUT_BD(USTATbits.ENDP).Stat.PID == SETUP_TOKEN) {
			// SETUP packet has been received
			ep_setup[GET_ACTIVE_CONFIGURATION()][USTATbits.ENDP]();
		} else {
			// OUT packet has been received
			ep_out[GET_ACTIVE_CONFIGURATION()][USTATbits.ENDP]();
		}
	} else if (USTATbits.DIR == IN) {
		// IN packet has been sent
		ep_in[GET_ACTIVE_CONFIGURATION()][USTATbits.ENDP]();
	}
}

void dispatch_usb_event(void) {
	// See if the device is connected yet.
	if (GET_DEVICE_STATE() == DETACHED_STATE)
		return;

	// If the USB became active then wake up from suspend
	if (UIRbits.ACTVIF && UIEbits.ACTVIE)
		unsuspend();

	// If we are supposed to be suspended, then don't try performing any
	// processing.
	if (UCONbits.SUSPND == 1)
		return;

	// Process a bus reset
	if (UIRbits.URSTIF && UIEbits.URSTIE)
		bus_reset();

	// No bus activity for a while - suspend the firmware
	if (UIRbits.IDLEIF && UIEbits.IDLEIE)
		suspend();

	if (UIRbits.SOFIF && UIEbits.SOFIE)
		start_of_frame();

	if (UIRbits.STALLIF && UIEbits.STALLIE)
		stall();

	// TBD: See where the error came from.
	// Clear errors
	if (UIRbits.UERRIF && UIEbits.UERRIE)
		UIRbits.UERRIF = 0;

	// Unless we have been reset by the host, no need to keep processing
	if (GET_DEVICE_STATE() < DEFAULT_STATE)
		return;

	// A transaction has finished.  Try default processing on endpoint 0.
	if (UIRbits.TRNIF && UIEbits.TRNIE) {
		process_event();

		// Turn off interrupt
		UIRbits.TRNIF = 0;
	}

}

void fill_in_buffer(u8 EPnum, u8 **source, u16 buffer_size, u16 *nb_byte) {
	static u16 byte_to_send;
	static u8 *dest;

	// First, have to figure out how many byte of data to send.
	if (*nb_byte < buffer_size) {
		byte_to_send = *nb_byte;
	} else {
		byte_to_send = buffer_size;
	}

	EP_IN_BD(EPnum).Cnt = byte_to_send;

	// Set destination pointer
	dest = (u8 __data *) EP_IN_BD(EPnum).ADR;

	// Subtract the number of bytes just about to be sent from the total.
	*nb_byte -= byte_to_send;

	// Copy bytes to be sent
	while (byte_to_send) {
		*dest = **source;
		dest++;
		(*source)++;
		byte_to_send--;
	}
}


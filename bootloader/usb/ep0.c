/*
 * PIC18F DFU Bootloader
 *
 * Author: Bernd Krumböck <krumboeck@universalnet.at>
 *         Based on Pierre Gaufillet's <pierre.gaufillet@magic.fr> PUF
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

#include "typedef.h"
#include "ep0.h"
#include "debug.h"
#include "usb/usb_descriptors.h"
#include "usb/usb_std_req.h"
#include "usb/usb.h"
#include "dfu/dfu.h"

/* Control Transfer States */
#define WAIT_SETUP          0
#define WAIT_IN             1
#define WAIT_OUT            2
#define WAIT_DFU_IN         3
#define WAIT_DFU_OUT        4

#pragma udata usbram5 SetupBuffer
volatile far StandardRequest SetupBuffer;

#pragma udata usbram5 InBuffer
volatile far u8 InBuffer[EP0_BUFFER_SIZE];

static u8 ep0_state;
static u16 num_bytes_to_be_send;
static u8 *sourceData;
static u8 coming_cfg;

u8 ep0_usb_std_request(void) {
	// hack to avoid register allocation bug in sdcc
	static u8 unknown_request;

	unknown_request = FALSE;

	if (SetupBuffer.request_type != STANDARD) {
		return FALSE;
	}

	switch (SetupBuffer.bRequest) {
	case CLEAR_FEATURE:
		debug_usb("CLEAR_FEATURE\n");
		// TODO not implemented
		break;
	case GET_CONFIGURATION:
		debug_usb("GET_CONFIGURATION\n");
		sourceData = &GET_ACTIVE_CONFIGURATION();
		num_bytes_to_be_send = 1;
		break;
	case GET_DESCRIPTOR:
		debug_usb("GET_DESCRIPTOR\n");
		switch (SetupBuffer.bDescType) {

		case DEVICE_DESCRIPTOR:
			debug_usb("device\n");
			sourceData = (u8 *) device_descriptor;
			num_bytes_to_be_send = device_descriptor->bLength;
			break;
		case CONFIGURATION_DESCRIPTOR:
			debug_usb("configuration\n");
			sourceData = configuration_descriptor[SetupBuffer.bDescIndex];
			num_bytes_to_be_send =
					((USB_Configuration_Descriptor*) sourceData)->wTotalLength;
			break;
		case STRING_DESCRIPTOR:
			debug_usb("string\n");
			sourceData = string_descriptor[SetupBuffer.bDescIndex];
			num_bytes_to_be_send = sourceData[0];
			break;
		default:
			debug_usb("unknown\n");
			// This is required to stall the DEVICE_QUALIFIER request
			unknown_request = TRUE;
			break;
		}
		break;
	case GET_INTERFACE:
		debug_usb("GET_INTERFACE\n");
		// TODO not implemented
		break;
	case GET_STATUS:
		debug_usb("GET_STATUS\n");
		// TODO not implemented
		break;
	case SET_ADDRESS:
		debug_usb("SET_ADDRESS\n");
		SET_DEVICE_STATE(ADDRESS_PENDING_STATE)
		;
		break;
	case SET_CONFIGURATION:
		debug_usb("SET_CONFIGURATION\n");
		// is this configuration valid ?
		if (device_descriptor->bNumConfigurations
				>= SetupBuffer.bConfigurationValue) {
			coming_cfg = SetupBuffer.bConfigurationValue;
			SET_DEVICE_STATE(CONFIGURATION_PENDING_STATE);
		} else // invalid configuration
		{
			debug_usb("invalid configuration\n");
			// Reply with a request error (STALL)
			unknown_request = TRUE;
		}

		break;
	case SET_FEATURE:
		debug_usb("SET_FEATURE\n");
		// TODO not implemented
		break;
	case SET_INTERFACE:
		debug_usb("SET_INTERFACE\n");
		// TODO not implemented
		break;
//        case SYNCH_FRAME:
// only for isochronous synchronization
//            break;
	default:
		unknown_request = TRUE;
		break;
	}
	return !unknown_request;
}

u8 ep0_dfu_request(void) {
	static u8 unknown_request;

	unknown_request = FALSE;

	if (SetupBuffer.request_type != CLASS
			|| SetupBuffer.recipient != RECIPIENT_INTERFACE) {
		return FALSE;
	}

	unknown_request = !process_dfu_request((u8 __data *)&SetupBuffer);

	if (!unknown_request) {
		switch (SetupBuffer.bRequest) {
		/*
		case DFU_DNLOAD:
			sourceData = (u8 __data *)InBuffer;
			num_bytes_to_be_send = SetupBuffer.wLength;
			break;
			*/
		case DFU_GETSTATE:
			sourceData = &(dfu_status.bState);
			num_bytes_to_be_send = 1;
			break;
		case DFU_GETSTATUS:
			sourceData = &dfu_status;
			num_bytes_to_be_send = 6;
			break;
		default:
			break;
		}
	}

	return !unknown_request;
}

void ep0_init(void) {
	debug_usb("ep0_init\r\n");
	ep0_state = WAIT_SETUP;
	EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
	EP_OUT_BD(0).ADR = (u8 __data *)&SetupBuffer;
	EP_OUT_BD(0).Stat.uc = BDS_USIE | BDS_DAT0 | BDS_DTSEN;
	EP_IN_BD(0).Stat.uc = BDS_UCPU;
	UEP0 = EPINEN_EN | EPOUTEN_EN | EPHSHK_EN;
}

void ep0_out(void) {
	if (ep0_state == WAIT_DFU_OUT) {
		process_dfu_data((u8 __data *)InBuffer, SetupBuffer.wLength);
	}
	ep0_state = WAIT_SETUP;
	EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
	EP_OUT_BD(0).ADR = (u8 __data *)&SetupBuffer;
	if (EP_OUT_BD(0).Stat.DTS == 0) {
		EP_OUT_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;
	} else {
		EP_OUT_BD(0).Stat.uc = BDS_USIE | BDS_DAT0 | BDS_DTSEN;
	}
}

void ep0_in(void) {
	debug2_usb("ep0_in %d\n", (u16) num_bytes_to_be_send);
	if (GET_DEVICE_STATE() == ADDRESS_PENDING_STATE) {
		UADDR = SetupBuffer.bAddress;
		if (UADDR != 0) {
			SET_DEVICE_STATE(ADDRESS_STATE);
		} else {
			SET_DEVICE_STATE(DEFAULT_STATE);
		}
	}

	if (ep0_state == WAIT_IN) {
		fill_in_buffer(0, &sourceData, EP0_BUFFER_SIZE, &num_bytes_to_be_send);

		if (EP_IN_BD(0).Stat.DTS == 0) {
			EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;
		} else {
			EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT0 | BDS_DTSEN;
		}
	} else if (ep0_state == WAIT_DFU_IN) {
		fill_in_buffer(0, &sourceData, EP0_BUFFER_SIZE, &num_bytes_to_be_send);

		if (EP_IN_BD(0).Stat.DTS == 0) {
			EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;
		} else {
			EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT0 | BDS_DTSEN;
		}
	} else {
		debug("3\n");
		ep0_init();
	}

	if (GET_DEVICE_STATE() == CONFIGURATION_PENDING_STATE) {

		// First, disable all endpoints.
		// UEP0 is never disabled
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

		// switch the functions vectors
		/*
		 if(coming_cfg <= FLASH_CONFIGURATION)
		 {
		 // switch back to the bootloader vectors
		 ep_init  = boot_ep_init;
		 ep_in    = boot_ep_in;
		 ep_out   = boot_ep_out;
		 ep_setup = boot_ep_setup;
		 }
		 else
		 {
		 // switch to the application vectors
		 ep_init  = application_data.ep_init;
		 ep_in    = application_data.ep_in;
		 ep_out   = application_data.ep_out;
		 ep_setup = application_data.ep_setup;
		 }
		 */
		ep_init = boot_ep_init;
		ep_in = boot_ep_in;
		ep_out = boot_ep_out;
		ep_setup = boot_ep_setup;

		SET_ACTIVE_CONFIGURATION(coming_cfg);

		if (coming_cfg == 0) {
			SET_DEVICE_STATE(ADDRESS_STATE);
		} else {
			static u8 i;

			// Switch to decrement loop because of a sdcc bug
			for (i = 15; i > 0; i--)
//            for(i = 1; i < 16; i++)
					{
				ep_init[coming_cfg][i]();
			}

			SET_DEVICE_STATE(CONFIGURED_STATE);
		}
	}
}

void ep0_setup(void) {
	debug_usb("ep0_setup\n");

	ep0_state = WAIT_SETUP;
	num_bytes_to_be_send = 0;

	if (ep0_usb_std_request()) {
		UCONbits.PKTDIS = 0;
		if (SetupBuffer.data_transfer_direction == DEVICE_TO_HOST) {
			ep0_state = WAIT_IN;

			EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
			EP_OUT_BD(0).ADR = (u8 __data *)&SetupBuffer;
			EP_OUT_BD(0).Stat.uc = BDS_USIE;

			EP_IN_BD(0).ADR = (u8 __data *)InBuffer;
			if (SetupBuffer.wLength < num_bytes_to_be_send) {
				num_bytes_to_be_send = SetupBuffer.wLength;
			} debug2_usb("bytes to send: %d\r\n", num_bytes_to_be_send);
			fill_in_buffer(0, &sourceData, EP0_BUFFER_SIZE,
					&num_bytes_to_be_send);
			EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;

		} else // HOST_TO_DEVICE
		{
			ep0_state = WAIT_OUT;

			EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
			EP_OUT_BD(0).ADR = (u8 __data *)InBuffer;
			EP_OUT_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;

			EP_IN_BD(0).Cnt = 0;
			EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;
		}
	} else if (ep0_dfu_request()) {
		UCONbits.PKTDIS = 0;
		if (SetupBuffer.data_transfer_direction == DEVICE_TO_HOST) {
			ep0_state = WAIT_DFU_IN;

			EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
			EP_OUT_BD(0).ADR = (u8 __data *)&SetupBuffer;
			EP_OUT_BD(0).Stat.uc = BDS_USIE;

			EP_IN_BD(0).ADR = (u8 __data *)InBuffer;
			if (SetupBuffer.wLength < num_bytes_to_be_send) {
				num_bytes_to_be_send = SetupBuffer.wLength;
			} debug2_usb("bytes to send: %d\r\n", num_bytes_to_be_send);
			fill_in_buffer(0, &sourceData, EP0_BUFFER_SIZE,
					&num_bytes_to_be_send);
			EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;

		} else // HOST_TO_DEVICE
		{
			ep0_state = WAIT_DFU_OUT;

			EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
			EP_OUT_BD(0).ADR = (u8 __data *)InBuffer;
			EP_OUT_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;

			EP_IN_BD(0).Cnt = 0;
			EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_DAT1 | BDS_DTSEN;

			// printf("C%d\r\n", InBuffer[0]);
			// debug2("Address: %08x\r\n", (u8 __data *)InBuffer + 1);

		}
	} else {
		debug_usb("unknown request\n");
		UCONbits.PKTDIS = 0;
		EP_OUT_BD(0).Cnt = EP0_BUFFER_SIZE;
		EP_OUT_BD(0).ADR = (u8 __data *)&SetupBuffer;
		EP_OUT_BD(0).Stat.uc = BDS_USIE | BDS_BSTALL;

		EP_IN_BD(0).Stat.uc = BDS_USIE | BDS_BSTALL;
	}
}


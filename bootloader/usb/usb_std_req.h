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

#ifndef USB_STD_REQ_H_
#define USB_STD_REQ_H_

#include "config.h"

/* bmRequestType Definitions */
#define HOST_TO_DEVICE      0
#define DEVICE_TO_HOST      1

#define STANDARD            0x00
#define CLASS               0x01
#define VENDOR              0x02

#define RECIPIENT_DEVICE    0
#define RECIPIENT_INTERFACE 1
#define RECIPIENT_ENDPOINT  2
#define RECIPIENT_OTHER     3

/******************************************************************************
 * bRequest Standard Device Requests
 * USB 2.0 Spec Ref Table 9-4
 *****************************************************************************/
#define GET_STATUS        0
#define CLEAR_FEATURE     1
#define SET_FEATURE       3
#define SET_ADDRESS       5
#define GET_DESCRIPTOR    6
#define SET_DESCRIPTOR    7
#define GET_CONFIGURATION 8
#define SET_CONFIGURATION 9
#define GET_INTERFACE     10
#define SET_INTERFACE     11
#define SYNCH_FRAME       12

typedef union {
	/* Buffer as a char array */
	struct {
		u8 uc[EP0_BUFFER_SIZE];
	};

	/* Standard Device Requests */
	struct {
		u8 bmRequestType;
		u8 bRequest;
		u16 wValue;
		u16 wIndex;
		u16 wLength;
	};
	struct {
		unsigned recipient :5;             // Device, Interface, Endpoint, Other
		unsigned request_type :2;            // Standard, Class, Vendor
		unsigned data_transfer_direction :1; // Host-to-device, Device-to-host
		unsigned :8;
		unsigned :8;
		unsigned :8;
		unsigned :8;
		unsigned :8;
		unsigned :8;
		unsigned :8;
	};

	// Get Descriptor
	struct {
		unsigned :8;
		unsigned :8;
		u8 bDescIndex;                // For Configuration and String DSC Only
		u8 bDescType;                 // Device, Configuration, String,
		u16 wLanguageID;                // Language ID
		unsigned :8;
		unsigned :8;
	};

	// Set Address
	struct {
		unsigned :8;
		unsigned :8;
		u8 bAddress;                   // Device Address (0 to 127)
		unsigned :8;             // Should be Zero (behavior unspecified if not)
		unsigned :8;
		unsigned :8;
		unsigned :8;
		unsigned :8;
	};

	// Set Configuration
	struct {
		unsigned :8;
		unsigned :8;
		u8 bConfigurationValue;       // Configuration Value
		unsigned :8;
		unsigned :8;
		unsigned :8;
		unsigned :8;
		unsigned :8;
	};

	// Set Interface
	struct {
		unsigned :8;
		unsigned :8;
		u8 bLSBAlternateSetting;      // Alternate Setting LSB
		u8 bMSBAlternateSetting;      // Alternate Setting MSB
		u8 bLSBInterface;             // Interface Number LSB
		u8 bMSBInterface;             // Interface Number MSB
		unsigned :8;
		unsigned :8;
	};

} StandardRequest;

#endif /*USB_STD_REQ_H_*/

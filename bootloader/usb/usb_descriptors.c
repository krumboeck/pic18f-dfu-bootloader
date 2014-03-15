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

#include "typedef.h"
#include "usb/usb_descriptors.h"
#include "usb/ep0.h"

const USB_Device_Descriptor boot_device_descriptor = {
		sizeof(USB_Device_Descriptor),    // Size of this descriptor in bytes
		DEVICE_DESCRIPTOR,                // Device descriptor type
		0x0100,                 // USB Spec Release Number in BCD format
		0x00,                   // Class Code
		0x00,                   // Subclass code
		0x00,                   // Protocol code
		EP0_BUFFER_SIZE,        // Max packet size for EP0
		0x0483,                 // Vendor ID
		0xdf11,                 // Product ID
		0x011a,                 // Device release number in BCD format
		1,                      // Manufacturer string index
		2,                      // Product string index
		3,                      // Device serial number string index
		1                       // Number of possible configurations
		};

/* Configurations Descriptor */
const USB_Default_Composite_Descriptor boot_default_cfg = {
// Configuration Descriptor
		{ sizeof(USB_Configuration_Descriptor), // Size of this descriptor in bytes
				CONFIGURATION_DESCRIPTOR,       // CONFIGURATION descriptor type
				sizeof(boot_default_cfg), // Total length of data for this configuration
				1,                 // Number of interfaces in this configuration
				1,                      // Index value of this configuration
				4,                      // Configuration string index
				DEFAULT,                // Attributes
				50 },                    // Max power consumption (2X mA)

		// Boot Interface Descriptor
		{ sizeof(USB_Interface_Descriptor),  // Size of this descriptor in bytes
				INTERFACE_DESCRIPTOR,               // Interface descriptor type
				0,                      // Interface Number
				0,                      // Alternate Setting Number
				0,                      // Number of endpoints in this interface
				0xfe,                   // Class code
				0x01,                   // Subclass code
				0x02,                   // Protocol code
				5 },                     // Interface string index

		{ sizeof(DFU_Functional_Descriptor), // Size of this descriptor in bytes
				DFU_INTERFACE_DESCRIPTOR,       // DFU Interface descriptor type
				0x0b,  // bmAttributes: bitCanDnload | bitCanUpload | willDetach
				0x00ff,                             // Detach timeout in ms: 255
				0x0040,                           // Transfersize in bytes: 64
				0x011a },                              // DFU Version. 1.1a
		};

const u8 * const boot_configuration_descriptor[] = {
		(const u8*) &boot_default_cfg };

/* String descriptors */
/* Language desriptors (Unicode 3.0 (UTF-16) */
const u8 str0[] = {sizeof(str0),  STRING_DESCRIPTOR, 0x09,0x04};// french = 0x040c, english = 0x409

const u8 str1[] = {sizeof(str1),  STRING_DESCRIPTOR,
                                              'K',0x00,
                                              'r',0x00,
                                              'u',0x00,
                                              'm',0x00,
                                              'b',0x00,
                                              'o',0x00,
                                              'e',0x00,
                                              'c',0x00,
                                              'k',0x00,
                                              ' ',0x00,
                                              'B',0x00,
                                              'e',0x00,
                                              'r',0x00,
                                              'n',0x00,
                                              'd',0x00};

const u8 str2[] = {sizeof(str2),  STRING_DESCRIPTOR,
                                              'D',0x00,
                                              'F',0x00,
                                              'U',0x00,
                                              '-',0x00,
                                              'B',0x00,
                                              'o',0x00,
                                              'o',0x00,
                                              't',0x00,
                                              'l',0x00,
                                              'o',0x00,
                                              'a',0x00,
                                              'd',0x00,
                                              'e',0x00,
                                              'r',0x00};

const u8 str3[] = {sizeof(str3), STRING_DESCRIPTOR, '1',0x00};

const u8 str4[] = {sizeof(str4),  STRING_DESCRIPTOR,
                                              'D',0x00,
                                              'e',0x00,
                                              'f',0x00,
                                              'a',0x00,
                                              'u',0x00,
                                              'l',0x00,
                                              't',0x00};

const u8 str5[] = {sizeof(str5),  STRING_DESCRIPTOR,
                                              '@',0x00,
                                              'I',0x00,
                                              'n',0x00,
                                              't',0x00,
                                              'e',0x00,
                                              'r',0x00,
                                              'n',0x00,
                                              'a',0x00,
                                              'l',0x00,
                                              ' ',0x00,
                                              'F',0x00,
                                              'l',0x00,
                                              'a',0x00,
                                              's',0x00,
                                              'h',0x00,
                                              ' ',0x00,
                                              '0',0x00,
                                              '/',0x00,
                                              '0',0x00,
                                              'x',0x00,
                                              '4',0x00,
                                              '0',0x00,
                                              '0',0x00,
                                              '0',0x00,
                                              '/',0x00,
                                              '3',0x00,
                                              '2',0x00,
                                              '0',0x00,
                                              '*',0x00,
                                              '0',0x00,
                                              '6',0x00,
                                              '4',0x00,
                                              'B',0x00,
                                              'g',0x00};

const u8 * const boot_string_descriptor[] = {str0, str1, str2, str3, str4, str5};

/******************************************************************************
 * USB Endpoints callbacks
 *****************************************************************************/
void null_function() __naked
{
    __asm
        return
    __endasm;
}

static void (* const boot_ep_init_cfg0 [])(void) = {
                                        ep0_init,     // 0
                                        null_function, // 1
                                        null_function, // 2
                                        null_function, // 3
                                        null_function, // 4
                                        null_function, // 5
                                        null_function, // 6
                                        null_function, // 7
                                        null_function, // 8
                                        null_function, // 9
                                        null_function, // 10
                                        null_function, // 11
                                        null_function, // 12
                                        null_function, // 13
                                        null_function, // 14
                                        null_function};
static void (* const boot_ep_init_cfg1 [])(void) = {
                                        ep0_init,      // 0
                                        null_function, // 1
                                        null_function, // 2
                                        null_function, // 3
                                        null_function, // 4
                                        null_function, // 5
                                        null_function, // 6
                                        null_function, // 7
                                        null_function, // 8
                                        null_function, // 9
                                        null_function, // 10
                                        null_function, // 11
                                        null_function, // 12
                                        null_function, // 13
                                        null_function, // 14
                                        null_function};// 15

/*
 *  boot_ep_init_cfg0 is duplicated so it is used on cfg 0 (device not configured)
 * and 1 (device configured)
 */

void (** const boot_ep_init[])(void) = {
                                         boot_ep_init_cfg0,
                                         boot_ep_init_cfg0,
                                         boot_ep_init_cfg1
                                       };

static void (* const boot_ep_in_cfg0 [])(void) = {
                                        ep0_in,       // 0
                                        null_function, // 1
                                        null_function, // 2
                                        null_function, // 3
                                        null_function, // 4
                                        null_function, // 5
                                        null_function, // 6
                                        null_function, // 7
                                        null_function, // 8
                                        null_function, // 9
                                        null_function, // 10
                                        null_function, // 11
                                        null_function, // 12
                                        null_function, // 13
                                        null_function, // 14
                                        null_function};// 15

static void (* const boot_ep_in_cfg1 [])(void) = {
                                        ep0_in,       // 0
                                        null_function, // 1
                                        null_function, // 2
                                        null_function, // 3
                                        null_function, // 4
                                        null_function, // 5
                                        null_function, // 6
                                        null_function, // 7
                                        null_function, // 8
                                        null_function, // 9
                                        null_function, // 10
                                        null_function, // 11
                                        null_function, // 12
                                        null_function, // 13
                                        null_function, // 14
                                        null_function};// 15

void (** const boot_ep_in[])(void) =   {
                                        boot_ep_in_cfg0,
                                        boot_ep_in_cfg0,
                                        boot_ep_in_cfg1
                                       };

static void (* const boot_ep_out_cfg0 [])(void) = {
                                        ep0_out,      // 0
                                        null_function, // 1
                                        null_function, // 2
                                        null_function, // 3
                                        null_function, // 4
                                        null_function, // 5
                                        null_function, // 6
                                        null_function, // 7
                                        null_function, // 8
                                        null_function, // 9
                                        null_function, // 10
                                        null_function, // 11
                                        null_function, // 12
                                        null_function, // 13
                                        null_function, // 14
                                        null_function};// 15

static void (* const boot_ep_out_cfg1 [])(void) = {
                                        ep0_out,      // 0
                                        null_function, // 1
                                        null_function, // 2
                                        null_function, // 3
                                        null_function, // 4
                                        null_function, // 5
                                        null_function, // 6
                                        null_function, // 7
                                        null_function, // 8
                                        null_function, // 9
                                        null_function, // 10
                                        null_function, // 11
                                        null_function, // 12
                                        null_function, // 13
                                        null_function, // 14
                                        null_function};// 15

void (** const boot_ep_out[])(void) =  {
                                        boot_ep_out_cfg0,
                                        boot_ep_out_cfg0,
                                        boot_ep_out_cfg1
                                       };

static void (* const boot_ep_setup_cfg [])(void) = {
                                        ep0_setup,     // 0
                                        null_function, // 1
                                        null_function, // 2
                                        null_function, // 3
                                        null_function, // 4
                                        null_function, // 5
                                        null_function, // 6
                                        null_function, // 7
                                        null_function, // 8
                                        null_function, // 9
                                        null_function, // 10
                                        null_function, // 11
                                        null_function, // 12
                                        null_function, // 13
                                        null_function, // 14
                                        null_function};// 15

void (** const boot_ep_setup[])(void) = {
                                         boot_ep_setup_cfg,
                                         boot_ep_setup_cfg,
                                         boot_ep_setup_cfg
                                        };


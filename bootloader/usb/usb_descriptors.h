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

/* Descriptor Types */
#define DEVICE_DESCRIPTOR        0x01
#define CONFIGURATION_DESCRIPTOR 0x02
#define STRING_DESCRIPTOR        0x03
#define INTERFACE_DESCRIPTOR     0x04
#define ENDPOINT_DESCRIPTOR      0x05
#define DFU_INTERFACE_DESCRIPTOR 0x21
#define MICROSOFT_OS_DESCRIPTOR  0xee

/* Configuration Descriptor */

/* Configuration Attributes */
#define DEFAULT               (0x01<<7)         // Default value (bit 7 has to be set)
#define SELF_POWERED          (0x01<<6)         // Self-powered
#define REMOTE_WAKE_UP        (0x01<<5)         // Remote wakeup

/* Endpoint Descriptor */

/* bmAttributes : Endpoint Transfer Type */
#define CONTROL          0x00            // Control Transfer
#define ISOCHRONOUS      0x01            // Isochronous Transfer
#define BULK             0x02            // Bulk Transfer
#define INTERRUPT        0x03            // Interrupt Transfer

/* bEndpointAddress : Endpoint number */
#define EP(n) (n & 0x0f)

/* bEndpointAddress : direction */
#define IN_EP  0x80
#define OUT_EP 0x00

/******************************************************************************
 * DFU Functional Descriptor
 *****************************************************************************/
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bmAttributes;
	u16 wDetachTimeOut;
	u16 wTransferSize;
	u16 bcdDFUVersion;
} DFU_Functional_Descriptor;

/******************************************************************************
 * USB Device Descriptor (table 9-8)
 *****************************************************************************/
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize0;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;
} USB_Device_Descriptor;

/******************************************************************************
 * USB Configuration Descriptor (table 9-10)
 *****************************************************************************/
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u16 wTotalLength;
	u8 bNumInterfaces;
	u8 bConfigurationValue;
	u8 iConfiguration;
	u8 bmAttributes;
	u8 bMaxPower;
} USB_Configuration_Descriptor;

/******************************************************************************
 * USB Interface Descriptor (table 9-12)
 *****************************************************************************/
typedef struct _USB_INTF_DSC {
	u8 bLength;
	u8 bDescriptorType;
	u8 bInterfaceNumber;
	u8 bAlternateSetting;
	u8 bNumEndpoints;
	u8 bInterfaceClass;
	u8 bInterfaceSubClass;
	u8 bInterfaceProtocol;
	u8 iInterface;
} USB_Interface_Descriptor;

/******************************************************************************
 * USB Endpoint Descriptor (table 9-13)
 *****************************************************************************/
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bEndpointAddress;
	u8 bmAttributes;
	u16 wMaxPacketSize;
	u8 bInterval;
} USB_Endpoint_Descriptor;

/******************************************************************************
 * USB Composite Descriptors (used for GET_DSC/CFG request)
 *****************************************************************************/
typedef struct {
	USB_Configuration_Descriptor cd;
	USB_Interface_Descriptor i0;
	DFU_Functional_Descriptor fd;
} USB_Default_Composite_Descriptor;

typedef struct {
	USB_Configuration_Descriptor cd;
	USB_Interface_Descriptor i0;
	USB_Endpoint_Descriptor ep_dsc[2];
} USB_Flash_Composite_Descriptor;

/******************************************************************************
 * USB Endpoints callbacks
 *****************************************************************************/

extern const USB_Device_Descriptor boot_device_descriptor;
extern const u8 * const boot_configuration_descriptor[];
extern const u8 * const boot_string_descriptor[];
extern const u8 str0[];
extern const u8 str1[];
extern const u8 str2[];
extern const u8 str3[];

extern void (** const boot_ep_init[])(void);
extern void (** const boot_ep_in[])(void);
extern void (** const boot_ep_out[])(void);
extern void (** const boot_ep_setup[])(void);


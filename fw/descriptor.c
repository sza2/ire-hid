/*
 *  Incremental Rotary Encoder USB HID firmware for Silicon Labs C8051F326
 *  Copyright (C) 2013  sza2
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "descriptor.h"
#include "usb_isr.h"

//-----------------------------------------------------------------------------
// Descriptor Declarations
//-----------------------------------------------------------------------------

SEGMENT_VARIABLE(device_desc, const device_descriptor, SEG_CODE) = {
	18,                     // bLength
	0x01,                   // bDescriptorType
	LE(0x0110),             // bcdUSB
	0x00,                   // bDeviceClass
	0x00,                   // bDeviceSubClass
	0x00,                   // bDeviceProtocol
	EP0_PACKET_SIZE,        // bMaxPacketSize0
	LE(0x10c4),             // idVendor
	LE(0x81ba),             // idProduct
	LE(0x0001),             // bcdDevice
	0x01,                   // iManufacturer
	0x02,                   // iProduct
	0x00,                   // iSerialNumber
	0x01                    // bNumConfigurations
};

SEGMENT_VARIABLE(config_desc, const configuration_descriptor, SEG_CODE) = {
	0x09,                   // Length
	0x02,                   // Type
	LE(0x0022),             // Totallength (= 9+9+9+7)
	0x01,                   // NumInterfaces
	0x01,                   // bConfigurationValue
	0x00,                   // iConfiguration
	0x80,                   // bmAttributes
	0x20                    // MaxPower (in 2mA units)
};

SEGMENT_VARIABLE(interface_desc, const interface_descriptor, SEG_CODE) = {
	0x09,                   // bLength
	0x04,                   // bDescriptorType
	0x00,                   // bInterfaceNumber
	0x00,                   // bAlternateSetting
	0x01,                   // bNumEndpoints
	0x03,                   // bInterfaceClass (3 = HID)
	0x01,                   // bInterfaceSubClass
	0x01,                   // bInterfaceProcotol
	0x00                    // iInterface
};

SEGMENT_VARIABLE(class_desc, const class_descriptor, SEG_CODE) = {
	0x09,                   // bLength
	0x21,                   // bDescriptorType
	LE(0x0101),             // bcdHID
	0x00,                   // bCountryCode
	0x01,                   // bNumDescriptors
	0x22,                   // bDescriptorType
	LE(HID_REPORT_DESCRIPTOR_SIZE)    // wItemLength
};


SEGMENT_VARIABLE(in_ep1_desc, const endpoint_descriptor, SEG_CODE) = {
	0x07,                   // bLength
	0x05,                   // bDescriptorType
	0x81,                   // bEndpointAddress
	0x03,                   // bmAttributes
	LE(EP1_PACKET_SIZE),    // MaxPacketSize (LITTLE ENDIAN)
	10                      // bInterval
};

SEGMENT_VARIABLE(hid_report_desc, const hid_report_descriptor, SEG_CODE) = {
	0x05, 0x01,             // Usage Page (Generic Desktop)
	0x09, 0x06,             // Usage (Keyboard)
	0xa1, 0x01,             // Collection (Application)
	0x05, 0x07,             //     Usage Page (KeyCodes)
	0x19, 0xe0,             //     Usage Minimum (224)
	0x29, 0xe7,             //     Usage Maximum (231)
	0x15, 0x00,             //     Logical Minimum (0)
	0x25, 0x01,             //     Logical Maximum (1)
	0x75, 0x01,             //     Report Size (1)
	0x95, 0x08,             //     Report Count (8)
	0x81, 0x02,             //     Input (Data, Variable, Absolute)
	0x95, 0x01,             //     Report Count (1)
	0x75, 0x08,             //     Report Size (8)
	0x81, 0x01,             //     Input (Constant) for padding
	0x95, 0x05,             //     Report Count (5)
	0x75, 0x01,             //     Report Size (1)
	0x05, 0x08,             //     Usage Page (Page# for LEDs)
	0x19, 0x01,             //     Usage Minimum (1)
	0x29, 0x05,             //     Usage Minimum (5)
	0x91, 0x02,             //     Output (Data, Variable, Absolute)
	0x95, 0x01,             //     Report Count (2)
	0x75, 0x03,             //     Report Size (3)
	0x91, 0x01,             //     Output (Constant)
	0x95, 0x06,             //     Report Count (6)
	0x75, 0x08,             //     Report Size (8)
	0x15, 0x00,             //     Logical Minimum (0)
	0x25, 0x65,             //     Logical Maximum (101)
	0x05, 0x07,             //     Usage Page (KeyCodes)
	0x19, 0x00,             //     Usage Minimum (0)
	0x29, 0x65,             //     Usage Maximum (101)
	0x81, 0x00,             //     Input (Data, Array)
	0xC0                    // End Collection (Application)
};

#define LANGSTRLEN 4

static SEGMENT_VARIABLE(lang_str[LANGSTRLEN], const U8, SEG_CODE) = {
	LANGSTRLEN, 0x03, 0x09, 0x04
};

#define MFRSTRLEN sizeof("METech") * 2

static SEGMENT_VARIABLE(mfr_str[MFRSTRLEN], const U8, SEG_CODE) = {
	MFRSTRLEN, 0x03,
	'M', 0,
	'E', 0,
	'T', 0,
	'e', 0,
	'c', 0,
	'h', 0
};

#define PRODSTRLEN sizeof("Rotary Encoder and Push-button HID") * 2

static SEGMENT_VARIABLE(prod_str[PRODSTRLEN], const U8, SEG_CODE) = {
	PRODSTRLEN, 0x03,
	'R', 0,
	'o', 0,
	't', 0,
	'a', 0,
	'r', 0,
	'y', 0,
	' ', 0,
	'E', 0,
	'n', 0,
	'c', 0,
	'o', 0,
	'd', 0,
	'e', 0,
	'r', 0,
	' ', 0,
	'a', 0,
	'n', 0,
	'd', 0,
	' ', 0,
	'P', 0,
	'u', 0,
	's', 0,
	'h', 0,
	'-', 0,
	'b', 0,
	'u', 0,
	't', 0,
	't', 0,
	'o', 0,
	'n', 0,
	' ', 0,
	'H', 0,
	'I', 0,
	'D', 0
};

SEGMENT_VARIABLE_SEGMENT_POINTER(
		string_desc_table[], U8, const SEG_CODE, const SEG_CODE) = {
	lang_str,
	mfr_str,
	prod_str
};

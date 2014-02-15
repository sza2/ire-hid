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

#ifndef _DESCRIPTOR_H
#define _DESCRIPTOR_H

#include "compiler_defs.h"

#define LE(value)     ((U8)(value)), ((U8)(((U16)(value) >> 8) & 0xff))

typedef struct device_descriptor {
	U8 bLength;             // Size of this Descriptor in Bytes
	U8 bDescriptorType;     // Descriptor Type (=1)
	U8 bcdUsbLsb;
	U8 bcdUsbMsb;
	U8 bDeviceClass;        // Device Class Code
	U8 bDeviceSubClass;     // Device Subclass Code
	U8 bDeviceProtocol;     // Device Protocol Code
	U8 bMaxPacketSize0;     // Maximum Packet Size for EP0
	U8 idVendorLsb;         // Vendor ID
	U8 idVendorMsb;
	U8 idProductLsb;        // Product ID
	U8 idProductMsb;
	U8 bcdDeviceLsb;        // Device Release Number in BCD
	U8 bcdDeviceMsb;
	U8 iManufacturer;       // Index of String Desc for Manufacturer
	U8 iProduct;            // Index of String Desc for Product
	U8 iSerialNumber;       // Index of String Desc for SerNo
	U8 bNumConfigurations;  // Number of possible Configurations
} device_descriptor;

typedef struct configuration_descriptor {
	U8 bLength;             // Size of this Descriptor in Bytes
	U8 bDescriptorType;     // Descriptor Type (=2)
	U8 wTotalLengthLsb;     // Total Length of Data for this Conf^M
	U8 wTotalLengthMsb;
	U8 bNumInterfaces;      // No of Interfaces supported by this Conf
	U8 bConfigurationValue; // Designator Value for *this* Configuration
	U8 iConfiguration;      // Index of String Desc for this Conf
	U8 bmAttributes;        // Configuration Characteristics (see below)
	U8 bMaxPower;           // Max. Power Consumption in this Conf (*2mA)
} configuration_descriptor;

typedef struct interface_descriptor {
	U8 bLength;             // Size of this Descriptor in Bytes
	U8 bDescriptorType;     // Descriptor Type (=4)
	U8 bInterfaceNumber;    // Number of *this* Interface (0..)
	U8 bAlternateSetting;   // Alternative for this Interface (if any)
	U8 bNumEndpoints;       // No of EPs used by this IF (excl. EP0)
	U8 bInterfaceClass;     // Interface Class Code
	U8 bInterfaceSubClass;  // Interface Subclass Code
	U8 bInterfaceProtocol;  // Interface Protocol Code
	U8 iInterface;          // Index of String Desc for this Interface
} interface_descriptor;

typedef struct class_descriptor {
	U8 bLength;             // Size of this Descriptor in Bytes (=9)
	U8 bDescriptorType;     // Descriptor Type (HID=0x21)
	U8 bcdHIDLsb;
	U8 bcdHIDMsb;
	U8 bCountryCode;        // Localized country SEG_CODE
	U8 bNumDescriptors;     // Number of class descriptors to follow
	U8 bReportDescriptorType;       // Report descriptor type (HID=0x22)
	U8 wItemLengthLsb;      // Total length of report descriptor table
	U8 wItemLengthMsb;      // Total length of report descriptor table
} class_descriptor;

typedef struct endpoint_descriptor {
	U8 bLength;             // Size of this Descriptor in Bytes
	U8 bDescriptorType;     // Descriptor Type (=5)
	U8 bEndpointAddress;    // Endpoint Address (Number + Direction)
	U8 bmAttributes;        // Endpoint Attributes (Transfer Type)
	U8 wMaxPacketSizeLsb;   // Max. Endpoint Packet Size^M
	U8 wMaxPacketSizeMsb;
	U8 bInterval;           // Polling Interval (Interrupt) ms
} endpoint_descriptor;

#define HID_REPORT_DESCRIPTOR_SIZE 0x003f

typedef U8 hid_report_descriptor[HID_REPORT_DESCRIPTOR_SIZE];

typedef struct setup_buffer {
	U8 bmRequestType;       // Request recipient, type, and dir.
	U8 bRequest;            // Specific standard request number
	UU16 wValue;            // varies according to request
	UU16 wIndex;            // varies according to request
	UU16 wLength;           // Number of bytes to transfer
} setup_buffer;

extern SEGMENT_VARIABLE(device_desc, const device_descriptor, SEG_CODE);
extern SEGMENT_VARIABLE(config_desc, const configuration_descriptor, SEG_CODE);
extern SEGMENT_VARIABLE(interface_desc, const interface_descriptor, SEG_CODE);
extern SEGMENT_VARIABLE(class_desc, const class_descriptor, SEG_CODE);
extern SEGMENT_VARIABLE(in_ep1_desc, const endpoint_descriptor, SEG_CODE);
extern SEGMENT_VARIABLE(hid_report_desc, const hid_report_descriptor, SEG_CODE);
extern SEGMENT_VARIABLE_SEGMENT_POINTER(
		string_desc_table[], U8, const SEG_CODE, const SEG_CODE);

#endif /* _DESCRIPTOR_H */

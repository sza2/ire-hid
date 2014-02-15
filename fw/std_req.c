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

#include "usb_defs.h"
#include "usb_isr.h"
#include "descriptor.h"
#include "report_handler.h"

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

 // Buffer for current device request information
setup_buffer setup;

U16 data_size;
U16 data_sent;
U8 const *data_ptr;

// Determines current usb device state
U8 usb0_state;

static SEGMENT_VARIABLE(ones_packet[2], const U8, SEG_CODE) = {0x01, 0x00};
static SEGMENT_VARIABLE(zero_packet[2], const U8, SEG_CODE) = {0x00, 0x00};

//-----------------------------------------------------------------------------
// get_status
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change for custom HID designs.
//
// ----------------------------------------------------------------------------
// This routine returns a two byte
// status packet to the host
void get_status(void)
{
	if (setup.wValue.U16 != 0 || setup.wLength.U16 != 2) {
		// If non-zero return length or data
		// length not
		// Equal to 2 then send a stall
		// indicating invalid request
		force_stall();
	}
	// Determine if recipient was device,
	// interface, or EP
	switch(setup.bmRequestType) {
		// If recipient was device
		case OUT_DEVICE:
			if(setup.wIndex.U16 != 0) {
				// Send stall if request is invalid
				force_stall();
			} else {
				// Otherwise send 0x00, indicating bus power
				// and no remote wake-up supported
				data_ptr = (U8 *)&zero_packet;
				data_size = 2;
			}
			break;
		// See if recipient was interface
		case OUT_INTERFACE:
			if(
				(usb0_state != DEV_CONFIGURED) ||
				setup.wIndex.U16 != 0
			) {
				// Only valid if device is configured
				// and non-zero index
				// Otherwise send stall to host
				force_stall ();
			} else {
				// Status packet always returns 0x00
				data_ptr = (U8 *)&zero_packet;
				data_size = 2;
			}
			break;
		// See if recipient was an endpoint
		case OUT_ENDPOINT:
			if(
				(usb0_state != DEV_CONFIGURED) ||
				setup.wIndex.U8[MSB]
			) {
				// Make sure device is configured
				// and index msb = 0x00
				// otherwise return stall to host
				force_stall();
			} else {
				// Handle case if request is directed to EP 1
				if(setup.wIndex.U8[LSB] == IN_EP1) {
					if (ep_status[1] == EP_HALT) {
						// If endpoint is halted,
						// return 0x01,0x00
						data_ptr = (U8 *)&ones_packet;
						data_size = 2;
					} else {
						// Otherwise return 0x00,0x00
						// to indicate endpoint active
						data_ptr = (U8 *)&zero_packet;
						data_size = 2;
					}
				} else {
					// Send stall if unexpected
					// data  encountered
					force_stall();
				}
			}
			break;

		default:
			force_stall();
			break;
	}
	if (ep_status[0] != EP_STALL) {
		// Set serviced setup Packet, Endpoint 0 in transmit mode, and
		// reset data_sent counter
		POLL_WRITE_BYTE(E0CSR, rbSOPRDY);
		ep_status[0] = EP_TX;
		data_sent = 0;
	}
}

//-----------------------------------------------------------------------------
// clear_feature
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
// This routine can clear Halt Endpoint
// features on endpoint 1
void clear_feature()
{
	// Send procedural stall if device isn't configured
	if(
		(usb0_state != DEV_CONFIGURED) ||
		// Or request is made to host(remote wakeup not supported)
		(setup.bmRequestType == IN_DEVICE) ||
		// Or request is made to interface
		(setup.bmRequestType == IN_INTERFACE) ||
		// Or msbs of value or index set to non-zero value
		(setup.wValue.U16 != 0) ||
		// Or data length set to non-zero.
		(setup.wLength.U16 != 0)
	) {
		force_stall();
	} else {
		// Verify that packet was directed at an endpoint
		if(
			(setup.bmRequestType == IN_ENDPOINT)&&
			// The feature selected was HALT_ENDPOINT
			(setup.wValue.U8[LSB] == ENDPOINT_HALT)  &&
			// And that the request was directed at EP 1 in
			(setup.wIndex.U8[LSB] == IN_EP1)
		) {
			// Clear feature endpoint 1 halt
			POLL_WRITE_BYTE(INDEX, 1);

			POLL_WRITE_BYTE(EINCSR1, rbInCLRDT);

			// Set endpoint 1 status back to idle
			ep_status[1] = EP_IDLE;
		} else {
			// Send procedural stall
			force_stall();
		}
	}

	// Reset Index to 0
	POLL_WRITE_BYTE(INDEX, 0);

	if(ep_status[0] != EP_STALL) {
		// Set Serviced Out packet ready and
		// data end to indicate transaction
		// is over
		POLL_WRITE_BYTE(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}

//-----------------------------------------------------------------------------
// set_feature
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
// This routine will set the EP Halt
// feature for endpoint 1
void set_feature(void)
{
	// Make sure device is configured, setup data
	if(
		(usb0_state != DEV_CONFIGURED) ||
		// is all valid and that request is directed at an endpoint
		(setup.bmRequestType == IN_DEVICE) ||
		(setup.bmRequestType == IN_INTERFACE) ||
		(setup.wValue.U16 != 0) ||
		(setup.wLength.U16 != 0)
	) {
		// Otherwise send stall to host
		force_stall();
	} else {
		// Make sure endpoint exists and that halt
		if(
			(setup.bmRequestType == IN_ENDPOINT)&&
			// endpoint feature is selected
			(setup.wValue.U8[LSB] == ENDPOINT_HALT) &&
			(setup.wIndex.U8[LSB] == IN_EP1)
		) {
			// Set feature endpoint 1 halt
			POLL_WRITE_BYTE(INDEX, 1);
			POLL_WRITE_BYTE(EINCSR1, rbInSDSTL);
			ep_status[1] = EP_HALT;
		} else {
			// Send procedural stall
			force_stall();
		}
	}

	POLL_WRITE_BYTE(INDEX, 0);

	if(ep_status[0] != EP_STALL) {
		// Indicate setup packet has been
		// serviced
		POLL_WRITE_BYTE(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}

//-----------------------------------------------------------------------------
// set_address
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
// Set new function address
void set_address(void)
{
	// Request must be directed to device
	if(
		(setup.bmRequestType != IN_DEVICE) ||
		// with index and length set to zero.
		(setup.wIndex.U16 != 0) ||
		(setup.wLength.U16 != 0) ||
		(setup.wValue.U16 == 0x0000) ||
		(setup.wValue.U16 >  0x007f)
	) {
		// Send stall if setup data invalid
		force_stall();
	}

	// Set endpoint zero to update
	// address next status phase
	ep_status[0] = EP_ADDRESS;

	if(setup.wValue.U8[LSB] != 0) {
		// Indicate that device state is now
		// address
		usb0_state = DEV_ADDRESS;
	} else {
		// If new address was 0x00, return
		// device to default state
		usb0_state = DEV_DEFAULT;
	}

	if (ep_status[0] != EP_STALL) {
		// Indicate setup packet has
		// been serviced
		POLL_WRITE_BYTE(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}

//-----------------------------------------------------------------------------
// get_descriptor
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
// This routine sets the data pointer
// and size to correct descriptor and
// sets the endpoint status to transmit
void get_descriptor(void)
{
	// Determine which type of descriptor
	// was requested, and set data ptr and
	// size accordingly
	switch(setup.wValue.U8[MSB]) {
		case DSC_DEVICE:
			data_ptr = (U8 *) &device_desc;
			data_size = device_desc.bLength;
			break;
		case DSC_CONFIG:
			data_ptr = (U8 *)&config_desc;

			// Compiler Specific - The next statement
			// reverses the bytes in the configuration
			// descriptor for the compiler
			data_size = config_desc.wTotalLengthMsb +
				config_desc.wTotalLengthLsb << 8;
			break;
		case DSC_STRING:
			// NOTE: if strings are added to this project
			// the hard-coded
			// value of 2 will need to be increased
			if(setup.wValue.U8[LSB] > 2) {
				// If asking for string that's N/A
				force_stall();
			} else {
				data_ptr = string_desc_table[
					setup.wValue.U8[LSB]
				];

				// Can have a maximum of 255 strings
				data_size = data_ptr[0];
			}
			break;
		case DSC_INTERFACE:
			data_ptr = (U8 *)&interface_desc;
			data_size = interface_desc.bLength;
			break;
		case DSC_ENDPOINT:
			// This example splits endpoint 1 into an
			// IN endpoint and an OUT endpoint
			// In the ...Descriptor.c and ...Descriptor.h files,
			// OUT endpoint 1 is referred to as Endpoint 2.
			if(setup.wValue.U8[LSB] == IN_EP1) {
				data_ptr = (U8 *)&in_ep1_desc;
				data_size = in_ep1_desc.bLength;
			} else {
				force_stall();
			}
			break;
		case DSC_HID:
			// HID Specific (HID class descriptor)
			data_ptr = (U8 *)&class_desc;
			data_size = class_desc.bLength;
			break;
		case DSC_HID_REPORT:
			// HID Specific (HID report descriptor)
			data_ptr = (U8 *)&hid_report_desc;
			data_size = HID_REPORT_DESCRIPTOR_SIZE;
			break;
		default:
			// Send Stall if unsupported request
			force_stall();
			break;
	}

	// Verify that the requested descriptor is valid
	if(
		(setup.wValue.U8[MSB] == DSC_DEVICE) ||
		(setup.wValue.U8[MSB] == DSC_CONFIG) ||
		(setup.wValue.U8[MSB] == DSC_STRING) ||
		(setup.wValue.U8[MSB] == DSC_INTERFACE) ||
		(setup.wValue.U8[MSB] == DSC_ENDPOINT)
	) {
		if(
			(setup.wLength.U8[LSB] < data_size) &&
			(setup.wLength.U8[MSB] == 0)
		) {
			// Send only requested amount of data
			data_size = setup.wLength.U16;
		}
	}
	if(ep_status[0] != EP_STALL) {
		// Make sure endpoint not in stall mode

		// Service setup Packet
		POLL_WRITE_BYTE(E0CSR, rbSOPRDY);

		// Put endpoint in transmit mode
		ep_status[0] = EP_TX;

		// Reset Data Sent counter
		data_sent = 0;
	}
}

//-----------------------------------------------------------------------------
// get_configuration
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
// This routine returns current
// configuration value
void get_configuration(void)
{
	// This request must be directed to the device
	if(
		(setup.bmRequestType != OUT_DEVICE) ||
		// With value word set to zero
		(setup.wValue.U16 !=0) ||
		// And index set to zero
		(setup.wIndex.U16 !=0) ||
		// And setup length set to one
		(setup.wLength.U16 != 1)
	) {
		// Otherwise send a stall to host
		force_stall();
	} else {
		if(usb0_state == DEV_CONFIGURED) {
			// If the device is configured, then
			// return value 0x01 since this software
			// only supports one configuration
			data_ptr = (U8 *)&ones_packet;
			data_size = 1;
		}
		if(usb0_state == DEV_ADDRESS) {
			// If the device is in address state, it
			// is not configured, so return 0x00
			data_ptr = (U8 *)&zero_packet;
			data_size = 1;
		}
	}

	if(ep_status[0] != EP_STALL) {
		// Set Serviced Out Packet bit
		POLL_WRITE_BYTE(E0CSR, rbSOPRDY);

		// Put endpoint into transmit mode
		ep_status[0] = EP_TX;

		// Reset Data Sent counter to zero
		data_sent = 0;
	}
}

//-----------------------------------------------------------------------------
// set_configuration
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
// This routine allows host to change
// current device configuration value
void set_configuration(void)
{
	// Device must be addressed before configured
	if(
		(usb0_state == DEV_DEFAULT) ||
		// and request recipient must be the device
		(setup.bmRequestType != IN_DEVICE) ||
		// the index and length words must be zero
		(setup.wIndex.U16 != 0) ||
		(setup.wLength.U16 != 0) ||
		(setup.wValue.U16 > 1)
	) {
		// This software only supports config = 0,1
		force_stall();
		// Send stall if setup data is invalid
	} else {
		if(setup.wValue.U8[LSB] > 0) {
			// Any positive configuration request
			// results in configuration being set
			// to 1
			usb0_state = DEV_CONFIGURED;

			// Set endpoint status to idle (enabled)
			ep_status[1] = EP_IDLE;

			// Change index to endpoint 1
			// Set DIRSEL to indicate endpoint 1 is IN/OUT
			POLL_WRITE_BYTE(INDEX, 1);
			POLL_WRITE_BYTE(EINCSR2, rbInSPLIT);

			// Set index back to endpoint 0
			POLL_WRITE_BYTE(INDEX, 0);
			handle_in1();
		} else {
			// Unconfigures device by setting state
			usb0_state = DEV_ADDRESS;
			// to address, and changing endpoint  1 and 2
			ep_status[1] = EP_HALT;
		}
	}

	if(ep_status[0] != EP_STALL) {
		POLL_WRITE_BYTE(E0CSR, (rbSOPRDY | rbDATAEND));
		// Indicate setup packet has been
		// serviced
	}
}

//-----------------------------------------------------------------------------
// get_interface
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - Non
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
// This routine returns 0x00, since
// only one interface is supported by
// this firmware
void get_interface(void)
{
	// If device is not configured
	if(
		(usb0_state != DEV_CONFIGURED) ||
		// or recipient is not an interface
		(setup.bmRequestType != OUT_INTERFACE) ||
		// or non-zero value or index fields
		(setup.wValue.U16 != 0) ||
		// or data length not equal to one
		(setup.wIndex.U16 != 0) ||
		(setup.wLength.U16 != 1)
	) {
		// Then return stall due to invalid request
		force_stall();
	} else {
		// Otherwise, return 0x00 to host
		data_ptr = (U8 *)&zero_packet;
		data_size = 1;
	}

	if(ep_status[0] != EP_STALL) {
		// Set Serviced setup packet, put endpoint in transmit mode and reset
		// Data sent counter
		POLL_WRITE_BYTE(E0CSR, rbSOPRDY);
		ep_status[0] = EP_TX;
		data_sent = 0;
	}
}

//-----------------------------------------------------------------------------
// set_interface
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
void set_interface(void)
{
	// Make sure request is directed at interface
	if(
		(setup.bmRequestType != IN_INTERFACE) ||
		// and all other packet values are set to zero
		(setup.wLength.U16 != 0) ||
		(setup.wValue.U16 != 0) ||
		(setup.wIndex.U16 != 0)
	) {
		// Othewise send a stall to host
		force_stall();
	}

	if(ep_status[0] != EP_STALL) {
		POLL_WRITE_BYTE(E0CSR, (rbSOPRDY | rbDATAEND));
		// Indicate setup packet has been serviced
	}
}

//-----------------------------------------------------------------------------
// get_idle
//-----------------------------------------------------------------------------
// Not supported.
//
//-----------------------------------------------------------------------------
void get_idle(void)
{
}

//-----------------------------------------------------------------------------
// set_idle()
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Description: Sets the idle feature on interrupt in endpoint.
//-----------------------------------------------------------------------------
void set_idle(void)
{
	if(ep_status[0] != EP_STALL) {
		// Set serviced setup Packet
		POLL_WRITE_BYTE(E0CSR, (rbSOPRDY | rbDATAEND));
	}
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// get_protocol
//-----------------------------------------------------------------------------
// Not supported.
//
//-----------------------------------------------------------------------------
void get_protocol(void)
{
}

//-----------------------------------------------------------------------------
// set_protocol
//-----------------------------------------------------------------------------
// Not supported.
//
//-----------------------------------------------------------------------------
void set_protocol(void)
{
}

//-----------------------------------------------------------------------------
// get_report()
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Description: Sends a given report type to the host.
//
//-----------------------------------------------------------------------------
void get_report(void)
{
	// call appropriate handler to prepare buffer
	report_handler_in_isr(setup.wValue.U8[LSB]);
	// set data_ptr to buffer used inside Control Endpoint
	data_ptr = in_buffer.Ptr;
	data_size = in_buffer.Length;

	if(ep_status[0] != EP_STALL) {
		// Set serviced setup Packet
		POLL_WRITE_BYTE(E0CSR, rbSOPRDY);

		// Endpoint 0 in transmit mode
		ep_status[0] = EP_TX;

		// Reset data_sent counter
		data_sent = 0;
	}
}

//-----------------------------------------------------------------------------
// set_report()
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Description: Receives a report sent from the host.
//
//-----------------------------------------------------------------------------
void set_report(void)
{
	// prepare buffer for OUT packet
	setup_out_buffer();

	// set data_ptr to buffer
	data_ptr = out_buffer.Ptr;
	data_size = setup.wLength.U16;

	if(ep_status[0] != EP_STALL) {
		// Set serviced setup Packet
		POLL_WRITE_BYTE(E0CSR, rbSOPRDY);

		// Endpoint 0 in transmit mode
		ep_status[0] = EP_RX;

		// Reset data_sent counter
		data_sent = 0;
	}
}

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

#include "usb_isr.h"
#include "usb_defs.h"
#include "descriptor.h"
#include "report_handler.h"
#include "std_req.h"

//-----------------------------------------------------------------------------
// Global Variable Definitions
//-----------------------------------------------------------------------------

// Holds the status for each endpoint
U8 ep_status[3] = {EP_IDLE, EP_HALT, EP_HALT};

//-----------------------------------------------------------------------------
// usb_isr
//-----------------------------------------------------------------------------
//
// Called after any USB type interrupt, this handler determines which type
// of interrupt occurred, and calls the specific routine to handle it.
//
//-----------------------------------------------------------------------------
INTERRUPT(usb_isr, 8)
{
	U8 bCommon, bIn, bOut;

	// Read all interrupt registers
	POLL_READ_BYTE(CMINT, bCommon);

	// this read also clears the register
	POLL_READ_BYTE(IN1INT, bIn);

	POLL_READ_BYTE(OUT1INT, bOut);

	if(bCommon & rbRSUINT) {
		// Handle Resume interrupt
		usb_resume();
	}
	if(bCommon & rbRSTINT) {
		// Handle Reset interrupt
		usb_reset();
	}
	if(bIn & rbEP0) {
		// Handle setup packet receivedor packet transmitted
		// if Endpoint 0 is in transmit mode
		handle_control();
	}
	if(bIn & rbIN1) {
		// Handle In Packet sent, put new data on endpoint 1 fifo
		handle_in1();
	}
	if(bCommon & rbSUSINT) {
		// Handle Suspend interrupt
		usb_suspend();
	}
}

//-----------------------------------------------------------------------------
// usb_resume
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Resume normal USB operation
//
//-----------------------------------------------------------------------------

void usb_resume(void)
{
	volatile S16 k = 0;
	k++;
}

//-----------------------------------------------------------------------------
// usb_suspend
//-----------------------------------------------------------------------------
// Enter suspend mode after suspend signaling is present on the bus
//
void usb_suspend(void)
{
	volatile S16 k = 0;
	k++;
}

//-----------------------------------------------------------------------------
// Support Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// usb_reset
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// - Set state to default
// - Clear Usb Inhibit bit
//
//-----------------------------------------------------------------------------

void usb_reset(void)
{
	// Set device state to default
	usb0_state = DEV_DEFAULT;

	// Clear usb inhibit bit to enable USB suspend detection
	POLL_WRITE_BYTE (POWER, 0x01);

	// Set default Endpoint Status
	ep_status[0] = EP_IDLE;
	ep_status[1] = EP_HALT;
	ep_status[2] = EP_HALT;
}

//-----------------------------------------------------------------------------
// handle_control
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// - Decode Incoming setup requests
// - Load data packets on fifo while in transmit mode
//
//-----------------------------------------------------------------------------

void handle_control(void)
{
	// Temporary storage for EP control register
	U8 ControlReg;

	// Set Index to Endpoint Zero
	POLL_WRITE_BYTE(INDEX, 0);

	// Read control register
	POLL_READ_BYTE(E0CSR, ControlReg);

	// Handle Status Phase of Set Address command
	if(ep_status[0] == EP_ADDRESS) {
		POLL_WRITE_BYTE(FADDR, setup.wValue.U8[LSB]);
		ep_status[0] = EP_IDLE;
	}

	// If last packet was a sent stall, reset STSTL bit and return EP0
	// to idle state
	if(ControlReg & rbSTSTL) {
		POLL_WRITE_BYTE (E0CSR, 0);
		ep_status[0] = EP_IDLE;
		return;
	}

	// If last setup transaction was ended prematurely then set
	if (ControlReg & rbSUEND) {
		POLL_WRITE_BYTE (E0CSR, rbDATAEND);

		// Serviced setup End bit and return EP0
		POLL_WRITE_BYTE (E0CSR, rbSSUEND);

		// to idle state
		ep_status[0] = EP_IDLE;
	}

	// If Endpoint 0 is in idle mode
	if (ep_status[0] == EP_IDLE) {
		// Make sure that EP 0 has an Out Packet
		// ready from host although if EP0
		// is idle, this should always be the
		// case
		if (ControlReg & rbOPRDY) {
			fifo_read (FIFO_EP0, 8, (U8 *)&setup);
			// Get setup Packet off of Fifo,
			// it is currently Big-Endian

#if (MSB == 0)
			setup.wValue.U16 = setup.wValue.U8[MSB] + 256*setup.wValue.U8[LSB];
			setup.wIndex.U16 = setup.wIndex.U8[MSB] + 256*setup.wIndex.U8[LSB];
			setup.wLength.U16 = setup.wLength.U8[MSB] + 256*setup.wLength.U8[LSB];
#endif

			// Intercept HID class-specific requests
			if((setup.bmRequestType & ~0x80) == DSC_HID) {
				switch(setup.bRequest) {
					case GET_REPORT:
						get_report();
						break;
					case SET_REPORT:
						set_report();
						break;
					case GET_IDLE:
						get_idle();
						break;
					case SET_IDLE:
						set_idle();
						break;
					case GET_PROTOCOL:
						get_protocol();
						break;
					case SET_PROTOCOL:
						set_protocol();
						break;
					default:
						// Send stall to host if
						// invalid request
						force_stall();
						break;
				}
			} else {
				// Call correct subroutine to handle
				// each kind of standard request
				switch (setup.bRequest) {
					case GET_STATUS:
						get_status();
						break;
					case CLEAR_FEATURE:
						clear_feature();
						break;
					case SET_FEATURE:
						set_feature();
						break;
					case SET_ADDRESS:
						set_address();
						break;
					case GET_DESCRIPTOR:
						get_descriptor();
						break;
					case GET_CONFIGURATION:
						get_configuration();
						break;
					case SET_CONFIGURATION:
						set_configuration();
						break;
					case GET_INTERFACE:
						get_interface();
						break;
					case SET_INTERFACE:
						set_interface();
						break;
					default:
						// Send stall to host if
						// invalid request
						force_stall ();
						break;
				}
			}
		}
	}

	// See if endpoint should transmit
	if(ep_status[0] == EP_TX) {

		// Don't overwrite last packet
		if(!(ControlReg & rbINPRDY)) {
			// Read control register
			POLL_READ_BYTE (E0CSR, ControlReg);

			// Check to see if setup End or Out Packet received,
			// if so do not put any new data on FIFO
			if(
				(!(ControlReg & rbSUEND)) ||
				(!(ControlReg & rbOPRDY))
			) {
				// Add In Packet ready flag to E0CSR bitmask
				ControlReg = rbINPRDY;
				if(data_size >= EP0_PACKET_SIZE) {
					// Break Data into multiple packets
					// if larger than Max Packet
					fifo_write_isr(
						FIFO_EP0, EP0_PACKET_SIZE,
						(U8 *)data_ptr
					);
					// Advance data pointer
					data_ptr += EP0_PACKET_SIZE;
					// Decrement data size
					data_size -= EP0_PACKET_SIZE;
					// Increment data sent counter
					data_sent += EP0_PACKET_SIZE;
				} else {
					// If data is less than Max Packet size
					// or zero
					fifo_write_isr(
						FIFO_EP0, data_size,
						(U8*)data_ptr);

					// Add Data End bit to bitmask
					ControlReg |= rbDATAEND;

					// Return EP 0 to idle state
					ep_status[0] = EP_IDLE;
				}
				if(data_sent == setup.wLength.U16) {
					// This case exists when the host
					// requests an even multiple of
					// your endpoint zero max packet size,
					// and you need to exit transmit mode
					// without sending a zero length packet

					// Add Data End bit to mask
					ControlReg |= rbDATAEND;

					// Return EP 0 to idle state
					ep_status[0] = EP_IDLE;
				}
				// Write mask to E0CSR
				POLL_WRITE_BYTE(E0CSR, ControlReg);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// handle_in1
//-----------------------------------------------------------------------------
//
// Handler will be entered after the endpoint's buffer has been
// transmitted to the host.  In1_StateMachine is set to Idle, which
// signals the foreground routine send_packet that the Endpoint
// is ready to transmit another packet.
//-----------------------------------------------------------------------------
void handle_in1()
{
	ep_status[1] = EP_IDLE;
}

//-----------------------------------------------------------------------------
// fifo_read
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) BYTE addr : target address
//                2) unsigned int uNumBytes : number of bytes to unload
//                3) BYTE * pData : read data destination
//
// Read from the selected endpoint FIFO
//
//-----------------------------------------------------------------------------
void fifo_read(U8 addr, U16 uNumBytes, U8 *pData)
{
	S16 cnt;

	// Check if >0 bytes requested,
	if(uNumBytes) {
		// Set address
		USB0ADR = (addr);

		// Set auto-read and initiate first read
		USB0ADR |= 0xC0;


		// Unload <NumBytes> from the selected FIFO
		for(cnt = 0; cnt < (uNumBytes); cnt++) {
			// Wait for BUSY->'0' (data ready)
			while(USB0ADR & 0x80);

			// Copy data byte
			pData[cnt] = USB0DAT;
		}

		// Clear auto-read
		USB0ADR = 0;
	}
}

//-----------------------------------------------------------------------------
// Fifo_Write
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) BYTE addr : target address
//                2) unsigned int uNumBytes : number of bytes to unload
//                3) BYTE * pData : location of source data
//
// Write to the selected endpoint FIFO
//
// fifo_write_fg is used for function calls made in the foreground routines,
// and fifo_write_isr is used for calls made in an ISR.

//-----------------------------------------------------------------------------

void fifo_write_fg(U8 addr, U16 uNumBytes, U8 const *pData)
{
	S16 cnt;

	// If >0 bytes requested,
	if(uNumBytes) {
		// Wait for BUSY->'0' (register available)
		while(USB0ADR & 0x80);

		// Set address (mask out bits7-6)
		USB0ADR = (addr);

		// Write <NumBytes> to the selected FIFO
		for(cnt = 0; cnt < uNumBytes; cnt++) {
			USB0DAT = pData[cnt];

			// Wait for BUSY->'0' (data ready)
			while(USB0ADR & 0x80);
		}
	}
}

void fifo_write_isr(U8 addr, U16 uNumBytes, U8 const *pData)
{
	S16 cnt;

	// If > 0 bytes requested
	if(uNumBytes) {
		// Wait for BUSY->'0' (register available)
		while(USB0ADR & 0x80);

		// Set address (mask out bits7-6)
		USB0ADR = (addr);

		// Write <NumBytes> to the selected FIFO
		for(cnt = 0; cnt < uNumBytes; cnt++) {
			USB0DAT = pData[cnt];

			// Wait for BUSY->'0' (data ready)
			while(USB0ADR & 0x80);
		}
	}
}

//-----------------------------------------------------------------------------
// force_stall
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Force a procedural stall to be sent to the host
//
//-----------------------------------------------------------------------------

void force_stall(void)
{
	POLL_WRITE_BYTE(INDEX, 0);

	// Set the send stall bit
	POLL_WRITE_BYTE(E0CSR, rbSDSTL);

	// Put the endpoint in stall status
	ep_status[0] = EP_STALL;
}

//-----------------------------------------------------------------------------
// send_packet
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - Report ID that's used to call the appropriate IN handler
//
// This function can be called by other routines to force an IN packet
// transmit. It takes as an input the Report ID of the packet to be
// transmitted.
//-----------------------------------------------------------------------------

void send_packet(U8 ReportID)
{
	BIT EAState;
	U8 ControlReg;

	EAState = EA;
	EA = 0;

	// Set index to endpoint 1 registers
	POLL_WRITE_BYTE(INDEX, 1);

	// Read contol register for EP 1
	POLL_READ_BYTE(EINCSR1, ControlReg);

	// If endpoint is currently halted,
	if(ep_status[1] == EP_HALT) {
		// send a stall
		POLL_WRITE_BYTE(EINCSR1, rbInSDSTL);
	} else {
		if(ep_status[1] == EP_IDLE) {
			// the state will be updated inside the ISR handler
			ep_status[1] = EP_TX;

			// Clear sent stall if last
			// packet returned a stall
			if(ControlReg & rbInSTSTL) {
				POLL_WRITE_BYTE(EINCSR1, rbInCLRDT);
			}

			// Clear underrun bit if it was set
			if(ControlReg & rbInUNDRUN) {
				POLL_WRITE_BYTE(EINCSR1, 0x00);
			}

			report_handler_in_fg(ReportID);

			// Put new data on Fifo
			fifo_write_fg(
					FIFO_EP1, in_buffer.Length,
					(U8 *)in_buffer.Ptr
			);

			POLL_WRITE_BYTE(EINCSR1, rbInINPRDY);
			// Set In Packet ready bit,
			// indicating fresh data on FIFO 1
		}
	}
	EA = EAState;
}

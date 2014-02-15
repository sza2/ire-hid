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

#include "report_handler.h"

#define IN_VECTORTABLE_SIZE 1
#define OUT_VECTORTABLE_SIZE 1

extern scan_code;


// ****************************************************************************
// Link all Report Handler functions to corresponding Report IDs
// ****************************************************************************

const vector_table_entry SEG_CODE in_vectortable[IN_VECTORTABLE_SIZE] = {
	// FORMAT: Report ID, Report Handler
	{0, in_report}
};

// ----------------------------------------------------------------------------
// Global Variable Declaration
// ----------------------------------------------------------------------------

BufferStructure in_buffer, out_buffer;

// ****************************************************************************
// For Input Reports:
// Point in_buffer.Ptr to the buffer containing the report
// Set in_buffer.Length to the number of bytes that will be
// transmitted.
// ****************************************************************************


void in_report(void)
{
	static U8 in_packet[8];
	in_packet[0] = 0;
	in_packet[1] = 0;
	in_packet[2] = scan_code;
	in_packet[3] = 0;
	in_packet[4] = 0;
	in_packet[5] = 0;
	in_packet[6] = 0;
	in_packet[7] = 0;
	in_buffer.Ptr = in_packet;
	in_buffer.Length = 8;

	scan_code = 0;
}

// ----------------------------------------------------------------------------
// Global Functions
// ----------------------------------------------------------------------------

// ****************************************************************************
// Configure setup_out_buffer
//
// Reminder:
// This function should set out_buffer.Ptr so that it
// points to an array in data space big enough to store
// any output report.
// It should also set out_buffer.Length to the size of
// this buffer.
//
// ****************************************************************************

void setup_out_buffer(void)
{
}

// ----------------------------------------------------------------------------
// ReportHandler_IN...
// ----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - Report ID
//
// These functions match the Report ID passed as a parameter
// to an Input Report Handler.
// the ...FG function is called in the send_packet foreground routine,
// while the ...ISR function is called inside the USB ISR.  A lock
// is set whenever one function is called to prevent a call from the
// other from disrupting the routine.
// However, this should never occur, as interrupts are disabled by send_packet
// before USB operation begins.
// ----------------------------------------------------------------------------

void report_handler_in_isr(U8 report_id) {
	U8 index = 0;
	while(index <= IN_VECTORTABLE_SIZE) {
		// check to see if Report ID passed into function
		// matches the Report ID for this entry in the Vector Table
		if(in_vectortable[index].report_id == report_id) {
			in_vectortable[index].hdlr();
			break;
		}
		// if Report IDs didn't match, increment the index pointer
		index++;
	}
}

void report_handler_in_fg(U8 report_id)
{
	U8 index = 0;
	while(index <= IN_VECTORTABLE_SIZE) {
		// check to see if Report ID passed into function
		// matches the Report ID for this entry in the Vector Table
		if(in_vectortable[index].report_id == report_id) {
			in_vectortable[index].hdlr();
			break;
		}
		// if Report IDs didn't match, increment the index pointer
		index++;
	}
}

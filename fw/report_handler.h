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

#ifndef _UUSB_REPORTHANDLER_H
#define _USB_REPORTHANDLER_H

#include "compiler_defs.h"

typedef struct {
	U8 report_id;
	void(*hdlr)();
} vector_table_entry;

typedef struct {
	U8 Length;
	U8 *Ptr;
} BufferStructure;

extern BufferStructure in_buffer, out_buffer;

void in_report(void);
void setup_out_buffer(void);
void report_handler_in_isr(U8);
void report_handler_in_fg(U8);

#endif /* _USB_REPORTHANDLER_H */

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

#ifndef _USB_STD_REQ_H
#define _USB_STD_REQ_H

#include "compiler_defs.h"

extern setup_buffer setup;             // Buffer for current device request
                                       // information
extern U16 data_size;
extern U16 data_sent;
extern U8 const *data_ptr;
extern U8 usb0_state;                  // Determines current usb device state

// Standard Requests
void get_status(void);
void clear_feature(void);
void set_feature(void);
void set_address(void);
void get_descriptor(void);
void get_configuration(void);
void set_configuration(void);
void get_interface(void);
void set_interface(void);

void get_idle(void);
void set_idle (void);
void get_protocol(void);
void set_protocol(void);

// HID Specific Requests
void get_report(void);
void set_report(void);

#endif /* _USB_STD_REQ_H */

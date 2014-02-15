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

#ifndef _IRE_HID
#define _IRE_HID

#include <C8051F326.h>
#include "compiler_defs.h"

#define SC_KP_MINUS     0x56
#define SC_KP_PLUS      0x57
#define SC_KP_ENTER     0x58
#define SC_KP_DOWN      0x5A
#define SC_KP_UP        0x60

#ifdef SDCC
#define PB P0_2
#define SWRB P0_1
#else
SBIT(SWRB, P0, 1);
SBIT(PB, P0, 2);
#endif

INTERRUPT_PROTO(usb_isr, 8);

#endif /* _IRE_HID */

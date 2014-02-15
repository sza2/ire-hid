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

#include "init.h"
#include "usb_defs.h"

void usb_init(void)
{
	// Force Asynchronous USB Reset
	POLL_WRITE_BYTE(POWER, 0x08);

	// Enable Endpoint 0-1 in interrupts
	POLL_WRITE_BYTE(IN1IE, 0x03);

	// Enable Endpoint 0-1 out interrupts
	POLL_WRITE_BYTE(OUT1IE, 0x02);

	// Enable Reset, Resume, and Suspend interrupts
	POLL_WRITE_BYTE(CMIE, 0x07);

	// Enable transceiver; select full speed
	USB0XCN = 0xE0;

	// Enable clock recovery, single-step mode disabled
	POLL_WRITE_BYTE(CLKREC, 0x89);

	// Enable USB0 Interrupts
	EIE1 |= 0x02;

	// Global Interrupt enable
	EA = 1;

	// Enable USB0 by clearing the USB Inhibit Bit and
	// enable suspend detection
	POLL_WRITE_BYTE(POWER, 0x01);
}

void sysclk_init(void)
{
	int cnt;
	OSCICN |= 0x82;
	CLKMUL = 0x00;

	// Enable clock multiplier
	CLKMUL |= 0x80;

	for(cnt = 0; cnt < 1000; cnt)
		cnt++;

	// Initialize the clock multiplier
	CLKMUL |= 0xC0;

	// Wait for multiplier to lock
	while(!(CLKMUL & 0x20));

	// Use Clock Multiplier/2 as system clock
	CLKSEL = 0x02;
}

void int0_init(void)
{
	// Enable /INT0 interrupt
	EX0 = 1;

	// Clear TMOD.3
	TMOD &= ~0x08;

	// /INT0 is edge sensitive
	TCON |= 0x01;

	IE0 = 0;
}

void system_init(void)
{
	sysclk_init();
	int0_init();
	usb_init();
}

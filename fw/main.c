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

#include "main.h"
#include "usb_isr.h"
#include "report_handler.h"
#include "init.h"

int scan_code = 0;

INTERRUPT(int0_isr, 0)
{
	if(SWRB) {
		scan_code = SC_KP_UP;
	} else {
		scan_code = SC_KP_DOWN;
	}
}

void main(void)
{
	system_init();

	EA = 1;
	while(1) {
		send_packet(0);
		if(!PB) {
			scan_code = SC_KP_ENTER;
		}
	}
}

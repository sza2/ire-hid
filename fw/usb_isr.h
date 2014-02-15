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

#ifndef _USB_ISR_H
#define _USB_ISR_H

#include <C8051F326.h>
#include "compiler_defs.h"

// Define Endpoint Packet Sizes
#define  EP0_PACKET_SIZE        0x40
#define  EP1_PACKET_SIZE        0x0008

// Standard Descriptor Types
#define  DSC_DEVICE             0x01 // Device Descriptor
#define  DSC_CONFIG             0x02 // Configuration Descriptor
#define  DSC_STRING             0x03 // String Descriptor
#define  DSC_INTERFACE          0x04 // Interface Descriptor
#define  DSC_ENDPOINT           0x05 // Endpoint Descriptor

// HID Descriptor Types
#define DSC_HID                 0x21 // HID Class Descriptor
#define DSC_HID_REPORT          0x22 // HID Report Descriptor

// Standard Request Codes
#define  GET_STATUS             0x00 // Code for Get Status
#define  CLEAR_FEATURE          0x01 // Code for Clear Feature
#define  SET_FEATURE            0x03 // Code for Set Feature
#define  SET_ADDRESS            0x05 // Code for Set Address
#define  GET_DESCRIPTOR         0x06 // Code for Get Descriptor
#define  SET_DESCRIPTOR         0x07 // Code for Set Descriptor(not used)
#define  GET_CONFIGURATION      0x08 // Code for Get Configuration
#define  SET_CONFIGURATION      0x09 // Code for Set Configuration
#define  GET_INTERFACE          0x0A // Code for Get Interface
#define  SET_INTERFACE          0x0B // Code for Set Interface
#define  SYNCH_FRAME            0x0C // Code for Synch Frame(not used)

// HID Request Codes
#define GET_REPORT              0x01 // Code for Get Report
#define GET_IDLE                0x02 // Code for Get Idle
#define GET_PROTOCOL            0x03 // Code for Get Protocol
#define SET_REPORT              0x09 // Code for Set Report
#define SET_IDLE                0x0A // Code for Set Idle
#define SET_PROTOCOL            0x0B // Code for Set Protocol

// Define device states
#define  DEV_ATTACHED           0x00 // Device is in Attached State
#define  DEV_POWERED            0x01 // Device is in Powered State
#define  DEV_DEFAULT            0x02 // Device is in Default State
#define  DEV_ADDRESS            0x03 // Device is in Addressed State
#define  DEV_CONFIGURED         0x04 // Device is in Configured State
#define  DEV_SUSPENDED          0x05 // Device is in Suspended State

// Define bmRequestType bitmaps
#define  IN_DEVICE              0x00
#define  OUT_DEVICE             0x80
#define  IN_INTERFACE           0x01
#define  OUT_INTERFACE          0x81
#define  IN_ENDPOINT            0x02
#define  OUT_ENDPOINT           0x82

// Define wIndex bitmaps
#define  IN_EP1                 0x81 // Index values (Set and Clear feature)
#define  OUT_EP1                0x01 // commands for Endpoint_Halt

// Define wValue bitmaps for Standard Feature Selectors
#define  DEVICE_REMOTE_WAKEUP   0x01 // Remote wakeup feature(not used)
#define  ENDPOINT_HALT          0x00 // Endpoint_Halt feature selector

// Define Endpoint States
#define  EP_IDLE                0x00 // This signifies Endpoint Idle State
#define  EP_TX                  0x01 // Endpoint Transmit State
#define  EP_RX                  0x02 // Endpoint Receive State
#define  EP_HALT                0x03 // Endpoint Halt State (return stalls)
#define  EP_STALL               0x04 // Endpoint Stall (send procedural stall
                                     // next status phase)
#define  EP_ADDRESS             0x05 // Endpoint Address (change FADDR during
                                     // next status phase)
#define  EP_GetReport           0x06 // Special Control Endpoint State for
                                     // GetReport HID Request
#define  EP_SetReport           0x07 // Special Control Endpoint State for
                                     // SetReport HID Request

void usb_resume(void);         // resumes USB operation
void usb_suspend(void);        // This routine called when
void usb_reset(void);          // Called after USB bus reset
void handle_control(void);     // Handle setup packet on EP 0
void handle_in1(void);          // used by SetConfiguration in
                                // USB_STD_REQ to initialize
                                // ReadyToTransfer
void fifo_read(U8, U16, U8*);
void fifo_write_fg(U8, U16, U8 const *);
void fifo_write_isr(U8, U16, U8 const *);
void force_stall(void);         // Forces a procedural stall on Endpoint 0
void send_packet(U8);

extern U8 ep_status[];

#endif /* _USB_ISR_H */

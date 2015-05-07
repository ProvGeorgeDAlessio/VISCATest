/*

 visca_consts.h

 Constants for the VISCA camera control library

 * VISCA(tm) Camera Control
 * Copyright (C) 2005 - 2008 MGC Works Inc.
 *
 * Written by John Mazza <maz@mgcworks.com>
 * Based on LibVISCA library for Unix and LibViscaWin 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ****************************************************************************

 */
#pragma once

#define VISCA_MAX_NAME_LEN		30

#define VISCA_DEV_TYPE_CAMERA	1
#define VISCA_DEV_TYPE_VCR		2
#define VISCA_TILT_BASE			3

typedef unsigned long uint32;

// VISCA_DEV_CAPS	Type is a struct to store specifics for 
//					devices we know about.

typedef struct VISCA_DEV_CAPS {
	int	 dev_cap_id;
	int	 dev_type_id;
	int  full_id;
	int  mfg_id;
	int  model_id;
	char model_name [VISCA_MAX_NAME_LEN];
	bool zoom_lens;
	int	 min_zoom;
	int  max_zoom;
	bool pan_tilt;
} VISCA_DEV_CAPS;

// Allow up to 4 cameras
#define VISCA_LIB_MAXCAMS 4

// Send Headers
#define VISCA_COMMAND_HEADER	0x01
#define VISCA_INQUIRY_CHEADER	0x09	//command header
#define VISCA_DATA_HEADER		0x04	//this precedes the actual command string
#define VISCA_PAN_DATA_HEADER	0x06	// this precedes command string for pan/tilt
#define VISCA_SEND_HEADER		0x80	//this plus the device number is the send header
#define VISCA_BROADCAST_HEADER	0x88

#define VISCA_CAM_ADDR			0x01
#define VISCA_NULL				0x00

#define VISCA_IF_CLEAR			'\x88\x01\x00\xFF'	// Entire packet for IF Clear

// Receive Headers
#define VISCA_ERROR_HEADER		0x06		// Sent before error code
#define VISCA_INQUIRY_HEADER	0x05		// Note: Inquiries do NOT return an ACK
#define VISCA_ACK				0x04		// ACK, this is sent on receipt of command
#define VISCA_COMPLETE			0x05		// Sent after command is finished executing
#define VISCA_INQ_COMPLETE		0x50		// Sent as completion code for inquiries

// Error Codes
#define VISCA_LENGTH_ERROR		0x01
#define VISCA_SYNTAX_ERROR		0x02
#define VISCA_BUFFER_FULL		0x03
#define VISCA_CANCELLED			0x04
#define VISCA_NO_SOCKET_ERROR	0x05
#define VISCA_NOT_EXECUTABLE	0x41

// General Constants for Parameter Values
#define VISCA_ON				0x02
#define VISCA_OFF				0x03
#define VISCA_UP				0x02
#define VISCA_DOWN				0x03
#define VISCA_RESET				0x00
#define VISCA_STOP				0x00
#define VISCA_EMPTY				0x00
#define VISCA_TOGGLE			0x10
#define VISCA_MANUAL			0x03
#define VISCA_LENS_INIT			0x01
#define VISCA_COMP_SCAN			0x02
#define VISCA_ENABLE			0x01
#define VIACA_DISABLE			0x00

// "Normal" command headers
// Normal means that the above constants apply as far as they are supported 
// by the hardware (i.e., VISCA_ON can be used to turn these on)
#define VISCA_CAMERA_POWER		0x00 //on off toggle
#define VISCA_RGAIN				0x03 //reset up down direct
#define VISCA_BGAIN				0x04 //reset up don direct
#define VISCA_SHUTTER			0x0A //reset up down direct
#define VISCA_IRIS				0x0B //reset up down direct
#define VISCA_GAIN				0x0C //reset up down direct
#define VISCA_BRIGHTNESS		0x0D //reset up down direct
#define VISCA_BACKLIGHT			0x33 //on off
#define VISCA_SPOTAE			0x59 //on off +
#define VISCA_REVERSE			0x61 //on off
#define VISCA_FREEZE			0x62 //on off
#define VISCA_DISPLAY			0x15 //on off toggle
#define VISCA_DATE_DISPLAY		0x71 //on off
#define VISCA_TIME_DISPLAY		0x72 //on off
#define VISCA_CAM_MUTE			0x75 //on off toggle
#define VISCA_STABILIZER		0x34 //on off 
#define VISCA_AUTOIR			0x51 // use to enable automatic IR
#define VISCA_IR_MODE			0x01 // Turn IR on/off

// Pan/Tilt command headers
#define VISCA_TILT_MAXSPEED_INQ 0x11 // Inquire max tilt speed
#define VISCA_TILT_MODE_INQ		0x10 // Inquire tilt mode
#define VISCA_TILT_POS_INQ		0x12 // Inquire tilt position
#define VISCA_TILT_DRIVE		0x01 // Drive pan/tilt
#define VISCA_TILT_DRIVE_ABS	0x02 // Drive absolute position
#define VISCA_TILT_DRIVE_REL	0x03 // Drive relative position
#define VISCA_TILT_DRIVE_HOME	0x04 // Drive to home position
#define VISCA_TILT_DRIVE_RESET	0x05 // Reset tilt drive

// TEST
#define TEST_VISCA_06			0x06 
#define TEST_VISCA_01			0x01
#define TEST_VISCA_PAN_SLOW		0x08
#define TEST_VISCA_TILT_SLOW	0x08
#define TEST_VISCA_03			0x03


#define VISCA_TILT_DRIVE_RESET	0x05 // Reset tilt drive

// Pan/Tilt general constants
#define VISCA_SPEED_TILT_MAX	0x14 // Max tilt speed
#define VISCA_SPEED_PAN_MAX		0x18 // Max pan speed

// Modes for multi-standard HD cameras
// In FCB-H10, they are stored in register 0x70
#define VISCA_STD_1080I_60		0x01	// Use for 1080i mode, 59.94 fps
#define VISCA_STD_720P_60		0x02	// Use for 720p mode, 59.94 fps
#define VISCA_STD_NTSC_CROP		0x03	// NTSC, Crop image 
#define VISCA_STD_NTSC_SQ		0x04	// NTSC, Squish Image
#define VISCA_STD_1080I_50		0x11	// 1080i mode, 50.0 fps 
#define VISCA_STD_720P_50		0x12	// 720p mode, 50.0 fps
#define	VISCA_STD_PAL_CROP		0x13	// PAL, Crop image
#define VISCA_STD_PAL_SQ		0x14	// PAL, Squish Image
#define VISCA_PAL				0x01	// PAL mode camera
#define VISCA_NTSC				0x00	// NTSC mode camera

// Strange command headers
// "Strange" means that either the VISCA_ON etc. constants do not 
// apply to these, or those parameters are not supported
#define VISCA_AFSENSITIVITY		0x58	//normal low
#define VISCA_INITIALIZE		0x19	//lens, fault correction movement
#define VISCA_CTRL_LOCK			0x17	//off=0x00, on =0x02
#define VISCA_PIC_EFFECT		0x63	//neg art, b & w
#define VISCA_AF_MODE			0x57	//normal interval zoom trigger

// Register command headers
// Used for specific read/write of camera registers
#define VISCA_REGISTER			0x24    // Use to inquire or set a register
#define VISCA_REG_VID_SYS		0x70	// Register to retrieve video type
										// from FCB-H10 and possibly other multi-system
										// cams
// Exposure Compensation
#define VISCA_EXP_COMP_EN		0x3E	// Use for Exp comp on/off
#define VISCA_EXP_COMP_RST		0x0E	// Use for Exp comp reset
#define VISCA_EXP_COMP_DCT		0x4E	// Use to set exp comp directly

//Auto-Exposure constants
#define VISCA_AE				0x39 
#define VISCA_SLOW_SHUTTER		0x5A	// Auto or Manual
#define VISCA_AE_AUTO			0x00
#define VISCA_AE_SHUTTER		0x0A
#define VISCA_AE_IRIS			0x0B
#define VISCA_AE_BRIGHT			0x0D
#define VISCA_AE_MANUAL			0x03
#define VISCA_SLOW_ON			0x02
#define VISCA_SLOW_OFF			0x03

// Manual-Exposure constants
#define VISCA_SHUTTER_DIRECT	0x4A
#define VISCA_IRIS_DIRECT		0x4B
#define VISCA_GAIN_DIRECT		0x4C
#define VISCA_BRIGHT_DIRECT		0x4D

// White Balance Constants
#define VISCA_WB				0x35  
#define VISCA_WB_AUTO			0x00
#define VISCA_WB_INDOOR			0x01
#define VISCA_WB_OUTDOOR		0x02
#define VISCA_WB_ONEPUSH		0x03
#define VISCA_WB_ATW			0x04
#define VISCA_WB_MANUAL			0x05
#define VISCA_WB_ONEPUSH_T1		0x10
#define VISCA_WB_ONEPUSH_T2		0x05

// Zoom Constants
#define VISCA_ZOOM				0x07
#define VISCA_TELE_STANDARD		0x02
#define VISCA_WIDE_STANDARD		0x03
#define VISCA_TELE_VARIABLE		0x20
#define VISCA_WIDE_VARIABLE		0x30
#define VISCA_ZOOM_DIRECT		0x47	// Direct zoom setting goes from 
										// 0x0000 to 0x4000, data length is 4 byes 
										// stored in low-nibble of each byte
										// (i.e, 04 00 00 00 = 4000, 00 00 00 00= 0000)

// Sharpness Aperture Constants
#define VISCA_APERTURE			0x02	// reset up down direct
#define VISCA_APERTURE_DCT		0x42	// Direct goes from 0x00 to 0x0F

//Title constants
#define VISCA_TITLE_SET			0x73	// used along with the func. set_title to set the title
#define VISCA_TITLE				0x74	// used with set_param. to turn display on and off and to clear
#define VISCA_TITLE_ONE			0x00	// step one: set color, blink, position
#define VISCA_TITLE_TWO			0x01	// step two: set first 10 chars
#define VISCA_TITLE_THREE		0x02	// step three: set second 10 chars(max is twenty)
#define VISCA_TITLE_CLEAR		0x00
#define VISCA_NOBLINK			0x00
#define VISCA_BLINK				0x01
#define VISCA_SPACE				0x1B

// Colors: used for setting the color for the title
#define VISCA_WHITE				0x00
#define VISCA_YELLOW			0x01
#define VISCA_VIOLET			0x02
#define VISCA_RED				0x03
#define VISCA_CYAN				0x04
#define VISCA_GREEN				0x05
#define VISCA_BLUE				0x06

// These features require their own functions
#define VISCA_SET_DATETIME		0x70  
#define VISCA_ADDRESS_SET		'\x30\x01'

// INQUIRY CONSTANTS
// "Bulk" inquiries

#define VISCA_INQ_BLK		0x7e	// Use as 3rd & 4th byte of "bulk" inquiries that return 16-byte packets
#define VISCA_INQ_LCS		0x00	// Use as 5th byte of Lens Control System Inquiry
#define VISCA_INQ_CCS		0x01	// Use as 5th byte of Camera Control System Inquiry
#define VISCA_INQ_GEN		0x02	// Use as 5th byte of Other Inquiry 
#define VISCA_INQ_EFQ		0x03	// Use as 5th byte of Enlargement Function Query

#define VISCA_INQ_LONG		0x01	// Flag for long inquiry sent
#define VISCA_INQ_INT		0x00	// Flag for short inquiry sent

// "Specific" Inquiries
#define VISCA_INQ_SHORT		0x04	// Use as 3rd byte of short inquiries
#define VISCA_INQ_IF		0x00	// Use as 3rd byte of interface inquiries
#define VISCA_INQ_TILT		0x06	// Use as 3rd byte of tilt inquiries
#define VISCA_INQ_VERSION	0x02	// Inquire camera type 

#define VISCA_REG_VID_SYS	0x70	// Register to retrieve video type
									// from FCB-H10 and possibly other multi-system cams

// Constants for various camera models
#define VISCA_UNKNOWN		0x0000
#define VISCA_MFG_SONY		0x0020

#define VISCA_FCB_EX980		0x0430
#define VISCA_FCB_EX980P	0x0431
#define VISCA_FCB_EX980S	0x0432
#define VISCA_FCB_EX980SP	0x0433
#define VISCA_FCB_H10		0x044a
#define VISCA_FCB_H11		0x044b
#define VISCA_EVI_D100P		0x0402
#define VISCA_EVI_D30		0x040d

// Names for various models
#define VISCA_NAME_UNKNOWN		"Unknown"
#define VISCA_NAME_MFG_SONY		"Sony"
#define VISCA_NAME_FCB_EX980	"FCB-EX980"
#define VISCA_NAME_FCB_EX980P	"FCB-EX980P"
#define VISCA_NAME_FCB_EX980S	"FCB-EX980S"
#define VISCA_NAME_FCB_EX980SP	"FCB-EX980SP"
#define VISCA_NAME_FCB_H10		"FCB-H10"
#define VISCA_NAME_FCB_H11		"FCB-H11"
#define VISCA_NAME_EVI_D100P	"EVI-D100/P"
#define VISCA_NAME_EVI_D30		"EVI-D30"

// TERMINATOR (this comes at the end of each packet)
#define VISCA_TERMINATOR	0xff

// visca_camera.h

/*
 * VISCA(tm) Camera Control
 * Copyright (C) 2005 - 2008 MGC Works Inc.
 *
 * Written by John Mazza <maz@mgcworks.com>
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

 This file contains definitions of VISCA camera class utilizing multi-threaded
 serial communications and outbound packet queue processing.  Inbound packets 
 are processed asychronously when terminators are recieved.
  
 Serial library (TSERIAL_EVENT) is fully freeware, so no LGPL incompatability
 results from its inclusion in this project.

 All constants are defined visca_consts.h file.

*/

// Header to define camera
#pragma once
#pragma unmanaged

#include <queue>
#include <deque>
#include <list>

// Queue typedefs

#include "Tserial_event.h"
#include <string>

// 18 ms per timer click.
#define timer_interval 18

using namespace std;

typedef deque<char*, allocator<char*> > CHARDEQUE;
typedef queue<char*,CHARDEQUE> CHARQUEUE;

namespace visca_cam
{
	class VISCA_Timer2
	{
		public:
			void Setup();
			void Stop();
			void ProcessQueue(int Iteration);

		private:
			unsigned long idTimerThread;
			HANDLE hTimerThread;
			HANDLE hWaitTimer;	
	};

	// Define our packet helper class

	class VISCA_Packet
	{ 
		protected:
			unsigned char *packet;
			int length;

		public:
			VISCA_Packet()
			{
				length = 0;
				packet = new unsigned char [32];
				packet[0]='\0';
			}

			void NextByte(const unsigned char *);

			unsigned char *get_Packet()
			{
				return packet;
			}

			int get_Length()
			{
				return length;
			}
			void clear();
	};

	// Inbound packet helper class

	class VISCA_inPacket : public VISCA_Packet
	{
	private:		
		unsigned char *data;
		unsigned char sender;
		int type;
		bool parsed;
		void parse();

	public:
		VISCA_inPacket()
		{
			length = 0;
			packet = new unsigned char [32];
			packet [0] = '\0';
			parsed = false;
			type = 0;
		}
		
		unsigned char get_Sender();
		unsigned char get_Type();
		
		void get_Data(char * indat); // Take bytes from inQueue and bang them into an inPacket, then process.  
									 // Triggered when a VISCA_TERMINATOR is recieved. 
	};

    
	///////////////////////////////////////////////////////////////
	//  VISCA_Cam Class
	///////////////////////////////////////////////////////////////
	class VISCA_Cam 
	{
	public:

		VISCA_Cam(char *,int, int, DWORD&); // Constructor	
		Tserial_event *com;    // Serial manager

		// Add cam functions below (ie zoom, set_date, etc.)
		int  cmds_running();			// How many commands are in process
		bool if_clear();				// Clear interface	
		void get_cam_type();			// Get camera type
		bool power_on(int camera);		// turn on
		bool power_off(int camera);		// turn off
		bool lens_init(int camera);		// init lens
		bool comp_scan(int camera);		// init camera
		bool set_wb_auto(int camera);	// Set auto white balance
		bool set_wb_onepush(int camera);	// Set and execute one-push WB
		bool stabilizer(int camera, bool onoff);     // Turn stabilizer on or off
		bool enable_auto_ir(int camera, bool onoff); // Turn auto IR on/off
		bool enable_ir_mode(int camera, bool onoff); // Turn IR on/off 

		// Exposure stuff 
		bool set_auto_slow(int camera, bool onoff);
		bool set_exposure_comp_onoff(int camera, bool onoff);
		bool set_exposure_comp(int camera, int compval);
		bool set_ae_mode(int camera, int ae_mode);
		int  get_ae_mode();

		// Set shutter - only valid in bright, manual, or shutter modes
		bool set_shutter(int camera, int shut_speed);
		bool set_zoom_direct(int zoom,int camera);

		// Set iris - only valid in bright, manual, or iris modes
		bool set_iris(int camera, int iris_setting);
		bool set_gain(int camera, int cam_gain);
		bool reset_bright(int camera);
		bool set_bright(int camera, int cam_bright);
		bool set_sharpness(int camera, int sharp_val);
		int  translate_shutter(int shut_speed);
		double translate_iris(int iris_val);
		int translate_gain(int gain_val);
		double translate_bright_iris(int bright_val);
		int translate_bright_gain(int bright_val);
		bool set_parameter(int camera, const unsigned char parameter, const unsigned char newValue);

		// Pan/Tilt functions
		bool VISCA_Cam::set_pt_home(int camera);
		bool VISCA_Cam::set_pt_relative(int pan, int tilt, int pan_speed, int tilt_speed, int camera);

		// Packet and serial stuff
		bool queue_packet();
		void send_packet();
		int serial_status();
		void update_status(int FullMode);	// Run status update inquiries
			
		// Camera state variables
		int exp_mode;			// Exposure mode  0 = Full Auto, 1 = Aperture 2 = Shutter 3= Manual
		int shut_speed;			// Shutter speed code
		int aperture_code;		// Aperture code
		int gain_code;			// Gain code
		int brt_code;			// Bright code
		int exp_comp_pos;		// Exposure compensation, 0 - F, 7 = normal.
		int zoom_position;		// Zoom Position (0x0000 - 0x4000)
		int zoom_running;		// Zoom running, 0 = false, 1 = true
		int focus_mode;			// Focus mode, 0 = Manual, 1 = Auto
		int focus_position;		// Focus position
		int focus_sensitivity;	// Focus sensitivity, 0 = normal, 1 = low
		int af_mode;			// Focus mode, 0 = Normal, 1 = Interval, 2 = Zoom Trigger
		int af_sensitivity;		// AF Sensitivity, 0 = Normal, 1 = Low
		int af_low_contrast;	// AF Low Contrast Detect, 0 = No, 1 = Yes
		int dz_mode;			// Digital zoom mode, 0 = Combine, 1 = separate
		int focus_near_limit;   // Focus near limit
		int wb_r_gain;			// White balance gain R
		int wb_b_gain;			// White balance gain B
		int wb_mode;			// White balance mode
		int aper_gain;			// Aperture Gain
		int spot_ae;			// Spot AE, 0 = off, 1 = on
		int backlight_enabled;	// Backlight mode, 0 = off, 1 = on
		int exp_comp_enabled;	// Exposure Compensation, 0 = off, 1 = on
		int slow_shut_enabled;	// Slow shutter enabled, 0 = Manual, 1 = Auto
		int auto_icr_enabled;	// IR Automatic mode enabled, 0 = off, 1 = on
		int cam_keylock;		// Keylock, 0 = off, 1 = on
		int cam_power_state;	// Power, 0 = off, 1 = on
		int stabilizer_enabled;	// Image stabilizer, 0 = off, 1 = on
		int icr_on;				// IR Mode, 0 = off, 1 = on
		int image_freeze;		// Image freeze, 0 = off, 1 = on
		int cam_lr_rev;			// Left/Right flip, 0 = off, 1 = on
		int privacy_zone_on;	// Privacy zone enabled, 0 = off, 1 = on
		int image_mute;			// Image mute, 0 = off, 1 = on
		int title_display;		// Title display, 0 = off, 1 = on
		int display_on;			// Display on/off
		int pict_effect_mode;	// Picture effect mode code
		int pict_freeze_on;		// Picture freeze on/off
		int v_phase_code;		// V-Phase, 0 = 0 degree, 1 = 180 degree
		int ext_lock_on;		// External lock, 0 = off, 1 = on
		int digital_zoom_pos;	// Digital zoom position
		int af_act_time;		// AF activation time
		int af_interval;		// AF interval 
		int af_spot_x;			// Spot AF X
		int af_spot_y;			// Spot AF Y
		int alarm_on;			// Alarm mode, 0 = off, 1 = on
		int picture_flip;		// Picture flip mode, 0 = off, 1 = on
		int mem_recall;			// Camera Memory Recall, 1 = Executing, 0 = Stopped
		int spot_ae_x;			// Spot AE X
		int spot_ae_y;			// Spot AE Y
		int gamma_mode;			// Gamma Mode 0, 1, or 2

		// Camera model and capabilities
		int cam_type_id;		// Camera ID code
		string cam_model_name;	// Camera model name
		int ext_lock;			// External lock, 0 = none, 1 = provided
		int cam_memory;			// Camera memory, 0 = none, 1 = provided
		int cam_clock_1;		// Camera clock, 0 = none, 1 = provided
		int cam_icr;			// Camera ICR capable, 0 = none, 1 = provided
		int cam_stabilizer;		// Camera stabilizer capable, 0 = none, 1 = provided
		int cam_system_type;	// Camera system type, 0 = NTSC, 1 = PAL
		int cam_system_mode;	// Camera system mode for HD
		int cam_adv_privacy;	// Advanced privacy, 0 = none, 1 = provided
		int cam_alarm;			// Alarm capabile, 0 = none, 1 = provided
		int cam_picture_flip;	// Picture flip capable, 0 = none, 1 = provided
		int cam_pan_tilt;		// Pan/Tilt capable, 0 = none, 1 = provided

		// Serial handling - called when data comes in/.
		void SerialDatIn (int , char *, int);
		CHARQUEUE inQueue;
		CHARQUEUE outQueue;

	private:
		// command_header() builds initial packet header
		bool command_header(int DevNum);
		bool command_header_tilt(int DevNum);
		bool inquiry_header(int DevNum);

		// Inquiry handlers
		void process_inquiry();
		int process_inq_lcs(unsigned char * pkt);
		int process_inq_ccs(unsigned char * pkt);
		int process_inq_gen(unsigned char * pkt);
		int process_inq_enl(unsigned char * pkt);
		int process_inq_ver(unsigned char * pkt);
		int process_inq_register(unsigned char * pkt);
 
		// set addresses
		bool set_address;

		// Private variables
		int errCode;				// Error code - false indicates serial error.
		char *port;					// COM port name
		VISCA_inPacket *inPacket;	// Inbound packet buffer
		int num_devices;			// How many devices on com port -- not to be used 
		HANDLE VIDH;				// Serial port handle - probably will be unused
		LPDCB commstate;			// For serial status info -- probably not used
		VISCA_Packet *packet;		// Packet to write out
		int pkts_outstanding;		// How many packets are out there.
		int pkts_waiting;			// How many packets are awaiting their turn to go out.
		bool pkt_queueing;			// Are we composing a packet?
		int pkt_timer;				// Update each time we hit the timer to detect timeout conditions. 
									// Reset to zero when we either timeout or send a packet.
		int last_inq_cat;			// Last inquiry category - 0 = normal, 1 = long/block, 2 = tilt, -1 = none
		int last_inq_typ;			// Last inquiry type.  Set to last inquiry type code or -1 for none
		int	last_inq_parm;			// Last inquiry parameter for special tracking.

		// Inquiry thread vars
		unsigned long idInQThread;
		HANDLE hInqThread;
	};
};

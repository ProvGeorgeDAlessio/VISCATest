// visca-MGC.cpp : Defines the entry point for the console application.
//
//
// This is a test application.  VISCA library is in visca_camera.cpp and visca_camera.h
// License: LGPL


#include "stdafx.h"
#include <iostream>
#include "visca_camera.h"

using namespace std;
using namespace visca_cam;

HANDLE hStatThread = NULL;
unsigned long idStatThread = NULL;


int _tmain(int argc, _TCHAR* argv[])

{
	int comPort = 7;
	if (argc > 1) {
		comPort = _ttoi(argv[1]);
	}

	DWORD result;
	bool xx;
	int zoomlevel = 0;
	int irmode = 0;
	int cammod = 0;
	string input;
	VISCA_Timer2 *tm1 = new VISCA_Timer2;
	VISCA_Cam * Cam1 = 0;
	
	// Get the timer going for packets
	tm1->Setup();
	
	
	// First, create the cam objects, and set them to the appropriate ports.
	// NOTE: Speed/parity/etc are defaulted to 9600/8/N/1 inconstructor
	
	if (comPort == 5) {
		Cam1 = new VISCA_Cam("COM5", 1, 1, result);
	}
	if (comPort == 7) {
		Cam1 = new VISCA_Cam("COM7", 1, 1, result);
	}
	if (comPort == 4) {
		Cam1 = new VISCA_Cam("COM4", 1, 1, result);
	}
	 
	

	// If cams exist then get them going.

	Sleep(150);

	//hStatThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StatThread,NULL,0,&idStatThread);

	if (Cam1 != 0) 
	{
		cammod = Cam1->cam_type_id;

		std::cout << "Cam1: Camera Type: " << cammod << " (Name: " << Cam1->cam_model_name << ") Found \n";
	

		// Now, let's send a "lens_init" command to the cameras.
		xx = Cam1->lens_init(1);
		

		// And set them to minimum zoom

		xx = Cam1->set_zoom_direct(0, 1);
		

		// And turn on auto white balance
		xx = Cam1->set_wb_auto(1);
	

		// Loop to prompt for a zoom level.
		// enter -1 to quit.

		while (zoomlevel >= 0)
		{
			cout << "\nEnter Zoom (0 - 16384),\n OR -2 for WB, -3 for SLOW, 10 to turn the camera off, 11 to turn the camera on, -4 for IR, or -1 to quit): ";
			cout << "\nEnter 1 (Left), 2(Right), 3(Up), 4(Down)";
			std::cin >> zoomlevel;

			if (zoomlevel == -3)
			{
				xx = Cam1->set_auto_slow(1, true);
				
				zoomlevel = 0;
			}

			if (zoomlevel >= 0)
			{
				xx = Cam1->set_zoom_direct(zoomlevel, 1);
				
			}
			else if (zoomlevel == -2)
			{
				xx = Cam1->set_wb_onepush(1);
				
				zoomlevel = 0;
			}

			if (zoomlevel == -4)
			{
				zoomlevel = 0;

				if (irmode == 0)
				{
					xx = Cam1->enable_ir_mode(1, true);
					
					irmode = 1;
				}
				else
				{
					xx = Cam1->enable_ir_mode(1, false);
					
					irmode = 0;
				}
			}
			if (zoomlevel == 10)
			{
				Cam1->power_off(1);
			}
			if (zoomlevel == 11)
			{
				Cam1->power_on(1);
			}
			if (zoomlevel == 1)
			{
				Cam1->set_pt_relative(50, 15, 10, 10, 1);
			}
			if (zoomlevel == 2)
			{
				Cam1->set_pt_relative(1046216, 63176, 10, 10, 1);
			}
			if (zoomlevel == 3) 
			{
				Cam1->go_up(1);
			}
		}

		tm1->Stop();

		delete Cam1;
		
		delete tm1;

		return 0;
	}
	else
	{
		std::cout << "ERROR SETTING UP CAMERAS!";
		return 1;
	}
}




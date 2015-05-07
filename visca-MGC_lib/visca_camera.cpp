// visca_camera.cpp

/*
 * VISCA(tm) Camera Control
 * Copyright (C) 2005 - 2008 MGC Works Inc.
 *
 * Written by John Mazza <maz@mgcworks.com>
 * Based very loosely on LibVISCA library for Unix and LibViscaWin 
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
 *

 */

//#define debug
//#define debpkts

#ifdef debug
	#ifndef debpkts
		#define debpkts
	#endif
#endif

#include <iostream>
#include <iomanip>
#include "visca_consts.h"
#include "Tserial_event.h"
#include "visca_camera.h"
#include "visca_devices.h"
#include "conio.h"
#include "Tserial_event.h"

//using namespace System;
using namespace visca_cam;

void * camobjects[VISCA_LIB_MAXCAMS];

// THIS IS THE CALLBACK DISPACHER FOR THE TIMER!!!

void TimerCallback()
{	
	int instance;
	VISCA_Cam* caminst;
	instance = 0;

	while  (!((camobjects[instance])==0))
	{
		caminst = (VISCA_Cam*) camobjects[instance];
		caminst->send_packet();
		instance++;
	}
}

// Callback function to interface between TSerialObject and VISCACam

void SerialEventManager(uint32 object, uint32 event)
{
	char *buffer;
	int   size;
	int   inst;
	Tserial_event *com;
	VISCA_Cam *camobj;

	com = (Tserial_event *) object;
	camobj =  (VISCA_Cam *) com->owner;
	
	if (com!=0)
	{
		inst = com->instance;

		switch(event)
		{
			case  SERIAL_CONNECTED  :
			break;

		case  SERIAL_DISCONNECTED  :
			break;

		case  SERIAL_DATA_SENT  :
			break;

       case  SERIAL_RING       :
			break;

       case  SERIAL_CD_ON      :
			break;

       case  SERIAL_CD_OFF     :
			break;

       case  SERIAL_DATA_ARRIVAL  :
			// Here we need to put RX data into inPacket
			size   = com->getDataInSize();
			buffer = com->getDataInBuffer();
			inst = com->instance;
			camobj->SerialDatIn(size,buffer,inst);
			com->dataHasBeenRead();
			break;
		}
	}
}

void VT2_ThreadDispatch(void * TimerObject)
{
	VISCA_Timer2 * tobj;
	tobj = (VISCA_Timer2*) TimerObject;
	  
	do 
	{
		tobj->ProcessQueue(0);
		Sleep (timer_interval);
	} while (true);
}

void VISCA_Timer2::Setup()
{
	int x = 1;
	// This function starts the timer thread for the queues
	this->idTimerThread = 0;
	this->hWaitTimer = NULL;
	this->hTimerThread = NULL;

	hTimerThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)VT2_ThreadDispatch,LPVOID(this),0,&idTimerThread);
}

void VISCA_Timer2::Stop()
{
	TerminateThread(hTimerThread,0);
}

void VISCA_Timer2::ProcessQueue(int Iteration)
{
	int instance = 0;
	VISCA_Cam* caminst;

	while  (camobjects[instance]!= NULL )
	{
		caminst = (VISCA_Cam*) camobjects[instance];
		caminst->send_packet();
		instance++;
 	}
}   


void VISCA_Cam::SerialDatIn (int size, char *buffer, int inst)
{
	int charval;
	char inchar;
	//  int qs;
	char cv;
	char *ch;
	unsigned char PktType;

	// Called back from SerialEventManager
	
	if (size > 0) // make sure we have something to process!
	{
		charval = (int(unsigned char(buffer[0])));
		inchar = buffer[0];

		#ifdef debug
			std::cout << "OUTSTANDING PACKETS: " << pkts_outstanding <<"\n";
			std::cout << "\n IN: COM" << inst <<": SIZE: " << size << " BUFFER: 0x" << 
			std::setfill('0') << std::setw(2)<< std::setbase(16) << int (unsigned char(inchar))<< "\n";
		#endif

		this->inQueue.push((char *)inchar);

		if (int(unsigned char(inchar)) == 0xFF) 
		{
			#ifdef debug
				std::cout <<" TERMINATOR FOUND!\n";			
			#endif
			
			cv = 0;
			this->inPacket->clear();

			// Here's where we need to process the inQueue!
			while ((!(cv == 0xFF)) && (!(inQueue.empty())))
			{
				cv = (unsigned char) this->inQueue.front();
				ch = &cv;

				this->inQueue.pop();
				this->inPacket->get_Data(ch);
			}

			// We now have the inbound packet into the inPacket.  Get its type.
			
			#ifdef debpkts
				std::cout << "PACKET IN: TYPE: ";
			#endif

			PktType = this->inPacket->get_Type();

			switch (PktType)
			{
				case VISCA_COMPLETE:
					if (pkts_outstanding > 0) pkts_outstanding--;

					#ifdef debpkts
						std::cout << "COMPLETION - PORT COM" ;
					#endif			
					
					break;

				case VISCA_ACK:
					#ifdef debpkts	
						std::cout<<"ACK PACKET - PORT COM";
					#endif			

					break;

				case VISCA_INQ_COMPLETE:
					if (pkts_outstanding > 0) pkts_outstanding--;

					#ifdef debpkts
					  std::cout<<"INQUIRY REPLY PACKET - PORT COM";
					#endif

					// Process the inquiry.
					this->process_inquiry(); 
					break; 

				case VISCA_ERROR_HEADER:
					if (pkts_outstanding > 0) pkts_outstanding--;

					#ifdef debpkts	
						std::cout<< "ERR PACKET - PORT COM" ;
					#endif			
					
					break;

				default:
					#ifdef debpkts
						std::cout << "UNKNOWN PACKET - PORT COM";
					#endif
					break;
			}

			#ifdef debpkts
				std::cout << inst <<": PKT RUNNING = " << pkts_outstanding << " WAITING = "<< pkts_waiting << "\n";
			#endif
		}

		#ifdef debug
			qs = this->inQueue.size();
			std::cout << "QUEUE LENGTH: " << std::setbase(10) << qs << "\n";
		#endif
	}
}

// Higher-level stuff

void VISCA_Packet::NextByte(const unsigned char *toAppend)
{
	if (length < 32) // Watch out we don't overrun packet size!
	{
		packet[length]=*toAppend;
		packet[length+1]=(0x00);

		#ifdef debug
    		std::cout<<"\tPacket[length] 0x"<< std::setfill('0') << std::setw(2) <<std::setbase(16) << (int)packet[length] <<" Packet: " << packet << " length: "<< length<<"\n";
		#endif

	  length++;
	}
}

void VISCA_Packet::clear()
{
	#ifdef debug
		std::cout << "CLEAR PACKET!\n";
	#endif
	
    //Clear the packet data and set length to 0
	length=0;
	packet[length]='\0';
}

void VISCA_inPacket::parse()
{
	sender=(packet[0]/0x0F)- 0x08;

	switch(length)
	{
		case 4: // 4 byte packets indicate errors or inquiry replies

			if (packet[1] == VISCA_INQ_COMPLETE)
			{
				type = VISCA_INQ_COMPLETE;
			}
			else
			{
				switch(packet[1]/0x0F)
				{
					case VISCA_ERROR_HEADER:
					type = VISCA_ERROR_HEADER;
					break;
		
				default:
			    type=0;

				break;
			}
		}
		break;

		// Ack and complete are 3 bytes long

		case 3:
			switch(packet[1]/0x0F)
			{
				case VISCA_ACK:
					type=VISCA_ACK;
					break;

				case VISCA_COMPLETE:
				type=VISCA_COMPLETE;
					break;

				default:
					type=0;
					break;
			}
			break;

		// 5, 7, 8, 10, 11, 15 and 16-byte packets are inquiry responses

		case 5:
		case 7:
		case 8:
		case 10:
		case 11:
		case 15: 
		case 16:
			if (packet[1]== VISCA_INQ_COMPLETE)
			{
				type = VISCA_INQ_COMPLETE;
			}
			break;

		// Type not identified - we don't know what to do.
		default:
			type=0;
			break;
	}
}

unsigned char VISCA_inPacket::get_Sender()
 // Return sending address of inbound packet
{
	if(parsed)
		return sender;
	else
	{
		parse();
		return sender;
	}
}

unsigned char VISCA_inPacket::get_Type()
{
	if(parsed)
		return type;
	else
	{
		this->parse();
		return this->type;
	}
}


void VISCA_inPacket::get_Data(char * indat)
{
	#ifdef debug
		std::cout << "inPacket::getData: IN= "  << indat << "\n";
	#endif
	
	this->NextByte((const unsigned char*)indat);
}


//**************************************************

VISCA_Cam::VISCA_Cam(char *newPort, int devCount, int instNo, DWORD &result)
// This constructor builds the camera.  
{
  // First - init the arrays, pointers, and vars
	#ifdef debug
		std::cout <<"--CONSTRUCTOR: CREATE VISCA_CAM AT: " <<this << "--\n";
	#endif

	pkts_outstanding = 0;
	pkts_waiting = 0;
	pkt_queueing = false;
	pkt_timer = 0;
	exp_mode = 0;
	last_inq_cat = -1;
	zoom_position = 0;
	zoom_running = 0;

	port=(newPort);
	inPacket = new VISCA_inPacket;
	
	packet = new VISCA_Packet;
	pkt_queueing = false;

	// Now setup the connection to the serial port.
	com = new Tserial_event();
	com->owner = this;

	// Here's where we register our object's address for the timer to callback.
	// Big time hack to get around managed/unmanaged code issues.

	camobjects[instNo-1] = this;
   
	#ifdef debug
		std::cout<<"QUEUESIZE: "<<inQueue.size()<<"\n";
		std::cout<<"QUEUE ADDR: " << &inQueue << "\n";
	#endif

  if (com!=0)
  {
		// com object created successfully!
		// Register the callback function and set instance.

		com->instance = instNo;
		com->setManager(SerialEventManager);

		// Now connect to the port and set parameters.
		errCode = com->connect(newPort, 9600, SERIAL_PARITY_NONE, 8, false);
		cam_type_id = 0;

		if (!errCode)
		{
			// We connected okay.
			// Detect cam type
			this->get_cam_type();
		}
		else
		{
		 // No connection possible.  Serious error - disable this cam.
			this->cam_type_id = 0;
		}
	}
	
	else // com was zero -- couldn't create object.  Indicate error condition!	
	{	
		errCode = false;
	}
}

// ********** WRITE PACKET TO SERIAL!!!! **********

void VISCA_Cam::send_packet()
{
	// This should be invoked via a timer thread to send waiting packets out to the cam.

	char sendbuf[33];
	int cpos = 0;			// Contains number of bytes to send.
	int cnt = 0;
	unsigned char ch;
	//	char * cp;
	bool timed_out = false;
	bool ok_sending = false;
	bool term_found = false;
	bool sending_inq = false;		// Set true if sending an inquiry
	bool sending_int_inq = false;	// Set true if sending interface inquiry
	bool sending_long_inq = false;	// Set true if sending a long inq, false otherwise
	bool sending_short_inq = false; // Set true if sending short inquiry
	bool sending_tilt_inq = false;	// Set true if sending pan/tilt inquiry
	int send_inq_code = 0;			// Type of inquiry sent
	int  send_inq_parm = 0;			// Used for special cases where one inq has a parm
	timed_out = false;
	
	// pkt_timer contains time in seconds that we are waiting.  Give up after 15 seconds.

	if ((pkt_timer > 15000) && (pkts_outstanding > 1))
	{
	#ifdef debpkts
		std::cout << "PACKET TIMEOUT ON COM" << com->instance <<" -- RESETTING AND MOVING ON! OUTSTANDING " << pkts_outstanding << "\n";
	#endif

		pkts_outstanding--;
		pkt_timer = 0;
		timed_out = true;
	}

	ch = 0;	
	cpos = 0;
	ok_sending = ((this->pkts_waiting > 0) && (this->pkts_outstanding < 2) && (this->pkt_queueing == false));

	if (ok_sending)  // Check if anything to send and if any other packets are out there now.
	{
		sending_inq = false;		// Assume command packet until we find an inquiry packet.
		sending_long_inq = false;	// And default other inquiry parameters
		send_inq_code = -1;

		pkt_timer = 0;	
		ch = '\x00';
		
		//remove from waiting packets and add to outstanding.
		if (this->pkts_waiting > 0)
		{
			this->pkts_waiting--;
		}
		
		this->pkts_outstanding++;
		
		// loop chars out of queue
		if ((this->outQueue.size()>0) && (cpos < 31))
		{
			// Loop through the outQueue, reading one char at a time until we get a terminator.  
			//Give up after 31 bytes as VISCA packets aren't that long ever!
			// Check outbound packet for inquiries, and set status if necessary

			term_found = false;
			while (!(term_found))
			{
				ch = (unsigned char)this->outQueue.front();
				this->outQueue.pop();
			
				sendbuf[cpos] = (unsigned char) ch;
				
				// Check for inquiry packet

				if ((ch == VISCA_INQUIRY_CHEADER) && (cpos == 1))
				{
					// Second packet character indicates an inquiry.
					sending_inq = true;
					#ifdef debpkts
						std::cout<<"INQUIRY PACKET FOUND \n";
					#endif
				}

				if ((cpos == 2) && sending_inq)
				{
					if (ch == VISCA_INQ_BLK)
					{
						// Sending a long inquiry
						sending_long_inq = true;
						sending_short_inq = false;
						sending_int_inq = false;
						sending_tilt_inq = false;
						#ifdef debpkts
							std::cout<<"  INQ TYPE: Block \n";
						#endif
					};

					if (ch == VISCA_INQ_SHORT)
					{
						// Short inquriy - get type
						sending_long_inq = false;
						sending_int_inq = false;
						sending_short_inq = true;
						sending_tilt_inq = false;

						send_inq_code = (ch);
						#ifdef debpkts
							std::cout<<"  INQ TYPE: Short \n";
						#endif
					}

					if (ch == VISCA_INQ_IF)
					{
						sending_long_inq = false;
						sending_short_inq = false;
						sending_int_inq = true;
						sending_tilt_inq = false;

						#ifdef debpkts
							std::cout<<"  INQ TYPE: Interface\n ";
						#endif
					}

					if (ch == VISCA_INQ_TILT)
					{
						sending_long_inq = false;
						sending_short_inq = false;
						sending_int_inq = false;
						sending_tilt_inq = true;

						#ifdef debpkts
							std::cout<<"  INQ TYPE: Pan/Tilt\n ";
						#endif
					}
				}

				// If we're sending an interface or short inquiry, determine type
				if ((sending_int_inq || sending_short_inq || sending_tilt_inq) && (cpos == 3))
				{
					send_inq_code = (ch);
				}

				// If we're sending a "long inquiry" then figure out type

				if (sending_long_inq && (cpos == 4))
				{
					send_inq_code = (ch);
				}

				// Some short inquiries need a parameter saved, so save it
				if (sending_short_inq && (cpos == 4))
				{
					this->last_inq_parm = ch;
				}

				// Check for terminator

				if (ch == VISCA_TERMINATOR)
				{ 
					#ifdef debug
						std::cout << "TERM FOUND! Queue Pos:" << cpos << " COM " << com->instance << "\n";
					#endif			
					term_found = true;
				}

				sendbuf[cpos+1] = '\x00';
				cpos++;
			}
		}

		// cpos contains number of bytes to send now.  Shoot them out serial port

		if ((cpos > 0) && (cpos < 31))
		{
			#ifdef debpkts
			std::cout<< "PACKET OUT: PORT: COM" << int(com->instance) << ": Bytes: "<< cpos << 
				" PKT RUNNING: " << pkts_outstanding << " WAITING: " << pkts_waiting << " \n";
			#endif
			
			// Update "last inquiry" variables just before sending packet since response can come
			// back really fast and we want to be ready for it!

			if (sending_inq)
			{
				if (sending_long_inq)
				{
					this->last_inq_cat = 1;
				};

				if (sending_short_inq || sending_int_inq)
				{
					this->last_inq_cat = 0;
				};

				if (sending_tilt_inq)
				{ 
					this->last_inq_cat = 2;
				}

				this->last_inq_typ = send_inq_code;
			}

			// Send packet bytes out serial port to camera

			this->com->sendData( (char *) &sendbuf, cpos);	
		}
	}

	else
	{
		// Either nothing to send or we are blocked by outstandings.  

		if (pkts_outstanding > 0)
		// We waited.  Add timer_interval miliseconds to pkt_timer.
		{
			pkt_timer = pkt_timer + timer_interval;
		}
	}
}

int VISCA_Cam::cmds_running()
{
	return pkts_outstanding + pkts_waiting;
}

int VISCA_Cam::serial_status()
{
	return errCode;
}

bool VISCA_Cam::queue_packet()
{	
	char * pkt_txt;
	char pkt_byte;
	int len;
	int cnt;

	if (packet == NULL) return false;

	pkt_txt = (char *) packet->get_Packet();
	len = packet->get_Length();
	
	if (len > 0)
	{
		for (cnt=0;cnt < len; cnt++)
		{
			pkt_byte = (char) pkt_txt[cnt];
			outQueue.push((char*)pkt_byte);
		}
	}

	this->pkt_queueing = false;
	this->pkts_waiting++;
	return true;
}

void VISCA_Cam::process_inquiry()
{
  #ifdef debpkts
	std::cout << "\nProcess Inquiry() called! PORT=" << port  \
		<< " \nPkt Len: " << this->inPacket->get_Length() << \
		" Last Cat: " << this->last_inq_cat << " Last Type: " \
		<< this->last_inq_typ << "\n";
  #endif

	// Copy inPacket to temp variable
	unsigned char * TMP_PKT;
	int IP_LEN = this->inPacket->get_Length(); 

	// Handle Pan/Tilt Inquiries are 5, 11, or 15-bytes
	if ((IP_LEN == 5) || (IP_LEN == 15) || (IP_LEN == 11))
	{
		TMP_PKT = (unsigned char*)malloc(IP_LEN + 1);
		memcpy(TMP_PKT,this->inPacket->get_Packet(),IP_LEN);
		TMP_PKT[IP_LEN] = 0;

		// Dispatch packet to appropriate handler
		if (this->last_inq_typ == VISCA_TILT_MAXSPEED_INQ)
		{
			// Nothing here
		}

		if (this->last_inq_typ == VISCA_TILT_MODE_INQ)
		{
			// Nothing here
		}

		if (this->last_inq_typ == VISCA_TILT_POS_INQ)
		{
			// Nothing here
		}

	}

    // Handle "Block Inquiries 
	if ((IP_LEN == 16) || (IP_LEN == 10) || (IP_LEN == 8))
	{
		// Copy recieved packet to TMP_PKT
		TMP_PKT = (unsigned char*)malloc(IP_LEN + 1);
		memcpy(TMP_PKT,this->inPacket->get_Packet(),IP_LEN);
		TMP_PKT[IP_LEN] = 0;

		// Dispatch packet to appropriate handler

		if (this->last_inq_typ == VISCA_INQ_LCS)
		{
		  this->process_inq_lcs(TMP_PKT);
		}

		if (this->last_inq_typ == VISCA_INQ_CCS)
		{
			this->process_inq_ccs(TMP_PKT);
		}

		if (this->last_inq_typ == VISCA_INQ_GEN)
		{
			this->process_inq_gen(TMP_PKT);
		}

		if (this->last_inq_typ == VISCA_INQ_EFQ)
		{
			this->process_inq_enl(TMP_PKT);
		}
		
		if (this->last_inq_typ == VISCA_INQ_VERSION)
		{
			this->process_inq_ver(TMP_PKT);
		}

		if (this->last_inq_typ == VISCA_REGISTER)
		{
			this->process_inq_register(TMP_PKT);
		}

		free(TMP_PKT);
	}

}

// Specific inquiry handlers here

int VISCA_Cam::process_inq_lcs(unsigned char *pkt)
{
	unsigned char ch;
	int ZOOMVALH = 0;
	int ZOOMVALL = 0;
	int ZOOMPOS = 0;
	int FNL = 0;
	int FPH = 0;
	int FPL = 0;
	int FOCUSPOS = 0;
	int DZOOM = 0;
	int AFMOD = 0;
	int AFSENS = 0;
	int DZOOMON = 0;
	int FOCMODE = 0;
	int LCDET = 0;
	int CMEMREC = 0;
	int FOCEXEC = 0;
	int ZOOMEXEC = 0;

	// Skip first two bytes of packet - our data starts at the third one 
	// Bytes 2 - 5 contain encoded zoom.  Retrieve it.

	ch = *(pkt + 2);
	ZOOMVALL = (ch & 0x0f)<< 12;
	ch = *(pkt + 3);
	ZOOMVALL = ZOOMVALL | ((ch & 0x0f)<< 8);
	ch = *(pkt + 4);
	ZOOMVALL = ZOOMVALL |(ch & 0x0f) << 4;
	ch = *(pkt + 5);
	ZOOMVALL = ZOOMVALL | (ch & 0x0f);
  
	ZOOMPOS = ZOOMVALL;

	// Bytes 6 - 7 is Focus Near Limit
	ch = *(pkt+6);
	FNL = (ch << 4);
	ch = *(pkt+7);
	FNL = FNL | (ch & 0x0f);

	// Bytes 8 - 11 are focus position
	ch = *(pkt+8);
	FPH = (ch << 4);
	ch = *(pkt+9);
	FPH = FPH | (ch & 0x0f);

	ch = *(pkt + 10);
	FPL = (ch << 4);
	ch = *(pkt + 11);
	FPL = FPL | (ch & 0x0f);

	FOCUSPOS = FPH << 8 | FPL;
	// Byte 12 unused
	
	// Byte 13 contains DZoomMode, AFIntervalMode, 
	// AF Sensitivity, DigitalZoom, and FocusMode 

	ch = *(pkt + 13);
  
	DZOOM = ((ch & 0x20) >> 5);
	AFMOD = ((ch & 0x18) >> 3);
	AFSENS = ((ch & 0x04) >> 2);
	DZOOMON = ((ch & 0x02) >> 1);
	FOCMODE = ((ch & 0x01));

	// Byte 14 contains LowContrastDetect, MemRecall, FocusExec, and ZoomExec
	
	ch = *(pkt + 14);

	LCDET = ((ch & 0x08) >> 3);
	CMEMREC = ((ch & 0x04) >> 2);
	FOCEXEC = ((ch & 0x02) >> 1);
	ZOOMEXEC = ((ch & 0x01));

#ifdef debpkts
	
	if (this->com->instance == 1) 
	{
	  cout << "\n INQUIRY: ZOOMEXEC = " << ZOOMEXEC <<" ch (*pkt +14) = 0x" << std::setw(2)<< std::setbase(16) << int(ch) << "\n PACKET = ";
	  int counter = 0;
	  for (counter = 0; counter < 16; counter++)
		  cout << " - "  << std::setfill('0')<< std::setw(2)  << std::setbase(16) << int((unsigned char)*(pkt + counter));
	  cout << "\n";
	}
#endif

	// We've extracted values from packet.  Write to our status variables

	this->zoom_position = ZOOMPOS;
	this->focus_position = FOCUSPOS;
	this->focus_near_limit = FNL;
	this->dz_mode = DZOOM;
	this->af_mode = AFMOD;
	this->af_sensitivity = AFSENS;
	this->focus_mode = FOCMODE;
	this->af_low_contrast = LCDET;
	this->mem_recall = CMEMREC;
	this->zoom_running = ZOOMEXEC;

  return 0;
}

int VISCA_Cam::process_inq_ccs(unsigned char * pkt)
{
	unsigned char ch;
	int RGAIN = 0;
	int BGAIN = 0;
	int WBMODE = 0;
	int STABILIZER = 0;
	int AGAIN = 0;
	int EXPMODE = 0;
	int SPOTAE = 0;
	int BACKLTON = 0;
	int EXPCMPON = 0;
	int AUTOSLOW = 0;
	int SHUTSPD = 0;
	int APERTURE = 0;
	int GAIN = 0;
	int BRIGHT = 0;
	int EXPCMP = 0;

	// Skip first two bytes.  RGAIN is at bytes 2 - 3

	ch = *(pkt+2);
	RGAIN = (ch & 0x0f) << 4;
	ch = *(pkt+3);
	RGAIN = RGAIN | (ch & 0x0f);

	// Stabilizer is bit 6 of byte 3
	STABILIZER = (ch & 0x40) >> 6;

	// BGAIN is bytes 4 - 5
	ch = *(pkt+4);
	BGAIN = (ch & 0x0f) << 4;
	ch = *(pkt+5);
	BGAIN = BGAIN | (ch & 0x0f);

	// WBMODE is in byte 6, lowest 3 bits
	ch = *(pkt+6);
	WBMODE = (ch & 0x07);

	// Aperture Gain is byte 7 lowest 4 bits
	ch = *(pkt+7);
	AGAIN = (ch & 0x0f);
  
	// Exposure mode is byte 8, lowest 5 bits
	ch = *(pkt+8);
	EXPMODE = (ch & 0x1f);

	// Byte 9 encodes SpotAE, BackLightOn, ExposureCompOn, and AutoSlow
	ch = *(pkt+9);
	SPOTAE = (ch & 0x08) >> 3;
	BACKLTON = (ch & 0x04) >> 2;
	EXPCMPON = (ch & 0x02) >> 1;
	AUTOSLOW = (ch & 0x01);

	// Byte 10 is shutter speed (lowest 5 bits)
	ch = *(pkt+10);
	SHUTSPD = (ch & 0x1f);

	// Byte 11 is Iris (low 5 bits)
	ch = *(pkt+11);
	APERTURE = (ch & 0x1f);

	// Byte 12 is Gain, low 4 bits
	ch = *(pkt+12);
	GAIN = (ch & 0x0f);

	// Byte 13 is Bright, low 5 bits
	ch = *(pkt+13);
	BRIGHT = (ch & 0x1f);

	// Byte 14 is Exposure Compensation, low 4 bits

	ch = *(pkt+14);
	EXPCMP = (ch & 0x0f);

	// Update main camera object variables
	this->aperture_code = APERTURE;
	this->aper_gain = AGAIN;
	this->wb_r_gain = RGAIN;
	this->wb_b_gain = BGAIN;
	this->wb_mode = WBMODE;
	this->exp_mode = EXPMODE;
	this->cam_stabilizer = STABILIZER;
	this->spot_ae = SPOTAE;
	this->backlight_enabled = BACKLTON;
	this->exp_comp_enabled = EXPCMPON;
	this->exp_comp_pos = EXPCMP;
	this->brt_code = BRIGHT;
	this->gain_code = GAIN;
	this->slow_shut_enabled = AUTOSLOW;
	this->shut_speed = SHUTSPD;

	return 0;
}	

int VISCA_Cam::process_inq_gen(unsigned char * pkt)
{
	unsigned char ch;
	int AUTOICR = 0;
	int KEYLOCK = 0;
	int POWERON = 0;
	int STABILON = 0;
	int ICRON = 0;
	int FREEZEON = 0;
	int LRR = 0;
	int PRVZONE = 0;
	int VIDMUTE = 0;
	int TITLEDSP = 0;
	int DISPLAYON = 0;
	int PEMODE = 0;
	int CAMERAID = 0;
	int ELOCKCAP = 0;
	int MEMCAP = 0;
	int CLKCAP = 0;
	int ICRCAP = 0;
	int STABCAP = 0;
	int NTSCPAL = 0;
	int VPHASE = 0;
	int ELOCKON = 0;
	int VPHASEA = 0;
	int GAMMODE = 0;

	// Skip bytes 0 and 1
	// Byte 2 has AutoICR, KeyLock, and power
	ch = *(pkt+2);
	AUTOICR = (ch & 0x04) >> 2;
	KEYLOCK = (ch & 0x02) >> 1;
	POWERON = (ch & 0x01);

	// Byte 3 has Stabilizer On, ICR On, Freeze On, and LR Reverse On
	ch = *(pkt+3);
	STABILON = (ch & 0x40) >> 6;
	ICRON = (ch & 0x10) >> 4;
	FREEZEON = (ch & 0x08) >> 3;
	LRR = (ch & 0x04) >> 2;

	// Byte 4 has Privacy On, Mute On, Title Display, and Display On
	ch = *(pkt+4);
	PRVZONE = (ch & 0x20)>>5;
	VIDMUTE = (ch & 0x10)>>4;
	TITLEDSP = (ch & 0x08) >> 3;
	DISPLAYON = (ch & 0x04) >> 2;

	// Byte 5 (low 4 bits) is Picture Effect Mode
	ch = *(pkt+5);
	PEMODE = (ch & 0x0f);

	// Byte 6 (low 2 bits) contains Gamma Mode 
	ch = *(pkt+6);
    GAMMODE = (ch & 0x03);

	// Byte 7 is unused

	// Byte 8 - 11 contains highest camid
	ch = *(pkt+8);
	CAMERAID = (ch & 0x0f) << 12;
	ch = *(pkt+9);
	CAMERAID = CAMERAID | ((ch & 0x0f) << 8);
	ch = *(pkt+10);
	CAMERAID = CAMERAID | ((ch & 0x0f) << 4);
	ch = *(pkt+11);
	CAMERAID = CAMERAID | ((ch & 0x0f));

	// Byte 12 contains Ext Lock Cap, Mem Cap, Clock Cap, ICR Cap, Stabilizer Cap, 
	// and System Type

	ch = *(pkt+12);
	ELOCKCAP = (ch & 0x20) >> 5;
	MEMCAP = (ch & 0x10) >> 4;
	CLKCAP = (ch & 0x08) >> 3;
	ICRCAP = (ch & 0x04) >> 2;
	STABCAP = (ch & 0x02) >> 1;
	NTSCPAL = (ch & 0x01);

	// Byte 13 contains V-Phase high 4 bits, V-PHase 0/180, and ExtLock On

	ch = *(pkt+13);
	VPHASEA = (ch & 0x20) >> 5;
	ELOCKON = (ch & 0x10) >> 4;
	VPHASE = (ch & 0x0f) << 4;

	// Byte 14 contains low 4 bits of V-Phase
	ch = *(pkt+14);
	VPHASE = VPHASE | (ch & 0x0f);

	// Update main VISCA_Cam variables
	this->auto_icr_enabled = AUTOICR;
	this->cam_keylock = KEYLOCK;
	this->cam_power_state = POWERON;
	this->stabilizer_enabled = STABILON;
	this->icr_on = ICRON;
	this->pict_freeze_on = FREEZEON;
	this->image_mute = VIDMUTE;
	this->title_display = TITLEDSP;
	this->display_on = DISPLAYON;
	this->pict_effect_mode = PEMODE;
	this->ext_lock = ELOCKCAP;
	this->cam_memory = MEMCAP;
	this->cam_clock_1 = CLKCAP;
	this->cam_stabilizer = STABCAP;
	this->cam_system_type = NTSCPAL;
	this->v_phase_code = VPHASE;
	this->ext_lock_on = ELOCKON;
	this->gamma_mode = GAMMODE;

	return 0;
}

int VISCA_Cam::process_inq_enl(unsigned char * pkt)
{
	unsigned char ch;
	int DZP = 0; 
	int AFATIME = 0;;
	int AFITIME = 0;
	int SPOTAEX = 0;
	int SPOTAEY = 0;
	int ALARMON = 0;;
	int PICTFLPON = 0;;
	int ADVPRVCAP = 0;;
	int ALARMCAP = 0;
	int PICTFLPCAP = 0;

	// Skip first two bytes.  Data starts in BYTE 2

	// Digital zoom is low-order in bytes 2 - 3
	ch = *(pkt+2);
	DZP = (ch & 0x0f) << 4;
	ch = *(pkt+3);
	DZP = DZP | (ch & 0x0f);

	// AF Activation Time in low order of bytes 4 - 5
	ch = *(pkt+4);
	AFATIME = (ch & 0x0f) << 4;
	ch = *(pkt+5);
	AFATIME = AFATIME | (ch & 0x0f);

	// AF Interval Time in low order of bytes 6 - 7
	ch = *(pkt+6);
	AFITIME = (ch & 0x0f) << 4;
	ch = *(pkt+7);
	AFITIME = AFITIME | (ch & 0x0f);

	// Spot AE X is low order of byte 8
	ch = *(pkt+8);
	SPOTAEX = (ch & 0x0f);

	// Spot AE Y is low order of byte 9
	ch = *(pkt+9);
	SPOTAEY = (ch & 0x0f);

	// Alarm On and Picture Flip On are lowest bits of Byte 10
	ch = *(pkt+10);
	ALARMON = (ch & 0x02) >> 1;
	PICTFLPON = (ch & 0x01);

	// Byte 11 has Adv. Privacy capability, Alarm Capability, and Picture Flip Capability
	ch = *(pkt+11);
	ADVPRVCAP = (ch & 0x04) >> 2;
	ALARMCAP = (ch & 0x02) >> 1;
	PICTFLPCAP = (ch & 0x01);

	// Update VISCA_Cam object variables
	this->af_act_time = AFATIME;
	this->af_interval = AFITIME;
	this->alarm_on = ALARMON;
	this->cam_alarm = ALARMCAP;
	this->picture_flip = PICTFLPON;
	this->cam_picture_flip = PICTFLPCAP;
	this->digital_zoom_pos = DZP;
	this->spot_ae_x = SPOTAEX;
	this->spot_ae_y = SPOTAEY;
	this->cam_adv_privacy = ADVPRVCAP;

	return 0;
}

int VISCA_Cam::process_inq_ver(unsigned char * pkt)
{
    int VENDID = 0;
	int MODELID = 0;

	unsigned char ch;

	cam_pan_tilt = 0;  // Assume no pan/tilt unless specific model allows it.

	// Skip first two bytes
	// Bytes 2 - 3 contain Vendor ID
	ch = *(pkt+2);
	VENDID = (ch << 8);
	ch = *(pkt+3);
	VENDID = VENDID | (ch);

    // Bytes 4 - 5 contain Model ID
	ch = *(pkt+4);
	MODELID = MODELID | (ch << 8);
	ch = *(pkt+5);
	MODELID = MODELID | ch;

	this->cam_type_id = (VENDID << 16) | MODELID;
#ifdef debpkts
	std::cout << "\nCamera detected - type: "<< MODELID <<"\n";
#endif

	if (VENDID == VISCA_MFG_SONY)
	{
		switch (MODELID)
		{
			case VISCA_FCB_EX980:
				cam_model_name.assign(VISCA_NAME_FCB_EX980);
				break;

			case VISCA_FCB_EX980P:
				cam_model_name.assign(VISCA_NAME_FCB_EX980P);
				break;

			case VISCA_FCB_EX980S:
				cam_model_name.append(VISCA_NAME_FCB_EX980S);
				break;

			case VISCA_FCB_EX980SP:
				cam_model_name.assign(VISCA_NAME_FCB_EX980SP);
				break;

			case VISCA_FCB_H10:
				cam_model_name.assign(VISCA_NAME_FCB_H10);
				break;

			case VISCA_FCB_H11:
				cam_model_name.assign(VISCA_NAME_FCB_H11);
				break;

			case VISCA_EVI_D100P:
				cam_model_name.assign(VISCA_NAME_EVI_D100P);
				cam_pan_tilt = 1;
				break;

			default:
				cam_model_name.assign("Unknown");
				cam_pan_tilt = 0;

		}
	}

	return 0;
}

int VISCA_Cam::process_inq_register(unsigned char * pkt)
{
	char ch;
	int  SYSTYPE = 0;

	if (last_inq_parm == VISCA_REG_VID_SYS)
	{
		// Check for FCB-H10 camera or others that use register 0x70 for
		// system mode information.  

		if ((this->cam_type_id & 0xffff) == VISCA_FCB_H10)
		{
			last_inq_parm = -1;
			// Skip first two bytes of packet
			// High nibble is in byte 2
			ch = *(pkt +2);
			SYSTYPE = (ch & 0x0f) << 4;

			// Low nibble is in byte 3
			ch = *(pkt+3);
			SYSTYPE = SYSTYPE | (ch & 0x0f);

			this->cam_system_mode = SYSTYPE;
		}
		else 
		{
			this->cam_system_mode = 0;
		}
	}

	return 0;
}


// Command functions 
bool VISCA_Cam::set_zoom_direct(int zoom,int camera)
{
	// ZOOM is a hex value from 0x0000 through 0x4000
	
	if ((!errCode) && (this->pkt_queueing == false))
	{
		unsigned char p;
		unsigned char q;
		unsigned char r;
		unsigned char s;
		unsigned int zdirect;

		p = 0;
		q = 0;
		r = 0;
		s = 0;

		// This command is an odd one.
		// Zoom value is specified as x0p x0q x0r x0s 
		//   -- where p, q, r, and s are 4-bit nibbles of a 16-bit value 
		//      ranging from 0x0000 to 0x4000  
		//      ex: zoom of 0x1234 will be sent as \x01 \x02 \x03 \x04
		//   
		// Overall packet is 8x 01 04 47 0p 0q 0r 0s FF
		// NOTE: 8x is the cam address (0x80 + camera)

		if (zoom > 0x4000)
		{ 
			zdirect = 0x4000;
		}
		else if (zoom > 0)
		{
		  zdirect = zoom;
		}
		else
		{
			zdirect = 0;
		}
   
		zdirect = zdirect & 0xffff;

		p = (zdirect & 0xf000) >> 12;
		q = (zdirect & 0x0f00) >> 8;
		r = (zdirect & 0x00f0) >> 4;
		s = zdirect & 0x000f;
		
		unsigned char cm = VISCA_ZOOM_DIRECT;
		unsigned char tm = VISCA_TERMINATOR;

		// We now have our P Q R and S values.  Build rest of packet
		command_header(camera);
		packet->NextByte(&cm);
		packet->NextByte(&p);
		packet->NextByte(&q);
		packet->NextByte(&r);
		packet->NextByte(&s);
		packet->NextByte(&tm);
	    
		// Write packet to queue
		queue_packet();

		return true;
	}
	else	// com object doesn't exist.  Can't talk to camera - return a failure.
	{
		return false;
	}
}

bool VISCA_Cam::set_pt_home(int camera)
{
	if ((cam_pan_tilt) && (!errCode) && (pkt_queueing == false)) 
	{
		unsigned char cm = VISCA_TILT_DRIVE_HOME;		// use the relative position method
		unsigned char tm = VISCA_TERMINATOR;			

		command_header_tilt(camera);
		packet->NextByte(&cm);
		packet->NextByte(&tm);
		    
		// Write packet to queue
		queue_packet();
		return true;
	}
	else
	{
		return false;
	}
}

bool VISCA_Cam::set_pt_relative(int pan, int tilt, int pan_speed, int tilt_speed, int camera)
{
	// Pan is a integer value from 100 through -100
	int panSteps, tiltSteps; 
	double pan_factor, tilt_factor;

	unsigned char pspd = 0;
	unsigned char tspd = 0;
	 
	pspd = pan_speed & 0xff;
	tspd = tilt_speed & 0xff;

	// Range check and clamp speeds
	if (pspd > VISCA_SPEED_PAN_MAX)
	{
		pspd = VISCA_SPEED_PAN_MAX;
	}
	
	if (tspd > VISCA_SPEED_TILT_MAX)
	{
		tspd = VISCA_SPEED_TILT_MAX;
	}
	
	// Make sure we have a pan/tilt camera, and it's okay to send
	if ((cam_pan_tilt) && (!errCode) && (pkt_queueing == false)) 
	{
		unsigned char y1=0;
		unsigned char y2=0;
		unsigned char y3=0;
		unsigned char y4=0;
		unsigned char z1=0;
		unsigned char z2=0;
		unsigned char z3=0;
		unsigned char z4=0;
 
		/* Camera maximum position, can be changed */
		if(pan >100) 
		{
			pan = 100;
		}
		
		if (pan < -100)
		{
			pan = -100;
		}

		if(tilt > 25) 
		{
			tilt = 25;
		}
	
		if (tilt < -25)
		{
			tilt = -25;
		}
		
		if(pan > 0) pan_factor = 8.61;
		else pan_factor = 8.6;

		if(tilt < 0) tilt_factor = 11.4;
		else tilt_factor = 11.44;  
		
		/*set the step*/
		panSteps =  0xffff & (int) (0x10000 + pan * pan_factor + 0.5);
		tiltSteps = 0xffff & (int) (0x10000 + tilt * tilt_factor + 0.5);
	 
		/* Pan argument */
		y1 = 0x0f & (panSteps >> 12);  
		y2 = 0x0f & (panSteps >> 8); 
		y3 = 0x0f & (panSteps >> 4); 
		y4 = 0x0f & panSteps; 

		/* Tilt argument */
		z1 = 0x0f & (tiltSteps >> 12);
		z2 = 0x0f & (tiltSteps >> 8);
		z3 = 0x0f & (tiltSteps >> 4);
		z4 = 0x0f & tiltSteps;

		/* make package */
		unsigned char cm = VISCA_TILT_DRIVE_REL;	// use the relative position method
		unsigned char tm = VISCA_TERMINATOR;		// stop bit
		
		command_header_tilt(camera);
		packet->NextByte(&cm);
		packet->NextByte(&pspd);
		packet->NextByte(&tspd);
		packet->NextByte(&y1);
		packet->NextByte(&y2);
		packet->NextByte(&y3);
		packet->NextByte(&y4);
		packet->NextByte(&z1);
		packet->NextByte(&z2);
		packet->NextByte(&z3);
		packet->NextByte(&z4);
		packet->NextByte(&tm);
	    
		// Write packet to queue
		queue_packet();
		return true;
	}
	else	// com object doesn't exist.  Can't talk to camera - return a failure.
	{
		return false;
	}
}

bool VISCA_Cam::power_on(int camera)
{
	unsigned char CPWR = VISCA_CAMERA_POWER;
	unsigned char VSON = VISCA_ON;
	unsigned char VTRM = VISCA_TERMINATOR;

	if (!errCode)
	{
		command_header(camera);
		packet->NextByte (&CPWR);
		packet->NextByte (&VSON);
		packet->NextByte (&VTRM);
		queue_packet();
		return true;
	}
	else
	{
		return false;
	}
}

bool VISCA_Cam::power_off(int camera)
{
	unsigned char CPWR = VISCA_CAMERA_POWER;
	unsigned char VSOFF = VISCA_OFF;
	unsigned char VTRM = VISCA_TERMINATOR;

	if (!errCode)
	{
		command_header(camera);
		packet->NextByte (&CPWR);
		packet->NextByte (&VSOFF);
		packet->NextByte (&VTRM);
		queue_packet();
		return true;
	}
	else
	{
		return false;
	}
}

bool VISCA_Cam::lens_init(int camera)
{
	unsigned char VINIT = VISCA_INITIALIZE;
	unsigned char LINIT = VISCA_LENS_INIT;
	unsigned char TERM = VISCA_TERMINATOR;

	if (!errCode)
	{
		command_header(camera);
		packet->NextByte (&VINIT);
		packet->NextByte  (&LINIT);
		packet->NextByte(&TERM);
	
		queue_packet();	
		return true;
	}
	else
	{
		return false;
	}
}

bool VISCA_Cam::comp_scan(int camera)
{
	unsigned char VINIT = VISCA_INITIALIZE;
	unsigned char LINIT = VISCA_COMP_SCAN;
	unsigned char TERM = VISCA_TERMINATOR;

	if (!errCode)
	{
		command_header(camera);
		packet->NextByte (&VINIT);
		packet->NextByte (&LINIT);
		packet->NextByte(&TERM);
	
		queue_packet();	
		return true;
	}
	else
	{
		return false;
	}
}

bool VISCA_Cam::set_wb_auto(int camera)
{
	unsigned char VCMD = VISCA_WB;
	unsigned char VCM2 = VISCA_WB_AUTO;
	unsigned char TERM = VISCA_TERMINATOR;

	if (!errCode)
	{
		command_header(camera);
		packet->NextByte (&VCMD);
		packet->NextByte (&VCM2);
		packet->NextByte (&TERM);
		queue_packet();
		return true;
	}
	else
	{
		return false;
	}
}

bool VISCA_Cam::set_wb_onepush(int camera)
{
	unsigned char VCMD = VISCA_WB;
	unsigned char VCM2 = VISCA_WB_ONEPUSH;
	unsigned char TERM = VISCA_TERMINATOR;
  unsigned char T1 = VISCA_WB_ONEPUSH_T1;
	unsigned char T2 = VISCA_WB_ONEPUSH_T2;

	if (!errCode)
	{
		command_header(camera);
		packet->NextByte(&VCMD);
		packet->NextByte(&VCM2);
		packet->NextByte(&TERM);
		queue_packet();

		command_header(camera);
		packet->NextByte(&T1);
		packet->NextByte(&T2);
		packet->NextByte(&TERM);
		queue_packet();
		return true;
	}
	else
	{
		return false;
	}
}

bool VISCA_Cam::set_ae_mode(int camera, int ae_mode)
{
	char NewMode = 0;

  if (!errCode)
	{
	  switch (ae_mode)
		{
			case VISCA_AE_AUTO:
				NewMode = VISCA_AE_AUTO;
				break;
      
			case VISCA_AE_SHUTTER:
				NewMode = VISCA_AE_SHUTTER;
				break;

			case VISCA_AE_IRIS:
				NewMode = VISCA_AE_IRIS;
				break;

			case VISCA_AE_BRIGHT:
				NewMode = VISCA_AE_BRIGHT;
				break;

			case VISCA_AE_MANUAL:
				NewMode = VISCA_AE_MANUAL;
				break;

			default:
				NewMode = VISCA_AE_AUTO;
		}

		this->set_parameter(camera,VISCA_AE,NewMode);
		this->exp_mode=NewMode;

	  return true;
	}
	else
	{
		return false;
	}
}

int VISCA_Cam::get_ae_mode()
{
  if (!errCode)
	{
		return this->exp_mode;
	}
	else
	{
		return 0;
	}
}

int VISCA_Cam::translate_shutter(int shut_speed)
{
	// NOTE: PAL and NTSC cameras use different shutter speeds
	//		 due to timing issues.  Check cam type first!
	//		 HD cams will report PAL or NTSC in "General" inquiry
	//		 depending on the framerate that they are set to 
	//		 (ie: if set to 1080i/59.94 they report NTSC)

	int RetSpeedN [] = {	1, 2, 4, 8, 15, 30, 60, 90, 100, 125, 180, 250, 350,
							500, 725, 1000, 1500, 2000, 3000, 4000, 6000, 10000 };

	int RetSpeedP [] = {	1, 3, 6, 12, 25, 50, 75, 100, 120, 150, 215, 300,
							425, 600, 1000, 1250, 1750, 2500, 3500, 6000, 10000 };
					
	if (shut_speed < 0x00)
	{
		return -1;
	}

	if (shut_speed > 0x15)
	{
		return -1;
	}

	if (this->cam_system_type == VISCA_PAL)
	{
		return RetSpeedP[shut_speed];
	}
	else
	{
		return RetSpeedN[shut_speed];
	}
}


bool VISCA_Cam::set_shutter(int camera, int shut_speed)
{
	unsigned char P = 0;
	unsigned char Q = 0;
	unsigned char VNULL = VISCA_NULL;
	unsigned char CMDB = VISCA_SHUTTER_DIRECT;
	unsigned char TERMB = VISCA_TERMINATOR;

	if (!errCode)
	{
		P = (char)((shut_speed & 0xF0)>>4);
		Q = (char)((shut_speed & 0x0F));

		this->command_header(camera);
		packet->NextByte(&CMDB);
		packet->NextByte(&VNULL);
		packet->NextByte(&VNULL);
		packet->NextByte(&P);
		packet->NextByte(&Q);
		packet->NextByte(&TERMB);

		queue_packet();
		return true;
	}
	else
	{
		return false;
	}
}

double VISCA_Cam::translate_iris(int iris_val)
{
	double IrisVals[] = {	1000.0, 28.0, 22.0, 19.0, 16.0, 
							14.0, 11.0, 9.6, 8.0, 6.8, 5.6, 
							4.8, 4.0, 3.4, 2.8, 2.4, 2.0, 1.6 };

	if (iris_val < 0x00)
	{
		return -1;
	}

	if (iris_val > 0x11)
	{
		return -1;
	}

	return IrisVals[iris_val];

}

bool VISCA_Cam::set_iris(int camera, int iris_setting)
{
	unsigned char P = 0;
	unsigned char Q = 0;
	unsigned char VNULL = VISCA_NULL;
	unsigned char CMDB = VISCA_IRIS_DIRECT;
	unsigned char TERMB = VISCA_TERMINATOR;

	if (!errCode)
	{
		P = (char)((iris_setting & 0xF0)>>4);
		Q = (char)((iris_setting & 0x0F));

		this->command_header(camera);
		packet->NextByte(&CMDB);
		packet->NextByte(&VNULL);
		packet->NextByte(&VNULL);
		packet->NextByte(&P);
		packet->NextByte(&Q);
		packet->NextByte(&TERMB);

		queue_packet();

		return true;
	}
	else
	{
		return false;
	}
}

int VISCA_Cam::translate_gain(int gain_val)
{
	int GainVals[] = {	-3, 0, 2, 4, 6, 8, 10, 12, 14, 
						16, 18, 20, 22,	24, 26, 28 };

	if (gain_val < 0x00)
	{
		return -1;
	}

	if (gain_val > 0x0F)
	{
		return -1;
	}

	return GainVals[gain_val];
}

double VISCA_Cam::translate_bright_iris(int bright_val)
{
	double BrIrisVals[] = { 1000.0, 28.0, 22.0, 19.0, 16.0, 
							14.0, 11.0, 9.6, 8.0, 6.8, 5.6, 
							4.8, 4.0, 3.4, 2.8, 2.4, 2.0, 1.6, 
							1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 
							1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6 };

	if (bright_val < 0x00)
	{
		return -1;
	}
	
	if (bright_val > 0x1F)
	{
		return -1;
	}

	return BrIrisVals[bright_val];
}

int VISCA_Cam::translate_bright_gain(int bright_val)
{
	int BrGainVals[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                 0, 0, 0, 0, 0, 0, 0, 0, 2, 4,
						 6, 8, 10, 12, 14, 16, 18, 20, 
						 22, 24, 26, 28 };

	if (bright_val < 0x00)
	{
		return -1;
	}

	if (bright_val > 0x1F)
	{
		return -1;
	}

	return BrGainVals[bright_val];
}

bool VISCA_Cam::reset_bright(int camera)
{
  if (!errCode)
	{
		bool RETCD = this->set_parameter(camera, VISCA_AE_BRIGHT, VISCA_NULL);
		return RETCD;
	}
	else
		return false;
}

bool VISCA_Cam::set_bright(int camera, int cam_bright)
{
	unsigned char P = 0;
	unsigned char Q = 0;
	unsigned char VNULL = VISCA_NULL;
	unsigned char CMDB = VISCA_BRIGHT_DIRECT;
	unsigned char TERMB = VISCA_TERMINATOR;

	if (!errCode)
	{
		P = (char)((cam_bright & 0xF0)>>4);
		Q = (char)((cam_bright & 0x0F));

		this->command_header(camera);
		packet->NextByte(&CMDB);
		packet->NextByte(&VNULL);
		packet->NextByte(&VNULL);
		packet->NextByte(&P);
		packet->NextByte(&Q);
		packet->NextByte(&TERMB);

		queue_packet();

		return true;
	}
	else
	{
		return false;
	}
}


bool VISCA_Cam::set_gain(int camera, int cam_gain)
{
	unsigned char P = 0;
	unsigned char Q = 0;
	unsigned char VNULL = VISCA_NULL;
	unsigned char CMDB = VISCA_GAIN_DIRECT;
	unsigned char TERMB = VISCA_TERMINATOR;

	if (!errCode)
	{
		P = (char)((cam_gain & 0xF0)>>4);
		Q = (char)((cam_gain & 0x0F));

		this->command_header(camera);
		packet->NextByte(&CMDB);
		packet->NextByte(&VNULL);
		packet->NextByte(&VNULL);
		packet->NextByte(&P);
		packet->NextByte(&Q);
		packet->NextByte(&TERMB);

		queue_packet();

		return true;
	}
	else
	{
		return false;
	}
}

bool VISCA_Cam::stabilizer(int camera, bool onoff)
{
	bool Stat_Code;
	if (!errCode)
	 {
		if (onoff == false)
		{
			Stat_Code = set_parameter(camera,VISCA_STABILIZER,VISCA_ON);
		}
		else
		{
			Stat_Code = set_parameter(camera,VISCA_STABILIZER,VISCA_OFF);
		}

		return true;
	 }
	 else
	 {
		 return false;
	 }
}

bool VISCA_Cam::enable_auto_ir(int camera, bool onoff)
{
	bool Stat_Code;
	if (!errCode)
	{
		if (onoff==true)
		{
			Stat_Code = set_parameter(camera,VISCA_AUTOIR,VISCA_ON);
		}
		else
		{
			Stat_Code = set_parameter(camera,VISCA_AUTOIR,VISCA_OFF);
		}

		return Stat_Code;
	}

	else
	{
		return false;
	}
}

bool VISCA_Cam::enable_ir_mode(int camera, bool onoff)
{
	bool Stat_Code;

	if (!errCode)
	{
		if (onoff==true)
		{
			Stat_Code = set_parameter(camera,VISCA_IR_MODE,VISCA_ON);
		}
		else
		{
			Stat_Code = set_parameter(camera,VISCA_IR_MODE,VISCA_OFF);
		}
		return Stat_Code;
	}

	else
	{
		return false;
	}
}

bool VISCA_Cam::set_exposure_comp_onoff(int camera, bool onoff)
{
	unsigned char CODE = VISCA_EXP_COMP_EN;
	unsigned char ON_OFF;

	if (onoff == true)
		ON_OFF = VISCA_ON;
	else
		ON_OFF = VISCA_OFF;

	set_parameter(camera,CODE,ON_OFF);

	return true;
}

bool VISCA_Cam::set_exposure_comp(int camera, int compval)
{
//	bool Stat_Code;
	const unsigned char CODE = VISCA_EXP_COMP_DCT;
	const unsigned char ZERO = VISCA_NULL;
	const unsigned char TERM = VISCA_TERMINATOR;

	if (compval > 7) compval = 7;
	if (compval < -7) compval = -7;

	unsigned char ExpCompCode = (compval + 7);

	// build command packet
	command_header(camera);
	packet->NextByte(&CODE);
	packet->NextByte(&ZERO);
	packet->NextByte(&ZERO);
	packet->NextByte(&ZERO);
	packet->NextByte(&ExpCompCode);
	packet->NextByte(&TERM);
	queue_packet();
	return true;
}

bool VISCA_Cam::set_sharpness(int camera, int sharp_val)
{
	const unsigned char CODE = VISCA_APERTURE_DCT;
	const unsigned char ZERO = VISCA_NULL;
	const unsigned char TERM = VISCA_TERMINATOR;
	unsigned char APVAL = sharp_val;

	if (!errCode)
	{
		if (sharp_val > 0x0F) APVAL = 0x0F;
		if (sharp_val < 0x00) APVAL = 0x00;

		command_header(camera);
		packet->NextByte(&CODE);
		packet->NextByte(&ZERO);
		packet->NextByte(&ZERO);
		packet->NextByte(&ZERO);
		packet->NextByte(&APVAL);
		packet->NextByte(&TERM);

		queue_packet();
 
		return true;
	}
	else
	{
		return false;
	}
}


bool VISCA_Cam::set_parameter(int camera, const unsigned char parameter,  const unsigned char newValue )
// Use for all "normal" commands that don't require anything special
{
	const unsigned char TERM = VISCA_TERMINATOR;
	if (!errCode)
	{
		//build command packet
		command_header(camera);
		packet->NextByte(&parameter);
		packet->NextByte(&newValue);	//append to
		packet->NextByte(&TERM);		//terminator

		queue_packet();
	
		return true;
	}
	else
	{
		return false;
	}
}

bool VISCA_Cam::set_auto_slow(int camera, bool onoff)
{
	unsigned char VCMD = VISCA_SLOW_SHUTTER;
	unsigned char VCM2 = VISCA_SLOW_ON;
	unsigned char VCM3 = VISCA_SLOW_OFF;
	unsigned char TERM = VISCA_TERMINATOR;

	if (!errCode)
	{

		command_header(camera);
		packet->NextByte(&VCMD);

		if (onoff == true)
		{
			packet->NextByte(&VCM2);
		}
		else
		{
			packet->NextByte(&VCM3);
		}

		packet->NextByte(&TERM); //terminator
		queue_packet();

	  return true;
	}
	else
	{
		return false;
	}
}


bool VISCA_Cam::if_clear()
{
  unsigned char CMD1 = VISCA_COMMAND_HEADER;
	unsigned char VRST = VISCA_RESET;
	unsigned char TERM = VISCA_TERMINATOR;

	if (!errCode)
	{
		command_header(8);
		packet->NextByte(&CMD1);
		packet->NextByte(&VRST);
		packet->NextByte(&TERM);
		queue_packet();

		return true;
	}
	else
	{
		return false;
	}
}

bool VISCA_Cam::command_header(int DevNum)
{
	// Be nice to other packet compositions
	while (pkt_queueing == true)
	{
		#ifdef debug
		  std::cout<<"PACKET CREATE WAIT \n";
	  #endif
		Sleep(5); 
	}

	unsigned char CHeader;
	unsigned char CHeader2 = VISCA_COMMAND_HEADER;
	unsigned char DHeader = VISCA_DATA_HEADER;

	#ifdef debug
		std::cout <<" IN COMMAND_HEADER -- DevNum = " << DevNum << " Num_devices = " << num_devices <<"\n";
	#endif

	this->pkt_queueing = true;

	//check for valid device number (DevNum)
	
	if(DevNum >8)
	{
		// There are a maximum of 7 devices, but 8 is allowed for broadcast.
		// If greater than 8, we have a problem.
		return false;
	}
	
	packet->clear();

	#ifdef debug
		std::cout<<"Creating write header \n";
	#endif
	
	CHeader = VISCA_SEND_HEADER + DevNum;
	packet->NextByte(&CHeader);
	packet->NextByte(&CHeader2);
	packet->NextByte(&DHeader);
	return true;
}

bool VISCA_Cam::command_header_tilt(int DevNum)
{
	// Be nice to other packet compositions
	while (pkt_queueing == true)
	{
		#ifdef debug
		  std::cout<<"PACKET CREATE WAIT \n";
	  #endif
		Sleep(5); 
	}

	unsigned char CHeader;
	unsigned char CHeader2 = VISCA_COMMAND_HEADER;
	unsigned char DHeader = VISCA_PAN_DATA_HEADER;

	#ifdef debug
		std::cout <<" IN COMMAND_HEADER -- DevNum = " << DevNum << " Num_devices = " << num_devices <<"\n";
	#endif

	this->pkt_queueing = true;

	//check for valid device number (DevNum)
	
	if(DevNum >8)
	{
		// There are a maximum of 7 devices, but 8 is allowed for broadcast.
		// If greater than 8, we have a problem.
		return false;
	}
	
	packet->clear();

	#ifdef debug
		std::cout<<"Creating write header \n";
	#endif
	
	CHeader = VISCA_SEND_HEADER + DevNum;
	packet->NextByte(&CHeader);
	packet->NextByte(&CHeader2);
	packet->NextByte(&DHeader);
	return true;
}

bool VISCA_Cam::inquiry_header(int DevNum)
{
	// Be nice to other packet compositions
	while (pkt_queueing == true)
	{
		#ifdef debug
		  std::cout<<"PACKET CREATE WAIT \n";
	  #endif
		Sleep(2); // Try again in 2 mS
	}

	unsigned char CHeader;
	unsigned char CHeader2 = VISCA_INQUIRY_CHEADER;

	#ifdef debug
		std::cout <<" IN INQUIRY_HEADER -- DevNum = " << DevNum << " Num_devices = " << num_devices <<"\n";
	#endif

	this->pkt_queueing = true;
	
	//check for valid device number (DevNum)
	if(DevNum >8)
	{
		// There are a maximum of 7 devices, but 8 is allowed for broadcast.
		// If greater than 8, we have a problem.
		return false;
	}
	
	packet->clear();

	#ifdef debug
		std::cout<<"Creating write header \n";
	#endif
	
	CHeader = VISCA_SEND_HEADER + DevNum;
	packet->NextByte(&CHeader);
	packet->NextByte(&CHeader2);

	return true;
}

// Get Camera type inquiry

void VISCA_Cam::get_cam_type()
{
  int camera = 1;
  const unsigned char CODE = VISCA_INQ_IF;
  const unsigned char INQTYP = VISCA_INQ_VERSION;
  const unsigned char TERM = VISCA_TERMINATOR;

#ifdef debpkts
  std::cout << "get_cam_type() called.\n";
#endif

  inquiry_header(camera);
  packet->NextByte(&CODE);
  packet->NextByte(&INQTYP);
  packet->NextByte(&TERM);
  queue_packet();
}

// Generate and send inquiries to update status

void VISCA_Cam::update_status(int FullMode)
{
	int camera = 1;
	const unsigned char CODE = VISCA_INQ_BLK;
	const unsigned char ZERO = VISCA_NULL;
	const unsigned char TERM = VISCA_TERMINATOR;

	// We send 4 inquiries to update all the status variables
	const unsigned char INQ_LCS = VISCA_INQ_LCS;
	const unsigned char INQ_CCS = VISCA_INQ_CCS;
	const unsigned char INQ_GEN = VISCA_INQ_GEN;
	const unsigned char INQ_EFQ = VISCA_INQ_EFQ;
  
	if ((this->cmds_running() < 3) && (this->pkt_queueing == false))
	{
		// Lens System first
		inquiry_header(camera);
		packet->NextByte(&CODE);
		packet->NextByte(&CODE);
		packet->NextByte(&INQ_LCS);
		packet->NextByte(&TERM);
		queue_packet();

		if (FullMode == 1)  // Full mode sends more inquiries, and takes longer 
		{
			// Now Camera Control System
			inquiry_header(camera);
			packet->NextByte(&CODE);
			packet->NextByte(&CODE);
			packet->NextByte(&INQ_CCS);
			packet->NextByte(&TERM);
			queue_packet();

			// Now General Stuff
			inquiry_header(camera);
			packet->NextByte(&CODE);
			packet->NextByte(&CODE);
			packet->NextByte(&INQ_GEN);
			packet->NextByte(&TERM);
			queue_packet();

			// Finally Enlargement System
			inquiry_header(camera);
			packet->NextByte(&CODE);
			packet->NextByte(&CODE);
			packet->NextByte(&INQ_EFQ);
			packet->NextByte(&TERM);
			queue_packet();
		}	
	}
}


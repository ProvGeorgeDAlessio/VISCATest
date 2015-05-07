/*

 visca_devices.h

 Device list for the VISCA camera control library

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
 ****************************************************************************/

// DEVICE CAPABILITIES LISTING

#define VISCA_DEV_CAPS_COUNT 8

VISCA_DEV_CAPS visca_dev_caps [VISCA_DEV_CAPS_COUNT] =	{

	{	0, VISCA_DEV_TYPE_CAMERA, VISCA_UNKNOWN, VISCA_UNKNOWN,	VISCA_UNKNOWN, 
		VISCA_NAME_UNKNOWN, false, 1, 1, false },

	{	1, VISCA_DEV_TYPE_CAMERA, (VISCA_MFG_SONY << 16) | VISCA_FCB_EX980, VISCA_MFG_SONY,
		VISCA_FCB_EX980, VISCA_NAME_FCB_EX980, true, 1, 26, false },

	{	2, VISCA_DEV_TYPE_CAMERA, (VISCA_MFG_SONY << 16) | VISCA_FCB_EX980P, VISCA_MFG_SONY,
		VISCA_FCB_EX980P, VISCA_NAME_FCB_EX980P, true, 1, 26, false },

	{	3, VISCA_DEV_TYPE_CAMERA, (VISCA_MFG_SONY << 16) | VISCA_FCB_EX980S, VISCA_MFG_SONY,
		VISCA_FCB_EX980S, VISCA_NAME_FCB_EX980S, true, 1, 26, false },

	{	4, VISCA_DEV_TYPE_CAMERA,(VISCA_MFG_SONY << 16) | VISCA_FCB_EX980SP, VISCA_MFG_SONY,
		VISCA_FCB_EX980SP, VISCA_NAME_FCB_EX980SP, true, 1, 26, false },

	{	5, VISCA_DEV_TYPE_CAMERA, (VISCA_MFG_SONY << 16) | VISCA_FCB_H10, VISCA_MFG_SONY,
		VISCA_FCB_H10, VISCA_NAME_FCB_H10, true, 1, 10, false },

	{	6, VISCA_DEV_TYPE_CAMERA, (VISCA_MFG_SONY << 16) | VISCA_EVI_D100P, VISCA_MFG_SONY,
		VISCA_EVI_D100P, VISCA_NAME_EVI_D100P, true, 1, 10, true },

	{	7, VISCA_DEV_TYPE_CAMERA, (VISCA_MFG_SONY << 16) | VISCA_FCB_H11, VISCA_MFG_SONY,
		VISCA_FCB_H11, VISCA_NAME_FCB_H11, true, 1, 10, false },

};

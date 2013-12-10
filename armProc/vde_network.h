/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2004 Renzo Davoli
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef UMPS_VDE_NETWORK_H
#define UMPS_VDE_NETWORK_H

#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/un.h>

#include "armProc/libvdeplug_dyn.h"

class netblockq;

#define PROMISQ  0x4
#define INTERRUPT  0x2
#define NAMED  0x1

unsigned int testnetinterface(const char *name);

class netinterface
{
	public:
		netinterface(const char *name, const char *addr, int intnum);
	
		~netinterface(void);

		unsigned int readdata(char *buf, int len);
		unsigned int writedata(char *buf, int len);
		unsigned int polling();
		void setaddr(char *iethaddr);
		void getaddr(char *pethaddr);
		void setmode(int imode);
		unsigned int getmode();

	private:
		VDECONN *vdeconn;
		char ethaddr[6];
		char mode;
		struct pollfd polldata;
		class netblockq *queue;
};

#endif // UMPS_VDE_NETWORK_H

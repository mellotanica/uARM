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

#ifndef UARM_VDE_NETWORK_CC
#define UARM_VDE_NETWORK_CC

#include "armProc/vde_network.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/poll.h>

#include "armProc/const.h"
#include "armProc/types.h"
#include "armProc/blockdev_params.h"

#include "services/utility.h"
#include "services/error.h"


enum request_type { REQ_NEW_CONTROL };

// FIXME: what is this for?
struct request_v3 {
  uint32_t magic;
  uint32_t version;
  enum request_type type;
  struct sockaddr_un sock;
};

#define SWITCH_MAGIC 0xfeedface
#define STRBUFLEN 128
#define MAXNETQUEUE 16
#define MAXPACKETLEN 1536

HIDDEN struct vdepluglib vdepluglib;
HIDDEN char strbuf[STRBUFLEN];
HIDDEN char packbuf[MAXPACKETLEN];


class netblock {
public:
    netblock(const char *icontent,int ilen);
    ~netblock(void);
    class netblock *getNext();
    void setNext(class netblock *p);
    char * getContent();
    int getLen();

private:
    char *content;
    int len;
    struct netblock *next;
};

class netblockq {
public:
    netblockq(int imaxelem);
    ~netblockq();
    int enqueue(const char *content,int len);
    int empty();
    int dequeue(char *pcontent, int len);

private:
    class netblock *head,*tail;
    int maxelem,nelem;
};

unsigned int testnetinterface(const char *name)
{
    char name2[1024];
    int size;

    if (!vdepluglib.dl_handle)
        libvdeplug_dynopen(vdepluglib);
    /* vde lib does not exist */
    if (vdepluglib.dl_handle == NULL)
        return 0;

    if ((size = readlink(name,name2,1023)) > 0) {
        name2[size]=0;
        name=name2;
    }
    /* file does not exist or read error */
    if (access(name,R_OK) != 0)
        return 0;
    return 1;
}

netinterface::netinterface(const char *name, const char *addr, int intnum)
{ 
	char name2[1024];
	int size;

	if ((size=readlink(name,name2,1023)) > 0) {
		name2[size]=0;
		name=name2;
	}

	vdeconn = vdepluglib.vde_open(name, (char*) "uMPS", NULL);
	queue=NULL;
	polldata.fd = vdepluglib.vde_datafd(vdeconn);
	polldata.events = POLLIN | POLLOUT | POLLERR | POLLHUP | POLLNVAL;

	if (addr != NULL) {
		for (int i=0;i<6;i++)
			ethaddr[i]=addr[i];
	} else {
		sprintf(ethaddr," %5d",getpid());
		ethaddr[0]=intnum*2;
	}

	mode = PROMISQ | NAMED;
	queue = new netblockq(MAXNETQUEUE);
}

netinterface::~netinterface(void)
{
	vdepluglib.vde_close(vdeconn);
	if (queue != NULL) delete queue;
}

unsigned int netinterface::readdata(char *buf, int len)
{
	if (queue->empty() && !this->polling())
		return 0;
	else
		return queue->dequeue(buf, len);
}

unsigned int netinterface::writedata(char *buf, int len)
{
	int retval,pollout;
	if ((pollout=poll(&polldata,1,0)) < 0) {
		sprintf(strbuf,"poll: %s",strerror(errno));
		Panic(strbuf);
		return 0; // -1 ??
	} else {
		if (!(polldata.revents & POLLOUT))
			retval=0;
		else {
			if (len >= 12 && (mode & NAMED) != 0)
				memcpy(buf+6,ethaddr,6);
			retval=vdepluglib.vde_send(vdeconn,buf,len,0);
		}
	}
	return retval;
}


unsigned int netinterface::polling()
{
	int len, fd;
	//socklen_t datainsize;
	//struct sockaddr_un datain;
	do {
		if ((poll(&polldata,1,0)) < 0) {
			sprintf(strbuf,"poll: %s",strerror(errno));
			Panic(strbuf);
		} else 
			if (polldata.revents & POLLIN) {
				/* We don't store sender address to avoid EINVAL in recvfrom */
				len=vdepluglib.vde_recv(vdeconn,packbuf,MAXPACKETLEN,0);
				if (mode & PROMISQ //promiquous mode: receive everything
						|| (len > 12 // header okay and
							&& (memcmp(packbuf,ethaddr,6)==0 //it is sent to this interface
								|| (packbuf[0] & 1)))) //or it's a broadcast
					queue->enqueue(packbuf,len);
			}
	}
	while (polldata.revents & POLLIN);
	return (!queue->empty());
}

void netinterface::setaddr(char *iethaddr)
{
	register int i;
	for (i=0;i<6;i++) 
		ethaddr[i]=iethaddr[i];
		
//	for (int i = 0; i < 6; i++)
//		printf("%x:",ethaddr[i]);
//	printf("\n");
}

void netinterface::getaddr(char *pethaddr)
{
	register int i;
	for (i=0;i<6;i++)
		pethaddr[i]=ethaddr[i];
}

void netinterface::setmode(int imode)
{
	mode=imode;

//	printf("mode %x\n", mode);
}

unsigned int netinterface::getmode()
{
	return mode;
}

netblock::netblock(const char *icontent,int ilen)
{
	if (ilen>0) {
		content=new char [ilen];
		memcpy(content,icontent,ilen);
	} else 
		content=NULL;
	len=ilen;
}

netblock::~netblock(void)
{
	if (content != NULL)
		delete(content);
}

class netblock *netblock::getNext()
{
	return next;
}

void netblock::setNext(class netblock *p)
{
	next=p;
}

char *netblock::getContent()
{
	return content;
}

int netblock::getLen()
{
	return len;
}


netblockq::netblockq(int imaxelem)
{
	maxelem=imaxelem;
	head=tail=NULL;
	nelem=0;
}

netblockq::~netblockq()
{
	class netblock *next;	
	while (head != NULL)
	{
		next=head->getNext();
		delete (head);
		head=next;
	}
}

int netblockq::enqueue(const char *content,int len)
{
	if (nelem >= maxelem)
		return 0;
	else
	{
		class netblock *oldtail=tail;	
		tail=new netblock(content,len);
		if (oldtail!=NULL)
			oldtail->setNext(tail);
		tail->setNext(NULL);
		if (head==NULL) 
			head=tail;
		nelem++;
		return 1;
	}
}

int netblockq::empty()
{
	return head==NULL;
}

int netblockq::dequeue(char *pcontent, int len)
{
	if (head==NULL)
		return 0;
	else 
	{
		class netblock *oldhead=head;
		int packlen;
		head=oldhead->getNext();
		packlen=oldhead->getLen();
		if (len < packlen) packlen=len;
		memcpy(pcontent,oldhead->getContent(),packlen);
		delete oldhead;
		nelem--;
		return packlen;
	}
}

#endif //UARM_VDE_NETWORK_CC

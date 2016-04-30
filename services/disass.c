/*
 *
 *	disassembly for ARM
 *
 *   Copyright 2014 Renzo Davoli University of Bologna - Italy
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, as
 *   published by the Free Software Foundation; either version 2 of the
 *	 License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA.
 *
 */

#include "disass.h"

#ifdef MACOS_BUILD
#include "fmemopen.h"
#endif

#ifdef UARM_DUMMY_DISASSEMBLER
void arm_disass(uint32_t addr, uint32_t instr, char *out)
{
    *out=0;
}
#else

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

static char *dcond[16]={
	"eq","ne","cs","cc","mi","pl","vs","vc",
	"hi","ls","ge","lt","gt","le","",""};

#define DCOND(instr) (dcond[instr >> 28])

static char *dreg[16]={
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
	"r8", "r9",
	"sl", //"r10",
	"fp", //"r11",
	"ip", //"r12",
	"sp", //"r13",
	"lr", //"r14",
	"pc", //"r15"
};

#define DREG(R) (dreg[(R) & 0xf])
#define DREGN(instr) (DREG((instr) >> 16))
#define DREGD(instr) (DREG((instr) >> 12))
#define DREGS(instr) (DREG((instr) >> 8))
#define DREGM(instr) (DREG(instr))

static void arm_disass00(uint32_t addr, uint32_t instr, char *out) {
	(void)addr;
	if ((instr & 0x0ffffff0) == 0x012fff10)
		snprintf(out,DBUFSIZE,"bx%s %s",DCOND(instr),DREGM(instr));
	else if ((instr & 0x0e0000f0) == 0x00000090) {
		if (instr & 0x01000000) {
			// Single Data Swap
			snprintf(out,DBUFSIZE,"swp%s%s %s, %s, %s", DCOND(instr),
					(instr & 0x00400000)?"b":"",
					DREGD(instr), DREGM(instr),DREGN(instr));
		} else {
			// MUL
			if (instr & 0x00800000) {
				// Multiply long
				char *lmulop[]={"umull","umlal","smull","smlal"};
				uint32_t lmul=(instr >> 21) & 0x3;
				snprintf(out,DBUFSIZE,"%s%s%s %s, %s, %s, %s",
						lmulop[lmul],DCOND(instr),
						(instr & 0x00100000)?"s":"",
						DREGD(instr), DREGN(instr),DREGM(instr),DREGS(instr));
			} else {
				// Multiply
				if (instr & 0x00200000)
					snprintf(out,DBUFSIZE,"mla%s%s %s, %s, %s, %s", DCOND(instr),
							(instr & 0x00100000)?"s":"",DREGN(instr),
							DREGM(instr),DREGS(instr),DREGD(instr));
				else
					snprintf(out,DBUFSIZE,"mul%s%s %s, %s, %s", DCOND(instr),
							(instr & 0x00100000)?"s":"",DREGN(instr),
							DREGM(instr),DREGS(instr));
			}
		}
	} else if ((instr & 0x0e000090) == 0x00000090) {
		// halfword
		char *opcode=(instr & 0x00100000)?"ldr":"str";
		char *ssuffix[]={"","h","sh","sb"};
		int32_t type=(instr >> 5) & 0x3;
		char arg[DBUFSIZE];
		uint32_t pre=instr &      0x01000000;
		uint32_t up=instr &       0x00800000;
		uint32_t imm=instr &      0x00400000;
		uint32_t writeback=instr& 0x00200000;
		if (imm) {
			if (instr & 0xf0f) {
				uint32_t offset=((instr >> 4) & 0xf0) | (instr & 0xf);
				snprintf(arg,DBUFSIZE,"[%s%s, #%s%d%s%s",DREGN(instr),
						pre?"":"]",
						up?"":"-",
						offset,
						pre?"]":"",
						writeback?"!":"");
			} else {
				snprintf(arg,DBUFSIZE,"[%s]",DREGN(instr));
			}
		} else {
			snprintf(arg,DBUFSIZE,"[%s%s, %s%s%s%s",DREGN(instr),
					pre?"":"]",
					up?"":"-",
					DREGM(instr),
					pre?"]":"",
					writeback?"!":"");
		}
		snprintf(out,DBUFSIZE,"%s%s%s %s, %s",
				opcode, DCOND(instr), ssuffix[type],
				DREGD(instr),arg);
	} else {
		//Data Processing
		char op2[DBUFSIZE];
		static char *opcode[] = {
			"and","eor","sub","rsb","add","adc","sbc","rsc",
			"tst","teq","cmp","cmn","orr","mov","bic","mvn"};
		static char optype[] = {
			3,3,3,3,3,3,3,3,
			2,2,2,2,3,1,3,1};
		uint32_t opn = (instr >> 21) & 0xf;
		*op2=0;
		if (instr & 0x02000000) {
			snprintf(op2, DBUFSIZE, "#%d",
					(instr & 0xff) << ((instr >> 8) & 0xf));
		} else {
			// TODO ROR #0 means RRX
			static char *rotate[] = {"asl","lsl","asr","ror"};
			uint32_t rotn = (instr >> 5) & 0x3;
			if (instr & 0x10) {
				 snprintf(op2, DBUFSIZE, "%s, %s %s",
						 DREGM(instr), rotate[rotn], DREGS(instr));
			} else {
				uint32_t amount = (instr >> 7) & 0x1f;
				if (amount) {
					snprintf(op2, DBUFSIZE, "%s, %s #%d",
							DREGM(instr), rotate[rotn], amount);
				} else
					snprintf(op2, DBUFSIZE, "%s", DREGM(instr));
			}
		}
		switch (optype[opn]) {
			case 1:
				snprintf(out, DBUFSIZE, "%s%s%s %s, %s",
						opcode[opn], DCOND(instr),
						(instr & 0x00100000)?"s":"",
						DREGD(instr), op2);
				break;
			case 2:
				snprintf(out, DBUFSIZE, "%s%s %s, %s",
						opcode[opn], DCOND(instr),
						DREGN(instr), op2);
				break;
			case 3:
				snprintf(out, DBUFSIZE, "%s%s%s %s, %s, %s",
						opcode[opn], DCOND(instr),
						(instr & 0x00100000)?"s":"",
						DREGD(instr), DREGN(instr), op2);
				break;
		}
	}
}

static void arm_disass01(uint32_t addr, uint32_t instr, char *out) {
	(void)addr;
	//Single Data Transfer
	char *opcode=(instr & 0x00100000)?"ldr":"str";
	char arg[DBUFSIZE];
	uint32_t regoff=instr &   0x02000000;
	uint32_t pre=instr &      0x01000000;
	uint32_t up=instr &       0x00800000;
	uint32_t byte=instr &     0x00400000;
	uint32_t writeback=instr& 0x00200000;
	if (regoff) {
		uint32_t shift=((instr & 0xfff)>>4);
		char sh[16];
		if (shift)
			snprintf(sh,16,", %d",shift);
		else
			*sh=0;
		snprintf(arg,DBUFSIZE,"[%s%s, %s%s%s%s%s",DREGN(instr),
				pre?"":"]",
				up?"":"-",
				DREGM(instr),
				sh,
				pre?"]":"",
				writeback?"!":"");
	} else if (instr & 0xfff) {
		snprintf(arg,DBUFSIZE,"[%s%s, #%s%d%s%s",DREGN(instr),
				pre?"":"]",
				up?"":"-",
				instr & 0xfff,
				pre?"]":"",
				writeback?"!":"");
	} else
		snprintf(arg,DBUFSIZE,"[%s]",DREGN(instr));
	snprintf(out,DBUFSIZE,"%s%s%s%s %s, %s",
			opcode, DCOND(instr),
			byte?"b":"",
			((instr & 0x01200000) == 0x00200000)?"t":"",
			DREGD(instr),arg);
}


#define BEGIN 0
#define NUM 1
#define SEQ 2
#define NONUM 3
#ifdef NUMERICREGLIST
#	define REGFMT "r%d"
#	define REGARG(x) (x)
#else
#	define REGFMT "%s"
#	define REGARG(x) DREG(x)
#endif

static void create_reglist(int regset, char *reglist)
{
	int status=BEGIN;
	FILE *f=fmemopen(reglist,DBUFSIZE,"w");
	int i;
	for (i=0; i<16; i++) {
		if (regset & 1<<i) {
			switch (status) {
				case BEGIN: fprintf(f,REGFMT,REGARG(i)); status=NUM; break;
				case NUM: status=SEQ; break;
				case SEQ: break;
				case NONUM: fprintf(f,", " REGFMT,REGARG(i)); status=NUM; break;
			}
		} else {
			switch (status) {
				case BEGIN: break;
				case NUM: status=NONUM; break;
				case SEQ: fprintf(f,"-" REGFMT,REGARG(i-1)); status=NONUM; break;
				case NONUM: break;
			}
		}
	}
	if (status == SEQ) fprintf(f,"-r%d",i-1);
	fclose(f);
}

static void arm_disass10(uint32_t addr, uint32_t instr, char *out) {
	if (instr & 0x02000000) {
		//B-BL Branch
		long offset = ((instr & 0x800000)?0xff000000:0) | (instr & 0xffffff);
		offset <<= 2;
		offset += 8; //Prefetch
		uint32_t target = addr + offset;
		snprintf(out,DBUFSIZE,"b%s%s #0x%08x ; %+ld ",
				(instr & 0x01000000)?"l":"",DCOND(instr),target,offset);
	} else {
		//Block Data Transfer
		uint32_t isload=(instr & 0x00100000);
		uint32_t isstack=(((instr >> 16) & 0xf) == 0xd);
		char *opcode=isload?"ldm":"stm";
		static char *mod[]={
			"da", "ia", "db", "ib", "da", "ia", "db", "ib",
			"ed", "ea", "fd", "fa", "fa", "fd", "ea", "ed" };
		uint32_t modix=(isstack?0x8:0) | (isload?0x4:0) | ((instr >> 23) & 0x3);
		char reglist[DBUFSIZE];
		create_reglist((instr & 0xffff),reglist);
		if ((instr & 0x00200000) && (modix == 0xa || modix == 0xd)) {
			opcode=isload?"pop":"push";
			snprintf(out,DBUFSIZE,"%s%s {%s}%s",
					opcode,DCOND(instr),reglist,
					(instr & 0x00400000)?"^":"");
		} else {
			snprintf(out,DBUFSIZE,"%s%s%s %s%s, {%s}%s",
					opcode,DCOND(instr),mod[modix],
					DREGN(instr),
					(instr & 0x00200000)?"!":"",
					reglist,
					(instr & 0x00400000)?"^":""
					);
		}
	}
}

static void arm_disass11(uint32_t addr, uint32_t instr, char *out) {
	(void)addr;
	if (instr & 0x02000000) {
		if (instr & 0x01000000) {
			// Software interrupt
			snprintf(out,DBUFSIZE,"swi%s 0x%06x", DCOND(instr),instr & 0xffffff);
		} else {
			uint32_t coproc=(instr >> 8) & 0xf;
			uint32_t cp=(instr >> 5) & 0x7;
			uint32_t copcode=(instr >> 16) & 0xf;
			uint32_t crn = (instr >> 16) & 0xf;
			uint32_t crm = (instr) & 0xf;
			char scp[16];
			if (cp)
				snprintf(scp,16,", %d",cp);
			else
				*scp=0;
			if (instr & 0x10) {
				char *opcode=(instr & 0x00100000)?"mcr":"mrc";
				copcode >>= 1;
				snprintf(out,DBUFSIZE,"%s%s  p%d, %d, %s, c%d, c%d%s",
						opcode, DCOND(instr), coproc, copcode,
						DREGD(instr), crn, crm, scp);
				// Coprocessor register transfer
			} else {
				uint32_t crd = (instr >> 12) & 0xf;
				// Coprocessor data operation
				snprintf(out,DBUFSIZE,"cdp%s p%d, %d, c%d, c%d, c%d%s",
						DCOND(instr), coproc, copcode,
						crd, crn, crm, scp);
			}
		}
	} else {
		// Coprocessor data transfer
	}
}

void arm_disass(uint32_t addr, uint32_t instr, char *out)
{
	*out=0;
	switch ((instr >> 26) & 0x3) {
		case 00: arm_disass00(addr, instr, out); break;
		case 01: arm_disass01(addr, instr, out); break;
		case 02: arm_disass10(addr, instr, out); break;
		case 03: arm_disass11(addr, instr, out); break;
	}
}

#endif

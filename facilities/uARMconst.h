/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uARM
 *
 * Copyright (C) 2014 Marco Melletti
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

#ifndef UARMCONST_H
#define UARMCONST_H

/* Addresses for new and old areas (where new and old processor states are
   stored on exceptions) */
#define EXCV_BASE 0x00007000
#define STATE_T_SIZE 88
#define INT_OLDAREA EXCV_BASE
#define INT_NEWAREA (EXCV_BASE + STATE_T_SIZE)
#define TLB_OLDAREA (EXCV_BASE + (2 * STATE_T_SIZE))
#define TLB_NEWAREA (EXCV_BASE + (3 * STATE_T_SIZE))
#define PGMTRAP_OLDAREA (EXCV_BASE + (4 * STATE_T_SIZE))
#define PGMTRAP_NEWAREA (EXCV_BASE + (5 * STATE_T_SIZE))
#define SYSBK_OLDAREA (EXCV_BASE + (6 * STATE_T_SIZE))
#define SYSBK_NEWAREA (EXCV_BASE + (7 * STATE_T_SIZE))

#define BIOS_SRV_SWI 8
#define BIOS_SRV_BP 9

#define IS_SWI_BP(instr) ((instr & BIOS_SRV_BP) == BIOS_SRV_BP)
#define IS_SWI_SYS(instr) ((instr & BIOS_SRV_SWI) == BIOS_SRV_SWI)

#define DEV_USED_INTS 5 /* Number of ints reserved for devices: 3,4,5,6,7 */

#define DEV_PER_INT 8 /* Maximum number of devices per interrupt line */

/* Total maximum number of devices: terminals are really two devices each, so
   re-add DEV_PER_INT devices, plus one for the pseudo-clock */
#define MAX_DEVICES (DEV_USED_INTS * DEV_PER_INT) + DEV_PER_INT + 1

/* The last semaphore is the pseudo-clock one */
#define CLOCK_SEM (MAX_DEVICES - 1)

/* Interrupt lines used by the devices */
#define INT_TIMER 2    /* timer interrupt */
#define INT_LOWEST 3   /* minimum interrupt number used by real devices */
#define INT_DISK 3
#define INT_TAPE 4
#define INT_UNUSED 5   /* network? */
#define INT_PRINTER 6
#define INT_TERMINAL 7

/* Device command and status codes
   Only those actually used in the code are defined here. If the need arises,
   other commands and states may be defined here. */

#define DEV_C_ACK   1 /* command common to all devices */

#define DEV_DISK_C_SEEKCYL  2        /* disk:       */
#define DEV_DISK_C_READBLK  3
#define DEV_DISK_C_WRITEBLK 4

#define DEV_TAPE_C_READBLK 3   /* tape:       */

#define DEV_PRNT_C_PRINTCHR 2   /* printer */

#define DEV_TRCV_C_RECVCHAR 2   /* terminal */
#define DEV_TTRS_C_TRSMCHAR 2

#define DEV_S_READY   1   /* status common to all devices */

#define DEV_TRCV_S_RECVERR  4  /* terminal-specific */
#define DEV_TRCV_S_CHARRECV 5

#define DEV_TTRS_S_TRSMERR  4
#define DEV_TTRS_S_CHARTRSM 5

#define DEV_TERM_STATUS 0x000000FF

#define DEV_NOT_INSTALLED 0

#define TAPE_EOF 1
#define TAPE_EOT 0


/* Physical memory frame size */
#define FRAME_SIZE 4096   /* or 0x1000 bytes, or 4K */

/* Size of a device register group */
#define DEV_REGBLOCK_SIZE (DEV_REG_SIZE * DEV_PER_INT)

/* Utility definitions for the status register: use suggested operation
    to set/clear status register fields*/

/* status & STATUS_CLEAR_MODE */
#define STATUS_CLEAR_MODE	0xFFFFFFF0

/* status | STATUS_x_MODE */
#define STATUS_USER_MODE	0x00000010
#define STATUS_SYS_MODE		0x0000001F
#define STATUS_SRV_MODE 	0x00000013
#define STATUS_INT_MODE 	0x00000012
#define STATUS_PGMT_MODE 	0x0000001B
#define STATUS_TLB_MODE 	0x00000017

/* use dedicated functions below.. */
#define STATUS_ID			0x00000080
#define STATUS_TIMER_ID		0x00000040

#define STATUS_ENABLE_INT(status)	((status) & ~(STATUS_ID))
#define STATUS_DISABLE_INT(status)	((status) | STATUS_ID)
#define STATUS_ENABLE_TIMER(status)	((status) & ~(STATUS_TIMER_ID))
#define STATUS_DISABLE_TIMER(status)	((status) | STATUS_TIMER_ID)

/* All interrupts unmasked */
#define STATUS_ALL_INT_ENABLE(status)	((status) & ~(STATUS_TIMER_ID) & ~(STATUS_ID))
#define STATUS_ALL_INT_DISABLE(status)	((status) | (STATUS_TIMER_ID) | (STATUS_ID))

/* empty status */
#define STATUS_NULL 		0x00000000

/* CP15 control */
#define CP15_VM_ON	0x00000001

#define CP15_ENABLE_VM(control)	((control) | (CP15_VM_ON))
#define CP15_DISABLE_VM(control) ((control) & ~(CP15_VM_ON))

#define CP15_CONTROL_NULL 0x00000000

/* Utility definitions for the entryHI register */
#define ENTRYHI_SEGNO_GET(entryHI) (((entryHI) & 0xc0000000) >> 30)
#define ENTRYHI_VPN_GET(entryHI) (((entryHI) & 0x3ffff000) >> 12)
#define ENTRYHI_ASID_GET(entryHI) (((entryHI) & 0x00000fe0) >> 5)

#define ENTRYHI_SEGNO_SET(entryHI, seg_no) (((entryHI) & 0x3fffffff) | ((seg_no) << 30) )
#define ENTRYHI_VPN_SET(entryHI, vpn) (((entryHI) & 0xc0000fff) | ((vpn) << 12))
#define ENTRYHI_ASID_SET(entryHI, asid) (((entryHI) & 0xfffff01f) | ((asid) << 5))

/* Utility definitions for the entryLO register */
#define ENTRYLO_PFN_GET(entryLO) (((entryLO) & 0xfffff000) >> 12)
#define ENTRYLO_PFN_SET(entryLO, pfn) (((entryLO) & 0x00000fff) | ((pfn) << 12))
#define ENTRYLO_NOCACHE 0x00000800  /* Not used in uMPS */
#define ENTRYLO_DIRTY 0x00000400
#define ENTRYLO_VALID 0x00000200
#define ENTRYLO_GLOBAL 0x00000100
#define ENTRYLO_ACCESSED 0x00000080  /* NEW flag, not in the specs */

/* Utility definitions for the Cause CP15 register */
#define CAUSE_EXCCODE_GET(cause) ((cause) & 0xFFFFFF)
#define CAUSE_EXCCODE_SET(cause, exc_code) (((cause) & 0xFF000000) | (exc_code))

/* Returns 1 if the interrupt int_no is pending */
#define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 24)))

/* Values for CP15 Cause.ExcCode */
#define EXC_INTERRUPT       INTEXCEPTION
#define EXC_TLBMOD          MODEXCEPTION
#define EXC_TLBINVLOAD      TLBLEXCEPTION
#define EXC_TLBINVSTORE     TLBSEXCEPTION
#define EXC_ADDRINVLOAD     ADELEXCEPTION
#define EXC_ADDRINVSTORE    ADESEXCEPTION
#define EXC_BUSINVFETCH     BUSERROR
#define EXC_BUSINVLDSTORE   BUSERROR
#define EXC_SYSCALL         SYSEXCEPTION
#define EXC_BREAKPOINT      BPEXCEPTION
#define EXC_RESERVEDINSTR   20
#define EXC_COPROCUNUSABLE  COPROCEXCEPTION
#define EXC_BADPTE          INVALIDPAGE
#define EXC_PTEMISS         PAGENOTFOUND
#define EXC_BADPAGTBL       PAGEERROR
#define EXC_BADSEGTBL       SEGERROR

#ifndef UARM_MACHINE_COMPILING
/* Used for disk_op() and in the pager to identify request types */
#define READ 0
#define WRITE 1
#endif

#define SEGTABLE_START 0x0007600

#define PTE_MAGICNO 0x2a


/* Utility definitions */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define	HIDDEN static

#ifndef UARM_MACHINE_COMPILING
#define	FALSE	0
#define	TRUE 1

#define NULL ((void *)0)
#endif

#define CR 0x0a   /* carriage return as returned by the terminal */

// hardware constants

#define VM_PSEG_START   0x00008000
#define VM_PSEG_TOP     0xF0000000
#define VM_SHSEG_START  0xF0000000
#define VM_SHSEG_TOP    0xFFFFFFFF

#define PAGE_TBL_MAGICNO    0x2A

/* ROM ADDRESSES */
#define ROMF_EXCVBASE   0x00007000
#define ROMF_EXCVTOP    0x00007500

#define ROMF_SEGTBASE   0x00007600
#define ROMF_SEGTTOP    0x00007E00

#define ROMF_STACKBASE  0x00007FF0
#define ROMF_STACKTOP   0x00008000

// physical memory page frame size (in words)
#define FRAMESIZE	1024

// KB per frame
#define FRAMEKB	4

// number of ASIDs
#define MAXASID 128

// eth packet size
#define PACKETSIZE 1514

// exception type constants (simulator internal coding)
#define NOEXCEPTION 	0
#define MEMERROR     	1
#define BUSERROR     	2
#define INTEXCEPTION	3
#define SYSEXCEPTION	4
#define UNDEDEXCEPTION	5
#define COPROCEXCEPTION	6
#define BPEXCEPTION		7

#define SEGERROR     	8
#define PAGEERROR     	9
#define INVALIDPAGE     10
#define PAGENOTFOUND    11
#define UTLBLEXCEPTION	12
#define UTLBSEXCEPTION	13
#define TLBLEXCEPTION 	14
#define TLBSEXCEPTION	15
#define ADELEXCEPTION	16
#define ADESEXCEPTION	17
#define MODEXCEPTION    18

#endif // UARMCONST_H

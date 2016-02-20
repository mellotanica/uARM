/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010, 2011 Tomislav Jonjic
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

/*
 * uMPS machine-specific constants, most notably bus & device memory
 * mapped register addresses.
 *
 * IMPORTANT: Keep this header assembler-safe!
 */

#ifndef UARM_ARCH_H
#define UARM_ARCH_H

/*
 * Generalities
 */
#define WORD_SIZE 4
#define WS        WORD_SIZE

#define MMIO_BASE 0x00000040
#define RAM_BASE  0x00008000

#define RAM_TOP ((*((unsigned int *)BUS_REG_RAM_BASE)) + (*((unsigned int *)BUS_REG_RAM_SIZE)))

/* Coprocessor instructions opcodes */
#define OP_TLBWR    0xEE280F00  // opcode 2
#define OP_TLBWI    0xEE3A0F00  // opcode 3
#define OP_TLBR     0xEE4A2F00  // opcode 4
#define OP_TLBP     0xEE52AF00  // opcode 5
#define OP_TLBCLR   0xEE600F00  // opcode 6

/* Segment-related constants */
#define KSEGOS_BASE        0x00000000
#define KSEGOS_BIOS_BASE   0x00000300
#define KUSEG2_BASE        0x00008000

#define KSEG0	0
#define USEG2	2
#define USEG3	3

// segments base addresses
#define KSEG0BASE	0x00008000UL
#define USEG2BASE	0x80000000UL
#define USEG3BASE	0xC0000000UL

/* Device register size */
#define DEV_REG_SIZE_W   4
#define DEV_REG_SIZE     (DEV_REG_SIZE_W * WS)

/*
 * Interrupt lines
 */

#define N_INTERRUPT_LINES   8

#define N_IL                N_INTERRUPT_LINES

/* Number of interrupt lines available to devices */
#define N_EXT_IL            5

/* Devices per interrupt line */
#define N_DEV_PER_IL        8

#define DEV_IL_START        (N_INTERRUPT_LINES - N_EXT_IL)

#define IL_IPI              0
#define IL_CPUTIMER         1
#define IL_TIMER            2
#define IL_DISK             3
#define IL_TAPE             4
#define IL_ETHERNET         5
#define IL_PRINTER          6
#define IL_TERMINAL         7

#define EXT_IL_INDEX(il)    ((il) - DEV_IL_START)

#define INT_DEV_VECTOR(il)	(0x6FE0 + (EXT_IL_INDEX(il) * 4))

/*
 * Bus and device register definitions
 *
 * Device interrupt lines are identified by the range [3, 7],
 * i.e. their repsective physical interrupt lines. Keep this in mind
 * when using the macros below. This is slightly confusing, but so is
 * any alternative.
 */

/* Bus register area. Among other informations, the start and amount of
   installed RAM are stored here */
#define BUS_REG_RAM_BASE        0x000002D0
#define BUS_REG_RAM_SIZE        0x000002D4
#define BUS_REG_DEV_BASE        0x000002D8

/* Elapsed clock ticks (CPU instructions executed) since system power on.
   Only the "low" part is actually used. */
#define BUS_REG_TOD_HI          0x000002DC
#define BUS_REG_TOD_LO          0x000002E0

/* How many clock ticks per microsecond (read 1) */
#define BUS_REG_TIMER           0x000002E4
#define BUS_REG_TIME_SCALE      0x000002E8

/* Interrupting devices bitmaps starting address: the actual bitmap address is
   computed with PENDING_BITMAP_START + (WORD_SIZE * (int_no - 3)) */
#define CDEV_BITMAP_BASE        0x00006FE0
#define CDEV_BITMAP_END         (CDEV_BITMAP_BASE + N_EXT_IL * WS)
#define CDEV_BITMAP_ADDR(line)  (CDEV_BITMAP_BASE + ((line) - DEV_IL_START) * WS)

/* Installed devices bitmap starting address: same as above */
#define IDEV_BITMAP_BASE        0x00000020
#define IDEV_BITMAP_END         (IDEV_BITMAP_BASE + N_EXT_IL * WS)
#define IDEV_BITMAP_ADDR(line)  (IDEV_BITMAP_BASE + ((line) - DEV_IL_START) * WS)

/* Device register area */
#define DEV_REG_START           0x00000040
#define DEV_REG_ADDR(line, dev) (DEV_REG_START + ((line) - DEV_IL_START) * N_DEV_PER_IL * DEV_REG_SIZE + (dev) * DEV_REG_SIZE)

/* End of memory mapped external device registers area */
#define DEV_REG_END             (DEV_REG_START + N_EXT_IL * N_DEV_PER_IL * DEV_REG_SIZE)

#define CPUCTL_TPR_PRIORITY_MASK    0x0000000f
#define IRT_POLICY_FIXED   0

#endif //UARM_ARCH_H

/*-
 * Copyright (c) 2014-2015 Rui Paulo <rpaulo@felyko.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

extern int ti_initialise(pru_t) __hidden;

#define	AM18XX_REV		0x4E825900
#define	AM33XX_REV		0x4E82A900

#define	AM18XX_IRAM_SIZE	0x00001000
#define	AM18XX_RAM_REG		0x00000000
#define AM18XX_INTC_REG		0x00004000
#define AM18XX_PRU0CTL_REG	0x00007000
#define AM18XX_PRU1CTL_REG	0x00007800
#define	AM18XX_PRUnCTL(n)	AM18XX_PRU0CTL_REG + n * 0x800
#define	AM18XX_PRUnSTATUS(n)	AM18XX_PRU0CTL_REG + 0x4 + n * 0x800
#define	AM18XX_PRU0IRAM_REG	0x00008000
#define	AM18XX_PRU1IRAM_REG	0x0000C000
#define	AM18XX_PRUnIRAM(n)	AM18XX_PRU0CTL_REG + n * AM18XX_IRAM_SIZE
#define	AM18XX_MMAP_SIZE	0x00007C00

#define	AM33XX_IRAM_SIZE	0x00002000
#define	AM33XX_RAM_REG		0x00000000
#define	AM33XX_RAM_SIZE		0x00020000
#define AM33XX_INTC_REG		0x00020000
#define AM33XX_PRU0CTL_REG	0x00022000
#define AM33XX_PRU1CTL_REG	0x00024000
#define	AM33XX_PRUnCTL(n)	AM33XX_PRU0CTL_REG + n * 0x2000
#define	AM33XX_PRUnSTATUS(n)	AM33XX_PRU0CTL_REG + 0x4 + n * 0x2000
#define AM33XX_PRU0DBG_REG	0x00022400
#define AM33XX_PRU1DBG_REG	0x00024400
#define	AM33XX_PRUnDBG(n)	AM33XX_PRU0DBG_REG + n * 0x2000
#define	AM33XX_PRU0IRAM_REG	0x00034000
#define	AM33XX_PRU1IRAM_REG	0x00038000
#define	AM33XX_PRUnIRAM(n)	AM33XX_PRU0IRAM_REG + n * 0x4000
#define	AM33XX_MMAP_SIZE	0x00040000

/* Control register */
#define	CTL_REG_RESET	        (1U << 0)	/* Clear to reset */
#define	CTL_REG_ENABLE		(1U << 1)
#define	CTL_REG_COUNTER		(1U << 2)	/* Cycle counter */
#define	CTL_REG_SINGLE_STEP	(1U << 8)
#define	CTL_REG_RUNSTATE	(1U << 15)


/*
 * Definitions for the disassembler.
 */
#define	TI_OP_ADD	0x00
#define	TI_OP_ADDI	0x01
#define	TI_OP_ADC	0x02
#define	TI_OP_ADCI	0x03
#define	TI_OP_SUB	0x04
#define	TI_OP_SUBI	0x05
#define	TI_OP_SUC	0x06
#define	TI_OP_SUCI	0x07
#define	TI_OP_LSL	0x08
#define	TI_OP_LSLI	0x09
#define	TI_OP_LSR	0x0a
#define	TI_OP_LSRI	0x0b
#define	TI_OP_RSB	0x0c
#define	TI_OP_RSBI	0x0d
#define	TI_OP_RSC	0x0e
#define	TI_OP_RSCI	0x0f
#define	TI_OP_AND	0x10
#define	TI_OP_OR	0x12
#define	TI_OP_ORI	0x13
#define	TI_OP_XOR	0x14
#define	TI_OP_XORI	0x15
#define	TI_OP_NOT	0x17
#define	TI_OP_MIN	0x18
#define	TI_OP_MAX	0x1a
#define	TI_OP_CLR	0x1c
#define	TI_OP_CLRI	0x1d
#define	TI_OP_SET	0x1e
#define	TI_OP_JMP	0x20
#define	TI_OP_JAL	0x22
#define	TI_OP_LDI	0x24
#define	TI_OP_LMBD	0x26
#define	TI_OP_HALT	0x2a
#define	TI_OP_MVI	0x2c
#define	TI_OP_XIN	0x2e
#define	TI_OP_XOUT	0x2f
#define	TI_OP_SLP	0x3e
#define	TI_OP_QBLT	0x49
#define	TI_OP_QBEQ	0x51
#define	TI_OP_QBLE	0x59
#define	TI_OP_QBGT	0x61
#define	TI_OP_QBNE	0x69
#define	TI_OP_QBGE	0x71
#define	TI_OP_QBA	0x79
#define	TI_OP_SBCO	0x81
#define	TI_OP_LBCO	0x91
#define	TI_OP_QBBC	0xc8
#define	TI_OP_QBBS	0xd0
#define	TI_OP_SBBO	0xe1
#define	TI_OP_LBBO	0xf1

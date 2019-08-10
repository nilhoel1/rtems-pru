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
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <stdbool.h>

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/mman.h>
#include <sys/cdefs.h>

#include <libpru.h>
#include <pru-private.h>
#include <ti-pru.h>

static uint8_t
ti_reg_read_1(char *mem, unsigned int reg)
{
	DPRINTF("reg 0x%x\n", reg);
	return *(volatile uint8_t *)(void *)(mem + reg);
}

static uint32_t
ti_reg_read_4(char *mem, unsigned int reg)
{
	DPRINTF("reg 0x%x\n", reg);
	return *(volatile uint32_t *)(void *)(mem + reg);
}

static void
ti_reg_write_4(char *mem, unsigned int reg, uint32_t value)
{
	DPRINTF("reg 0x%x, val 0x%x\n", reg, value);
	*(volatile uint32_t *)(void *)(mem + reg) = value;
}

static int
ti_disable(pru_t pru, unsigned int pru_number)
{
	unsigned int reg;

	if (pru_number > 1)
		return -1;
	DPRINTF("pru %d\n", pru_number);
	if (pru->md_stor[0] == AM18XX_REV)
		reg = AM18XX_PRUnCTL(pru_number);
	else
		reg = AM33XX_PRUnCTL(pru_number);
	ti_reg_write_4(pru->mem, reg,
	    ti_reg_read_4(pru->mem, reg) & ~CTL_REG_ENABLE);

	return 0;
}

static int
ti_enable(pru_t pru, unsigned int pru_number, int single_step)
{
	unsigned int reg;
	uint32_t val;

	if (pru_number > 1)
		return -1;
	DPRINTF("pru %d\n", pru_number);
	if (pru->md_stor[0] == AM18XX_REV)
		reg = AM18XX_PRUnCTL(pru_number);
	else
		reg = AM33XX_PRUnCTL(pru_number);
	val = ti_reg_read_4(pru->mem, reg);
	if (single_step)
		val |= CTL_REG_SINGLE_STEP;
	else
		val &= ~CTL_REG_SINGLE_STEP;
	val |= CTL_REG_ENABLE;
	ti_reg_write_4(pru->mem, reg, val);

	return 0;
}

static int
ti_reset(pru_t pru, unsigned int pru_number)
{
	unsigned int reg;

	if (pru_number > 1)
		return -1;
	DPRINTF("pru %d\n", pru_number);
	if (pru->md_stor[0] == AM18XX_REV)
		reg = AM18XX_PRUnCTL(pru_number);
	else
		reg = AM33XX_PRUnCTL(pru_number);
	ti_reg_write_4(pru->mem, reg,
	    ti_reg_read_4(pru->mem, reg) & ~CTL_REG_RESET);
	ti_reg_write_4(pru->mem, reg,
	    ti_reg_read_4(pru->mem, reg) | CTL_REG_COUNTER);
	/* Set the PC. */
	ti_reg_write_4(pru->mem, reg,
	    ti_reg_read_4(pru->mem, reg) & (0x0000ffff));

	return 0;
}

static int
ti_upload(pru_t pru, unsigned int pru_number, const char *buffer, size_t size)
{
	unsigned char *iram;

	if (pru_number > 1)
		return -1;
	DPRINTF("pru %d\n", pru_number);
	iram = (unsigned char *)pru->mem;
	if (pru->md_stor[0] == AM18XX_REV) {
		if (size > AM18XX_IRAM_SIZE) {
			errno = EFBIG;
			return -1;
		}
		iram += AM18XX_PRUnIRAM(pru_number);
		DPRINTF("IRAM at %p\n", (void*)iram);
		memset(iram, 0, AM18XX_IRAM_SIZE);
	} else {
		if (size > AM33XX_IRAM_SIZE) {
			errno = EFBIG;
			return -1;
		}
		iram += AM33XX_PRUnIRAM(pru_number);
		DPRINTF("IRAM at %p\n", (void*)iram);
		memset(iram, 0, AM33XX_IRAM_SIZE);
	}
	DPRINTF("copying buf %p size %zu\n", (const void *)buffer, size);
	memcpy(iram, buffer, size);

	return 0;
}

static int
ti_wait(pru_t pru, unsigned int pru_number)
{
	unsigned int reg;
	struct timespec ts;
	int i;

	DPRINTF("pru %d\n", pru_number);
	/* 0.01 seconds */
	ts.tv_nsec = 10000000;
	ts.tv_sec = 0;
	if (pru_number > 1)
		return -1;
	if (pru->md_stor[0] == AM18XX_REV)
		reg = AM18XX_PRUnCTL(pru_number);
	else
		reg = AM33XX_PRUnCTL(pru_number);
	/*
	 * Wait for the PRU to start running.
	 */
	i = 0;
	while (i < 10 && !(ti_reg_read_4(pru->mem, reg) & CTL_REG_RUNSTATE)) {
		nanosleep(&ts, NULL);
		i++;
	}
	if (i == 10)
		return -1;
	while (ti_reg_read_4(pru->mem, reg) & CTL_REG_RUNSTATE)
		nanosleep(&ts, NULL);

	return 0;
}

static int
ti_check_intr(pru_t pru)
{
	struct kevent change, event;
	int nev;

	EV_SET(&change, pru->fd, EVFILT_READ, EV_ADD | EV_ONESHOT, 0, 0, NULL);
	nev = kevent(pru->kd, &change, 1, &event, 1, NULL);
	if (nev <= 0)
		return -1;
	else
		return (int)event.data;
}

static int
ti_deinit(pru_t pru)
{
	if (pru->mem != MAP_FAILED)
		munmap(pru->mem, pru->mem_size);
	close(pru->fd);

	return 0;
}

static uint32_t
ti_read_imem(pru_t pru, unsigned int pru_number, uint32_t mem)
{
	/* XXX missing bounds check. */
	return ti_reg_read_4(pru->mem, AM33XX_PRUnIRAM(pru_number) + mem);
}

static int
ti_write_imem(pru_t pru, unsigned int pru_number, uint32_t mem, uint32_t ins)
{
	/* XXX missing bounds check. */
	ti_reg_write_4(pru->mem, AM33XX_PRUnIRAM(pru_number) + mem, ins);

	return 0;
}

static uint8_t
ti_read_mem(pru_t pru, unsigned int pru_number __unused, uint32_t mem)
{
	/* XXX missing bounds check. */
	return (ti_reg_read_1(pru->mem, AM33XX_RAM_REG + mem));
}

static void
ti_reg_str(uint8_t reg, char *buf, size_t len)
{
	if (reg < 0xe0) {
		snprintf(buf, len, "r%d.%c%d", reg & 0x1f,
		    reg & 0x80 ? 'w' : 'b', (reg >> 5) & 0x3);
	} else
		snprintf(buf, len, "r%d", reg - 0xe0);
}

static void
ti_std_ins(const char *ins, const char *op1, const char *op2, const char *op3,
    char *buf, size_t len)
{
	bool comma = false;

	snprintf(buf, len, "%s", ins);
	if (op1 && op1[0] != 0) {
		snprintf(buf, len, "%s %s", buf, op1);
		comma = true;
	}
	if (op2 && op2[0] != 0) {
		snprintf(buf, len, "%s%s %s", buf, comma == true ? "," : "",
		    op2);
		comma = true;
	}
	if (op3 && op3[0] != 0)
		snprintf(buf, len, "%s%s %s", buf, comma == true ? "," : "",
		    op3);
}

static int
ti_disassemble(pru_t pru __unused, uint32_t opcode, char *buf, size_t len)
{
	uint8_t ins, op1, op2, op3;
	uint16_t imm;
	char c_op1[16], c_op2[16], c_op3[16];
	const char *c_ins;

	DPRINTF("disassembling 0x%x\n", opcode);
	c_ins = NULL;
	bzero(c_op1, sizeof(c_op1));
	bzero(c_op2, sizeof(c_op2));
	bzero(c_op3, sizeof(c_op3));
	ins = (opcode & 0xff000000) >> 24;
	op1 = (opcode & 0x00ff0000) >> 16;
	op2 = (opcode & 0x0000ff00) >> 8;
	op3 = opcode & 0xff;
	/*
	 * Parse the name of the instruction.
	 */
	switch (ins) {
	case TI_OP_ADD:
	case TI_OP_ADDI:
		c_ins = "add";
		break;
	case TI_OP_ADC:
	case TI_OP_ADCI:
		c_ins = "adc";
		break;
	case TI_OP_SUB:
	case TI_OP_SUBI:
		c_ins = "sub";
		break;
	case TI_OP_SUC:
	case TI_OP_SUCI:
		c_ins = "suc";
		break;
	case TI_OP_LSL:
	case TI_OP_LSLI:
		c_ins = "lsl";
		break;
	case TI_OP_LSR:
	case TI_OP_LSRI:
		c_ins = "lsr";
		break;
	case TI_OP_RSB:
	case TI_OP_RSBI:
		c_ins = "rsb";
		break;
	case TI_OP_RSC:
	case TI_OP_RSCI:
		c_ins = "rsc";
		break;
	case TI_OP_AND:
		/* An instruction following 'slp'. We ignore it. */
		if (opcode == 0x10000000) {
			buf[0] = 0;
			return EINVAL;
		}
		if (op1 == op2)
			c_ins = "mov";
		else
			c_ins = "and";
		break;
	case TI_OP_OR:
	case TI_OP_ORI:
		c_ins = "or";
		break;
	case TI_OP_XOR:
	case TI_OP_XORI:
		c_ins = "xor";
		break;
	case TI_OP_NOT:
		c_ins = "not";
		break;
	case TI_OP_MIN:
		c_ins = "min";
		break;
	case TI_OP_MAX:
		c_ins = "max";
		break;
	case TI_OP_CLR:
	case TI_OP_CLRI:
		c_ins = "clr";
		break;
	case TI_OP_SET:
		c_ins = "set";
		break;
	case TI_OP_JMP:
		if (op1 == 0x9e)
			c_ins = "ret";
		else
			c_ins = "jmp";
		break;
	case TI_OP_JAL:
		if (op3 == 0x9e)
			c_ins = "call";
		else
			c_ins = "jal";
		break;
	case TI_OP_LDI:
		c_ins = "ldi";
		break;
	case TI_OP_LMBD:
		c_ins = "lmbd";
		break;
	case TI_OP_HALT:
		c_ins = "halt";
		break;
	case TI_OP_MVI:
		if (op1 == 0x20)
			c_ins = "mvib";
		else if (op1 == 0x21)
			c_ins = "mviw";
		else if (op1 == 0x22)
			c_ins = "mvid";
		else
			c_ins = "mvi";
		break;
	case TI_OP_XIN:
		c_ins = "xin";
		break;
	case TI_OP_XOUT:
		c_ins = "xout";
		break;
	case TI_OP_SLP:
		c_ins = "slp";
		break;
	case TI_OP_QBLT:
		c_ins = "qblt";
		break;
	case TI_OP_QBEQ:
		c_ins = "qbeq";
		break;
	case TI_OP_QBLE:
		c_ins = "qble";
		break;
	case TI_OP_QBGT:
		c_ins = "qbgt";
		break;
	case TI_OP_QBNE:
		c_ins = "qbne";
		break;
	case TI_OP_QBGE:
		c_ins = "qbge";
		break;
	case TI_OP_QBA:
		c_ins = "qba";
		break;
	case TI_OP_SBCO:
		c_ins = "sbco";
		break;
	case TI_OP_LBCO:
		c_ins =  "lbco";
		break;
	case TI_OP_QBBC:
		c_ins = "qbbc";
		break;
	case TI_OP_QBBS:
		c_ins = "qbbs";
		break;
	case TI_OP_SBBO:
		c_ins = "sbbo";
		break;
	case TI_OP_LBBO:
		c_ins = "lbbo";
		break;
	default:
		c_ins = "UD#";
		break;
	}
	/*
	 * Process operators.
	 */
	switch (ins) {
	/* Standard 3 register instructions. */
	case TI_OP_ADD:
	case TI_OP_ADC:
	case TI_OP_SUB:
	case TI_OP_SUC:
	case TI_OP_LSL:
	case TI_OP_LSR:
	case TI_OP_RSB:
	case TI_OP_RSC:
	case TI_OP_AND:
	case TI_OP_OR:
	case TI_OP_XOR:
	case TI_OP_MIN:
	case TI_OP_MAX:
	case TI_OP_SET:
	case TI_OP_LMBD:
		ti_reg_str(op1, c_op1, sizeof(c_op1));
		ti_reg_str(op2, c_op2, sizeof(c_op2));
		ti_reg_str(op3, c_op3, sizeof(c_op3));
		if (ins == TI_OP_AND && op1 == op2)
			bzero(c_op2, sizeof(c_op2));
		break;
	/* Instructions with two registers. */
	case TI_OP_NOT:
		/* N.B. op1 is never used. */
		ti_reg_str(op2, c_op1, sizeof(c_op1));
		ti_reg_str(op3, c_op2, sizeof(c_op2));
		break;
	case TI_OP_CLR:
		ti_reg_str(op1, c_op1, sizeof(c_op1));
		ti_reg_str(op2, c_op2, sizeof(c_op2));
		break;
	/* Instructions with immediate values */
	case TI_OP_ADDI:
	case TI_OP_ADCI:
	case TI_OP_CLRI:
	case TI_OP_SUBI:
	case TI_OP_SUCI:
	case TI_OP_LSLI:
	case TI_OP_LSRI:
	case TI_OP_RSCI:
	case TI_OP_RSBI:
	case TI_OP_ORI:
	case TI_OP_XORI:
		snprintf(c_op1, sizeof(c_op1), "0x%x", op1);
		ti_reg_str(op2, c_op2, sizeof(c_op2));
		ti_reg_str(op3, c_op3, sizeof(c_op3));
		break;
	/* Special instructions. */
	case TI_OP_LDI:
		imm = (opcode & 0x00ffff00) >> 8;
		snprintf(c_op1, sizeof(c_op1), "0x%x", imm);
		ti_reg_str(op3, c_op3, sizeof(c_op3));
		break;
	case TI_OP_MVI:
		ti_reg_str(op3, c_op3, sizeof(c_op3));
		ti_reg_str(op2, c_op2, sizeof(c_op2));
		memmove(c_op2 + 1, c_op2, sizeof(c_op2));
		c_op2[0] = '*';
		break;
	case TI_OP_JMP:
		if (op1 != 0x9e)
			snprintf(c_op1, sizeof(c_op1), "r%d.w0", op1 - 0x80);
		break;
	case TI_OP_JAL:
		if (op3 == 0x9e) {
			snprintf(c_op1, sizeof(c_op1), "r%d.w0", op1 - 0x80);
		} else {
			snprintf(c_op1, sizeof(c_op1), "r%d.w0", op1 - 0xc0);
			snprintf(c_op3, sizeof(c_op3), "r%d.w0", op3 - 0x80);
		}
	case TI_OP_SLP:
		if (op1 == 0x80)
			snprintf(c_op1, sizeof(c_op1), "1");
		else
			snprintf(c_op1, sizeof(c_op1), "0");
	}
	/* N.B. the order of the arguments is reversed. */
	ti_std_ins(c_ins, c_op3, c_op2, c_op1, buf, len);

	return 0;
}

static uint32_t
ti_read_reg(pru_t pru, unsigned int pru_number, uint32_t reg)
{
	if (pru->md_stor[0] == AM18XX_REV)
		return 0;
	if (pru_number > 1 || reg > 31)
		return 0;

	DPRINTF("read reg %u\n", reg);
	return ti_reg_read_4(pru->mem,
		AM33XX_PRUnDBG(pru_number) + reg * 4);
}

static int
ti_write_reg(pru_t pru, unsigned int pru_number, uint32_t reg, uint32_t val)
{
	if (pru->md_stor[0] == AM18XX_REV)
		return -1;
	if (pru_number > 1 || reg > 31)
		return -1;
	DPRINTF("write reg %u val %u\n", reg, val);
	ti_reg_write_4(pru->mem, AM33XX_PRUnDBG(pru_number) + reg * 4, val);

	return 0;
}

static uint16_t
ti_get_pc(pru_t pru, unsigned int pru_number)
{
	uint32_t reg;

	if (pru->md_stor[0] == AM18XX_REV)
		reg = AM18XX_PRUnSTATUS(pru_number);
	else
		reg = AM33XX_PRUnSTATUS(pru_number);

	return (ti_reg_read_4(pru->mem, reg) & 0xffff) * 4;
}

static int
ti_set_pc(pru_t pru, unsigned int pru_number, uint16_t pc)
{
	uint32_t reg, val;

	if (pru_number > 1)
		return -1;
	if (pru->md_stor[0] == AM18XX_REV)
		reg = AM18XX_PRUnCTL(pru_number);
	else
		reg = AM33XX_PRUnCTL(pru_number);

	val = ti_reg_read_4(pru->mem, reg);
	val &= 0x0000ffff;
	val |= (uint32_t)pc << 16;
	ti_reg_write_4(pru->mem, reg, val);

	return 0;
}

static int
ti_insert_breakpoint(pru_t pru, unsigned int pru_number, uint32_t pc,
    uint32_t *orig_ins)
{
	if (orig_ins)
		*orig_ins = ti_read_imem(pru, pru_number, pc);
	DPRINTF("inserting breakpoint: pc 0x%x, ins 0x%x\n", pc,
	    orig_ins ? *orig_ins : 0);
	ti_write_imem(pru, pru_number, pc, TI_OP_HALT << 24);

	return 0;
}

int
ti_initialise(pru_t pru)
{
	size_t i;
	int fd = 0;
	char dev[64];
	size_t mmap_sizes[2] = { AM33XX_MMAP_SIZE, AM18XX_MMAP_SIZE };
	int saved_errno = 0;

	for (i = 0; i < 4; i++) {
		snprintf(dev, sizeof(dev), "/dev/pruss%zu", i);
		fd = open(dev, O_RDWR);
		if (fd == -1 && errno == EACCES)
			break;
		if (fd > 0)
			break;
	}
	if (fd < 0)
		return EINVAL;
	pru->fd = fd;
	/* N.B.: The order matters. */
	for (i = 0; i < sizeof(mmap_sizes)/sizeof(mmap_sizes[0]); i++) {
		pru->mem = mmap(0, mmap_sizes[i], PROT_READ|PROT_WRITE,
		    MAP_SHARED, fd, 0);
		saved_errno = errno;
		if (pru->mem != MAP_FAILED) {
			pru->mem_size = mmap_sizes[i];
			break;
		}
	}
	if (pru->mem == MAP_FAILED) {
		DPRINTF("mmap failed %d\n", saved_errno);
		errno = saved_errno;
		close(fd);
		return -1;
	}
	/*
	 * Use the md_stor field to save the revision.
	 */
	if (ti_reg_read_4(pru->mem, AM18XX_INTC_REG) == AM18XX_REV) {
		DPRINTF("found AM18XX PRU @ %p\n", (void *)pru->mem);
		pru->md_stor[0] = AM18XX_REV;
	} else if (ti_reg_read_4(pru->mem, AM33XX_INTC_REG) == AM33XX_REV) {
		DPRINTF("found AM33XX PRU @ %p\n", (void *)pru->mem);
		pru->md_stor[0] = AM33XX_REV;
	} else {
		munmap(pru->mem, pru->mem_size);
		close(fd);
		return EINVAL;
	}
	ti_disable(pru, 0);
	ti_disable(pru, 1);
	ti_reset(pru, 0);
	ti_reset(pru, 1);
	pru->disable = ti_disable;
	pru->enable = ti_enable;
	pru->reset = ti_reset;
	pru->upload_buffer = ti_upload;
	pru->wait = ti_wait;
	pru->check_intr = ti_check_intr;
	pru->deinit = ti_deinit;
	pru->read_imem = ti_read_imem;
	pru->write_imem = ti_write_imem;
	pru->read_mem = ti_read_mem;
	pru->disassemble = ti_disassemble;
	pru->read_reg = ti_read_reg;
	pru->write_reg = ti_write_reg;
	pru->get_pc = ti_get_pc;
	pru->set_pc = ti_set_pc;
	pru->insert_breakpoint = ti_insert_breakpoint;

	return 0;
}

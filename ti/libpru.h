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

#include <sys/types.h>

struct pru;
typedef struct pru * pru_t;

#define	LIBPRU_VERSION "0.1"

typedef enum {
	PRU_TYPE_UNKNOWN,
	PRU_TYPE_TI,
} pru_type_t;

pru_type_t pru_name_to_type(const char *);

/*
 * Allocates and initialises a PRU structure.
 *
 * This is usually the first step to interface with a PRU.
 */
pru_t	pru_alloc(pru_type_t);

/*
 * Deallocates a PRU structure after stopping the PRU.
 */
void	pru_free(pru_t);
#ifdef __BLOCKS__
void	pru_set_handler(pru_t, void (^)(int));
#endif
void	pru_set_handler_f(pru_t, void (*)(int));

/*
 * Resets the PRU.
 *
 * This function disables the PRU execution unit.
 * To re-enable it, call pru_enable().
 */
int	pru_reset(pru_t, unsigned int);

/*
 * Disables a specific PRU.
 *
 * Execution can continue with pru_enable().
 */
int	pru_disable(pru_t, unsigned int);

/*
 * Enables a specific PRU.
 *
 * Execution can be stopped with pru_disable().
 */
int	pru_enable(pru_t, unsigned int, int);

/*
 * Upload a file to be run on the PRU.
 *
 * To be safe, the PRU should be reset before this function is called.
 */
int	pru_upload(pru_t, unsigned int, const char *);

/*
 * Upload a buffer to be run on the PRU.
 *
 * To be safe, the PRU should be reset before this function is called.
 */
int	pru_upload_buffer(pru_t, unsigned int, const char *, size_t);

/*
 * Wait for the PRU to halt.
 */
int	pru_wait(pru_t, unsigned int);

/*
 * Read the data memory.
 */
uint8_t pru_read_mem(pru_t, unsigned int, uint32_t);

/*
 * Read the instruction memory.
 */
uint32_t pru_read_imem(pru_t, unsigned int, uint32_t);

/*
 * Write to the instruction memory.
 */
int pru_write_imem(pru_t, unsigned int, uint32_t, uint32_t);

/*
 * Disassemble the opcode passed in as argument.
 */
int	pru_disassemble(pru_t, uint32_t, char *, size_t);

enum pru_reg {
	REG_R0 = 0,
	REG_R1,
	REG_R2,
	REG_R3,
	REG_R4,
	REG_R5,
	REG_R6,
	REG_R7,
	REG_R8,
	REG_R9,
	REG_R10,
	REG_R11,
	REG_R12,
	REG_R13,
	REG_R14,
	REG_R15,
	REG_R16,
	REG_R17,
	REG_R18,
	REG_R19,
	REG_R20,
	REG_R21,
	REG_R22,
	REG_R23,
	REG_R24,
	REG_R25,
	REG_R26,
	REG_R27,
	REG_R28,
	REG_R29,
	REG_R30,
	REG_R31,
	REG_PC,

	REG_INVALID,
};

/*
 * Read a PRU register.
 */
uint32_t pru_read_reg(pru_t, unsigned int, enum pru_reg);

/*
 * Modify a PRU register.
 */
int pru_write_reg(pru_t, unsigned int, enum pru_reg, uint32_t);

/*
 * Inserts a breakpoint instruction, saving the original instruction.
 */
int pru_insert_breakpoint(pru_t, unsigned int, uint32_t, uint32_t *);

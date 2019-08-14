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

struct pru {
	int fd;
	int kd; /* kqueue */
	pru_type_t type;
	pthread_t thread;
	/* Abstraction layer */
	char *mem;
	size_t mem_size;
	unsigned int md_stor[4];
	int (*disable)(pru_t, unsigned int);
	int (*enable)(pru_t, unsigned int, int);
	int (*reset)(pru_t, unsigned int);
	int (*upload_buffer)(pru_t, unsigned int, const char *, size_t);
	int (*wait)(pru_t, unsigned int);
	int (*check_intr)(pru_t);
	int (*deinit)(pru_t);
	uint32_t (*read_imem)(pru_t, unsigned int, uint32_t);
	uint8_t (*read_mem)(pru_t, unsigned int, uint32_t);
	int (*write_imem)(pru_t, unsigned int, uint32_t, uint32_t);
	uint32_t (*read_reg)(pru_t, unsigned int, uint32_t);
	int (*write_reg)(pru_t, unsigned int, uint32_t, uint32_t);
	uint16_t (*get_pc)(pru_t, unsigned int);
	int (*set_pc)(pru_t, unsigned int, uint16_t);
	int (*disassemble)(pru_t, uint32_t, char *, size_t);
	int (*insert_breakpoint)(pru_t, unsigned int, uint32_t, uint32_t *);
	void (*intr_func)(int);
#ifdef __BLOCKS__
	void (^intr_block)(int);
#endif
};

extern int libpru_debug;

#define	DPRINTF(fmt, ...)	do {		\
	if (libpru_debug) {			\
		printf("%s: ", __func__);	\
		printf(fmt, __VA_ARGS__);	\
	}					\
} while (0)

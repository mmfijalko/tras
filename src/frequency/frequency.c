/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2023 Marek Marcin Fijałkowski
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
 * 3. The names of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>

#include <tras.h>
#include <hamming8.h>
#include <frequency.h>

/*
 * TODO: content, frequency test algorithms.
 */

#define	DATA_BYTE(p, i)	(((uint8_t *)(p))[i])

static uint8_t mmask8[9] = {
	0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff
};

static uint8_t lmask8[9] = {
	0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};

static uint16_t mmask16[17] = {
	0x0000,
	0x8000, 0xc000, 0xe000, 0xf000, 0xf800, 0xfc00, 0xfe00, 0xff00,
	0xff80, 0xffc0, 0xffe0, 0xfff0, 0xfff8, 0xfffc, 0xfffe, 0xffff,
};

static uint16_t	lmask16[17] = {
	0x0000,
	0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
	0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff,
};

static unsigned int
frequency_sum1(void *data, unsigned int bits)
{
	unsigned int sum, i, n;
	uint8_t *p;

	n = bits & ~0x07;
	p = (uint8_t *)data;

	for (i = 0, sum = 0; i < n; i++, p++)
		sum += hamming8[*p];
	n = bits & 0x07;
	if (n > 0) 
		sum += hamming8[*p & mmask8[n]];

	return (sum);
}

#define	min(a, b) (((a) < (b)) ? (a) : (b))

static unsigned int
frequency_sum2(void *data, unsigned int bits)
{
	unsigned int sum, n;
	uint8_t m, *p;

	p = (uint8_t *)data;
	sum = 0;

	while (bits > 0) {
		n = min(bits, 8);
		for (m = 0x80; n > 0; n--, m >>= 1) {
			if (*p & m)
				sum++;
		}
		bits -= n;
		p++;
	}
	return (sum);
}

/*
 * Parameters of frequency test:
 * - number of bits, N
 * 
 * Constant value:
 * - minimum number of bits for the test, Nmin
 *
 * Test result evaluation:
 * - P-value
 */

int
frequency_init(struct tras_ctx *ctx, void *params)
{

	if (ctx == NULL || params != NULL)
		return (EINVAL);

	tras_ctx_init(ctx);

	ctx->algo = &frequency_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
frequency_update(struct tras_ctx *ctx, void *data, unsigned int bits)
{

	/* todo: */
	return (0);
}

int
frequency_final(struct tras_ctx *ctx)
{

	return (0);
}

int
frequency_test(struct tras_ctx *ctx, void *data, unsigned int bits)
{
	int error;

	error = frequency_update(ctx, data, bits);
	if (error != 0)
		return (error);

	error = frequency_final(ctx);
	if (error != 0)
		return (error);

	return (0);
}

int
frequency_restart(struct tras_ctx *ctx, void *params)
{

	return (0);
}

int
frequency_free(struct tras_ctx *ctx)
{

	return (0);
}

const struct tras_algo frequency_algo = {
	.name =		"Frequency Test",
	.desc =		"Generic Frequency (Monobit) Test",
	.version = 	{ 0, 1, 1 },
	.init =		frequency_init,
	.update =	frequency_update,
	.test =		frequency_test,
	.final =	frequency_final,
	.restart =	frequency_restart,
	.free =		frequency_free,
};

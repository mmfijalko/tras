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
#include <stdlib.h>

#include <tras.h>
#include <hamming8.h>
#include <frequency.h>

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

struct frequency_ctx {
	unsigned int	sum;		/* number of ones */
	unsigned int	nbits;		/* number of bits updated */
	unsigned int	discarded;	/* number of bits discarded */
	double		pvalue;		/* significance level if any */
};

#define	FREQUENCY_ID_GENERIC		0
#define	FREQUENCY_ID_FIPS_140_1		1
#define	FREQUENCY_ID_FIPS_140_2		2

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
frequency_init_algo(struct tras_ctx *ctx, void *params, struct tras_algo *algo)
{
	struct frequency_ctx *c;
	struct frequency_params *p = params;
	size_t size;
	int error;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	c = malloc(sizeof(struct frequency_ctx));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}

	c->nbits = 0;
	c->discarded = 0;
	c->alpha = p->alpha;
	c->sum = 0;

	ctx->context = c;
	ctx->algo = algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
frequency_init(struct tras_ctx *ctx, void *params)
{

	return (frequency_init_algo(ctx, param, &frequency_algo));
}

int
frequency_update(struct tras_ctx *ctx, void *data, unsigned int bits)
{
	struct frequency_ctx *c;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	c->sum += frequency_sum1(data, bits);
	c->nbits += bits;

	return (0);
}

int
frequency_final(struct tras_ctx *ctx)
{
	struct frequency_ctx *c;
	double pvalue;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	if (c->nbits < FREQUENCY_MIN_BITS)
		return (EALREADY);

	/* todo: here calculation of statistics */
	pvalue = 0.0;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discarded = 0;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
frequency_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_test_test(ctx, data, nbits));
}

int
frequency_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
frequency_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo frequency_algo = {
	.name =		"Frequency Test",
	.desc =		"Generic Frequency (Monobit) Test",
	.id =		FREQUENCY_ID_GENERIC,
	.version = 	{ 0, 1, 1 },
	.init =		frequency_init,
	.update =	frequency_update,
	.test =		frequency_test,
	.final =	frequency_final,
	.restart =	frequency_restart,
	.free =		frequency_free,
};

int
frequency_fips_140_init_algo(struct tras_ctx *ctx, void *params,
    struct tras_algo *algo)
{
	struct frequency_ctx *c;
	struct frequency_params *p = params;
	size_t size;
	int error;

	if (ctx == NULL || param != NULL)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	c = malloc(sizeof(struct frequency_ctx));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}

	c->nbits = 0;
	c->discarded = 0;
	c->alpha = 0.0;
	c->sum = 0;
	
	ctx->context = c;
	ctx->algo = algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

frequency_fips_140_final(struct tras_ctx *ctx, unsigned int minsum,
    unsigned int maxsum)
{
	struct frequency_ctx *c;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	if (c->nbits < FREQUENCY_FIPS_MIN_BITS)
		return (EALREADY);

	if (c->sum > minsum && c->sum < maxsum)
		ctx->result.status = TRAS_TEST_PASSED;
	else
		ctx->result.status = TRAS_TEST_FAILED;

	/* todo: Calculate discarded */
	ctx->result.discarded = c->discarded;

	ctx->state = TRAS_STATE_FINAL;

	return (0);

int
frequency_fips_140_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (0);
}

int
frequency_fips_140_1_init(struct tras_ctx *ctx, void *params)
{

	return (frequency_fips_140_init_algo(ctx, param,
	    &frequency_fips_140_1_algo));
}

int
frequency_fips_140_1_final(struct tras_ctx *ctx)
{
	unsigned int minsum, maxsum;

	minsum = FREQUENCY_FIPS_140_1_MIN_SUM;
	maxsum = FREQUENCY_FIPS_140_1_MAX_SUM;

	return (frequency_fips_140_final(ctx, minsum, maxsum));
}

const struct tras_algo frequency_fips_140_1_algo = {
	.name =		"140-1 Monobit",
	.desc =		"FIPS 140-1 Frequency Test",
	.id =		FREQUENCY_ID_FIPS_140_1,
	.version =	{ 0, 1, 1 },
	.init =		frequency_fips_140_1_init,
	.update =	frequency_fips_140_update,
	.test =		frequency_test,
	.final =	frequency_fips_140_1_final,
	.restart =	frequency_restart,
	.free =		frequency_free,
};

int
frequency_fips_140_2_init(struct tras_ctx *ctx, void *params)
{

	return (frequency_fips_140_init_algo(ctx, param,
	    &frequency_fips_140_2_algo));
}

int
frequency_fips_140_2_final(struct tras_ctx *ctx)
{
	unsigned int minsum, maxsum;

	minsum = FREQUENCY_FIPS_140_2_MIN_SUM;
	maxsum = FREQUENCY_FIPS_140_2_MAX_SUM;

	return (frequency_fips_140_final(ctx, minsum, maxsum));
}

const struct tras_algo frequency_fips_140_1_algo = {
	.name =		"140-1 Monobit",
	.desc =		"FIPS 140-2 Frequency Test",
	.id =		FREQUENCY_ID_FIPS_140_2,
	.version =	{ 0, 1, 1 },
	.init =		frequency_fips_140_2_init,
	.update =	frequency_fips_140_update,
	.test =		frequency_test,
	.final =	frequency_fips_140_2_final,
	.restart =	frequency_restart,
	.free =		frequency_free,
};

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
#include <math.h>

#include <tras.h>
#include <cdefs.h>
#include <utils.h>
#include <bits.h>
#include <hamming8.h>
#include <frequency.h>

		#include <stdio.h>

/*
 * The generic frequency test context.
 */
struct frequency_ctx {
	unsigned int	sum;		/* number of ones */
	unsigned int	nbits;		/* number of bits updated */
	unsigned int	minbits;	/* mimimum number of bits */
	unsigned int	maxbits;	/* maximum number of bits */
	unsigned int	discard;	/* number of bits discarded */
	double		alpha;		/* significance level if any */
};

#define	FREQUENCY_ID_GENERIC		0
#define	FREQUENCY_ID_FIPS_140_1		1
#define	FREQUENCY_ID_FIPS_140_2		2

/*
 * Calculate number of bits set in the sequence of bits.
 */
unsigned int
frequency_sum1(void *data, unsigned int nbits)
{
	unsigned int sum, i, n;
	uint8_t *p;

	n = nbits >> 3;
	p = (uint8_t *)data;

	for (i = 0, sum = 0; i < n; i++, p++)
		sum += hamming8[*p];
	n = nbits & 0x07;
	if (n > 0) 
		sum += hamming8[*p & mmask8[n]];

	return (sum);
}

/*
 * The function unused because of very poor performance. It is about ~15 times
 * slower than frequency_sum1.
 */
unsigned int
frequency_sum2(void *data, unsigned int nbits)
{
	unsigned int sum, n;
	uint8_t m, *p;

	p = (uint8_t *)data;
	sum = 0;

	while (nbits > 0) {
		n = min(nbits, 8);
		for (m = 0x80; m > 0; m >>= 1) {
			if (*p & m)
				sum++;
		}
		nbits -= n;
		p++;
	}
	return (sum);
}

/*
 * Calculate sum of bits using bitcount macro for 32-bits words. The remainder
 * must be calculated using bytes from the table to avoid endianess problem.
 * The function performes a little better than frequency_sum1 and this can be
 * shown by iterating over large number of sequences and using ministat tool.
 */
unsigned int
frequency_sum3(void *data, unsigned int nbits)
{
	unsigned int sum, i, n;
	uint32_t *p, last;
	uint8_t *p8;

	n = nbits >> 5;
	p = (uint32_t *)data;

	for (i = 0, sum = 0; i < n; i++, p++)
		sum += bitcount_32(*p);
	p8 = (uint8_t *)p;
	n = (nbits & 0x1f) >> 3;
	for (i = 0, last = 0; i < n; i++)
		last = (last << 8) | (uint32_t)(*p8++);
	n = nbits & 0x07;
	if (n > 0)
		last = (last << 8) | (uint32_t)(*p8 & mmask8[n]);
	sum += bitcount_32(last);

	return (sum);
}

unsigned int
frequency_sum1_offs(void *data, unsigned int offs, unsigned int nbits)
{
	unsigned int sum, n;
	uint8_t *p, p0;

	p = (uint8_t *)data + (offs >> 3);

	if (offs & 0x07) {
		n = 8 - (offs & 0x07);
		if (nbits > n) {
			sum = hamming8[*p & lmask8[n]];
			p++;
			sum += frequency_sum1(p, nbits - n);
		} else {
			p0 = *p & lmask8[n] & mmask8[8 - n + nbits];
			sum += hamming8[p0];
		}
		return (sum);
	}

	return (frequency_sum1(p, nbits));
}

unsigned int
frequency_sum2_offs(void *data, unsigned int offs, unsigned int nbits)
{

	/* todo: */
	return (0);
}

static int
frequency_init_algo(struct tras_ctx *ctx, void *params,
    const struct tras_algo *algo)
{
	struct frequency_ctx *c;
	struct frequency_params *p = params;
	size_t size;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	c = malloc(sizeof(struct frequency_ctx));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}

	c->sum = 0;
	c->discard = 0;
	c->nbits = 0;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
frequency_init(struct tras_ctx *ctx, void *params)
{

	return (frequency_init_algo(ctx, params, &frequency_algo));
}

int
frequency_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct frequency_ctx *c;
	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context;
	c->sum += frequency_sum3(data, nbits);
	c->nbits += nbits;

	return (0);
}

int
frequency_final(struct tras_ctx *ctx)
{
	struct frequency_ctx *c;
	double pvalue, sobs;
	int sum;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nbits < FREQUENCY_MIN_BITS)
		return (EALREADY);

	sum = (int)(2 * c->sum) - (int)c->nbits;
	sobs = abs((double)(sum));
	sobs = sobs / sqrt((double)c->nbits);
	sobs = sobs / sqrt((double)2.0);
	pvalue = erfc(sobs);

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = 0;
	ctx->result.stats1 = sum;
	ctx->result.stats2 = sobs;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
frequency_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
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

static int
frequency_fips_140_init_algo(struct tras_ctx *ctx, void *params,
    const struct tras_algo *algo)
{
	struct frequency_ctx *c;
	struct frequency_params *p = params;
	size_t size;
	int error;

	TRAS_CHECK_INIT(ctx);

	c = malloc(sizeof(struct frequency_ctx));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}

	c->sum = 0;
	c->discard = 0;
	c->alpha = 0.0;
	c->nbits = 0.0;
	
	ctx->context = c;
	ctx->algo = algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

static int
frequency_fips_140_final(struct tras_ctx *ctx, unsigned int minsum,
    unsigned int maxsum)
{
	struct frequency_ctx *c;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nbits < FREQUENCY_FIPS_MIN_BITS)
		return (EALREADY);

	if (c->sum > minsum && c->sum < maxsum)
		ctx->result.status = TRAS_TEST_PASSED;
	else
		ctx->result.status = TRAS_TEST_FAILED;

	/* todo: Calculate discard */
	ctx->result.discard = c->discard;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
frequency_fips_140_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (0);
}

int
frequency_fips_140_1_init(struct tras_ctx *ctx, void *params)
{

	return (frequency_fips_140_init_algo(ctx, params,
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

	return (frequency_fips_140_init_algo(ctx, params,
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

const struct tras_algo frequency_fips_140_2_algo = {
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

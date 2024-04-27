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
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <tras.h>
#include <cdefs.h>
#include <const.h>
#include <maurer.h>

/* Notes:
 * - Q shoud be selected as Q := 10 * 2 ^ L (why ?)
 */

/*
 * Structure for universal statistics parameters.
 */
struct universal_stats {
	double		expected;
	double		variance;
};

/*
 * Minimum number of bits to update for particular L value.
 */
static uint64_t universal_minbits[UNIVERSAL_MAX_L + 1] = {
	0,		/* L = 0 */
	0,		/* L = 1 */
	0,		/* L = 2 */
	0, 		/* L = 3 */
	0,		/* L = 4 */
	0,		/* L = 5 */
	387840,		/* L = 6 */
	904960,		/* L = 7 */
	2068480,	/* L = 8 */
	4654080,	/* L = 9 */
	10342400,	/* L = 10 */
	22753280,	/* L = 11 */
	49643520,	/* L = 12 */
	107560960,	/* L = 13 */
	231669760,	/* L = 14 */
	496435200,	/* L = 15 */
	1059061760,	/* L = 16 */
};

/*
 * Expected statistics values and variance for each supported block length.
 */
static const struct universal_stats universal_stats[UNIVERSAL_MAX_L + 1] = {
	{ 0.0, 0.0 },		/* L = 0 */
	{ 0.0, 0.0 },		/* L = 1 */
	{ 0.0, 0.0 },		/* L = 2 */
	{ 0.0, 0.0 },		/* L = 3 */
	{ 0.0, 0.0 },		/* L = 4 */
	{ 0.0, 0.0 },		/* L = 5 */
	{ 5.2177052, 2.954 },	/* L = 6 */
	{ 6.1962507, 3.125 },	/* L = 7 */
	{ 7.1836656, 3.238 },	/* L = 8 */
	{ 8.1764248, 3.311 },	/* L = 9 */
	{ 9.1723243, 3.356 },	/* L = 10 */
	{ 10.170032, 3.384 },	/* L = 11 */
	{ 11.168765, 3.401 },	/* L = 12 */
	{ 12.168070, 3.410 },	/* L = 13 */
	{ 13.167693, 3.416 },	/* L = 14 */
	{ 14.167488, 3.419 },	/* L = 15 */
	{ 15.167379, 3.421 },	/* L = 16 */
};

int
maurer_init_algo(struct tras_ctx *ctx, void *params,
    const struct tras_algo *algo)
{
	struct universal_ctx *c;
	struct universal_params *p = params;
	size_t size;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (p->L < UNIVERSAL_MIN_L || p->L > UNIVERSAL_MAX_L)
		return (EINVAL);
	if (p->Q != (10 * (1UL << p->L)))
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	size = (1 << p->L) * sizeof(unsigned int);
	c = malloc(sizeof(struct universal_ctx) + size);
	if (c == NULL)
		return (ENOMEM);
	c->lblks = (unsigned int *)(c + 1);
	memset(c->lblks, 0, size);

	c->K = 0;
	c->L = p->L;
	c->Q = 10 * (1UL << p->L);
	c->stats = 0.0;
	c->nbits = 0;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
maurer_init(struct tras_ctx *ctx, void *params)
{

	return (maurer_init_algo(ctx, params, &maurer_algo));
}

inline static uint32_t
maurer_get_sequence_1(int offs, int nbits, uint8_t *data)
{
	uint32_t seq = 0;
	uint8_t mask, b, *d;
	int n;

	d = data + offs / 8;

	while (nbits > 0) {
		b = *d++ & (0xff >> (offs & 0x07));
		n = 8 - (offs & 0x7);
		if (nbits < n) {
			b = b >> (n - nbits);
			n = nbits;
		}
		seq = (seq << n) | (uint32_t)b;
		offs += n;
		nbits -= n;
	}
	return (seq);
}

inline static uint32_t
maurer_get_sequence_2(int offs, int nbits, uint8_t *data)
{
	uint32_t seq = 0;
	int n;

	while (nbits > 0) {
		seq = seq << 1;
		n = 7 - (offs & 0x07);
		if (data[offs >> 3] & (1 << n))
			seq |= 0x1;
		offs++;
		nbits--;
	}
	return (seq);
}

int
maurer_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct universal_ctx *c;
	uint32_t block;
	unsigned int i, n, m, offs;
	uint8_t *p;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	if (nbits == 0)
		return (0);

	c = ctx->context;
	p = (uint8_t *)data;

	/* how long is stored subsequence */
	m = c->nbits % c->L;
	offs = 0;
	if (m > 0) {
		/* Get subsequence and try to append to not full block */
		n = min(nbits, c->L - m);
		block = maurer_get_sequence_1(0, n, data);
		block = (c->block << n) | block;
		if ((m + n) < c->L) {
			c->block = block;
			c->nbits += nbits;
			return (0);
		}
		/* Full block collected, do init or normal block table update */
		if (c->K >= c->Q)
			c->stats += log2(c->K + 1 - c->lblks[block]);
		c->lblks[block] = ++c->K;
		offs = n;
	}
	

	/* Iterate over m full blocks */
	m = (nbits - offs) / c->L;
	for (i = 0; i < m; i++, offs += c->L) {
		block = maurer_get_sequence_1(offs, c->L, data);
		if (c->K >= c->Q)
			c->stats += log2(c->K + 1 - c->lblks[block]);
		c->lblks[block] = ++c->K;
	}

	/* Store subsequence shorter than L */
	if (offs < nbits)
		c->block = maurer_get_sequence_1(offs, (nbits - offs), data);

	c->nbits += nbits;

	return (0);
}

static int
maurer_allow_final(struct universal_ctx *c)
{

	return (c->nbits >= universal_minbits[c->L]);
}

static double
maurer_coef(struct universal_ctx *c)
{

	return (0.7 - 0.8 / c->L + (4.0 + 32.0 / c->L) * 
	    pow(c->K, -3.0 / c->L) / 15.0);
}

int
maurer_final(struct tras_ctx *ctx)
{
	struct universal_ctx *c;
	double pvalue, coef, stdev;
	double stats;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	if (!maurer_allow_final(c))
		return (EALREADY);

	stats = c->stats / (double)(c->K - c->Q);
	stdev = maurer_coef(c) * sqrt(universal_stats[c->L].variance / c->K);
	stats = fabs(stats - universal_stats[c->L].expected) / (SQRT_2 * stdev);
	pvalue = erfc(stats);

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = 0;
	ctx->result.stats1 = stats;
	ctx->result.stats2 = 0.0;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
maurer_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
maurer_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
maurer_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo maurer_algo = {
	.name =		"Universal",
	.desc =		"Maurer's Universal Statistical Test",
	.id =		UNIVERSAL_ID_MAURER,
	.version = 	{ 0, 1, 1 },
	.init =		maurer_init,
	.update =	maurer_update,
	.test =		maurer_test,
	.final =	maurer_final,
	.restart =	maurer_restart,
	.free =		maurer_free,
};

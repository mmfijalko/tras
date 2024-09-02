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
#include <math.h>

#include <tras.h>
#include <cdefs.h>

#include <hamming8.h>
#include <frequency.h>
#include <runs.h>

struct runs_ctx {
	unsigned int	nbits;		/* number of bits processed */
	unsigned int	ones;		/* number of ones for frequency */
	uint8_t		last;		/* byte to keep last bit */
	unsigned int	runs;		/* statistics ??? */
	int		flags;		/* flags from parameters */
	double		alpha;		/* significance level for H0*/
};

/*
 * Runs for 8 bits sequences.
 */
static const uint8_t runs8[256] = {
	0, 1, 2, 1, 2, 3, 2, 1, 2, 3, 4, 3, 2, 3, 2, 1,
	2, 3, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 3, 2, 1,
	2, 3, 4, 3, 4, 5, 4, 3, 4, 5, 6, 5, 4, 5, 4, 3,
	2, 3, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 3, 2, 1,
	2, 3, 4, 3, 4, 5, 4, 3, 4, 5, 6, 5, 4, 5, 4, 3,
	4, 5, 6, 5, 6, 7, 6, 5, 4, 5, 6, 5, 4, 5, 4, 3,
	2, 3, 4, 3, 4, 5, 4, 3, 4, 5, 6, 5, 4, 5, 4, 3,
	2, 3, 4, 3, 4, 5, 4, 3, 2, 3, 4, 3, 2, 3, 2, 1,
	1, 2, 3, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 3, 2,
	3, 4, 5, 4, 5, 6, 5, 4, 3, 4, 5, 4, 3, 4, 3, 2,
	3, 4, 5, 4, 5, 6, 5, 4, 5, 6, 7, 6, 5, 6, 5, 4,
	3, 4, 5, 4, 5, 6, 5, 4, 3, 4, 5, 4, 3, 4, 3, 2,
	1, 2, 3, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 3, 2,
	3, 4, 5, 4, 5, 6, 5, 4, 3, 4, 5, 4, 3, 4, 3, 2,
	1, 2, 3, 2, 3, 4, 3, 2, 3, 4, 5, 4, 3, 4, 3, 2,
	1, 2, 3, 2, 3, 4, 3, 2, 1, 2, 3, 2, 1, 2, 1, 0,
};

#define	__BIT(p, i)	(((p)[(i) / 8] >> (7 - ((i) & 0x07))) & 0x01)

/*
 * Slow bit per bit algorithm to calculate number of runs.
 */
static unsigned int
runs_runs_count1(uint8_t *p, unsigned int nbits)
{
	unsigned int runs, i;

	if (nbits == 0 || nbits == 1)
		return (0);

	for (runs = 0, i = 0; i < nbits - 1; i++) {
		if (__BIT(p, i) != __BIT(p, i + 1))
			runs++;
	}

	return (runs);
}

/*
 * Table version of the algorithm for number of runs.
 */
static unsigned int
runs_runs_count2(uint8_t *p, unsigned int nbits)
{
	uint8_t u8, t;
	unsigned int n, i, runs;

	n = nbits >> 3;
	t = *p & 0x80;
	for (runs = 0, i = 0; i < n; i++, p++) {
		if ((*p & 0x80) ^ t)
			runs++;
		runs += runs8[*p];
		t = (*p << 7) & 0x80;
	}

	n = nbits & 0x07;
	if (n > 0) {
		if ((*p & 0x80) ^ t)
			runs++;
		t = *p;
		for (i = 0; i < n - 1; i++) {
			if ((t & 0x80) ^ ((t << 1) & 0x80))
				runs++;
			t = t << 1;
		}
	}
	return (runs);
}

int
runs_init(struct tras_ctx *ctx, void *params)
{
	struct runs_params *p = params;
	struct runs_ctx *c;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	error = tras_init_context(ctx, &runs_algo, sizeof(struct runs_ctx),
	    TRAS_F_ZERO);
	if (error != 0)
		return (error);
	c = ctx->context;

	c->runs = 1;
	c->flags = p->flags;
	c->alpha = p->alpha;

	return (0);
}

int
runs_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct runs_ctx *c;
	unsigned int n;
	uint8_t *p;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	if (nbits == 0)
		return (0);

	c = ctx->context;
	p = (uint8_t *)data;

	c->ones += frequency_sum1(data, nbits);

	if (c->nbits != 0 && (c->last ^ ((*p) & 0x80)))
		c->runs++;
	c->runs += runs_runs_count2(p, nbits);

	n = ((nbits >> 3) + 7) / 8;
	c->last = *(p + n - 1);
	n = nbits & 0x07;
	n = (n != 0) ? n - 1 : 7;
	c->last = (c->last << n) & 0x80;

	c->nbits += nbits;

	return (0);
}

int
runs_final(struct tras_ctx *ctx)
{
	struct runs_ctx *c;
	double pvalue, stats;
	double pi, arg;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nbits < RUNS_MIN_BITS)
		return (EALREADY);

	pi = (double)c->ones / c->nbits;
	pi = pi * (1.0 - pi);

	arg = (double)c->runs - 2.0 * c->nbits * pi;
	arg = abs(arg) / (2.0 * sqrt(2.0 * c->nbits) * pi);

	pvalue = erfc(arg);

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.stats1 = arg;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	tras_fini_context(ctx, 0);

	return (0);
}

int
runs_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
runs_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
runs_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo runs_algo = {
	.name =		"Runs",
	.desc =		"Runs Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		runs_init,
	.update =	runs_update,
	.test =		tras_do_test,
	.final =	runs_final,
	.restart =	tras_do_restart,
	.free =		tras_do_free,
};

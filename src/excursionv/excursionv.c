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
#include <excursionv.h>

/*
 * Private context for random excursion variant test.
 */
struct excursionv_ctx {
	unsigned int	nbits;		/* number of bits updated */
	double		alpha;		/* significance level */
	double *	pvalue;		/* P-values table */
	unsigned int *	counts;		/* states counters */	
	int		state;		/* current state */
	unsigned int	cycle;		/* current cycle */
};

int
excursionv_init(struct tras_ctx *ctx, void *params)
{
	struct excursionv_ctx *c;
	struct excursionv_params *p = params;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	c = malloc(sizeof(struct excursionv_ctx) + 19 *
	    (sizeof(int) + sizeof(double)));
	if (c == NULL)
		return (ENOMEM);

	c->counts = (unsigned int *)(c + 1);
	c->pvalue = (double *)(c->counts + 19);
	c->state = 0;
	c->cycle = 0;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &excursionv_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
excursionv_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct excursionv_ctx *c;
	uint8_t *p, mask;
	unsigned int i;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;
	p = (uint8_t *)data;

	for (i = 0, mask = 0; i < nbits; i++, p++) {
		mask = (mask != 0) ? mask >> 1 : 0x80;
		c->state += (*p & mask) ? 1 : -1;
		if (c->state < -9 || c->state > 9)
			continue;
		if (c->state != 0)
			c->counts[c->state + 9]++;
		else
			c->cycle++;
	}
	c->nbits += nbits;

	return (0);
}

/* 
 * Todo: consider to return all 18 p-values and statistics.
 */
int
excursionv_final(struct tras_ctx *ctx)
{
	struct excursionv_ctx *c;
	double stats, pvmin, pvmax;
	int i, fail;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	c = ctx->context;
	if (c->nbits < EXCURSION_V_MIN_BITS)
		return (EALREADY);

	if (c->state != 0)
		c->cycle++;

	pvmin = pvmax = 0.0;

	for (i = 0, fail = 0; i < 18; i++) {
		stats = abs(c->counts[i] - c->cycle);
		stats = stats / sqrt(2.0 * c->cycle * (4.0 * abs(i - 9) - 2.0));
		c->pvalue[i] = erfc(stats);
		if (c->pvalue[i] < c->alpha)
			fail++;
		if (c->pvalue[i] < pvmin)
			pvmin = c->pvalue[i];
		if (c->pvalue[i] > pvmax)
			pvmax = c->pvalue[i];
	}

	ctx->result.status = fail ? TRAS_TEST_FAILED : TRAS_TEST_PASSED;	

	ctx->result.discard = 0;
	ctx->result.stats1 = 0.0;
	ctx->result.stats2 = 0.0;
	ctx->result.pvalue1 = pvmin;
	ctx->result.pvalue2 = pvmax;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
excursionv_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
excursionv_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
excursionv_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo excursionv_algo = {
	.name =		"ExcursionV",
	.desc =		"Random Excursion Variant Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		excursionv_init,
	.update =	excursionv_update,
	.test =		excursionv_test,
	.final =	excursionv_final,
	.restart =	excursionv_restart,
	.free =		excursionv_free,
};

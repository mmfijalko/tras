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

/*
 * Implementation of the Non-overlapping Template Matching Test.
 */

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>

#include <tras.h>
#include <utils/hamming8.h>
#include <ntmatch.h>

/*
 * Private context for the test.
 */
struct ntmatch_ctx {
	unsigned int	nbits;
	unsigned int *	w;	/* templates frequency table */
	unsigned int	m;
	unsigned int	M;
	unsigned int 	N;
	double		alpha;
};

int
ntmatch_init(struct tras_ctx *ctx, void *params)
{
	struct ntmatch_ctx *c;
	struct ntmatch_params *p = params;
	unsigned int i;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	if (p->m < NTMATCH_MIN_M)
		return (EINVAL);
	if (p->M < NTMATCH_MIN_SUBS_M)
		return (EINVAL);
	if (p->N < NTMATCH_MIN_N || p->N > NTMATCH_MAX_N)
		return (EINVAL);

	c = malloc(sizeof(struct ntmatch_ctx) + p->N * sizeof(unsigned int));
	if (c == NULL)
		return (ENOMEM);

	/* Initialize templates frequency table */
	c->w = (unsigned int *)(c + 1);
	for (i = 0; i < p->N; i++)
		c->w[i] = 0;

	c->nbits = 0;
	c->m = p->m;
	c->M = p->M;
	c->N = p->N;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &ntmatch_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
ntmatch_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	/*
	 * TODO: implementation.
	 */

	return (ENOSYS);
}

int
ntmatch_final(struct tras_ctx *ctx)
{
	struct ntmatch_ctx *c;
	double mean, var;
	double chi2, pvalue;
	unsigned int i;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

#ifdef __not_yet__
	mean = (double)c->M - (double)c->m + 1.0;
	var = (double)c->M;
	i = c->m;
	while (i > 0) {
		k = min(i, 16);
		mean = mean / (double)(1UL << k);
		i = i - k;
	}
#endif

	for (chi2 = 0.0, i = 0; i < c->N; i++)
		chi2 += ((double)c->w[i] - mean) * ((double)c->w[i] - mean);
	chi2 = chi2 / var;

#ifdef __not_yet__

	pvalue = igamc((double)c->N / 2.0, chi2 / 2.0);
#else
	pvalue = 0.0;
#endif
	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits % c->M;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0.0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
ntmatch_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
ntmatch_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
ntmatch_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo ntmatch_algo = {
	.name =		"ntmatch",
	.desc =		"Non-Overlapping Template Matching Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		ntmatch_init,
	.update =	ntmatch_update,
	.test =		tras_do_test,
	.final =	ntmatch_final,
	.restart =	tras_do_restart,
	.free =		tras_do_free,
};

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
 * Implementation for the Overlapping Template Matching Test.
 */

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <tras.h>
#include <hamming8.h>
#include <otmatch.h>

/*
 * Private context for the test.
 */
struct otmatch_ctx {
	unsigned int	nbits;		/* number of bits processed */
	unsigned int	m;
	uint8_t	*	B;
	unsigned int	K;
	unsigned int	M;
	unsigned int	N;
	unsigned int	v[5];
	double		alpha;		/* significance level fo H0 */
};

#define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))

int
otmatch_init(struct tras_ctx *ctx, void *params)
{
	struct otmatch_params *p = params;
	struct otmatch_ctx *c;
	size_t size;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	/*
	 * TODO: verify multiple parameters.
	 */

	c = malloc(sizeof(struct otmatch_ctx) + (p->m + 7) / 8);
	if (c == NULL)
		return (ENOMEM);

	c->B = __DECONST(uint8_t *, (c + 1));
	memcpy(c->B, p->B, (p->m + 7) / 8);

	memset(c->v, 0, sizeof(c->v));
	c->m = p->m;
	c->K = p->K;
	c->M = p->M;
	c->N = p->N;
	c->nbits = 0;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &otmatch_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
otmatch_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct otmatch_ctx *c;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	/*
	 * TODO: implementation.
	 */

	return (ENOSYS);
}

/*
 * Constant precomputed values of Pi(i) for chi-square 0..5.
 */
static const double pi[6] = {
	0.364091,	/* Pi#0 */
	0.185659,	/* Pi#1 */
	0.139381,	/* Pi#2 */
	0.100571,	/* Pi#3 */
	0.070432,	/* Pi#4 */
	0.139865,	/* Pi#4 */
};

int
otmatch_final(struct tras_ctx *ctx)
{
	struct otmatch_ctx *c;
	double lambda, ni;
	double pvalue;

	TRAS_CHECK_FINAL(ctx);

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	lambda = ((double)(c->M - c->m + 1)) / pow(2.0, (double)c->m);
	ni = lambda / 2.0;

	/* todo: implementation */
	pvalue = 0.0;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits % c->N;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0.0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
otmatch_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
otmatch_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
otmatch_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo otmatch_algo = {
	.name =		"otmatch",
	.desc =		"Overlapping Template Matching Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		otmatch_init,
	.update =	otmatch_update,
	.test =		otmatch_test,
	.final =	otmatch_final,
	.restart =	otmatch_restart,
	.free =		otmatch_free,
};

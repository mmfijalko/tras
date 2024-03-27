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
 * The Minimum Distance Test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include <tras.h>
#include <hamming8.h>
#include <utils.h>
#include <bits.h>
#include <bspace.h>

/*
 * The minimum distance test context.
 */
struct bspace_ctx {
	unsigned int	nbits;	/* number of bits processed */
	unsigned int	m;	/* number of birthdays */
	unsigned int	q;	/* number of bits for a day */
	unsigned int	n;	/* number of days in a year */
	unsigned int	nk;	/* number of K values for chi2 */
	unsigned int	ik;	/* current number of K values */
	unsigned int *	K;	/* number of pairs table */
	double *	pprob;	/* Poisson theoretical probabilities */
	unsigned int *	bdays;	/* birtdays list for single K */
	double		alpha;	/* significance level for H0 */
};

#include <assert.h>
#define	tras_assert(cond)	assert((cond))

static void
bspace_birtdays_sort(struct bspace_ctx *c)
{

	/* todo : sort the birthdays with quicksort */
}

static void
bspace_birtdays_spacing(struct bspace_ctx *c)
{
	unsigned int i;

	for (i = 1; i < c->nk; i++) {
		tras_assert(c->bdays[i] >= c->bdays[i - 1]);
		if (c->bdays[i] > (c->bdays[i - 1] + 1))
			c->K[c->ik]++;
	}
	c->ik++;
}

int
bspace_init(struct tras_ctx *ctx, void *params)
{
	struct bspace_ctx *c;
	struct bspace_params *p = params;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);
	if (p->n != (1 << p->q))
		return (EINVAL);

	/* todo: check m, n nk */

	c = malloc(sizeof(struct bspace_ctx) * p->nk * sizeof(unsigned int) +
	    p->nk * sizeof(double));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}
	c->K = (unsigned int *)(c + 1);
	c->pprob = (double *)(((char *)p->K) + p->nk * sizeof(unsigned int));

	/* todo: other initializations when defined */

	c->nbits = 0;
	c->m = p->m;
	c->q = p->q;
	c->n = p->n;
	c->nk = p->nk;
	memset(c->K, 0, c->nk * sizeof(unsigned int));

	ctx->context = c;
	ctx->algo = &bspace_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
bspace_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct bspace_ctx *c;
	unsigned int r, b;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);

	c = ctx->context;

	b = c->m * sizeof(unsigned int);
	b = b * 8;
	r = c->nbits % b;

	(void)r;

	/* todo: implementation */

#ifdef __full_bunch_of_birthdays__
	bspace_birtdays_sort(c);
	bspace_birtdays_spacing(c);
#endif

	c->nbits += nbits;

	return (0);
}

int
bspace_final(struct tras_ctx *ctx)
{
	struct bspace_ctx *c;
	double pvalue, sobs;
	int sum, lambda;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	c = ctx->context;
	if (c->ik < c->nk)
		return (EALREADY);

	/* todo: implementation */

	pvalue = 0.0;

	/* Compute the Poisson distribution parameter */
	lambda = (pow((double)c->m, 3.0) / 4.0 / (double)c->n);
	(void)lambda;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - c->nk * c->m * c->q;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
bspace_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
bspace_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
bspace_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo bspace_algo = {
	.name =		"bspace",
	.desc =		"The Birthday Spacing Test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		bspace_init,
	.update =	bspace_update,
	.test =		bspace_test,
	.final =	bspace_final,
	.restart =	bspace_restart,
	.free =		bspace_free,
};

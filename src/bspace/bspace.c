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
 * The Birthday Spacings Test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <tras.h>
#include <cdefs.h>
#include <utils.h>
#include <bits.h>
#include <bspace.h>

#include <stdio.h>

/*
 * The single birthday spacings test context.
 */
struct sbs_ctx {
	unsigned int	m;	/* number of birthdays */
	unsigned int	n;	/* number of days in a year */
	unsigned int	q;	/* number of bits for a day */
	unsigned int	b;	/* shift/bit offset for integer */
//	unsigned int	ik;	/* current number of K values */
//	unsigned int *	K;	/* ??? */
//	double *	pprob;	/* Poisson theoretical probabilities */
	unsigned int *	bdays;	/* birthdays list for single K */
	unsigned int *	intvs;	/* intervals table */
	unsigned int	nbits;	/* number of bits processed */
	double		alpha;	/* significance level for H0 */
};

#define	tras_assert(cond)	assert((cond))

static void
quicksort_int(unsigned int *table, int l, int r)
{
	unsigned int p, v, t, a;
	unsigned int i, j;

	if (l >= r || l < 0)
		return;

	/*
	 * Use simple "in the middle" for pivot point.
	 */
	p = l + (r - l) / 2;
	v = table[p];

	/*
	 * Swap pivot value with last element.
	 */
	t = table[r];
	table[r] = v;
	table[p] = t;

	a = l;
	for (i = l; i < r; i++) {
		if (table[i] < v) {
			t = table[i];
			table[i] = table[a];
			table[a] = t;
			a++;
		}
	}

	t = table[a];
	table[a] = table[r];
	table[r] = t;

	quicksort_int(table, l, a - 1);
	quicksort_int(table, a + 1, r);
}

static void
quicksort(unsigned int *table, int n)
{

	quicksort_int(table, 0, n - 1);
}

inline static void
sbs_sort_bdays(struct sbs_ctx *c)
{

	quicksort(c->bdays, c->m);
}

inline static void
sbs_sort_intvs(struct sbs_ctx *c)
{

	quicksort(c->intvs, c->m);
}

static inline uint32_t
bswap32(void *d)
{
	uint8_t *p = (uint8_t *)d;

	return (((uint32_t)p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0]);
}

static inline uint32_t
rotate32(uint32_t u, int off)
{

	return ((u << (31 - off)) | (u >> off));
}

static inline uint32_t
sbs_be32enc(void *d)
{
	uint8_t *p = (uint8_t *)d;

	return (((uint32_t)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

/*
 * TODO: To implement. Temporary take 24 LSb.
 */
static inline uint32_t
sbs_extract32(void *d, int o, int q)
{

	return (sbs_be32enc(d) & 0x00ffffff);
}

static int
bspace_verify_params(struct bspace_params *p, int single)
{

	if ((p->q == 0) || (p->q > 31) || (p->n != (1 << p->q)))
		return (EINVAL);
	if (p->b < BSPACE_MIN_BIT_OFFSET || p->b > BSPACE_MAX_BIT_OFFSET)
		return (EINVAL);
	if (p->m == 0)
	       return (EINVAL);
	if (single && p->jn != 1)
		return (EINVAL);
	if (!single && ((p->jn < BSPACE_MIN_JN || p->jn > BSPACE_MAX_JN)))
		return (EINVAL);

	/* todo: check m, n */

	return (0);
}

int
sbs_init(struct tras_ctx *ctx, void *params)
{
	struct bspace_params *p = params;
	struct sbs_ctx *c;
	size_t size;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	error = bspace_verify_params(p, 1);
	if (error != 0)
		return (error);

	size = sizeof(struct sbs_ctx) + p->m * sizeof(uint32_t) +
	    p->m * sizeof(uint32_t);

	error = tras_init_context(ctx, &sbs_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);

	c = ctx->context;

	c->bdays = (unsigned int *)(c + 1);
	c->intvs = c->bdays + p->m;
//	c->pprob = (double *)(c->intvs + p->m);

	/* todo: other initializations when defined */

	c->b = 7 - p->b;
	c->m = p->m;
	c->q = p->q;
	c->n = p->n;
	c->alpha = p->alpha;

	return (0);
}

int
sbs_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct sbs_ctx *c;
	unsigned int n, i, b;
	uint32_t *p;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	if (nbits & 0x1f)
		return (EINVAL);

	c = ctx->context;
	p = (uint32_t *)data;

	b = c->nbits / 32;
	n = miss(b, c->m);
	n = min(n, nbits / 32);
	n = n + b;

	for (i = b; i < n; i++, p++)
		c->bdays[i] = sbs_extract32(p, c->b, c->q);
	c->nbits += nbits;

	return (0);
}

static double
sbs_poison_pdf(int k, double lambda)
{

	return (exp(-lambda) * pow(lambda, k) / tgamma(k + 1));
}

int
sbs_final(struct tras_ctx *ctx)
{
	struct sbs_ctx *c;
	double pvalue, sobs, lambda;
	unsigned int i, K, diff;
	int sum;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nbits / 32 < c->m)
		return (EALREADY);

	/*
	 * Sort generated birthdays, let them be positioned as in calendar.
	 */
	sbs_sort_bdays(c);

	for (i = 0; i < c->m; i++)
		printf("b[%d] = %u\n", i, c->bdays[i]);
	printf("\n");

	/*
	 * Create intervals.
	 */
	c->intvs[0] = c->bdays[0];
	for (i = 1; i < c->m; i++)
		c->intvs[i] = c->bdays[i] - c->bdays[i - 1];

	for (i = 0; i < c->m; i++)
		printf("intv[%d] = %u\n", i, c->intvs[i]);
	printf("\n");

	/*
	 * Sort intervals list.
	 */
	sbs_sort_intvs(c);

	for (i = 0; i < c->m; i++)
		printf("intv[%d] = %u\n", i, c->intvs[i]);
	printf("\n");

	/*
	 * Count spacing values between birthdays that occurs more than once.
	 */
	for (i = 0, K = 0, diff = 1; i < c->m - 1; i++) {
		if (c->intvs[i] == c->intvs[i + 1]) {
			if (diff)
				K++;
			diff = 0;
		} else {
			diff = 1;
		}
	}

	printf("%s: final K = %u\n", __func__, K);

	/*
	 * todo: implementation.
	 */

	pvalue = 0.0;

	/*
	 * Compute the Poisson distribution parameter.
	 */
	lambda = (pow((double)c->m, 3.0) / 4.0 / (double)c->n);
	(void)lambda;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - 32 * c->m;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0.0;
	ctx->result.stats1 = (double)K;
	ctx->result.stats2 = 0.0;

	tras_fini_context(ctx, 0);

	return (0);
}

int
sbs_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
sbs_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
sbs_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo sbs_algo = {
	.name =		"births",
	.desc =		"The single Birthday Spacing Test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		sbs_init,
	.update =	sbs_update,
	.test =		sbs_test,
	.final =	sbs_final,
	.restart =	sbs_restart,
	.free =		sbs_free,
};

/*
 * The first level birthday spacing test.
 */
struct bspace_ctx {
	unsigned int		jidx;	/* index of current statistics */
	unsigned int		jmax;	/* maximum number of statistics */
	unsigned int *		jtab;	/* table of K statistics */
	unsigned int		nbits;	/* number of bits updated */
	double			alpha;	/* significance level for H0 */
	struct sbs_ctx		bsctx;	/* context for single birtdady test */
	struct tras_ctx		sbctx;	/* tras context for single test */
	struct bspace_params	param;	/* the single test params */
};

int
bspace_init(struct tras_ctx *ctx, void *params)
{
	struct bspace_params *p = params;
	struct bspace_ctx *c;
	size_t size;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	error = bspace_verify_params(p, 0);
	if (error != 0)
		return (error);

	size = sizeof(struct bspace_ctx) + p->jn * sizeof(unsigned int);

	error = tras_init_context(ctx, &bspace_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);

	c = ctx->context;

	c->param.m = p->m;
	c->param.n = p->n;
	c->param.b = p->b;
	c->param.q = p->q;
	c->param.jn = 1;
	c->param.alpha = p->alpha;

	c->jtab = (unsigned int *)(c + 1);
	c->jmax = p->jn;
	c->jidx = 0;
	c->alpha = p->alpha;

	return (0);
}

int
bspace_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct bspace_ctx *c;
	struct sbs_ctx *sbc;
	unsigned int i, k, sn;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context

	if (nbits & 0x1f)
		return (EINVAL);

	/* The number of words updated */
	b = c->nbits / 32;

	/* How many single statistics to update */
	n = miss(c->jidx, c->jmax);

	/* The number of words for single birthday spacing test */
	sn = 

	n = min(n, nbits / 32);
	n = n + b;

	if (c->jidx >= c->max) {
		c->nbits += nbits;
		return (0);
	}

	ctx->nbits += nbits;

	return (0);
}

int
bspace_final(struct tras_ctx *ctx)
{
	struct bspace_ctx *c;
	double pvalue;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->jidx < c->jmax)
		return (EALREADY);

	/* XXX: temporary */
	pvalue = 0.0;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = 0; /* ??? */
	ctx->result.pvalue1 = pvalue;

	tras_fini_context(ctx, 0);

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



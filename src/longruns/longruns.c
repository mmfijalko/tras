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
#include <igamc.h>
#include <hamming8.h>
#include <longruns.h>

#include <stdio.h>

struct longruns_classes {
	unsigned int	M;		/* the length of each block */
	unsigned int	K;		/* the degrees of freedom */
	const double *	pi;		/* chi2 classes probabilities */
};

/*
 * The longest run of ones in a block test context.
 */
struct longruns_ctx {
	unsigned int *	v;		/* the frequency table */
	unsigned int	nbmax;		/* max number of bits */
	unsigned int	nblks;		/* full blocks updated */
	unsigned int	M;		/* the length of each block */
	unsigned int	N;		/* the number of blocks */
	unsigned int	run;		/* runs length for update */
	unsigned int	maxrun;		/* block maximum run length */
	unsigned int	nbits;		/* number of bits processed */
	double		alpha;		/* significance level for H0*/
int version;
};

/*
 * The probabilities for classes with M = 8, K = 3
 */
static const double longruns_pi_0[] = {
	0.2148, 0.3672, 0.2305, 0.1875,
};

/*
 * The probabilities for classes with M = 128, K = 5
 */
static const double longruns_pi_1[] = {
	0.1174, 0.2430, 0.2493, 0.1752, 0.1027, 0.1124,
};

/*
 * The probabilities for classes with M = 512, K = 5
 */
static const double longruns_pi_2[] = {
	0.1170, 0.2460, 0.2523, 0.1755, 0.1027, 0.1124
};

/*
 * The probabilities for classes with M = 1000, K = 5
 */
static const double longruns_pi_3[] = {
	0.1307, 0.2437, 0.2452, 0.1714, 0.1002, 0.1088
};

/*
 * The probabilities for classes with M = 10000, K = 6
 */
static const double longruns_pi_4[] = {
	0.0882, 0.2092, 0.2483, 0.1933, 0.1208, 0.0675, 0.0727
};

/*
 * The list of supported lengths of blocks K param and probabilities.
 */
static const struct longruns_classes longruns_classes[] = {
	{ .M = LONGRUNS_M0, .K = 3, .pi = longruns_pi_0 },
	{ .M = LONGRUNS_M1, .K = 5, .pi = longruns_pi_1 },
	{ .M = LONGRUNS_M2, .K = 5, .pi = longruns_pi_2 },
	{ .M = LONGRUNS_M3, .K = 5, .pi = longruns_pi_3 },
	{ .M = LONGRUNS_M4, .K = 6, .pi = longruns_pi_4 },
	{ .M = 0, .K = 0, .pi = NULL} /* sentinel */
};

/*
 * The function return class index for frequency table.
 */
inline static unsigned int
longruns_category_index(unsigned int head, unsigned int tail, unsigned int lrun)
{

	if (lrun > tail)
		return (tail - head);
	if (lrun < head)
		return (0);
	return (lrun - head);
}

/*
 * Increment element of the frequency table depending on M.
 */
inline static void
longruns_category_inc(struct longruns_ctx *c, unsigned int lrun)
{
	unsigned int index;

	switch (c->M) {
	case LONGRUNS_M0:
		index = longruns_category_index(1, 4, lrun);
		break;
	case LONGRUNS_M1:
		index = longruns_category_index(4, 9, lrun);
		break;
	case LONGRUNS_M2:
		index = longruns_category_index(6, 11, lrun);
		break;
	case LONGRUNS_M3:
		index = longruns_category_index(7, 12, lrun);
		break;
	case LONGRUNS_M4:
		index = longruns_category_index(10, 16, lrun);
		break;
	}
	c->v[index]++;
}

inline static const struct longruns_classes *
longruns_find_classes(unsigned int M)
{
	const struct longruns_classes *cl = &longruns_classes[0];

	while (cl->pi != NULL) {
		if (cl->M == M)
			return (cl);
		cl++;
	}
	return (NULL);
}

int
longruns_init(struct tras_ctx *ctx, void *params)
{
	struct longruns_params *p = params;
	const struct longruns_classes *cl;
	struct longruns_ctx *c;
	size_t size;
	int i, error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

if (p->version != 1 && p->version != 2)
	return (EINVAL);

	if ((cl = longruns_find_classes(p->M)) == NULL)
		return (EINVAL);

	size = sizeof(struct longruns_ctx) + (cl->K + 1) * sizeof(unsigned int);

	error = tras_init_context(ctx, &longruns_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);

	c = ctx->context;
	
	c->v = (unsigned int *)(c + 1);
	c->M = p->M;
	c->N = p->N;
	c->nbmax = p->M * p->N;
	c->alpha = p->alpha;

c->version = p->version;

	return (0);
}

/*
 * Slow version of the update algorithm, scan bit by bit.
 */
static int
longruns_update1(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct longruns_ctx *c;
	unsigned int i, n, k, o, b, run;
	uint8_t *p, mask;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context;

	n = min(c->nbmax, c->nbits);
	n = min(nbits, c->nbmax - n);

	o = c->nbits % c->M;
	b = 0;

	while (n > 0) {
		k = c->M - o;
		k = min(k, n);
		p = (uint8_t *)data + b / 8;
		run = (o != 0) ? c->run : 0;
		mask = 0x80 >> (b & 0x07);
		for (i = 0; i < k; i++, mask >>= 1) {
			if (mask == 0x00) {
				mask = 0x80;
				p++;
			}
			if (*p & mask) {
				run++;
			} else {
				if (run > c->maxrun)
					c->maxrun = run;
				run = 0;
			}
		}
		if (run > c->maxrun)
			c->maxrun = run;
		if (o == c->M) {
			longruns_category_inc(c, c->maxrun);
			c->maxrun = 0;
			run = 0;
			o = 0;
			c->nblks++;
		} else {
			o = o + k;
		}
		b = b + k;
		n = n - k;
	}
	c->run = run;

	c->nbits += nbits;

	return (0);
}

/*
 * The idea of a little bit faster algorithm. Still idea.
 */
static int
longruns_update2(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct longruns_ctx *c;
	unsigned int n, k, o;
	(void)c;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context;

	/*
	 * TODO: implementation.
	 */
	return (ENOSYS);
}

int
longruns_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct longruns_ctx *c;
	unsigned int n, k, o;
	(void)c;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context;

	switch (c->version) {
	case 1:
		return (longruns_update1(ctx, data, nbits));
	case 2:
		return (longruns_update2(ctx, data, nbits));
	}
	return (ENXIO);
}

int
longruns_final(struct tras_ctx *ctx)
{
	struct longruns_ctx *c;
	const struct longruns_classes *cl;
	double pvalue, stats;
	double npi;
	unsigned int i;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nbits < c->nbmax)
		return (EALREADY);

	if ((cl = longruns_find_classes(c->M)) == NULL)
		return (EINVAL);

	for (i = 0, stats = 0.0; i <= cl->K; i++) {
		npi = c->N * cl->pi[i];
		stats += (c->v[i] - npi) * (c->v[i] - npi) / npi;
	}

	pvalue = igamc((double)cl->K / 2, stats / 2);

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - c->nbmax;
	ctx->result.stats1 = stats;
	ctx->result.pvalue1 = pvalue;

	tras_fini_context(ctx, 0);

	return (0);
}

int
longruns_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
longruns_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
longruns_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo longruns_algo = {
	.name =		"Longruns",
	.desc =		"The Longest Run of Ones in a Block",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		longruns_init,
	.update =	longruns_update,
	.test =		longruns_test,
	.final =	longruns_final,
	.restart =	longruns_restart,
	.free =		longruns_free,
};

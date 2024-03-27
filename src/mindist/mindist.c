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
#include <mindist.h>

			#include <stdio.h>

/*
 * Vector respresenting point in 2D space.
 */
struct point {
	double	x;		/* x component */
	double	y;		/* y component */
};

/*
 * The minimum distance test context.
 */
struct mindist_ctx {
	unsigned int	nbits;	/* number of bits processed */
	struct point *	points;	/* list of points collected */
	unsigned int	npoint;	/* number of points collected */
	unsigned int	K;	/* number of points to get */
	double		alpha;	/* significance level for H0 */
};

static double
mindist_distance_euclidean_pow2(struct point *p1, struct point *p2)
{
	double dx, dy;

	dx = (p2->x - p1->x);
	dy = (p2->y - p1->y);

	return (dx * dx + dy * dy);
}

inline static double
mindist_uint_to_floatU01(uint32_t u32)
{

	/* XXX: this is wrong function that need to be corrected
	 * because since u32 is uniformly distributed the
	 * return value is not.
	 */
	return ((double)u32 / pow(2.0, 32));
}

inline static double
mindist_point_component(uint32_t u32)
{

	return ((mindist_uint_to_floatU01(u32) * 10000.0));
}

static double
mindist_distance_min_pow2(struct point *points, unsigned int n)
{
	struct point *p1, *p2;
	unsigned int i, j;
	double d2min, d2;

	d2min = mindist_distance_euclidean_pow2(points, points + 1);

	for (i = 0, p1 = points; i < n - 1; i++, p1++) {
		for (j = i + 1, p2 = p1 + 1; j < n; j++, p2++) {
			d2 = mindist_distance_euclidean_pow2(p1, p2);
			if (d2 < d2min)
				d2min = d2;
		}
	}
	return (d2min);
}

int
mindist_init(struct tras_ctx *ctx, void *params)
{
	struct mindist_ctx *c;
	struct mindist_params *p = params;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (p->K < MINDIST_MIN_POINTS || p->K > MINDIST_MAX_POINTS)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	c = malloc(sizeof(struct mindist_ctx) + p->K * sizeof(struct point));
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}
	c->points = (struct point *)(c + 1);
	c->npoint = 0;
	c->nbits = 0;
	c->K = p->K;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &mindist_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

int
mindist_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct mindist_ctx *c;
	unsigned int n;
	uint32_t *d;
	double *v;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	if (nbits == 0 || nbits & 0x01f)
		return (EINVAL);

	c = ctx->context;
	d = (uint32_t *)data;	/* endianism ??? */
	n = c->nbits / 32;
	v = ((double *)c->points) + n;
	c->nbits += nbits;

	while (nbits > 0) {
		if (c->npoint >= c->K)
			break;
		*v = mindist_point_component(*d);
		v++;
		d++;
		n++;
		if ((n & 0x01) == 0)
			c->npoint++;
		nbits -= 32;
	}
	return (0);
}

int
mindist_final(struct tras_ctx *ctx)
{
	struct mindist_ctx *c;
	double pvalue, d2min, mean;
	int sum;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	c = ctx->context;
	if (c->npoint < c->K)
		return (EALREADY);

	/*
	 * TODO: comment about statistics.
	 */

	d2min = mindist_distance_min_pow2(c->points, c->K);
	mean = 0.995;
	pvalue = 1.0 - exp(-d2min / mean);

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - c->K * 64;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = d2min;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

int
mindist_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
mindist_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
mindist_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

const struct tras_algo mindist_algo = {
	.name =		"mindist",
	.desc =		"The Minimum Distance Test",
	.id =		0,
	.version =	{ 0, 1, 1 },
	.init =		mindist_init,
	.update =	mindist_update,
	.test =		mindist_test,
	.final =	mindist_final,
	.restart =	mindist_restart,
	.free =		mindist_free,
};

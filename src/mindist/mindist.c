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
#include <float.h>
#include <limits.h>
#include <math.h>

#include <tras.h>
#include <cdefs.h>
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

/*
 * Swap two points in the table of points.
 */
inline static void
swap_points(struct point *table, int a, int b)
{
	double x, y;

	x = table[a].x;
	y = table[a].y;
	table[a].x = table[b].x;
	table[a].y = table[b].y;
	table[b].x = x;
	table[b].y = y;
}

/*
 * Sort the collected list of points in ascending order with x-axis.
 */
static void
quicksort_int(struct point *table, int l, int r)
{
	unsigned int p, a;
	unsigned int i, j;
	double v;

	if (l >= r || l < 0)
		return;

	/*
	 * Use simple "in the middle" for pivot point.
	 */
	p = l + (r - l) / 2;
	v = table[p].x;

	/*
	 * Swap pivot value with last element.
	 */
	swap_points(table, p, r);

	for (a = l, i = l; i < r; i++) {
		if (table[i].x < v) {
			swap_points(table, i, a);
			a++;
		}
	}
	swap_points(table, a, r);

	quicksort_int(table, l, a - 1);
	quicksort_int(table, a + 1, r);
}

/*
 * Quick sort a list of points.
 */
inline static void
quicksort(struct point *table, int n)
{

	quicksort_int(table, 0, n - 1);
}

/*
 * Quick sort the collected list of points for minimimum distance algorithm.
 */
inline static void
quicksort_points(struct mindist_ctx *c)
{

	quicksort(c->points, c->K);
}

/*
 * The function returns the Euclidean distance between two points
 * raised to the power of 2.
 */
static double
mindist_distance_euclidean_pow2(struct point *p1, struct point *p2)
{
	double dx, dy;

	dx = (p2->x - p1->x);
	dy = (p2->y - p1->y);

	return (dx * dx + dy * dy);
}

/*
 * Convert 32-bit unsigned int value to the double in the range <0.0, 1.0)
 */
inline static double
mindist_uint_to_floatU01(uint32_t u32)
{

	/* XXX: this is wrong function that need to be corrected
	 * because since u32 is uniformly distributed the
	 * return value is not.
	 */
	return ((double)u32 / pow(2.0, 32));
}

/*
 * Normalize the single point component to <0, 1000> range.
 */
inline static double
mindist_point_component(uint32_t u32)
{

	return ((mindist_uint_to_floatU01(u32) * 10000.0));
}

static double
mindist_min_distance_pow2(struct point *points, unsigned int n)
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

static double
mindist_min_distance_pow2_sorted(struct point *points, unsigned int n)
{
	struct point *p1, *p2;
	unsigned int i, j;
	double d2min, d2;

	quicksort(points, n);

	d2min = DBL_MAX;

	for (i = 0, p1 = points; i < n - 1; i++, p1++) {
		for (j = i + 1, p2 = p1 + 1; j < n; j++, p2++) {
			if (abs(p1->x - p2->x) >= sqrt(d2min))
				break;
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
	struct mindist_params *p = params;
	struct mindist_ctx *c;
	size_t size;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	if (p->K < MINDIST_MIN_POINTS || p->K > MINDIST_MAX_POINTS)
		return (EINVAL);

	size = sizeof(struct mindist_ctx) + p->K * sizeof(struct point);

	error = tras_init_context(ctx, &mindist_algo, size, TRAS_F_ZERO);
	if (error != 0)
		return (error);

	c = ctx->context;
	c->points = (struct point *)(c + 1);

	c->K = p->K;
	c->alpha = p->alpha;

	return (0);
}

int
mindist_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct mindist_ctx *c;
	unsigned int n, b;
	struct point *p;
	uint32_t *d;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	if (nbits == 0)
		return (0);
	if (nbits & 0x01f)
		return (EINVAL);

	c = ctx->context;
	d = (uint32_t *)data;	/* endianism ??? */
	p = c->points + c->npoint;

	/*
	 * Get number of coordinates to update.
	 */
	b = c->nbits / 32;
	b = min(b, 2 * c->K);
	n = 2 * c->K - b;
	n = min(n, nbits / 32);

	while (n > 0) {
		if (b & 0x01) {
			p->y = mindist_point_component(*d);
			c->npoint++;
			p++;
		} else {
			p->x = mindist_point_component(*d);
		}
		b++;
		d++;
		n--;
	}
	c->nbits += nbits;

	return (0);
}

#ifdef MINDIST_DEBUG
static void
mindist_show_points(struct mindist_ctx *c)
{
	unsigned int i;

	for (i = 0; i < c->K; i++) {
		printf("point %d : <%.16f, %.16f>\n", i,
		    c->points[i].x, c->points[i].y);
	}
}

static int
mindst_quicksort_verify(struct mindist_ctx *c)
{
	unsigned int i;


	for (i = 0; i < c->K - 1; i++) {
		if (c->points[i].x > c->points[i + 1].x)
			return (EINVAL);
	}
	return (0);
}
#endif

int
mindist_final(struct tras_ctx *ctx)
{
	struct mindist_ctx *c;
	double pvalue, d2min, mean;
	int sum;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->npoint < c->K)
		return (EALREADY);

	d2min = mindist_min_distance_pow2_sorted(c->points, c->K);
	mean = 0.995;
	pvalue = 1.0 - exp(-d2min / mean);

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - c->K * 64;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = d2min;

	tras_fini_context(ctx, 0);

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

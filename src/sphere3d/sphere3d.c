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
 * The 3D spheres test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tras.h>
#include <sphere3d.h>

#define	min(a, b)	(((a) < (b)) ? (a) : (b))

#define	SPHERE3D_ID_FULL_NUMBERS	0
#define	SPHERE3D_ID_PART_UPDATES	1

#define	NBITS(b)			((b) * 8)

#define	SPHERE3D_COMPONENT_BITS		NBITS(sizeof(double))	

/*
 * Vector representing point in 3D space.
 */
struct point {
	double	x;		/* x component */
	double	y;		/* y component */
	double	z;		/* z component */
};

/*
 * The context structure for the 3D spheres test.
 */
struct sphere3d_ctx {
	unsigned int	nbits;	/* number of bits processed */
	unsigned int	ntups;	/* number of tuples stored */
	struct point *	points;	/* points list for update */
	unsigned int	npoint;	/* number of points updated */
	unsigned int	K;	/* */
	double		alpha;	/* significance level for H0 */
};

/*
 * Initialize the 3D spheres test.
 */
int
sphere3d_init(struct tras_ctx *ctx, void *params)
{
	struct sphere3d_ctx *c;
	struct sphere3d_params *p = params;
	size_t size;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (p->K < SPHERE3D_MIN_TRIPLETS || p->K > SPHERE3D_MAX_TRIPLETS)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	/*
	 * Unfortunatelly we have to store all points on consecutive
	 * updates until final state. It should not be a problem
	 * to malloc and keep all incomming data when K is limited.
	 */
	size = sizeof(struct sphere3d_ctx) + p->K * sizeof(struct point);
	c = malloc(size);
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
	ctx->algo = &sphere3d_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

static double
sphere3d_distance_axis_min(struct point *p1, struct point *p2)
{
	double dx, dy, dz;

	dx = fabs(p2->x - p1->x);
	dy = fabs(p2->y - p1->y);
	dy = fabs(p2->z - p1->z);

	return (min(min(dx, dy), dz));
}

static double
sphere3d_distance_euclidean_pow2(struct point *p1, struct point *p2)
{
	double dx, dy, dz;

	dx = (p2->x - p1->x);
	dy = (p2->y - p1->y);
	dz = (p2->z - p1->z);

	return (dx * dx + dy * dy + dz * dz);
}

static double
sphere3d_distance_min_pow2(struct point *points, unsigned int n)
{
	struct point *p1, *p2;
	unsigned int i, j;
	double dmin, d;

	dmin = sphere3d_distance_euclidean_pow2(points, points + 1);

	for (i = 0, p1 = points; i < n - 1; i++, p1++) {
		for (j = i + 1, p2 = p1 + 1; j < n; j++, p2++) {
			d = sphere3d_distance_euclidean_pow2(p1, p2);
			if (d < dmin)
				dmin = d;
		}
	}
	return (dmin);
}

inline static double
sphere3d_uint_to_floatU01(uint32_t u32)
{

	/* XXX: this is wrong function that need to be corrected
	 * because since u32 is uniformly distributed the
	 * return value is not.
	 */
	return ((double)u32 / pow(2.0, 32));
}

inline static double
sphere3d_point_component(uint32_t u32)
{

	return ((sphere3d_uint_to_floatU01(u32) * 1000.0));
}

/*
 * Update state of the 3D spheres test with subsequent binary sequence.
 */
int
sphere3d_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct sphere3d_ctx *c;
	unsigned int n;
	uint32_t *d;
	double *v;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	if (nbits == 0 || (nbits & 0x1f))
		return (EINVAL);

	/* TODO: no check for number of points processed */

	c = ctx->context;

	d = (uint32_t *)data;
	n = c->nbits / 32;
	v = ((double *)c->points) + n;
	c->nbits += nbits;
	while (nbits > 0) {
		if (c->npoint >= c->K)
			break;
		*v = sphere3d_point_component(*d);
		v++;
		d++;
		n++;
		if ((n % 3) == 0)
			c->npoint++;
		nbits -= 32;
	}

	return (0);
}

/*
 * Finalize the 3D sphereds test and determine its result.
 */
int
sphere3d_final(struct tras_ctx *ctx)
{
	struct sphere3d_ctx *c;
	double pvalue, mean;
       	double r1min, r2min, r3min;

	if (ctx == NULL)
		return (EINVAL);
	c = ctx->context;
	if (c == NULL || ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	if (c->npoint < SPHERE3D_MIN_TRIPLETS)
		return (EALREADY);

	/*
	 * Cubed min radius is expotential with mean 30. Thus,
	 * the volume of the sphere with dmin radius is very close
	 * expotentially distributed with mean 120 * Pi / 3. No need
	 * to calculate volume, using radius.
	 */

	r2min = sphere3d_distance_min_pow2(c->points, c->K);
	r1min = sqrt(r2min);
	r3min = r1min * r2min;
	mean = 30.0;
	pvalue = 1.0 - exp(-r3min / mean);

	/* Determine and store results */
	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - SPHERE3D_MIN_NBITS;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0.0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

/*
 * The 3D spheres test for update and finalize in one call.
 */
int
sphere3d_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

/*
 * Restart the initialized 3D spheres test.
 */
int
sphere3d_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

/*
 * Uninitialize the 3D spheres test and free its allocated resources.
 */
int
sphere3d_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

/*
 * The 3D spheres test algorithm description for tras software.
 */
const struct tras_algo sphere3d_algo = {
	.name =		"sphere3d",
	.desc =		"3D Spheres Test",
	.id =		SPHERE3D_ID_FULL_NUMBERS,
	.version = 	{ 0, 1, 1 },
	.init =		sphere3d_init,
	.update =	sphere3d_update,
	.test =		sphere3d_test,
	.final =	sphere3d_final,
	.restart =	sphere3d_restart,
	.free =		sphere3d_free,
};

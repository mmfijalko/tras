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
 * The Parking Lot Test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <math.h>

#include <tras.h>
#include <plot.h>

#define	min(a, b)		(((a) < (b)) ? (a) : (b))
#define	max(a, b)		(((a) > (b)) ? (a) : (b))

/*
 * The point structure to represent car position.
 */
struct point {
	double	x;
	double	y;
};

/*
 * The context structure for the parking lot test.
 */
struct plot_ctx {
	unsigned int	nbits;	/* number of bits processed */
	struct point *	cars;	/* list of cars parked */
	unsigned int	ncars;	/* number of cars parked */
	unsigned int	tries;	/* number of park attempts */
	unsigned int	nslot;	/* size of table for cars */
	unsigned int	bumps;	/* number of collisions */
	uint8_t		buf[8];	/* to store incomplete point */
	double		alpha;	/* significance level ??? */
};

/*
 * Initialize the parking lot test, non-parameters test.
 */
int
plot_init(struct tras_ctx *ctx, void *params)
{
	struct plot_ctx *c;
	struct plot_params *p = params;
	size_t size;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (p->idist != PARKING_LOT_IDIST_COORD_MAX)
		return (EINVAL);

	/*
	 * Notice: only maximum coordinates as distance function.
	 */

	/*
	 * Only 3523 points park successfully in average. Half of slots
	 * should be enough. If not, the udpate method will resize and
	 * reallocate the cars positions list.
	 */
	c = malloc(sizeof(struct plot_ctx));
	if (c == NULL)
		return (ENOMEM);
	c->cars = malloc(100 * PARKING_LOT_MAX_CARS / 2 * sizeof(struct point));
	if (c->cars == NULL) {
		free(c);
		return (ENOMEM);
	}
	c->nbits = 0;
	c->ncars = 0;
	c->tries = 0;
	c->nslot = 100 * PARKING_LOT_MAX_CARS / 2;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &plot_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

/*
 * Wrong function of distance as for description in many places
 * that the distance is min(|x2 - x1|, |y2 - y1| and should be
 * greater than 1 to successfully park a car.
 */
static double
plot_distance_axis_max(struct point *p1, struct point *p2)
{
	double dx, dy;

	dx = fabs(p2->x - p1->x);
	dy = fabs(p2->y - p1->y);

	return (max(dx, dy));
}

/*
 * Experimentally discovered that the measurement function is correct.
 */
static double
plot_distance_axis_min(struct point *p1, struct point *p2)
{
	double dx, dy;

	dx = fabs(p2->x - p1->x);
	dy = fabs(p2->y - p1->y);

	return (min(dx, dy));
}

/*
 * Wrong function of distance as mentioned in description on the wiki
 * or other places; "n a square of side 100, randomly "park" a car –
 * a circle of radius 1." The function produces mean value for successfully
 * parked cars about 4080.
 */
static double
plot_distance_euclidean(struct point *p1, struct point *p2)
{
	double dx, dy;

	dx = (p2->x - p1->x);
	dy = (p2->y - p1->y);

	return (sqrt(dx * dx + dy * dy));
}

static double
tras_uint_to_floatU01(uint32_t u32)
{

	/* XXX: this is wrong function that need to be corrected
	 * because since u32 is uniformly distributed the
	 * return value is not.
	 */
	return ((double)u32 / pow(2.0, 32));
}

inline static double
plot_uint_to_component(uint32_t u32)
{

	return (tras_uint_to_floatU01(u32) * 100.0);
}

static int
plot_reallocate(struct plot_ctx *c)
{
	struct point *list;
	unsigned int nslot;

	nslot = 3 * c->nslot / 2;
	nslot = min(nslot, PARKING_LOT_MAX_CARS);
	if (c->nslot >= nslot)
		return (ENXIO);

	list = malloc(nslot * sizeof(struct point));
	if (list == NULL)
		return (ENOMEM);
	bcopy(c->cars, list, c->ncars * sizeof(struct point));
	free(c->cars);
	c->cars = list;
	c->nslot = nslot;
	
	return (0);
}

static int
plot_park_attempt(struct plot_ctx *c, struct point *car)
{
	unsigned int i;
	double d;
	int error;

	c->tries++;

	for (i = 0; i < c->ncars; i++) {
		d = plot_distance_axis_max(&c->cars[i], car);
		if (d <= 1.0) {
			c->bumps++;
			return (0);
		}
	}
	if (c->ncars >= c->nslot) {
		error = plot_reallocate(c);
		if (error != 0)
			return (error);
	}

	c->cars[c->ncars].x = car->x;
	c->cars[c->ncars].y = car->y;
	c->ncars++;

	return (0);
}

/*
 * Update state of the parking lot test with subsequent binary sequence.
 */
int
plot_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	static int idx;
	struct plot_ctx *c;
	uint32_t *p;
	struct point car;
	unsigned int r, n, i;
	int error;
	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	if (nbits & 0x3f)
		return (EINVAL);

	c = ctx->context;
	p = (uint32_t *)data;

	r = c->nbits & 0x3f;
	if (r > 0) {
return (EINVAL);
		/* todo: try to complete the car point */
		n = nbits >> 6;
		for (i = 0; i < n; i++) {
			/* todo: get two 32-bit substring as x, y coord */
			car.x = 0.0;	/* temp */
			car.y = 0.0;	/* temp */
			error = plot_park_attempt(c, &car);
			if (error != 0) {
				ctx->state = TRAS_STATE_ERROR;
				return (error);
			}
		}
	} else {
		n = nbits >> 6;
		for (i = 0; i < n; i++) {
			car.x = plot_uint_to_component(be32toh(*p));
			p++;
			car.y = plot_uint_to_component(be32toh(*p));
			p++;
			error = plot_park_attempt(c, &car);
			if (error != 0) {
				ctx->state = TRAS_STATE_ERROR;
				return (error);
			}
		}
	}

	c->nbits += nbits;

	return (0);
}

/*
 * Finalize the parking lot test and determine its result.
 */
int
plot_final(struct tras_ctx *ctx)
{
	struct plot_ctx *c;
	double mean, var, s;
	double pvalue;

	if (ctx == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	
	c = ctx->context;

	if (c->tries < PARKING_LOT_MIN_CARS)
		return (EALREADY);

	mean = 3523.0;
	var = 21.9;

	s = ((double)c->ncars - mean) / var;
	s = s / sqrt((double)2.0);
	pvalue =  erfc(fabs(s));

	/* Determine and store results */
	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - PARKING_LOT_MIN_NBITS;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0.0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

/*
 * The parking lot test for update and finalize in one call.
 */
int
plot_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

/*
 * Restart the initialized parking lot test.
 */
int
plot_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

/*
 * Uninitialize the parking lot test and free its allocated resources.
 */
int
plot_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

/*
 * The parking lot test algorithm description for tras software.
 */
const struct tras_algo plot_algo = {
	.name =		"plot",
	.desc =		"The Parking Lot Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		plot_init,
	.update =	plot_update,
	.test =		plot_test,
	.final =	plot_final,
	.restart =	plot_restart,
	.free =		plot_free,
};

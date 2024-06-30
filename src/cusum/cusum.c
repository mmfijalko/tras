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
 * Cumulative Sums (Cusum) Test
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <math.h>

#include <tras.h>
#include <hamming8.h>
#include <utils.h>
#include <cdefs.h>
#include <const.h>
#include <cusum.h>


	#include <stdio.h>
/*
 * The table represent a absolute of minimum value for
 * random walk for each 8-bit sequence, where 0 represents
 * one step down and 1 represents one step up.
 */
static uint8_t cusum_mintab[256] = {
	8, 7, 6, 6, 6, 5, 5, 5, 6, 5, 4, 4, 4, 4, 4, 4,
	6, 5, 4, 4, 4, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3,
	6, 5, 4, 4, 4, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 2,
	4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	6, 5, 4, 4, 4, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 2,
	4, 3, 2, 2, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1,
	4, 3, 2, 2, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	6, 5, 4, 4, 4, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 2,
	4, 3, 2, 2, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1,
	4, 3, 2, 2, 2, 1, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0,
	2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 3, 2, 2, 2, 1, 1, 1, 2, 1, 0, 0, 0, 0, 0, 0,
	2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static uint8_t cusum_maxtab[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 1, 2, 2, 2, 3, 4,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 1, 2, 1, 1, 1, 2, 2, 2, 3, 4,
	1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 3, 4,
	2, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 4, 4, 4, 5, 6,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
	1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 3, 4,
	1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 3, 4,
	2, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 4, 4, 4, 5, 6,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4,
	2, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 4, 4, 4, 5, 6,
	3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 4, 4, 4, 5, 6,
	4, 4, 4, 4, 4, 4, 5, 6, 5, 5, 5, 6, 6, 6, 7, 8,
};

struct cusum_ctx {
	int		mins;		/* minimal sum so far */
	int		maxs;		/* maximum sum so far */
	int		sum;		/* sum for all subsequences */
	int		mode;		/* forward or backward direction */
	unsigned int	nbits;		/* number of bits processed */
	double		alpha;		/* significance level */
};

int
cusum_init(struct tras_ctx *ctx, void *params)
{
	struct cusum_params *p = params;
	struct cusum_ctx *c;
	int error;

	TRAS_CHECK_INIT(ctx);
	TRAS_CHECK_PARA(p, p->alpha);

	if (p->mode != CUSUM_MODE_FORWARD && p->mode != CUSUM_MODE_BACKWARD)
		return (EINVAL);

	error = tras_init_context(ctx, &cusum_algo, sizeof(struct cusum_ctx),
	    TRAS_F_ZERO);
	if (error != 0)
		return (error);
	c = ctx->context;

	c->mode = p->mode;
	c->alpha = p->alpha;

	return (0);
}

static int
cusum_update_forward(struct cusum_ctx *c, void *data, unsigned int nbits)
{
	uint8_t *p = (uint8_t *)data;
	unsigned int i, n;
	int mins, maxs;

	n = nbits >> 3;
	for (i = 0; i < n; i++, p++) {
		mins = cusum_mintab[*p];
		maxs = cusum_maxtab[*p];
		if (c->sum - mins < c->mins)
			c->mins = c->sum - mins;
		if (c->sum + maxs > c->maxs)
			c->maxs = c->sum + maxs;
		c->sum = c->sum + hamming8_norm[*p];
	}

	if (nbits & 0x07) {
		n = nbits & 0x07;
		for (i = 0; i < n; i++) {
			if (*p & (1 << (7 - i))) {
				if (c->sum == c->maxs)
					c->maxs++;
				c->sum++;
			} else {
				if (c->sum == c->mins)
					c->mins--;
				c->sum--;
			}
		}
	}
	c->nbits += nbits;

	return (0);
}

static int
cusum_update_bacward_contig(struct cusum_ctx *ctx, void *data, unsigned int nbits);

static int
cusum_update_backward(struct cusum_ctx *c, void *data, unsigned int nbits)
{
	unsigned int i, n, m;
	uint8_t *p, mask;

	if (c->nbits != 0)
		return (ENXIO);
	if (nbits < CUSUM_MIN_BITS)
		return (EBADMSG);

	/*
	 * The only one update call. We cannot run the sequence of the
	 * updates because the algorithm needs all bits at a clip.
	 */
	n = (nbits + 7) / 8;
	p = (uint8_t *)data + n - 1;
	n = nbits;

	while (n > 0) {
		if (n & 0x07) {
			m = n & 0x07;
			mask = 0x80 >> (m - 1);
//			mask = 0x80 >> m;
		} else {
			m = 8;
			mask = 0x01;
		}
		for (i = 0; i < m; i++, mask <<= 1) {
			if (*p & mask) {
				c->sum++;
				if (c->sum > c->maxs)
					c->maxs = c->sum;
			} else {
				c->sum--;
				if (c->sum < c->mins)
					c->mins = c->sum;
			}
		}
		n = n - m;
		p--;
	}
	c->nbits += nbits;
printf("%s: p = %p, data = %p\n", __func__, p, data);

cusum_update_bacward_contig(c, data, nbits);

	return (0);
}

#include <stdio.h>

#define	BIT(p, b)	\
	((p)[b / 8] & (0x80 >> ((b) & 0x07)))

static int
cusum_update_bacward_contig(struct cusum_ctx *ctx, void *data, unsigned int nbits)
{
	unsigned int i;
	uint8_t *p;
	int sum, delta;
	int smin, smax, z0, z1;

	p = (uint8_t *)data;

	delta = 0;
	sum = 0;

	delta = BIT(p, 0) ? 1 : -1;
	smin = smax = 0;

	for (i = 1; i < nbits; i++) {
		if (BIT(p, i)) {
			sum++;
			if (sum > smax)
				smax = sum;
			delta++;
		} else {
			sum--;
			if (sum < smin)
				smin = sum;
			delta--;
		}
	}

	printf("delta = %d, smin = %d, smax = %d\n", delta, smin, smax);

	z0 = smin;
	z1 = smax;
	smin = delta - z1;
	smax = delta - z0;

	printf("smin = %d, smax = %d\n", smin, smax);

	smin = abs(smin);
	smax = abs(smax);

	smax = max(smin, smax);

	printf("stats = %d\n", smax);

	return (0);
}

int
cusum_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct cusum_ctx *c;

	TRAS_CHECK_UPDATE(ctx, data, nbits);

	c = ctx->context;

	switch (c->mode) {
	case CUSUM_MODE_FORWARD:
		return (cusum_update_forward(ctx->context, data, nbits));
	case CUSUM_MODE_BACKWARD:
		return (cusum_update_backward(ctx->context, data, nbits));
	}
	return (ENXIO);
}

/*
 * Standard normal cumulative probability distribution function.
 */
#define	stdnorm_cpdf(x)	\
	((1.0 + erf((double)(x) / SQRT_2)) / 2.0)

int
cusum_final(struct tras_ctx *ctx)
{
	struct cusum_ctx *c;
	double pvalue, sum, sqrtn;
	int first, last, k, n, z;

	TRAS_CHECK_FINAL(ctx);

	c = ctx->context;

	if (c->nbits < CUSUM_MIN_BITS)
		return (EALREADY);

	z = (abs(c->mins) > c->maxs) ? abs(c->mins) : c->maxs;
printf("final stats = %d\n", z);
	z = (abs(c->mins) > abs(c->maxs)) ? abs(c->mins) : abs(c->maxs);
printf("final stats = %d\n", z);

	n = (int)c->nbits;
	sqrtn = sqrt(c->nbits);

	first = (-n / z + 1) / 4;
	last = (n / z - 1) / 4;
	for (k = first, sum = 0.0; k <= last; k++) {
		sum += stdnorm_cpdf((double)(4 * k + 1) * z / sqrtn);
		sum -= stdnorm_cpdf((double)(4 * k - 1) * z / sqrtn);
	}

	first = (-n / z - 3) / 4;
	last = (n / z - 1) / 4;
	for (k = first; k <= last; k++) {
		sum -= stdnorm_cpdf((double)(4 * k + 3) * z / sqrtn);
		sum += stdnorm_cpdf((double)(4 * k + 1) * z / sqrtn);
	}
	pvalue = 1.0 - sum;

	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.stats1 = (double)z;
	ctx->result.stats2 = sum;
	ctx->result.pvalue1 = pvalue;

	tras_fini_context(ctx, 0);

	return (0);
}

int
cusum_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

int
cusum_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

int
cusum_free(struct tras_ctx *ctx)
{

	tras_do_free(ctx);
}

const struct tras_algo cusum_algo = {
	.name =		"Cusum",
	.desc =		"Cumulative Sums Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		cusum_init,
	.update =	cusum_update,
	.test =		cusum_test,
	.final =	cusum_final,
	.restart =	cusum_restart,
	.free =		cusum_free,
};

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
 * The Squeeze Test.
 */

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tras.h>
#include <squeeze.h>

/*
 * The context structure for the squeeze test.
 */
struct squeeze_ctx {
	unsigned int	nbits;	/* number of bits processed */
	unsigned int	nint;	/* number of integers processed */
	unsigned int *	freqt;	/* frequency table for iterations */
	unsigned int	nfreq;	/* number of slots for chi-square */
	unsigned int	K;	/* max number of integers */
	double		alpha;	/* significance level for H0 */
};

/*
 * Initialize the squeeze test.
 */
int
squeeze_init(struct tras_ctx *ctx, void *params)
{
	struct squeeze_ctx *c;
	struct squeeze_params *p = params;
	size_t size;

	if (ctx == NULL || params == NULL)
		return (EINVAL);
	if (p->alpha <= 0.0 || p->alpha >= 1.0)
		return (EINVAL);
	if (p->K < SQUEEZE_MIN_NUMBERS || p->K > SQUEEZE_MAX_NUMBERS)
		return (EINVAL);
	if (ctx->state > TRAS_STATE_NONE)
		return (EINPROGRESS);

	size = sizeof(struct squeeze_ctx) + SQUEEZE_CHI_SQUARE_SLOTS *
	    sizeof(unsigned int);
	c = malloc(size);
	if (c == NULL) {
		ctx->state = TRAS_STATE_NONE;
		return (ENOMEM);
	}

	c->freqt = (unsigned int *)(c + 1);
	c->nfreq = SQUEEZE_CHI_SQUARE_SLOTS;
	c->nbits = 0;
	c->nint = 0;
	c->K = p->K;
	c->alpha = p->alpha;

	ctx->context = c;
	ctx->algo = &squeeze_algo;
	ctx->state = TRAS_STATE_INIT;

	return (0);
}

inline static double
squeeze_uint_to_floatU01(uint32_t u32)
{

	/* XXX: this is wrong function that need to be corrected
	 * because since u32 is uniformly distributed the
	 * return value is not.
	 */
	return ((double)u32 / pow(2.0, 32));
}

static unsigned int
squeeze_iterations(uint32_t u32)
{
	unsigned int k, i;
	double u;

	k = 1UL << 31;
	u = squeeze_uint_to_floatU01(u32);
	i = 0;

	while ((i < 48) && (k != 1)) {
		k = (unsigned int)ceil(u * k);
		i++;
	}
	return (i);
}

/*
 * Update state of the squeeze test with subsequent binary sequence.
 */
int
squeeze_update(struct tras_ctx *ctx, void *data, unsigned int nbits)
{
	struct squeeze_ctx *c;
	unsigned int *u, i, n;

	if (ctx == NULL || data == NULL)
		return (EINVAL);
	if (ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	if (nbits == 0 || (nbits & 0x1f))
		return (EINVAL);

	c = ctx->context;
	u = (unsigned int *)data;	/* TODO: consider endianism */
	n = nbits;

	while (n > 0) {
		if (c->nint >= c->K)
			break;
		i = squeeze_iterations(*u);
		i = (i <= 6) ? 0 : i - 6;
		c->freqt[i]++;
		c->nint++;
		u++;
		n -= 32;
	}
	c->nbits += nbits;

	return (0);
}

/*
 * Finalize the squeeze test and determine its result.
 */
int
squeeze_final(struct tras_ctx *ctx)
{
	struct squeeze_ctx *c;
	double pvalue, mean;

	if (ctx == NULL)
		return (EINVAL);
	c = ctx->context;
	if (c == NULL || ctx->state != TRAS_STATE_INIT)
		return (ENXIO);
	if (c->nint < c->K)
		return (EALREADY);

	pvalue = 0.0;

	/* Determine and store results */
	if (pvalue < c->alpha)
		ctx->result.status = TRAS_TEST_FAILED;
	else
		ctx->result.status = TRAS_TEST_PASSED;

	ctx->result.discard = c->nbits - c->K * sizeof(unsigned int) * 8;
	ctx->result.pvalue1 = pvalue;
	ctx->result.pvalue2 = 0.0;

	ctx->state = TRAS_STATE_FINAL;

	return (0);
}

/*
 * The squeeze test for update and finalize in one call.
 */
int
squeeze_test(struct tras_ctx *ctx, void *data, unsigned int nbits)
{

	return (tras_do_test(ctx, data, nbits));
}

/*
 * Restart the initialized squeeze test.
 */
int
squeeze_restart(struct tras_ctx *ctx, void *params)
{

	return (tras_do_restart(ctx, params));
}

/*
 * Uninitialize the squeeze test and free its allocated resources.
 */
int
squeeze_free(struct tras_ctx *ctx)
{

	return (tras_do_free(ctx));
}

/*
 * The squeeze test algorithm description for tras software.
 */
const struct tras_algo squeeze_algo = {
	.name =		"squeeze",
	.desc =		"Squeeze Test",
	.id =		0,
	.version = 	{ 0, 1, 1 },
	.init =		squeeze_init,
	.update =	squeeze_update,
	.test =		squeeze_test,
	.final =	squeeze_final,
	.restart =	squeeze_restart,
	.free =		squeeze_free,
};
